#pragma once

#include "include/lce/processor.hpp"

#include "../data/DataReader.hpp"
#include "../data/DataWriter.hpp"
#include "include/lzx/lzx.h"

// https://github.com/matchaxnb/wimlib/blob/master/src/lzx-compress.c

namespace codec {
    enum class XmemErr {
        Ok = 0,
        Overflow,
        BadData,
        LzxInit,
        LzxRun,
        BufferTooSmall,
    };


    ND inline constexpr const char* to_string(XmemErr e) noexcept
    {
        switch (e) {
            case XmemErr::Ok            : return "no error";
            case XmemErr::Overflow      : return "input exhausted";
            case XmemErr::BadData       : return "invalid block header";
            case XmemErr::LzxInit       : return "cannot initialise LZX";
            case XmemErr::LzxRun        : return "LZX decompression failed";
            case XmemErr::BufferTooSmall: return "output buffer too small";
        }
        return "unknown error";
    }


    struct LzxDecompressor {
        explicit LzxDecompressor(int window_bits = 17)
            : ptr(lzx_init(window_bits)) {}
        ~LzxDecompressor() { if (ptr) lzx_teardown(ptr); }

        LzxDecompressor(const LzxDecompressor&)            = delete;
        LzxDecompressor& operator=(const LzxDecompressor&) = delete;
        LzxDecompressor(LzxDecompressor&& other) noexcept  : ptr(std::exchange(other.ptr, nullptr)) {}
        LzxDecompressor& operator=(LzxDecompressor&&)      = delete;

        ND lzx_state* get() const noexcept { return ptr; }
        explicit operator bool() const noexcept { return ptr != nullptr; }
    private:
        lzx_state* ptr{};
    };


    ND static XmemErr XDecompress(const u8* dataIn, u32 sizeIn, u8* dataOut, u32* sizeOut) {
        static constexpr int32_t CHUNK_SIZE = 0x8000;

        DataReader reader(dataIn, sizeIn);
        DataWriter writer(*sizeOut);

        u8 dst[CHUNK_SIZE] = {0};
        u8 src[CHUNK_SIZE * 2] = {0};

        LzxDecompressor lzx;
        if (!lzx) {
            *sizeOut = 0;
            return XmemErr::LzxInit;
        }

        auto bail = [&](XmemErr e){ *sizeOut = 0; return e; };

        bool last = false;
        while (!last) {
            int dst_size = CHUNK_SIZE;

            if EXPECT_FALSE(reader.peek() == 0xFF) {
                if (!reader.canRead(3))
                    return bail(XmemErr::Overflow);
                reader.read<u8>(); // consume the 0xFF byte
                dst_size = reader.read<u16>();
                last = true;
            }

            if (!reader.canRead(2))
                return bail(XmemErr::Overflow);

            int src_size = reader.read<u16>();

            // validate dst_size and src_size
            if (src_size == 0 || src_size > CHUNK_SIZE * 2 ||
                dst_size == 0 || dst_size > CHUNK_SIZE) {
                return bail(XmemErr::BadData);
            }

            // read data into the buffer,
            if (!reader.canRead(src_size))
                return bail(XmemErr::Overflow);
            reader.readBytes(src_size, src);

            // then decompress the data.
            c_int lzx_error = lzx_decompress(lzx.get(), src, dst, src_size, dst_size);
            if (lzx_error == 0) {
                writer.writeBytes(dst, dst_size);
            } else {
                return bail(XmemErr::LzxRun);
            }
        }

        std::memcpy(dataOut, writer.data(), writer.size());
        *sizeOut = writer.tell();
        return XmemErr::Ok;
    }
}
