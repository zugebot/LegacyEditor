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

        NBTBase root = makeCompound({});
        root.read(reader);
        const auto& level = root[""]["Level"];
        if (!level || !level->is<NBTCompound>()) return;

        chunkData->chunkX = level->getOr<i32>("xPos", 0);
        chunkData->chunkZ = level->getOr<i32>("zPos", 0);
        chunkData->lastUpdate = level->getOr<i64>("LastUpdate", 0);

        chunkData->terrainPopulated = level->getOr<u8>(
                "TerrainPopulated", "TerrainPopulatedFlags", 0);



        if (auto blocks = level->extract("Blocks")) {
            chunkData->oldBlocks = std::move(blocks->get<NBTByteArray>());
            if (chunkData->oldBlocks.size() == 32768) {
                chunkData->chunkHeight = 128;
            }
        } else {

        }

        if (auto data = level->extract("Data")) {
            chunkData->blockData = std::move(data->get<NBTByteArray>());
        }
        if (auto heightMap = level->extract("HeightMap")) {
            chunkData->heightMap = std::move(heightMap->get<NBTByteArray>());
        }
        if (auto biomes = level->extract("Biomes")) {
            chunkData->biomes = std::move(biomes->get<NBTByteArray>());
        }
        if (auto skylight = level->extract("SkyLight")) {
            memcpy(chunkData->skyLight.data(),
                   skylight->get<NBTByteArray>().data(),
                   skylight->get<NBTByteArray>().size());
        }
        if (auto blockLight = level->extract("BlockLight")) {
            memcpy(chunkData->blockLight.data(),
                   blockLight->get<NBTByteArray>().data(),
                   blockLight->get<NBTByteArray>().size());
        }

        chunkData->entities = level->extract("Entities").value_or(makeList(eNBT::COMPOUND));
        chunkData->tileEntities = level->extract("TileEntities").value_or(makeList(eNBT::COMPOUND));
        chunkData->tileTicks = level->extract("TileTicks").value_or(makeList(eNBT::COMPOUND));

        chunkData->validChunk = true;

    }


    void ChunkVNBT::writeChunkInternal(DataWriter& writer, bool fastMode) {
        NBTCompound level;

        level["xPos"] = makeInt(chunkData->chunkX);
        level["zPos"] = makeInt(chunkData->chunkZ);
        level["LastUpdate"] = makeLong(chunkData->lastUpdate);
        level["TerrainPopulated"] = makeByte(chunkData->terrainPopulated);

        if (!chunkData->oldBlocks.empty()) {
            int size = (chunkData->chunkHeight * 16 * 16);
            // std::cout << "oldBlocks: " << size << "\n";
            level["Blocks"] = makeByteArray(NBTByteArray(
                    chunkData->oldBlocks.begin(),
                    chunkData->oldBlocks.begin() + size));
        }


        if (!chunkData->blockData.empty()) {
            // std::cout << "blockData: " << chunkData->blockData.size() << "\n";
            level["Data"] = makeByteArray(chunkData->blockData);
        }

        if (!chunkData->heightMap.empty()) {
            // std::cout << "heightMap: " << chunkData->heightMap.size() << "\n";
            level["HeightMap"] = makeByteArray(chunkData->heightMap);
        }

        if (!chunkData->biomes.empty()) {
            // std::cout << "biomes: " << chunkData->biomes.size() << "\n";
            level["Biomes"] = makeByteArray(chunkData->biomes);
        }

        if (!chunkData->skyLight.empty()) {
            int size = (chunkData->chunkHeight * 16 * 16 / 2);
            // std::cout << "skyLight: " << size << "\n";

            level["SkyLight"] = makeByteArray(NBTByteArray(
                    chunkData->skyLight.begin(),
                    chunkData->skyLight.begin() + size));
        }

        if (!chunkData->blockLight.empty()) {
            int size = (chunkData->chunkHeight * 16 * 16 / 2);
            // std::cout << "blockLight: " << size << "\n";
            level["BlockLight"] = makeByteArray(NBTByteArray(
                    chunkData->blockLight.begin(),
                    chunkData->blockLight.begin() + size));
        }

        if (chunkData->entities.is<NBTList>()) {
            level["Entities"] = chunkData->entities;
        }

        if (chunkData->tileEntities.is<NBTList>()) {
            level["TileEntities"] = chunkData->tileEntities;
        }

        if (chunkData->tileTicks.is<NBTList>()) {
            level["TileTicks"] = chunkData->tileTicks;
        }

        NBTCompound root;
        root[""]["Level"] = makeCompound(std::move(level));

        NBTBase finalNBT = makeCompound(std::move(root));
        finalNBT.write(writer);
    }

}