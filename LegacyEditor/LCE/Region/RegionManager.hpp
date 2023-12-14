#pragma once

#include "ChunkManager.hpp"

#include "LegacyEditor/utils/processor.hpp"
#include "LegacyEditor/utils/enums.hpp"


class File;

class RegionManager {
private:
    static constexpr u32 REGION_WIDTH = 32;
    static constexpr u32 SECTOR_BYTES = 4096;
    static constexpr u32 SECTOR_INTS = SECTOR_BYTES / 4;

public:
    ChunkManager chunks[SECTOR_INTS] = {};
    CONSOLE console = CONSOLE::NONE;

    explicit RegionManager(CONSOLE consoleIn) : console(consoleIn) {}
    ~RegionManager() = default;

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