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

#ifndef INLINES_H_INCLUDED
#define INLINES_H_INCLUDED

#include "functions.h"
#include "macros.h"
#include "mapdata.h"
#include "variables.h"
#include "ovrlinline.h"
#include <string>
#include <vector>
#include <ranges>


inline BOOL isTrue(CString expr)
{
	expr.MakeLower();
	if (expr == "yes" || expr == "true" || expr == "1")
		return TRUE;
	return FALSE;
}

inline BOOL isFalse(CString expr)
{
	expr.MakeLower();
	if (expr == "no" || expr == "false" || expr == "0")
		return TRUE;
	return FALSE;
}

// retrieve the picture filename of a unit (as it is saved in the pics map). The returned file may not exist in the pics map (you need to do a check!).
inline CString GetUnitPictureFilename(LPCTSTR lpUnitName, DWORD dwPicIndex)
{
	CIniFile& ini = Map->GetIniFile();

	CString UnitName = lpUnitName;

	UnitName = rules.sections[lpUnitName].GetValueByName("Image", lpUnitName);

	if (ini.sections.find(lpUnitName) != ini.sections.end())
		UnitName = ini.sections[lpUnitName].GetValueByName("Image", UnitName);

	if (rules.sections[lpUnitName].values.find("Image") != rules.sections[lpUnitName].values.end())
		UnitName = rules.sections[lpUnitName].values["Image"];

	CString artname = UnitName;

	if (art.sections[UnitName].values.find("Image") != art.sections[UnitName].values.end())
	{
		if (!isTrue(g_data.sections["IgnoreArtImage"].AccessValueByName(UnitName)))
			artname = art.sections[UnitName].AccessValueByName("Image");
	}
		

	CString filename = UnitName;

	if (art.sections[UnitName].FindName("NewTheater") >= 0 && art.sections[UnitName].AccessValueByName("DemandLoad") != "yes")
		if (art.sections[UnitName].AccessValueByName("NewTheater") == "yes")
			filename.SetAt(1, 'T');

	char n[50];
	itoa(dwPicIndex, n, 10);


	if (pics.find(artname + n) != pics.end())
	{
		filename = artname; // yes, found
		filename += n;
	}
	else if (pics.find(artname + ".bmp") != pics.end()) // since June, 15th (Matze): Only use BMP if no SHP/VXL exists
	{
		filename = (CString)artname + ".bmp";
	}
	else
		filename = "";

	return filename;
}

inline CString GetParam(const CString& data, const int param)
{
	int paramStrPos = 0;
	int curParam = param;

	while (curParam--)
	{
		auto nextComma = data.Find(',', paramStrPos);
		if (nextComma < 0)
			return CString(); // RVO; param not found
		paramStrPos = nextComma + 1;
	}

	auto nextComma = data.Find(',', paramStrPos);
	CString res = data.Mid(paramStrPos, nextComma < 0 ? data.GetLength() - paramStrPos : nextComma - paramStrPos);
	return res; // RVO
}

inline std::string GetParam(const std::string& data, const int param)
{
	int paramStrPos = 0;
	int curParam = param;

	while (curParam--)
	{
		auto nextComma = data.find(',', paramStrPos);
		if (nextComma == std::string::npos)
			return std::string(); // RVO; param not found
		paramStrPos = nextComma + 1;
	}

	auto nextComma = data.find(',', paramStrPos);
	std::string res = data.substr(paramStrPos, nextComma == std::string::npos ? data.size() - paramStrPos : nextComma - paramStrPos);
	return res; // RVO
}

inline CString GetParam(const char* data, const int param)
{
	return GetParam(CString(data), param);
}

inline std::vector<CString> Split(const CString& data, char separator)
{
	int nextComma = -1;
	int lastComma = -1;
	const auto len = data.GetLength();
	std::vector<CString> res;
	while (lastComma < len)
	{
		nextComma = data.Find(separator, lastComma + 1);
		if (nextComma < 0)
		{
			res.push_back(data.Mid(lastComma + 1));
			break;
		}

		res.push_back(data.Mid(lastComma + 1, (nextComma - lastComma - 1)));
		lastComma = nextComma;
	}
	return res; // RVO
}

inline std::vector<CString> SplitParams(const CString& data)
{
	return Split(data, ',');
}


inline CString Join(const CString& join, const std::vector<CString>& strings)
{
	CString res;
	int len = 0;
	for (auto& s : strings)
		len += s.GetLength() + join.GetLength();
	res.Preallocate(len + 1);
	int remaining = strings.size();
	for (auto& s : strings)
	{
		res += s;
		if (--remaining)
			res += join;
	}
	return res; // RVO
}

inline std::string Join(const std::string& join, const std::ranges::input_range auto&& strings)
{
	std::string res;
	int len = 0;
	for (const auto& s : strings)
		len += s.size() + join.size();
	res.reserve(len + 1);
	int remaining = strings.size();
	for (const auto& s : strings)
	{
		res += s;
		if (--remaining)
			res += join;
	}
	return res; // RVO
}

inline CString SetParam(const CString& data, const int param, const CString& value)
{
	// This could be optimized, but SetParam is usually no performance issue.
	if (param < 0)
		return data;
	std::vector<CString> params = SplitParams(data);
	params.resize(max(param + 1, static_cast<int>(params.size())));
	params[param] = value;
	return Join(",", params);
}


[[deprecated("Instead use CMapData::ToMapCoords")]]
inline void ToIso(int* x, int* y)
{
	auto r = Map->ToMapCoords(ProjectedCoords(*x, *y));
	*x = r.x;
	*y = r.y;
}

[[deprecated("Instead use CMapData::ProjectCoords")]]
inline void ToPhys(int* x, int* y)
{
	auto r = Map->ProjectCoords(MapCoords(*x, *y));
	*x = r.x;
	*y = r.y;
}

[[deprecated("Instead use CMapData::ProjectCoords3d")]]
inline void ToPhys3d(int* x, int* y)
{
	auto r = Map->ProjectCoords3d(MapCoords(*x, *y));
	*x = r.x;
	*y = r.y;
}

[[deprecated("Instead use CMapData::ProjectCoords3d")]]
inline void ToPhys3d(int* x, int* y, int mapZ)
{
	auto r = Map->ProjectCoords3d(MapCoords(*x, *y), mapZ);
	*x = r.x;
	*y = r.y;
}

[[deprecated("Instead use CMapData::ToMapCoords3d")]]
inline void ToIso3d(int* x, int* y, int mapZ)
{
	auto r = Map->ToMapCoords3d(ProjectedCoords(*x, *y), mapZ);
	*x = r.x;
	*y = r.y;
}


inline BOOL isSame(CString expr1, CString expr2)
{
	expr1.MakeLower();
	expr2.MakeLower();
	if (expr1 == expr2) return TRUE;
	return FALSE;
}

inline BOOL isIncluded(CString searchString, CString base)
{
	searchString.MakeLower();
	base.MakeLower();
	if (searchString.Find(base) < 0) return FALSE;
	return TRUE;
}

inline BOOL isTrack(int type)
{
	return(type >= OVRL_TRACK_BEGIN && type <= OVRL_TRACK_END);
}

inline BOOL isBigBridge(int type)
{
#ifndef RA2_MODE
	return((type >= 0x18 && type <= 0x19) || (type >= 0x3b && type <= 0x3c));
#else
	return((type >= 0x18 && type <= 0x19) || (type >= 0x3b && type <= 0x3c) || (type >= 0xed && type <= 0xee));
#endif
}



#endif