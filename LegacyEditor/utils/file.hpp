#pragma once

#include "LegacyEditor/utils/processor.hpp"
#include "data.hpp"

#include <string>
#include <utility>


enum class FileType : u8 {
    NONE,               // NONE
    STRUCTURE,          // data/
    VILLAGE,            // data/
    DATA_MAPPING,       // data/
    MAP,                // data/
    REGION_NETHER,      // ...
    REGION_OVERWORLD,   // ...
    REGION_END,         // ...
    PLAYER,             // ...
    LEVEL,              // ...
    GRF                 // ...
};




struct File {
public:
    Data data;
    std::string name;
    FileType fileType = FileType::NONE;
    u64 timestamp = 0;
    u32 index = 0;
    u32 additionalData = 0;

    File() = default;

    explicit File(u32 sizeIn) {
        data = Data(sizeIn);
    }

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



