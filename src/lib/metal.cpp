#include <miquella/core/metal.h>

namespace miquella
{

namespace core
{

bool Metal::scatter(const Ray & incoming, const hitRecord& record, glm::vec3& color, Ray& out) const
{
    glm::vec3 reflected = _reflect(incoming.direction(), record.normal);
    out = Ray(record.p, reflected + m_fuzz*randomUnitVec3());
    color = m_albedo;
    return glm::dot(out.direction(), record.normal) > 0.f;
}

std::shared_ptr<Material> Metal::clone()
{
    return std::make_shared<Metal>(m_albedo, m_fuzz);
}


glm::vec3 Metal::_reflect(const glm::vec3& in, const glm::vec3& normal) const
{
    return in - 2.f*glm::dot(in, normal) * normal;
}

} // core

} // miquella