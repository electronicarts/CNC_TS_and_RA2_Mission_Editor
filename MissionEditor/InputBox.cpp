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

// InputBox.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "InputBox.h"
#include "functions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/*
InputBox();

Shows a inputbox with specified caption and text.
Returns the user input. If the user cancels, the user input is "".
*/
CString InputBox(const char* Sentence, const char* Caption)
{
	CInputBox inp;
	inp.SetCaption(Caption);
	inp.SetSentence(Sentence);
	char* res=(char*) inp.DoModal();
	CString cstr=res;
	
	return cstr;
}

CInputBox::CInputBox(CWnd* pParent /*=NULL*/)
	: CDialog(CInputBox::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInputBox)
		
	//}}AFX_DATA_INIT
}


void CInputBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInputBox)
		
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInputBox, CDialog)
	//{{AFX_MSG_MAP(CInputBox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



void CInputBox::OnOK() 
{
	CString text;
	GetDlgItem(IDC_VAL)->GetWindowText(text);

	if(text.GetLength()==0){EndDialog(NULL);};

	char* str;
	str=new(char[text.GetLength()]);
	strcpy(str, (LPCTSTR)text);
	EndDialog((int)str);
}

void CInputBox::OnCancel() 
{
	EndDialog(NULL);
}

void CInputBox::SetCaption(CString Caption)
{
	m_Caption=Caption;
}

void CInputBox::SetSentence(CString Sentence)
{
	m_Text=Sentence;
}

BOOL CInputBox::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetWindowText(m_Caption);
	SetDlgItemText(IDC_SENTENCE, m_Text);

	SetDlgItemText(IDOK, GetLanguageStringACP("OK"));
	SetDlgItemText(IDCANCEL, GetLanguageStringACP("Cancel"));

	GetDlgItem(IDC_VAL)->SetFocus();
		
	return FALSE; 
}
