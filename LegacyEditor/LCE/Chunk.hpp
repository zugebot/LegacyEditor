#pragma once

#include "LegacyEditor/utils/LZX/XboxCompression.hpp"
#include "LegacyEditor/utils/RLE/rle.hpp"
#include "LegacyEditor/utils/data.hpp"
#include "LegacyEditor/utils/enums.hpp"
#include "LegacyEditor/utils/processor.hpp"
#include "LegacyEditor/utils/tinf/tinf.h"
#include "LegacyEditor/utils/zlib-1.2.12/zlib.h"


class Chunk : public Data {
public:
    u32 dec_size;
    u8 sectors;
    u32 location;
    u32 timestamp;
    bool isCompressed = true;
    bool rleFlag = false;

    // ND inline uint32_t getOffset() const { return location; }
    // ND inline uint8_t getSectorCount() const { return sectors; }

    void ensure_decompress(CONSOLE console) {
        if (!isCompressed) {
            printf("cannot Chunk.ensure_decompress the chunk if its already decompressed\n");
            return;
        }

        if (console == CONSOLE::NONE) {
            printf("passed CONSOLE::NONE to Chunk.ensure_decompress, results will not work\n");
            return;
        }

        if (data == nullptr) {
            printf("chunk data is nullptr, cannot Chunk.ensure_decompress nothing\n");
            return;
        }

        isCompressed = false;

        u32 dec_size_copy = dec_size;

        Data decompData(dec_size);
        // auto* decompressedData = new u8[dec_size];
        switch (console) {
            case CONSOLE::XBOX360:
                dec_size_copy = XDecompress(decompData.start(), &decompData.size, data, size);
                break;
            case CONSOLE::PS3:
                tinf_uncompress(decompData.start(), &decompData.size, data, size);
                break;
            case CONSOLE::WIIU:
                tinf_zlib_uncompress(decompData.start(), &decompData.size, data, size);
                break;
            default:
                break;
        }

        delete[] data;
        data = nullptr;

        if (rleFlag) {
            Data rle(dec_size);
            RLE_uncompress(decompData.start(), decompData.size, rle.start(), dec_size);
            data = rle.data;
            rle.using_memory = false;
            size = dec_size;
            return;
        }

        data = decompData.data;
        decompData.using_memory = false;
        size = dec_size_copy;
    }

    MU void ensure_compressed(CONSOLE console) {
        if (isCompressed) {
            // printf("cannot Chunk.ensure_compress if the chunk if its already compressed\n");
            return;
        }

        if (console == CONSOLE::NONE) {
            printf("passed CONSOLE::NONE to Chunk.ensure_compress, results will not work\n");
            return;
        }

        if (data == nullptr) {
            printf("chunk data is nullptr, cannot Chunk.ensure_compress nothing\n");
            return;
        }

        dec_size = size;

        if (rleFlag) {
            u32 rle_size = size;
            u8* rle_ptr = new u8[rle_size];
            RLE_compress(rle_ptr, rle_size, data, size);
            delete[] data;
            data = nullptr;
            data = rle_ptr;
            size = rle_size;
        }

        // allocate memory and recompress
        u8* comp_ptr = new u8[size];
        uint64_t comp_size = size;

        switch (console) {
            case CONSOLE::XBOX360:
                // XCompress(comp_ptr, comp_size, data_ptr, data_size);
                break;
            case CONSOLE::PS3:
                // tinf_compress(comp_ptr, comp_size, data_ptr, data_size);
                break;
            case CONSOLE::WIIU: {
                uLongf _comp_size = comp_size;
                ::compress(comp_ptr, &_comp_size, data, size);
                break;
            }
            default:
                break;
        }


    }

};
