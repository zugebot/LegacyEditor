#pragma once

#include "ChunkParserBase.hpp"
#include "ChunkParserData.hpp"


namespace universal {
    /**
     * "NBT" chunks.
     */
    class V10Chunk : public ChunkParserBase {
    public:
        ChunkParserData chunkData;
        DataManager dataManager;
    };
}
