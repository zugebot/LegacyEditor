#pragma once

#include <vector>

#include "../Utils/NBT.hpp"


class AquaticChunkData {
public:
    std::vector<uint16_t> blocks;
    std::vector<uint8_t> blockLight;
    std::vector<uint8_t> skyLight;
    std::vector<uint8_t> heightMap;
    std::vector<uint8_t> biomes;
    NBTBase* NBTData = nullptr;
    short terrainPopulated;
    int64_t lastUpdate;
    int64_t inhabitedTime;
    int DataGroupCount;
    int chunkX;
    int chunkZ;
    std::vector<uint16_t> subAmerged;
};


class LCEChunkData {
public:
    std::vector<uint8_t> blocks;
    std::vector<uint8_t> data;
    std::vector<uint8_t> blockLight;
    std::vector<uint8_t> skyLight;
    std::vector<uint8_t> heightMap;
    std::vector<uint8_t> biomes;
    NBTBase* NBTData = nullptr;
    short terrainPopulated;
    int64_t lastUpdate;
    int64_t inhabitedTime;
    int DataGroupCount;
    int chunkX;
    int chunkZ;
    int version;
};
