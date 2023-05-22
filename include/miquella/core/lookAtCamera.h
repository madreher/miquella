#pragma once

#include <miquella/core/ray.h>
#include <miquella/core/camera.h>
#include <miquella/core/utility.h>

namespace miquella
{

namespace core
{

class LookAtCamera : public Camera {
    public:
        LookAtCamera() : Camera(){}
        LookAtCamera(
                glm::vec3 lookFrom,
                glm::vec3 lookAt,
                glm::vec3 up,
                float vfov, // vertical field of view in degrees
                float aspectRatio,
                float aperture,
                float focusDistance,
                int width)
        {
            m_imageWidth = width;
            m_imageHeight = static_cast<int>(static_cast<float>(m_imageWidth) / aspectRatio);

            auto theta = degToRad(vfov);
            float h = tan(theta/2.f);
            auto viewportHeight = 2.0f * h;
            auto viewportWidth = aspectRatio * viewportHeight;

            m_w = glm::normalize(lookFrom - lookAt);
            m_u = glm::normalize(glm::cross(up, m_w));
            m_v = glm::cross(m_w, m_u);

            m_origin = lookFrom;
            m_horizontal = focusDistance * viewportWidth * m_u;
            m_vertical = focusDistance * viewportHeight * m_v;
            m_lowerLeftCorner = m_origin - m_horizontal/2.f - m_vertical/2.f - focusDistance*m_w;

            m_lensRadius = aperture / 2.f;
        }

        virtual ~LookAtCamera(){}

        virtual Ray generateRay(float u, float v) const override
        {
            return Ray(m_origin, m_lowerLeftCorner + u*m_horizontal + v*m_vertical - m_origin);
        }

    private:
        glm::vec3 m_origin;
        glm::vec3 m_lowerLeftCorner;
        glm::vec3 m_horizontal;
        glm::vec3 m_vertical;
        glm::vec3 m_u, m_v, m_w;
        float m_lensRadius;
};

} // core

} // miquella
