#pragma once

#include <bit>

#include "buffer.hpp"
#include "utils.hpp"

#include "include/lce/processor.hpp"
#include "include/ghc/fs_std.hpp"


class DataWriter {
    static constexpr auto kDeleteArr = [](u8* p){ delete[] p; };


    std::unique_ptr<u8[], void(*)(u8*)> _buf{nullptr, [](u8*){}};
    std::size_t _cap = 0;       // total allocated
    std::size_t _pos = 0;       // write cursor
    Endian _end = Endian::Big;
    bool _external = false;

    void grow(std::size_t requiredSize) {
        if (_external)
            throw std::length_error("DataWriter overflow (external buffer)");

        std::size_t newCap = _cap ? _cap * 2 : 256;
        while (newCap < requiredSize) newCap *= 2;

        auto newBuf = std::unique_ptr<u8[], void(*)(u8*)>(new u8[newCap], kDeleteArr);
        if (_buf) std::memcpy(newBuf.get(), _buf.get(), _pos);
        _buf = std::move(newBuf);
        _cap = newCap;
    }
    
    void need(std::size_t n) {
        if (_pos + n > _cap) grow(_pos + n);
    }
    
    void needAt(std::size_t off, std::size_t n) {
        if (off + n > _cap) grow(off + n);
    }

public:

    explicit DataWriter(std::size_t reserve = 256, Endian e = Endian::Big)
        : _buf(new u8[reserve], [](u8* p){ delete[] p; }),
          _cap(reserve),
          _end(e)
    {}

    DataWriter(u8* extBuf, std::size_t capacity,
               Endian e = Endian::Big)              noexcept
        : _buf(extBuf, [](u8*){}),
          _cap(capacity),
          _end(e),
          _external(true)
    {}

    void setEndian(Endian e) { _end = e; }
    Endian getEndian() { return _end; }

    // navigation ----------------------------------------------------

    ND c_u8* data() const { return _buf.get(); }
    ND u32 size() const { return _pos; }
    ND u32 tell() const { return _pos; }

    template<std::integral N>
    void seek(N p) {
        needAt(p, 4);
        _pos = p;
    }

    template<std::integral N>
    void skip(N n) { need(n); _pos += n; }
    template<std::size_t N>
    void skip() { _pos += N; }
    void rewind() { _pos = 0; }

    // release / expose ---------------------------------------------

    ND std::span<c_u8> span() const { return {_buf.get(), _pos}; }
    Buffer take();

    // primitive write ----------------------------------------------


    // TODO: inline this piece of garbage
    void writeSwitchU64(u64 v) {
        v = detail::maybe_bswap(v, Endian::Native, _end);
        u32* ptr = reinterpret_cast<u32*>(&v);
        write<u32>(ptr[1]);
        write<u32>(ptr[0]);
    }


    template<typename T>
        requires(std::integral<T> || std::floating_point<T>)
    void write(T v) {
        if constexpr (std::is_floating_point_v<T>) {
            using I = std::conditional_t<sizeof(T) == 4, u32, u64>;
            write<I>(std::bit_cast<I>(v));
        } else {
            need(sizeof(T));
            v = detail::maybe_bswap(v, Endian::Native, _end);
            std::memcpy(_buf.get() + _pos, &v, sizeof(T));
            _pos += sizeof(T);
        }
    }

    template<typename T>
        requires(std::integral<T> || std::floating_point<T>)
    void writeAtOffset(std::size_t off, T v) {
        if constexpr (std::is_floating_point_v<T>) {
            using I = std::conditional_t<sizeof(T) == 4, u32, u64>;
            writeAtOffset<I>(off, std::bit_cast<I>(v));
        } else {
            needAt(off, sizeof(T));
            v = detail::maybe_bswap(v, Endian::Native, _end);
            std::memcpy(_buf.get() + off, &v, sizeof(T));
        }
    }

    // raw blocks ----------------------------------------------------

    void writeBytes(c_u8* src, std::size_t n) {
        need(n);
        std::memcpy(_buf.get() + _pos, src, n);
        _pos += n;
    }
    
    MU void writeBytesAtOffset(std::size_t off, c_u8* src, std::size_t n) {
        needAt(off, n);
        std::memcpy(_buf.get() + off, src, n);
    }
    
    void writePad(std::size_t n, u8 val = 0) {
        need(n);
        std::memset(_buf.get() + _pos, val, n);
        _pos += n;
    }

    // string helpers ------------------------------------------------

    void writeStringLengthPrefixed(std::string str) {
        c_u32 str_size = str.size();
        write<u16>(str_size);
        writeBytes(reinterpret_cast<u8*>(str.data()), str_size);
    }

    void writeUTF16(const std::wstring& w, std::size_t max) {
        c_u32 wstr_size_min = std::min(w.size(), max);
        for (u32 i = 0; i < max && i < wstr_size_min; ++i) {
            write<u16>(w[i]);
        }
        // hack, write null char if there is space, and fill rest of space with null as well
        if (wstr_size_min < max) {
            u32 count = max - wstr_size_min;
            for (u32 i = 0; i < count; i++) {
                write<u16>(0);
            }
        }
    }

    // Switch/WiiU "WWW" string (char + null char)
    void writeWWWString(const std::wstring& w, std::size_t max) {
        c_u32 wstr_size_min = std::min(w.size(), max);
        for (std::size_t i = 0; i < max && i < wstr_size_min; ++i) {
            write<u16>(w[i]);
            write<u16>(0);
        }
        // fill rest of space with null
        if (wstr_size_min < max) {
            for (int i = 0; i < max - wstr_size_min; i++) {
                write<u32>(0); // null char
            }
        }
    }

    // UTF‑16 field from narrow ASCII/UTF‑8 string (old writeWStringFromString)
    void writeWStringFromString(const std::string& s, std::size_t maxWchars) {
        MU c_u8 zero = 0;
        for (std::size_t i = 0; i < maxWchars && i < s.size(); ++i) {
            if (_end == Endian::Native) {
                write<u8>(static_cast<u8>(s[i]));
                write<u8>(0);
            } else {
                write<u8>(0);
                write<u8>(static_cast<u8>(s[i]));
            }
        }
        // pad remaining slots with two null bytes each
        for (std::size_t i = s.size(); i < maxWchars; ++i) {
            write<u8>(0);
            write<u8>(0);
        }
    }

    // file I/O ------------------------------------------------------

    void save(const fs::path& p) const {
        std::ofstream os(p, std::ios::binary);
        if (!os.is_open()) throw std::runtime_error("create failed " + p.string());
        os.write((const char*) _buf.get(), static_cast<std::streamsize>(_pos));
        os.close();
    }


    static void writeFile(const fs::path& p,
                          std::span<c_u8> bytes) {
        std::ofstream out(p, std::ios::binary);
        if (!out.is_open()) throw std::runtime_error("create failed " + p.string());
        out.write(reinterpret_cast<const char*>(bytes.data()), (u32)bytes.size());
        out.close();
    }
};

