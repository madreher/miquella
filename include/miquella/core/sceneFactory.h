#pragma once 

#include <map>
#include <cstdint>

#include <miquella/core/simpleCamera.h>
#include <miquella/core/lookAtCamera.h>
#include <miquella/core/scene.h>
#include <miquella/core/lambertian.h>
#include <miquella/core/metal.h>
#include <miquella/core/dielectric.h>
#include <miquella/core/diffuseLight.h>
#include <miquella/core/rectangle.h>

namespace miquella {

namespace core {

enum class SceneID : uint8_t
{
    SCENE_THREE_BALLS = 0,
    SCENE_RANDOM_BALLS = 1,
    SCENE_RECTANGLE_LIGHT = 2,
    SCENE_ONE_WEEKEND = 3,
    SCENE_LAMBERTIEN = 4,
    SCENE_DIELECTRIC = 5,
    SCENE_EMPTY_CORNEL = 6,
    SCENE_SPHERE_CORNEL = 7,
    MAX_NB_SCENE = 8
};

std::string to_string(SceneID id)
{
    switch(id)
    {
        case SceneID::SCENE_THREE_BALLS:        return "SCENE_THREE_BALLS";
        case SceneID::SCENE_RANDOM_BALLS:       return "SCENE_RANDOM_BALLS";
        case SceneID::SCENE_RECTANGLE_LIGHT:    return "SCENE_RECTANGLE_LIGHT";
        case SceneID::SCENE_ONE_WEEKEND:        return "SCENE_ONE_WEEKEND";
        case SceneID::SCENE_LAMBERTIEN:         return "SCENE_LAMBERTIEN";
        case SceneID::SCENE_DIELECTRIC:         return "SCENE_DIELECTRIC";
        case SceneID::SCENE_EMPTY_CORNEL:       return "SCENE_EMPTY_CORNEL";
        case SceneID::SCENE_SPHERE_CORNEL:      return "SCENE_SPHERE_CORNEL";
        default: return "";
    }
}

class SceneFactory
{
public:
    SceneFactory()
    {

    }

    ~SceneFactory(){}

    std::tuple<std::shared_ptr<miquella::core::Scene>, std::shared_ptr<miquella::core::Camera>, miquella::core::Background > createScene(SceneID id) const
    {
        switch(id)
        {
            case SceneID::SCENE_THREE_BALLS:
                return createThreeBalls();
            case SceneID::SCENE_RANDOM_BALLS:
                return createRandomBalls();
            case SceneID::SCENE_RECTANGLE_LIGHT:
                return createRectangleLight();
            case SceneID::SCENE_ONE_WEEKEND:
                return createOneWeekend();
            case SceneID::SCENE_LAMBERTIEN:
                return createLambertian();
            case SceneID::SCENE_DIELECTRIC:
                return createDielectric();
            case SceneID::SCENE_EMPTY_CORNEL:
                return createEmptyCornel();
            case SceneID::SCENE_SPHERE_CORNEL:
                return createSphereCornel();
            default:
                return createThreeBalls();
        }
    }

private:
    std::tuple<std::shared_ptr<miquella::core::Scene>, std::shared_ptr<miquella::core::Camera>, miquella::core::Background > createThreeBalls() const
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

        return { scene, camera, miquella::core::Background::BLACK };
    }

    std::tuple<std::shared_ptr<miquella::core::Scene>, std::shared_ptr<miquella::core::Camera>, miquella::core::Background > createRandomBalls() const
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
            //std::cout<<"Sphere "<<i<<" ("<<x<<","<<y<<","<<z<<")"<<std::endl;
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

        return { scene, camera, miquella::core::Background::BLACK };
    }

    std::tuple<std::shared_ptr<miquella::core::Scene>, std::shared_ptr<miquella::core::Camera>, miquella::core::Background > createRectangleLight() const
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

        return { scene, camera, miquella::core::Background::BLACK };
    }

    std::tuple<std::shared_ptr<miquella::core::Scene>, std::shared_ptr<miquella::core::Camera>, miquella::core::Background > createOneWeekend() const
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

        return { scene, camera, miquella::core::Background::GRADIANT };
    }

    std::tuple<std::shared_ptr<miquella::core::Scene>, std::shared_ptr<miquella::core::Camera>, miquella::core::Background > createLambertian() const
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

        return { scene, camera, miquella::core::Background::GRADIANT };
    }

    std::tuple<std::shared_ptr<miquella::core::Scene>, std::shared_ptr<miquella::core::Camera>, miquella::core::Background > createDielectric() const
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

        return { scene, camera, miquella::core::Background::GRADIANT };        
    }

    std::tuple<std::shared_ptr<miquella::core::Scene>, std::shared_ptr<miquella::core::Camera>, miquella::core::Background > createEmptyCornel() const
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

        return { scene, camera, miquella::core::Background::BLACK };
    }

    std::tuple<std::shared_ptr<miquella::core::Scene>, std::shared_ptr<miquella::core::Camera>, miquella::core::Background > createSphereCornel() const
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

        return { scene, camera, miquella::core::Background::BLACK };
    }

}; // SceneFactory

} // core

} // miquella