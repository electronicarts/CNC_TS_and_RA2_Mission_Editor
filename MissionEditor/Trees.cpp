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

// Trees.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "TiberianSun Mission Editor.h"
#include "Trees.h"
#include "pos.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Eigenschaftenseite CTrees 

IMPLEMENT_DYNCREATE(CTrees, CPropertyPage)

CTrees::CTrees() : CPropertyPage(CTrees::IDD)
{
	//{{AFX_DATA_INIT(CTrees)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}

CTrees::~CTrees()
{
}

void CTrees::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTrees)
	DDX_Control(pDX, IDC_POS, m_Pos);
	DDX_Control(pDX, IDC_TYPE, m_Type);
	DDX_Control(pDX, IDC_TREELIST, m_TreeList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTrees, CPropertyPage)
	//{{AFX_MSG_MAP(CTrees)
	ON_LBN_SELCHANGE(IDC_TREELIST, OnSelchangeTreelist)
	ON_CBN_EDITCHANGE(IDC_TYPE, OnEditchangeType)
	ON_CBN_KILLFOCUS(IDC_TYPE, OnKillfocusType)
	ON_WM_KILLFOCUS()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_CBN_SELCHANGE(IDC_TYPE, OnEditchangeType)
	ON_BN_CLICKED(IDC_NEW, OnNew)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CTrees 

void CTrees::UpdateDialog()
{
	m_TreeList.SetRedraw(FALSE);

	// first clear the list
	while(m_TreeList.DeleteString(0)!=LB_ERR);

	// okay add all trees

	int i;
	CIniFileSection& sec=ini.sections["Terrain"];
	
	
	for(i=0;i<sec.values.size();i++)
	{
		CString str;
		str=sec.GetValueName(i)->data();

		int pos=atoi(str);

		str+=(CString)" "+ sec.GetValue(i)->data();
		int x,y,z;
		GetXYPos((char*)sec.GetValueName(i)->data(), &x, &y);
		char c[50];
		itoa(x, c, 10);
		str+=" (";
		str+=c;
		str+="/";
		itoa(y, c, 10);
		str+=c;
		
		z=GetPos(x,y,0);

		itoa(z, c, 10);
		str+="/";
		str+=c;
		str+=")";

		m_TreeList.InsertString(-1, str);

	}

	m_TreeList.SetRedraw(TRUE);
	m_TreeList.RedrawWindow();
}

BOOL CTrees::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Zusätzliche Initialisierung hier einfügen
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CTrees::OnSelchangeTreelist() 
{
	int i=m_TreeList.GetCurSel();

	CString str;
	m_TreeList.GetText(i, str);

	str.SetAt(str.Find(" ", 0),0);

	m_Type.SetWindowText( ini.sections["Terrain"].values[ (char*)(LPCTSTR)str ].data() );

	string pos;
	int x,y,z;
	GetXYPos((char*)ini.sections["Terrain"].GetValueName(i)->data(), &x, &y);

	char c[50];
	itoa(x, c, 10);
	pos=c;
	pos+="/";
	itoa(y, c, 10);
	pos+=c;
	pos+="/";
	itoa(z, c, 10);
	pos+=c;

	m_Pos.SetWindowText(pos.data());

}

void CTrees::OnEditchangeType() 
{
	

	
}

void CTrees::OnKillfocusType() 
{
	int i=m_TreeList.GetCurSel();

	if(i==-1) return;

	m_TreeList.SetRedraw(FALSE);

	CString str;
	m_TreeList.GetText(i, str);

	str.SetAt(str.Find(" ", 0),0);
		

	CString type;
	m_Type.GetWindowText(type);

	if(ini.sections["Terrain"].values[(char*)(LPCTSTR)str]==(char*)(LPCTSTR)type) return;

	ini.sections["Terrain"].values[(char*)(LPCTSTR)str]=type;

	UpdateDialog();

	m_TreeList.SetCurSel(i);
	CTrees::OnSelchangeTreelist();

	/*String str_=str;
		//str_=ini.sections["Terrain"].GetValueName(i)->data();

		//int pos=atoi(str_);

		str_+=(CString)" "+ ini.sections["Terrain"].values[(char*)(LPCTSTR)str].data();
		int x,y;
		GetXYPos(atoi(str), x, y);
		char c[50];
		itoa(x, c, 10);
		str_+=" (";
		str_+=c;
		str_+="/";
		itoa(y, c, 10);
		str_+=c;
		str_+=")";
		
		m_TreeList.DeleteString(i);
		m_TreeList.InsertString(i, str_);*/

	

	m_TreeList.SetRedraw(TRUE);
	m_TreeList.RedrawWindow();
	m_TreeList.SetCurSel(i);
}

void CTrees::OnKillFocus(CWnd* pNewWnd) 
{
	CPropertyPage::OnKillFocus(pNewWnd);
	
	
	
	
}

void CTrees::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
	OnKillfocusType();	
}

void CTrees::OnDelete() 
{
	int pos=m_TreeList.GetCurSel();
	if(pos==-1) return;
	
	CString cutree;
	//m_TreeList.GetText(pos, cutree);
	cutree=ini.sections["Terrain"].GetValueName(pos)->data();

	ini.sections["Terrain"].values.erase((string)(char*)(LPCTSTR) cutree);

	//m_TreeList.SetRedraw(FALSE);
	UpdateDialog();
	m_TreeList.SetCurSel(pos);
	//m_TreeList.SetRedraw(TRUE);
	

}

void CTrees::OnNew() 
{
	int pos=m_TreeList.GetCurSel(); 

	CPos p;
	if(p.DoModal()==IDCANCEL) return;

	int h=GetPos(atoi(p.m_x), atoi(p.m_y), 0);	

	char k[50];
	itoa(h, k, 10);

	ini.sections["Terrain"].values[k]="TREE01";

	UpdateDialog();
	m_TreeList.SetCurSel(pos);
	
}
