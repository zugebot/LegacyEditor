#include "chunkData.hpp"

#include "code/Chunk/helpers/helpers.hpp"
#include "common/nbt.hpp"
#include "include/lce/blocks/blockID.hpp"


namespace editor::chunk {


    MU void ChunkData::setSubmerged(c_i32 xIn, c_i32 yIn, c_i32 zIn, c_u16 block) {
        c_i32 offset = toIndex<CANONICAL_BLOCK_ORDER>(xIn, yIn, zIn);
        submerged[offset] = block;
    }


    MU u16 ChunkData::getSubmerged(i32 xIn, i32 yIn, i32 zIn) const {
        c_i32 offset = toIndex<CANONICAL_BLOCK_ORDER>(xIn, yIn, zIn);
        return submerged[offset];
    }


    MU void ChunkData::setBlock(c_i32 xIn, c_i32 yIn, c_i32 zIn, c_u16 block) {
        c_i32 offset = toIndex<CANONICAL_BLOCK_ORDER>(xIn, yIn, zIn);
        blocks[offset] = block;
    }

    /// Returns (blockID << 4 | dataTag).
    MU u16 ChunkData::getBlock(c_i32 xIn, c_i32 yIn, c_i32 zIn) const {
        c_i32 offset = toIndex<CANONICAL_BLOCK_ORDER>(xIn, yIn, zIn);
        return blocks[offset];
    }


    MU void ChunkData::setBlockLight(i32 xIn, i32 yIn, i32 zIn, u8 light) {
        c_i32 offset = toIndex<CANONICAL_LIGHT_ORDER>(xIn, yIn, zIn);
        setNibble(blockLight, offset, light);
    }


    MU u8 ChunkData::getBlockLight(i32 xIn, i32 yIn, i32 zIn) const {
        c_i32 offset = toIndex<CANONICAL_LIGHT_ORDER>(xIn, yIn, zIn);
        return getNibble(blockLight, offset);
    }


    MU void ChunkData::setSkyLight(i32 xIn, i32 yIn, i32 zIn, u8 light) {
        c_i32 offset = toIndex<CANONICAL_LIGHT_ORDER>(xIn, yIn, zIn);
        setNibble(skyLight, offset, light);
    }


    MU u8 ChunkData::setSkyLight(i32 xIn, i32 yIn, i32 zIn) const {
        c_i32 offset = toIndex<CANONICAL_LIGHT_ORDER>(xIn, yIn, zIn);
        return getNibble(skyLight, offset);
    }

}