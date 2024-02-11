#include <iostream>
#include <chrono>
#include <fstream>

#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#include <glbinding/Version.h>
#include <glbinding-aux/ContextInfo.h>
#include <glbinding-aux/types_to_string.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <SDL.h>

#include <lyra/lyra.hpp>

#include <cpr/cpr.h>

#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <miquella/core/io/ppm.h>

// Useful ressources:
// - https://github.com/retifrav/sdl-imgui-example
// - https://github.com/uysalaltas/Pixel-Engine/tree/main/Pixel
// - https://github.com/ThoSe1990/opengl_imgui
// - https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples#Example-for-OpenGL-users
// - https://github.com/CheerWizard/Gabriel
// - https://raytracing.github.io/books/RayTracingInOneWeekend.html
// - https://pbr-book.org/3ed-2018/contents


using namespace gl;

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
/*static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}*/



namespace ImGui
{

void HelpMarker(const char *desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

}

// Make the UI compact because there are so many fields
static void PushStyleCompact()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, static_cast<float>(static_cast<int>(style.FramePadding.y * 0.60f))));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, static_cast<float>(static_cast<int>(style.ItemSpacing.y * 0.60f))));
}

static void PopStyleCompact()
{
    ImGui::PopStyleVar(2);
}

struct JobSatus 
{
    std::string jobID;
    int sceneID;
    int nSamples;
    int freqOutput;
    int lastSample;
    std::string lastImage;
    std::string jobStatus;

};

std::string submitRenderingRequest(const std::string& server,
                            int port,
                            int sceneID,
                            int nSamples,
                            int freqOutput)
{
    std::string url = server + ":" + std::to_string(port) + "/submit";
    cpr::Response r = cpr::Post(cpr::Url{url},
        cpr::Parameters{
            {"sceneID", std::to_string(sceneID)},
            {"nSamples", std::to_string(nSamples)},
            {"freqOutput", std::to_string(freqOutput)}
    });

    if (r.status_code != 200)
    {
        //std::cerr<<"Error: unable to contact the controller."<<std::endl;
        spdlog::warn("Unable to contact the controller, job not submitted.");
        return "";
    }
    else
    {
        spdlog::info("Job accepted by controller with ID {}", r.text);
        return r.text;
    }
}

std::tuple<std::string, int, std::string> lastSampleRequest(const std::string& server,
                            int port,
                            const std::string& jobID)
{
    // Create an HTTP request.
    std::string url = server + ":" + std::to_string(port) + "/requestLastLocalSample";
    cpr::Response r = cpr::Get(cpr::Url{url},
        cpr::Parameters{{"jobID", jobID}});

    if (r.status_code != 200)
    {
        spdlog::warn("Unable to contact the controller, unable to query for the last frame.");
        return {"", 0, ""};
    }

    // Parsing the response
    json data = json::parse(r.text);
    if(data.count("error") == 1)
    {
        spdlog::warn("Received error from controller: {}", data["error"].get<std::string>());
        return {"", 0, ""};
    }
    else if(data.count("image") == 1 && data.count("lastSample") == 1 && data.count("status") == 1)
    {
        return { 
            data["image"].get<std::string>(), 
            data["lastSample"].get<int>(),
            data["status"].get<std::string>() };
        
    }
    else
    {
        spdlog::warn("Error: unable to parse the response from the controller when querrying from the last sample.");
        return {"", 0, ""};
    }
}

bool fullListOfJobsRequest(const std::string& server, int port, std::vector<JobSatus>& jobList)
{
    // Create an HTTP request.
    std::string url = server + ":" + std::to_string(port) + "/requestAllJobs";
    cpr::Response r = cpr::Get(cpr::Url{url});

    if (r.status_code != 200)
    {
        spdlog::warn("Unable to contact the controller, unable to query for the last frame.");
        return false;
    }

    // Parsing the response
    json data = json::parse(r.text);

    for(auto && job : data["jobs"])
    {
        // Dev note: Switch this to emplace_back when moving to c++20 standart
        jobList.push_back({job["jobID"], job["sceneID"], job["nSamples"], job["freqOutput"], job["lastSample"], job["lastImage"], job["status"]});
    }
    return true;
}

bool cancelJobRequest(const std::string& server, int port, std::string& jobID)
{
    // Create an HTTP request.
    std::string url = server + ":" + std::to_string(port) + "/cancelJob";
    cpr::Response r = cpr::Post(cpr::Url{url},
        cpr::Parameters{{"jobID", jobID}});

    if (r.status_code != 200)
    {
        spdlog::warn("Unable to contact the controller, unable to query for the last frame.");
        return false;
    }

    // Parsing the response
    json data = json::parse(r.text);
    if(data.count("error") > 0)
    {
        spdlog::warn("Received error from controller: {}", data["error"].get<std::string>());
        return false;
    }
    else
    {
        spdlog::info("JobID {} has been cancelled.", jobID);
        return true;
    }
}


int main(int argc, char** argv)
{
    std::string preloadPath;
    std::string loglvl = "info";

    auto cli = lyra::cli()
        | lyra::opt( loglvl, "loglvl")
            ["--loglvl"]
            ("Log level to apply. info (default), warn, critical, debug")
        | lyra::opt( preloadPath, "preloadpath" )
            ["--image"]
            ("Path to a PPM image to load.");

    auto result = cli.parse( { argc, argv } );
    if ( !result )
    {
        spdlog::critical("Unable to parse the command line: {}.", result.errorMessage());
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

    // Image
    miquella::core::io::PPMImage image;
    
    if(preloadPath.size() > 0)
    {
        std::ifstream file;
        file.open(preloadPath);
        image = miquella::core::io::readPPM(file);
        if(image.image.size() == 0)
        {
            spdlog::critical("Error while loading image {}. Abording.", preloadPath);
            return -1;
        }
    }


    int windowWidth = 1200;
    int windowHeight = 800;

    // ---------------------- SDL Setup----------------------------------

    // SDL Context creation
    SDL_Init( SDL_INIT_VIDEO );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 6 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    SDL_Window * window = SDL_CreateWindow( "", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight,  SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == NULL)
    {
        spdlog::critical("Failed to create a GL context: {}", SDL_GetError());
        return -1;
    }
    SDL_GL_MakeCurrent(window, gl_context);

    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    // enable VSync
    SDL_GL_SetSwapInterval(1);

    // ---------------------- GLBinding Setup ----------------------------------

    // Initializing glbinding
    glbinding::initialize([](const char* name) { return reinterpret_cast<glbinding::ProcAddress>(SDL_GL_GetProcAddress(name)); });

    spdlog::info("OpenGL renderer: {}", glGetString(GL_RENDERER));

    spdlog::info("OpenGL Version: {}.{}", glbinding::aux::ContextInfo::version().majorVersion(), glbinding::aux::ContextInfo::version().minorVersion());

    // ---------------------- Imgui Setup ----------------------------------

    ImGui::CreateContext();
    auto & io = ImGui::GetIO();
    io.FontDefault = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf", 14);

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    ImGui::StyleColorsDark();

    // ---------------------- Ray tracing time ----------------------------------
    // Create a picture on CPU side

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same
    
    // ImGui settings
    int sceneID = 0;
    bool ret = false;
    int maxSamples = 1000;
    int freqOutput = 50;
    std::string serverURL = "http://localhost";
    int port = 8000;
    std::string jobID;
    int lastSample = 0;
    bool autoRetrieve = false;
    int refreshRate = 5;
    auto lastAutoSampleRetrieve = std::chrono::system_clock::now();
    auto lastJoblistRetrieve = std::chrono::system_clock::now();
    std::string jobStatus = "";

    std::vector<JobSatus> jobs;


    // ---------------------- Event loop handling ----------------------------------

    SDL_Event event;
    bool quit = false;
    while(!quit)
    {

        ImGui_ImplOpenGL3_NewFrame();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        while( SDL_PollEvent( &event ) )
        {
            ImGui_ImplSDL2_ProcessEvent(&event);

            switch( event.type )
            {
                case SDL_WINDOWEVENT:
                {
                    if(event.window.event == SDL_WINDOWEVENT_RESIZED ||
                       event.window.event == SDL_WINDOWEVENT_MAXIMIZED ||
                       event.window.event == SDL_WINDOWEVENT_MINIMIZED)
                    {
                        windowWidth = event.window.data1;
                        windowHeight = event.window.data2;

                        glViewport(0, 0,windowWidth, windowHeight);
                    }
                    break;
                }
                case SDL_QUIT:
                {
                    quit = true;
                    break;
                }
            }
        }

        // start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        // standard demo window
        //bool show_demo_window = true;
        //ImGui::ShowDemoWindow(&show_demo_window);

        ImGui::Begin("Job Table");
        {
            static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;
            PushStyleCompact();
            //ImGui::CheckboxFlags("ImGuiTableFlags_Resizable", &flags, ImGuiTableFlags_Resizable);
            //ImGui::CheckboxFlags("ImGuiTableFlags_Reorderable", &flags, ImGuiTableFlags_Reorderable);
            //ImGui::CheckboxFlags("ImGuiTableFlags_Hideable", &flags, ImGuiTableFlags_Hideable);
            //ImGui::CheckboxFlags("ImGuiTableFlags_NoBordersInBody", &flags, ImGuiTableFlags_NoBordersInBody);
            //ImGui::CheckboxFlags("ImGuiTableFlags_NoBordersInBodyUntilResize", &flags, ImGuiTableFlags_NoBordersInBodyUntilResize); ImGui::SameLine(); ImGui::HelpMarker("Disable vertical borders in columns Body until hovered for resize (borders will always appear in Headers)");
            PopStyleCompact();
            if (ImGui::BeginTable("jobs", 8, flags))
            {
                ImGui::TableSetupColumn("JobID");
                ImGui::TableSetupColumn("Status");
                ImGui::TableSetupColumn("Max samples");
                ImGui::TableSetupColumn("Last sample");
                ImGui::TableSetupColumn("Last image");
                ImGui::TableSetupColumn("Scene ID");
                ImGui::TableSetupColumn("Output freq");
                ImGui::TableSetupColumn("Action");
                ImGui::TableHeadersRow();

                for(size_t i = 0; i < jobs.size(); ++i)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", jobs[i].jobID.c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", jobs[i].jobStatus.c_str());
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%i", jobs[i].nSamples);
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%i", jobs[i].lastSample);
                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%s", jobs[i].lastImage.c_str());
                    ImGui::TableSetColumnIndex(5);
                    ImGui::Text("%i", jobs[i].sceneID);
                    ImGui::TableSetColumnIndex(6);
                    ImGui::Text("%i", jobs[i].freqOutput);
                    ImGui::TableSetColumnIndex(7);
                    
                    // Create a unique id for the buttons
                    std::string buttonLabel = "Cancel##cancelid" + std::to_string(i);
                    if (ImGui::Button(buttonLabel.c_str()))
                    {
                        auto check = cancelJobRequest(serverURL, port, jobs[i].jobID);

                        // Cancel as well the auto retrieve if it was the same jobID
                        // Doing it here so that it does't require to parse the job table 
                        // at every update of the table.
                        if(check && jobID.compare(jobs[i].jobID) == 0)
                        {
                            spdlog::debug("Canceling auto retrieve of {} as the job has been canceled.", jobID);
                            autoRetrieve = false;
                        }
                    }
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();

        // Menu to submit a rendering job
        ImGui::Begin("Rendering Execution");
        {
            // Scene ID
            {
                const char* items[] = { "3 Balls", "Random balls", "Rectangle Light", "OneWeekend", "Lambertien test", "Dielectric", "Empty Cornel", "Sphere Cornel"};
                ImGui::Combo("Scene Selection", &sceneID, items, IM_ARRAYSIZE(items));
                ImGui::SameLine(); ImGui::HelpMarker(
                    "Scene selection model.");
            }

            // Max samples
            {
                ImGui::Text("Total number of samples");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1); // so we dont have a label
                ret |= ImGui::InputInt("samples",  &maxSamples);
                ImGui::PopItemWidth();
            }

            // Output freq
            {
                ImGui::Text("Output frequency");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1); // so we dont have a label
                ret |= ImGui::InputInt("freq",  &freqOutput);
                ImGui::PopItemWidth();
            }

            // Server
            {
                ImGui::Text("Server adress");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1); // so we dont have a label
                ret |= ImGui::InputText("addr", &serverURL);
                ImGui::PopItemWidth();
            }

            // Port
            {
                ImGui::Text("Port");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1); // so we dont have a label
                ret |= ImGui::InputInt("port",  &port);
                ImGui::PopItemWidth();
            }

            if (ImGui::Button("Submit"))
            {
                jobID = submitRenderingRequest(serverURL,
                            port,
                            sceneID,
                            maxSamples,
                            freqOutput);
            }

            // Job ID info
            {
                ImGui::Text("Job ID");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1); // so we dont have a label
                ret |= ImGui::InputText("jobID",  &jobID);
                ImGui::PopItemWidth();
            }

            // Job Status
            {
                ImGui::Text("Job Status");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1); // so we dont have a label
                ret |= ImGui::InputText("jobStatus",  &jobStatus);
                ImGui::PopItemWidth();
            }

            // Last sample
            {
                ImGui::Text("Last Sample");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1); // so we dont have a label
                ret |= ImGui::InputInt("lastSample",  &lastSample);
                ImGui::PopItemWidth();
            }

            // Retrieve last sample
            if (ImGui::Button("Retrive last sample"))
            {
                auto [ filePath, sample, status ] = lastSampleRequest(serverURL, port, jobID);
                lastSample = sample;
                jobStatus = status;
                if(filePath.size() > 0)
                {
                    std::ifstream file;
                    file.open(filePath);
                    image = miquella::core::io::readPPM(file);
                    spdlog::debug("Loading sample {} from file {}", lastSample, filePath);
                }
            }

            // Auto retrieve
            {
                ImGui::Text("Auto retrieve"); ImGui::SameLine();
                ImGui::HelpMarker("Periodically automatically request the last samples available."); ImGui::SameLine();
                ImGui::PushItemWidth(-1); // so we dont have a label
                ImGui::Checkbox("##autoRetrieve", &autoRetrieve);
                ImGui::PopItemWidth();
            }
        }
        ImGui::End();

        if(autoRetrieve && jobID.size() > 0)
        {
            auto now = std::chrono::system_clock::now();
            auto timeElapsed = std::chrono::duration<double>( now - lastAutoSampleRetrieve);
            
            if(timeElapsed.count() > static_cast<double>(refreshRate))
            {
                auto [ filePath, sample, status ] = lastSampleRequest(serverURL, port, jobID);
                lastSample = sample;
                jobStatus = status;
                if(filePath.size() > 0)
                {
                    std::ifstream file;
                    file.open(filePath);
                    image = miquella::core::io::readPPM(file);
                    spdlog::debug("Loading sample {} from file {}", lastSample, filePath);
                }

                if(status == "COMPLETED")
                {
                    // Disabling the auto retrieve
                    spdlog::info("Last frame of the job received. Disabling Auto Retrieve.");
                    autoRetrieve = false;
                }

                lastAutoSampleRetrieve = std::chrono::system_clock::now();
            }
        }

        // Retrieve the job list
        auto now = std::chrono::system_clock::now();
        auto timeElapsed = std::chrono::duration<double>(now - lastJoblistRetrieve);
        if(timeElapsed.count() > static_cast<double>(refreshRate))
        {
            jobs.clear();
            if(fullListOfJobsRequest(serverURL, port, jobs))
            {
                spdlog::debug("Received a list of {} jobs.", jobs.size());
            }
            else
            {
                spdlog::warn("Failed to receive the list of jobs from the controller.");
            }
            lastJoblistRetrieve = std::chrono::system_clock::now();
        }

        if(image.image.size() > 0)
        {
            // Upload pixels into texture
            #if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            #endif
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.w, image.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.image.data());

            ImGui::Begin("Renderer");
            ImGui::Text("size = %d x %d, sample %d", image.w, image.h, lastSample);
            ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(image_texture)), ImVec2(static_cast<float>(image.w), static_cast<float>(image.h)));
            ImGui::End();
        }

        // rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
