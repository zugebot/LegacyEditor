#include "ChunkData.hpp"


ChunkData::ChunkData(int xIn, int zIn, DIM dimensionIn)
        : x(xIn), z(zIn), dimension(dimensionIn) {
    worldHeight = 0;
    blocksInChunk = 0;
    blocks = nullptr;// * 2 because it is uint16_t
    data = nullptr;
    blockLight = nullptr;
    skyLight = nullptr;
    heightMap = nullptr;// 256 * sizeof(int) = 1024
    biomes = nullptr;
    isWaterLogged = nullptr;

    searchData = nullptr;

    lastUpdate = 0;
    terrainPopulated = false;
    inhabitedTime = 0;
    version = 0;

    entities = nullptr;
    tileEntities = nullptr;
    tileTicks = nullptr;
}


ChunkData::ChunkData(int xIn, int zIn, int worldHeightIn, DIM dimensionIn)
        : x(xIn), z(zIn), worldHeight(worldHeightIn), dimension(dimensionIn) {

    int sizeOfChunk = 256 * worldHeightIn;
    blocksInChunk = sizeOfChunk;
    blocks = new u16[sizeOfChunk];
    data = new u8[sizeOfChunk];
    blockLight = new u8[sizeOfChunk];
    skyLight = new u8[sizeOfChunk];
    heightMap = new i32[256];
    biomes = new u8[256];
    isWaterLogged = new bool[sizeOfChunk];

    // overrideBlocks = new PaletteBlock[sizeOfChunk];
    searchData = (uint16_t*) malloc(sizeOfChunk * 2);

    lastUpdate = 0;
    terrainPopulated = false;
    inhabitedTime = 0;
    version = 0;
    entities = nullptr;
    tileEntities = nullptr;
    tileTicks = nullptr;
}


ChunkData::~ChunkData() {
    delete[] blocks;
    delete[] data;
    delete[] blockLight;
    delete[] skyLight;
    delete[] heightMap;
    delete[] biomes;
    delete[] isWaterLogged;
    delete[] searchData;
    //there might be a better solution of free the data
    //NBTBase(entities, 9).NbtFree();
    //NBTBase(tileEntities, 9).NbtFree();
    //NBTBase(tileTicks, 9).NbtFree();
    // delete[] overrideBlocks;
    // delete[] submergedBlocks;
    // delete[] submergedData;
}
