#pragma once

#include "ChunkData.hpp"
#include "LegacyEditor/utils/processor.hpp"

#include "v10Chunk.hpp"
#include "v11Chunk.hpp"
#include "v12Chunk.hpp"

class DataManager;


namespace universal {

    class ChunkParser {
    private:
        V10Chunk v10Chunk;
        V11Chunk v11Chunk;
        V12Chunk v12Chunk;

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

