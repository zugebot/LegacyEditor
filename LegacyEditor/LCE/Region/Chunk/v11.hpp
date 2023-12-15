#pragma once

#include "chunkData.hpp"


namespace chunk {

    /// "Elytra" chunks.
    class ChunkV11 {
    public:
        ChunkData* chunkData = nullptr;
        DataManager* dataManager = nullptr;

        ChunkV11() = default;
        MU void allocChunk() const;
        MU void readChunk(ChunkData* chunkDataIn, DataManager* managerIn);
        MU void writeChunk(ChunkData* chunkDataIn, DataManager* managerOut);

    private:
        static constexpr i32 GRID_HEADER_SIZE = 1024;
        MU void readBlocks() const;
        template<size_t BitsPerBlock>
        MU bool readGrid(u8 const* buffer, u8 grid[128]) const;
        MU void readData() const;
    };


}