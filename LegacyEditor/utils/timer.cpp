#include "timer.hpp"

#include <chrono>


[[maybe_unused]] uint64_t getNanoSeconds() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}


[[maybe_unused]] uint64_t getMilliseconds() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}


[[maybe_unused]] uint64_t getSeconds() {
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}


Timer::Timer() {
    time = getNanoSeconds();
}


[[maybe_unused]] float Timer::getSeconds() const {
    static constexpr int NANO_TO_SEC = 1000000000;
    const uint64_t end = getNanoSeconds();
    return static_cast<float>(end - time) / static_cast<float>(NANO_TO_SEC);
}