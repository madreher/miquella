#include <miquella/core/sphere.h>

namespace miquella {

namespace core {

bool Sphere::intersect(const Ray & r, float tmin, float tmax, hitRecord& record)
{
    glm::vec3 oc = r.origin() - m_center;
    //auto a = r.direction().length_squared();
    auto a = glm::length(r.direction()) * glm::length(r.direction());
    auto half_b = glm::dot(oc, r.direction());
    auto c = glm::length(oc) * glm::length(oc)- m_r*m_r;

    auto discriminant = half_b*half_b - a*c;
    if (discriminant < 0) return false;
    auto sqrtd = std::sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    auto root = (-half_b - sqrtd) / a;
    if (root < tmin || tmax < root)
    {
        root = (-half_b + sqrtd) / a;
        if (root < tmin || tmax < root)
            return false;
    }

    record.t = root;
    record.p = r.at(record.t);
    auto normal = (record.p - m_center) / m_r;
    record.setFaceNormal(r, normal);
    record.material = m_material;

    return true;
}

} // core

} // miquella