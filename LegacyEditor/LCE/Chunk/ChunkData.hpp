#pragma once

#include "LegacyEditor/utils/NBT.hpp"
#include "LegacyEditor/utils/enums.hpp"


class ChunkData {
public:
    u16_vec blocks;
    u8_vec blockLight;
    u8_vec skyLight;
    u8_vec heightMap;
    u8_vec biomes;
    NBTBase* NBTData = nullptr;
    i16 terrainPopulated = 0;
    i64 lastUpdate = 0;
    i64 inhabitedTime = 0;

    /// Used to skip the lights in the chunk
    i32 DataGroupCount = 0;

    i32 chunkX = 0;
    i32 chunkZ = 0;

    i32 lastVersion = 0;

    /// Aquatic only
    bool hasSubmerged = false;
    u16_vec submerged;

    ~ChunkData() {
        if (NBTData != nullptr) {
            NBTData->NbtFree();
            delete NBTData;
            NBTData = nullptr;
        }
    }

    ND std::string getCoords() const {
        return "(" + std::to_string(chunkX) + ", " + std::to_string(chunkZ) + ")";
    }
};
