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
#include "tmp_ts_file.h"

#include "image_file.h"

void Ctmp_ts_file::get_rect(int& x, int& y, int& cx, int& cy) const
{
	x = INT_MAX;
	y = INT_MAX;
	cx = INT_MIN;
	cy = INT_MIN;	
	int bigy = INT_MIN;
	int bigyval = 0;
	for (int i = 0; i < get_c_tiles(); i++)
	{
		if (get_index()[i])
		{
			int height = get_height() - get_height(i);
			int x_t = get_x(i);
			int y_t = get_y(i) + (height * (get_cy() / 2));
			int x2_t = x_t + get_cx();
			int y2_t = y_t + get_cy();
			if (has_extra_graphics(i))
			{
				int y_t_extra = get_y_extra(i) + (height * (get_cy() / 2));
				int y2_t_extra = y_t_extra + get_cy_extra(i);
				if (y_t_extra < y)
					y = y_t_extra;
				if (y2_t_extra > cy)
					cy = y2_t_extra;
			}
			if (x_t < x)
				x = x_t;
			if (x2_t > cx)
				cx = x2_t;
			if (y_t < y)
				y = y_t;
			if (y2_t > cy)
				cy = y2_t;
			if (bigy < get_y(i))
			{
				bigy = get_y(i);
				bigyval = get_y(i) + get_cy() + (get_height(i) * (get_cy() / 2));
				if (has_extra_graphics(i))
					bigyval -= get_y_extra(i);
			}
		}
	}
	cx -= x;
	cy -= y;
	if (cy < bigyval)
		cy = bigyval;
}

void Ctmp_ts_file::draw(byte* d) const
{
	int global_x, global_y, global_cx, global_cy;
	get_rect(global_x, global_y, global_cx, global_cy);
	memset(d, 0, global_cx * global_cy);
	for (int i = 0; i < get_c_tiles(); i++)
	{
		if (get_index()[i])
		{
			int height = get_height() - get_height(i);
			const byte* r = get_image(i);
			byte* w_line = d + get_x(i) - global_x + global_cx * (get_y(i) - global_y + (height * (get_cy() / 2)));
			int x = get_cx() / 2;
			int cx = 0;
			int y = 0;
			for (; y < get_cy() / 2; y++)
			{
				cx += 4;
				x -= 2;
				memcpy(w_line + x, r, cx);
				r += cx;
				w_line += global_cx;
			}
			for (; y < get_cy(); y++)
			{
				cx -= 4;
				x += 2;
				memcpy(w_line + x, r, cx);
				r += cx;
				w_line += global_cx;
			}
			if (has_extra_graphics(i))
			{
				r += get_cx() * get_cy() / 2;
				w_line = d + get_x_extra(i) - global_x + global_cx * (get_y_extra(i) - global_y + (height * (get_cy() / 2)));
				int cx = get_cx_extra(i);
				int cy = get_cy_extra(i);
				for (y = 0; y < cy; y++)
				{
					byte* w = w_line;
					for (int i = 0; i < cx; i++)
					{
						int v = *r++;
						if (v)
							*w = v;
						w++;
					}
					w_line += global_cx;
				}
			}
		}
	}
}

int decode_tile(const byte* s, byte* d, int cx_d)
{
	int cy = cx_d >> 1;
	memset(d, 0, cx_d * cy);
	const byte* r = s;
	byte* w = d;
	int x = cx_d / 2;
	int cx = 0;
	int y = 0;
	for (; y < cy / 2; y++)
	{
		cx += 4;
		x -= 2;
		memcpy(w + x, r, cx);
		r += cx;
		w += cx_d;
	}
	for (; y < cy; y++)
	{
		cx -= 4;
		x += 2;
		memcpy(w + x, r, cx);
		r += cx;
		w += cx_d;
	}
	return w - d;
}

int encode_tile(const byte* s, byte* d, int cx_s)
{
	int cy = cx_s >> 1;
	const byte* r = s;
	byte* w = d;
	int x = cx_s / 2;
	int cx = 0;
	int y = 0;
	for (; y < cy / 2; y++)
	{
		cx += 4;
		x -= 2;
		memcpy(w, r + x, cx);
		r += cx_s;
		w += cx;
	}
	for (; y < cy; y++)
	{
		cx -= 4;
		x += 2;
		memcpy(w, r + x, cx);
		r += cx_s;
		w += cx;
	}
	return w - d;
}

void Ctmp_ts_file::decode(void* d) const
{
	draw(reinterpret_cast<byte*>(d));
}
