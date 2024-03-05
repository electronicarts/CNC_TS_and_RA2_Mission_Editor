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

// RTPDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "RTPDlg.h"
#include "variables.h"
#include "functions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CRTPDlg 


CRTPDlg::CRTPDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRTPDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRTPDlg)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
}


void CRTPDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRTPDlg)
	DDX_Control(pDX, IDC_PREVIEW, m_Preview);
	DDX_Control(pDX, IDC_USED, m_Used);
	DDX_Control(pDX, IDC_AVAIL, m_Available);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRTPDlg, CDialog)
	//{{AFX_MSG_MAP(CRTPDlg)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_LBN_SELCHANGE(IDC_AVAIL, OnSelchangeAvail)
	ON_LBN_DBLCLK(IDC_AVAIL, OnDblclkAvail)
	ON_LBN_SELCHANGE(IDC_USED, OnSelchangeUsed)
	ON_LBN_DBLCLK(IDC_USED, OnDblclkUsed)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CRTPDlg 

BOOL CRTPDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	int i;
	for(i=0;i<rules.sections["TerrainTypes"].values.size();i++)
	{
		CString unitname=*rules.sections["TerrainTypes"].GetValue(i);
		CString addedString=unitname;

		if(g_data.sections["IgnoreRA2"].FindValue(unitname)>=0) continue;
	
		addedString=TranslateStringACP(addedString);
			
				
		if(unitname.Find("TREE")>=0) 
		{
			
						
			if(unitname.GetLength()>0 && unitname!="VEINTREE") // out with it :-)
			{
				int TreeMin=atoi(g_data.sections[Map->GetTheater()+"Limits"].values["TreeMin"]);
				int TreeMax=atoi(g_data.sections[Map->GetTheater()+"Limits"].values["TreeMax"]);
				
				CString id=unitname;
				id.Delete(0, 4);
				int n=atoi(id);

				
				if(n<TreeMin || n>TreeMax) continue;

				m_Available.AddString(unitname);			
			}
		}
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CRTPDlg::OnAdd() 
{
	int sel=m_Available.GetCurSel();
	if(sel<0) return;

	CString currentname;
	m_Available.GetText(sel, currentname);

	m_Available.DeleteString(sel);
	m_Used.AddString(currentname);
}

void CRTPDlg::OnRemove() 
{
	int sel=m_Used.GetCurSel();
	if(sel<0) return;

	CString currentname;
	m_Used.GetText(sel, currentname);

	m_Used.DeleteString(sel);
	m_Available.AddString(currentname);
}

void CRTPDlg::OnOK() 
{
	if(m_Used.GetCount()<=0)
	{
		//AfxMessageBox("Please select at least one tree","Error", MB_OK);
		return;
	}

	rndterrainsrc.clear();

	int i;
	for(i=0;i<m_Used.GetCount();i++)
	{
		CString str;
		m_Used.GetText(i, str);
		rndterrainsrc.push_back(str);
	}
	
	CDialog::OnOK();
}

void CRTPDlg::OnSelchangeAvail() 
{
	int sel=m_Available.GetCurSel();

	if(sel<0)
	{
		m_LastSelected="";
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		return;
	}

	CString s;
	m_Available.GetText(sel, s);
	m_LastSelected=s;
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	
}

void CRTPDlg::OnDblclkAvail() 
{
	int sel=m_Available.GetCurSel();
	if(sel<0) return;

	CString currentname;
	m_Available.GetText(sel, currentname);

	m_Available.DeleteString(sel);
	m_Used.AddString(currentname);
}

void CRTPDlg::OnSelchangeUsed() 
{
	int sel=m_Used.GetCurSel();

	if(sel<0)
	{
		m_LastSelected="";
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		return;
	}

	CString s;
	m_Used.GetText(sel, s);
	m_LastSelected=s;
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	
}

void CRTPDlg::OnDblclkUsed() 
{
	int sel=m_Used.GetCurSel();
	if(sel<0) return;

	CString currentname;
	m_Used.GetText(sel, currentname);

	m_Used.DeleteString(sel);
	m_Available.AddString(currentname);
}

void CRTPDlg::OnPaint() 
{
	
	if(m_LastSelected=="") return;

	CPaintDC dc(this); // device context for painting
	
	int id=Map->GetUnitTypeID(m_LastSelected);
	
	PICDATA* p=&treeinfo[id].pic; //ovrlpics[m_currentOverlay][i];
			

	if(p->pic==NULL)
	{
		CString type;
		type=m_LastSelected;

		if(missingimages.find(type)==missingimages.end())
		{			
			theApp.m_loading->LoadUnitGraphic(type);
			Map->UpdateTreeInfo(type);
			p=&treeinfo[id].pic;
		}
		if(p->pic==NULL)
		{
		  missingimages[type]=TRUE;
		}
	}

	int curwidth=p->wMaxWidth;
	int curheight=p->wMaxHeight;

	BITMAPINFO biinfo;
	memset(&biinfo, 0, sizeof(BITMAPINFO));
	biinfo.bmiHeader.biBitCount=24;
	biinfo.bmiHeader.biWidth=curwidth;
	biinfo.bmiHeader.biHeight=curheight;
	biinfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	biinfo.bmiHeader.biClrUsed=0;
	biinfo.bmiHeader.biPlanes=1;
	biinfo.bmiHeader.biCompression=BI_RGB;
	biinfo.bmiHeader.biClrImportant=0;


	int pitch=curwidth*3;
	if(pitch==0) return;

	if(pitch%sizeof(DWORD))
	{
		pitch+=sizeof(DWORD)-(curwidth*3)%sizeof(DWORD);
	}

	BYTE* colors=new(BYTE[pitch*curheight]);
	memset(colors, 255, pitch*(curheight));

	RGBTRIPLE* pal=palIso;
#ifdef NOSURFACES
	if(p->pal==iPalTheater) pal=palTheater;
	if(p->pal==iPalUnit) pal=palUnit;
#endif

	int k,l;
	for(k=0;k<curheight;k++)
	{
		for(l=0;l<curwidth;l++)
		{
			if(((BYTE*)p->pic)[l+k*curwidth])
			{
				memcpy(&colors[l*3+(curheight-k-1)*pitch], &pal[((BYTE*)p->pic)[l+k*curwidth]], 3);
			}
		}
	}

	RECT pr;
	m_Preview.GetWindowRect(&pr);
	this->ScreenToClient(&pr);

	pr.top += 15;
	pr.bottom -= 5;
	pr.left += 5;
	pr.right -= 5;

	CBrush f;
	f.Attach(GetStockObject(WHITE_BRUSH));
	dc.FillRect(&pr,&f);
	f.Detach();

	auto clientDC = m_Preview.GetWindowDC();
	StretchDIBits(clientDC->GetSafeHdc(), 20, 30,curwidth, curheight,
		0, 0, curwidth, curheight, colors, &biinfo, DIB_RGB_COLORS, SRCCOPY);

	delete[] colors;
	
	// Kein Aufruf von CDialog::OnPaint() für Zeichnungsnachrichten
}
