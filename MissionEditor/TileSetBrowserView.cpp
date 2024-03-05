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

// TileSetBrowserView.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "TileSetBrowserView.h"
#include "FinalSunDlg.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"

extern ACTIONDATA AD;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTileSetBrowserView

IMPLEMENT_DYNCREATE(CTileSetBrowserView, CScrollView)

CTileSetBrowserView::CTileSetBrowserView()
{
	m_lpDDS = NULL;
	m_bottom_needed = 1000;
	m_CurrentMode = 0;
}

CTileSetBrowserView::~CTileSetBrowserView()
{
	if (m_lpDDS)
	{
		int i;
		for (i = 0;i < m_tilecount;i++)
		{
			if (m_lpDDS[i]) m_lpDDS[i]->Release();
		}
		delete[] m_lpDDS;
	}
	m_lpDDS = NULL;

}


BEGIN_MESSAGE_MAP(CTileSetBrowserView, CScrollView)
	//{{AFX_MSG_MAP(CTileSetBrowserView)
	ON_WM_LBUTTONDOWN()
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Zeichnung CTileSetBrowserView 

void CTileSetBrowserView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();


}

// for fast overlay drawing use IsoView´s overlay table
extern PICDATA* ovrlpics[0xFF][max_ovrl_img];



void CTileSetBrowserView::OnDraw(CDC* pDC)
{
	//ReleaseDC(pDC);



	if (((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->b_IsLoading || ((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->lpds == NULL || ((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->lpds->IsLost() != DD_OK)
		return;


	RECT r;
	GetClientRect(&r);

	if (tiledata == NULL || (*tiledata) == NULL)
		return;

	if (m_tilecount == 0)
		return;

	if (m_tile_width == 0)
		return; // just to make sure I never divide through 0 here...

	int max_r = r.right / m_tile_width;

	int cur_y = 0;
	int cur_x = 0;

	if (m_CurrentMode == 1)
	{
		DWORD dwID = GetTileID(m_currentTileSet, 0);

		int i;
		for (i = 0;i < m_tilecount;i++)
		{
			char c[50];
			itoa(i, c, 10);

			int curwidth = (*tiledata)[dwID].rect.right - (*tiledata)[dwID].rect.left;
			int curheight = GetAddedHeight(dwID) + (*tiledata)[dwID].rect.bottom - (*tiledata)[dwID].rect.top;
			//pDC.TextOut(cur_x, cur_y, c);

#ifdef RA2_MODE
			if ((m_currentTileSet == 80 && Map->GetTheater() == "TEMPERATE") || (m_currentTileSet == 73 && Map->GetTheater() == "SNOW") || (m_currentTileSet == 101 && Map->GetTheater() == "URBAN"))
			{
				if (i == 10 || i == 15)
				{
					dwID++; // don´t forget this here, too
					continue;
				}
			}
#endif

			if (!m_lpDDS[i]) continue;

			RECT r;
			GetClientRect(&r);
			if (cur_y + curheight + (m_tile_height - curheight) / 2 >= this->GetScrollPos(SB_VERT) && cur_y <= GetScrollPos(SB_VERT) + r.bottom)
			{

				HDC hDC = NULL;
				m_lpDDS[i]->GetDC(&hDC);


				HDC hTmpDC = CreateCompatibleDC(hDC);
				HBITMAP hBitmap = CreateCompatibleBitmap(hDC, curwidth, curheight);
				SelectObject(hTmpDC, hBitmap);

				BitBlt(hTmpDC, 0, 0, curwidth, curheight, hDC, 0, 0, SRCCOPY);

				m_lpDDS[i]->ReleaseDC(hDC);


				BitBlt(pDC->GetSafeHdc(), cur_x + (m_tile_width - curwidth) / 2, cur_y + (m_tile_height - curheight) / 2, curwidth, curheight, hTmpDC, 0, 0, SRCCOPY);


				DeleteDC(hTmpDC);
				DeleteObject(hBitmap);

				if (AD.mode == ACTIONMODE_SETTILE && AD.type == dwID)
				{
					CPen p;
					CBrush b;
					p.CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
					b.CreateStockObject(NULL_BRUSH);

					CPen* old = pDC->SelectObject(&p);

					pDC->SetBkMode(TRANSPARENT);
					pDC->SelectObject(&b);
					pDC->Rectangle(cur_x + 2, cur_y + 2, cur_x + m_tile_width - 2, cur_y + m_tile_height - 2);

					pDC->SelectObject(old);
				}
			}

			cur_x += m_tile_width;
			if (max_r == 0) max_r = 1;
			if (i % max_r == max_r - 1)
			{
				cur_y += m_tile_height;
				cur_x = 0;
			}



			dwID++;
		}
	}
#ifndef NOSURFACES
	else if (m_CurrentMode == 2)
	{
		int i;

		for (i = 0;i < max_ovrl_img;i++)
		{
			PICDATA* p = ovrlpics[m_currentOverlay][i];
			if (p != NULL && p->pic != NULL)
			{

				DDSURFACEDESC2 desc;
				memset(&desc, 0, sizeof(DDSURFACEDESC2));
				desc.dwSize = sizeof(DDSURFACEDESC2);
				desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
				p->pic->GetSurfaceDesc(&desc);

				int curwidth = desc.dwWidth;
				int curheight = desc.dwHeight;

				HDC hDC = NULL;
				p->pic->GetDC(&hDC);


				HDC hTmpDC = CreateCompatibleDC(hDC);
				HBITMAP hBitmap = CreateCompatibleBitmap(hDC, curwidth, curheight);
				SelectObject(hTmpDC, hBitmap);

				BitBlt(hTmpDC, 0, 0, curwidth, curheight, hDC, 0, 0, SRCCOPY);

				p->pic->ReleaseDC(hDC);


				BitBlt(pDC->GetSafeHdc(), cur_x + (m_tile_width - curwidth) / 2, cur_y + (m_tile_height - curheight) / 2, curwidth, curheight, hTmpDC, 0, 0, SRCCOPY);


				DeleteDC(hTmpDC);
				DeleteObject(hBitmap);

				if (AD.mode == ACTIONMODE_PLACE && AD.data2 == m_currentOverlay && AD.data3 == i && AD.data == 33 && AD.type == 6)
				{
					CPen p;
					CBrush b;
					p.CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
					b.CreateStockObject(NULL_BRUSH);

					CPen* old = pDC->SelectObject(&p);

					pDC->SetBkMode(TRANSPARENT);
					pDC->SelectObject(&b);
					pDC->Rectangle(cur_x + 2, cur_y + 2, cur_x + m_tile_width - 2, cur_y + m_tile_height - 2);

					pDC->SelectObject(old);
				}

				cur_x += m_tile_width;
				if (max_r == 0) max_r = 1;
				if (i % max_r == max_r - 1)
				{
					cur_y += m_tile_height;
					cur_x = 0;
				}
			}
		}
	}
#else
	else if (m_CurrentMode == 2)
	{
		int i;

		for (i = 0;i < max_ovrl_img;i++)
		{
			PICDATA* p = ovrlpics[m_currentOverlay][i];
			if (p != NULL && p->pic != NULL)
			{

				int curwidth = p->wMaxWidth;
				int curheight = p->wMaxHeight;

				BITMAPINFO biinfo;
				memset(&biinfo, 0, sizeof(BITMAPINFO));
				biinfo.bmiHeader.biBitCount = 24;
				biinfo.bmiHeader.biWidth = curwidth;
				biinfo.bmiHeader.biHeight = curheight;
				biinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				biinfo.bmiHeader.biClrUsed = 0;
				biinfo.bmiHeader.biPlanes = 1;
				biinfo.bmiHeader.biCompression = BI_RGB;
				biinfo.bmiHeader.biClrImportant = 0;

				int pitch = curwidth * 3;
				if (pitch == 0)
					continue;

				if (pitch % sizeof(DWORD))
				{
					pitch += sizeof(DWORD) - (curwidth * 3) % sizeof(DWORD);
				}

				BYTE* colors = new(BYTE[pitch * curheight]);
				memset(colors, 255, pitch * (curheight));

				RGBTRIPLE* pal = palIso;
				if (p->pal == iPalTheater)
					pal = palTheater;
				if (p->pal == iPalUnit)
					pal = palUnit;

				int k, l;
				for (k = 0;k < curheight;k++)
				{
					for (l = 0;l < curwidth;l++)
					{
						if (((BYTE*)p->pic)[l + k * curwidth])
						{
							memcpy(&colors[l * 3 + (curheight - k - 1) * pitch], &pal[((BYTE*)p->pic)[l + k * curwidth]], 3);
						}
					}
				}

				StretchDIBits(pDC->GetSafeHdc(), cur_x + (m_tile_width - curwidth) / 2, cur_y + (m_tile_height - curheight) / 2, curwidth, curheight,
					0, 0, curwidth, curheight, colors, &biinfo, DIB_RGB_COLORS, SRCCOPY);

				delete[] colors;

				if (AD.mode == ACTIONMODE_PLACE && AD.data2 == m_currentOverlay && AD.data3 == i && AD.data == 33 && AD.type == 6)
				{
					CPen p;
					CBrush b;
					p.CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
					b.CreateStockObject(NULL_BRUSH);

					CPen* old = pDC->SelectObject(&p);

					pDC->SetBkMode(TRANSPARENT);
					pDC->SelectObject(&b);
					pDC->Rectangle(cur_x + 2, cur_y + 2, cur_x + m_tile_width - 2, cur_y + m_tile_height - 2);

					pDC->SelectObject(old);
				}

				cur_x += m_tile_width;
				if (max_r == 0)
					max_r = 1;
				if (i % max_r == max_r - 1)
				{
					cur_y += m_tile_height;
					cur_x = 0;
				}
			}
		}
	}
#endif


}

/////////////////////////////////////////////////////////////////////////////
// Diagnose CTileSetBrowserView

#ifdef _DEBUG
void CTileSetBrowserView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CTileSetBrowserView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CTileSetBrowserView 

void CTileSetBrowserView::PostNcDestroy()
{
	// TODO: Speziellen Code hier einfügen und/oder Basisklasse aufrufen

	// CScrollView::PostNcDestroy();
}

DWORD CTileSetBrowserView::GetTileID(DWORD dwTileSet, DWORD dwType)
{
	int i, e;
	DWORD tilecount = 0;
	for (i = 0;i < 10000;i++)
	{
		CString tset;
		char c[50];
		itoa(i, c, 10);
		int e;
		for (e = 0;e < 4 - strlen(c);e++)
			tset += "0";
		tset += c;
		CString sec = "TileSet";
		sec += tset;

		if (tiles->sections.find(sec) == tiles->sections.end())
			return 0xFFFFFFFF;


		for (e = 0;e < atoi(tiles->sections[sec].values["TilesInSet"]);e++)
		{
			if (i == dwTileSet && e == dwType)
				return tilecount;
			tilecount++;

		}


	}

	return tilecount;
}

void CTileSetBrowserView::SetTileSet(DWORD dwTileSet, BOOL bOnlyRedraw)
{
	m_currentTileSet = dwTileSet;
	m_CurrentMode = 1;

	char currentTileSet[50];
	itoa(m_currentTileSet, currentTileSet, 10);
	CString tset;

	int e;
	for (e = 0;e < 4 - strlen(currentTileSet);e++)
		tset += "0";

	tset += currentTileSet;

	m_tile_width = 0;
	m_tile_height = 0;

	int i;
	int max = atoi(tiles->sections[(CString)"TileSet" + tset].values["TilesInSet"]);
	DWORD dwStartID = GetTileID(dwTileSet, 0);
	if ((*tiledata)[dwStartID].wTileCount && (*tiledata)[dwStartID].tiles[0].pic)
	{
		if (!bOnlyRedraw)
		{
			AD.mode = ACTIONMODE_SETTILE;
			AD.type = dwStartID;
			AD.data = 0;
			AD.data2 = 0;
			AD.z_data = 0;

			((CFinalSunDlg*)theApp.m_pMainWnd)->m_settingsbar.m_BrushSize = 0;
			((CFinalSunDlg*)theApp.m_pMainWnd)->m_settingsbar.UpdateData(FALSE);
			((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_x = 1;
			((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_y = 1;

			int i;
			for (i = 0;i < g_data.sections["StdBrushSize"].values.size();i++)
			{
				CString n = *g_data.sections["StdBrushSize"].GetValueName(i);
				if ((*tiles).sections["General"].FindName(n) >= 0)
				{
					int tset = atoi((*tiles).sections["General"].values[n]);
					if (tset == m_currentTileSet)
					{
						int bs = atoi(*g_data.sections["StdBrushSize"].GetValue(i));
						((CFinalSunDlg*)theApp.m_pMainWnd)->m_settingsbar.m_BrushSize = bs - 1;
						((CFinalSunDlg*)theApp.m_pMainWnd)->m_settingsbar.UpdateData(FALSE);
						((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_x = bs;
						((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_y = bs;
					}
				}
			}
		}
	}
	DWORD dwID;
	for (i = 0;i < max;i++)
	{
		CString str;
		char c[50];
		itoa(i, c, 10);
		for (e = 0;e < 2 - strlen(c);e++)
			str += "0";
		str += c;

		dwID = dwStartID + i; // just faster than always calling GetTileID()
		if (dwID < *tiledata_count)
		{
			if ((*tiledata)[dwID].rect.right - (*tiledata)[dwID].rect.left > m_tile_width) m_tile_width = (*tiledata)[dwID].rect.right - (*tiledata)[dwID].rect.left;
			if (GetAddedHeight(dwID) + (*tiledata)[dwID].rect.bottom - (*tiledata)[dwID].rect.top > m_tile_height) m_tile_height = GetAddedHeight(dwID) + (*tiledata)[dwID].rect.bottom - (*tiledata)[dwID].rect.top;
		}
	}

	m_tile_width += 6;
	m_tile_height += 6;

	if (m_lpDDS)
	{
		int i;
		for (i = 0;i < m_tilecount;i++)
		{
			if (m_lpDDS[i]) m_lpDDS[i]->Release();
		}
		delete[] m_lpDDS;
	}

	m_tilecount = max;

	m_lpDDS = new(LPDIRECTDRAWSURFACE4[m_tilecount]);
	for (i = 0;i < m_tilecount;i++)
	{
		m_lpDDS[i] = RenderTile(dwStartID + i);
	}

	RECT r;
	GetClientRect(&r);
	int max_r = r.right / m_tile_width;
	if (max_r <= 0) max_r = 1;
	m_bottom_needed = m_tile_height * (1 + m_tilecount / max_r);
	GetParentFrame()->RecalcLayout(TRUE);


	RedrawWindow();

	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->SetForegroundWindow();
	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->SetFocus();

	//DrawIt();
}


#ifdef NOSURFACES

struct BlitRect
{
	short left;
	short top;
	short right;
	short bottom;
};

__forceinline void BlitTerrainTSB(void* dst, int x, int y, int dleft, int dtop, int dpitch, int dright, int dbottom, SUBTILE& st)//BYTE* src, int swidth, int sheight)
{
	BYTE* src = st.pic;
	unsigned short& swidth = st.wWidth;
	unsigned short& sheight = st.wHeight;


	if (src == NULL || dst == NULL)
		return;

	if (x + swidth < dleft || y + sheight < dtop)
		return;
	if (x >= dright || y >= dbottom)
		return;


	BlitRect blrect;
	BlitRect srcRect;
	srcRect.left = 0;
	srcRect.top = 0;
	srcRect.right = swidth;
	srcRect.bottom = sheight;
	blrect.left = x;
	if (blrect.left < 0)
	{
		srcRect.left = 1 - blrect.left;
		blrect.left = 1;
	}
	blrect.top = y;
	if (blrect.top < 0)
	{
		srcRect.top = 1 - blrect.top;
		blrect.top = 1;
	}
	blrect.right = (x + swidth);
	if (x + swidth > dright)
	{
		srcRect.right = dright - x;//swidth-((x+swidth)-dright);
		blrect.right = dright;
	}
	blrect.bottom = (y + sheight);
	if (y + sheight > dbottom)
	{
		srcRect.bottom = dbottom - y;//sheight-((y+sheight)-dbottom);
		blrect.bottom = dbottom;
	}


	short i, e;



#ifdef NOSURFACES_EXTRACT
	int pos = 0;
	if (!st.bNotExtracted)
	{
		for (e = srcRect.top;e < srcRect.bottom;e++)
		{
			short& left = st.vborder[e].left;
			short& right = st.vborder[e].right;

			if (right >= left)
			{
				void* dest = ((BYTE*)dst + (blrect.left + left) * bpp + (blrect.top + e) * dpitch);

				memcpy(dest, &st.pic[pos], bpp * (right - left + 1));
				pos += (right - left + 1) * bpp;
			}
		}
	}
	else

#endif

		for (e = srcRect.top;e < srcRect.bottom;e++)
		{
			short& left = st.vborder[e].left;
			short& right = st.vborder[e].right;

			for (i = left;i <= right;i++)
			{
				if (i < srcRect.left || i >= srcRect.right)
				{
					//dest+=bpp;
				}
				else
				{

					BYTE& val = src[i + e * swidth];
					if (val)
					{
						void* dest = ((BYTE*)dst + (blrect.left + i) * bpp + (blrect.top + e) * dpitch);

						memcpy(dest, &iPalIso[val], bpp);
					}
				}
			}

		}


}
#endif


LPDIRECTDRAWSURFACE4 CTileSetBrowserView::RenderTile(DWORD dwID)
{
	if (theApp.m_Options.bMarbleMadness)
	{
		if ((*tiledata)[dwID].wMarbleGround != 0xFFFF)
		{
			dwID = (*tiledata)[dwID].wMarbleGround;
		}
	}

	LPDIRECTDRAWSURFACE4 lpdds = NULL;
	LPDIRECTDRAW4 lpdd = ((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->dd;

	DDSURFACEDESC2 ddsd;
	memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	int added_height = GetAddedHeight(dwID);
	ddsd.dwHeight = (*tiledata)[dwID].rect.bottom - (*tiledata)[dwID].rect.top + added_height;
	ddsd.dwWidth = (*tiledata)[dwID].rect.right - (*tiledata)[dwID].rect.left;
	if (lpdd->CreateSurface(&ddsd, &lpdds, NULL) != DD_OK)
	{
		return NULL;
	}

	DDBLTFX ddfx;
	memset(&ddfx, 0, sizeof(DDBLTFX));
	ddfx.dwSize = sizeof(DDBLTFX);
	lpdds->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddfx);


	int y_added = ddsd.dwHeight - ((*tiledata)[dwID].cx * f_y / 2 + (*tiledata)[dwID].cy * f_y / 2);

	int i, e, p = 0;;
	for (i = 0;i < (*tiledata)[dwID].cx;i++)
	{
		for (e = 0;e < (*tiledata)[dwID].cy;e++)
		{
			int drawx = e * f_x / 2 - i * f_x / 2 - (*tiledata)[dwID].rect.left;
			int drawy = e * f_y / 2 + i * f_y / 2 - (*tiledata)[dwID].rect.top;

			drawx += (*tiledata)[dwID].tiles[p].sX;
			drawy += added_height + (*tiledata)[dwID].tiles[p].sY - (*tiledata)[dwID].tiles[p].bZHeight * f_y / 2;
			//drawy+=y_added;

			if ((*tiledata)[dwID].tiles[p].pic)
			{
				RECT dest;
				dest.left = drawx;
				dest.top = drawy;
				dest.right = drawx + (*tiledata)[dwID].tiles[p].wWidth;
				dest.bottom = drawy + (*tiledata)[dwID].tiles[p].wHeight;
				DDBLTFX fx;
				memset(&fx, 0, sizeof(DDBLTFX));
				fx.dwSize = sizeof(DDBLTFX);

#ifndef NOSURFACES
				if (lpdds->Blt(&dest, (*tiledata)[dwID].tiles[p].pic, NULL, DDBLT_KEYSRC, &fx) != DD_OK)
					TRACE("Blit failed\n");
#else
				DDSURFACEDESC2 ddsd;
				ZeroMemory(&ddsd, sizeof(ddsd));
				ddsd.dwSize = sizeof(DDSURFACEDESC2);
				ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;

				lpdds->GetSurfaceDesc(&ddsd);

				lpdds->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);

				BlitTerrainTSB(ddsd.lpSurface, drawx, drawy, 0, 0, ddsd.lPitch, ddsd.dwWidth, ddsd.dwHeight, (*tiledata)[dwID].tiles[p]);
				lpdds->Unlock(NULL);
#endif

			}

			p++;
		}
	}

	FSunPackLib::SetColorKey(lpdds, -1);

	return lpdds;
}

void CTileSetBrowserView::DrawIt()
{
	CPaintDC myDC(this);


}

void CTileSetBrowserView::OnLButtonDown(UINT nFlags, CPoint point)
{
	RECT r;
	GetClientRect(&r);

	if (m_tilecount == 0)
		return;

	SCROLLINFO scrinfo;
	scrinfo.cbSize = sizeof(SCROLLINFO);
	scrinfo.fMask = SIF_ALL;
	GetScrollInfo(SB_HORZ, &scrinfo);
	point.x += scrinfo.nPos;
	GetScrollInfo(SB_VERT, &scrinfo);
	point.y += scrinfo.nPos;


	int max_r = r.right / m_tile_width;

	if (max_r == 0) max_r = 1;

	int cur_y = 0;
	int cur_x = 0;

	int tile_width = m_tile_width;
	int tile_height = m_tile_height;

	if (m_CurrentMode == 1)
	{
		DWORD dwID = GetTileID(m_currentTileSet, 0);



		int i;
		for (i = 0;i < m_tilecount;i++)
		{
			int curwidth = (*tiledata)[dwID].rect.right - (*tiledata)[dwID].rect.left;
			int curheight = (*tiledata)[dwID].rect.bottom - (*tiledata)[dwID].rect.top;
			curwidth = m_tile_width;
			curheight = m_tile_height;

#ifdef RA2_MODE
			if ((m_currentTileSet == 80 && Map->GetTheater() == "TEMPERATE") || (m_currentTileSet == 73 && Map->GetTheater() == "SNOW") || (m_currentTileSet == 101 && Map->GetTheater() == "URBAN"))
			{

				if (i == 10 || i == 15)
				{
					dwID++; // don´t forget this
					continue;
				}
			}
#endif

			int posaddedx = (m_tile_width - curwidth) / 2;
			int posaddedy = (m_tile_height - curheight) / 2;

			if (point.x > cur_x + posaddedx && point.y > cur_y + posaddedy && point.x < cur_x + tile_width - posaddedx && point.y < cur_y + tile_height - posaddedy)
			{
				char c[50];
				itoa(GetAddedHeight(dwID), c, 10);
				OutputDebugString(c);

				int oldmode = AD.mode;
				int oldid = AD.type;

				AD.mode = ACTIONMODE_SETTILE;
				AD.type = dwID;
				AD.data = 0;
				AD.data2 = 0;
				AD.data3 = 0;
				AD.z_data = 0;

				if (oldid > *tiledata_count) oldid = 0;

				if (oldmode != ACTIONMODE_SETTILE || (*tiledata)[oldid].wTileSet != m_currentTileSet)
				{
					((CFinalSunDlg*)theApp.m_pMainWnd)->m_settingsbar.m_BrushSize = 0;
					((CFinalSunDlg*)theApp.m_pMainWnd)->m_settingsbar.UpdateData(FALSE);
					((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_x = 1;
					((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_y = 1;

					int i;
					for (i = 0;i < g_data.sections["StdBrushSize"].values.size();i++)
					{
						CString n = *g_data.sections["StdBrushSize"].GetValueName(i);
						if ((*tiles).sections["General"].FindName(n) >= 0)
						{
							int tset = atoi((*tiles).sections["General"].values[n]);
							if (tset == m_currentTileSet)
							{
								int bs = atoi(*g_data.sections["StdBrushSize"].GetValue(i));
								((CFinalSunDlg*)theApp.m_pMainWnd)->m_settingsbar.m_BrushSize = bs - 1;
								((CFinalSunDlg*)theApp.m_pMainWnd)->m_settingsbar.UpdateData(FALSE);
								((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_x = bs;
								((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_y = bs;
							}
						}
					}
				}

				RedrawWindow();
				return;
			}

			cur_x += tile_width;
			if (i % max_r == max_r - 1)
			{
				cur_y += tile_height;
				cur_x = 0;
			}



			dwID++;
		}
	}
	else if (m_CurrentMode == 2)
	{
		int i;
		for (i = 0;i < max_ovrl_img;i++)
		{
			PICDATA* p = ovrlpics[m_currentOverlay][i];
			if (p != NULL && p->pic != NULL)
			{
				int curwidth = m_tile_width;
				int curheight = m_tile_height;

				int posaddedx = (m_tile_width - curwidth) / 2;
				int posaddedy = (m_tile_height - curheight) / 2;

				if (point.x > cur_x + posaddedx && point.y > cur_y + posaddedy && point.x < cur_x + tile_width - posaddedx && point.y < cur_y + tile_height - posaddedy)
				{
					AD.mode = ACTIONMODE_PLACE;
					AD.type = 6;
					AD.data = 33;
					AD.data2 = m_currentOverlay;
					AD.data3 = i;
					RedrawWindow();
					return;
				}

				cur_x += tile_width;
				if (i % max_r == max_r - 1)
				{
					cur_y += tile_height;
					cur_x = 0;
				}
			}

		}
	}


	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->SetForegroundWindow();
	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->SetFocus();

	CScrollView::OnLButtonDown(nFlags, point);
}

// calculates additional height added to the top of a tile if needed
int CTileSetBrowserView::GetAddedHeight(DWORD dwID)
{

	//int i;
	int cur_added = 0;
	//for(i=0;i<(*tiledata)[dwID].wTileCount;i++)
	{
		int i, e, p = 0;;
		for (i = 0;i < (*tiledata)[dwID].cx;i++)
		{
			for (e = 0;e < (*tiledata)[dwID].cy;e++)
			{
				int drawy = e * f_y / 2 + i * f_y / 2 - (*tiledata)[dwID].rect.top;

				drawy += (*tiledata)[dwID].tiles[p].sY - (*tiledata)[dwID].tiles[p].bZHeight * f_y / 2;

				if (drawy < cur_added) cur_added = drawy;

				p++;
			}
		}


	}



	return -cur_added;
}

void CTileSetBrowserView::SetOverlay(DWORD dwID)
{
	int k;
	int need_pos = -1;
	int need_width = 0;
	int need_height = 0;
	// m_tilecount=0;
	int iovrlcount = 0;
	BOOL bFound = FALSE;
	for (k = 0;k < max_ovrl_img;k++)
	{
		PICDATA* p = ovrlpics[dwID][k];
		if (p != NULL && p->pic != NULL)
		{
			bFound = TRUE;
		}
	}
	if (!bFound)
	{
		theApp.m_loading->LoadOverlayGraphic(*rules.sections["OverlayTypes"].GetValue(dwID), dwID);
		((CFinalSunDlg*)(theApp.m_pMainWnd))->m_view.m_isoview->UpdateOverlayPictures();
		//p=ovrlpics[dwID][k];
	}
	for (k = 0;k < max_ovrl_img;k++)
	{
		PICDATA* p = ovrlpics[dwID][k];
		if (p == NULL || p->pic == NULL)
		{
			//if(!p->bTried)
			{

			}
		}
		if (p != NULL && p->pic != NULL)
		{
			iovrlcount++;
		}
	}
	for (k = 0;k < max_ovrl_img;k++)
	{
		PICDATA* p = ovrlpics[dwID][k];
		if (p != NULL && p->pic != NULL)
		{
			need_pos = k;
			need_width = p->wMaxWidth;
			need_height = p->wMaxHeight;
			break;
		}
	}

	if (need_pos < 0)
		return;

	((CFinalSunDlg*)theApp.m_pMainWnd)->m_settingsbar.m_BrushSize = 0;
	((CFinalSunDlg*)theApp.m_pMainWnd)->m_settingsbar.UpdateData(FALSE);
	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_x = 1;
	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->m_BrushSize_y = 1;



	m_CurrentMode = 2;
	m_tile_width = 0;
	m_tile_height = 0;
	m_currentOverlay = dwID;



	m_tile_width = need_width + 6;
	m_tile_height = need_height + 6;



	RECT r;
	GetClientRect(&r);
	int max_r = r.right / m_tile_width;
	if (max_r <= 0) max_r = 1;
	m_bottom_needed = m_tile_height * (1 + (iovrlcount) / max_r);
	GetParentFrame()->RecalcLayout(TRUE);
	RedrawWindow();

	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->SetForegroundWindow();
	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->SetFocus();

}

#ifdef IGNORETHIS
LPDIRECTDRAWSURFACE4 CTileSetBrowserView::RenderOverlay(DWORD dwType, DWORD dwData)
{
	LPDIRECTDRAWSURFACE4 lpdds = NULL;
	LPDIRECTDRAW4 lpdd = ((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->dd;

	DDSURFACEDESC2 ddsd;
	memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	int added_height = GetAddedHeight(dwID);
	ddsd.dwHeight =//(*tiledata)[dwID].rect.bottom-(*tiledata)[dwID].rect.top+added_height;
		ddsd.dwWidth = (*tiledata)[dwID].rect.right - (*tiledata)[dwID].rect.left;
	if (lpdd->CreateSurface(&ddsd, &lpdds, NULL) != DD_OK)
	{
		return NULL;
	}

	DDBLTFX ddfx;
	memset(&ddfx, 0, sizeof(DDBLTFX));
	ddfx.dwSize = sizeof(DDBLTFX);
	lpdds->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddfx);


	int y_added = ddsd.dwHeight - ((*tiledata)[dwID].cx * f_y / 2 + (*tiledata)[dwID].cy * f_y / 2);

	int i, e, p = 0;;
	for (i = 0;i < (*tiledata)[dwID].cx;i++)
	{
		for (e = 0;e < (*tiledata)[dwID].cy;e++)
		{
			int drawx = e * f_x / 2 - i * f_x / 2 - (*tiledata)[dwID].rect.left;
			int drawy = e * f_y / 2 + i * f_y / 2 - (*tiledata)[dwID].rect.top;

			drawx += (*tiledata)[dwID].tiles[p].sX;
			drawy += added_height + (*tiledata)[dwID].tiles[p].sY - (*tiledata)[dwID].tiles[p].bZHeight * f_y / 2;
			//drawy+=y_added;

			if ((*tiledata)[dwID].tiles[p].pic)
			{
				RECT dest;
				dest.left = drawx;
				dest.top = drawy;
				dest.right = drawx + (*tiledata)[dwID].tiles[p].wWidth;
				dest.bottom = drawy + (*tiledata)[dwID].tiles[p].wHeight;
				DDBLTFX fx;
				memset(&fx, 0, sizeof(DDBLTFX));
				fx.dwSize = sizeof(DDBLTFX);


				DDSURFACEDESC2 ddsd;
				ZeroMemory(&ddsd, sizeof(ddsd));
				ddsd.dwSize = sizeof(DDSURFACEDESC2);
				ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;

				lpdds->GetSurfaceDesc(&ddsd);

				lpdds->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);

				BlitTerrainTSB(ddsd.lpSurface, drawx, drawy, 0, 0, ddsd.lPitch, ddsd.dwWidth, ddsd.dwHeight, (*tiledata)[dwID].tiles[p]);
				lpdds->Unlock(NULL);


			}

			p++;
		}
	}

	SetSurfaceColorKey(lpdds, -1);
}

#endif

void CTileSetBrowserView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	//RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);	
	CScrollView::OnVScroll(nSBCode, nPos, pScrollBar);
}
