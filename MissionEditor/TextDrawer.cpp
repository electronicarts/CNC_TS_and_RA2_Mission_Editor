#include "TextDrawer.h"
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

#include "stdafx.h"
#include "TextDrawer.h"
#include <afxwin.h>
#include "Vec2.h"
#include "MissionEditorPackLib.h"

TextDrawer::TextDrawer(IDirectDraw4* pDirectDraw, int fontSizeInPoints, COLORREF col, COLORREF shadowCol): m_fontSizeInPoints(fontSizeInPoints), m_col(col), m_shadowCol(shadowCol)
{
    auto dc = CDC::FromHandle(::GetDC(NULL));
    auto fontSizeInPixels = -MulDiv(fontSizeInPoints, dc->GetDeviceCaps(LOGPIXELSY), 72);
    m_fontSizeInPixels = fontSizeInPixels;

    CFont f;
    f.CreateFont(fontSizeInPixels, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, VARIABLE_PITCH, "COURIER NEW");

    // Build a string that contains all required characters in order
    std::string s;
    for (char c = 32; c <= 126; ++c)
        s.push_back(c);

    // get the extent in pixels of all characters
    dc->SelectObject(f);
    const auto extent = dc->GetTextExtent(s.c_str(), s.size());

    // Now create the DirectDraw surface
    DDSURFACEDESC2 desc = { 0 };
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    desc.dwWidth = extent.cx;
    desc.dwHeight = extent.cy * 2;

    m_charExtent.set(extent.cx / s.size(), extent.cy);

    auto bkcol = col == RGB(10, 10, 10) ? RGB(11, 11, 11) : RGB(10, 10, 10);

    auto pSurface = CComPtr<IDirectDrawSurface4>();
    if (pDirectDraw->CreateSurface(&desc, &pSurface, nullptr) != DD_OK)
        return;

    desc.dwFlags |= DDSD_PIXELFORMAT;
    pSurface->GetSurfaceDesc(&desc);
    if (pSurface->Lock(NULL, &desc, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL) == DD_OK)
    {
        FSunPackLib::ColorConverter c(desc.ddpfPixelFormat);
        std::int32_t backcolor = c.GetColor(bkcol);
        auto bytes_per_pixel = (desc.ddpfPixelFormat.dwRGBBitCount + 7) / 8;
        BYTE* const pImage = static_cast<BYTE*>(desc.lpSurface);
        for (int i=0; i < desc.dwWidth; ++i)
        {
            for (int e = 0; e < desc.dwHeight; ++e)
            {
                memcpy(&pImage[e * desc.lPitch + i * bytes_per_pixel], &backcolor, bytes_per_pixel);
            }
        }
        pSurface->Unlock(NULL);
    }


    
  
    HDC hDC;
    if (pSurface->GetDC(&hDC) != DD_OK)
        return;

    // Draw the string with all characters onto the surface
    SelectObject(hDC, f);
    SetBkMode(hDC, TRANSPARENT);
    if (shadowCol != CLR_INVALID)
    {
        SetTextColor(hDC, shadowCol);
        if (!TextOutA(hDC, 0, extent.cy, s.c_str(), s.size()))
            return;
    }
    SetTextColor(hDC, col);
    if (!TextOutA(hDC, 0, 0, s.c_str(), s.size()))
        return;

    if (pSurface->ReleaseDC(hDC) != DD_OK)
        return;

    // set transparency key to top left
    FSunPackLib::SetColorKey(pSurface, CLR_INVALID);
   
    // Everything fine, pass ownership of surface to m_fontSurface
    m_fontSurface.Attach(pSurface.Detach());
}

bool TextDrawer::isValid() const
{
    return m_fontSurface != nullptr;
}

void TextDrawer::RenderText(IDirectDrawSurface4* target, int x, int y, const std::string& text, bool centered) const
{
    if (!isValid())
        return;

    auto shadowOffset = 1 + m_fontSizeInPixels / 32;

    const int lineOffset = m_charExtent.y / 4;
    ProjectedVec cur(x, y);
    const int cw = m_charExtent.x;
    const int ch = m_charExtent.y;

    if (centered)
    {
        cur -= GetExtent(text) / 2;
    }

    for (const auto c: text)
    {
        if (c == '\n')
        {
            cur.set(x, cur.y + ch + lineOffset);
        }
        else if (c >= 32 && c <= 126)
        {
            auto i = c - 32;

            
            if (m_shadowCol != CLR_INVALID)
            {
                RECT s_shadow{ i * cw, ch, i * cw + cw, ch + ch };
                
                target->BltFast(cur.x + 0 * shadowOffset, cur.y + 1 * shadowOffset, m_fontSurface, &s_shadow, DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT);
                target->BltFast(cur.x + 0 * shadowOffset, cur.y - 1 * shadowOffset, m_fontSurface, &s_shadow, DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT);
                target->BltFast(cur.x + 1 * shadowOffset, cur.y + 0 * shadowOffset, m_fontSurface, &s_shadow, DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT);
                target->BltFast(cur.x - 1 * shadowOffset, cur.y + 0 * shadowOffset, m_fontSurface, &s_shadow, DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT);

                target->BltFast(cur.x + 1 * shadowOffset, cur.y + 1 * shadowOffset, m_fontSurface, &s_shadow, DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT);
                target->BltFast(cur.x - 1 * shadowOffset, cur.y + 1 * shadowOffset, m_fontSurface, &s_shadow, DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT);
                target->BltFast(cur.x + 1 * shadowOffset, cur.y - 1 * shadowOffset, m_fontSurface, &s_shadow, DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT);
                target->BltFast(cur.x - 1 * shadowOffset, cur.y - 1 * shadowOffset, m_fontSurface, &s_shadow, DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT);
            }

            RECT s{ i * cw, 0, i * cw + cw, ch };
            target->BltFast(cur.x, cur.y, m_fontSurface, &s, DDBLTFAST_SRCCOLORKEY | DDBLTFAST_WAIT);           
            cur.x += cw;
        }
    }
}

ProjectedVec TextDrawer::GetExtent(const std::string& text) const
{
    ProjectedVec cur(0, 0);
    const int lineOffset = m_charExtent.y / 4;
    const int cw = m_charExtent.x;
    const int ch = m_charExtent.y;
    ProjectedVec maxpos(0, 0);
    for (const auto c : text)
    {
        if (c == '\n')
        {
            cur.set(0, cur.y + ch + lineOffset);
        }
        else if (c >= 32 && c <= 126)
        {
            cur.x += cw;
            maxpos.set(max(maxpos.x, cur.x), max(maxpos.y, cur.y + ch));
        }
        
    }
    return maxpos;
}
