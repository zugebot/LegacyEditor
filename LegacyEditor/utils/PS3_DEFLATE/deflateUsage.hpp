#pragma once

#include <cstdint>


/// negative window bits for deflate (no zlib header) regular for zlib and regular + 16 is gzip header
static int def(uint8_t* inBuffer, uint8_t *outBuffer, unsigned long sizeIn, unsigned long *sizeOut, int windowBits = 15);
