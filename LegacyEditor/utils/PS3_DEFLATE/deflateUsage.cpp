#include "deflateUsage.hpp"

#include "include/zlib-1.2.12/zlib.h"

#define ZIPCHUNK_SIZE 32768

/// negative window bits for deflate (no zlib header) regular for zlib and regular + 16 is gzip header
static int def(u8* inBuffer, u8* outBuffer, uLongf sizeIn, uLongf* sizeOut, int windowBits) {
    // Prepare output buffer memory.
    int level = Z_DEFAULT_COMPRESSION;
    int ret = 0;
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    ret = deflateInit2(&stream, level, Z_DEFLATED, windowBits, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) return ret;
    stream.next_out = outBuffer;
    stream.avail_out = 0;
    stream.next_in = inBuffer;
    stream.avail_in = 0;
    int err;
    uLong left = *sizeOut;
    *sizeOut = 0;
    do {
        if (stream.avail_out == 0) {
            stream.avail_out = left > (u16) ZIPCHUNK_SIZE ? ZIPCHUNK_SIZE : (uInt) left;
            left -= stream.avail_out;
        }
        if (stream.avail_in == 0) {
            stream.avail_in = sizeIn > (u16) ZIPCHUNK_SIZE ? ZIPCHUNK_SIZE : (uInt) sizeIn;
            sizeIn -= stream.avail_in;
        }
        err = deflate(&stream, sizeIn ? Z_NO_FLUSH : Z_FINISH);
    } while (err == Z_OK);
    deflateEnd(&stream);
    *sizeOut = stream.total_out;
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
