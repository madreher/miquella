#pragma once

#include <miquella/core/material.h>
#include <miquella/core/utility.h>
#include <algorithm>

namespace miquella
{

namespace core
{

class Metal : public Material
{

public:
    Metal(const glm::vec3& color, float fuzz) : Material(), m_albedo(color)
    {
        m_fuzz = std::clamp(fuzz, 0.f, 1.f);
    }

    virtual bool scatter(const Ray & incoming, const hitRecord& record, glm::vec3& color, Ray& out) const override;

    virtual std::shared_ptr<Material> clone() override;

private:
    glm::vec3 _reflect(const glm::vec3& in, const glm::vec3& normal) const;

public:
    glm::vec3 m_albedo;
    float m_fuzz;
};

} // core

} // miquella
