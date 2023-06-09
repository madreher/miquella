#pragma once

#include <miquella/core/material.h>
#include <miquella/core/utility.h>

namespace miquella
{

namespace core
{

class DiffuseLight : public Material
{
public:
    DiffuseLight(const glm::vec3& color) : Material(), m_albedo(color){}

    virtual bool scatter(const Ray & incoming, const hitRecord& record, glm::vec3& color, Ray& out) const override
    {
        (void)incoming;
        (void)record;
        (void)color;
        (void)out;
        return false;
    }

    virtual glm::vec3 emitted() const override
    {
        return m_albedo;
    }

    virtual bool isLightSource() const override
    {
        return true;
    }

    virtual std::shared_ptr<Material> clone() override
    {
        return std::make_shared<DiffuseLight>(m_albedo);
    }



public:
    glm::vec3 m_albedo;

};

}   // core

}   // miquella
