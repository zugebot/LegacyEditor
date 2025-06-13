#pragma once

#include "include/lce/processor.hpp"
#include "common/fixedVector.hpp"

#include "../../common/data/DataReader.hpp"
#include "../../common/data/DataWriter.hpp"
#include "vBase.hpp"


namespace editor::chunk {

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

        u8 m_byte0{};
        u8 m_byte1{};

        Grid() = default;

        Grid(c_u8 byte0, c_u8 byte1) {
            setBytes(byte0, byte1);
        }

        void setBytes(c_u8 byte0in, c_u8 byte1in) {
            m_byte0 = byte0in;
            m_byte1 = byte1in;
        }

        ND u32 getOffset() const {
            return ((m_byte1 << 6U) + ((m_byte0 & 0b11111100U) >> 2)) * 2;
        }

        MU ND u32 getFormat() const {
            return m_byte0 & 0b11U;
        }

        void setFormatOffset(u16 offset, V11GridFormat format) {
            offset = offset / 2;
            m_byte0 = (m_byte0 & 0b11111100) | (format & 0b11);
            // Set the 6 most significant bits of the offset in m_byte0
            m_byte0 = (m_byte0 & 0b00000011) | ((offset & 0b1111110000) >> 6);
            // Set the 2 least significant bits of the offset in m_byte1
            m_byte1 = (m_byte1 & 0b00111111) | ((offset & 0b00000011) << 6);
        }

        MU ND bool isSingleBlock() const {
            return (m_byte0 & 0b111U) == Grid::IS_SINGLE_BLOCK_FLAG;
        }

        MU ND u32 getSingleBlock() const {
            return m_byte1;
        }
    };


    /// "Elytra" chunks.
    class ChunkV11 : VChunkBase {
        static constexpr i32 MAX_BLOCKS_SIZE = 64;
        static constexpr i32 GRID_COUNT = 512;
        static constexpr int MAP_SIZE = 256;

        using u8FixVec_t = FixedVector<u8, MAX_BLOCKS_SIZE>;

        // Read

        void readBlocks(std::span<const u8> dataIn, u8* oldBlockPtr) const;
        template<size_t BitsPerBlock>
        MU static bool readGrid(u8 const* gridDataPtr, u8 blockBuffer[MAX_BLOCKS_SIZE]);


        // Write

        MU void writeBlocks(DataWriter& writer, u8 const* oldBlockPtr) const;
        template<size_t BitsPerBlock>
        void writeGrid(DataWriter& writer, u8FixVec_t& blockVector, u8FixVec_t& blockLocations, u8 blockMap[MAP_SIZE]) const;


    public:
        explicit ChunkV11(ChunkData* chunkDataIn) : VChunkBase(chunkDataIn) {}


        MU void allocChunk() const override;
        MU void readChunk(DataReader& reader) override;
        MU void writeChunk(DataWriter& writer) override;
    };

}