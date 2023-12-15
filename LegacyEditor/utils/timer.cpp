#include "timer.hpp"

#include <chrono>


u64 getNanoSeconds() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}


inline u64 getMilliseconds() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}


inline u64 getSeconds() {
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

Timer::Timer() {
    time = getNanoSeconds();
}

float Timer::getSeconds() const {
    const u64 end = getNanoSeconds();
    return static_cast<float>(end - time) / static_cast<float>(1000000000);
}