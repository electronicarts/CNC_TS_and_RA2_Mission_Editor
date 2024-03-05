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

// MiniMap.cpp: implementaion file
//

#include "stdafx.h"
#include "finalsun.h"
#include "MiniMap.h"
#include "finalsundlg.h"
#include "macros.h"
#include "mapdata.h"
#include "variables.h"
#include "inlines.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMiniMap

IMPLEMENT_DYNCREATE(CMiniMap, CView)

CMiniMap::CMiniMap(): m_scale(theApp.m_Options.fMiniMapScale)
{
}

CMiniMap::~CMiniMap()
{
}


BEGIN_MESSAGE_MAP(CMiniMap, CView)
	//{{AFX_MSG_MAP(CMiniMap)
	ON_WM_SYSCOMMAND()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
	ON_WM_SIZING()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// draw CMiniMap 

void CMiniMap::OnDraw(CDC* pDC)
{
	
	BYTE* colors;
	BITMAPINFO biinfo;
	int pitch;
	DrawMinimap(&colors, biinfo, pitch);

	if(!colors) return;
	
	// set the bits
	//SetDIBitsToDevice(pDC->m_hDC, 0, 0, Map->GetWidth()*2, Map->GetHeight(),0,0,0, Map->GetHeight(), colors, &biinfo, DIB_RGB_COLORS);
	RECT r;
	GetClientRect(&r);
	StretchDIBits(pDC->m_hDC, 0, 0, r.right, r.bottom, 0, 0, Map->GetWidth() * 2, Map->GetHeight(), colors, &biinfo, DIB_RGB_COLORS, SRCCOPY);


	// now draw the current position
	CFinalSunDlg& dlg=*(CFinalSunDlg*)theApp.GetMainWnd();
	CIsoView& isoview=*dlg.m_view.m_isoview;
	RECT selRect;
	auto isoRect = isoview.GetScaledDisplayRect();
	
	int mapwidth=Map->GetWidth();
	int mapheight=Map->GetHeight();

	
	// Get iso view display rectangle
	RECT cr;
	isoview.GetClientRect(&cr);
	auto topLeft = Map->GetMiniMapPos(isoview.GetMapCoordinatesFromClientCoordinates(CPoint(0, 0), false, true));
	auto topRight = Map->GetMiniMapPos(isoview.GetMapCoordinatesFromClientCoordinates(CPoint(cr.right, 0), false, true));
	auto bottomLeft = Map->GetMiniMapPos(isoview.GetMapCoordinatesFromClientCoordinates(CPoint(0, cr.bottom), false, true));
	auto bottomRight = Map->GetMiniMapPos(isoview.GetMapCoordinatesFromClientCoordinates(CPoint(cr.right, cr.bottom), false, true));
	auto left = min(topLeft.x, topRight.x);
	auto top = min(topLeft.y, topRight.y);
	auto right = max(bottomLeft.x, bottomRight.x);
	auto bottom = max(bottomLeft.y, bottomRight.y);

	CPoint center(r.right / 2, r.bottom / 2);
	selRect.left = left * m_scale;
	selRect.top = top * m_scale;
	selRect.right = right * m_scale;
	selRect.bottom = bottom * m_scale;


	pDC->Draw3dRect(&selRect, RGB(200,0,0), RGB(120,0,0));
	
}

/////////////////////////////////////////////////////////////////////////////
// diagnose CMiniMap

#ifdef _DEBUG
void CMiniMap::AssertValid() const
{
	CView::AssertValid();
}

void CMiniMap::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// handlers

BOOL CMiniMap::PreCreateWindow(CREATESTRUCT& cs) 
{
	// MW 07/17/2001: Style changed

	cs.style = WS_CAPTION | WS_OVERLAPPED | WS_SYSMENU;// | WS_CHILD;
	cs.cx=0;
	cs.cy=0;
	
	CFinalSunDlg& dlg=*(CFinalSunDlg*)theApp.GetMainWnd();
	//CIsoView& isoview=*dlg.m_view.m_isoview;
	RECT r;
	dlg.GetWindowRect(&r);
	
	cs.x=0;//r.right-250 ;
	cs.y=r.bottom-250;
	if(cs.y<0) cs.y=0;
	//cs.dwExStyle=WS_EX_TOOLWINDOW;
	//cs.dwExStyle=WS_EX_PALETTEWINDOW;
	
	// this here will cause an assert in debug mode, ignore it (window must be a child window)
	int res=CWnd::PreCreateWindow(cs);

	cs.style=WS_POPUPWINDOW | WS_CAPTION /*| WS_DLGFRAME */ | WS_THICKFRAME | WS_OVERLAPPED | DS_3DLOOK | WS_MINIMIZEBOX; //WS_CAPTION | WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX;

	return res;
	
}

void CMiniMap::UpdateView()
{
	CRect r;
	GetWindowRect(r);

	if(Map->GetIsoSize()==0)
	{
		ShowWindow(SW_HIDE);
	}
	else
	{
		// calculate the needed width=height
		int axissizex=Map->GetWidth()*2;
		int axissizey=Map->GetHeight();

		SetIcon(theApp.m_pMainWnd->GetIcon(FALSE), FALSE);
		SetIcon(theApp.m_pMainWnd->GetIcon(TRUE), TRUE);
		SetWindowPos(&wndTopMost, r.left, r.top, axissizex * m_scale + 2 * (GetSystemMetrics(SM_CXFIXEDFRAME)), axissizey * m_scale + 2 * GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION), SWP_SHOWWINDOW);
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}

	
}


BOOL bMiniMapClosedByUser=FALSE;

void CMiniMap::OnSysCommand(UINT nID, LPARAM lParam) 
{

	if(nID==SC_CLOSE)
	{
		bMiniMapClosedByUser=TRUE;
		ShowWindow(SW_HIDE);
		return;
	}
	
	CWnd::OnSysCommand(nID, lParam);
}


void CMiniMap::OnMouseMove(UINT nFlags, CPoint point) 
{
	if(nFlags==MK_LBUTTON)
	{
		int x,y;
		int xiso;
		int yiso;

		//y=Map->GetIsoSize()-(point.y - point.x+ /*(float)Map->GetIsoSize()/2.0f*/+1);
		//x=Map->GetIsoSize()-(point.x + point.y- /*(float)Map->GetIsoSize()/2.0f*/+1);
		//y-=Map->GetHeight();

		RECT cr;
		GetClientRect(&cr);
		float defaultXSize = (Map->GetWidth() * 2 * m_scale);
		float defaultYSize = (Map->GetHeight() * m_scale);
		float resizedXScale = cr.right / defaultXSize;
		float resizedYScale = cr.bottom / defaultYSize;

		CFinalSunDlg& dlg = *(CFinalSunDlg*)theApp.GetMainWnd();
		CIsoView& isoview = *dlg.m_view.m_isoview;

		auto viewScale = isoview.GetViewScale();
		auto viewOffset = isoview.GetViewOffset();

		x = (point.x / m_scale / resizedXScale) / 2 +Map->GetHeight() / 2;
		y = (point.y / m_scale / resizedYScale) + Map->GetWidth() / 2;

		
		RECT r = isoview.GetScaledDisplayRect();

		
		isoview.SetScroll((x-r.right/f_x/2)*f_x, (y-r.bottom/f_y/2)*f_y);

		
		isoview.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

void CMiniMap::OnLButtonDown(UINT nFlags, CPoint point) 
{
	
	OnMouseMove(nFlags, point);
}

void CMiniMap::DrawMinimap(BYTE **lpData, BITMAPINFO &biinfo, int& Pitch)
{
	Map->GetMinimap(lpData, &biinfo, &Pitch);	

	// fix straw pixels
	auto* data = *lpData;
	if (data)
	{
		// fix bottom left and right top pixels in the bottom-up bitmap
		memcpy(&data[0], &data[sizeof(RGBTRIPLE)], sizeof(RGBTRIPLE));
		auto firstLine = (biinfo.bmiHeader.biHeight - 1) * Pitch;
		memcpy(&data[firstLine + (biinfo.bmiHeader.biWidth - 1) * sizeof(RGBTRIPLE)], &data[firstLine + (biinfo.bmiHeader.biWidth - 2) * sizeof(RGBTRIPLE)], sizeof(RGBTRIPLE));
	}
}



void CMiniMap::OnSizing(UINT fwSide, LPRECT pRect)
{
	CView::OnSizing(fwSide, pRect);

	int axissizex = Map->GetWidth() * 2;
	int axissizey = Map->GetHeight();
	float ratio = float(axissizex) / float(axissizey);

	if (pRect)
	{
		int heightAdd = 2 * GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION);
		int widthAdd = 2 * GetSystemMetrics(SM_CXFIXEDFRAME);
		if (fwSide == WMSZ_LEFT || fwSide == WMSZ_TOPLEFT || fwSide == WMSZ_BOTTOMLEFT ||
			fwSide == WMSZ_RIGHT || fwSide == WMSZ_BOTTOMRIGHT)
		{
			pRect->bottom = pRect->top + (pRect->right - pRect->left - widthAdd) / ratio + heightAdd;
		}
		else
		{
			pRect->right = pRect->left + (pRect->bottom - pRect->top - heightAdd) * ratio + widthAdd;
		}
	}

	//width = axissizex* m_scale + 2 * (GetSystemMetrics(SM_CXFIXEDFRAME))

	m_scale = ((pRect->right - pRect->left) - 2 * GetSystemMetrics(SM_CXFIXEDFRAME)) / (float)axissizex;
	
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}


BOOL CMiniMap::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (!CView::OnSetCursor(pWnd, nHitTest, message))
		SetCursor(m_hArrowCursor);
	return TRUE;
}
