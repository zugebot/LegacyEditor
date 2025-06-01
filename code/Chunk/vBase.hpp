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
        virtual void writeChunkInternal(DataWriter& writer, bool fastMode) = 0;

        void writeChunk(DataWriter& writer) {
            writeChunkInternal(writer, false);
        }


        void readNBT(DataReader& reader) const {
            if (*reader.ptr() == 0x0A) {
                NBTBase nbtRoot = makeCompound({});
                nbtRoot.read(reader);
                auto nbt = nbtRoot[""];
                chunkData->entities = nbt->extract("Entities").value_or(makeList(eNBT::COMPOUND));
                chunkData->tileEntities = nbt->extract("TileEntities").value_or(makeList(eNBT::COMPOUND));
                chunkData->tileTicks = nbt->extract("TileTicks").value_or(makeList(eNBT::COMPOUND));
            }
        }

        void writeNBT(DataWriter& writer) const {
            NBTBase nbt = makeCompound();

            nbt[""]["Entities"]     = std::move(chunkData->entities);
            nbt[""]["TileEntities"] = std::move(chunkData->tileEntities);
            nbt[""]["TileTicks"]    = std::move(chunkData->tileTicks);

            nbt.write(writer);
        }



    };

}




