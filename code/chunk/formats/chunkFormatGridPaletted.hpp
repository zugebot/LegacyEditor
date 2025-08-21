#pragma once

#include "include/lce/processor.hpp"
#include "common/fixedVector.hpp"

#include "chunkFormatBase.hpp"
#include "common/data/DataReader.hpp"
#include "common/data/DataWriter.hpp"


namespace editor {

    class ChunkData;


    enum V11GridFormat : u16 {
        V11_0_BIT = 65535, //<  0 bytes palette,  0 bytes pos =  0 bytes
        V11_1_BIT = 0,     //<  2 bytes palette,  8 bytes pos = 10 bytes
        V11_2_BIT = 1,     //<  4 bytes palette, 16 bytes pos = 20 bytes
        V11_3_BIT = 2,     //< 16 bytes palette, 32 bytes pos = 48 bytes
        V11_4_BIT = 3,     //< 64 bytes raw blocks
    };


    static constexpr u32 V11_GRID_SIZES[4] = {
            10,
            20,
            48,
            64
    };


    /**
     * the data is stored in the m_order of
     * [ byte0 | byte1 ]
     *
     * <br>             [    byte0 |    byte1 ]
     * <br>     VAR:    [ ######## | ######## ]
     * <br>IF:
     * <br>     value : [ 00000111 | -------- ]
     * <br>THEN:
     * <br>     block : [ -------- | ######## ]
     * <br>ELSE:
     * <br>     format: [ ------## | -------- ]
     * <br>     offset: [ ######-- | -------# ] (swap bytes)
     */

    struct Grid {
        static constexpr u8 IS_SINGLE_BLOCK_FLAG = 0b00000111;

        u16 data{};

        Grid() = default;
        Grid(u8 byte0, u8 byte1) {
            data = static_cast<u16>(byte0) | (static_cast<u16>(byte1) << 8);
        }

        MU void setBytes(u8 byte0, u8 byte1) {
            data = static_cast<u16>(byte0) | (static_cast<u16>(byte1) << 8);
        }

        MU void setData(u16 val) { data = val; }

        ND bool isSingleBlock() const {
            return (data & 0b111) == IS_SINGLE_BLOCK_FLAG;
        }

        u8 getSingleBlock() const {
            return static_cast<u8>(data >> 8);
        }

        u32 getFormat() const {
            return data & 0b11;
        }

        u32 getOffset() const {
            u16 offsetBits = data >> 2; // bits 2–15
            return offsetBits * 2;
        }

        void setFormatOffset(u16 offset, u8 format) {
            offset /= 2; // convert byte offset to word offset
            data = ((offset & 0x3FFF) << 2) | (format & 0b11);
        }
    };



    /// "Elytra" chunks.
    /// in-game block order: XZY
    class ChunkFormatGridPaletted : public ChunkFormatBase<ChunkFormatGridPaletted> {
        static constexpr i32 MAX_BLOCKS_SIZE = 64;
        static constexpr i32 GRID_COUNT = 512;
        static constexpr int MAP_SIZE = 256;
        static constexpr eBlockOrder BLOCK_ORDER = XZY;

        using u8FixVec_t = FixedVector<u8, MAX_BLOCKS_SIZE>;

        // Read

        static void readBlocks(std::span<const u8> dataIn, u8* oldBlockPtr) ;
        template<size_t BitsPerBlock>
        static bool readGrid(u8 const* gridDataPtr, u8 blockBuffer[MAX_BLOCKS_SIZE]);


        // Write

        static void writeBlocks(DataWriter& writer, WriteSettings& settings, u8 const* oldBlockPtr, bool isBottom);
        template<size_t BitsPerBlock>
        static void writeGrid(DataWriter& writer, u8FixVec_t& blockVector, u8FixVec_t& blockLocations, u8 blockMap[MAP_SIZE]);


    public:
        MU static void readChunk(ChunkData* chunkData, DataReader& reader);
        MU static void writeChunkInternal(ChunkData* chunkData, WriteSettings& settings, DataWriter& writer, bool fastMode);
    };

}