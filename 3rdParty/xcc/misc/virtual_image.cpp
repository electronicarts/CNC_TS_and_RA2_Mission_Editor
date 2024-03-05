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

#include <windows.h>
#include <gdiplus.h>
#include <shlwapi.h>

#ifndef XCC_MINIMAL_BUILD
#include "dds_file.h"
#include "image_file.h"
#include "jpeg_file.h"
#include "pcx_file.h"
#include "pcx_file_write.h"
#include "png_file.h"
#include "tga_file.h"
#endif
#include "virtual_image.h"

using namespace Gdiplus;

Cvirtual_image::Cvirtual_image(const Cvirtual_binary& image, int cx, int cy, int cb_pixel, const t_palet_entry* palet, bool inflate)
{
	load(image, cx, cy, cb_pixel, palet, inflate);
}

Cvirtual_image::Cvirtual_image(const void* image, int cx, int cy, int cb_pixel, const t_palet_entry* palet, bool inflate)
{
	load(image, cx, cy, cb_pixel, palet, inflate);
}

const Cvirtual_image& Cvirtual_image::palet(const t_palet_entry* palet, bool inflate)
{
	if (palet)
	{
		memcpy(m_palet.write_start(sizeof(t_palet)), palet, sizeof(t_palet));
		if (inflate)
			convert_palet_18_to_24(reinterpret_cast<t_palet_entry*>(m_palet.data_edit()));
	}
	else
		m_palet.clear();
	return *this;
}

void Cvirtual_image::load(const Cvirtual_binary& image, int cx, int cy, int cb_pixel, const t_palet_entry* p, bool inflate)
{
	assert(cb_pixel == 1 || cb_pixel == 3 || cb_pixel == 4);
	m_cx = cx;
	m_cy = cy;
	mcb_pixel = cb_pixel;
	if (image.size() == cb_image())
		m_image = image;
	else
		m_image.write_start(cb_image());
	palet(p, inflate);
}

void Cvirtual_image::load(const void* image, int cx, int cy, int cb_pixel, const t_palet_entry* p, bool inflate)
{
	assert(cb_pixel == 1 || cb_pixel == 3 || cb_pixel == 4);
	m_cx = cx;
	m_cy = cy;
	mcb_pixel = cb_pixel;
	m_image.write_start(cb_image());
	if (image)
		memcpy(m_image.data_edit(), image, cb_image());
	palet(p, inflate);
}

#ifndef XCC_MINIMAL_BUILD

int Cvirtual_image::load(const Cvirtual_binary& s)
{
	Cdds_file dds_f;
	Cpcx_file pcx_f;
	Cpng_file png_f;
	Ctga_file tga_f;
	if (dds_f.load(s), dds_f.is_valid())
		*this = dds_f.vimage();
	else if (pcx_f.load(s), pcx_f.is_valid())
		*this = pcx_f.vimage();
	else if (png_f.load(s), png_f.is_valid())
		return png_f.decode(*this);
	else if (tga_f.load(s), tga_f.is_valid())
		return tga_f.decode(*this);
	else
	{
		IStream* is = SHCreateMemStream(s.data(), s.size());
		Gdiplus::Bitmap bmp(is);
		is->Release();
		if (bmp.GetLastStatus() != Ok)
			return 1;
		PixelFormat pf = bmp.GetPixelFormat();
		if (bmp.GetPixelFormat() & PixelFormatIndexed)
		{
			load(NULL, bmp.GetWidth(), bmp.GetHeight(), 1, NULL);
			BitmapData d;
			d.Stride = bmp.GetWidth();
			d.Scan0 = image_edit();
			bmp.LockBits(NULL, ImageLockModeRead | ImageLockModeUserInputBuf, PixelFormat8bppIndexed, &d);
			bmp.UnlockBits(&d);
		}
		else
		{
			load(NULL, bmp.GetWidth(), bmp.GetHeight(), 3, NULL);
			BitmapData d;
			d.Stride = bmp.GetWidth() * 3;
			d.Scan0 = image_edit();
			bmp.LockBits(NULL, ImageLockModeRead | ImageLockModeUserInputBuf, PixelFormat24bppRGB, &d);
			bmp.UnlockBits(&d);
			swap_rb();
		}
	}
	return 0;
}

int Cvirtual_image::load(const Cvirtual_file& f)
{
	return load(f.read());
}

int Cvirtual_image::load(const string& fname)
{
	Cvirtual_binary s;
	int error = s.load(fname);
	if (!error)
		error = load(s);
	return error;
}

int Cvirtual_image::save(Cvirtual_file& f, t_file_type ft) const
{
	return image_file_write(f, ft, image(), palet(), m_cx, m_cy);
}

Cvirtual_file Cvirtual_image::save(t_file_type ft) const
{
	return image_file_write(ft, image(), palet(), m_cx, m_cy);
}

int Cvirtual_image::save(const string& fname, t_file_type ft) const
{
	return image_file_write(fname, ft, image(), palet(), m_cx, m_cy);
}

void Cvirtual_image::swap_rb()
{
	int count = m_cx * m_cy;
	t_palet_entry* r = reinterpret_cast<t_palet_entry*>(m_image.data_edit());
	while (count--)
	{
		swap(r->r, r->b);
		r++;
	}
}

static void flip_frame(const byte* s, byte* d, int cx, int cy, int cb_pixel)
{
	int cb_line = cx * cb_pixel;
	const byte* r = s;
	byte* w = d + cb_line * cy;
	while (cy--)
	{
		w -= cb_line;
		memcpy(w, r, cb_line);
		r += cb_line;
	}
}

void Cvirtual_image::flip()
{
	Cvirtual_binary t = m_image;
	flip_frame(t.data(), image_edit(), cx(), cy(), cb_pixel());
}


void Cvirtual_image::cb_pixel(int cb_pixel, const t_palet_entry* palet)
{
	if (cb_pixel < mcb_pixel)
		decrease_color_depth(cb_pixel, palet);
	else if (cb_pixel > mcb_pixel)
		increase_color_depth(cb_pixel);
}

void Cvirtual_image::decrease_color_depth(int new_cb_pixel, const t_palet_entry* palet)
{
	if (new_cb_pixel == 3)
	{
		remove_alpha();
		return;
	}
	assert(new_cb_pixel == 1);
	int old_cb_pixel = cb_pixel();
	Cvirtual_binary t = m_image;
	load(NULL, cx(), cy(), new_cb_pixel, palet);
	byte* w = image_edit();
	int count = m_cx * m_cy;
	if (old_cb_pixel == 3)
	{
		const t_palet_entry* r = reinterpret_cast<const t_palet_entry*>(t.data());
		while (count--)
		{
			*w++ = find_color(r->r, r->g, r->b, palet);
			r++;
		}
	}
	else
	{
		assert(old_cb_pixel == 4);
		const t_palet32entry* r = reinterpret_cast<const t_palet32entry*>(t.data());
		while (count--)
		{
			*w++ = r->a < 0x80 ? find_color(r->r, r->g, r->b, palet) : 0;
			r++;
		}
	}
}

static t_palet32entry p32e(int r, int g, int b, int a = 0)
{
	t_palet32entry e;
	e.r = r;
	e.g = g;
	e.b = b;
	e.a = a;
	return e;
}

static t_palet32entry p32e(t_palet_entry e)
{
	return p32e(e.r, e.g, e.b);
}

static t_palet32entry p32e(const t_palet palet, int i)
{
	return i ? p32e(palet[i]) : p32e(0x80, 0x80, 0x80, 0xff);
}

void Cvirtual_image::increase_color_depth(int new_cb_pixel)
{
	if (cb_pixel() == 3)
	{
		if (new_cb_pixel == 4)
			add_alpha();
		return;
	}
	assert(cb_pixel() == 1);
	Cvirtual_image t = *this;
	const byte* r = t.image();
	load(NULL, cx(), cy(), new_cb_pixel, NULL);
	int count = m_cx * m_cy;
	if (cb_pixel() == 3)
	{
		t_palet_entry* w = reinterpret_cast<t_palet_entry*>(image_edit());
		while (count--)
			*w++ = t.palet()[*r++];
	}
	else
	{
		assert(cb_pixel() == 4);
		t_palet32entry* w = reinterpret_cast<t_palet32entry*>(image_edit());
		while (count--)
			*w++ = p32e(t.palet(), *r++);
	}
}

void Cvirtual_image::add_alpha()
{
	assert(cb_pixel() == 3);
	Cvirtual_binary t = m_image;
	load(NULL, cx(), cy(), 4, NULL);
	int count = m_cx * m_cy;
	const byte* r = t.data();
	byte* w = image_edit();
	while (count--)
	{
		*w++ = *r++;
		*w++ = *r++;
		*w++ = *r++;
		*w++ = 0;
	}
}

void Cvirtual_image::remove_alpha()
{
	if (cb_pixel() != 4)
		return;
	Cvirtual_binary t = m_image;
	load(NULL, cx(), cy(), 3, NULL);
	int count = m_cx * m_cy;
	const byte* r = t.data();
	byte* w = image_edit();
	while (count--)
	{
		*w++ = *r++;
		*w++ = *r++;
		*w++ = *r++;
		r++;
	}
}

void Cvirtual_image::increase_palet_depth()
{
	assert(false);
	Cvirtual_binary t = m_palet;
	const t_palet_entry* s = reinterpret_cast<const t_palet_entry*>(t.data());
	t_palet_entry* d = reinterpret_cast<t_palet_entry*>(t.data_edit());
	for (int i = 0; i < 256; i++)
	{
		d[i].r = (s[i].r & 63) * 255 / 63;
		d[i].g = (s[i].g & 63) * 255 / 63;
		d[i].b = (s[i].b & 63) * 255 / 63;
	}
}

#endif
