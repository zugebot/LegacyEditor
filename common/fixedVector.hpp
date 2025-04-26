#pragma once

#include <array>

template <typename T, std::size_t N>
class FixedVector {
    std::array<T, N> myData;
    std::size_t mySize = 0;

public:
    void push_back(const T& value) {
        myData[mySize++] = value;
    }

    template <typename... Args>
    void emplace_back(Args&&... args) {
        new (&myData[mySize]) T(std::forward<Args>(args)...);
        ++mySize;
    }

    T* begin() { return &myData[0]; }
    T* end() { return &myData[0] + mySize; }
    const T* begin() const { return &myData[0]; }
    const T* end() const { return &myData[0] + mySize; }

    T* data() {
        return &myData[0];
    }

    const T* data() const {
        return &myData[0];
    }

    T& operator[](std::size_t index) { return myData[index]; }
    const T& operator[](std::size_t index) const { return myData[index]; }

    void set_size(const std::size_t sizeIn) {
        if (sizeIn <= N) {
            mySize = sizeIn;
        }
    }

    [[nodiscard]] std::size_t current_size() const { return mySize; }
    [[nodiscard]] static constexpr std::size_t capacity() { return N; }
};
