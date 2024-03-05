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

#include <ddraw.h>
#include <afx.h>
#include <string>
#include "Vec2.h"

// Very simple class that renders text through a prepared DirectDraw surface
// Only supports ASCII chars 32 to 126, only supports monospace font
// This is for rendering waypoint ids or credits on map without using GDI during rendering, nothing fancy
class TextDrawer
{
public:
    TextDrawer(IDirectDraw4* pDirectDraw, int fontSizeInPoints, COLORREF col, COLORREF shadowCol=CLR_INVALID);

    bool isValid() const;

    void RenderText(IDirectDrawSurface4* target, int x, int y, const std::string& text, bool centered=false) const;

    ProjectedVec GetExtent(const std::string& text) const;

private:
    CComPtr<IDirectDrawSurface4> m_fontSurface;
    ProjectedVec m_charExtent;
    int m_fontSizeInPoints;
    int m_fontSizeInPixels;
    COLORREF m_col;
    COLORREF m_shadowCol;
};