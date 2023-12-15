#pragma once

#include <cstddef>
#include <utility>

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "LegacyEditor/libs/stb_image_write.h"
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

    MU explicit Picture(int size) : Picture(size, size) {}

    ~Picture() { delete[] data; }

    MU ND inline u32 getWidth() const { return width; }
    MU ND inline u32 getHeight() const { return height; }
    ND inline u32 getIndex(const u32 x, const u32 y) const { return x + y * height; }

    MU void drawPixel(const unsigned char* rgb, const u32 x, const u32 y) const {
        const u32 index = getIndex(x, y);
        memcpy(&data[static_cast<size_t>(index * 3)], rgb, 3);
    }

    MU ND bool drawBox(const u32 startX, const u32 startY, const u32 endX, const u32 endY, const u8 red, const u8 green, const u8 blue) const {

        if (startX > width || startY > height) { return false; }
        if (endX > width || endY > height) { return false; }
        if (endX < startX || endY < startY) { return false; }

        for (u32 x = startX; x < endX; x++) {
            const u32 index = getIndex(x, startY) * RGB_SIZE;
            data[index] = red;
            data[index + 1] = green;
            data[index + 2] = blue;
        }

        const u32 rowSize = (endX - startX) * RGB_SIZE;
        const u32 firstRowIndex = getIndex(startX, startY) * RGB_SIZE;
        for (u32 y = startY + 1; y < endY; y++) {
            const u32 index = getIndex(startX, y) * RGB_SIZE;
            memcpy(&data[index], &data[firstRowIndex], rowSize);
        }
        return true;
    }

    MU void fillColor(const u8 red, const u8 green, const u8 blue) const {
        for (u32 x = 0; x < width; x++) {
            const u32 index = getIndex(x, 0) * RGB_SIZE;
            data[index] = red;
            data[index + 1] = green;
            data[index + 2] = blue;
        }

        const u32 rowSize = (width) *RGB_SIZE;
        for (u32 y = 0; y < height; y++) {
            const u32 index = getIndex(0, y) * RGB_SIZE;
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

