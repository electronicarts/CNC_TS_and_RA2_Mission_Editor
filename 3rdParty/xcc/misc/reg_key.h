/*
    XCC Utilities and Library
    Copyright (C) 2006  Olaf van der Spek  <olafvdspek@gmail.com>

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

#include <boost/noncopyable.hpp>
#include <string>
#include <windows.h>

class Creg_key : boost::noncopyable
{
public:
	Creg_key() = default;
	Creg_key(HKEY, const std::string& name, REGSAM sam_desired);
	Creg_key(const Creg_key&, const std::string& name, REGSAM sam_desired);
	~Creg_key();
	LONG create(HKEY key, const std::string& name);
	LONG open(HKEY, const std::string& name, REGSAM sam_desired);
	LONG open(const Creg_key&, const std::string& name, REGSAM sam_desired);
	LONG close();
	LONG query_value(const std::string& name, std::string& value);
	std::string query_value(const std::string& name);
	LONG set_value(const std::string& name, const std::string& value);
private:
	HKEY m_h = NULL;
};
