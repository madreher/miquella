#pragma once

#include <miquella/core/material.h>
#include <miquella/core/utility.h>
#include <algorithm>

namespace miquella
{

namespace core
{

class Dielectric : public Material
{

public:
    Dielectric(float refractionIndice) :
        Material(), m_refractionIndice(refractionIndice){};


    virtual bool scatter(const Ray & incoming, const hitRecord& record, glm::vec3& color, Ray& out) const override;

    virtual std::shared_ptr<Material> clone() override;

private:
    float _reflectance(float cosine, float refIdx) const;

    glm::vec3 _refract(const glm::vec3& in, const glm::vec3& normal, float etaiOverEtat) const;

    glm::vec3 _reflect(const glm::vec3& in, const glm::vec3& normal) const;

public:
    float m_refractionIndice;
};

} // core

} // miquella
