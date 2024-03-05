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

int a2ip(const std::string&);
std::string ip2a(int);
int get_net_mask(int);
int get_net_part(int);
bool atob(std::string);
std::string btoa(bool);
std::string js_encode(const std::string&);
std::string n(long long);
std::string swsl(int l, std::string);
std::string swsr(int l, std::string);
std::string nwp(int l, unsigned int);
std::string nwsl(int l, unsigned int);
std::string nwzl(int l, unsigned int);
std::string nh(int l, long long v);
void split_key(const std::string& key, std::string& name, std::string& value);
std::string tabs2spaces(const std::string&);
std::string time2a(time_t);
std::string trim_field(const std::string&);
std::string trim_text(const std::string&);
