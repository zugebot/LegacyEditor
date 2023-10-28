#pragma once

#include "Chunk.hpp"
#include "LegacyEditor/utils/enums.hpp"
#include "LegacyEditor/utils/processor.hpp"


class Region {
private:
    static constexpr u32 CHUNK_COUNT = 1024;
    static constexpr u32 SECTOR_SIZE = 4096;

public:
    Chunk chunks[CHUNK_COUNT] = {};
    u32 totalSectors = 0;
    CONSOLE console = CONSOLE::NONE;

    explicit Region(CONSOLE consoleIn) : console(consoleIn) {}

    Chunk* getChunk(int x, int z);
    Chunk* getChunk(int index);

    void read(File* fileIn);
    Data write(CONSOLE consoleIn);
};