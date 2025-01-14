#pragma once

#include <chrono>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>


std::vector<std::string> split(const std::string &s, char delimiter);

std::wstring stringToWstring(const std::string& str);

std::string wStringToString(const std::wstring& wstr);


static bool isSystemLittleEndian() {
    static constexpr int num = 1;
    static const bool isLittle = *reinterpret_cast<const char*>(&num) == 1;
    return isLittle;
}

static uint16_t swapEndian16(const uint16_t value) {
    return value << 8 | value >> 8;
}

static uint32_t swapEndian32(const uint32_t value) {
    return (value & 0xFF000000U) >> 24 |
           (value & 0x00FF0000U) >>  8 |
           (value & 0x0000FF00U) <<  8 |
           (value & 0x000000FFU) << 24;
}

// FIXME: this supposedly does not work?
static uint64_t swapEndian64(uint64_t value) {
    value = (value & 0x00000000FFFFFFFFLL) << 32 | (value & 0xFFFFFFFF00000000LL) >> 32;
    value = (value & 0x0000FFFF0000FFFFLL) << 16 | (value & 0xFFFF0000FFFF0000LL) >> 16;
    value = (value & 0x00FF00FF00FF00FFLL) <<  8 | (value & 0xFF00FF00FF00FF00LL) >>  8;
    return value;
}


static int16_t extractMapNumber(const std::string& str) {
    static const std::string start = "map_";
    static const std::string end = ".dat";
    size_t startPos = str.find(start);
    const size_t endPos = str.find(end);

    if (startPos != std::string::npos && endPos != std::string::npos) {
        startPos += start.length();

        const std::string numberStr = str.substr(startPos, endPos - startPos);
        return static_cast<int16_t>(std::stoi(numberStr));
    }
    return 32767;
}


static std::pair<int, int> extractRegionCoords(const std::string& filename) {
    const size_t lastDot = filename.find_last_of('.');
    const std::string relevantPart = filename.substr(0, lastDot);

    std::istringstream iss(relevantPart);
    std::string part;
    std::vector<std::string> parts;

    while (std::getline(iss, part, '.')) {
        parts.push_back(part);
    }

    int num1 = std::stoi(parts[parts.size() - 2]);
    int num2 = std::stoi(parts[parts.size() - 1]);
    return {num1, num2};
}


/**
 * Gets the current UTC time and creates a string from it.
 * This is used by LCE for filenames and folders for saves.
 * @return the UTC time string.
 */
[[maybe_unused]] static std::string getCurrentDateTimeString() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* utc_tm = std::gmtime(&now_c);

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << ((utc_tm->tm_year + 1900) % 100);
    oss << std::setfill('0') << std::setw(2) << utc_tm->tm_mon + 1;
    oss << std::setfill('0') << std::setw(2) << utc_tm->tm_mday;
    oss << std::setfill('0') << std::setw(2) << utc_tm->tm_hour;
    oss << std::setfill('0') << std::setw(2) << utc_tm->tm_min;
    oss << std::setfill('0') << std::setw(2) << utc_tm->tm_sec;
    return oss.str();
}










