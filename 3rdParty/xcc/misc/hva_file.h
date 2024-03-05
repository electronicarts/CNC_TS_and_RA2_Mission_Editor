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

#include <cc_file_sh.h>
#include <cc_structures.h>

class Chva_file : public Ccc_file_sh<t_hva_header>
{
public:
	int extract_as_csv(const string& name) const;
	bool is_valid() const;

	int get_c_frames() const
	{
		return header().c_frames;
	}

	int get_c_sections() const
	{
		return header().c_sections;
	}

	const char* get_section_id(int i) const
	{
		return reinterpret_cast<const char*>(data() + sizeof(t_hva_header) + 16 * i);
	}

	const float* get_transform_matrix(int i, int j) const
	{
		return reinterpret_cast<const float*>(data() + sizeof(t_hva_header) + 16 * get_c_sections() + (get_c_frames() * i + j) * sizeof(t_hva_transform_matrix));
	}
};

Cvirtual_binary hva_file_write(const byte* s, int cb_s);
