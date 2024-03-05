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

#if !defined(AFX_ISOPAINTTHREAD_H__13F04541_01CB_11D5_9C88_FB4495B57649__INCLUDED_)
#define AFX_ISOPAINTTHREAD_H__13F04541_01CB_11D5_9C88_FB4495B57649__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IsoPaintThread.h : Header-Datei
//



/////////////////////////////////////////////////////////////////////////////
// Thread CIsoPaintThread 

class CIsoPaintThread : public CWinThread
{
	DECLARE_DYNCREATE(CIsoPaintThread)
protected:
	CIsoPaintThread();           // Dynamische Erstellung verwendet geschützten Konstruktor

// Attribute
public:

// Operationen
public:

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CIsoPaintThread)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementierung
protected:
	virtual ~CIsoPaintThread();

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CIsoPaintThread)
		// HINWEIS - Der Klassen-Assistent fügt hier Member-Funktionen ein und entfernt diese.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_ISOPAINTTHREAD_H__13F04541_01CB_11D5_9C88_FB4495B57649__INCLUDED_
