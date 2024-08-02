#include "utils.hpp"

#include <codecvt>
#include <locale>
#include <sstream>


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


std::string wStringToString(const std::wstring& wstr) {
    std::mbstate_t state = std::mbstate_t();
    const wchar_t* src = wstr.c_str();
    std::size_t len = std::wcsrtombs(nullptr, &src, 0, &state);
    if (len == static_cast<std::size_t>(-1)) {
        throw std::runtime_error("Conversion error");
    }

    std::string dest(len, '\0');
    std::wcsrtombs(&dest[0], &src, len, &state);
    return dest;
}