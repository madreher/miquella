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

    virtual std::shared_ptr<Object> clone() override
    {
        return std::make_shared<xyRectangle>(m_x0, m_x1, m_y0, m_y1, m_z, m_material->clone());
    }

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

class xzRectangle : public Object
{
public:
    xzRectangle() : Object(){};
    xzRectangle(float x0, float x1, float z0, float z1, float y, std::shared_ptr<Material> mat) :
        Object(mat), m_x0(x0), m_x1(x1), m_z0(z0), m_z1(z1), m_y(y){};

    virtual bool intersect(const Ray & r, float tmin, float tmax, hitRecord& record) override
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

    virtual std::shared_ptr<Object> clone() override
    {
        return std::make_shared<xzRectangle>(m_x0, m_x1, m_z0, m_z1, m_y, m_material->clone());
    }

public:
    float m_x0;
    float m_x1;
    float m_z0;
    float m_z1;
    float m_y;
};

class yzRectangle : public Object
{
public:
    yzRectangle() : Object(){};
    yzRectangle(float y0, float y1, float z0, float z1, float x, std::shared_ptr<Material> mat) :
        Object(mat), m_y0(y0), m_y1(y1), m_z0(z0), m_z1(z1), m_x(x){};

    virtual bool intersect(const Ray & r, float tmin, float tmax, hitRecord& record) override
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
    virtual std::shared_ptr<Object> clone() override
    {
        return std::make_shared<yzRectangle>(m_y0, m_y1, m_z0, m_z1, m_x, m_material->clone());
    }

public:
    float m_y0;
    float m_y1;
    float m_z0;
    float m_z1;
    float m_x;
};



} // core

} // miquella
