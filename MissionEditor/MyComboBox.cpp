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

// MyComboBox.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "MyComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMyComboBox

CMyComboBox::CMyComboBox()
{
}

CMyComboBox::~CMyComboBox()
{
}


BEGIN_MESSAGE_MAP(CMyComboBox, CComboBox)
	//{{AFX_MSG_MAP(CMyComboBox)
	ON_CONTROL_REFLECT(CBN_SELCHANGE, OnSelchange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CMyComboBox 


/*
This will automatically send a CBN_EDITCHANGE command.
Also it will set the window text to the selected item
*/
void CMyComboBox::OnSelchange() 
{
	CString SelectedItem;
	GetLBText(GetCurSel(),SelectedItem);

	
	SetWindowText(SelectedItem);
	
	WPARAM wparam;
	int ctrlid=GetDlgCtrlID();
	WORD command=CBN_EDITCHANGE;
	memcpy((BYTE*)&wparam+2, &command, 2);
	memcpy((BYTE*)&wparam, &ctrlid, 2);
	GetOwner()->SendMessage(WM_COMMAND,wparam,(LPARAM)m_hWnd);
	
}
