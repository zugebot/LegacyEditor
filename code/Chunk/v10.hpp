#pragma once

#include "vBase.hpp"
#include "code/Chunk/chunkData.hpp"


namespace editor::chunk {

    /// "NBT" chunks.
    class ChunkVNBT : public VChunkBase {
    public:
        explicit ChunkVNBT(ChunkData* chunkDataIn) : VChunkBase(chunkDataIn) {}

        MU void readChunk(DataReader& reader) override;
        MU void writeChunkInternal(DataWriter& writer, bool fastMode) override;
    };

}