#include "chunkData.hpp"

#include "common/nbt.hpp"
#include "helpers.hpp"
#include "include/lce/blocks/blockID.hpp"


namespace editor::chunk {


    ChunkData::~ChunkData() = default;


    void ChunkData::defaultNBT() {
        entities = makeList(eNBT::COMPOUND, {});
        tileEntities = makeList(eNBT::COMPOUND, {});
        tileTicks = makeList(eNBT::COMPOUND, {});
    }


    MU ND std::string ChunkData::getCoords() const {
        return "(" + std::to_string(chunkX) + ", " + std::to_string(chunkZ) + ")";
    }






    MU void ChunkData::convertNBT128ToAquatic() {

        // BLOCKS
        newBlocks = u16_vec(65536);
        for (int xIter = 0; xIter < 16; xIter++) {
            for (int zIter = 0; zIter < 16; zIter++) {
                for (int yIter = 0; yIter < 128; yIter++) {
                    c_u16 block = getBlock<eChunkVersion::V_UNVERSIONED>(xIter, yIter, zIter);
                    setBlock<eChunkVersion::V_12>(xIter, yIter, zIter, block);
                }
            }
        }
        u8_vec().swap(oldBlocks);

        // BLOCK LIGHT
        u8_vec tempLight(32'768);
        for (int xIter = 0; xIter < 16; ++xIter) {
            for (int zIter = 0; zIter < 16; ++zIter) {
                for (int yIter = 0; yIter < 128; ++yIter) {
                    c_i32 oldOffset = toIndex<yXZ>(xIter, yIter, zIter);
                    c_u8 nib = getNibble(blockLight, oldOffset);
                    c_i32 newOffset = toIndex<XZY>(xIter, yIter, zIter);
                    setNibble(tempLight, newOffset, nib);
                }
            }
        }
        blockLight.swap(tempLight);

        // SKYLIGHT
        for (int xIter = 0; xIter < 16; ++xIter) {
            for (int zIter = 0; zIter < 16; ++zIter) {
                for (int yIter = 0; yIter < 128; ++yIter) {
                    c_i32 oldOffset = toIndex<yXZ>(xIter, yIter, zIter);
                    c_u8 nib = getNibble(skyLight, oldOffset);
                    c_i32 newOffset = toIndex<XZY>(xIter, yIter, zIter);
                    setNibble(tempLight, newOffset, nib);
                }
            }
        }
        skyLight.swap(tempLight);
        memset(skyLight.data() + 16384, 0xFF, 16384);

        lastVersion = 12;
    }


    MU void ChunkData::convertNBT256ToAquatic() {
        newBlocks = u16_vec(65536);
        for (int xIter = 0; xIter < 16; xIter++) {
            for (int zIter = 0; zIter < 16; zIter++) {
                for (int yIter = 0; yIter < 256; yIter++) {
                    c_u16 block = getBlock<eChunkVersion::V_NBT>(xIter, yIter, zIter);
                    setBlock<eChunkVersion::V_12>(xIter, yIter, zIter, block);
                }
            }
        }
        // std::cout << "made it past get and set\n";
        // std::cout << std::flush;

        u8_vec().swap(oldBlocks);

        // BLOCK LIGHT
        u8_vec tempLight(32'768);
        for (int xIter = 0; xIter < 16; ++xIter) {
            for (int zIter = 0; zIter < 16; ++zIter) {
                for (int yIter = 0; yIter < 256; ++yIter) {
                    c_i32 oldOffset = toIndex<yXZy>(xIter, yIter, zIter);
                    c_i32 newOffset = toIndex<XZY>(xIter, yIter, zIter);
                    c_u8 nib = getNibble(blockLight, oldOffset);
                    setNibble(tempLight, newOffset, nib);
                }
            }
        }
        blockLight.swap(tempLight);

        // SKYLIGHT
        for (int xIter = 0; xIter < 16; ++xIter) {
            for (int zIter = 0; zIter < 16; ++zIter) {
                for (int yIter = 0; yIter < 256; ++yIter) {
                    c_i32 oldOffset = toIndex<yXZy>(xIter, yIter, zIter);
                    c_i32 newOffset = toIndex<XZY>(xIter, yIter, zIter);
                    c_u8 nib = getNibble(skyLight, oldOffset);
                    setNibble(tempLight, newOffset, nib);
                }
            }
        }
        skyLight.swap(tempLight);
        u8_vec().swap(tempLight);

        lastVersion = 12;
    }


    MU void ChunkData::convertOldToAquatic() {
        newBlocks = u16_vec(65536);
        for (int xIter = 0; xIter < 16; xIter++) {
            for (int zIter = 0; zIter < 16; zIter++) {
                for (int yIter = 0; yIter < 256; yIter++) {

                    c_u16 block = getBlock<eChunkVersion::V_11>(xIter, yIter, zIter);
                    setBlock<eChunkVersion::V_12>(xIter, yIter, zIter, block);

                    // c_i32 newOffset = toIndex<yXZy>(xIter, yIter, zIter);
                    // newBlocks[newOffset] = block;
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






    template<eChunkVersion chunkVersion>
    MU void ChunkData::setSubmerged(c_i32 xIn, c_i32 yIn, c_i32 zIn, c_u16 block) {
        switch (chunkVersion) {
            case eChunkVersion::V_12:
            case eChunkVersion::V_13: {
                c_i32 offset = toIndex<yXZy>(xIn, yIn, zIn);
                submerged[offset] = block;
                break;
            }
            default:
                break;
        }
    }
    template void ChunkData::setSubmerged<eChunkVersion::V_12>(int,int,int,u16);
    template void ChunkData::setSubmerged<eChunkVersion::V_13>(int,int,int,u16);


    MU void ChunkData::setBlock(c_i32 xIn, c_i32 yIn, c_i32 zIn, c_u16 block) {
        switch (lastVersion) {
            case eChunkVersion::V_UNVERSIONED:
            case eChunkVersion::V_NBT: {
                c_i32 offset = toIndex<yXZy>(xIn, yIn, zIn);
                oldBlocks[offset] = block;
                setNibble(blockData, offset, block & 0x0F);
                break;
            }
            case eChunkVersion::V_8:
            case eChunkVersion::V_9:
            case eChunkVersion::V_11: {
                c_i32 offset = toIndex<XZY>(xIn, yIn, zIn);
                oldBlocks[offset] = block;
                setNibble(blockData, offset, block & 0x0F);
                break;
            }
            case eChunkVersion::V_12:
            case eChunkVersion::V_13: {
                c_i32 offset = toIndex<yXZy>(xIn, yIn, zIn);
                newBlocks[offset] = block;
                break;
            }
            default:
                break;
        }
    }


    template<eChunkVersion chunkVersion>
    MU void ChunkData::setBlock(c_i32 xIn, c_i32 yIn, c_i32 zIn, c_u16 block) {
        switch (chunkVersion) {
            case eChunkVersion::V_UNVERSIONED:
            case eChunkVersion::V_NBT: {
                c_i32 offset = toIndex<yXZy>(xIn, yIn, zIn);
                oldBlocks[offset] = block >> 4;
                setNibble(blockData, offset, block & 0x0F);
                break;
            }
            case eChunkVersion::V_8:
            case eChunkVersion::V_9:
            case eChunkVersion::V_11: {
                c_i32 offset = toIndex<XZY>(xIn, yIn, zIn);
                oldBlocks[offset] = block;
                setNibble(blockData, offset, block & 0x0F);
                break;
            }
            case eChunkVersion::V_12:
            case eChunkVersion::V_13: {
                c_i32 offset = toIndex<yXZy>(xIn, yIn, zIn);
                newBlocks[offset] = block;
                break;
            }
            default:
                break;
        }
    }
    template void ChunkData::setBlock<eChunkVersion::V_UNVERSIONED>(int,int,int,u16);
    template void ChunkData::setBlock<eChunkVersion::V_NBT>(int,int,int,u16);
    template void ChunkData::setBlock<eChunkVersion::V_8>(int,int,int,u16);
    template void ChunkData::setBlock<eChunkVersion::V_9>(int,int,int,u16);
    template void ChunkData::setBlock<eChunkVersion::V_11>(int,int,int,u16);
    template void ChunkData::setBlock<eChunkVersion::V_12>(int,int,int,u16);










    /// Returns (blockID << 4 | dataTag).
    u16 ChunkData::getBlock(c_i32 xIn, c_i32 yIn, c_i32 zIn) {
        switch (lastVersion) {
            case eChunkVersion::V_UNVERSIONED:
            case eChunkVersion::V_NBT: {
                c_i32 offset = toIndex<yXZy>(xIn, yIn, zIn);
                return oldBlocks[offset] << 4 | getNibble(blockData, offset);
            }
            case eChunkVersion::V_8:
            case eChunkVersion::V_9:
            case eChunkVersion::V_11: {
                c_i32 offset = toIndex<XZY>(xIn, yIn, zIn);
                return oldBlocks[offset] << 4 | getNibble(blockData, offset);
            }
            case eChunkVersion::V_12:
            case eChunkVersion::V_13: {
                c_i32 offset = toIndex<yXZy>(xIn, yIn, zIn);
                return newBlocks[offset];
            }
            default:
                return 0;
        }
    }


    template<eChunkVersion chunkVersion>
    u16 ChunkData::getBlock(c_i32 xIn, c_i32 yIn, c_i32 zIn) {
        switch (chunkVersion) {
            case eChunkVersion::V_UNVERSIONED:
            case eChunkVersion::V_NBT: {
                c_i32 offset = toIndex<yXZy>(xIn, yIn, zIn);
                return oldBlocks[offset] << 4 | getNibble(blockData, offset);
            }
            case eChunkVersion::V_8:
            case eChunkVersion::V_9:
            case eChunkVersion::V_11: {
                c_i32 offset = toIndex<XZY>(xIn, yIn, zIn);
                return oldBlocks[offset] << 4 | getNibble(blockData, offset);
            }
            case eChunkVersion::V_12:
            case eChunkVersion::V_13: {
                c_i32 offset = toIndex<yXZy>(xIn, yIn, zIn);
                return newBlocks[offset];
            }
            default:
                return 0;
        }
    }

    template u16 ChunkData::getBlock<eChunkVersion::V_UNVERSIONED>(int,int,int);
    template u16 ChunkData::getBlock<eChunkVersion::V_NBT>(int,int,int);
    template u16 ChunkData::getBlock<eChunkVersion::V_8>(int,int,int);
    template u16 ChunkData::getBlock<eChunkVersion::V_9>(int,int,int);
    template u16 ChunkData::getBlock<eChunkVersion::V_11>(int,int,int);
    template u16 ChunkData::getBlock<eChunkVersion::V_12>(int,int,int);
    template u16 ChunkData::getBlock<eChunkVersion::V_13>(int,int,int);








    /*
    template<eChunkVersion chunkVersion>
    MU void ChunkData::setBlockLight(i32 xIn, i32 yIn, i32 zIn, u8 light) {

    }
    MU void ChunkData::setBlockLight(i32 xIn, i32 yIn, i32 zIn, u8 light) {

    }

    /// Returns blocklight
    template<eChunkVersion chunkVersion>
    u8 ChunkData::getBlockLight(i32 xIn, i32 yIn, i32 zIn) {

    }
    u8 ChunkData::getBlockLight(i32 xIn, i32 yIn, i32 zIn) {

    }

    /// sets skylight
    template<eChunkVersion chunkVersion>
    MU void ChunkData::setSkyLight(i32 xIn, i32 yIn, i32 zIn, u8 light) {

    }
    MU void ChunkData::setSkyLight(i32 xIn, i32 yIn, i32 zIn, u8 light) {

    }

    /// Returns skylight
    template<eChunkVersion chunkVersion>
    u8 ChunkData::setSkyLight(i32 xIn, i32 yIn, i32 zIn) {

    }
    u8 ChunkData::setSkyLight(i32 xIn, i32 yIn, i32 zIn) {

    }*/





}
