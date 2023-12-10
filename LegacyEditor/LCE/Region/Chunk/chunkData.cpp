#include "chunkData.hpp"

namespace chunk {


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


}


