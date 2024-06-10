#pragma once

#include <cstddef>
#include <string>

#include "lce/processor.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"


class Picture {
public:
    uint32_t myRGBSize = 3;
    uint32_t myWidth = 0;
    uint32_t myHeight = 0;

    /// goes left to right, top to bottom I think
    uint8_t* myData = nullptr;

    void allocate(const uint32_t rgbSize) {
        delete[] myData;
        myRGBSize = rgbSize;
        myData = new uint8_t[static_cast<size_t>(myWidth * myHeight * myRGBSize)];
        memset(myData, 0, myWidth * myHeight * myRGBSize);
    }

    Picture(const uint32_t width, const uint32_t height) : myWidth(width), myHeight(height) {
        myData = nullptr;
        allocate(3);
    }

    MU explicit Picture(const int size) : Picture(size, size) {}

    MU Picture() = default;

    ~Picture() { delete[] myData; }

    MU ND uint32_t getWidth() const { return myWidth; }
    MU ND uint32_t getHeight() const { return myHeight; }
    ND uint32_t getIndex(const uint32_t xIn, const uint32_t yIn) const { return xIn + yIn * myHeight; }

    MU void drawPixel(const unsigned char* rgb, const uint32_t xIn, const uint32_t yIn) const {
        const uint32_t index = getIndex(xIn, yIn);
        memcpy(&myData[static_cast<size_t>(index * myRGBSize)], rgb, myRGBSize);
    }

    MU ND bool drawBox(const uint32_t startX, const uint32_t startY,
                       const uint32_t endX, const uint32_t endY,
                       const uint8_t red, const uint8_t green, const uint8_t blue) const {

        if (startX > myWidth || startY > myHeight) { return false; }
        if (endX > myWidth || endY > myHeight) { return false; }
        if (endX < startX || endY < startY) { return false; }

        for (uint32_t xIter = startX; xIter < endX; xIter++) {
            const uint32_t index = getIndex(xIter, startY) * myRGBSize;
            myData[index] = red;
            myData[index + 1] = green;
            myData[index + 2] = blue;
        }

        const uint32_t rowSize = (endX - startX) * myRGBSize;
        const uint32_t firstRowIndex = getIndex(startX, startY) * myRGBSize;
        for (uint32_t yIter = startY + 1; yIter < endY; yIter++) {
            const uint32_t index = getIndex(startX, yIter) * myRGBSize;
            memcpy(&myData[index], &myData[firstRowIndex], rowSize);
        }
        return true;
    }

    MU void fillColor(const uint8_t red, const uint8_t green, const uint8_t blue) const {
        for (uint32_t xIter = 0; xIter < myWidth; xIter++) {
            const uint32_t index = getIndex(xIter, 0) * myRGBSize;
            myData[index] = red;
            myData[index + 1] = green;
            myData[index + 2] = blue;
        }

        const uint32_t rowSize = (myWidth) * myRGBSize;
        for (uint32_t yIter = 0; yIter < myHeight; yIter++) {
            const uint32_t index = getIndex(0, yIter) * myRGBSize;
            memcpy(&myData[index], &myData[0], rowSize);
        }
    }


    void loadFromFile(const char* filename) {
        int x,y,n;
        delete[] myData;
        myData = stbi_load(filename, &x, &y, &n, 0);
        myWidth = x;
        myHeight = y;
        myRGBSize = n;
    }


    MU void getSubImage(Picture& picture, const uint32_t startX, const uint32_t startY) const {
        if (myData == nullptr || myWidth == 0 || myHeight == 0 || startX > myWidth || startY > myHeight) {
            return;
        }

        picture.allocate(myRGBSize);
        const uint32_t rowSize = picture.myWidth * myRGBSize;
        for (uint32_t yIter = 0; yIter < picture.myHeight; yIter++) {
            const uint32_t indexIn = (startX + (startY + yIter) * myWidth) * myRGBSize;
            const uint32_t indexOut = yIter * picture.myWidth * picture.myRGBSize;
            memcpy(&picture.myData[indexOut], &this->myData[indexIn], rowSize);
        }
    }


    void saveWithName(std::string filename, const std::string& directory) const {
        filename = directory + filename;
        stbi_write_png(filename.c_str(),
                       static_cast<int>(myWidth),
                       static_cast<int>(myHeight),
                       static_cast<int>(myRGBSize), myData,
                       static_cast<int>(myWidth * myRGBSize)
        );
    }


};