#pragma once

#include <cstdint>
#include <cstdarg>
#include <string>
#include <vector>
#include <ctime>

#ifdef UNIT_TESTS
extern std::string dir_path;
extern std::string out_path;
extern std::string wiiu;
extern std::string ps3_;
#endif

#define ND [[nodiscard]]
#define MU [[maybe_unused]]

#define EXPECT_FALSE(COND) (__builtin_expect((COND), 0)) // [[unlikely]]
#define EXPECT_TRUE(COND) (__builtin_expect((COND), 1))  // [[likely]]

typedef const std::string& stringRef_t;

// unsigned
MU typedef uint8_t u8;
MU typedef uint16_t u16;
MU typedef uint32_t u32;
MU typedef uint64_t u64;
MU typedef std::vector<uint8_t> u8_vec;
MU typedef std::vector<uint16_t> u16_vec;
MU typedef std::vector<uint32_t> u32_vec;
MU typedef std::vector<uint64_t> u64_vec;
MU typedef std::vector<u8_vec> u8_vec_vec;
MU typedef std::vector<u16_vec> u16_vec_vec;
MU typedef std::vector<u32_vec> u32_vec_vec;
MU typedef std::vector<u64_vec> u64_vec_vec;

// signed
MU typedef int8_t i8;
MU typedef int16_t i16;
MU typedef int32_t i32;
MU typedef int64_t i64;
MU typedef std::vector<int8_t> i8_vec;
MU typedef std::vector<int16_t> i16_vec;
MU typedef std::vector<int32_t> i32_vec;
MU typedef std::vector<int64_t> i64_vec;
MU typedef std::vector<i8_vec> i8_vec_vec;
MU typedef std::vector<i16_vec> i16_vec_vec;
MU typedef std::vector<i32_vec> i32_vec_vec;
MU typedef std::vector<i64_vec> i64_vec_vec;


/// printf, but returns -1.
static int printf_err(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    return -1;
}


/// printf, but returns -1.
static int printf_err(const std::string& format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format.c_str(), args);
    va_end(args);
    return -1;
}


/// Function to shuffle an array using Fisher-Yates algorithm
static void shuffleArray(uint16_t arr[], const int size) {
    std::srand(static_cast<unsigned int>(time(nullptr)));
    for (int i = size - 1; i > 0; --i) {
        const int randIndex = std::rand() % (i + 1);
        std::swap(arr[i], arr[randIndex]);
    }
}
