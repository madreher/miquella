#pragma once

#include <cmath>

#include <miquella/core/object.h>
#include <miquella/core/material.h>

namespace miquella {

namespace core {


class Sphere : public Object {

public:
    Sphere(glm::vec3 center, float r, std::shared_ptr<Material> mat) : Object(mat),
        m_center(center), m_r(r){};

    virtual std::shared_ptr<Object> clone() override
    {
        return std::make_shared<Sphere>(m_center,m_r, m_material->clone());
    }

    virtual bool intersect(const Ray & r, float tmin, float tmax, hitRecord& record) override;

public:
    glm::vec3 m_center;
    float m_r;
};

} // core

} // miquella
