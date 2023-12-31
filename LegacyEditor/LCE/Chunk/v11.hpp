#pragma once

#include "chunkData.hpp"


class DataManager;

namespace editor::chunk {

    /// "Elytra" chunks.
    class ChunkV11 {
        static constexpr i32 GRID_SIZE = 128;
        static constexpr int DATA_SECTION_SIZE = 128;
        static constexpr i32 GRID_HEADER_SIZE = 1024;

    public:
        ChunkData* chunkData = nullptr;
        DataManager* dataManager = nullptr;

        ChunkV11() = default;
        MU void allocChunk() const;
        MU void readChunk(ChunkData* chunkDataIn, DataManager* managerIn);
        MU void writeChunk(ChunkData* chunkDataIn, DataManager* managerOut);

    private:
        MU void readBlocks() const;
        template<size_t BitsPerBlock>
        MU static bool readGrid(u8 const* buffer, u8 grid[GRID_SIZE]);
        MU void readData() const;
    };


}