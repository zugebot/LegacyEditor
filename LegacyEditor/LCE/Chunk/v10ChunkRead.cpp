#include "v10Chunk.hpp"


namespace universal {

    // TODO: This can definitely be made much faster if you could somehow replace
    // TODO: The pointer a vector points to... I will look that up later.

    // TODO: add cases for when tags are not found
    void V10Chunk::readChunk(ChunkData* chunkDataIn, DataManager* managerIn, DIM dim) {
        dataManager = managerIn;
        chunkData = chunkDataIn;

        dataManager->readInt8();
        auto* nbt = NBT::readTag(*dataManager);
        auto* chunkNBT = nbt->toType<NBTTagCompound>();

        chunkData->lastVersion = 10;
        chunkData->chunkX = chunkNBT->getPrimitive<i32>("xPos");
        chunkData->chunkZ = chunkNBT->getPrimitive<i32>("zPos");
        chunkData->lastUpdate = chunkNBT->getPrimitive<i64>("LastUpdate");

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
        auto* compound_root = chunkData->NBTData->toType<NBTTagCompound>();
        auto* compound = chunkData->NBTData->toType<NBTTagCompound>();
        compound_root->setCompoundTag("Level", compound);
        compound->setListTag("Entities", static_cast<NBTTagList*>(chunkNBT->getTag("Entities").copy().data));
        compound->setListTag("TileEntities", static_cast<NBTTagList*>(chunkNBT->getTag("TileEntities").copy().data));
        compound->setListTag("TileTicks", static_cast<NBTTagList*>(chunkNBT->getTag("TileTicks").copy().data));

        // IDK if this is enough nbt is cringe
        chunkNBT->deleteAll();
        delete chunkNBT;
        delete nbt;

    }


}