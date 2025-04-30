#pragma once

#include "code/Chunk/chunkData.hpp"

class DataManager;


namespace editor::chunk {

    /// "NBT" chunks.
    class ChunkVNBT {
        ChunkData* chunkData = nullptr;
        DataManager* dataManager = nullptr;

    public:
        ChunkVNBT(ChunkData* chunkDataIn, DataManager* managerIn) :
           chunkData(chunkDataIn), dataManager(managerIn) {}

        MU void allocChunk() const;
        MU void readChunk();
        MU void writeChunk();
    };

}