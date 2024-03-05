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

#include "virtual_file.h"

class Cvirtual_audio  
{
public:
	void load(Cvirtual_file f, int c_samples, int samplerate, int cb_sample, int c_channels);
	Cvirtual_file save_as_aud() const;
	int save_as_aud(string fname) const;
	Cvirtual_file save_as_wav_ima_adpcm() const;
	int save_as_wav_ima_adpcm(string fname) const;
	Cvirtual_file save_as_wav_pcm() const;
	int save_as_wav_pcm(string fname) const;
	Cvirtual_audio();
	~Cvirtual_audio();

	const __int16* audio() const
	{
		return reinterpret_cast<const __int16*>(m_f.data());
	}

	int cb_audio() const
	{
		return mc_samples * mcb_sample * mc_channels;
	}

	int c_samples() const
	{
		return mc_samples;
	}

	int samplerate() const
	{
		return m_samplerate;
	}

	int cb_sample() const
	{
		return mcb_sample;
	}

	int c_channels() const
	{
		return mc_channels;
	}
private:
	Cvirtual_file m_f;
	int mc_samples;
	int m_samplerate;
	int mcb_sample;
	int mc_channels;
};
