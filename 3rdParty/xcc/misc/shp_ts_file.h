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

#include "cc_structures.h"
#include "fname.h"
#include "palet.h"
#include "video_file.h"
#include "virtual_image.h"

class Cshp_ts_file : public Cvideo_file<t_shp_ts_header>  
{
public:
	Cvideo_decoder* decoder(const t_palet_entry*);
#ifndef XCC_MINIMAL_BUILD
	int extract_as_pcx(const Cfname& name, t_file_type ft, const t_palet palet, bool combine_shadows = false) const;
	Cvirtual_image extract_as_pcx_single(const t_palet _palet, bool combine_shadows = false) const;
#endif
	bool is_valid() const;

	int cb_pixel() const
	{
		return 1;
	}

	int cf() const
	{
		return header().c_images;
	}

	int cx() const
	{
		return header().cx;
	}

	int cy() const
	{
		return header().cy;
	}

	int get_x(int i) const
	{
		return get_image_header(i)->x;
	}

	int get_y(int i) const
	{
		return get_image_header(i)->y;
	}

	int get_cx(int i) const
	{
		return get_image_header(i)->cx;
	}

	int get_cy(int i) const
	{
		return get_image_header(i)->cy;
	}

    int get_cb_index() const
    {
        return sizeof(t_shp_ts_image_header) * cf();
    }

	bool is_compressed(int i) const
	{
		return get_image_header(i)->compression & 2;
	}

	const t_shp_ts_image_header* get_image_header(int i) const
	{
		return reinterpret_cast<const t_shp_ts_image_header*>(data() + sizeof(t_shp_ts_header) + sizeof(t_shp_ts_image_header) * i);
	}

	int get_offset(int i) const
	{
		return get_image_header(i)->offset;
	}

	const byte* get_image(int i) const
	{
		return data() + get_offset(i);
	}

	static int get_max_size(int cx, int cy, int c_images)
	{
		return sizeof(t_shp_ts_header) + (sizeof(t_shp_ts_image_header) + (2 * (cx + 1) * cy + 7 & ~7)) * c_images;
	}
};

int shp_decode4_size(const byte* s);
Cvirtual_binary shp_decode4(const byte* s, int cb_d);
int shp_encode4(const Cshp_ts_file& f, byte* d);

#ifndef XCC_MINIMAL_BUILD
void shp_split_frames(Cvirtual_image& image, int cblocks_x, int cblocks_y);
void shp_split_shadows(Cvirtual_image& image);
void shp_xor_decode_frames(Cvirtual_image& image, int c_frames);
void shp_xor_encode_frames(Cvirtual_image& image, int c_frames);
#endif
