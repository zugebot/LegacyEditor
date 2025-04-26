#include "chunkData.hpp"

#include "common/nbt.hpp"
#include "include/lce/blocks/blockID.hpp"


namespace editor::chunk {


    ChunkData::~ChunkData() {

    }


    void ChunkData::defaultNBT() {
        // oldNBTData.get<NBTCompound>().clear();
        // oldNBTData = makeCompound({
        //         {"Entities", makeList(eNBT::COMPOUND, {})},
        //         {"TileEntities", makeList(eNBT::COMPOUND, {})},
        //         {"TileTicks", makeList(eNBT::COMPOUND, {})}
        // });

        entities = makeList(eNBT::COMPOUND, {});
        tileEntities = makeList(eNBT::COMPOUND, {});
        tileTicks = makeList(eNBT::COMPOUND, {});
    }


    MU ND std::string ChunkData::getCoords() const {
        return "(" + std::to_string(chunkX) + ", " + std::to_string(chunkZ) + ")";
    }


    enum eBlockOrder {
        // XYZ,
        XZY,
        YXZ,
        YZX,
        // ZXY,
        // ZYX,
        // XyZ,
        // XZy,
        yXZ,
        // yZX,
        // ZXy,
        // ZyX,
        yXZy
    };

    template<eBlockOrder ORDER>
    int toIndex(int x, int y, int z) {
        switch (ORDER) {
            // case eBlockOrder::XYZ: return x      + y* 16 + z*4096;
            case eBlockOrder::XZY: return x      + y*256 + z*  16;
            case eBlockOrder::YXZ: return x* 256 + y     + z*4096;
            case eBlockOrder::YZX: return x*4096 + y     +z * 256;
            // case eBlockOrder::ZXY: return x*  16 + y*256 + z;
            // case eBlockOrder::ZYX: return x*4096 + y*16  + z;
            // case eBlockOrder::XyZ: return x      + y* 16 + z*2048;
            // case eBlockOrder::XZy: return x      + y*256 + z*  16;
            case eBlockOrder::yXZ: return x* 128 + y     + z*2048;
            // case eBlockOrder::yZX: return x*2048 + y     + z* 128;
            // case eBlockOrder::ZXy: return x*  16 + y*256 + z;
            // case eBlockOrder::ZyX: return x*2048 + y*16  + z;
            case eBlockOrder::yXZy: return x * 128 + (y % 128) + 32768 * (y > 127) + z * 128 * 16;
        }
    }



    MU void ChunkData::convertNBT128ToAquatic() {
        newBlocks = u16_vec(65536);
        for (int xIter = 0; xIter < 16; xIter++) {
            for (int zIter = 0; zIter < 16; zIter++) {
                for (int yIter = 0; yIter < 128; yIter++) {
                    c_int offset = toIndex<yXZ>(xIter, yIter, zIter);
                    c_u16 block = oldBlocks[offset] << 4 |
                                  ((blockData[offset >> 1] >> ((offset & 1) * 4)) & 0x0F);
                    c_int AquaticOffset = toIndex<YXZ>(xIter, yIter, zIter);
                    newBlocks[AquaticOffset] = block;
                }
            }
        }
        u8_vec().swap(oldBlocks);
        lastVersion = 12;
    }


    MU void ChunkData::convertNBT256ToAquatic() {
        newBlocks = u16_vec(65536);
        for (int xIter = 0; xIter < 16; xIter++) {
            for (int zIter = 0; zIter < 16; zIter++) {
                for (int yIter = 0; yIter < 256; yIter++) {
                    c_int offset = toIndex<yXZy>(xIter, yIter, zIter);
                    c_u16 block = oldBlocks[offset] << 4 |
                                  ((blockData[offset >> 1] >> ((offset & 1) * 4)) & 0x0F);
                    c_int AquaticOffset = toIndex<YXZ>(xIter, yIter, zIter);
                    newBlocks[AquaticOffset] = block;
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
                    c_int oldOffset = toIndex<XZY>(xIter, yIter, zIter);
                    c_u16 elytraBlock = oldBlocks[oldOffset] << 4 |
                                      ((blockData[oldOffset >> 1] >> ((oldOffset & 1) * 4)) & 0x0F);
                    c_int AquaticOffset = toIndex<YXZ>(xIter, yIter, zIter);
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
                c_int offset = toIndex<yXZy>(xIn, yIn, zIn);
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

                c_int offset = toIndex<XZY>(xIn, yIn, zIn);
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
                c_int offset = toIndex<YZX>(xIn, yIn, zIn);
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
                c_int offset = toIndex<yXZy>(xIn, yIn, zIn);
                return oldBlocks[offset] << 4 |
                       ((blockData[offset >> 1] >> ((offset & 1) * 4)) & 0x0F);
            }
            case 8:
            case 9:
            case 11: {
                c_int offset = toIndex<XZY>(xIn, yIn, zIn);
                return oldBlocks[offset] << 4 |
                       ((blockData[offset >> 1] >> ((offset & 1) * 4)) & 0x0F);
            }
            case 12:
            case 13: {
                c_int offset = toIndex<YZX>(xIn, yIn, zIn);
                return newBlocks[offset];
            }
            default:
                return 0;
        }
    }


}
