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

#include "stdafx.h"
#include "resource.h"
#include "mapdata.h"
#include "variables.h"
#include "functions.h"

#include <winreg.h>
#include <sys\stat.h>
#include <sys\types.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTipDlg dialog field

#define MAX_BUFLEN 1000

static const TCHAR szSection[] = _T("Tip");
static const TCHAR szIntFilePos[] = _T("FilePos");
static const TCHAR szTimeStamp[] = _T("TimeStamp");
static const TCHAR szIntStartup[] = _T("StartUp");

extern char AppPath[];

CTipDlg::CTipDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_TIP, pParent)
{
	//{{AFX_DATA_INIT(CTipDlg)
	m_bStartup = TRUE;
	//}}AFX_DATA_INIT

	CIniFile optini;
	CString iniFile;
		
	iniFile=AppPath;
#ifndef RA2_MODE
	iniFile+="\\FinalSun.ini";
#else
	iniFile+="\\FinalAlert.ini";
#endif
	optini.LoadFile(iniFile);

	CWinApp* pApp = AfxGetApp();
	m_bStartup = !atoi(optini.sections[szSection].values[szIntStartup]);
	UINT iFilePos = atoi(optini.sections[szSection].values[szIntFilePos]);

	// try top open the tips file
	CString tipsfile=AppPath;
	tipsfile+="\\tips.";
	tipsfile+=language.sections[theApp.m_Options.LanguageName+"Header"].values["ExtensionName"];

	m_pStream = fopen(tipsfile, "r");
	if (m_pStream == NULL) 
	{
		m_strTip=GetLanguageStringACP("CG_IDS_FILE_ABSENT");
		return;
	} 

	//now check if the tips file is changed! (check the date the tips file was created)
	struct _stat buf;
	_fstat(_fileno(m_pStream), &buf);
	CString strCurrentTime = ctime(&buf.st_ctime);
	strCurrentTime.TrimRight();
	CString strStoredTime = optini.sections[szSection].values[szTimeStamp];
	if (strCurrentTime != strStoredTime) 
	{
		iFilePos = 0;
		optini.sections[szSection].values[szTimeStamp]=(LPCTSTR)strCurrentTime;

	}

	if (fseek(m_pStream, iFilePos, SEEK_SET) != 0) 
	{
		AfxMessageBox(GetLanguageStringACP("CG_IDP_FILE_CORRUPT"));
	}
	else 
	{
		GetNextTipString(m_strTip);
	}

	optini.SaveFile(iniFile);
}

CTipDlg::~CTipDlg()
{
	if (m_pStream != NULL) 
	{

		CIniFile optini;
		CString iniFile;
		
		iniFile=AppPath;
#ifndef RA2_MODE
		iniFile+="\\FinalSun.ini";
#else
		iniFile+="\\FinalAlert.ini";
#endif
		optini.LoadFile(iniFile);

		char val[50];
		itoa(ftell(m_pStream),val, 10);
		optini.sections[szSection].values[szIntFilePos]=val;
		optini.SaveFile(iniFile);
		fclose(m_pStream);
	}
}
        
void CTipDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTipDlg)
	DDX_Check(pDX, IDC_STARTUP, m_bStartup);
	DDX_Text(pDX, IDC_TIPSTRING, m_strTip);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTipDlg, CDialog)
	//{{AFX_MSG_MAP(CTipDlg)
	ON_BN_CLICKED(IDC_NEXTTIP, OnNextTip)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTipDlg - handlers

void CTipDlg::OnNextTip()
{
	GetNextTipString(m_strTip);
	UpdateData(FALSE);
}

void CTipDlg::GetNextTipString(CString& strNext)
{
	LPTSTR lpsz = strNext.GetBuffer(MAX_BUFLEN);

	BOOL bStop = FALSE;
	while (!bStop) 
	{
		if (_fgetts(lpsz, MAX_BUFLEN, m_pStream) == NULL) 
		{
			if (fseek(m_pStream, 0, SEEK_SET) != 0) 
				AfxMessageBox(GetLanguageStringACP("CG_IDP_FILE_CORRUPT"));
		} 
		else 
		{
			if (*lpsz != ' ' && *lpsz != '\t' && 
				*lpsz != '\n' && *lpsz != ';') 
			{
				bStop = TRUE;
			}
		}
	}
	strNext.ReleaseBuffer();
}

HBRUSH CTipDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (pWnd->GetDlgCtrlID() == IDC_TIPSTRING)
		return (HBRUSH)GetStockObject(WHITE_BRUSH);

	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CTipDlg::OnOK()
{
	CDialog::OnOK();
	
    // actualize startup
	CIniFile optini;
	CString iniFile;
		
	iniFile=AppPath;
#ifndef RA2_MODE
	iniFile+="\\FinalSun.ini";
#else
	iniFile+="\\FinalAlert.ini";
#endif

	optini.LoadFile(iniFile);

	char val[50];
	itoa(!m_bStartup,val, 10);
	optini.sections[szSection].values[szIntStartup]=val;
	optini.SaveFile(iniFile);
}

BOOL CTipDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// deactivate next tip if no tips are present
	if (m_pStream == NULL)
		GetDlgItem(IDC_NEXTTIP)->EnableWindow(FALSE);

	SetDlgItemText(IDC_STARTUP, GetLanguageStringACP("TipDialogShowAtStartup"));
	SetDlgItemText(IDC_NEXTTIP, GetLanguageStringACP("TipDialogNext"));
	SetDlgItemText(IDOK, GetLanguageStringACP("TipDialogClose"));
	SetWindowText(GetLanguageStringACP("TipDialogCaption"));

	return TRUE;  
}

void CTipDlg::OnPaint()
{
	CPaintDC dc(this);

	// prepare draw area for the big static rectangle
	CWnd* pStatic = GetDlgItem(IDC_TOOLTIPCENTER);
	CRect rect;
	pStatic->GetWindowRect(&rect);
	ScreenToClient(&rect);

	// draw white background
	CBrush brush;
	brush.CreateStockObject(WHITE_BRUSH);
	dc.FillRect(rect, &brush);

	// load bitmap
	CBitmap bmp;
	bmp.LoadBitmap(IDB_LIGHTBULB);
	BITMAP bmpInfo;
	bmp.GetBitmap(&bmpInfo);

	// draw bitmap in the left corner
	CDC dcTmp;
	dcTmp.CreateCompatibleDC(&dc);
	dcTmp.SelectObject(&bmp);
	rect.bottom = bmpInfo.bmHeight + rect.top;
	dc.BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), 
		&dcTmp, 0, 0, SRCCOPY);

	// draw "Did you know" in the correct language
	CString strMessage;
	strMessage=GetLanguageStringACP("CG_IDS_DIDYOUKNOW");
	rect.left += bmpInfo.bmWidth;
	dc.DrawText(strMessage, rect, DT_VCENTER | DT_SINGLELINE);

	
}

