#pragma once

#include "LegacyEditor/LCE/Chunk/ChunkData.hpp"


namespace universal {


    /// "Elytra" chunks.
    class V11Chunk {
    public:
        ChunkData* chunkData = nullptr;
        DataManager dataManager;
    };


}
