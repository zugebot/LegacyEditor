#pragma once

#include "chunkData.hpp"

#include "LegacyEditor/LCE/MC/enums.hpp"



class DataManager;

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
        static constexpr int SECTION_COUNT = 16;
        static constexpr int GRID_COUNT = 64;
        static constexpr int GRID_SIZE = 128;
        static constexpr int DATA_SECTION_SIZE = 128;
        static constexpr int MAP_SIZE = 65536;

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
        bool readGrid(const u8* buffer, u8 grid[GRID_SIZE]) const;
        template<size_t BitsPerBlock>
        bool readGridSubmerged(u8 const* buffer, u8 blockGrid[GRID_SIZE], u8 SbmrgGrid[GRID_SIZE]) const;
        void readLightData() const;

        // #####################################################
        // #               Write Section
        // #####################################################

        void writeBlockData() const;
        template<size_t BitsPerBlock, size_t BlockCount, size_t EmptyCount>
        void writeGrid(u16_vec& blockVector, u16_vec& blockLocations, u8 blockMap[MAP_SIZE]) const;
        void writeWithMaxBlocks(const u16_vec& blockVector, const u16_vec& blockLocations, u8 blockMap[MAP_SIZE]) const;
        void writeLightSection(u32& readOffset, const u8_vec& light) const;
        void writeLightData() const;
    };
}
