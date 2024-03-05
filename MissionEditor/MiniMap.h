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

#if !defined(AFX_MINIMAP_H__43F7CCE0_CC11_11D3_B63B_00485453E8BA__INCLUDED_)
#define AFX_MINIMAP_H__43F7CCE0_CC11_11D3_B63B_00485453E8BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MiniMap.h : Header file
//

/////////////////////////////////////////////////////////////////////////////
// View CMiniMap 

class CMiniMap : public CView
{
public:
	CMiniMap();  
	virtual ~CMiniMap();
	DECLARE_DYNCREATE(CMiniMap)

// attributes
public:

// operations
public:
	void DrawMinimap(BYTE** lpData, BITMAPINFO& biinfo, int& Pitch);
	
	void UpdateView();

// overwriteables
	//{{AFX_VIRTUAL(CMiniMap)
	protected:
	virtual void OnDraw(CDC* pDC);      
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementierung
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	
	
	// generated message maps
protected:
	//{{AFX_MSG(CMiniMap)
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	float m_scale;
public:
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}

#endif // AFX_MINIMAP_H__43F7CCE0_CC11_11D3_B63B_00485453E8BA__INCLUDED_
