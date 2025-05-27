#pragma once

#include <bit>
#include <concepts>

#include "include/lce/processor.hpp"
#include "include/ghc/fs_std.hpp"

#include "utils.hpp"
#include "buffer.hpp"


class DataReader {
    std::span<c_u8> _buf{};
    c_u8* _ptr = nullptr;
    Endian _end = Endian::Big;

public:
    DataReader() = default;
    explicit DataReader(std::span<c_u8> s, Endian e = Endian::Big)
        : _buf(s), _ptr(s.data()), _end(e) {}
    DataReader(c_u8* p, std::size_t n, Endian e = Endian::Big)
        : DataReader(std::span<c_u8>(p, n), e) {}
    DataReader(const Buffer& b, Endian e = Endian::Big)
        : DataReader(std::span<c_u8>(b.data(), b.size()), e) {}

    void setEndian(Endian order) { _end = order; }

    // navigation ----------------------------------------------------

    ND c_u8* data() const { return _buf.data(); }
    ND u32 size() const { return _buf.size(); }
    ND c_u8* ptr() const { return _ptr; }
    ND std::size_t tell() const { return _ptr - _buf.data(); }
    ND bool eof() const { return tell() >= _buf.size(); }
    void rewind() { _ptr = _buf.data(); }
    MU ND u8 peek() const { return _ptr[0]; }

    template<std::integral N>
    void seek(N pos) { _ptr = _buf.data() + pos; }
    template<std::integral N>
    void skip(N n) { _ptr += n; }
    template<std::size_t N>
    void skip() { _ptr += N; }

    template<std::integral T>
    c_u8* fetch(T amount) {
        c_u8* retPtr = _ptr;
        skip(amount);
        return retPtr;
    }

    template<std::size_t N>
    c_u8* fetch() {
        c_u8* retPtr = _ptr;
        skip<N>();
        return retPtr;
    }

    template<std::integral T>
    MU bool canRead(T amount) const { return tell() + amount <= _buf.size(); }

    // release / expose ---------------------------------------------

    ND std::span<c_u8> span() const { return _buf; }

    // primitive read -----------------------------------------------

    i32 readInt24();

    template<typename T>
        requires(std::integral<T> || std::floating_point<T>)
    T read() {
        if (tell() + sizeof(T) > _buf.size())
            throw std::out_of_range("DataReader::read past end");
        if constexpr (std::is_floating_point_v<T>) {
            using I = std::conditional_t<sizeof(T) == 4, u32, u64>;
            return std::bit_cast<T>(read<I>());
        } else {
            T v;
            std::memcpy(&v, _ptr, sizeof(T));
            skip<sizeof(T)>();
            return detail::maybe_bswap(v, _end, Endian::Native);
        }
    }

    template<typename T>
        requires(std::integral<T> || std::floating_point<T>)
    T peek_at(std::size_t offset) const {
        if (offset + sizeof(T) > _buf.size())
            throw std::out_of_range("DataReader::peek_at past end");
        if constexpr (std::is_floating_point_v<T>) {
            using I = std::conditional_t<sizeof(T) == 4, u32, u64>;
            return std::bit_cast<T>(peek_at<I>(offset));
        } else {
            T v;
            std::memcpy(&v, _buf.data() + offset, sizeof(T));
            return detail::maybe_bswap(v, _end, Endian::Native);
        }
    }

    // string helpers ------------------------------------------------

    std::string readString(u32 length);
    MU std::wstring readWString(u32 length);
    std::string readWAsString(u32 length);

    MU std::string readNullTerminatedString();
    std::wstring readNullTerminatedWString();
    std::wstring readNullTerminatedWWWString();

    // raw blocks ----------------------------------------------------

    Buffer readBuffer(u32 length);
    void readBytes(u32 length, u8* dataIn);
    std::span<const u8> readSpan(u32 length);

    // file I/O ------------------------------------------------------

    static Buffer readFile(const fs::path& p) {
        std::ifstream in(p, std::ios::binary | std::ios::ate);
        if (!in) throw std::runtime_error("open failed " + p.string());
        Buffer buf(in.tellg());
        in.seekg(0);
        in.read(reinterpret_cast<char*>(buf.data()), buf.size());
        return buf;
    }
};
