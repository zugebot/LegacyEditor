#pragma once

#include "LegacyEditor/utils/processor.hpp"
#include "data.hpp"

#include <string>
#include <utility>

struct File {
public:
    Data data;
    std::string name;
    u64 timestamp = 0;
    u32 additionalData = 0;

    File() = default;

    explicit File(u32 sizeIn) {
        data = Data(sizeIn);
    }

    // File(u8* dataIn, u32 sizeIn) : Data(dataIn, sizeIn) {}

    File(u32 sizeIn, std::string nameIn, u64 timestampIn)
        : name(std::move(nameIn)), timestamp(timestampIn) {
        data = Data(sizeIn);
    }

    File(u8* dataIn, u32 sizeIn, std::string nameIn, u64 timestampIn)
        : data(dataIn, sizeIn), name(std::move(nameIn)), timestamp(timestampIn) {}

    ND bool isEmpty() const {
        return data.size != 0;
    }
};



