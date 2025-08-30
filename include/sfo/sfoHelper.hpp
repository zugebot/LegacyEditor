#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string_view>
#include <vector>

#include "sfo.hpp"


// ---------- ANSI color config (edit these as you like) ----------
static const std::string COL_EQ_OPEN      = "\x1b[32m";        // equal bytes (e.g., green text) or "\x1b[42m" for green BG
static const std::string COL_DIFF_OPEN    = "\x1b[37;41m";     // different bytes (e.g., white on red BG) or "\x1b[41m"
static const std::string COL_MISS_OPEN    = "\x1b[2m";         // missing byte on one side (dim) — optional
static const std::string COL_RESET        = "\x1b[0m";         // reset

// If you prefer background colors instead of colored letters, example:
// static const std::string COL_EQ_OPEN   = "\x1b[42m";
// static const std::string COL_DIFF_OPEN = "\x1b[41m";
// static const std::string COL_MISS_OPEN = "\x1b[100m";

// ---------- small helpers ----------



// this is used for a single hex variable dump
static std::string hexDumpToString(const std::vector<std::uint8_t>& data) {
    constexpr std::size_t bytes_per_line = 16;
    constexpr std::size_t group_size     = 4;

    std::vector<std::string> lines;

    for (std::size_t i = 0; i < data.size(); i += bytes_per_line)
    {
        std::ostringstream line;
        line << "| ";

        for (std::size_t j = 0; j < bytes_per_line; ++j)
        {
            if (i + j < data.size())
                line << std::uppercase << std::hex
                     << std::setw(2) << std::setfill('0')
                     << static_cast<int>(data[i + j]);
            else
                line << "  ";

            /*  add spacing only when another byte follows  */
            if (j + 1 < bytes_per_line)                      // not the last byte
            {
                line << ((j + 1) % group_size == 0 ? "   "
                                                   : " ");
            }
        }

        line << " |";
        lines.emplace_back(line.str());
    }

    std::size_t width = lines.empty() ? 4 : lines.front().size();
    std::string border = "+" + std::string(width - 2, '-') + "+";

    std::ostringstream out;
    out << border << '\n';
    for (auto const& l : lines) out << l << '\n';
    out << border;

    return out.str();
}



// Visible length of a string ignoring ANSI escape codes (ESC [ ... m)
static std::size_t visibleLen(std::string_view s) {
    std::size_t len = 0;
    bool inEsc = false;
    for (std::size_t i = 0; i < s.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (!inEsc) {
            if (c == 0x1B && i + 1 < s.size() && s[i+1] == '[') { // ESC[
                inEsc = true;
                ++i; // skip '['
            } else {
                ++len;
            }
        } else {
            if (c == 'm') inEsc = false; // end of CSI
        }
    }
    return len;
}

// Pad (with spaces) to the target visible width (ignores ANSI codes).
static std::string padToVisibleWidth(std::string s, std::size_t target) {
    const auto v = visibleLen(s);
    if (v < target) s.append(target - v, ' ');
    return s;
}

// Render one colored hex line for a 16-byte slice, matching your spacing/layout.
static std::string renderHexLineColored(const std::vector<uint8_t>& data,
                                        const std::vector<char>& colorMask, // 'E' equal, 'D' diff, 'M' missing
                                        std::size_t start,
                                        std::size_t bytes_per_line = 16,
                                        std::size_t group_size     = 4) {
    std::ostringstream raw;        // same layout but without ANSI (to measure)
    std::ostringstream colored;    // with ANSI

    raw     << "| ";
    colored << "| ";

    for (std::size_t j = 0; j < bytes_per_line; ++j) {
        const std::size_t idx = start + j;
        if (idx < data.size()) {
            // token without color (for width)
            std::ostringstream tok;
            tok << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(data[idx]);
            const std::string t = tok.str();
            raw << t;

            // choose color
            const char mask = colorMask[idx];
            const std::string& open =
                    (mask == 'E') ? COL_EQ_OPEN :
                    (mask == 'D') ? COL_DIFF_OPEN : COL_MISS_OPEN;

            colored << open << t << COL_RESET;
        } else {
            raw     << "  ";
            colored << "  ";
        }

        if (j + 1 < bytes_per_line) {
            const char* gap = ((j + 1) % group_size == 0) ? "   " : " ";
            raw     << gap;
            colored << gap;
        }
    }

    raw     << " |";
    colored << " |";

    // Ensure the colored line has exactly the same visible width
    const auto need = visibleLen(raw.str());
    return padToVisibleWidth(colored.str(), need);
}

// Build a full colored "box" (top, body, bottom) for given bytes and mask.
static std::vector<std::string>
makeHexBoxLinesColored(const std::vector<uint8_t>& data,
                       const std::vector<char>& colorMask,
                       std::size_t bytes_per_line = 16) {
    // Make one reference raw line (no color) to compute border width
    std::ostringstream ref;
    ref << "| ";
    for (std::size_t j = 0; j < bytes_per_line; ++j) {
        ref << "AA" << ((j + 1) < bytes_per_line ? ((j + 1) % 4 == 0 ? "   " : " ") : "");
    }
    ref << " |";
    const std::size_t lineWidth = visibleLen(ref.str());
    const std::string border = "+" + std::string(lineWidth - 2, '-') + "+";

    std::vector<std::string> box;
    box.emplace_back(border);

    for (std::size_t i = 0; i < data.size(); i += bytes_per_line) {
        box.emplace_back(renderHexLineColored(data, colorMask, i, bytes_per_line));
    }

    box.emplace_back(border);
    return box;
}

// Join two boxes with a configurable gap, aligning visually (accounting for ANSI codes).
static std::string joinSideBySideColored(const std::vector<std::string>& leftBox,
                                         const std::vector<std::string>& rightBox,
                                         std::string_view gap = "   ") {
    const std::size_t leftW  = leftBox.empty()  ? 0 : visibleLen(leftBox.front());
    const std::size_t rightW = rightBox.empty() ? 0 : visibleLen(rightBox.front());
    const std::size_t rows   = std::max(leftBox.size(), rightBox.size());

    std::ostringstream out;
    for (std::size_t i = 0; i < rows; ++i) {
        std::string L = (i < leftBox.size())  ? leftBox[i]  : std::string();
        std::string R = (i < rightBox.size()) ? rightBox[i] : std::string();
        L = padToVisibleWidth(L, leftW);
        R = padToVisibleWidth(R, rightW);
        out << L << gap << R << '\n';
    }
    return out.str();
}

// Attribute -> bytes (tries RAW, then STRING, then INT 32-bit LE)
static std::vector<uint8_t> getAttributeBytes(const SFOManager& m, const std::string& key) {
    if (auto raw = m.getRawAttribute(key))    return *raw;
    if (auto s   = m.getStringAttribute(key)) return {s->begin(), s->end()};
    if (auto iv  = m.getIntAttribute(key)) {
        auto v = static_cast<uint32_t>(*iv);
        std::vector<uint8_t> b(4);
        std::memcpy(b.data(), &v, 4);
        return b;
    }
    return {}; // not found
}

// Public entry: colorized, side-by-side hex of the same-named attribute (default "PARAMS").
static void printAttributeHexSideBySideColored(const SFOManager& left,
                                        const SFOManager& right,
                                        std::string_view attributeKey = "PARAMS",
                                        std::ostream& os = std::cout) {
    const std::string key(attributeKey);

    const auto leftBytes  = getAttributeBytes(left,  key);
    const auto rightBytes = getAttributeBytes(right, key);

    const std::size_t n = std::max(leftBytes.size(), rightBytes.size());
    std::vector<char> leftMask(n, 'M');   // E = equal, D = diff, M = missing
    std::vector<char> rightMask(n, 'M');

    for (std::size_t i = 0; i < n; ++i) {
        const bool L = (i < leftBytes.size());
        const bool R = (i < rightBytes.size());
        if (L && R) {
            const bool eq = (leftBytes[i] == rightBytes[i]);
            leftMask[i]  = eq ? 'E' : 'D';
            rightMask[i] = eq ? 'E' : 'D';
        } else if (L) {
            leftMask[i] = 'D';  // mark as different if only one side has data
        } else if (R) {
            rightMask[i] = 'D';
        }
    }

    // Trim masks to their respective data lengths
    leftMask.resize(leftBytes.size());
    rightMask.resize(rightBytes.size());

    const auto leftBox  = makeHexBoxLinesColored(leftBytes,  leftMask);
    const auto rightBox = makeHexBoxLinesColored(rightBytes, rightMask);

    os << joinSideBySideColored(leftBox, rightBox);
}

