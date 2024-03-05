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

#if !defined(AFX_MAPVALIDATOR_H__B290D180_F761_11D3_B63B_00485453E8BA__INCLUDED_)
#define AFX_MAPVALIDATOR_H__B290D180_F761_11D3_B63B_00485453E8BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MapValidator.h : Header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// dialog field CMapValidator 

class CMapValidator : public CDialog
{
// construction
public:
	CMapValidator(CWnd* pParent = NULL);   // standard constructor

// dialog field data
	//{{AFX_DATA(CMapValidator)
	enum { IDD = IDD_MAPVALIDATOR };
	
	//}}AFX_DATA


// overwriteables
	//{{AFX_VIRTUAL(CMapValidator)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// implementation
protected:

	// generated message maps
	//{{AFX_MSG(CMapValidator)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	CImageList m_ProblemImages;
	CListCtrl m_MapProblemList;
	void UpdateStrings();
	BOOL CheckMap();
};

//{{AFX_INSERT_LOCATION}}

#endif // AFX_MAPVALIDATOR_H__B290D180_F761_11D3_B63B_00485453E8BA__INCLUDED_

CString TranslateTubeString(const char* error_, const CTube& tube, int count);
