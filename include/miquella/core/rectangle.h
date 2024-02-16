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

    virtual std::shared_ptr<Object> clone() override;

    virtual bool intersect(const Ray & r, float tmin, float tmax, hitRecord& record) override;

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

    virtual bool intersect(const Ray & r, float tmin, float tmax, hitRecord& record) override;

    virtual std::shared_ptr<Object> clone() override;

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

    virtual bool intersect(const Ray & r, float tmin, float tmax, hitRecord& record) override;

    virtual std::shared_ptr<Object> clone() override;

public:
    float m_y0;
    float m_y1;
    float m_z0;
    float m_z1;
    float m_x;
};


} // core

} // miquella
