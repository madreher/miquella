#pragma once

#include <iostream>

#include <miquella/core/camera.h>
#include <miquella/core/simpleCamera.h>
#include <miquella/core/scene.h>
#include <miquella/core/utility.h>

#include <numeric>
#include <chrono>
#include <algorithm>
#include <string.h>

#include <miquella/core/io/ppm.h>

namespace miquella
{

namespace core
{

class Renderer
{
public:
    Renderer(){}
    Renderer(std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera) :
        m_scene(scene),
        m_camera(camera),
        m_background(Background::BLACK)
    {
        updateImageFromCamera();
    }

    virtual ~Renderer(){}

    void setBackground(const Background& b){ m_background = b; }

    virtual void updateImageFromCamera();

    void setScene(std::shared_ptr<Scene> scene){ m_scene = scene; }
    void setCamera(std::shared_ptr<Camera> camera)
    {
        m_camera = camera;
        updateImageFromCamera();
    }

    glm::vec3 getGradiantBackground(const Ray& r) const
    {
        // Color for the background which serves as the source of light
        glm::vec3 dir = glm::normalize(r.direction());
        auto t = 0.5f*(dir.y + 1.0f);
        return (1.f-t)*glm::vec3(1.f,1.f, 1.f) + t*glm::vec3(0.5f, 0.7f, 1.f);
    }

    glm::vec3 getBlackBackground(const Ray& r) const
    {
        (void)r;
        return glm::vec3(0.f, 0.f, 0.f);
    }

    glm::vec3 getBackground(const Ray& r) const
    {
        switch(m_background)
        {
            case Background::BLACK:
            {
                return getBlackBackground(r);
            }
            case Background::GRADIANT:
            {
                return getGradiantBackground(r);
            }
            default:
                return getBlackBackground(r);
        }
    }

    glm::vec3 processRay(const Ray& r, int maxDepth) const
    {
        return processRay(r, maxDepth, m_scene);
    }


    glm::vec3 processRay(const Ray& r, int maxDepth, const std::shared_ptr<Scene> scene) const;

    virtual void render();

    unsigned char* getImagePointer(){ return m_image.data(); }
    int getImageWidth() const { return m_camera->getImageWidth(); }
    int getImageHeight() const { return m_camera->getImageHeight(); }

    void writeToPPM(const std::string& path) const;

public:
    std::shared_ptr<Scene> m_scene;
    std::shared_ptr<Camera> m_camera;

    std::vector<unsigned char> m_image;
    std::vector<glm::vec3> m_imageAccumulated;
    int m_width = 0;
    int m_height = 0;

    size_t m_executionTime = 0;

    bool accumulate = true;
    size_t m_nbFrameAccumulated = 1;

    Background m_background;
};

} // core

} // miquella
