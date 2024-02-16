#include <miquella/core/scene.h>

namespace miquella {

namespace core {

void Scene::addObject(std::shared_ptr<Object> obj)
{
    m_objects.push_back(obj);

    if(obj->isLightSource())
        m_lights.push_back(obj);
}

bool Scene::intersect(const Ray & r, float tmin, float tmax, hitRecord& record)
{
    hitRecord localRecord;
    bool hitFound = false;
    float currentMax = tmax;
    for(auto & obj : m_objects)
    {
        if(obj->intersect(r, tmin, currentMax, localRecord))
        {
            hitFound = true;
            currentMax = localRecord.t;
            record = localRecord;
        }
    }

    return hitFound;
}

std::shared_ptr<Scene> Scene::clone() 
{
    auto copy = std::make_shared<Scene>();
    for (auto& obj : m_objects)
        copy->m_objects.push_back(obj->clone());

    for (auto& light : m_lights)
        copy->m_lights.push_back(light->clone());

    return copy;
}

} // core

} // miquella