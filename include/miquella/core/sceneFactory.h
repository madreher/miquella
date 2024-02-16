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

std::string to_string(SceneID id);



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
    std::tuple<std::shared_ptr<miquella::core::Scene>, std::shared_ptr<miquella::core::Camera>, miquella::core::Background > createThreeBalls() const;
    

    std::tuple<std::shared_ptr<miquella::core::Scene>, std::shared_ptr<miquella::core::Camera>, miquella::core::Background > createRandomBalls() const;
    

    std::tuple<std::shared_ptr<miquella::core::Scene>, std::shared_ptr<miquella::core::Camera>, miquella::core::Background > createRectangleLight() const;
    

    std::tuple<std::shared_ptr<miquella::core::Scene>, std::shared_ptr<miquella::core::Camera>, miquella::core::Background > createOneWeekend() const;
    

    std::tuple<std::shared_ptr<miquella::core::Scene>, std::shared_ptr<miquella::core::Camera>, miquella::core::Background > createLambertian() const;
    

    std::tuple<std::shared_ptr<miquella::core::Scene>, std::shared_ptr<miquella::core::Camera>, miquella::core::Background > createDielectric() const;
    

    std::tuple<std::shared_ptr<miquella::core::Scene>, std::shared_ptr<miquella::core::Camera>, miquella::core::Background > createEmptyCornel() const;
    

    std::tuple<std::shared_ptr<miquella::core::Scene>, std::shared_ptr<miquella::core::Camera>, miquella::core::Background > createSphereCornel() const;
    

}; // SceneFactory

} // core

} // miquella