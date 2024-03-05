/*
    XCC Utilities and Library
    Copyright (C) 2000  Olaf van der Spek  <olafvdspek@gmail.com>

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

#include <vartypes.h>
#include <virtual_binary.h>

class Cvirtual_tfile
{
private:
	Cvirtual_binary m_data;
	int pos;

	const char* data() const
	{
		return reinterpret_cast<const char*>(m_data.data());
	}

	int size() const
	{
		return m_data.size();
	}
public:
	string read_line();
	string read_line(bool remove_ws);
	void load_data(const Cvirtual_binary s);

	bool eof() const
	{
		return pos >= size();
	}

	void seek(int ipos)
	{
		pos = ipos;
	}
};
