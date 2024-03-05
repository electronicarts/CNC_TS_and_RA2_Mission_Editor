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

#pragma once

#include <cinttypes>

enum class LineStyle: int
{
	Standard,
	Dotted_2,
	Dotted_3,
	Dotted_4,
};

class LineDrawer
{
public:
	LineDrawer(void* dest, int bytes_per_pixel, int width, int height, int pitch);

	void MoveTo(int to_x, int to_y);
	void LineTo(int to_x, int to_y, int color, LineStyle style = LineStyle::Standard);
	void DrawLine(int from_x, int from_y, int to_x, int to_y, int color, LineStyle style = LineStyle::Standard);
	void Rectangle(int from_x, int from_y, int to_x, int to_y, int color, LineStyle style = LineStyle::Standard);

private:
	void SetPixel(char* dest, int cur_x, int cur_y, int color);
	void DrawLineImplX(int from_x, int from_y, int to_x, int to_y, int color, LineStyle style);
	void DrawLineImplY(int from_x, int from_y, int to_x, int to_y, int color, LineStyle style);

private:
	void* m_dest;
	int m_bytes_per_pixel;
	int m_width;
	int m_height;
	int m_pitch;
	int m_last_x;
	int m_last_y;
};