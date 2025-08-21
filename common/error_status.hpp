#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>


enum STATUS : int8_t {
    SUCCESS = 0,
    COMPRESS = -1,
    DECOMPRESS = -2,
    MALLOC_FAILED = -3,
    INVALID_SAVE = -4,
    FILE_ERROR = -5,
    INVALID_CONSOLE = -6,
    INVALID_ARGUMENT = -7,
    NOT_IMPLEMENTED = -8,
    ALREADY_WRITTEN = -9
};



[[maybe_unused]] static constexpr char ERROR_1[68]
        = "Could not allocate %d bytes of data for input file buffer, exiting\n";
[[maybe_unused]] static constexpr char ERROR_2[80]
        = "Could not allocate %d bytes of data for input and decompressed buffer, exiting\n";
[[maybe_unused]] static constexpr char ERROR_3[43]
        = "Not a Minecraft console savefile, exiting\n";
[[maybe_unused]] static constexpr char ERROR_4[24] = "Cannot open infile %s\n\n";
[[maybe_unused]] static constexpr char ERROR_5[45] = "Input file is to small to read header union\n";