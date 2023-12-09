#pragma once

#include <cstdarg>
#include <cstdint>
#include <functional>
#include <string>
#include <thread>
#include <vector>


static inline std::string dir_path;
static inline std::string tst_path;
static inline std::string out_path;

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


/**
 * \n
 * It is a requirement that the first argument passed to the
 * function is an index based on the maximum thread count.
 * \n
 * \n
 * use 'std::ref' for references.
 * @tparam threadCount how many threads to create
 * @tparam Function
 * @tparam Args
 * @param func the function to call
 * @param args the arguments to pass to the function
 * @return
 */
template<int threadCount, typename Function, typename... Args>
static int inline run_parallel(Function func, Args... args) {
    std::vector<std::thread> threads;
    for (int i = 0; i < threadCount; ++i) {
        threads.emplace_back(std::bind(func, i, args...));
    }
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    return 0;
}


/// printf, but returns -1.
static int inline printf_err(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    return -1;
}


/// Function to shuffle an array using Fisher-Yates algorithm
void shuffleArray(uint16_t arr[], int size) {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    for (int i = size - 1; i > 0; --i) {
        int j = std::rand() % (i + 1);
        std::swap(arr[i], arr[j]);
    }
}
