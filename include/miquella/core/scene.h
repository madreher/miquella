#pragma once

#include <vector>

#include <miquella/core/sphere.h>

namespace miquella {

namespace core {


class Scene
{
public:
    Scene(){}

    void addObject(std::shared_ptr<Object> obj)
    {
        m_objects.push_back(obj);

        if(obj->isLightSource())
            m_lights.push_back(obj);
    }

    bool intersect(const Ray & r, float tmin, float tmax, hitRecord& record)
    {
        hitRecord localRecord;
        bool hitFound = false;
        float currentMax = tmax;
        for(auto & obj : m_objects)
        {
            if(obj->intersect(r, tmin, currentMax, localRecord))
            {
                hitFound = true;
                currentMax = localRecord.t;
                record = localRecord;
            }
        }

        return hitFound;
    }

public:
    std::vector<std::shared_ptr<Object>> m_objects;

    std::vector<std::shared_ptr<Object>> m_lights;
};

} // core

} // miquella
