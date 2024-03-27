#pragma once

#include <cstddef>

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../../include/stb_image_write.h"
#include "LegacyEditor/utils/processor.hpp"


class Picture {
public:
    static constexpr int RGB_SIZE = 3;
    u32 width = 0;
    u32 height = 0;

    /// goes left to right, top to bottom I think
    u8* data;

    void allocate() {
        delete[] data;
        data = new u8[static_cast<size_t>(width * height * RGB_SIZE)];
        memset(data, 0, width * height * RGB_SIZE);
    }

    Picture(const int width, const int height) : width(width), height(height) {
        data = nullptr;
        allocate();
    }

    MU explicit Picture(const int size) : Picture(size, size) {}

    ~Picture() { delete[] data; }

    MU ND u32 getWidth() const { return width; }
    MU ND u32 getHeight() const { return height; }
    ND u32 getIndex(const u32 xIn, const u32 yIn) const { return xIn + yIn * height; }

    MU void drawPixel(const unsigned char* rgb, const u32 xIn, const u32 yIn) const {
        const u32 index = getIndex(xIn, yIn);
        memcpy(&data[static_cast<size_t>(index * 3)], rgb, 3);
    }

    MU ND bool drawBox(const u32 startX, const u32 startY, const u32 endX, const u32 endY, const u8 red, const u8 green, const u8 blue) const {

        if (startX > width || startY > height) { return false; }
        if (endX > width || endY > height) { return false; }
        if (endX < startX || endY < startY) { return false; }

        for (u32 xIter = startX; xIter < endX; xIter++) {
            const u32 index = getIndex(xIter, startY) * RGB_SIZE;
            data[index] = red;
            data[index + 1] = green;
            data[index + 2] = blue;
        }

        const u32 rowSize = (endX - startX) * RGB_SIZE;
        const u32 firstRowIndex = getIndex(startX, startY) * RGB_SIZE;
        for (u32 yIter = startY + 1; yIter < endY; yIter++) {
            const u32 index = getIndex(startX, yIter) * RGB_SIZE;
            memcpy(&data[index], &data[firstRowIndex], rowSize);
        }
        return true;
    }

    MU void fillColor(const u8 red, const u8 green, const u8 blue) const {
        for (u32 xIter = 0; xIter < width; xIter++) {
            const u32 index = getIndex(xIter, 0) * RGB_SIZE;
            data[index] = red;
            data[index + 1] = green;
            data[index + 2] = blue;
        }

        const u32 rowSize = (width) *RGB_SIZE;
        for (u32 yIter = 0; yIter < height; yIter++) {
            const u32 index = getIndex(0, yIter) * RGB_SIZE;
            memcpy(&data[index], &data[0], rowSize);
        }
    }


    void saveWithName(std::string filename, const std::string& directory) const {
        filename = directory + filename;
        stbi_write_png(filename.c_str(),
            static_cast<int>(width),
            static_cast<int>(height),
            RGB_SIZE,
            data,
            static_cast<int>(width) * RGB_SIZE
        );
    }


};

