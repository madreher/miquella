#pragma once

#include <memory>

#include <miquella/core/ray.h>
#include <miquella/core/hit.h>

#include <miquella/core/material.h>

namespace miquella {

namespace core {

class Object {

public:
    Object(){};
    Object(std::shared_ptr<Material> mat) : m_material(mat){}

    virtual ~Object(){}

    virtual bool intersect(const Ray & r, float tmin, float tmax, hitRecord& record) = 0;

    void setMaterial(std::shared_ptr<Material> material)
    {
        m_material = material;
    }

    virtual bool isLightSource() const
    {
        if(m_material)
            return m_material->isLightSource();

        return false;
    }

public:
    std::shared_ptr<Material> m_material;
};

} // core

} // miquella
