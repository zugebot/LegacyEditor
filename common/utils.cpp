#include "utils.hpp"

#include <codecvt>
#include <locale>
#include <sstream>
#include <string>
#include <stdexcept>

#if defined(_WIN32)
#include <windows.h>
#endif



#if defined(_WIN32)
  #include <windows.h>
#endif


std::vector<std::string> split(const std::string &s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


std::wstring stringToWstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}


static inline void append_utf8(std::string& out, uint32_t cp) {
    if (cp <= 0x7F) {
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0x10FFFF) {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        throw std::runtime_error("Invalid Unicode code point");
    }
}

std::string wStringToString(const std::wstring& wstr) {
#if defined(_WIN32)
    // Windows wstring is UTF-16
    if (wstr.empty()) return {};
    const int needed = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    if (needed <= 0) throw std::runtime_error("WideCharToMultiByte size failed");
    std::string out(needed, '\0');
    const int written = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), out.data(), needed, nullptr, nullptr);
    if (written <= 0) throw std::runtime_error("WideCharToMultiByte convert failed");
    return out;
#else
    // Unix-y: wstring is usually UTF-32 (wchar_t holds code points)
    std::string out;
    out.reserve(wstr.size()); // rough
    for (wchar_t wc : wstr) {
        uint32_t cp = static_cast<uint32_t>(wc);
        append_utf8(out, cp);
    }
    return out;
#endif
}
