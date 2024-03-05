/*
    XCC Utilities and Library
    Copyright (C) 2004  Olaf van der Spek  <olafvdspek@gmail.com>

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

#include "cc_file_sh.h"
#include "palet.h"
#include "video_decoder.h"

template <class T>
class Cvideo_file : public Ccc_file_sh<T>
{
public:
	virtual int cb_pixel() const = 0;
	virtual int cf() const = 0;
	virtual int cx() const = 0;
	virtual int cy() const = 0;

	int cb_image() const
	{
		return cb_pixel() * cx() * cy();
	}

	int cb_video() const
	{
		return cf() * cb_image();
	}

	virtual const t_palet_entry* palet() const
	{
		return NULL;
	}
};
