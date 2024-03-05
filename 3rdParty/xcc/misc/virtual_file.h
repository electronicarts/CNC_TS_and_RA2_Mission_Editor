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

#include <list>
#include <virtual_binary.h>

using namespace std;

class Cvirtual_file  
{
public:
	int save(const string& fname) const;
	int load(const string& fname);
	void clear();
	void compact();
	const byte* data() const;
	int size() const;
	Cvirtual_binary read() const;
	int read(void* d) const;
	void write(const Cvirtual_binary& d);
	void write(const void* d, int cb_d);
	Cvirtual_file();
	Cvirtual_file(const Cvirtual_binary& d);
private:
	using t_data = list<Cvirtual_binary>;
	t_data m_data;
};
