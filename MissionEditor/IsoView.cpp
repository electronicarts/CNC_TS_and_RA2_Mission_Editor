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

/*
ISOVIEW.CPP
Implementation of the isometric view.
*/

#include "stdafx.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/* Includes */
#include "FinalSun.h"
#include "IsoView.h"
#include "DynamicGraphDlg.h"
#include "infantry.h"
#include "building.h"
#include "unit.h"
#include "aircraft.h"
#include "waypointid.h"
#include "celltag.h"
#include "MyViewframe.h"
#include "inlines.h"
#include "macros.h"
#include "FinalSunDlg.h"
#include "loading.h"
#include "Structs.h"
#include "frontcliffmodifier.h"
#include "backcliffmodifier.h"
#include "LineDrawer.h"
#include "MapTool.h"
#include <format>
#include <chrono>
#include <algorithm>
#include "TextDrawer.h"

/* -------- */

/* Externals */
extern ACTIONDATA AD;
void GetNodeName(CString& name, int n);
/* --------- */

/* Overlay picture table (maximum overlay count=0xFF) */
PICDATA* ovrlpics[0xFF][max_ovrl_img];

// cancel draw flag
BOOL bCancelDraw = FALSE;
BOOL bNoDraw = FALSE;


// this variable will be set to TRUE when the window has lost focus
// once the mouse is moved in the isoview again WITHOUT pressed button, it is set to FALSE again
BOOL bDoNotAllowScroll = FALSE;



/////////////////////////////////////////////////////////////////////////////
// CIsoView

IMPLEMENT_DYNCREATE(CIsoView, CScrollView)

BOOL bNoThreadDraw = FALSE;
BOOL bDrawStats = TRUE;

/*UINT PaintThreadProc( LPVOID pParam )
{
	while(!bNoThreadDraw)
	{
		if(((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview!=NULL) ((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->UpdateWindow();
	}

	return 0;   // Thread erfolgreich ausgeführt
}
*/

class SurfaceLocker
{
public:
	SurfaceLocker(IDirectDrawSurface4* pDDS, LPRECT rect = nullptr) :
		SurfaceLocker()
	{
		m_hasRect = rect != nullptr;
		if (rect)
			m_rect = *rect;
		m_pDDS = pDDS;
		m_ddsd.dwSize = sizeof(DDSURFACEDESC2);
		m_ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;
	}

	DDSURFACEDESC2* ensure_locked()
	{
		if (m_locked)
			return &m_ddsd;

		if (m_pDDS->GetSurfaceDesc(&m_ddsd) != DD_OK)
			return nullptr;

		if (m_pDDS->Lock(m_hasRect ? &m_rect : nullptr, &m_ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL) != DD_OK)
			return nullptr;

		m_locked = true;

		if (!m_ddsd.lpSurface)
			return nullptr;  // probably does not happen in reality

		return &m_ddsd;
	}

	void ensure_unlocked()
	{
		if (m_locked)
		{
			m_pDDS->Unlock(m_hasRect ? &m_rect : nullptr);
		}
		m_locked = false;
	}

	~SurfaceLocker()
	{
		ensure_unlocked();
	}

	SurfaceLocker(const SurfaceLocker& other) = delete;
	SurfaceLocker& operator=(const SurfaceLocker& other) = delete;

private:
	SurfaceLocker() = default;

private:
	IDirectDrawSurface4* m_pDDS = nullptr;
	DDSURFACEDESC2 m_ddsd = { 0 };
	RECT m_rect = { 0 };
	bool m_hasRect = false;
	bool m_locked = false;
};


CIsoView::CIsoView()
{
	srand(GetTickCount());
	m_bAltCliff = FALSE;
	m_NoMove = FALSE;
	b_IsLoading = FALSE;
	m_viewOffset = ProjectedVec(0, 0);
	dd = NULL;
	dd_1 = NULL;
	lpds = NULL;
	lpdsBack = NULL;
	lpdsBackHighRes = nullptr;
	lpdsTemp = NULL;
	pf = DDPIXELFORMAT{ 0 };
	line.left = 0;
	line.top = 0;
	line.right = 0;
	line.bottom = 0;
	m_menu.LoadMenu(IDR_MAPVIEW);
	m_drag = FALSE;
	// _map=NULL;
	AD.reset();
	m_BrushSize_x = 1;
	m_BrushSize_y = 1;
	rscroll = FALSE;
	m_zooming = false;
	bThreadPainting = TRUE;
	m_viewScale = Vec2<CSProjected, float>(1.0f, 1.0f);
	m_viewScaleControl = 1.0f;
	updateFontScaled();
	//m_paintthread=new(CIsoPaintThread);
	//m_paintthread->CreateThread();

}

CIsoView::~CIsoView()
{
	// if(_map!=NULL) delete[] _map;
	// _map=NULL;
	//delete m_paintthread;
	bNoThreadDraw = TRUE;
}


BEGIN_MESSAGE_MAP(CIsoView, CView)
	//{{AFX_MSG_MAP(CIsoView)
	ON_WM_SYSCOMMAND()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOVE()
	ON_WM_SIZE()
	ON_WM_CHAR()
	ON_WM_DEADCHAR()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_TIMER()
	ON_WM_RBUTTONDOWN()
	ON_WM_KILLFOCUS()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP	
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Zeichnung CIsoView 

void CIsoView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
}


struct CHANGEHEIGHTDATA
{
	int pos;
	BOOL bNonMorpheable;
	DWORD toHeight;
};


DWORD WINAPI ChangeHeightThread(
	LPVOID lpParameter   // thread data
)
{
	CHANGEHEIGHTDATA* p = (CHANGEHEIGHTDATA*)lpParameter;

	((CFinalSunDlg*)theApp.m_pMainWnd)->m_view.m_isoview->ChangeTileHeight(p->pos, p->toHeight, p->bNonMorpheable);

	return 0;
}

/*
dst must be surface data, src must be a 8 bit palette image
*/
#ifdef NOSURFACES

struct BlitRect
{
	short left;
	short top;
	short right;
	short bottom;
};

__forceinline void BlitTerrain(void* dst, int x, int y, int dleft, int dtop, int dpitch, int dright, int dbottom, const SUBTILE& st)//BYTE* src, int swidth, int sheight)
{
	BYTE* src = st.pic;
	const unsigned short swidth = st.wWidth;
	const unsigned short sheight = st.wHeight;


	if (src == NULL || dst == NULL) return;

	//x += 1;
	//y += 1;
	//y -= f_y;

	if (x + swidth < dleft || y + sheight < dtop) return;
	if (x >= dright || y >= dbottom) return;


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
			short left = st.vborder[e].left;
			short right = st.vborder[e].right;

			short realright = right;
			//short realleft=left;
			//short addx_s=0;

			if (right >= srcRect.right) right = srcRect.right - 1;
			if (srcRect.left > 0)
			{
				pos += (realright - left + 1) * bpp;
				continue; // just cancel. this area is used by object browser. saves some performance
				//addx_s=srcRect.left;
				//left=max(left, srcRect.left);
				//left=//addx=left-srcRect.left;
			}

			if (realright >= left)
			{
				if (/*left<srcRect.left ||*/ left >= srcRect.right)
				{
					pos += (realright - left + 1) * bpp;
					continue;
				}


				void* dest = ((BYTE*)dst + (blrect.left + left) * bpp + (blrect.top + e) * dpitch);

				memcpy(dest, &st.pic[pos], bpp * (right - left + 1));
				pos += (realright - left + 1) * bpp;
			}
		}
	}
	else

#endif
		for (e = srcRect.top;e < srcRect.bottom;e++)
		{
			short& left = st.vborder[e].left;
			short& right = st.vborder[e].right;

			auto l = max(left, srcRect.left);
			auto r = min(right, static_cast<short>(srcRect.right - 1));
			for (i = l;i <= r;i++)
			{
				//if (i < srcRect.left || i >= srcRect.right)
				{
					//dest+=bpp;
				}
				//else
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


__forceinline void BlitTerrainHalfTransp(void* dst, int x, int y, int dleft, int dtop, int dpitch, int dright, int dbottom, const SUBTILE& st)//BYTE* src, int swidth, int sheight)
{
	BYTE* src = st.pic;
	const unsigned short swidth = st.wWidth;
	const unsigned short sheight = st.wHeight;


	if (src == NULL || dst == NULL) return;

	//x += 1;
	//y += 1;
	//y -= f_y;

	if (x + swidth < dleft || y + sheight < dtop) return;
	if (x >= dright || y >= dbottom) return;


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
		int a = 0;
		for (e = srcRect.top;e < srcRect.bottom;e += 1)
		{
			short left = st.vborder[e].left;
			short right = st.vborder[e].right;

			short realright = right;

			if (right >= srcRect.right) right = srcRect.right - 1;
			if (srcRect.left > 0)
			{
				pos += (realright - left + 1) * bpp;
				continue; // just cancel. this area is used by object browser. saves some performance
			}

			a++;

			if (realright >= left)
			{
				if (/*left<srcRect.left ||*/ left >= srcRect.right)
				{
					pos += (realright - left + 1) * bpp;
					continue;
				}


				void* dest = ((BYTE*)dst + (blrect.left + left) * bpp + (blrect.top + e) * dpitch);

				//memcpy(dest, &st.pic[pos], bpp*(right-left+1));
				//pos+=(right-left+1)*bpp;

				for (i = left + a % 2;i <= right;i += 2)
				{
					if (i < srcRect.left || i >= srcRect.right)
					{
						//dest+=bpp;
					}
					else
					{

						//BYTE& val=st.pic[pos+i*bpp];
						//if(val)
						{
							void* dest2 = (BYTE*)dest + (i - left) * bpp; //((BYTE*)dst+(blrect.left+i)*bpp+(blrect.top+e)*dpitch);

							memcpy(dest2, &st.pic[pos + (i - left) * bpp], bpp);
						}
					}
				}

				pos += (realright - left + 1) * bpp;
			}
		}
	}
	else

#endif
	{
		int a = 0;
		for (e = srcRect.top;e < srcRect.bottom;e++)
		{
			short& left = st.vborder[e].left;
			short& right = st.vborder[e].right;


			a++;

			for (i = left + a % 2;i <= right;i += 2)
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

}


const int houseColorMin = 0x10;
const int houseColorMax = 0x1f;
const int houseColorRelMax = houseColorMax - houseColorMin;

inline void CalculateHouseColorPalette(int house_pal[houseColorRelMax + 1], const int* default_pal, const int* color = NULL)
{
	if (color)
	{
		if (bpp == 4)
		{
			for (int n = 0; n <= houseColorRelMax; ++n)
			{
				const int v = (houseColorRelMax - n);
				auto src = reinterpret_cast<const BYTE*>(color);
				auto bdest = reinterpret_cast<BYTE*>(&house_pal[n]);
				bdest[0] = src[0] * v / houseColorRelMax;
				bdest[1] = src[1] * v / houseColorRelMax;
				bdest[2] = src[2] * v / houseColorRelMax;
				bdest[3] = src[3];
			}
		}
		else
		{
			// nowadays only 32 bit rendering should be used, so we take a shortcut here
			for (int n = 0; n <= houseColorRelMax; ++n)
			{
				house_pal[n] = *color;
			}
		}
	}
	else
	{
		for (int n = 0; n <= houseColorRelMax; ++n)
		{
			house_pal[n] = default_pal[n + houseColorMin];
		}
	}
}


/*
There is no need for newpal
*/
__forceinline void BlitPic(void* dst, int x, int y, int dleft, int dtop, int dpitch, int dright, int dbottom, PICDATA& pd, int* color = NULL, int* newPal = NULL)//BYTE* src, int swidth, int sheight)
{
	ASSERT(pd.bType != PICDATA_TYPE_BMP);

	if (newPal == NULL) newPal = pd.pal;

	BYTE* src = (BYTE*)pd.pic;
	int swidth = pd.wMaxWidth;
	int sheight = pd.wMaxHeight;

	if (src == NULL || dst == NULL) return;

	//x += 1;
	//y += 1;
	//y -= f_y;

	if (x + swidth < dleft || y + sheight < dtop) return;
	if (x >= dright || y >= dbottom) return;


	RECT blrect;
	RECT srcRect;
	srcRect.left = 0;
	srcRect.top = 0;
	srcRect.right = swidth;
	srcRect.bottom = sheight;
	blrect.left = x;
	if (blrect.left < 0)
	{
		srcRect.left = 1 - blrect.left;
		//blrect.left=1;
	}
	blrect.top = y;
	if (blrect.top < 0)
	{
		srcRect.top = 1 - blrect.top;
		//blrect.top=1;
	}
	blrect.right = (x + swidth);
	if (x + swidth > dright)
	{
		srcRect.right = swidth - ((x + swidth) - dright);
		blrect.right = dright;
	}
	blrect.bottom = (y + sheight);
	if (y + sheight > dbottom)
	{
		srcRect.bottom = sheight - ((y + sheight) - dbottom);
		blrect.bottom = dbottom;
	}


	int i, e;

	// calculate a palette for given color for values 0x10 to 0x1f - we might move this outside
	const int houseColorMin = 0x10;
	const int houseColorMax = 0x1f;
	const int houseColorRelMax = houseColorMax - houseColorMin;
	int houseColors[houseColorRelMax + 1] = { 0 };
	CalculateHouseColorPalette(houseColors, newPal, color);
	const BYTE* const pLighting = (bpp == 4 && pd.lighting && !pd.lighting->empty()) ? pd.lighting->data() : nullptr;

	for (e = srcRect.top;e < srcRect.bottom;e++)
	{
		int left = pd.vborder[e].left;
		int right = pd.vborder[e].right;

		if (left < srcRect.left)
			left = srcRect.left;
		if (right >= srcRect.right)
			right = srcRect.right - 1;

		for (i = left;i <= right;i++)
		{
			if (blrect.left + i < 0)
				continue;

			const int spos = i + e * swidth;
			BYTE val = src[spos];

			if (val)
			{
				void* dest = ((BYTE*)dst + (blrect.left + i) * bpp + (blrect.top + e) * dpitch);

				if (dest >= dst)
				{
					int c;
					if (!color || newPal != iPalUnit || val < houseColorMin || val > houseColorMax)
					{
						c = newPal[val];
					}
					else
					{
						// Replace the original palette color with the house color
						ASSERT(val >= houseColorMin && val <= houseColorMax);
						const int v = (val - houseColorMin);
						c = houseColors[v];
					}
					if (pLighting)
					{
						// bpp == 4
						ASSERT(bpp == 4);
						int l = pLighting[spos];
						BYTE* bc = reinterpret_cast<BYTE*>(&c);
						for (int i = 0; i < 4; ++i)
							bc[i] = min(255, bc[i] * (200 + l * 300 / 255) / 255);  // game seems to overbrighten and have a lot of ambient - if you change this, also change Loading.cpp shp lighting value so that shp light stays at 1.0
							//bc[i] = min(255, bc[i] * (0 + l * (255 - 0) / 255) / 255);
					}
					memcpy(dest, &c, bpp);
				}
			}
		}
	}
}

__forceinline void BlitPicHalfTransp(void* dst, int x, int y, int dleft, int dtop, int dpitch, int dright, int dbottom, PICDATA& pd, int* color = NULL, int* newPal = NULL)//BYTE* src, int swidth, int sheight)
{
	ASSERT(pd.bType != PICDATA_TYPE_BMP);

	if (newPal == NULL) newPal = pd.pal;

	BYTE* src = (BYTE*)pd.pic;
	int swidth = pd.wMaxWidth;
	int sheight = pd.wMaxHeight;

	if (src == NULL || dst == NULL) return;

	//x += 1;
	//y += 1;
	//y -= f_y;

	if (x + swidth < dleft || y + sheight < dtop) return;
	if (x >= dright || y >= dbottom) return;


	RECT blrect;
	RECT srcRect;
	srcRect.left = 0;
	srcRect.top = 0;
	srcRect.right = swidth;
	srcRect.bottom = sheight;
	blrect.left = x;
	if (blrect.left < 0)
	{
		srcRect.left = 1 - blrect.left;
		//blrect.left=1;
	}
	blrect.top = y;
	if (blrect.top < 0)
	{
		srcRect.top = 1 - blrect.top;
		//blrect.top=1;
	}
	blrect.right = (x + swidth);
	if (x + swidth > dright)
	{
		srcRect.right = swidth - ((x + swidth) - dright);
		blrect.right = dright;
	}
	blrect.bottom = (y + sheight);
	if (y + sheight > dbottom)
	{
		srcRect.bottom = sheight - ((y + sheight) - dbottom);
		blrect.bottom = dbottom;
	}


	int i, e;




	for (e = srcRect.top;e < srcRect.bottom;e++)
	{


		int left = pd.vborder[e].left;
		int right = pd.vborder[e].right;
		if (left < srcRect.left) left = srcRect.left;
		if (right >= srcRect.right) right = srcRect.right - 1;

		int a = e % 2 + left % 2;

		for (i = left + a;i <= right;i += 2)
		{
			//a++;

			if (blrect.left + i < 0) continue;
			//if(a%2) continue;

			BYTE& val = src[i + e * swidth];
			//0x10-0x1f,
			if (val)
			{
				void* dest = ((BYTE*)dst + (blrect.left + i) * bpp + (blrect.top + e) * dpitch);
				//*dest=newPal[val];
				if (dest >= dst)
				{
					if (!color || newPal != iPalUnit || val < 0x10 || val>0x1f)
					{
						memcpy(dest, &newPal[val], bpp);
					}
					else
					{
						//int col=0;
						memcpy(dest, color, bpp);
					}
				}
			}
		}
	}
}
#endif




/////////////////////////////////////////////////////////////////////////////
// Diagnose CIsoView

#ifdef _DEBUG
void CIsoView::AssertValid() const
{
	CView::AssertValid();
}

void CIsoView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////updateFontScaled////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CIsoView 

void CIsoView::updateFontScaled()
{
	auto dc = CDC::FromHandle(::GetDC(NULL));
	m_fontDefaultHeight = -MulDiv(12, dc->GetDeviceCaps(LOGPIXELSY), 72);
	m_Font9Height = -MulDiv(9, dc->GetDeviceCaps(LOGPIXELSY), 72);

	if (dd)
	{
		m_textDefault.reset(new TextDrawer(dd, m_fontDefaultHeight, RGB(0,0,0), RGB(255, 255, 255)));
		m_text9.reset(new TextDrawer(dd, m_Font9Height, RGB(0, 0, 0), RGB(255, 255, 255)));
		m_textScaled.reset(new TextDrawer(dd, m_fontDefaultHeight / m_viewScale.y, RGB(0, 0, 0), RGB(255, 255, 255)));
		m_text9Scaled.reset(new TextDrawer(dd, m_Font9Height / m_viewScale.y, RGB(0, 0, 0), RGB(255, 255, 255)));
		m_textBlue.reset(new TextDrawer(dd, m_fontDefaultHeight, RGB(0, 0, 255), RGB(255, 255, 255)));
		m_textBlueScaled.reset(new TextDrawer(dd, m_fontDefaultHeight / m_viewScale.y, RGB(0, 0, 255), RGB(255, 255, 255)));
		m_textBlue9.reset(new TextDrawer(dd, m_Font9Height, RGB(0, 0, 255), RGB(255, 255, 255)));
		m_textBlue9Scaled.reset(new TextDrawer(dd, m_Font9Height / m_viewScale.y, RGB(0, 0, 255), RGB(255, 255, 255)));
	}
}

BOOL CIsoView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{

	BOOL r = CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);

	this->SetScrollRange(SB_HORZ, 0, 1, TRUE);
	this->SetScrollRange(SB_VERT, 0, 1, TRUE);

	// AfxBeginThread(PaintThreadProc, 0, 0, 0xFFFF);

	return r;
}

void CIsoView::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	if (nID == SC_CLOSE)
	{
		// ok now just hide the window
		ShowWindow(SW_HIDE);
		return;
	}

	CView::OnSysCommand(nID, lParam);
}

inline int GetScrPos(CWnd* pOwner, UINT nSB, UINT nSBCode, UINT& nPos)
{
	SCROLLINFO scr;
	scr.cbSize = sizeof(SCROLLINFO);
	scr.fMask = SIF_ALL;

	pOwner->GetScrollInfo(nSB, &scr);

	switch (nSBCode)
	{
	case SB_LEFT:
	{
		nPos = scr.nMin;
		break;
	}
	case SB_RIGHT:
	{
		nPos = scr.nMax;
		break;
	}
	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
	{
		break;
	}
	case SB_LINELEFT:
	{
		nPos = scr.nPos - 1;
		break;
	}
	case SB_LINERIGHT:
	{
		nPos = scr.nPos + 1;
		break;
	}
	case SB_PAGELEFT:
	{
		nPos = scr.nPos - 5;
		break;
	}
	case SB_PAGERIGHT:
	{
		nPos = scr.nPos + 5;
		break;
	}
	}

	return nPos;

}

void CIsoView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (nSBCode == SB_ENDSCROLL || nSBCode == SB_THUMBTRACK) return;


	nPos = GetScrPos(this, SB_VERT, nSBCode, nPos);

	this->SetScrollPos(SB_VERT, nPos, TRUE);

	m_viewOffset.y = (nPos + Map->GetWidth() / 2 - 4) * f_y;

	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

	CMyViewFrame& dlg = *(CMyViewFrame*)owner;
	dlg.m_minimap.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

void CIsoView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (nSBCode == SB_ENDSCROLL || nSBCode == SB_THUMBTRACK) return;



	nPos = GetScrPos(this, SB_HORZ, nSBCode, nPos);

	RECT r;
	GetWindowRect(&r);
	m_viewOffset.x = (nPos + Map->GetHeight() / 2 - 1) * f_x;

	this->SetScrollPos(SB_HORZ, nPos, TRUE);

	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

	CMyViewFrame& dlg = *(CMyViewFrame*)owner;
	dlg.m_minimap.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

/*
This function is called when any data has been changed in another dialog.
It is used to initialize and update the dialog.
*/
void CIsoView::UpdateDialog(BOOL bRepos)
{
	OutputDebugString("Isoview updated\n");

	UpdateOverlayPictures();


	if (bRepos && Map->GetIsoSize() != 0)
	{
		m_viewScale = Vec2<CSProjected, float>(1.0f, 1.0f);
		m_viewScaleControl = 1.0f;
		auto r = GetScaledDisplayRect();
		UpdateScrollRanges();
		SetScroll((Map->GetIsoSize() / 2 - r.right / f_x / 2) * f_x, (Map->GetIsoSize() / 2 - r.bottom / f_y / 2) * f_y);		
	}

	RedrawWindow(NULL, NULL, RDW_INVALIDATE);

}

void CIsoView::UpdateScrollRanges()
{
	auto r = GetScaledDisplayRect();
	SetScrollRange(SB_HORZ, 0, Map->GetWidth() - r.right / f_x + 4, TRUE);
	SetScrollRange(SB_VERT, 0, Map->GetHeight() - r.bottom / f_y + 6, TRUE);
}

BOOL CIsoView::PreCreateWindow(CREATESTRUCT& cs)
{
	// we overwrite that, because in debug version its necessary that a view is a child window!

	// save the old style and let CView::PreCreateWindow() think we are using a child window, then restore old style.
	int ostyle = cs.style;
	cs.style |= WS_CHILD | WS_VSCROLL | WS_HSCROLL;

	BOOL bres = CView::PreCreateWindow(cs);
	cs.style = ostyle;

	return bres;

}


void CIsoView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (b_IsLoading) return;

	static BOOL isMoving = FALSE;

	CIniFile& ini = Map->GetIniFile();

	cur_x_mouse = point.x;
	cur_y_mouse = point.y;

	if (!(nFlags & MK_RBUTTON)) bDoNotAllowScroll = FALSE;

	if (rscroll)
	{
		if (!(nFlags & MK_RBUTTON))
		{
			ReleaseCapture();
			KillTimer(11);
			rscroll = FALSE;
			ShowCursor(TRUE);

			CMyViewFrame& dlg = *(CMyViewFrame*)owner;
			dlg.m_minimap.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

		}
		else
		{

			isMoving = FALSE;
			return;
		}
	}

	if (!bDoNotAllowScroll && (nFlags & MK_RBUTTON) && !rscroll) // check if scroll should start
	{
		if (abs(point.x - rclick_x) > 2 || abs(point.y - rclick_y) > 2)
		{
			// yes, begin scrolling!

			rscroll = TRUE;
			SetTimer(11, 25, NULL);
			SetCapture();
			//while(ShowCursor(FALSE)>0);
			isMoving = FALSE;
			return;
		}
	}

	if ((nFlags & MK_MBUTTON) == 0)
		m_zooming = false;

	if (!bDoNotAllowScroll && m_zooming)
	{
		if ((nFlags & MK_MBUTTON) == MK_MBUTTON)
		{
			auto diff = point - m_MButtonMoveZooming;
			m_MButtonMoveZooming = point;
			Zoom(m_MButtonDown, -diff.cy / 1000.0f);
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			return;
		}
	}

	m_moved = TRUE;

	if (b_IsLoading == TRUE) return;
	if (Map->GetIsoSize() == 0 || isMoving == TRUE || m_zooming) return;

	isMoving = TRUE;

	// SetError(TranslateStringACP("Ready"));

	RECT r;
	//GetClientRect(&r);
	//ClientToScreen(&r);
	GetWindowRect(&r);

	const auto viewOffsetWR = m_viewOffset + ProjectedVec(r.left, r.top);
	const auto projCoords = GetProjectedCoordinatesFromClientCoordinates(point);
	const MapCoords mapCoords = GetMapCoordinatesFromClientCoordinates(point, (nFlags & MK_CONTROL) == MK_CONTROL);
	m_cellCursor = MapCoords(-1, -1);
	const int x = mapCoords.x; // compat
	const int y = mapCoords.y;

	int mapwidth = Map->GetWidth();
	int mapheight = Map->GetHeight();

	BOOL bOutside = FALSE;

	if (AD.mode != ACTIONMODE_COPY && AD.mode != ACTIONMODE_PASTE)
	{
		if (x < 1 || y < 1 || x + y<mapwidth + 1 || x + y>mapwidth + mapheight * 2 || (y + 1 > mapwidth && x - 1 < y - mapwidth) || (x + 1 > mapwidth && y + mapwidth - 1 < x))
		{
			isMoving = FALSE;
			return;
		}
	}
	else
	{
		if (x < 1 || y < 1 || x >= mapwidth + mapheight - 1 || y >= mapwidth + mapheight - 1)
		{
			bOutside = TRUE;
			if (AD.mode == ACTIONMODE_COPY || AD.mode == ACTIONMODE_PASTE)
			{
				isMoving = FALSE;
				return;
			}
		}
	}



	if (lpdsBack)
		// reset back buffer to last DrawMap()
		lpdsBack->BltFast(0, 0, lpdsTemp, NULL, DDBLTFAST_WAIT);
		//lpdsBack->Blt(NULL, lpdsTemp, NULL, 0, 0);

	//int cell_x = x;
	//int cell_y = y;

	{
		// Preview rendering
		if (AD.mode == ACTIONMODE_SETTILE && !((nFlags & MK_LBUTTON) && !(nFlags & MK_CONTROL) && (nFlags & MK_SHIFT)))
		{
			// No placement, just preview

			bCancelDraw = TRUE;

			int i, e, f, n;
			int p = 0;
			//FIELDDATA oldData[36][36][10][10];
			int width = (*tiledata)[AD.type].cx;
			int height = (*tiledata)[AD.type].cy;
			int pos = x - width + 1 + (y - height + 1) * Map->GetIsoSize();
			int startheight = Map->GetHeightAt(x + y * Map->GetIsoSize()) + AD.z_data;
			int ground = (Map->GetFielddataAt(x + y * Map->GetIsoSize()))->wGround;
			if (ground == 0xFFFF) ground = 0;
			startheight -= (*tiledata)[ground].tiles[Map->GetFielddataAt(x + y * Map->GetIsoSize())->bSubTile].bZHeight;
			TILEDATA td = (*tiledata)[AD.type];

			Map->TakeSnapshot(TRUE, x - width - 4, y - height - 4, x - width + m_BrushSize_x * width + 7, y - height + m_BrushSize_y * height + 7);
			int cur_pos = pos;
			int isosize = Map->GetIsoSize();
			int height_add = height * isosize;
			char c[50];
			itoa(AD.data, c, 10);
			for (f = 0;f < m_BrushSize_x;f++)
			{
				for (n = 0;n < m_BrushSize_y;n++)
				{

					int tile = AD.type;

					if (AD.data == 1)
					{
						int n = rand() * 5 / RAND_MAX;
						tile += n;
					}



					cur_pos = pos + f * width + n * height_add;
					p = 0;
					for (i = 0;i < td.cx;i++)
					{
						for (e = 0;e < td.cy;e++)
						{
							if (x - width + 1 + f * width + i >= Map->GetIsoSize() ||
								y - height + 1 + n * height + e >= Map->GetIsoSize())
							{
							}
							else
								if (td.tiles[p].pic != NULL)
								{
									int mypos = cur_pos + i + e * isosize;

									Map->SetHeightAt(mypos, startheight + td.tiles[p].bZHeight);
									Map->SetTileAt(mypos, tile, p);



								}
							p++;

						}
					}


				}
			}

			if (!((nFlags & MK_CONTROL) && (nFlags & MK_SHIFT)))
			{
				if (!theApp.m_Options.bDisableAutoShore) Map->CreateShore(x - width - 2, y - height - 2, x - width + td.cx * m_BrushSize_x + 5, y - height + td.cy * m_BrushSize_y + 5, FALSE);

				for (f = 0;f < m_BrushSize_x;f++)
				{
					for (n = 0;n < m_BrushSize_y;n++)
					{
						cur_pos = pos + f * width + n * height_add;
						p = 0;
						for (i = -1;i < td.cx + 1;i++)
						{
							for (e = -1;e < td.cy + 1;e++)
							{
								Map->SmoothAllAt(cur_pos + i + (e)*isosize);
							}
						}


					}
				}
			}



			bCancelDraw = FALSE;
			DrawMap();


			Map->Undo();

			m_drag = FALSE;

		}
		else if (m_drag && AD.mode == ACTIONMODE_CLIFFFRONT)
		{
			RECT affect;
			if (m_mapx < x) affect.left = m_mapx - 2; else affect.left = x - 2;
			if (m_mapx < x) affect.right = x + 2; else affect.right = m_mapx + 2;
			if (m_mapy < y) affect.top = m_mapy - 2; else affect.top = y - 2;
			if (m_mapy < y) affect.bottom = y + 2; else affect.bottom = m_mapy + 2;

			//Map->TakeSnapshot();
			Map->TakeSnapshot(TRUE, affect.left, affect.top, affect.right, affect.bottom);
			CFrontCliffModifier f;
			BOOL bAlt = FALSE;
			if (Map->GetTheater() == THEATER3 && m_bAltCliff)
				bAlt = TRUE;
			f.PlaceCliff(m_mapx, m_mapy, x, y, bAlt);
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			//Map->TakeSnapshot();
			Map->TakeSnapshot(TRUE, affect.left, affect.top, affect.right, affect.bottom);
			Map->Undo();
			Map->Undo();
		}
		else if (m_drag && AD.mode == ACTIONMODE_CLIFFBACK)
		{
			RECT affect;
			if (m_mapx < x) affect.left = m_mapx - 2; else affect.left = x - 2;
			if (m_mapx < x) affect.right = x + 2; else affect.right = m_mapx + 2;
			if (m_mapy < y) affect.top = m_mapy - 2; else affect.top = y - 2;
			if (m_mapy < y) affect.bottom = y + 2; else affect.bottom = m_mapy + 2;

			//Map->TakeSnapshot();
			Map->TakeSnapshot(TRUE, affect.left, affect.top, affect.right, affect.bottom);
			CBackCliffModifier f;
			BOOL bAlt = FALSE;
			if (Map->GetTheater() == THEATER3 && m_bAltCliff)
				bAlt = TRUE;
			f.PlaceCliff(m_mapx, m_mapy, x, y);
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			//Map->TakeSnapshot();
			Map->TakeSnapshot(TRUE, affect.left, affect.top, affect.right, affect.bottom);
			Map->Undo();
			Map->Undo();
		}
		else if (AD.mode == ACTIONMODE_COPY && m_drag)
		{
			last_succeeded_operation = 80301; //bugtracing once more

			int x1, x2, y1, y2;
			if (m_mapx < x)
			{
				x1 = m_mapx;
				x2 = x;
			}
			else
			{
				x1 = x;
				x2 = m_mapx;
			}
			if (m_mapy < y)
			{
				y1 = m_mapy;
				y2 = y;
			}
			else
			{
				y1 = y;
				y2 = m_mapy;
			}

			DDSURFACEDESC2 ddsd;
			ZeroMemory(&ddsd, sizeof(ddsd));
			ddsd.dwSize = sizeof(DDSURFACEDESC2);
			ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;

			lpdsBack->GetSurfaceDesc(&ddsd);


			lpdsBack->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);

			int i, e;
			int isosize = Map->GetIsoSize();
			for (i = 0;i < x2 - x1 + 1;i++)
			{
				for (e = 0;e < y2 - y1 + 1;e++)
				{
					last_succeeded_operation = 80302;
					int x_s = x1 + i;
					int y_s = y1 + e;

					if (x_s < 0 || y_s < 0 || x_s >= isosize || y_s >= isosize) continue;

					if (theApp.m_Options.bFlat) ToPhys(&x_s, &y_s); else ToPhys3d(&x_s, &y_s);
					x_s -= m_viewOffset.x;
					y_s -= m_viewOffset.y;

					last_succeeded_operation = 80305;
					DrawCell(ddsd.lpSurface, ddsd.dwWidth, ddsd.dwHeight, ddsd.lPitch, x_s, y_s, 1, 1, m_color_converter->GetColor(255, 0, 0));
					last_succeeded_operation = 80303;
				}
			}

			lpdsBack->Unlock(NULL);

			BlitBackbufferToHighRes();
			RenderUIOverlay();
			FlipHighResBuffer();
			last_succeeded_operation = 80304;
		}
		else if (AD.mode == ACTIONMODE_PASTE)
		{
			Map->TakeSnapshot();
			Map->Paste(x, y, AD.z_data);
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			Map->Undo();
		}
		else if ((AD.mode == ACTIONMODE_PLACE || AD.mode == ACTIONMODE_RANDOMTERRAIN) && (nFlags & ~MK_CONTROL) == 0 && AD.type != 7 && (AD.type != 6 || (AD.type == 6 && ((AD.data >= 30 && AD.data <= 33) || AD.data == 2 || AD.data == 3)))) // everything placing but not overlay!
		{
			FIELDDATA oldData[32][32];
			INFANTRY infData[SUBPOS_COUNT][32][32];
			int i, e;

			//if(AD.type!=1 || Map->GetInfantryCountAt(x+y*Map->GetIsoSize())==0)
			{
				for (i = 0;i < 32;i++)
				{
					for (e = 0;e < 32;e++)
					{
						oldData[i][e] = *Map->GetFielddataAt(i + x + (e + y) * Map->GetIsoSize());
						int z;
						for (z = 0;z < SUBPOS_COUNT;z++)
							if (oldData[i][e].infantry[z] > -1)
								Map->GetInfantryData(oldData[i][e].infantry[z], &infData[z][i][e]);
					}
				}

				PlaceCurrentObjectAt(x, y);
				RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

				for (i = 0;i < 32;i++)
				{
					for (e = 0;e < 32;e++)
					{
						DWORD dwPos = i + x + (e + y) * Map->GetIsoSize();
						FIELDDATA cur_field;
						cur_field = *Map->GetFielddataAt(dwPos);

						if (cur_field.aircraft != oldData[i][e].aircraft)
							Map->DeleteAircraft(cur_field.aircraft);
						int z;
						for (z = 0;z < SUBPOS_COUNT;z++)
							if (cur_field.infantry[z] != oldData[i][e].infantry[z])
							{
								Map->DeleteInfantry(cur_field.infantry[z]);
							}

						if (cur_field.node.index != oldData[i][e].node.index)
						{
							CString house;
							int id = Map->GetNodeAt(dwPos, house);
							Map->DeleteNode(house, id);
						}
						if (cur_field.structure != oldData[i][e].structure)
							Map->DeleteStructure(cur_field.structure);
						if (cur_field.terrain != oldData[i][e].terrain)
							Map->DeleteTerrain(cur_field.terrain);
#ifdef SMUDGE_SUPP
						if (cur_field.smudge != oldData[i][e].smudge)
							Map->DeleteSmudge(cur_field.smudge);
#endif
						if (cur_field.unit != oldData[i][e].unit)
							Map->DeleteUnit(cur_field.unit);

						if (cur_field.overlay != oldData[i][e].overlay)
							Map->SetOverlayAt(dwPos, oldData[i][e].overlay);
						if (cur_field.overlaydata != oldData[i][e].overlaydata)
							Map->SetOverlayDataAt(dwPos, oldData[i][e].overlaydata);

						Map->SetFielddataAt(dwPos, &oldData[i][e]);
					}
				}
			}
			//else
			//	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

		}
		else
		{

			SurfaceLocker locker(lpdsBack);
			auto desc = locker.ensure_locked();
			if (desc)
				DrawCellCursor(mapCoords, *desc);
			m_cellCursor = mapCoords;
			//RedrawWindow(NULL, NULL, RDW_INVALIDATE);
			locker.ensure_unlocked();

			BlitBackbufferToHighRes();
			RenderUIOverlay();
			FlipHighResBuffer();
		}
	}
	





	// display the coordinates	
	char c[50];
	CString cap;
	itoa(x, c, 10);
	cap += c;
	cap += " / ";
	itoa(y, c, 10);
	cap += c;
	cap += " - ";
	itoa(Map->GetHeightAt(x + y * Map->GetIsoSize()), c, 10);
	cap += c;

	CStatusBarCtrl& stat = ((CMyViewFrame*)owner)->m_statbar.GetStatusBarCtrl();
	stat.SetText(cap, 1, 0);


	// drag
	if (m_drag && AD.mode == 0)
	{
		RedrawWindow(NULL, NULL, RDW_INVALIDATE);

		CPaintDC dc(this);
		dc.SetROP2(R2_NOT);

		const auto renderOffset = CPoint(f_x / 2 / m_viewScale.x, f_y / 2 / m_viewScale.y);
		const auto from = GetClientCoordinates(MapCoords(m_mapx, m_mapy)) + renderOffset;
		const auto to = GetClientCoordinates(mapCoords) + renderOffset;

		dc.MoveTo(from.x, from.y);
		dc.LineTo(to.x, to.y);
	}
	else if (AD.mode == ACTIONMODE_SETTILE && (nFlags & MK_LBUTTON) && ((nFlags & MK_SHIFT) || Map->hasLat(AD.type)) && !(nFlags & MK_CONTROL))
	{
		// Sound(SOUND_LAYDOWNTILE);  // too many sounds

		PlaceTile(x, y, nFlags);
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		isMoving = FALSE;
		return;
	}
	else if ((nFlags & MK_LBUTTON) && (nFlags & MK_SHIFT) && AD.mode == ACTIONMODE_HEIGHTEN)
	{
		OnLButtonDown(nFlags, point);
		isMoving = FALSE;
		return;
	}
	else if ((nFlags & MK_LBUTTON) && (nFlags & MK_SHIFT) && AD.mode == ACTIONMODE_LOWER)
	{
		OnLButtonDown(nFlags, point);
		isMoving = FALSE;
		return;
	}

	else if ((nFlags == MK_LBUTTON) && AD.mode == ACTIONMODE_FLATTENGROUND)
	{
		m_TileChangeCount = 0;

		// if(abs(m_FlattenLastX-x)<2 && abs(m_FlattenLastY-y)<2) //ReachableFrom(x+y*Map->GetIsoSize(), m_mapx+m_mapy*Map->GetIsoSize()))
		int ground = Map->GetFielddataAt(x + (y)*Map->GetIsoSize())->wGround;
		if (ground == 0xFFFF) ground = 0;

		//if((*tiledata)[ground].bMorphable)
		{

			int left = -m_BrushSize_x / 2;
			int right = m_BrushSize_x / 2 + 1;
			int top = -m_BrushSize_y / 2;
			int bottom = m_BrushSize_y / 2 + 1;

			BOOL doNotSave = FALSE;
			int n, m;
			int isosize = Map->GetIsoSize();

			for (m = left;m < right;m++)
			{
				for (n = top;n < bottom;n++)
				{
					int pos = x + m + (y + n) * isosize;

					int ground = Map->GetFielddataAt(pos)->wGround;
					if (ground == 0xFFFF) ground = 0;
					/*int ground1=Map->GetFielddataAt(pos-m)->wGround;
					if(ground1==0xFFFF) ground=0;
					int ground2=Map->GetFielddataAt(pos-n*isosize)->wGround;
					if(ground2==0xFFFF) ground=0;*/



					if (!(*tiledata)[ground].bMorphable)
					{

						{
							if (n <= 0 /*&& !(*tiledata)[ground1].bMorphable*/) top = n + 1;
							if (m <= 0 /*&& !(*tiledata)[ground2].bMorphable*/) left = m + 1;
							if (n > 0 /*&& !(*tiledata)[ground1].bMorphable*/) bottom = n;
							if (m > 0 /*&& !(*tiledata)[ground2].bMorphable*/) right = m;
						}

					}

					if (n < top)
					{
						m = left;
						n = top;
					}
					if (m < left)
					{
						n = top;
						m = left;
					}
				}
			}

			m_funcRect.left = x;
			m_funcRect.top = y;
			m_funcRect.bottom = y;
			m_funcRect.right = x;

			for (m = left;m < right;m++)
			{
				for (n = top;n < bottom;n++)
				{
					int ground = Map->GetFielddataAt(x + m + (y + n) * Map->GetIsoSize())->wGround;
					if (ground == 0xFFFF) ground = 0;



					if ((*tiledata)[ground].bMorphable)
					{
						int i;
						int max = isosize * isosize;
						for (i = 0;i < max;i++)
							Map->SetReserved(i, 0);


						// do not make any slopes yet, we do this once we´re finished with the whole area!
						// helps to speed up performance!
						ChangeTileHeight(x + m + (y + n) * isosize, m_FlattenHeight, FALSE, FALSE, TRUE);



					}
					else
					{
						if (n == 0 && m == 0)
							doNotSave = TRUE;


					}

				}

			}

			/*
			Some trick for improving performance is using the m_funcRect parameter to get
			the area that any ChangeTileHeight() calls affected.
			*/
			ASSERT(m_funcRect.left <= m_funcRect.right);
			ASSERT(m_funcRect.top <= m_funcRect.bottom);
			for (m = m_funcRect.left - 1;m <= m_funcRect.right + 1;m++)
				for (n = m_funcRect.top - 1;n <= m_funcRect.bottom + 1;n++)
					Map->CreateSlopesAt(m + n * isosize);

			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			if (!doNotSave)
			{
				m_FlattenLastX = x;
				m_FlattenLastY = y;
			}
		}
	}
	else if ((nFlags & MK_LBUTTON) && (nFlags & MK_SHIFT) && AD.mode == ACTIONMODE_HEIGHTENTILE)
	{
		OnLButtonDown(nFlags, point);
	}
	else if ((nFlags & MK_LBUTTON) && (nFlags & MK_SHIFT) && AD.mode == ACTIONMODE_LOWERTILE)
	{
		OnLButtonDown(nFlags, point);
	}
	else if ((nFlags == MK_LBUTTON) && AD.mode == ACTIONMODE_WAYPOINT) // waypoints
	{
		if (AD.type == 1)
		{
			//delete waypoint
			int w = Map->GetWaypointAt(x + y * Map->GetIsoSize());
			if (w < 0)
			{
				isMoving = FALSE;
				return;
			}

			Map->DeleteWaypoint(w);
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
		else if (AD.type == 0)
		{
			int n = Map->GetWaypointAt(x + y * Map->GetIsoSize());
			if (n >= 0)
			{
				isMoving = FALSE;
				return;
			}

			Map->AddWaypoint("", x + y * Map->GetIsoSize());

			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
		else if (AD.type >= 3)
		{
			int n = Map->GetWaypointAt(x + y * Map->GetIsoSize());
			if (n >= 0)
			{
				isMoving = FALSE;
				return;
			}

			char c[50];
			int max = AD.type - 3;

			// MW April 8th: place waypoints before selected one if they are not there!
			// makes the user aware that he is doing something he should not do.
			// he should not skip starting locations
			// this logic is here to make sure there are no random starting points.
			// I do not want to change this when saving the map, as triggers may rely on the
			// actual starting position!
			int i;
			int e;
			int notfound = -1;
			for (e = 0;e < max;e++)
			{
				BOOL bFound = FALSE;

				for (i = 0;i < Map->GetWaypointCount();i++)
				{
					CString id;
					DWORD pos;
					Map->GetWaypointData(i, &id, &pos);
					if (atoi(id) == e) bFound = TRUE;
					if (bFound) break;
				}
				if (!bFound)
				{
					notfound = e;
					break;
				}
			}

			if (notfound >= 0) max = notfound;

			AD.type = 3 + max;

			itoa(max, c, 10);

			if (!Map->IsMultiplayer())
			{
				Map->AddWaypoint("99", x + (y + 1) * Map->GetIsoSize());
				Map->AddWaypoint("98", x + (y)*Map->GetIsoSize());
			}
			else
			{
				Map->AddWaypoint(c, x + y * Map->GetIsoSize());
			}

			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}

	}
	else if ((nFlags == MK_LBUTTON) && AD.mode == ACTIONMODE_CELLTAG) // celltags
	{
		if (AD.type == 1)
		{
			int n = Map->GetCelltagAt(x + y * Map->GetIsoSize());
			if (n < 0)
			{
				isMoving = FALSE;
				return;
			}
			Map->DeleteCelltag(n);

			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
		else if (AD.type == 4)
		{
			Map->AddCelltag(AD.data_s, x + y * Map->GetIsoSize());
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}
	else if ((nFlags == MK_LBUTTON) && AD.mode == ACTIONMODE_NODE) // nodes
	{
		if (AD.type == 1) // create node, delete building
		{
			int n = Map->GetStructureAt(x + y * Map->GetIsoSize());
			if (n < 0)
			{
				isMoving = FALSE;
				return;
			}

			STDOBJECTDATA sod;
			Map->GetStdStructureData(n, &sod);

			CString tmp;
			if (Map->GetNodeAt(atoi(sod.x) + atoi(sod.y) * Map->GetIsoSize(), tmp) >= 0)
			{
				SetError("You cannot place a node on another node");
				{
					isMoving = FALSE;
					return;
				};
			}

			Map->DeleteStructure(n);

			NODE node;
			node.x = sod.x;
			node.y = sod.y;
			node.house = sod.house;
			node.type = sod.type;

			Map->AddNode(&node, 0);
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

		}
		else if (AD.type == 0) // create node, don´t delete building
		{
			int n = Map->GetStructureAt(x + y * Map->GetIsoSize());
			if (n < 0)
			{
				isMoving = FALSE;
				return;
			}

			STDOBJECTDATA sod;
			Map->GetStdStructureData(n, &sod);

			CString tmp;
			if (Map->GetNodeAt(atoi(sod.x) + atoi(sod.y) * Map->GetIsoSize(), tmp) >= 0)
			{
				SetError("You cannot place a node on another node");
				{
					isMoving = FALSE;
					return;
				};
			}

			NODE node;
			node.x = sod.x;
			node.y = sod.y;
			node.type = sod.type;
			node.house = sod.house;

			Map->AddNode(&node, 0);
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

		}
		else if (AD.type == 2) // delete node
		{
			CString owner;
			int n = Map->GetNodeAt(x + y * Map->GetIsoSize(), owner);
			if (n < 0)
			{
				isMoving = FALSE;
				return;
			}
			Map->DeleteNode(owner, n);

			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

		}
	}
	else if ((nFlags == MK_LBUTTON) && AD.mode == ACTIONMODE_ERASEFIELD)
	{
		int h;
		DWORD dwPos = x + y * Map->GetIsoSize();
		h = Map->GetInfantryAt(dwPos);
		if (h > -1)
		{
			Map->DeleteInfantry(h);
		}

		h = Map->GetUnitAt(dwPos);
		if (h > -1)
		{
			Map->DeleteUnit(h);
		}

		h = Map->GetStructureAt(dwPos);
		if (h > -1)
		{
			Map->DeleteStructure(h);
		}

		h = Map->GetAirAt(dwPos);
		if (h > -1)
		{
			Map->DeleteAircraft(h);
		}

		h = Map->GetTerrainAt(dwPos);
		if (h > -1)
		{
			Map->DeleteTerrain(h);
		}

#ifdef SMUDGE_SUPP
		const FIELDDATA& fd = *Map->GetFielddataAt(dwPos);
		h = fd.smudge;
		if (h > -1)
		{
			Map->DeleteSmudge(h);
		}
#endif

		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

	}
	else if (AD.mode == ACTIONMODE_MAPTOOL)
	{
		if (AD.tool)
			AD.tool->onMouseMove(projCoords, mapCoords, MapToolMouseFlagsFromWin32(nFlags));
	}
	else if ((nFlags & MK_LBUTTON) == MK_LBUTTON && (AD.mode == ACTIONMODE_PLACE || AD.mode == ACTIONMODE_RANDOMTERRAIN))
	{
		// ADD OBJECTS

		if (AD.mode == ACTIONMODE_PLACE && AD.type == 6) Map->TakeSnapshot();

		if (AD.mode == ACTIONMODE_PLACE && AD.type == 6 && AD.data == 5) // bridges		
		{


			// calculate the x and y value...
			int x = mapCoords.x;
			int y = mapCoords.y;

			if (abs(x - m_mapx) >= abs(y - m_mapy))
			{
				// it´s from bottom left to top right
				y = m_mapy;
			}
			else
			{
				// from bottom right to top left
				x = m_mapx;
			}

			// draw a line

			int px1, py1, px2, py2;
			px1 = m_mapx;
			py1 = m_mapy;
			px2 = x;
			py2 = y;
			if (theApp.m_Options.bFlat)
				ToPhys(&px1, &py1);
			else
				ToPhys3d(&px1, &py1);
			if (theApp.m_Options.bFlat)
				ToPhys(&px2, &py2);
			else
				ToPhys3d(&px2, &py2);
			px1 += f_x / 2 - m_viewOffset.x - r.left;
			py1 += f_y / 2 - m_viewOffset.y - r.top;
			px2 += f_x / 2 - m_viewOffset.x - r.left;
			py2 += f_y / 2 - m_viewOffset.y - r.top;

			CRect r;
			GetWindowRect(r);
			px1 += r.left;
			px2 += r.left;
			py1 += r.top;
			py2 += r.top;


			HDC dc;
			lpdsBack->GetDC(&dc);

			POINT p;
			MoveToEx(dc, px1, py1, &p);
			LineTo(dc, px2, py2);

			lpdsBack->ReleaseDC(dc);

			BlitBackbufferToHighRes();
			RenderUIOverlay();
			FlipHighResBuffer();
			//lpds->Blt(NULL, lpdsBack, NULL, 0, 0);

		}
		else
		{
			PlaceCurrentObjectAt(x, y);
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}


		if (AD.mode == ACTIONMODE_PLACE && AD.type == 6)
		{
			Map->TakeSnapshot();
			Map->Undo();
		}


	}

	UpdateStatusBar(x, y);


	isMoving = FALSE;



	CView::OnMouseMove(nFlags, point);
}



void CIsoView::DrawCellCursor(const MapCoords& mapCoords, const DDSURFACEDESC2& desc)
{
	ProjectedVec drawOffset(0*4, 0*3);
	ProjectedCoords drawCoords2d = GetRenderTargetCoordinates(mapCoords, 0) + drawOffset;
	ProjectedCoords drawCoords = GetRenderTargetCoordinates(mapCoords) + drawOffset;

	static const COLORREF _cell_hilight_colors[16] = { 
		RGB(255, 255, 255),	// level 0
		RGB( 170, 0, 170),	// level 1
		RGB(0, 170, 170),	// level 2
		RGB(0, 170, 0),		// level 3
		RGB(90, 255, 90),	// level 4
		RGB(255, 255, 90),	// level 5
		RGB(255, 50, 50),	// level 6
		RGB(170, 85, 0),	// level 7
		RGB(170, 0, 0),		// level 8
		RGB(85, 255, 255),	// level 9
		RGB(80, 80, 255),	// level 10
		RGB(0, 0, 170),		// level 11
		RGB(0, 0, 0),		// level 12
		RGB(85,85 ,85),		// level 13
		RGB(170, 170, 170),	// level 14
		RGB(255, 255, 255)	// level 15
	};

	const FIELDDATA& m = *Map->GetFielddataAt(mapCoords.x + mapCoords.y * Map->GetIsoSize());
	int tileheight = m.bHeight;
	int new_tileheight = std::clamp(tileheight, 0, 16);

	DrawCell(desc.lpSurface, desc.dwWidth, desc.dwHeight, desc.lPitch, drawCoords.x, drawCoords.y, 1, 1, m_color_converter->GetColor(_cell_hilight_colors[new_tileheight]), false, true, m_color_converter->GetColor(60, 60, 255));
	
	LineDrawer l(desc.lpSurface, bpp, desc.dwWidth, desc.dwHeight, desc.lPitch);

	POINT p1, p2, p3, p4;
	p2.x = drawCoords2d.x + 1 * f_x / 2 + f_x / 2;
	p2.y = drawCoords2d.y + 1 * f_y / 2;
	p3.x = drawCoords2d.x + 1 * f_x / 2 - 1 * f_x / 2 + f_x / 2 - 1;
	p3.y = drawCoords2d.y + 1 * f_y / 2 + 1 * f_y / 2 - 1;
	p4.x = drawCoords2d.x - 1 * f_x / 2 + f_x / 2 - 1;
	p4.y = drawCoords2d.y + 1 * f_y / 2 - 1;

	POINT p5, p6, p7, p8;
	p6.x = drawCoords.x + 1 * f_x / 2 + f_x / 2;
	p6.y = drawCoords.y + 1 * f_y / 2;
	p7.x = drawCoords.x + 1 * f_x / 2 - 1 * f_x / 2 + f_x / 2 - 1;
	p7.y = drawCoords.y + 1 * f_y / 2 + 1 * f_y / 2 - 1;
	p8.x = drawCoords.x - 1 * f_x / 2 + f_x / 2 - 1;
	p8.y = drawCoords.y + 1 * f_y / 2 - 1;

	auto col = m_color_converter->GetColor(60, 60, 60);
	if (drawCoords2d != drawCoords)
	{
		for (int y = 0; y < 2; ++y)
		{
			l.MoveTo(p2.x + 1, p2.y + y);
			l.LineTo(p6.x + 1, p6.y + y, col, LineStyle::Dotted_4);
			l.MoveTo(p3.x, p3.y + 1 + y);
			l.LineTo(p7.x, p7.y + 1 + y, col, LineStyle::Dotted_4);
			l.MoveTo(p4.x - 1, p4.y + y);
			l.LineTo(p8.x - 1, p8.y + y, col, LineStyle::Dotted_4);

			l.MoveTo(p2.x, p2.y + y);
			l.LineTo(p6.x, p6.y + y, col, LineStyle::Dotted_4);
			l.MoveTo(p3.x + 1, p3.y + 1 + y);
			l.LineTo(p7.x + 1, p7.y + 1 + y, col, LineStyle::Dotted_4);
			l.MoveTo(p4.x, p4.y + y);
			l.LineTo(p8.x, p8.y + y, col, LineStyle::Dotted_4);
		}
	}
}

const int valadded = 10000;


void CIsoView::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (rscroll)
	{
		if (b_IsLoading) return;

		ReleaseCapture();
		KillTimer(11);
		ShowCursor(TRUE);
		CMyViewFrame& dlg = *(CMyViewFrame*)owner;
		dlg.m_minimap.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

	}


	if (nFlags == 0 && point.x == 0 && point.y == 0)
	{
		m_drag = FALSE; return;
	}

	CIniFile& ini = Map->GetIniFile();

	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

	if (!rscroll) m_drag = FALSE;

	const auto projCoords = GetProjectedCoordinatesFromClientCoordinates(point);
	const MapCoords mapCoords = GetMapCoordinatesFromClientCoordinates(point, (nFlags & MK_CONTROL) == MK_CONTROL);

	const int x = mapCoords.x; // compat
	const int y = mapCoords.y;

	// context menu
	if (AD.mode != 0 && !rscroll)
	{
		bool ignoreClick = false;
		if (AD.mode == ACTIONMODE_MAPTOOL)
		{
			if (AD.tool)
				ignoreClick = AD.tool->onRButtonUp(projCoords, mapCoords, MapToolMouseFlagsFromWin32(nFlags));
		}

		if (!ignoreClick)
		{
			AD.reset();

			CMyViewFrame& frame = *((CMyViewFrame*)owner);
			frame.m_objectview->GetTreeCtrl().Select(frame.m_objectview->GetTreeCtrl().GetRootItem(), TVGN_CARET);
		}
		return;
	}

	rscroll = FALSE;

	return;




	CView::OnRButtonUp(nFlags, point);
}

// from old (popup menu) interface, taken over and used by OnLButtonDblClick
BOOL CIsoView::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wNotifyCode = HIWORD(wParam); // notification code 
	int wID = LOWORD(wParam);         // item, control, or accelerator identifier 
	HWND hwndCtl = (HWND)lParam;      // handle of control 

	CIniFile& ini = Map->GetIniFile();


	if (wID < valadded)
	{
		// a standard menu, no popup!

		switch (wID)
		{
		case 4:
		{
			last_succeeded_operation = 54310;
			//get the number
			if (Map->GetAirAt(m_mapx + m_mapy * Map->GetIsoSize()) != -1)
			{
				HandleProperties(Map->GetAirAt(m_mapx + m_mapy * Map->GetIsoSize()), 2);
			}
			else if (Map->GetInfantryAt(m_mapx + m_mapy * Map->GetIsoSize()) != -1)
			{
				last_succeeded_operation = 54311;
				int z;
				for (z = 0;z < SUBPOS_COUNT;z++)
					if (Map->GetInfantryAt(m_mapx + m_mapy * Map->GetIsoSize(), z) != -1) HandleProperties(Map->GetInfantryAt(m_mapx + m_mapy * Map->GetIsoSize(), z), 0);
			}
			else if (Map->GetUnitAt(m_mapx + m_mapy * Map->GetIsoSize()) != -1)
			{
				HandleProperties(Map->GetUnitAt(m_mapx + m_mapy * Map->GetIsoSize()), 3);
			}
			else if (Map->GetStructureAt(m_mapx + m_mapy * Map->GetIsoSize()) != -1)
			{
				HandleProperties(Map->GetStructureAt(m_mapx + m_mapy * Map->GetIsoSize()), 1);
			}


			break;
		}

		}

		return 0;
	}





	return CView::OnCommand(wParam, lParam);
}

void CIsoView::HandleProperties(int n, int type)
{
	CIniFile& ini = Map->GetIniFile();

	if (n < 0) return;
	switch (type)
	{
	case 0:
	{
		last_succeeded_operation = 54312;

		// infantry

		INFANTRY data;

		Map->GetInfantryData(n, &data);

		last_succeeded_operation = 54313;


		CInfantrie dlg(this);
		char tmp[255];
		dlg.Init((LPCTSTR)data.house, (LPCTSTR)data.strength, (LPCTSTR)data.action,
			(LPCTSTR)data.direction, (LPCTSTR)data.tag, (LPCTSTR)data.flag1,
			(LPCTSTR)data.flag2, (LPCTSTR)data.flag3, (LPCTSTR)data.flag4, (LPCTSTR)data.flag5);

		int res = dlg.DoModal();
		if (res == IDCANCEL) return;

		data.action = dlg.m_action;
		data.strength = dlg.m_strength;
		data.tag = dlg.m_tag;
		data.direction = dlg.m_direction;
		data.house = dlg.m_house;
		data.flag1 = dlg.m_flag1;
		data.flag2 = dlg.m_flag2;
		data.flag3 = dlg.m_flag3;
		data.flag4 = dlg.m_flag4;
		data.flag5 = dlg.m_flag5;


		Map->DeleteInfantry(n);
		Map->AddInfantry(&data);

		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		break;
	}
	case 1:
	{
		// building

		STRUCTURE data;
		Map->GetStructureData(n, &data);


		CBuilding dlg(this);
		char tmp[255];
		dlg.Init((LPCTSTR)data.house, (LPCTSTR)data.strength, (LPCTSTR)data.direction, (LPCTSTR)data.tag,
			(LPCTSTR)data.flag1, (LPCTSTR)data.flag2, (LPCTSTR)data.energy,
			(LPCTSTR)data.upgradecount, (LPCTSTR)data.spotlight, (LPCTSTR)data.upgrade1, (LPCTSTR)data.upgrade2,
			(LPCTSTR)data.upgrade3, (LPCTSTR)data.flag3, (LPCTSTR)data.flag4);
		dlg.m_type = data.type;

		int res = dlg.DoModal();
		if (res == IDCANCEL) return;

		data.strength = dlg.m_strength;
		data.tag = dlg.m_tag;
		data.direction = dlg.m_direction;
		data.house = dlg.m_house;
		data.spotlight = dlg.m_spotlight;
		data.flag1 = dlg.m_flag1;
		data.flag2 = dlg.m_flag2;
		data.flag3 = dlg.m_flag3;
		data.flag4 = dlg.m_flag4;
		data.energy = dlg.m_energy;
		data.upgradecount = dlg.m_upgradecount;
		data.upgrade1 = dlg.m_upgrade1;
		data.upgrade2 = dlg.m_upgrade2;
		data.upgrade3 = dlg.m_upgrade3;



		Map->DeleteStructure(n);
		Map->AddStructure(&data);

		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		break;

	}
	case 2:
	{
		// aircraft

		AIRCRAFT data;
		Map->GetAircraftData(n, &data);


		CAircraft dlg(this);
		char tmp[255];
		dlg.Init(data.house, data.strength, data.direction, data.action,
			data.tag, data.flag1, data.flag2, data.flag3, data.flag4);

		int res = dlg.DoModal();
		if (res == IDCANCEL) return;

		data.action = dlg.m_action;
		data.strength = dlg.m_strength;
		data.tag = dlg.m_tag;
		data.direction = dlg.m_direction;
		data.house = dlg.m_house;
		data.flag1 = dlg.m_flag1;
		data.flag2 = dlg.m_flag2;
		data.flag3 = dlg.m_flag3;
		data.flag4 = dlg.m_flag4;


		Map->DeleteAircraft(n);
		Map->AddAircraft(&data);

		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		break;
	}
	case 3:
	{
		// vehicle

		UNIT data;
		Map->GetUnitData(n, &data);


		CUnit dlg(this);
		char tmp[255];
		dlg.Init(data.house, data.strength, data.direction, data.action,
			data.tag, data.flag1, data.flag2, data.flag3, data.flag4, data.flag5, data.flag6);

		int res = dlg.DoModal();
		if (res == IDCANCEL) return;

		data.action = dlg.m_action;
		data.strength = dlg.m_strength;
		data.tag = dlg.m_tag;
		data.direction = dlg.m_direction;
		data.house = dlg.m_house;
		data.flag1 = dlg.m_flag1;
		data.flag2 = dlg.m_flag2;
		data.flag3 = dlg.m_flag3;
		data.flag4 = dlg.m_flag4;
		data.flag5 = dlg.m_flag5;
		data.flag6 = dlg.m_flag6;

		Map->DeleteUnit(n);
		Map->AddUnit(&data);

		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		break;


	}
	}
}

void CIsoView::OnLButtonDblClk(UINT nFlags, CPoint point)
{


	if (b_IsLoading == TRUE) return;

	m_NoMove = TRUE;
	m_drag = FALSE;

	const auto projCoords = GetProjectedCoordinatesFromClientCoordinates(point);
	const MapCoords mapCoords = GetMapCoordinatesFromClientCoordinates(point, (nFlags & MK_CONTROL) == MK_CONTROL);

	if (AD.mode == ACTIONMODE_MAPTOOL)
	{
		if (AD.tool)
			AD.tool->onLButtonDblClick(projCoords, mapCoords, MapToolMouseFlagsFromWin32(nFlags));
	}
	else
	{
		if (!Map->isInside(mapCoords))
			return;

		m_mapx = mapCoords.x;
		m_mapy = mapCoords.y;

		int pos = m_mapx + m_mapy * Map->GetIsoSize();
		if (Map->GetCelltagAt(pos) < 0)
		{
			OnCommand(4, 0);
		}
		else
			OnCommand(9, 0);
	}

	m_NoMove = FALSE;

	CView::OnLButtonDblClk(nFlags, point);
}

void CIsoView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (Map->GetIsoSize() == 0) return;
	if (b_IsLoading == TRUE) return;


	CIniFile& ini = Map->GetIniFile();

	const auto projCoords = GetProjectedCoordinatesFromClientCoordinates(point);
	const MapCoords mapCoords = GetMapCoordinatesFromClientCoordinates(point, (nFlags & MK_CONTROL) == MK_CONTROL);

	

	const int x = mapCoords.x; // compat
	const int y = mapCoords.y;


	int mapwidth = Map->GetWidth();
	int mapheight = Map->GetHeight();
	//if(x>=Map->GetIsoSize() || y>=Map->GetIsoSize() || x<0 || y<0) return;
	if (AD.mode != ACTIONMODE_COPY && AD.mode != ACTIONMODE_PASTE && AD.mode != ACTIONMODE_MAPTOOL)
	{
		if (!Map->isInside(mapCoords))
			return;
		if (x < 1 || y < 1 || x + y<mapwidth + 1 || x + y>mapwidth + mapheight * 2 || (y + 1 > mapwidth && x - 1 < y - mapwidth) || (x + 1 > mapwidth && y + mapwidth - 1 < x))
			return;
	}

	int px = m_mapx;
	int py = m_mapy;

	m_mapx = x;
	m_mapy = y;

	CStatusBarCtrl& stat = ((CMyViewFrame*)owner)->m_statbar.GetStatusBarCtrl();

	if (AD.mode == ACTIONMODE_CLIFFFRONT || AD.mode == ACTIONMODE_CLIFFBACK)
	{
		m_drag = TRUE;
	}
	else if (AD.mode == ACTIONMODE_PASTE)
	{
		Map->TakeSnapshot();
		Map->Paste(m_mapx, m_mapy, AD.z_data);
		Map->TakeSnapshot();
		Map->Undo();
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (AD.mode == ACTIONMODE_COPY)
	{
		if (!m_drag)
		{
			m_drag = TRUE;
		}
		else // 2nd click
		{
			int x1, x2, y1, y2;
			if (px < x)
			{
				x1 = px;
				x2 = x;
			}
			else
			{
				x1 = x;
				x2 = px;
			}
			if (py < y)
			{
				y1 = py;
				y2 = y;
			}
			else
			{
				y1 = y;
				y2 = py;
			}


			Map->Copy(x1, y1, x2 + 1, y2 + 1);

			m_drag = FALSE;
		}
	}
	else if (Map->IsGroundObjectAt(m_mapx + m_mapy * Map->GetIsoSize()) && AD.mode == 0 && m_NoMove == FALSE)
	{
		m_drag = TRUE;
		m_moved = FALSE;
		m_id = Map->GetInfantryAt(m_mapx + m_mapy * Map->GetIsoSize());
		m_type = 0;
		if (m_id < 0)
		{
			m_id = Map->GetUnitAt(m_mapx + m_mapy * Map->GetIsoSize());
			m_type = 3;
		}
		if (m_id < 0)
		{
			m_id = Map->GetAirAt(m_mapx + m_mapy * Map->GetIsoSize());
			m_type = 2;
		}
		if (m_id < 0)
		{
			m_id = Map->GetTerrainAt(m_mapx + m_mapy * Map->GetIsoSize());
			m_type = 4;
		}
		if (m_id < 0)
		{
			m_id = Map->GetStructureAt(m_mapx + m_mapy * Map->GetIsoSize());

			STDOBJECTDATA sod;
			Map->GetStdStructureData(m_id, &sod);
			m_mapx = atoi(sod.x);
			m_mapy = atoi(sod.y);

			m_type = 1;
		}

	}
	else if (AD.mode == ACTIONMODE_SETTILE)
	{


		if ((nFlags & MK_CONTROL) && !(nFlags & MK_SHIFT))
		{
			// Fill the whole area
			Sound(SOUND_LAYDOWNTILE);

			Map->TakeSnapshot();
			int i;
			for (i = 0;i < Map->GetIsoSize() * Map->GetIsoSize();i++)
				Map->SetReserved(i, 0);

			FillArea(x, y, AD.type, 0);
			Map->TakeSnapshot();
			Map->Undo();
		}
		else
		{
			// just place a single tile

			if (!(nFlags & MK_SHIFT))
			{
				Sound(SOUND_LAYDOWNTILE);
			}

			PlaceTile(x, y, nFlags);
		}




		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (AD.mode == ACTIONMODE_HIDETILESET)
	{
		int ground = Map->GetFielddataAt(x + (y)*Map->GetIsoSize())->wGround;
		if (ground == 0xFFFF) ground = 0;

		//if(ground!=0)
		HideTileSet((*tiledata)[ground].wTileSet);
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (AD.mode == ACTIONMODE_HIDEFIELD)
	{
		Map->HideField(x + y * Map->GetIsoSize(), TRUE);
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (AD.mode == ACTIONMODE_HEIGHTEN)
	{
		Map->TakeSnapshot();

		m_TileChangeCount = 0;
		//if(x==m_mapx && y==m_mapy)
		//try
		{
			int ground = Map->GetFielddataAt(x + (y)*Map->GetIsoSize())->wGround;
			if (ground == 0xFFFF) ground = 0;

			if ((nFlags & MK_CONTROL) && !(*tiledata)[ground].bMorphable)
			{
				SetError("Applying non-morphable terrain height change, please wait...");
				stat.RedrawWindow();
				stat.UpdateWindow();
			}

			m_funcRect.left = x;
			m_funcRect.top = y;
			m_funcRect.bottom = y;
			m_funcRect.right = x;

			if ((*tiledata)[ground].bMorphable)
			{
				int f, n;
				int oheight = Map->GetHeightAt(x + y * Map->GetIsoSize());
				for (f = -m_BrushSize_x / 2;f < m_BrushSize_x / 2 + 1;f++)
				{
					for (n = -m_BrushSize_y / 2;n < m_BrushSize_y / 2 + 1;n++)
					{
						int pos = x + f + (y + n) * Map->GetIsoSize();
						int ground = Map->GetFielddataAt(pos)->wGround;
						if (ground == 0xFFFF) ground = 0;

						if ((*tiledata)[ground].bMorphable && Map->GetHeightAt(pos) == oheight)
						{
							Map->SetHeightAt(pos, oheight + 1);
						}
					}
				}
				for (f = -m_BrushSize_x / 2;f < m_BrushSize_x / 2 + 1;f++)
				{
					for (n = -m_BrushSize_y / 2;n < m_BrushSize_y / 2 + 1;n++)
					{
						int pos = x + f + (y + n) * Map->GetIsoSize();
						int ground = Map->GetFielddataAt(pos)->wGround;
						if (ground == 0xFFFF) ground = 0;

						if ((*tiledata)[ground].bMorphable && Map->GetHeightAt(x + f + (y + n) * Map->GetIsoSize()) == oheight + 1)
						{
							int i;
							for (i = 0;i < Map->GetIsoSize() * Map->GetIsoSize();i++)
								Map->SetReserved(i, 0);

							ChangeTileHeight(x + f + (y + n) * Map->GetIsoSize(), oheight + 1, FALSE, (nFlags & MK_CONTROL), TRUE);
						}
					}
				}
			}
			else
			{
				int i;
				for (i = 0;i < Map->GetIsoSize() * Map->GetIsoSize();i++)
					Map->SetReserved(i, 0);

				// TODO: replace recursion, otherwise be aware of stack issues!
				try
				{
					ChangeTileHeight(x + (y)*Map->GetIsoSize(), Map->GetHeightAt(x + y * Map->GetIsoSize()) + 1, FALSE, !(nFlags & MK_CONTROL));
				}
				catch (...)
				{
					MessageBox("Stack is too small to complete operation!", "Error");
				}
			}

			int f, n;
			for (f = m_funcRect.left - 1;f <= m_funcRect.right + 1;f++)
			{
				for (n = m_funcRect.top - 1;n <= m_funcRect.bottom + 1;n++)
				{
					int pos = f + (n)*Map->GetIsoSize();
					Map->CreateSlopesAt(pos);
				}
			}


			/*CHANGEHEIGHTDATA d;
			d.pos=x+y*Map->GetIsoSize();
			d.bNonMorpheable=FALSE;
			d.toHeight=Map->GetHeightAt(x+y*Map->GetIsoSize())+1;
			DWORD id;
			HANDLE hThread=CreateThread(NULL, 4000000, ChangeHeightThread, &d, NULL, &id);
			SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
			WaitForSingleObjectEx(hThread, INFINITE, FALSE);*/



			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			Map->TakeSnapshot();
			Map->Undo();
		}
		//catch(...)
		{
			//	MessageBox("Error occured","Error");
		}
	}
	else if (AD.mode == ACTIONMODE_LOWER)
	{
		Map->TakeSnapshot();

		m_TileChangeCount = 0;
		//if(x==m_mapx && y==m_mapy)
		//try
		{
			int ground = Map->GetFielddataAt(x + (y)*Map->GetIsoSize())->wGround;
			if (ground == 0xFFFF) ground = 0;

			if ((nFlags & MK_CONTROL) && !(*tiledata)[ground].bMorphable)
			{
				SetError("Applying non-morphable terrain height change, please wait...");
				stat.RedrawWindow();
				stat.UpdateWindow();
			}

			int i;

			m_funcRect.left = x;
			m_funcRect.top = y;
			m_funcRect.bottom = y;
			m_funcRect.right = x;

			if ((*tiledata)[ground].bMorphable)
			{
				int f, n;
				int oheight = Map->GetHeightAt(x + y * Map->GetIsoSize());
				for (f = -m_BrushSize_x / 2;f < m_BrushSize_x / 2 + 1;f++)
				{
					for (n = -m_BrushSize_y / 2;n < m_BrushSize_y / 2 + 1;n++)
					{
						int pos = x + f + (y + n) * Map->GetIsoSize();
						int ground = Map->GetFielddataAt(pos)->wGround;
						if (ground == 0xFFFF) ground = 0;

						if ((*tiledata)[ground].bMorphable && Map->GetHeightAt(pos) == oheight)
						{
							Map->SetHeightAt(pos, oheight - 1);
						}
					}
				}
				for (f = -m_BrushSize_x / 2;f < m_BrushSize_x / 2 + 1;f++)
				{
					for (n = -m_BrushSize_y / 2;n < m_BrushSize_y / 2 + 1;n++)
					{
						int pos = x + f + (y + n) * Map->GetIsoSize();
						int ground = Map->GetFielddataAt(pos)->wGround;
						if (ground == 0xFFFF) ground = 0;

						if ((*tiledata)[ground].bMorphable && Map->GetHeightAt(x + f + (y + n) * Map->GetIsoSize()) == oheight - 1)
						{
							for (i = 0;i < Map->GetIsoSize() * Map->GetIsoSize();i++)
								Map->SetReserved(i, 0);

							ChangeTileHeight(x + f + (y + n) * Map->GetIsoSize(), oheight - 1, FALSE, (nFlags & MK_CONTROL), TRUE);
						}
					}
				}
			}
			else
			{
				for (i = 0;i < Map->GetIsoSize() * Map->GetIsoSize();i++)
					Map->SetReserved(i, 0);

				try
				{
					ChangeTileHeight(x + (y)*Map->GetIsoSize(), Map->GetHeightAt(x + y * Map->GetIsoSize()) - 1, FALSE, !(nFlags & MK_CONTROL));
				}
				catch (...)
				{
					MessageBox("Stack is too small to complete operation!", "Error");
				}
			}

			int f, n;
			for (f = m_funcRect.left - 1;f <= m_funcRect.right + 1;f++)
			{
				for (n = m_funcRect.top - 1;n <= m_funcRect.bottom + 1;n++)
				{
					int pos = f + (n)*Map->GetIsoSize();
					Map->CreateSlopesAt(pos);
				}
			}

			/*CHANGEHEIGHTDATA d;
			d.pos=x+y*Map->GetIsoSize();
			d.bNonMorpheable=FALSE;
			d.toHeight=Map->GetHeightAt(x+y*Map->GetIsoSize())-1;
			DWORD id;
			HANDLE hThread=CreateThread(NULL, 4000000, ChangeHeightThread, &d, NULL, &id);
			SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
			WaitForSingleObjectEx(hThread, INFINITE, FALSE);*/

			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
		//catch(...)
		{
			//	MessageBox("Error occured","Error");
		}

		Map->TakeSnapshot();
		Map->Undo();
	}
	else if (AD.mode == ACTIONMODE_HEIGHTENTILE)
	{
		Map->TakeSnapshot();

		int n, m;
		for (m = -m_BrushSize_x / 2;m < m_BrushSize_x / 2 + 1;m++)
		{
			for (n = -m_BrushSize_y / 2;n < m_BrushSize_y / 2 + 1;n++)
			{
				Map->SetHeightAt(x + m + (y + n) * Map->GetIsoSize(), Map->GetHeightAt(x + m + (y + n) * Map->GetIsoSize()) + 1);
			}
		}

		if (nFlags & MK_CONTROL)
			for (m = -m_BrushSize_x / 2 - 1;m < m_BrushSize_x / 2 + 2;m++)
			{
				for (n = -m_BrushSize_y / 2 - 1;n < m_BrushSize_y / 2 + 2;n++)
				{
					Map->CreateSlopesAt(x + m + (y + n) * Map->GetIsoSize());
				}
			}

		Map->TakeSnapshot();
		Map->Undo();

		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (AD.mode == ACTIONMODE_LOWERTILE)
	{
		Map->TakeSnapshot();

		int n, m;
		for (m = -m_BrushSize_x / 2;m < m_BrushSize_x / 2 + 1;m++)
		{
			for (n = -m_BrushSize_y / 2;n < m_BrushSize_y / 2 + 1;n++)
			{
				Map->SetHeightAt(x + m + (y + n) * Map->GetIsoSize(), Map->GetHeightAt(x + m + (y + n) * Map->GetIsoSize()) - 1);
			}
		}

		if (nFlags & MK_CONTROL)
			for (m = -m_BrushSize_x / 2 - 1;m < m_BrushSize_x / 2 + 2;m++)
			{
				for (n = -m_BrushSize_y / 2 - 1;n < m_BrushSize_y / 2 + 2;n++)
				{
					Map->CreateSlopesAt(x + m + (y + n) * Map->GetIsoSize());
				}
			}

		Map->TakeSnapshot();
		Map->Undo();

		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (AD.mode == ACTIONMODE_FLATTENGROUND)
	{
		Map->TakeSnapshot();

		m_FlattenHeight = Map->GetHeightAt(x + y * Map->GetIsoSize());
		m_FlattenLastX = x;
		m_FlattenLastY = y;

		Map->TakeSnapshot();
		Map->Undo();
	}
	else if (Map->GetWaypointAt(m_mapx + m_mapy * Map->GetIsoSize()) >= 0 && AD.mode == 0)
	{
		m_drag = TRUE;
		m_id = Map->GetWaypointAt(m_mapx + m_mapy * Map->GetIsoSize());
		m_type = 6;
	}
	else if (Map->GetCelltagAt(m_mapx + m_mapy * Map->GetIsoSize()) >= 0 && AD.mode == 0)
	{
		m_drag = TRUE;
		m_id = Map->GetCelltagAt(m_mapx + m_mapy * Map->GetIsoSize());
		m_type = 5;
	}
	else if ((AD.mode < 3 || AD.mode>4) || (AD.mode == 3 && AD.type == 0) || (AD.mode >= 3 && AD.mode <= 4 && AD.type == 1))
	{
		OnMouseMove(nFlags, point);
	}
	else if (AD.mode == 3)
	{
		if (AD.type == 2)
		{
			// create waypoint
			CWaypointID w(this);
			if (w.DoModal() != IDCANCEL)
			{
				int n = w.m_value;

				char id[50];
				itoa(n, id, 10);
				Map->AddWaypoint(id, x + y * Map->GetIsoSize());
				RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
		}
		if (AD.type >= 3) OnMouseMove(nFlags, point);

	}

	else if (AD.mode == 4)
	{
		if (AD.type == 0)
		{
			// create celltag
			CCellTag w(this);
			if (w.DoModal() != IDCANCEL)
			{
				Map->AddCelltag(w.m_tag, x + y * Map->GetIsoSize());

				RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
		}
		if (AD.type == 2)
		{
			// change celltag properties
			int n = Map->GetCelltagAt(x + y * Map->GetIsoSize());
			if (n < 0) return;

			DWORD dwPos;
			CString tag;

			Map->GetCelltagData(n, &tag, &dwPos);



			CCellTag dlg(this);

			dlg.m_tag = tag;
			dlg.m_tag += " (";
			dlg.m_tag += GetParam(Map->GetIniFile().sections["Tags"].values[(LPCTSTR)tag], 1);
			dlg.m_tag += ")";

			if (dlg.DoModal() == IDCANCEL) return;

			Map->DeleteCelltag(n);

			Map->AddCelltag(dlg.m_tag, dwPos);

			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}

	}



	CView::OnLButtonDown(nFlags, point);
}

void CIsoView::PlaceTile(const int x, const int y, const UINT nMouseFlags)
{
	Map->TakeSnapshot(TRUE, x - 6, y - 6, x + (*tiledata)[AD.type].cx * m_BrushSize_x + 7, y + (*tiledata)[AD.type].cy * m_BrushSize_y + 7);
	int i, e, f, n;
	int p = 0;
	int width = (*tiledata)[AD.type].cx;
	int height = (*tiledata)[AD.type].cy;
	int pos = x - width + 1 + (y - height + 1) * Map->GetIsoSize();
	int startheight = Map->GetHeightAt(x + y * Map->GetIsoSize()) + AD.z_data;
	int ground = Map->GetFielddataAt(x + y * Map->GetIsoSize())->wGround;
	if (ground == 0xFFFF) ground = 0;
	startheight -= (*tiledata)[ground].tiles[Map->GetFielddataAt(x + y * Map->GetIsoSize())->bSubTile].bZHeight;
	for (f = 0;f < m_BrushSize_x;f++)
	{
		for (n = 0;n < m_BrushSize_y;n++)
		{
			int tile = AD.type;
			if (AD.data == 1)
			{
				int n = rand() * 5 / RAND_MAX;
				tile += n;

			}
			p = 0;
			for (i = 0;i < (*tiledata)[AD.type].cx;i++)
			{
				for (e = 0;e < (*tiledata)[AD.type].cy;e++)
				{

					if (x - width + 1 + f * width + i >= Map->GetIsoSize() ||
						y - height + 1 + n * height + e >= Map->GetIsoSize())
					{

					}
					else
						if ((*tiledata)[AD.type].tiles[p].pic != NULL)
						{
							Map->SetHeightAt(pos + i + f * width + (e + n * height) * Map->GetIsoSize(), startheight + (*tiledata)[AD.type].tiles[p].bZHeight);
							Map->SetTileAt(pos + i + f * width + (e + n * height) * Map->GetIsoSize(), tile, p);

						}
					p++;
				}
			}

		}
	}

	if (!((nMouseFlags & MK_CONTROL) && (nMouseFlags & MK_SHIFT)))
	{
		if (!theApp.m_Options.bDisableAutoShore) Map->CreateShore(x - 5, y - 5, x + (*tiledata)[AD.type].cx * m_BrushSize_x + 5, y + (*tiledata)[AD.type].cy * m_BrushSize_y + 5, FALSE);
		//Map->CreateShore(0,0,Map->GetIsoSize(), Map->GetIsoSize());

		for (f = 0;f < m_BrushSize_x;f++)
		{
			for (n = 0;n < m_BrushSize_y;n++)
			{
				p = 0;
				for (i = -1;i < (*tiledata)[AD.type].cx + 1;i++)
				{
					for (e = -1;e < (*tiledata)[AD.type].cy + 1;e++)
					{

						Map->SmoothAllAt(pos + i + f * width + (e + n * height) * Map->GetIsoSize());

						p++;
					}
				}

			}
		}
	}

	// now make current tiles available for redo
	Map->TakeSnapshot(TRUE, x - 6, y - 6, x + (*tiledata)[AD.type].cx * m_BrushSize_x + 6, y + (*tiledata)[AD.type].cy * m_BrushSize_y + 6);
	Map->Undo();
}





void CIsoView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (b_IsLoading == TRUE) return;

	CIniFile& ini = Map->GetIniFile();

	const auto projCoords = GetProjectedCoordinatesFromClientCoordinates(point);
	const MapCoords mapCoords = GetMapCoordinatesFromClientCoordinates(point, (nFlags & MK_CONTROL) == MK_CONTROL);

		const int dx = mapCoords.x; // compat
	const int dy = mapCoords.y;


	//if(dx>=Map->GetIsoSize() || dy>=Map->GetIsoSize() || dx<0 || dy<0) return;
	int mapwidth = Map->GetWidth();
	int mapheight = Map->GetHeight();
	if (AD.mode != ACTIONMODE_COPY && AD.mode != ACTIONMODE_PASTE && AD.mode != ACTIONMODE_MAPTOOL)
	{
		if (!Map->isInside(mapCoords))
			return;
		if (dx < 1 || dy < 1 || dx + dy<mapwidth + 1 || dx + dy>mapwidth + mapheight * 2 || (dy + 1 > mapwidth && dx - 1 < dy - mapwidth) || (dx + 1 > mapwidth && dy + mapwidth - 1 < dx))
			return;
	}


	char fx[50], fy[50];
	itoa(dx, fx, 10);
	itoa(dy, fy, 10);

	int x, y;
	x = dx;
	y = dy;

	// check if user did select something else than 0 in object browser
	if (AD.mode != 0 && AD.mode != ACTIONMODE_COPY)
	{
		if (m_moved)
			m_drag = FALSE;
		m_moved = FALSE;
	}

	if (AD.mode == ACTIONMODE_FLATTENGROUND)
	{
		Map->TakeSnapshot();
		Map->Undo();
	}
	if (AD.mode == ACTIONMODE_CLIFFFRONT || AD.mode == ACTIONMODE_CLIFFBACK)
	{
		m_drag = FALSE;
		Map->Redo();
		/*Map->TakeSnapshot();

		if(AD.mode==ACTIONMODE_CLIFFFRONT)
		{
			CFrontCliffModifier f;
			f.PlaceCliff(m_mapx, m_mapy, x, y);
		}
		else if(AD.mode==ACTIONMODE_CLIFFBACK)
		{
			CBackCliffModifier f;
			f.PlaceCliff(m_mapx, m_mapy, x, y);
		}*/

		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	if (AD.mode == ACTIONMODE_COPY && m_drag)
	{
		/*m_drag=FALSE;

		int x1, x2, y1, y2;
		if(m_mapx<x) { x1=m_mapx; x2=x; } else { x1=x; x2=m_mapx; }
		if(m_mapy<y) { y1=m_mapy; y2=y; } else { y1=y; y2=m_mapy; }


		Map->Copy(x1, y1, x2+1, y2+1);*/
		m_moved = FALSE;
	}

	if (m_drag && m_moved)
	{
		m_moved = FALSE;

		// ok, drag the object at m_mapx/m_mapy to x/y
		int x = dx, y = dy;
		char strX[15], strY[15];
		itoa(x, strX, 10);
		itoa(y, strY, 10);
		switch (m_type)
		{
		case 0: // drag infantry
		{
			INFANTRY infantry;
			Map->GetInfantryData(m_id, &infantry);
			infantry.x = strX;
			infantry.y = strY;
			infantry.pos = "-1";

			if ((nFlags != MK_SHIFT))
			{
				Map->DeleteInfantry(m_id);
			}

			Map->AddInfantry(&infantry);

			break;
		}
		case 1: // drag structure
		{
			STRUCTURE structure;
			Map->GetStructureData(m_id, &structure);
			structure.x = strX;
			structure.y = strY;

			if ((nFlags != MK_SHIFT))
			{
				Map->DeleteStructure(m_id);
			}

			Map->AddStructure(&structure);

			break;
		}
		case 2: // drag aircraft
		{
			AIRCRAFT aircraft;
			Map->GetAircraftData(m_id, &aircraft);
			aircraft.x = strX;
			aircraft.y = strY;


			if ((nFlags != MK_SHIFT))
			{
				Map->DeleteAircraft(m_id);
			}

			Map->AddAircraft(&aircraft);

			break;
		}
		case 3: // drag vehicles
		{
			UNIT unit;
			Map->GetUnitData(m_id, &unit);
			unit.x = strX;
			unit.y = strY;

			if (!(nFlags == MK_SHIFT))
			{
				Map->DeleteUnit(m_id);
			}

			Map->AddUnit(&unit);

			break;
		}
		case 4: // drag terrain
		{
			CString type;

			Map->GetTerrainData(m_id, &type);

			if ((nFlags != MK_SHIFT))
			{
				Map->DeleteTerrain(m_id);
			}

			Map->AddTerrain(type, x + y * Map->GetIsoSize());
			break;
		}

		case 5: // drag celltag
		{
			CString tag;
			DWORD dwPos;
			Map->GetCelltagData(m_id, &tag, &dwPos);

			if ((nFlags != MK_SHIFT))
			{
				Map->DeleteCelltag(m_id);
			}

			Map->AddCelltag(tag, x + y * Map->GetIsoSize());
			break;
		}
		case 6: // drag waypoint
		{
			CString ID;
			DWORD dwPos;

			Map->GetWaypointData(m_id, &ID, &dwPos);

			if (!(nFlags == MK_SHIFT))
			{
				Map->DeleteWaypoint(m_id);
			}

			Map->AddWaypoint(ID, x + y * Map->GetIsoSize());
			break;
		}

		}



		m_drag = FALSE;
		line.left = 0;
		line.top = 0;
		line.right = 0;
		line.bottom = 0;

		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (AD.mode == ACTIONMODE_MAPTOOL)
	{
		if (AD.tool)
			AD.tool->onLButtonUp(projCoords, mapCoords, MapToolMouseFlagsFromWin32(nFlags));
	}
	else if (AD.mode == ACTIONMODE_PLACE) // painting stuff
	{
		m_moved = FALSE;
		if (AD.type == 6) // Overlay
		{
			if (AD.data == 5) // birdges
			{
				if (x == m_mapx && y == m_mapy) return;

				int ovrltype = 0x18;
				if (abs(x - m_mapx) >= abs(y - m_mapy))
				{
					// it´s from bottom left to top right
					y = m_mapy;
					ovrltype = 0x19;
				}
				else
				{
					// from bottom right to top left
					x = m_mapx;
					ovrltype = 0x18;
				}

				int tmp;

				if (x < m_mapx)
				{
					tmp = m_mapx;
					m_mapx = x;
					x = tmp;
				}
				if (y < m_mapy)
				{
					tmp = m_mapy;
					m_mapy = y;
					y = tmp;
				}

				// okay, now we can do it...
				int i;
				if (AD.data2 == 0) // big bridge
				{
					int startheight = Map->GetFielddataAt(m_mapx + m_mapy * Map->GetIsoSize())->bHeight;

					if (y == m_mapy)
					{
						for (i = m_mapx;i <= x;i++)
						{
							if (Map->GetFielddataAt(i + m_mapy * Map->GetIsoSize())->bHeight == startheight - 4)
							{
								startheight = Map->GetFielddataAt(i + m_mapy * Map->GetIsoSize())->bHeight;
								break;
							}
						}
					}
					else
					{
						for (i = m_mapy;i <= y;i++)
						{
							if (Map->GetFielddataAt(m_mapx + i * Map->GetIsoSize())->bHeight == startheight - 4)
							{
								startheight = Map->GetFielddataAt(m_mapx + i * Map->GetIsoSize())->bHeight;
								break;
							}
						}
					}

					if (y == m_mapy)
					{
						for (i = m_mapx;i <= x;i++)
						{
							if (Map->GetFielddataAt(i + m_mapy * Map->GetIsoSize())->bHeight == startheight)
							{
								Map->SetOverlayAt(i + m_mapy * Map->GetIsoSize(), ovrltype);
								Map->SetOverlayDataAt(i + m_mapy * Map->GetIsoSize(), 0x9);
							}
						}
					}
					else
					{
						for (i = m_mapy;i <= y;i++)
						{
							if (Map->GetFielddataAt(m_mapx + i * Map->GetIsoSize())->bHeight == startheight)
							{
								Map->SetOverlayAt(m_mapx + i * Map->GetIsoSize(), ovrltype);
								Map->SetOverlayDataAt(m_mapx + i * Map->GetIsoSize(), 0x0);
							}
						}
					}
				}
				if (AD.data2 == 2) // big track bridge
				{
					int startheight = Map->GetFielddataAt(m_mapx + m_mapy * Map->GetIsoSize())->bHeight;

					if (y == m_mapy)
					{
						for (i = m_mapx;i <= x;i++)
						{
							if (Map->GetFielddataAt(i + m_mapy * Map->GetIsoSize())->bHeight == startheight - 4)
							{
								startheight = Map->GetFielddataAt(i + m_mapy * Map->GetIsoSize())->bHeight;
								break;
							}
						}
					}
					else
					{
						for (i = m_mapy;i <= y;i++)
						{
							if (Map->GetFielddataAt(m_mapx + i * Map->GetIsoSize())->bHeight == startheight - 4)
							{
								startheight = Map->GetFielddataAt(m_mapx + i * Map->GetIsoSize())->bHeight;
								break;
							}
						}
					}

					if (y == m_mapy)
					{
						for (i = m_mapx;i <= x;i++)
						{
							if (Map->GetFielddataAt(i + m_mapy * Map->GetIsoSize())->bHeight == startheight)
							{

								Map->SetOverlayAt(i + m_mapy * Map->GetIsoSize(), 0x3c);
#ifdef RA2_MODE
								Map->SetOverlayAt(i + m_mapy * Map->GetIsoSize(), 0xee);
#endif
								Map->SetOverlayDataAt(i + m_mapy * Map->GetIsoSize(), 0x9);
							}
						}
					}
					else
					{
						for (i = m_mapy;i <= y;i++)
						{
							if (Map->GetFielddataAt(m_mapx + i * Map->GetIsoSize())->bHeight == startheight)
							{
								Map->SetOverlayAt(m_mapx + i * Map->GetIsoSize(), 0x3b);
#ifdef RA2_MODE
								Map->SetOverlayAt(m_mapx + i * Map->GetIsoSize(), 0xed);
#endif
								Map->SetOverlayDataAt(m_mapx + i * Map->GetIsoSize(), 0x0);
							}
						}
					}

					/*if(y==m_mapy)
					{
						for(i=m_mapx;i<=x;i++)
						{
							Map->SetOverlayAt(i+m_mapy*Map->GetIsoSize(), 0x3b);
							Map->SetOverlayDataAt(i+m_mapy*Map->GetIsoSize(), 0x9);
						}
					}
					else
					{
						for(i=m_mapy;i<=y;i++)
						{
							Map->SetOverlayAt(m_mapx+i*Map->GetIsoSize(), 0x3b);
							Map->SetOverlayDataAt(m_mapx+i*Map->GetIsoSize(), 0x0);
						}
					}*/
				}
				else if (AD.data2 == 1 || AD.data2 == 3) // small bridge
				{
					int start = 0x4a;
					if (AD.data2 == 3) start = 0xcd;

					if (y == m_mapy)
					{
						srand((unsigned)time(NULL));
						for (i = m_mapx + 1;i <= x - 1;i++)
						{

							int v2 = ((float)rand() / (float)RAND_MAX) * 4;

							Map->SetOverlayAt(i + (m_mapy - 1) * Map->GetIsoSize(), start + 9 + v2);
							Map->SetOverlayDataAt(i + (m_mapy - 1) * Map->GetIsoSize(), 0x0);
							Map->SetOverlayAt(i + (m_mapy + 0) * Map->GetIsoSize(), start + 9 + v2);
							Map->SetOverlayDataAt(i + (m_mapy + 0) * Map->GetIsoSize(), 0x1);
							Map->SetOverlayAt(i + (m_mapy + 1) * Map->GetIsoSize(), start + 9 + v2);
							Map->SetOverlayDataAt(i + (m_mapy + 1) * Map->GetIsoSize(), 0x2);
						}
						Map->SetOverlayAt(m_mapx + (m_mapy - 1) * Map->GetIsoSize(), start + 22);
						Map->SetOverlayAt(m_mapx + (m_mapy - 0) * Map->GetIsoSize(), start + 22);
						Map->SetOverlayAt(m_mapx + (m_mapy + 1) * Map->GetIsoSize(), start + 22);
						Map->SetOverlayDataAt(m_mapx + (m_mapy - 1) * Map->GetIsoSize(), 0x0);
						Map->SetOverlayDataAt(m_mapx + (m_mapy - 0) * Map->GetIsoSize(), 0x1);
						Map->SetOverlayDataAt(m_mapx + (m_mapy + 1) * Map->GetIsoSize(), 0x2);

						Map->SetOverlayAt(x + (m_mapy - 1) * Map->GetIsoSize(), start + 24);
						Map->SetOverlayAt(x + (m_mapy - 0) * Map->GetIsoSize(), start + 24);
						Map->SetOverlayAt(x + (m_mapy + 1) * Map->GetIsoSize(), start + 24);
						Map->SetOverlayDataAt(x + (m_mapy - 1) * Map->GetIsoSize(), 0x0);
						Map->SetOverlayDataAt(x + (m_mapy - 0) * Map->GetIsoSize(), 0x1);
						Map->SetOverlayDataAt(x + (m_mapy + 1) * Map->GetIsoSize(), 0x2);


					}
					else
					{
						srand((unsigned)time(NULL));

						for (i = m_mapy + 1;i <= y - 1;i++)
						{

							int v2 = ((float)rand() / (float)RAND_MAX) * 4;

							Map->SetOverlayAt((m_mapx - 1) + i * Map->GetIsoSize(), start + v2);
							Map->SetOverlayDataAt((m_mapx - 1) + i * Map->GetIsoSize(), 0x0);
							Map->SetOverlayAt((m_mapx - 0) + i * Map->GetIsoSize(), start + v2);
							Map->SetOverlayDataAt((m_mapx - 0) + i * Map->GetIsoSize(), 0x1);
							Map->SetOverlayAt((m_mapx + 1) + i * Map->GetIsoSize(), start + v2);
							Map->SetOverlayDataAt((m_mapx + 1) + i * Map->GetIsoSize(), 0x2);
						}
						Map->SetOverlayAt((m_mapx - 1) + m_mapy * Map->GetIsoSize(), start + 20);
						Map->SetOverlayAt((m_mapx - 0) + m_mapy * Map->GetIsoSize(), start + 20);
						Map->SetOverlayAt((m_mapx + 1) + m_mapy * Map->GetIsoSize(), start + 20);
						Map->SetOverlayDataAt((m_mapx - 1) + m_mapy * Map->GetIsoSize(), 0x0);
						Map->SetOverlayDataAt((m_mapx - 0) + m_mapy * Map->GetIsoSize(), 0x1);
						Map->SetOverlayDataAt((m_mapx + 1) + m_mapy * Map->GetIsoSize(), 0x2);

						Map->SetOverlayAt((m_mapx - 1) + y * Map->GetIsoSize(), start + 18);
						Map->SetOverlayAt((m_mapx - 0) + y * Map->GetIsoSize(), start + 18);
						Map->SetOverlayAt((m_mapx + 1) + y * Map->GetIsoSize(), start + 18);
						Map->SetOverlayDataAt((m_mapx - 1) + y * Map->GetIsoSize(), 0x0);
						Map->SetOverlayDataAt((m_mapx + 0) + y * Map->GetIsoSize(), 0x1);
						Map->SetOverlayDataAt((m_mapx + 1) + y * Map->GetIsoSize(), 0x2);


					}
				}




				//UpdateMap();
				RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
		}
	}

	CFinalSunDlg& dlg = *(CFinalSunDlg*)theApp.m_pMainWnd;
	dlg.m_view.m_minimap.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

	m_moved = FALSE;

	CView::OnLButtonUp(nFlags, point);
}




void CIsoView::TextOut(int x, int y, const char* text, COLORREF col)
{
	HDC hdc;
	lpdsBack->GetDC(&hdc);

	SetTextColor(hdc, col);
	SetBkMode(hdc, TRANSPARENT);
	::TextOut(hdc, x, y, text, strlen(text));


	lpdsBack->ReleaseDC(hdc);
}

void CIsoView::TextOut(HDC hDC, int x, int y, const char* text, COLORREF col)
{
	SetTextColor(hDC, col);
	SetBkMode(hDC, TRANSPARENT);
	::TextOut(hDC, x, y, text, strlen(text));
}



void CIsoView::OnMove(int x, int y)
{
	CView::OnMove(x, y);

	if (lpds == NULL) return;
	LPDIRECTDRAWCLIPPER ddc;

	lpds->GetClipper(&ddc);
	ddc->SetHWnd(0, m_hWnd);
	updateFontScaled();

	RedrawWindow();	
}

void CIsoView::OnSize(UINT nType, int cx, int cy)
{
	// CView::OnSize(nType, cx, cy);
	if (lpds == NULL) return;

	LPDIRECTDRAWCLIPPER ddc;

	lpds->GetClipper(&ddc);
	ddc->SetHWnd(0, m_hWnd);
	updateFontScaled();

	CMyViewFrame& dlg = *(CMyViewFrame*)owner;
	dlg.m_minimap.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

	UpdateScrollRanges();

	GetWindowRect(&m_myRect);
}

COLORREF CIsoView::GetColor(const char* house, const char* vcolor)
{

	COLORREF neutral = RGB(120, 120, 120);
	COLORREF GDI = RGB(170, 170, 0);
	COLORREF Nod = RGB(255, 0, 0);
	COLORREF other = RGB(255, 255, 0);

	CString color;
	CIniFile& ini = Map->GetIniFile();
	if (house && strlen(house))
	{
		if (ini.sections.find(house) != ini.sections.end())
		{
			color = ini.sections[house].values["Color"];
		}
		else
			color = rules.sections[house].values["Color"];
	}

	if (vcolor)
		color = vcolor;

	if (color)
	{
		CString colorValues;
		if (ini.sections.contains("Colors"))
		{
			colorValues = ini.sections["Colors"].GetValueByName(color);
		}
		if (colorValues.IsEmpty() && rules.sections.contains("Colors"))
		{
			colorValues = rules.sections["Colors"].GetValueByName(color);
		}
		auto colorArray = SplitParams(colorValues);
		if (colorArray.size() == 3)
		{
			unsigned char hsv[3] = { static_cast<unsigned char>(std::atoi(colorArray[0])), static_cast<unsigned char>(std::atoi(colorArray[1])), static_cast<unsigned char>(std::atoi(colorArray[2])) };
			unsigned char rgb[3];
			HSVToRGB(hsv, rgb);
			return RGB(rgb[0], rgb[1], rgb[2]);
		}
	}

#ifndef RA2_MODE
	if (strstr(house, houses[0].name) != NULL)
	{
		return GDI;
	}
	else if (strstr(house, houses[1].name) != NULL)
	{
		return Nod;
	}
	else if (strstr(house, houses[2].name) != NULL)
	{
		return neutral;
	}
	else
	{
		if (isIncluded(color, "darkred") != NULL)
		{
			return Nod;
		}
		if (isIncluded(color, "grey") != NULL)
		{
			return RGB(120, 120, 120);
		}
		if (isIncluded(color, "gold") != NULL)
		{
			return GDI;
		}


		return other;
	}
#else
	if (isIncluded(color, "darkred") != NULL)
	{
		return RGB(130, 20, 20);
	}
	if (isIncluded(color, "grey") != NULL)
	{
		return RGB(120, 120, 120);
	}
	if (isIncluded(color, "red") != NULL)
	{
		return RGB(240, 20, 20);
	}
	if (isIncluded(color, "lightgold") != NULL)
	{
		return RGB(220, 220, 150);
	}
	if (isIncluded(color, "yellow") != NULL)
	{
		return RGB(240, 240, 0);
	}
	if (isIncluded(color, "gold") != NULL)
	{
		return RGB(230, 200, 0);
	}
	if (isIncluded(color, "darkgreen") != NULL)
	{
		return RGB(50, 160, 70);
	}
	if (isIncluded(color, "neongreen") != NULL)
	{
		return RGB(10, 255, 10);
	}
	if (isIncluded(color, "green") != NULL)
	{
		return RGB(50, 200, 70);
	}
	if (isIncluded(color, "darkblue") != NULL)
	{
		return RGB(30, 30, 150);
	}
	if (isIncluded(color, "lightblue") != NULL)
	{
		return RGB(100, 100, 200);
	}
	if (isIncluded(color, "neonblue") != NULL)
	{
		return RGB(35, 205, 255);
	}

	{
		return other;
	}

#endif


}




/*
GetOverlayPic(BYTE ovrl, BYTE ovrldata);

Returns the picture that should be used to draw the overlay tile.
First checks if a explicite bitmap exists for this tile, if not
checks if it can be displayed by a standard bitmap.
For bridges, it also needs the overlaydata.
*/
inline PICDATA* GetOverlayPic(BYTE ovrl, BYTE ovrldata)
{

	char c[50];
	char d[50];
	itoa(ovrl, c, 10);
	itoa(ovrldata, d, 10);

	CString fname = (CString)"OVRL" + c + "_" + d;

	// MessageBox(0,fname,"",0);

	if (pics.find(fname) != pics.end()) return &pics[fname];

	//errstream << "pic " << (LPCSTR)fname << " not found" << endl;


	return NULL;
}




void CIsoView::OnDraw(CDC* pDC)
{
	DrawMap();
}


void CIsoView::ReInitializeDDraw()
{
	b_IsLoading = TRUE;
	ReleaseCapture();
	KillTimer(11);
	// rscroll=FALSE;



#ifdef NOSURFACES
	while ((GetDeviceCaps(::GetDC(::GetDesktopWindow()), BITSPIXEL) <= 8))
	{
		if (MessageBox("You currently only have 8 bit color mode enabled. FinalAlert 2 does not work in 8 bit color mode. Please change the color mode and then click on OK. Click Cancel to quit (and save the map as backup.map).", "Error", MB_OKCANCEL) == IDCANCEL)
		{
			((CFinalSunDlg*)theApp.m_pMainWnd)->SaveMap((u8AppDataPath + "\\backup.map").c_str());
			PostQuitMessage(0);
			return;
		}
	}
#endif


	CDynamicGraphDlg dlg;

	dlg.ShowWindow(SW_SHOW);
	dlg.UpdateWindow();

	dd->RestoreAllSurfaces();

	updateFontScaled();


	missingimages.clear();

	theApp.m_loading->FreeAll();
	theApp.m_loading->InitDirectDraw();
	theApp.m_loading->InitPics();
	theApp.m_loading->InitTMPs(&dlg.m_Progress);

	memset(ovrlpics, 0, max_ovrl_img * 0xFF * sizeof(LPDIRECTDRAWSURFACE4));
	//UpdateOverlayPictures(-1);
	//Map->UpdateIniFile(MAPDATA_UPDATE_FROM_INI);
	Map->UpdateBuildingInfo();
	Map->UpdateTreeInfo();
#ifdef SMUDGE_SUPP
	Map->UpdateSmudgeInfo();
#endif



	b_IsLoading = FALSE;

	//Sleep(2500);



	dlg.DestroyWindow();





	RedrawWindow();



	/*

	CString CommandLine;

	if(res==IDYES)
	{
		CFileDialog dlg(FALSE, ".mpr", "noname.mpr", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "TS maps|*.mpr;*.map|TS multi maps|*.mpr|TS single maps|*.map|");
		do
		{


			char cuPath[MAX_PATH];

			GetCurrentDirectory(MAX_PATH, cuPath);
			dlg.m_ofn.lpstrInitialDir=cuPath;

			if(theApp.m_Options.TSExe.GetLength()) dlg.m_ofn.lpstrInitialDir=(char*)(LPCTSTR)theApp.m_Options.TSExe;

		}while(dlg.DoModal()==IDCANCEL) ;

		CommandLine=dlg.GetPathName();

		((CFinalSunDlg*)theApp.m_pMainWnd)->SaveMap(dlg.GetPathName());
	}

	if(res==IDNO || res==IDYES)
	{
		PROCESS_INFORMATION pi;
		STARTUPINFO si;
		memset(&si, 0, sizeof(STARTUPINFO));
		si.cb=sizeof(STARTUPINFO);

		char myExe[MAX_PATH];
		GetModuleFileName(NULL, myExe, MAX_PATH);

		BOOL success=CreateProcess(myExe,
			(LPTSTR)(LPCTSTR)CommandLine,
			NULL,
			NULL,
			FALSE,
			NORMAL_PRIORITY_CLASS,
			NULL,
			AppPath,
			&si,
			&pi);
	}

	theApp.m_pMainWnd->DestroyWindow();*/





	/*CLoading load;
	load.InitDirectDraw();
	load.InitMixFiles();
	load.InitPics();



	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

	b_IsLoading=FALSE;*/


}

void CIsoView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{


	CView::OnChar(nChar, nRepCnt, nFlags);
}

void CIsoView::OnDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{

	CView::OnDeadChar(nChar, nRepCnt, nFlags);
}

void CIsoView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	int nPos;



	if (nChar == 68) // D key
	{
		theApp.m_Options.bFlat = !theApp.m_Options.bFlat;
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	if (nChar == 65)
		m_bAltCliff = !m_bAltCliff;

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CIsoView::SetError(const char* text)
{
	CMyViewFrame& v = *((CMyViewFrame*)owner);
	v.m_statbar.GetStatusBarCtrl().SetText(text, 0, 0);
}




int CIsoView::GetOverlayDirection(int x, int y)
{
	DWORD dwIsoSize = Map->GetIsoSize();
	int p = -1;
	int type = Map->GetOverlayAt(x + y * dwIsoSize);

	BOOL isTrail = FALSE;
	int i;
	for (i = 0;i < overlay_count;i++)
		if (overlay_number[i] == type)
			if (overlay_trail[i]) isTrail = TRUE;

	if (isTrack(type)) // handling a train track
	{


		if (isTrack(Map->GetOverlayAt((x - 1) + (y - 1) * dwIsoSize)) && isTrack(Map->GetOverlayAt((x + 1) + (y + 1) * dwIsoSize)))
			return 0;
		if (isTrack(Map->GetOverlayAt((x + 1) + (y - 1) * dwIsoSize)) && isTrack(Map->GetOverlayAt((x - 1) + (y + 1) * dwIsoSize)))
			return 1;
		if (isTrack(Map->GetOverlayAt((x - 1) + (y - 0) * dwIsoSize)) && isTrack(Map->GetOverlayAt((x + 1) + (y + 0) * dwIsoSize)))
			return 2;
		if (isTrack(Map->GetOverlayAt((x - 0) + (y - 1) * dwIsoSize)) && isTrack(Map->GetOverlayAt((x + 0) + (y + 1) * dwIsoSize)))
			return 3;
		if (isTrack(Map->GetOverlayAt((x - 1) + (y - 0) * dwIsoSize)) && isTrack(Map->GetOverlayAt((x + 1) + (y + 1) * dwIsoSize)))
			return 4;
		if (isTrack(Map->GetOverlayAt((x - 0) + (y - 1) * dwIsoSize)) && isTrack(Map->GetOverlayAt((x + 1) + (y + 1) * dwIsoSize)))
			return 5;
		if (isTrack(Map->GetOverlayAt((x - 1) + (y - 1) * dwIsoSize)) && isTrack(Map->GetOverlayAt((x - 0) + (y + 1) * dwIsoSize)))
			return 6;
		if (isTrack(Map->GetOverlayAt((x - 1) + (y - 1) * dwIsoSize)) && isTrack(Map->GetOverlayAt((x + 1) + (y + 0) * dwIsoSize)))
			return 7;
		if (isTrack(Map->GetOverlayAt((x - 1) + (y - 0) * dwIsoSize)) && isTrack(Map->GetOverlayAt((x + 1) + (y - 1) * dwIsoSize)))
			return 8;
		if (isTrack(Map->GetOverlayAt((x + 0) + (y + 1) * dwIsoSize)) && isTrack(Map->GetOverlayAt((x + 1) + (y - 1) * dwIsoSize)))
			return 9;
		if (isTrack(Map->GetOverlayAt((x - 0) + (y - 1) * dwIsoSize)) && isTrack(Map->GetOverlayAt((x - 1) + (y + 1) * dwIsoSize)))
			return 10;
		if (isTrack(Map->GetOverlayAt((x - 1) + (y + 1) * dwIsoSize)) && isTrack(Map->GetOverlayAt((x + 1) + (y + 0) * dwIsoSize)))
			return 11;
		if (isTrack(Map->GetOverlayAt((x - 1) + (y - 1) * dwIsoSize)))
			return 0;
		if (isTrack(Map->GetOverlayAt((x - 1) + (y + 1) * dwIsoSize)))
			return 1;
		if (isTrack(Map->GetOverlayAt((x - 1) + (y - 0) * dwIsoSize)))
			return 2;
		if (isTrack(Map->GetOverlayAt((x - 0) + (y - 1) * dwIsoSize)))
			return 3;
		if (isTrack(Map->GetOverlayAt((x + 1) + (y + 1) * dwIsoSize)))
			return 0;
		if (isTrack(Map->GetOverlayAt((x + 1) + (y - 1) * dwIsoSize)))
			return 1;
		if (isTrack(Map->GetOverlayAt((x + 1) + (y + 0) * dwIsoSize)))
			return 2;
		if (isTrack(Map->GetOverlayAt((x + 0) + (y + 1) * dwIsoSize)))
			return 3;

		return 0;
	}
	else if (isTrail) // handling something like a sandbag
	{
		p = 0;
		if (Map->GetOverlayAt(x - 1 + (y - 0) * dwIsoSize) == type) p |= 0x1;
		if (Map->GetOverlayAt(x + (y + 1) * dwIsoSize) == type) p |= 0x2;
		if (Map->GetOverlayAt(x + 1 + (y + 0) * dwIsoSize) == type) p |= 0x4;
		if (Map->GetOverlayAt(x + (y - 1) * dwIsoSize) == type) p |= 0x8;
	}

	return p;

}

void CIsoView::HandleTrail(int x, int y)
{
	int i, e;
	int type = Map->GetOverlayAt(x + y * Map->GetIsoSize());

	if (isTrack(type)) // is a track (overlay must be changed)
	{

		for (i = x - 2;i <= x + 2;i++)
		{
			for (e = y - 1;e <= y + 1;e++)
			{
				if (isTrack(Map->GetOverlayAt(i + e * Map->GetIsoSize())))
					Map->SetOverlayAt(i + e * Map->GetIsoSize(), 0x27 + GetOverlayDirection(i, e));
			}
		}
	}
	else // something like a sandbag (overlaydata must be changed)
	{

		for (i = x - 2;i <= x + 2;i++)
		{
			for (e = y - 1;e <= y + 1;e++)
			{
				if (Map->GetOverlayAt(i + e * Map->GetIsoSize()) == type)
				{
					int dir = GetOverlayDirection(i, e);
					if (dir >= 0)	Map->SetOverlayDataAt(i + e * Map->GetIsoSize(), dir);
				}
			}
		}
	}

}


/*
DrawCell(int x, int y, int w, int h, COLORREF col)

Draws a frame around one or more cells.
w is the width, h is the height (in cells).
x and y are pixel coordinates.
col specifies the color.
*/
void CIsoView::DrawCell(int x, int y, int w, int h, COLORREF col, BOOL dotted, HDC hDC_)
{


	// correct the y value:
	//y -= f_y;

	POINT p1, p2, p3, p4;
	p1.x = x + f_x / 2;
	p1.y = y;
	p2.x = x + w * f_x / 2 + f_x / 2;
	p2.y = y + w * f_y / 2;
	p3.x = x + w * f_x / 2 - h * f_x / 2 + f_x / 2 - 1;
	p3.y = y + h * f_y / 2 + w * f_y / 2 - 1;
	p4.x = x - h * f_x / 2 + f_x / 2 - 1;
	p4.y = y + h * f_y / 2 - 1;

	HDC hDC = hDC_;
	if (!hDC)
		while (lpdsBack->GetDC(&hDC) == DDERR_WASSTILLDRAWING);

	HPEN p;
	int width = 1;
#ifdef RA2_MODE
	width = 2;
#endif
	if (!dotted)	p = CreatePen(PS_SOLID, width, col);
	else	p = CreatePen(PS_DOT, 0, col);

	SelectObject(hDC, p);

	if (dotted)
	{
		SetBkMode(hDC, TRANSPARENT);
		MoveToEx(hDC, p1.x, p1.y - 1, NULL);
		LineTo(hDC, p2.x + 1, p2.y);
		LineTo(hDC, p3.x, p3.y + 1);
		LineTo(hDC, p4.x - 1, p4.y);
		LineTo(hDC, p1.x, p1.y - 1);
	}

	if (!dotted)
	{
		MoveToEx(hDC, p1.x, p1.y, NULL);
		LineTo(hDC, p2.x, p2.y);
		LineTo(hDC, p3.x, p3.y);
		LineTo(hDC, p4.x, p4.y);
		LineTo(hDC, p1.x, p1.y);
	}





	if (!hDC_)
		lpdsBack->ReleaseDC(hDC);

	DeleteObject(p);
}

void CIsoView::DrawCell(void* dest, int dest_width, int dest_height, int dest_pitch, int drawx, int drawy, int w, int h, int col, bool dotted, bool touchNeighbours, int colNeighbour) const
{
	if (colNeighbour == CLR_INVALID)
		colNeighbour = col;

	// correct the y value:
	//y -= f_y;

	POINT p1, p2, p3, p4;
	p1.x = drawx + f_x / 2;
	p1.y = drawy;
	p2.x = drawx + w * f_x / 2 + f_x / 2;
	p2.y = drawy + w * f_y / 2;
	p3.x = drawx + w * f_x / 2 - h * f_x / 2 + f_x / 2;
	p3.y = drawy + h * f_y / 2 + w * f_y / 2;
	p4.x = drawx - h * f_x / 2 + f_x / 2;
	p4.y = drawy + h * f_y / 2;

	LineDrawer drawer(dest, bpp, dest_width, dest_height, dest_pitch);

	const LineStyle style = dotted ? LineStyle::Dotted_4 : LineStyle::Standard;

	for (int x = 0; x < 1; ++x)  // you could use this loop for more thickness
	{
		for (int y = 0; y < 1; ++y)
		{
			// inside the MM tile
			drawer.MoveTo(p1.x + x + 1, p1.y + y + 1);
			drawer.LineTo(p2.x + x - 2, p2.y + y - 1, col, style);
			drawer.LineTo(p3.x + x + 1, p3.y + y - 3, col, style);
			drawer.LineTo(p3.x + x, p3.y + y - 3, col, style);
			drawer.LineTo(p4.x + x + 3, p4.y + y - 1, col, style);
			drawer.LineTo(p4.x + x + 2, p4.y + y - 1, col, style);
			drawer.MoveTo(p4.x + x + 3, p4.y + y - 1);
			drawer.LineTo(p1.x + x, p1.y + y + 1, col, style);
			drawer.LineTo(p1.x + x + 1, p1.y + y + 1, col, style);

			// on inner pixels of MM tile boundary
			drawer.MoveTo(p1.x + x + 1, p1.y + y);
			drawer.LineTo(p2.x + x, p2.y + y - 1, col, style);
			drawer.LineTo(p3.x + x + 1, p3.y + y - 2, col, style);
			drawer.LineTo(p3.x + x, p3.y + y - 2, col, style);
			drawer.LineTo(p4.x + x + 1, p4.y + y - 1, col, style);
			drawer.LineTo(p1.x + x, p1.y + y, col, style);
			drawer.LineTo(p1.x + x + 1, p1.y + y, col, style);
		}
	}

	//  pixels of neighboured MM tiles boundary
	if (touchNeighbours)
	{
		for (int x = 0; x < 1; ++x)  // you could use this loop for more thickness
		{
			for (int y = 0; y < 1; ++y)
			{				
				drawer.MoveTo(p1.x + x + 1, p1.y + y - 1);
				drawer.LineTo(p2.x + x - 2, p2.y + y - 3, colNeighbour, style);
				drawer.MoveTo(p2.x + x - 2, p2.y + y - 2);
				drawer.LineTo(p2.x + x - 1, p2.y + y, colNeighbour, style);
				drawer.LineTo(p2.x + x, p2.y + y, colNeighbour, style);
				drawer.LineTo(p3.x + x + 1, p3.y + y - 1, colNeighbour, style);
				drawer.LineTo(p3.x + x, p3.y + y - 1, colNeighbour, style);
				drawer.LineTo(p4.x + x + 1, p4.y + y, colNeighbour, style);
				drawer.LineTo(p4.x + x, p4.y + y - 1, colNeighbour, style);
				drawer.LineTo(p1.x + x, p1.y + y - 1, colNeighbour, style);
				drawer.LineTo(p1.x + x + 1, p1.y + y - 1, colNeighbour, style);
			}
		}
	}
}

void CIsoView::DrawTube(const CTube& tube, const DDSURFACEDESC2* lockedDDSD, const COLORREF* color) const
{
	SurfaceLocker locker(lpdsBack);

	if (!lockedDDSD || !lockedDDSD->lpSurface)
	{
		lockedDDSD = locker.ensure_locked();
	}
	if (!lockedDDSD)
		return;

	// TODO: culling if required

	// find out if we are tube #1 or #2 for bidirectional tubes
	bool type0 = tube.getStartX() == tube.getEndX() ? (tube.getStartY() < tube.getEndY()) : (tube.getStartX() < tube.getEndX());
	const auto col = color ? m_color_converter->GetColor(*color) : (type0 ? m_color_converter->GetColor(255, 0, 0) : m_color_converter->GetColor(0, 0, 255));
	ProjectedVec lineOffset = type0 ? ProjectedVec() : ProjectedVec(1, 1);

	int lowestHeight = min(Map->GetHeightAt(tube.getStartCoords()), Map->GetHeightAt(tube.getEndCoords()));
	LineDrawer ld(lockedDDSD->lpSurface, bpp, lockedDDSD->dwWidth, lockedDDSD->dwHeight, lockedDDSD->lPitch);
	auto startDraw = GetRenderTargetCoordinates(tube.getStartCoords(), lowestHeight);
	ld.MoveTo(startDraw.x + f_x / 2 + lineOffset.x, startDraw.y + f_y / 2 + lineOffset.y);
	

	tube.walk([this, &tube, lockedDDSD, &ld, lowestHeight, col, lineOffset](const auto& wi) {
		auto draw = GetRenderTargetCoordinates(wi.pos, lowestHeight);
		if (wi.pos == tube.getEndCoords())
		{
			// end coordinates are drawn as cross
			LineDrawer startLD(lockedDDSD->lpSurface, bpp, lockedDDSD->dwWidth, lockedDDSD->dwHeight, lockedDDSD->lPitch);
			ProjectedVec off(7, 4);
			startLD.DrawLine(draw.x + off.x, draw.y + f_y / 2, draw.x + f_x - off.x, draw.y + f_y / 2, col);
			startLD.DrawLine(draw.x + f_x / 2 - 1, draw.y + off.y, draw.x + f_x / 2 - 1, draw.y + f_y - off.y, col);
		}
		if (wi.pos == tube.getStartCoords())
		{
			// Start coordinates are drawn as cell
			DrawCell(lockedDDSD->lpSurface, lockedDDSD->dwWidth, lockedDDSD->dwHeight, lockedDDSD->lPitch, draw.x, draw.y, 1, 1, col, true);
		}
		ld.LineTo(draw.x + f_x / 2 + lineOffset.x, draw.y + f_y / 2 + lineOffset.y, col);
		return true;
	});
}

MapCoords CIsoView::GetMapCoordinates(const ProjectedCoords& projCoords, bool bAllowAccessBehindCliffs, bool ignoreHideFlagsAndOutside) const
{
	return theApp.m_Options.bFlat ? Map->ToMapCoords(projCoords) : Map->ToMapCoords3d(projCoords, bAllowAccessBehindCliffs, ignoreHideFlagsAndOutside);
}

MapCoords CIsoView::GetMapCoordinatesFromRenderTargetCoordinates(const ProjectedCoords& projCoords, bool bAllowAccessBehindCliffs, bool ignoreHideFlagsAndOutside) const
{
	return GetMapCoordinates(projCoords + m_viewOffset, bAllowAccessBehindCliffs, ignoreHideFlagsAndOutside);
}

MapCoords CIsoView::GetMapCoordinatesFromClientCoordinates(const CPoint& clientPt, bool bAllowAccessBehindCliffs, bool ignoreHideFlagsAndOutside) const
{
	return GetMapCoordinates(GetProjectedCoordinatesFromClientCoordinates(clientPt), bAllowAccessBehindCliffs, ignoreHideFlagsAndOutside);
}

ProjectedCoords CIsoView::GetProjectedCoordinatesFromClientCoordinates(const CPoint& clientPt) const
{
	RECT r;
	GetWindowRect(&r);
	const auto viewOffsetWR = m_viewOffset + ProjectedVec(r.left, r.top);
	const auto projCoords = ProjectedCoords(clientPt.x * m_viewScale.x, clientPt.y * m_viewScale.y) + viewOffsetWR;
	return projCoords;
}

ProjectedCoords CIsoView::GetProjectedCoordinates(const MapCoords& mapCoords) const
{
	return theApp.m_Options.bFlat ? Map->ProjectCoords(mapCoords) : Map->ProjectCoords3d(mapCoords);
}

ProjectedCoords CIsoView::GetProjectedCoordinates(const MapCoords& mapCoords, int mapZ) const
{
	return Map->ProjectCoords3d(mapCoords, mapZ);
}

ProjectedCoords CIsoView::GetRenderTargetCoordinates(const MapCoords& mapCoords) const
{
	// Draw coordinates are display coordinates
	return GetProjectedCoordinates(mapCoords) - m_viewOffset;
}

ProjectedCoords CIsoView::GetRenderTargetCoordinates(const MapCoords& mapCoords, int mapZ) const
{
	return GetProjectedCoordinates(mapCoords, mapZ) - m_viewOffset;
}

CPoint CIsoView::GetClientCoordinates(const MapCoords& mapCoords) const
{
	RECT r;
	GetWindowRect(&r);
	const auto windowPos = ProjectedVec(r.left, r.top);
	//const auto p = ScaleBackToFrontBuffer(GetRenderTargetCoordinates(mapCoords)) - windowPos;
	const auto rtc = GetRenderTargetCoordinates(mapCoords);
	const auto p = ((ProjectedCoords(rtc.x, rtc.y) - windowPos) / m_viewScale).convertT<std::int32_t>();
	return CPoint(p.x, p.y);
}

CPoint CIsoView::GetClientCoordinatesFromWorld(const ProjectedCoords& projectedCoords) const
{
	RECT r;
	GetWindowRect(&r);
	const auto windowPos = ProjectedVec(r.left, r.top);
	//const auto p = ScaleBackToFrontBuffer(GetRenderTargetCoordinates(mapCoords)) - windowPos;
	const auto rtc = projectedCoords - m_viewOffset;
	const auto p = ((ProjectedCoords(rtc.x, rtc.y)) / m_viewScale).convertT<std::int32_t>();
	return CPoint(p.x, p.y);
}

ProjectedCoords CIsoView::ScaleBackToFrontBuffer(const ProjectedCoords& backBufferCoords) const
{
	RECT r;
	GetWindowRect(&r);
	const auto windowPos = ProjectedVec(r.left, r.top);
	return ((ProjectedCoords(backBufferCoords.x, backBufferCoords.y) - windowPos) / m_viewScale).convertT<std::int32_t>() + windowPos;
}


void CIsoView::UpdateStatusBar(int x, int y)
{
	CString statusbar;//=TranslateStringACP("Ready");

	FIELDDATA m = *Map->GetFielddataAt(x + y * Map->GetIsoSize());
	if (m.wGround == 0xFFFF) m.wGround = 0;

	if (m.wGround < (*tiledata_count) && m.bSubTile < (*tiledata)[m.wGround].wTileCount)
	{
		statusbar = "Terrain type: 0x";
		char c[50];
		itoa((*tiledata)[m.wGround].tiles[m.bSubTile].bTerrainType, c, 16);
		statusbar += c;
		statusbar += ", height: ";
		itoa(m.bHeight, c, 10);
		statusbar += c;
		statusbar += " / ";
	}


	if (Map->GetOverlayAt(x + y * Map->GetIsoSize()) != 0xFF)
	{
		char ov[50];
		itoa(Map->GetOverlayAt(x + y * Map->GetIsoSize()), ov, 16);

		int i;
		CString name;
		name = "0x";
		name += ov;
		for (i = 0;i < overlay_count;i++)
		{
			if (overlay_number[i] == Map->GetOverlayAt(x + y * Map->GetIsoSize()))
				name = overlay_name[i];
		}


		itoa(Map->GetOverlayDataAt(x + y * Map->GetIsoSize()), ov, 16);

		statusbar += GetLanguageStringACP("OvrlStatus");
		statusbar += TranslateStringACP(name);
		statusbar += ", OverlayData: 0x";
		statusbar += ov;
	}

	STDOBJECTDATA sod;
	sod.type = "";

	int n = Map->GetStructureAt(x + y * Map->GetIsoSize());
	int on = -1;
	if (n >= 0)
	{

		Map->GetStdStructureData(n, &sod);
		statusbar = GetLanguageStringACP("StructStatus");
		on = n;
	}

	n = Map->GetUnitAt(x + y * Map->GetIsoSize());
	if (n >= 0)
	{

		Map->GetStdUnitData(n, &sod);
		statusbar = GetLanguageStringACP("UnitStatus");
		on = n;

	}

	n = Map->GetAirAt(x + y * Map->GetIsoSize());
	if (n >= 0)
	{

		Map->GetStdAircraftData(n, &sod);
		statusbar = GetLanguageStringACP("AirStatus");
		on = n;
	}

	n = Map->GetInfantryAt(x + y * Map->GetIsoSize());
	if (n >= 0)
	{
		Map->GetStdInfantryData(n, &sod);
		statusbar = GetLanguageStringACP("InfStatus");
		on = n;
	}

	if (sod.type.GetLength() > 0)
	{
		char c[50];
		itoa(on, c, 10);
		statusbar += "ID ";
		statusbar += c;
		statusbar += ", ";

		statusbar += TranslateStringACP(Map->GetUnitName(sod.type));
		statusbar += " (";

		statusbar += TranslateHouse(sod.house, TRUE);
		statusbar += ", ";
		statusbar += sod.type;
		statusbar += ")";
	}

	/*
	// activate this code to display information about the mappack!

	statusbar="MAPPACK: ";
	int i;
	FIELDDATA td=Map->GetFielddataAt(x+y*Map->GetIsoSize());
	char c[50];
	itoa(td.wGround, c, 10);
	statusbar+="tile: ";
	statusbar+=c;
	statusbar+="  ";
	for(i=0;i<3;i++)
	{
		itoa(td.bMapData[i], c, 10);
		statusbar+=c;
		statusbar+=" ";
	}

	statusbar+=" height: ";
	itoa(td.bHeight, c, 10);
	statusbar+=c;
	statusbar+="  ";
	itoa(td.bMapData2[0],c,10);
	statusbar+=c;*/


	n = Map->GetCelltagAt(x + y * Map->GetIsoSize());
	if (n >= 0)
	{
		CString type;
		CString name;
		DWORD pos;
		Map->GetCelltagData(n, &type, &pos);
		CIniFile& ini = Map->GetIniFile();
		if (ini.sections["Tags"].values.find(type) != ini.sections["Tags"].values.end())
			name = GetParam(ini.sections["Tags"].values[type], 1);

		statusbar += GetLanguageStringACP("CellTagStatus");
		statusbar += name;
		statusbar += " (";
		statusbar += type;
		statusbar += ")";
	}

	if (AD.mode == ACTIONMODE_SETTILE)
	{
		statusbar = GetLanguageStringACP("TilePlaceStatus");
	}

	if (AD.mode == ACTIONMODE_COPY)
		statusbar = GetLanguageStringACP("CopyHelp");

	if (statusbar.GetLength() > 0)
		SetError(statusbar);

}

void CIsoView::UpdateOverlayPictures(int id)
{
	if (id < 0)
	{
		memset(ovrlpics, 0, max_ovrl_img * 0xFF * sizeof(LPDIRECTDRAWSURFACE4));

		int i, e;
		for (i = 0;i < 0xFF;i++)
		{
			for (e = 0;e < max_ovrl_img;e++)
			{
				/*PICDATA& p=GetOverlayPic(i, e);
				if(p.pic!=NULL)		ovrlpics[i][e]=&p;
				else
					ovrlpics[i][e]=NULL;*/
				ovrlpics[i][e] = GetOverlayPic(i, e);
			}
		}
	}
	else
	{
		int e;
		for (e = 0;e < max_ovrl_img;e++)
		{
			ovrlpics[id][e] = GetOverlayPic(id, e);
		}
	}
}


__forceinline void FixTileHeight(DWORD dwPos, int x, int y)
{
	return; // we use CMapData::CreateSlopes() to fix slopes now

	int oheight = Map->GetHeightAt(dwPos);
	int isosize = Map->GetIsoSize();

	char bHeightDiff[9];

	int i, e;
	for (i = -1;i < 2;i++)
	{
		for (e = -1;e < 2;e++)
		{
			if (x + i < 0 || y + e < 0 || x + i >= isosize || y + e >= isosize) bHeightDiff[i + 1 + (e + 1) * 3] = 0;
			else
				bHeightDiff[i + 1 + (e + 1) * 3] = oheight - Map->GetHeightAt(dwPos + i + e * isosize);
		}
	}

	if (bHeightDiff[1] < 0 && bHeightDiff[7] < 0)
	{
		Map->SetHeightAt(dwPos, oheight + 1);
		return;
	}
	if (bHeightDiff[3] < 0 && bHeightDiff[5] < 0)
	{
		Map->SetHeightAt(dwPos, oheight + 1);
		return;
	}

	if (
		(bHeightDiff[1] < 0 && bHeightDiff[6] < 0) ||
		(bHeightDiff[1] < 0 && bHeightDiff[8] < 0) /*||
		(bHeightDiff[3]<0 && bHeightDiff[2]<0) ||
		(bHeightDiff[3]<0 && bHeightDiff[8]<0) ||
		(bHeightDiff[5]<0 && bHeightDiff[0]<0) ||
		(bHeightDiff[5]<0 && bHeightDiff[6]<0) ||
		(bHeightDiff[7]<0 && bHeightDiff[0]<0) ||
		(bHeightDiff[7]<0 && bHeightDiff[2]<0) */
		)
	{
		Map->SetHeightAt(dwPos, oheight + 1);
		return;
	}
}


/*
ChangeTileHeight();

General routine for changing the height of a tile.
Warning: This routine calls itself if necessary, and therefore may be quite performance hungry!
*/
void __fastcall CIsoView::ChangeTileHeight(DWORD dwPos, DWORD dwNewHeight, BOOL bNonMorpheableMove, BOOL bOnlyThisTile, BOOL bNoSlopes)
{
	int isosize = Map->GetIsoSize();
	int mapsize = isosize * isosize;
	int mapwidth = Map->GetWidth();
	int mapheight = Map->GetHeight();

	int dwX = dwPos % isosize;
	int dwY = dwPos / isosize;

	//if(dwX==0 || dwY==0 || dwX+dwY<mapwidth || dwX+dwY>mapwidth+mapheight*2 || (dwY>mapwidth && dwX<dwY-mapwidth) || (dwX>mapwidth && dwY+mapwidth<dwX)) return;
	//if( dwX<2|| dwY<2 || dwX+dwY<mapwidth+2 || dwX+dwY+2>mapwidth+mapheight*2 || (dwY+2>mapwidth && dwX-2<dwY-mapwidth) || (dwX+2>mapwidth && dwY+mapwidth-2<dwX)) return;
	if (dwX < 1 || dwY < 1 || dwX + dwY<mapwidth + 1 || dwX + dwY>mapwidth + mapheight * 2 || (dwY + 1 > mapwidth && dwX - 1 < dwY - mapwidth) || (dwX + 1 > mapwidth && dwY + mapwidth - 1 < dwX)) return;



	FIELDDATA orig_fd = *Map->GetFielddataAt(dwPos);

	//if(bOnlyHeighten && dwNewHeight<orig_fd.bHeight) return;

	if (orig_fd.wGround == 0xFFFF) orig_fd.wGround = 0;
	int orig_ground = orig_fd.wGround;

	if (orig_fd.bReserved) return;

	if (dwX < m_funcRect.left) m_funcRect.left = dwX;
	if (dwY < m_funcRect.top) m_funcRect.top = dwY;
	if (dwX > m_funcRect.right) m_funcRect.right = dwX;
	if (dwY > m_funcRect.bottom) m_funcRect.bottom = dwY;

	BOOL bCliffNext = FALSE;
	/*if(bOnlyHeighten)
	{
		bCliffNext=FALSE;
		/*int i;
		int e;
		for(i=-1;i<2;i++)
		{
			for(e=-1;e<2;e++)
			{
				int pos=dwPos+i+(e)*isosize;

				FIELDDATA fd=*Map->GetFielddataAt(pos);
				if(fd.wGround==0xFFFF) fd.wGround=0;

				if((*tiledata)[fd.wGround].wTileSet==cliffset || fd.bCliffHack)
				{
					bCliffNext=TRUE;
					break;
				}
			}
			if(bCliffNext) break;
		}*//*

	}*/

	Map->SetReserved(dwPos, 1);


	if (!(*tiledata)[orig_ground].bMorphable) bNonMorpheableMove = TRUE;

	Map->SetHeightAt(dwPos, dwNewHeight);


	int i, e;
	for (i = -1;i < 2;i++)
	{
		for (e = -1;e < 2;e++)
		{

			int pos = dwPos + i + (e)*isosize;

			FIELDDATA fd = *Map->GetFielddataAt(pos);
			if (fd.wGround == 0xFFFF) fd.wGround = 0;

			BOOL bAllowed = TRUE;

			if (dwX == 0 && i == -1) bAllowed = FALSE;
			else if (dwY == 0 && e == -1) bAllowed = FALSE;
			else if (dwX == isosize - 1 && i == 1) bAllowed = FALSE;
			else if (dwY == isosize - 1 && e == 1) bAllowed = FALSE;


			if (bOnlyThisTile)
			{
				if (fd.wGround != orig_fd.wGround) bAllowed = FALSE; // was another tile

				else
				{
					// we now need to check if it is really the same tile or just the same tile type (!)
					int of = orig_fd.bSubTile;
					int f = fd.bSubTile;
					int width = (*tiledata)[orig_ground].cy;
					int height = (*tiledata)[orig_ground].cx;
					int ox = of / width;
					int oy = of % width;
					int x = f / width;
					int y = f % width;

					if (x - ox != i || y - oy != e) bAllowed = FALSE;
				}
			}


			int ground = fd.wGround;


			if (bAllowed && !fd.bReserved && pos > 0 && pos < mapsize)
			{




				BOOL bDestMorphable = (*tiledata)[fd.wGround].bMorphable;
				TILEDATA destTile = (*tiledata)[fd.wGround];
				TILEDATA sourceTile = (*tiledata)[orig_fd.wGround];

				if (bAllowed && (!bNonMorpheableMove || bDestMorphable))
				{
					if (e == 1 && i == 1)
					{
						int leftGround = Map->GetFielddataAt(dwPos + 1)->wGround;
						int rightGround = Map->GetFielddataAt(dwPos + isosize)->wGround;
						if (leftGround == 0xFFFF) leftGround = 0;
						if (rightGround == 0xFFFF) rightGround = 0;
						if (!(*tiledata)[leftGround].bMorphable && !(*tiledata)[rightGround].bMorphable)
							bAllowed = FALSE;
					}
					if (e == -1 && i == -1)
					{
						int leftGround = Map->GetFielddataAt(dwPos - 1)->wGround;
						int rightGround = Map->GetFielddataAt(dwPos - isosize)->wGround;
						if (leftGround == 0xFFFF) leftGround = 0;
						if (rightGround == 0xFFFF) rightGround = 0;
						if (!(*tiledata)[leftGround].bMorphable && !(*tiledata)[rightGround].bMorphable)
							bAllowed = FALSE;
					}
					if (e == -1 && i == 1)
					{
						int leftGround = Map->GetFielddataAt(dwPos - 1)->wGround;
						int rightGround = Map->GetFielddataAt(dwPos + isosize)->wGround;
						if (leftGround == 0xFFFF) leftGround = 0;
						if (rightGround == 0xFFFF) rightGround = 0;
						if (!(*tiledata)[leftGround].bMorphable && !(*tiledata)[rightGround].bMorphable)
							bAllowed = FALSE;
					}
					if (e == 1 && i == -1)
					{
						int leftGround = Map->GetFielddataAt(dwPos + 1)->wGround;
						int rightGround = Map->GetFielddataAt(dwPos - isosize)->wGround;
						if (leftGround == 0xFFFF) leftGround = 0;
						if (rightGround == 0xFFFF) rightGround = 0;
						if (!(*tiledata)[leftGround].bMorphable && !(*tiledata)[rightGround].bMorphable)
							bAllowed = FALSE;
					}
				}






				if (/*(*tiledata)[ground].bMorphable &&*/ bAllowed)
				{
					int need_diff = (*tiledata)[ground].tiles[fd.bSubTile].bZHeight - (*tiledata)[orig_ground].tiles[orig_fd.bSubTile].bZHeight;


					//if(Map->GetHeightAt(pos)-dwNewHeight!=need_diff || !bNonMorpheableMove)
					{
						int diff = fd.bHeight - dwNewHeight;

						/*
						Activate this code to use the huge slopes
						Removed because it did look worse than the normal slopes
						*/
						/*
						if(i && abs(i)==abs(e))
						{
							if(diff<-1) diff++;
							if(diff>1) diff--;
						}*/


						if (!bNonMorpheableMove && bDestMorphable)
						{
							//if(!bOnlyHeighten)//!bCliffNext)
							{
								if (diff < -1)
									ChangeTileHeight(pos, dwNewHeight - 1, bNonMorpheableMove, bOnlyThisTile, bNoSlopes);
								if (diff > 1)
									ChangeTileHeight(pos, dwNewHeight + 1, bNonMorpheableMove, bOnlyThisTile, bNoSlopes);
							}
							/*else
							{
								ChangeTileHeight(pos, dwNewHeight, bNonMorpheableMove, bOnlyThisTile, bOnlyHeighten);
							}*/
						}
						else if (bNonMorpheableMove)
						{
							if (!bDestMorphable /*&& (*tiledata)[fd.wGround].wTileSet==(*tiledata)[orig_fd.wGround].wTileSet*/ /*&& need_diff!=fd.bHeight-orig_fd.bHeight*/)
							{
								if (destTile.tiles[fd.bSubTile].bTerrainType == sourceTile.tiles[orig_fd.bSubTile].bTerrainType || destTile.wTileSet == sourceTile.wTileSet)
								{
									ChangeTileHeight(pos, dwNewHeight + need_diff, TRUE, bOnlyThisTile, bNoSlopes);
								}
							}
							else if (bDestMorphable /*&& need_diff!=fd.bHeight-orig_fd.bHeight*/)
							{
								//diff-=need_diff;

								if ((sourceTile.cx > 1 && sourceTile.cy > 1 && (e < 0 && i < 0)) || ((sourceTile.cx < 2 || sourceTile.cy < 2) && (e > 0 && i > 0)))
								{
									ChangeTileHeight(pos, dwNewHeight, FALSE, bOnlyThisTile, bNoSlopes);
								}
								else
								{
									if (diff - need_diff > 0)
										ChangeTileHeight(pos, dwNewHeight + need_diff, FALSE, bOnlyThisTile, bNoSlopes);
									else if (diff - need_diff < 0)
										ChangeTileHeight(pos, dwNewHeight + need_diff, FALSE, bOnlyThisTile, bNoSlopes);
								}

							}
						}

					}
				}
			}
		}
	}

	/*for(i=-1;i<2;i++)
	{
		for(e=-1;e<2;e++)
		{
			FixTileHeight(dwPos+i+e*isosize, dwX+i, dwY+e);
		}
	}*/

	if (!bNoSlopes)
	{
		for (i = -1;i < 2;i++)
		{
			for (e = -1;e < 2;e++)
			{
				int pos = dwPos + i + (e)*isosize;
				if (pos > 0 && pos < mapsize)
					Map->CreateSlopesAt(pos);
			}
		}
	}

}

BOOL CIsoView::ReachableFrom(DWORD dwStart, DWORD dwEnd)
{
	FIELDDATA orig_fd = *Map->GetFielddataAt(dwStart);
	if (orig_fd.wGround == 0xFF) orig_fd.wGround = 0;
	int orig_ground = orig_fd.wGround;

	if (orig_fd.bReserved) return FALSE;
	orig_fd.bReserved = 1;

	if (dwStart == dwEnd) return TRUE;


	int i, e;
	for (i = -1;i < 2;i++)
	{
		for (e = -1;e < 2;e++)
		{
			int new_ground = orig_fd.wGround;
			if (new_ground == 0xFFFF) new_ground = 0;

			if (!(*tiledata)[new_ground].bMorphable && ReachableFrom(dwStart + i + e * Map->GetIsoSize(), dwEnd))
				return TRUE;
		}
	}

	return FALSE;
}

void CIsoView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CIsoView::HideTileSet(DWORD dwTileSet)
{
	int i;
	for (i = 0;i < *tiledata_count;i++)
	{
		if ((*tiledata)[i].wTileSet == dwTileSet) (*tiledata)[i].bHide = TRUE;
	}
}

void CIsoView::BlitBackbufferToHighRes()
{
	// MW:
	// the primary buffer and the (optional) high-res buffer cover the whole screen (Windows automatic high DPI scaling does not change this)
	// the backbuffer also has the size of the whole screen, however we don't render to the whole screen
	// we only render to the visible window rectangle (which IS affected by Windows' automatic DPI scaling features)
	// Additionally, we have a user-controlled scale factor.
	// If the user zooms in, we only render to a smaller part of the backbuffer and stretch it to the window rectangle (which may then again be stretched by Windows DPI if enabled in the Windows settings).
	// the rectangle we render to has the same left/top border but a lower value for right/bottom.
	//
	// This is a simple scaling and also scales any text that has already been rendered to the backbuffer (which is why we're usually rendering text afterwards to the HighRes backbuffer).
	// Altogether it'd probably be best to move from DirectDraw to Direct3D, support high DPI awareness and do the scaling right when initially drawing the map.

	
	if (m_viewScale != Vec2<CSProjected, float>(1.0f, 1.0f) && lpdsBackHighRes)
	{
		// copy scene backbuffer to the high-res buffer that will receive text rendering
		RECT dr;
		GetWindowRect(&dr);
		auto cr = GetScaledDisplayRect();
		lpdsBackHighRes->Blt(&dr, lpdsBack, &cr, DDBLT_WAIT, 0);
	}
	else
	{
		// do nothing, backbuffer will receive additional text rendering as text doesn't have to be scaled
	}
}

void CIsoView::FlipHighResBuffer()
{
	if (m_viewScale != Vec2<CSProjected, float>(1.0f, 1.0f) && lpdsBackHighRes)
	{
		// This flip copies the high-res backbuffer to the front buffer
		RECT dr;
		GetWindowRect(&dr);
		lpds->Blt(&dr, lpdsBackHighRes, &dr, DDBLT_WAIT, 0);
	}
	else
	{
		// copy scene backbuffer to the front buffer that will receive text/gdi rendering
		RECT dr;
		GetWindowRect(&dr);
		auto cr = GetScaledDisplayRect();
		lpds->Blt(&dr, lpdsBack, &cr, DDBLT_WAIT, 0);
	}
}

void CIsoView::ShowAllTileSets()
{
	int i;
	for (i = 0;i < *tiledata_count;i++)
	{
		(*tiledata)[i].bHide = FALSE;
	}
}



void CIsoView::PlaceCurrentObjectAt(int x, int y)
{
	if (AD.mode == ACTIONMODE_RANDOMTERRAIN)
	{
		if (Map->GetTerrainAt(x + y * Map->GetIsoSize()) >= 0)
		{
			return;
		}

		CString s;

		int n = rand() * rndterrainsrc.size() / RAND_MAX;

		// safety checks...
		if (n >= rndterrainsrc.size()) n = rndterrainsrc.size() - 1;
		if (n < 0) n = 0;

		s = rndterrainsrc[n];

		Map->AddTerrain(s, x + y * Map->GetIsoSize());

		return;
	}

	if (AD.type == 1) // ADD INFANTRY
	{
		if (Map->GetInfantryCountAt(x + y * Map->GetIsoSize()) >= SUBPOS_COUNT)
		{
			return;
		}

		Map->AddInfantry(NULL, AD.data_s, currentOwner, x + y * Map->GetIsoSize());
		//RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (AD.type == 2) // add structure
	{
		int n = Map->GetStructureAt(x + y * Map->GetIsoSize());
		if (n >= 0)
		{
			STDOBJECTDATA sod;
			Map->GetStdStructureData(n, &sod);
			if (strcmp(sod.type, "GAPAVE") != NULL)
			{
				//isMoving=FALSE;
				return;
			}
		}


		Map->AddStructure(NULL, AD.data_s, currentOwner, x + y * Map->GetIsoSize());
		//RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (AD.type == 3) // add aircraft
	{

		if (Map->GetAirAt(x + y * Map->GetIsoSize()) >= 0)
		{
			return;
		}

		Map->AddAircraft(NULL, AD.data_s, currentOwner, x + y * Map->GetIsoSize());

		//RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (AD.type == 4) // add vehicle
	{
		if (Map->GetUnitAt(x + y * Map->GetIsoSize()) >= 0)
		{
			return;
		}

		Map->AddUnit(NULL, AD.data_s, currentOwner, x + y * Map->GetIsoSize());

		//RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (AD.type == 5) // add terrain
	{
		if (Map->GetTerrainAt(x + y * Map->GetIsoSize()) >= 0)
		{
			return;
		}

		Map->AddTerrain(AD.data_s, x + y * Map->GetIsoSize());

		//RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (AD.type == 6)
	{
		// Overlay



		DWORD dwPos = x + y * Map->GetIsoSize();

		FIELDDATA m = *Map->GetFielddataAt(dwPos);
		if (m.wGround == 0xFFFF) m.wGround = 0;

		BYTE oldOverlay = Map->GetOverlayAt(dwPos);
		BYTE oldOverlayData = Map->GetOverlayDataAt(dwPos);

		if (AD.data == 1) // delete overlay
		{
			int gx, gy;
			for (gx = x - AD.data2;gx <= x + AD.data2;gx++)
			{
				for (gy = y - AD.data2;gy <= y + AD.data2;gy++)
				{
					Map->SetOverlayDataAt(gx + gy * Map->GetIsoSize(), 0xFF);
					Map->SetOverlayAt(gx + gy * Map->GetIsoSize(), 0xFF);


					HandleTrail(gx - 1, gy - 1);
					HandleTrail(gx + 1, gy - 1);
					HandleTrail(gx - 1, gy + 1);
					HandleTrail(gx + 1, gy + 1);
					HandleTrail(gx - 0, gy - 1);
					HandleTrail(gx + 0, gy + 1);
					HandleTrail(gx - 1, gy + 0);
					HandleTrail(gx + 1, gy + 0);
				}
			}


		}
		else if (AD.data == 2) // green tiberium mode
		{

			{
				if (AD.data2 == 0)
				{
					int i, e;
					for (i = 0;i < m_BrushSize_x;i++)
					{
						for (e = 0;e < m_BrushSize_y;e++)
						{
							int curground = Map->GetFielddataAt(dwPos + i + e * Map->GetIsoSize())->wGround;
							if (curground == 0xFFFF) curground = 0;

							if ((*tiledata)[curground].bAllowTiberium)
							{
								//srand( GetTickCount()+i+e*5 );
								int v1 = ((float)rand() / (float)RAND_MAX) * 8.0f + 0x68;
								int v2 = ((float)rand() / (float)RAND_MAX) * 8.0f + 1;
								Map->SetOverlayAt(dwPos + i + e * Map->GetIsoSize(), v1);
								Map->SetOverlayDataAt(dwPos + i + e * Map->GetIsoSize(), v2);
							}
						}
					}
				}
				else if (AD.data2 == 10)
				{
					int i, e;
					for (i = 0;i < m_BrushSize_x;i++)
					{
						for (e = 0;e < m_BrushSize_y;e++)
						{
							int curground = Map->GetFielddataAt(dwPos + i + e * Map->GetIsoSize())->wGround;
							if (curground == 0xFFFF) curground = 0;

							if ((*tiledata)[curground].bAllowTiberium)
							{
								Map->SetOverlayAt(dwPos + i + e * Map->GetIsoSize(), 0x70);
								Map->SetOverlayDataAt(dwPos + i + e * Map->GetIsoSize(), 0x5);
							}
						}
					}
				}
				else if (AD.data2 == 20)
				{
					int i, e;
					for (i = 0;i < m_BrushSize_x;i++)
					{
						for (e = 0;e < m_BrushSize_y;e++)
						{
							int ovrl = Map->GetOverlayAt(dwPos + i + e * Map->GetIsoSize());
							if (ovrl > 0x65 && ovrl < 0x72)
							{
								if (Map->GetOverlayDataAt(i + e * Map->GetIsoSize() + dwPos) < 9) Map->SetOverlayDataAt(dwPos + i + e * Map->GetIsoSize(), Map->GetOverlayDataAt(dwPos + i + e * Map->GetIsoSize()) + 1);
							}
						}
					}
				}
				else if (AD.data2 == 21)
				{

					int i, e;
					for (i = 0;i < m_BrushSize_x;i++)
					{
						for (e = 0;e < m_BrushSize_y;e++)
						{
							int ovrl = Map->GetOverlayAt(dwPos + i + e * Map->GetIsoSize());
							if (ovrl > 0x65 && ovrl < 0x72)
							{
								if (Map->GetOverlayDataAt(dwPos + i + e * Map->GetIsoSize()) > 0) Map->SetOverlayDataAt(dwPos + i + e * Map->GetIsoSize(), Map->GetOverlayDataAt(dwPos + i + e * Map->GetIsoSize()) - 1);
							}
						}
					}
				}
			}

		}
		else if (AD.data == 3) // blue tiberium mode
		{
			int o = 0x7f;
#ifdef RA2_MODE 
			o = 0x1e;
#endif
			//if((*tiledata)[m.wGround].bAllowTiberium)
			{
				if (AD.data2 == 0)
				{

					int i, e;
					for (i = 0;i < m_BrushSize_x;i++)
					{
						for (e = 0;e < m_BrushSize_y;e++)
						{
							int curground = Map->GetFielddataAt(dwPos + i + e * Map->GetIsoSize())->wGround;
							if (curground == 0xFFFF) curground = 0;

							if ((*tiledata)[curground].bAllowTiberium)
							{

								int v2 = ((float)rand() / (float)RAND_MAX) * 8 + 1;
								Map->SetOverlayAt(dwPos + i + e * Map->GetIsoSize(), o);
								Map->SetOverlayDataAt(dwPos + i + e * Map->GetIsoSize(), v2);
							}
						}
					}

				}
				else if (AD.data2 == 10)
				{
					int i, e;
					for (i = 0;i < m_BrushSize_x;i++)
					{
						for (e = 0;e < m_BrushSize_y;e++)
						{
							int curground = Map->GetFielddataAt(dwPos + i + e * Map->GetIsoSize())->wGround;
							if (curground == 0xFFFF) curground = 0;

							if ((*tiledata)[curground].bAllowTiberium)
							{
								Map->SetOverlayAt(dwPos + i + e * Map->GetIsoSize(), o);
								Map->SetOverlayDataAt(dwPos + i + e * Map->GetIsoSize(), 0x5);
							}
						}
					}
				}
				else if (AD.data2 == 20)
				{
					int i, e;
					for (i = 0;i < m_BrushSize_x;i++)
					{
						for (e = 0;e < m_BrushSize_y;e++)
						{
							int ovrl = Map->GetOverlayAt(dwPos + i + e * Map->GetIsoSize());
							if (ovrl == o)
							{
								if (Map->GetOverlayDataAt(dwPos + i + e * Map->GetIsoSize()) < 9) Map->SetOverlayDataAt(dwPos + i + e * Map->GetIsoSize(), Map->GetOverlayDataAt(dwPos + i + e * Map->GetIsoSize()) + 1);
							}
						}
					}
				}
				else if (AD.data2 == 21)
				{
					int i, e;
					for (i = 0;i < m_BrushSize_x;i++)
					{
						for (e = 0;e < m_BrushSize_y;e++)
						{
							int ovrl = Map->GetOverlayAt(dwPos + i + e * Map->GetIsoSize());
							if (ovrl == o)
							{
								if (Map->GetOverlayDataAt(dwPos + i + e * Map->GetIsoSize()) > 0) Map->SetOverlayDataAt(dwPos + i + e * Map->GetIsoSize(), Map->GetOverlayDataAt(dwPos + i + e * Map->GetIsoSize()) - 1);
							}
						}
					}
				}
			}

		}
		else if (AD.data == 4) // veinhole stuff mode
		{
			if (AD.data2 == 0) // Set veinhole!
			{
				int gx, gy;
				for (gx = x - 1;gx <= x + 1;gx++)
				{
					for (gy = y - 1;gy <= y + 1;gy++)
					{
						Map->SetOverlayAt(gx + gy * Map->GetIsoSize(), OVRL_VEINHOLEBORDER);
						Map->SetOverlayDataAt(gx + gy * Map->GetIsoSize(), 0x0);
					}
				}
				Map->SetOverlayAt(dwPos, OVRL_VEINHOLE);
				Map->SetOverlayDataAt(dwPos, 0x0);
				Map->SetHeightAt(dwPos, Map->GetHeightAt(dwPos) - 1);
			}
			else if (AD.data2 == 1) // set veins
			{
				Map->SetOverlayAt(dwPos, OVRL_VEINS);
				Map->SetOverlayDataAt(dwPos, 0x30);
			}

		}
		else if (AD.data == 30) // any overlay
		{
			Map->SetOverlayAt(dwPos, AD.data2);
			Map->SetOverlayDataAt(dwPos, 0);
			int i;
			for (i = 0;i < overlay_count;i++)
			{
				if (overlay_number[i] == AD.data2)
				{
					if (overlay_trail[i])
					{
						// handle trail stuff!
						HandleTrail(x, y);
					}
				}
			}
		}
		else if (AD.data == 33)
		{
			int i, e;
			for (i = 0;i < m_BrushSize_x;i++)
			{
				for (e = 0;e < m_BrushSize_y;e++)
				{
					Map->SetOverlayAt(dwPos + i + e * Map->GetIsoSize(), AD.data2);
					Map->SetOverlayDataAt(dwPos + i + e * Map->GetIsoSize(), AD.data3);
				}
			}
		}

		else if (AD.data == 31)
		{
			Map->SetOverlayAt(dwPos, AD.data2);
		}
		else if (AD.data == 32)
		{
			Map->SetOverlayDataAt(dwPos, AD.data2);
		}

		// RedrawWindow(NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else if (AD.type == 7)
	{
		// set owner!
		BOOL bchanged = FALSE;

		int t = Map->GetStructureAt(x + y * Map->GetIsoSize());
		if (t >= 0)
		{
			STRUCTURE structure;
			Map->GetStructureData(t, &structure);
			Map->DeleteStructure(t);
			structure.house = AD.data_s;
			Map->AddStructure(&structure);
			bchanged = TRUE;
		}
		t = Map->GetUnitAt(x + y * Map->GetIsoSize());
		if (t >= 0)
		{
			UNIT unit;
			Map->GetUnitData(t, &unit);
			Map->DeleteUnit(t);
			unit.house = AD.data_s;
			Map->AddUnit(&unit);
			bchanged = TRUE;
		}
		t = Map->GetAirAt(x + y * Map->GetIsoSize());
		if (t >= 0)
		{
			AIRCRAFT aircraft;
			Map->GetAircraftData(t, &aircraft);
			Map->DeleteAircraft(t);
			aircraft.house = AD.data_s;
			Map->AddAircraft(&aircraft);
			bchanged = TRUE;
		}
		int z;
		for (z = 0;z < SUBPOS_COUNT;z++)
		{
			t = Map->GetInfantryAt(x + y * Map->GetIsoSize(), z);
			if (t >= 0)
			{
				INFANTRY infantry;
				Map->GetInfantryData(t, &infantry);
				Map->DeleteInfantry(t);
				infantry.house = AD.data_s;
				Map->AddInfantry(&infantry);
				bchanged = TRUE;
			}
		}

		if (bchanged)
		{
			//RedrawWindow(NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW);
		}


	}
#ifdef SMUDGE_SUPP		
	else if (AD.type == 8) // add smudge
	{

		if (Map->GetFielddataAt(x + y * Map->GetIsoSize())->smudge >= 0)
		{
			return;
		}

		SMUDGE s;
		s.type = AD.data_s;
		s.x = x;
		s.y = y;
		s.deleted = 0;
		Map->AddSmudge(&s);

		//RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
#endif

}

void CIsoView::OnTimer(UINT nIDEvent)
{
	// theApp.m_loading->FreeAll();
	// theApp.m_loading->InitPics();

	last_succeeded_operation = 1000000;


	if (nIDEvent == 11)
	{
		// BUGSEARCH
		//if(b_IsLoading) return;	
		// 
		m_viewOffset += ProjectedVec((cur_x_mouse - rclick_x) / 2, (cur_y_mouse - rclick_y) / 2);

		SetScroll(m_viewOffset.x, m_viewOffset.y);

		InvalidateRect(NULL, FALSE);
	}
	else
	{
		errstream << "Timer calls InitTMPs()" << endl;
		errstream.flush();

		theApp.m_loading->InitTMPs();

		CView::OnTimer(nIDEvent);

		KillTimer(nIDEvent);

		b_IsLoading = FALSE;
		RedrawWindow();
	}
}

void CIsoView::FillArea(DWORD dwX, DWORD dwY, DWORD dwID, BYTE bSubTile)
{
	if ((*tiledata)[dwID].cx != 1 || (*tiledata)[dwID].cy != 1)
	{
		MessageBox("You can only use 1x1 tiles to fill areas.");
		return;
	}

	FIELDDATA* f = Map->GetFielddataAt(dwX + dwY * Map->GetIsoSize());
	f->bReserved = TRUE;

	int mapwidth, mapheight;
	mapwidth = Map->GetWidth();
	mapheight = Map->GetHeight();

	int i, e;
	for (i = -1;i < 2;i++)
	{
		for (e = -1;e < 2;e++)
		{
			if (abs(i) == abs(e)) continue;
			int cur_x, cur_y;
			cur_x = dwX + i;
			cur_y = dwY + e;
			if (cur_x < 1 || cur_y < 1 || cur_x + cur_y<mapwidth + 1 || cur_x + cur_y>mapwidth + mapheight * 2 || (cur_y + 1 > mapwidth && cur_x - 1 < cur_y - mapwidth) || (cur_x + 1 > mapwidth && cur_y + mapwidth - 1 < cur_x)) continue;

			FIELDDATA* f2 = Map->GetFielddataAt(cur_x + cur_y * Map->GetIsoSize());

			if (f2->bReserved) continue;



			if (f2->wGround != dwID && f2->wGround == f->wGround && f2->bSubTile == f->bSubTile)
			{
				FillArea(cur_x, cur_y, dwID, bSubTile);
			}

		}
	}
	f->wGround = dwID;
	f->bSubTile = bSubTile;
}

void CIsoView::AutoLevel()
{
	Map->TakeSnapshot();



	int mapsize = Map->GetIsoSize() * Map->GetIsoSize();
	int mapedge = Map->GetIsoSize();

	if (mapsize == 0) return;

	BOOL* bChanged;
	bChanged = new(BOOL[mapsize]);
	memset(bChanged, 0, mapsize);

	int iCliffStart = -1; //, iSlopeSetStart = -1;

	int i;

	for (i = 0;i < *tiledata_count;i++)
	{
		if ((*tiledata)[i].wTileSet == cliffset)
		{
			iCliffStart = i;
			break;

		}
	}

	if (iCliffStart < 0)
		return;

	/*for (i = 0;i<*tiledata_count;i++)
	{
		if((*tiledata)[i].wTileSet==slopesetpiecesset)
		{
			iSlopeSetStart=i;
			break;
		}
	}*/

	/*int count=atoi(g_data.sections["SlopeSetPiecesDirections"].values["Count"]);
	BYTE* bXLeftSearch=new(BYTE[count]);
	BYTE* bXRightSearch=new(BYTE[count]);
	BYTE* bYTopSearch=new(BYTE[count]);
	BYTE* bYBottomSearch=new(BYTE[count]);

	for(i=0;i<count;i++)
	{
		char c[50];
		itoa(i,c,10);
		CString s=*g_data.sections["SlopeSetPiecesDirections"].GetValue(i);
		if(s=="Right_1") { bXLeftSearch=0; bXRightSearch=1; b
		else if(s=="Left_1") bDirections[i]=1;
		else if(s=="Bottom_1") bDirections[i]=2;
		else if(s=="Top_1") bDirections[i]=3;
		else if(s=="Right_2") bDirections[i]=4;
		else if(s=="Left_2") bDirections[i]=5;
		else if(s=="Bottom_2") bDirections[i]=6;
		else if(s=="Top_2") bDirections[i]=7;
	}*/

	for (i = 0;i < mapsize;i++)
	{
		FIELDDATA* f1 = Map->GetFielddataAt(i);
		int o_ground = f1->wGround;
		if (o_ground == 0xFFFF) o_ground = 0;
		int o_width = (*tiledata)[o_ground].cx;
		int o_height = (*tiledata)[o_ground].cy;

		int ox, oy;
		ox = i % mapedge;
		oy = i / mapedge;


		int x, y;
		for (x = -1;x < 2;x++)
		{
			for (y = -1;y < 2;y++)
			{
				if (x == -1 && y == -1) continue;
				if (x == 1 && y == 1) continue;
				if (x == 1 && y == -1) continue;
				if (x == -1 && y == 1) continue;

				int count = 0;
				int p, k;
				int heights[32][32];
				memset(heights, 0, 32 * 32 * sizeof(int));

				for (p = 0;p < o_width;p++)
				{
					for (k = 0;k < o_height;k++)
					{
						count++;
						heights[p][k] = (*tiledata)[o_ground].tiles[p + k * o_width].bZHeight;
					}
				}



				if (o_ground < 8 + iCliffStart || o_ground>13 + iCliffStart)
				{
					if (count < 4 || count>4)
					{
						if (x < 0 || y < 0) continue;
					}




					if (heights[0][0] && !heights[1][1])
					{
						if (heights[1][0] && heights[0][1])
						{

							if (x < 0 || y < 0) continue;

						}
					}

					if (heights[0][0] && heights[1][1])
					{
						if (x < 0 || y>0)  continue;
					}

					if (!heights[0][0] && heights[1][0] && heights[0][1] && heights[1][1])
					{
						if (x < 0 || y < 0) continue;
					}
				}
				else
				{
					if (x > 0 || y > 0) continue;
				}

				if (x + ox > mapedge || x + ox<0 || y + oy>mapedge || y + oy < 0)
					continue;

				FIELDDATA* cur_f = Map->GetFielddataAt(i + x + y * mapedge);

				int cur_ground = cur_f->wGround;
				if (cur_ground == 0xFFFF) cur_ground = 0;
				int cur_set;
				if ((*tiledata)[cur_ground].wTileSet != (*tiledata)[o_ground].wTileSet)
				{

					if (((*tiledata)[o_ground].wTileSet == cliffset || (*tiledata)[o_ground].wTileSet == slopesetpiecesset) && (*tiledata)[cur_ground].bMorphable)
					{
						int height_diff = f1->bHeight - cur_f->bHeight;
						if (o_ground >= 8 + iCliffStart && o_ground <= 13 + iCliffStart && (*tiledata)[o_ground].tiles[cur_f->bSubTile].bZHeight == 0) height_diff += 4;
						int need_diff = height_diff;//(*tiledata)[o_ground].tiles[f1->bSubTile].bZHeight-(*tiledata)[cur_ground].tiles[cur_f->bSubTile].bZHeight;
						//if((*tiledata)[o_ground].tiles[f1->bSubTile].bZ
						if (abs(height_diff) > 1)
						{
							//ChangeTileHeight(i+x+y*mapedge,f1->bHeight,FALSE, FALSE);
							int height = f1->bHeight;
							if (o_ground >= 8 + iCliffStart && o_ground <= 13 + iCliffStart && (*tiledata)[o_ground].tiles[cur_f->bSubTile].bZHeight == 0) height = f1->bHeight + 4;
							Map->SetHeightAt(i + x + y * mapedge, height);
							bChanged[i + x + y * mapedge] = TRUE;
						}
						else if (height_diff)
							Map->CreateSlopesAt(i + x + y * mapedge);
						//bChanged[i+x+y*mapedge]=TRUE;
					}
				}
			}
		}
	}

	for (i = 0;i < mapsize;i++)
	{

		if (bChanged[i])
		{
			int e;
			for (e = 0;e < Map->GetIsoSize() * Map->GetIsoSize();e++)
				Map->SetReserved(e, 0);
			ChangeTileHeight(i, Map->GetHeightAt(i), FALSE, FALSE);
		}

	}

	/*	delete[] bXSearch;
		delete[] bYSearch;*/

	if (bChanged) delete[] bChanged;

	RedrawWindow();

	Map->TakeSnapshot();
	Map->Undo();
}

inline void ToIso3d_NoAccessCheck(int* x, int* y)
{
	int cx = *x, cy = *y;

	ToIso(&cx, &cy);

	int i, e = 0;


	for (i = 15;i >= 0;i--)
	{
		for (e = 0;e < 3;e++)
		{
			int a;


			int m, n;
			int px, py;
			m = cx + i;
			n = cy + i;

			if (e == 1) m -= 1;
			if (e == 2) n -= 1;

			px = m;
			py = n;

			if (m >= 0 && n >= 0 && m < Map->GetIsoSize() && n < Map->GetIsoSize())
			{
				FIELDDATA mfd = *Map->GetFielddataAt(px + py * Map->GetIsoSize());
				int ground = mfd.wGround;
				if (ground == 0xFFFF) ground = 0;
				//if(mfd.bHide==FALSE && !(*tiledata)[ground].bHide)
				{

					ToPhys3d(&m, &n);

					// now m and n hold the pixel coordinates for the current field... 
					// we now need to check if cx and cy are in this field
					//if(*x >= m && *x<= m+f_x && *y>=n && *y<=n+f_y)
					{
						int dx = *x - m;
						int dy = *y - n - f_y / 2;//-f_y/2;
						int dx1 = dx;
						int dy1 = dy;

						dx = ((float)dy1) / ((float)f_y) - ((float)dx1) / ((float)f_x) + 1.0f;//+ 1 + (float)0.5 +1;
						dy = ((float)dx1) / ((float)f_x) + ((float)dy1) / ((float)f_y) + 0.0f;//- 1-(float)0.5 +1;

						if (((dx == 0 && dy == 0) || (!bAllowAccessBehindCliffs && *y - n > f_y)))
						{
							*x = px - 0;
							*y = py + 0;
							return;
						}
					}
				}
			}
		}
	}

	/*
for(i=15;i>=0;i--)
	{
		for(e=0;e<3;e++)
		{
			int a;


			int m,n;
			int px,py;
			m=cx+i;
			n=cy+i;

			if(e==1) m-=1;
			if(e==2) n-=1;

			px=m;
			py=n;

			if(m>=0 && n>=0 && m< Map->GetIsoSize() && n<Map->GetIsoSize())
			{

				ToPhys3d(&m, &n);

				// just find at least one tile if possible...

				if(*x>m && *y>n)
				{
					*x=px;
					*y=py;
					return;
				}
			}
		}
	}*/


	*x = cx;//-1;
	*y = cy;//-1;
}



void CIsoView::DrawMap()
{
	const CMapData* const Map = ::Map;

	if (bNoDraw) return;

	if (bCancelDraw)
	{
		bCancelDraw = FALSE;
		return;
	}

	last_succeeded_operation = 100;

	if (lpds == NULL || b_IsLoading || tiledata == NULL || (*tiledata) == NULL || (*tiledata_count == 0)) return; // just to make sure...

	if (lpds->IsLost() != DD_OK)
	{
		// we lost our surfaces, we need to reinitialize directdraw and all associated objects
		ReInitializeDDraw();
		return;
	}



	if (Map->GetIsoSize() == 0) return;

	auto startTime = std::chrono::steady_clock::now();

	// draw a white background
	DDBLTFX fx;
	memset(&fx, 0, sizeof(DDBLTFX));
	fx.dwSize = sizeof(DDBLTFX);
	fx.dwFillColor = RGB(255, 255, 255);
	lpdsBack->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &fx);


	// get the window rect
	
	RECT r = GetScaledDisplayRect();

	// the code here draws the coordinate system
	int i;

	
	// now, we draw all the objects
	

	int left, right, top, bottom;

	{
		RECT cr;
		GetClientRect(&cr);
		auto topLeft = GetMapCoordinatesFromClientCoordinates(CPoint(0, 0), false, true);
		auto topRight = GetMapCoordinatesFromClientCoordinates(CPoint(cr.right, 0), false, true);
		auto bottomLeft = GetMapCoordinatesFromClientCoordinates(CPoint(0, cr.bottom), false, true);
		auto bottomRight = GetMapCoordinatesFromClientCoordinates(CPoint(cr.right, cr.bottom), false, true);
		left = min(topLeft.x, topRight.x);
		top = min(topLeft.y, topRight.y);
		right = max(bottomLeft.x, bottomRight.x);
		bottom = max(bottomLeft.y, bottomRight.y);
	}

	// some large buildings may be out of reach:
	left -= 2;
	top -= 2;
	right += 7;
	bottom += 7;



	// validate the coordinates
	if (left < 0) left = 0;
	if (bottom >= Map->GetIsoSize() || bottom < top) bottom = Map->GetIsoSize();
	if (right >= Map->GetIsoSize() || right < left) right = Map->GetIsoSize();
	if (top < 0) top = 0;


	BOOL bMarbleHeight = TRUE;
	//if(Map->GetTheater()==THEATER4 || Map->GetTheater()==THEATER3) bMarbleHeight=FALSE;

	// Now left, right, top & bottom contain the needed values

	DWORD MM_heightstart = tilesets_start[atoi((*tiles).sections["General"].values["HeightBase"])];

	// now draw everything
	int u, v, z;
	int mapwidth, mapheight;
	mapwidth = Map->GetWidth();
	mapheight = Map->GetHeight();

	DDSURFACEDESC2 ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;

	lpdsBack->GetSurfaceDesc(&ddsd);


#ifdef NOSURFACES				
	lpdsBack->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
#endif

	// we render texts and waypoints last as they should be always visible anyway and because text rendering is currently done using GDI -> getting a DC for every text is too slow nowadays
	m_texts_to_render.clear();
	m_waypoints_to_render.clear();

	for (u = left;u < right;u++)
	{
		for (v = top;v < bottom;v++)
		{
			const MapCoords mapCoords(u, v);
			if (bCancelDraw)
			{
				bCancelDraw = FALSE;
			}
				
			if (u < 1 || v < 1 || u + v<mapwidth + 1 || u + v>mapwidth + mapheight * 2 || (v + 1 > mapwidth && u - 1 < v - mapwidth) || (u + 1 > mapwidth && v + mapwidth - 1 < u))
				continue;


			FIELDDATA m = *Map->GetFielddataAt(mapCoords);
			const auto drawCoords = GetRenderTargetCoordinates(mapCoords);

			// draw terrain
			if (m.wGround >= (*tiledata_count))
				m.wGround = 0;

			DWORD dwOrigGround = m.wGround;

			if (theApp.m_Options.bMarbleMadness)
			{
				if ((*tiledata)[m.wGround].wMarbleGround != 0xFFFF)
				{

					m.wGround = (*tiledata)[m.wGround].wMarbleGround;
				}
				else if (bMarbleHeight)
				{
					//drawy+=f_y*m.bHeight;

					m.wGround = MM_heightstart + m.bHeight;
					m.bSubTile = 0;
				}
			}

			if (!m.bRedrawTerrain)
			{

				TILEDATA* td = &(*tiledata)[m.wGround];
				if (td->bReplacementCount)
				{
					if (m.bRNDImage > 0)
					{
						m.bRNDImage <= td->bReplacementCount ? td = &td->lpReplacements[m.bRNDImage - 1] : td = &td->lpReplacements[td->bReplacementCount - 1];
					}
				}

				if (m.bSubTile < td->wTileCount && td->tiles[m.bSubTile].pic != NULL)
				{
					const SUBTILE& st = td->tiles[m.bSubTile];
					const auto stDrawCoords = drawCoords + st.drawOffset();


					if (!m.bHide && (*tiledata)[dwOrigGround].bHide == FALSE)
					{
#ifndef NOSURFACES
						Blit(st.pic, stDrawCoords.x, stDrawCoords.y, st.wWidth, st.wHeight);
#else
						BlitTerrain(ddsd.lpSurface, stDrawCoords.x, stDrawCoords.y, r.left, r.top, ddsd.lPitch, r.right, r.bottom, st);
#endif
					}
					else // draw soemthing representing the tile
					{
#ifndef NOSURFACES
						DrawCell(stDrawCoords.x, stDrawCoords.y, 1, 1, RGB(0, 140, 0), FALSE, FALSE);
#else
						BlitTerrainHalfTransp(ddsd.lpSurface, stDrawCoords.x, stDrawCoords.y, r.left, r.top, ddsd.lPitch, r.right, r.bottom, st);
#endif							

					}
				}

			}

		}
	}


	for (u = left;u < right;u++)
	{
		for (v = top;v < bottom;v++)
		{
			const MapCoords mapCoords(u, v);
			if (bCancelDraw)
			{
				bCancelDraw = FALSE;
			}

			if (u < 1 || v < 1 || u + v<mapwidth + 1 || u + v>mapwidth + mapheight * 2 || (v + 1 > mapwidth && u - 1 < v - mapwidth) || (u + 1 > mapwidth && v + mapwidth - 1 < u))
				continue;

			FIELDDATA m = *Map->GetFielddataAt(mapCoords);  // copy
			const auto drawCoords = GetRenderTargetCoordinates(mapCoords);

			if (m.wGround == 0xFFFF)
				m.wGround = 0;

			DWORD dwOrigGround = m.wGround;

			if (theApp.m_Options.bMarbleMadness)
			{
				if ((*tiledata)[m.wGround].wMarbleGround != 0xFFFF)
				{
					m.wGround = (*tiledata)[m.wGround].wMarbleGround;
				}
				else if (bMarbleHeight)
				{
					//y+=f_y*m.bHeight;										
					m.wGround = MM_heightstart + m.bHeight;
					m.bSubTile = 0;
				}
			}

			if (m.bRedrawTerrain)
			{
				// draw cliff again to hide buildings behind

				if (m.wGround < *tiledata_count)
				{
					TILEDATA* td = &(*tiledata)[m.wGround];
					if (td->bReplacementCount)
					{
						if (m.bRNDImage > 0)
						{
							m.bRNDImage <= td->bReplacementCount ? td = &td->lpReplacements[m.bRNDImage - 1] : td = &td->lpReplacements[td->bReplacementCount - 1];
						}
					}

					if (m.bSubTile < td->wTileCount && td->tiles[m.bSubTile].pic != NULL)
					{
						const SUBTILE& st = td->tiles[m.bSubTile];
						const auto stDrawCoords = drawCoords + st.drawOffset();


						if (!m.bHide && (*tiledata)[dwOrigGround].bHide == FALSE)
						{
#ifndef NOSURFACES
							Blit(st.pic, stDrawCoords.x, stDrawCoords.y, st.wWidth, st.wHeight);
#else
							BlitTerrain(ddsd.lpSurface, stDrawCoords.x, stDrawCoords.y, r.left, r.top, ddsd.lPitch, r.right, r.bottom, st);
							if (st.anim)
							{
								const auto animDrawCoords = drawCoords + ProjectedVec(f_x / 2 - st.anim->wMaxWidth / 2, f_y / 2 - st.anim->wMaxHeight / 2) + st.anim->drawOffset();
								BlitPic(ddsd.lpSurface, animDrawCoords.x, animDrawCoords.y, r.left, r.top, ddsd.lPitch, r.right, r.bottom, *st.anim);
							}
#endif

						}
						else // draw soemthing representing the tile
						{
#ifndef NOSURFACES
							DrawCell(stDrawCoords.x, stDrawCoords.y, 1, 1, RGB(0, 140, 0), FALSE, FALSE);
#else
							BlitTerrainHalfTransp(ddsd.lpSurface, stDrawCoords.x, stDrawCoords.y, r.left, r.top, ddsd.lPitch, r.right, r.bottom, st);
#endif							

						}
					}

				}
			}

			// draw terrain animation (tunnel tops, waterfalls)
			// tunnel tops might be repainted later so that units do not appear on top - this probably could be done in CLoading so that instead of a back tile a front tile is responsible for drawing the animation
			{
				if (m.wGround < *tiledata_count)
				{
					TILEDATA* td = &(*tiledata)[m.wGround];

					if (m.bSubTile < td->wTileCount)
					{
						const SUBTILE& st = td->tiles[m.bSubTile];

						if (!m.bHide && (*tiledata)[dwOrigGround].bHide == FALSE && st.anim)
						{
#ifndef NOSURFACES
#else
							const auto animDrawCoords = drawCoords + ProjectedVec(f_x / 2 - st.anim->wMaxWidth / 2, f_y / 2 - st.anim->wMaxHeight / 2) + st.anim->drawOffset();
							BlitPic(ddsd.lpSurface, animDrawCoords.x, animDrawCoords.y, r.left, r.top, ddsd.lPitch, r.right, r.bottom, *st.anim);
#endif
						}

					}
				}
			}

			// draw overlay
			if (m.overlay != 0xFF)
			{
				PICDATA pic;
				pic.pic = NULL;

				if (ovrlpics[m.overlay][m.overlaydata] != NULL)
				{
					pic = *ovrlpics[m.overlay][m.overlaydata];
				}


				if (pic.pic == NULL)
				{
					if (!pic.bTried)
					{
						SetError("Loading graphics");
						theApp.m_loading->LoadOverlayGraphic(*rules.sections["OverlayTypes"].GetValue(m.overlay), m.overlay);
						UpdateOverlayPictures(m.overlay);
						if (ovrlpics[m.overlay][m.overlaydata] != NULL)
							pic = *ovrlpics[m.overlay][m.overlaydata];
					}

					if (pic.pic == NULL)
					{
						if (!(m.overlay >= 0x4a && m.overlay <= 0x65) && !(m.overlay >= 0xcd && m.overlay <= 0xec))
						{
							char cd[50];
							cd[0] = '0';
							cd[1] = 'x';
							itoa(m.overlay, cd + 2, 16);

							m_texts_to_render.push_back({ cd, drawCoords.x + f_x / 2, drawCoords.y + f_y / 2, RGB(0,0,0), false, true, true});
							// TextOut(drawx,drawy, cd, RGB(0,0,0));
						}
					}
				}

				if (pic.pic != NULL)
				{
					ProjectedVec offset(f_x / 2 - pic.wMaxWidth / 2, -pic.wMaxHeight / 2);

					if (m.overlay == OVRL_VEINHOLE) // veinhole, big, special case
					{
						offset.y -= f_y * 3 / 2;
					}
					else if (isBigBridge(m.overlay)) // bridge special case
					{
						// drawy-=f_y;
						if (m.overlaydata >= 0x09 && m.overlaydata <= 0x11)
						{
							offset.y -= f_y / 2;
						}

						offset.x -= 1; // hmm... strange, but this is needed

					}
					else if (isTrack(m.overlay))
						offset.y += f_y / 2;

					if (m.overlay >= 0x4a && m.overlay <= 0x65) offset.y += f_y / 2;
					if (m.overlay >= 0xcd && m.overlay <= 0xec) offset.y += f_y / 2;


					const auto drawCoordsOvrl = drawCoords + offset;

#ifndef NOSURFACES
					Blit(pic.pic, drawCoordsOvrl.x, drawCoordsOvrl.y);
#else

#ifdef RA2_MODE
					BlitPic(ddsd.lpSurface, drawCoordsOvrl.x, drawCoordsOvrl.y, r.left, r.top, ddsd.lPitch, r.right, r.bottom, pic);
#endif
#ifdef TS_MODE 
					if (!isGreenTiberium(m.overlay) && !(m.overlay == 0x7f)) // no tib
						BlitPic(ddsd.lpSurface, drawCoordsOvrl.x, drawCoordsOvrl.y, r.left, r.top, ddsd.lPitch, r.right, r.bottom, pic);
					else if (m.overlay == 0x7f) // blue tib
					{
						int n = RGB(200, 0, 0);
						BlitPic(ddsd.lpSurface, drawCoordsOvrl.x, drawCoordsOvrl.y, r.left, r.top, ddsd.lPitch, r.right, r.bottom, pic, &n);
					}
					else
					{
						int n = RGB(0, 200, 0);
						BlitPic(ddsd.lpSurface, drawCoordsOvrl.x, drawCoordsOvrl.y, r.left, r.top, ddsd.lPitch, r.right, r.bottom, pic, &n);
					}
#endif

#endif

				}
			}

			if (m.structure != -1)
			{
				last_succeeded_operation = 10101;

				// for structures we need to check if they weren´t drawn earlier
				// (every field that this building achieves has this building as .structure)
				if (Map->GetStructureAt(mapCoords - MapVec(-1, 0)) != m.structure && Map->GetStructureAt(mapCoords - MapVec(0, -1)) != m.structure)
				{

					STRUCTUREPAINT objp;
					Map->GetStructurePaint(m.structure, &objp);

					const auto drawCoordsBld = GetRenderTargetCoordinates(MapCoords(objp.x, objp.y));
					int id = m.structuretype;


					int w = 1, h = 1;
					PICDATA pic;
					if (id > -1 && id < 0x0F00)
					{
						w = buildinginfo[id].w;
						h = buildinginfo[id].h;
						int dir = objp.direction / 32;

						// MW April 13th 2001: fix for building direction
						dir = (7 - dir) % 8;

						pic = buildinginfo[id].pic[dir];
						if (pic.pic == NULL) pic = buildinginfo[id].pic[0];

					}

#ifndef NOSURFACES
					DrawCell(drawCoordsBld.x, drawCoordsBld.y, w, h, colorref_conv[objp.col]);
#else
					// MW 07/19/01: Paint cell if user chose so...
					if (theApp.m_Options.bShowCells)
					{
						DrawCell(ddsd.lpSurface, ddsd.dwWidth, ddsd.dwHeight, ddsd.lPitch, drawCoordsBld.x, drawCoordsBld.y, w, h, colorref_conv[objp.col]);
					}
#endif


					if (pic.pic == NULL)
					{
						if (!missingimages[objp.type])
						{
							SetError("Loading graphics");
							theApp.m_loading->LoadUnitGraphic(objp.type);
							::Map->UpdateBuildingInfo(objp.type);
							int dir = (7 - objp.direction / 32) % 8;
							pic = buildinginfo[id].pic[dir];
							if (pic.pic == NULL) pic = buildinginfo[id].pic[0];
						}
						if (pic.pic == NULL)
						{
#ifndef NOSURFACES
							Blit(pics["HOUSE"].pic, drawCoordsBld.x, drawCoordsBld.y - 19); // draw a ugly house
#endif
							missingimages[objp.type] = TRUE;
						}
					}

					if (pic.pic) // picture
					{

						// it was that easy! just center on top end!!!
						const auto drawCoordsBldShp = drawCoordsBld + ProjectedVec(f_x / 2 - pic.wMaxWidth / 2, -pic.wMaxHeight / 2);

#ifndef NOSURFACES
						Blit(pic.pic, drawCoordsBldShp.x, drawCoordsBldShp.y, pic.wMaxWidth, pic.wMaxHeight);
#else

						BlitPic(ddsd.lpSurface, drawCoordsBldShp.x, drawCoordsBldShp.y, r.left, r.top, ddsd.lPitch, r.right, r.bottom, pic, &colorref_conv[objp.col]);
#endif

						for (int upgrade = 0; upgrade < objp.upradecount; ++upgrade)
						{
							const auto& upg = upgrade == 0 ? objp.upgrade1 : (upgrade == 1 ? objp.upgrade2 : objp.upgrade3);
							if (upg.GetLength() == 0)
								continue;

							PICDATA pic;
							int dir = (7 - objp.direction / 32) % 8;
							pic = pics[GetUnitPictureFilename(upg, dir)];
							if (!missingimages[upg] && pic.pic == NULL)
							{
								SetError("Loading graphics");
								theApp.m_loading->LoadUnitGraphic(upg);
								::Map->UpdateBuildingInfo(upg);
								pic = pics[GetUnitPictureFilename(upg, dir)];
								if (pic.pic == NULL) missingimages[upg] = TRUE;
							}

							if (pic.pic != NULL)
							{
								static const CString LocLookup[3][2] = { {"PowerUp1LocXX", "PowerUp1LocYY"}, {"PowerUp2LocXX", "PowerUp2LocYY"}, {"PowerUp3LocXX", "PowerUp3LocYY"} };
								const auto drawCoordsPowerUp = drawCoordsBldShp + ProjectedVec(
									atoi(art.sections[objp.type].values[LocLookup[upgrade][0]]),
									atoi(art.sections[objp.type].values[LocLookup[upgrade][1]])
								);
								// py-=atoi(art.sections[obj.type].values["PowerUp1LocZZ"]); 
#ifndef NOSURFACES
								Blit(pic.pic, drawCoordsPowerUp.x, drawCoordsPowerUp.y);
#else
								BlitPic(ddsd.lpSurface, drawCoordsPowerUp.x, drawCoordsPowerUp.y, r.left, r.top, ddsd.lPitch, r.right, r.bottom, pic, &colorref_conv[objp.col]);
#endif

							}

						}

					}
				}

			}


			if (m.node.type > -1)
			{
				last_succeeded_operation = 10102;

				CString house = m.node.house;
				CString tmp;
						
				if (Map->GetNodeAt(mapCoords + MapVec(1, 0), tmp) != m.node.index && Map->GetNodeAt(mapCoords + MapVec(0, 1), tmp) != m.node.index)
				{
					const auto drawCoordsBld = GetRenderTargetCoordinates(mapCoords - MapVec(buildinginfo[m.node.type].h - 1, buildinginfo[m.node.type].w - 1));

					COLORREF c;
					c = GetColor(house);


					int id = m.node.type;
					int w = 1, h = 1;
					PICDATA pic;
					if (id > -1 && id < 0x0F00)
					{
						w = buildinginfo[id].w;
						h = buildinginfo[id].h;
						pic = buildinginfo[id].pic[0];
					}


					//#ifndef NOSURFACES							
#ifdef NOSURFACES 
					if (m.structure >= 0) // only paint cell if we have a structure preplaced, as we have a half transparent image
					{
#endif
						// place it 2 pixels lower so that user can see the dotted lines even if the building itself has the cells drawn
						DrawCell(ddsd.lpSurface, ddsd.dwWidth, ddsd.dwHeight, ddsd.lPitch, drawCoordsBld.x, drawCoordsBld.y + 3, w, h, colorref_conv[c], true);

#ifdef NOSURFACES				
					}
#endif
					//#endif

					if (pic.pic == NULL)
					{
						if (!missingimages[*rules.sections["BuildingTypes"].GetValue(m.node.type)])
						{
							SetError("Loading graphics");
							theApp.m_loading->LoadUnitGraphic(*rules.sections["BuildingTypes"].GetValue(m.node.type));
							::Map->UpdateBuildingInfo(*rules.sections["BuildingTypes"].GetValue(m.node.type));
							pic = buildinginfo[id].pic[0];
						}
						if (pic.pic == NULL);
						{
#ifndef NOSURFACES
							Blit(pics["HOUSE"].pic, drawCoordsBld.x, drawCoordsBld.y - 19);
#endif
							missingimages[*rules.sections["BuildingTypes"].GetValue(m.node.type)] = TRUE;
						}
					}


					if (pic.pic) // picture
					{
						const auto drawCoordsBldShp = drawCoordsBld + ProjectedVec(f_x / 2 - pic.wMaxWidth / 2, -pic.wMaxHeight / 2);

#ifndef NOSURFACES					
						Blit(pic.pic, drawCoordsBldShp.x, drawCoordsBldShp.y);
#else
						BlitPicHalfTransp(ddsd.lpSurface, drawCoordsBldShp.x, drawCoordsBldShp.y, r.left, r.top, ddsd.lPitch, r.right, r.bottom, pic, &colorref_conv[c]);
#endif
					}
				}

			}
			if (m.unit != -1)
			{

				UNIT obj;
				Map->GetUnitData(m.unit, &obj);

				COLORREF c = GetColor(obj.house);


				CString lpPicFile = GetUnitPictureFilename(obj.type, atoi(obj.direction) / 32);

#ifndef NOSURFACES
				DrawCell(drawCoords.x, drawCoords.y, 1, 1, c);
#endif

				PICDATA p = pics[lpPicFile];

				if (p.pic == NULL || lpPicFile.GetLength() == 0)
				{
					if (!missingimages[obj.type])
					{
						SetError("Loading graphics");
						theApp.m_loading->LoadUnitGraphic(obj.type);
						lpPicFile = GetUnitPictureFilename(obj.type, atoi(obj.direction) / 32);
						p = pics[lpPicFile];
					}

					if (p.pic == NULL)
					{
#ifndef NOSURFACES
						Blit(pics["TANK"].pic, drawCoords.x, drawCoords.y);
						// TextOut(drawx+f_x/4,drawy+f_y/4, obj.type,c);
						m_texts_to_render.push_back({ obj.type, drawCoords.x + f_x / 4, drawCoords.y + f_y / 4, m_color_converter->GetColor(c)});
#endif
						missingimages[obj.type] = TRUE;
					}
				}

				if (p.pic)// we have a picture!
				{
					const auto drawCoordsOffset = (p.bType == PICDATA_TYPE_BMP) ? ProjectedVec((f_y / 4) + p.x, (f_y - p.wHeight) + p.y) - p.drawOffset() :
						(p.bType == PICDATA_TYPE_SHP) ? ProjectedVec(f_x / 2 - (p.wMaxWidth / 2), f_y / 2 - (p.wMaxHeight / 2)) : ProjectedVec(f_x / 2, f_y / 2) + p.drawOffset();
					auto drawCoordsUnit = drawCoords + drawCoordsOffset;

#ifndef NOSURFACES
					Blit(p.pic, drawCoordsUnit.x, drawCoordsUnit.y);
#else
					BlitPic(ddsd.lpSurface, drawCoordsUnit.x, drawCoordsUnit.y, r.left, r.top, ddsd.lPitch, r.right, r.bottom, p, &colorref_conv[c]);
#endif
				}
			}
			if (m.aircraft != -1)
			{
				last_succeeded_operation = 10102;

				AIRCRAFT obj;
				Map->GetAircraftData(m.aircraft, &obj);

				COLORREF c = GetColor(obj.house);


				CString lpPicFile = GetUnitPictureFilename(obj.type, atoi(obj.direction) / 32);

#ifndef NOSURFACES
				DrawCell(drawCoords.x, drawCoords.y, 1, 1, c);
#endif

				PICDATA p = pics[lpPicFile];

				if (p.pic == NULL)
				{
					if (!missingimages[obj.type])
					{
						SetError("Loading graphics");
						theApp.m_loading->LoadUnitGraphic(obj.type);
						p = pics[lpPicFile];
					}

					if (p.pic == NULL)
					{
#ifndef NOSURFACES
						Blit(pics["TANK"].pic, drawCoords.x, drawCoords.y);
						//TextOut(drawx+f_x/4,drawy+f_y/4, obj.type,c);
						m_texts_to_render.push_back({ obj.type, drawCoords.x + f_x / 4, drawCoords.y + f_y / 4, m_color_converter->GetColor(c) });
#endif
						missingimages[obj.type] = TRUE;
					}
				}

				if (p.pic)// we have a picture!
				{
					const auto drawCoordsOffset = (p.bType == PICDATA_TYPE_BMP) ? ProjectedVec(f_x / 2 - p.wWidth / 2, f_y - p.wHeight) - p.drawOffset() :
						(p.bType == PICDATA_TYPE_SHP) ? ProjectedVec(f_x / 2 - (p.wMaxWidth / 2), f_y / 2 - (p.wMaxHeight / 2)) : ProjectedVec(f_x / 2, f_y / 2) + p.drawOffset();
					auto drawCoordsAir = drawCoords + drawCoordsOffset;

#ifndef NOSURFACES
					Blit(p.pic, drawCoordsAir.x, drawCoordsAir.y);
#else
					BlitPic(ddsd.lpSurface, drawCoordsAir.x, drawCoordsAir.y, r.left, r.top, ddsd.lPitch, r.right, r.bottom, p, &colorref_conv[c]);
#endif
				}
			}
			int ic;
			for (ic = 0;ic < SUBPOS_COUNT;ic++)
				if (m.infantry[ic] != -1)
				{
					last_succeeded_operation = 10103;

					//errstream << "GetInfantryData()" << endl;
					//errstream.flush();

					INFANTRY obj;
					Map->GetInfantryData(m.infantry[ic], &obj);

					//errstream << "Done " << (LPCSTR)obj.type << endl;
					//errstream.flush();


					COLORREF c = GetColor(obj.house);

							
					int dir = (7 - atoi(obj.direction) / 32) % 8;
					CString lpPicFile = GetUnitPictureFilename(obj.type, dir);

#ifndef NOSURFACES
					DrawCell(drawCoords.x, drawCoords.y, 1, 1, c);
#endif

					static const ProjectedVec subPosLookup[5] = { ProjectedVec(0, -f_y / 4), ProjectedVec(f_x / 4 , 0), ProjectedVec(-f_x / 4, 0), ProjectedVec(0, f_y / 4), ProjectedVec()};
					auto drawCoordsInf = drawCoords + subPosLookup[ic > 4 ? 4 : ic];

					PICDATA p = pics[lpPicFile];

					if (p.pic == NULL)
					{
						if (!missingimages[obj.type])
						{
							SetError("Loading graphics");
							theApp.m_loading->LoadUnitGraphic(obj.type);
							p = pics[lpPicFile];
						}

						if (p.pic == NULL)
						{
#ifndef NOSURFACES
							Blit(pics["MAN"].pic, drawCoordsInf.x, drawCoordsInf.y);
							// TextOut(drawx+f_x/4,drawy+f_y/4, obj.type,c);
							m_texts_to_render.push_back({ obj.type, drawCoordsInf.x + f_x / 4, drawCoordsInf.y + f_y / 4, RGB(0,0,0) });
#endif
							missingimages[obj.type] = TRUE;
						}
					}



					if (p.pic)// we have a picture!
					{
						auto drawCoordsInfShp = drawCoordsInf + ProjectedVec(f_x / 2 - (p.wMaxWidth / 2), f_y / 2 - (p.wMaxHeight / 2));

#ifndef NOSURFACES
						Blit(p.pic, drawCoordsInfShp.x, drawCoordsInfShp.y, p.wWidth, p.wHeight);
#else
						BlitPic(ddsd.lpSurface, drawCoordsInfShp.x, drawCoordsInfShp.y, r.left, r.top, ddsd.lPitch, r.right, r.bottom, p, &colorref_conv[c]);
#endif

					}

				}
			if (m.terrain != -1)
			{
				last_succeeded_operation = 10104;


				int id = m.terraintype;
				int w = 1, h = 1;
				PICDATA pic;
				if (id > -1 && id < 0x0F00)
				{
					w = treeinfo[id].w;
					h = treeinfo[id].h;
					pic = treeinfo[id].pic;
				}

				//CString lpPicFile=GetUnitPictureFilename(type, 0);				

				if (pic.pic == NULL)
				{
					CString type;
					Map->GetTerrainData(m.terrain, &type);

					if (missingimages.find(type) == missingimages.end())
					{
						SetError("Loading graphics");
						theApp.m_loading->LoadUnitGraphic(type);
						::Map->UpdateTreeInfo(type);
						pic = treeinfo[id].pic;
					}
					if (pic.pic == NULL)
					{
#ifndef NOSURFACES
						Blit(pics["TREE"].pic, drawCoords.x, drawCoords.y - 19);
#endif
						missingimages[type] = TRUE;
					}
				}

				if (pic.pic);
				{

					auto drawCoordsTerrain = drawCoords + ProjectedVec(f_x / 2 - (pic.wMaxWidth / 2), f_y / 2 - 3 - (pic.wMaxHeight / 2));

#ifndef NOSURFACES
					Blit(pic.pic, drawCoordsTerrain.x, drawCoordsTerrain.y);
#else
					BlitPic(ddsd.lpSurface, drawCoordsTerrain.x, drawCoordsTerrain.y, r.left, r.top, ddsd.lPitch, r.right, r.bottom, pic);

#endif


				}

			}


#ifdef SMUDGE_SUPP
			if (m.smudge != -1)
			{
				last_succeeded_operation = 10104;


				int id = m.smudgetype;

				PICDATA pic;
				if (id > -1 && id < 0x0F00)
				{
					pic = smudgeinfo[id].pic;
				}

				if (pic.pic == NULL)
				{
					SMUDGE data;
					CString& type = data.type;
					Map->GetSmudgeData(m.smudge, &data);

					if (missingimages.find(type) == missingimages.end())
					{
						SetError("Loading graphics");
						theApp.m_loading->LoadUnitGraphic(type);
						::Map->UpdateSmudgeInfo(type);
						pic = smudgeinfo[id].pic;
					}
					if (pic.pic == NULL)
					{
#ifndef NOSURFACES
						// Blit(pics["TREE"].pic,drawCoords.x,drawCoords.y-19);
#endif
						missingimages[type] = TRUE;
					}
				}

				if (pic.pic)
				{
					auto drawCoordsSmudge = drawCoords + ProjectedVec(f_x / 2 - (pic.wMaxWidth / 2), /*f_y / 2 - 3*/ - (pic.wMaxHeight / 2));

#ifndef NOSURFACES
					Blit(pic.pic, drawCoordsSmudge.x, drawCoordsSmudge.y);
#else
					BlitPic(ddsd.lpSurface, drawCoordsSmudge.x, drawCoordsSmudge.y, r.left, r.top, ddsd.lPitch, r.right, r.bottom, pic);

#endif


				}

			}
#endif


			if (m.celltag != -1)
			{

#ifdef NOSURFACES
				lpdsBack->Unlock(NULL);
#endif
				Blit((LPDIRECTDRAWSURFACE4)pics["CELLTAG"].pic, drawCoords.x - 1, drawCoords.y - 1);

#ifdef NOSURFACES				
				lpdsBack->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
#endif
			}

			if (m.waypoint != -1)
			{

				DWORD dwPos;
				CString ID;
				Map->GetWaypointData(m.waypoint, &ID, &dwPos);


#ifdef NOSURFACES
				lpdsBack->Unlock(NULL);
#endif

				// move the graphic and text into place
#ifdef RA2_MODE
				int image_fudge_x = 4;
				int image_fudge_y = -20;
				int text_fudge_x = 12;
				int text_fudge_y = -24;
#else
				int image_fudge_x = 4;
				int image_fudge_y = -15;
				int text_fudge_x = 9;
				int text_fudge_y = -17;
#endif

				const ProjectedVec waypointImageOffset(image_fudge_x, image_fudge_y);
				const ProjectedVec waypointTextOffset((f_x / 2) + text_fudge_x, (f_y / 2) + text_fudge_y);
#ifdef RA2_MODE
				bool useFont9 = false;
#else
				bool useFont9 = true;
#endif
				const auto waypointImageCoords = ProjectedCoords({ drawCoords.x, drawCoords.y }) + waypointImageOffset;
				const auto waypointTextCoords = ProjectedCoords({ drawCoords.x, drawCoords.y }) + waypointTextOffset;
				m_waypoints_to_render.push_back({ waypointImageCoords.x, waypointImageCoords.y });
				m_texts_to_render.push_back({ ID.GetString(), waypointTextCoords.x, waypointTextCoords.y, RGB(0,0,255), false, useFont9, true});
#ifdef NOSURFACES				
				lpdsBack->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
#endif
			}

		}

	}

	for (const auto& tube : Map->GetTubes())
	{
		DrawTube(*tube, &ddsd);
	}

#ifdef NOSURFACES
	lpdsBack->Unlock(NULL);
#endif

	// delayed waypoint rendering
	for (const auto& wp : m_waypoints_to_render)
	{
		Blit((LPDIRECTDRAWSURFACE4)pics["FLAG"].pic, wp.drawx, wp.drawy);
	}

	// map tool rendering
	if (AD.mode == ACTIONMODE_MAPTOOL)
	{
		if (AD.tool)
			AD.tool->render();
	}

	lpdsTemp->Blt(NULL, lpdsBack, NULL, 0, 0); // lpdsTemp always holds the scene drawn above, unscaled to the window

	if (m_cellCursor != MapCoords(-1, -1))
	{
		SurfaceLocker locker(lpdsBack);
		auto desc = locker.ensure_locked();
		if (desc)
			DrawCellCursor(m_cellCursor, *desc);
	}

	auto renderDuration = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::steady_clock::now() - startTime); // not including text output, ReleaseDC, waitForVerticalBlank etc below

	if (bDrawStats)
	{
		if (theApp.m_Options.bShowStats)
		{
			auto systemStats = std::format("ms: {0}  Left: {1}  Top: {2}  Right: {3}  Bottom: {4}", renderDuration.count(), left, top, right, bottom);
			m_texts_to_render.push_back({ systemStats.c_str(), r.left + 10, r.top + 10 + (-m_fontDefaultHeight) * 3 / 2, RGB(0,0,0), true });
		}

		auto moneyStr = std::format("Credits on map: {0}", Map->GetMoneyOnMap());
		m_texts_to_render.push_back({ moneyStr.c_str(), r.left + 10, r.top + 10, RGB(0,0,0), true });
	}

	if (rscroll)
	{
		const auto& sc = pics["SCROLLCURSOR"];
		Blit((LPDIRECTDRAWSURFACE4)sc.pic, rclick_x * m_viewScale.x + r.left - sc.wWidth / 2, rclick_y * m_viewScale.y + r.top - sc.wHeight / 2);
	}

	BlitBackbufferToHighRes(); // lpdsBackHighRes contains the same graphic, but scaled to the whole window
		
	RenderUIOverlay();
	if (theApp.m_Options.bVSync)
		dd->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, NULL);
	FlipHighResBuffer();
	last_succeeded_operation = 10100;

}

void CIsoView::RenderUIOverlay()
{
	if (!m_textDefault)
		updateFontScaled();

	LPDIRECTDRAWSURFACE4 dds = lpdsBack;
	bool useHighRes = false;
	if (m_viewScale != Vec2<CSProjected, float>(1.0f, 1.0f) && lpdsBackHighRes)
	{
		dds = lpdsBackHighRes;
		useHighRes = true;
	}

	DDSURFACEDESC2 ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;

	dds->GetSurfaceDesc(&ddsd);

	dds->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT | DDLOCK_NOSYSLOCK, NULL);
	LineDrawer d(ddsd.lpSurface, bpp, ddsd.dwWidth, ddsd.dwHeight, ddsd.lPitch);

	int lr, lt, ll, lb;

	ll = Map->GetWidth();
	lt = 1;
	lr = Map->GetHeight();
	lb = Map->GetIsoSize() - 1;

	int h_lefttop = 0;//Map->GetHeightAt(ll+lt*Map->GetIsoSize());
	int h_leftbottom = 0;//Map->GetHeightAt(lr+lb*Map->GetIsoSize());

	ToPhys((int*)&ll, (int*)&lt);
	ToPhys((int*)&lr, (int*)&lb);
	ll -= m_viewOffset.x - f_x / 2;
	lt -= m_viewOffset.y - f_y / 2;
	lr -= m_viewOffset.x - f_x / 2;
	lb -= m_viewOffset.y - f_y / 2;

	lt -= h_lefttop * f_y / 2;
	lb -= h_leftbottom * f_y / 2;

	auto sllt = useHighRes ? ScaleBackToFrontBuffer(ProjectedCoords(ll, lt)) : ProjectedCoords(ll, lt);
	auto slbr = useHighRes ? ScaleBackToFrontBuffer(ProjectedCoords(lr, lb)) : ProjectedCoords(lr, lb);

	RECT ls;
	Map->GetLocalSize(&ls);
	ProjectedCoords l_lt(
		ll + ls.left * f_x - f_x / 2,
		lt + (ls.top - 4) * f_y
	);
	ProjectedCoords l_br(
		ll + ls.left * f_x + ls.right * f_x - f_x / 2,
		lt + (ls.top - 4 + 4) * f_y + ls.bottom * f_y
	);
	auto ls_lt = useHighRes ? ScaleBackToFrontBuffer(l_lt) : l_lt;
	auto ls_br = useHighRes ? ScaleBackToFrontBuffer(l_br) : l_br;

	auto red = m_color_converter->GetColor(255, 0, 0);
	auto blue = m_color_converter->GetColor(0, 0, 255);
	d.Rectangle(sllt.x, sllt.y, slbr.x, slbr.y, red);
	d.Rectangle(sllt.x-1, sllt.y-1, slbr.x+1, slbr.y+1, red);
	d.Rectangle(ls_lt.x, ls_lt.y, ls_br.x, ls_br.y, blue);
	d.Rectangle(ls_lt.x+1, ls_lt.y+1, ls_br.x-1, ls_br.y-1, blue);

	dds->Unlock(NULL);
	
	RECT r;
	GetWindowRect(&r);

	for (const auto& s : m_texts_to_render)
	{
		const bool blue = s.color == RGB(0, 0, 255);  // TODO: TextRenderer should support setting the color at render time
		if (s.fixedScreenPos || !useHighRes)
		{
			auto textRenderer = s.useFont9 ? (blue ? *m_textBlue9 : *m_text9) : (blue ? *m_textBlue : *m_textDefault);
			textRenderer.RenderText(dds, s.drawx, s.drawy, s.text, s.centered);
		}
		else
		{
			auto textRenderer = s.useFont9 ? (blue ? *m_textBlue9Scaled : *m_text9Scaled) : (blue ? *m_textBlueScaled : *m_textScaled);
			textRenderer.RenderText(dds, r.left + (s.drawx - r.left) / m_viewScale.x, r.top + (s.drawy - r.top) / m_viewScale.y, s.text, s.centered);
		}
	}
}


void CIsoView::OnRButtonDown(UINT nFlags, CPoint point)
{
	rclick_x = point.x;
	rclick_y = point.y;
	rscroll = FALSE;

	CView::OnRButtonDown(nFlags, point);
}

void CIsoView::SetScroll(int xscroll, int yscroll)
{
	RECT r;
	GetWindowRect(&r);

	m_viewOffset = ProjectedVec(xscroll, yscroll);

	
	if (m_viewOffset.x < (Map->GetHeight() / 2 - 14 - r.left / f_x) * f_x)
		m_viewOffset.x = (Map->GetHeight() / 2 - 14 - r.left / f_x) * f_x;
	if (m_viewOffset.x + r.right * m_viewScale.x > (Map->GetHeight() / 2 + Map->GetWidth() + 14) * f_x)
		m_viewOffset.x = (Map->GetHeight() / 2 + Map->GetWidth() + 14) * f_x - r.right * m_viewScale.x;
	if (m_viewOffset.y < (Map->GetWidth() / 2 - 14 - r.top / f_y) * f_y)
		m_viewOffset.y = (Map->GetWidth() / 2 - 14 - r.top / f_y) * f_y;
	if (m_viewOffset.y + r.bottom * m_viewScale.y > (Map->GetWidth() / 2 + Map->GetHeight() + 4) * f_y)
		m_viewOffset.y = (Map->GetWidth() / 2 + Map->GetHeight() + 4) * f_y - r.bottom * m_viewScale.y;

	SetScrollPos(SB_VERT, (m_viewOffset.y / f_y - Map->GetWidth() / 2 + 4), TRUE);
	SetScrollPos(SB_HORZ, (m_viewOffset.x / f_x - Map->GetHeight() / 2 + 1), TRUE);

}

RECT CIsoView::GetScaledDisplayRect() const
{
	RECT r;
	GetWindowRect(&r);
	r.right = r.right - (r.right - r.left) * (1.0f - m_viewScale.x);
	r.bottom = r.bottom - (r.bottom - r.top) * (1.0f - m_viewScale.y);
	return r; // RVO
}

void CIsoView::GetScroll(int& xscroll, int& yscroll) const
{
	xscroll = m_viewOffset.x;
	yscroll = m_viewOffset.y;
}

void CIsoView::OnKillFocus(CWnd* pNewWnd)
{
	CView::OnKillFocus(pNewWnd);

	if (rscroll)
	{
		errstream << "Killing scroll" << endl;
		errstream.flush();

		ReleaseCapture();
		KillTimer(11);
		ShowCursor(TRUE);
		rscroll = FALSE;
		bDoNotAllowScroll = TRUE;
	}

}

// takes an index, not an waypoint id!
void CIsoView::FocusWaypoint(int index)
{
	int x, y;

	DWORD dwPos;

	Map->GetWaypointData(index, NULL, &dwPos);

	x = dwPos % Map->GetIsoSize();
	y = dwPos / Map->GetIsoSize();

	ToPhys3d(&x, &y);

	RECT r;
	GetWindowRect(&r);

	auto pos = ProjectedVec((x - (r.right - r.left) / 2 * m_viewScale.x)  - r.left, (y - (r.bottom - r.top) / 2 * m_viewScale.y) - r.top);
	SetScroll(pos.x, pos.y);

	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}


BOOL CIsoView::OnMouseWheel(UINT nFlags, short zDelta, CPoint ptScreen)
{
	CPoint pt = ptScreen;
	ScreenToClient(&pt);  // Attention: MouseWheel event has screen coordinates in contrast to e.g. MouseMove

	float smoothF = float(zDelta) / std::numeric_limits<short>::max();
	float fixedF = zDelta > 0 ? std::numeric_limits<short>::max() : std::numeric_limits<short>::min();

	Zoom(pt, theApp.m_Options.viewScaleUseSteps ? fixedF : smoothF);

	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	CMyViewFrame& dlg = *(CMyViewFrame*)owner;
	dlg.m_minimap.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CIsoView::Zoom(CPoint& pt, float f)
{
	auto oldScaledMPos = GetProjectedCoordinatesFromClientCoordinates(pt);

	auto oldScale = m_viewScale;

	auto oldViewScaleControl = m_viewScaleControl;
	m_viewScaleControl *= (1.0f - f * theApp.m_Options.viewScaleSpeed);
	if (m_viewScaleControl > 1.0f)
		m_viewScaleControl = 1.0f;
	if (m_viewScaleControl < 0.1f)
		m_viewScaleControl = 0.1f;
		
	if (!theApp.m_Options.viewScaleUseSteps)
		m_viewScale.set(m_viewScaleControl, m_viewScaleControl);
	else
	{
		// find previous step
		auto prevStepIt = std::find_if(theApp.m_Options.viewScaleSteps.rbegin(), theApp.m_Options.viewScaleSteps.rend(), [oldScale](auto f) {return f >= oldScale.x;});
		auto stepIt = std::find_if(theApp.m_Options.viewScaleSteps.rbegin(), theApp.m_Options.viewScaleSteps.rend(), [this](auto f) {return f >= m_viewScaleControl;});
		if (stepIt > prevStepIt)
			stepIt = prevStepIt + 1;
		if (stepIt < prevStepIt)
			stepIt = prevStepIt - 1;
		auto nextScale = stepIt == theApp.m_Options.viewScaleSteps.rend() ? 1.0f : *stepIt;
		m_viewScale.set(nextScale, nextScale);
	}
	
	auto newScaledMPos = GetProjectedCoordinatesFromClientCoordinates(pt);

	m_viewOffset = m_viewOffset + ((oldScaledMPos - newScaledMPos)).convertT<std::int32_t>();  // this lets the mouse cursor pixel stay constant

	UpdateScrollRanges();
	SetScroll(m_viewOffset.x, m_viewOffset.y);
	updateFontScaled();
}


void CIsoView::OnMButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Fügen Sie hier Ihren Meldungshandlercode ein, und/oder benutzen Sie den Standard.
	m_MButtonDown = point;
	m_MButtonMoveZooming = point;
	m_zooming = true;
	CView::OnMButtonDown(nFlags, point);
}


void CIsoView::OnMButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Fügen Sie hier Ihren Meldungshandlercode ein, und/oder benutzen Sie den Standard.
	m_zooming = false;
	CView::OnMButtonUp(nFlags, point);
}
