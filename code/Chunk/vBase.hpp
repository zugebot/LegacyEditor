#pragma once

#pragma once

#include "../../common/data/DataReader.hpp"
#include "../../common/data/DataWriter.hpp"
#include "code/Chunk/chunkData.hpp"

namespace editor::chunk {

    class VChunkBase {
    public:
        ChunkData* chunkData = nullptr;

        VChunkBase(ChunkData* chunkDataIn) : chunkData(chunkDataIn) {}

        virtual void allocChunk() const = 0;
        virtual void readChunk(DataReader& reader) = 0;
        virtual void writeChunk(DataWriter& writer) = 0;

    };

}




