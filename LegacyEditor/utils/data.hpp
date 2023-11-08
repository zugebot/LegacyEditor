#pragma once

#include "LegacyEditor/utils/processor.hpp"


class Data {
public:
    u32 size = 0;
    u8* data = nullptr;

    Data() = default;

    explicit Data(u32 sizeIn) : size(sizeIn) {
        data = new u8[sizeIn];
    }

    explicit Data(u8* dataIn, u32 sizeIn) : data(dataIn), size(sizeIn) {}

    ~Data() {
        if (data != nullptr) {

        }
    }

    void allocate(u32 sizeIn) {
        size = sizeIn;
        delete[] data;
        data = new u8[sizeIn];
    }

    ND u8* start() const { return data; }
    ND u32 getSize() const { return size; }
};