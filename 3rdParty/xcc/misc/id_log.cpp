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
#include <id_log.h>

#include <fstream>
#include <mix_file.h>
#include <string_conversion.h>
#include <xcc_dirs.h>

struct t_idinfo
{
	string name;
	string description;
};

using t_id_list = map<int, t_idinfo>;

t_id_list td_list, ra_list, ts_list, dune2_list, ra2_list;

static t_id_list& get_list(t_game game)
{
	switch (game)
	{
	case game_ra:
		return ra_list;
	case game_ts:
		return ts_list;
	case game_dune2:
		return dune2_list;
	case game_ra2:
		return ra2_list;
	}
	return td_list;
}

static void read_list(t_game game, const char*& s)
{
	t_id_list& d = get_list(game);
	int count = *reinterpret_cast<const int*>(s);
	s += 4;
	t_idinfo idinfo;
	while (count--)
	{
		idinfo.name = s;
		s += idinfo.name.length() + 1;
		idinfo.description = s;
		s += idinfo.description.length() + 1;
		d[Cmix_file::get_id(game, idinfo.name)] = idinfo;
	}
}

int mix_database::load()
{
	if (!td_list.empty() || !ra_list.empty() || !ts_list.empty())
		return 0;
	Cvirtual_binary f;
	if (f.load(xcc_dirs::get_data_dir() + "global mix database.dat") || f.size() < 16)
		return 1;
	const char* data = reinterpret_cast<const char*>(f.data());
	read_list(game_td, data);
	read_list(game_ra, data);
	read_list(game_ts, data);
	read_list(game_ra2, data);
	if (0)
	{
		ofstream log_f("c:\\log.txt");
		for (auto& i : ts_list)
			log_f << i.second.name << '\t' << i.second.description << endl;
	}
	return 0;
	char name[12] = "scg00ea.bin";
	const char char1[] = "bgjm";
	const char char2[] = "ew";
	const char char3[] = "abc";
	for (int i = 0; i < 2; i++)
	{
		if (i)
			strcpy(name + 8, "ini");
		for (int j = 0; j < 4; j++)
		{
			name[2] = char1[j];
			for (int k = 0; k < 100; k++)
			{
				memcpy(name + 3, nwzl(2, k).c_str(), 2);
				for (int l = 0; l < 2; l++)
				{
					name[5] = char2[l];
					for (int m = 0; m < 3; m++)
					{
						name[6] = char3[m];
						mix_database::add_name(game_td, name, "");
						mix_database::add_name(game_ra, name, "");
						mix_database::add_name(game_ts, name, "");
					}
				}
			}
		}
	}
	return 0;
}

void mix_database::add_name(t_game game, const string& name, const string& description)
{
	t_idinfo idinfo;
	idinfo.name = name;
	idinfo.description = description;
	get_list(game)[Cmix_file::get_id(game, name)] = idinfo;
}

string mix_database::get_name(t_game game, int id)
{
	auto i = find_ptr(get_list(game), id);
	return i ? i->name : "";
}

string mix_database::get_description(t_game game, int id)
{
	auto i = find_ptr(get_list(game), id);
	return i ? i->description : "";
}
