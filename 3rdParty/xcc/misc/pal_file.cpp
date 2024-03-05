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
#include "pal_file.h"

#include <fstream>

bool Cpal_file::is_valid() const
{
	if (get_size() != sizeof(t_palet))
		return false;
	const t_palet_entry* p = get_palet();
	for (int i = 0; i < 256; i++)
	{
		if ((p[i].r | p[i].g | p[i].b) & 0xc0)
			return false;
	}
	return true;
}

ostream& Cpal_file::extract_as_pal_jasc(ostream& os, bool shift_left) const
{
	os << "JASC-PAL" << endl
		<< "0100" << endl
		<< "256" << endl;
	t_palet palet;
	if (shift_left)
		convert_palet_18_to_24(get_palet(), palet);
	else
		memcpy(palet, get_palet(), sizeof(t_palet));
	for (int i = 0; i < 256; i++)
		os << static_cast<int>(palet[i].r) << ' ' << static_cast<int>(palet[i].g) << ' ' << static_cast<int>(palet[i].b) << endl;
	return os;
}
