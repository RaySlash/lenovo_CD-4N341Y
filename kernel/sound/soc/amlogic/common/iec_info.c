/*
 * sound/soc/amlogic/common/spdif_info.c
 *
 * Copyright (C) 2018 Amlogic, Inc. All rights reserved.
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
#define DEBUG
#undef pr_fmt
#define pr_fmt(fmt) "iec_info: " fmt

#include <linux/amlogic/media/sound/aout_notify.h>
#include <linux/amlogic/media/sound/iec_info.h>
#ifdef CONFIG_AMLOGIC_HDMITX
#include <linux/amlogic/media/vout/hdmi_tx/hdmi_tx_ext.h>
#endif

const struct soc_enum aud_codec_type_enum =
	SOC_ENUM_SINGLE(SND_SOC_NOPM, 0, ARRAY_SIZE(aud_codec_type_names),
			aud_codec_type_names);

bool codec_is_raw(enum aud_codec_types codec_type)
{
	return ((codec_type != AUD_CODEC_TYPE_STEREO_PCM) &&
		(codec_type != AUD_CODEC_TYPE_HSR_STEREO_PCM));
}

bool raw_is_4x_clk(enum aud_codec_types codec_type)
{
	bool is_4x = false;

	if (codec_type == AUD_CODEC_TYPE_EAC3 ||
	    codec_type == AUD_CODEC_TYPE_DTS_HD ||
	    codec_type == AUD_CODEC_TYPE_TRUEHD ||
	    codec_type == AUD_CODEC_TYPE_DTS_HD_MA) {
		is_4x = true;
	}

	return is_4x;
}

void iec_get_channel_status_info(
	struct iec958_chsts *chsts,
	enum aud_codec_types codec_type,
	unsigned int rate)
{
	int rate_bit = snd_pcm_rate_to_rate_bit(rate);

	if (rate_bit == SNDRV_PCM_RATE_KNOT) {
		pr_err("Unsupport sample rate\n");
		return;
	}

	if (codec_is_raw(codec_type)) {
		chsts->chstat0_l = 0x1902;
		chsts->chstat0_r = 0x1902;
		if (codec_type == AUD_CODEC_TYPE_EAC3 ||
		    codec_type == AUD_CODEC_TYPE_DTS_HD) {
			/* DD+ */
			if (rate_bit == SNDRV_PCM_RATE_32000) {
				chsts->chstat1_l = 0x300;
				chsts->chstat1_r = 0x300;
			} else if (rate_bit == SNDRV_PCM_RATE_44100) {
				chsts->chstat1_l = 0xc00;
				chsts->chstat1_r = 0xc00;
			} else {
				chsts->chstat1_l = 0Xe00;
				chsts->chstat1_r = 0Xe00;
			}
		} else if (codec_type == AUD_CODEC_TYPE_TRUEHD ||
			   codec_type == AUD_CODEC_TYPE_DTS_HD_MA) {
			/* True HD, MA */
			chsts->chstat1_l = 0x900;
			chsts->chstat1_r = 0x900;
		} else {
			/* DTS,DD */
			if (rate_bit == SNDRV_PCM_RATE_32000) {
				chsts->chstat1_l = 0x300;
				chsts->chstat1_r = 0x300;
			} else if (rate_bit == SNDRV_PCM_RATE_44100) {
				chsts->chstat1_l = 0;
				chsts->chstat1_r = 0;
			} else {
				chsts->chstat1_l = 0x200;
				chsts->chstat1_r = 0x200;
			}
		}
	} else {
		chsts->chstat0_l = 0x0100;
		chsts->chstat0_r = 0x0100;
		chsts->chstat1_l = 0x200;
		chsts->chstat1_r = 0x200;

		if (rate_bit == SNDRV_PCM_RATE_44100) {
			chsts->chstat1_l = 0;
			chsts->chstat1_r = 0;
		} else if (rate_bit == SNDRV_PCM_RATE_88200) {
			chsts->chstat1_l = 0x800;
			chsts->chstat1_r = 0x800;
		} else if (rate_bit == SNDRV_PCM_RATE_96000) {
			chsts->chstat1_l = 0xa00;
			chsts->chstat1_r = 0xa00;
		} else if (rate_bit == SNDRV_PCM_RATE_176400) {
			chsts->chstat1_l = 0xc00;
			chsts->chstat1_r = 0xc00;
		} else if (rate_bit == SNDRV_PCM_RATE_192000) {
			chsts->chstat1_l = 0xe00;
			chsts->chstat1_r = 0xe00;
		}
	}
	pr_debug("rate: %d, codec_type:0x%x, channel status L:0x%x, R:0x%x\n",
		 rate,
		 codec_type,
		 ((chsts->chstat1_l >> 8) & 0xf) << 24 | chsts->chstat0_l,
		 ((chsts->chstat1_r >> 8) & 0xf) << 24 | chsts->chstat0_r);
}

void spdif_notify_to_hdmitx(struct snd_pcm_substream *substream,
			    enum aud_codec_types codec_type)
{
	if (codec_type == AUD_CODEC_TYPE_AC3) {
		aout_notifier_call_chain(
			AOUT_EVENT_RAWDATA_AC_3,
			substream);
	} else if (codec_type == AUD_CODEC_TYPE_DTS) {
		aout_notifier_call_chain(
			AOUT_EVENT_RAWDATA_DTS,
			substream);
	} else if (codec_type == AUD_CODEC_TYPE_EAC3) {
		aout_notifier_call_chain(
			AOUT_EVENT_RAWDATA_DOBLY_DIGITAL_PLUS,
			substream);
	} else if (codec_type == AUD_CODEC_TYPE_DTS_HD) {
		aout_notifier_call_chain(
			AOUT_EVENT_RAWDATA_DTS_HD,
			substream);
	} else if (codec_type == AUD_CODEC_TYPE_TRUEHD) {
		aout_notifier_call_chain(
			AOUT_EVENT_RAWDATA_MAT_MLP,
			substream);
	} else if (codec_type == AUD_CODEC_TYPE_DTS_HD_MA) {
		aout_notifier_call_chain(
			AOUT_EVENT_RAWDATA_DTS_HD_MA,
			substream);
	} else {
		aout_notifier_call_chain(
			AOUT_EVENT_IEC_60958_PCM,
			substream);
	}
}

#ifdef CONFIG_AMLOGIC_HDMITX
unsigned int aml_audio_hdmiout_mute_flag;
/* call HDMITX API to enable/disable internal audio out */
int aml_get_hdmi_out_audio(struct snd_kcontrol *kcontrol,
			   struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = !hdmitx_ext_get_audio_status();

	aml_audio_hdmiout_mute_flag =
			ucontrol->value.integer.value[0];
	return 0;
}

int aml_set_hdmi_out_audio(struct snd_kcontrol *kcontrol,
			   struct snd_ctl_elem_value *ucontrol)
{
	bool mute = ucontrol->value.integer.value[0];

	if (aml_audio_hdmiout_mute_flag != mute) {
		hdmitx_ext_set_audio_output(!mute);
		aml_audio_hdmiout_mute_flag = mute;
	}
	return 0;
}
#endif
