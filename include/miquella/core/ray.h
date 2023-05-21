#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include <iostream>

namespace miquella {

namespace core {

class Ray {
    public:
        Ray() {}
        Ray(const glm::vec3& origin, const glm::vec3& direction)
            : m_orig(origin), m_dir(direction)
        {}

        glm::vec3 origin() const  { return m_orig; }
        glm::vec3 direction() const { return m_dir; }

        glm::vec3 at(float t) const {
            return m_orig + t*m_dir;
        }

    public:
        glm::vec3 m_orig;
        glm::vec3 m_dir;
};

}

}
