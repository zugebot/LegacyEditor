#pragma once

#include "chunkData.hpp"

#include "LegacyEditor/utils/error_status.hpp"


class DataManager;

namespace editor::chunk {


    enum V13_GRID_STATE : u8 {
        V13_0_UNO = 0,
        V13_1_BIT = 2,
        V13_1_BIT_SUBMERGED = 3,
        V13_2_BIT = 4,
        V13_2_BIT_SUBMERGED = 5,
        V13_3_BIT = 6,
        V13_3_BIT_SUBMERGED = 7,
        V13_4_BIT = 8,
        V13_4_BIT_SUBMERGED = 9,
        V13_8_FULL = 0x0E,
        V13_8_FULL_BLOCKS_SUBMERGED = 0x0F,
    };


    /**
     * key: grid format\n
     * return: size of memory that is being written to for that grid\n
     */
    static constexpr u32 V13_GRID_SIZES[16] = {
        0, 0, 12, 20, 24, 40, 40, 64, 64, 96, 0, 0, 0, 0, 128, 256
};


    /// "Aquatic" chunks.
    class ChunkV13 {
        static constexpr u32 DATA_HEADER_SIZE = 28;
        static constexpr u32 SECTION_HEADER_SIZE = 50;

        static constexpr u32 SECTION_COUNT = 16;
        static constexpr u32 GRID_COUNT = 64;
        static constexpr u32 GRID_SIZE = 128;
        static constexpr u32 MAP_SIZE = 65536;

        // Read Section

        void readBlockData() const;
        template<size_t BitsPerBlock>
        bool readGrid(const u8* buffer, u8 grid[GRID_SIZE]) const;
        template<size_t BitsPerBlock>
        bool readGridSubmerged(u8 const* buffer, u8 blockGrid[GRID_SIZE], u8 SbmrgGrid[GRID_SIZE]) const;

        // Write Section

        void writeBlockData() const;
        template<size_t BitsPerBlock, size_t BlockCount, size_t EmptyCount>
        void writeGrid(u16_vec& blockVector, u16_vec& blockLocations, u8 blockMap[MAP_SIZE]) const;
        void writeWithMaxBlocks(const u16_vec& blockVector, const u16_vec& blockLocations, u8 blockMap[MAP_SIZE]) const;

    public:
        ChunkData* chunkData = nullptr;
        DataManager* dataManager = nullptr;
        uint16_t maxGridAmount = 0;

        ChunkV13(ChunkData* chunkDataIn, DataManager* managerIn) : chunkData(chunkDataIn), dataManager(managerIn) {}
        MU void allocChunk() const;
        MU void readChunk();
        MU void writeChunk() const;

    };
}
