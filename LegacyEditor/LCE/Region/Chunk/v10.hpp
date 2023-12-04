#pragma once

#include "ChunkData.hpp"


namespace chunk {


    /// "NBT" chunks.
    class ChunkV10 {
    private:
        ChunkData* chunkData = nullptr;
        DataManager* dataManager = nullptr;
    public:

        MU void readChunk(ChunkData* chunkDataIn, DataManager* managerIn, DIM dim);
        MU void writeChunk(ChunkData* chunkDataIn, DataManager* managerOut, DIM dim);

        ChunkV10() = default;
    };

}