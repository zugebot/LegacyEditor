#pragma once

#include "chunkData.hpp"

class DataManager;


namespace editor::chunk {

    /// "Elytra" chunks.
    class ChunkV11 {
        static constexpr i32 GRID_SIZE = 64;
        static constexpr i32 GRID_HEADER_SIZE = 1024;

        // Read

        MU void readBlocks() const;
        template<size_t BitsPerBlock>
        MU static bool readGrid(u8 const* buffer, u8 grid[GRID_SIZE]);

        // Write
        // ...

    public:
        ChunkData* chunkData = nullptr;
        DataManager* dataManager = nullptr;

        ChunkV11(ChunkData* chunkDataIn, DataManager* managerIn) :
            chunkData(chunkDataIn), dataManager(managerIn) {}

        MU void allocChunk() const;
        MU void readChunk();
        MU void writeChunk();
    };

}