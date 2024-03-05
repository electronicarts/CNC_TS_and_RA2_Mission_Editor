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

#if !defined(AFX_SINGLEPLAYERSETTINGS_H__082D6780_CA89_11D3_B63B_00485453E8BA__INCLUDED_)
#define AFX_SINGLEPLAYERSETTINGS_H__082D6780_CA89_11D3_B63B_00485453E8BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SingleplayerSettings.h : Header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// dialog field CSingleplayerSettings 

class CSingleplayerSettings : public CDialog
{
	DECLARE_DYNCREATE(CSingleplayerSettings)

// construction
public:
	void UpdateStrings();
	void UpdateDialog();
	CSingleplayerSettings();
	~CSingleplayerSettings();

// dialog field data
	//{{AFX_DATA(CSingleplayerSettings)
	enum { IDD = IDD_SINGLEPLAYER };
	CMyComboBox	m_TimerInherit;
	CMyComboBox	m_StartingDropships;
	CMyComboBox	m_FillSilos;
	CFloatEdit	m_CarryOverMoney;
	CMyComboBox	m_Win;
	CMyComboBox	m_PreMapSelect;
	CMyComboBox	m_PostScore;
	CMyComboBox	m_Lose;
	CMyComboBox	m_Intro;
	CMyComboBox	m_Brief;
	CMyComboBox	m_Action;
	//}}AFX_DATA


// overwriteables
	//{{AFX_VIRTUAL(CSingleplayerSettings)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // ddx/ddv support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// implementation
protected:
	// generated message maps
	//{{AFX_MSG(CSingleplayerSettings)
	afx_msg void OnEditchangeIntro();
	afx_msg void OnEditchangeBrief();
	afx_msg void OnEditchangeWin();
	afx_msg void OnEditchangeLose();
	afx_msg void OnEditchangeAction();
	afx_msg void OnEditchangePostscore();
	afx_msg void OnEditchangePremapselect();
	afx_msg void OnEditchangeStartingdropships();
	afx_msg void OnChangeCarryovermoney();
	afx_msg void OnEditchangeTimerinherit();
	afx_msg void OnEditchangeFillsilos();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // AFX_SINGLEPLAYERSETTINGS_H__082D6780_CA89_11D3_B63B_00485453E8BA__INCLUDED_
