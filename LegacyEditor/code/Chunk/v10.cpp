#include "v10.hpp"

#include <cstring>

#include "LegacyEditor/utils/NBT.hpp"


namespace editor::chunk {

    void ChunkV10::allocChunk() const {
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
    void ChunkV10::readChunk() {
        allocChunk();

        dataManager->readInt8();
        c_auto* nbt = NBT::readTag(*dataManager);
        auto* chunkNBT = nbt->toType<NBTTagCompound>();

        chunkData->lastVersion = 10;
        chunkData->chunkX = chunkNBT->getPrimitive<i32>("xPos");
        chunkData->chunkZ = chunkNBT->getPrimitive<i32>("zPos");
        chunkData->lastUpdate = chunkNBT->getPrimitive<i64>("LastUpdate");

        c_auto* blockArray = chunkNBT->getByteArray("Blocks");
        std::memcpy(chunkData->oldBlocks.data(), blockArray->array, 65536);

        c_auto* blockDataArray = chunkNBT->getByteArray("Data");
        std::memcpy(chunkData->blockData.data(), blockDataArray->array, 32768);

        c_auto* heightMapArray = chunkNBT->getByteArray("HeightMap");
        std::memcpy(chunkData->heightMap.data(), heightMapArray->array, 256);

        c_auto* biomeArray = chunkNBT->getByteArray("Biomes");
        std::memcpy(chunkData->biomes.data(), biomeArray->array, 256);

        c_auto* skyLightArray = chunkNBT->getByteArray("SkyLight");
        std::memcpy(chunkData->skyLight.data(), skyLightArray->array, 32768);

        c_auto* blockLightArray = chunkNBT->getByteArray("BlockLight");
        std::memcpy(chunkData->blockLight.data(), blockLightArray->array, 32768);

        auto createAndCopy = [](c_auto* byteArray, const size_t size) {
            u8_vec result(size);
            std::memcpy(result.data(), byteArray->array, size);
            return result;
        };

        chunkData->oldBlocks = createAndCopy(chunkNBT->getByteArray("Blocks"), 65536);
        chunkData->blockData = createAndCopy(chunkNBT->getByteArray("Data"), 256);
        chunkData->heightMap = createAndCopy(chunkNBT->getByteArray("HeightMap"), 256);
        chunkData->biomes = createAndCopy(chunkNBT->getByteArray("Biomes"), 256);
        chunkData->skyLight = createAndCopy(chunkNBT->getByteArray("SkyLight"), 32768);
        chunkData->blockLight = createAndCopy(chunkNBT->getByteArray("BlockLight"), 32768);


        chunkData->NBTData = new NBTBase(new NBTTagCompound(), TAG_COMPOUND);
        chunkData->NBTData->toType<NBTTagCompound>()->setTag("Entities", chunkNBT->getTag("Entities").copy());
        chunkData->NBTData->toType<NBTTagCompound>()->setTag("TileEntities", chunkNBT->getTag("TileEntities").copy());
        chunkData->NBTData->toType<NBTTagCompound>()->setTag("TileTicks", chunkNBT->getTag("TileTicks").copy());

        // IDK if this is enough nbt is cringe
        chunkNBT->deleteAll();
        delete chunkNBT;
        delete nbt;

        chunkData->validChunk = true;

    }


    void ChunkV10::writeChunk() {

    }


}