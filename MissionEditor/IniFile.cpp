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

// IniFile.cpp: Implementierung der Klasse CIniFile.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IniFile.h"
#include <string>
#include <algorithm>
#include <stdexcept>



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

using namespace std;

bool SortDummy::operator()(const CString& x, const CString& y) const
{
	// the length is more important than spelling (numbers!!!)...
	if (x.GetLength() < y.GetLength()) return true;
	if (x.GetLength() == y.GetLength())
	{
		if (x < y) return true;
	}

	return false;

}

typedef map<CString, CIniFileSection>::iterator CIniI;
typedef map<CString, CString, SortDummy>::iterator SI;
typedef map<CString, int, SortDummy>::iterator SII;


//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CIniFile::CIniFile()
{
	Clear();
}

CIniFile::~CIniFile()
{
	sections.clear();
}

WORD CIniFile::LoadFile(const CString& filename,  BOOL bNoSpaces)
{
	return LoadFile(std::string(filename.GetString()), bNoSpaces);
}

WORD CIniFile::LoadFile(const std::string& filename, BOOL bNoSpaces)
{
	Clear();

	if (filename.size() == NULL) return 1;
	m_filename = filename;

	return(InsertFile(filename, NULL, bNoSpaces));

}


void CIniFile::Clear()
{

	sections.clear();
}

CIniFileSection::CIniFileSection()
{
	values.clear();
	value_orig_pos.clear();
};

CIniFileSection::~CIniFileSection()
{
	values.clear();
	value_orig_pos.clear();
};

WORD CIniFile::InsertFile(const CString& filename, const char* Section, BOOL bNoSpaces)
{
	return InsertFile(std::string(filename.GetString()), Section, bNoSpaces);
}

WORD CIniFile::InsertFile(const std::string& filename, const char* Section, BOOL bNoSpaces)
{
	if (filename.size() == 0)
		return 1;

	fstream file;

	file.open(filename, ios::in);
	if (!file.good())
		return 2;


	//char cSec[256];
	//char cLine[4096];

	//memset(cSec, 0, 256);
	//memset(cLine, 0, 4096);
	CString cSec;
	std::string cLine;

	const auto npos = std::string::npos;

	while (!file.eof())
	{
		std::getline(file, cLine);

		// strip to left side of newline or comment
		cLine.erase(std::find_if(cLine.begin(), cLine.end(), [](const char c) { return c == '\r' || c == '\n' || c == ';'; }), cLine.end());

		const auto openBracket = cLine.find('[');
		const auto closeBracket = cLine.find(']');
		const auto equals = cLine.find('=');

		if (openBracket != npos && closeBracket != npos && openBracket < closeBracket && (equals == npos || equals > openBracket))
		{
			if ((Section != nullptr) && cSec == Section)
				return 0; // the section we want to insert is finished

			cSec = cLine.substr(openBracket + 1, closeBracket - openBracket - 1).c_str();
		}
		else if (equals != npos && !cSec.IsEmpty())
		{
			if (Section == NULL || cSec == Section)
			{
				// a value is set and we have a valid current section!
				CString name = cLine.substr(0, equals).c_str();
				CString value = cLine.substr(equals + 1, cLine.size() - equals - 1).c_str();

				int cuValueIndex = sections[cSec].values.size();

				if (bNoSpaces)
				{
					name.Trim();
					value.Trim();
				}

				sections[cSec].values[name] = value;
				sections[cSec].value_orig_pos[name] = cuValueIndex;
			}
		}

	}



	file.close();

	return 0;
}

const CIniFileSection* CIniFile::GetSection(std::size_t index) const
{
	if (index > sections.size() - 1)
		return NULL;

	auto i = sections.cbegin();
	for (auto e = 0;e < index;e++)
		i++;

	return &i->second;
}

CIniFileSection* CIniFile::GetSection(std::size_t index)
{
	if (index > sections.size() - 1)
		return NULL;

	CIniI i = sections.begin();
	for (auto e = 0;e < index;e++)
		i++;

	return &i->second;
}

const CIniFileSection* CIniFile::GetSection(const CString& section) const
{
	auto it = sections.find(section);
	if (it == sections.end())
		return nullptr;
	return &it->second;
}

CIniFileSection* CIniFile::GetSection(const CString& section)
{
	auto it = sections.find(section);
	if (it == sections.end())
		return nullptr;
	return &it->second;
}

const CString* CIniFileSection::GetValue(std::size_t index) const noexcept
{
	if (index > values.size() - 1)
		return NULL;

	auto i = values.begin();
	for (auto e = 0;e < index;e++)
		i++;

	return &i->second;
}

CString* CIniFileSection::GetValue(std::size_t index) noexcept
{
	if (index > values.size() - 1)
		return NULL;

	auto i = values.begin();
	for (auto e = 0;e < index;e++)
		i++;

	return &i->second;
}

CString CIniFileSection::GetValueByName(const CString& valueName, const CString& defaultValue) const
{
	auto it = values.find(valueName);
	return (it == values.end()) ? defaultValue : it->second;
}

const CString* CIniFile::GetSectionName(std::size_t index) const noexcept
{
	if (index > sections.size() - 1)
		return NULL;

	auto i = sections.cbegin();
	for (auto e = 0; e < index; ++e)
		i++;

	return &(i->first);
}

CString& CIniFileSection::AccessValueByName(const CString& valueName)
{
	return values[valueName];
}

const CString* CIniFileSection::GetValueName(std::size_t index) const noexcept
{
	if (index > values.size() - 1)
		return NULL;

	auto i = values.begin();
	for (auto e = 0; e < index; ++e)
		i++;


	return &(i->first);
}

BOOL CIniFile::SaveFile(const CString& filename) const
{
	return SaveFile(std::string(filename.GetString()));
}

BOOL CIniFile::SaveFile(const std::string& Filename) const
{
	fstream file;

	file.open(Filename, ios::out | ios::trunc);

	int i;
	for (i = 0;i < sections.size();i++)
	{
		file << "[" << (LPCTSTR)*GetSectionName(i) << "]" << endl;
		int e;
		for (e = 0;e < GetSection(i)->values.size();e++)
		{
			file << (LPCTSTR) * (GetSection(i)->GetValueName(e)) << "=" << (LPCTSTR)*GetSection(i)->GetValue(e) << endl;
		}
		file << endl;
	}

	file << endl;

	return TRUE;
}


int CIniFileSection::FindValue(CString sval) const noexcept
{
	int i;
	auto it = values.cbegin();
	for (i = 0;i < values.size();i++)
	{
		if (sval == it->second)
			return i;
		it++;
	}
	return -1;
}

int CIniFileSection::FindName(CString sval) const noexcept
{
	int i;
	auto it = values.cbegin();
	for (i = 0;i < values.size();i++)
	{
		if (sval == it->first)
			return i;
		it++;
	}
	return -1;
}

void CIniFile::DeleteLeadingSpaces(BOOL bValueNames, BOOL bValues)
{
	int i;
	for (i = 0;i < sections.size();i++)
	{
		CIniFileSection& sec = *GetSection(i);
		int e;
		for (e = 0;e < sec.values.size();e++)
		{
			if (bValues) sec.GetValue(e)->TrimLeft();
			if (bValueNames)
			{
				CString value = *sec.GetValue(e);
				CString name = *sec.GetValueName(e);

				sec.values.erase(name);
				name.TrimLeft();
				sec.values[name] = value;
			}
		}
	}
}

void CIniFile::DeleteEndingSpaces(BOOL bValueNames, BOOL bValues)
{
	int i;
	for (i = 0;i < sections.size();i++)
	{
		CIniFileSection& sec = *GetSection(i);
		int e;
		for (e = 0;e < sec.values.size();e++)
		{
			if (bValues) sec.GetValue(e)->TrimRight();
			if (bValueNames)
			{
				//CString& name=(CString&)*sec.GetValueName(e);
				//name.TrimRight();
				CString value = *sec.GetValue(e);
				CString name = *sec.GetValueName(e);

				sec.values.erase(name);
				name.TrimRight();
				sec.values[name] = value;
			}
		}
	}
}

CString CIniFile::GetValueByName(const CString& sectionName, const CString& valueName, const CString& defaultValue) const
{
	auto section = GetSection(sectionName);
	if (!section)
		return defaultValue;
	return section->GetValueByName(valueName, defaultValue);
}

int CIniFileSection::GetValueOrigPos(int index) const noexcept
{
	if (index > value_orig_pos.size() - 1)
		return -1;

	auto i = value_orig_pos.cbegin();
	for (int e = 0;e < index;e++)
		i++;

	return i->second;
}

