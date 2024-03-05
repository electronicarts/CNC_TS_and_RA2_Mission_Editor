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

template <class T, class U>
const typename T::value_type::second_type* find_ptr(const T& c, const U& v)
{
	typename T::const_iterator i = c.find(v);
	return i == c.end() ? NULL : &i->second;
}

template <class T, class U>
typename T::value_type::second_type* find_ptr(T& c, const U& v)
{
	typename T::iterator i = c.find(v);
	return i == c.end() ? NULL : &i->second;
}

template <class T, class U>
const typename T::value_type::second_type& find_ptr2(const T& c, const U& v)
{
	static typename T::value_type::second_type z = typename T::value_type::second_type();
	typename T::const_iterator i = c.find(v);
	return i == c.end() ? z : i->second;
}

template <class T, class U>
typename T::value_type::second_type& find_ptr2(T& c, const U& v)
{
	static typename T::value_type::second_type z = typename T::value_type::second_type();
	typename T::iterator i = c.find(v);
	return i == c.end() ? z : i->second;
}

template <class T, class U>
const typename T::value_type* find_ptr0(const T& c, const U& v)
{
	typename T::const_iterator i = c.find(v);
	return i == c.end() ? NULL : &*i;
}

template <class T, class U>
typename T::value_type::second_type& find_ref(T& c, const U& v)
{
	typename T::iterator i = c.find(v);
	assert(i != c.end());
	return i->second;
}

template <class T, class U>
const typename T::value_type::second_type& find_ref(const T& c, const U& v)
{
	typename T::const_iterator i = c.find(v);
	assert(i != c.end());
	return i->second;
}

template <class T, class U>
const typename T::value_type::second_type& find_ref(const T& c, const U& v, const typename T::value_type::second_type& z)
{
	typename T::const_iterator i = c.find(v);
	return i == c.end() ? z : i->second;
}

template <class T, class U>
typename T::value_type::second_type& find_ref(T& c, const U& v, typename T::value_type::second_type& z)
{
	typename T::iterator i = c.find(v);
	return i == c.end() ? z : i->second;
}
