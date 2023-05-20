#pragma once

#include <iostream>

#include <miquella/core/camera.h>
#include <miquella/core/simpleCamera.h>
#include <miquella/core/scene.h>
#include <miquella/core/utility.h>

#include <numeric>
#include <chrono>

namespace miquella
{

namespace core
{

class Renderer
{
public:
    Renderer(){}
    Renderer(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera) : m_scene(scene), m_camera(camera){}

    void setImageResolution(const int width, const int height)
    {
        assert(m_camera);
        m_width = width;
        m_height = height;

        if( auto c = std::dynamic_pointer_cast<SimpleCamera>(m_camera))
        {
            c->setAspectRatio(static_cast<float>(m_width) / static_cast<float>(m_height));
        }


        m_image.resize(static_cast<size_t>(m_width * m_height * 4));
        m_imageAccumulated.resize(static_cast<size_t>(m_width * m_height));
    }

    void setScene(std::shared_ptr<Scene> scene){ m_scene = scene; }
    void setCamera(std::shared_ptr<Camera> camera){ m_camera = camera; }


    glm::vec3 processRay(const miquella::core::Ray& r, int maxDepth) const
    {
        hitRecord rec;

        if(maxDepth <= 0)
            return glm::vec3(0.0, 0.0, 0.0);

        // Start at more than 0.0 to avoid self intersection
        if(m_scene->intersect(r, 0.00001f, std::numeric_limits<float>::max(), rec))
        {
            miquella::core::Ray scatter;
            glm::vec3 attenuation;
            if(rec.material->scatter(r, rec, attenuation, scatter))
                return attenuation * processRay(scatter, maxDepth-1);
            else
                return glm::vec3(0.0, 0.0, 0.0);
        }

        // Color for the background which serves as the source of light
        glm::vec3 dir = glm::normalize(r.direction());
        auto t = 0.5f*(dir.y + 1.0f);
        return (1.f-t)*glm::vec3(1.f,1.f, 1.f) + t*glm::vec3(0.5f, 0.7f, 1.f);
    }

    void render()
    {
        if(m_image.size() == 0 || m_image.size() != static_cast<size_t>(m_height*m_width*4))
        {
            std::cerr<<"ERROR: image resolution not initialized properly."<<std::endl;
            return;
        }

        int maxDepth = 5;

        auto startTime = std::chrono::steady_clock::now();

        for (int j = m_height-1; j >= 0; --j)
        {
            for (int i = 0; i < m_width; ++i)
            {

                miquella::core::Ray ray = m_camera->generateRay(
                            (static_cast<float>(i) + miquella::core::randomFloat()) / static_cast<float>(m_width - 1),
                            (static_cast<float>(m_height - j - 1)  + miquella::core::randomFloat()) / static_cast<float>(m_height - 1)   // The camera (0,0) is bottom left, the texture is (0,0) is top left
                            );

                glm::vec3 color = processRay(ray, maxDepth);


                auto indexAcc = static_cast<size_t>(j*m_width + i);
                m_imageAccumulated[indexAcc] += color;

                glm::vec3 pixelColor = m_imageAccumulated[indexAcc] / static_cast<float>(nbFrameAccumulated);

                int ir = static_cast<int>(254.999f * std::fabs(pixelColor.x));
                int ig = static_cast<int>(254.999f * std::fabs(pixelColor.y));
                int ib = static_cast<int>(254.999f * std::fabs(pixelColor.z));

                auto index = static_cast<size_t>(j*m_width*4 + i*4);
                m_image[index] = static_cast<unsigned char>(ir);
                m_image[index+1] = static_cast<unsigned char>(ig);
                m_image[index+2] = static_cast<unsigned char>(ib);
                m_image[index+3] = static_cast<unsigned char>(255);
            }
        }

        auto endTime = std::chrono::steady_clock::now();
        m_executionTime = static_cast<size_t>(std::chrono::duration<double, std::milli>(endTime - startTime).count());
        std::cout<<"Image computed in "<<m_executionTime<<" ms."<<std::endl;

        nbFrameAccumulated++;
    }

    unsigned char* getImagePointer(){ return m_image.data(); }

public:
    std::shared_ptr<Scene> m_scene;
    std::shared_ptr<Camera> m_camera;

    std::vector<unsigned char> m_image;
    std::vector<glm::vec3> m_imageAccumulated;
    int m_width = 0;
    int m_height = 0;

    size_t m_executionTime = 0;

    bool accumulate = true;
    size_t nbFrameAccumulated = 1;
};

} // core

} // miquella
