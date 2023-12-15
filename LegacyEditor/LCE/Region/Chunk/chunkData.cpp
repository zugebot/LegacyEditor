#include "chunkData.hpp"

namespace chunk {

    ChunkData::~ChunkData() {
        if (NBTData != nullptr) {
            NBTData->NbtFree();
            delete NBTData;
            NBTData = nullptr;
        }
    }

    void ChunkData::defaultNBT() {
        if (NBTData != nullptr) {
            NBTData->toType<NBTTagCompound>()->deleteAll();
            delete NBTData;
        }

        NBTData = new NBTBase(new NBTTagCompound(), TAG_COMPOUND);
        auto* chunkRootNbtData = static_cast<NBTTagCompound*>(NBTData->data);
        auto* entities = new NBTTagList();
        auto* tileEntities = new NBTTagList();
        auto* tileTicks = new NBTTagList();
        chunkRootNbtData->setListTag("Entities", entities);
        chunkRootNbtData->setListTag("TileEntities", tileEntities);
        chunkRootNbtData->setListTag("TileTicks", tileTicks);
    }

    ND std::string ChunkData::getCoords() const {
        return "(" + std::to_string(chunkX) + ", " + std::to_string(chunkZ) + ")";
    }


}


