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

#include <ddraw.h>

class Cddpf_conversion
{
public:
	int get_color(int a, int r, int g, int b);
	int get_color(int r, int g, int b);
	int get_color(int v);
	void set_pf(const DDPIXELFORMAT& pf);
private:
	int a_shift, r_shift, g_shift, b_shift;
	int a_size, r_size, g_size, b_size;
};