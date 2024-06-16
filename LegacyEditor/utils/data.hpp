#pragma once

#include "lce/processor.hpp"


class Data {
public:
    u32 size = 0;
    u8* data = nullptr;

    Data() = default;

    explicit Data(c_u32 sizeIn) : size(sizeIn) {
        data = new u8[sizeIn];
    }

    explicit Data(u8* dataIn, c_u32 sizeIn) : size(sizeIn), data(dataIn) {}


    bool allocate(c_u32 sizeIn) {
        size = sizeIn;
        delete[] data;

        data = new(std::nothrow) u8[sizeIn];
        return data != nullptr;
    }


    void deallocate() {
        if (data != nullptr) {
            delete[] data;
            data = nullptr;
            size = 0;
        }
    }

    void steal(Data other) {
        deallocate();
        data = other.data;
        size = other.size;
        other.reset();
    }

    void reset() {
        data = nullptr;
        size = 0;
    }

    ND u8* start() const { return data; }
    ND u32 getSize() const { return size; }
};