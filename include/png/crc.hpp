#pragma once

#include <array>
#include <cstdint>

constexpr uint32_t CRC_INITIAL = 0xffffffffL;
constexpr uint32_t CRC_XOR_OUT = 0xffffffffL;
constexpr uint32_t CRC_POLY = 0xedb88320L;


/// generates the crc table at compile time for speed
constexpr std::array<uint32_t, 256> generate_crc_table() {
    std::array<uint32_t, 256> crc_table{};
    for (int index = 0; index < 256; ++index) {
        auto c = static_cast<uint32_t>(index);
        for (int k = 0; k < 8; ++k) {
            if (c & 1) {
                c = CRC_POLY ^ (c >> 1);
            } else {
                c >>= 1;
            }
        }
        crc_table[index] = c;
    }
    return crc_table;
}

/**
  Purpose:
    CRC returns the CRC of the bytes in BUF[0...LEN-1].

Discussion:
    Recall that ^ is the bitwise XOR operator and that
    0xNNN introduces a hexadecimal constant..

    Modified:
    21 December 2023 by Jerrin Shirks
     1 July 2023 by Jerrin Shirks

     Original Credits:
            Reference:
    Glenn Randers-Pehrson, et al,
        PNG (Portable Network Graphics) Specification,
        Version 1.2, July 1999.

        Parameters:
    Input, unsigned char* BUF, a string whose CRC is to be computed.
    Input, int LEN, the number of characters in the string.
    Output, unsigned long CRC, the CRC of the string.
                    */
inline uint32_t crc(const uint8_t* bufferIn, const uint32_t len) {
    static constexpr auto crc_table = generate_crc_table();

    uint32_t crc = CRC_INITIAL;
    for (uint32_t index = 0; index < len; ++index) {
        crc = crc_table[(crc ^ bufferIn[index]) & 0xff] ^ (crc >> 8);
    }
    return crc ^ CRC_XOR_OUT;
}