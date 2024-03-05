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
#include "virtual_tfile.h"

void Cvirtual_tfile::load_data(const Cvirtual_binary s)
{
	m_data = s;
	pos = 0;
}

string Cvirtual_tfile::read_line()
{
	assert(data());
	const int first_non_ws = pos;
	int last_non_ws;
	while (pos < size())
	{
		switch (data()[pos++])
		{
		case '\r':
			last_non_ws = pos - 2;
			if (pos < size() && data()[pos] == '\n')
				pos++;
			return string(data() + first_non_ws, last_non_ws - first_non_ws + 1);
		case '\n':
			last_non_ws = pos - 2;
			return string(data() + first_non_ws, last_non_ws - first_non_ws + 1);
		}
	}
	last_non_ws = size() - 1;
	return string(data() + first_non_ws, last_non_ws - first_non_ws + 1);
}

string Cvirtual_tfile::read_line(bool remove_ws)
{
	if (!remove_ws)
		return read_line();
	assert(data());
	int first_non_ws;
	int last_non_ws;
	while (pos < size())
	{
		switch (data()[pos++])
		{
		case '\r':
			if (pos < size() && data()[pos] == '\n')
				pos++;
		case '\n':
			return "";
		case '\t':
		case ' ':
			break;
		default:
			first_non_ws = pos - 1;
			last_non_ws = pos - 2;
			while (pos < size())
			{
				switch (data()[pos++])
				{
				case '\r':
					if (pos < size() && data()[pos] == '\n')
						pos++;
				case '\n':
					return string(data() + first_non_ws, last_non_ws - first_non_ws + 1);
				case '\t':
				case ' ':
					break;
				default:
					last_non_ws = pos - 1;
				}
			}
			return string(data() + first_non_ws, last_non_ws - first_non_ws + 1);
		}
	}
	return "";
}