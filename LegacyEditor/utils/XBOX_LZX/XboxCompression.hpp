#pragma once

#include "lzx.h"
#include <iostream>
#include <cstring>

#include "LegacyEditor/utils/processor.hpp"

// https://github.com/matchaxnb/wimlib/blob/master/src/lzx-compress.c


/// the max "amount" here is 0xffff which is only 2^16 - 1, so it won't overflow (0xff < 8) | 0xff
inline int hasOverFlow(const i64 bytesRead, const i64 size, const int amount) {
    if (bytesRead + amount > size) {
        printf("Tried to readBytes past buffer when decompressing buffer with xmem\n");
        return 1;
    }
    return 0;
}


static int XDecompress(u8* output, const u32* sizeOut, const u8* input, const u32 sizeIn) {
    auto* dst = static_cast<u8*>(malloc(0x8000));
    auto* src = static_cast<u8*>(malloc(0x8000));
    if (src == nullptr || dst == nullptr) {
        printf("Out of memory, could not allocate 2 * 32768 bytes for buffer, exiting\n");
        if (src != nullptr) {
            free(src);
        }
        if (dst != nullptr) {
            free(dst);
        }
        return 0;
    }
    u8* outputPointer = output;
    lzx_state* strm = lzx_init(17);
    if (!strm) {
        printf("Failed to initialize lzx decompressor, exiting\n");
        free(src);
        free(dst);
        return 0;
    }

    i64 bytes = 0;
    int wasLargerThan0x8000 = 0;
    while (bytes < *sizeOut) {
        constexpr i64 bytesRead = 0;
        if (hasOverFlow(bytesRead, sizeIn, 1)) {
            goto DECOMP_ERROR;
        }
        int src_size, dst_size, lo;
        int hi = *input;
        input++;
        if (hi == 0xFF) {
            if (hasOverFlow(bytesRead, sizeIn, 4)) {
                goto DECOMP_ERROR;
            }
            hi = *input;
            input++;
            lo = *input;
            input++;
            dst_size = hi << 8 | lo;
            if (dst_size > 0x8000) {
                printf("Invalid data, exiting\n");
                bytes = 0;
                break;
            }
            hi = *input;
            input++;
            lo = *input;
            input++;
            src_size = hi << 8 | lo;
        } else {
            if (hasOverFlow(bytesRead, sizeIn, 1)) {
                goto DECOMP_ERROR;
            }
            dst_size = 0x8000;

            lo = *input;
            input++;
            src_size = hi << 8 | lo;
        }

        if (src_size == 0 || dst_size == 0) {
            break;
        }
        if (wasLargerThan0x8000 && src_size <= 0x8000) {
            wasLargerThan0x8000 = 0;
            free(src);
            src = static_cast<u8*>(malloc(0x8000));
            if (src == nullptr) {
                printf("Out of memory, could not allocate 32768 bytes for buffer, exiting\n");
                bytes = 0;
                goto DECOMP_ERROR;
            }
        }

        // the compressed size if rarely larger than the decompressed size
        if (src_size > 0x8000) {
            wasLargerThan0x8000 = 1;
            free(src);
            src = static_cast<u8*>(malloc(src_size));
            if (src == nullptr) {
                printf("Out of memory, could not allocate %d bytes for buffer, exiting\n", src_size);
                bytes = 0;
                // goto DECOMP_ERROR;
                break;
            }
        }
        if (hasOverFlow(bytesRead, sizeIn, src_size)) {
            // goto DECOMP_ERROR;
            break;
        }
        memcpy(src, input, src_size);
        input += src_size;
        const int error = lzx_decompress(strm, src, dst, src_size, dst_size);
        if (error == 0) {
            memcpy(outputPointer, dst, dst_size);
            outputPointer += dst_size;
            bytes += dst_size;
        } else {
            printf("Error decompressing, exiting\n");
            bytes = 0;
            break;
        }
    }
DECOMP_ERROR:
    if (src != nullptr) {
        free(src);
    }
    free(dst);
    lzx_teardown(strm);
    return static_cast<int>(bytes);
}
