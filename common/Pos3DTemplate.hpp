#pragma once

#include <cmath>
#include <ostream>

#ifdef INCLUDE_QT
        #include <QDebug>
#endif

#include "lce/processor.hpp"
#include "lce/enums.hpp"
#include "Pos2DTemplate.hpp"

/**
 * @class Pos3DTemplate
 * @brief A template class representing a 3D position with x, y, and z coordinates.
 * @tparam T The type of the coordinates (e.g., int, double).
 */
template<typename T>
class Pos3DTemplate {
public:
    T x, y, z; ///< The x, y, and z coordinates.

    /**
     * @brief Default constructor initializing x, y, and z to 0.
     */
    Pos3DTemplate() : x(0), y(0), z(0) {
    }

    /**
     * @brief Constructor initializing x and z from a 2D position, with y set to 0.
     * @param pos A 2D position.
     */
    explicit Pos3DTemplate(Pos2DTemplate<T> pos) : x(pos.x), y(0), z(pos.z) {
    }

    /**
     * @brief Constructor initializing x, y, and z with given values.
     * @param xIn The x-coordinate.
     * @param yIn The y-coordinate.
     * @param zIn The z-coordinate.
     */
    Pos3DTemplate(T xIn, T yIn, T zIn) : x(xIn), y(yIn), z(zIn) {
    }

    /**
     * @brief Equality operator to compare two Pos3DTemplate objects.
     * @param other The other Pos3DTemplate object.
     * @return True if equal, false otherwise.
     */
    bool operator==(const Pos3DTemplate &other) const;

    /**
     * @brief Adds a 2D position to the current 3D position.
     * @param other The 2D position to add.
     */
    void operator+=(const Pos2DTemplate<T> &other);

    /**
     * @brief Subtracts a 2D position from the current 3D position.
     * @param other The 2D position to subtract.
     */
    void operator-=(const Pos2DTemplate<T> &other);

    /**
     * @brief Addition operator to add two 3D positions.
     * @param other The other 3D position.
     * @return A new Pos3DTemplate object with the summed coordinates.
     */
    Pos3DTemplate operator+(const Pos3DTemplate &other) const;

    /**
     * @brief Addition operator to add a 2D position to the current 3D position.
     * @param other The 2D position.
     * @return A new Pos3DTemplate object with the summed coordinates.
     */
    Pos3DTemplate operator+(const Pos2DTemplate<T> &other) const;

    /**
     * @brief Addition operator to add a scalar value to all coordinates.
     * @param other The scalar value.
     * @return A new Pos3DTemplate object with the summed coordinates.
     */
    Pos3DTemplate operator+(T other) const;

    /**
     * @brief Subtraction operator to subtract two 3D positions.
     * @param other The other 3D position.
     * @return A new Pos3DTemplate object with the subtracted coordinates.
     */
    Pos3DTemplate operator-(const Pos3DTemplate &other) const;

    /**
     * @brief Subtraction operator to subtract a 2D position from the current 3D position.
     * @param other The 2D position.
     * @return A new Pos3DTemplate object with the subtracted coordinates.
     */
    Pos3DTemplate operator-(const Pos2DTemplate<T> &other) const;

    /**
     * @brief Subtraction operator to subtract a scalar value from all coordinates.
     * @param other The scalar value.
     * @return A new Pos3DTemplate object with the subtracted coordinates.
     */
    Pos3DTemplate operator-(T other) const;

    /**
     * @brief Comparison operators to compare the coordinates with a scalar value.
     * @param value The scalar value.
     * @return True if the condition is met, false otherwise.
     */
    bool operator>(int value) const;

    bool operator<(int value) const;

    bool operator>=(int value) const;

    bool operator<=(int value) const;

    /**
     * @brief Right-shift operator for integral types.
     * @param shiftAmount The number of bits to shift.
     * @return A new Pos3DTemplate object with shifted coordinates.
     */
    template<typename U = T, typename = std::enable_if_t<std::is_integral_v<U> > >
    Pos3DTemplate operator>>(int shiftAmount) const {
        return {x >> shiftAmount, y >> shiftAmount, z >> shiftAmount};
    }

    /**
     * @brief Left-shift operator for integral types.
     * @param shiftAmount The number of bits to shift.
     * @return A new Pos3DTemplate object with shifted coordinates.
     */
    template<typename U = T, typename = std::enable_if_t<std::is_integral_v<U> > >
    Pos3DTemplate operator<<(int shiftAmount) const {
        return {x << shiftAmount, y << shiftAmount, z << shiftAmount};
    }

    /**
     * @brief Converts the position to another type.
     * @tparam U The target type.
     * @return A new Pos3DTemplate object with converted coordinates.
     */
    template<typename U, typename = std::enable_if_t<std::is_fundamental_v<U> > >
    MU ND Pos3DTemplate<U> asType() const {
        Pos3DTemplate<U> pos(static_cast<U>(x), static_cast<U>(y), static_cast<U>(z));
        return pos;
    }

    /**
     * @brief Converts the position to chunk coordinates (divided by 16).
     * @return A new Pos3DTemplate object with chunk coordinates.
     */
    template<typename V = T, typename = std::enable_if_t<std::is_integral_v<V> > >
    MU ND Pos3DTemplate convertToChunkCoords() const;

#ifdef INCLUDE_QT
            /**
             * @brief Outputs the position to a QDebug stream.
             * @param out The QDebug stream.
             * @param pos The position to output.
             * @return The modified QDebug stream.
             */
            friend QDebug operator<<(QDebug out, const Pos3DTemplate &pos) {
                out.nospace() << "(" << pos.x << ", " << pos.y << ", " << pos.z << ")";
                return out.space();
            }
#endif

    /**
     * @brief Outputs the position to an ostream.
     * @param out The ostream.
     * @param pos The position to output.
     * @return The modified ostream.
     */
    friend std::ostream &operator<<(std::ostream &out, const Pos3DTemplate &pos) {
        out << "[" << pos.x << ", " << pos.y << ", " << pos.z << "]";
        return out;
    }

    /**
     * @brief Gets the x, y, or z coordinate.
     * @return The respective coordinate value.
     */
    MU ND T getX() const { return x; }
    MU ND T getY() const { return y; }
    MU ND T getZ() const { return z; }

    /**
     * @brief Checks if the position is null (all coordinates are 0).
     * @return True if null, false otherwise.
     */
    MU ND bool isNull() const { return x == 0 && y == 0 && z == 0; }

    /**
     * @brief Computes the absolute value of the position.
     * @param pos The input position.
     * @return A new Pos3DTemplate object with absolute coordinates.
     */
    friend Pos3DTemplate abs(const Pos3DTemplate &pos) {
        return {std::abs(pos.x), std::abs(pos.y), std::abs(pos.z)};
    }

    /**
     * @brief Sets the position to new coordinates.
     * @param xIn The new x-coordinate.
     * @param yIn The new y-coordinate.
     * @param zIn The new z-coordinate.
     */
    void setPos(T xIn, T yIn, T zIn) {
        this->x = xIn;
        this->y = yIn;
        this->z = zIn;
    }

    /**
     * @brief Adds offsets to the current position.
     * @param xOff The x offset.
     * @param yOff The y offset.
     * @param zOff The z offset.
     * @return A new Pos3DTemplate object with the updated coordinates.
     */
    Pos3DTemplate add(T xOff, T yOff, T zOff) const {
        return {x + xOff, y + yOff, z + zOff};
    }

    /**
     * @brief Moves the position down by a specified offset.
     * @param yOff The offset (default is 1).
     * @return A new Pos3DTemplate object with the updated coordinates.
     */
    Pos3DTemplate down(T yOff = 1) const { return {x, y - yOff, z}; }

    /**
     * @brief Moves the position up by a specified offset.
     * @param yOff The offset (default is 1).
     * @return A new Pos3DTemplate object with the updated coordinates.
     */
    Pos3DTemplate up(T yOff = 1) const { return {x, y + yOff, z}; }

    /**
     * @brief Moves the position east by a specified offset.
     * @param xOff The offset (default is 1).
     * @return A new Pos3DTemplate object with the updated coordinates.
     */
    Pos3DTemplate east(T xOff = 1) const { return {x + xOff, y, z}; }

    /**
     * @brief Moves the position west by a specified offset.
     * @param xOff The offset (default is 1).
     * @return A new Pos3DTemplate object with the updated coordinates.
     */
    Pos3DTemplate west(T xOff = 1) const { return {x - xOff, y, z}; }

    /**
     * @brief Moves the position north by a specified offset.
     * @param zOff The offset (default is 1).
     * @return A new Pos3DTemplate object with the updated coordinates.
     */
    Pos3DTemplate north(T zOff = 1) const { return {x, y, z - zOff}; }

    /**
     * @brief Moves the position south by a specified offset.
     * @param zOff The offset (default is 1).
     * @return A new Pos3DTemplate object with the updated coordinates.
     */
    Pos3DTemplate south(T zOff = 1) const { return {x, y, z + zOff}; }

    /**
     * @brief Offsets the position in a specified direction.
     * @param facing The direction to offset.
     * @param n The magnitude of the offset (default is 1).
     * @return A new Pos3DTemplate object with the updated coordinates.
     */
    Pos3DTemplate offset(EnumFacing facing, int n = 1) const;

    /**
     * @brief Computes the squared distance in the XZ plane.
     * @return The squared distance.
     */
    ND double distanceSqXZ() const;

    /**
     * @brief Computes the squared distance from the origin.
     * @return The squared distance.
     */
    ND double distanceSq() const;

    /**
     * @brief Computes the squared distance to a specified point.
     * @param toX The x-coordinate of the target point.
     * @param toY The y-coordinate of the target point.
     * @param toZ The z-coordinate of the target point.
     * @return The squared distance.
     */
    ND double distanceSq(double toX, double toY, double toZ) const;

    /**
     * @brief Computes the squared distance to another position.
     * @param to The target position.
     * @return The squared distance.
     */
    ND double distanceSq(const Pos3DTemplate &to) const {
        return distanceSq(static_cast<double>(to.x), static_cast<double>(to.y), static_cast<double>(to.z));
    }

    /**
     * @brief Converts the 3D position to a 2D position by dropping the y-coordinate.
     * @return A Pos2DTemplate object representing the 2D position.
     */
    MU ND Pos2DTemplate<T> asPos2D() const;

    /**
     * @brief Gets all positions within a bounding box.
     * @param from The starting position of the box.
     * @param to The ending position of the box.
     * @return A vector of Pos3DTemplate objects within the box.
     */
    static std::vector<Pos3DTemplate> getAllInBox(const Pos3DTemplate &from, const Pos3DTemplate &to);
};

/** @typedef Pos3D
 *  @brief Alias for Pos3DTemplate with int coordinates.
 */
typedef Pos3DTemplate<int> Pos3D;

/** @typedef Pos3DVec_t
 *  @brief Alias for a vector of Pos3D objects.
 */
typedef std::vector<Pos3D> Pos3DVec_t;

/** @typedef DoublePos3D
 *  @brief Alias for Pos3DTemplate with double coordinates.
 */
typedef Pos3DTemplate<double> DoublePos3D;

#include "Pos3DTemplate.inl"
