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

#include "cc_file_sh.h"
#include "cc_structures.h"
#include "fname.h"
#include "virtual_tfile.h"
#ifndef XCC_MINIMAL_BUILD
#include "xif_key.h"
#endif

class Cvxl_file : public Ccc_file_sh<t_vxl_header>
{
public:
#ifndef XCC_MINIMAL_BUILD
	int extract_as_pcx(const Cfname& name, t_file_type ft, const t_palet _palet) const;
#endif
	ostream& extract_as_text(ostream& os) const;
#ifndef XCC_MINIMAL_BUILD
	int extract_as_xif(const string& name) const;
#endif
	bool is_valid() const;
	
	int get_c_section_headers() const
	{
		return header().c_section_headers;
	}

	int get_c_spans(int i) const
	{
		return get_section_tailer(i)->span_end_ofs - get_section_tailer(i)->span_start_ofs >> 2;
	}

	int get_c_section_tailers() const
	{
		return header().c_section_tailers;
	}

	const t_palet_entry* get_palet() const
	{
		return header().palet;
	}

	const byte* get_section_body() const
	{
		return data() + sizeof(t_vxl_header) + sizeof(t_vxl_section_header) * get_c_section_tailers();
	}

	const t_vxl_section_header* get_section_header(int i) const
	{
		return reinterpret_cast<const t_vxl_section_header*>(data() + sizeof(t_vxl_header) + sizeof(t_vxl_section_header) * i);
	}

	const t_vxl_section_tailer* get_section_tailer(int i) const
	{
		return reinterpret_cast<const t_vxl_section_tailer*>(data() + get_size() + sizeof(t_vxl_section_tailer) * (i - get_c_section_tailers()));
	}

	const byte* get_span_data(int i, int j) const
	{
		if (get_span_start_list(i)[j] == -1)
			return NULL;
		return get_section_body() + get_section_tailer(i)->span_data_ofs + get_span_start_list(i)[j];
	}

	int get_span_size(int i, int j) const
	{
		return get_span_end_list(i)[j] - get_span_start_list(i)[j] + 1;
	}

	const int* get_span_start_list(int i) const
	{
		return reinterpret_cast<const int*>(get_section_body() + get_section_tailer(i)->span_start_ofs);
	}

	const int* get_span_end_list(int i) const
	{
		return reinterpret_cast<const int*>(get_section_body() + get_section_tailer(i)->span_end_ofs);
	}
};

int vxl_decode4_size(const byte* s);
Cvirtual_binary vxl_decode4(const byte* s, int cb_d);
int vxl_encode4(const Cvxl_file& f, byte* d);
#ifndef XCC_MINIMAL_BUILD
Cvirtual_binary vxl_file_write(const Cxif_key& s);
Cvirtual_binary vxl_file_write(const byte* s, const byte* s_normals, int cx, int cy, int cz);
Cvirtual_binary vxl_file_write(Cvirtual_tfile s);
#endif
