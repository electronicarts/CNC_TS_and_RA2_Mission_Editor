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

#include <vartypes.h>

class Cblowfish
{
public:
	using t_bf_p = uint32_t[18];
	using t_bf_s = uint32_t[4][256];

	void set_key(data_ref);
	void encipher(const void* s, void* d, int size) const;
	void decipher(const void* s, void* d, int size) const;
private:
	void encipher(uint32_t& xl, uint32_t& xr) const;
	void decipher(uint32_t& xl, uint32_t& xr) const;
	uint32_t S(uint32_t x, int i) const;
	uint32_t bf_f(uint32_t x) const;
	void ROUND(uint32_t& a, uint32_t b, int n) const;

	t_bf_p m_p;
	t_bf_s m_s;
};
