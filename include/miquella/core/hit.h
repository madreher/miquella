#pragma once

#include <miquella/core/ray.h>

#include <memory>

namespace miquella
{

namespace core
{

class Material; // forward declaration to avoid circular dependency

struct hitRecord {
    glm::vec3 p;
    glm::vec3 normal;
    float t;
    bool front_face;
    std::shared_ptr<Material> material;

    inline void setFaceNormal(const Ray& r, const glm::vec3& outward_normal) {
        front_face = glm::dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal :-outward_normal;
    }
};

} // core

} // miquella
