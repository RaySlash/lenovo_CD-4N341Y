/*
 * drivers/amlogic/mailbox/meson_mhu_dsp.h
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

#ifndef __MESON_MHU_DSP_H__
#define __MESON_MHU_DSP_H__

#define RX_OFFSET_STAT          (0x20 << 2)
#define RX_OFFSET_CLR           (0x10 << 2)
#define RX_OFFSET_SET           (0x0 << 2)
#define TX_OFFSET_STAT          (0x21 << 2)
#define TX_OFFSET_CLR           (0x11 << 2)
#define TX_OFFSET_SET           (0x1 << 2)

#define PAYLOAD_MAX_SIZE        0x200
#define RX_PAYLOAD(chan)        ((chan) * PAYLOAD_MAX_SIZE)
#define TX_PAYLOAD(chan)        ((chan) * PAYLOAD_MAX_SIZE)
#define MHU_BUFFER_SIZE         512
#define DEBUG_MAILBOX
#define ASYNC_CMD		1
#define SYNC_CMD		2
#define SYNC_MASK		0x7
#define SYNC_SHIFT		25
/*Async 1, sync 2*/
#define ASYNC_OR_SYNC(val)      ((val) << 25)
#define SIZE_MASK               0x1FF
#define CMD_MASK                0xFFFF
#define SIZE_SHIFT              16
#define MBOX_TX_SIZE            244
#define MBOX_ALLOWED_SIZE       (MBOX_TX_SIZE + sizeof(uint32_t))
#define TYPE_SHIFT              16
#define TYPE_VALUE(val)         ((val) << TYPE_SHIFT)
#define NR_DSP			2
enum call_type {
	LISTEN_CALLBACK = TYPE_VALUE(0),
	LISTEN_DATA = TYPE_VALUE(1),
};

struct mhu_chan {
	int index;
	int rx_irq;
	struct mhu_ctlr *ctlr;
	struct mhu_data_buf *data;
};

struct mbox_message {
	struct list_head list;
	struct task_struct *task;
	struct completion complete;
	int cmd;
	char *data;
};

struct mbox_data {
	u64 reserve;
	u64 complete;
	char data[MBOX_TX_SIZE];
};

struct mbox_data_sync {
	u64 reserve;
	char data[MBOX_TX_SIZE];
};

struct mhu_ctlr {
	struct device *dev;
	void __iomem *mbox_dspa_base;
	void __iomem *mbox_dspb_base;
	void __iomem *payload_base;
	struct mbox_controller mbox_con;
	struct mhu_chan *channels;
};
#endif
