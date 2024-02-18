#include <benchmark/benchmark.h>

#include <miquella/core/rendererThreads.h>
#include <miquella/core/sceneFactory.h>


static void runBenchmarkScene(miquella::core::SceneID sceneID, size_t nSamples, uint32_t nbThreads, uint32_t nbBlocks)
{
    miquella::core::SceneFactory sceneFactory;
        auto [ scene, camera, background ] = sceneFactory.createScene(sceneID);

        miquella::core::RendererThreads renderer(scene, camera, nbThreads);
        renderer.setNbBlocks(nbBlocks);
        renderer.setBackground(background);

        
        for(size_t i = 1; i <= nSamples; ++i)
        {
            // Compute the image
            renderer.render();
        }
        std::cout<<"Completed "<<nSamples<<" samples for "<<nbThreads<<" threads and "<<nbBlocks<<" blocks."<<std::endl;
}

static void BM_ThreeBall(benchmark::State& state)
{
    for(auto _ : state)
    {
        auto sceneID = miquella::core::SceneID::SCENE_THREE_BALLS;
        size_t nSamples = 500;
        uint32_t nbThreads = static_cast<uint32_t>(state.range(0));
        uint32_t nbBlocks = static_cast<uint32_t>(state.range(1));

        runBenchmarkScene(sceneID, nSamples, nbThreads, nbBlocks);
    }
}

static void BM_OneWeekend(benchmark::State& state)
{
    for(auto _ : state)
    {
        auto sceneID = miquella::core::SceneID::SCENE_ONE_WEEKEND;
        size_t nSamples = 50;
        uint32_t nbThreads = static_cast<uint32_t>(state.range(0));
        uint32_t nbBlocks = static_cast<uint32_t>(state.range(1));

        runBenchmarkScene(sceneID, nSamples, nbThreads, nbBlocks);
    }
}

BENCHMARK(BM_ThreeBall)->Args({6,6})->Args({6, 12})->MeasureProcessCPUTime();
BENCHMARK(BM_OneWeekend)->Args({6,6})->Args({6, 12});

BENCHMARK_MAIN();