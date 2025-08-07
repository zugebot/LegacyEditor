#pragma once

#include <bit>
#include <concepts>
#include <cstdint>
#include <span>

#include "common/data/ghc/fs_std.hpp"


namespace detail {
    template<std::integral T>
    constexpr T maybe_bswap(T v, const Endian from, const Endian to) {
        if constexpr (sizeof(T) == 1) return v;
        return from == to ? v : std::byteswap(v);
    }

    template<std::integral T>
    constexpr T maybe_bswap(T v, const Endian to) {
        if constexpr (sizeof(T) == 1) return v;
        return Endian::Native == to ? v : std::byteswap(v);
    }

}


template <typename T = uint8_t>
class BufferBase {
    std::unique_ptr<T[]> m_data;
    std::uint32_t m_size = 0;

public:
    using value_type = std::byte;

    // ───────── construction ─────────
    BufferBase() = default;
    [[maybe_unused]] explicit BufferBase(const uint32_t n) { allocate(n); }
    [[maybe_unused]] BufferBase(std::unique_ptr<T[]> data, const uint32_t size)
        : m_data(std::move(data)), m_size(size) {}


    // ───────── moving ─────────
    BufferBase(BufferBase&& other) noexcept
        : m_data(std::move(other.m_data)), m_size(other.m_size) {
        other.m_size = 0;
    }

    BufferBase& operator=(BufferBase&& other) noexcept {
        if (this != &other) {
            m_data = std::move(other.m_data);
            m_size = other.m_size;
            other.m_size = 0;
        }
        return *this;
    }

    // ───────── no copying ─────────
    BufferBase(const BufferBase&) = delete;
    BufferBase& operator=(const BufferBase&) = delete;

    // ───────── basic API ─────────
    [[nodiscard]] uint32_t size() const { return m_size; }
    [[nodiscard]] uint32_t* size_ptr() { return &m_size; }
    [[nodiscard]] uint32_t& size_ref() { return m_size; }
    [[nodiscard]] bool empty() const { return m_size == 0; }

    [[nodiscard]] T* data() { return m_data.get(); }
    [[nodiscard]] const T* data() const { return m_data.get(); }

    [[maybe_unused]] [[nodiscard]] std::span<T>
    span() { return {data(), m_size}; }
    [[maybe_unused]] [[nodiscard]] std::span<const T>
    span() const { return {data(), m_size}; }

    void clear() {
        m_data.reset();
        m_size = 0;
    }

    bool allocate(const std::size_t n) {
        auto p = std::make_unique<T[]>(n);
        if (!p) return false;
        m_data = std::move(p);
        m_size = n;
        return true;
    }

};


using Buffer = BufferBase<>;