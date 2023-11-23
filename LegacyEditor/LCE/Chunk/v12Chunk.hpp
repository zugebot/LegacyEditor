#pragma once

#include "LegacyEditor/utils/enums.hpp"

#include "LegacyEditor/LCE/Chunk/ChunkData.hpp"


namespace universal {


    enum GRID_STATE {
        V12_0_SINGLE_BLOCK = 0,
        V12_1_BIT = 2,
        V12_1_BIT_SUBMERGED = 3,
        V12_2_BIT = 4,
        V12_2_BIT_SUBMERGED = 5,
        V12_3_BIT = 6,
        V12_3_BIT_SUBMERGED = 7,
        V12_4_BIT = 8,
        V12_4_BIT_SUBMERGED = 9,
        V12_8_FULL_BLOCKS = 0x0e,
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
    class V12Chunk {
    public:
        ChunkData* chunkData = nullptr;
        DataManager* dataManager = nullptr;

        MU void readChunk(ChunkData* chunkDataIn, DataManager* managerIn, DIM dim);
        MU void writeChunk(ChunkData* chunkDataIn, DataManager* managerOut, DIM dim);

        V12Chunk() = default;

    private:

        // #####################################################
        // #               Read Section
        // #####################################################

        void readNBTData();
        void readLightData();
        void readBlockData();
        static void placeBlocks(u16_vec& writeVec, const u8* grid, int writeOffset);
        static void fillWithMaxBlocks(const u8* buffer, u8* grid);
        template<size_t BitsPerBlock>
        bool parseLayer(const u8* buffer, u8* grid);
        template<size_t BitsPerBlock>
        bool parseWithLayers(u8 const* buffer, u8* grid, u8* submergedGrid);

        // #####################################################
        // #               Write Section
        // #####################################################

        void writeBlockData();
        /// used to write only the palette and positions.\n
        /// It does not write liquid data, because I have been told that that is unnecessary.
        template<size_t BitsPerBlock>
        void writeLayer(u16_vec& blocks, u16_vec& positions);
        /// used to write full block data, instead of using palette.
        void writeWithMaxBlocks(u16_vec& blocks, u16_vec& positions);
        void writeLightSection(int index, u32& readOffset, u8_vec& light);
        void writeLightData();
        void writeNBTData();
    };
}
