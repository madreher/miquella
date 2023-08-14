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


    // Create an HTTP request.
    // Encode the URI query since it could contain special characters like spaces.
    http_client client(U("http://localhost:8000/"), client_config_for_proxy());
    auto response = client.request(methods::POST, uri_builder(U("/submit")).append_query("sceneID=1").to_string());
    std::cout<<"Return code: "<<response.get().status_code()<<std::endl;
    std::cout<<"Body: "<<response.get().extract_string().get()<<std::endl;
    


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

        if(image.image.size() > 0)
        {
            // Upload pixels into texture
            #if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            #endif
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.w, image.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.image.data());

            ImGui::Begin("Renderer");
            ImGui::Text("size = %d x %d", image.w, image.h);
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
