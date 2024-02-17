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
        Renderer(scene, camera), m_pool(poolSize), m_nbThreads(poolSize), m_nbBlocks(2*poolSize){}

    virtual ~RendererThreads(){  }

    void setNbThreads(uint32_t nbThreads)
    {
        m_nbThreads = nbThreads;
        m_pool.reset(m_nbThreads);

        // Testing heuristic of double the number of block. 
        m_nbBlocks = 2*nbThreads;
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
    uint32_t m_nbThreads = 1;
    uint32_t m_nbBlocks = 1;

//    std::vector<int> m_heightIndexes;   // Array used to store a counter from m_height-1 to 0
};

} // core

} // miquella
