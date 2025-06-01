#include "v10.hpp"

#include "common/nbt.hpp"


namespace editor::chunk {

    void ChunkVNBT::allocChunk() const {
        chunkData->oldBlocks = u8_vec(65536);
        chunkData->blockData = u8_vec(32768);
        chunkData->heightMap = u8_vec(256);
        chunkData->biomes = u8_vec(256);
        chunkData->skyLight = u8_vec(32768);
        chunkData->blockLight = u8_vec(32768);

    }

    // TODO: This can definitely be made much faster if you could somehow replace
    // TODO: The pointer a vector points to... I will look that up later.

    // TODO: add cases for when tags are not found
    void ChunkVNBT::readChunk(DataReader& reader) {
        allocChunk();

        NBTBase nbt = makeCompound({});
        nbt.read(reader);
        auto compound = nbt.value<NBTCompound>("").value_or(NBTCompound{})
                           .value<NBTCompound>("Level").value_or(NBTCompound{});


        chunkData->chunkX = compound.value<i32>("xPos").value_or(0);
        chunkData->chunkZ = compound.value<i32>("zPos").value_or(0);
        chunkData->lastUpdate = compound.value<i64>("LastUpdate").value_or(0);
        chunkData->terrainPopulated = compound.value<u8>("TerrainPopulated").value_or(
                compound.value<u8>("TerrainPopulatedFlags").value_or(
                        0
                        )
        );


        if (auto blocks = compound.extract("Blocks")) {
            chunkData->oldBlocks = std::move(blocks->get<NBTByteArray>());
            if (chunkData->oldBlocks.size() == 32768) {
                chunkData->chunkHeight = 128;
            }
        } else {

        }

        if (auto data = compound.extract("Data")) {
            chunkData->blockData = std::move(data->get<NBTByteArray>());
        }
        if (auto heightMap = compound.extract("HeightMap")) {
            chunkData->heightMap = std::move(heightMap->get<NBTByteArray>());
        }
        if (auto biomes = compound.extract("Biomes")) {
            chunkData->biomes = std::move(biomes->get<NBTByteArray>());
        }
        if (auto skylight = compound.extract("SkyLight")) {
            memcpy(chunkData->skyLight.data(),
                   skylight->get<NBTByteArray>().data(),
                   skylight->get<NBTByteArray>().size());
        }
        if (auto blockLight = compound.extract("BlockLight")) {
            memcpy(chunkData->blockLight.data(),
                   blockLight->get<NBTByteArray>().data(),
                   blockLight->get<NBTByteArray>().size());
        }

        chunkData->entities = compound.extract("Entities").value_or(makeList(eNBT::COMPOUND));
        chunkData->tileEntities = compound.extract("TileEntities").value_or(makeList(eNBT::COMPOUND));
        chunkData->tileTicks = compound.extract("TileTicks").value_or(makeList(eNBT::COMPOUND));

        chunkData->validChunk = true;

    }


    void ChunkVNBT::writeChunk(DataWriter& writer, bool fastMode) {

    }


}