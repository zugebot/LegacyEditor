#pragma once

#include "ChunkData.hpp"
#include "LegacyEditor/utils/DataManager.hpp"
#include "LegacyEditor/utils/processor.hpp"

namespace universal {

    class V11Chunk;
    class V12Chunk;

    class ChunkParser {
    public:
        ChunkData chunkData;

        V11Chunk* v11Chunk = nullptr;
        V12Chunk* v12Chunk = nullptr;

        MU void readChunk(DataManager* managerIn, DIM dim);
        MU void writeChunk(DataManager* managerOut, DIM dim);

        void placeBlock(int x, int y, int z, u16 block, u16 data = 0, bool waterlogged = false);
        u16 getBlock(int x, int y, int z);
        void rotateUpsideDown();


    };

}

