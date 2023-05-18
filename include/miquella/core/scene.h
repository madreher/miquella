#pragma once

#include <vector>

#include <miquella/core/sphere.h>

namespace miquella {

namespace core {


class Scene
{
public:
    Scene(){}

    void addSphere(const glm::vec3 & center, float radius, std::shared_ptr<Material> mat)
    {
        spheres.emplace_back(center, radius, mat);
    }

    bool intersect(const Ray & r, float tmin, float tmax, hitRecord& record)
    {
        hitRecord localRecord;
        bool hitFound = false;
        float currentMax = tmax;
        for(auto & sphere : spheres)
        {
            if(sphere.intersect(r, tmin, currentMax, localRecord))
            {
                hitFound = true;
                currentMax = localRecord.t;
                record = localRecord;
            }
        }

        return hitFound;
    }

public:
    std::vector<Sphere> spheres;
};

} // core

} // miquella
