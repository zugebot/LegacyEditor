#pragma once

#include "chunkFormatBase.hpp"
#include "code/chunk/chunkData.hpp"


namespace editor {

    /// "NBT" chunks.
    class ChunkFormatNBT : public ChunkFormatBase<ChunkFormatNBT> {
    public:

        MU static void readChunk(ChunkData* chunkData, DataReader& reader);
        MU static void writeChunkInternal(ChunkData* chunkData, WriteSettings& settings, DataWriter& writer, bool fastMode);
    };

}