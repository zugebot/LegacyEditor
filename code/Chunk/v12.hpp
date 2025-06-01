#pragma once

#include "code/Chunk/chunkData.hpp"
#include "common/error_status.hpp"
#include "common/fixedVector.hpp"
#include "vBase.hpp"


namespace editor::chunk {


    enum V12_GRID_STATE : u8 {
        V12_0_UNO = 0,
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
            0, 0, 12, 20, 24, 40, 40, 64, 64, 96, 0, 0, 0, 0, 128, 256
    };


    /// "Aquatic" + "Village & Pillage" chunks.
    class ChunkV12 : public VChunkBase {
        static constexpr int SECTION_COUNT = 16;
        static constexpr int GRID_COUNT = 64;
        static constexpr int MAP_SIZE = 65536;

        static constexpr u32 SECTION_HEADER_SIZE = 50;
        static constexpr int GRID_SIZE = 128;

        static constexpr eBlockOrder BLOCK_ORDER = eBlockOrder::yXZy;
        // Read Section

        static void setBlocks(u16_vec& writeVec, c_u8* grid, MU int gridOffset) ;
        void readBlockData(DataReader& reader) const;
        template<size_t BitsPerBlock>
        bool readGrid(c_u8* buffer, u8 grid[GRID_SIZE]) const;
        template<size_t BitsPerBlock>
        bool readGridSubmerged(u8 const* buffer, u8 blockGrid[GRID_SIZE], u8 SbmrgGrid[GRID_SIZE]) const;

        // Write Section

        using u16FixVec_t = FixedVector<u16, GRID_COUNT>;

        void writeBlockData(DataWriter& writer) const;

        template<size_t BitsPerBlock, size_t BlockCount, size_t EmptyCount>
        void writeGrid(DataWriter& writer, u16FixVec_t& blockVector, u16FixVec_t& blockLocations, u8 blockMap[MAP_SIZE]) const;
        static void writeWithMaxBlocks(DataWriter& writer, const u16FixVec_t& blockVector, const u16FixVec_t& blockLocations, u8 blockMap[MAP_SIZE]) ;

        template<size_t BitsPerBlock, size_t BlockCount, size_t EmptyCount>
        void writeGridSubmerged(DataWriter& writer, u16FixVec_t& blockVector, u16FixVec_t& blockLocations,
                                const u16FixVec_t& sbmrgLocations, u8 blockMap[MAP_SIZE]) const;

    public:
        explicit ChunkV12(ChunkData* chunkDataIn) : VChunkBase(chunkDataIn) {}


        MU void allocChunk() const override;
        MU void readChunk(DataReader& reader) override;
        MU void writeChunkInternal(DataWriter& writer, bool fastMode) override;

    };
}
