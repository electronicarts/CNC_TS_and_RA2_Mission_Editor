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

// ProgressDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "finalsun.h"
#include "ProgressDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CProgressDlg 


CProgressDlg::CProgressDlg(CString lpDescription, CWnd* pParent /*=NULL*/)
	: CDialog(CProgressDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProgressDlg)
	m_Label = lpDescription;
	m_ProgLabel = _T("");
	//}}AFX_DATA_INIT

	m_bCancel=FALSE;

	Create(CProgressDlg::IDD, pParent);
}


void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProgressDlg)
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	DDX_Text(pDX, IDC_LABEL, m_Label);
	DDX_Text(pDX, IDC_PROGLABEL, m_ProgLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
	//{{AFX_MSG_MAP(CProgressDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CProgressDlg 

void CProgressDlg::SetRange(int min, int max)
{
	m_Progress.SetRange32(min, max);
}

void CProgressDlg::SetPosition(int nPos)
{
	m_Progress.SetPos(nPos);
	m_Progress.UpdateWindow();
	m_ProgLabel="Progress: ";
	
	

	float p=m_Progress.GetPos();
	int min,max;
	m_Progress.GetRange(min,max);
	
	if(max-min==0) return;

	float f=p/((float)(max-min));
	char c[50];
	itoa(f*100, c, 10);
	m_ProgLabel+=c;
	m_ProgLabel+=" %";

	UpdateData(FALSE);
}

void CProgressDlg::PostNcDestroy() 
{
	delete this;
	
	//CDialog::PostNcDestroy();
}

void CProgressDlg::OnCancel() 
{
	m_bCancel=TRUE;
	
	CDialog::OnCancel();
}
