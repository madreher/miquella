#pragma once

#include <vector>

#include <miquella/core/object.h>

namespace miquella {

namespace core {

class xyRectangle : public Object
{
public:
    xyRectangle() : Object(){};
    xyRectangle(float x0, float x1, float y0, float y1, float z, std::shared_ptr<Material> mat) :
        Object(mat), m_x0(x0), m_x1(x1), m_y0(y0), m_y1(y1), m_z(z){};

    virtual bool intersect(const Ray & r, float tmin, float tmax, hitRecord& record) override
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

public:
    float m_x0;
    float m_x1;
    float m_y0;
    float m_y1;
    float m_z;
};

} // core

} // miquella
