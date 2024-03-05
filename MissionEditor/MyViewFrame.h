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

#if !defined(AFX_MYVIEWFRAME_H__98929AE1_9B75_11D3_B63B_D046A7216340__INCLUDED_)
#define AFX_MYVIEWFRAME_H__98929AE1_9B75_11D3_B63B_D046A7216340__INCLUDED_

#include "IsoView.h"
	
#if _MSC_VER > 1000
#pragma once
#endif 

// MyViewFrame.h : Header file
//

#include "isoview.h"
#include "viewobjects.h"
#include "MiniMap.h"
#include "RightFrame.h"	
#include "TileSetBrowserFrame.h"

/////////////////////////////////////////////////////////////////////////////
// Frame CMyViewFrame 

class CMyViewFrame : public CFrameWnd
{
	DECLARE_DYNCREATE(CMyViewFrame)
protected:
	

// attributes
public:
	CMiniMap m_minimap;
	CStatusBar m_statbar;
	CViewObjects* m_objectview;
	CIsoView* m_isoview;
	CSplitterWnd m_Splitter;

// operations
public:
	CTileSetBrowserFrame* m_browser;
	CRightFrame* m_rightFrame;
	CMyViewFrame();           
	~CMyViewFrame();

// overwriteables
	
	//{{AFX_VIRTUAL(CMyViewFrame)
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// implementation
protected:
	
	// generated message maps
	//{{AFX_MSG(CMyViewFrame)
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}

#endif // AFX_MYVIEWFRAME_H__98929AE1_9B75_11D3_B63B_D046A7216340__INCLUDED_
