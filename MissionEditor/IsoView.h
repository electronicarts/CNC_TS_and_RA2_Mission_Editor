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

#if !defined(AFX_ISOVIEW_H__7FB6D6A0_7B52_11D3_99E1_DA6DFD21E706__INCLUDED_)
#define AFX_ISOVIEW_H__7FB6D6A0_7B52_11D3_99E1_DA6DFD21E706__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IsoView.h : header file
//



/////////////////////////////////////////////////////////////////////////////
// View CIsoView 

#include <vector>
#include <memory>
#include "MissionEditorPackLib.h"
#include "Structs.h"



struct TextToRender
{
	std::string text;
	int drawx;
	int drawy;
	int color;
	bool fixedScreenPos = false;	
	bool useFont9 = false;
	bool centered = false;
};

struct WaypointToRender
{
	int drawx;
	int drawy;
};

class TextDrawer;
class CTube;


class CIsoView : public CView
{
protected:
	DECLARE_DYNCREATE(CIsoView)

// attributes
public:
	INT m_mapx;
	INT m_mapy;
	int m_FlattenHeight; // for flatten ground
	int m_FlattenLastX;
	int m_FlattenLastY;
	int m_TileChangeCount;
	BOOL bThreadPainting;

private:
	ProjectedVec m_viewOffset;
	Vec2<CSProjected, float> m_viewScale;  // this is used for display and may e.g. be locked to zoom steps
	float m_viewScaleControl;  // you control these with your mouse
	CPoint m_MButtonDown;
	CPoint m_MButtonMoveZooming;

	int rclick_x; // scroll pixel coordinates
	int rclick_y;
	int cur_x_mouse;
	int cur_y_mouse;
	BOOL rscroll;
	BOOL m_bAltCliff;
	bool m_zooming;

	std::vector<TextToRender> m_texts_to_render;
	std::vector<WaypointToRender> m_waypoints_to_render;

	

// operations
public:
	CIsoView();           
	virtual ~CIsoView();

// overwriteables
	//{{AFX_VIRTUAL(CIsoView)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual void OnDraw(CDC* pDC);      // Überschrieben zum Zeichnen dieser Ansicht
	virtual void OnInitialUpdate();     // Zum ersten Mal nach der Konstruktion
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// implementation
protected:
	
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


	// generated message maps
	//{{AFX_MSG(CIsoView)
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);	
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDeadChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void FocusWaypoint(int index);
	RECT m_funcRect;
	Vec2<CSProjected, float> GetViewScale() const
	{
		return m_viewScale;
	}
	ProjectedVec GetViewOffset() const
	{
		return m_viewOffset;
	}
	RECT GetScaledDisplayRect() const;
	void GetScroll(int& xscroll, int& yscroll) const;
	void SetScroll(int xscroll, int yscroll);
	void DrawMap();
	void AutoLevel();
	void FillArea(DWORD dwX, DWORD dwY, DWORD dwID, BYTE bSubTile);
	BOOL m_NoMove;
	void PlaceCurrentObjectAt(int x, int y);
	void PlaceTile(const int x, const int y, const UINT nMouseFlags);
	void ShowAllTileSets();
	void HideTileSet(DWORD dwTileSet);
	void FlipHighResBuffer();
	void BlitBackbufferToHighRes();
	void RenderUIOverlay();
	int m_BrushSize_x;
	int m_BrushSize_y;
	BOOL ReachableFrom(DWORD dwStart, DWORD dwEnd);
	void __fastcall ChangeTileHeight(DWORD dwPos, DWORD dwNewHeight, BOOL bNonMorphableMove, BOOL bOnlyThisTile=FALSE, BOOL bNoSlopes=FALSE);
	void UpdateOverlayPictures(int id=-1);
	void UpdateStatusBar(int x, int y);
	void DrawCell(int x, int y, int w, int h, COLORREF col, BOOL dotted=FALSE, HDC hDC=nullptr);
	void DrawCell(void* dest, int dest_width, int dest_height, int dest_pitch, int x, int y, int w, int h, int col, bool dotted=false, bool touchNeighbours=false, int colNeighbour=CLR_INVALID) const;
	void DrawCellCursor(const MapCoords& mapCoords, const DDSURFACEDESC2& desc);
	void DrawTube(const CTube& tube, const DDSURFACEDESC2* ddsd=nullptr, const COLORREF* color=nullptr) const;

	/// <summary>
	/// Converts from (world) pixel coordinates to logical map coordinates
	/// </summary>
	/// <param name="projCoords">World pixel coordinates</param>
	/// <returns>Logical map coordinates</returns>
	MapCoords GetMapCoordinates(const ProjectedCoords& projCoords, bool bAllowAccessBehindCliffs=false, bool ignoreHideFlags = false) const;
	
	/// <summary>
	/// Converts from view / render target pixel coordinates (0/0 is top left corner of screen) to logical map coordinates 
	/// If you use mouse-move coordinates you need to 
	/// </summary>
	/// <param name="projCoords">Texel coordinates of the backbuffer render target</param>
	/// <returns>Logical map coordinates</returns>
	MapCoords GetMapCoordinatesFromRenderTargetCoordinates(const ProjectedCoords& screenViewCoords, bool bAllowAccessBehindCliffs = false, bool ignoreHideFlags = false) const;

	/// <summary>
	/// Converts from Win32 window client coordinates (0/0 at the top left corner of the view window) to logical map coordinates
	/// </summary>
	/// <param name="clientPt">Client coordinates as given e.g. by window messages</param>
	/// <returns>Logical map coordinates</returns>
	MapCoords GetMapCoordinatesFromClientCoordinates(const CPoint& clientPt, bool bAllowAccessBehindCliffs = false, bool ignoreHideFlags = false) const;

	/// <summary>
	/// Converts from Win32 window client coordinates (0/0 at the top left corner of the view window) to (world) pixel coordinates
	/// </summary>
	/// <param name="clientPt">Client coordinates as given e.g. by window messages</param>
	/// <returns>World pixel coordinates</returns>
	ProjectedCoords GetProjectedCoordinatesFromClientCoordinates(const CPoint& clientPt) const;

	/// <summary>
	/// Converts from logical map coordinates to (world) pixel coordinates
	/// </summary>
	/// <param name="mapCoords">Logical map coordinates</param>
	/// <returns>World pixel coordinates</returns>
	ProjectedCoords GetProjectedCoordinates(const MapCoords& mapCoords) const;

	/// <summary>
	/// Converts from logical map coordinates to (world) pixel coordinates
	/// </summary>
	/// <param name="mapCoords">Logical map coordinates</param>
	/// <param name="mapZ">Fixed logical height</param>
	/// <returns>World pixel coordinates</returns>
	ProjectedCoords GetProjectedCoordinates(const MapCoords& mapCoords, int mapZ) const;

	/// <summary>
	/// Converts from logical map coordinates to view / render target pixel coordinates (screen backbuffer, e.g. currently not yet scaled by view zoom)
	/// </summary>
	/// <param name="mapCoords">Logical map coordinates</param>
	/// <returns>Texel coordinates of the screen backbuffer</returns>
	ProjectedCoords GetRenderTargetCoordinates(const MapCoords& mapCoords) const;

	/// <summary>
	/// Converts from logical map coordinates to view / render target pixel coordinates (screen backbuffer, e.g. currently not yet scaled by view zoom)
	/// </summary>
	/// <param name="mapCoords">Logical map coordinates</param>
	/// <param name="mapZ">Fixed logical height</param>
	/// <returns>Texel coordinates of the screen backbuffer</returns>
	ProjectedCoords GetRenderTargetCoordinates(const MapCoords& mapCoords, int mapZ) const;

	/// <summary>
	/// Converts from logical map coordinates to Win32 window client coordinates (0/0 at the top left corner of the view window)
	/// </summary>
	/// <param name="projCoords">Logical map coordinates</param>
	/// <returns>Texel coordinates of the backbuffer render target</returns>
	CPoint GetClientCoordinates(const MapCoords& mapCoords) const;

	/// <summary>
	/// Converts from (world) pixel coordinates to Win32 window client coordinates (0/0 at the top left corner of the view window)
	/// </summary>
	/// <param name="projCoords">World pixel coordinates</param>
	/// <returns>Client coordinates as given e.g. by window messages</returns>
	CPoint GetClientCoordinatesFromWorld(const ProjectedCoords& projectedCoords) const;

	/// <summary>
	/// Applies the scaling as it is being done when blitting from back to frontbuffer, required e.g. for drawing calls directly to the frontbuffer
	/// If you use mouse-move coordinates you need to 
	/// </summary>
	/// <param name="projCoords">Texel coordinates of the backbuffer render target</param>
	/// <returns>Texel coordinates of the frontbuffer render target</returns>
	ProjectedCoords ScaleBackToFrontBuffer(const ProjectedCoords& backBufferCoords) const;

	void HandleTrail(int x, int y);
	int GetOverlayDirection(int x, int y);
	void SetError(const char* text);
	CWnd* owner;
	void ReInitializeDDraw();
	COLORREF GetColor(const char* house, const char* color=NULL);
	void Blit(LPDIRECTDRAWSURFACE4 pic, int x, int y, int width=-1, int height=-1)
	{
		if(pic==NULL) return;

		x+=1;
		y+=1;
		//y-=f_y;

		RECT r;
		GetDesktopWindow()->GetWindowRect(&r);
		//GetWindowRect(&r);

		if(width==-1 || height==-1)
		{
			DDSURFACEDESC2 ddsd;
			memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
			ddsd.dwSize=sizeof(DDSURFACEDESC2);
			ddsd.dwFlags=DDSD_WIDTH | DDSD_HEIGHT;
			pic->GetSurfaceDesc(&ddsd);
			width=ddsd.dwWidth;
			height=ddsd.dwHeight;
		}

		if(x+width<0 || y+height<0) return;
		if(x>r.right || y>r.bottom) return;

		if(x<0 || y<0 || x+width>=r.right || y+height>=r.bottom)
		{
			

			RECT blrect;
			RECT srcRect;
			srcRect.left=0;
			srcRect.top=0;
			srcRect.right=width;
			srcRect.bottom=height;
			blrect.left=x;
			if(blrect.left<0) 
			{
				srcRect.left=r.left+1-blrect.left;
				blrect.left=r.left+1;
			}
			blrect.top=y;
			if(blrect.top<0) 
			{
				
				//errstream << "BlRect.top=" << blrect.top << endl;
				srcRect.top=(r.top+1)-blrect.top; //(r.top-blrect.top);
				blrect.top=r.top+1;//r.top;
				// errstream << blrect.top << " " << srcRect.top << endl;
			}
			blrect.right=(x+width);
			if(x+width>r.right)
			{
				srcRect.right=width-((x+width)-r.right);// -(-blrect.right+r.right);
				blrect.right=r.right;
			}
			blrect.bottom=(y+height);
			if(y+height>r.bottom)
			{
				srcRect.bottom=height-((y+height)-r.bottom); // -(-blrect.bottom+r.bottom);
				blrect.bottom=r.bottom;
			}
			DDBLTFX fx;
			memset(&fx, 0, sizeof(DDBLTFX));
			fx.dwSize=sizeof(DDBLTFX);
			lpdsBack->Blt(&blrect, pic, &srcRect, DDBLT_KEYSRC, &fx);
		}
		else
			lpdsBack->BltFast(x,y, pic, NULL, DDBLTFAST_SRCCOLORKEY);
	}
	void updateFontScaled();
	void TextOut(int x, int y, const char* text, COLORREF col);
	void TextOut(HDC hDC, int x, int y, const char* text, COLORREF col);
	LPDIRECTDRAWSURFACE4 lpdsBack;
	LPDIRECTDRAWSURFACE4 lpdsTemp; // used for saving the isoview when drawing current tile
	LPDIRECTDRAWSURFACE4 lpdsBackHighRes; // used for rendering text and some lines in high-res
	LPDIRECTDRAWSURFACE4 lpds;
	DDPIXELFORMAT pf;	
	std::unique_ptr<FSunPackLib::ColorConverter> m_color_converter;
	LPDIRECTDRAW4 dd;
	LPDIRECTDRAW dd_1;
	HGLRC m_hglrc;
	void HandleProperties(int n, int type);
	void UpdateDialog(BOOL bRepos=TRUE);
	CMenu m_menu;
	BOOL b_IsLoading;
	int m_fontDefaultHeight;
	int m_Font9Height;
	std::unique_ptr<TextDrawer> m_textDefault;	
	std::unique_ptr<TextDrawer> m_textScaled;
	std::unique_ptr<TextDrawer> m_text9;
	std::unique_ptr<TextDrawer> m_text9Scaled;
	std::unique_ptr<TextDrawer> m_textBlue;
	std::unique_ptr<TextDrawer> m_textBlue9;
	std::unique_ptr<TextDrawer> m_textBlueScaled;
	std::unique_ptr<TextDrawer> m_textBlue9Scaled;

private:
	void UpdateScrollRanges();

private:
	RECT m_myRect;
	
	
	// mapdata* _map;
	COLORREF m_linecolor;
	RECT line;
	int m_type;
	int m_id;
	BOOL m_drag;
	BOOL m_moved;
	MapCoords m_cellCursor;
public:
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	void Zoom(CPoint& pt, float f);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}

#endif // AFX_ISOVIEW_H__7FB6D6A0_7B52_11D3_99E1_DA6DFD21E706__INCLUDED_
