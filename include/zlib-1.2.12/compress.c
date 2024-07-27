/* compress.c -- compress a memory buffer
 * Copyright (C) 1995-2005, 2014, 2016 Jean-loup Gailly, Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#define ZLIB_INTERNAL
#include "zlib.h"
#include <stdint.h>

/* ===========================================================================
     Compresses the source buffer into the destination buffer. The level
   parameter has the same meaning as in deflateInit.  sourceLen is the byte
   length of the source buffer. Upon entry, destLen is the total size of the
   destination buffer, which must be at least 0.1% larger than sourceLen plus
   12 bytes. Upon exit, destLen is the actual size of the compressed buffer.

     compress2 returns Z_OK if success, Z_MEM_ERROR if there was not enough
   memory, Z_BUF_ERROR if there was not enough room in the output buffer,
   Z_STREAM_ERROR if the level parameter is invalid.
*/
int ZEXPORT compress2 (Bytef *dest,
                      uLongf *destLen,
                      const Bytef *source,
                      uLong sourceLen,
                      int level)
{
    z_stream stream;
    int err;
    const uInt max = (uInt)-1;
    uLong left;

    left = *destLen;
    *destLen = 0;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = deflateInit(&stream, level);
    if (err != Z_OK) return err;

    stream.next_out = dest;
    stream.avail_out = 0;
    stream.next_in = (z_const Bytef *)source;
    stream.avail_in = 0;

    do {
        if (stream.avail_out == 0) {
            stream.avail_out = left > (uLong)max ? max : (uInt)left;
            left -= stream.avail_out;
        }
        if (stream.avail_in == 0) {
            stream.avail_in = sourceLen > (uLong)max ? max : (uInt)sourceLen;
            sourceLen -= stream.avail_in;
        }
        err = deflate(&stream, sourceLen ? Z_NO_FLUSH : Z_FINISH);
    } while (err == Z_OK);

    *destLen = stream.total_out;
    deflateEnd(&stream);
    return err == Z_STREAM_END ? Z_OK : err;
}

/* ===========================================================================
 */
int ZEXPORT compress (Bytef *dest,
                     uLongf *destLen,
                     const Bytef *source,
                     uLong sourceLen)
{
    return compress2(dest, destLen, source, sourceLen, Z_DEFAULT_COMPRESSION);
}

/* ===========================================================================
     If the default memLevel or windowBits for deflateInit() is changed, then
   this function needs to be updated.
 */
uLong ZEXPORT compressBound (uLong sourceLen)
{
    return sourceLen + (sourceLen >> 12) + (sourceLen >> 14) +
           (sourceLen >> 25) + 13;
}



//

#define ZIPCHUNK_SIZE 32768

/// negative window bits for deflate (no zlib header) regular for zlib and regular + 16 is gzip header
int ZEXPORT def(uint8_t* inBuffer, uint8_t* outBuffer, uLongf sizeIn, uLongf* sizeOut, int windowBits) {
    windowBits = 15;
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
            stream.avail_out = left > (uint16_t) ZIPCHUNK_SIZE ? ZIPCHUNK_SIZE : (uInt) left;
            left -= stream.avail_out;
        }
        if (stream.avail_in == 0) {
            stream.avail_in = sizeIn > (uint16_t) ZIPCHUNK_SIZE ? ZIPCHUNK_SIZE : (uInt) sizeIn;
            sizeIn -= stream.avail_in;
        }
        err = deflate(&stream, sizeIn ? Z_NO_FLUSH : Z_FINISH);
    } while (err == Z_OK);
    deflateEnd(&stream);
    *sizeOut = stream.total_out;
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
