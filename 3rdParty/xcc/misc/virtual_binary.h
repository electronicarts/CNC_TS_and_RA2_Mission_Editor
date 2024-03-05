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

#include <cassert>
#include <memory>
#include <string>
#include <vartypes.h>
#include <xcc/data_ref.h>

using namespace std;

class Cvirtual_binary_source
{
public:
	Cvirtual_binary_source(const void* d, size_t cb_d, const std::shared_ptr<const void>& source = NULL);
	Cvirtual_binary_source* attach();
	void detach();
	Cvirtual_binary_source* pre_edit();

	const byte* data() const
	{
		return m_data;
	}

	const byte* data_end() const
	{
		return data() + size();
	}

	byte* data_edit()
	{
		assert(mc_references == 1 && !m_source);
		return m_data;
	}

	size_t size() const
	{
		return m_size;
	}

	void size(size_t v)
	{
		assert(mc_references == 1 && !m_source && v <= m_size);
		m_size = v;
	}

  bool unique() const
  {
    return mc_references == 1;
  }
private:
	byte* m_data;
	size_t m_size;
	int mc_references;
	std::shared_ptr<const void> m_source;
};

class Cvirtual_binary
{
public:
	Cvirtual_binary sub_bin(size_t offset, size_t size) const;
	int save(const string& fname) const;
	int load(const string& fname, bool use_mm = true);
	void clear();
	void memset(int v);
	size_t read(void* d) const;
	byte* write_start(size_t cb_d);
	void write(const void* d, size_t cb_d);
	const Cvirtual_binary& operator=(const Cvirtual_binary& v);
	Cvirtual_binary();
	Cvirtual_binary(const Cvirtual_binary&);
	Cvirtual_binary(data_ref);
	Cvirtual_binary(const void*, size_t);
	Cvirtual_binary(const void*, size_t, const std::shared_ptr<void>& source);
	explicit Cvirtual_binary(const string& fname, bool use_mm = true);
	~Cvirtual_binary();

	const byte* data() const
	{
		return m_source ? m_source->data() : NULL;
	}

	const byte* data_end() const
	{
		return m_source ? m_source->data_end() : NULL;
	}

  const byte* begin() const
  {
    return data();
  }

  const byte* end() const
  {
    return data_end();
  }

	byte* data_edit()
	{
		assert(m_source);
		m_source = m_source->pre_edit();
		return m_source->data_edit();
	}

	byte* mutable_end()
	{
		return data_edit() + size();
	}

	size_t size() const
	{
		return m_source ? m_source->size() : 0;
	}

	void set_size(size_t v)
	{
		assert(m_source);
		m_source = m_source->pre_edit();
		m_source->size(v);
	}
private:
	Cvirtual_binary_source* m_source;
};
