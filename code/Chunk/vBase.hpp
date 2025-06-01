#pragma once

#pragma once

#include "code/Chunk/chunkData.hpp"
#include "common/DataReader.hpp"
#include "common/DataWriter.hpp"

namespace editor::chunk {

    class VChunkBase {
    public:
        ChunkData* chunkData = nullptr;

        explicit VChunkBase(ChunkData* chunkDataIn) : chunkData(chunkDataIn) {}

        virtual void allocChunk() const = 0;
        virtual void readChunk(DataReader& reader) = 0;
        virtual void writeChunk(DataWriter& writer, bool fastMode) = 0;

        void writeChunk(DataWriter& writer) {
            writeChunk(writer, false);
        }

    };

}




