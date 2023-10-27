#pragma once

#include "LegacyEditor/utils/processor.hpp"
#include "data.hpp"

#include <string>
#include <utility>

struct File : public Data {
public:
    std::string name;
    u64 timestamp = 0;
    u32 additionalData = 0;

    File() = default;

    explicit File(u32 sizeIn) : Data(sizeIn) {}

    explicit File(Data& dataIn) : Data(dataIn.start(), dataIn.size) {
        using_memory = false;
    }

    File(u8* dataIn, u32 sizeIn) : Data(dataIn, sizeIn) {}

    File(u32 sizeIn, std::string nameIn, u64 timestampIn)
        : Data(sizeIn), name(std::move(nameIn)), timestamp(timestampIn) {}

    File(u8* dataIn, u32 sizeIn, std::string nameIn, u64 timestampIn)
        : Data(dataIn, sizeIn), name(std::move(nameIn)), timestamp(timestampIn) {}

    bool isEmpty() {
        return size != 0;
    }
};



