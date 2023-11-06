#pragma once

#include "LegacyEditor/utils/enums.hpp"
#include "LegacyEditor/utils/NBT.hpp"


class ChunkData {
    int blocksInChunk;
    int worldHeight;

    i64 lastUpdate;
    bool terrainPopulated;
    i64 inhabitedTime;
    u16* blocks;
    u8* data;
    u8* blockLight;
    u8* skyLight;
    i32* heightMap;
    u8* biomes;
    bool* isWaterLogged;

    // PaletteBlock* overrideBlocks;
    u16* searchData;

    NBTTagList* entities;
    NBTTagList* tileEntities;
    NBTTagList* tileTicks;
    i32 x;
    i32 z;
    DIM dimension;
    int version;

    /// empty chunk
    ChunkData(int xIn, int zIn, DIM dimensionIn);

    /// the world height determines how much space to allocate for blocks in that chunk
    ChunkData(int xIn, int zIn, int worldHeightIn, DIM dimensionIn);

    ~ChunkData();

    ND inline int numBlocksInChunk() const { return this->blocksInChunk; }
};
