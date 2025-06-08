// DecompressionHelpers.hpp
#pragma once

// —————————————————————————————————————————————————————————————————————————
// WARNING: Only include <zlib.h> from your system/install‐wide zlib.
// DO NOT include local “include/zlib-1.2.12/zlib.h” unless you also
// set your compiler to look exactly there (and that risks conflicts).
// —————————————————————————————————————————————————————————————————————————
#include <zlib.h>

#include "common/buffer.hpp" // your Buffer class (assumes Buffer(u32) allocates a data() block)
#include "tinf/tinf.h"
#include <span>
#include <stdexcept>

// ----------------------------------------------------------------------------
// Decompress a zlib‐wrapped stream (RFC 1950).  Equivalent to SharpZipLib’s
// InflaterInputStream or .NET’s DeflateStream(with ZLIB wrapper).
//
//   src:          compressed bytes (zlib header + deflate data + adler32 trailer)
//   expectedSize: the exact uncompressed length you expect
//
// Returns a Buffer of length expectedSize, or throws if inflate fails or
// if the actual uncompressed length != expectedSize.
// ----------------------------------------------------------------------------
inline Buffer DecompressWithTinf(std::span<const uint8_t> src, uint32_t expectedSize) {
    if (expectedSize == 0) {
        // No output bytes expected → return an empty Buffer.
        return Buffer(0);
    }

    // Allocate a Buffer of exactly expectedSize bytes:
    Buffer out(expectedSize);
    if (out.size() != expectedSize) {
        throw std::runtime_error("Failed to allocate output buffer of size " + std::to_string(expectedSize));
    }

    // tinf_uncompress signature:
    //    int tinf_uncompress(Bytef* dest, unsigned long* destLen, const Bytef* src, unsigned long srcLen);
    //
    // It will decode either a zlib‐wrapped stream or raw deflate. On success, destLen becomes actual output length.
    unsigned int actualSize = expectedSize;
    int ret = tinf_uncompress(out.data(), &actualSize, src.data(), static_cast<unsigned long>(src.size()));
    if (ret != 0) {
        throw std::runtime_error("tinf_uncompress failed (code " + std::to_string(ret) + ")");
    }
    if (actualSize != expectedSize) {
        throw std::runtime_error(
                "tinf output size mismatch: expected " + std::to_string(expectedSize) +
                ", got " + std::to_string(actualSize));
    }
    return out;
}