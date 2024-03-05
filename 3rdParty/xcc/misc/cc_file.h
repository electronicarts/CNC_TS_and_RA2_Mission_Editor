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

#include "file32.h"
#include <virtual_binary.h>

enum t_file_type
{
	ft_aud,
	ft_avi,
	ft_big,
	ft_bin,
	ft_bink,
	ft_bmp,
	ft_clipboard,
	ft_csf,
	ft_cps,
	ft_csv,
	ft_dds,
	ft_dir,
	ft_drive,
	ft_fnt,
	ft_html,
	ft_hva,
	ft_ini,
	ft_jpeg,
	ft_jpeg_single,
	ft_map_dune2,
	ft_map_td,
	ft_map_ra,
	ft_map_ts,
	ft_map_ts_preview,
	ft_map_ra2,
	ft_mix,
	ft_mix_rg,
	ft_mng,
	ft_mp3,
	ft_mrf, 
	ft_ogg,
	ft_pak,
	ft_pal,
	ft_pal_jasc,
	ft_pcx_single,
	ft_pcx,
	ft_png_single,
	ft_png,
	ft_pkt_ts,
	ft_riff,
	ft_rules_ini_ts,
	ft_rules_ini_ra2,
	ft_shp_dune2,
	ft_shp,
	ft_shp_ts, 
	ft_sound_ini_ts,
	ft_sound_ini_ra2,
	ft_st,
	ft_text,
	ft_tga,
	ft_tga_single,
	ft_theme_ini_ts,
	ft_theme_ini_ra2,
	ft_tmp,
	ft_tmp_ra,
	ft_tmp_ts,
	ft_voc,
	ft_vpl,
	ft_vqa,
	ft_vqp,
	ft_vxl,
	ft_w3d,
	ft_wav, 
	ft_wav_ima_adpcm,
	ft_wav_pcm,
	ft_wsa_dune2,
	ft_wsa,
	ft_xcc_lmd,
	ft_xcc_unknown,
	ft_xif,
	ft_zip,
	ft_unknown,
	ft_count
};

const char* ft_name[];

class Cmix_file;

class Ccc_file  
{
public:
	const Cwin_handle& h();
	int open(const Cwin_handle& h);
	int open(unsigned int id, Cmix_file& mix_f);
	int open(const string& name, Cmix_file& mix_f);
	int open(const string& name);
	void load(const Cvirtual_binary& d, int size = -1);
	void load(const Ccc_file& f);
	t_file_type get_file_type(bool fast = true);
	int read();
	int read(void* data, int size);
	int extract(const string& name);
	virtual void close();
	Ccc_file(bool read_on_open);

	const byte* begin() const
  {
    return m_data.begin();
  }

	const byte* end() const
  {
    return m_data.end();
  }

	const byte* data() const
  {
    return m_data.data();
  }

  long long size() const
  {
    return m_size;
  }

	const byte* get_data() const
	{
		return m_data.data();
	}

	const byte* data_end() const
	{
		return m_data.data_end();
	}

	Cvirtual_binary vdata() const
	{
		return m_data;
	}

	int get_p() const
	{
		return m_p;
	}

	long long get_size() const
	{
		return m_size;
	}

	bool is_open() const
	{
		return m_is_open;
	}

	void seek(int p)
	{
		m_p = p;
	}

	void skip(int p)
	{
		m_p += p;
	}

	virtual int post_open()
	{
		return 0;
	}
protected:
	int m_offset;
private:
	Cvirtual_binary m_data;
	Cfile32 m_f;
	bool m_is_open = false;
	int m_p;
	const bool m_read_on_open;
	long long m_size;
};
