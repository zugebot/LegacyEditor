#include "ChunkParser.hpp"
#include <iostream>

#include "LegacyEditor/utils/DataManager.hpp"
#include "LegacyEditor/utils/endian.hpp"


static inline int toPos(int x, int y, int z) {
    return y + 256 * z + 4096 * x;
}


namespace universal {


    MU void ChunkParser::readChunk(DataManager* managerIn, DIM dim) {
        managerIn->seekStart();
        chunkData.lastVersion = managerIn->readInt16();

        switch(chunkData.lastVersion) {
            case 0x0a00: {
                chunkData.lastVersion = 0x000A;
                V10Chunk v10Chunk;
                v10Chunk.readChunk(&chunkData, managerIn, dim);
                break;
            }
            case 0x0008:
            case 0x0009:
            case 0x000B: {
                V11Chunk v11Chunk;
                v11Chunk.readChunk(&chunkData, managerIn, dim);
                break;
            }
            case 0x000C: {
                V12Chunk v12Chunk;
                v12Chunk.readChunk(&chunkData, managerIn, dim);
                break;
            }
        }
    }

    MU void ChunkParser::writeChunk(DataManager* managerOut, DIM dim) {
        managerOut->seekStart();
        switch(chunkData.lastVersion) {
            case 0x0a00: {
                V10Chunk v10Chunk;
                v10Chunk.writeChunk(&chunkData, managerOut, dim);
                break;
            }
            case 0x0008:
            case 0x0009:
            case 0x000B: {
                managerOut->writeInt16(chunkData.lastVersion);
                V11Chunk v11Chunk;
                v11Chunk.writeChunk(&chunkData, managerOut, dim);
                break;
            }
            case 0x000C: {
                managerOut->writeInt16(chunkData.lastVersion);
                V12Chunk v12Chunk;
                v12Chunk.writeChunk(&chunkData, managerOut, dim);
                break;
            }
        }
    }



    MU void ChunkParser::convertOldToNew() {
        chunkData.newBlocks = u16_vec(65536);

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

                        u16 oldBlock = chunkData.oldBlocks[count];
                        chunkData.newBlocks[index] = (oldBlock << 4) | data;
                        count++;
                    }
                }
            }
        }

        if (chunkData.NBTData != nullptr) {
            chunkData.NBTData->toType<NBTTagCompound>()->deleteAll();
            delete chunkData.NBTData;
            chunkData.NBTData = nullptr;
        }

        /*
        u16 data;
        for (int i = 0; i < 65536; i++) {
            data = chunkData.blockData[i / 2] >> 4 * ((i + 1) % 2);
            chunkData.newBlocks[i] = chunkData.oldBlocks[i] << 4 | data;
        }
         */

        chunkData.lastVersion = 12;
        u8_vec().swap(chunkData.oldBlocks);
    }


    MU void ChunkParser::fixHeightMap() {

    }


    MU void ChunkParser::placeBlock(int x, int y, int z, u16 block, u16 data, bool waterlogged) {
        int offset = toPos(x, y, z);
        u16 value = block << 4 | data;
        if (waterlogged) {
            value |= 0x8000;
        }
        chunkData.newBlocks[offset] = value;
    }


    u16 ChunkParser::getBlock(int x, int y, int z) {
        int offset = toPos(x, y, z);
        return chunkData.newBlocks[offset];
    }


    MU void ChunkParser::rotateUpsideDown() {
        u16 blocks[65536];
        for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
                for (int y = 0; y < 255; y++) {
                    blocks[toPos(15 - x, 255 - y, 15 - z)] = chunkData.newBlocks[toPos(x, y, z)];
                }
            }
        }
        memcpy(&chunkData.newBlocks[0], &blocks[0], 65536);
    }



}