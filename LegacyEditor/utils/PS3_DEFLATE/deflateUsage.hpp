#pragma once

#include "lce/processor.hpp"


/// negative window bits for deflate (no zlib header) regular for zlib and regular + 16 is gzip header
static int def(u8* inBuffer, u8 *outBuffer, unsigned long sizeIn, unsigned long *sizeOut, int windowBits = 15);
