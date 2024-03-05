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

#include "xcc_file_sh.h"

class Cxcc_lmd_file : public Cxcc_file_sh<t_xcc_lmd_header>
{
public:
	bool is_valid() const
	{
		return Cxcc_file_sh<t_xcc_lmd_header>::is_valid() && get_ft() == xcc_ft_lmd;
	}

	t_game get_game() const
	{
		return static_cast<t_game>(header().game);
	}

	int get_c_fnames() const
	{
		return header().c_fnames;
	}

	const char* get_fnames() const
	{
		return reinterpret_cast<const char*>(data() + sizeof(t_xcc_header) + sizeof(t_xcc_lmd_header));
	}
};
