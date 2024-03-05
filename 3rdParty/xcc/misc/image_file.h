/*
    XCC Utilities and Library
    Copyright (C) 2001  Olaf van der Spek  <olafvdspek@gmail.com>

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
#include "palet.h"
#include "video_file.h"
#include "virtual_file.h"
#include "virtual_image.h"

template <class T>
class Cimage_file : public Cvideo_file<T>
{
public:
	virtual void decode(void*) const = 0;
	
#ifndef XCC_MINIMAL_BUILD
	virtual Cvirtual_image vimage() const
	{
		Cvirtual_binary image;
		decode(image.write_start(this->cb_image()));
		return Cvirtual_image(image, this->cx(), this->cy(), this->cb_pixel(), this->palet());
	}
#endif
	
	int cf() const override
	{
		return 1;
	}
};

#ifndef XCC_MINIMAL_BUILD
int image_file_write(Cvirtual_file&, t_file_type, const byte* image, const t_palet_entry*, int cx, int cy);
Cvirtual_file image_file_write(t_file_type, const byte* image, const t_palet_entry*, int cx, int cy);
int image_file_write(const string& name, t_file_type, const byte* image, const t_palet_entry*, int cx, int cy);
#endif
