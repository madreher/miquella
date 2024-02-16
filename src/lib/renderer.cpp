#include <miquella/core/renderer.h>

namespace miquella
{

namespace core
{

void Renderer::updateImageFromCamera()
{
    assert(m_camera);
    m_width = m_camera->getImageWidth();
    m_height = m_camera->getImageHeight();

    m_image.resize(static_cast<size_t>(m_width * m_height * 4));
    memset(m_image.data(), 0, static_cast<size_t>(m_width * m_height * 4) * sizeof(unsigned char));
    m_imageAccumulated.resize(static_cast<size_t>(m_width * m_height));
    memset(m_imageAccumulated.data(), 0, static_cast<size_t>(m_width * m_height) * sizeof(glm::vec3));
}

glm::vec3 Renderer::processRay(const Ray& r, int maxDepth, const std::shared_ptr<Scene> scene) const
{
    hitRecord rec;

    if(maxDepth <= 0)
        return glm::vec3(0.0, 0.0, 0.0);

    // Start at more than 0.0 to avoid self intersection
    if(scene->intersect(r, 0.001f, std::numeric_limits<float>::max(), rec))
    {
        miquella::core::Ray scatter;
        glm::vec3 attenuation;
        glm::vec3 emitted = rec.material->emitted();
        if(rec.material->scatter(r, rec, attenuation, scatter))
            return emitted + attenuation * processRay(scatter, maxDepth-1, scene);
        else
            return emitted;
    }

    // Color for the background which serves as the source of light
    return getBackground(r);
}

void Renderer::render()
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

    auto endTime = std::chrono::steady_clock::now();
    m_executionTime = static_cast<size_t>(std::chrono::duration<double, std::milli>(endTime - startTime).count());
    std::cout<<"Sample "<< m_nbFrameAccumulated<<" computed in "<<m_executionTime<<" ms."<<std::endl;

    m_nbFrameAccumulated++;
}

void Renderer::writeToPPM(const std::string& path) const
{
    std::ofstream file;
    file.open(path, std::ofstream::binary);
    io::writePPM(file, m_width, m_height, m_image);
    file.close();
}

} // core

} // miquella