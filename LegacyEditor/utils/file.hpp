#pragma once

#include <string>
#include <utility>

#include "processor.hpp"
#include "NBT.hpp"
#include "data.hpp"
#include "enums.hpp"



class NBTBase;
class NBTTagCompound;

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
    GRF,                // ...
    ENTITY_NETHER,      // ...
    ENTITY_OVERWORLD,   // ...
    ENTITY_END,         // ...
};

class File {
private:
    bool deleteData = true;
public:
    NBTBase* nbt  = nullptr;
    Data data;
    u64 timestamp = 0;
    u32 additionalData = 0;
    FileType fileType = FileType::NONE;

    ~File();
    File() = default;
    explicit File(u32 sizeIn) : data(Data(sizeIn)) {}
    File(u32 sizeIn, u64 timestampIn) : data(Data(sizeIn)), timestamp(timestampIn) {}
    File(u8* dataIn, u32 sizeIn, u64 timestampIn) : data(dataIn, sizeIn), timestamp(timestampIn) {}

    void doNotDelete() {
        deleteData = false;
    }

    ND NBTTagCompound* createNBTTagCompound();
    ND NBTTagCompound* getNBTCompound() const;
    ND std::string constructFileName(CONSOLE console) const;
    MU ND bool isEmpty() const { return data.size != 0; }
};



