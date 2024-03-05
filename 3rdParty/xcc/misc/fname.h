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

#include <string>
#include <windows.h>

using namespace std;

class Cfname
{
private:
	string path;
	string title;
	string ext;
public:
	const Cfname& operator=(string_view);
	bool exists() const;
	void make_path();
	void use_default_ext(string_view);
	void use_default_path(string_view);
	void set_ext(string_view);
	void set_path(string_view);
	void set_title(string_view);
	void expand();
	string get_all() const;
	string get_path() const;
	string get_fname() const;
	string get_ftitle() const;
	string get_fext() const;
	Cfname() = default;
	Cfname(const string&);

	operator string() const
	{
		return get_all();
	}
};

Cfname GetModuleFileName(HMODULE hModule = 0);
bool fname_filter(const string& fname, const string& filter);
int create_dir(const string&  dir);
void create_deep_dir(string dir, const string& name);
int delete_file(string fname);
int copy_file(string s, string d);
int move_file(string s, string d);
string get_temp_fname(string path);
string get_temp_path();
string get_temp_fname();
string operator+(const string& a, const Cfname& b);
