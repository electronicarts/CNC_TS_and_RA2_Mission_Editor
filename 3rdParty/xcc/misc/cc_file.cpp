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
#include "cc_file.h"

#ifndef XCC_MINIMAL_BUILD
#include "aud_file.h"
#include "avi_file.h"
#include "big_file.h"
#include "bin_file.h"
#include "bink_file.h"
#include "bmp_file.h"
#include "csf_file.h"
#include "cps_file.h"
#include "dds_file.h"
#include "fnt_file.h"
#include "hva_file.h"
#include "id_log.h"
#include "jpeg_file.h"
#include "map_td_ini_reader.h"
#include "map_ra_ini_reader.h"
#include "map_ts_ini_reader.h"
#include "mix_file.h"
#include "mix_rg_file.h"
#include "mp3_file.h"
#include "null_ini_reader.h"
#include "ogg_file.h"
#include "pak_file.h"
#include "pal_file.h"
#include "pcx_file.h"
#include "png_file.h"
#include "pkt_ts_ini_reader.h"
#include "shp_dune2_file.h"
#include "shp_file.h"
#include "shp_ts_file.h"
#include "sound_ts_ini_reader.h"
#include "st_file.h"
#include "string_conversion.h"
#include "text_file.h"
#include "tga_file.h"
#include "theme_ts_ini_reader.h"
#include "tmp_file.h"
#include "tmp_ra_file.h"
#include "tmp_ts_file.h"
#include "virtual_tfile.h"
#include "voc_file.h"
#include "vqa_file.h"
#include "vqp_file.h"
#include "vxl_file.h"
#include "w3d_file.h"
#include "wav_file.h"
#include "wsa_dune2_file.h"
#include "wsa_file.h"
#include "xcc_dirs.h"
#include "xcc_file.h"
#include "xcc_lmd_file.h"
#include "xif_file.h"
#else
#include "mix_file.h"
#include "xcc_dirs.h"
#endif


const char* ft_name[] =
{
	"aud",
	"avi",
	"big",
	"bin",
	"bink",
	"bmp",
	"clipboard",
	"csf",
	"cps",
	"csv",
	"dds",
	"dir",
	"drive",
	"fnt",
	"html",
	"hva",
	"ini",
	"jpeg",
	"jpeg (single)",
	"map (dune2)",
	"map (td)",
	"map (ra)",
	"map (ts)",
	"map (ts) preview",
	"map (ra2)",
	"mix",
	"mix (rg)",
	"mng",
	"mp3",
	"mrf",
	"ogg",
	"pak",
	"pal",
	"pal (jasc)",
	"pcx (single)",
	"pcx",
	"png (single)",
	"png",
	"pkt (ts)",
	"riff",
	"rules ini (ts)",
	"rules ini (ra2)",
	"shp (dune2)",
	"shp",
	"shp (ts)",
	"sound ini (ts)",
	"sound ini (ra2)",
	"string table",
	"text",
	"tga",
	"tga (single)",
	"theme ini (ts)",
	"theme ini (ra2)",
	"tmp",
	"tmp (ra)",
	"tmp (ts)",
	"voc",
	"vpl",
	"vqa",
	"vqp",
	"vxl",
	"w3d",
	"wav",
	"pcm wav",
	"ima adpcm wav",
	"wsa (dune2)",
	"wsa",
	"xcc lmd",
	"xcc unknown",
	"xif",
	"zip",
	"unknown"
};

Ccc_file::Ccc_file(bool read_on_open) :
	m_read_on_open(read_on_open)
{
}

#define test_fail(res) { int v = res; if (v) { close(); return v; }}

#ifndef NO_MIX_SUPPORT
	int Ccc_file::open(unsigned int id, Cmix_file& mix_f)
	{
		if (mix_f.get_index(id) == -1)
			return -1;
		m_f = mix_f.m_f;
		m_offset = mix_f.m_offset + mix_f.get_offset(id);
		m_size = mix_f.get_size(id);
		m_p = 0;
		m_is_open = true;
		if (m_read_on_open || mix_f.data())
			m_data = mix_f.get_vdata(id);
		test_fail(post_open())
			return 0;
	}

	int Ccc_file::open(const string& name, Cmix_file& mix_f)
	{
		return open(Cmix_file::get_id(mix_f.get_game(), name), mix_f);
	}
#endif

	int Ccc_file::open(const string& name)
	{
		close();
		assert(!is_open());
#ifdef NO_MIX_SUPPORT
		test_fail(m_f.open_read(name));
#else
		test_fail(m_f.open_read(xcc_dirs::find_file(name)));
#endif
		m_offset = 0;
		m_size = m_f.size();
		m_p = 0;
		m_is_open = true;
		m_data = m_f.get_mm();
		if (m_read_on_open)
			m_f.close();
#ifndef NO_FT_SUPPORT
		Cfname fname = to_lower_copy(name);
		if (fname.get_fext() == ".mmx")
		{
			fname.set_ext(".map");
			mix_database::add_name(game_ra2, fname.get_fname(), "-");
			fname.set_ext(".pkt");
			mix_database::add_name(game_ra2, fname.get_fname(), "-");
		}
#endif
		test_fail(post_open())
			return 0;
	}

	const Cwin_handle& Ccc_file::h()
	{
		assert(is_open());
		return m_f.h();
	}

	int Ccc_file::open(const Cwin_handle& h)
	{
		assert(!is_open());
		m_f.open(h);
		m_offset = 0;
		m_size = m_f.size();
		m_p = 0;
		m_is_open = true;
		m_data = m_f.get_mm();
		if (m_read_on_open)
			m_f.close();
		test_fail(post_open())
		return 0;
	}

	void Ccc_file::load(const Cvirtual_binary& d, int size)
	{
		m_data = d;
		m_offset = 0;
		m_is_open = true;
		m_p = 0;
		m_size = size == -1 ? d.size() : size;
		post_open();
	}

	void Ccc_file::load(const Ccc_file& f)
	{
		load(f.vdata());
	}

	int Ccc_file::read()
	{
		seek(0);
		Cvirtual_binary d;
		if (int error = read(d.write_start(get_size()), get_size()))
			return error;
		m_data = d;
		return 0;
	}

	int Ccc_file::read(void* data, int size)
	{
		if (get_p() < 0 || get_p() + size > get_size())
			return 1;
		if (get_data())
		{
			memcpy(data, m_data.data() + m_p, size);
			skip(size);
			return 0;
		}
		assert(is_open());
		m_f.seek(m_offset + m_p);
		int res = m_f.read(data, size);
		if (!res)
			skip(size);
		return res;
	}

	int Ccc_file::extract(const string& name)
	{
		assert(is_open());
		if (data())
			return vdata().save(name);
		seek(0);
		Cfile32 f;
		if (int error = f.open_write(name))
			return error;
		Cvirtual_binary data;
		for (int size = get_size(); size; )
		{
			int cb_write = min(size, 1 << 20);
			if (int error = read(data.write_start(cb_write), cb_write))
				return error;
			if (int error = f.write(data.data(), cb_write))
				return error;
			size -= cb_write;
		}
		return 0;
	}

	void Ccc_file::close()
	{
		m_data.clear();
		m_f.close();
		m_is_open = false;
	}


#ifndef NO_MIX_SUPPORT
#ifndef NO_FT_SUPPORT
	t_file_type Ccc_file::get_file_type(bool fast)
	{
		Cvirtual_binary data;
		int size;
		if (m_data.data())
		{
			data = m_data;
			size = m_size;
		}
		else
		{
			size = min<int>(m_size, 64 << 10);
			seek(0);
			if (read(data.write_start(size), size))
				return ft_unknown;
			seek(0);
		}
		Caud_file aud_f;
		Cbin_file bin_f;
		Cbink_file bink_f;
		Cbig_file big_f;
		Cbmp_file bmp_f;
		Ccsf_file csf_f;
		Ccps_file cps_f;
		Cdds_file dds_f;
		Cfnt_file fnt_f;
		Chva_file hva_f;
		Cmix_file mix_f;
		Cmix_rg_file mix_rg_f;
		Cmp3_file mp3_f;
		Cjpeg_file jpeg_f;
		Cogg_file ogg_f;
		Cpak_file pak_f;
		Cpal_file pal_f;
		Cpcx_file pcx_f;
		Cpng_file png_f;
		Criff_file riff_f;
		Cshp_dune2_file shp_dune2_f;
		Cshp_file shp_f;
		Cshp_ts_file shp_ts_f;
		Cst_file st_f;
		Ctext_file text_f;
		Ctga_file tga_f;
		Ctmp_file tmp_f;
		Ctmp_ra_file tmp_ra_f;
		Ctmp_ts_file tmp_ts_f;
		Cvoc_file voc_f;
		Cvqa_file vqa_f;
		Cvqp_file vqp_f;
		Cvxl_file vxl_f;
		Cw3d_file w3d_f;
		Cwsa_dune2_file wsa_dune2_f;
		Cwsa_file wsa_f;
		Cxcc_file xcc_f;
		Cxif_file xif_f;
		if (aud_f.load(data, m_size), aud_f.is_valid())
			return ft_aud;
		if (big_f.load(data, m_size), big_f.is_valid())
			return ft_big;
		if (bin_f.load(data, m_size), bin_f.is_valid())
			return ft_bin;
		if (bink_f.load(data, m_size), bink_f.is_valid())
			return ft_bink;
		if (bmp_f.load(data, m_size), bmp_f.is_valid())
			return ft_bmp;
		if (csf_f.load(data, m_size), csf_f.is_valid())
			return ft_csf;
		if (cps_f.load(data, m_size), cps_f.is_valid())
			return ft_cps;
		if (dds_f.load(data, m_size), dds_f.is_valid())
			return ft_dds;
		if (fnt_f.load(data, m_size), fnt_f.is_valid())
			return ft_fnt;
		if (hva_f.load(data, m_size), hva_f.is_valid())
			return ft_hva;
		if (mp3_f.load(data, m_size), mp3_f.is_valid())
			return ft_mp3;
		if (jpeg_f.load(data, m_size), jpeg_f.is_valid())
			return ft_jpeg;
		if (ogg_f.load(data, m_size), ogg_f.is_valid())
			return ft_ogg;
		if (pal_f.load(data, m_size), pal_f.is_valid())
			return ft_pal;
		if (pcx_f.load(data, m_size), pcx_f.is_valid())
			return ft_pcx;
		if (png_f.load(data, m_size), png_f.is_valid())
			return ft_png;
		if (riff_f.load(data, m_size), riff_f.is_valid())
		{
			Cavi_file avi_f;
			Cwav_file wav_f;
			avi_f.load(data, m_size);
			wav_f.load(data, m_size);
			if (avi_f.is_valid())
				return ft_avi;
			if (wav_f.is_valid())
				return ft_wav;
			return ft_riff;
		}
		if (shp_dune2_f.load(data, m_size), shp_dune2_f.is_valid())
			return ft_shp_dune2;
		if (shp_f.load(data, m_size), shp_f.is_valid())
			return ft_shp;
		if (shp_ts_f.load(data, m_size), shp_ts_f.is_valid())
			return ft_shp_ts;
		if (st_f.load(data, m_size), st_f.is_valid())
			return ft_st;
		if (tga_f.load(data, m_size), tga_f.is_valid())
			return ft_tga;
		if (tmp_f.load(data, m_size), tmp_f.is_valid())
			return ft_tmp;
		if (tmp_ra_f.load(data, m_size), tmp_ra_f.is_valid())
			return ft_tmp_ra;
		if (tmp_ts_f.load(data, m_size), tmp_ts_f.is_valid())
			return ft_tmp_ts;
		if (voc_f.load(data, m_size), voc_f.is_valid())
			return ft_voc;
		if (vqa_f.load(data, m_size), vqa_f.is_valid())
			return ft_vqa;
		if (vqp_f.load(data, m_size), vqp_f.is_valid())
			return ft_vqp;
		if (vxl_f.load(data, m_size), vxl_f.is_valid())
			return ft_vxl;
		if (wsa_dune2_f.load(data, m_size), wsa_dune2_f.is_valid())
			return ft_wsa_dune2;
		if (wsa_f.load(data, m_size), wsa_f.is_valid())
			return ft_wsa;
		if (xcc_f.load(data, m_size), xcc_f.is_valid())
		{
			switch (xcc_f.get_ft())
			{
			case xcc_ft_lmd:
				return ft_xcc_lmd;
			}
			return ft_xcc_unknown;
		}
		if (xif_f.load(data, m_size), xif_f.is_valid())
			return ft_xif;
		if (mix_f.load(data, m_size), mix_f.is_valid())
			return ft_mix;
		if (mix_rg_f.load(data, m_size), mix_rg_f.is_valid())
			return ft_mix_rg;
		if (pak_f.load(data, m_size), pak_f.is_valid())
			return ft_pak;
		if (w3d_f.load(data, m_size), w3d_f.is_valid())
			return ft_w3d;
		if (text_f.load(data, m_size), text_f.is_valid())
		{
			if (fast)
				return ft_text;
			Cvirtual_tfile tf;
			tf.load_data(data);
			Cnull_ini_reader ir;
			int error = 0;
			while (!error && !tf.eof())
			{
				error = ir.process_line(tf.read_line());
				if (tf.eof())
					error = 0;
			}
			if (!error && ir.is_valid())
			{
				if (!m_read_on_open && m_size != size)
				{
					size = m_size;
					if (read(data.write_start(size), size))
						return ft_unknown;
					seek(0);
				}
				{
					Cmap_td_ini_reader ir;
					if (!ir.process(data) && ir.is_valid())
						return ft_map_td;
				}
				{
					Cmap_ra_ini_reader ir;
					if (!ir.process(data) && ir.is_valid())
						return ft_map_ra;
				}
				{
					Cmap_ts_ini_reader ir;
					ir.fast(true);
					if (!ir.process(data) && ir.is_valid())
						return ft_map_ts;
				}
				{
					Cpkt_ts_ini_reader ir;
					ir.fast(true);
					if (!ir.process(data) && ir.is_valid())
						return ft_pkt_ts;
				}
				{
					Csound_ts_ini_reader ir;
					ir.fast(true);
					if (!ir.process(data) && ir.is_valid())
						return ft_sound_ini_ts;
				}
				{
					Ctheme_ts_ini_reader ir;
					if (!ir.process(data) && ir.is_valid())
						return ft_theme_ini_ts;
				}
				return ft_ini;
			}
			return ft_text;
		}
		return ft_unknown;
	}
#endif
#endif
