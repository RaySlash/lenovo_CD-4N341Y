/*
 * include/linux/amlogic/media/sound/iec_info.h
 *
 * Copyright (C) 2017 Amlogic, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */

#ifndef __IEC_INFO_H__
#define __IEC_INFO_H__

#include <sound/soc.h>
#include <sound/pcm.h>
#include <sound/control.h>

/* info for iec60958/iec60937 */

enum aud_codec_types {
	AUD_CODEC_TYPE_STEREO_PCM     = 0x0,
	AUD_CODEC_TYPE_DTS_RAW_MODE   = 0x1,
	AUD_CODEC_TYPE_AC3            = 0x2,
	AUD_CODEC_TYPE_DTS            = 0x3,
	AUD_CODEC_TYPE_EAC3           = 0x4,
	AUD_CODEC_TYPE_DTS_HD         = 0x5,
	AUD_CODEC_TYPE_MULTI_LPCM     = 0x6,
	AUD_CODEC_TYPE_TRUEHD         = 0x7,
	AUD_CODEC_TYPE_DTS_HD_MA      = 0x8,
	AUD_CODEC_TYPE_HSR_STEREO_PCM = 0x9
};

static const char * const aud_codec_type_names[] = {
	/* 0 */"2 CH PCM",
	/* 1 */"DTS RAW Mode",
	/* 2 */"Dolby Digital",
	/* 3 */"DTS",
	/* 4 */"Dolby Digital Plus",
	/* 5 */"DTS-HD",
	/* 6 */"Multi-channel LPCM",
	/* 7 */"Dolby TrueHD",
	/* 8 */"DTS-HD MA",
	/* 9 */"HIGH SR Stereo LPCM"
};

extern const struct soc_enum aud_codec_type_enum;

struct iec958_chsts {
	unsigned short chstat0_l;
	unsigned short chstat1_l;
	unsigned short chstat0_r;
	unsigned short chstat1_r;
};

bool codec_is_raw(enum aud_codec_types codec_type);

bool raw_is_4x_clk(enum aud_codec_types codec_type);

void iec_get_channel_status_info(struct iec958_chsts *chsts,
				 enum aud_codec_types codec_type,
				 unsigned int rate);

void spdif_notify_to_hdmitx(struct snd_pcm_substream *substream,
			    enum aud_codec_types codec_type);

#ifdef CONFIG_AMLOGIC_HDMITX
int aml_get_hdmi_out_audio(struct snd_kcontrol *kcontrol,
			   struct snd_ctl_elem_value *ucontrol);
int aml_set_hdmi_out_audio(struct snd_kcontrol *kcontrol,
			   struct snd_ctl_elem_value *ucontrol);
#endif
#endif
