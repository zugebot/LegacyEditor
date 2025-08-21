#pragma once

#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#ifdef INCLUDE_QT
    #include <QDebug>
#endif

#include "include/lce/processor.hpp"

/**
 * @class Pos2DTemplate
 * @brief A template class representing a 2D position with x and z coordinates.
 * @tparam classType The type of the coordinates (e.g., int, double).
 */
template<class classType>
class Pos2DTemplate {
public:
    classType x; ///< The x-coordinate.
    classType z; ///< The z-coordinate.

    /**
     * @brief Default constructor initializing x and z to 0.
     */
    Pos2DTemplate() : x(0), z(0) {
    }

    /**
     * @brief Constructor initializing x and z with given values.
     * @param xIn The x-coordinate.
     * @param zIn The z-coordinate.
     */
    Pos2DTemplate(classType xIn, classType zIn) : x(xIn), z(zIn) {
    }

    Pos2DTemplate(Pos2DTemplate &&) = default; ///< Move constructor.
    Pos2DTemplate(const Pos2DTemplate &) = default; ///< Copy constructor.
    Pos2DTemplate &operator=(const Pos2DTemplate &) = default; ///< Copy assignment operator.

    /**
     * @brief Equality operator to compare two Pos2DTemplate objects.
     * @param other The other Pos2DTemplate object.
     * @return True if equal, false otherwise.
     */
    bool operator==(const Pos2DTemplate &other) const;

    /**
     * @brief Equality operator to compare with an integer.
     * @param other The integer value.
     * @return True if equal, false otherwise.
     */
    bool operator==(int other) const;

    /**
     * @brief Inequality operator to compare two Pos2DTemplate objects.
     * @param other The other Pos2DTemplate object.
     * @return True if not equal, false otherwise.
     */
    bool operator!=(const Pos2DTemplate &other) const;

    /**
     * @brief Inequality operator to compare with an integer.
     * @param other The integer value.
     * @return True if not equal, false otherwise.
     */
    bool operator!=(int other) const;

    /**
     * @brief Addition operator to add two Pos2DTemplate objects.
     * @param other The other Pos2DTemplate object.
     * @return A new Pos2DTemplate object with the summed coordinates.
     */
    Pos2DTemplate operator+(const Pos2DTemplate &other) const;

    /**
     * @brief Addition operator to add an integer to both coordinates.
     * @param other The integer value.
     * @return A new Pos2DTemplate object with the summed coordinates.
     */
    Pos2DTemplate operator+(int other) const;

    /**
     * @brief Multiplication operator to multiply two Pos2DTemplate objects.
     * @param other The other Pos2DTemplate object.
     * @return A new Pos2DTemplate object with the multiplied coordinates.
     */
    Pos2DTemplate operator*(const Pos2DTemplate &other) const;

    /**
     * @brief Multiplication operator to multiply both coordinates by an integer.
     * @param other The integer value.
     * @return A new Pos2DTemplate object with the multiplied coordinates.
     */
    Pos2DTemplate operator*(int other) const;

    /**
     * @brief Division operator to divide both coordinates by an integer.
     * @param other The integer value.
     * @return A new Pos2DTemplate object with the divided coordinates.
     */
    Pos2DTemplate operator/(int other) const;

    /**
     * @brief Subtraction operator to subtract two Pos2DTemplate objects.
     * @param other The other Pos2DTemplate object.
     * @return A new Pos2DTemplate object with the subtracted coordinates.
     */
    Pos2DTemplate operator-(const Pos2DTemplate &other) const;

    /**
     * @brief Subtraction operator to subtract an integer from both coordinates.
     * @param other The integer value.
     * @return A new Pos2DTemplate object with the subtracted coordinates.
     */
    Pos2DTemplate operator-(int other) const;

    /**
     * @brief Greater-than operator to compare the x-coordinate with a value.
     * @param value The value to compare.
     * @return True if x is greater, false otherwise.
     */
    bool operator>(classType value) const;

    /**
     * @brief Less-than operator to compare the x-coordinate with a value.
     * @param value The value to compare.
     * @return True if x is less, false otherwise.
     */
    bool operator<(classType value) const;

    /**
     * @brief Less-than operator to compare two Pos2DTemplate objects.
     * @param other The other Pos2DTemplate object.
     * @return True if this object is less, false otherwise.
     */
    bool operator<(const Pos2DTemplate &other) const;

    /**
     * @brief Greater-than-or-equal operator to compare the x-coordinate with a value.
     * @param value The value to compare.
     * @return True if x is greater or equal, false otherwise.
     */
    bool operator>=(classType value) const;

    /**
     * @brief Less-than-or-equal operator to compare the x-coordinate with a value.
     * @param value The value to compare.
     * @return True if x is less or equal, false otherwise.
     */
    bool operator<=(classType value) const;

    /**
     * @brief Right-shift operator for integral types.
     * @tparam T The type of the coordinates.
     * @param shiftAmount The number of bits to shift.
     * @return A new Pos2DTemplate object with shifted coordinates.
     */
    template<typename T = classType, typename = std::enable_if_t<std::is_integral_v<T> > >
    Pos2DTemplate operator>>(int shiftAmount) const {
        return {x >> shiftAmount, z >> shiftAmount};
    }

    /**
     * @brief Left-shift operator for integral types.
     * @tparam T The type of the coordinates.
     * @param shiftAmount The number of bits to shift.
     * @return A new Pos2DTemplate object with shifted coordinates.
     */
    template<typename T = classType, typename = std::enable_if_t<std::is_integral_v<T> > >
    Pos2DTemplate operator<<(int shiftAmount) const {
        return {x << shiftAmount, z << shiftAmount};
    }

    /**
     * @brief Converts the position to chunk coordinates (divided by 16).
     * @tparam T The type of the coordinates.
     * @return A new Pos2DTemplate object with chunk coordinates.
     */
    template<typename T = classType, typename = std::enable_if_t<std::is_integral_v<T> > >
    MU ND Pos2DTemplate toChunkPos() const {
        return {x >> 4, z >> 4};
    }

    /**
     * @brief Converts the position to block coordinates (multiplied by 16).
     * @tparam T The type of the coordinates.
     * @return A new Pos2DTemplate object with block coordinates.
     */
    template<typename T = classType, typename = std::enable_if_t<std::is_integral_v<T> > >
    MU ND Pos2DTemplate toBlockPos() const {
        return {x << 4, z << 4};
    }

    /**
     * @brief Converts the position to another type.
     * @tparam U The target type.
     * @return A new Pos2DTemplate object with converted coordinates.
     */
    template<typename U, typename = std::enable_if_t<std::is_fundamental_v<U> > >
    MU ND Pos2DTemplate<U> asType() const {
        return {static_cast<U>(x), static_cast<U>(z)};
    }

#ifdef INCLUDE_QT
        /**
         * @brief Outputs the position to a QDebug stream.
         * @param out The QDebug stream.
         * @param pos The position to output.
         * @return The modified QDebug stream.
         */
        friend QDebug operator<<(QDebug out, const Pos2DTemplate<classType>& pos) {
            out.nospace() << "(" << pos.x << ", " << pos.z << ")";
            return out.space();
        }
#endif

    /**
     * @brief Outputs the position to an ostream.
     * @param out The ostream.
     * @param pos The position to output.
     * @return The modified ostream.
     */
    friend std::ostream &operator<<(std::ostream &out, const Pos2DTemplate &pos) {
        out << "(" << pos.x << ", " << pos.z << ")";
        return out;
    }

    /**
     * @brief Computes the absolute value of the position.
     * @param pos The input position.
     * @return A new Pos2DTemplate object with absolute coordinates.
     */
    MU friend Pos2DTemplate abs(const Pos2DTemplate &pos) {
        return {std::abs(pos.x), std::abs(pos.z)};
    }

    /**
     * @brief Computes the squared distance from the origin.
     * @return The squared distance.
     */
    ND double distanceSq() const;

    /**
     * @brief Sets the position to new coordinates.
     * @param xIn The new x-coordinate.
     * @param zIn The new z-coordinate.
     */
    MU void setPos(classType xIn, classType zIn);

    /**
     * @brief Converts the position to a string representation.
     * @return A string representing the position.
     */
    MU ND std::string toString() const;

    /**
     * @brief Checks if the position is within specified bounds.
     * @param lowerX The lower bound for x.
     * @param lowerZ The lower bound for z.
     * @param upperX The upper bound for x.
     * @param upperZ The upper bound for z.
     * @return True if inside bounds, false otherwise.
     */
    MU ND bool insideBounds(classType lowerX, classType lowerZ, classType upperX, classType upperZ) const;

    /**
     * @brief Converts the position to a 64-bit integer representation.
     * @return The 64-bit integer representation.
     */
    MU ND uint64_t asLong() const {
        return (static_cast<uint64_t>(x) << 32) | static_cast<uint64_t>(z);
    }

    /**
     * @brief Computes a hash value for the position.
     * @return The hash value.
     */
    MU ND uint64_t hash() const {
        return std::hash<uint64_t>()(asLong());
    }

    /**
     * @struct Hasher
     * @brief A functor for hashing Pos2DTemplate objects.
     */
    struct Hasher {
        /**
         * @brief Computes a hash value for a Pos2DTemplate object.
         * @tparam T The type of the coordinates.
         * @param pos The position to hash.
         * @return The hash value.
         */
        template<typename T = classType, typename = std::enable_if_t<std::is_integral_v<T> > >
        std::size_t operator()(const Pos2DTemplate &pos) const {
            return pos.asLong();
        }
    };
};

typedef Pos2DTemplate<int> Pos2D; ///< Alias for Pos2DTemplate with int coordinates.
typedef std::vector<Pos2D> Pos2DVec_t; ///< Alias for a vector of Pos2D objects.

typedef Pos2DTemplate<double> DoublePos2D; ///< Alias for Pos2DTemplate with double coordinates.

#include "Pos2DTemplate.inl"
