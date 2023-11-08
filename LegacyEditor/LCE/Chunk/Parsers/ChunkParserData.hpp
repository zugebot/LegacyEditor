#pragma once

#include <cstdint>

#include "LegacyEditor/utils/enums.hpp"
#include "LegacyEditor/utils/NBT.hpp"


class ChunkParserData {
private:
public:

    u16_vec blocks;
    u8_vec blockLight;
    u8_vec skyLight;
    u8_vec heightMap;
    u8_vec biomes;
    NBTBase* NBTData = nullptr;

    i32 chunkX;
    i32 chunkZ;
    i64 inhabitedTime;
    i64 lastUpdate;
    i32 terrainPopulated;

    i32 DataGroupCount;
    u16_vec submerged;
};