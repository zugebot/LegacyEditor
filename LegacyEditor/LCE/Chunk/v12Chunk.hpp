#pragma once

#include "LegacyEditor/utils/enums.hpp"

#include "ChunkParserBase.hpp"
#include "LegacyEditor/LCE/Chunk/ChunkData.hpp"


namespace universal {
    /**
     * "Aquatic" chunks.
     */
    class V12Chunk : public ChunkParserBase {
    public:
        ChunkData chunkData;
        DataManager dataManager;

        MU void readChunk(DataManager& managerIn, DIM dim);
        MU void readChunkForAccess(DataManager& managerIn, DIM dim);

        MU void writeChunk(DataManager& managerOut, DIM);

    private:

        // #####################################################
        // #               Read Section
        // #####################################################

        void readNBTData();
        void readLights();
        void readBlocks();
        static void putBlocks(u16_vec& writeVec, const u16* readArray, int writeOffset);

        static void singleBlock(u16 v1, u16 v2, u16* grid);
        static void maxBlocks(u8 const* buffer, u16* grid);

        template<size_t BitsPerBlock>
        bool read(u8 const* buffer, u16* grid);

        template<size_t BitsPerBlock>
        bool parseWithLayers(u8 const* buffer, u16* grid, u16* submergedGrid);

        // #####################################################
        // #               Write Section
        // #####################################################

        void writeBlocks();
        void writeLights();
        void writeNBTData();
    };
}
