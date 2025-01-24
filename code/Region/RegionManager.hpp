#pragma once

#include "include/lce/processor.hpp"

#include "code/Region/ChunkManager.hpp"


namespace editor {
    class LCEFile;

    class RegionManager {
        static constexpr u32 REGION_WIDTH = 32;
        static constexpr u32 SECTOR_INTS = REGION_WIDTH * REGION_WIDTH; // SECTOR_BYTES / 4
        static constexpr u32 SECTOR_BYTES = 4 * SECTOR_INTS;
        static constexpr u32 CHUNK_HEADER_SIZE = 12;

    public:
        ChunkManager chunks[SECTOR_INTS];
        lce::CONSOLE myConsole = lce::CONSOLE::NONE;

        /// CONSTRUCTORS

        RegionManager() = default;
        ~RegionManager() = default;

        /// FUNCTIONS

        MU ChunkManager* getChunk(int xIn, int zIn);
        MU ChunkManager* getChunk(u32 index);
        MU ChunkManager* getNonEmptyChunk();

        /// READ AND WRITE

        int read(const LCEFile* fileIn);
        MU void convertChunks(lce::CONSOLE consoleIn);
        Data write(lce::CONSOLE consoleIn);

    };

}
