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

#include <string>
#include <vartypes.h>
#include <virtual_binary.h>
#include "win_handle.h"

using namespace std;

class Cfile32
{
public:
	Cvirtual_binary get_mm();
	int open(const Cwin_handle& h);
	int open(const string& name, int access);
	int open(const string& name, int access, int creation, int share);
	int open_read(const string& name);
	int open_edit(const string& name);
	int open_write(const string& name);
	long long size() const;
	FILETIME get_creation_time() const;
	FILETIME get_last_access_time() const;
	FILETIME get_last_write_time() const;
	int read(void* data, int size);
	int write(const void* data, int size);
	int write(data_ref);
	int set_eof();
	void close();

	bool eof() const
	{
		return m_p >= size();
	}

	const Cwin_handle& h() const
	{
		return m_h;
	}

	bool is_open() const
	{
		return h();
	}

	int get_p() const
	{
		return m_p;
	}

	void seek(int p)
	{
		m_p = p;
	}

	void skip(int p)
	{
		m_p += p;
	}
private:
	Cwin_handle m_h;
	int m_p;
};

Cvirtual_binary file32_read(const string& name);
int file32_write(const string& name, const void* s, int cb_s);
