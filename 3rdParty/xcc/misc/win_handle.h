/*
    XCC Utilities and Library
    Copyright (C) 2002  Olaf van der Spek  <olafvdspek@gmail.com>

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

#include <memory>
#include <string>
#include <windows.h>

template<class T>
class Cwin_handle_base
{
public:
	Cwin_handle_base() = default;

	explicit Cwin_handle_base(T h) : m_source(h == INVALID_HANDLE_VALUE ? NULL : h, CloseHandle)
	{
	}

	void clear()
	{
		m_source.reset();
	}

	operator T() const
	{
		return static_cast<T>(m_source.get());
	}
private:
	std::shared_ptr<void> m_source;
};

using Cwin_handle = Cwin_handle_base<HANDLE>;

int create_process(const std::string& exe_name, const std::string& _cmd_line = "", bool wait = false);
