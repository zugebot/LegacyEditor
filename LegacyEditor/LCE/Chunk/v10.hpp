#pragma once

#include "chunkData.hpp"

class DataManager;


namespace editor::chunk {


    /// "NBT" chunks.
    class ChunkV10 {
    private:
        ChunkData* chunkData = nullptr;
        DataManager* dataManager = nullptr;

    public:
        ChunkV10() = default;
        MU void allocChunk() const;
        MU void readChunk(ChunkData* chunkDataIn, DataManager* managerIn);
        MU void writeChunk(ChunkData* chunkDataIn, DataManager* managerOut);
    };

}