#pragma once

#include "include/lce/processor.hpp"

#include <iostream>


class DataManager;


namespace editor::chunk {

    class ChunkData;

    enum V11_GRID_STATE : u16 {
        V11_0_BIT = 65535,
        V11_1_BIT = 0,
        V11_2_BIT = 1,
        V11_3_BIT = 2,
        V11_4_BIT = 3,
    };


    /**
     * the data is stored in the m_order of
     * [ byte1 | byte0 ]
     * but for readability we will swap it.
     *
     * <br>             [    byte0 |    byte1 ]
     * <br>     VAR:    [ ######## | ######## ]
     * <br>IF:
     * <br>     value : [ -------- | 00000111 ]
     * <br>THEN:
     * <br>     block : [ ######## | -------- ]
     * <br>ELSE:
     * <br>     format: [ -------- | ------## ]
     * <br>     offset: [ -------# | ######-- ]
     *
     * <br>This does not skip dataManager->data.
     * <br>c_u32 dataOffset = (byte0 << 7U) + ((byte1 & 0b11111100U) >> 1);
     */

    struct Grid {
        static constexpr u8 IS_SINGLE_BLOCK_FLAG = 0b00000111;

        union {
            struct {
                u8 byte0;
                u8 byte1;
            } as_bytes{};
            struct {
                u8 block;
                u8 flag;
            } single;
            struct {
                u16 not_used_Z_ : 6;
                u16 grid_offset : 8;
                u16 grid_format : 2;
            } multiple;
        };

        Grid(c_u8 byte0, c_u8 byte1) {
            setBytes(byte0, byte1);
        }

        void setBytes(c_u8 byte0, c_u8 byte1) {
            as_bytes.byte0 = byte1;
            as_bytes.byte1 = byte0;
        }

        ND u32 getDataOffset() const {
            return (as_bytes.byte1 << 7U) + ((as_bytes.byte0 & 0b11111100U) >> 1);
        }

        void print() const;
    };


    /// "Elytra" chunks.
    class ChunkV11 {
        static constexpr i32 GRID_SIZE = 64;
        static constexpr i32 GRID_COUNT = 512;
        static constexpr int MAP_SIZE = 65536;

        // Read

        void readBlocks(u8* oldBlockPtr) const;
        template<size_t BitsPerBlock>
        MU static bool readGrid(u8 const* gridDataPtr, u8 blockBuffer[GRID_SIZE]);


        // Write

        MU void writeBlocks(u8 const* oldBlockPtr) const;
        template<size_t BitsPerBlock>
        void writeGrid(u16_vec& blockVector, u16_vec& blockLocations, u8 blockMap[MAP_SIZE]) const;
        void writeWithMaxBlocks(const u16_vec& blockVector, const u16_vec& blockLocations, u8 blockMap[MAP_SIZE]) const;


    public:
        ChunkData* chunkData = nullptr;
        DataManager* dataManager = nullptr;

        ChunkV11(ChunkData* chunkDataIn, DataManager* managerIn) :
            chunkData(chunkDataIn), dataManager(managerIn) {}

        MU void allocChunk() const;
        MU void readChunk() const;
        MU void writeChunk();
    };

}