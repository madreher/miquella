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

        int getImageWidth() const { return m_imageWidth; }
        int getImageHeight() const { return m_imageHeight; }

public:
        int m_imageWidth;
        int m_imageHeight;

};

} // core

} // miquella

