#pragma once

#include <random>

#include <glm/glm.hpp>
#include <cstdlib>

namespace miquella {

namespace core {

const float pi = 3.1415926535897932385f;

thread_local std::random_device rd;
thread_local std::mt19937 generator = std::mt19937(rd());
thread_local std::uniform_real_distribution<float> distribution(0.f, 1.f);

inline float degreeToRadians(float degrees)
{
    return degrees * pi / 180.0f;
}

#if 0
inline float randomFloat()
{
    return static_cast<float>(rand()) / static_cast<float>((RAND_MAX + 1.0));
}

inline float randomFloat(float minValue, float maxValue)
{
    return minValue + (maxValue-minValue)*static_cast<float>(randomFloat());
}
#else
inline float randomFloat(float minValue = 0.0f, float maxValue = 1.0f)
{
    return minValue + (maxValue-minValue)*distribution(generator);
}
#endif

inline glm::vec3 randomColor(float minValue = 0.f, float maxValue = 1.f)
{
    return glm::vec3(randomFloat(minValue, maxValue), randomFloat(minValue, maxValue), randomFloat(minValue, maxValue));
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
    if(glm::dot(normal, result) > 0.0f)
        return -result;
    return result;
}

inline bool nearZeroVec3(const glm::vec3& vec)
{
    const auto delta = 1e-10f;
    return (fabsf(vec.x) < delta) && (fabsf(vec.y) < delta) && (fabsf(vec.z) < delta);
}

inline float clamp(float x, float min, float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

inline float degToRad(float deg)
{
    return (deg * pi) / 180.f;
}

} // core

} // miquella
