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


    virtual bool scatter(const Ray & incoming, const hitRecord& record, glm::vec3& color, Ray& out) const override
    {
        color = {1.f, 1.f, 1.f};

        // Check if we come from in or out the material
        // This only works because we assume refraction between air and material and air has
        // a refraction indice of 1
        float refractionRatio = record.front_face ? (1.f/m_refractionIndice) : m_refractionIndice;

        glm::vec3 dir = glm::normalize(incoming.direction());

        float cosTheta = fminf(glm::dot(-dir, record.normal), 1.f);
        float sinTheta = std::sqrt(1.f - cosTheta*cosTheta);

        bool cannotRefract = refractionRatio * sinTheta > 1.f;
        glm::vec3 direction;
        if(cannotRefract || _reflectance(cosTheta, refractionRatio) > randomFloat())
            direction = _reflect(incoming.direction(), record.normal);
        else
            direction = _refract(dir, record.normal, refractionRatio);

        out = Ray(record.p, direction);
        return true;
    }

    virtual std::shared_ptr<Material> clone() override
    {
        return std::make_shared<Dielectric>(m_refractionIndice);
    }

private:
    float _reflectance(float cosine, float refIdx) const
    {
        // Schlick's approximation for reflectance
        auto r0 = (1.f - refIdx) / (1.f + refIdx);
        r0 = r0*r0;
        return r0 + (1.f-r0)*powf((1.f - cosine), 5);
    }

    glm::vec3 _refract(const glm::vec3& in, const glm::vec3& normal, float etaiOverEtat) const
    {
        auto cosTheta = fminf(glm::dot(-in, normal), 1.f);
        glm::vec3 rOutPerp = etaiOverEtat * (in + cosTheta * normal);
        glm::vec3 rOutParallel = -std::sqrt(fabsf(1.f - (glm::length(rOutPerp)*glm::length(rOutPerp)))) * normal;
        return rOutPerp + rOutParallel;
    }

    glm::vec3 _reflect(const glm::vec3& in, const glm::vec3& normal) const
    {
        return in - 2.f*glm::dot(in, normal) * normal;
    }
public:
    float m_refractionIndice;
};

} // core

} // miquella
