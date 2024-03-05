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

#include "cc_structures.h"
#include "fname.h"

class xcc_dirs  
{
public:
	static bool enable_log();
	static void load_from_registry();
	static void save_to_registry();
	static string find_file(Cfname s);
	static bool is_available(t_game game);
	static string get_audio_mix(t_game game);
	static string get_csf_fname(t_game game);
	static string get_dir(t_game game);
	static string get_exe(t_game game);
	static string get_ecache_mix(t_game game, bool dir, int i);
	static string get_ecache_mix(t_game game, bool dir, const string& s);
	static string get_expand_mix(t_game game, int i);
	static string get_expand_mix(t_game game, const string& s);
	static string get_language_mix(t_game game);
	static string get_local_mix(t_game game);
	static string get_main_mix(t_game game);
	static void set_td_secondary_dir(const string& s);
	static void set_cd_dir(const string &s);
	static void set_data_dir(const string &s);
	static void set_dir(t_game game, const string& s);
	static void reset_cd_dir();
	static void reset_data_dir();
	static const string& get_td_secondary_dir();
	static const string& get_cd_dir();
	static const string& get_data_dir();
};
