#pragma once

#include "LegacyEditor/utils/NBT/NBT.hpp"
#include "LegacyEditor/utils/enums.hpp"


class ChunkData {
public:
    u16_vec blocks;
    u8_vec blockLight;
    u8_vec skyLight;
    u8_vec heightMap;
    u8_vec biomes;
    NBTBase* NBTData = nullptr;
    i16 terrainPopulated;
    i64 lastUpdate;
    i64 inhabitedTime;
    i32 DataGroupCount;
    i32 chunkX;
    i32 chunkZ;
    u16_vec submerged;
};
