#pragma once

#include <miquella/core/ray.h>

namespace miquella {

namespace core {

struct hitRecord {
    glm::vec3 p;
    glm::vec3 normal;
    float t;
    bool front_face;

    inline void setFaceNormal(const Ray& r, const glm::vec3& outward_normal) {
        front_face = glm::dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal :-outward_normal;
    }
};

class Object {

public:
    Object(){};

    virtual ~Object(){}

    virtual bool intersect(const Ray & r, float tmin, float tmax, hitRecord& record) = 0;
};

} // core

} // miquella
