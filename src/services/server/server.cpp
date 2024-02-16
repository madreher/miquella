#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>
#include <map>

#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <glbinding/Version.h>
#include <glbinding-aux/ContextInfo.h>
#include <glbinding-aux/types_to_string.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <SDL.h>

#include <lyra/lyra.hpp>

#include <miquella/core/renderer.h>
#include <miquella/core/rendererThreads.h>
#include <miquella/core/utility.h>
#include <miquella/core/sceneFactory.h>

#include <miquella/http/http.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;



// Useful ressources:
// - https://github.com/retifrav/sdl-imgui-example
// - https://github.com/uysalaltas/Pixel-Engine/tree/main/Pixel
// - https://github.com/ThoSe1990/opengl_imgui
// - https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples#Example-for-OpenGL-users
// - https://github.com/CheerWizard/Gabriel
// - https://raytracing.github.io/books/RayTracingInOneWeekend.html
// - https://pbr-book.org/3ed-2018/contents


using namespace gl;

void runRenderer(
                size_t sceneID, 
                bool remote,
                size_t maxSamples,
                size_t outputFrequency,
                const std::string& jobID,
                const std::string& serverURL,
                int port,
                int nbThreads)
{
    miquella::core::SceneFactory sceneFactory;
    auto [ scene, camera, background ] = sceneFactory.createScene(miquella::core::SceneID(sceneID));
    
    miquella::core::RendererThreads renderer(scene, camera, static_cast<uint32_t>(nbThreads));
    //renderer.setScene(scene);
    //renderer.setCamera(camera);
    renderer.setBackground(background);
    renderer.setNbThreads(nbThreads);

    for(size_t i = 1; i <= maxSamples; ++i)
    {
        // Compute the image
        renderer.render();

        if(i % outputFrequency == 0)
        {
            std::stringstream fileName;
            fileName<<"scene"<<sceneID<<"_sample"<<i<<".ppm";
            std::filesystem::path sampleImage(fileName.str());
            auto absPath = std::filesystem::absolute(sampleImage);
            renderer.writeToPPM(absPath.string());
            spdlog::debug("Sample {} saved to file {}.", i, absPath.string());

            // Manual method with cppRestsdk, didn't work
            // source: https://stackoverflow.com/questions/56497375/cpprestsdk-how-to-post-multipart-data
            // Switching to CPR
            if(remote)
            {
                auto [returnCode, content] = miquella::http::uploadJobToRemoteController(serverURL, port, absPath, jobID, i);
                spdlog::debug("Update remote server return code: {}", returnCode);
            }
            else 
            {
                // Notify the controller that we have a new sample image
                auto [ returnCode, text ] = miquella::http::uploadJobToLocalController(absPath, jobID, i);
                if(returnCode == 200)
                {
                    json data = json::parse(text);
                    if(data.count("status") == 0)
                    {
                        spdlog::warn("Unable to parse the status when sending a sample update to the local controller. Response: {}", text);
                    }
                    else if(data["status"].get<std::string>().compare("RUNNING") != 0)
                    {
                        spdlog::info("Job status changed to {}, stopping the job loop.", data["status"].get<std::string>());
                        break;
                    }
                }
                else 
                {
                    spdlog::debug("Update local server return code: {}", returnCode);
                }
            }
        }
    }
}

int main(int argc, char** argv)
{

    size_t sceneID = 3;

    size_t maxSamples = 1000;
    size_t outputFrequency = 20;
    bool remote = false;
    std::string jobID;
    std::string loglvl = "info";
    std::string serverURL = "http://localhost";
    int port = 8000;
    int nbThreads = 1;

    auto cli = lyra::cli()
        | lyra::opt( sceneID, "sceneid" )
            ["--scene-id"]
            ("0: 3 balls, 1: random balls, 2: rectangle light, 3: RaytracingOneWeekend, 4: Lambertien test, 5: Dieletric test, 6: Empty cornel, 7: Glass cornel")
        | lyra::opt( maxSamples, "maxsamples" )
            ["--maxSamples"]
            ("Total number of samples to generate on the image.")
        | lyra::opt( outputFrequency, "freq" )
            ["--freq"]
            ("Output image frequency")
        | lyra::opt( remote )
            ["-r"]["--remote"]
            ("Query a remote controller to get a rendering job" )
        | lyra::opt( loglvl, "loglvl")
            ["--loglvl"]
            ("Log level to apply. info (default), warn, critical, debug")
        | lyra::opt( serverURL, "controllerurl" )
            ["--controller"]
            ("URL of the controller to contact to query rendering jobs.")
        | lyra::opt( port, "port" )
            ["-p"]["--port"]
            ("Port to use to contact the controller.")
        | lyra::opt( nbThreads, "nthreads" )
            ["--nthreads"]
            ("Number of threads to use by the renderer.");

    auto result = cli.parse( { argc, argv } );
    if ( !result )
    {
        spdlog::critical("Unable to parse the command line: {}.", result.errorMessage());
        exit(1);
    }

    if(miquella::core::SceneID(sceneID) >= miquella::core::SceneID::MAX_NB_SCENE)
    {
        spdlog::critical("Scene ID does not exist ({}).", sceneID);
        exit(1);
    }

    // Setting up the logging level
    std::map<std::string, spdlog::level::level_enum> loglvlTable {
        {"info", spdlog::level::info},
        {"debug", spdlog::level::debug},
        {"trace", spdlog::level::trace},
        {"warn", spdlog::level::warn},
        {"crit", spdlog::level::critical}
    };
    if(loglvlTable.count(loglvl) > 0)
    {
        spdlog::set_level(loglvlTable[loglvl]);
        spdlog::info("Setting logging level to {}.", loglvl);
    }
    else
        spdlog::info("Unrecognized log level. Using info by default.");

    srand(static_cast<unsigned int>(time(nullptr)));

    // ---------------------- Ray tracing time ----------------------------------
    
    while(1)
    {

        auto [ returnCode, text ] = miquella::http::requestJob(serverURL, port);

        if(returnCode != 200)
        {
            spdlog::warn("Unable to contact the controller.");
            std::this_thread::sleep_for(std::chrono::seconds(5));
            continue;
        }

        // Parsing the text
        json data = json::parse(text);
        if(data.empty())
        {
            spdlog::debug("No job available on the controller.");
            std::this_thread::sleep_for(std::chrono::seconds(5));
            continue;
        }
        else if(data.count("jobID") > 0)
        {
            jobID = data.at("jobID").get<std::string>();
            sceneID = data.at("sceneID").get<size_t>();
            maxSamples = data.at("nSamples").get<size_t>();
            outputFrequency = data.at("freqOutput").get<size_t>();
            spdlog::info("Rendering job {} received from the controller.", jobID);

            auto start = std::chrono::steady_clock::now();
            // Rendering the scene
            runRenderer(sceneID, remote, maxSamples, outputFrequency, jobID, serverURL, port, nbThreads);
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed(end - start);

            spdlog::info("Rendering Job {} completed in {}s.", jobID, elapsed.count());
        }
        else
        {
            spdlog::critical("jobID not found in the request job response. Aborting.");
            return 0;
        }

    }
    
        
    return 0;
}
