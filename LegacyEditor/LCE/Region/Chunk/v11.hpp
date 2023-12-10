#pragma once

#include "chunkData.hpp"


namespace chunk {


    /// "Elytra" chunks.
    class ChunkV11 {
    private:
        static constexpr i32 GRID_HEADER_SIZE = 1024;

    public:
        ChunkData* chunkData = nullptr;
        DataManager* dataManager = nullptr;

        MU void readChunk(ChunkData* chunkDataIn, DataManager* managerIn, DIM dim);
        MU void writeChunk(ChunkData* chunkDataIn, DataManager* managerOut, DIM dim);

        ChunkV11() = default;

    private:
        MU void readBlocks() const;
        template<size_t BitsPerBlock>
        MU bool readGrid(u8 const* buffer, u8 grid[128]) const;
        MU void readData() const;
    };


}