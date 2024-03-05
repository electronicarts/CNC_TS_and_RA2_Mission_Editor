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

#include <vector>
#include <string>
#include <functional>
#include "Structs.h"

enum class ETubeDirection : char
{
    Undefined = -1,
    Top=0,
    TopRight=1,
    Right=2,
    BottomRight=3,
    Bottom=4,
    BottomLeft=5,
    Left=6,
    TopLeft=7,
};

inline ETubeDirection ToTubeDirection(const std::underlying_type_t<ETubeDirection> dir)
{
    if (dir > 7 || dir < -1)
        return ETubeDirection::Undefined;
    return static_cast<ETubeDirection>(dir);
}

inline auto to_int(const ETubeDirection dir)
{
    return static_cast<std::underlying_type_t<ETubeDirection>>(dir);
}

ETubeDirection opposite_dir(ETubeDirection dir);
bool dir_to_xy(ETubeDirection dir, MapVec& y);

class CTube
{
public:
    struct WalkInfo
    {
        MapCoords pos;
        ETubeDirection direction = ETubeDirection::Undefined;
        MapCoords next_pos;
    };

public:
    CTube() = default;
    CTube(std::uint16_t tubeId, const std::string& s);
    CTube(
        std::uint16_t tubeID,
        std::uint16_t startX,
        std::uint16_t startY,
        ETubeDirection direction,
        std::uint16_t endX,
        std::uint16_t endY,
        const std::vector<ETubeDirection>& tubeParts
    );
    CTube(
        std::uint16_t startX,
        std::uint16_t startY,
        ETubeDirection direction,
        std::uint16_t endX,
        std::uint16_t endY,
        const std::vector<ETubeDirection>& tubeParts
    );

    CTube reverse(std::uint16_t newTubeID=0xFFFF) const;

    // full equivalency including id
    bool operator==(const CTube& r) const = default;

    bool isEqual(const CTube& r, bool ignoreId) const;

    // Create tube with simple algorithm
    static CTube autocreate(std::uint16_t startX, std::uint16_t startY, std::uint16_t endX, std::uint16_t endY, int straightStartParts=1);


    const std::vector<ETubeDirection>& GetTubeParts() const
    {
        return m_tubeParts;
    }

    void setId(std::uint16_t id)
    {
        m_tubeId = id;
    }

    bool isCounterpart(const CTube& other) const
    {
        return getStartCoords() == other.getEndCoords() && getEndCoords() == other.getStartCoords();
    }

    bool hasId() const
    {
        return m_tubeId != 0xFFFF;
    }
    auto getId() const { return m_tubeId; }
    auto getStartX() const { return m_startX; }
    auto getStartY() const { return m_startY; }
    MapCoords getStartCoords() const { return MapCoords(m_startX, m_startY); }
    auto getInitialDirection() const { return m_direction; }
    ETubeDirection getLastDirection() const;
    auto getEndX() const { return m_endX; }
    auto getEndY() const { return m_endY; }
    MapCoords getEndCoords() const { return MapCoords(m_endX, m_endY); }

    bool touches(const MapCoords& mc) const;

    std::string toString() const;

    bool append(std::uint16_t endX, std::uint16_t endY, int forceStraightParts=-1);

    bool walk(const std::function<bool(const WalkInfo&)>& walker) const;

    bool isValid() const;

private:
    std::uint16_t m_tubeId = 0xFFFF; // ID of tube
    std::uint16_t m_startX = 0;
    std::uint16_t m_startY = 0;
    ETubeDirection m_direction = ETubeDirection::Undefined;
    std::uint16_t m_endX = 0;
    std::uint16_t m_endY = 0;
    std::vector<ETubeDirection> m_tubeParts;
};