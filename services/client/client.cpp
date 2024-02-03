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

#include <cpprest/filestream.h>
#include <cpprest/http_client.h>

using namespace utility;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

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

/* Can pass proxy information via environment variable http_proxy.
   Example:
   Linux:   export http_proxy=http://192.1.8.1:8080
 */
web::http::client::http_client_config client_config_for_proxy()
{
    web::http::client::http_client_config client_config;
#ifdef _WIN32
    wchar_t* pValue = nullptr;
    std::unique_ptr<wchar_t, void (*)(wchar_t*)> holder(nullptr, [](wchar_t* p) { free(p); });
    size_t len = 0;
    auto err = _wdupenv_s(&pValue, &len, L"http_proxy");
    if (pValue) holder.reset(pValue);
    if (!err && pValue && len)
    {
        std::wstring env_http_proxy_string(pValue, len - 1);
#else
    if (const char* env_http_proxy = std::getenv("http_proxy"))
    {
        std::string env_http_proxy_string(env_http_proxy);
#endif
        if (env_http_proxy_string == U("auto"))
            client_config.set_proxy(web::web_proxy::use_auto_discovery);
        else
            client_config.set_proxy(web::web_proxy(env_http_proxy_string));
    }

    return client_config;
}

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

std::string submitRenderingRequest(const std::string& server,
                            int port,
                            int sceneID,
                            int nSamples,
                            int freqOutput)
{
    // Create an HTTP request.
    // Encode the URI query since it could contain special characters like spaces.
    std::string url = server + ":" + std::to_string(port) + "/";
    http_client client(url, client_config_for_proxy());

    auto uri = web::uri_builder(U("/submit"));
    uri.append_query("sceneID=" + std::to_string(sceneID));
    uri.append_query("nSamples=" + std::to_string(nSamples));
    uri.append_query("freqOutput=" + std::to_string(freqOutput));
    auto response = client.request(methods::POST, uri.to_string());

    try
    {
        std::cout<<"Return code: "<<response.get().status_code()<<std::endl;
        std::string jobID = response.get().extract_string().get();
        std::cout<<"Body: "<<jobID<<std::endl;
        return jobID;
    }
    catch(std::exception& e)
    {
        std::cerr<<"Error encountered while try to submit a job."<<std::endl;
        std::cerr<<e.what();
        return "";
    }
}

std::tuple<std::string, int, std::string> lastSampleRequest(const std::string& server,
                            int port,
                            const std::string& jobID)
{
    // Create an HTTP request.
    // Encode the URI query since it could contain special characters like spaces.
    std::string url = server + ":" + std::to_string(port) + "/";
    http_client client(url, client_config_for_proxy());

    auto uri = web::uri_builder(U("/requestLastLocalSample"));
    uri.append_query("jobID=" + jobID);
    auto response = client.request(methods::GET, uri.to_string());

    try
    {
        auto r = response.get();
        std::cout<<"Return code: "<<r.status_code()<<std::endl;
        auto obj = r.extract_json().get();
        std::cout<<"Body: "<<obj.serialize()<<std::endl;

        if(obj.has_string_field("error"))
        {
            std::cerr<<"Error while requesting sample."<<std::endl;
            std::cerr<<obj["error"].as_string()<<std::endl;
            return {"", 0, ""};
        }

        return { 
            obj["image"].as_string(), 
            obj["lastSample"].as_integer(),
            obj["status"].as_string() };
    }
    catch(std::exception& e)
    {
        std::cerr<<"Error encountered while try to submit a job."<<std::endl;
        std::cerr<<e.what();
        return {"", 0, ""};
    }
}


int main(int argc, char** argv)
{
    std::string preloadPath;

    auto cli = lyra::cli()
        | lyra::opt( preloadPath, "preloadpath" )
            ["--image"]
            ("Path to a PPM image to load.");

    auto result = cli.parse( { argc, argv } );
    if ( !result )
    {
        std::cerr << "Error in command line: " << result.errorMessage() << std::endl;
        exit(1);
    }



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
            std::cerr<<"Error while loading image "<<preloadPath<<". Abording."<<std::endl;
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
        std::cerr << "[ERROR] Failed to create a GL context: "
                  << SDL_GetError() << std::endl;
        return -1;
    }
    SDL_GL_MakeCurrent(window, gl_context);

    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    // enable VSync
    SDL_GL_SetSwapInterval(1);

    // ---------------------- GLBinding Setup ----------------------------------

    // Initializing glbinding
    glbinding::initialize([](const char* name) { return reinterpret_cast<glbinding::ProcAddress>(SDL_GL_GetProcAddress(name)); });
    std::cout << "[INFO] OpenGL renderer: "
              << glGetString(GL_RENDERER)
              << std::endl;

    std::cout << "[INFO] OpenGL Version: " << glbinding::aux::ContextInfo::version() << std::endl;

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
    auto lastRetrieve = std::chrono::system_clock::now();
    std::string jobStatus = "";


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
            auto timeElapsed = std::chrono::duration<double>( now - lastRetrieve);
            
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
                }

                if(status == "COMPLETED")
                {
                    // Disabling the auto retrieve
                    std::cout<<"Last frame of the job received. Disabling Auto Retrieve."<<std::endl;
                    autoRetrieve = false;
                }

                lastRetrieve = std::chrono::system_clock::now();
            }
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
