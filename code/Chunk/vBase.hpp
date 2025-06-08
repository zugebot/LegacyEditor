#pragma once

#include "code/Chunk/chunkData.hpp"
#include "code/Chunk/helpers/NBTFixer.hpp"

#include "common/DataReader.hpp"
#include "common/DataWriter.hpp"


namespace editor::chunk {

    class VChunkBase {
    public:
        ChunkData* chunkData = nullptr;

        explicit VChunkBase(ChunkData* chunkDataIn);

        virtual void readChunk(DataReader& reader) = 0;
        virtual void writeChunkInternal(DataWriter& writer, bool fastMode) = 0;

        void writeChunk(DataWriter& writer) {
            writeChunkInternal(writer, false);
        }


        void readNBT(DataReader& reader) const {
            if (*reader.ptr() == 0x0A) {
                NBTBase nbtRoot = NBTBase::read(reader);
                auto nbt = nbtRoot[""];

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



        void writeNBT(DataWriter& writer) const {




            // chunkData->entities = NBTList(eNBT::COMPOUND);
            // chunkData->tileEntities = NBTList(eNBT::COMPOUND);
            // chunkData->tileTicks = NBTList(eNBT::COMPOUND);

            NBTBase nbt = makeCompound();

            nbt[""]["Entities"]     = makeList(std::move(chunkData->entities));
            nbt[""]["TileEntities"] = makeList(std::move(chunkData->tileEntities));
            nbt[""]["TileTicks"]    = makeList(std::move(chunkData->tileTicks));

            nbt.write(writer);
        }

    private:


    };

}




