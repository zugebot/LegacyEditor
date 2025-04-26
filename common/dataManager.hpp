#pragma once

#include <filesystem>
#include <string>

#include "include/lce/processor.hpp"

#include "include/ghc/fs_std.hpp"

#include "common/data.hpp"


namespace editor {
    class LCEFile;
}


/// Starts writing in "Big Endian".
class DataManager {

    mutable Endian m_order = Endian::Big;
    u8 *m_data = nullptr, *m_ptr = nullptr;
    u32 m_size = 0;
public:

    DataManager() = default;

    explicit DataManager(const Data& dataIn, c_bool isBigIn = true)
        : m_data(dataIn.start()), m_ptr(m_data), m_size(dataIn.size) {
        setEndian(isBigIn);
    }

    explicit DataManager(const Data* dataIn, c_bool isBigIn = true)
        : m_data(dataIn->start()), m_ptr(m_data), m_size(dataIn->size) {
        setEndian(isBigIn);
    }

    explicit DataManager(u8* dataIn, c_u32 sizeIn, c_bool isBigIn = true)
        : m_data(dataIn), m_ptr(dataIn), m_size(sizeIn) {
        setEndian(isBigIn);
    }

    void setEndian(bool isBigIn) {
        if (isBigIn) {
            m_order = Endian::Big;
        } else {
            m_order = Endian::Little;
        }
    }

    void take(const Data& dataIn);
    void take(const Data* dataIn);

    void update(u8* dataIn, u32 sizeIn) {
        m_data = dataIn;
        m_ptr = dataIn;
        m_size = sizeIn;
    }

    ND u8* start() const { return m_data; }
    ND u8* ptr() const { return m_ptr; }
    template<std::integral T>
    void seek(T position) { m_ptr = m_data + position; }
    u32 size() const { return m_size; }
    void rewind() { m_ptr = m_data; }
    template<int amount>
    void skip() { m_ptr += amount; }
    template<std::integral T>
    void skip(T amount) { m_ptr += amount; }
    u32 tell() const { return m_ptr - m_data; }
    bool eof() const { return m_ptr >= m_data + m_size - 1; }
    template<std::integral T>
    MU bool canRead(T amount) const { return tell() + amount <= m_size; }
    MU u8 peek() const { return m_ptr[0]; }

    /// FILE STUFF

    int readFromFile(const std::string& fileStrIn);
    int writeToFile(const fs::path& inFilePath) const;

    /// generic read

    template<typename T>
        requires (std::integral<T> || std::floating_point<T>)
    T read() {
        static_assert(std::is_trivially_copyable_v<T>);
        // floating point
        if constexpr (std::is_same_v<T, float>) {
            return std::bit_cast<float>(read<u32>());
        } else if constexpr (std::is_same_v<T, double>) {
            return std::bit_cast<double>(read<u64>());
        } else { // remaining integral types
            T val;
            memcpy(&val, m_ptr, sizeof(T));
            skip<sizeof(T)>();
            if constexpr (sizeof(T) == 1) {
                return val;
            } else {
                return m_order == Endian::Native ? val : std::byteswap(val);
            }
        }
    }

    template<typename T>
        requires (std::integral<T> || std::floating_point<T>)
    T readAtOffset(u32 offset) const {
        static_assert(std::is_trivially_copyable_v<T>);
        // floating point
        if constexpr (std::is_same_v<T, float>) {
            return std::bit_cast<float>(readAtOffset<u32>(offset));
        } else if constexpr (std::is_same_v<T, double>) {
            return std::bit_cast<double>(readAtOffset<u64>(offset));
        } else { // remaining integral types
            T val;
            memcpy(&val, m_data + offset, sizeof(T));
            if constexpr (sizeof(T) == 1) {
                return val;
            } else {
                return m_order == Endian::Native ? val : std::byteswap(val);
            }
        }
    }

    /// generic write

    template<typename T>
        requires (std::integral<T> || std::floating_point<T>)
    void write(T val) {
        static_assert(std::is_trivially_copyable_v<T>);
        // floating point
        if constexpr (std::is_same_v<T, float>) {
            write<u32>(std::bit_cast<u32>(val));
        } else if constexpr (std::is_same_v<T, double>) {
            write<u64>(std::bit_cast<u64>(val));
        // remaining integral types
        } else if constexpr (sizeof(T) == 1) {
            *m_ptr = static_cast<u8>(val);
            skip<sizeof(T)>();
        } else {
            if (m_order != Endian::Native)
                val = std::byteswap(val);
            memcpy(m_ptr, &val, sizeof(T));
            skip<sizeof(T)>();
        }
    }

    /// writes at offset from .data, not .ptr! Does not skip .ptr.
    template<typename T>
        requires (std::integral<T> || std::floating_point<T>)
    void writeAtOffset(u32 offset, T val) const {
        static_assert(std::is_trivially_copyable_v<T>);
        // floating point
        if constexpr (std::is_same_v<T, float>) {
            writeAtOffset<u32>(offset, std::bit_cast<u32>(val));
        } else if constexpr (std::is_same_v<T, double>) {
            writeAtOffset<u64>(offset, std::bit_cast<u64>(val));
        // remaining integral types
        } else if constexpr (sizeof(T) == 1) {
            m_data[offset] = static_cast<u8>(val);
        } else {
            if (m_order != Endian::Native)
                val = std::byteswap(val);
            memcpy(m_data + offset, &val, sizeof(T));
        }
    }

    // dumb xbox

    i32 readInt24();
    MU void writeInt24(u32 intIn);

    // READING SECTION

    std::string readString(u32 length);
    MU std::wstring readWString(u32 length);
    std::string readWAsString(u32 length);

    MU std::string readNullTerminatedString();
    std::wstring readNullTerminatedWString();
    std::wstring readNullTerminatedWWWString();

    u8_vec readIntoVector(u32 amount);
    u8* readBytes(u32 length);
    void readBytes(u32 length, u8* dataIn);

    // WRITING SECTION

    void writeBytes(c_u8* dataPtrIn, u32 length);

    /// note that upperbounds is per 2 bytes, so use 64 for 128 bytes.
    void writeString(std::string str);
    void writeWString(const std::wstring& wstr, u32 upperbounds);
    void writeWWWString(const std::wstring& wstr, u32 upperbounds);
    void writeWStringFromString(const std::string& str, u32 upperbounds);

};
