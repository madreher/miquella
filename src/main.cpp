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
#include <miquella/core/rectangle.h>

// Useful ressources:
// - https://github.com/retifrav/sdl-imgui-example
// - https://github.com/uysalaltas/Pixel-Engine/tree/main/Pixel
// - https://github.com/ThoSe1990/opengl_imgui
// - https://github.com/ocornut/imgui/wiki/Image-Loading-and-Displaying-Examples#Example-for-OpenGL-users
// - https://github.com/CheerWizard/Gabriel


using namespace gl;


void generateScene1(miquella::core::Renderer& renderer)
{
    std::shared_ptr<miquella::core::Scene> scene = std::make_shared<miquella::core::Scene>();
    auto groundMat = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.8f, 0.8f, 0.0f));
    auto sphereMat = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.7f, 0.3f, 0.3f));
    auto sphereMatLeft = std::make_shared<miquella::core::Metal>(glm::vec3(0.8f, 0.8f,0.8f), 0.3f);
    auto sphereMatRight = std::make_shared<miquella::core::Metal>(glm::vec3(0.8f, 0.6f,0.2f), 1.f);

    auto groundSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(0,-100.5,-1), 100.f, groundMat);
    auto centerSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(0,0,-1), 0.5, sphereMat);
    auto leftSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(-1.0, 0.0, -1.0), 0.5, sphereMatLeft);
    auto rightSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(1.0, 0.0, -1.0), 0.5, sphereMatRight);

    auto sphereLightMat = std::make_shared<miquella::core::DiffuseLight>(glm::vec3(1.f, 1.f, 1.f));
    auto lightSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(0,1.5,-1), 0.5, sphereLightMat);

    scene->addObject(groundSphere);
    scene->addObject(centerSphere);
    scene->addObject(leftSphere);
    scene->addObject(rightSphere);
    scene->addObject(lightSphere);

    const auto aspectRatio = 16.0f / 9.0f;
    //std::shared_ptr<miquella::core::SimpleCamera> camera = std::make_shared<miquella::core::SimpleCamera>();
    //camera->setAspectRatio(aspectRatio);

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




    renderer.setCamera(camera);
    renderer.setScene(scene);
}

void generateScene2(miquella::core::Renderer& renderer)
{
    std::shared_ptr<miquella::core::Scene> scene = std::make_shared<miquella::core::Scene>();

    auto sphereLight = std::make_shared<miquella::core::DiffuseLight>(glm::vec3(1.f, 1.f, 1.f));
    auto lightRadius = 1.0f;
    auto light = std::make_shared<miquella::core::Sphere>(glm::vec3(0,0,0), lightRadius, sphereLight);
    scene->addObject(light);

    for(size_t i = 0; i< 10; ++i)
    {
        auto x = miquella::core::randomFloat(-3.0f, 3.0f);
        auto y = miquella::core::randomFloat(-3.0f, 3.0f);
        auto z = miquella::core::randomFloat(-3.0f, 3.0f);

        // Prevent spheres in the light source
        while((x < lightRadius && x > -lightRadius)  ||
              (y < lightRadius && y > -lightRadius)  ||
              (z < lightRadius && z > -lightRadius))
        {
            x = miquella::core::randomFloat(-3.0f, 3.0f);
            y = miquella::core::randomFloat(-3.0f, 3.0f);
            z = miquella::core::randomFloat(-3.0f, 3.0f);
        }

        auto r = miquella::core::randomFloat(0.0f, 1.0f);
        auto g = miquella::core::randomFloat(0.0f, 1.0f);
        auto b = miquella::core::randomFloat(0.0f, 1.0f);

        auto mat = std::make_shared<miquella::core::Lambertian>(glm::vec3(r, g, b));
        std::cout<<"Sphere "<<i<<" ("<<x<<","<<y<<","<<z<<")"<<std::endl;
        auto sphere = std::make_shared<miquella::core::Sphere>(glm::vec3(x,y,z), 0.5, mat);
        scene->addObject(sphere);
    }

    const auto aspectRatio = 16.0f / 9.0f;
    //std::shared_ptr<miquella::core::SimpleCamera> camera = std::make_shared<miquella::core::SimpleCamera>();

    glm::vec3 lookFrom = {6.f, 6.f, 6.f};
    glm::vec3 lookAt = {0.f, 0.f, 0.f};
    std::shared_ptr<miquella::core::LookAtCamera> camera = std::make_shared<miquella::core::LookAtCamera>(
                lookFrom,
                lookAt,
                glm::vec3{0.f, 1.f, 0.f},
                20.f,
                aspectRatio,
                2.f,
                glm::distance(lookFrom, lookAt));


    renderer.setCamera(camera);
    renderer.setScene(scene);
}

void generateScene3(miquella::core::Renderer& renderer)
{
    std::shared_ptr<miquella::core::Scene> scene = std::make_shared<miquella::core::Scene>();
    auto groundMat = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.8f, 0.8f, 0.0f));
    auto sphereMat = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.7f, 0.3f, 0.3f));

    auto groundSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(0,-1000.f,0), 1000.f, groundMat);
    auto centerSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(0,2,0), 2, sphereMat);

    auto difflight = std::make_shared<miquella::core::DiffuseLight>(glm::vec3(4,4,4));
    auto lightRect = std::make_shared<miquella::core::xyRectangle>(3, 5, 1, 3, -2, difflight);
    auto lightSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(0,7,0), 2, difflight);

    scene->addObject(groundSphere);
    scene->addObject(centerSphere);
    scene->addObject(lightRect);
    scene->addObject(lightSphere);

    const auto aspectRatio = 16.0f / 9.0f;
    //std::shared_ptr<miquella::core::SimpleCamera> camera = std::make_shared<miquella::core::SimpleCamera>();

    glm::vec3 lookFrom = {26.f, 3.f, 6.f};
    glm::vec3 lookAt = {0.f, 2.f, 0.f};
    std::shared_ptr<miquella::core::LookAtCamera> camera = std::make_shared<miquella::core::LookAtCamera>(
                lookFrom,
                lookAt,
                glm::vec3{0.f, 1.f, 0.f},
                15.f,
                aspectRatio,
                2.f,
                glm::distance(lookFrom, lookAt));

    renderer.setScene(scene);
    renderer.setCamera(camera);
}

void generateScene4(miquella::core::Renderer& renderer)
{
    std::shared_ptr<miquella::core::Scene> scene = std::make_shared<miquella::core::Scene>();
    auto groundMaterial = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.5f, 0.5f, 0.5f));
    auto groundSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(0.f,-1000.f,0.f), 1000.f, groundMaterial);
    scene->addObject(groundSphere);

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = miquella::core::randomFloat();
            glm::vec3 center(static_cast<float>(a) + 0.9f*miquella::core::randomFloat(), 0.2f, static_cast<float>(b) + 0.9f*miquella::core::randomFloat());

            if (glm::length(center - glm::vec3(4.f, 0.2f, 0.f)) > 0.9f) {
                std::shared_ptr<miquella::core::Material> sphereMaterial;

                if (choose_mat < 0.8f) {
                    // diffuse
                    auto albedo = miquella::core::randomColor() * miquella::core::randomColor();
                    sphereMaterial = std::make_shared<miquella::core::Lambertian>(albedo);
                    auto sphere = std::make_shared<miquella::core::Sphere>(center, 0.2f, sphereMaterial);
                    scene->addObject(sphere);
                } else if (choose_mat < 0.95f) {
                    // metal
                    auto albedo = miquella::core::randomColor(0.5f, 1.f);
                    auto fuzz = miquella::core::randomFloat(0.f, 0.5f);
                    sphereMaterial = std::make_shared<miquella::core::Metal>(albedo, fuzz);
                    auto sphere = std::make_shared<miquella::core::Sphere>(center, 0.2f, sphereMaterial);
                    scene->addObject(sphere);
                } else {
                    // TODO create glass
                    // metal
                    auto albedo = miquella::core::randomColor(0.5f, 1.f);
                    auto fuzz = miquella::core::randomFloat(0.f, 0.5f);
                    sphereMaterial = std::make_shared<miquella::core::Metal>(albedo, fuzz);
                    auto sphere = std::make_shared<miquella::core::Sphere>(center, 0.2f, sphereMaterial);
                    scene->addObject(sphere);
                }
            }
        }
    }

    //auto material1 = make_shared<dielectric>(1.5);
    //world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.4f, 0.2f, 0.1f));
    auto sphere2 = std::make_shared<miquella::core::Sphere>(glm::vec3(-4.f, 1.f, 0.f), 1.0f, material2);
    scene->addObject(sphere2);

    auto material3 = std::make_shared<miquella::core::Metal>(glm::vec3(0.7f, 0.6f, 0.5f), 0.0f);
    auto sphere3 = std::make_shared<miquella::core::Sphere>(glm::vec3(4.f, 1.f, 0.f), 1.0f, material3);
    scene->addObject(sphere3);

    const auto aspectRatio = 16.0f / 9.0f;
    glm::vec3 lookFrom = {13.f, 2.f, 3.f};
    glm::vec3 lookAt = {0.f, 0.f, 0.f};
    std::shared_ptr<miquella::core::LookAtCamera> camera = std::make_shared<miquella::core::LookAtCamera>(
                lookFrom,
                lookAt,
                glm::vec3{0.f, 1.f, 0.f},
                20.f,
                aspectRatio,
                2.f,
                glm::distance(lookFrom, lookAt));


    renderer.setScene(scene);
    renderer.setCamera(camera);
}
void generateScene5(miquella::core::Renderer& renderer)
{
    std::shared_ptr<miquella::core::Scene> scene = std::make_shared<miquella::core::Scene>();

    auto matDiffuse = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.5f, 0.5f, 0.5f));
    auto sphereGround = std::make_shared<miquella::core::Sphere>(glm::vec3(0.f, -100.5f, -1.f), 100.f, matDiffuse);
    auto sphereSurface = std::make_shared<miquella::core::Sphere>(glm::vec3(0.f, 0.f, -1.f), 0.5f, matDiffuse);
    scene->addObject(sphereGround);
    scene->addObject(sphereSurface);

    const auto aspectRatio = 16.0f / 9.0f;
    std::shared_ptr<miquella::core::SimpleCamera> camera = std::make_shared<miquella::core::SimpleCamera>();
    camera->setAspectRatio(aspectRatio);

    renderer.setScene(scene);
    renderer.setCamera(camera);
    renderer.setBackground(miquella::core::Background::GRADIANT);
}

int main() {

    srand(static_cast<unsigned int>(time(nullptr)));

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
    miquella::core::Renderer renderer;
    //generateScene1(renderer);
    //generateScene2(renderer);
    //generateScene3(renderer);
    //generateScene4(renderer);
    generateScene5(renderer);

    // ---------------------- Ray tracing time ----------------------------------
    // Create a picture on CPU side

    int imageWidth = 1080;
    const auto aspectRatio = 16.0f / 9.0f;
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
