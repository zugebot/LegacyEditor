#include "v10.hpp"


namespace chunk {

    // TODO: This can definitely be made much faster if you could somehow replace
    // TODO: The pointer a vector points to... I will look that up later.

    // TODO: add cases for when tags are not found
    void ChunkV10::readChunk(ChunkData* chunkDataIn, DataManager* managerIn, DIM dim) {
        dataManager = managerIn;
        chunkData = chunkDataIn;

        dataManager->readInt8();
        auto* nbt = NBT::readTag(*dataManager);
        auto* chunkNBT = nbt->toType<NBTTagCompound>();

        chunkData->lastVersion = 10;
        chunkData->chunkX = chunkNBT->getPrimitive<i32>("xPos");
        chunkData->chunkZ = chunkNBT->getPrimitive<i32>("zPos");
        chunkData->lastUpdate = chunkNBT->getPrimitive<i64>("LastUpdate");

        auto* blockArray = chunkNBT->getByteArray("Blocks");
        chunkData->oldBlocks = u8_vec(65536);
        memcpy(&chunkData->oldBlocks[0], blockArray->array, 65536);

        auto* blockDataArray = chunkNBT->getByteArray("Data");
        chunkData->blockData = u8_vec(32768);
        memcpy(&chunkData->blockData[0], blockDataArray->array, 32768);

        auto* heightMapArray = chunkNBT->getByteArray("HeightMap");
        chunkData->heightMap = u8_vec(256);
        memcpy(&chunkData->heightMap[0], heightMapArray->array, 256);

        auto* biomeArray = chunkNBT->getByteArray("Biomes");
        chunkData->biomes = u8_vec(256);
        memcpy(&chunkData->biomes[0], biomeArray->array, 256);

        auto* skyLightArray = chunkNBT->getByteArray("SkyLight");
        chunkData->skyLight = u8_vec(32768);
        memcpy(&chunkData->skyLight[0], skyLightArray->array, 32768);

        auto* blockLightArray = chunkNBT->getByteArray("BlockLight");
        chunkData->blockLight = u8_vec(32768);
        memcpy(&chunkData->blockLight[0], blockLightArray->array, 32768);

        auto createAndCopy = [](const auto* byteArray, size_t size) {
            u8_vec result(size);
            memcpy(&result[0], byteArray->array, size);
            return result;
        };
        chunkData->oldBlocks = createAndCopy(chunkNBT->getByteArray("Blocks"), 65536);
        chunkData->blockData = createAndCopy(chunkNBT->getByteArray("Data"), 256);
        chunkData->heightMap = createAndCopy(chunkNBT->getByteArray("HeightMap"), 256);
        chunkData->biomes = createAndCopy(chunkNBT->getByteArray("Biomes"), 256);
        chunkData->skyLight = createAndCopy(chunkNBT->getByteArray("SkyLight"), 32768);
        chunkData->blockLight = createAndCopy(chunkNBT->getByteArray("BlockLight"), 32768);


        chunkData->NBTData = new NBTBase(new NBTTagCompound(), NBTType::TAG_COMPOUND);
        chunkData->NBTData->toType<NBTTagCompound>()->setTag("Entities", chunkNBT->getTag("Entities").copy());
        chunkData->NBTData->toType<NBTTagCompound>()->setTag("TileEntities", chunkNBT->getTag("TileEntities").copy());
        chunkData->NBTData->toType<NBTTagCompound>()->setTag("TileTicks", chunkNBT->getTag("TileTicks").copy());

        // IDK if this is enough nbt is cringe
        chunkNBT->deleteAll();
        delete chunkNBT;
        delete nbt;

        chunkData->validChunk = true;

    }


    void ChunkV10::writeChunk(ChunkData* chunkDataIn, DataManager* managerOut, DIM dim) {

    }


}