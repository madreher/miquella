#include <miquella/core/rectangle.h>

namespace miquella {

namespace core {

std::shared_ptr<Object> xyRectangle::clone()
{
    return std::make_shared<xyRectangle>(m_x0, m_x1, m_y0, m_y1, m_z, m_material->clone());
}

bool xyRectangle::intersect(const Ray & r, float tmin, float tmax, hitRecord& record)
{
    auto t = (m_z - r.origin().z) / r.direction().z;
    if (t < tmin || t > tmax)
            return false;
    auto x = r.origin().x + t*r.direction().x;
    auto y = r.origin().y + t*r.direction().y;
    if (x < m_x0 || x > m_x1 || y < m_y0 || y > m_y1)
        return false;

    record.t = t;
    auto normal = glm::vec3(0, 0, 1);
    record.setFaceNormal(r, normal);
    record.material = m_material;
    record.p = r.at(t);
    return true;
}

bool xzRectangle::intersect(const Ray & r, float tmin, float tmax, hitRecord& record)
{
    auto t = (m_y - r.origin().y) / r.direction().y;
    if (t < tmin || t > tmax)
            return false;
    auto x = r.origin().x + t*r.direction().x;
    auto z = r.origin().z + t*r.direction().z;
    if (x < m_x0 || x > m_x1 || z < m_z0 || z > m_z1)
        return false;

    record.t = t;
    auto normal = glm::vec3(0, 1, 0);
    record.setFaceNormal(r, normal);
    record.material = m_material;
    record.p = r.at(t);
    return true;
}

std::shared_ptr<Object> xzRectangle::clone()
{
    return std::make_shared<xzRectangle>(m_x0, m_x1, m_z0, m_z1, m_y, m_material->clone());
}

bool yzRectangle::intersect(const Ray & r, float tmin, float tmax, hitRecord& record)
{
    auto t = (m_x - r.origin().x) / r.direction().x;
    if (t < tmin || t > tmax)
            return false;
    auto y = r.origin().y + t*r.direction().y;
    auto z = r.origin().z + t*r.direction().z;
    if (y < m_y0 || y > m_y1 || z < m_z0 || z > m_z1)
        return false;

    record.t = t;
    auto normal = glm::vec3(1, 0, 0);
    record.setFaceNormal(r, normal);
    record.material = m_material;
    record.p = r.at(t);
    return true;
}

std::shared_ptr<Object> yzRectangle::clone()
{
    return std::make_shared<yzRectangle>(m_y0, m_y1, m_z0, m_z1, m_x, m_material->clone());
}

} // core

} // miquella 