#pragma once

#include "ChunkData.hpp"
#include "LegacyEditor/utils/processor.hpp"

class DataManager;


namespace universal {

    class V11Chunk;
    class V12Chunk;

    class ChunkParser {
    private:
        MU V11Chunk* v11Chunk = nullptr;
        MU V12Chunk* v12Chunk = nullptr;

    public:
        ChunkData chunkData;

        MU void readChunk(DataManager* managerIn, DIM dim);
        MU void writeChunk(DataManager* managerOut, DIM dim);


        u16 getBlock(int x, int y, int z);

        MU void fixHeightMap();
        MU void placeBlock(int x, int y, int z, u16 block, u16 data = 0, bool waterlogged = false);
        MU void rotateUpsideDown();

    };

}

