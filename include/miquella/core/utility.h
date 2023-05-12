#pragma once

#include <random>

namespace miquella {

namespace core {

const float pi = 3.1415926535897932385f;

inline float degreeToRadians(float degrees)
{
    return degrees * pi / 180.0f;
}

inline float randomFloat()
{
    static std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    static std::mt19937 generator;
    return distribution(generator);
}

inline float clamp(float x, float min, float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

} // core

} // miquella
