#pragma once

#include "ChunkManager.hpp"
#include "LegacyEditor/utils/enums.hpp"
#include "LegacyEditor/utils/processor.hpp"


class RegionManager {
private:
    static constexpr u32 CHUNK_COUNT = 1024;
    static constexpr u32 SECTOR_SIZE = 4096;

public:
    ChunkManager chunks[CHUNK_COUNT] = {};
    u32 totalSectors = 0;
    CONSOLE console = CONSOLE::NONE;

    explicit RegionManager(CONSOLE consoleIn) : console(consoleIn) {}

    ChunkManager* getChunk(int x, int z);
    ChunkManager* getChunk(int index);

    void read(File* fileIn);
    Data write(CONSOLE consoleIn);
};