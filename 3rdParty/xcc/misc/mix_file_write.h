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

#pragma once

#include "cc_structures.h"
#include <virtual_binary.h>
#include "xcc_lmd_file_write.h"

class Cmix_file_write  
{
public:
	void add_file(int id, const Cvirtual_binary d);
	void add_file(string name, const Cvirtual_binary d);
	void clear();
	Cvirtual_binary write();
	Cmix_file_write(t_game game);
private:
	using t_index = map<int, Cvirtual_binary>;

	int write_start();
	int write(byte* d) const;

	t_game m_game;
	t_index m_index;
	Cxcc_lmd_file_write m_lmd_fw;
};
