#pragma once

template<class T>
bool Pos3DTemplate<T>::operator==(const Pos3DTemplate &other) const { return x == other.x && z == other.z; }

template<class T>
void Pos3DTemplate<T>::operator+=(const Pos2DTemplate<T>& other) {
    x += other.x;
    z += other.z;
}

template<class T>
void Pos3DTemplate<T>::operator-=(const Pos2DTemplate<T>& other) {
    x -= other.x;
    z -= other.z;
}

template<class T>
Pos3DTemplate<T> Pos3DTemplate<T>::operator+(const Pos3DTemplate &other) const {
    return {x + other.x, y + other.y, z + other.z};
}

template<class T>
Pos3DTemplate<T> Pos3DTemplate<T>::operator+(const T other) const {
    return {x + other, y + other, z + other};
}
template<class T>
Pos3DTemplate<T> Pos3DTemplate<T>::operator+(const Pos2DTemplate<T>& other) const {
    return {x + other.x, y, z + other.z};
}

template<class T>
Pos3DTemplate<T> Pos3DTemplate<T>::operator-(const Pos3DTemplate &other) const {
    return {x - other.x, y - other.y, z - other.z};
}


template<class T>
Pos3DTemplate<T> Pos3DTemplate<T>::operator-(const T other) const {
    return {x - other, y - other, z - other};
}

template<class T>
Pos3DTemplate<T> Pos3DTemplate<T>::operator-(const Pos2DTemplate<T>& other) const {
    return {x - other.x, y, z - other.z};
}

template<class T>
bool Pos3DTemplate<T>::operator>(int value) const { return x > value && y > value && z > value; }


template<class T>
bool Pos3DTemplate<T>::operator<(int value) const { return x < value && y < value && z < value; }


template<class T>
bool Pos3DTemplate<T>::operator>=(int value) const { return x >= value && y >= value && z >= value; }


template<class T>
bool Pos3DTemplate<T>::operator<=(int value) const { return x <= value && y <= value && z <= value; }

template<class T>
Pos3DTemplate<T> Pos3DTemplate<T>::offset(const EnumFacing facing, const int n) const {
    switch (facing) {
        case EnumFacing::NORTH:
            return {x, y, z - n};
        case EnumFacing::SOUTH:
            return {x, y, z + n};
        case EnumFacing::WEST:
            return {x - n, y, z};
        case EnumFacing::EAST:
            return {x + n, y, z};
        case EnumFacing::UP:
            return {x, y + n, z};
        case EnumFacing::DOWN:
            return {x, y - n, z};
        default:
            return *this;
    }
}


template<class T>
ND double Pos3DTemplate<T>::distanceSqXZ() const {
    using ValueType = std::conditional_t<std::is_same_v<T, double>, T, double>;

    auto d0 = static_cast<ValueType>(x);
    auto d1 = static_cast<ValueType>(z);

    return d0 * d0 + d1 * d1;
}


template<class T>
ND double Pos3DTemplate<T>::distanceSq() const {
    using ValueType = std::conditional_t<std::is_same_v<T, double>, T, double>;

    auto d0 = static_cast<ValueType>(x);
    auto d1 = static_cast<ValueType>(y);
    auto d2 = static_cast<ValueType>(z);

    return d0 * d0 + d1 * d1 + d2 * d2;
}



template<class T>
double Pos3DTemplate<T>::distanceSq(c_double toX, c_double toY, c_double toZ) const {
    /*
    c_double d0 = static_cast<double>(x) - toX;
    c_double d1 = static_cast<double>(y) - toY;
    c_double d2 = static_cast<double>(z) - toZ;
    return d0 * d0 + d1 * d1 + d2 * d2;
    */
    using ValueType = std::conditional_t<std::is_same_v<T, double>, T, double>;

    constexpr auto cast = [](auto value) -> ValueType {
        return static_cast<ValueType>(value);
    };

    ValueType d0 = cast(x) - toX;
    ValueType d1 = cast(y) - toY;
    ValueType d2 = cast(z) - toZ;
    return d0 * d0 + d1 * d1 + d2 * d2;
}

template<class T>
Pos2DTemplate<T> Pos3DTemplate<T>::asPos2D() const { return {x, z}; }

template<class T>
template<typename, typename>
Pos3DTemplate<T> Pos3DTemplate<T>::convertToChunkCoords() const {
    return {x & 15, y & 255, z & 15};
}


template<class T>
std::vector<Pos3DTemplate<T>> Pos3DTemplate<T>::getAllInBox(
    const Pos3DTemplate &from, const Pos3DTemplate &to) {
    c_int minX = std::min(from.getX(), to.getX());
    c_int minY = std::min(from.getY(), to.getY());
    c_int minZ = std::min(from.getZ(), to.getZ());
    c_int maxX = std::max(from.getX(), to.getX());
    c_int maxY = std::max(from.getY(), to.getY());
    c_int maxZ = std::max(from.getZ(), to.getZ());

    std::vector<Pos3DTemplate> positions((maxX - minX + 1) * (maxY - minY + 1) * (maxZ - minZ + 1));
    int posIndex = 0;
    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            for (int z = minZ; z <= maxZ; ++z) {
                positions[posIndex++] = {static_cast<T>(x), static_cast<T>(y), static_cast<T>(z)};
            }
        }
    }
    return positions;
}


template
class Pos3DTemplate<int>;

template
class Pos3DTemplate<double>;

template Pos3DTemplate<int> Pos3DTemplate<int>::convertToChunkCoords<int, void>() const;
