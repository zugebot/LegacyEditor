#pragma once

#include <utility>

#include "LegacyEditor/utils/processor.hpp"


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "LegacyEditor/utils/stb_image_write.h"


class Picture {
public:
    static constexpr int RGB_SIZE = 3;
    u32 width = 0;
    u32 height = 0;
    /**
     * goes left to right, top to bottom I think
     */
    u8* data;

    void allocate() {
        delete[] data;
        data = new u8[width * height * RGB_SIZE];
        memset(data, 0, width * height * RGB_SIZE);
    }

    Picture(int width, int height) : width(width), height(height) {
        data = nullptr;
        allocate();
    }

    explicit Picture(int size) : Picture(size, size) {}

    ND u32 getWidth() const { return width; }
    ND u32 getHeight() const { return height; }
    ND u32 inline getIndex(u32 x, u32 y) const { return x + y * height; }

    MU void drawPixel(unsigned char* rgb, u32 x, u32 y) const {
        u32 index = getIndex(x, y);
        memcpy(&data[index * 3], rgb, 3);
    }

    ND bool drawBox(u32 startX, u32 startY, u32 endX, u32 endY, u8 red, u8 green, u8 blue) const {

        if (startX > width || startY > height) return false;
        if (endX > width || endY > height) return false;
        if (endX < startX || endY < startY) return false;

        for (u32 x = startX; x < endX; x++) {
            u32 index = getIndex(x, startY) * RGB_SIZE;
            data[index] = red;
            data[index + 1] = green;
            data[index + 2] = blue;
        }

        u32 rowSize = (endX - startX) * RGB_SIZE;
        u32 firstRowIndex = getIndex(startX, startY) * RGB_SIZE;
        for (u32 y = startY + 1; y < endY; y++) {
            u32 index = getIndex(startX, y) * RGB_SIZE;
            memcpy(&data[index], &data[firstRowIndex], rowSize);
        }
        return true;
    }

    void fillColor(u8 red, u8 green, u8 blue) const {
        for (u32 x = 0; x < width; x++) {
            u32 index = getIndex(x, 0) * RGB_SIZE;
            data[index] = red;
            data[index + 1] = green;
            data[index + 2] = blue;
        }

        u32 rowSize = (width) *RGB_SIZE;
        for (u32 y = 0; y < height; y++) {
            u32 index = getIndex(0, y) * RGB_SIZE;
            memcpy(&data[index], &data[0], rowSize);
        }
    }


    void saveWithName(std::string filename, const std::string& directory) const {
        filename = directory + filename;
        stbi_write_png(filename.c_str(), (int) width, (int) height, RGB_SIZE, data, (int) width * RGB_SIZE);
    }

    ~Picture() { delete[] data; }
};

