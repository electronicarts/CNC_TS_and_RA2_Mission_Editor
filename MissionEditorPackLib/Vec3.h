/*
    FinalSun/FinalAlert 2 Mission Editor

    Copyright (C) 1999-2024 Electronic Arts, Inc.
    Authored by Matthias Wagner

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <cassert>

template<class T>
class Vec3
{
public:
    Vec3() = default;
    Vec3(T x, T y, T z) : v{ x, y, z }
    {
    }
    Vec3(T v_[3]) : v{ v_[0], v_[1], v_[2] }
    {
    }

    T& operator[](unsigned int i) {
        assert(i < 3);
        return v[i];
    }

    const T& operator[](unsigned int i) const {
        assert(i < 3);
        return v[i];
    }

    inline T dot(const Vec3& other) const {
        return v[0] * other.v[0] + v[1] * other.v[1] + v[2] * other.v[2];
    }

    inline T length() const
    {
        return sqrt(squaredLength());
    }

    inline T squaredLength() const
    {
        return dot(*this);
    }

    inline Vec3& normalize()
    {
        T invL = T(1) / length();
        v[0] *= invL;
        v[1] *= invL;
        v[2] *= invL;
        return *this;
    }

    inline Vec3& negate()
    {
        v[0] = -v[0];
        v[1] = -v[1];
        v[2] = -v[2];
        return *this;
    }

    inline Vec3& inverse()
    {
        v[0] = T(1) / v[0];
        v[1] = T(1) / v[1];
        v[2] = T(1) / v[2];
        return *this;
    }

    inline Vec3& minimum(const Vec3& v2)
    {
        v[0] = min(v[0], v2[0]);
        v[1] = min(v[1], v2[1]);
        v[2] = min(v[2], v2[2]);
        return *this;
    }

    inline Vec3& maximum(const Vec3& v2)
    {
        v[0] = max(v[0], v2[0]);
        v[1] = max(v[1], v2[1]);
        v[2] = max(v[2], v2[2]);
        return *this;
    }

    inline Vec3& operator *=(const Vec3& v2) {
        v[0] *= v2[0];
        v[1] *= v2[1];
        v[2] *= v2[2];
        return *this;
    }

    inline Vec3& operator *=(const T scale) {
        v[0] *= scale;
        v[1] *= scale;
        v[2] *= scale;
        return *this;
    }

    inline Vec3& operator /=(const Vec3& v2) {
        v[0] /= v2[0];
        v[1] /= v2[1];
        v[2] /= v2[2];
        return *this;
    }

    inline Vec3& operator /=(const T scale) {
        v[0] /= scale;
        v[1] /= scale;
        v[2] /= scale;
        return *this;
    }

    inline Vec3& operator +=(const Vec3& other) {
        v[0] += other.v[0];
        v[1] += other.v[1];
        v[2] += other.v[2];
        return *this;
    }

    inline Vec3& operator -=(const Vec3& other) {
        v[0] -= other.v[0];
        v[1] -= other.v[1];
        v[2] -= other.v[2];
        return *this;
    }

    inline bool equals(const Vec3& other, T epsilon=T(0.001))
    {
        return fabs(v[0] - other.v[0]) <= epsilon && fabs(v[1] - other.v[1]) <= epsilon && fabs(v[2] - other.v[2]) <= epsilon;
    }

    T x() const { return v[0]; }
    T y() const { return v[1]; }
    T z() const { return v[2]; }

    T v[3] = { 0, 0, 0 };
};

template< class T>
inline Vec3<T> operator+(const Vec3<T>& l, const Vec3<T>& r)
{
    auto res = l;
    res += r;
    return res;
}

template< class T>
inline Vec3<T> operator-(const Vec3<T>& l, const Vec3<T>& r)
{
    auto res = l;
    res -= r;
    return res;
}

template< class T>
inline Vec3<T> normalize(Vec3<T> v)
{
    return v.normalize();
}

template< class T>
inline Vec3<T> negate(Vec3<T> v)
{
    return v.negate();
}

template< class T>
inline Vec3<T> inverse(Vec3<T> v)
{
    return v.inverse();
}

template< class T>
inline Vec3<T> minimum(Vec3<T> v, const Vec3<T>& v2)
{
    v.minimum(v2);
    return v;
}

template< class T>
inline Vec3<T> maximum(Vec3<T> v, const Vec3<T>& v2)
{
    v.maximum(v2);
    return v;
}

template< class T>
inline Vec3<T> operator /(Vec3<T> v, T scale)
{
    v /= scale;
    return v;
}

template< class T>
inline Vec3<T> operator *(Vec3<T> v, Vec3<T> v2)
{
    v *= v2;
    return v;
}

template< class T>
inline Vec3<T> operator /(Vec3<T> v, Vec3<T> v2)
{
    v /= v2;
    return v;
}

template< class T>
inline Vec3<T> operator *(Vec3<T> v, T scale)
{
    v *= scale;
    return v;
}

typedef Vec3<float> Vec3f;


template<class T>
class Matrix3_4
{
public:
    Matrix3_4(const T (&m_)[3][4])
    {
        for (int row = 0; row < 3; ++row)
        {
            for (int col = 0; col < 4; ++col)
            {
                m[row][col] = m_[row][col];
            }
        }
    }

    Matrix3_4(const T* m_)
    {
        for (int row = 0; row < 3; ++row)
        {
            for (int col = 0; col < 4; ++col)
            {
                m[row][col] = m_[row * 4 + col];
            }
        }
    }
    
    Vec3<T> operator *(const Vec3<T> v) const
    {
        auto x = v[0];
        auto y = v[1];
        auto z = v[2];
        return Vec3<T>(
            x * m[0][0] + y * m[0][1] + z * m[0][2] + m[0][3],
            x * m[1][0] + y * m[1][1] + z * m[1][2] + m[1][3],
            x * m[2][0] + y * m[2][1] + z * m[2][2] + m[2][3]
        );
    }

    Matrix3_4& scaledColumn(unsigned int iColumn, T scale)
    {
        assert(iColumn < 4);
        m[0][iColumn] *= scale;
        m[1][iColumn] *= scale;
        m[2][iColumn] *= scale;
        return *this;
    }

    Matrix3_4 scaleColumn(unsigned int iColumn, T scale) const
    {
        assert(iColumn < 4);
        Matrix3_4 copy(*this);
        copy.m[0][iColumn] *= scale;
        copy.m[1][iColumn] *= scale;
        copy.m[2][iColumn] *= scale;
        return copy;
    }

    Matrix3_4& setColumn(unsigned int iColumn, const Vec3<T>& v)
    {
        assert(iColumn < 4);
        m[0][iColumn] = v[0];
        m[1][iColumn] = v[1];
        m[2][iColumn] = v[2];
        return *this;
    }

    Vec3f getColumn(unsigned int iColumn) const
    {
        assert(iColumn < 4);
        return Vec3f(m[0][iColumn], m[1][iColumn], m[2][iColumn]);
    }

    static Matrix3_4 translation(const Vec3<T> v)
    {
        return Matrix3_4({ {1, 0, 0, v.x()}, {0, 1, 0, v.y()}, {0, 0, 1, v.z()}});
    }

    static Matrix3_4 scale(const Vec3<T> v)
    {
        return Matrix3_4({ {v.x(), 0, 0, 0}, {0, v.y(), 0, 0}, {0, 0, v.z(), 0}});
    }

public:
    T m[3][4] = { {1, 0, 0, 1}, {0, 1, 0, 1}, {0, 0, 1, 1}};
};

typedef Matrix3_4<float> Matrix3_4f;