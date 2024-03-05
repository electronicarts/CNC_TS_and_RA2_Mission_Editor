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
#include "virtual_file.h"

#include "file32.h"

Cvirtual_file::Cvirtual_file()
{
}

Cvirtual_file::Cvirtual_file(const Cvirtual_binary& d)
{
	write(d);
}

void Cvirtual_file::clear()
{
	m_data.clear();
}

void Cvirtual_file::compact()
{
	if (m_data.size() == 1)
		return;
	Cvirtual_binary t = read();
	clear();
	write(t);
}

const byte* Cvirtual_file::data() const
{
	if (m_data.size() != 1)
		return NULL;
	return m_data.begin()->data();
}

int Cvirtual_file::size() const
{
	int r = 0;
	for (auto& i : m_data)
		 r += i.size();
	return r;
}

int Cvirtual_file::save(const string& fname) const
{
	Cfile32 f;
	int error = f.open_write(fname);
	if (!error)
	{
		for (auto& i : m_data)
			error = f.write(i);
	}
	return error;
}

int Cvirtual_file::load(const string& fname)
{
	clear();
	Cvirtual_binary t;
	int error = t.load(fname);
	if (!error)
		write(t);
	return error;
}

Cvirtual_binary Cvirtual_file::read() const
{
	if (m_data.size() == 1)
		return *m_data.begin();
	Cvirtual_binary r;
	read(r.write_start(size()));
	return r;
}

int Cvirtual_file::read(void* d) const
{
	byte* w = reinterpret_cast<byte*>(d);
	for (auto& i : m_data)
		w += i.read(w);
	return w - reinterpret_cast<byte*>(d);
}

void Cvirtual_file::write(const Cvirtual_binary& d)
{
	m_data.push_back(d);
}

void Cvirtual_file::write(const void* d, int cb_d)
{
	write(Cvirtual_binary(d, cb_d));
}