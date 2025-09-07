#pragma once

#include "chunkFormatBase.hpp"
#include "code/Chunk/chunkData.hpp"
#include "common/error_status.hpp"
#include "common/fixedVector.hpp"


namespace editor {


    /// "Aquatic" + "Village & Pillage" chunks.
    class ChunkFormatGridPalettedSubmerged : public ChunkFormatBase<ChunkFormatGridPalettedSubmerged> {

        enum V12_GRID_STATE : u8 {
            V12_0_UNO = 0,
            V12_0_UNO_SUBMERGED = 1,
            V12_1_BIT = 2,
            V12_1_BIT_SUBMERGED = 3,
            V12_2_BIT = 4,
            V12_2_BIT_SUBMERGED = 5,
            V12_3_BIT = 6,
            V12_3_BIT_SUBMERGED = 7,
            V12_4_BIT = 8,
            V12_4_BIT_SUBMERGED = 9,
            V12_8_FULL = 0x0E,
            V12_8_FULL_SUBMERGED = 0x0F,
        };

        /**
         * key: grid format\n
         * return: size of memory that is being written to for that grid\n
         */
        static constexpr u32 V12_GRID_SIZES[16] = {
                  0, // V12_0_UNO = 0,
                  0, // unused
                 12, // V12_1_BIT = 2,
                 20, // V12_1_BIT_SUBMERGED = 3,
                 24, // V12_2_BIT = 4,
                 40, // V12_2_BIT_SUBMERGED = 5,
                 40, // V12_3_BIT = 6,
                 64, // V12_3_BIT_SUBMERGED = 7,
                 64, // V12_4_BIT = 8,
                 96, // V12_4_BIT_SUBMERGED = 9,
                  0,
                  0,
                  0,
                  0,
                128, // V12_8_FULL = 0x0E,
                256  // V12_8_FULL_SUBMERGED = 0x0F,
        };

        static constexpr int SECTION_COUNT = 16;
        static constexpr int GRID_COUNT = 64;
        static constexpr int MAP_SIZE = 65536;

        static constexpr u32 SECTION_HEADER_SIZE = 50;
        static constexpr int GRID_SIZE = 128;

        // Read Section

        static void setBlocks(u16_vec& writeVec, c_u8* grid, MU int gridOffset) ;
        static void readBlockData(ChunkData* chunkData, DataReader& reader);
        template<size_t BitsPerBlock>
        static bool readGrid(c_u8* buffer, u8 grid[GRID_SIZE]);
        template<size_t BitsPerBlock>
        static bool readGridSubmerged(u8 const* buffer, u8 blockGrid[GRID_SIZE], u8 SbmrgGrid[GRID_SIZE]);

        // Write Section

        using u16FixVec_t = FixedVector<u16, GRID_COUNT>;

        static void writeBlockData(ChunkData* chunkData, WriteSettings& settings, DataWriter& writer);

        template<size_t BitsPerBlock, size_t BlockCount, size_t EmptyCount>
        static void writeGrid(DataWriter& writer, u16FixVec_t& blockVector, u16FixVec_t& blockLocations, u8 blockMap[MAP_SIZE]);
        static void writeWithMaxBlocks(DataWriter& writer, const u16FixVec_t& blockVector, const u16FixVec_t& blockLocations, u8 blockMap[MAP_SIZE]);

        template<size_t BitsPerBlock, size_t BlockCount, size_t EmptyCount>
        static void writeGridSubmerged(DataWriter& writer, u16FixVec_t& blockVector, u16FixVec_t& blockLocations,
                                const u16FixVec_t& sbmrgLocations, u8 blockMap[MAP_SIZE]);

    public:

        MU static void readChunk(ChunkData* chunkData, DataReader& reader);
        MU static void writeChunkInternal(ChunkData* chunkData, WriteSettings& settings, DataWriter& writer, bool fastMode);

    };
}
