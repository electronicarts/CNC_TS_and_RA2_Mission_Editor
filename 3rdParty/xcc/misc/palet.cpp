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
#include <climits>
#include "palet.h"

void convert_image_8_to_24(const byte* s, byte* d, int cx, int cy, const t_palet palet)
{
	const byte* r = s;
	byte* w = d;
	int c = cx * cy;
	while (c--)
	{
		const t_palet_entry* v = palet + *r++;
		*w++ = v->r;
		*w++ = v->g;
		*w++ = v->b;
	}
}

void create_downsample_table(const t_palet palet, byte* rp)
{
	byte* rp_w = rp;
	for (int b = 0; b < 0x40; b++)
	{
		for (int g = 0; g < 0x40; g++)
		{
			for (int r = 0; r < 0x40; r++)
				*rp_w++ = find_color(r * 255 / 63, g * 255 / 63, b * 255 / 63, palet);
		}
	}
}

void convert_image_24_to_8(const byte* s, byte* d, int cx, int cy, const byte* rp)
{
	const byte* r = s;
	byte* w = d;
	int c = cx * cy;
	while (c--)
	{
		int red = *r++ >> 2 & 0x3f;
		int green = *r++ >> 2 & 0x3f;
		int blue = *r++ >> 2 & 0x3f;
		*w++ = rp[red | green << 6 | blue << 12];
	}
}

void convert_image_24_to_8(const byte* s, byte* d, int cx, int cy, const t_palet palet)
{
	const byte* r = s;
	byte* w = d;
	int c = cx * cy;
	while (c--)
	{
		int red = *r++;
		int green = *r++;
		int blue = *r++;
		*w++ = find_color(red, green, blue, palet);
	}
}

void downsample_image(const t_palet32entry* s, byte* d, int cx, int cy, const byte* rp)
{
	const t_palet32entry* r = s;
	byte* w = d;
	int c = cx * cy;
	while (c--)
	{
		t_palet32entry e = *r++;
		*w++ = e.a < 0x80 ? 0 : rp[e.r >> 2 | (e.g & 0xfc) << 4 | (e.b & 0xfc) << 10];
	}
}

void downsample_image(const t_palet32entry* s, byte* d, int cx, int cy, const t_palet palet)
{
	const t_palet32entry* r = s;
	byte* w = d;
	int c = cx * cy;
	while (c--)
	{
		t_palet32entry e = *r++;
		if (e.a < 0x80)
			*w++ = 0;
		else
			*w++ = find_color(e.r, e.g, e.b, palet);
	}
}

void upsample_image(const byte* s, t_palet32entry* d, int cx, int cy, const t_palet palet)
{
	const byte* r = s;
	t_palet32entry* w = d;
	int c = cx * cy;
	while (c--)
	{
		t_palet32entry& e = *w++;
		int z = *r++;
		if (z)
		{
			const t_palet_entry* v = palet + z;
			e.r = v->r;
			e.g = v->g;
			e.b = v->b;
			e.a = 0xff;
		}
		else
		{
			e.r = 0x80;
			e.g = 0x80;
			e.b = 0x80;
			e.a = 0;
		}
	}
}

void convert_palet_18_to_24(const t_palet s, t_palet d)
{
	for (int i = 0; i < 256; i++)
	{
		d[i].r = (s[i].r & 63) * 255 / 63;
		d[i].g = (s[i].g & 63) * 255 / 63;
		d[i].b = (s[i].b & 63) * 255 / 63;
	}
}

void convert_palet_24_to_18(const t_palet s, t_palet d)
{
	for (int i = 0; i < 256; i++)
	{
		d[i].r = s[i].r >> 2;
		d[i].g = s[i].g >> 2;
		d[i].b = s[i].b >> 2;
	}
}

void convert_palet_18_to_24(t_palet palet)
{
	convert_palet_18_to_24(palet, palet);
}

void convert_palet_24_to_18(t_palet palet)
{
	convert_palet_24_to_18(palet, palet);
}

int find_color(int r, int g, int b, const t_palet p)
{
	int best_i;
	int min_d = INT_MAX;
	for (int i = 0; i < 256; i++)
	{
		int d_r = p[i].r - r;
		int d_g = p[i].g - g;
		int d_b = p[i].b - b;
		int d = d_r * d_r + d_g * d_g + d_b * d_b;
		if (d < min_d)
		{
			min_d = d;
			best_i = i;
		}
	}
	return best_i;
}

int find_color_t(int r, int g, int b, const t_palet p)
{
	int best_i;
	int min_d = INT_MAX;
	for (int i = 1; i < 256; i++)
	{
		int d_r = p[i].r - r;
		int d_g = p[i].g - g;
		int d_b = p[i].b - b;
		int d = d_r * d_r + d_g * d_g + d_b * d_b;
		if (d < min_d)
		{
			min_d = d;
			best_i = i;
		}
	}
	return best_i;
}

void create_rp(const t_palet s1, const t_palet s2, byte* d)
{
	d[0] = 0;
	for (int i = 1; i < 256; i++)
		d[i] = find_color(s1[i].r, s1[i].g, s1[i].b, s2);
}

void apply_rp(byte* d, int cb_d, const byte* rp)
{
	while (cb_d--)
	{
		*d = rp[*d];
		d++;
	}
}