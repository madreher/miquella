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
            setAspectRatio(aspect_ratio);
        }

        virtual ~SimpleCamera(){}

        void setAspectRatio(float ratio)
        {
            auto viewport_height = 2.0f;
            auto viewport_width = ratio * viewport_height;
            auto focal_length = 1.0f;

            m_origin = glm::vec3(0, 0, 0);
            m_horizontal = glm::vec3(viewport_width, 0.0, 0.0);
            m_vertical = glm::vec3(0.0, viewport_height, 0.0);
            m_lower_left_corner = m_origin - m_horizontal/2.0f - m_vertical/2.0f - glm::vec3(0.0f, 0.0f, focal_length);
        }

        virtual Ray generateRay(float u, float v) const override
        {
            return Ray(m_origin, m_lower_left_corner + u*m_horizontal + v*m_vertical - m_origin);
        }

    private:
        glm::vec3 m_origin;
        glm::vec3 m_lower_left_corner;
        glm::vec3 m_horizontal;
        glm::vec3 m_vertical;
};

} // core

} // miquella

