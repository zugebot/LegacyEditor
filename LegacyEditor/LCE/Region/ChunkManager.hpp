#pragma once

#include "LegacyEditor/utils/enums.hpp"
#include "LegacyEditor/utils/data.hpp"



class ChunkManager : public Data {
public:
    u32 dec_size = 0;
    u32 location = 0;
    u32 timestamp = 0;
    u8 sectors = 0;
    bool isCompressed = true;
    bool rleFlag = false;

    ~ChunkManager() {
        deallocate();
    }

    void ensure_decompress(CONSOLE console);
    void ensure_compressed(CONSOLE console);
};
