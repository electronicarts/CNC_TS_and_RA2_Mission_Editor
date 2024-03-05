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

#include "StdAfx.h"
#include "Tube.h"
#include <ranges>

#include "inlines.h"

#ifndef __INTELLISENSE__
// This file had problems with Intellisense. Until this is fixed, deactivate intellisense parsing here.

int sgn(int v)
{
	if (v < 0) return -1;
	return v > 0 ? 1 : 0;
}

CTube::CTube(std::uint16_t tubeID, std::uint16_t startX, std::uint16_t startY, ETubeDirection direction, std::uint16_t endX, std::uint16_t endY, const std::vector<ETubeDirection>& tubeParts):
	CTube(startX, startY, direction, endX, endY, tubeParts)
{
	m_tubeId = tubeID;
}

CTube::CTube(const std::uint16_t startX, const std::uint16_t startY, const ETubeDirection direction, const std::uint16_t endX, const std::uint16_t endY, const std::vector<ETubeDirection>& tubeParts):
	CTube()
{
	m_startX = startX;
	m_startY = startY;
	m_direction = direction;
	m_endX = endX;
	m_endY = endY;
	m_tubeParts = tubeParts;
}

CTube::CTube(std::uint16_t tubeId, const std::string& value):
	m_tubeId(tubeId)
{
	m_startY = stoi(GetParam(value, 0));
	m_startX = stoi(GetParam(value, 1));
	m_direction = ToTubeDirection(stoi(GetParam(value, 2)));
	m_endY = stoi(GetParam(value, 3));
	m_endX = stoi(GetParam(value, 4));

	std::string readDirS;
	for (std::size_t e = 5; !(readDirS = GetParam(value, e)).empty(); ++e)
	{
		const auto readDir = stoi(readDirS);
		m_tubeParts.push_back(ToTubeDirection(readDir));
	}
	if (m_tubeParts.empty() || m_tubeParts.back() != ETubeDirection::Undefined)
		m_tubeParts.push_back(ETubeDirection::Undefined);
}

bool CTube::isValid() const
{
	return m_direction != ETubeDirection::Undefined && m_tubeParts.size() > 1;
}

CTube CTube::reverse(std::uint16_t newTubeID) const
{
	CTube ti2;

	ti2.m_tubeId = newTubeID;

	ti2.m_startX = m_endX;
	ti2.m_startY = m_endY;
	ti2.m_endX = m_startX;
	ti2.m_endY = m_startY;

	const auto last_d = getLastDirection();
	ti2.m_direction = opposite_dir(last_d);

	auto x = m_startX;
	auto y = m_startY;

	// reverse direction tube parts
	auto reversed = m_tubeParts |
		std::views::reverse |
		std::views::transform([](auto dir) {return opposite_dir(dir);}) |
		std::views::common;

	// find the first defined direction in the reversed list
	auto reversedFirst = std::find_if(reversed.begin(), reversed.end(), [](ETubeDirection dir) { return dir != ETubeDirection::Undefined; });
	auto reversedSize = reversed.end() - reversedFirst;
	ti2.m_tubeParts.reserve(reversed.size() + 2);
	ti2.m_tubeParts.assign(reversedFirst, reversed.end());
	ti2.m_tubeParts.push_back(ETubeDirection::Undefined);

	return ti2;
}

static const ETubeDirection kDiffToDir[3][3] = {
		{ETubeDirection::TopLeft, ETubeDirection::Top, ETubeDirection::TopRight},  // xadd == -1
		{ETubeDirection::Left, ETubeDirection::Undefined, ETubeDirection::Right}, // xadd == 0
		{ETubeDirection::BottomLeft, ETubeDirection::Bottom, ETubeDirection::BottomRight}   // xadd == 1
};

bool CTube::isEqual(const CTube& r, bool ignoreId) const
{
	if (!ignoreId)
		return *this == r;

	const auto& largerTubeParts = r.m_tubeParts.size() >= m_tubeParts.size() ? r.m_tubeParts : m_tubeParts;
	const auto& smallerTubeParts = r.m_tubeParts.size() < m_tubeParts.size() ? r.m_tubeParts : m_tubeParts;
	for (auto i = 0; i < smallerTubeParts.size(); ++i)
	{
		if (largerTubeParts[i] != smallerTubeParts[i])
			return false;
	}
	for (auto i = smallerTubeParts.size(); i < largerTubeParts.size(); ++i)
	{
		if (largerTubeParts[i] != ETubeDirection::Undefined)
			return false;
	}
	
	return m_direction == r.m_direction && m_startX == r.m_startX && m_startY == r.m_startY && r.m_endX == r.m_endX && r.m_endY == r.m_endY;
}

CTube CTube::autocreate(std::uint16_t startX, std::uint16_t startY, std::uint16_t endX, std::uint16_t endY, int straightStartParts)
{
	CTube ti;
	ti.m_startX = startX;
	ti.m_startY = startY;
	ti.m_endX = startX;
	ti.m_endY = startY;
	ti.append(endX, endY, straightStartParts);
	return ti;

	/*
	int curx = startX;
	int cury = startY;

	CTube ti;
	ti.m_startX = startX;
	ti.m_startY = startY;
	ti.m_endX = endX;
	ti.m_endY = endY;

	

	int xadd = sgn(endX - curx);
	int yadd = sgn(endY - cury);
	if (straightStartTiles > 0)
	{
		const bool xMajor = abs(endX - startX) > abs(endY - startY);
		xadd = xMajor ? xadd : 0;
		yadd = xMajor ? 0 : yadd;
	}

	curx += xadd;
	cury += yadd;
	ti.m_direction = kDiffToDir[xadd + 1][yadd + 1];

	int n = 1;
	while (endX != curx || endY != cury)
	{
		xadd = n < straightStartTiles ? xadd : sgn(endX - curx); // keep initial direction for straightStartTiles
		yadd = n < straightStartTiles ? yadd : sgn(endY - cury);
		n++;
		curx += xadd;
		cury += yadd;
		ti.m_tubeParts.push_back(kDiffToDir[xadd + 1][yadd + 1]);
	}

	ti.m_tubeParts.push_back(ETubeDirection::Undefined);
	return ti;
	*/
}

ETubeDirection opposite_dir(const ETubeDirection dir)
{
	static const ETubeDirection opposite_dir_tbl[8] = {
		ETubeDirection::Bottom,
		ETubeDirection::BottomLeft,
		ETubeDirection::Left,
		ETubeDirection::TopLeft,
		ETubeDirection::Top,
		ETubeDirection::TopRight,
		ETubeDirection::Right,
		ETubeDirection::BottomRight
	};
	const auto iDir = to_int(dir);
	if (iDir < 0 || iDir > 7)
		return ETubeDirection::Undefined;
	return opposite_dir_tbl[iDir];
}


bool dir_to_xy(ETubeDirection dir, MapVec& vec)
{
	static int dir_to_xy_table[8][2] = {
			   {0, -1},   // 0
			   {1, -1},   // 1
			   {1, 0},    // 2
			   {1, 1},    // 3
			   {0, 1},    // 4
			   {-1, 1},   // 5
			   {-1, 0},   // 6
			   {-1, -1},  // 7
	};
	const auto iDir = to_int(dir);
	if (iDir < 0 || iDir > 7)
	{
		vec.x = 0;
		vec.y = 0;
		return false;
	}
	vec.x = dir_to_xy_table[iDir][1];  // swap x/y
	vec.y = dir_to_xy_table[iDir][0];
	return true;
}

ETubeDirection CTube::getLastDirection() const
{
	if (m_tubeParts.empty())
		return m_direction;
	auto last = std::find_if(m_tubeParts.crbegin(), m_tubeParts.crend(), [](ETubeDirection dir) { return dir != ETubeDirection::Undefined; });
	return last == m_tubeParts.crend() ? m_direction : *last;
}

bool CTube::touches(const MapCoords& mc) const
{
	bool touches = false;
	walk([this, &mc, &touches](const auto& wi) {
		if (wi.pos == mc)
		{
			touches = true;
			return false;
		}
		return true;
	});
	return touches;
}

std::string CTube::toString() const
{
	auto partsAsInt = m_tubeParts | std::views::transform([](auto dir) {return to_int(dir); }) | std::views::common;

	std::vector<int> values;
	values.reserve(5 + m_tubeParts.size());
	values.assign({ m_startY, m_startX, to_int(m_direction), m_endY, m_endX });
	values.insert(values.end(), partsAsInt.begin(), partsAsInt.end());

	std::string res = Join(",", values | std::views::transform([](auto v) {return std::to_string(v); }));
	if (m_tubeParts.empty() || m_tubeParts.back() != ETubeDirection::Undefined)
		res += ",-1"; // TS/RA2 crash when no delimiter exists
	return res;
}

bool CTube::append(std::uint16_t endX, std::uint16_t endY, int forceStraightParts)
{
	auto newTubeParts = m_tubeParts;
	MapCoords end(endX, endY);
	
	// remove delimiters
	auto oldLast = std::find(newTubeParts.begin(), newTubeParts.end(), ETubeDirection::Undefined);
	newTubeParts.resize(oldLast - newTubeParts.begin());

	// now find current x/y
	MapCoords cur = getStartCoords();
	std::vector<MapCoords> existingPositions;
	if(!walk([&cur, &existingPositions](const WalkInfo& wi) {
		cur = wi.pos;
		existingPositions.push_back(wi.pos);
		return true;
	}))
		return false;
	
	// if no enter direction was given, set it now - this should only be true if there are no existingPositions
	if (m_direction == ETubeDirection::Undefined)
	{
		const bool xMajor = abs(end.x - m_startX) > abs(end.y - m_startY);
		MapVec add(sgn(end.x - m_startX), sgn(end.y - m_startY));		
		add.x = xMajor ? add.x : 0;
		add.y = xMajor ? 0 : add.y;
		m_direction = kDiffToDir[add.x + 1][add.y + 1];
	}

	auto endOnExistingIt = std::find(existingPositions.begin(), existingPositions.end(), end);
	if (endOnExistingIt != existingPositions.end())
	{
		if (endOnExistingIt == existingPositions.begin())
			return false; // zero length remaining

		// shorten
		auto remaining = endOnExistingIt - existingPositions.begin();
		newTubeParts.resize(remaining);
		newTubeParts.push_back(ETubeDirection::Undefined);
		m_endX = end.x;
		m_endY = end.y;
		m_tubeParts = std::move(newTubeParts);
		return true;
	}
	
	int n = 0;
	while (end != cur)
	{
		MapVec add(sgn(end.x - cur.x), sgn(end.y - cur.y));
		if (newTubeParts.size() < 1 && forceStraightParts < 0)
		{
			// the first tube part should be in the same direction as the enter direction
			dir_to_xy(m_direction, add);
		}
		if (n++ < forceStraightParts)
		{
			const bool xMajorPart = abs(end.x - m_endX) > abs(end.y - m_endY);
			add.x = xMajorPart ? add.x : 0;
			add.y = xMajorPart ? 0 : add.y;
		}
		cur += add;
		//if (std::find(existingPositions.begin(), existingPositions.end(), std::make_pair(curX, curY)) != existingPositions.end())
		//	return false; // intersection - maybe shorten? maybe we can even allow it if TS accepts it?
		newTubeParts.push_back(kDiffToDir[add.x + 1][add.y + 1]);
	}

	newTubeParts.push_back(ETubeDirection::Undefined);
	m_endX = cur.x;
	m_endY = cur.y;
	m_tubeParts = std::move(newTubeParts);
	
	return true;
}

bool CTube::walk(const std::function<bool(const WalkInfo&)>& walker) const
{
	MapVec diff;
	WalkInfo wi;
	wi.pos = MapCoords(m_startX, m_startY);

	for (auto dir : m_tubeParts)
	{
		wi.direction = dir;
		dir_to_xy(wi.direction, diff);
		wi.next_pos = (wi.direction == ETubeDirection::Undefined) ? MapCoords(-1, -1) : wi.pos + diff;
		if (!walker(wi))
			return false;
		if (wi.direction == ETubeDirection::Undefined)
			break;
		wi.pos = wi.next_pos;
	}	
	return true;
}


#endif