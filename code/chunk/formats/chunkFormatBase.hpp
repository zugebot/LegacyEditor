#pragma once

#include "code/Chunk/chunkData.hpp"
#include "code/Chunk/helpers/NBTFixer.hpp"

#include "code/SaveFile/writeSettings.hpp"
#include "common/DataReader.hpp"
#include "common/DataWriter.hpp"


namespace editor {

    template<typename Derived>
    class ChunkFormatBase {
    public:

        ChunkFormatBase() = delete;

        static void readChunk(ChunkData* chunkData, DataReader& reader) {
            Derived::readChunk(chunkData, reader);
        }

        static void writeChunkInternal(ChunkData* chunkData, WriteSettings& settings, DataWriter& writer, bool fastMode) {
            Derived::writeChunkInternal(chunkData, settings, writer, fastMode);
        }

        static void writeChunk(ChunkData* chunkData, WriteSettings& settings, DataWriter& writer) {
            Derived::writeChunkInternal(chunkData, settings, writer, false);
        }

        static void readNBT(ChunkData* chunkData, DataReader& reader) {
            if (*reader.ptr() == 0x0A) {
                NBTBase nbtRoot = NBTBase::read(reader);
                auto& nbt = nbtRoot[""];

                chunkData->entities = nbt.extract("Entities").value_or(makeList(eNBT::COMPOUND)).get<NBTList>();

                if (chunkData->entities.subType() == eNBT::NONE) {
                    chunkData->entities = NBTList(eNBT::COMPOUND);
                }

                chunkData->tileEntities = nbt.extract("TileEntities").value_or(makeList(eNBT::COMPOUND)).get<NBTList>();
                if (chunkData->tileEntities.subType() == eNBT::NONE) {
                    chunkData->tileEntities = NBTList(eNBT::COMPOUND);
                }

                chunkData->tileTicks = nbt.extract("TileTicks").value_or(makeList(eNBT::COMPOUND)).get<NBTList>();
                if (chunkData->tileTicks.subType() == eNBT::NONE) {
                    chunkData->tileTicks = NBTList(eNBT::COMPOUND);
                }
            }
        }

        static void writeNBT(ChunkData* chunkData, DataWriter& writer) {
            // chunkData->entities = NBTList(eNBT::COMPOUND);
            // chunkData->tileEntities = NBTList(eNBT::COMPOUND);
            // chunkData->tileTicks = NBTList(eNBT::COMPOUND);
            NBTBase nbt = makeCompound();
            nbt[""]["Entities"]     = makeList(std::move(chunkData->entities));
            nbt[""]["TileEntities"] = makeList(std::move(chunkData->tileEntities));
            nbt[""]["TileTicks"]    = makeList(std::move(chunkData->tileTicks));
            nbt.write(writer);
        }
    };

}




