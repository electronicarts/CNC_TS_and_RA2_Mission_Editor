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

// Waypoints.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "TiberianSun Mission Editor.h"
#include "Waypoints.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CWaypoints 

IMPLEMENT_DYNCREATE(CWaypoints, CPropertyPage)

CWaypoints::CWaypoints() : CPropertyPage(CWaypoints::IDD)
{
	//{{AFX_DATA_INIT(CWaypoints)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}

CWaypoints::~CWaypoints()
{
}

void CWaypoints::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWaypoints)
	DDX_Control(pDX, IDC_DBG, m_dbg);
	DDX_Control(pDX, IDC_WAYPOINTS, m_Waypoints);
	DDX_Control(pDX, IDC_POS, m_Pos);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWaypoints, CPropertyPage)
	//{{AFX_MSG_MAP(CWaypoints)
	ON_LBN_SELCHANGE(IDC_WAYPOINTS, OnSelchangeWaypoints)
	ON_EN_KILLFOCUS(IDC_POS, OnKillfocusPos)
	ON_WM_KILLFOCUS()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CWaypoints 

void CWaypoints::UpdateDialog()
{
	m_Waypoints.SetRedraw(FALSE);

	// first clear the list
	while(m_Waypoints.DeleteString(0)!=LB_ERR);

	// okay add all trees

	int i;
	CIniFileSection& sec=ini.sections["Waypoints"];
	
	char c[50];
	for(i=0;i<sec.values.size();i++)
	{
		CString str;
		str=sec.GetValueName(i)->data();

		
		str+=", ";
		str+=sec.GetValue(i)->data();

		int x,y,z;
		GetXYPos((char*)sec.GetValue(i)->data(), &x, &y);

		
		itoa(x, c,10);
		str+=", ";
		str+=c;
		itoa(y, c,10);
		str+="/";
		str+=c;
		
		z=GetPos(x, y, 0);
		itoa(z, c, 10);
		str+="/";
		str+=c;

		/*int pos=atoi(str);

		str+=(CString)" "+ sec.GetValue(i)->data();
		int x,y;
		GetXYPos(atoi(sec.GetValueName(i)->data()), x, y);
		char c[50];
		itoa(x, c, 10);
		str+=" (";
		str+=c;
		str+="/";
		itoa(y, c, 10);
		str+=c;
		str+=")";*/
		
		m_Waypoints.InsertString(-1, str);

	}

	m_Waypoints.SetRedraw(TRUE);

}

void CWaypoints::OnSelchangeWaypoints() 
{
	int i=m_Waypoints.GetCurSel();
	if(i==-1) return;

	CString str;
	m_Waypoints.GetText(i, str);

	str.SetAt(str.Find(",",0), 0);

	// ok str now specifies the waypoint id
	m_Pos.SetWindowText(ini.sections["Waypoints"].values[(char*)(LPCTSTR)str].data());
}

void CWaypoints::OnKillfocusPos() 
{
	int i=m_Waypoints.GetCurSel();
	if(i==-1) return;

	CString str;
	m_Waypoints.GetText(i, str);

	str.SetAt(str.Find(",",0), 0);

	// ok str now specifies the waypoint id
	ini.sections["Waypoints"].values[(char*)(LPCTSTR)str]=GetText(&m_Pos);

	UpdateDialog();
	m_Waypoints.SetCurSel(i);
}

void CWaypoints::OnKillFocus(CWnd* pNewWnd) 
{
	CPropertyPage::OnKillFocus(pNewWnd);
	
	
}

void CWaypoints::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
	OnKillfocusPos();	
}

void CWaypoints::OnDelete() 
{
	int pos=m_Waypoints.GetCurSel();
	if(pos==-1) return;
	
	CString cuwayp;
	//m_TreeList.GetText(pos, cutree);
	cuwayp=ini.sections["Waypoints"].GetValueName(pos)->data();

	ini.sections["Waypoints"].values.erase((string)(char*)(LPCTSTR) cuwayp);

	m_Waypoints.SetRedraw(FALSE);
	UpdateDialog();
	m_Waypoints.SetCurSel(pos);
	m_Waypoints.SetRedraw(TRUE);	
}

BOOL CWaypoints::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	CBitmap m;
	m.LoadBitmap(IDB_DBG);
	m_dbg.SetBitmap(m);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}
