// common/fmt.hpp
#pragma once

#include <format>
#include <string>
#include <string_view>
#include <array>
#include <sstream>
#include <charconv>
#include <type_traits>
#include <utility>
#include <iostream>

#include "include/lce/processor.hpp"



namespace cmn {

    enum class eLog { info, success, warning, error, time, input, output, detail };

    // map enum -> glyph
    [[nodiscard]] constexpr std::string_view to_symbol(eLog t) noexcept {
        using enum eLog;
        switch (t) {
            case info   : return "*";
            case success: return "+";
            case warning: return "!";
            case error  : return "X";
            case time   : return "T";
            case input  : return "→";
            case output : return "←";
            case detail : return "#";
        }
        return "?";
    }

    // ── helper: stringify any single value via string-stream ───────────────
    template<class T>
    [[nodiscard]] std::string to_string_any(const T& value)
    {
        if constexpr (std::is_same_v<std::decay_t<T>, std::string>)
            return value;
        else if constexpr (std::is_same_v<std::decay_t<T>, const char*>)
            return std::string(value);
        else if constexpr (std::is_convertible_v<T, std::string>)
            return std::string(value);
        else {
            std::ostringstream oss;
            oss << value;
            return oss.str();
        }
    }

    // ── core formatter (replace sequential `{}` tokens) ────────────────────
    template<class... Ts>
    [[nodiscard]] std::string
    format_text(std::string_view fmt, Ts&&... args)
    {
        constexpr std::size_t N = sizeof...(Ts);
        std::array<std::string, N> pieces{ to_string_any(std::forward<Ts>(args))... };

        std::string out;
        out.reserve(fmt.size() + (pieces.size() * 8));          // cheap guesstimate

        std::size_t pos = 0, arg = 0;
        while (true) {
            std::size_t brace = fmt.find("{}", pos);
            if (brace == std::string_view::npos) break;

            out.append(fmt.substr(pos, brace - pos));           // copy text before {}
            if (arg < pieces.size())
                out.append(pieces[arg++]);                      // insert next arg
            else
                out.append("{}");                               // not enough args

            pos = brace + 2;
        }
        out.append(fmt.substr(pos));                            // tail
        return out;
    }

    // ── user-facing helpers – with / without explicit category ─────────────
    template<class... Ts>
    [[nodiscard]] std::string fmt(eLog cat, std::string_view f, Ts&&... args)
    {
        std::string result;
        result.reserve(f.size() + 4);
        result += '[';
        result += to_symbol(cat);
        result += "] ";
        result += format_text(f, std::forward<Ts>(args)...);
        return result;
    }

    template<class... Ts>
    [[nodiscard]] std::string fmt(std::string_view f, Ts&&... args)
    {
        return fmt(eLog::info, f, std::forward<Ts>(args)...);
    }


    template<class... Ts>
    void log(eLog cat, std::string_view fmtstr, Ts&&... args)
    {
        auto msg = fmt(cat, fmtstr, std::forward<Ts>(args)...);
        if (cat == eLog::error)   std::cerr << msg;  // route errors to stderr
        else                      std::cout << msg;  // everything else to stdout
    }
    template<class... Ts>
    void log(std::string_view fmtstr, Ts&&... args)
    {
        log(eLog::info, fmtstr, std::forward<Ts>(args)...);
    }
} // namespace cmn
// ───────────────────────────────────────────────────────────────────────────