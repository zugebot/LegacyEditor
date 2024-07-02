#pragma once

#include "lce/processor.hpp"

#include "LegacyEditor/code/Region/ChunkManager.hpp"


namespace editor {
    class LCEFile;

    class RegionManager {
        // TODO: maybe PS4 / SWITCH have this value as 27?
        static constexpr u32 REGION_WIDTH = 32;
        static constexpr u32 SECTOR_BYTES = 4096;
        static constexpr u32 SECTOR_INTS = 1024; // SECTOR_BYTES / 4
        static constexpr u32 CHUNK_HEADER_SIZE = 12;

    public:
        ChunkManager chunks[SECTOR_INTS];
        lce::CONSOLE console = lce::CONSOLE::NONE;

        /// CONSTRUCTORS

        RegionManager();
        ~RegionManager();

        /// FUNCTIONS

        MU ChunkManager* getChunk(int xIn, int zIn);
        MU ChunkManager* getChunk(const uint32_t index);
        MU ChunkManager* getNonEmptyChunk();

        /// READ AND WRITE

        void read(const LCEFile* fileIn);
        Data write(lce::CONSOLE consoleIn);

    };

}
