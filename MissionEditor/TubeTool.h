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

#include "MapTool.h"
#include <vector>
class CTube;

class AddTubeTool: public MapTool
{
public:
    AddTubeTool(CMapData& map, CIsoView& view, bool bidirectional);
    virtual ~AddTubeTool() = default;

    void finish();

    virtual bool onRButtonUp(const ProjectedCoords& projCoords, const MapCoords& mapCoords3d, MapToolMouseFlags flags);
    virtual void onLButtonDblClick(const ProjectedCoords& projCoords, const MapCoords& mapCoords3d, MapToolMouseFlags flags);
    virtual void onLButtonUp(const ProjectedCoords& projCoords, const MapCoords& mapCoords3d, MapToolMouseFlags flags);
    virtual void onMouseMove(const ProjectedCoords& projCoords, const MapCoords& mapCoords3d, MapToolMouseFlags flags);
    virtual void render();

protected:
    AddTubeTool& operator=(const AddTubeTool& other) = delete;

    virtual std::unique_ptr<CTube> getTubeToModify(const MapCoords& mapCoords3d, const ProjectedCoords& projCoords, MapToolMouseFlags flags) const;
    virtual std::unique_ptr<CTube> createNewTube(const MapCoords& mapCoords3d) const;

private:
    std::unique_ptr<CTube> m_tube;
    std::unique_ptr<CTube> m_mm_tube;
    std::unique_ptr<CTube> m_hover_tube;
    std::vector<std::unique_ptr<CTube>> m_modified_tubes;
    bool m_bidirectional;
};

class ModifyTubeTool : public AddTubeTool
{
public:
    ModifyTubeTool(CMapData& map, CIsoView& view, bool bidirectional);
    virtual ~ModifyTubeTool() = default;

protected:
    ModifyTubeTool& operator=(const AddTubeTool& other) = delete;

    virtual std::unique_ptr<CTube> getTubeToModify(const MapCoords& mapCoords3d, const ProjectedCoords& projCoords, MapToolMouseFlags flags) const;
    virtual std::unique_ptr<CTube> createNewTube(const MapCoords& mapCoords3d) const;

private:
};

class RemoveTubeTool : public MapTool
{
public:
    RemoveTubeTool(CMapData& map, CIsoView& view);
    virtual ~RemoveTubeTool() = default;

    virtual void onMouseMove(const ProjectedCoords& projCoords, const MapCoords& mapCoords3d, MapToolMouseFlags flags);
    virtual void render();

protected:
    RemoveTubeTool& operator=(const RemoveTubeTool& other) = delete;

private:
    std::vector<std::unique_ptr<CTube>> m_hover_tubes;
};

