#pragma once

#include <miquella/core/renderer.h>

#include <algorithm>
#include <BS_thread_pool.hpp>
//#include <execution>  // Not available with gcc8/9
#include <chrono>

#include <spdlog/spdlog.h>

namespace miquella
{

namespace core
{

class RendererThreads : public Renderer
{
public:
    RendererThreads() : Renderer(){}
    RendererThreads(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera, uint32_t poolSize = 1) :
        Renderer(scene, camera), m_pool(poolSize){}

    virtual ~RendererThreads(){  }

    void setNbThreads(int nbThreads)
    {
        m_nbThreads = nbThreads;
    }

    virtual void updateImageFromCamera() override
    {
        Renderer::updateImageFromCamera();


        //for (int j = m_height-1; j >= 0; --j)
        //    m_heightIndexes.push_back(j);
    }

    virtual void render() override;

public:
    BS::thread_pool m_pool;
    size_t m_totalExecutionAccumulated = 0;
    int m_nbThreads = 1;
//    std::vector<int> m_heightIndexes;   // Array used to store a counter from m_height-1 to 0
};

} // core

} // miquella
