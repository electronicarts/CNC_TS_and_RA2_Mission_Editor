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
#include "fname.h"

Cfname::Cfname(const string& s)
{
	*this = string_view(s);
}

string Cfname::get_fname() const
{
	return title + ext;
}

string Cfname::get_ftitle() const
{
	return title;
}

string Cfname::get_fext() const
{
	return ext;
}

string Cfname::get_path() const
{
	return path;
}

string Cfname::get_all() const
{
	return path + title + ext;
}

/*
void Cfname::expand()
{
	char t[MAX_PATH];
	_fullpath(t, get_all().c_str(), MAX_PATH);
	Cfname(<string>(t));
}
*/

Cfname GetModuleFileName(HMODULE hModule)
{
	char s[MAX_PATH];
	if (GetModuleFileNameA(hModule, s, MAX_PATH))
		return Cfname(s);
	return {};
}

string get_temp_path()
{
	char temp_dir[MAX_PATH];
	return GetTempPathA(MAX_PATH, temp_dir) ? temp_dir : ".\\";
}

string get_temp_fname(string path)
{
	char temp_fname[MAX_PATH];
	return GetTempFileNameA(path.c_str(), "XCC", 0, temp_fname) ? temp_fname : "";
}

string get_temp_fname()
{
	return get_temp_fname(get_temp_path());
}

void Cfname::set_title(string_view s)
{
	title = s;
}

void Cfname::set_ext(string_view s)
{
	ext = s;
}

void Cfname::use_default_ext(string_view s)
{
	if (ext == "")
		ext = s;
}

void Cfname::set_path(string_view s)
{
	path = s;
	if (!path.empty() && path[path.length() - 1] != '\\')
		path += '\\';
}

void Cfname::use_default_path(string_view s)
{
	if (path.empty())
		set_path(s);
}

void Cfname::make_path()
{
	if ((title + ext).empty())
		return;
	path += title + ext + '\\';
	title = ext = "";
}

bool Cfname::exists() const
{
	HANDLE h;
	WIN32_FIND_DATAA d;
	h = FindFirstFileA(get_all().c_str(), &d);
	if (h == INVALID_HANDLE_VALUE)
		return false;
	FindClose(h);
	return true;
}

const Cfname& Cfname::operator=(string_view s)
{
	long p1 = s.rfind('\\');
	long p2 = s.rfind('.');
	char t[MAX_PATH];
	if (p1 != string::npos)
	{
		//copy last \ also
		t[s.copy(t, p1 + 1)] = 0;
		path = t;
	}
	t[s.copy(t, p2 - p1 - 1, p1 + 1)] = 0;
	title = t;
	if (p2 != s.npos && p1 < p2)
	{
		t[s.copy(t, s.npos, p2)] = 0;
		ext = t;
	}
	return *this;
}

string operator+(const string& a, const Cfname& b)
{
	return a + static_cast<string>(b);
}

int create_dir(const string& dir)
{
	return !CreateDirectoryA(dir.c_str(), NULL);
}

void create_deep_dir(string dir, const string& name)
{
	int a = 0;
	int b;
	while ((b = name.find_first_of("/\\", a)) != string::npos)
	{
		dir += '/' + name.substr(a, b - a);
		create_dir(dir);
		a = b + 1;
	}
}

int copy_file(string s, string d)
{
	return !CopyFileA(s.c_str(), d.c_str(), false);
}

int delete_file(string fname)
{
	return !DeleteFileA(fname.c_str());
}

int move_file(string s, string d)
{
	return !MoveFileA(s.c_str(), d.c_str());
}

bool fname_filter(const string& fname, const string& filter)
{
	size_t i;
	for (i = 0; i < filter.size(); i++)
	{
		char c = filter[i];
		if (c == '*')
		{
			if (filter.find('*', i + 1) == string::npos)
			{
				int j = fname.length() - filter.length() + 1;
				return j < 0 ? false : fname_filter(fname.substr(i + j), filter.substr(i + 1));
			}
			// for (int j = 0; j < min(fname.length(), filter.length()) - i; j++)
			for (size_t j = 0; j < fname.size(); j++)
			{
				if (fname_filter(fname.substr(i + j), filter.substr(i + 1)))
					return true;
			}
			return false;
		}
		if (c != '?' && c != fname[i])
			return false;
	}
	return fname.length() == i;
}
