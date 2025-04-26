#pragma once

#include <cstring>
#include <iostream>

#include "include/lzx/lzx.h"

#include "common/dataManager.hpp"
#include "include/lce/processor.hpp"

// https://github.com/matchaxnb/wimlib/blob/master/src/lzx-compress.c

enum XMEM_ERROR { OVERFLOW_ = -1, MALLOC = -2, LZX = -3, BAD_DATA = -4 };

#define ERROR(_error, _message) \
    {                           \
        managerOut.rewind(); \
        error = _error;         \
        printf(_message);       \
        break;                  \
    }

/// the max "amount" here is 0xffff which is only 2^16 - 1, so it won't overflow (0xff < 8) | 0xff
#define CHECK_CAN_READ(manager_in, amount)                                             \
    if (!manager_in.canRead(amount)) {                                             \
        ERROR(XMEM_ERROR::OVERFLOW_,                                                    \
              "Tried to readBytes past buffer when decompressing buffer with xmem\n"); \
    }

/**]
 *
 * made up of blocks:
 * for all blocks (except the last one):
 *
 *
 *
 *
 * @param the_data_out
 * @param the_size_out
 * @param the_data_in
 * @param the_size_in
 * @return
 */
static int XDecompress(u8* the_data_out, u32* the_size_out, u8* the_data_in, u32 the_size_in) {
    static constexpr int32_t CHUNK_SIZE = 0x8000;

    DataManager managerIn(the_data_in, the_size_in);
    DataManager managerOut(the_data_out, *the_size_out);
    std::vector<std::pair<int, int>> sizes;

    int error = 0;
    bool reached_end_of_data = false;

    u8 dst[CHUNK_SIZE] = {0};
    u8 src[CHUNK_SIZE * 2] = {0};

    lzx_state* strm = lzx_init(17);
    if (!strm) {
        error = XMEM_ERROR::LZX;
        printf("XMEM: Failed to initialize lzx decompressor, exiting\n");
        goto FUNC_END;
    }

    while (!reached_end_of_data) {
        int dst_size = CHUNK_SIZE;

        if EXPECT_FALSE(managerIn.peek() == 0xFF) {
            CHECK_CAN_READ(managerIn, 3)
            managerIn.read<u8>(); // consume the 0xFF byte
            dst_size = managerIn.read<u16>();
            reached_end_of_data = true;
        }
        CHECK_CAN_READ(managerIn, 2)
        int src_size = managerIn.read<u16>();

        // validate dst_size and src_size
        if (src_size == 0 || dst_size == 0)
            ERROR(XMEM_ERROR::BAD_DATA, "XMEM: dst_size == 0 | src_size == 0, exiting\n")
        if (dst_size > CHUNK_SIZE)
            ERROR(XMEM_ERROR::BAD_DATA, "XMEM: dst_size > 32768; invalid data, exiting\n")
        if (src_size > CHUNK_SIZE * 2)
            ERROR(XMEM_ERROR::MALLOC, "XMEM: increase the hardcoded src size")

        // read data into the buffer,
        CHECK_CAN_READ(managerIn, src_size)
        managerIn.readBytes(src_size, src);

        // then decompress the data.
        c_int lzx_error = lzx_decompress(strm, src, dst, src_size, dst_size);
        if (lzx_error == 0) {
            managerOut.writeBytes(dst, dst_size);
        } else {
            ERROR(XMEM_ERROR::LZX, "XMEM: Error decompressing, exiting\n")
        }
    }
FUNC_END:
    lzx_teardown(strm);
    *the_size_out = managerOut.tell();
    return error;
}