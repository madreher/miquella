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

#include <miquella/core/ray.h>
#include <miquella/core/simpleCamera.h>
#include <miquella/core/lookAtCamera.h>
#include <miquella/core/scene.h>
#include <miquella/core/renderer.h>
#include <miquella/core/utility.h>
#include <miquella/core/lambertian.h>
#include <miquella/core/metal.h>
#include <miquella/core/diffuseLight.h>

// Useful ressources:
// - https://github.com/retifrav/sdl-imgui-example
// - https://github.com/uysalaltas/Pixel-Engine/tree/main/Pixel
// - https://github.com/ThoSe1990/opengl_imgui
// - https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples#Example-for-OpenGL-users
// - https://github.com/CheerWizard/Gabriel

using namespace gl;

int main() {

    // Image

    int windowWidth = 800;
    int windowHeight = 600;

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

    // ---------------------- Scene setup ----------------------------------
    std::shared_ptr<miquella::core::Scene> scene = std::make_shared<miquella::core::Scene>();
    auto groundMat = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.8f, 0.8f, 0.0f));
    auto sphereMat = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.7f, 0.3f, 0.3f));

    auto sphereMatLeft = std::make_shared<miquella::core::Metal>(glm::vec3(0.8f, 0.8f,0.8f), 1.f);
    auto sphereMatRight = std::make_shared<miquella::core::Metal>(glm::vec3(0.8f, 0.6f,0.2f), 1.f);
    scene->addSphere(glm::vec3(0,0,-1), 0.5, sphereMat);

    scene->addSphere(glm::vec3(0,-100.5,-1), 100, groundMat);
    scene->addSphere(glm::vec3(-1.0, 0.0, -1.0), 0.5, sphereMatLeft);
    scene->addSphere(glm::vec3(1.0, 0.0, -1.0), 0.5, sphereMatRight);

    auto sphereLight = std::make_shared<miquella::core::DiffuseLight>(glm::vec3(1.f, 1.f, 1.f));
    scene->addSphere(glm::vec3(0,1.5,-1), 0.5, sphereLight);

    const auto aspectRatio = 16.0f / 9.0f;
    //std::shared_ptr<miquella::core::SimpleCamera> camera = std::make_shared<miquella::core::SimpleCamera>();

    glm::vec3 lookFrom = {3.f, 3.f, 2.f};
    glm::vec3 lookAt = {0.f, 0.f, -1.f};
    std::shared_ptr<miquella::core::LookAtCamera> camera = std::make_shared<miquella::core::LookAtCamera>(
                lookFrom,
                lookAt,
                glm::vec3{0.f, 1.f, 0.f},
                20.f,
                aspectRatio,
                2.f,
                glm::distance(lookFrom, lookAt));



    // ---------------------- Ray tracing time ----------------------------------
    miquella::core::Renderer renderer;
    renderer.setCamera(camera);
    renderer.setScene(scene);

    // Create a picture on CPU side

    int imageWidth = 600;
    int imageHeight = static_cast<int>(static_cast<float>(imageWidth) / aspectRatio);

    renderer.setImageResolution(imageWidth, imageHeight);

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

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

        // Compute the image
        renderer.render();

        // Upload pixels into texture
        #if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        #endif
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, renderer.getImagePointer());


        // start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        // standard demo window
        bool show_demo_window = true;
        ImGui::ShowDemoWindow(&show_demo_window);

        ImGui::Begin("OpenGL Texture Text");
        ImGui::Text("size = %d x %d", imageWidth, imageHeight);
        ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(image_texture)), ImVec2(static_cast<float>(imageWidth), static_cast<float>(imageHeight)));
        ImGui::End();

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
