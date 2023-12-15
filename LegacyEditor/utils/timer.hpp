#pragma once


#include "LegacyEditor/utils/processor.hpp"


MU static u64 getNanoSeconds();
MU static u64 getMilliseconds();
MU static u64 getSeconds();


class Timer {
public:
    u64 time = 0;

    Timer();
    MU ND float getSeconds() const;

};