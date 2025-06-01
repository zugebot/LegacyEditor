#pragma once

#include "code/Chunk/chunkData.hpp"
#include "vBase.hpp"


namespace editor::chunk {

    /// "NBT" chunks.
    class ChunkVNBT : public VChunkBase {
    public:
        explicit ChunkVNBT(ChunkData* chunkDataIn) : VChunkBase(chunkDataIn) {}

        MU void allocChunk() const override;
        MU void readChunk(DataReader& reader) override;
        MU void writeChunkInternal(DataWriter& writer, bool fastMode) override;
    };

}