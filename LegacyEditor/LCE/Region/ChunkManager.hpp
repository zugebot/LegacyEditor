#pragma once

#include "LegacyEditor/utils/enums.hpp"
#include "LegacyEditor/utils/data.hpp"


class ChunkManager : public Data {
public:
    u32 dec_size = 0, location = 0, timestamp = 0;
    bool isCompressed = true, rleFlag = true, unknownFlag = true;
    u8 sectors = 0;

    ~ChunkManager() { deallocate(); }

    void ensure_decompress(CONSOLE console);
    void ensure_compressed(CONSOLE console);
};
