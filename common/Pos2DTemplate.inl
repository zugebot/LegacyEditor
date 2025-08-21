#pragma once

template<class T>
bool Pos2DTemplate<T>::operator==(const Pos2DTemplate &other) const { return x == other.x && z == other.z; }


template<class T>
bool Pos2DTemplate<T>::operator==(int other) const { return x == other && z == other; }


template<class T>
bool Pos2DTemplate<T>::operator!=(const Pos2DTemplate &other) const { return x != other.x || z != other.z; }

template<class T>
bool Pos2DTemplate<T>::operator!=(int other) const { return x != other || z != other; }


template<class T>
Pos2DTemplate<T> Pos2DTemplate<T>::operator+(const Pos2DTemplate &other) const {
    return {x + other.x, z + other.z};
}

template<class T>
Pos2DTemplate<T> Pos2DTemplate<T>::operator+(c_int other) const { return {x + other, z + other}; }



template<class T>
Pos2DTemplate<T> Pos2DTemplate<T>::operator*(const Pos2DTemplate &other) const {
    return {x * other.x, z * other.z};
}

template<class T>
Pos2DTemplate<T> Pos2DTemplate<T>::operator*(c_int other) const {
    return {x * other, z * other};
}


template<class T>
Pos2DTemplate<T> Pos2DTemplate<T>::operator/(c_int other) const {
    return {x / other, z / other};
}


template<class T>
Pos2DTemplate<T> Pos2DTemplate<T>::operator-(const Pos2DTemplate &other) const {
    return {x - other.x, z - other.z};
}


template<class T>
Pos2DTemplate<T> Pos2DTemplate<T>::operator-(c_int other) const { return {x - other, z - other}; }


template<class T>
bool Pos2DTemplate<T>::operator>(T value) const { return x > value && z > value; }


template<class T>
bool Pos2DTemplate<T>::operator<(T value) const { return x < value && z < value; }


template<class T>
bool Pos2DTemplate<T>::operator<(const Pos2DTemplate& other) const {
    if (x < other.x) return true;
    if (x > other.x) return false;
    return z < other.z;
}

template<class T>
bool Pos2DTemplate<T>::operator>=(T value) const { return x >= value && z >= value; }


template<class T>
bool Pos2DTemplate<T>::operator<=(T value) const { return x <= value && z <= value; }


template<class T>
std::string Pos2DTemplate<T>::toString() const {
    return "(" + std::to_string(this->x) + ", " + std::to_string(this->z) + ")";
}

template<class T>
ND double Pos2DTemplate<T>::distanceSq() const {
    using ValueType = std::conditional_t<std::is_same_v<T, double>, T, double>;

    auto d0 = static_cast<ValueType>(x);
    auto d1 = static_cast<ValueType>(z);

    return d0 * d0 + d1 * d1;
}

template<class T>
bool Pos2DTemplate<T>::insideBounds(T lowerX, T lowerZ, T upperX, T upperZ) const {
    return x >= lowerX && x <= upperX && z >= lowerZ && z <= upperZ;
}

template<class T>
void Pos2DTemplate<T>::setPos(T xIn, T zIn) {
    this->x = xIn;
    this->z = zIn;
}

template
class Pos2DTemplate<int>;

template
class Pos2DTemplate<double>;