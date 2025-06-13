#pragma once

#include <chrono>
#include <iomanip>
#include <sstream>

#include "lce/processor.hpp"


std::vector<std::string> split(const std::string &s, char delimiter);

std::wstring stringToWstring(const std::string& str);

std::string wStringToString(const std::wstring& wstr);


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










