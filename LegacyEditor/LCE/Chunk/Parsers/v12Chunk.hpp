#pragma once

#include "ChunkParserBase.hpp"
#include "ChunkParserData.hpp"


namespace universal {
    /**
     * "Aquatic" chunks.
     */
    class V12Chunk : public ChunkParserBase {
    public:
        ChunkParserData chunkData;
        DataManager dataManager;

        void readChunk(DataManager& managerIn, DIM dim);
        void readChunkForAccess(DataManager& managerIn, DIM dim);

        void writeChunk(DataManager& managerOut, DIM);
        void writeChunkForAccess(DataManager& managerIn, DIM dim);


    private:

        // #####################################################
        // #               Read Section
        // #####################################################

        void readNBTData();
        void readLights();
        void readBlocks();
        void putBlocks(u16_vec writeVec, const u16* readArray, int writeOffset);

        static void singleBlock(u16 v1, u16 v2, u16* grid);
        static void maxBlocks(u8 const* buffer, u16* grid);

        template<size_t BitsPerBlock>
        bool read(u8 const* buffer, u16* grid);

        template<size_t BitsPerBlock>
        bool parseWithLayers(u8 const* buffer, u16* grid, u16* submergedGrid);

        // #####################################################
        // #               Write Section
        // #####################################################
    };
}
