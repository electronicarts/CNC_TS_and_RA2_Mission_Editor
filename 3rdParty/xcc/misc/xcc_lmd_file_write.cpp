/*
    XCC Utilities and Library
    Copyright (C) 2000  Olaf van der Spek  <olafvdspek@gmail.com>

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
#include "xcc_lmd_file_write.h"

#include "cc_structures.h"
#include "xcc_file.h"

void Cxcc_lmd_file_write::add_fname(const string& fname)
{
	m_index.push_back(to_lower_copy(fname));
}

Cvirtual_binary Cxcc_lmd_file_write::write(t_game game) const
{
	int size = sizeof(t_xcc_header) + sizeof(t_xcc_lmd_header);
	for (auto& i : m_index)
		size += i.length() + 1;
	Cvirtual_binary data;
	byte* w = data.write_start(size);
	t_xcc_header& header = *reinterpret_cast<t_xcc_header*>(w);
	strcpy(header.id, xcc_id);
	header.size = size;
	header.type = xcc_ft_lmd;
	header.version = 0;
	w += sizeof(t_xcc_header);
	t_xcc_lmd_header& lmd_header = *reinterpret_cast<t_xcc_lmd_header*>(w);
	lmd_header.c_fnames = m_index.size();
	lmd_header.game = game;
	w += sizeof(t_xcc_lmd_header);
	for (auto& i : m_index)
	{
		strcpy(reinterpret_cast<char*>(w), i.c_str());
		w += i.length() + 1;
	}
	return data;
}