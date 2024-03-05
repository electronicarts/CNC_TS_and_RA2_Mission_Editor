/*
	FinalSun/FinalAlert 2 Mission Editor

	Copyright (C) 1999-2024 Electronic Arts, Inc.
	Authored by Matthias Wagner

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

#include "StdAfx.h"
#include "LineDrawer.h"
#include <cassert>
#include <memory>

static int ModLookup[] = {
	1,
	2,
	3,
	4
};

LineDrawer::LineDrawer(void* dest, int bytes_per_pixel, int width, int height, int pitch):
	m_dest(dest),
	m_bytes_per_pixel(bytes_per_pixel),
	m_width(width),
	m_height(height),
	m_pitch(pitch),
	m_last_x(0),
	m_last_y(0)
{
}

void LineDrawer::MoveTo(int to_x, int to_y)
{
	m_last_x = to_x;
	m_last_y = to_y;
}

void LineDrawer::LineTo(int to_x, int to_y, int color, LineStyle style)
{
	DrawLine(m_last_x, m_last_y, to_x, to_y, color, style);
}

void LineDrawer::DrawLine(int from_x, int from_y, int to_x, int to_y, int color, LineStyle style)
{
	const int w = to_x - from_x;
	const int h = to_y - from_y;
	m_last_x = to_x;
	m_last_y = to_y;
	if (abs(w) >= abs(h)) {
		return w >= 0 ? DrawLineImplX(from_x, from_y, to_x, to_y, color, style) : DrawLineImplX(to_x, to_y, from_x, from_y, color, style);
	}
	else {
		return h >= 0 ? DrawLineImplY(from_x, from_y, to_x, to_y, color, style) : DrawLineImplY(to_x, to_y, from_x, from_y, color, style);
	}
}

void LineDrawer::Rectangle(int from_x, int from_y, int to_x, int to_y, int color, LineStyle style)
{
	MoveTo(from_x, from_y);
	LineTo(to_x, from_y, color, style);
	LineTo(to_x, to_y, color, style);
	LineTo(from_x, to_y, color, style);
	LineTo(from_x, from_y, color, style);
}

void LineDrawer::SetPixel(char* const dest, const int cur_x, const int cur_y, const int color)
{
	if (cur_x >= 0 && cur_y >= 0 && cur_x < m_width && cur_y < m_height)
		memcpy(&dest[cur_x * m_bytes_per_pixel + cur_y * m_pitch], &color, m_bytes_per_pixel);
}

void LineDrawer::DrawLineImplX(const int from_x, const int from_y, const int to_x, const int to_y, const int color, const LineStyle style)
{
    // X major line drawing

	assert (from_x <= to_x);
	const int w = to_x - from_x;
	const int h = abs(to_y - from_y);
	assert (w >= h);
	const int increment_e = 2 * h;
	const int increment_ne = 2 * (h - w);
	const int y_inc = (to_y - from_y) < 0 ? -1 : 1;
	int d = 2 * h - w;
	int cur_x = from_x;
	int cur_y = from_y;
	const int mod = ModLookup[static_cast<int>(style)];

	char* dest = reinterpret_cast<char*>(m_dest);
	SetPixel(dest, cur_x, cur_y, color);

	for(; cur_x < to_x; ++cur_x)
	{
		if (d < 0) {
			d += increment_e;
		}
		else {
			d += increment_ne;
			cur_y += y_inc;
		}
		if (mod == 1 || (cur_x - from_x) % mod == 0)
			SetPixel(dest, cur_x, cur_y, color);
	}
}

void LineDrawer::DrawLineImplY(const int from_x, const int from_y, const int to_x, const int to_y, const int color, LineStyle style)
{
	// Y major line drawing

	assert(from_y <= to_y);
	const int w = abs(to_x - from_x);
	const int h = to_y - from_y;
	assert(h >= w);
	const int increment_e = 2 * w;
	const int increment_ne = 2 * (w - h);
	const int x_inc = (to_x - from_x) < 0 ? -1 : 1;
	int d = 2 * w - h;
	int cur_x = from_x;
	int cur_y = from_y;
	const int mod = ModLookup[static_cast<int>(style)];

	char* dest = reinterpret_cast<char*>(m_dest);
	SetPixel(dest, cur_x, cur_y, color);

	for (; cur_y < to_y; ++cur_y)
	{
		if (d < 0) {
			d += increment_e;
		}
		else {
			d += increment_ne;
			cur_x += x_inc;
		}
		if (mod == 1 || (cur_y - from_y) % mod == 0)
			SetPixel(dest, cur_x, cur_y, color);
	}
}
