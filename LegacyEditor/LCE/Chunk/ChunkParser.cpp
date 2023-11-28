#include "ChunkParser.hpp"

#include "v11Chunk.hpp"
#include "v12Chunk.hpp"
#include "LegacyEditor/utils/DataManager.hpp"



static inline int toPos(int x, int y, int z) {
    return y + 256 * z + 4096 * x;
}


namespace universal {


    MU void ChunkParser::readChunk(DataManager* managerIn, DIM dim) {
        managerIn->seekStart();
        chunkData.lastVersion = managerIn->readInt16();
        switch(chunkData.lastVersion) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
                v11Chunk = new V11Chunk();
                if (v11Chunk == nullptr) return;
                v11Chunk->readChunk(&chunkData, managerIn, dim);
                delete v11Chunk;
                break;
            case 12:
                v12Chunk = new V12Chunk();
                if (v12Chunk == nullptr) return;
                v12Chunk->readChunk(&chunkData, managerIn, dim);
                delete v12Chunk;
                break;
        }
    }


    MU void ChunkParser::writeChunk(DataManager* managerOut, DIM dim) {
        managerOut->seekStart();
        managerOut->writeInt16(chunkData.lastVersion);
        switch(chunkData.lastVersion) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
                v11Chunk = new V11Chunk();
                if (v11Chunk == nullptr) return;
                v11Chunk->writeChunk(&chunkData, managerOut, dim);
                delete v11Chunk;
                break;
            case 12:
                v12Chunk = new V12Chunk();
                if (v12Chunk == nullptr) return;
                v12Chunk->writeChunk(&chunkData, managerOut, dim);
                delete v12Chunk;
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
        chunkData.blocks[offset] = value;
    }


    u16 ChunkParser::getBlock(int x, int y, int z) {
        int offset = toPos(x, y, z);
        return chunkData.blocks[offset];
    }


    MU void ChunkParser::rotateUpsideDown() {
        u16 blocks[65536];
        for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
                for (int y = 0; y < 255; y++) {
                    blocks[toPos(15 - x, 255 - y, 15 - z)] = chunkData.blocks[toPos(x, y, z)];
                }
            }
        }
        memcpy(&chunkData.blocks[0], &blocks[0], 65536);
    }



}