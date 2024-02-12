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

#include <miquella/core/ray.h>
#include <miquella/core/simpleCamera.h>
#include <miquella/core/lookAtCamera.h>
#include <miquella/core/scene.h>
#include <miquella/core/renderer.h>
#include <miquella/core/rendererThreads.h>
#include <miquella/core/utility.h>
#include <miquella/core/lambertian.h>
#include <miquella/core/metal.h>
#include <miquella/core/dielectric.h>
#include <miquella/core/diffuseLight.h>
#include <miquella/core/rectangle.h>

#include <cpr/cpr.h>

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

void generateScene1(miquella::core::Renderer& renderer)
{
    std::shared_ptr<miquella::core::Scene> scene = std::make_shared<miquella::core::Scene>();
    auto groundMat = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.8f, 0.8f, 0.0f));
    auto sphereMat = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.7f, 0.3f, 0.3f));
    auto sphereMatLeft = std::make_shared<miquella::core::Metal>(glm::vec3(0.8f, 0.8f,0.8f), 0.3f);
    auto sphereMatRight = std::make_shared<miquella::core::Metal>(glm::vec3(0.8f, 0.6f,0.2f), 1.f);

    auto groundSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(0,-100.5,-1), 100.f, groundMat);
    auto centerSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(0,0,-1), 0.5f, sphereMat);
    auto leftSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(-1.0, 0.0, -1.0), 0.5f, sphereMatLeft);
    auto rightSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(1.0, 0.0, -1.0), 0.5f, sphereMatRight);

    auto sphereLightMat = std::make_shared<miquella::core::DiffuseLight>(glm::vec3(1.f, 1.f, 1.f));
    auto lightSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(0,1.5,-1), 0.5f, sphereLightMat);

    scene->addObject(groundSphere);
    scene->addObject(centerSphere);
    scene->addObject(leftSphere);
    scene->addObject(rightSphere);
    scene->addObject(lightSphere);

    const auto aspectRatio = 16.0f / 9.0f;
    //std::shared_ptr<miquella::core::SimpleCamera> camera = std::make_shared<miquella::core::SimpleCamera>();
    //camera->setAspectRatio(aspectRatio, 1080);

    glm::vec3 lookFrom = {3.f, 3.f, 2.f};
    glm::vec3 lookAt = {0.f, 0.f, -1.f};
    std::shared_ptr<miquella::core::LookAtCamera> camera = std::make_shared<miquella::core::LookAtCamera>(
                lookFrom,
                lookAt,
                glm::vec3{0.f, 1.f, 0.f},
                20.f,
                aspectRatio,
                2.f,
                glm::distance(lookFrom, lookAt),
                1080);




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

    for(size_t i = 0; i< 20; ++i)
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
        auto sphere = std::make_shared<miquella::core::Sphere>(glm::vec3(x,y,z), 0.5f, mat);
        scene->addObject(sphere);
    }

    const auto aspectRatio = 16.0f / 9.0f;
    //std::shared_ptr<miquella::core::SimpleCamera> camera = std::make_shared<miquella::core::SimpleCamera>();
    //camera->setAspectRatio(aspectRatio, 1080);

    glm::vec3 lookFrom = {10.f, 10.f, 10.f};
    glm::vec3 lookAt = {0.f, 0.f, 0.f};
    std::shared_ptr<miquella::core::LookAtCamera> camera = std::make_shared<miquella::core::LookAtCamera>(
                lookFrom,
                lookAt,
                glm::vec3{0.f, 1.f, 0.f},
                20.f,
                aspectRatio,
                2.f,
                glm::distance(lookFrom, lookAt),
                1080);


    renderer.setCamera(camera);
    renderer.setScene(scene);
}

void generateScene3(miquella::core::Renderer& renderer)
{
    std::shared_ptr<miquella::core::Scene> scene = std::make_shared<miquella::core::Scene>();
    auto groundMat = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.8f, 0.8f, 0.0f));
    auto sphereMat = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.7f, 0.3f, 0.3f));

    auto groundSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(0,-1000.f,0), 1000.f, groundMat);
    auto centerSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(0,2,0), 2.f, sphereMat);

    auto difflight = std::make_shared<miquella::core::DiffuseLight>(glm::vec3(4,4,4));
    auto lightRect = std::make_shared<miquella::core::xyRectangle>(3.f, 5.f, 1.f, 3.f, -2.f, difflight);
    auto lightSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(0,7,0), 2.f, difflight);

    scene->addObject(groundSphere);
    scene->addObject(centerSphere);
    scene->addObject(lightRect);
    scene->addObject(lightSphere);

    const auto aspectRatio = 16.0f / 9.0f;
    //std::shared_ptr<miquella::core::SimpleCamera> camera = std::make_shared<miquella::core::SimpleCamera>();
    //camera->setAspectRatio(aspectRatio, 1080);

    glm::vec3 lookFrom = {26.f, 3.f, 6.f};
    glm::vec3 lookAt = {0.f, 2.f, 0.f};
    std::shared_ptr<miquella::core::LookAtCamera> camera = std::make_shared<miquella::core::LookAtCamera>(
                lookFrom,
                lookAt,
                glm::vec3{0.f, 1.f, 0.f},
                15.f,
                aspectRatio,
                2.f,
                glm::distance(lookFrom, lookAt),
                1080);

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
                    sphereMaterial = std::make_shared<miquella::core::Dielectric>(1.5f);
                    auto sphere = std::make_shared<miquella::core::Sphere>(center, 0.2f, sphereMaterial);
                    scene->addObject(sphere);
                }
            }
        }
    }

    //auto material1 = make_shared<dielectric>(1.5);
    //world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));
    auto material1 = std::make_shared<miquella::core::Dielectric>(1.5f);
    auto sphere1 = std::make_shared<miquella::core::Sphere>(glm::vec3(0.f, 1.f, 0.f), 1.0f, material1);
    scene->addObject(sphere1);

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
                glm::distance(lookFrom, lookAt),
                1080);


    renderer.setScene(scene);
    renderer.setCamera(camera);
    renderer.setBackground(miquella::core::Background::GRADIANT);
}

// Diffuse light test
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
    camera->setAspectRatio(aspectRatio, 1080);

    renderer.setScene(scene);
    renderer.setCamera(camera);
    renderer.setBackground(miquella::core::Background::GRADIANT);
}


// Dieletric material test
void generateScene6(miquella::core::Renderer& renderer)
{
    std::shared_ptr<miquella::core::Scene> scene = std::make_shared<miquella::core::Scene>();
    auto groundMat = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.8f, 0.8f, 0.0f));
    auto sphereMat = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.1f, 0.2f, 0.5f));
    //auto sphereMatLeft = std::make_shared<miquella::core::Metal>(glm::vec3(0.8f, 0.8f,0.8f), 0.3f);
    //auto sphereMat = std::make_shared<miquella::core::Dielectric>(1.5);
    auto sphereMatLeft = std::make_shared<miquella::core::Dielectric>(1.5f);

    auto sphereMatRight = std::make_shared<miquella::core::Metal>(glm::vec3(0.8f, 0.6f,0.2f), 0.f);

    auto groundSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(0,-100.5,-1), 100.f, groundMat);
    auto centerSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(0,0,-1), 0.5f, sphereMat);
    auto leftSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(-1.0, 0.0, -1.0), 0.5f, sphereMatLeft);
    auto rightSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(1.0, 0.0, -1.0), 0.5f, sphereMatRight);

    //auto sphereLightMat = std::make_shared<miquella::core::DiffuseLight>(glm::vec3(1.f, 1.f, 1.f));
    //auto lightSphere = std::make_shared<miquella::core::Sphere>(glm::vec3(0,1.5,-1), 0.5, sphereLightMat);

    scene->addObject(groundSphere);
    scene->addObject(centerSphere);
    scene->addObject(leftSphere);
    scene->addObject(rightSphere);
    //scene->addObject(lightSphere);

    const auto aspectRatio = 16.0f / 9.0f;
    std::shared_ptr<miquella::core::SimpleCamera> camera = std::make_shared<miquella::core::SimpleCamera>();
    camera->setAspectRatio(aspectRatio, 1080);

    /*glm::vec3 lookFrom = {3.f, 3.f, 2.f};
    glm::vec3 lookAt = {0.f, 0.f, -1.f};
    std::shared_ptr<miquella::core::LookAtCamera> camera = std::make_shared<miquella::core::LookAtCamera>(
                lookFrom,
                lookAt,
                glm::vec3{0.f, 1.f, 0.f},
                20.f,
                aspectRatio,
                2.f,
                glm::distance(lookFrom, lookAt),
                1080);*/




    renderer.setCamera(camera);
    renderer.setScene(scene);
    renderer.setBackground(miquella::core::Background::GRADIANT);
}

void generateEmptyCornell(miquella::core::Renderer& renderer)
{
    std::shared_ptr<miquella::core::Scene> scene = std::make_shared<miquella::core::Scene>();

    auto red = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.65f, 0.05f, 0.05f));
    auto white = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.73f, 0.73f, 0.73f));
    auto green = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.12f, 0.45f, 0.15f));
    auto light = std::make_shared<miquella::core::DiffuseLight>(glm::vec3(15.f, 15.f, 15.f));

    auto left = std::make_shared<miquella::core::yzRectangle>(0.f, 555.f, 0.f, 555.f, 555.f, green);
    auto right = std::make_shared<miquella::core::yzRectangle>(0.f, 555.f, 0.f, 555.f, 0.f, red);
    auto lightO = std::make_shared<miquella::core::xzRectangle>(213.f, 343.f, 227.f, 332.f, 554.f, light);
    //auto lightO = std::make_shared<miquella::core::xzRectangle>(0.f, 555.f, 0.f, 555.f, 554.f, light);
    auto floor = std::make_shared<miquella::core::xzRectangle>(0.f, 555.f, 0.f, 555.f, 0.f, white);
    auto roof = std::make_shared<miquella::core::xzRectangle>(0.f, 555.f, 0.f, 555.f, 555.f, white);
    auto back = std::make_shared<miquella::core::xyRectangle>(0.f, 555.f, 0.f, 555.f, 555.f, white);

    scene->addObject(left);
    scene->addObject(right);
    scene->addObject(lightO);
    scene->addObject(floor);
    scene->addObject(roof);
    scene->addObject(back);

    const auto aspectRatio = 1.f;
    glm::vec3 lookFrom = {278.f, 278.f, -800.f};
    glm::vec3 lookAt = {278.f, 278.f, 0.f};
    std::shared_ptr<miquella::core::LookAtCamera> camera = std::make_shared<miquella::core::LookAtCamera>(
                lookFrom,
                lookAt,
                glm::vec3{0.f, 1.f, 0.f},
                40.f,
                aspectRatio,
                2.f,
                glm::distance(lookFrom, lookAt),
                600);

    renderer.setCamera(camera);
    renderer.setScene(scene);
    renderer.setBackground(miquella::core::Background::BLACK);
}

void generateGlassCornell(miquella::core::Renderer& renderer)
{
    std::shared_ptr<miquella::core::Scene> scene = std::make_shared<miquella::core::Scene>();

    auto red = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.65f, 0.05f, 0.05f));
    auto white = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.73f, 0.73f, 0.73f));
    auto green = std::make_shared<miquella::core::Lambertian>(glm::vec3(0.12f, 0.45f, 0.15f));
    auto light = std::make_shared<miquella::core::DiffuseLight>(glm::vec3(15.f, 15.f, 15.f));
    auto glassLeftMat = std::make_shared<miquella::core::Dielectric>(1.9f);
    auto glassRightMat = std::make_shared<miquella::core::Dielectric>(1.5f);

    auto left = std::make_shared<miquella::core::yzRectangle>(0.f, 555.f, 0.f, 555.f, 555.f, green);
    auto right = std::make_shared<miquella::core::yzRectangle>(0.f, 555.f, 0.f, 555.f, 0.f, red);
    auto lightO = std::make_shared<miquella::core::xzRectangle>(213.f, 343.f, 227.f, 332.f, 554.f, light);
    //auto lightO = std::make_shared<miquella::core::xzRectangle>(0.f, 555.f, 0.f, 555.f, 554.f, light);
    auto floor = std::make_shared<miquella::core::xzRectangle>(0.f, 555.f, 0.f, 555.f, 0.f, white);
    auto roof = std::make_shared<miquella::core::xzRectangle>(0.f, 555.f, 0.f, 555.f, 555.f, white);
    auto back = std::make_shared<miquella::core::xyRectangle>(0.f, 555.f, 0.f, 555.f, 555.f, white);
    auto glassLeft = std::make_shared<miquella::core::Sphere>(glm::vec3(430.f, 100.f, 350.f), 100.f, glassLeftMat);
    auto glassRight = std::make_shared<miquella::core::Sphere>(glm::vec3(120.f, 100.f, 200.f), 100.f, glassRightMat);

    scene->addObject(left);
    scene->addObject(right);
    scene->addObject(lightO);
    scene->addObject(floor);
    //scene->addObject(roof);
    scene->addObject(back);
    scene->addObject(glassLeft);
    scene->addObject(glassRight);

    const auto aspectRatio = 1.f;
    glm::vec3 lookFrom = {278.f, 278.f, -800.f};
    glm::vec3 lookAt = {278.f, 278.f, 0.f};
    std::shared_ptr<miquella::core::LookAtCamera> camera = std::make_shared<miquella::core::LookAtCamera>(
                lookFrom,
                lookAt,
                glm::vec3{0.f, 1.f, 0.f},
                40.f,
                aspectRatio,
                2.f,
                glm::distance(lookFrom, lookAt),
                600);

    renderer.setCamera(camera);
    renderer.setScene(scene);
    renderer.setBackground(miquella::core::Background::BLACK);
}

void runRenderer(std::vector<void (*)(miquella::core::Renderer&)>& scenes,
                size_t sceneID, 
                bool remote,
                size_t maxSamples,
                size_t outputFrequency,
                const std::string& jobID,
                const std::string& serverURL,
                int port)
{
    miquella::core::RendererThreads renderer;
    scenes[sceneID](renderer);

    std::string serverAddr = serverURL + ":" + std::to_string(port);
    if(remote)
        serverAddr += "/updateRemoteJobExec";
    else
        serverAddr += "/updateLocalJobExec";

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
                // IMPORTANT: the part name "file" must match the parameter name in the 
                // controller function!
                cpr::Response r = cpr::Post(cpr::Url{"http://localhost:8000/updateRemoteJobExec"},
                cpr::Multipart{
                    {"file", cpr::File{absPath.string()}},
                    {"jobID", jobID},
                    {"lastSample", std::to_string(i)}
                    });

                spdlog::debug("Update remote server return code: {}", r.status_code);
            }
            else 
            {
                // Notify the controller that we have a new sample image
                cpr::Response r = cpr::Post(cpr::Url{serverAddr},
                cpr::Parameters{
                    {"jobID", jobID},
                    {"filePath", absPath.string()},
                    {"lastSample", std::to_string(i)}
                    });  
                

                // Check the status of the job 
                if(r.status_code == 200)
                {
                    json data = json::parse(r.text);
                    if(data.count("status") == 0)
                    {
                        spdlog::warn("Unable to parse the status when sending a sample update to the local controller. Response: {}", r.text);
                    }
                    else if(data["status"].get<std::string>().compare("RUNNING") != 0)
                    {
                        spdlog::info("Job status changed to {}, stopping the job loop.", data["status"].get<std::string>());
                        break;
                    }
                }
                else 
                {
                    spdlog::debug("Update local server return code: {}", r.status_code);
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

    std::vector<void (*)(miquella::core::Renderer&)> scenes;
    scenes.push_back(generateScene1);
    scenes.push_back(generateScene2);
    scenes.push_back(generateScene3);
    scenes.push_back(generateScene4);
    scenes.push_back(generateScene5);
    scenes.push_back(generateScene6);
    scenes.push_back(generateEmptyCornell);
    scenes.push_back(generateGlassCornell);

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
            ("Port to use to contact the controller.");

    auto result = cli.parse( { argc, argv } );
    if ( !result )
    {
        spdlog::critical("Unable to parse the command line: {}.", result.errorMessage());
        exit(1);
    }

    if(sceneID >= scenes.size())
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
        std::string url = serverURL + ":" + std::to_string(port) + "/requestJob";
        cpr::Response r = cpr::Post(cpr::Url{url});

        //spdlog::debug("Return code: {}", r.status_code);
        //spdlog::debug("Body: {}", r.text);

        if(r.status_code != 200)
        {
            //std::cerr<<"Error while querying for a job. Abording."<<std::endl;
            //return 0;
            spdlog::warn("Unable to contact the controller.");
            std::this_thread::sleep_for(std::chrono::seconds(5));
            continue;
        }

        // Parsing the text
        json data = json::parse(r.text);
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
            runRenderer(scenes, sceneID, remote, maxSamples, outputFrequency, jobID, serverURL, port);
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
