#pragma once

#include <miquella/core/ray.h>


namespace miquella
{

namespace core
{

class Camera {
    public:
        Camera()
        {
        }

        virtual ~Camera(){}

        virtual Ray generateRay(float u, float v) const  = 0;

};

} // core

} // miquella

