#pragma once

#include "LegacyEditor/utils/LZX/XboxCompression.hpp"
#include "LegacyEditor/utils/RLE/rle.hpp"
#include "LegacyEditor/utils/data.hpp"
#include "LegacyEditor/utils/enums.hpp"
#include "LegacyEditor/utils/processor.hpp"
#include "LegacyEditor/utils/tinf/tinf.h"
#include "LegacyEditor/utils/zlib-1.2.12/zlib.h"



class ChunkManager : public Data {
public:
    u32 dec_size;
    u8 sectors;
    u32 location;
    u32 timestamp;
    bool isCompressed = true;
    bool rleFlag = false;

    ~ChunkManager() {
        delete[] data;
    }

    void ensure_decompress(CONSOLE console);
    void ensure_compressed(CONSOLE console);
};
