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

    virtual glm::vec3 emitted() const
    {
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }

};


}   // core

}   // miquella
