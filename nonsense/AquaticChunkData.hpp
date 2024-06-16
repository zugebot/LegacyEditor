#pragma once

#include <vector>

#include "../Utils/NBT.hpp"


class AquaticChunkData {
public:
    std::vector<uint16_t> blocks;
    std::vector<u8> blockLight;
    std::vector<u8> skyLight;
    std::vector<u8> heightMap;
    std::vector<u8> biomes;
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
    std::vector<u8> blocks;
    std::vector<u8> data;
    std::vector<u8> blockLight;
    std::vector<u8> skyLight;
    std::vector<u8> heightMap;
    std::vector<u8> biomes;
    NBTBase* NBTData = nullptr;
    short terrainPopulated;
    int64_t lastUpdate;
    int64_t inhabitedTime;
    int DataGroupCount;
    int chunkX;
    int chunkZ;
    int version;
};
