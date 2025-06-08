#pragma once

#include "include/lce/processor.hpp"
#include "common/nbt.hpp"

namespace editor::chunk {


    enum eChunkVersion : i16 {
        V_NBT = 0x0007,
        V_8   = 0x0008,
        V_9   = 0x0009,
        V_10  = 0x000A,
        V_11  = 0x000B,
        V_12  = 0x000C,
        V_13  = 0x000D,
    };


    enum eBlockOrder {
        XZY,
        YXZ,
        YZX,
        yXZ,
        yXZy,
        Yqq
    };

    MU static std::string toString(MU eBlockOrder order) {
        switch (order) {
            case eBlockOrder::XZY: return "XZY";
            case eBlockOrder::YXZ: return "YXZ";
            case eBlockOrder::YZX: return "YZX";
            case eBlockOrder::yXZ: return "yXZ";
            case eBlockOrder::yXZy: return "yXZy";
            case eBlockOrder::Yqq: return "???";
        }
        return "oops";
    }

    template<eBlockOrder ORDER>
    static i32 toIndex(i32 x, i32 y, i32 z) {
        switch (ORDER) {
            case eBlockOrder::XZY: return x      + y*256 + z*  16;
            case eBlockOrder::YXZ: return x * 256 + y     + z*4096;
            case eBlockOrder::YZX: return x *4096 + y     + z* 256;
            case eBlockOrder::yXZ: return x * 128 + y     + z*2048;
            case eBlockOrder::yXZy: return x * 128 + (y % 128) + 32768 * (y > 127) + z * 128 * 16;
        }
    }

    static i32 toIndex(eBlockOrder order, i32 x, i32 y, i32 z) {
        switch (order) {
            case eBlockOrder::XZY: return toIndex<XZY>(x, y, z);
            case eBlockOrder::YXZ: return toIndex<YXZ>(x, y, z);
            case eBlockOrder::YZX: return toIndex<YZX>(x, y, z);
            case eBlockOrder::yXZ: return toIndex<yXZ>(x, y, z);
            case eBlockOrder::yXZy: return toIndex<yXZy>(x, y, z);
            default: return 0;
        }
    }

    static constexpr eBlockOrder CANONICAL_BLOCK_ORDER = XZY;
    static constexpr eBlockOrder CANONICAL_LIGHT_ORDER = XZY;

    class ChunkData {
    public:
        struct Intel {
            bool wasNBTChunk = false;
            bool hasBiomes = true;
            bool hasTerraFlagVariant = false;
            bool hasSubmerged = false;
        };


        // new version
        u16_vec blocks;
        u16_vec submerged;


        // all versions
        u8_vec blockLight;          //
        u8_vec skyLight;            //
        u8_vec heightMap;           //
        u8_vec biomes;              //
        i16 terrainPopulatedFlags = 0;   //
        i64 lastUpdate = 0;         //
        i64 inhabitedTime = 0;      //
        NBTList entities = NBTList(eNBT::COMPOUND);
        NBTList tileEntities = NBTList(eNBT::COMPOUND);
        NBTList tileTicks = NBTList(eNBT::COMPOUND);

        /// Used to skip the lights in the chunk

        i32 chunkX = 0;
        i32 chunkZ = 0;
        i32 chunkHeight = 256;

        u64 DataGroupCount = 0;
        u16 maxGridCount = 0;

        Intel intel;
        i32 lastVersion = 0;
        bool validChunk = false;

        MU void setSubmerged(i32 xIn, i32 yIn, i32 zIn, u16 block);
        MU u16 getSubmerged(i32 xIn, i32 yIn, i32 zIn) const;

        MU void setBlock(i32 xIn, i32 yIn, i32 zIn, u16 block);
        MU u16 getBlock(i32 xIn, i32 yIn, i32 zIn) const;

        MU void setBlockLight(i32 xIn, i32 yIn, i32 zIn, u8 light);
        MU u8 getBlockLight(i32 xIn, i32 yIn, i32 zIn) const;

        MU void setSkyLight(i32 xIn, i32 yIn, i32 zIn, u8 light);
        MU u8 setSkyLight(i32 xIn, i32 yIn, i32 zIn) const;

    };
}
