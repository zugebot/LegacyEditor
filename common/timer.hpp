#pragma once

#include <cstdint>


[[maybe_unused]] static uint64_t getNanoSeconds();
[[maybe_unused]] static uint64_t getMilliseconds();
[[maybe_unused]] static uint64_t getSeconds();


class [[maybe_unused]] Timer {
    uint64_t time = 0;
public:
    Timer();
    [[maybe_unused]] [[nodiscard]] float getSeconds() const;
};