#pragma once

#include "include/lce/processor.hpp"
#if defined(__AVX2__)
#include <immintrin.h>
#endif




namespace editor::chunk {

    /**
    * This checks if the next 1024 bits are all zeros.\n
    * this is u8[128]
    * @param ptr
    * @return true if all bits are zero, else 0.
    */
    static bool is_zero_128_slow(c_u8* ptr) {
        for (int i = 0; i < 128; ++i) {
            if (ptr[i] != 0x00) {
                return false;
            }
        }
        return true;
    }


    static bool is_ff_128_slow(c_u8* ptr) {
        for (int i = 0; i < 128; ++i) {
            if (ptr[i] != 0xFF) {
                return false;
            }
        }
        return true;
    }


#if defined(__AVX2__)
#define HAS_AVX2 1
    static inline bool _chunk_has_byte_NEQ(const __m256i& v,
                                          __m256i pattern) noexcept {
        // v == pattern  → 0xFF bytes (cmp-true = 0xFF)
        // v != pattern  → 0x00 somewhere → mask ≠ −1
        __m256i cmp = _mm256_cmpeq_epi8(v, pattern);
        return !_mm256_testc_si256(cmp, _mm256_set1_epi8(-1));
    }
#else
#define HAS_AVX2 0
#endif

    /// Fast path for AVX2 (4 × 32 B = 128 B, unaligned OK)
    template<u8 V>
    static inline bool _is_all_val_128_avx2(const u8* p) noexcept {
#if HAS_AVX2
        __m256i pattern = _mm256_set1_epi8(V);

        for (int i = 0; i < 4; ++i) {
            __m256i v = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(p + i * 32));
            if (_chunk_has_byte_NEQ(v, pattern)) return false;
        }
        return true;
#else
        (void) p; // suppress “unused” when AVX2 absent
        return false;
#endif
    }

    /// Portable scalar fallback (8 B at a time, no branches in loop)
    template<u8 V>
    static inline bool _is_all_val_128_scalar(const u8* p) noexcept {
        const std::uint64_t patt64 = 0x0101010101010101ULL * V;

        for (int i = 0; i < 16; ++i) { // 16 × 8 B = 128 B
            if (reinterpret_cast<const std::uint64_t*>(p)[i] ^ patt64)
                return false;
        }
        return true;
    }

    /// public wrapper
    /// checks if the next 128 bytes are all "0x00"
    static inline bool is_zero_128(const u8* p) noexcept {
#if HAS_AVX2
        return _is_all_val_128_avx2<0x00>(p);
#else
        return _is_all_val_128_scalar<0x00>(p);
#endif
    }

    /// public wrapper
    /// checks if the next 128 bytes are all "0xFF"
    static inline bool is_ff_128(const u8* p) noexcept {
#if HAS_AVX2
        return _is_all_val_128_avx2<0xFF>(p);
#else
        return _is_all_val_128_scalar<0xFF>(p);
#endif
    }
}