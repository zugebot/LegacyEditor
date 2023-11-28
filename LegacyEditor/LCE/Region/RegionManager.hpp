#pragma once

#include "ChunkManager.hpp"

#include "LegacyEditor/utils/enums.hpp"
#include "LegacyEditor/utils/processor.hpp"


class File;

class RegionManager {
private:
    static constexpr u32 REGION_WIDTH = 32;
    static constexpr u32 CHUNK_COUNT = 1024;
    static constexpr u32 SECTOR_SIZE = 4096;

public:
    ChunkManager chunks[CHUNK_COUNT] = {};
    u32 totalSectors = 0;
    CONSOLE console = CONSOLE::NONE;

    explicit RegionManager(CONSOLE consoleIn) : console(consoleIn) {}

    /// FUNCTIONS

    MU ChunkManager* getChunk(int x, int z);
    MU ChunkManager* getChunk(int index);
    MU ChunkManager* getNonEmptyChunk();

    /// READ

    void read(File* fileIn);
    void read(Data* dataIn);

    /// WRITE

    Data write(CONSOLE consoleIn);
};