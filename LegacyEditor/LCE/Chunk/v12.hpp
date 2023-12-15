#pragma once

#include "chunkData.hpp"

#include "LegacyEditor/LCE/MC/enums.hpp"



class DataManager;

namespace editor::chunk {


    enum V12_GRID_STATE {
        V12_0_UNO = 0,
        V12_1_BIT = 2,
        V12_1_BIT_SUBMERGED = 3,
        V12_2_BIT = 4,
        V12_2_BIT_SUBMERGED = 5,
        V12_3_BIT = 6,
        V12_3_BIT_SUBMERGED = 7,
        V12_4_BIT = 8,
        V12_4_BIT_SUBMERGED = 9,
        V12_8_FULL = 0x0e,
        V12_8_FULL_BLOCKS_SUBMERGED = 0x0f
    };


    /**
     * key: grid format\n
     * return: size of memory that is being written to for that grid\n
     */
    static constexpr u32 V12_GRID_SIZES[16] = {
            0, 0, 12, 20, 24, 40, 40, 64, 64, 96, 0, 0, 0, 0, 128, 256
    };


    /// "Aquatic" chunks.
    class ChunkV12 {
    public:
        ChunkData* chunkData = nullptr;
        DataManager* dataManager = nullptr;

        ChunkV12() = default;
        MU void allocChunk() const;
        MU void readChunk(ChunkData* chunkDataIn, DataManager* managerIn);
        MU void writeChunk(ChunkData* chunkDataIn, DataManager* managerOut);


    private:

        // #####################################################
        // #               Read Section
        // #####################################################

        void readBlockData() const;
        template<size_t BitsPerBlock>
        bool readGrid(const u8* buffer, u8 grid[128]) const;
        template<size_t BitsPerBlock>
        bool readGridSubmerged(u8 const* buffer, u8 blockGrid[128], u8 SbmrgGrid[128]) const;
        void readLightData() const;

        // #####################################################
        // #               Write Section
        // #####################################################

        void writeBlockData() const;
        template<size_t BitsPerBlock, size_t BlockCount, size_t EmptyCount>
        void writeGrid(u16_vec& blockVector, u16_vec& blockLocations, u8 blockMap[65536]) const;
        void writeWithMaxBlocks(u16_vec& blockVector, u16_vec& blockLocations, u8 blockMap[65536]) const;
        void writeLightSection(u32& readOffset, u8_vec& light) const;
        void writeLightData() const;
    };
}
