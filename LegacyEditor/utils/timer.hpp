#pragma once

#include "lce/processor.hpp"


MU static u64 getNanoSeconds();
MU static u64 getMilliseconds();
MU static u64 getSeconds();


class Timer {
    u64 time = 0;
public:
    Timer();
    MU ND float getSeconds() const;
};