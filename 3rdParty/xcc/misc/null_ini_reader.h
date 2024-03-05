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

#include "ini_reader.h"

class Cnull_ini_reader : public Cini_reader  
{
public:
	int process_section_start(string_view) override
	{
		mc_sections++;
		return 0;
	}

	bool process_section() const override
	{
		return true;
	}
	
	int process_key(string_view, string_view) override
	{
		return 0;
	}

	bool is_valid() const 
	{
		return mc_sections;
	}
private:
	int mc_sections = 0;
};
