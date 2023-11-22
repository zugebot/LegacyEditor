#pragma once

#include "LegacyEditor/utils/enums.hpp"

#include "ChunkParserBase.hpp"
#include "LegacyEditor/LCE/Chunk/ChunkData.hpp"


namespace universal {

    /// "Aquatic" chunks.
    class V12Chunk : public ChunkParserBase {
    private:
        /// used for making writeLights faster
        std::vector<int> sectionOffsets;

        enum GRID_STATE {
            _0_SINGLE_BLOCK = 0,
            _1_BIT = 2,
            _1_BIT_SUBMERGED = 3,
            _2_BIT = 4,
            _2_BIT_SUBMERGED = 5,
            _3_BIT = 6,
            _3_BIT_SUBMERGED = 7,
            _4_BIT = 8,
            _4_BIT_SUBMERGED = 9,
            _8_FULL_BLOCKS = 0x0e,
            _8_FULL_BLOCKS_SUBMERGED = 0x0f
        };

        /**
         * key: grid format\n
         * return: size of memory that is being written to for that grid\n
         */
        static constexpr u32 GRID_SIZES[16] = {0, 0, 12, 20, 24, 40, 40, 64, 64, 96, 0, 0, 0, 0, 128, 256};


    public:
        ChunkData chunkData;
        DataManager dataManager;

        MU void readChunk(DataManager& managerIn, DIM dim);
        MU void writeChunk(DataManager& managerOut, DIM);

        V12Chunk() { sectionOffsets.reserve(64); }

        void placeBlock(int x, int y, int z, u16 block, u16 data = 0, bool waterlogged = false);
        u16 getBlock(int x, int y, int z);


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
        void writeLightSection(u8_vec& light, int& readOffset);
        void writeLight(int index, int& readOffset, u8_vec& light);
        void writeLightData();
        void writeNBTData();
    };
}
