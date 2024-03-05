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

#if !defined(AFX_AITRIGGERTYPES_H__47F0A0E0_A8DC_11D3_B63B_B4F5BEE55940__INCLUDED_)
#define AFX_AITRIGGERTYPES_H__47F0A0E0_A8DC_11D3_B63B_B4F5BEE55940__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AITriggerTypes.h : Header file
//

#include "structs.h"

/////////////////////////////////////////////////////////////////////////////
// dialog field CAITriggerTypes 

class CAITriggerTypes : public CDialog
{
	DECLARE_DYNCREATE(CAITriggerTypes)

// construction
public:
	char* ConvertToHexFromAITrigInfo(AITrigInfo info, char* buffer);
	void SetAITriggerParam(const char* value, int param);
	void UpdateDialog();
	CAITriggerTypes();
	~CAITriggerTypes();

// dialog field data
	//{{AFX_DATA(CAITriggerTypes)
	enum { IDD = IDD_AITRIGGERTYPES };
	CMyComboBox	m_TeamType1;
	CMyComboBox	m_UnitType;
	CMyComboBox	m_TeamType2;
	CMyComboBox	m_Flag2;
	CMyComboBox	m_Flag1;
	CMyComboBox	m_Data;
	CMyComboBox	m_Owner;
	CFloatEdit	m_Float3;
	CFloatEdit	m_Float2;
	CFloatEdit	m_Float1;
	CMyComboBox	m_AITriggerType;
	CString	m_Flag3;
	CString	m_Flag4;
	CString	m_Name;
	CString	m_Flag6;
	CString	m_Flag7;
	CString	m_Flag8;
	CString	m_Flag9;
	BOOL	m_Enabled;
	int		m_Condition;
	int		m_Number;
	BOOL	m_Easy;
	BOOL	m_Medium;
	BOOL	m_Hard;
	BOOL	m_BaseDefense;
	BOOL	m_Skirmish;
	CString	m_Flag5;
	CString	m_MultiSide;
	//}}AFX_DATA


// overwriteables
	// generated virtual overwriteables
	//{{AFX_VIRTUAL(CAITriggerTypes)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// implementation
protected:
	AITrigInfo ConvertToAITrigInfoFromHex(char* aitinfo);
	// generated message maps
	//{{AFX_MSG(CAITriggerTypes)
	afx_msg void OnSelchangeAitriggertype();
	afx_msg void OnChangeName();
	afx_msg void OnEditchangeOwner();
	afx_msg void OnEditchangeTeamtype1();
	afx_msg void OnEditchangeTeamtype2();
	afx_msg void OnEditchangeFlag1();
	afx_msg void OnEditchangeFlag2();
	afx_msg void OnEditchangeUnittype();
	afx_msg void OnEditchangeData();
	afx_msg void OnChangeFloat1();
	afx_msg void OnChangeFloat2();
	afx_msg void OnChangeFloat3();
	afx_msg void OnChangeFlag3();
	afx_msg void OnChangeFlag4();
	afx_msg void OnChangeFlag5();
	afx_msg void OnChangeFlag6();
	afx_msg void OnChangeFlag7();
	afx_msg void OnChangeFlag8();
	afx_msg void OnChangeFlag9();
	afx_msg void OnEnabled();
	afx_msg void OnAdd();
	afx_msg void OnDelete();
	afx_msg void OnEditchangeAitriggertype();
	afx_msg void OnSelchangeCondition();
	afx_msg void OnChangeNumber();
	afx_msg void OnEasy();
	afx_msg void OnMedium();
	afx_msg void OnHard();
	afx_msg void OnBasedefense();
	afx_msg void OnSkirmish();
	afx_msg void OnEditchangeMultiside();
	afx_msg void OnSelchangeMultiside();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}

#endif // AFX_AITRIGGERTYPES_H__47F0A0E0_A8DC_11D3_B63B_B4F5BEE55940__INCLUDED_
