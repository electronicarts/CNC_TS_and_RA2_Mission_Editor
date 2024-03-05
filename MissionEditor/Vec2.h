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

#include <cstdint>

struct CSMap {};
struct CSProjected {};

template<class CS, class T>
struct Vec2
{
public:
	Vec2() = default;
	Vec2(T x_, T y_) : x(x_), y(y_) {}
	bool operator==(const Vec2& other) const = default;

	inline Vec2& operator +=(const Vec2& other) {
		x += other.x;
		y += other.y;
		return *this;
	}

	inline Vec2& operator -=(const Vec2& other) {
		x -= other.x;
		y -= other.y;
		return *this;
	}

	inline Vec2& operator *=(const T v)
	{
		x *= v;
		y *= v;
		return *this;
	}

	inline Vec2& operator *=(const Vec2& other)
	{
		x *= other.x;
		y *= other.y;
		return *this;
	}

	inline Vec2& operator /=(const Vec2& other)
	{
		x /= other.x;
		y /= other.y;
		return *this;
	}

	inline Vec2& operator /=(const T v)
	{
		x /= v;
		y /= v;
		return *this;
	}

	void set(const T x_, const T y_)
	{
		x = x_;
		y = y_;
	}

	template<class T1>
	inline Vec2<CS, T1> convertT() const
	{
		return Vec2<CS, T1>(static_cast<T1>(x), static_cast<T1>(y));
	}

	inline Vec2<CS, float> inverted() const
	{
		return Vec2<CS, float>(1.0f / static_cast<float>(x), 1.0f / static_cast<float>(y));
	}

	inline Vec2 negated() const
	{
		return Vec2(-x, -y);
	}

public:
	T x = 0;
	T y = 0;
};

template<class CS, class T>
inline Vec2<CS, T> operator+(const Vec2<CS, T>& l, const Vec2<CS, T>& r)
{
	auto res = l;
	res += r;
	return res;
}

template<class CS, class T>
inline Vec2<CS, T> operator-(const Vec2<CS, T>& l, const Vec2<CS, T>& r)
{
	auto res = l;
	res -= r;
	return res;
}

template<class CS, class T>
inline Vec2<CS, T> operator*(const Vec2<CS, T>& l, const Vec2<CS, T>& r)
{
	auto res = l;
	res *= r;
	return res;
}

template<class CS>
inline Vec2<CS, float> operator*(const Vec2<CS, std::int32_t>& l, const Vec2<CS, float>& r)
{
	return Vec2<CS, float>(l.x * r.x, l.y * r.y);
}

template<class CS, class T>
inline Vec2<CS, T> operator*(const Vec2<CS, T>& l, const T r)
{
	auto res = l;
	res *= r;
	return res;
}

template<class CS, class T>
inline Vec2<CS, T> operator/(const Vec2<CS, T>& l, const Vec2<CS, T>& r)
{
	auto res = l;
	res /= r;
	return res;
}

template<class CS>
inline Vec2<CS, float> operator/(const Vec2<CS, std::int32_t>& l, const Vec2<CS, float>& r)
{
	return Vec2<CS, float>(l.x / r.x, l.y / r.y);
}

template<class CS, class T>
inline Vec2<CS, T> operator/(const Vec2<CS, T>& l, const T r)
{
	auto res = l;
	res /= r;
	return res;
}

template<class CS, class T>
struct Coords2
{
	Coords2() = default;
	Coords2(T x_, T y_) : x(x_), y(y_) {}
	bool operator==(const Coords2& other) const = default;
	T x = 0;
	T y = 0;
	Coords2& operator +=(const Vec2<CS, T>& other) {
		x += other.x;
		y += other.y;
		return *this;
	}
	Coords2& operator -=(const Vec2<CS, T>& other) {
		x -= other.x;
		y -= other.y;
		return *this;
	}
	void set(const T x_, const T y_)
	{
		x = x_;
		y = y_;
	}
	template<class T1>
	inline Coords2<CS, T1> convertT() const
	{
		return Coords2<CS, T1>(static_cast<T1>(x), static_cast<T1>(y));
	}
};


template<class CS, class T>
inline Coords2<CS, T> operator+(const Coords2<CS, T>& l, const Vec2<CS, T>& r)
{
	auto res = l;
	res += r;
	return res;
}

template<class CS, class T>
inline Coords2<CS, T> operator-(const Coords2<CS, T>& l, const Vec2<CS, T>& r)
{
	auto res = l;
	res -= r;
	return res;
}

template<class CS, class T>
inline Vec2<CS, T> operator-(const Coords2<CS, T>& l, const Coords2<CS, T >& r)
{
	return Vec2<CS, T>(l.x - r.x, l.y - r.y);
}

template<class CS, class T>
inline Coords2<CS, T> operator*(const Coords2<CS, T>& l, const Vec2<CS, T>& r)
{
	return Coords2<CS, float>(l.x * r.x, l.y * r.y);
}

template<class CS>
inline Coords2<CS, float> operator*(const Coords2<CS, std::int32_t>& l, const Vec2<CS, float>& r)
{
	return Coords2<CS, float>(l.x * r.x, l.y * r.y);
}

template<class CS>
inline Coords2<CS, float> operator/(const Coords2<CS, std::int32_t>& l, const Vec2<CS, float>& r)
{
	return Coords2<CS, float>(l.x / r.x, l.y / r.y);
}


typedef Vec2<CSMap, std::int16_t> MapVec;

/// <summary>
/// Logical map coordinates in tiles
/// </summary>
typedef Coords2<CSMap, std::int16_t> MapCoords;

typedef Vec2<CSProjected, std::int32_t> ProjectedVec;
typedef Coords2<CSProjected, std::int32_t> ProjectedCoords;