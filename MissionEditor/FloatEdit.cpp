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

//
// FloatEdit.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "FinalSun.h"
#include "FloatEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFloatEdit

CFloatEdit::CFloatEdit()
{
}

CFloatEdit::~CFloatEdit()
{
}


BEGIN_MESSAGE_MAP(CFloatEdit, CEdit)
	//{{AFX_MSG_MAP(CFloatEdit)
	ON_WM_ACTIVATE()
	ON_CONTROL_REFLECT(EN_KILLFOCUS, OnKillfocus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CFloatEdit 






void CFloatEdit::OnKillfocus() 
{

	// okay, we need to convert it to a float
	CString text, originaltext;
	GetWindowText(text);
	originaltext=text;

	if(strlen(text)<1)
	{
		//SetWindowText("0.000000");
		return;
	}
	
	double res=atof(text);
	int c,d;
	char* j=_fcvt(res, 6, &c, &d);
	
	int i, slen=strlen(j);
	char j2[50];

	for(i=0;i<6-slen;i++)
	{
		strcpy(j2, j+i);
		j[i]=0;
		strcat(j, "0");
		strcat(j,j2);
	}

	if(j==NULL) return;
	//MessageBox(j,text);
	text=j;
	text.Insert(c, ".");
	
	//MessageBox(text);
	if(strchr(text, '.')==text) text.Insert(0, "0");
	//delete[](j);	
	if(originaltext==text) return;
	SetWindowText(text);

}
