#pragma once

#include "chunkData.hpp"

#include "LegacyEditor/utils/NBT.hpp"
#include "lce/processor.hpp"


namespace editor::chunk {


    /// Order: YZX
    static int toPos(c_int xIn, c_int yIn, c_int zIn) {
        return yIn + 256 * zIn + 4096 * xIn;
    }


    MU inline void convertOldToNew(ChunkData* chunkData) {
        chunkData->newBlocks = u16_vec(65536);

        int count = 0;
        for (int offset = 0; offset < 256; offset += 128) {
            for (int xIter = 0; xIter < 16; xIter++) {
                for (int zIter = 0; zIter < 16; zIter++) {
                    for (int yIter = 0; yIter < 128; yIter++) {
                        c_int index = toPos(xIter, yIter + offset, zIter);
                        c_int newIndex = index / 2;
                        u16 data = chunkData->blockData[newIndex];
                        if (count % 2 == 0) {
                            data >>= 4;
                        } else {
                            data &= 15;
                        }

                        c_u16 oldBlock = chunkData->oldBlocks[count];
                        chunkData->newBlocks[index] = (oldBlock << 4) | data;
                        count++;
                    }
                }
            }
        }

        if (chunkData->NBTData != nullptr) {
            chunkData->NBTData->toType<NBTTagCompound>()->deleteAll();
            delete chunkData->NBTData;
            chunkData->NBTData = nullptr;
        }

        /*
        u16 data;
        for (int i = 0; i < 65536; i++) {
            data = chunkData.blockData[i / 2] >> 4 * ((i + 1) % 2);
            chunkData.newBlocks[i] = chunkData.oldBlocks[i] << 4 | data;
        }
         */

        chunkData->lastVersion = 12;
        u8_vec().swap(chunkData->oldBlocks);
    }


    MU inline void placeBlock(ChunkData* chunkData,
        c_int xIn, c_int yIn, c_int zIn,
        c_u16 block, c_u16 data, c_bool waterlogged, c_bool submerged = false) {
        switch (chunkData->lastVersion) {
            case 8:
            case 9:
            case 11: {
                c_int yOffset = yIn & 0x80 << 8;
                c_int offset = yOffset + toPos(xIn, yIn & 0x7F, zIn);
                chunkData->oldBlocks[offset] = block;
                if (offset % 2 == 0) {
                    chunkData->blockData[offset] = chunkData->blockData[offset] & 0x0F | data << 8;
                } else {
                    chunkData->blockData[offset] = chunkData->blockData[offset] & 0xF0 | data;
                }
            }
            case 12:
            case 13: {
                c_int offset = toPos(xIn, yIn, zIn);
                u16 value = block << 4 | data;
                if (waterlogged) {
                    value |= 0x8000;
                }
                if (!submerged) {
                    chunkData->newBlocks[offset] = value;
                } else {
                    chunkData->submerged[offset] = value;
                }

            }
            default:;
        }
    }

    MU inline void placeBlock(ChunkData* chunkData,
        c_int xIn, c_int yIn, c_int zIn,
        c_u16 block, c_bool submerged = false) {
        c_bool waterloggedIn = block & 0x8000;
        c_bool dataIn = block & 0x0F;
        c_bool blockIn = block & 0x7FF0 >> 4;
        placeBlock(chunkData, xIn, yIn, zIn, blockIn, dataIn, waterloggedIn, submerged);
    }


    /// Returns (blockID << 4 | dataTag).
    inline u16 getBlock(const ChunkData* chunkData,
        c_int xIn, c_int yIn, c_int zIn) {
        switch (chunkData->lastVersion) {
            case 8:
            case 9:
            case 11: {
                c_int yOffset = yIn & 0x80 << 8;
                c_int offset = yOffset + toPos(xIn, yIn & 0x7F, zIn);
                c_u16 blockID = chunkData->newBlocks[offset];
                u16 dataTag;
                if (offset % 2 == 0) {
                    dataTag = chunkData->blockData[offset] & 0x0F;
                } else {
                    dataTag = (chunkData->blockData[offset] & 0xF0) >> 8;
                }
                return blockID << 4 | dataTag;
            }
            case 12:
            case 13: {
                c_int offset = toPos(xIn, yIn, zIn);
                return chunkData->newBlocks[offset];
            }
            default:
                return 0;
        }
    }


}







