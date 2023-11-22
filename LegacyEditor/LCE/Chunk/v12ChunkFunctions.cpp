#include "v12Chunk.hpp"


namespace universal {


    void V12Chunk::placeBlock(int x, int y, int z, u16 block, u16 data, bool waterlogged) {
        int offset = y + 256 * z + 4096 * x;
        u16 value = block << 4 | data;
        if (waterlogged) {
            value |= 0b1000000000000000;
        }
        chunkData.blocks[offset] = value;
    }


    u16 V12Chunk::getBlock(int x, int y, int z) {
        int offset = y + 256 * z + 4096 * x;
        return chunkData.blocks[offset];
    }


}



