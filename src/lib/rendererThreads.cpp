#include <miquella/core/rendererThreads.h>

namespace miquella
{

namespace core
{

void RendererThreads::render()
{
    if(m_image.size() == 0 || m_image.size() != static_cast<size_t>(m_height*m_width*4))
    {
        std::cerr<<"ERROR: image resolution not initialized properly."<<std::endl;
        return;
    }

    int maxDepth = 5;

    auto startTime = std::chrono::steady_clock::now();

    //for (int j = m_height-1; j >= 0; --j)

    // Will not work with Ubuntu 18, gcc 7 and 8 too old, need 10 minimum
    //std::for_each(std::execution::par_unseq, std::begin(m_heightIndexes), std::end(m_heightIndexes), [&](int j))

    //int nbTasks = 6;

    // TODO: document the parallelization method, the difference between the pool size and the number of task,
    // and how are the tasks sliced from the image.

    spdlog::trace("Number of threads: {}, number of blocks: {}", m_nbThreads, m_nbBlocks);

    auto loop = [this, maxDepth](const int start, const int end)
    {
        (void)end;
        auto scene = m_scene->clone();
        //auto scene = m_scene;
        auto startTask = std::chrono::high_resolution_clock::now();
        //spdlog::trace("Block starting from {} to {}", start, end);
#define LOAD_BALANCE 1
#if LOAD_BALANCE 
        for(int i = 0; i < m_width; ++i)
        {
            if (i % static_cast<int>(m_nbBlocks) != start) continue; 
#else 
        for (int i = start; i < end; ++i)
        {
#endif
            for(int j = 0; j < m_height; j++)
            {

                miquella::core::Ray ray = m_camera->generateRay(
                            (static_cast<float>(i) + miquella::core::randomFloat()) / static_cast<float>(m_width - 1),
                            (static_cast<float>(m_height - j - 1)  + miquella::core::randomFloat()) / static_cast<float>(m_height - 1)   // The camera (0,0) is bottom left, the texture is (0,0) is top left
                            );

                glm::vec3 color = processRay(ray, maxDepth, scene);

                auto indexAcc = static_cast<size_t>(j*m_width + i);
                m_imageAccumulated[indexAcc] += color;

                // Gamma correction
                auto scale = 1.f / static_cast<float>(m_nbFrameAccumulated+1);
                auto r = sqrtf(m_imageAccumulated[indexAcc].x * scale);
                auto g = sqrtf(m_imageAccumulated[indexAcc].y * scale);
                auto b = sqrtf(m_imageAccumulated[indexAcc].z * scale);



                int ir = static_cast<int>(256.f * std::clamp(r, 0.0f, 0.999f));
                int ig = static_cast<int>(256.f * std::clamp(g, 0.0f, 0.999f));
                int ib = static_cast<int>(256.f * std::clamp(b, 0.0f, 0.999f));

                auto index = static_cast<size_t>(j*m_width*4 + i*4);
                m_image[index] = static_cast<unsigned char>(ir);
                m_image[index+1] = static_cast<unsigned char>(ig);
                m_image[index+2] = static_cast<unsigned char>(ib);
                m_image[index+3] = static_cast<unsigned char>(255);
            }
        }
        auto endTask = std::chrono::high_resolution_clock::now();
        auto taskDuration = std::chrono::duration<double, std::milli>(endTask-startTask);
        //std::cout<<"[Sample "<< m_nbFrameAccumulated<<"] Task completed in "<<taskDuration.count()<<" ms."<<std::endl;
        spdlog::trace("[Sample {}] Task completed in {} ms.", m_nbFrameAccumulated, taskDuration.count());
    };

#if LOAD_BALANCE 
    BS::multi_future<void> loopFuture = m_pool.submit_blocks(0, static_cast<int>(m_nbBlocks), loop, static_cast<size_t>(m_nbBlocks));
#else
    BS::multi_future<void> loopFuture = m_pool.submit_blocks(0, m_width, loop, static_cast<size_t>(nbTasks));
#endif
    
    loopFuture.wait();

    auto endTime = std::chrono::steady_clock::now();
    m_executionTime = static_cast<size_t>(std::chrono::duration<double, std::milli>(endTime - startTime).count());
    m_totalExecutionAccumulated += m_executionTime;
    //std::cout<<"Sample "<< m_nbFrameAccumulated<<" computed in "<<m_executionTime<<" ms, accumulated average " << m_totalExecutionAccumulated / (m_nbFrameAccumulated)<<std::endl;
    spdlog::trace("Sample {} computed in {} ms, accumulated average {} ms.", m_nbFrameAccumulated, m_executionTime, m_totalExecutionAccumulated / (m_nbFrameAccumulated));
    m_nbFrameAccumulated++;
}

} // core

} // miquella