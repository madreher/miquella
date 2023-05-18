#pragma once

#include <random>

#include <glm/glm.hpp>

namespace miquella {

namespace core {

const float pi = 3.1415926535897932385f;

inline float degreeToRadians(float degrees)
{
    return degrees * pi / 180.0f;
}

inline float randomFloat(float minValue = 0.0f, float maxValue = 1.0f)
{
    static std::uniform_real_distribution<float> distribution(minValue, maxValue);
    static std::mt19937 generator;
    return distribution(generator);
}


// Generate a random point in a sphere and normalize it if acceptable
inline glm::vec3 randomUnitVec3()
{
    while(true)
    {
        auto p = glm::vec3(randomFloat(-1.0, 1.0), randomFloat(-1.0, 1.0), randomFloat(-1.0, 1.0));
        if(glm::length(p) > 1.0f) continue;

        // We know have a point in the sphere
        return glm::normalize(p);
    }
}

inline glm::vec3 randomHemisphereVec3(const glm::vec3& normal)
{
    auto result = randomUnitVec3();
    if(glm::dot(normal, result) < 0.0f)
        return -result;
    return result;
}

inline bool nearZeroVec3(const glm::vec3& vec)
{
    const auto delta = 1e-8f;
    return fabsf(vec.x) < delta && fabsf(vec.y) < delta && fabsf(vec.z) < delta;
}

inline float clamp(float x, float min, float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

} // core

} // miquella
