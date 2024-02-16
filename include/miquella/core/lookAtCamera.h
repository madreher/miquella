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
                int width);

        virtual ~LookAtCamera(){}

        virtual Ray generateRay(float u, float v) const override;

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
