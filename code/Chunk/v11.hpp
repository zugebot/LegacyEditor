#pragma once

#include "include/lce/processor.hpp"


class DataManager;


namespace editor::chunk {

    class ChunkData;

    enum V11_GRID_STATE : u8 {
        V11_1_BIT = 0,
        V11_2_BIT = 1,
        V11_3_BIT = 2,
        V11_4_BIT = 3,
    };

    /// "Elytra" chunks.
    class ChunkV11 {
        static constexpr i32 GRID_SIZE = 64;
        static constexpr i32 GRID_HEADER_SIZE = 1024;
        static constexpr i32 GRID_COUNT = 512;
        static constexpr int MAP_SIZE = 65536;

        // Read

        MU void readBlockData() const;
        template<size_t BitsPerBlock>
        MU static bool readGrid(u8 const* buffer, u8 grid[GRID_SIZE]);


        // Write

        MU void writeBlockData() const;
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