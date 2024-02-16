#pragma once

#include <miquella/core/ray.h>
#include <miquella/core/camera.h>


namespace miquella
{

namespace core
{

class SimpleCamera : public Camera {
    public:
        SimpleCamera() : Camera()
        {
            auto aspect_ratio = 16.0f / 9.0f;
            setAspectRatio(aspect_ratio, 800);
        }

        virtual ~SimpleCamera(){}


        void setAspectRatio(float ratio, int imageWidth);

        virtual Ray generateRay(float u, float v) const override;

    private:
        glm::vec3 m_origin;
        glm::vec3 m_lower_left_corner;
        glm::vec3 m_horizontal;
        glm::vec3 m_vertical;

};

} // core

} // miquella

