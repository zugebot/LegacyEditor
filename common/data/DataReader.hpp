#pragma once

#include "buffer.hpp"


class DataReader {
    std::span<const uint8_t> _buf{};
    const uint8_t* _ptr = nullptr;
    Endian _end = Endian::Big;

public:
    DataReader() = default;
    explicit DataReader(const std::span<const uint8_t> s, const Endian e = Endian::Big)
        : _buf(s), _ptr(s.data()), _end(e) {}
    DataReader(const uint8_t* p, const std::size_t n, const Endian e = Endian::Big)
        : DataReader(std::span(p, n), e) {}
    explicit DataReader(const Buffer& b, const Endian e = Endian::Big)
        : DataReader(std::span(b.data(), b.size()), e) {}

    void setEndian(const Endian order) { _end = order; }

    // navigation ----------------------------------------------------

    [[nodiscard]] const uint8_t* data() const { return _buf.data(); }
    [[nodiscard]] uint32_t size() const { return _buf.size(); }
    [[nodiscard]] const uint8_t* ptr() const { return _ptr; }
    [[nodiscard]] std::size_t tell() const { return _ptr - _buf.data(); }
    [[nodiscard]] bool eof() const { return tell() >= _buf.size(); }
    void rewind() { _ptr = _buf.data(); }
    [[maybe_unused]] [[nodiscard]] uint8_t peek() const { return _ptr[0]; }

    template<std::integral N>
    void seek(N pos) { _ptr = _buf.data() + pos; }
    template<std::integral N>
    void skip(N n) { _ptr += n; }
    template<std::size_t N>
    void skip() { _ptr += N; }

    template<std::integral T>
    const uint8_t* fetch(T amount) {
        const uint8_t* retPtr = _ptr;
        skip(amount);
        return retPtr;
    }

    template<std::size_t N>
    const uint8_t* fetch() {
        const uint8_t* retPtr = _ptr;
        skip<N>();
        return retPtr;
    }

    template<std::integral T>
    [[maybe_unused]] bool canRead(T amount) const { return tell() + amount <= _buf.size(); }

    // release / expose ---------------------------------------------

    [[nodiscard]] std::span<const uint8_t> span() const { return _buf; }

    // primitive read -----------------------------------------------

    i32 readInt24();

    template<typename T>
        requires(std::integral<T> || std::floating_point<T>)
    T read() {
        if (tell() + sizeof(T) > _buf.size())
             throw std::out_of_range("DataReader::read past end");
        if constexpr (std::is_floating_point_v<T>) {
            using I = std::conditional_t<sizeof(T) == 4, uint32_t, uint64_t>;
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
    T peek_at(const std::size_t offset) const {
        if (offset + sizeof(T) > _buf.size())
            throw std::out_of_range("DataReader::peek_at past end");
        if constexpr (std::is_floating_point_v<T>) {
            using I = std::conditional_t<sizeof(T) == 4, uint32_t, uint64_t>;
            return std::bit_cast<T>(peek_at<I>(offset));
        } else {
            T v;
            std::memcpy(&v, _buf.data() + offset, sizeof(T));
            return detail::maybe_bswap(v, _end, Endian::Native);
        }
    }

    // string helpers ------------------------------------------------

    [[maybe_unused]] std::string readString(uint32_t length);
    [[maybe_unused]] std::wstring readWString(uint32_t length);
    [[maybe_unused]] std::string readWAsString(uint32_t length);

    [[maybe_unused]] std::string readNullTerminatedString();
    [[maybe_unused]] std::wstring readNullTerminatedWString();
    [[maybe_unused]] std::wstring readNullTerminatedWWWString();

    // raw blocks ----------------------------------------------------

    Buffer readBuffer(uint32_t length);
    void readBytes(uint32_t length, uint8_t* dataIn);
    std::span<const uint8_t> readSpan(uint32_t length);

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
