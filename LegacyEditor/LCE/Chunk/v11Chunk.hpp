#pragma once

#include "ChunkParserBase.hpp"
#include "LegacyEditor/LCE/Chunk/ChunkData.hpp"


namespace universal {
    /**
     * "Elytra" chunks.
     */
    class V11Chunk : public ChunkParserBase {
    public:
        ChunkData chunkData;
        DataManager dataManager;
    };
}
