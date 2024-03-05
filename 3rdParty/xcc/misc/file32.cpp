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

#include "stdafx.h"
#include "file32.h"

int Cfile32::open(const Cwin_handle& h)
{
	m_h = h;
	m_p = 0;
	return !is_open();
}

int Cfile32::open(const string& name, int access)
{
	return access & GENERIC_WRITE ? open(name, access, CREATE_ALWAYS, 0) : open(name, access, OPEN_EXISTING, FILE_SHARE_READ);
}

int Cfile32::open(const string& name, int access, int creation, int share)
{
	m_h = Cwin_handle(CreateFileA(name.c_str(), access, share, NULL, creation, FILE_ATTRIBUTE_NORMAL, NULL));
	m_p = 0;
	return !is_open();
}

FILETIME Cfile32::get_creation_time() const
{
	assert(is_open());
	FILETIME time;
	int r = GetFileTime(h(), &time, NULL, NULL);
	assert(r);
	return time;
}

FILETIME Cfile32::get_last_access_time() const
{
	assert(is_open());
	FILETIME time;
	int r = GetFileTime(h(), NULL, &time, NULL);
	assert(r);
	return time;
}

FILETIME Cfile32::get_last_write_time() const
{
	assert(is_open());
	FILETIME time;
	int r = GetFileTime(h(), NULL, NULL, &time);
	assert(r);
	return time;
}

int Cfile32::open_read(const string& name)
{
	return open(name, GENERIC_READ, OPEN_EXISTING, FILE_SHARE_READ);
}

int Cfile32::open_edit(const string& name)
{
	return open(name, GENERIC_READ | GENERIC_WRITE, OPEN_ALWAYS, 0);
}

int Cfile32::open_write(const string& name)
{
	return open(name, GENERIC_WRITE, CREATE_ALWAYS, 0);
}

long long Cfile32::size() const
{
	assert(is_open());
	LARGE_INTEGER v;
	return GetFileSizeEx(h(), &v) ? v.QuadPart : -1;
}

int Cfile32::read(void* data, int size)
{
	assert(is_open());
	if (SetFilePointer(h(), m_p, 0, FILE_BEGIN) == -1)
		return 1;
	DWORD cb_read;
	if (!ReadFile(h(), data, size, &cb_read, 0) || cb_read != size)
		return 1;
	m_p += size;
	return 0;
}

int Cfile32::write(const void* data, int size)
{
	assert(is_open());
	if (SetFilePointer(h(), m_p, 0, FILE_BEGIN) == -1)
		return 1;
	DWORD cb_write;
	if (!WriteFile(h(), data, size, &cb_write, 0) || cb_write != size)
		return 1;
	m_p += size;
	return 0;
}

int Cfile32::write(data_ref v)
{
	return write(v.data(), v.size());
};

int Cfile32::set_eof()
{
	assert(is_open());
	if (SetFilePointer(h(), m_p, 0, FILE_BEGIN) == -1)
		return 1;
	return !SetEndOfFile(h());
}

void Cfile32::close()
{
	m_h.clear();
}

Cvirtual_binary Cfile32::get_mm()
{
	if (!size())
		return Cvirtual_binary();
  Cwin_handle mh(CreateFileMapping(h(), NULL, PAGE_READONLY, 0, 0, NULL));
  void* d = mh ? MapViewOfFile(mh, FILE_MAP_READ, 0, 0, 0) : NULL;
  return d ? Cvirtual_binary(d, size(), std::shared_ptr<void>(d, UnmapViewOfFile)) : Cvirtual_binary();
}

Cvirtual_binary file32_read(const string& name)
{
	Cfile32 f;
	return f.open_read(name) ? Cvirtual_binary() : f.get_mm();
}

int file32_write(const string& name, const void* s, int cb_s)
{
	Cfile32 f;
	if (int error = f.open_write(name))
		return error;
	return f.write(s, cb_s);
}
