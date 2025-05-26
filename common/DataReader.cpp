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


Buffer DataReader::readBuffer(c_u32 length) {
    Buffer buffer(length);
    std::memcpy(buffer.data(), _ptr, length);
    skip(length);
    return buffer;
}


void DataReader::readBytes(c_u32 length, u8* dataIn) {
    std::memcpy(dataIn, _ptr, length);
    skip(length);
}


std::string DataReader::readString(c_u32 length) {
    const char* start = reinterpret_cast<const char*>(_ptr);
    // Look for a NUL in the next `length` bytes (nullptr if none found).
    const char* nul = static_cast<const char*>(std::memchr(start, 0, length));
    const char* end = nul ? nul : start + length;
    std::string str{start, end};
    skip(length);
    return str;
}


MU std::wstring DataReader::readWString(c_u32 length) {
    std::wstring returnString;
    for (u32 i = 0; i < length; i++) {
        if (c_auto c = static_cast<wchar_t>(this->read<u16>()); c != 0) {
            returnString += c;
        }
    }
    return returnString;
}


std::string DataReader::readWAsString(c_u32 length) {
    auto* const letters = new u8[length + 1];
    letters[length] = 0;

    u32 iter;
    for (iter = 0; iter < length; iter++) {
        if (_end == Endian::Native) {
            letters[iter] = read<u8>();
            skip<1>();
        } else {
            skip<1>();
            letters[iter] = read<u8>();
        }
        if (constexpr u8 empty = 0; letters[iter] == empty) {
            skip(2 * (length - iter - 1));
            break;
        }
    }

    std::string result(reinterpret_cast<char*>(letters), iter);
    delete[] letters;
    return result;
}


MU std::string DataReader::readNullTerminatedString() {
    std::string returnString;
    u8 nextChar;
    while ((nextChar = read<u8>()) != 0) {
        returnString += static_cast<char>(nextChar);
    }
    return returnString;
}


std::wstring DataReader::readNullTerminatedWString() {
    std::wstring returnString;
    wchar_t nextChar;
    while ((nextChar = read<u16>()) != 0) {
        returnString += nextChar;
    }
    return returnString;
}


/**
 * Only used by nintendo switch edition...
 * @return
 */
std::wstring DataReader::readNullTerminatedWWWString() {
    std::wstring returnString;
    wchar_t nextChar1;
    wchar_t nextChar2;
    while (true) {
        nextChar1 = read<u16>();
        nextChar2 = read<u16>();
        if (nextChar1 == 0 && nextChar2 == 0) {
            break;
        }
        returnString += nextChar1;
    }
    return returnString;
}
