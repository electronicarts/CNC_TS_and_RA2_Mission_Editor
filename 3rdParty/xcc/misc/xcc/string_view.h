/*
    XCC Utilities and Library
    Copyright (C) 2021  Olaf van der Spek  <olafvdspek@gmail.com>

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

#include <charconv>
#include <string>
#include <string_view>

inline std::string& operator<<(std::string& a, std::string_view b)
{
	return a += b;
}

template<class T>
inline std::enable_if_t<std::is_integral<T>::value, std::string&> operator<<(std::string& a, T b)
{
	return a += std::to_string(b);
}

inline float to_float(std::string_view s)
{
	float v;
	auto res = std::from_chars(s.data(), s.data() + s.size(), v);
	return res.ec == std::errc() && res.ptr == s.data() + s.size() ? v : 0;
}

inline long long to_int(std::string_view s)
{
	long long v;
	auto res = std::from_chars(s.data(), s.data() + s.size(), v);
	return res.ec == std::errc() && res.ptr == s.data() + s.size() ? v : 0;
}

template<size_t N>
std::string_view to_string_view(const std::array<char, N>& v)
{
	return std::string_view(v.data(), v.size());
}

template<size_t N>
std::string_view to_string_view(const std::array<unsigned char, N>& v)
{
	return std::string_view(reinterpret_cast<const char*>(v.data()), v.size());
}

inline std::string_view read_until(std::string_view& v, char sep, bool keep_sep = false)
{
	size_t i = v.find(sep);
	std::string_view ret = v.substr(0, i);
	v.remove_prefix(i == std::string_view::npos ? v.size() : keep_sep ? i : i + 1);
	return ret;
}
