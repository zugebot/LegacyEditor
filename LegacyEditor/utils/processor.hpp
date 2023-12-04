#pragma once

#include <cstdarg>
#include <cstdint>
#include <string>
#include <vector>


static inline std::string dir_path;

#define ND [[nodiscard]]
#define MU [[maybe_unused]]

#define EXPECT_FALSE(COND) (__builtin_expect((COND), 0)) // [[unlikely]]
#define EXPECT_TRUE(COND) (__builtin_expect((COND), 1))  // [[likely]]


// unsigned
MU typedef uint8_t u8;
MU typedef uint16_t u16;
MU typedef uint32_t u32;
MU typedef uint64_t u64;
MU typedef std::vector<uint8_t> u8_vec;
MU typedef std::vector<uint16_t> u16_vec;
MU typedef std::vector<uint32_t> u32_vec;
MU typedef std::vector<uint64_t> u64_vec;
MU typedef std::vector<std::vector<uint8_t>> u8_vec_vec;
MU typedef std::vector<std::vector<uint16_t>> u16_vec_vec;
MU typedef std::vector<std::vector<uint32_t>> u32_vec_vec;
MU typedef std::vector<std::vector<uint64_t>> u64_vec_vec;

// signed
MU typedef int8_t i8;
MU typedef int16_t i16;
MU typedef int32_t i32;
MU typedef int64_t i64;
MU typedef std::vector<int8_t> i8_vec;
MU typedef std::vector<int16_t> i16_vec;
MU typedef std::vector<int32_t> i32_vec;
MU typedef std::vector<int64_t> i64_vec;
MU typedef std::vector<std::vector<int8_t>> i8_vec_vec;
MU typedef std::vector<std::vector<int16_t>> i16_vec_vec;
MU typedef std::vector<std::vector<int32_t>> i32_vec_vec;
MU typedef std::vector<std::vector<int64_t>> i64_vec_vec;


static int inline printf_err(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    return -1;
}
