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

#include "cc_file.h"
#include "palet.h"
#include "virtual_file.h"

class Cvirtual_image
{
public:
	const Cvirtual_image& palet(const t_palet_entry* palet, bool inflate = false);
	void remove_alpha();
	void add_alpha();
	void cb_pixel(int cb_pixel, const t_palet_entry* palet = NULL);
	void decrease_color_depth(int cb_pixel, const t_palet_entry* palet = NULL);
	void increase_color_depth(int cb_pixel);
	void increase_palet_depth();
	void flip();
	int get_clipboard();
	int set_clipboard() const;
	int load();
	int save() const;
	void load(const Cvirtual_binary& image, int cx, int cy, int cb_pixel, const t_palet_entry* palet, bool inflate = false);
	void load(const void* image, int cx, int cy, int cb_pixel, const t_palet_entry* palet, bool inflate = false);
	int load(const Cvirtual_binary& s);
	int load(const Cvirtual_file& f);
	int load(const string& fname);
	int save(Cvirtual_file& f, t_file_type ft) const;
	Cvirtual_file save(t_file_type ft) const;
	int save(const string& fname, t_file_type ft) const;
	void swap_rb();
	Cvirtual_image() = default;
	Cvirtual_image(const Cvirtual_binary& image, int cx, int cy, int cb_pixel, const t_palet_entry* palet = NULL, bool inflate = false);
	Cvirtual_image(const void* image, int cx, int cy, int cb_pixel, const t_palet_entry* palet = NULL, bool inflate = false);

	const byte* image() const
	{
		return m_image.data();
	}

	byte* image_edit()
	{
		return m_image.data_edit();
	}

	int cx() const
	{
		return m_cx;
	}

	int cy() const
	{
		return m_cy;
	}

	int cb_pixel() const
	{
		return mcb_pixel;
	}

	const t_palet_entry* palet() const
	{
		return reinterpret_cast<const t_palet_entry*>(m_palet.data());
	}

	int cb_image() const
	{
		return m_cx * m_cy * mcb_pixel;
	}

	int ofs8(int x, int y) const
	{
		return x + m_cx * y;
	}

	int ofs24(int x, int y) const
	{
		return ofs8(x, y) * 3;
	}

	int pixel8(int x, int y) const
	{
		return m_image.data()[ofs8(x, y)];
	}

	void pixel8(int x, int y, int v)
	{
		m_image.data_edit()[ofs8(x, y)] = v;
	}

	t_palet_entry pixel24(int x, int y) const
	{
		return reinterpret_cast<const t_palet_entry*>(m_image.data())[ofs8(x, y)];
	}
	
	void pixel24(int x, int y, t_palet_entry v)
	{
		reinterpret_cast<t_palet_entry*>(m_image.data_edit())[ofs8(x, y)] = v;
	}
private:
	Cvirtual_binary m_image;
	Cvirtual_binary m_palet;
	int m_cx = 0;
	int m_cy = 0;
	int mcb_pixel = 0;
};

