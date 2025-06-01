#pragma once

#include "include/lce/processor.hpp"

#include "common/error_status.hpp"

#include "common/nbt.hpp"

namespace editor::chunk {


    enum eChunkVersion : i16 {
        V_8   = 0x0008,
        V_9   = 0x0009,
        V_NBT = 0x000A,
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
            case eBlockOrder::YXZ: return x* 256 + y     + z*4096;
            case eBlockOrder::YZX: return x*4096 + y     + z* 256;
            case eBlockOrder::yXZ: return x * 128 + y     + z*2048;
            case eBlockOrder::yXZy: return x * 128 + (y % 128) + 32768 * (y > 127) + z * 128 * 16;
        }
    }

    static i32 toIndex(eBlockOrder order, i32 x, i32 y, i32 z) {
        switch (order) {
            case eBlockOrder::XZY: return x      + y*256 + z*  16;
            case eBlockOrder::YXZ: return x* 256 + y     + z*4096;
            case eBlockOrder::YZX: return x*4096 + y     +z * 256;
            case eBlockOrder::yXZ: return x* 128 + y     + z*2048;
            case eBlockOrder::yXZy: return x * 128 + (y % 128) + 32768 * (y > 127) + z * 128 * 16;
            default:
                return 0;
        }
    }


    class ChunkData {
    public:
        // old version
        u8_vec oldBlocks;
        u8_vec blockData;

        // new version
        u16_vec newBlocks;
        u16_vec submerged;
        bool hasSubmerged = false;

        // all versions
        u8_vec blockLight;          //
        u8_vec skyLight;            //
        u8_vec heightMap;           //
        u8_vec biomes;              //
        i16 terrainPopulated = 0;   //
        i64 lastUpdate = 0;         //
        i64 inhabitedTime = 0;      //
        NBTBase entities;
        NBTBase tileEntities;
        NBTBase tileTicks;

        /// Used to skip the lights in the chunk
        size_t DataGroupCount = 0;
        u16 maxGridCount = 0;

        i32 chunkX = 0;
        i32 chunkZ = 0;
        i32 chunkHeight = 256;

        i32 lastVersion = 0;
        bool validChunk = false;

        MU ND std::string getCoords() const;


        // MODIFIERS

        MU void convertNBT128ToAquatic();
        MU void convertNBT256ToAquatic();
        MU void convertOldToAquatic();
        MU void convert114ToAquatic();
        MU void convertAquaticToElytra();


        /// places a block
        template<eChunkVersion chunkVersion>
        MU void setSubmerged(i32 xIn, i32 yIn, i32 zIn, u16 block);
        MU void setSubmerged(i32 xIn, i32 yIn, i32 zIn, u16 block);

        /// places a block
        template<eChunkVersion chunkVersion>
        MU void setBlock(i32 xIn, i32 yIn, i32 zIn, u16 block);
        MU void setBlock(i32 xIn, i32 yIn, i32 zIn, u16 block);

        /// Returns (blockID << 4 | dataTag)
        template<eChunkVersion chunkVersion>
        u16 getBlock(i32 xIn, i32 yIn, i32 zIn);
        u16 getBlock(i32 xIn, i32 yIn, i32 zIn);


        /*
        /// sets blocklight
        template<eChunkVersion chunkVersion>
        MU void setBlockLight(i32 xIn, i32 yIn, i32 zIn, u8 light);
        MU void setBlockLight(i32 xIn, i32 yIn, i32 zIn, u8 light);

        /// Returns blocklight
        template<eChunkVersion chunkVersion>
        u8 getBlockLight(i32 xIn, i32 yIn, i32 zIn);
        u8 getBlockLight(i32 xIn, i32 yIn, i32 zIn);

        /// sets skylight
        template<eChunkVersion chunkVersion>
        MU void setSkyLight(i32 xIn, i32 yIn, i32 zIn, u8 light);
        MU void setSkyLight(i32 xIn, i32 yIn, i32 zIn, u8 light);

        /// Returns skylight
        template<eChunkVersion chunkVersion>
        u8 setSkyLight(i32 xIn, i32 yIn, i32 zIn);
        u8 setSkyLight(i32 xIn, i32 yIn, i32 zIn);
         */


    };
}
