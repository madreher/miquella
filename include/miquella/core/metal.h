#pragma once

#include <miquella/core/material.h>
#include <miquella/core/utility.h>

namespace miquella
{

namespace core
{

class Metal : public Material
{

public:
    Metal(const glm::vec3& color, float fuzz) : Material(), m_albedo(color), m_fuzz(fuzz){}

    virtual bool scatter(const Ray & incoming, const hitRecord& record, glm::vec3& color, Ray& out) const override
    {
        glm::vec3 reflected = _reflect(incoming.direction(), record.normal);
        out = Ray(record.p, reflected);
        color = m_albedo;
        return glm::dot(out.direction(), record.normal) > 0.f;
    }

private:
    glm::vec3 _reflect(const glm::vec3& in, const glm::vec3& normal) const
    {
        return in - 2.f*glm::dot(in, normal) * normal;
    }

public:
    glm::vec3 m_albedo;
    float m_fuzz;
};

} // core

} // miquella
