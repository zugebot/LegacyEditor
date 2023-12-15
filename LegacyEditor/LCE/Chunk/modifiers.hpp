#pragma once

#include "chunkData.hpp"


namespace editor::chunk {

    static inline int toPos(int x, int y, int z) {
        return y + 256 * z + 4096 * x;
    }


    MU void convertOldToNew(ChunkData* chunkData) {
        chunkData->newBlocks = u16_vec(65536);

        int count = 0;
        for (int offset = 0; offset < 256; offset += 128) {
            for (int x = 0; x < 16; x++) {
                for (int z = 0; z < 16; z++) {
                    for (int y = 0; y < 128; y++) {
                        int index = toPos(x, y + offset, z);

                        u16 data = 0; // chunkData.blockData[count / 2];
                        if (count % 2 == 0) {
                            data >>= 4;
                        } else {
                            data &= 15;
                        }

                        u16 oldBlock = chunkData->oldBlocks[count];
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

    MU void placeBlock(ChunkData* chunkData, int x, int y, int z, u16 block, u16 data, bool waterlogged) {
        int offset = toPos(x, y, z);
        u16 value = block << 4 | data;
        if (waterlogged) {
            value |= 0x8000;
        }
        chunkData->newBlocks[offset] = value;
    }

    u16 getBlock(ChunkData* chunkData, int x, int y, int z) {
        int offset = toPos(x, y, z);
        return chunkData->newBlocks[offset];
    }

}







