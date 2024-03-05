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
#include "hva_file.h"

#include <fstream>
#include "multi_line.h"
#include "virtual_tfile.h"

bool Chva_file::is_valid() const
{
	const t_hva_header& h = header();
	int size = get_size();
	return !(sizeof(t_hva_header) > size
		|| !h.c_frames 
		|| !h.c_sections
		|| sizeof(t_hva_header) + (48 * h.c_frames + 16) * h.c_sections != size);
}

int Chva_file::extract_as_csv(const string& name) const
{
	ofstream f(name.c_str());
	const int c_frames = get_c_frames();
	const int c_sections = get_c_sections();
	f << "Count frames;" << c_frames << endl
		<< "Count sections;" << c_sections << endl
		<< endl;
	for (int i = 0; i < c_sections; i++)
		f << get_section_id(i) << ";;;;;";
	for (int j = 0; j < c_frames; j++)
	{
		f << endl;
		for (int y = 0; y < 3; y++)
		{
			for (int i = 0; i < c_sections; i++)
			{
				for (int x = 0; x < 4; x++)
				{
					f << get_transform_matrix(i, j)[x + 4 * y] << ';';
				}
				f << ';';
			}
			f << endl;
		}
	}
	return f.fail();
}

Cvirtual_binary hva_file_write(const byte* s, int cb_s)
{
	Cvirtual_binary d;
	const char sep = ';';
	Cvirtual_tfile f;
	f.load_data(Cvirtual_binary(s, cb_s));
	Cmulti_line l;
	l = f.read_line();
	l.get_next_line(sep);
	int c_frames = atoi(l.get_next_line(sep).c_str());
	l = f.read_line();
	l.get_next_line(sep);
	int c_sections = atoi(l.get_next_line(sep).c_str());
	f.read_line();
	byte* w = d.write_start(64 << 10);
	t_hva_header& header = *reinterpret_cast<t_hva_header*>(w);
	strcpy(header.id, "NONE");
	header.c_frames = c_frames;
	header.c_sections = c_sections;
	w += sizeof(t_hva_header);
	l = f.read_line();
	for (int i = 0; i < c_sections; i++)
	{
		strcpy(reinterpret_cast<char*>(w), l.get_next_line(sep).c_str());
		w += 16;
		l.get_next_line(sep);
		l.get_next_line(sep);	
		l.get_next_line(sep);	
		l.get_next_line(sep);	
	}
	float* transform_matrix = reinterpret_cast<float*>(w);
	for (int j = 0; j < c_frames; j++)
	{
		for (int y = 0; y < 3; y++)
		{
			l = f.read_line();
			for (int i = 0; i < c_sections; i++)
			{
				for (int x = 0; x < 4; x++)
				{
					transform_matrix[12 * (c_frames * i + j) + x + 4 * y] = atof(l.get_next_line(sep).c_str());
				}
				l.get_next_line(sep);
			}
		}
		f.read_line();
	}
	w += 4 * 12 * c_frames * c_sections;
	d.set_size(w - d.data());
	return d;
}
