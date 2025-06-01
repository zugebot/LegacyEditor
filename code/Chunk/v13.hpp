#pragma once

#include "include/lce/processor.hpp"
#include "chunkData.hpp"

#include "common/error_status.hpp"
#include "vBase.hpp"

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
    class ChunkV13 : VChunkBase {
        static constexpr u32 DATA_HEADER_SIZE = 28;
        static constexpr u32 SECTION_HEADER_SIZE = 50;

        static constexpr u32 SECTION_COUNT = 16;
        static constexpr u32 GRID_COUNT = 64;
        static constexpr u32 GRID_SIZE = 128;
        static constexpr u32 MAP_SIZE = 65536;

        // Read Section

        void readBlockData(DataReader& reader) const;
        template<size_t BitsPerBlock>
        bool readGrid(c_u8* buffer, u8 grid[128]) const;
        template<size_t BitsPerBlock>
        bool readGridSubmerged(u8 const* buffer, u8 blockGrid[GRID_SIZE], u8 SbmrgGrid[GRID_SIZE]) const;

        // Write Section

        void writeBlockData(DataWriter& writer) const;
        template<size_t BitsPerBlock, size_t BlockCount, size_t EmptyCount>
        void writeGrid(DataWriter& writer, u16_vec& blockVector, u16_vec& blockLocations, u8 blockMap[MAP_SIZE]) const;
        void writeWithMaxBlocks(DataWriter& writer, const u16_vec& blockVector, const u16_vec& blockLocations, u8 blockMap[MAP_SIZE]) const;

    public:
        u16 maxGridAmount = 0;
        explicit ChunkV13(ChunkData* chunkDataIn) : VChunkBase(chunkDataIn) {}


        MU void allocChunk() const override;
        MU void readChunk(DataReader& reader) override;
        MU void writeChunk(DataWriter& writer, bool fastMode) override;

    };
}
