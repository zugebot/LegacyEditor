#pragma once

#include "LegacyEditor/utils/processor.hpp"

#include <cstdio>
#include <cstdlib>
#include <new>

class Data {
public:
    u8* data = nullptr;
    u8* ptr = nullptr;
    u32 size = 0;
    bool using_memory = false;
public:
    Data() = default;

    explicit Data(u32 sizeIn) {
        if (sizeIn == 0) {
            printf("Cannot initialize Data with size of 0");
            exit(1);
        }
        allocate(sizeIn);
    }

    explicit Data(u8* dataIn, u32 sizeIn) {
        data = dataIn;
        ptr = data;
        size = sizeIn;
        using_memory = true;
    }

    ~Data() {
        deallocate();
    }

    void deallocate() {
        if (!using_memory)
            return;
        delete[] data;
        data = nullptr;
        ptr = nullptr;
        using_memory = false;
    }

    bool allocate(u32 sizeIn) {
        if (using_memory) {
            deallocate();
        }
        size = sizeIn;
        data = new (std::nothrow) u8[size];
        if (data == nullptr) {
            return false;
        }
        ptr = data;
        using_memory = true;
        return true;
    }

    ND u8* getStartPtr() const {
        return data;
    }

    ND u32 getSize() const {
        return size;
    }

    ND u8* getPtr() const {
        return ptr;
    }

    void incrementPointer(i64 amount) {
        ptr += amount;
    }

};