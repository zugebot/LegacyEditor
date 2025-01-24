#include "chunkData.hpp"

#include "common/NBT.hpp"
#include "include/lce/blocks/blockID.hpp"


namespace editor::chunk {


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


    MU ND std::string ChunkData::getCoords() const {
        return "(" + std::to_string(chunkX) + ", " + std::to_string(chunkZ) + ")";
    }


    MU void ChunkData::convertNBTToAquatic() {
        newBlocks = u16_vec(65536);
        for (int xIter = 0; xIter < 16; xIter++) {
            for (int zIter = 0; zIter < 16; zIter++) {
                for (int yIter = 0; yIter < 256; yIter++) {
                    c_int offset = (yIter % 128) + xIter * 128 + zIter * 128 * 16 + 32768 * (yIter > 127);
                    c_u16 blockID = oldBlocks[offset];
                    u16 dataTag;
                    if (offset % 2 == 0) {
                        dataTag = blockData[offset / 2] & 0x0F;
                    } else {
                        dataTag = (blockData[offset / 2] & 0xF0) >> 4;
                    }
                    u16 elytraBlock = blockID << 4 | dataTag;
                    c_int AquaticOffset = yIter + 4096 * zIter + 256 * xIter;
                    newBlocks[AquaticOffset] = elytraBlock;
                }
            }
        }
        lastVersion = 12;
        u8_vec().swap(oldBlocks);
    }


    MU void ChunkData::convertOldToAquatic() {
        newBlocks = u16_vec(65536);
        for (int xIter = 0; xIter < 16; xIter++) {
            for (int zIter = 0; zIter < 16; zIter++) {
                for (int yIter = 0; yIter < 256; yIter++) {
                    c_int offset = yIter * 256 + zIter * 16 + xIter;
                    c_u16 blockID = oldBlocks[offset];
                    u16 dataTag;
                    if (offset % 2 == 0) {
                        dataTag = blockData[offset / 2] & 0x0F;
                    } else {
                        dataTag = (blockData[offset] & 0xF0) >> 8;
                    }
                    u16 elytraBlock = blockID << 4 | dataTag;
                    c_int AquaticOffset = yIter + 4096 * zIter + 256 * xIter;
                    newBlocks[AquaticOffset] = elytraBlock;
                }
            }
        }
        lastVersion = 12;
        u8_vec().swap(oldBlocks);
    }


    /**
     * Still a work in progress.
     *
     */
    MU void ChunkData::convert114ToAquatic() {

        // remove 1.14 blocks here...
        for (int i = 0; i < 65536; i++) {
            c_u16 id = newBlocks[i] >> 4 & 1023;
            if (id > 318) {
                newBlocks[i] = lce::blocks::COBBLESTONE_ID;
            }
        }

        lastVersion = 12;

        // This for now, until nbt can be cleaned up
        defaultNBT();
    }


    MU void ChunkData::placeBlock(
                       c_int xIn, c_int yIn, c_int zIn,
                       c_u16 block, c_u16 data, c_bool waterlogged, c_bool isSubmerged) {
        switch (lastVersion) {
            case 10: {
                int offset = (yIn % 128) + xIn * 128 + zIn * 128 * 16;
                offset += 32768 * (yIn > 127);
                oldBlocks[offset] = block;
                if (offset % 2 == 0) {
                    blockData[offset] = (blockData[offset] & 0x0F) | data << 4;
                } else {
                    blockData[offset] = (blockData[offset] & 0xF0) | data;
                }
                break;
            }
            case 8:
            case 9:
            case 11: {
                c_int offset = yIn * 256 + zIn * 16 + xIn;
                oldBlocks[offset] = block;
                if (offset % 2 == 0) {
                    blockData[offset] = (blockData[offset] & 0x0F) | data << 4;
                } else {
                    blockData[offset] = (blockData[offset] & 0xF0) | data;
                }
            }
            break;
            case 12:
            case 13: {
                c_int offset = yIn + 256 * zIn + 4096 * xIn;
                u16 value = block << 4 | data;
                if (waterlogged) {
                    value |= 0x8000;
                }
                if (!isSubmerged) {
                    newBlocks[offset] = value;
                } else {
                    submerged[offset] = value;
                }
                break;
            }
            default:
                break;
        }
    }


    MU void ChunkData::placeBlock(c_int xIn, c_int yIn, c_int zIn, c_u16 block, c_bool isSubmerged) {
        c_bool waterloggedIn = block & 0x8000;
        c_bool dataIn = block & 0x0F;
        c_bool blockIn = block & 0x7FF0 >> 4;
        placeBlock(xIn, yIn, zIn, blockIn, dataIn, waterloggedIn, isSubmerged);
    }


    /// Returns (blockID << 4 | dataTag).
    u16 ChunkData::getBlock(c_int xIn, c_int yIn, c_int zIn) {
        switch (lastVersion) {
            case 10: {
                int offset = (yIn % 128) + xIn * 128 + zIn * 128 * 16;
                offset += 32768 * (yIn > 127);
                c_u16 blockID = oldBlocks[offset];
                u16 dataTag;
                if (offset % 2 == 0) {
                    dataTag = blockData[offset] & 0x0F;
                } else {
                    dataTag = (blockData[offset] & 0xF0) >> 4;
                }
                return blockID << 4 | dataTag;
            }
            case 8:
            case 9:
            case 11: {
                c_int offset = yIn * 256 + zIn * 16 + xIn;
                c_u16 blockID = oldBlocks[offset];
                u16 dataTag;
                if (offset % 2 == 0) {
                    dataTag = blockData[offset] & 0x0F;
                } else {
                    dataTag = (blockData[offset] & 0xF0) >> 4;
                }
                return blockID << 4 | dataTag;
            }
            case 12:
            case 13: {
                c_int offset = yIn + 256 * zIn + 4096 * xIn;
                return newBlocks[offset];
            }
            default:
                return 0;
        }
    }


}
