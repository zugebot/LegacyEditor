#pragma once

#include <cstdint>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#define ND [[nodiscard]]
#define MU [[maybe_unused]]

#define EXPECT_FALSE(COND) (__builtin_expect((COND), 0))// [[unlikely]]
#define EXPECT_TRUE(COND) (__builtin_expect((COND), 1)) // [[likely]]


