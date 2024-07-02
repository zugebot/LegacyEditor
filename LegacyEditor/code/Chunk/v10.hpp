#pragma once

#include "LegacyEditor/code/chunk/chunkData.hpp"

class DataManager;


namespace editor::chunk {


    /// "NBT" chunks.
    class ChunkV10 {
        ChunkData* chunkData = nullptr;
        DataManager* dataManager = nullptr;

    public:
        ChunkV10(ChunkData* chunkDataIn, DataManager* managerIn) :
           chunkData(chunkDataIn), dataManager(managerIn) {}

        MU void allocChunk() const;
        MU void readChunk();
        MU void writeChunk();
    };

}