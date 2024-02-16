#include <miquella/core/lookAtCamera.h>

namespace miquella
{

namespace core
{

LookAtCamera::LookAtCamera(
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
    float h = std::tan(theta/2.f);
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

Ray LookAtCamera::generateRay(float u, float v) const
{
    return Ray(m_origin, m_lowerLeftCorner + u*m_horizontal + v*m_vertical - m_origin);
}

} // core

} // miquella