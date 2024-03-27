#pragma once


/**
  Purpose:
    CRC returns the CRC of the bytes in BUF[0...LEN-1].

  Discussion:
    Recall that ^ is the bitwise XOR operator and that
    0xNNN introduces a hexadecimal constant..

  Modified:
    21 December 2023 by Jerrin Shirks

  Reference:
    Glenn Randers-Pehrson, et al,
    PNG (Portable Network Graphics) Specification,
    Version 1.2, July 1999.

  Parameters:
    Input, unsigned char* BUF, a string whose CRC is to be computed.
    Input, int LEN, the number of characters in the string.
    Output, unsigned long CRC, the CRC of the string.
*/
inline unsigned long crc(const char* bufferIn, const int len) {
    const auto* buf = reinterpret_cast<const unsigned char*>(bufferIn);

    unsigned long crc = 0xffffffffL;
    static unsigned long crc_table[256];
    static bool crc_table_computed = false;

    if (!crc_table_computed) {
        for (int index = 0; index < 256; index++) {
            auto c = static_cast<unsigned long>(index);
            for (int k = 0; k < 8; k++) {
                if (c & 1) {
                    c = 0xedb88320L ^ c >> 1;
                } else {
                    c = c >> 1;
                }
            }
            crc_table[index] = c;
        }
        crc_table_computed = true;
    }

    for (int index = 0; index < len; index++) {
        crc = crc_table[(crc ^ buf[index]) & 0xff] ^ crc >> 8;
    }

    return crc ^ 0xffffffffL;
}