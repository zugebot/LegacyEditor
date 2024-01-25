#pragma once

#include "chunkData.hpp"


namespace editor::chunk {





    /// Order: YZX
    static int toPos(const int xIn, const int yIn, const int zIn) {
        return yIn + 256 * zIn + 4096 * xIn;
    }


    MU inline void convertOldToNew(ChunkData* chunkData) {
        chunkData->newBlocks = u16_vec(65536);

        int count = 0;
        for (int offset = 0; offset < 256; offset += 128) {
            for (int xIter = 0; xIter < 16; xIter++) {
                for (int zIter = 0; zIter < 16; zIter++) {
                    for (int yIter = 0; yIter < 128; yIter++) {
                        const int index = toPos(xIter, yIter + offset, zIter);
                        const int newIndex = index / 2;
                        u16 data = chunkData->blockData[newIndex];
                        if (count % 2 == 0) {
                            data >>= 4;
                        } else {
                            data &= 15;
                        }

                        const u16 oldBlock = chunkData->oldBlocks[count];
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
        const int xIn, const int yIn, const int zIn,
        const u16 block, const u16 data, const bool waterlogged) {
        switch (chunkData->lastVersion) {
            case 8:
            case 9:
            case 11: {
                const int yOffset = yIn & 0x80 << 8;
                const int offset = yOffset + toPos(xIn, yIn & 0x7F, zIn);
                chunkData->oldBlocks[offset] = block;
                if (offset % 2 == 0) {
                    chunkData->blockData[offset] = chunkData->blockData[offset] & 0x0F | data << 8;
                } else {
                    chunkData->blockData[offset] = chunkData->blockData[offset] & 0xF0 | data;
                }
            }
            case 12: {
                const int offset = toPos(xIn, yIn, zIn);
                u16 value = block << 4 | data;
                if (waterlogged) {
                    value |= 0x8000;
                }
                chunkData->newBlocks[offset] = value;
            }
            default:;
        }
    }

    MU inline void placeBlock(ChunkData* chunkData,
        const int xIn, const int yIn, const int zIn,
        const u16 block) {
        const bool waterloggedIn = block & 0x8000;
        const bool dataIn = block & 0x0F;
        const bool blockIn = block & 0x7FF0 >> 4;
        placeBlock(chunkData, xIn, yIn, zIn, blockIn, dataIn, waterloggedIn);
    }


    inline u16 getBlock(const ChunkData* chunkData,
        const int xIn, const int yIn, const int zIn) {
        switch (chunkData->lastVersion) {
            case 8:
            case 9:
            case 11: {
                const int yOffset = yIn & 0x80 << 8;
                const int offset = yOffset + toPos(xIn, yIn & 0x7F, zIn);
                const u16 block = chunkData->newBlocks[offset];
                u16 data;
                if (offset % 2 == 0) {
                    data = chunkData->blockData[offset] & 0x0F;
                } else {
                    data = (chunkData->blockData[offset] & 0xF0) >> 8;
                }
                return block << 4 | data;
            }
            case 12: {
                const int offset = toPos(xIn, yIn, zIn);
                return chunkData->newBlocks[offset];
            }
            default:
                return 0;
        }
    }


}







