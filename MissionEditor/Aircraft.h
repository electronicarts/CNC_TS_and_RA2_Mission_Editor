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

#if !defined(AFX_AIRCRAFT_H__15BD6160_8953_11D3_B63B_A583BFBD8C41__INCLUDED_)
#define AFX_AIRCRAFT_H__15BD6160_8953_11D3_B63B_A583BFBD8C41__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Aircraft.h : Header-Datei
//


class CAircraft : public CDialog
{
// construction
public:
	void UpdateStrings();
	CString m_strength;
	void Init(CString house="", CString strength="256", CString direction="64", CString action="Guard", CString tag="None", CString flag1="0", CString flag2="0", CString flag3="0", CString flag4="0");
	CAircraft(CWnd* pParent = NULL);   // standard constructor

// dialog field data
	//{{AFX_DATA(CAircraft)
	enum { IDD = IDD_AIRCRAFT };
	CSliderCtrl	m_strength_ctrl;
	CString	m_direction;
	CString	m_house;
	CString	m_flag1;
	CString	m_flag2;
	CString	m_flag3;
	CString	m_flag4;
	CString	m_action;
	CString	m_tag;
	//}}AFX_DATA


// overwriteables
	// generated from class assistant
	//{{AFX_VIRTUAL(CAircraft)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// implementation
protected:

	// generated message maps
	//{{AFX_MSG(CAircraft)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}

#endif // AFX_AIRCRAFT_H__15BD6160_8953_11D3_B63B_A583BFBD8C41__INCLUDED_
