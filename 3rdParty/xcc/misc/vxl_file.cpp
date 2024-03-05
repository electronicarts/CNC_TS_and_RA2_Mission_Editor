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
#include "vxl_file.h"

#include "file32.h"
#include "image_file.h"
#ifndef XCC_MINIMAL_BUILD
#include "multi_line.h"
#include "pcx_decode.h"
#include "pcx_file_write.h"
#endif
#include "string_conversion.h"

bool Cvxl_file::is_valid() const
{
 	int size = get_size();
	const t_vxl_header& h = header();
	if (sizeof(t_vxl_header) > size ||
		strcmp(h.id, vxl_id) ||
		h.one != 1 ||
		h.c_section_headers != h.c_section_tailers ||
		h.size != size - sizeof(t_vxl_header) - h.c_section_headers * sizeof(t_vxl_section_header) - h.c_section_tailers * sizeof(t_vxl_section_tailer) ||
		h.unknown != 0x1f10)
		return false;
	/*
	for (int i = 0; i < header.c_section_headers; i++)
	{
		const t_vxl_section_header& section_header = *get_section_header(i);
		const t_vxl_section_tailer& section_tailer = *get_section_tailer(i);
		if (section_header.one != 1 ||
			section_header.zero ||
			section_tailer.span_end_ofs - section_tailer.span_start_ofs != section_tailer.span_data_ofs - section_tailer.span_end_ofs)
			return false;			
	}
	*/
	return true;
}

#ifndef XCC_MINIMAL_BUILD
int Cvxl_file::extract_as_pcx(const Cfname& name, t_file_type ft, const t_palet _palet) const
{
	t_palet palet;
	memcpy(palet, _palet, sizeof(t_palet));
	convert_palet_18_to_24(palet);
	const t_vxl_section_tailer& section_tailer = *get_section_tailer(0);
	const int cx = section_tailer.cx;
	const int cy = section_tailer.cy;
	const int cz = section_tailer.cz;
	Cvirtual_binary s;
	for (int i = 0; i < cz; i++)
	{
		memset(s.write_start(cx * cy), 0, cx * cy);
		int j = 0;
		for (int y = 0; y < cy; y++)
		{
			for (int x = 0; x < cx; x++)
			{
				const byte* r = get_span_data(0, j);
				if (r)
				{
					int z = 0;
					while (z < cz)
					{
						z += *r++;
						int c = *r++;
						while (c--)
						{
							if (i == z)
								s.data_edit()[x + cx * y] = *r;
							r += 2;
							z++;
						}
						r++;
					}
				}
				j++;
			}
		}
		Cfname t = name;
		t.set_title(name.get_ftitle() + " " + nwzl(4, i));
		int error = image_file_write(t, ft, s.data(), palet, cx, cy);
		if (error)
			return error;
	}
	return 0;
}
#endif

ostream& Cvxl_file::extract_as_text(ostream& os) const
{
	for (int i = 0; i < get_c_section_headers(); i++)
	{
		const t_vxl_section_tailer& section_tailer = *get_section_tailer(i);
		const int cx = section_tailer.cx;
		const int cy = section_tailer.cy;
		const int cz = section_tailer.cz;
		int j = 0;
		for (int y = 0; y < cy; y++)
		{
			for (int x = 0; x < cx; x++)
			{
				const byte* r = get_span_data(i, j);
				if (r)
				{
					int z = 0;
					while (z < cz)
					{
						z += *r++;
						int c = *r++;
						while (c--)
						{
							os << i << ',' << x << ',' << y << ',' << z << ',' << static_cast<int>(*r++) << ',' << static_cast<int>(*r++) << endl;
							z++;
						}
						r++;
					}
				}
				j++;
			}
		}
	}
	return os;
}

enum
{
	vi_header,
	vi_body,
	vi_tailer,
	vi_palet,
	vi_color,
	vi_surface_normal,

	vi_x,
	vi_y,
	vi_z,
	vi_cx,
	vi_cy,
	vi_cz,
	vi_id,
	vi_red,
	vi_green,
	vi_blue,
	vi_x_min,
	vi_y_min,
	vi_z_min,
	vi_x_max,
	vi_y_max,
	vi_z_max
};

#ifndef XCC_MINIMAL_BUILD
int Cvxl_file::extract_as_xif(const string& name) const
{
	Cxif_key k;
	Cxif_key& header = k.open_key_write(vi_header);
	Cxif_key& palet = header.open_key_write(vi_palet);
	int i;
	for (i = 0; i < 256; i++)
	{
		Cxif_key& palet_entry = palet.open_key_write(i);
		palet_entry.set_value_int(vi_red, get_palet()[i].r);
		palet_entry.set_value_int(vi_green, get_palet()[i].g);
		palet_entry.set_value_int(vi_blue, get_palet()[i].b);
	}
	Cxif_key& body = k.open_key_write(vi_body);
	for (i = 0; i < get_c_section_headers(); i++)
	{
		const t_vxl_section_header& section_header = *get_section_header(i);
		const t_vxl_section_tailer& section_tailer = *get_section_tailer(i);
		const int cx = section_tailer.cx;
		const int cy = section_tailer.cy;
		const int cz = section_tailer.cz;
		Cxif_key& section = body.open_key_write(i);
		Cxif_key& header = section.open_key_write(vi_header);
		header.set_value_string(vi_id, section_header.id);
		Cxif_key& body = section.open_key_write(vi_body);	
		int j = 0;
		for (int y = 0; y < cy; y++)
		{
			Cxif_key& yi = body.open_key_write(y);
			for (int x = 0; x < cx; x++)
			{
				Cxif_key& xi = yi.open_key_write(x);
				const byte* r = get_span_data(i, j);
				if (r)
				{
					int z = 0;
					while (z < cz)
					{
						z += *r++;
						int c = *r++;
						while (c--)
						{
							Cxif_key& zi = xi.open_key_write(z);
							zi.set_value_int(vi_color, *r++);
							zi.set_value_int(vi_surface_normal, *r++);
							z++;
						}
						r++;
					}
				}
				j++;
			}
		}
		Cxif_key& tailer = section.open_key_write(vi_tailer);	
		tailer.set_value_int(vi_x_min, section_tailer.x_min_scale * 1000000);
		tailer.set_value_int(vi_y_min, section_tailer.y_min_scale * 1000000);
		tailer.set_value_int(vi_z_min, section_tailer.z_min_scale * 1000000);
		tailer.set_value_int(vi_x_max, section_tailer.x_max_scale * 1000000);
		tailer.set_value_int(vi_y_max, section_tailer.y_max_scale * 1000000);
		tailer.set_value_int(vi_z_max, section_tailer.z_max_scale * 1000000);
		tailer.set_value_int(vi_cx, cx);
		tailer.set_value_int(vi_cy, cy);
		tailer.set_value_int(vi_cz, cz);
	}
	return k.vdata().save(name);
}

Cvirtual_binary vxl_file_write(const Cxif_key& s)
{
	Cvirtual_binary d;
	byte* w = d.write_start(1 << 20);
	const Cxif_key& header_key = s.open_key_read(vi_header);
	const Cxif_key& body_key = s.open_key_read(vi_body);
	int c_sections = body_key.c_keys();
	int* span_start_list_ofs = new int[c_sections];
	int* span_end_list_ofs = new int[c_sections];
	int* span_data_ofs = new int[c_sections];
	t_vxl_header& header = *reinterpret_cast<t_vxl_header*>(w);
	strcpy(header.id, vxl_id);
	header.one = 1;
	header.c_section_headers = c_sections;
	header.c_section_tailers = c_sections;
	header.unknown = 0x1f10;
	const Cxif_key& palet_key = header_key.open_key_read(vi_palet);
	int i;
	for (i = 0; i < 256; i++)
	{
		const Cxif_key& palet_entry_key = palet_key.open_key_read(i);
		header.palet[i].r = palet_entry_key.get_value_int(vi_red);
		header.palet[i].g = palet_entry_key.get_value_int(vi_green);
		header.palet[i].b = palet_entry_key.get_value_int(vi_blue);
	}
	w += sizeof(t_vxl_header);
	for (i = 0; i < c_sections; i++)
	{
		const Cxif_key& section_key = body_key.open_key_read(i);
		const Cxif_key& section_header_key = section_key.open_key_read(vi_header);
		t_vxl_section_header& section_header = *reinterpret_cast<t_vxl_section_header*>(w);
		strcpy(section_header.id, section_header_key.get_value_string(vi_id).c_str());
		section_header.section_i = i;
		section_header.one = 1;
		section_header.zero = 0;
		w += sizeof(t_vxl_section_header);
	}
	byte* body_start = w;
	for (i = 0; i < c_sections; i++)
	{
		const Cxif_key& section_key = body_key.open_key_read(i);
		const Cxif_key& section_body_key = section_key.open_key_read(vi_body);
		const Cxif_key& section_tailer_key = section_key.open_key_read(vi_tailer);
		const int cx = section_tailer_key.get_value_int(vi_cx);
		const int cy = section_tailer_key.get_value_int(vi_cy);
		const int cz = section_tailer_key.get_value_int(vi_cz);
		__int32* span_start_list = reinterpret_cast<__int32*>(w);
		span_start_list_ofs[i] = w - body_start;
		w += 4 * cx * cy;
		__int32* span_end_list = reinterpret_cast<__int32*>(w);
		span_end_list_ofs[i] = w - body_start;
		w += 4 * cx * cy;
		byte* span_data_start = w;
		span_data_ofs[i] = w - body_start;
		int span_i = 0;
		for (int y = 0; y < cy; y++)
		{
			const Cxif_key& yi = section_body_key.open_key_read(y);
			for (int x = 0; x < cx; x++)
			{
				byte* span_start = w;
				const Cxif_key& xi = yi.open_key_read(x);
				int z = 0;
				for (auto& i : xi.m_keys)
				{
					int z_inc = i.first - z;
					z = i.first + 1;
					*w++ = z_inc;
					*w++ = 1;
					*w++ = i.second.get_value_int(vi_color);
					*w++ = i.second.get_value_int(vi_surface_normal);
					*w++ = 1;
				}
				if (span_start == w)
				{
					span_start_list[span_i] = -1;
					span_end_list[span_i] = -1;
				}
				else
				{
					if (z != cz)
					{
						*w++ = cz - z;
						*w++ = 0;
						*w++ = 0;
					}
					span_start_list[span_i] = span_start - span_data_start;
					span_end_list[span_i] = w - span_data_start - 1;
				}
				span_i++;
			}
		}
	}
	header.size = w - body_start;
	for (i = 0; i < c_sections; i++)
	{
		const Cxif_key& section_key = body_key.open_key_read(i);
		const Cxif_key& section_tailer_key = section_key.open_key_read(vi_tailer);
		t_vxl_section_tailer& section_tailer = *reinterpret_cast<t_vxl_section_tailer*>(w);

		section_tailer.span_start_ofs = span_start_list_ofs[i];
		section_tailer.span_end_ofs = span_end_list_ofs[i];
		section_tailer.span_data_ofs = span_data_ofs[i];
		section_tailer.scale = 0;
		for (int ty = 0; ty < 3; ty++)
		{
			for (int tx = 0; tx < 4; tx++)
				section_tailer.transform[ty][tx] = 0;
		}
		section_tailer.x_min_scale = section_tailer_key.get_value_int(vi_x_min) / 1000000.0;
		section_tailer.y_min_scale = section_tailer_key.get_value_int(vi_y_min) / 1000000.0;
		section_tailer.z_min_scale = section_tailer_key.get_value_int(vi_z_min) / 1000000.0;
		section_tailer.x_max_scale = section_tailer_key.get_value_int(vi_x_max) / 1000000.0;
		section_tailer.y_max_scale = section_tailer_key.get_value_int(vi_y_max) / 1000000.0;
		section_tailer.z_max_scale = section_tailer_key.get_value_int(vi_z_max) / 1000000.0;
		section_tailer.cx = section_tailer_key.get_value_int(vi_cx);
		section_tailer.cy = section_tailer_key.get_value_int(vi_cy);
		section_tailer.cz = section_tailer_key.get_value_int(vi_cz);
		section_tailer.unknown = 2;
		w += sizeof(t_vxl_section_tailer);
	}
	delete[] span_data_ofs;
	delete[] span_end_list_ofs;
	delete[] span_start_list_ofs;
	d.set_size(w - d.data());
	return d;
}

Cvirtual_binary vxl_file_write(const byte* s, const byte* s_normals, int cx, int cy, int cz)
{
	Cxif_key k;
	Cxif_key& header = k.open_key_write(vi_header);
	Cxif_key& palet = header.open_key_write(vi_palet);
	for (int i = 0; i < 256; i++)
	{
		Cxif_key& palet_entry = palet.open_key_write(i);
		palet_entry.set_value_int(vi_red, i);
		palet_entry.set_value_int(vi_green, i);
		palet_entry.set_value_int(vi_blue, i);
	}
	Cxif_key& body = k.open_key_write(vi_body);
	{
		Cxif_key& section = body.open_key_write(0);
		Cxif_key& header = section.open_key_write(vi_header);
		header.set_value_string(vi_id, "NONE");
		Cxif_key& body = section.open_key_write(vi_body);	
		for (int y = 0; y < cy; y++)
		{
			Cxif_key& yi = body.open_key_write(y);
			for (int x = 0; x < cx; x++)
			{
				Cxif_key& xi = yi.open_key_write(x);
				const byte* r = s + x + cx * y;
				for (int z = 0; z < cz; z++)
				{
					int v = *r;
					if (v)
					{
						Cxif_key& zi = xi.open_key_write(z);
						zi.set_value_int(vi_color, v);
						zi.set_value_int(vi_surface_normal, s_normals ? s_normals[r - s]: -1);
					}
					r += cx * cy;
				}
			}
		}
		Cxif_key& tailer = section.open_key_write(vi_tailer);	
		tailer.set_value_int(vi_x_min, cx * -1000000 / 2);
		tailer.set_value_int(vi_y_min, cy * -1000000 / 2);
		tailer.set_value_int(vi_z_min, cz * -1000000 / 2);
		tailer.set_value_int(vi_x_max, cx * +1000000 / 2);
		tailer.set_value_int(vi_y_max, cy * +1000000 / 2);
		tailer.set_value_int(vi_z_max, cz * +1000000 / 2);
		tailer.set_value_int(vi_cx, cx);
		tailer.set_value_int(vi_cy, cy);
		tailer.set_value_int(vi_cz, cz);
	}
	return vxl_file_write(k);
}

struct t_voxel
{
	int x;
	int y;
	int z;
	int color;
	int normal;
};

Cvirtual_binary vxl_file_write(Cvirtual_tfile s)
{
	using t_list = list<t_voxel>;

	t_list list;

	int cs = 0;
	int cx = 0;
	int cy = 0;
	int cz = 0;
	while (!s.eof())
	{
		Cmulti_line l = s.read_line();
		t_voxel e;
		int s = l.get_next_int(',');
		e.x = l.get_next_int(',');
		e.y = l.get_next_int(',');
		e.z = l.get_next_int(',');
		e.normal = l.get_next_int(',');
		e.color = l.get_next_int(',');
		if (!s)
			list.push_back(e);
		cs = max(cs, s + 1);
		cx = max(cx, e.x + 1);
		cy = max(cy, e.y + 1);
		cz = max(cz, e.z + 1);
	}
	Cvirtual_binary colors(NULL, cx * cy * cz);
	Cvirtual_binary normals(NULL, cx * cy * cz);
	colors.memset(0);
	normals.memset(0);
	for (auto& e : list)
	{
		int o = e.x + cx * (e.y + cy * e.z);
		colors.data_edit()[o] = e.color;
		normals.data_edit()[o] = e.normal;
	}
	return vxl_file_write(colors.data(), normals.data(), cx, cy, cz);
}

#endif

struct t_vxl4_header
{
	unsigned __int32 c_sections;
};

struct t_vxl4_section_header
{
	char id[16];
	__int32 section_i;
	float scale;
	float x_min_scale;
	float y_min_scale;
	float z_min_scale;
	float x_max_scale;
	float y_max_scale;
	float z_max_scale;
	unsigned __int8 cx;
	unsigned __int8 cy;
	unsigned __int8 cz;
	__int8 unknown;
};

int vxl_encode4(const Cvxl_file& f, byte* d)
{
	const int c_sections = f.get_c_section_headers();

	byte* w = d;
	t_vxl4_header& header = *reinterpret_cast<t_vxl4_header*>(w);
	header.c_sections = c_sections;
	w += sizeof(t_vxl4_header);

	for (int i = 0; i < c_sections; i++)
	{
		const t_vxl_section_header& s_section_header = *f.get_section_header(i);
		const t_vxl_section_tailer& section_tailer = *f.get_section_tailer(i);
		const int cx = section_tailer.cx;
		const int cy = section_tailer.cy;
		const int cz = section_tailer.cz;

		t_vxl4_section_header& section_header = *reinterpret_cast<t_vxl4_section_header*>(w);
		strcpy(section_header.id, s_section_header.id);
		section_header.section_i = s_section_header.section_i;
		assert(s_section_header.section_i == i);
		section_header.scale = section_tailer.scale;
		section_header.x_min_scale = section_tailer.x_min_scale;
		section_header.y_min_scale = section_tailer.y_min_scale;
		section_header.z_min_scale = section_tailer.z_min_scale;
		section_header.x_max_scale = section_tailer.x_max_scale;
		section_header.y_max_scale = section_tailer.y_max_scale;
		section_header.z_max_scale = section_tailer.z_max_scale;
		section_header.cx = section_tailer.cx;
		section_header.cy = section_tailer.cy;
		section_header.cz = section_tailer.cz;
		section_header.unknown = section_tailer.unknown;
		w += sizeof(t_vxl4_section_header);
		for (int j = 0; j < f.get_c_spans(i); j++)
		{
			const byte* r = f.get_span_data(i, j);
			if (r)
			{
				int z = 0;
				while (z < cz)
				{
					int z_inc = *w++ = *r++;
					if (z_inc == cz)
						break;
					z += z_inc;
					int c = *w++ = *r++;
					while (c--)
					{
						*w++ = *r++;
						*w++ = *r++;
						z++;
					}
					r++;
				}
			}
			else
				*w++ = cz;
		}
	}
	return w - d;
}

int vxl_decode4_size(const byte* s)
{
	const byte* r = s;
	const t_vxl4_header& s_header = *reinterpret_cast<const t_vxl4_header*>(r);
	const int c_sections = s_header.c_sections;
	r += sizeof(t_vxl4_header);
	int w = 0;
	for (int i = 0; i < c_sections; i++)
	{
		const t_vxl4_section_header& section_header = *reinterpret_cast<const t_vxl4_section_header*>(r);
		const int cx = section_header.cx;
		const int cy = section_header.cy;
		const int cz = section_header.cz;
		w += 8 * cx * cy;
		r += sizeof(t_vxl4_section_header);
		for (int j = 0; j < cx * cy; j++)
		{
			int z = 0;
			while (z < cz)
			{
				int z_inc = *r++;
				if (z_inc == cz)
					break;
				z += z_inc;
				int count = *r++;
				r += 2 * count;
				w += 2 * count + 3;
				z += count;
			}
		}
	}
	return w
		+ sizeof(t_vxl_header)
		+ (sizeof(t_vxl_section_header) + sizeof(t_vxl_section_tailer)) * c_sections;
}

Cvirtual_binary vxl_decode4(const byte* s, int cb_d)
{
	Cvirtual_binary d;
	const byte* r = s;
	const t_vxl4_header& s_header = *reinterpret_cast<const t_vxl4_header*>(r);
	const int c_sections = s_header.c_sections;
	r += sizeof(t_vxl4_header);
	byte* w = d.write_start(cb_d ? cb_d : 1 << 20);
	t_vxl_header& header = *reinterpret_cast<t_vxl_header*>(w);
	strcpy(header.id, vxl_id);
	header.one = 1;
	header.c_section_headers = c_sections;
	header.c_section_tailers = c_sections;
	header.unknown = 0x1f10;
	w += sizeof(t_vxl_header);
	t_vxl_section_header* d_section_header = reinterpret_cast<t_vxl_section_header*>(w);
	t_vxl_section_tailer* section_tailer = new t_vxl_section_tailer[c_sections];
	w += sizeof(t_vxl_section_header) * c_sections;
	byte* body_start = w;
	for (int i = 0; i < c_sections; i++)
	{
		const t_vxl4_section_header& section_header = *reinterpret_cast<const t_vxl4_section_header*>(r);
		const int cx = section_header.cx;
		const int cy = section_header.cy;
		const int cz = section_header.cz;
		strcpy(d_section_header->id, section_header.id);
		d_section_header->section_i = section_header.section_i;
		d_section_header->one = 1;
		d_section_header->zero = 0;
		d_section_header++;
		__int32* span_start_list = reinterpret_cast<__int32*>(w);
		section_tailer[i].span_start_ofs = w - body_start;
		w += 4 * cx * cy;
		__int32* span_end_list = reinterpret_cast<__int32*>(w);
		section_tailer[i].span_end_ofs = w - body_start;
		w += 4 * cx * cy;
		section_tailer[i].span_data_ofs = w - body_start;
		section_tailer[i].scale = section_header.scale;
		section_tailer[i].x_min_scale = section_header.x_min_scale;
		section_tailer[i].y_min_scale = section_header.y_min_scale;
		section_tailer[i].z_min_scale = section_header.z_min_scale;
		section_tailer[i].x_max_scale = section_header.x_max_scale;
		section_tailer[i].y_max_scale = section_header.y_max_scale;
		section_tailer[i].z_max_scale = section_header.z_max_scale;
		section_tailer[i].cx = section_header.cx;
		section_tailer[i].cy = section_header.cy;
		section_tailer[i].cz = section_header.cz;
		section_tailer[i].unknown = section_header.unknown;
		r += sizeof(t_vxl4_section_header);
		byte* span_data_start = w;
		for (int j = 0; j < cx * cy; j++)
		{
			byte* span_start = w;
			int z = 0;
			while (z < cz)
			{
				int z_inc = *r++;
				if (z_inc == cz)
					break;
				z += *w++ = z_inc;
				int count = *w++ = *r++;
				int c = count;
				while (c--)
				{
					*w++ = *r++;
					*w++ = *r++;
					z++;
				}
				*w++ = count;
			}
			if (span_start == w)
			{
				span_start_list[j] = -1;
				span_end_list[j] = -1;
			}
			else
			{
				span_start_list[j] = span_start - span_data_start;
				span_end_list[j] = w - span_data_start - 1;
			}
		}
	}
	header.size = w - body_start;
	memcpy(w, section_tailer, sizeof(t_vxl_section_tailer) * c_sections);
	delete[] section_tailer;
	w += sizeof(t_vxl_section_tailer) * c_sections;
	assert(!cb_d || d.size() == w - d.data());
	return cb_d ? d : Cvirtual_binary(d.data(), w - d.data());
}
