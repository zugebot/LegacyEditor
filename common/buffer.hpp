// BufferBase.hpp ────────────────────────────────────────────────
#pragma once

#include "include/lce/processor.hpp"


template <typename T = u8>
class BufferBase {
    std::unique_ptr<T[]> m_data;
    std::u32 m_size = 0;

public:
    using value_type = std::byte;

    // ───────── construction ─────────
    BufferBase() = default;
    MU explicit BufferBase(u32 n) { allocate(n); }
    MU BufferBase(std::unique_ptr<T[]> data, u32 size)
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
    ND u32 size() const { return m_size; }
    ND u32* size_ptr() { return &m_size; }
    ND u32& size_ref() { return m_size; }
    ND bool empty() const { return m_size == 0; }

    ND T* data() { return m_data.get(); }
    ND const T* data() const { return m_data.get(); }

    MU ND std::span<T> span() { return {data(), m_size}; }
    MU ND std::span<const T> span() const { return {data(), m_size}; }

    void clear() {
        m_data.reset();
        m_size = 0;
    }

    bool allocate(std::size_t n) {
        auto p = std::make_unique<T[]>(n);
        if (!p) return false;
        m_data = std::move(p);
        m_size = n;
        return true;
    }

};


using Buffer = BufferBase<u8>;