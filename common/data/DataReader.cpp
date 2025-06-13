#include "DataReader.hpp"


i32 DataReader::readInt24() {
    int val;
    if (_end == Endian::Native) {
        val = ((_ptr[2] << 16) | (_ptr[1] << 8) | _ptr[0]);
    } else {
        val = ((_ptr[0] << 16) | (_ptr[1] << 8) | _ptr[2]);
    }
    skip<3>();
    return val;
}


Buffer DataReader::readBuffer(const uint32_t length) {
    Buffer buffer(length);
    std::memcpy(buffer.data(), _ptr, length);
    skip(length);
    return buffer;
}


void DataReader::readBytes(const uint32_t length, uint8_t* dataIn) {
    std::memcpy(dataIn, _ptr, length);
    skip(length);
}


std::span<const uint8_t> DataReader::readSpan(const uint32_t length) {
    const std::span _span = {_ptr, length};
    skip(length);
    return std::move(_span);
}


std::string DataReader::readString(const uint32_t length) {
    const char* start = reinterpret_cast<const char*>(_ptr);
    // Look for a NUL in the next `length` bytes (nullptr if none found).
    const char* nul = static_cast<const char*>(std::memchr(start, 0, length));
    const char* end = nul ? nul : start + length;
    std::string str{start, end};
    skip(length);
    return str;
}


[[maybe_unused]] std::wstring DataReader::readWString(const uint32_t length) {
    std::wstring returnString;
    for (uint32_t i = 0; i < length; i++) {
        if (const auto c = static_cast<wchar_t>(this->read<uint16_t>()); c != 0) {
            returnString += c;
        }
    }
    return returnString;
}


std::string DataReader::readWAsString(const uint32_t length) {
    auto* const letters = new uint8_t[length + 1];
    letters[length] = 0;

    uint32_t iter;
    for (iter = 0; iter < length; iter++) {
        if (_end == Endian::Native) {
            letters[iter] = read<uint8_t>();
            skip<1>();
        } else {
            skip<1>();
            letters[iter] = read<uint8_t>();
        }
        if (constexpr uint8_t empty = 0; letters[iter] == empty) {
            skip(2 * (length - iter - 1));
            break;
        }
    }

    std::string result(reinterpret_cast<char*>(letters), iter);
    delete[] letters;
    return result;
}


[[maybe_unused]] std::string DataReader::readNullTerminatedString() {
    std::string returnString;
    uint8_t nextChar;
    while ((nextChar = read<uint8_t>()) != 0) {
        returnString += static_cast<char>(nextChar);
    }
    return returnString;
}


std::wstring DataReader::readNullTerminatedWString() {
    std::wstring returnString;
    wchar_t nextChar;
    while ((nextChar = read<uint16_t>()) != 0) {
        returnString += nextChar;
    }
    return returnString;
}

/**
 * Only used by nintendo switch edition...
 * @return
 */
// TODO: sizeof(wchar_t) is 2 on Windows (correct impl), but on every other system it's 4?
std::wstring DataReader::readNullTerminatedWWWString() {
    std::wstring returnString;
    while (true) {
        const wchar_t nextChar1 = read<uint16_t>();
        const wchar_t nextChar2 = read<uint16_t>();
        if (nextChar1 == 0 && nextChar2 == 0) {
            break;
        }
        returnString += nextChar1;
    }
    return returnString;
}
