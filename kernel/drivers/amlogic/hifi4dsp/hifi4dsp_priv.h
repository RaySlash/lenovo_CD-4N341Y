/*
 * drivers/amlogic/hifi4dsp/hifi4dsp_api.h
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

#ifndef _HIFI4DSP_PRIV_H
#define _HIFI4DSP_PRIV_H
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>

/*
 * #include <asm/dsp/hifi4dsp_control.h>
 * #include <asm/dsp/dsp_register.h>
 */
#include <linux/dma-mapping.h>

#include "hifi4dsp_api.h"
#include "hifi4dsp_dsp.h"
#include "hifi4dsp_firmware.h"

struct reg_iomem_t {
	void __iomem *dspa_addr;
	unsigned int	rega_size;
	void __iomem *dspb_addr;
	unsigned int	regb_size;
	void __iomem *hiu_addr;		/*HIU*/
	void __iomem *sram_base;
};

extern struct reg_iomem_t g_regbases;

struct class;

struct hifi4dsp_priv {
	char name[12];
	struct class  *class;
	struct device *dev;

	u32  dsp_freq;
	//bool dsp_is_started;

	struct hifi4dsp_dsp *dsp;
	struct hifi4dsp_dsp_device	*dsp_dev;
	struct hifi4dsp_firmware	*dsp_fw;
	struct hifi4dsp_pdata		*pdata;

	struct clk *p_clk;
	struct clk *p_clk_gate;
};

struct hifi4dsp_miscdev_t {
	struct miscdevice dsp_miscdev;
	struct hifi4dsp_priv *priv;
};

struct hifi4dsp_priv *hifi4dsp_privdata(void);

#ifndef HIFI4DSP_PRNT
#define HIFI4DSP_PRNT(...)  pr_info(__VA_ARGS__)
#endif

#endif /*_HIFI4DSP_PRIV_H*/
