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
#include "TubeTool.h"
#include "MapData.h"
#include "IsoView.h"
#include "Structs.h"
#include "inlines.h"
#include <memory>
#include <algorithm>

std::unique_ptr<CTube> findTubeEndAt(const MapCoords& mapCoords3d, const ProjectedCoords& projCoords, const CMapData& map)
{
	const auto& tubes = map.GetTubes();
	auto it = std::find_if(tubes.begin(), tubes.end(), [mapCoords3d](const auto& tube) { return tube->getEndCoords() == mapCoords3d; });
	if (it != tubes.end())
		return std::make_unique<CTube>(**it);
	// if not found, also try with z - 4 (e.g. behind cliffs)
	auto backMapCoords = map.ToMapCoords3d(projCoords, map.GetHeightAt(mapCoords3d) - 4);
	it = std::find_if(tubes.begin(), tubes.end(), [backMapCoords](const auto& tube) { return tube->getEndCoords() == backMapCoords; });
	if (it != tubes.end())
		return std::make_unique<CTube>(**it);
	return nullptr;
}

std::vector<std::uint16_t> findTubesAt(const ProjectedCoords& projCoords, const CMapData& map)
{
	std::vector<std::uint16_t> foundTubes;
	for (auto& t : map.GetTubes())
	{
		auto coords = map.ToMapCoords3d(projCoords, map.GetHeightAt(t->getStartCoords()));
		if (t->touches(coords))
			foundTubes.push_back(t->getId());
	}
	return foundTubes;
}

AddTubeTool::AddTubeTool(CMapData& map, CIsoView& view, bool bidirectional): MapTool(map, view),
	m_bidirectional(bidirectional)
{
	
}

void AddTubeTool::finish()
{
	if (m_tube) {
		if (m_tube->isValid())
		{
			getMap().SetTube(m_tube.get());
			if (m_bidirectional)
			{
				auto t2 = m_tube->reverse();
				getMap().SetTube(&t2);
			}
		}
		getView().RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	m_mm_tube.reset();
	m_tube.reset();
	m_modified_tubes.clear();
}

bool AddTubeTool::onRButtonUp(const ProjectedCoords& projCoords, const MapCoords& mapCoords3d, MapToolMouseFlags flags)
{
	if (!m_modified_tubes.empty())
	{
		// reset modified tubes to original state
		for (auto& tube : m_modified_tubes)
		{
			getMap().SetTube(tube.get());
		}
	}
	bool ret = m_tube != nullptr;
	m_mm_tube.reset();
	m_tube.reset();
	m_modified_tubes.clear();
	getView().RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	return ret;  // if we were adding or modifying a tube, a right click should just cancel this current tube, not the whole tool
}

void AddTubeTool::onLButtonDblClick(const ProjectedCoords& projCoords, const MapCoords& mapCoords3d, MapToolMouseFlags flags)
{
}

void AddTubeTool::onLButtonUp(const ProjectedCoords& projCoords, const MapCoords& mapCoords3d, MapToolMouseFlags flags)
{
	if (!m_tube) {
		// first check if there is already a tube at the given location and take ownership if existing
		auto tube = getTubeToModify(mapCoords3d, projCoords, flags);
		if (tube) {
			m_tube = std::move(tube);

			// also find reverse direction
			if (m_bidirectional)
			{
				const auto& tubes = getMap().GetTubes();
				auto reversed = m_tube->reverse();
				auto reverseIt = std::find_if(tubes.begin(), tubes.end(), [&reversed](const auto& tube) { return tube->isEqual(reversed, true); });
				if (reverseIt != tubes.end())
				{
					m_modified_tubes.push_back(std::make_unique<CTube>(**reverseIt));
					getMap().DeleteTube((*reverseIt)->getId());
				}
			}
			m_modified_tubes.push_back(std::make_unique<CTube>(*m_tube));
			getMap().DeleteTube(m_tube->getId());
		}

		if (!m_tube) {
			// create a new tube, first click
			m_tube = std::move(createNewTube(mapCoords3d));
		}
	}
	else
	{
		int z = getMap().GetHeightAt(m_tube->getStartCoords());
		auto mc = getMap().ToMapCoords3d(projCoords, z);
		if (m_tube->getEndCoords() == MapCoords(mc.x, mc.y))
		{
			finish();
		}
		else
		{
			m_tube->append(mc.x, mc.y, m_tube->GetTubeParts().empty() ? 2 : 0);
		}
		
	}

	getView().RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

std::unique_ptr<CTube> AddTubeTool::createNewTube(const MapCoords& mapCoords3d) const
{
	return std::make_unique<CTube>(mapCoords3d.x, mapCoords3d.y, ETubeDirection::Undefined, mapCoords3d.x, mapCoords3d.y, std::vector<ETubeDirection>());
}

std::unique_ptr<CTube> AddTubeTool::getTubeToModify(const MapCoords& mapCoords3d, const ProjectedCoords& projCoords, MapToolMouseFlags flags) const
{
	if ((flags & MapToolMouseFlags::SHIFT) == MapToolMouseFlags::SHIFT)
		return findTubeEndAt(mapCoords3d, projCoords, getMap());
	return nullptr;
}

void AddTubeTool::onMouseMove(const ProjectedCoords& projCoords, const MapCoords& mapCoords3d, MapToolMouseFlags flags)
{
	if (m_tube) {
		m_mm_tube.reset(new CTube(*m_tube));
		int z = getMap().GetHeightAt(m_tube->getStartCoords());
		auto mc = getMap().ToMapCoords3d(projCoords, z);
		m_mm_tube->append(mc.x, mc.y, m_tube->GetTubeParts().empty() ? 2 : 0);
		getView().RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	else
	{
		auto tube = getTubeToModify(mapCoords3d, projCoords, flags);
		if (tube)
		{
			m_hover_tube = std::move(tube);		
			getView().RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
		else if (m_hover_tube)
		{			
			m_hover_tube.reset();
			getView().RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}		
	}
}

void AddTubeTool::render()
{
	if (m_mm_tube)
	{
		// mouse-move tube
		getView().DrawTube(*m_mm_tube);
	}
	else if (m_tube) {
		getView().DrawTube(*m_tube);
	}
	else if (m_hover_tube)
	{
		COLORREF col = RGB(0, 255, 0);
		getView().DrawTube(*m_hover_tube, nullptr, &col);
	}
	
}

RemoveTubeTool::RemoveTubeTool(CMapData& map, CIsoView& view) : MapTool(map, view)
{
}

void RemoveTubeTool::onMouseMove(const ProjectedCoords& projCoords, const MapCoords& mapCoords3d, MapToolMouseFlags flags)
{
	auto& m = getMap();
	std::vector<std::uint16_t> tubes = findTubesAt(projCoords, getMap());

	if ((flags & MapToolMouseFlags::LBUTTON) == MapToolMouseFlags::LBUTTON)
	{
		for (auto& id : tubes)
		{
			m.DeleteTube(id);
		}			
	}
	else
	{
		m_hover_tubes.reserve(tubes.size());
		std::transform(tubes.begin(), tubes.end(), std::back_inserter(m_hover_tubes), [this](std::uint16_t tubeId) { return std::make_unique<CTube>(*getMap().GetTube(tubeId)); });
	}
	getView().RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	m_hover_tubes.clear();
}

void RemoveTubeTool::render()
{
	for(auto& tube: m_hover_tubes)
	{
		COLORREF col = RGB(0, 255, 0);
		getView().DrawTube(*tube, nullptr, &col);
	}
}

ModifyTubeTool::ModifyTubeTool(CMapData& map, CIsoView& view, bool bidirectional): AddTubeTool(map, view, bidirectional)
{
}

std::unique_ptr<CTube> ModifyTubeTool::getTubeToModify(const MapCoords& mapCoords3d, const ProjectedCoords& projCoords, MapToolMouseFlags flags) const
{
	return findTubeEndAt(mapCoords3d, projCoords, getMap());
}

std::unique_ptr<CTube> ModifyTubeTool::createNewTube(const MapCoords& mapCoords3d) const
{
	return nullptr;
}
