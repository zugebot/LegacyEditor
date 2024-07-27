#pragma once

#include "lce/processor.hpp"

#include "LegacyEditor/code/Chunk/chunkData.hpp"
#include "LegacyEditor/utils/NBT.hpp"


namespace editor::chunk {


    /// Order: YZX
    static int toPos(c_int xIn, c_int yIn, c_int zIn) {
        return yIn + 256 * zIn + 4096 * xIn;
    }


    MU inline void convertNBTToAquatic(ChunkData* chunkData) {
        chunkData->newBlocks = u16_vec(65536);
        for (int xIter = 0; xIter < 16; xIter++) {
        for (int zIter = 0; zIter < 16; zIter++) {
        for (int yIter = 0; yIter < 256; yIter++) {
            c_int offset = (yIter % 128) + xIter * 128 + zIter * 128 * 16 + 32768 * (yIter > 127);
            c_u16 blockID = chunkData->oldBlocks[offset];
            u16 dataTag;
            if (offset % 2 == 0) {
                dataTag = chunkData->blockData[offset / 2] & 0x0F;
            } else {
                dataTag = (chunkData->blockData[offset / 2] & 0xF0) >> 4;
            }
            u16 elytraBlock = blockID << 4 | dataTag;
            c_int AquaticOffset = yIter + 4096 * zIter + 256 * xIter;
            chunkData->newBlocks[AquaticOffset] = elytraBlock;
        }
        }
        }
        chunkData->lastVersion = 12;
        u8_vec().swap(chunkData->oldBlocks);
    }




    MU inline void convertOldToAquatic(ChunkData* chunkData) {
        chunkData->newBlocks = u16_vec(65536);
        for (int xIter = 0; xIter < 16; xIter++) {
        for (int zIter = 0; zIter < 16; zIter++) {
        for (int yIter = 0; yIter < 256; yIter++) {
            c_int offset = yIter * 256 + zIter * 16 + xIter;
            c_u16 blockID = chunkData->oldBlocks[offset];
            u16 dataTag;
            if (offset % 2 == 0) {
                dataTag = chunkData->blockData[offset / 2] & 0x0F;
            } else {
                dataTag = (chunkData->blockData[offset] & 0xF0) >> 8;
            }
            u16 elytraBlock = blockID << 4 | dataTag;
            c_int AquaticOffset = yIter + 4096 * zIter + 256 * xIter;
            chunkData->newBlocks[AquaticOffset] = elytraBlock;
        }
        }
        }
        chunkData->lastVersion = 12;
        u8_vec().swap(chunkData->oldBlocks);
    }


    MU inline void placeBlock(ChunkData* chunkData,
        c_int xIn, c_int yIn, c_int zIn,
        c_u16 block, c_u16 data, c_bool waterlogged, c_bool submerged = false) {
        switch (chunkData->lastVersion) {
            case 10: {
                int offset = (yIn % 128) + xIn * 128 + zIn * 128 * 16;
                offset += 32768 * (yIn > 127);
                chunkData->oldBlocks[offset] = block;
                if (offset % 2 == 0) {
                    chunkData->blockData[offset] = (chunkData->blockData[offset] & 0x0F) | data << 4;
                } else {
                    chunkData->blockData[offset] = (chunkData->blockData[offset] & 0xF0) | data;
                }
                break;
            }
            case 8:
            case 9:
            case 11: {
                c_int offset = yIn * 256 + zIn * 16 + xIn;
                chunkData->oldBlocks[offset] = block;
                if (offset % 2 == 0) {
                    chunkData->blockData[offset] = (chunkData->blockData[offset] & 0x0F) | data << 4;
                } else {
                    chunkData->blockData[offset] = (chunkData->blockData[offset] & 0xF0) | data;
                }
            }
                break;
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
                break;
            }
            default:
                break;
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
            case 10: {
                int offset = (yIn % 128) + xIn * 128 + zIn * 128 * 16;
                offset += 32768 * (yIn > 127);
                c_u16 blockID = chunkData->oldBlocks[offset];
                u16 dataTag;
                if (offset % 2 == 0) {
                    dataTag = chunkData->blockData[offset] & 0x0F;
                } else {
                    dataTag = (chunkData->blockData[offset] & 0xF0) >> 4;
                }
                return blockID << 4 | dataTag;
            }
            case 8:
            case 9:
            case 11: {
                c_int offset = yIn * 256 + zIn * 16 + xIn;
                c_u16 blockID = chunkData->oldBlocks[offset];
                u16 dataTag;
                if (offset % 2 == 0) {
                    dataTag = chunkData->blockData[offset] & 0x0F;
                } else {
                    dataTag = (chunkData->blockData[offset] & 0xF0) >> 4;
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







