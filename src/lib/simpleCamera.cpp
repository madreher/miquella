#include <miquella/core/simpleCamera.h>

namespace miquella
{

namespace core
{

void SimpleCamera::setAspectRatio(float ratio, int imageWidth)
{
    m_imageWidth = imageWidth;
    m_imageHeight = static_cast<int>(static_cast<float>(m_imageWidth) / ratio);

    auto viewport_height = 2.0f;
    auto viewport_width = ratio * viewport_height;
    auto focal_length = 1.0f;

    m_origin = glm::vec3(0, 0, 0);
    m_horizontal = glm::vec3(viewport_width, 0.0, 0.0);
    m_vertical = glm::vec3(0.0, viewport_height, 0.0);
    m_lower_left_corner = m_origin - m_horizontal/2.0f - m_vertical/2.0f - glm::vec3(0.0f, 0.0f, focal_length);
}

Ray SimpleCamera::generateRay(float u, float v) const
{
    return Ray(m_origin, m_lower_left_corner + u*m_horizontal + v*m_vertical - m_origin);
}

} // core

} // miquella