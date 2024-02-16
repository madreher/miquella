#pragma once

#include <vector>

#include <miquella/core/sphere.h>

namespace miquella {

namespace core {


class Scene
{
public:
    Scene(){}

    virtual ~Scene(){}

    void addObject(std::shared_ptr<Object> obj);

    bool intersect(const Ray & r, float tmin, float tmax, hitRecord& record);

    virtual std::shared_ptr<Scene> clone();

public:
    std::vector<std::shared_ptr<Object>> m_objects;

    std::vector<std::shared_ptr<Object>> m_lights;
};

} // core

} // miquella
