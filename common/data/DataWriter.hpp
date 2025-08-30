#pragma once

#include <bit>
#include <fstream>

#include "buffer.hpp"

#include "include/lce/processor.hpp"
#include "common/data/ghc/fs_std.hpp"


class DataWriter {
    static constexpr auto kDeleteArr = [](uint8_t* p){ delete[] p; };


    std::unique_ptr<uint8_t[], void(*)(uint8_t*)> _buf{nullptr, [](uint8_t*){}};
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
#ifdef MEMSET_ZERO
        std::memset(newBuf.get(), 0, newCap);
#endif

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

    explicit DataWriter(const std::size_t reserve = 256, const Endian e = Endian::Big)
        : _buf(new uint8_t[reserve], [](uint8_t* p){ delete[] p; }),
          _cap(reserve),
          _end(e)
    {}

    DataWriter(uint8_t* extBuf, const std::size_t capacity,
               const Endian e = Endian::Big) noexcept
        : _buf(extBuf, [](uint8_t*){}),
          _cap(capacity),
          _end(e),
          _external(true)
    {}

    void setEndian(const Endian e) { _end = e; }
    Endian getEndian() { return _end; }

    // navigation ----------------------------------------------------

    [[nodiscard]] const uint8_t* data() const { return _buf.get(); }
    [[nodiscard]] uint32_t size() const { return _pos; }
    [[nodiscard]] uint32_t tell() const { return _pos; }

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

    [[nodiscard]] std::span<const uint8_t> span() const { return {_buf.get(), _pos}; }
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
            using I = std::conditional_t<sizeof(T) == 4, uint32_t, uint64_t>;
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
    void writeAtOffset(const std::size_t off, T v) {
        if constexpr (std::is_floating_point_v<T>) {
            using I = std::conditional_t<sizeof(T) == 4, uint32_t, uint64_t>;
            writeAtOffset<I>(off, std::bit_cast<I>(v));
        } else {
            needAt(off, sizeof(T));
            v = detail::maybe_bswap(v, Endian::Native, _end);
            std::memcpy(_buf.get() + off, &v, sizeof(T));
        }
    }

    // raw blocks ----------------------------------------------------

    void writeBytes(const uint8_t* src, const std::size_t n) {
        need(n);
        std::memcpy(_buf.get() + _pos, src, n);
        _pos += n;
    }
    
    [[maybe_unused]] void writeBytesAtOffset(const std::size_t off, const uint8_t* src, const std::size_t n) {
        needAt(off, n);
        std::memcpy(_buf.get() + off, src, n);
    }
    
    void writePad(const std::size_t n, const uint8_t val = 0) {
        need(n);
        std::memset(_buf.get() + _pos, val, n);
        _pos += n;
    }

    // string helpers ------------------------------------------------

    void writeStringLengthPrefixed(std::string str) {
        const uint32_t str_size = str.size();
        write<uint16_t>(str_size);
        writeBytes(reinterpret_cast<uint8_t*>(str.data()), str_size);
    }

    void writeUTF16(const std::wstring& w, const std::size_t max) {
        const uint32_t wstr_size_min = std::min(w.size(), max);
        for (uint32_t i = 0; i < max && i < wstr_size_min; ++i) {
            write<uint16_t>(w[i]);
        }
        // hack, write null char if there is space, and fill rest of space with null as well
        if (wstr_size_min < max) {
            const uint32_t count = max - wstr_size_min;
            for (uint32_t i = 0; i < count; i++) {
                write<uint16_t>(0);
            }
        }
    }

    // Switch/WiiU "WWW" string (char + null char)
    void writeWWWString(const std::wstring& w, const std::size_t max) {
        const uint32_t wstr_size_min = std::min(w.size(), max);
        for (std::size_t i = 0; i < max && i < wstr_size_min; ++i) {
            write<uint16_t>(w[i]);
            write<uint16_t>(0);
        }
        // fill rest of space with null
        if (wstr_size_min < max) {
            for (int i = 0; i < max - wstr_size_min; i++) {
                write<uint32_t>(0); // null char
            }
        }
    }

    // UTF‑16 field from narrow ASCII/UTF‑8 string (old writeWStringFromString)
    void writeWStringFromString(const std::string& s, const std::size_t maxWChars) {
        [[maybe_unused]] constexpr uint8_t zero = 0;
        for (std::size_t i = 0; i < maxWChars && i < s.size(); ++i) {
            if (_end == Endian::Native) {
                write<uint8_t>(static_cast<uint8_t>(s[i]));
                write<uint8_t>(0);
            } else {
                write<uint8_t>(0);
                write<uint8_t>(static_cast<uint8_t>(s[i]));
            }
        }
        // pad remaining slots with two null bytes each
        for (std::size_t i = s.size(); i < maxWChars; ++i) {
            write<uint8_t>(0);
            write<uint8_t>(0);
        }
    }

    // file I/O ------------------------------------------------------

    void save(const fs::path& p) const {
        fs::path path = p;
        path.make_preferred();

        std::ofstream os(path, std::ios::binary);
        if (!os) throw std::runtime_error("create failed " + path.string());
        os.write(reinterpret_cast<const char*>(_buf.get()), static_cast<std::streamsize>(_pos));
        os.close();
    }


    static void writeFile(const std::filesystem::path& p,
                          const std::span<const uint8_t> bytes) {
        fs::path path = p;
        path.make_preferred();
        std::filesystem::create_directories(path.parent_path());

        std::ofstream out(path, std::ios::binary);
        if (!out) throw std::runtime_error("create failed " + path.string());
        out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<uint32_t>(bytes.size()));
        out.close();
    }
};

