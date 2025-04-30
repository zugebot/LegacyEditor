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
    void ChunkVNBT::readChunk() {
        allocChunk();

        dataManager->read<u8>();
        chunkData->oldNBTData.read(*dataManager);
        // auto& level = chunkData->oldNBTData.get<NBTCompound>();
        auto compound = chunkData->oldNBTData.tryGet<NBTCompound>("Level").value_or(NBTCompound{});
        // compound = level.copy();

        chunkData->chunkX = compound.tryGet<i32>("xPos").value_or(0);
        chunkData->chunkZ = compound.tryGet<i32>("zPos").value_or(0);
        chunkData->lastUpdate = compound.tryGet<i64>("LastUpdate").value_or(0);
        chunkData->terrainPopulated = compound.tryGet<u8>("TerrainPopulated").value_or(
                compound.tryGet<u8>("TerrainPopulatedFlags").value_or(
                        0
                        )
        );


        if (auto blocks = compound.extract("Blocks")) {
            chunkData->oldBlocks = std::move(blocks->get<NBTByteArray>());
            if (chunkData->oldBlocks.size() == 32768) {
                chunkData->chunkHeight = 128;
            }
        } else {
            // std::cout << "No blocks found in NBT???\n";
        }
        // std::cout << chunkData->oldBlocks.size() << "\n";

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

        chunkData->entities = compound.extract("Entities").value_or(makeList(eNBT::COMPOUND, {}));
        chunkData->tileEntities = compound.extract("TileEntities").value_or(makeList(eNBT::COMPOUND, {}));
        chunkData->tileTicks = compound.extract("TileTicks").value_or(makeList(eNBT::COMPOUND, {}));

        // chunkData->oldNBTData = NBTBase();

        chunkData->validChunk = true;

    }


    void ChunkVNBT::writeChunk() {

    }


}