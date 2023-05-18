#pragma once


#include <miquella/core/ray.h>
#include <miquella/core/hit.h>

namespace miquella
{

namespace core
{

struct hitRecord;

class Material
{
public:
    Material(){}

    virtual ~Material(){}

    virtual bool scatter(const Ray & incoming, const hitRecord& record, glm::vec3& color, Ray& out) const = 0;

};


}   // core

}   // miquella
