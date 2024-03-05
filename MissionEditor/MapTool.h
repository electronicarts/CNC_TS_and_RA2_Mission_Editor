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

#pragma once

#include <cstdint>
#include "structs.h"
#include <winnt.h>

class CMapData;
class CIsoView;

enum class MapToolMouseFlags
{
    DEFAULT = 0,
    LBUTTON = 1,
    MBUTTON = 2,
    RBUTTON = 4,
    SHIFT = 8
};
DEFINE_ENUM_FLAG_OPERATORS(MapToolMouseFlags);

inline MapToolMouseFlags MapToolMouseFlagsFromWin32(UINT nFlags) {

    MapToolMouseFlags flags = MapToolMouseFlags::DEFAULT;

    if ((nFlags & MK_LBUTTON) == MK_LBUTTON)
        flags |= MapToolMouseFlags::LBUTTON;

    if ((nFlags & MK_MBUTTON) == MK_MBUTTON)
        flags |= MapToolMouseFlags::MBUTTON;

    if ((nFlags & MK_RBUTTON) == MK_RBUTTON)
        flags |= MapToolMouseFlags::RBUTTON;

    if ((nFlags & MK_SHIFT) == MK_SHIFT)
        flags |= MapToolMouseFlags::SHIFT;
        
    return flags;
}

class MapTool
{
public:
    virtual ~MapTool() = default;

    // return false if tool has not handled this call (or doesn't want to override any caller behavior)
    virtual bool onRButtonUp(const ProjectedCoords& projCoords, const MapCoords& mapCoords3d, MapToolMouseFlags flags)
    {        
        return false;
    };

    virtual void onLButtonDblClick(const ProjectedCoords& projCoords, const MapCoords& mapCoords3d, MapToolMouseFlags flags) {};
    virtual void onLButtonUp(const ProjectedCoords& projCoords, const MapCoords& mapCoords3d, MapToolMouseFlags flags) {};
    virtual void onMouseMove(const ProjectedCoords& projCoords, const MapCoords& mapCoords3d, MapToolMouseFlags flags) {};
    virtual void render() {};

protected:
    MapTool(CMapData& map, CIsoView& view) : m_map(map), m_view(view) {};    
    MapTool& operator=(const MapTool& other) = delete;

    CMapData& getMap() { return m_map; }
    const CMapData& getMap() const { return m_map; }
    CIsoView& getView() { return m_view; }
    const CIsoView& getView() const { return m_view; }

private:
    CMapData& m_map;
    CIsoView& m_view;
};