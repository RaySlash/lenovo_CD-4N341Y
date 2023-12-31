/*
 * include/linux/amlogic/media/registers/regs/vpp_regs.h
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

#ifndef VPP_REGS_HEADER_
#define VPP_REGS_HEADER_


#define VPP_DUMMY_DATA 0x1d00
#define VPP_LINE_IN_LENGTH 0x1d01
#define VPP_PIC_IN_HEIGHT 0x1d02
#define VPP_SCALE_COEF_IDX 0x1d03
#define VPP_SCALE_COEF 0x1d04
#define VPP_VSC_REGION12_STARTP 0x1d05
#define VPP_VSC_REGION34_STARTP 0x1d06
#define VPP_VSC_REGION4_ENDP 0x1d07
#define VPP_VSC_START_PHASE_STEP 0x1d08
#define VPP_VSC_REGION0_PHASE_SLOPE 0x1d09
#define VPP_VSC_REGION1_PHASE_SLOPE 0x1d0a
#define VPP_VSC_REGION3_PHASE_SLOPE 0x1d0b
#define VPP_VSC_REGION4_PHASE_SLOPE 0x1d0c
#define VPP_VSC_PHASE_CTRL 0x1d0d
#define VPP_VSC_INI_PHASE 0x1d0e
#define VPP_HSC_REGION12_STARTP 0x1d10
#define VPP_HSC_REGION34_STARTP 0x1d11
#define VPP_HSC_REGION4_ENDP 0x1d12
#define VPP_HSC_START_PHASE_STEP 0x1d13
#define VPP_HSC_REGION0_PHASE_SLOPE 0x1d14
#define VPP_HSC_REGION1_PHASE_SLOPE 0x1d15
#define VPP_HSC_REGION3_PHASE_SLOPE 0x1d16
#define VPP_HSC_REGION4_PHASE_SLOPE 0x1d17
#define VPP_HSC_PHASE_CTRL 0x1d18
#define VPP_SC_MISC 0x1d19
#define VPP_PREBLEND_VD1_H_START_END 0x1d1a
#define VPP_PREBLEND_VD1_V_START_END 0x1d1b
#define VPP_POSTBLEND_VD1_H_START_END 0x1d1c
#define VPP_POSTBLEND_VD1_V_START_END 0x1d1d
#define VPP_BLEND_VD2_H_START_END 0x1d1e
#define VPP_BLEND_VD2_V_START_END 0x1d1f
#define VPP_PREBLEND_H_SIZE 0x1d20
#define VPP_POSTBLEND_H_SIZE 0x1d21
#define VPP_HOLD_LINES 0x1d22
#define VPP_BLEND_ONECOLOR_CTRL 0x1d23
#define VPP_PREBLEND_CURRENT_XY 0x1d24
#define VPP_POSTBLEND_CURRENT_XY 0x1d25
#define VPP_MISC 0x1d26
#define VPP_OFIFO_SIZE 0x1d27
#define VPP_FIFO_STATUS 0x1d28
#define VPP_SMOKE_CTRL 0x1d29
#define VPP_SMOKE1_VAL 0x1d2a
#define VPP_SMOKE2_VAL 0x1d2b
#define VPP_SMOKE3_VAL 0x1d2c
#define VPP_SMOKE1_H_START_END 0x1d2d
#define VPP_SMOKE1_V_START_END 0x1d2e
#define VPP_SMOKE2_H_START_END 0x1d2f
#define VPP_SMOKE2_V_START_END 0x1d30
#define VPP_SMOKE3_H_START_END 0x1d31
#define VPP_SMOKE3_V_START_END 0x1d32
#define VPP_SCO_FIFO_CTRL 0x1d33
#define VPP_HSC_PHASE_CTRL1 0x1d34
#define VPP_HSC_INI_PAT_CTRL 0x1d35
#define VPP_VADJ_CTRL 0x1d40
#define VPP_VADJ1_Y 0x1d41
#define VPP_VADJ1_MA_MB 0x1d42
#define VPP_VADJ1_MC_MD 0x1d43
#define VPP_VADJ2_Y 0x1d44
#define VPP_VADJ2_MA_MB 0x1d45
#define VPP_VADJ2_MC_MD 0x1d46
#define VPP_HSHARP_CTRL 0x1d50
#define VPP_HSHARP_LUMA_THRESH01 0x1d51
#define VPP_HSHARP_LUMA_THRESH23 0x1d52
#define VPP_HSHARP_CHROMA_THRESH01 0x1d53
#define VPP_HSHARP_CHROMA_THRESH23 0x1d54
#define VPP_HSHARP_LUMA_GAIN 0x1d55
#define VPP_HSHARP_CHROMA_GAIN 0x1d56
#define VPP_MATRIX_PROBE_COLOR 0x1d5c
#define VPP_MATRIX_HL_COLOR 0x1d5d
#define VPP_MATRIX_PROBE_POS 0x1d5e
#define VPP_MATRIX_CTRL 0x1d5f
#define VPP_MATRIX_COEF00_01 0x1d60
#define VPP_MATRIX_COEF02_10 0x1d61
#define VPP_MATRIX_COEF11_12 0x1d62
#define VPP_MATRIX_COEF20_21 0x1d63
#define VPP_MATRIX_COEF22 0x1d64
#define VPP_MATRIX_OFFSET0_1 0x1d65
#define VPP_MATRIX_OFFSET2 0x1d66
#define VPP_MATRIX_PRE_OFFSET0_1 0x1d67
#define VPP_MATRIX_PRE_OFFSET2 0x1d68
#define VPP_DUMMY_DATA1 0x1d69
#define VPP_GAINOFF_CTRL0 0x1d6a
#define VPP_GAINOFF_CTRL1 0x1d6b
#define VPP_GAINOFF_CTRL2 0x1d6c
#define VPP_GAINOFF_CTRL3 0x1d6d
#define VPP_GAINOFF_CTRL4 0x1d6e
#define VPP_CHROMA_ADDR_PORT 0x1d70
#define VPP_CHROMA_DATA_PORT 0x1d71
#define VPP_GCLK_CTRL0 0x1d72
#define VPP_GCLK_CTRL1 0x1d73
#define VPP_SC_GCLK_CTRL 0x1d74
#define VPP_MISC1 0x1d76
#define VPP_BLACKEXT_CTRL 0x1d80
#define VPP_DNLP_CTRL_00 0x1d81
#define VPP_DNLP_CTRL_01 0x1d82
#define VPP_DNLP_CTRL_02 0x1d83
#define VPP_DNLP_CTRL_03 0x1d84
#define VPP_DNLP_CTRL_04 0x1d85
#define VPP_DNLP_CTRL_05 0x1d86
#define VPP_DNLP_CTRL_06 0x1d87
#define VPP_DNLP_CTRL_07 0x1d88
#define VPP_DNLP_CTRL_08 0x1d89
#define VPP_DNLP_CTRL_09 0x1d8a
#define VPP_DNLP_CTRL_10 0x1d8b
#define VPP_DNLP_CTRL_11 0x1d8c
#define VPP_DNLP_CTRL_12 0x1d8d
#define VPP_DNLP_CTRL_13 0x1d8e
#define VPP_DNLP_CTRL_14 0x1d8f
#define VPP_DNLP_CTRL_15 0x1d90
#define VPP_SRSHARP0_CTRL 0x1d91
#define VPP_SRSHARP1_CTRL 0x1d92
#define VPP_PEAKING_NLP_1 0x1d93
/* gxm has no super-core */
#define VPP_DOLBY_CTRL    0x1d93
#define VPP_PEAKING_NLP_2 0x1d94
#define VPP_PEAKING_NLP_3 0x1d95
#define VPP_PEAKING_NLP_4 0x1d96
#define VPP_PEAKING_NLP_5 0x1d97
#define VPP_SHARP_LIMIT 0x1d98
#define VPP_VLTI_CTRL 0x1d99
#define VPP_HLTI_CTRL 0x1d9a
#define VPP_CTI_CTRL 0x1d9b
#define VPP_BLUE_STRETCH_1 0x1d9c
#define VPP_BLUE_STRETCH_2 0x1d9d
#define VPP_BLUE_STRETCH_3 0x1d9e
#define VPP_CCORING_CTRL 0x1da0
#define VPP_VE_ENABLE_CTRL 0x1da1
#define VPP_VE_DEMO_LEFT_TOP_SCREEN_WIDTH 0x1da2
#define VPP_VE_DEMO_CENTER_BAR 0x1da3
#define VPP_VE_H_V_SIZE 0x1da4
#define VPP_PSR_H_V_SIZE 0x1da5
#define VPP_OUT_H_V_SIZE 0x1da5
#define VPP_IN_H_V_SIZE 0x1da6
#define VPP_VDO_MEAS_CTRL 0x1da8
#define VPP_VDO_MEAS_VS_COUNT_HI 0x1da9
#define VPP_VDO_MEAS_VS_COUNT_LO 0x1daa
#define VPP_INPUT_CTRL 0x1dab
#define VPP_CTI_CTRL2 0x1dac
#define VPP_PEAKING_SAT_THD1 0x1dad
#define VPP_PEAKING_SAT_THD2 0x1dae
#define VPP_PEAKING_SAT_THD3 0x1daf
#define VPP_PEAKING_SAT_THD4 0x1db0
#define VPP_PEAKING_SAT_THD5 0x1db1
#define VPP_PEAKING_SAT_THD6 0x1db2
#define VPP_PEAKING_SAT_THD7 0x1db3
#define VPP_PEAKING_SAT_THD8 0x1db4
#define VPP_PEAKING_SAT_THD9 0x1db5
#define VPP_PEAKING_GAIN_ADD1 0x1db6
#define VPP_PEAKING_GAIN_ADD2 0x1db7
#define VPP_PEAKING_DNLP 0x1db8
#define VPP_SHARP_DEMO_WIN_CTRL1 0x1db9
#define VPP_SHARP_DEMO_WIN_CTRL2 0x1dba
#define VPP_FRONT_HLTI_CTRL 0x1dbb
#define VPP_FRONT_CTI_CTRL 0x1dbc
#define VPP_FRONT_CTI_CTRL2 0x1dbd
#define VPP_OSD_VSC_PHASE_STEP 0x1dc0
#define VPP_OSD_VSC_INI_PHASE 0x1dc1
#define VPP_OSD_VSC_CTRL0 0x1dc2
#define VPP_OSD_HSC_PHASE_STEP 0x1dc3
#define VPP_OSD_HSC_INI_PHASE 0x1dc4
#define VPP_OSD_HSC_CTRL0 0x1dc5
#define VPP_OSD_HSC_INI_PAT_CTRL 0x1dc6
#define VPP_OSD_SC_DUMMY_DATA 0x1dc7
#define VPP_OSD_SC_CTRL0 0x1dc8
#define VPP_OSD_SCI_WH_M1 0x1dc9
#define VPP_OSD_SCO_H_START_END 0x1dca
#define VPP_OSD_SCO_V_START_END 0x1dcb
#define VPP_OSD_SCALE_COEF_IDX 0x1dcc
#define VPP_OSD_SCALE_COEF 0x1dcd
#define VPP_INT_LINE_NUM 0x1dce

#define VPP_CLIP_MISC0 0x1dd9
#define VPP_CLIP_MISC1 0x1dda
#define VPP_VD1_CLIP_MISC0 0x1de1
#define VPP_VD1_CLIP_MISC1 0x1de2

#define VPP2_MISC 0x1e26
#define VPP2_OFIFO_SIZE 0x1e27
#define VPP2_INT_LINE_NUM 0x1e20
#define VPP2_OFIFO_URG_CTRL 0x1e21

#define SRSHARP0_SHARP_HVSIZE 0x3200
#define SRSHARP0_SHARP_HVBLANK_NUM 0x3201
#define SRSHARP0_SHARP_PK_NR_ENABLE 0x3227
#define SRSHARP0_SHARP_DNLP_EN 0x3245
#define SRSHARP0_SHARP_SR2_CTRL 0x3257
#define SRSHARP1_SHARP_HVSIZE 0x3280
#define SRSHARP1_SHARP_HVBLANK_NUM 0x3281
#define SRSHARP1_SHARP_PK_NR_ENABLE 0x32a7
#define SRSHARP1_SHARP_DNLP_EN 0x32c5
#define SRSHARP1_SHARP_SR2_CTRL 0x32d7

#define VPP_POST_MATRIX_SAT 0x32c1

/* g12a vd2 pps */
#define VD2_SCALE_COEF_IDX 0x3943
#define VD2_SCALE_COEF 0x3944
#define VD2_VSC_REGION12_STARTP 0x3945
#define VD2_VSC_REGION34_STARTP 0x3946
#define VD2_VSC_REGION4_ENDP 0x3947
#define VD2_VSC_START_PHASE_STEP 0x3948
#define VD2_VSC_REGION0_PHASE_SLOPE 0x3949
#define VD2_VSC_REGION1_PHASE_SLOPE 0x394a
#define VD2_VSC_REGION3_PHASE_SLOPE 0x394b
#define VD2_VSC_REGION4_PHASE_SLOPE 0x394c
#define VD2_VSC_PHASE_CTRL 0x394d
#define VD2_VSC_INI_PHASE 0x394e
#define VD2_HSC_REGION12_STARTP 0x394f
#define VD2_HSC_REGION34_STARTP 0x3950
#define VD2_HSC_REGION4_ENDP 0x3951
#define VD2_HSC_START_PHASE_STEP 0x3952
#define VD2_HSC_REGION0_PHASE_SLOPE 0x3953
#define VD2_HSC_REGION1_PHASE_SLOPE 0x3954
#define VD2_HSC_REGION3_PHASE_SLOPE 0x3955
#define VD2_HSC_REGION4_PHASE_SLOPE 0x3956
#define VD2_HSC_PHASE_CTRL 0x3957
#define VD2_SC_MISC 0x3958
#define VD2_SCO_FIFO_CTRL 0x3959
#define VD2_HSC_PHASE_CTRL1 0x395a
#define VD2_HSC_INI_PAT_CTRL 0x395b
#define VD2_SC_GCLK_CTRL 0x395c
#define VPP_VD2_HDR_IN_SIZE 0x1df0

#define VD1_BLEND_SRC_CTRL 0x1dfb
#define VD2_BLEND_SRC_CTRL 0x1dfc
#define OSD1_BLEND_SRC_CTRL 0x1dfd
#define OSD2_BLEND_SRC_CTRL 0x1dfe

#define VPP_POST_BLEND_BLEND_DUMMY_DATA 0x3968
#define VPP_POST_BLEND_DUMMY_ALPHA 0x3969

/* after g12b */
#define SRSHARP0_SHARP_SYNC_CTRL 0x3eb0
#define SRSHARP1_SHARP_SYNC_CTRL 0x3fb0

#define VPU_RDARB_MODE_L2C1          0x279d
#define VPU_WRARB_MODE_L2C1          0x27a2

#define VPP_XVYCC_MISC   0x1dcf
#define VPP_XVYCC_MISC0  0x1ddf
#endif

