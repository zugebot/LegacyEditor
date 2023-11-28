#pragma once

#include "LegacyEditor/LCE/Chunk/ChunkData.hpp"


namespace universal {


    /// "NBT" chunks.
    class V10Chunk {
    public:
        ChunkData* chunkData = nullptr;
        DataManager* dataManager = nullptr;

        MU void readChunk(ChunkData* chunkDataIn, DataManager* managerIn, DIM dim);
        MU void writeChunk(ChunkData* chunkDataIn, DataManager* managerOut, DIM dim);

        V10Chunk() = default;
    };

}