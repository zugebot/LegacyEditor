#pragma once

#include "processor.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <variant>
#include <vector>

// Define types for various NBT tags
using NBTTagInt = int32_t;
using NBTTagLong = int64_t;
using NBTTagFloat = float;
using NBTTagDouble = double;
using NBTTagString = std::string;
using NBTTagByteArray = std::vector<uint8_t>;
using NBTTagIntArray = std::vector<int32_t>;
using NBTTagLongArray = std::vector<int64_t>;

// Define a variant to hold any NBT tag type
using NBTTagVariant = std::variant<std::monostate, // represents no value
                                   NBTTagInt,
                                   NBTTagLong,
                                   NBTTagFloat,
                                   NBTTagDouble,
                                   NBTTagString,
                                   NBTTagByteArray,
                                   NBTTagIntArray,
                                   NBTTagLongArray>;

class NBTBase2 {
private:
    NBTTagVariant data;

public:
    NBTBase2() : data() {}

    explicit NBTBase2(NBTTagInt value) : data(value) {}
    explicit NBTBase2(NBTTagLong value) : data(value) {}
    explicit NBTBase2(NBTTagFloat value) : data(value) {}
    explicit NBTBase2(NBTTagDouble value) : data(value) {}
    explicit NBTBase2(const NBTTagString& value) : data(value) {}
    explicit NBTBase2(const NBTTagByteArray& value) : data(value) {}
    explicit NBTBase2(const NBTTagIntArray& value) : data(value) {}
    explicit NBTBase2(const NBTTagLongArray& value) : data(value) {}

    // Copy and Move Constructors for NBTBase
    NBTBase2(const NBTBase2& other) = default;
    NBTBase2(NBTBase2&& other) noexcept : data(std::move(other.data)) {}

    // Assignment Operators
    NBTBase2& operator=(const NBTBase2& other) {
        if (this != &other) {
            data = other.data;
        }
        return *this;
    }
    NBTBase2& operator=(NBTBase2&& other) noexcept {
        if (this != &other) {
            data = std::move(other.data);
        }
        return *this;
    }

    // Method to return string representation for debugging
    ND std::string toString() const {
        return std::visit([](auto&& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                return "Empty";
            } else if constexpr (std::is_same_v<T, NBTTagString>) {
                return "String: " + arg;
            } else if constexpr (std::is_same_v<T, NBTTagByteArray> ||
                                 std::is_same_v<T, NBTTagIntArray> ||
                                 std::is_same_v<T, NBTTagLongArray>) {
                std::string result = "[";
                for (const auto& val : arg) {
                    if (result.length() > 1) result += ", ";
                    result += std::to_string(val);
                }
                return result + "]";
            } else {
                return "Value: " + std::to_string(arg);
            }
        }, data);
    }


    // ... other methods ...
};

// Example usage
int main() {
    NBTBase2 intTag(123);
    NBTBase2 stringTag(std::string("Hello World"));
    NBTBase2 floatTag(3.14f);

    std::cout << intTag.toString() << std::endl;
    std::cout << stringTag.toString() << std::endl;
    std::cout << floatTag.toString() << std::endl;

    return 0;
}
