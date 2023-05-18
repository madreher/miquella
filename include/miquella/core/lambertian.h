#pragma once

#include <miquella/core/material.h>
#include <miquella/core/utility.h>

namespace miquella
{

namespace core
{

class Lambertian : public Material
{
public:
    Lambertian(const glm::vec3& color) : Material(), m_albedo(color){}

    virtual bool scatter(const Ray & incoming, const hitRecord& record, glm::vec3& color, Ray& out) const override
    {
        (void)incoming;
        auto direction = record.normal + randomHemisphereVec3(record.normal);

        // Catch degenerate case (See RayTracingInOneWeekend Section 9.3)
        if(nearZeroVec3(direction))
            direction = record.normal;

        out = Ray(record.p, direction);
        color = m_albedo;

        return true;
    }


public:
    glm::vec3 m_albedo;

};

}   // core

}   // miquella
