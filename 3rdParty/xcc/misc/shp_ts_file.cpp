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
#include "shp_ts_file.h"

#include "image_file.h"
#include "shp_decode.h"
#include "string_conversion.h"
#ifndef XCC_MINIMAL_BUILD
#include "xcc_log.h"
#endif

class Cshp_ts_decoder : public Cvideo_decoder
{
public:
	int cb_pixel() const
	{
		return m_f.cb_pixel();
	}

	int cf() const
	{
		return m_f.cf();
	}

	int cx() const
	{
		return m_f.cx();
	}

	int cy() const
	{
		return m_f.cy();
	}

	int decode(void* d0)
	{
		if (m_frame_i >= cf())
			return 1;
		byte* d = reinterpret_cast<byte*>(d0);
		Cvirtual_binary s;
		const int cx = m_f.get_cx(m_frame_i);
		const int cy = m_f.get_cy(m_frame_i);
		const byte* r;
		if (m_f.is_compressed(m_frame_i))
		{
			decode3(m_f.get_image(m_frame_i), s.write_start(cx * cy), cx, cy);
			r = s.data();
		}
		else
			r = m_f.get_image(m_frame_i);
		memset(d, 0, cb_image());
		byte* w = d + m_f.get_x(m_frame_i) + Cshp_ts_decoder::cx() * m_f.get_y(m_frame_i);
		for (int y = 0; y < cy; y++)
		{
			memcpy(w, r, cx);
			r += cx;
			w += Cshp_ts_decoder::cx();
		}
		m_frame_i++;
		return 0;
	}

	const t_palet_entry* palet() const
	{
		return m_palet;
	}

	int seek(int f)
	{
		m_frame_i = f;
		return 0;
	}

	Cshp_ts_decoder(const Cshp_ts_file& f, const t_palet_entry* palet)
	{
		m_f.load(f);
		m_frame_i = 0;
		memcpy(m_palet, palet, sizeof(t_palet));
	}
private:
	Cshp_ts_file m_f;
	int m_frame_i;
	t_palet m_palet;
};

Cvideo_decoder* Cshp_ts_file::decoder(const t_palet_entry* palet)
{
	return new Cshp_ts_decoder(*this, palet);
}

bool Cshp_ts_file::is_valid() const
{
	const t_shp_ts_header& h = header();
	int size = get_size();
	if (sizeof(t_shp_ts_header) > size || 
		h.zero ||
		h.c_images < 1 || h.c_images > 10000 || 
		sizeof(t_shp_ts_header) + get_cb_index() > size)
		return false;
	for (int i = 0; i < min(cf(), 1000); i++)
	{
		const t_shp_ts_image_header& image_header = *get_image_header(i);
		if (!image_header.cx && !image_header.cy && !image_header.offset)
			continue;
		if (!image_header.cx || image_header.x + image_header.cx > h.cx ||
			!image_header.cy || image_header.y + image_header.cy > h.cy ||
			image_header.zero ||
			image_header.offset < sizeof(t_shp_ts_header) + get_cb_index())
			return false;
		if (is_compressed(i))
		{
			if (image_header.offset > size)
				return false;
		}
		else
		{
			if (image_header.offset + image_header.cx * image_header.cy > size)
				return false;
		}
	}
	return true;
}

int get_ofs(int x, int y, int cx, int cy)
{
	return x + cx * y;
}

#ifndef XCC_MINIMAL_BUILD
int Cshp_ts_file::extract_as_pcx(const Cfname& name, t_file_type ft, const t_palet _palet, bool combine_shadows) const
{
	t_palet palet;
	convert_palet_18_to_24(_palet, palet);
	int error = 0;
	const int global_cx = cx();
	const int global_cy = cy();
	const int c_images = cf();
	if (combine_shadows && ~c_images & 1)
	{
		bool shadow = false;
		byte* image = new byte[global_cx * global_cy];
		byte* d = new byte[global_cx * global_cy * c_images >> 1];
		byte* w = d;
		for (int i = 0; i < c_images; i++)
		{
			const int cx = get_cx(i);
			const int cy = get_cy(i);
			const byte* r;
			if (is_compressed(i))
			{
				decode3(get_image(i), image, cx, cy);
				r = image;
			}
			else
				r = get_image(i);
			if (!shadow)
			{
				if (i == c_images >> 1)
				{
					shadow = true;
					w = d;
				}
				else
					memset(w, 0, global_cx * global_cy);
			}
			byte* w_start = w;
			w += get_x(i) + global_cx * get_y(i);
			for (int y = 0; y < cy; y++)
			{
				if (shadow)
				{
					for (int x = 0; x < cx; x++)
					{
						if (*r++)
							w[x] = 4;
					}
				}
				else
				{
					memcpy(w, r, cx);
					r += cx;
				}
				w += global_cx;
			}
			if (shadow)
			{
				Cfname t = name;
				t.set_title(name.get_ftitle() + " " + nwzl(4, i - (c_images >> 1)));
				error = image_file_write(t, ft, w_start, palet, global_cx, global_cy);
				if (error)
					break;
			}
			w = w_start + global_cx * global_cy;
		}
		delete[] d;
		delete[] image;
	}
	else
	{
		byte* image = new byte[global_cx * global_cy];
		byte* s = new byte[global_cx * global_cy];
		for (int i = 0; i < c_images; i++)
		{
			const int cx = get_cx(i);
			const int cy = get_cy(i);
			const byte* r;
			if (is_compressed(i))
			{
				decode3(get_image(i), image, cx, cy);
				r = image;
			}
			else
				r = get_image(i);
			memset(s, 0, global_cx * global_cy);
			byte* w = s + get_x(i) + global_cx * get_y(i);
			for (int y = 0; y < cy; y++)
			{
				memcpy(w, r, cx);
				r += cx;
				w += global_cx;
			}
			// xcc_log::write_line("<tr><td>" + name.get_ftitle() + "</td><td><img src=" + name.get_fname() + "></td></tr>");
			Cfname t = name;
			t.set_title(name.get_ftitle() + " " + nwzl(4, i));
			error = image_file_write(t, ft, s, palet, global_cx, global_cy);
			if (error)
				break;
		}
		delete[] s;
		delete[] image;
	}
	return error;
}

Cvirtual_image Cshp_ts_file::extract_as_pcx_single(const t_palet _palet, bool combine_shadows) const
{
	t_palet palet;
	convert_palet_18_to_24(_palet, palet);
	const int global_cx = cx();
	const int global_cy = cy();
	int c_images = cf();
	combine_shadows &= ~c_images & 1;
	if (combine_shadows)
		c_images >>= 1;
	const int cblocks_x = min(c_images, 1024 / global_cx);
	const int cblocks_y = (c_images + cblocks_x - 1) / cblocks_x;
	int cx_s = cblocks_x * global_cx;
	int cy_s = cblocks_y * global_cy;
	Cvirtual_binary image;
	Cvirtual_binary s;
	memset(s.write_start(cx_s * cy_s), 0, cx_s * cy_s);
	if (combine_shadows)
	{
		c_images <<= 1;
		bool shadow = false;
		for (int i = 0; i < c_images; i++)
		{
			const int cx = get_cx(i);
			const int cy = get_cy(i);
			const byte* r;
			if (is_compressed(i))
			{
				decode3(get_image(i), image.write_start(global_cx * global_cy), cx, cy);
				r = image.data();
			}
			else
				r = get_image(i);
			if (!shadow && i == c_images >> 1)
				shadow = true;
			int j = i % (c_images >> 1);
			byte* w = s.data_edit() + get_ofs(j % cblocks_x * global_cx + get_x(i), j / cblocks_x * global_cy + get_y(i), cx_s, cy_s);
			for (int y = 0; y < cy; y++)
			{
				if (shadow)
				{
					for (int x = 0; x < cx; x++)
					{
						if (*r++)
							w[x] = 4;
					}
				}
				else
				{
					memcpy(w, r, cx);
					r += cx;
				}
				w += cx_s;
			}
		}
	}
	else
	{
		for (int i = 0; i < c_images; i++)
		{
			const int cx = get_cx(i);
			const int cy = get_cy(i);
			const byte* r;
			if (is_compressed(i))
			{
				decode3(get_image(i), image.write_start(global_cx * global_cy), cx, cy);
				r = image.data();
			}
			else
				r = get_image(i);
			byte* w = s.data_edit() + get_ofs(i % cblocks_x * global_cx + get_x(i), i / cblocks_x * global_cy + get_y(i), cx_s, cy_s);
			for (int y = 0; y < cy; y++)
			{
				memcpy(w, r, cx);
				r += cx;
				w += cx_s;
			}
		}
	}
	return Cvirtual_image(s, cx_s, cy_s, 1, palet);
}

void shp_split_frames(Cvirtual_image& image, int cblocks_x, int cblocks_y)
{
	int cx = image.cx() / cblocks_x;
	int cy = image.cy() / cblocks_y;
	int cx_d = image.cx() / cblocks_x;
	int cy_d = image.cy() * cblocks_x;
	byte* d = new byte[cx_d * cy_d];
	byte* w = d;
	const byte* r_line = image.image();
	for (int yb = 0; yb < cblocks_y; yb++)
	{
		for (int xb = 0; xb < cblocks_x; xb++)
		{
			const byte* r = r_line + cx * xb;
			for (int y = 0; y < cy; y++)
			{
				memcpy(w, r, cx);
				r += image.cx();
				w += cx_d;
			}
		}
		r_line += image.cx() * cy;
	}
	image.load(d, cx_d, cy_d, image.cb_pixel(), image.palet());
	delete[] d;
}

void shp_split_shadows(Cvirtual_image& image)
{
	int cx = image.cx();
	int cy = image.cy();
	int count = cx * cy;
	byte* d = new byte[count << 1];
	memcpy(d, image.image(), count); 
	byte* r = d;
	byte* w = d + count;
	while (count--)
	{
		byte& v = *r++;
		if (v == 4)
		{
			v = 0;
			*w++ = 1;
		}
		else
			*w++ = 0;
	}
	image.load(d, cx, cy << 1, image.cb_pixel(), image.palet());
	delete[] d;
}
#endif

/*
void shp_xor_decode_frames(Cvirtual_image& image, int c_frames)
{
	int cx = image.cx();
	int cy = image.cy() / c_frames;
	int count = cx * cy * (c_frames - 1);
	const byte* r = image.image();
	byte* w = image.image_edit() + cx * cy;
	while (count--)
	{
		*w++ ^= *r++;
	}
}

void shp_xor_encode_frames(Cvirtual_image& image, int c_frames)
{
	int cx = image.cx();
	int cy = image.cy() / c_frames;
	int count = cx * cy * c_frames;
	byte* w = image.image_edit() + count;
	count -= cx * cy;
	const byte* r = image.image() + count;
	while (count--)
	{
		*--w ^= *--r;
	}
}
*/

static int get_left_margin(const byte* r, int cx)
{
	int c = 0;
	while (cx-- && !*r++)
		c++;
	return c;
}

static int get_right_margin(const byte* r, int cx)
{
	int c = 0;
	while (cx-- && !*--r)
		c++;
	return c;
}

static int encode4_line(const byte* r, byte* d, int cx)
{
	const byte* s_end = r + cx;
	byte* w = d;
	while (r < s_end)
	{
		int v = *w++ = *r++;
		if (!v)
		{
			int c = min(get_run_length(r - 1, s_end), 0xff);
			r += c - 1;
			*w++ = c;
		}
	}
	return w - d;
}

static int decode4_line_size(const byte*& r, int cx)
{
	int w = 0;
	while (cx--)
	{
		w++;
		if (!*r++)
		{
			cx -= *r++ - 1;
			w++;
		}
	}
	return w;
}

static int decode4_line(const byte* s, byte*& w, int cx)
{
	const byte* r = s;
	while (cx--)
	{
		if (!(*w++ = *r++))
			cx -= (*w++ = *r++) - 1;
	}
	return r - s;
}

static int encode4(const byte* s, byte* d, int cx, int cy)
{
	const byte* s_end = s + cx * cy;
	const byte* r = s;
	byte* w = d;
	for (int y = 0; y < cy; y++)
	{
		int lm = min(get_left_margin(r, cx), 0xff);
		int rm = min(get_right_margin(r + cx, cx - lm), 0xff);
		*w++ = lm;
		*w++ = rm;
		w += encode4_line(r + lm, w, cx - lm - rm);
		r += cx;
	}
	return w - d;
}

static int decode4_size(const byte*& r, int cx, int cy)
{
	int w = 0;
	for (int y = 0; y < cy; y++)
	{
		int lm = *r++;
		int rm = *r++;
		w += 2;
		if (lm)
			w += 2;
		w += decode4_line_size(r, cx - lm - rm);
		if (rm)
			w += 2;
	}
	return w;
}

static int decode4(const byte* s, byte*& w, int cx, int cy)
{
	const byte* r = s;
	for (int y = 0; y < cy; y++)
	{
		int lm = *r++;
		int rm = *r++;
		byte* w_line = w;
		w += 2;
		if (lm)
		{
			*w++ = 0;
			*w++ = lm;
		}
		r += decode4_line(r, w, cx - lm - rm);
		if (rm)
		{
			*w++ = 0;
			*w++ = rm;
		}
		*reinterpret_cast<unsigned __int16*>(w_line) = w - w_line;
	}
	return r - s;
}

struct t_shp4_header
{
	unsigned __int16 cx;
	unsigned __int16 cy;
	unsigned __int16 c_frames;
};

struct t_shp4_frame_header
{
	unsigned __int8 lm;
	unsigned __int8 rm;
	unsigned __int8 tm;
	unsigned __int8 bm;
};

int shp_encode4(const Cshp_ts_file& f, byte* d)
{
	const int global_cx = f.cx();
	const int global_cy = f.cy();
	const int c_frames = f.cf();

	byte* w = d;
	t_shp4_header& header = *reinterpret_cast<t_shp4_header*>(w);
	header.cx = global_cx;
	header.cy = global_cy;
	header.c_frames = c_frames;
	w += sizeof(t_shp4_header);

	for (int i = 0; i < c_frames; i++)
	{
		const t_shp_ts_image_header& image_header = *f.get_image_header(i);

		const int cx = image_header.cx;
		const int cy = image_header.cy;

		t_shp4_frame_header& frame_header = *reinterpret_cast<t_shp4_frame_header*>(w);
		if (image_header.cx && image_header.cy)
		{
			frame_header.lm = image_header.x;
			frame_header.tm = image_header.y;
		}
		else
		{
			frame_header.lm = min(global_cx, 0xff);
			frame_header.tm = min(global_cy, 0xff);
		}
		if (global_cx - frame_header.lm - cx > 0xff
			|| global_cy - frame_header.tm - cy > 0xff)
			return 0;
		frame_header.rm = global_cx - frame_header.lm - cx;
		frame_header.bm = global_cy - frame_header.tm - cy;
		w += sizeof(t_shp4_frame_header);

		if (f.is_compressed(i))
		{
			Cvirtual_binary image;
			decode3(f.get_image(i), image.write_start(cx * cy), cx, cy);
			w += encode4(image.data(), w, cx, cy);
		}
		else
			w += encode4(f.get_image(i), w, cx, cy);
	}
	return w - d;
}

int shp_decode4_size(const byte* s)
{
	Cvirtual_binary d;
	const byte* r = s;
	const t_shp4_header& s_header = *reinterpret_cast<const t_shp4_header*>(r);
	const int global_cx = s_header.cx;
	const int global_cy = s_header.cy;
	const int c_frames = s_header.c_frames;
	r += sizeof(t_shp4_header);
	int w = 0;
	for (int i = 0; i < c_frames; i++)
	{
		const t_shp4_frame_header& frame_header = *reinterpret_cast<const t_shp4_frame_header*>(r);
		int x = frame_header.lm;
		int y = frame_header.tm;
		int cx = global_cx - x - frame_header.rm;
		int cy = global_cy - y - frame_header.bm;
		r += sizeof(t_shp4_frame_header);
		if (cy)
			w += decode4_size(r, cx, cy);
		w = w + 7 & ~7;
	}
	return w
		+ sizeof(t_shp_ts_header)
		+ c_frames * sizeof(t_shp_ts_image_header);
}

Cvirtual_binary shp_decode4(const byte* s, int cb_d)
{
	Cvirtual_binary d;
	const byte* r = s;
	const t_shp4_header& s_header = *reinterpret_cast<const t_shp4_header*>(r);
	const int global_cx = s_header.cx;
	const int global_cy = s_header.cy;
	const int c_frames = s_header.c_frames;
	r += sizeof(t_shp4_header);
	byte* w = d.write_start(cb_d ? cb_d : Cshp_ts_file::get_max_size(global_cx, global_cy, c_frames));
	t_shp_ts_header& header = *reinterpret_cast<t_shp_ts_header*>(w);
	header.zero = 0;
	header.cx = global_cx;
	header.cy = global_cy;
	header.c_images = c_frames;
	w += sizeof(t_shp_ts_header);
	byte* w1 = w + c_frames * sizeof(t_shp_ts_image_header);
	for (int i = 0; i < c_frames; i++)
	{
		const t_shp4_frame_header& frame_header = *reinterpret_cast<const t_shp4_frame_header*>(r);
		int x = frame_header.lm;
		int y = frame_header.tm;
		int cx = global_cx - x - frame_header.rm;
		int cy = global_cy - y - frame_header.bm;
		r += sizeof(t_shp4_frame_header);
		t_shp_ts_image_header& image_header = *reinterpret_cast<t_shp_ts_image_header*>(w);
		image_header.x = x;
		image_header.y = y;
		image_header.cx = cx;
		image_header.cy = cy;
		image_header.compression = 3;
		image_header.unknown = 0;
		image_header.zero = 0;
		image_header.offset = w1 - d.data();
		w += sizeof(t_shp_ts_image_header);
		if (cy)
			r += decode4(r, w1, cx, cy);
		else
			image_header.offset = 0;
		w1 = d.data_edit() + (w1 - d.data() + 7 & ~7);
	}
	assert(!cb_d || d.size() == w1 - d.data());
	return cb_d ? d : Cvirtual_binary(d.data(), w1 - d.data());
}
