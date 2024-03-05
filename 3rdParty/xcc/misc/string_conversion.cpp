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
#include "string_conversion.h"

#include <ctime>

using namespace std;

int a2ip(const string& s)
{
	int r = 0;
	int l = 0;
	for (int i = 0; i < 3; i++)
	{
		int c = s.find('.', l);
		r = r << 8 | atoi(s.substr(l, c - l).c_str());
		l = c + 1;
	}
	r = r << 8 | atoi(s.substr(l).c_str());
	return r;
}

string ip2a(int v)
{
	return n(v >> 24 & 0xff) + '.' + n(v >> 16 & 0xff) + '.' + n(v >> 8 & 0xff) + '.' + n(v & 0xff);
}

int get_net_mask(int v)
{
	if (v & 0x80000000)
		return 0xffffff00;
	if (v & 0x40000000)
		return 0xffff0000;
	return 0xff000000;
}

int get_net_part(int v)
{
	return v & get_net_mask(v);
}

bool atob(string s)
{
	return s == "true" || s == "yes";
}

string btoa(bool v)
{
	return v ? "yes" : "no";
}

string n(unsigned int v)
{
	return to_string(v);
}

string n(int v)
{
	return to_string(v);
}

string n(long long v)
{
	return to_string(v);
}

string n(unsigned long long v)
{
	return to_string(v);
}

string swsl(int l, string s)
{
	while (s.size() < l)
		s = ' ' + s;
	return s;
}

string swsr(int l, string s)
{
	while (s.size() < l)
		s += ' ';
	return s;
}

string nwzl(int l, unsigned int v)
{
	string s = n(v);
	while (s.size() < l)
		s = '0' + s;
	return s;
}

string nwsl(int l, unsigned int v)
{
	return swsl(l, n(v));
}

string nh(int l, __int64 v)
{
	string s;
	int w;
	while (l)
	{
		w = v & 0xf;
		if (w > 9)
			w += 7;
		s = char(w + 48) + s;
		v >>= 4;
		l--;
	}
	return s;
}

string nwp(int l, unsigned int v)
{
	string r;
	string s = n(v);
	while (1)
	{		
		int l = s.size();
		if (l > 3)
		{
			r = '.' + s.substr(l - 3, 3) + r;
			s.erase(l - 3, 3);
		}
		else
		{
			r = s + r;
			break;
		}

	}
	return swsl(l, r);
}

void split_key(const string& key, string& name, string& value)
{
	size_t i = key.find('=');
	if (i == string::npos)
	{
		name = key;
		value.erase();
	}
	else
	{
		name = key.substr(0, i);
		value = key.substr(i + 1);
	}
}

string tabs2spaces(const string& v)
{
	string r;
	for (size_t i = 0; i < v.size(); i++)
	{
		char c = v[i];
		if (c == '\t')
		{
			do
				r += ' ';
			while (r.size() & 3);
		}
		else
			r += c;
	}
	return r;
}

string time2a(time_t v)
{
	char b[20];
	const tm* date = gmtime(&v);
	if (date)
		sprintf(b, "%04d-%02d-%02d %02d:%02d:%02d", date->tm_year + 1900, date->tm_mon + 1, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec);
	else
		*b = 0;
	return b;
}

string js_encode(const string& v)
{
	string r;
	for (size_t i = 0; i < v.size(); i++)
	{
		switch (v[i])
		{
		case '<':
			r += "&lt;";
			break;
		case '\"':
		case '\'':
		case '\\':
			r += '\\';
		default:
			r += v[i];
		}
	}
	return r;
}

string trim_field(const string& v)
{
	string r;
	bool copy_white = false;
	for (size_t i = 0; i < v.size(); i++)
	{
		if (isspace(v[i]))
			copy_white = true;
		else 
		{
			if (copy_white)
			{
				if (!r.empty())
					r += ' ';				
				copy_white = false;
			}
			r += v[i];
		}
	}
	return r;
}

string trim_text(const string& v)
{
	string r;
	bool copy_white = false;
	for (size_t i = 0; i < v.size(); )
	{
		int p = v.find('\n', i);
		if (p == string::npos)
			p = v.size();
		string line = trim_field(v.substr(i, p - i));
		if (line.empty())
			copy_white = true;
		else
		{
			if (copy_white)
			{
				if (!r.empty())
					r += '\n';
				copy_white = false;
			}
			r += line + '\n';
		}
		i = p + 1;
	}
	return r;
}
