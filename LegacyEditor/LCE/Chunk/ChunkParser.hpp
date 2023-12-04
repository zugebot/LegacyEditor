#pragma once

#include "ChunkData.hpp"
#include "LegacyEditor/utils/processor.hpp"

#include "v10Chunk.hpp"
#include "v11Chunk.hpp"
#include "v12Chunk.hpp"

class DataManager;


namespace universal {

    class ChunkParser {
    public:
        ChunkData chunkData;

        MU void readChunk(DataManager* managerIn, DIM dim);
        MU void writeChunk(DataManager* managerOut, DIM dim);


        u16 getBlock(int x, int y, int z);
        MU void convertOldToNew();
        MU void fixHeightMap();
        MU void placeBlock(int x, int y, int z, u16 block, u16 data = 0, bool waterlogged = false);
        MU void rotateUpsideDown();

    };

}

