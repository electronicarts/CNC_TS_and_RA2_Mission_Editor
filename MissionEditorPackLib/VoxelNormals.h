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

#include <istream>
#include <vector>
#include "Vec3.h"

class VoxelNormalTable
{
public:
    VoxelNormalTable() = default;
    VoxelNormalTable(std::istream& f);
    VoxelNormalTable(VoxelNormalTable&& other) noexcept = default;
    VoxelNormalTable& operator=(const VoxelNormalTable& other) = default;
    VoxelNormalTable& operator=(VoxelNormalTable&& other) noexcept = default;

    Vec3f operator[] (unsigned int iNormal) const
    {
        return iNormal < m_normals.size() ? m_normals[iNormal] : Vec3f(0, 1, 0);
    }

private:
    void load(std::istream& f);

private:
    std::vector<Vec3f> m_normals;
};

class VoxelNormalTables
{
public:
    VoxelNormalTables() = default;
    VoxelNormalTables(std::istream& f);

    bool isValidTable(const std::uint8_t normalClass) const
    {
        return normalClass > 0 && normalClass <= m_tables.size();
    }

    // Returns the given normal table. Throws on invalid normalClass.
    const VoxelNormalTable& getTable(const std::uint8_t normalClass) const
    {
        if (!isValidTable(normalClass))
            throw std::range_error("Table for normal class does not exist");
        return m_tables[normalClass - 1];
    }

private:
    void load(std::istream& f);

private:
    // we use a vector instead of map for fast lookup
    std::vector<VoxelNormalTable> m_tables;
};
