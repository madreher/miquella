#pragma once

#include <fstream>
#include <iostream>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

namespace miquella 
{

namespace core 
{

namespace io 
{

    struct PPMImage
    {
        int w = 0;
        int h = 0;
        std::vector<unsigned char> image;
    };

    static void writePPM(std::ofstream& file, int w, int h, const std::vector<unsigned char>& image)
    {
        file << "P3\n" << w << ' ' << h << "\n255\n";

        for (int j = 0; j < h; ++j) 
        {
            for (int i = 0; i < w; ++i) 
            {
                auto index = static_cast<size_t>(j*w*4 + i*4);

                file << std::to_string(image[index]) << ' ' << std::to_string(image[index+1]) << ' ' << std::to_string(image[index+2]) << '\n';
            }
        }

        file.close();
    }

    static PPMImage readPPM(std::ifstream& file)
    {
        std::string mMagic;
        file >> mMagic;

        PPMImage result;

        // P3 = ASCII, P6 = binary
        if(mMagic.compare("P3") != 0)
        {
            std::cerr << "ERROR: unrecognizeable PPM format, only ASCII format is supported (P3).\n";
            return result;
        }
        file.seekg(1, file.cur);
        char c;
        file.get(c);
        if (c == '#')
        {
            // We got comments in the PPM image and skip the comments
            while (c != '\n')
            {
                file.get(c);
            }
        }
        else
        {
            file.seekg(-1, file.cur);
        }
        
        int mMax;
        file >> result.w >> result.h >> mMax;
        if (mMax != 255)
        {
            std::cout << "Got PPM maximum value: " << mMax << std::endl;
            std::cout << "Maximum pixel has to be 255" << std::endl;
            return result;
        }

        //mBuffer = new uint8_t[mW * mH * 3];
        size_t imageSize = static_cast<size_t>(result.w*result.h*4);
        result.image.resize(imageSize);

        for (size_t i = 0; i < imageSize; i ++)
        {
            if(i % 4 == 3)
                result.image[i] = static_cast<uint8_t>(255);
            else 
            {
                std::string pixel_str;
                file >> pixel_str;
                result.image[i] = static_cast<uint8_t> (std::stoi(pixel_str));
            }
        }

        return result;
    }



} // io


} // core

} // miquella

#pragma GCC diagnostic pop