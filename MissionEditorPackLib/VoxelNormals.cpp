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

#include "stdafx.h"
#include "VoxelNormals.h"
#include <array>
#include <math.h>

template<class T>
T read(std::istream& s)
{
    T t;
    auto p1 = s.tellg();
    auto n = sizeof(T);
    s.read(reinterpret_cast<char*>(&t), sizeof(T));
    auto p2 = s.tellg();
    auto p3 = p2 - p1;
    //s >> t;
    return t;
};

template<class T, int N>
std::array<T, N> read(std::istream& s)
{
    std::array<T, N> t;
    s.read(reinterpret_cast<char*>(t.data()), sizeof(T) * N);
    return t;
};

VoxelNormalTable::VoxelNormalTable(std::istream& f)
{
    auto oldEx = f.exceptions();
    f.exceptions(f.failbit | f.badbit);
    try
    {
        load(f);
    }
    catch (const std::exception& ex)
    {
        f.exceptions(oldEx);
        throw ex;
    }
    
}

void VoxelNormalTable::load(std::istream& f)
{
    auto count = read<std::uint8_t>(f);
    m_normals.clear();
    m_normals.reserve(count);
    for (auto i = 0; i < count; ++i)
    {
        auto v = read<float, 3>(f);
        m_normals.push_back(Vec3f(v.data()));
    }
}

VoxelNormalTables::VoxelNormalTables(std::istream& f)
{
    auto oldEx = f.exceptions();
    f.exceptions(f.failbit | f.badbit);
    try
    {      

        load(f);
                       
    }
    catch(const std::exception& ex)
    {
        f.exceptions(oldEx);
        throw ex;
    }
}

void VoxelNormalTables::load(std::istream& f)
{
    auto tableCount = read<std::uint8_t>(f);
    for (std::uint32_t i = 0; i < tableCount; ++i)
    {
        auto normalClass = read<std::uint8_t>(f);
        VoxelNormalTable table(f);
        auto normalIndex = normalClass - 1;
        m_tables.resize(std::max(m_tables.size(), static_cast<std::size_t>(normalClass)));
        m_tables[normalIndex] = std::move(table);
    }
}
