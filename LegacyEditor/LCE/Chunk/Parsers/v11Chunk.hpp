#pragma once

#include "ChunkParserBase.hpp"
#include "ChunkParserData.hpp"


namespace universal {
    /**
     * "Elytra" chunks.
     */
    class V11Chunk : public ChunkParserBase {
    public:
        ChunkParserData chunkData;
        DataManager dataManager;
    };
}
