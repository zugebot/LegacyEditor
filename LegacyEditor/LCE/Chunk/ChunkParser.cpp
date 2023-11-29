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
            case 0x0a00:
                chunkData.lastVersion = 0x000A;
                v10Chunk.readChunk(&chunkData, managerIn, dim);
                break;
            case 0x0008:
            case 0x0009:
            case 0x000B:
                v11Chunk.readChunk(&chunkData, managerIn, dim);
                break;
            case 0x000C:
                v12Chunk.readChunk(&chunkData, managerIn, dim);
                break;
        }
    }


    MU void ChunkParser::writeChunk(DataManager* managerOut, DIM dim) {
        managerOut->seekStart();
        switch(chunkData.lastVersion) {
            case 0x0a00:
                v10Chunk.writeChunk(&chunkData, managerOut, dim);
                break;
            case 0x0008:
            case 0x0009:
            case 0x000B:
                managerOut->writeInt16(chunkData.lastVersion);
                v11Chunk.writeChunk(&chunkData, managerOut, dim);
                break;
            case 0x000C:
                managerOut->writeInt16(chunkData.lastVersion);
                v12Chunk.writeChunk(&chunkData, managerOut, dim);
                break;
        }
    }


    MU void ChunkParser::fixHeightMap() {

    }


    MU void ChunkParser::placeBlock(int x, int y, int z, u16 block, u16 data, bool waterlogged) {
        int offset = toPos(x, y, z);
        u16 value = block << 4 | data;
        if (waterlogged) {
            value |= 0b1000000000000000;
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