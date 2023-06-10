/* drivers/input/misc/kionix_accel.c - Kionix accelerometer driver
 *
 * Copyright (C) 2012 Kionix, Inc.
 * Written by Kuching Tan <kuchingtan@kionix.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/hrtimer.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/slab.h>

#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/pm_runtime.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>

#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/device.h>
#include <linux/regulator/consumer.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>

//#include <linux/input/kionix_accel.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#ifdef    CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif /* CONFIG_HAS_EARLYSUSPEND */
#include "kionix_accel.h"

/* Debug Message Flags */
#define KIONIX_KMSG_ERR	1	/* Print kernel debug message for error */
#define KIONIX_KMSG_INF	1	/* Print kernel debug message for info */

#if KIONIX_KMSG_ERR
#define KMSGERR(format, ...)	\
		dev_err(format, ## __VA_ARGS__)
#else
#define KMSGERR(format, ...)
#endif

#if KIONIX_KMSG_INF
#define KMSGINF(format, ...)	\
		dev_info(format, ## __VA_ARGS__)
#else
#define KMSGINF(format, ...)
#endif


#define KIONIX_ENABLE_POLLING_MODE 		1//1

#define KIONIX_EVENT_SINCE_EN_LIMIT_DEF	(3)
#define KIONIX_OFFSET_REG_LSB_1G 128
#define KIONIX_LSB_1G_1024 1024
#define KIONIX_SAMPLE_NO				10
#define KIONIX_ACC_CALI_END			'\0'
#define KIONIX_ACC_CALI_FILE 			"/data/gsensor_calibration.conf"//
#define KIONIX_ACC_CALI_FILE_SIZE 		40


#define KIONIX_K_SUCCESS_QCOM			0x05
#define KIONIX_K_SUCCESS_TUNE			0x04
#define KIONIX_K_SUCCESS_FT2			0x03
#define KIONIX_K_SUCCESS_FT1			0x02
#define KIONIX_K_SUCCESS_FILE			0x01
#define KIONIX_K_NO_CALI				0xFF
#define KIONIX_K_RUNNING				0xFE
#define KIONIX_K_FAIL_LRG_DIFF			0xFD
#define KIONIX_K_FAIL_OPEN_FILE			0xFC
#define KIONIX_K_FAIL_W_FILE				0xFB
#define KIONIX_K_FAIL_R_BACK				0xFA
#define KIONIX_K_FAIL_R_BACK_COMP		0xF9
#define KIONIX_K_FAIL_I2C				0xF8
#define KIONIX_K_FAIL_K_PARA				0xF7
#define KIONIX_K_FAIL_OUT_RG			0xF6
#define KIONIX_K_FAIL_ENG_I2C			0xF5
#define KIONIX_K_FAIL_FT1_USD			0xF4
#define KIONIX_K_FAIL_FT2_USD			0xF3
#define KIONIX_K_FAIL_WRITE_NOFST		0xF2
#define KIONIX_K_FAIL_OTP_5T				0xF1
#define KIONIX_K_FAIL_PLACEMENT			0xF0



/******************************************************************************
 * Accelerometer WHO_AM_I return value
 *****************************************************************************/
#define KIONIX_ACCEL_WHO_AM_I_KXTE9 		0x00
#define KIONIX_ACCEL_WHO_AM_I_KXTF9 		0x01
#define KIONIX_ACCEL_WHO_AM_I_KXTI9_1001 	0x04
#define KIONIX_ACCEL_WHO_AM_I_KXTIK_1004 	0x05
#define KIONIX_ACCEL_WHO_AM_I_KXTJ9_1005 	0x07
#define KIONIX_ACCEL_WHO_AM_I_KXTJ9_1007 	0x08
#define KIONIX_ACCEL_WHO_AM_I_KXCJ9_1008 	0x0A
#define KIONIX_ACCEL_WHO_AM_I_KXTJ2_1009 	0x09
#define KIONIX_ACCEL_WHO_AM_I_KXTJ3_1057 	0x35
#define KIONIX_ACCEL_WHO_AM_I_KXCJK_1013 	0x11
#define KIONIX_ACCEL_WHO_AM_I_KX023		    0x15
#define KIONIX_ACCEL_WHO_AM_I_KX022         0x14

/******************************************************************************
 * Accelerometer Grouping
 *****************************************************************************/
#define KIONIX_ACCEL_GRP1	1	/* KXTE9 */
#define KIONIX_ACCEL_GRP2	2	/* KXTF9/I9-1001/J9-1005 */
#define KIONIX_ACCEL_GRP3	3	/* KXTIK-1004 */
#define KIONIX_ACCEL_GRP4	4	/* KXTJ9-1007/KXCJ9-1008 */
#define KIONIX_ACCEL_GRP5	5	/* KXTJ2-1009 */
#define KIONIX_ACCEL_GRP6	6	/* KXCJK-1013 */
#define KIONIX_ACCEL_GRP7	7	/* KX023/KX022 */

/******************************************************************************
 * Registers for All Accelerometer Group
 *****************************************************************************/
#define ACCEL_WHO_AM_I		0x0F

/*****************************************************************************/
/* Registers for Accelerometer Group 1 */
/*****************************************************************************/
/* Output Registers */
#define ACCEL_GRP1_XOUT			0x12
/* Control Registers */
#define ACCEL_GRP1_CTRL_REG1	0x1B
/* CTRL_REG1 */
#define ACCEL_GRP1_PC1_OFF		0x7F
#define ACCEL_GRP1_PC1_ON		(1 << 7)
#define ACCEL_GRP1_ODR40		(3 << 3)
#define ACCEL_GRP1_ODR10		(2 << 3)
#define ACCEL_GRP1_ODR3			(1 << 3)
#define ACCEL_GRP1_ODR1			(0 << 3)
#define ACCEL_GRP1_ODR_MASK		(3 << 3)

/*****************************************************************************/
/* Registers for Accelerometer Group 2 & 3 */
/*****************************************************************************/
/* Output Registers */
#define ACCEL_GRP2_XOUT_L		0x06
/* Control Registers */
#define ACCEL_GRP2_INT_REL		0x1A
#define ACCEL_GRP2_CTRL_REG1	0x1B
#define ACCEL_GRP2_INT_CTRL1	0x1E
#define ACCEL_GRP2_DATA_CTRL	0x21
/* CTRL_REG1 */
#define ACCEL_GRP2_PC1_OFF		0x7F
#define ACCEL_GRP2_PC1_ON		(1 << 7)
#define ACCEL_GRP2_DRDYE		(1 << 5)
#define ACCEL_GRP2_G_8G			(2 << 3)
#define ACCEL_GRP2_G_4G			(1 << 3)
#define ACCEL_GRP2_G_2G			(0 << 3)
#define ACCEL_GRP2_G_MASK		(3 << 3)
#define ACCEL_GRP2_RES_8BIT		(0 << 6)
#define ACCEL_GRP2_RES_12BIT	(1 << 6)
#define ACCEL_GRP2_RES_MASK		(1 << 6)

/* INT_CTRL1 */
#define ACCEL_GRP2_IEA			(1 << 4)
#define ACCEL_GRP2_IEN			(1 << 5)
/* DATA_CTRL_REG */
#define ACCEL_GRP2_ODR12_5		0x00
#define ACCEL_GRP2_ODR25		0x01
#define ACCEL_GRP2_ODR50		0x02
#define ACCEL_GRP2_ODR100		0x03
#define ACCEL_GRP2_ODR200		0x04
#define ACCEL_GRP2_ODR400		0x05
#define ACCEL_GRP2_ODR800		0x06
/*****************************************************************************/

/*****************************************************************************/
/* Registers for Accelerometer Group 4 & 5 & 6 */
/*****************************************************************************/
/* Output Registers */
#define ACCEL_GRP4_XOUT_L		0x06
/* Control Registers */
#define ACCEL_GRP4_INT_REL		0x1A
#define ACCEL_GRP4_CTRL_REG1	0x1B
#define ACCEL_GRP4_INT_CTRL1	0x1E
#define ACCEL_GRP4_DATA_CTRL	0x21
/* CTRL_REG1 */
#define ACCEL_GRP4_PC1_OFF		0x7F
#define ACCEL_GRP4_PC1_ON		(1 << 7)
#define ACCEL_GRP4_DRDYE		(1 << 5)
#define ACCEL_GRP4_G_8G			(2 << 3)
#define ACCEL_GRP4_G_4G			(1 << 3)
#define ACCEL_GRP4_G_2G			(0 << 3)
#define ACCEL_GRP4_G_MASK		(3 << 3)
#define ACCEL_GRP4_RES_8BIT		(0 << 6)
#define ACCEL_GRP4_RES_12BIT	(1 << 6)
#define ACCEL_GRP4_RES_MASK		(1 << 6)
/* INT_CTRL1 */
#define ACCEL_GRP4_IEA			(1 << 4)
#define ACCEL_GRP4_IEN			(1 << 5)
/* DATA_CTRL_REG */
#define ACCEL_GRP4_ODR0_781		0x08
#define ACCEL_GRP4_ODR1_563		0x09
#define ACCEL_GRP4_ODR3_125		0x0A
#define ACCEL_GRP4_ODR6_25		0x0B
#define ACCEL_GRP4_ODR12_5		0x00
#define ACCEL_GRP4_ODR25		0x01
#define ACCEL_GRP4_ODR50		0x02
#define ACCEL_GRP4_ODR100		0x03
#define ACCEL_GRP4_ODR200		0x04
#define ACCEL_GRP4_ODR400		0x05
#define ACCEL_GRP4_ODR800		0x06
#define ACCEL_GRP4_ODR1600		0x07
/*****************************************************************************/

/*****************************************************************************/
/* Registers for Accelerometer Group 7 */
/*****************************************************************************/
/* Output Registers */
#define ACCEL_GRP7_XOUT_L		0x06
/* Control Registers */
#define ACCEL_GRP7_INT_REL		0x17
#define ACCEL_GRP7_CTRL_REG1	0x18
#define ACCEL_GRP7_CTRL_REG3	0x18
#define ACCEL_GRP7_INT_CTRL1	0x1C
#define ACCEL_GRP7_DATA_CTRL	0x1B	/* ODCNTL */
#define ACCEL_GRP7_INT_CTRL2	0x1D
#define ACCEL_GRP7_INS2			0x13

/* CTRL_REG1 */
#define ACCEL_GRP7_PC1_OFF		0x7F
#define ACCEL_GRP7_PC1_ON		(1 << 7)
#define ACCEL_GRP7_DRDYE		(1 << 5)
#define ACCEL_GRP7_G_8G			(2 << 3)
#define ACCEL_GRP7_G_4G			(1 << 3)
#define ACCEL_GRP7_G_2G			(0 << 3)
#define ACCEL_GRP7_G_MASK		(3 << 3)
#define ACCEL_GRP7_RES_8BIT		(0 << 6)
#define ACCEL_GRP7_RES_16BIT	(1 << 6)
#define ACCEL_GRP7_RES_MASK		(1 << 6)
#define ACCEL_GRP7_WUFE_ON		(1 << 1)
#define ACCEL_GRP7_WUFE_OFF		 0xFD

/* INT_CTRL1 */
#define ACCEL_GRP7_IEA			(1 << 4)
#define ACCEL_GRP7_IEN			(1 << 5)
/* DATA_CTRL_REG */
#define ACCEL_GRP7_ODR0_781		0x08
#define ACCEL_GRP7_ODR1_563		0x09
#define ACCEL_GRP7_ODR3_125		0x0A
#define ACCEL_GRP7_ODR6_25		0x0B
#define ACCEL_GRP7_ODR12_5		0x00
#define ACCEL_GRP7_ODR25		0x01
#define ACCEL_GRP7_ODR50		0x02
#define ACCEL_GRP7_ODR100		0x03
#define ACCEL_GRP7_ODR200		0x04
#define ACCEL_GRP7_ODR400		0x05
#define ACCEL_GRP7_ODR800		0x06
#define ACCEL_GRP7_ODR1600		0x07

/* INT_CTRL2 */
#define ACCEL_GRP7_INT_CTRL2_MASK 0x3F

/* INT4_CTRL1 */
#define ACCEL_GRP7_INT_CTRL4	0x1F
#define ACCEL_GRP7_WUFI1	 	(1 << 1)
#define ACCEL_GRP7_DRDYI1	 	(1 << 4)

/* CTRL_REG3 */
#define	ACCEL_GRP7_CNTL3_OWUF_MASK    0x7 // wake-up ODR
#define ACCEL_GRP7_CNTL3_OWUF_0P781   0x0 // 0.78Hz
#define ACCEL_GRP7_CNTL3_OWUF_1P563   0x1 // 1.563Hz
#define	ACCEL_GRP7_CNTL3_OWUF_3P125   0x2 // 3.125Hz
#define ACCEL_GRP7_CNTL3_OWUF_6P25    0x3 // 6.25Hz
#define	ACCEL_GRP7_CNTL3_OWUF_12P5    0x4 // 12.5Hz
#define ACCEL_GRP7_CNTL3_OWUF_25      0x5 // 25Hz
#define	ACCEL_GRP7_CNTL3_OWUF_50      0x6 // 50Hz
#define	ACCEL_GRP7_CNTL3_OWUF_100     0x7 // 100Hz

/* Wake up theshold registers/values */
#define ACCEL_GRP7_WUFC			0x23
#define ACCEL_GRP7_ATH			0x30
#define ACCEL_GRP7_WUFC_VAL 	0x00
#define ACCEL_GRP7_ATH_VAL 		0x02

#define ACCEL_GRP7_INS2_WUFS	0x02
#define ACCEL_GRP7_INS2_DRDY	0x10

/*****************************************************************************/

/* Input Event Constants */
#define ACCEL_G_MAX			16384
#define ACCEL_FUZZ			0
#define ACCEL_FLAT			0
/* I2C Retry Constants */
#define KIONIX_I2C_RETRY_COUNT		10 	/* Number of times to retry i2c */
#define KIONIX_I2C_RETRY_TIMEOUT	1	/* Timeout between retry (miliseconds) */

/* Earlysuspend Contants */
#define KIONIX_ACCEL_EARLYSUSPEND_TIMEOUT	1000//5000	/* Timeout (miliseconds) */
#define KIONIX_ACCEL_POLLING_PERIOD		20 * 1000000	// 63ms

/*
 * The following table lists the maximum appropriate poll interval for each
 * available output data rate (ODR).
 */
static const struct {
	unsigned int cutoff;
	u8 mask;
} kionix_accel_grp1_odr_table[] = {
	{ 100,	ACCEL_GRP1_ODR40 },
	{ 334,	ACCEL_GRP1_ODR10 },
	{ 1000,	ACCEL_GRP1_ODR3  },
	{ 0,	ACCEL_GRP1_ODR1  },
};

static const struct {
	unsigned int cutoff;
	u8 mask;
} kionix_accel_grp2_odr_table[] = {
	{ 3,	ACCEL_GRP2_ODR800 },
	{ 5,	ACCEL_GRP2_ODR400 },
	{ 10,	ACCEL_GRP2_ODR200 },
	{ 20,	ACCEL_GRP2_ODR100 },
	{ 40,	ACCEL_GRP2_ODR50  },
	{ 80,	ACCEL_GRP2_ODR25  },
	{ 0,	ACCEL_GRP2_ODR12_5},
};

static const struct {
	unsigned int cutoff;
	u8 mask;
} kionix_accel_grp4_odr_table[] = {
	{ 2,	ACCEL_GRP4_ODR1600 },
	{ 3,	ACCEL_GRP4_ODR800 },
	{ 5,	ACCEL_GRP4_ODR400 },
	{ 10,	ACCEL_GRP4_ODR200 },
	{ 20,	ACCEL_GRP4_ODR100 },
	{ 40,	ACCEL_GRP4_ODR50  },
	{ 80,	ACCEL_GRP4_ODR25  },
	{ 160,	ACCEL_GRP4_ODR12_5},
	{ 320,	ACCEL_GRP4_ODR6_25},
	{ 640,	ACCEL_GRP4_ODR3_125},
	{ 1280,	ACCEL_GRP4_ODR1_563},
	{ 0,	ACCEL_GRP4_ODR0_781},
};

static const struct {
	unsigned int cutoff;
	u8 mask;
} kionix_accel_grp7_odr_table[] = {
	{ 2,	ACCEL_GRP7_ODR1600 },
	{ 3,	ACCEL_GRP7_ODR800 },
	{ 5,	ACCEL_GRP7_ODR400 },
	{ 10,	ACCEL_GRP7_ODR200 },
	{ 20,	ACCEL_GRP7_ODR100 },
	{ 40,	ACCEL_GRP7_ODR50  },
	{ 80,	ACCEL_GRP7_ODR25  },
	{ 160,	ACCEL_GRP7_ODR12_5},
	{ 320,	ACCEL_GRP7_ODR6_25},
	{ 640,	ACCEL_GRP7_ODR3_125},
	{ 1280,	ACCEL_GRP7_ODR1_563},
	{ 0,	ACCEL_GRP7_ODR0_781},
};

enum {
	accel_grp1_ctrl_reg1 = 0,
	accel_grp1_regs_count,
};

enum {
	accel_grp2_ctrl_reg1 = 0,
	accel_grp2_data_ctrl,
	accel_grp2_int_ctrl,
	accel_grp2_regs_count,
};

enum {
	accel_grp4_ctrl_reg1 = 0,
	accel_grp4_data_ctrl,
	accel_grp4_int_ctrl,
	accel_grp4_regs_count,
};

enum {
	accel_grp7_ctrl_reg1 = 0,
	accel_grp7_ctrl_reg3,
	accel_grp7_data_ctrl,
	accel_grp7_int_ctrl,
	accel_grp7_int2_ctrl,
	accel_grp7_int4_ctrl,
	accel_grp7_wufc,
	accel_grp7_ath,
	accel_grp7_regs_count,
};

struct kionix_accel_driver {
	struct i2c_client *client;
	struct kionix_accel_platform_data accel_pdata;
	struct input_dev *input_dev;
	/* hrtimer change */
	struct hrtimer accel_timer;
	int accel_wkp_flag;
	struct task_struct *accel_task;
	wait_queue_head_t accel_wq;	
	//struct delayed_work accel_work;
	//struct workqueue_struct *accel_workqueue;
	wait_queue_head_t wqh_suspend;
	
	int accel_data[3];
	int accel_cali[3];
	u8 axis_map_x;
	u8 axis_map_y;
	u8 axis_map_z;
	bool negate_x;
	bool negate_y;
	bool negate_z;
	u8 shift;

	unsigned int poll_interval;
	unsigned int poll_delay;
	unsigned int accel_group;
	u8 *accel_registers;

	atomic_t accel_suspended;
	atomic_t accel_suspend_continue;
	atomic_t accel_enabled;
	atomic_t accel_input_event;
	atomic_t accel_enable_resume;
	atomic_t accel_wuf_enabled;
	struct mutex mutex_earlysuspend;
	struct mutex mutex_resume;
	rwlock_t rwlock_accel_data;

	bool accel_drdy;

#if 1//#ifdef KIONIX_ENABLE_POLLING_MODE
	struct delayed_work accel_delaywork;
	//struct hrtimer accel_timer;
		
#endif	/* KIONIX_ENABLE_POLLING_MODE */


	/* Function callback */
	void (*kionix_accel_report_accel_data)(struct kionix_accel_driver *acceld);
	int (*kionix_accel_update_odr)(struct kionix_accel_driver *acceld, unsigned int poll_interval);
	int (*kionix_accel_power_on_init)(struct kionix_accel_driver *acceld);
	int (*kionix_accel_operate)(struct kionix_accel_driver *acceld);
	int (*kionix_accel_standby)(struct kionix_accel_driver *acceld);
	int (*kionix_accel_wufe)(struct kionix_accel_driver *acceld, int enable);

#ifdef    CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif /* CONFIG_HAS_EARLYSUSPEND */
};

static const struct of_device_id of_kionix_accel_match[] = {
	{ .compatible = "kionix,kionix_accel", },
	{},
};
MODULE_DEVICE_TABLE(of, of_kionix_accel_match);

static void kionix_load_cali(struct kionix_accel_driver *acceld);

static int accel_poll_thread(void *data);

static int kionix_i2c_read(struct i2c_client *client, u8 addr, u8 *data, int len)
{
	struct i2c_msg msgs[] = {
		{
			.addr = client->addr,
			.flags = client->flags,
			.len = 1,
			.buf = &addr,
		},
		{
			.addr = client->addr,
			.flags = client->flags | I2C_M_RD,
			.len = len,
			.buf = data,
		},
	};

	return i2c_transfer(client->adapter, msgs, 2);
}

static int kionix_strtok(const char *buf, size_t count, char **token, const int token_nr)
{
	char *buf2 = (char *)kzalloc((count + 1) * sizeof(char), GFP_KERNEL);
	char **token2 = token;
	unsigned int num_ptr = 0, num_nr = 0, num_neg = 0;
	int i = 0, start = 0, end = (int)count;

	strcpy(buf2, buf);

	/* We need to breakup the string into separate chunks in order for kstrtoint
	 * or strict_strtol to parse them without returning an error. Stop when the end of
	 * the string is reached or when enough value is read from the string */
	while((start < end) && (i < token_nr)) {
		/* We found a negative sign */
		if(*(buf2 + start) == '-') {
			/* Previous char(s) are numeric, so we store their value first before proceed */
			if(num_nr > 0) {
				/* If there is a pending negative sign, we adjust the variables to account for it */
				if(num_neg) {
					num_ptr--;
					num_nr++;
				}
				*token2 = (char *)kzalloc((num_nr + 2) * sizeof(char), GFP_KERNEL);
				strncpy(*token2, (const char *)(buf2 + num_ptr), (size_t) num_nr);
				*(*token2+num_nr) = '\n';
				i++;
				token2++;
				/* Reset */
				num_ptr = num_nr = 0;
			}
			/* This indicates that there is a pending negative sign in the string */
			num_neg = 1;
		}
		/* We found a numeric */
		else if((*(buf2 + start) >= '0') && (*(buf2 + start) <= '9')) {
			/* If the previous char(s) are not numeric, set num_ptr to current char */
			if(num_nr < 1)
				num_ptr = start;
			num_nr++;
		}
		/* We found an unwanted character */
		else {
			/* Previous char(s) are numeric, so we store their value first before proceed */
			if(num_nr > 0) {
				if(num_neg) {
					num_ptr--;
					num_nr++;
				}
				*token2 = (char *)kzalloc((num_nr + 2) * sizeof(char), GFP_KERNEL);
				strncpy(*token2, (const char *)(buf2 + num_ptr), (size_t) num_nr);
				*(*token2+num_nr) = '\n';
				i++;
				token2++;
			}
			/* Reset all the variables to start afresh */
			num_ptr = num_nr = num_neg = 0;
		}
		start++;
	}

	kfree(buf2);

	return (i == token_nr) ? token_nr : -1;
}

static int kionix_accel_grp1_power_on_init(struct kionix_accel_driver *acceld)
{
	int err;

	if(atomic_read(&acceld->accel_enabled) > 0) {
		err = i2c_smbus_write_byte_data(acceld->client,
						ACCEL_GRP1_CTRL_REG1, acceld->accel_registers[accel_grp1_ctrl_reg1] | ACCEL_GRP1_PC1_ON);
		if (err < 0)
			return err;
	}
	else {
		err = i2c_smbus_write_byte_data(acceld->client,
						ACCEL_GRP1_CTRL_REG1, acceld->accel_registers[accel_grp1_ctrl_reg1]);
		if (err < 0)
			return err;
	}

	return 0;
}

static int kionix_accel_grp1_operate(struct kionix_accel_driver *acceld)
{
	int err;

	err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP1_CTRL_REG1, \
			acceld->accel_registers[accel_grp2_ctrl_reg1] | ACCEL_GRP1_PC1_ON);
	if (err < 0)
		return err;

	//queue_delayed_work(acceld->accel_workqueue, &acceld->accel_work, 0);	
	if(acceld->accel_drdy == 0)
		hrtimer_start(&acceld->accel_timer, 
			ktime_set(acceld->poll_interval / 1000, (acceld->poll_interval % 1000) * 1000000),
			HRTIMER_MODE_REL);

	return 0;
}

static int kionix_accel_grp1_standby(struct kionix_accel_driver *acceld)
{
	int err;

	//cancel_delayed_work_sync(&acceld->accel_work);
	err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP1_CTRL_REG1, 0);
	if (err < 0)
		return err;

	return 0;
}

static void kionix_accel_grp1_report_accel_data(struct kionix_accel_driver *acceld)
{
	u8 accel_data[3];
	s16 x, y, z;
	int err;
	struct input_dev *input_dev = acceld->input_dev;
	int loop = KIONIX_I2C_RETRY_COUNT;

	if(atomic_read(&acceld->accel_enabled) > 0) {
		if(atomic_read(&acceld->accel_enable_resume) > 0)
		{
			while(loop) {
				mutex_lock(&input_dev->mutex);
				err = kionix_i2c_read(acceld->client, ACCEL_GRP1_XOUT, accel_data, 6);
				mutex_unlock(&input_dev->mutex);
				if(err < 0){
					loop--;
					mdelay(KIONIX_I2C_RETRY_TIMEOUT);
				}
				else
					loop = 0;
			}
			if (err < 0) {
				KMSGERR(&acceld->client->dev, "%s: read data output error = %d\n", __func__, err);
			}
			else {
				write_lock(&acceld->rwlock_accel_data);

				x = ((s16) le16_to_cpu(((s16)(accel_data[acceld->axis_map_x] >> 2)) - 32)) << 6;
				y = ((s16) le16_to_cpu(((s16)(accel_data[acceld->axis_map_y] >> 2)) - 32)) << 6;
				z = ((s16) le16_to_cpu(((s16)(accel_data[acceld->axis_map_z] >> 2)) - 32)) << 6;

				acceld->accel_data[acceld->axis_map_x] = (acceld->negate_x ? -x : x) + acceld->accel_cali[acceld->axis_map_x];
				acceld->accel_data[acceld->axis_map_y] = (acceld->negate_y ? -y : y) + acceld->accel_cali[acceld->axis_map_y];
				acceld->accel_data[acceld->axis_map_z] = (acceld->negate_z ? -z : z) + acceld->accel_cali[acceld->axis_map_z];

				if(atomic_read(&acceld->accel_input_event) > 0) {
					input_report_abs(acceld->input_dev, ABS_X, acceld->accel_data[acceld->axis_map_x]);
					input_report_abs(acceld->input_dev, ABS_Y, acceld->accel_data[acceld->axis_map_y]);
					input_report_abs(acceld->input_dev, ABS_Z, acceld->accel_data[acceld->axis_map_z]);
					input_sync(acceld->input_dev);
				}

				write_unlock(&acceld->rwlock_accel_data);
			}
		}
		else
		{
			atomic_inc(&acceld->accel_enable_resume);
		}
	}
}

static int kionix_accel_grp1_update_odr(struct kionix_accel_driver *acceld, unsigned int poll_interval)
{
	int err;
	int i;
	u8 odr;

	/* Use the lowest ODR that can support the requested poll interval */
	for (i = 0; i < ARRAY_SIZE(kionix_accel_grp1_odr_table); i++) {
		odr = kionix_accel_grp1_odr_table[i].mask;
		if (poll_interval < kionix_accel_grp1_odr_table[i].cutoff)
			break;
	}

	/* Do not need to update CTRL_REG1 register if the ODR is not changed */
	if((acceld->accel_registers[accel_grp1_ctrl_reg1] & ACCEL_GRP1_ODR_MASK) == odr)
		return 0;
	else {
		acceld->accel_registers[accel_grp1_ctrl_reg1] &= ~ACCEL_GRP1_ODR_MASK;
		acceld->accel_registers[accel_grp1_ctrl_reg1] |= odr;
	}

	/* Do not need to update CTRL_REG1 register if the sensor is not currently turn on */
	if(atomic_read(&acceld->accel_enabled) > 0) {
		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP1_CTRL_REG1, \
				acceld->accel_registers[accel_grp1_ctrl_reg1] | ACCEL_GRP1_PC1_ON);
		if (err < 0)
			return err;
	}

	return 0;
}

static int kionix_accel_grp2_power_on_init(struct kionix_accel_driver *acceld)
{
	int err;

	/* ensure that PC1 is cleared before updating control registers */
	err = i2c_smbus_write_byte_data(acceld->client,
					ACCEL_GRP2_CTRL_REG1, 0);
	if (err < 0)
		return err;

	err = i2c_smbus_write_byte_data(acceld->client,
					ACCEL_GRP2_DATA_CTRL, acceld->accel_registers[accel_grp2_data_ctrl]);
	if (err < 0)
		return err;

	/* only write INT_CTRL_REG1 if in irq mode */
	if (acceld->client->irq) {
		err = i2c_smbus_write_byte_data(acceld->client,
						ACCEL_GRP2_INT_CTRL1, acceld->accel_registers[accel_grp2_int_ctrl]);
		if (err < 0)
			return err;
	}

	if(atomic_read(&acceld->accel_enabled) > 0) {
		err = i2c_smbus_write_byte_data(acceld->client,
						ACCEL_GRP2_CTRL_REG1, acceld->accel_registers[accel_grp2_ctrl_reg1] | ACCEL_GRP2_PC1_ON);
		if (err < 0)
			return err;
	}
	else {
		err = i2c_smbus_write_byte_data(acceld->client,
						ACCEL_GRP2_CTRL_REG1, acceld->accel_registers[accel_grp2_ctrl_reg1]);
		if (err < 0)
			return err;
	}

	return 0;
}

static int kionix_accel_grp2_operate(struct kionix_accel_driver *acceld)
{
	int err;

	err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP2_CTRL_REG1, \
			acceld->accel_registers[accel_grp2_ctrl_reg1] | ACCEL_GRP2_PC1_ON);
	if (err < 0)
		return err;
			
	if(acceld->accel_drdy == 0)
		//queue_delayed_work(acceld->accel_workqueue, &acceld->accel_work, 0);
		hrtimer_start(&acceld->accel_timer, 
			ktime_set(acceld->poll_interval / 1000, (acceld->poll_interval % 1000) * 1000000),
			HRTIMER_MODE_REL);		

	return 0;
}

static int kionix_accel_grp2_standby(struct kionix_accel_driver *acceld)
{
	int err;

	//if(acceld->accel_drdy == 0)
		//cancel_delayed_work_sync(&acceld->accel_work);
	err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP2_CTRL_REG1, 0);
	if (err < 0)
		return err;

	return 0;
}

static void kionix_accel_grp2_report_accel_data(struct kionix_accel_driver *acceld)
{
	struct { union {
		s16 accel_data_s16[3];
		s8	accel_data_s8[6];
	}; } accel_data;
	s16 x, y, z;
	int err;
	struct input_dev *input_dev = acceld->input_dev;
	int loop;

	/* Only read the output registers if enabled */
	if(atomic_read(&acceld->accel_enabled) > 0) {
		if(atomic_read(&acceld->accel_enable_resume) > 0)
		{
			loop = KIONIX_I2C_RETRY_COUNT;
			while(loop) {
				mutex_lock(&input_dev->mutex);
				err = kionix_i2c_read(acceld->client, ACCEL_GRP2_XOUT_L, (u8 *)accel_data.accel_data_s16, 6);
				mutex_unlock(&input_dev->mutex);
				if(err < 0){
					loop--;
					mdelay(KIONIX_I2C_RETRY_TIMEOUT);
				}
				else
					loop = 0;
			}
			if (err < 0) {
				KMSGERR(&acceld->client->dev, "%s: read data output error = %d\n", __func__, err);
			}
			else {
				write_lock(&acceld->rwlock_accel_data);

				x = ((s16) le16_to_cpu(accel_data.accel_data_s16[acceld->axis_map_x])) >> acceld->shift;
				y = ((s16) le16_to_cpu(accel_data.accel_data_s16[acceld->axis_map_y])) >> acceld->shift;
				z = ((s16) le16_to_cpu(accel_data.accel_data_s16[acceld->axis_map_z])) >> acceld->shift;

				acceld->accel_data[acceld->axis_map_x] = (acceld->negate_x ? -x : x) + acceld->accel_cali[acceld->axis_map_x];
				acceld->accel_data[acceld->axis_map_y] = (acceld->negate_y ? -y : y) + acceld->accel_cali[acceld->axis_map_y];
				acceld->accel_data[acceld->axis_map_z] = (acceld->negate_z ? -z : z) + acceld->accel_cali[acceld->axis_map_z];

				if(atomic_read(&acceld->accel_input_event) > 0) {
					input_report_abs(acceld->input_dev, ABS_X, acceld->accel_data[acceld->axis_map_x]);
					input_report_abs(acceld->input_dev, ABS_Y, acceld->accel_data[acceld->axis_map_y]);
					input_report_abs(acceld->input_dev, ABS_Z, acceld->accel_data[acceld->axis_map_z]);
					input_sync(acceld->input_dev);
				}

				write_unlock(&acceld->rwlock_accel_data);
			}
		}
		else
		{
			atomic_inc(&acceld->accel_enable_resume);
		}
	}

	/* Clear the interrupt if using drdy */
	if(acceld->accel_drdy == 1) {
		loop = KIONIX_I2C_RETRY_COUNT;
		while(loop) {
			err = i2c_smbus_read_byte_data(acceld->client, ACCEL_GRP2_INT_REL);
			if(err < 0){
				loop--;
				mdelay(KIONIX_I2C_RETRY_TIMEOUT);
			}
			else
				loop = 0;
		}
		if (err < 0)
			KMSGERR(&acceld->client->dev, "%s: clear interrupt error = %d\n", __func__, err);
	}
}

static void kionix_accel_grp2_update_g_range(struct kionix_accel_driver *acceld)
{
	acceld->accel_registers[accel_grp2_ctrl_reg1] &= ~ACCEL_GRP2_G_MASK;

	switch (acceld->accel_pdata.accel_g_range) {
		case KIONIX_ACCEL_G_8G:
		case KIONIX_ACCEL_G_6G:
			acceld->shift = 2;
			acceld->accel_registers[accel_grp2_ctrl_reg1] |= ACCEL_GRP2_G_8G;
			break;
		case KIONIX_ACCEL_G_4G:
			acceld->shift = 3;
			acceld->accel_registers[accel_grp2_ctrl_reg1] |= ACCEL_GRP2_G_4G;
			break;
		case KIONIX_ACCEL_G_2G:
		default:
			acceld->shift = 4;
			acceld->accel_registers[accel_grp2_ctrl_reg1] |= ACCEL_GRP2_G_2G;
			break;
	}

	return;
}

static int kionix_accel_grp2_update_odr(struct kionix_accel_driver *acceld, unsigned int poll_interval)
{
	int err;
	int i;
	u8 odr;

	/* Use the lowest ODR that can support the requested poll interval */
	for (i = 0; i < ARRAY_SIZE(kionix_accel_grp2_odr_table); i++) {
		odr = kionix_accel_grp2_odr_table[i].mask;
		if (poll_interval < kionix_accel_grp2_odr_table[i].cutoff)
			break;
	}

	/* Do not need to update DATA_CTRL_REG register if the ODR is not changed */
	if(acceld->accel_registers[accel_grp2_data_ctrl] == odr)
		return 0;
	else
		acceld->accel_registers[accel_grp2_data_ctrl] = odr;

	/* Do not need to update DATA_CTRL_REG register if the sensor is not currently turn on */
	if(atomic_read(&acceld->accel_enabled) > 0) {
		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP2_CTRL_REG1, 0);
		if (err < 0)
			return err;

		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP2_DATA_CTRL, acceld->accel_registers[accel_grp2_data_ctrl]);
		if (err < 0)
			return err;

		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP2_CTRL_REG1, acceld->accel_registers[accel_grp2_ctrl_reg1] | ACCEL_GRP2_PC1_ON);
		if (err < 0)
			return err;
	}

	return 0;
}

static int kionix_accel_grp4_power_on_init(struct kionix_accel_driver *acceld)
{
	int err;
	//printk("wangdong kionix_accel_grp4_power_on_init start\n");
	/* ensure that PC1 is cleared before updating control registers */
	err = i2c_smbus_write_byte_data(acceld->client,
					ACCEL_GRP4_CTRL_REG1, 0);
	if (err < 0)
		return err;

	err = i2c_smbus_write_byte_data(acceld->client,
					ACCEL_GRP4_DATA_CTRL, acceld->accel_registers[accel_grp4_data_ctrl]);
	if (err < 0)
		return err;

	/* only write INT_CTRL_REG1 if in irq mode */
	if (acceld->client->irq) {
		//printk("wangdong kionix_accel_grp4_power_on_init INT_CTRL_REG1\n");
		err = i2c_smbus_write_byte_data(acceld->client,
						ACCEL_GRP4_INT_CTRL1, acceld->accel_registers[accel_grp4_int_ctrl]);
		if (err < 0)
			return err;
	}

	if(atomic_read(&acceld->accel_enabled) > 0) {
		//printk("wangdong kionix_accel_grp4_power_on_init ACCEL_GRP4_CTRL_REG1\n");
		err = i2c_smbus_write_byte_data(acceld->client,
						ACCEL_GRP4_CTRL_REG1, acceld->accel_registers[accel_grp4_ctrl_reg1] | ACCEL_GRP4_PC1_ON);
		if (err < 0)
			return err;
	}
	else {
		//printk("wangdong kionix_accel_grp4_power_on_init ACCEL_GRP4_CTRL_REG1 else!!!\n");
		err = i2c_smbus_write_byte_data(acceld->client,
						ACCEL_GRP4_CTRL_REG1, acceld->accel_registers[accel_grp4_ctrl_reg1]);
		if (err < 0)
			return err;
	}

	return 0;
}

static int kionix_accel_grp4_operate(struct kionix_accel_driver *acceld)
{
	int err;
	//printk("wangdong kionix_accel_grp4_operate ACCEL_GRP4_CTRL_REG1\n");
	
	err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP4_CTRL_REG1, \
			acceld->accel_registers[accel_grp4_ctrl_reg1] | ACCEL_GRP4_PC1_ON);
	if (err < 0)
		return err;

	if(acceld->accel_drdy == 0)
		//queue_delayed_work(acceld->accel_workqueue, &acceld->accel_work, 0);
		/*hrtimer_start(&acceld->accel_timer, 
			ktime_set(acceld->poll_interval / 1000, (acceld->poll_interval % 1000) * 1000000),
			HRTIMER_MODE_REL);*/
		hrtimer_start(&acceld->accel_timer, ktime_set(0, KIONIX_ACCEL_POLLING_PERIOD),
		       HRTIMER_MODE_REL);

	return 0;
}

static int kionix_accel_grp4_standby(struct kionix_accel_driver *acceld)
{
	int err;

	//if(acceld->accel_drdy == 0)
		//cancel_delayed_work_sync(&acceld->accel_work);

	err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP4_CTRL_REG1, 0);
	if (err < 0)
		return err;

	return 0;
}

static void kionix_accel_grp4_report_accel_data(struct kionix_accel_driver *acceld)
{
	struct { union {
		s16 accel_data_s16[3];
		s8	accel_data_s8[6];
	}; } accel_data;
	s16 x, y, z;
	int err;
	struct input_dev *input_dev = acceld->input_dev;
	int loop;
	//printk("wangdong gsensor atomic_read start !\n");
	/* Only read the output registers if enabled */
	if(atomic_read(&acceld->accel_enabled) > 0) {
		if(atomic_read(&acceld->accel_enable_resume) > 0)
		{
			loop = KIONIX_I2C_RETRY_COUNT;
			while(loop) {
				mutex_lock(&input_dev->mutex);
				 //accel_data.accel_data_s16 = i2c_smbus_read_byte_data(acceld->client, ACCEL_GRP4_XOUT_L);
				err = kionix_i2c_read(acceld->client, ACCEL_GRP4_XOUT_L, (u8 *)accel_data.accel_data_s16, 6);
				mutex_unlock(&input_dev->mutex);
				if(err < 0){
					//loop--;
					//mdelay(KIONIX_I2C_RETRY_TIMEOUT);
					loop = 0;
				}
				else
					loop = 0;
			}
			if (err < 0) {
				KMSGERR(&acceld->client->dev, "%s: read data output error = %d\n", __func__, err);
				//printk("wangdong gsensor kionix_i2c_read err:%d!\n",err);
			}
			else {
				write_lock(&acceld->rwlock_accel_data);

				x = ((s16) le16_to_cpu(accel_data.accel_data_s16[acceld->axis_map_x])) >> acceld->shift;
				y = ((s16) le16_to_cpu(accel_data.accel_data_s16[acceld->axis_map_y])) >> acceld->shift;
				z = ((s16) le16_to_cpu(accel_data.accel_data_s16[acceld->axis_map_z])) >> acceld->shift;

				acceld->accel_data[acceld->axis_map_x] = (acceld->negate_x ? -x : x) + acceld->accel_cali[acceld->axis_map_x];
				acceld->accel_data[acceld->axis_map_y] = (acceld->negate_y ? -y : y) + acceld->accel_cali[acceld->axis_map_y];
				acceld->accel_data[acceld->axis_map_z] = (acceld->negate_z ? -z : z) + acceld->accel_cali[acceld->axis_map_z];
				
				//printk("wangdong gsensor report data X:%d;Y:%d;Z:%d!!!\n", acceld->accel_data[acceld->axis_map_x], acceld->accel_data[acceld->axis_map_y],acceld->accel_data[acceld->axis_map_z] );
				
				if(atomic_read(&acceld->accel_input_event) > 0) {
					input_report_abs(acceld->input_dev, ABS_X, acceld->accel_data[acceld->axis_map_x]);
					input_report_abs(acceld->input_dev, ABS_Y, acceld->accel_data[acceld->axis_map_y]);
					input_report_abs(acceld->input_dev, ABS_Z, acceld->accel_data[acceld->axis_map_z]);
					input_sync(acceld->input_dev);
				}
				//printk("wangdong gsensor input_report_abs end !\n");
				write_unlock(&acceld->rwlock_accel_data);
			}
		}
		else
		{
			atomic_inc(&acceld->accel_enable_resume);
			//printk("wangdong gsensor atomic_inc end !\n");
		}
	}

	/* Clear the interrupt if using drdy */
	if(acceld->accel_drdy == 1) {
		loop = KIONIX_I2C_RETRY_COUNT;
		while(loop) {
			err = i2c_smbus_read_byte_data(acceld->client, ACCEL_GRP4_INT_REL);
			if(err < 0){
				loop--;
				mdelay(KIONIX_I2C_RETRY_TIMEOUT);
			}
			else
				loop = 0;
		}
		if (err < 0)
			KMSGERR(&acceld->client->dev, "%s: clear interrupt error = %d\n", __func__, err);
	}
}

static void kionix_accel_grp4_update_g_range(struct kionix_accel_driver *acceld)
{
	acceld->accel_registers[accel_grp4_ctrl_reg1] &= ~ACCEL_GRP4_G_MASK;

	switch (acceld->accel_pdata.accel_g_range) {
		case KIONIX_ACCEL_G_8G:
		case KIONIX_ACCEL_G_6G:
			acceld->shift = 2;
			acceld->accel_registers[accel_grp4_ctrl_reg1] |= ACCEL_GRP4_G_8G;
			break;
		case KIONIX_ACCEL_G_4G:
			acceld->shift = 3;
			acceld->accel_registers[accel_grp4_ctrl_reg1] |= ACCEL_GRP4_G_4G;
			break;
		case KIONIX_ACCEL_G_2G:
		default:
			acceld->shift = 4;
			acceld->accel_registers[accel_grp4_ctrl_reg1] |= ACCEL_GRP4_G_2G;
			break;
	}

	return;
}

static int kionix_accel_grp4_update_odr(struct kionix_accel_driver *acceld, unsigned int poll_interval)
{
	int err;
	int i;
	u8 odr;

	/* Use the lowest ODR that can support the requested poll interval */
	for (i = 0; i < ARRAY_SIZE(kionix_accel_grp4_odr_table); i++) {
		odr = kionix_accel_grp4_odr_table[i].mask;
		if (poll_interval < kionix_accel_grp4_odr_table[i].cutoff)
			break;
	}

	/* Do not need to update DATA_CTRL_REG register if the ODR is not changed */
	if(acceld->accel_registers[accel_grp4_data_ctrl] == odr)
		return 0;
	else
		acceld->accel_registers[accel_grp4_data_ctrl] = odr;

	/* Do not need to update DATA_CTRL_REG register if the sensor is not currently turn on */
	if(atomic_read(&acceld->accel_enabled) > 0) {
		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP4_CTRL_REG1, 0);
		if (err < 0)
			return err;

		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP4_DATA_CTRL, acceld->accel_registers[accel_grp4_data_ctrl]);
		if (err < 0)
			return err;

		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP4_CTRL_REG1, acceld->accel_registers[accel_grp4_ctrl_reg1] | ACCEL_GRP4_PC1_ON);
		if (err < 0)
			return err;
		//#############
		err = i2c_smbus_read_byte_data(acceld->client, ACCEL_GRP4_DATA_CTRL);
		if (err < 0)
			return err;
		switch(err) {
			case ACCEL_GRP4_ODR0_781:
				dev_info(&acceld->client->dev, "ODR = 0.781 Hz\n");
				break;
			case ACCEL_GRP4_ODR1_563:
				dev_info(&acceld->client->dev, "ODR = 1.563 Hz\n");
				break;
			case ACCEL_GRP4_ODR3_125:
				dev_info(&acceld->client->dev, "ODR = 3.125 Hz\n");
				break;
			case ACCEL_GRP4_ODR6_25:
				dev_info(&acceld->client->dev, "ODR = 6.25 Hz\n");
				break;
			case ACCEL_GRP4_ODR12_5:
				dev_info(&acceld->client->dev, "ODR = 12.5 Hz\n");
				break;
			case ACCEL_GRP4_ODR25:
				dev_info(&acceld->client->dev, "ODR = 25 Hz\n");
				break;
			case ACCEL_GRP4_ODR50:
				dev_info(&acceld->client->dev, "ODR = 50 Hz\n");
				break;
			case ACCEL_GRP4_ODR100:
				dev_info(&acceld->client->dev, "ODR = 100 Hz\n");
				break;
			case ACCEL_GRP4_ODR200:
				dev_info(&acceld->client->dev, "ODR = 200 Hz\n");
				break;
			case ACCEL_GRP4_ODR400:
				dev_info(&acceld->client->dev, "ODR = 400 Hz\n");
				break;
			case ACCEL_GRP4_ODR800:
				dev_info(&acceld->client->dev, "ODR = 800 Hz\n");
				break;
			case ACCEL_GRP4_ODR1600:
				dev_info(&acceld->client->dev, "ODR = 1600 Hz\n");
				break;
			default:
				dev_info(&acceld->client->dev, "Unknown ODR\n");
				break;
		}
		//#############
	}

	return 0;
}

static int kionix_accel_grp7_power_on_init(struct kionix_accel_driver *acceld)
{
	int err;

	/* ensure that PC1 is cleared before updating control registers */
	err = i2c_smbus_write_byte_data(acceld->client,
					ACCEL_GRP7_CTRL_REG1, 0);
	if (err < 0)
		return err;

	err = i2c_smbus_write_byte_data(acceld->client,
					ACCEL_GRP7_DATA_CTRL, acceld->accel_registers[accel_grp7_data_ctrl]);
	if (err < 0)
		return err;

	/* only write INT_CTRL_REG1 if in irq mode */
	if (acceld->client->irq) {
		
		err = i2c_smbus_write_byte_data(acceld->client,
						ACCEL_GRP7_INT_CTRL1, acceld->accel_registers[accel_grp7_int_ctrl]);
		if (err < 0)
			return err;
			
	}

	if(atomic_read(&acceld->accel_enabled) > 0) {
		err = i2c_smbus_write_byte_data(acceld->client,
						ACCEL_GRP7_CTRL_REG1, acceld->accel_registers[accel_grp7_ctrl_reg1] | ACCEL_GRP7_PC1_ON);
		if (err < 0)
			return err;
	}
	else {
		err = i2c_smbus_write_byte_data(acceld->client,
						ACCEL_GRP7_CTRL_REG1, acceld->accel_registers[accel_grp7_ctrl_reg1]);
		if (err < 0)
			return err;
	}

	return 0;
}

static int kionix_accel_grp7_operate(struct kionix_accel_driver *acceld)
{
	int err;

	if(acceld->accel_drdy == 1) {		
								
		err = i2c_smbus_write_byte_data(acceld->client, 
		ACCEL_GRP7_CTRL_REG1, acceld->accel_registers[accel_grp7_ctrl_reg1] & ACCEL_GRP7_PC1_OFF);
		if (err < 0)
			return err;		
		
		acceld->accel_registers[accel_grp7_ctrl_reg1] |= ACCEL_GRP4_DRDYE;		
		acceld->accel_registers[accel_grp7_int4_ctrl] |= ACCEL_GRP7_DRDYI1;
		
		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP7_CTRL_REG1,
			acceld->accel_registers[accel_grp7_ctrl_reg1]);
		if (err < 0)
			return err;	
			
		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP7_INT_CTRL4,
			acceld->accel_registers[accel_grp7_int4_ctrl]);
		if (err < 0)
			return err;	
	}
	
	acceld->accel_registers[accel_grp7_ctrl_reg1] |= ACCEL_GRP7_PC1_ON;	
	err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP7_CTRL_REG1,
		acceld->accel_registers[accel_grp7_ctrl_reg1]);
	if (err < 0)
		return err;

	if(acceld->accel_drdy == 0)
		//queue_delayed_work(acceld->accel_workqueue, &acceld->accel_work, 0);
		hrtimer_start(&acceld->accel_timer, 
			ktime_set(acceld->poll_interval / 1000, (acceld->poll_interval % 1000) * 1000000),
			HRTIMER_MODE_REL);

	return 0;
}

static int kionix_accel_grp7_standby(struct kionix_accel_driver *acceld)
{
	int err;

	if(acceld->accel_drdy == 0) {
		//cancel_delayed_work_sync(&acceld->accel_work);		
		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP7_CTRL_REG1, 0);
		if (err < 0)
			return err;
	}

	if(acceld->accel_drdy == 1) {
		
		acceld->accel_registers[accel_grp7_ctrl_reg1] &= ACCEL_GRP7_PC1_OFF;		
		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP7_CTRL_REG1, 
			acceld->accel_registers[accel_grp7_ctrl_reg1]);
		if (err < 0)
			return err;			
			
		acceld->accel_registers[accel_grp7_ctrl_reg1] &= ~ACCEL_GRP7_DRDYE;
		
		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP7_CTRL_REG1, 
			acceld->accel_registers[accel_grp7_ctrl_reg1]);				
		if (err < 0)
			return err;	
			
		/* wufe is on so set pci bit on*/	
		if(atomic_read(&acceld->accel_wuf_enabled) == 1) {
			acceld->accel_registers[accel_grp7_ctrl_reg1] |= ACCEL_GRP7_PC1_OFF;		
			err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP7_CTRL_REG1, 
			acceld->accel_registers[accel_grp7_ctrl_reg1]);
		}				
	}

	return 0;
}

static void kionix_accel_grp7_report_accel_data(struct kionix_accel_driver *acceld)
{
	struct { union {
		s16 accel_data_s16[3];
		s8	accel_data_s8[6];
	}; } accel_data;
	s16 x, y, z;
	int err = 0;
	int interrupt_val = 0;
	struct input_dev *input_dev = acceld->input_dev;
	int loop;

	if(acceld->accel_drdy == 1) {
		interrupt_val = i2c_smbus_read_byte_data(acceld->client, ACCEL_GRP7_INS2);
		//KMSGINF(&acceld->client->dev, "%s: ins2 val: %x\n", __func__, interrupt_val);	
		if (interrupt_val < 0){
			KMSGERR(&acceld->client->dev, "%s: reading interrupt cause error  = %d\n", __func__, err);
		}
		/* Check if wufe interrupt occured */	
		if ( interrupt_val & ACCEL_GRP7_INS2_WUFS){
			/* TODO: Add event to deliver wake up info to hal */
			KMSGINF(&acceld->client->dev, "wufe interrupt occured\n");				
		} 
	}	

	if( acceld->accel_drdy == 0 || 
		(acceld->accel_drdy == 1 && interrupt_val & ACCEL_GRP7_INS2_DRDY) ) {		
		if(atomic_read(&acceld->accel_enabled) > 0) {
			if(atomic_read(&acceld->accel_enable_resume) > 0)
			{
				loop = KIONIX_I2C_RETRY_COUNT;
				while(loop) {
					mutex_lock(&input_dev->mutex);
					err = kionix_i2c_read(acceld->client, ACCEL_GRP7_XOUT_L, (u8 *)accel_data.accel_data_s16, 6);
					mutex_unlock(&input_dev->mutex);
					if(err < 0){
						loop--;
						mdelay(KIONIX_I2C_RETRY_TIMEOUT);
					}
					else
						loop = 0;
				}
				if (err < 0) {
					KMSGERR(&acceld->client->dev, "%s: read data output error = %d\n", __func__, err);
				}
				else {
					write_lock(&acceld->rwlock_accel_data);

					x = ((s16) le16_to_cpu(accel_data.accel_data_s16[acceld->axis_map_x])) >> acceld->shift;
					y = ((s16) le16_to_cpu(accel_data.accel_data_s16[acceld->axis_map_y])) >> acceld->shift;
					z = ((s16) le16_to_cpu(accel_data.accel_data_s16[acceld->axis_map_z])) >> acceld->shift;

					acceld->accel_data[acceld->axis_map_x] = (acceld->negate_x ? -x : x) + acceld->accel_cali[acceld->axis_map_x];
					acceld->accel_data[acceld->axis_map_y] = (acceld->negate_y ? -y : y) + acceld->accel_cali[acceld->axis_map_y];
					acceld->accel_data[acceld->axis_map_z] = (acceld->negate_z ? -z : z) + acceld->accel_cali[acceld->axis_map_z];

					if(atomic_read(&acceld->accel_input_event) > 0) {
						input_report_abs(acceld->input_dev, ABS_X, acceld->accel_data[acceld->axis_map_x]);
						input_report_abs(acceld->input_dev, ABS_Y, acceld->accel_data[acceld->axis_map_y]);
						input_report_abs(acceld->input_dev, ABS_Z, acceld->accel_data[acceld->axis_map_z]);
						input_sync(acceld->input_dev);
					}

					write_unlock(&acceld->rwlock_accel_data);
				}
			}
			else
			{
				atomic_inc(&acceld->accel_enable_resume);
			}
		}
	}
	
	if(acceld->accel_drdy == 1 && interrupt_val > 0 ) {
		loop = KIONIX_I2C_RETRY_COUNT;
		while(loop) {
			err = i2c_smbus_read_byte_data(acceld->client, ACCEL_GRP7_INT_REL);
			if(err < 0){
				loop--;
				mdelay(KIONIX_I2C_RETRY_TIMEOUT);
			}
			else
				loop = 0;
		}
		if (err < 0)
			KMSGERR(&acceld->client->dev, "%s: clear interrupt error = %d\n", __func__, err);
	}
}

static void kionix_accel_grp7_update_g_range(struct kionix_accel_driver *acceld)
{
	acceld->accel_registers[accel_grp7_ctrl_reg1] &= ~ACCEL_GRP7_G_MASK;

	switch (acceld->accel_pdata.accel_g_range) {
		case KIONIX_ACCEL_G_8G:
		case KIONIX_ACCEL_G_6G:
			acceld->shift = 0;
			acceld->accel_registers[accel_grp7_ctrl_reg1] |= ACCEL_GRP7_G_8G;
			break;
		case KIONIX_ACCEL_G_4G:
			acceld->shift = 0;
			acceld->accel_registers[accel_grp7_ctrl_reg1] |= ACCEL_GRP7_G_4G;
			break;
		case KIONIX_ACCEL_G_2G:
		default:
			acceld->shift = 0;
			acceld->accel_registers[accel_grp7_ctrl_reg1] |= ACCEL_GRP7_G_2G;
			break;
	}

	return;
}

static int kionix_accel_grp7_update_odr(struct kionix_accel_driver *acceld, unsigned int poll_interval)
{
	int err;
	int i;
	u8 odr;

	/* Use the lowest ODR that can support the requested poll interval */
	for (i = 0; i < ARRAY_SIZE(kionix_accel_grp7_odr_table); i++) {
		odr = kionix_accel_grp7_odr_table[i].mask;
		if (poll_interval < kionix_accel_grp7_odr_table[i].cutoff)
			break;
	}

	/* Do not need to update DATA_CTRL_REG register if the ODR is not changed */
	if(acceld->accel_registers[accel_grp7_data_ctrl] == odr)
		return 0;
	else
		acceld->accel_registers[accel_grp7_data_ctrl] = odr;

	/* Do not need to update DATA_CTRL_REG register if the sensor is not currently turn on */
	if(atomic_read(&acceld->accel_enabled) > 0) {
		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP7_CTRL_REG1, 0);
		if (err < 0)
			return err;

		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP7_DATA_CTRL, acceld->accel_registers[accel_grp7_data_ctrl]);
		if (err < 0)
			return err;

		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP7_CTRL_REG1, acceld->accel_registers[accel_grp7_ctrl_reg1] | ACCEL_GRP7_PC1_ON);
		if (err < 0)
			return err;
		//#############
		err = i2c_smbus_read_byte_data(acceld->client, ACCEL_GRP7_DATA_CTRL);
		if (err < 0)
			return err;
		switch(err) {
			case ACCEL_GRP7_ODR0_781:
				dev_info(&acceld->client->dev, "ODR = 0.781 Hz\n");
				break;
			case ACCEL_GRP7_ODR1_563:
				dev_info(&acceld->client->dev, "ODR = 1.563 Hz\n");
				break;
			case ACCEL_GRP7_ODR3_125:
				dev_info(&acceld->client->dev, "ODR = 3.125 Hz\n");
				break;
			case ACCEL_GRP7_ODR6_25:
				dev_info(&acceld->client->dev, "ODR = 6.25 Hz\n");
				break;
			case ACCEL_GRP7_ODR12_5:
				dev_info(&acceld->client->dev, "ODR = 12.5 Hz\n");
				break;
			case ACCEL_GRP7_ODR25:
				dev_info(&acceld->client->dev, "ODR = 25 Hz\n");
				break;
			case ACCEL_GRP7_ODR50:
				dev_info(&acceld->client->dev, "ODR = 50 Hz\n");
				break;
			case ACCEL_GRP7_ODR100:
				dev_info(&acceld->client->dev, "ODR = 100 Hz\n");
				break;
			case ACCEL_GRP7_ODR200:
				dev_info(&acceld->client->dev, "ODR = 200 Hz\n");
				break;
			case ACCEL_GRP7_ODR400:
				dev_info(&acceld->client->dev, "ODR = 400 Hz\n");
				break;
			case ACCEL_GRP7_ODR800:
				dev_info(&acceld->client->dev, "ODR = 800 Hz\n");
				break;
			case ACCEL_GRP7_ODR1600:
				dev_info(&acceld->client->dev, "ODR = 1600 Hz\n");
				break;
			default:
				dev_info(&acceld->client->dev, "Unknown ODR\n");
				break;
		}
		//#############
	}

	return 0;
}

static int kionix_accel_grp7_wufe(struct kionix_accel_driver *acceld, int enable) {

	int err =0;
	
	KMSGINF(&acceld->client->dev, ">> kionix_accel_grp7_wufe\n");		
	/* Write wufe values to local cache */
	if(enable) {				
		KMSGINF(&acceld->client->dev, ">> enable wufe\n");		
		/* Enable wufe function */
		acceld->accel_registers[accel_grp7_ctrl_reg1] |= ACCEL_GRP7_WUFE_ON;
		/* Enable all directions for wufe */
		acceld->accel_registers[accel_grp7_int2_ctrl] |= ACCEL_GRP7_INT_CTRL2_MASK;			
		/* Set motion wakeup ODR */		
		acceld->accel_registers[accel_grp7_ctrl_reg3] &= ~ACCEL_GRP7_CNTL3_OWUF_MASK;
		acceld->accel_registers[accel_grp7_ctrl_reg3] |= ACCEL_GRP7_CNTL3_OWUF_50;		
		/* Route wufe to interrupt pin 1*/
		acceld->accel_registers[accel_grp7_int4_ctrl] |= ACCEL_GRP7_WUFI1;
		/* WUFE Settings */
		acceld->accel_registers[accel_grp7_wufc] = ACCEL_GRP7_WUFC_VAL;
		acceld->accel_registers[accel_grp7_ath] =  ACCEL_GRP7_ATH_VAL;
		/* Write PCI off */				
		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP7_CTRL_REG1, 
			acceld->accel_registers[accel_grp7_ctrl_reg1] & ACCEL_GRP7_PC1_OFF);
		if (err < 0)
			return err;
					
		/* Write int control 2 */
		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP7_INT_CTRL2, acceld->accel_registers[accel_grp7_int2_ctrl]);
		if (err < 0)
			return err;
		/* Write int control 3 */
		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP7_CTRL_REG3, acceld->accel_registers[accel_grp7_ctrl_reg3]);
		if (err < 0)
			return err;			
		/* Write int control 4 */
		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP7_INT_CTRL4, acceld->accel_registers[accel_grp7_int4_ctrl]);
		if (err < 0)
			return err;					
		/* Write int wufc */
		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP7_WUFC, acceld->accel_registers[accel_grp7_wufc]);
		if (err < 0)
			return err;				
		/* Write int ath */
		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP7_ATH, acceld->accel_registers[accel_grp7_ath]);
		if (err < 0)
			return err;			
		/* Write PCI bit on */
		acceld->accel_registers[accel_grp7_ctrl_reg1] |= ACCEL_GRP7_PC1_ON;
		/* Write control 1 */
		err = i2c_smbus_write_byte_data(acceld->client, 
			ACCEL_GRP7_CTRL_REG1, acceld->accel_registers[accel_grp7_ctrl_reg1]);
		if (err < 0)
			return err;	
		
		atomic_inc(&acceld->accel_wuf_enabled);	
		KMSGINF(&acceld->client->dev, ">> register write done\n");		
											
	} else {
		
		acceld->accel_registers[accel_grp7_ctrl_reg1] &= ACCEL_GRP7_PC1_OFF;		
		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP7_CTRL_REG1, 
			acceld->accel_registers[accel_grp7_ctrl_reg1]);
		if (err < 0)
			return err;
			
		/* Wufe feature off */		
		acceld->accel_registers[accel_grp7_ctrl_reg1] &= ACCEL_GRP7_WUFE_OFF;
		/* Interrupt routing off */
		acceld->accel_registers[accel_grp7_int4_ctrl] &= ~ACCEL_GRP7_WUFI1;
		
		/* Write int 4 control*/
		err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP7_INT_CTRL4, acceld->accel_registers[accel_grp7_int4_ctrl]);
		if (err < 0)
			return err;			
		/* Write control 1 */
		acceld->accel_registers[accel_grp7_ctrl_reg1] |= ACCEL_GRP7_PC1_ON;
		err = i2c_smbus_write_byte_data(acceld->client, 
			ACCEL_GRP7_CTRL_REG1, acceld->accel_registers[accel_grp7_ctrl_reg1]);
		if (err < 0)
			return err;							
		
		if(atomic_read(&acceld->accel_wuf_enabled) == 1) {
			atomic_dec(&acceld->accel_wuf_enabled);				
		}
	}
	KMSGINF(&acceld->client->dev, "<< kionix_accel_grp7_wufe\n");		
	return err;
}

static int kionix_accel_power_on(struct kionix_accel_driver *acceld)
{
	if (acceld->accel_pdata.power_on)
		return acceld->accel_pdata.power_on();

	return 0;
}

static void kionix_accel_power_off(struct kionix_accel_driver *acceld)
{
	if (acceld->accel_pdata.power_off)
		acceld->accel_pdata.power_off();
}

static irqreturn_t kionix_accel_isr(int irq, void *dev)
{
	struct kionix_accel_driver *acceld = dev;

	//if(acceld->accel_drdy == 0) 	
		//queue_delayed_work(acceld->accel_workqueue, &acceld->accel_work, 0);

	acceld->kionix_accel_report_accel_data(acceld);
		
	return IRQ_HANDLED;
}

#if 0
static void kionix_accel_work(struct work_struct *work)
{
	struct kionix_accel_driver *acceld = container_of((struct delayed_work *)work,	struct kionix_accel_driver, accel_work);

	if(acceld->accel_drdy == 0)
		queue_delayed_work(acceld->accel_workqueue, &acceld->accel_work, acceld->poll_delay);

	acceld->kionix_accel_report_accel_data(acceld);
}
#endif

static void kionix_accel_update_direction(struct kionix_accel_driver *acceld)
{
	unsigned int direction = acceld->accel_pdata.accel_direction;
	unsigned int accel_group = acceld->accel_group;

	write_lock(&acceld->rwlock_accel_data);
	acceld->axis_map_x = ((direction-1)%2);
	acceld->axis_map_y =  (direction%2);
	acceld->axis_map_z =  2;
	acceld->negate_z = ((direction-1)/4);
	switch(accel_group) {
		case KIONIX_ACCEL_GRP3:
		case KIONIX_ACCEL_GRP6:
			acceld->negate_x = (((direction+2)/2)%2);
			acceld->negate_y = (((direction+5)/4)%2);
			break;
		case KIONIX_ACCEL_GRP5:
			acceld->axis_map_x =  (direction%2);
			acceld->axis_map_y = ((direction-1)%2);
			acceld->negate_x =  (((direction+1)/2)%2);
			acceld->negate_y =  (((direction/2)+((direction-1)/4))%2);
			break;
		default:
			acceld->negate_x =  ((direction/2)%2);
			acceld->negate_y = (((direction+1)/4)%2);
			break;
	}
	write_unlock(&acceld->rwlock_accel_data);
	return;
}

static int kionix_accel_enable(struct kionix_accel_driver *acceld)
{
	int err = 0;
	long remaining;
	printk("%s: Line:%d!\n", __func__,__LINE__);
	kionix_load_cali(acceld);
	printk("%s: Line:%d!\n", __func__,__LINE__);

	mutex_lock(&acceld->mutex_earlysuspend);

	atomic_set(&acceld->accel_suspend_continue, 0);

	/* Make sure that the sensor had successfully resumed before enabling it */
	if(atomic_read(&acceld->accel_suspended) == 1) {
		KMSGINF(&acceld->client->dev, "%s: waiting for resume\n", __func__);
		remaining = wait_event_interruptible_timeout(acceld->wqh_suspend, \
				atomic_read(&acceld->accel_suspended) == 0, \
				msecs_to_jiffies(KIONIX_ACCEL_EARLYSUSPEND_TIMEOUT));

		if(atomic_read(&acceld->accel_suspended) == 1) {
			KMSGERR(&acceld->client->dev, "%s: timeout waiting for resume\n", __func__);
			err = -ETIME;
			goto exit;
		}
	}

	err = acceld->kionix_accel_operate(acceld);

	if (err < 0) {
		KMSGERR(&acceld->client->dev, \
				"%s: kionix_accel_operate returned err = %d\n", __func__, err);
		goto exit;
	}

	atomic_inc(&acceld->accel_enabled);

exit:
	mutex_unlock(&acceld->mutex_earlysuspend);

	return err;
}

static int kionix_accel_disable(struct kionix_accel_driver *acceld)
{
	int err = 0;

	mutex_lock(&acceld->mutex_resume);

	atomic_set(&acceld->accel_suspend_continue, 1);

	if(atomic_read(&acceld->accel_enabled) > 0){
		if(atomic_dec_and_test(&acceld->accel_enabled)) {
			if(atomic_read(&acceld->accel_enable_resume) > 0)
				atomic_set(&acceld->accel_enable_resume, 0);
			err = acceld->kionix_accel_standby(acceld);
			if (err < 0) {
				KMSGERR(&acceld->client->dev, \
						"%s: kionix_accel_standby returned err = %d\n", __func__, err);
				goto exit;
			}
			wake_up_interruptible(&acceld->wqh_suspend);
		}
	}

exit:
	mutex_unlock(&acceld->mutex_resume);

	return err;
}

static int kionix_accel_input_open(struct input_dev *input)
{
	struct kionix_accel_driver *acceld = input_get_drvdata(input);

	atomic_inc(&acceld->accel_input_event);

	return 0;
}

static void kionix_accel_input_close(struct input_dev *dev)
{
	struct kionix_accel_driver *acceld = input_get_drvdata(dev);

	atomic_dec(&acceld->accel_input_event);
}

static void kionix_accel_init_input_device(struct kionix_accel_driver *acceld,
					      struct input_dev *input_dev)
{
	__set_bit(EV_ABS, input_dev->evbit);
	input_set_abs_params(input_dev, ABS_X, -ACCEL_G_MAX, ACCEL_G_MAX, ACCEL_FUZZ, ACCEL_FLAT);
	input_set_abs_params(input_dev, ABS_Y, -ACCEL_G_MAX, ACCEL_G_MAX, ACCEL_FUZZ, ACCEL_FLAT);
	input_set_abs_params(input_dev, ABS_Z, -ACCEL_G_MAX, ACCEL_G_MAX, ACCEL_FUZZ, ACCEL_FLAT);
	
	input_dev->name = KIONIX_ACCEL_NAME;
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &acceld->client->dev;
}

static int kionix_accel_setup_input_device(struct kionix_accel_driver *acceld)
{
	struct input_dev *input_dev;
	int err;

	
	input_dev = input_allocate_device();
	if (!input_dev) {
		KMSGERR(&acceld->client->dev, "input_allocate_device failed\n");
		return -ENOMEM;
	}
	
	acceld->input_dev = input_dev;

	input_dev->open = kionix_accel_input_open;
	input_dev->close = kionix_accel_input_close;
	input_set_drvdata(input_dev, acceld);

	kionix_accel_init_input_device(acceld, input_dev);

	err = input_register_device(acceld->input_dev);
	if (err) {
		KMSGERR(&acceld->client->dev, \
				"%s: input_register_device returned err = %d\n", __func__, err);
		input_free_device(acceld->input_dev);
		return err;
	}

	return 0;
}

/* Returns the enable state of device */
static ssize_t kionix_accel_get_enable(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct kionix_accel_driver *acceld = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&acceld->accel_enabled) > 0 ? 1 : 0);
}

/* Allow users to enable/disable the device */
static ssize_t kionix_accel_set_enable(struct device *dev, struct device_attribute *attr,
						const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct kionix_accel_driver *acceld = i2c_get_clientdata(client);
	struct input_dev *input_dev = acceld->input_dev;
	char *buf2;
	const int enable_count = 1;
	unsigned long enable;
	int err = 0;

	/* Lock the device to prevent races with open/close (and itself) */
	mutex_lock(&input_dev->mutex);

	if(kionix_strtok(buf, count, &buf2, enable_count) < 0) {
		KMSGERR(&acceld->client->dev, \
				"%s: No enable data being read. " \
				"No enable data will be updated.\n", __func__);
	}

	else {
		/* Removes any leading negative sign */
		while(*buf2 == '-')
			buf2++;
		#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35))
		err = kstrtouint((const char *)buf2, 10, (unsigned int *)&enable);
		if (err < 0) {
			KMSGERR(&acceld->client->dev, \
					"%s: kstrtouint returned err = %d\n", __func__, err);
			goto exit;
		}
		#else
		err = strict_strtoul((const char *)buf2, 10, &enable);
		if (err < 0) {
			KMSGERR(&acceld->client->dev, \
					"%s: strict_strtoul returned err = %d\n", __func__, err);
			goto exit;
		}
		#endif

		if(enable)
			err = kionix_accel_enable(acceld);
		else
			err = kionix_accel_disable(acceld);
	}

exit:
	mutex_unlock(&input_dev->mutex);

	return (err < 0) ? err : count;
}

/* Returns currently selected poll interval (in ms) */
static ssize_t kionix_accel_get_delay(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct kionix_accel_driver *acceld = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", acceld->poll_interval);
}

/* Allow users to select a new poll interval (in ms) */
static ssize_t kionix_accel_set_delay(struct device *dev, struct device_attribute *attr,
						const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct kionix_accel_driver *acceld = i2c_get_clientdata(client);
	struct input_dev *input_dev = acceld->input_dev;
	char *buf2;
	const int delay_count = 1;
	unsigned long interval;
	int err = 0;

	/* Lock the device to prevent races with open/close (and itself) */
	mutex_lock(&input_dev->mutex);

	if(kionix_strtok(buf, count, &buf2, delay_count) < 0) {
		KMSGERR(&acceld->client->dev, \
				"%s: No delay data being read. " \
				"No delay data will be updated.\n", __func__);
	}

	else {
		/* Removes any leading negative sign */
		while(*buf2 == '-')
			buf2++;
		#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35))
		err = kstrtouint((const char *)buf2, 10, (unsigned int *)&interval);
		if (err < 0) {
			KMSGERR(&acceld->client->dev, \
					"%s: kstrtouint returned err = %d\n", __func__, err);
			goto exit;
		}
		#else
		err = strict_strtoul((const char *)buf2, 10, &interval);
		if (err < 0) {
			KMSGERR(&acceld->client->dev, \
					"%s: strict_strtoul returned err = %d\n", __func__, err);
			goto exit;
		}
		#endif

		if(acceld->accel_drdy == 1)
			disable_irq(client->irq);

		/*
		 * Set current interval to the greater of the minimum interval or
		 * the requested interval
		 */		
		acceld->poll_interval = max((unsigned int)interval, acceld->accel_pdata.min_interval);
		acceld->poll_delay = msecs_to_jiffies(acceld->poll_interval);

		err = acceld->kionix_accel_update_odr(acceld, acceld->poll_interval);

		if(acceld->accel_drdy == 1)
			enable_irq(client->irq);
	}

exit:
	mutex_unlock(&input_dev->mutex);

	return (err < 0) ? err : count;
}

/* Returns the direction of device */
static ssize_t kionix_accel_get_direct(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct kionix_accel_driver *acceld = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", acceld->accel_pdata.accel_direction);
}

/* Allow users to change the direction the device */
static ssize_t kionix_accel_set_direct(struct device *dev, struct device_attribute *attr,
						const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct kionix_accel_driver *acceld = i2c_get_clientdata(client);
	struct input_dev *input_dev = acceld->input_dev;
	char *buf2;
	const int direct_count = 1;
	unsigned long direction;
	int err = 0;

	/* Lock the device to prevent races with open/close (and itself) */
	mutex_lock(&input_dev->mutex);

	if(kionix_strtok(buf, count, &buf2, direct_count) < 0) {
		KMSGERR(&acceld->client->dev, \
				"%s: No direction data being read. " \
				"No direction data will be updated.\n", __func__);
	}

	else {
		/* Removes any leading negative sign */
		while(*buf2 == '-')
			buf2++;
		#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35))
		err = kstrtouint((const char *)buf2, 10, (unsigned int *)&direction);
		if (err < 0) {
			KMSGERR(&acceld->client->dev, \
					"%s: kstrtouint returned err = %d\n", __func__, err);
			goto exit;
		}
		#else
		err = strict_strtoul((const char *)buf2, 10, &direction);
		if (err < 0) {
			KMSGERR(&acceld->client->dev, \
					"%s: strict_strtoul returned err = %d\n", __func__, err);
			goto exit;
		}
		#endif

		if(direction < 1 || direction > 8)
			KMSGERR(&acceld->client->dev, "%s: invalid direction = %d\n", __func__, (unsigned int) direction);

		else {
			acceld->accel_pdata.accel_direction = (u8) direction;
			kionix_accel_update_direction(acceld);
		}
	}

exit:
	mutex_unlock(&input_dev->mutex);

	return (err < 0) ? err : count;
}

/* Returns the data output of device */
static ssize_t kionix_accel_get_data(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct kionix_accel_driver *acceld = i2c_get_clientdata(client);
	int x, y, z;

	read_lock(&acceld->rwlock_accel_data);

	x = acceld->accel_data[acceld->axis_map_x];
	y = acceld->accel_data[acceld->axis_map_y];
	z = acceld->accel_data[acceld->axis_map_z];

	read_unlock(&acceld->rwlock_accel_data);

	return sprintf(buf, "%d %d %d\n", x, y, z);
}


static int kionix_write_file(int mode, char write_buf[])
{
	struct file  *cali_file;
	char r_buf[KIONIX_ACC_CALI_FILE_SIZE] = {0};
	mm_segment_t fs;
	ssize_t ret;
	int8_t i;
	int err;


	cali_file = filp_open(KIONIX_ACC_CALI_FILE, O_CREAT | O_RDWR,0666);
	//printk("%s: Line:%d!\n", __func__,__LINE__);
	if(IS_ERR(cali_file)) {
		err = PTR_ERR(cali_file);
		if(mode == 0)
			printk(KERN_ERR "%s: filp_open error!err=%d,path=%s\n", __func__, err, KIONIX_ACC_CALI_FILE);	
		return -KIONIX_K_FAIL_OPEN_FILE;
	} else {
		fs = get_fs();
		set_fs(get_ds());
		//printk("%s: Line:%d!\n", __func__,__LINE__);
		if((write_buf==NULL) ||(cali_file ==NULL))
		{
			printk("%s: Line:%d!\n", __func__,__LINE__);
		}
		if(&cali_file->f_pos ==NULL)
		{
			printk("%s: Line:%d!\n", __func__,__LINE__);
		}

		ret = vfs_write(cali_file,write_buf, KIONIX_ACC_CALI_FILE_SIZE,&cali_file->f_pos);
		//ret = cali_file->f_op->write(cali_file,write_buf, KIONIX_ACC_CALI_FILE_SIZE,&cali_file->f_pos);
		if(ret != KIONIX_ACC_CALI_FILE_SIZE) {
			printk(KERN_ERR "%s: write error!\n", __func__);
			filp_close(cali_file,NULL);
			return -KIONIX_K_FAIL_W_FILE;
		}
		
		cali_file->f_pos=0x00;
		//printk("%s: Line:%d!\n", __func__,__LINE__);
		ret = vfs_read(cali_file,r_buf, KIONIX_ACC_CALI_FILE_SIZE,&cali_file->f_pos);
		//ret = cali_file->f_op->read(cali_file,r_buf, KIONIX_ACC_CALI_FILE_SIZE,&cali_file->f_pos);
		if(ret < 0) {
			printk(KERN_ERR "%s: read error!\n", __func__);
			filp_close(cali_file,NULL);
			return -KIONIX_K_FAIL_R_BACK;
		}
		set_fs(fs);
		//printk("%s: Line:%d!\n", __func__,__LINE__);

		for(i=0; i<KIONIX_ACC_CALI_FILE_SIZE; i++) {
			if(r_buf[i] != write_buf[i]) {
				printk(KERN_ERR "%s: read back error, r_buf[%x](0x%x) != write_buf[%x](0x%x)\n", __func__, i, r_buf[i], i, write_buf[i]);
				filp_close(cali_file,NULL);
				return -KIONIX_K_FAIL_R_BACK_COMP;
			}
		}
	}
	filp_close(cali_file,NULL);
	//kfree(cali_file);
	return 0;
}



static int kionix_store_in_file(int offset[], u8 status, u32 variance[])
{
	int err;
	char file_buf[KIONIX_ACC_CALI_FILE_SIZE];
	char cali_buf[13] = {0};
	int value = 0;

	memset(file_buf, 0, sizeof(file_buf));
	//printk("%s: Line:%d:!\n", __func__,__LINE__);

	value = offset[0];
	sprintf(cali_buf, "X=%d;\r\n",value);
	//printk("%s: Line:%d:cali_buf:%s!\n", __func__,__LINE__,cali_buf);
	strcat(file_buf,cali_buf);
	//printk("%s: Line:%d:file_buf:%s!\n", __func__,__LINE__,file_buf);

	value = offset[1];
	sprintf(cali_buf, "Y=%d;\r\n",value);
	strcat(file_buf,cali_buf);

	value = offset[2];
	sprintf(cali_buf, "Z=%d;\r\n",value);
	strcat(file_buf,cali_buf);

	printk("%s: Line:%d:file_buf:%s!\n", __func__,__LINE__,file_buf);

	//strcat(cali_buf_x,offset[0]);
	//kionix_write_file(1, file_buf);
	err = kionix_write_file(0, file_buf);
	if(err == 0)
		printk(KERN_INFO "%s successfully\n", __func__);

	return err;
}


static int32_t kionix_get_file_content(char * r_buf, int8_t buf_size)
{
	struct file  *cali_file;
	mm_segment_t fs;
	ssize_t ret;
	int err;

	cali_file = filp_open(KIONIX_ACC_CALI_FILE, O_RDONLY,0);
	if(IS_ERR(cali_file)) {
		err = PTR_ERR(cali_file);
		printk(KERN_ERR  "%s: filp_open error, no offset file!err=%d\n", __func__, err);
		return -ENOENT;
	} else {
		fs = get_fs();
		set_fs(get_ds());

		//printk("%s: Line:%d!\n", __func__,__LINE__);
		ret = vfs_read(cali_file,r_buf, KIONIX_ACC_CALI_FILE_SIZE,&cali_file->f_pos);
		//ret = cali_file->f_op->read(cali_file,r_buf, KIONIX_ACC_CALI_FILE_SIZE,&cali_file->f_pos);
		if(ret < 0) {
			printk(KERN_ERR  "%s: read error, ret=%d\n", __func__, (int)ret);
			filp_close(cali_file,NULL);
			return -EIO;
		}
		set_fs(fs);
	}
	filp_close(cali_file,NULL);
	return 0;
}


static void kionix_load_cali(struct kionix_accel_driver *acceld)
{
	char kionix_file[KIONIX_ACC_CALI_FILE_SIZE];
	int offset[3];
	//int aa;
	int value = 0;
	char cali_buff[10] ={0};
	int err =0;
	char *flag_offset;
	char y='Y',z='Z';

	if ((kionix_get_file_content(kionix_file, KIONIX_ACC_CALI_FILE_SIZE)) == 0) {
		//strchr
		strncpy(cali_buff,kionix_file,8); 
		//printk("kionix_load_cali cali_buff:%s:! Line:%d!kionix_file:%s!\n", cali_buff,__LINE__,kionix_file);
		err = sscanf(cali_buff, "X=%d", &value);
		printk("kionix_load_cali cali_buff:%s:! err:%d!value_X:%d!\n", cali_buff,err,value);
		offset[0] = value;
		value = 0;

		flag_offset = strchr(kionix_file,y);
		//strncpy(cali_buff,kionix_file+7,10); 
		err = sscanf(flag_offset, "Y=%d", &value);
		printk("kionix_load_cali flag_offset:%s:! err:%d!value_Y:%d!\n", flag_offset,err,value);
		offset[1] = value;
		value = 0;

		flag_offset = strchr(kionix_file,z);
		//strncpy(cali_buff,kionix_file+16,8); 
		err = sscanf(flag_offset, "Z=%d", &value);
		printk("kionix_load_cali flag_offset:%s:! err:%d!value_Z:%d!\n", flag_offset,err,value);
		offset[2] = value;
		value = 0;

		printk("%s: set offset:%d,%d,%d\n", __func__,
		         offset[0], offset[1], offset[2]);
		
		acceld->accel_cali[acceld->axis_map_x] = (int)offset[0];
		acceld->accel_cali[acceld->axis_map_y] = (int)offset[1];
		acceld->accel_cali[acceld->axis_map_z] = (int)offset[2];

	}
	else {
		offset[0] = offset[1] = offset[2] = 0;
		kionix_store_in_file(offset, KIONIX_K_NO_CALI, NULL);
		printk("%s:kionix_store x:0 y:0 z:0! Line:%d!\n", __func__,__LINE__);
		//atomic_set(&acceld->cali_status, KIONIX_K_NO_CALI);
	}
	printk("%s:end! Line:%d!\n", __func__,__LINE__);

}

/*static int convert2s(int num)
{
	// Function that performs conversion of an unsigned integer into signed
	if (num & 0x800) {
		num -= 1;
		num = ~num;
		num = -num;
	 }
	return num;
}*/


static int kionix_set_cali_do(struct kionix_accel_driver *acceld)
{
	int sample_no, axis;
	int acc_ave[3] = {0, 0, 0};
	int offset[3];
	//u8 offset_in_reg[3];
	int result;
	int err;
	int data_raw[10][3] = {0};
	//char data[6];
	int i;
	s16 x, y, z;
	struct { union {
		s16 accel_data_s16[3];
		s8	accel_data_s8[6];
	}; } accel_data;
	
	//printk("%s:kionix_set_cali_do Line:%d!\n", __func__,__LINE__);
/*	// set standby, full power, 2g
	err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP4_CTRL_REG1, 0x40);//0x1B ---0x40
	if (err < 0)
		return err;

	// odr=50hz
	err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP4_DATA_CTRL, 0x02);//0x21 ---0x02
	if (err < 0)
		return err;

	// pc1=1
	err = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP4_CTRL_REG1, 0xC0);//0x1B ---0xC0
	if (err < 0)
		return err;
*/
	//msleep(100);
	//printk("%s:kionix_set_cali_do Line:%d!\n", __func__,__LINE__);
	
	for(sample_no=0; sample_no<KIONIX_SAMPLE_NO; sample_no++) {
		printk("%s:kionix_set_cali_do Line:%d!\n", __func__,__LINE__);
		msleep(100);
		//err = kionix_i2c_read(acceld->client, ACCEL_GRP4_XOUT_L, (u8 *)data, 6);
		printk("%s:kionix_set_cali_do Line:%d!\n", __func__,__LINE__);
		err = kionix_i2c_read(acceld->client, ACCEL_GRP4_XOUT_L, (u8 *)accel_data.accel_data_s16, 6);
		if (err < 0)
			return err;

		x = ((s16) le16_to_cpu(accel_data.accel_data_s16[acceld->axis_map_x])) >> acceld->shift;
		y = ((s16) le16_to_cpu(accel_data.accel_data_s16[acceld->axis_map_y])) >> acceld->shift;
		z = ((s16) le16_to_cpu(accel_data.accel_data_s16[acceld->axis_map_z])) >> acceld->shift;
		
		data_raw[sample_no][0] = (acceld->negate_x ? -x : x);
		data_raw[sample_no][1] = (acceld->negate_y ? -y : y);
		data_raw[sample_no][2] = (acceld->negate_z ? -z : z);
		printk("%s: acc=%d,%d,%d\n", __func__, data_raw[sample_no][0], data_raw[sample_no][1], data_raw[sample_no][2]);


/*		data_raw[sample_no][0] = ((((short int)data[1])<<4) + ((((short int)data[0])>>4)&0x0F));  
		data_raw[sample_no][1] = ((((short int)data[3])<<4) + ((((short int)data[2])>>4)&0x0F));
		data_raw[sample_no][2] = ((((short int)data[5])<<4) + ((((short int)data[4])>>4)&0x0F));
		printk("%s: acc=%d,%d,%d\n", __func__, data_raw[sample_no][0], data_raw[sample_no][1], data_raw[sample_no][2]);
		
		data_raw[sample_no][0] = (data[1] << 8 | data[0]) >> 4;
		data_raw[sample_no][1] = (data[3] << 8 | data[2]) >> 4;
		data_raw[sample_no][2] = (data[5] << 8 | data[4]) >> 4;  
		//printk("%s: acc=%d,%d,%d\n", __func__, data_raw[sample_no][0], data_raw[sample_no][1], data_raw[sample_no][2]);
		printk("%s: acc=%d,%d,%d\n", __func__, convert2s(data_raw[sample_no][0]), convert2s(data_raw[sample_no][1]), convert2s(data_raw[sample_no][2]));


		data_raw[sample_no][0] = (short int)((int)(data[1] << 8) | (data[0] & 0xF0)) >> 4;
		data_raw[sample_no][1] = (short int)((int)(data[3] << 8) | (data[2] & 0xF0)) >> 4;
		data_raw[sample_no][2] = (short int)((int)(data[5] << 8) | (data[4] & 0xF0)) >> 4;
		//data_raw[sample_no][2] = (short int)((int)(data[5] << 8) | (data[4] & 0xF0)) >> 4;
		printk("%s: acc=%d,%d,%d\n", __func__, convert2s(data_raw[sample_no][0]), convert2s(data_raw[sample_no][1]), convert2s(data_raw[sample_no][2]));
	*/

	}

	// sum 10 sets data
	for(i=0;i<10;i++)
	{
		acc_ave[0] += data_raw[i][0];
		acc_ave[1] += data_raw[i][1];
		acc_ave[2] += data_raw[i][2];
		//printk("%s: acc=%d,%d,%d\n", __func__, acc_ave[0], acc_ave[1], acc_ave[2]);
	}

	//average 10 sets data
	for(axis=0; axis<3; axis++) {
		if(acc_ave[axis] >= 0)
			acc_ave[axis] = (acc_ave[axis] + KIONIX_SAMPLE_NO / 2) / KIONIX_SAMPLE_NO;
		else
			acc_ave[axis] = (acc_ave[axis] - KIONIX_SAMPLE_NO / 2) / KIONIX_SAMPLE_NO;
	}
	
	//cali offset
	offset[0] = -(acc_ave[0] + 0);
	offset[1] = -(acc_ave[1] + 1024);
	offset[2] = -(acc_ave[2] + 0);
	printk("%s: New_offset for reg:%d,%d,%d\n", __func__, offset[0], offset[1], offset[2]);


	result = kionix_store_in_file(offset, KIONIX_K_SUCCESS_FILE, NULL);
	printk("%s:kionix_set_cali_do Line:%d!result:%d!!\n", __func__,__LINE__,result);
	if (result < 0) {
		printk("%s:failed to kionix_store_in_file, error=%d\n", __func__, result);
		//atomic_set(&acceld->cali_status, KIONIX_K_FAIL_W_FILE);
		return result;
	}
	//atomic_set(&acceld->cali_status, KIONIX_K_SUCCESS_FILE);
	return 0;
}


/* Returns the calibration value of the device */
static ssize_t kionix_accel_get_cali(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct kionix_accel_driver *acceld = i2c_get_clientdata(client);
	int calibration[3];

	read_lock(&acceld->rwlock_accel_data);

	calibration[0] = acceld->accel_cali[acceld->axis_map_x];
	calibration[1] = acceld->accel_cali[acceld->axis_map_y];
	calibration[2] = acceld->accel_cali[acceld->axis_map_z];

	read_unlock(&acceld->rwlock_accel_data);

	return sprintf(buf, "%d %d %d\n", calibration[0], calibration[1], calibration[2]);
}

/* Allow users to change the calibration value of the device */
static ssize_t kionix_accel_set_cali(struct device *dev, struct device_attribute *attr,
						const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct kionix_accel_driver *acceld = i2c_get_clientdata(client);
	struct input_dev *input_dev = acceld->input_dev;
	const int cali_count = 3; /* How many calibration that we expect to get from the string */
	char **buf2;
	long calibration[cali_count];
	int err = 0, i = 0;

	/* Lock the device to prevent races with open/close (and itself) */
	mutex_lock(&input_dev->mutex);

	buf2 = (char **)kzalloc(cali_count * sizeof(char *), GFP_KERNEL);

	if(kionix_strtok(buf, count, buf2, cali_count) < 0) {
		KMSGERR(&acceld->client->dev, \
				"%s: Not enough calibration data being read. " \
				"No calibration data will be updated.\n", __func__);
	}
	else {
		/* Convert string to integers  */
		for(i = 0 ; i < cali_count ; i++) {
			#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35))
			err = kstrtoint((const char *)*(buf2+i), 10, (int *)&calibration[i]);
			if(err < 0) {
				KMSGERR(&acceld->client->dev, \
						"%s: kstrtoint returned err = %d." \
						"No calibration data will be updated.\n", __func__ , err);
				goto exit;
			}
			#else
			err = strict_strtol((const char *)*(buf2+i), 10, &calibration[i]);
			if(err < 0) {
				KMSGERR(&acceld->client->dev, \
						"%s: strict_strtol returned err = %d." \
						"No calibration data will be updated.\n", __func__ , err);
				goto exit;
			}
			#endif
		}
		if((calibration[0] == 0) && (calibration[1] == 0) && (calibration[2] == 0))
		{
			kionix_set_cali_do(acceld);
		}
		else
		{
			write_lock(&acceld->rwlock_accel_data);
			
			acceld->accel_cali[acceld->axis_map_x] = (int)calibration[0];
			acceld->accel_cali[acceld->axis_map_y] = (int)calibration[1];
			acceld->accel_cali[acceld->axis_map_z] = (int)calibration[2];
			
			write_unlock(&acceld->rwlock_accel_data);
		}

		
		
	}

exit:
	for(i = 0 ; i < cali_count ; i++)
		kfree(*(buf2+i));

	kfree(buf2);

	mutex_unlock(&input_dev->mutex);

	return (err < 0) ? err : count;
}

/* Read wuf */
static ssize_t kionix_accel_get_wuf(struct device *dev,
				struct device_attribute *attr, char *buf) {

	struct i2c_client *client = to_i2c_client(dev);
	struct kionix_accel_driver *acceld = i2c_get_clientdata(client);

	return sprintf(buf, "%d\n", atomic_read(&acceld->accel_wuf_enabled) > 0 ? 1 : 0);						
}

/* Allow users to enable/disable the device */
static ssize_t kionix_accel_set_wuf(struct device *dev, struct device_attribute *attr,
						const char *buf, size_t count) {
							
							
	struct i2c_client *client = to_i2c_client(dev);
	struct kionix_accel_driver *acceld = i2c_get_clientdata(client);
	struct input_dev *input_dev = acceld->input_dev;
	char *buf2;
	const int enable_count = 1;
	unsigned long enable;
	int err = 0;

	KMSGINF(&acceld->client->dev, "enter set wufe\n");		
	/* Lock the device to prevent races with open/close (and itself) */
	mutex_lock(&input_dev->mutex);

	if(kionix_strtok(buf, count, &buf2, enable_count) < 0) {
		KMSGERR(&acceld->client->dev, \
				"%s: No enable data being read. " \
				"No enable data will be updated.\n", __func__);
	}
	else {
		/* Removes any leading negative sign */
		while(*buf2 == '-')
			buf2++;
		#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35))
		err = kstrtouint((const char *)buf2, 10, (unsigned int *)&enable);
		if (err < 0) {
			KMSGERR(&acceld->client->dev, \
					"%s: kstrtouint returned err = %d\n", __func__, err);
			goto exit;
		}
		#else
		err = strict_strtoul((const char *)buf2, 10, &enable);
		if (err < 0) {
			KMSGERR(&acceld->client->dev, \
					"%s: strict_strtoul returned err = %d\n", __func__, err);
			goto exit;
		}
		#endif

		if(acceld->kionix_accel_wufe != NULL) {
			err = acceld->kionix_accel_wufe(acceld, enable);
			if (err < 0) {
				KMSGERR(&acceld->client->dev, \
						"%s: kionix_accel_wuf returned err = %d\n", __func__, err);
				goto exit;
			}	
		}				
	}
	
exit:
	mutex_unlock(&input_dev->mutex);

	return (err < 0) ? err : count;
														
}				

static DEVICE_ATTR(enable, S_IRUGO|S_IWUSR, kionix_accel_get_enable, kionix_accel_set_enable);
static DEVICE_ATTR(delay, S_IRUGO|S_IWUSR, kionix_accel_get_delay, kionix_accel_set_delay);
static DEVICE_ATTR(direct, S_IRUGO|S_IWUSR, kionix_accel_get_direct, kionix_accel_set_direct);
static DEVICE_ATTR(data, S_IRUGO, kionix_accel_get_data, NULL);
static DEVICE_ATTR(cali, S_IRUGO|S_IWUSR, kionix_accel_get_cali, kionix_accel_set_cali);
static DEVICE_ATTR(wuf, S_IRUGO|S_IWUSR, kionix_accel_get_wuf, kionix_accel_set_wuf);

static struct attribute *kionix_accel_attributes[] = {
	&dev_attr_enable.attr,
	&dev_attr_delay.attr,
	&dev_attr_direct.attr,
	&dev_attr_data.attr,
	&dev_attr_cali.attr,
	&dev_attr_wuf.attr,
	NULL
};

static struct attribute_group kionix_accel_attribute_group = {
	.attrs = kionix_accel_attributes
};


 static void kionix_accel_delay_work(struct work_struct *work) 
 {
 	//struct i2c_client *client = to_i2c_client(dev);
	//struct kionix_accel_driver *acceld = i2c_get_clientdata(client);

	 struct kionix_accel_driver *acceld =
		 container_of(work, struct kionix_accel_driver, accel_delaywork.work);

	  
	  kionix_accel_grp4_report_accel_data(acceld);
 }	

 static enum hrtimer_restart kionix_accel_timer_func(struct hrtimer
								 *timer) 
 {
	 struct kionix_accel_driver *acceld =
		 container_of(timer, struct kionix_accel_driver, accel_timer);
	 schedule_delayed_work(&acceld->accel_delaywork, 0);
	 hrtimer_forward_now(&acceld->accel_timer,
				  ktime_set(0, KIONIX_ACCEL_POLLING_PERIOD));
	  return HRTIMER_RESTART;
 }
 
  static int kionix_polling_mode_setup(struct kionix_accel_driver *acceld) 
 {
	 INIT_DELAYED_WORK(&acceld->accel_delaywork, kionix_accel_delay_work);
	 hrtimer_init(&acceld->accel_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	 acceld->accel_timer.function = kionix_accel_timer_func;
	 hrtimer_start(&acceld->accel_timer, ktime_set(0, KIONIX_ACCEL_POLLING_PERIOD),
				HRTIMER_MODE_REL);
	 printk("polling mode success!\n");
	 return 0;
 }

 
  static void kionix_polling_mode_exit(struct kionix_accel_driver *acceld) 
 {
	 hrtimer_try_to_cancel(&acceld->accel_timer);
	 cancel_delayed_work_sync(&acceld->accel_delaywork);
 } 



static int kionix_verify(struct kionix_accel_driver *acceld)
{
	int retval = i2c_smbus_read_byte_data(acceld->client, ACCEL_WHO_AM_I);

#if KIONIX_KMSG_INF
	switch (retval) {
		case KIONIX_ACCEL_WHO_AM_I_KXTE9:
			KMSGINF(&acceld->client->dev, "this accelerometer is a KXTE9.\n");
			break;
		case KIONIX_ACCEL_WHO_AM_I_KXTF9:
			KMSGINF(&acceld->client->dev, "this accelerometer is a KXTF9.\n");
			break;
		case KIONIX_ACCEL_WHO_AM_I_KXTI9_1001:
			KMSGINF(&acceld->client->dev, "this accelerometer is a KXTI9-1001.\n");
			break;
		case KIONIX_ACCEL_WHO_AM_I_KXTIK_1004:
			KMSGINF(&acceld->client->dev, "this accelerometer is a KXTIK-1004.\n");
			break;
		case KIONIX_ACCEL_WHO_AM_I_KXTJ9_1005:
			KMSGINF(&acceld->client->dev, "this accelerometer is a KXTJ9-1005.\n");
			break;
		case KIONIX_ACCEL_WHO_AM_I_KXTJ9_1007:
			KMSGINF(&acceld->client->dev, "this accelerometer is a KXTJ9-1007.\n");
			break;
		case KIONIX_ACCEL_WHO_AM_I_KXCJ9_1008:
			KMSGINF(&acceld->client->dev, "this accelerometer is a KXCJ9-1008.\n");
			break;
		case KIONIX_ACCEL_WHO_AM_I_KXTJ2_1009:
			KMSGINF(&acceld->client->dev, "this accelerometer is a KXTJ2-1009.\n");
			break;
		case KIONIX_ACCEL_WHO_AM_I_KXTJ3_1057:
			KMSGINF(&acceld->client->dev, "this accelerometer is a KXTJ3-1057.\n");
			break;
		case KIONIX_ACCEL_WHO_AM_I_KXCJK_1013:
			KMSGINF(&acceld->client->dev, "this accelerometer is a KXCJK-1013.\n");
			break;
        case KIONIX_ACCEL_WHO_AM_I_KX022:
			KMSGINF(&acceld->client->dev, "this accelerometer is a KX022.\n");
			break;                        
		case KIONIX_ACCEL_WHO_AM_I_KX023:
			KMSGINF(&acceld->client->dev, "this accelerometer is a KX023.\n");
			break;        
		default:
			break;
	}
#endif

	return retval;
}

#if 1 //#ifdef    CONFIG_HAS_EARLYSUSPEND
//void kionix_accel_earlysuspend_suspend(struct early_suspend *h)
static int kionix_accel_earlysuspend_suspend(struct device *dev)
{
	//struct kionix_accel_driver *acceld = container_of(h, struct kionix_accel_driver, early_suspend);
	long remaining;
	int err,ret;
	struct i2c_client *client = to_i2c_client(dev);
	struct kionix_accel_driver *acceld = i2c_get_clientdata(client);
	
	err = i2c_smbus_read_byte_data(acceld->client, ACCEL_GRP1_CTRL_REG1);
	printk("kionix_accel_earlysuspend_suspend:read ACCEL_GRP1_CTRL_REG1:%d!\n",err);
	if (err < 0)
		return err;

	err = err| ACCEL_GRP1_PC1_OFF;
	printk("kionix_accel_earlysuspend_suspend:write ACCEL_GRP1_CTRL_REG1:%d!\n",err);
	ret = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP1_CTRL_REG1, err);
	if (ret < 0)
	{
		printk("kionix_accel_earlysuspend_suspend:write error ret:%d!\n",ret);
		return ret;
	}

	mutex_lock(&acceld->mutex_earlysuspend);

	/* Only continue to suspend if enable did not intervene */
	if(atomic_read(&acceld->accel_suspend_continue) > 0) {
		/* Make sure that the sensor had successfully disabled before suspending it */
		if(atomic_read(&acceld->accel_enabled) > 0) {
			KMSGINF(&acceld->client->dev, "%s: waiting for disable\n", __func__);
			remaining = wait_event_interruptible_timeout(acceld->wqh_suspend, \
					atomic_read(&acceld->accel_enabled) < 1, \
					msecs_to_jiffies(KIONIX_ACCEL_EARLYSUSPEND_TIMEOUT));

			if(atomic_read(&acceld->accel_enabled) > 0) {
				KMSGERR(&acceld->client->dev, "%s: timeout waiting for disable\n", __func__);
			}
		}

		kionix_accel_power_off(acceld);

		atomic_set(&acceld->accel_suspended, 1);
	}

	mutex_unlock(&acceld->mutex_earlysuspend);

	return 0;
}

//void kionix_accel_earlysuspend_resume(struct early_suspend *h)
static int kionix_accel_earlysuspend_resume(struct device *dev)
{
	//struct kionix_accel_driver *acceld = container_of(h, struct kionix_accel_driver, early_suspend);
	int err,ret;
	struct i2c_client *client = to_i2c_client(dev);
	struct kionix_accel_driver *acceld = i2c_get_clientdata(client);

	mutex_lock(&acceld->mutex_resume);

	if(atomic_read(&acceld->accel_suspended) == 1) {
		err = kionix_accel_power_on(acceld);
		if (err < 0) {
			KMSGERR(&acceld->client->dev, "%s: kionix_accel_power_on returned err = %d\n", __func__, err);
			goto exit;
		}

		/* Only needs to reinitialized the registers if Vdd is pulled low during suspend */
		if(err > 0) {
			err = acceld->kionix_accel_power_on_init(acceld);
			if (err) {
				KMSGERR(&acceld->client->dev, "%s: kionix_accel_power_on_init returned err = %d\n", __func__, err);
				goto exit;
			}
		}

		atomic_set(&acceld->accel_suspended, 0);
	}
	err = i2c_smbus_read_byte_data(acceld->client, ACCEL_GRP1_CTRL_REG1);
	printk("kionix_accel_earlysuspend_resume:read ACCEL_GRP1_CTRL_REG1:%d!\n",err);
	if (err < 0)
		return err;

	err = err| ACCEL_GRP1_PC1_ON;
	printk("kionix_accel_earlysuspend_resume:write ACCEL_GRP1_CTRL_REG1:%d!\n",err);
	ret = i2c_smbus_write_byte_data(acceld->client, ACCEL_GRP1_CTRL_REG1, err);
	if (ret < 0)
	{
		printk("kionix_accel_earlysuspend_resume:write error ret:%d!\n",ret);
		return ret;
	}


	wake_up_interruptible(&acceld->wqh_suspend);

exit:
	mutex_unlock(&acceld->mutex_resume);

	return 0;
}
#endif /* CONFIG_HAS_EARLYSUSPEND */

static int accel_poll_thread(void *data)
{
	struct kionix_accel_driver *acceld = data;

	while (1) {	
		printk("kionix_accel accel_wkp_flag:%d;kthread_should_stop:%d!!",acceld->accel_wkp_flag,kthread_should_stop());
		wait_event_interruptible(acceld->accel_wq,
			((acceld->accel_wkp_flag != 0) ||
				kthread_should_stop()));
		acceld->accel_wkp_flag = 0;

		if (kthread_should_stop())
			break;
				
		acceld->kionix_accel_report_accel_data(acceld);
	}

	return 0;
}

enum hrtimer_restart acc_hrtimer_callback( struct hrtimer *timer)
{
	struct kionix_accel_driver *acceld;
	acceld = container_of(timer, struct kionix_accel_driver, accel_timer);
	
	if( atomic_read(&acceld->accel_enabled) > 0) {
		hrtimer_forward_now(&acceld->accel_timer, 
			ktime_set(acceld->poll_interval / 1000, (acceld->poll_interval % 1000) * 1000000));
		acceld->accel_wkp_flag = 1;
		wake_up_interruptible(&acceld->accel_wq);
		return HRTIMER_RESTART;

	} else {
		return HRTIMER_NORESTART;
	}
}

static int kionix_accel_probe(struct i2c_client *client,
				 const struct i2c_device_id *id)
{
	struct kionix_accel_platform_data *accel_pdata = NULL;//client->dev.platform_data;
	struct kionix_accel_driver *acceld;
	int err;
	//int irq;
	int ret=0;
	//
	int gpiopin;
	struct device_node *node = NULL;
	int headsetdebounce = 10;
	//struct proc_dir_entry *proc_dir, *proc_entry;
	printk("liuyufeng gsensor start111111111\n");
	if (!i2c_check_functionality(client->adapter,
				I2C_FUNC_I2C | I2C_FUNC_SMBUS_BYTE_DATA)) {
		KMSGERR(&client->dev, "client is not i2c capable. Abort.\n");
		return -ENXIO;
	}
	accel_pdata = kzalloc(sizeof(struct kionix_accel_platform_data), GFP_KERNEL);
	if (!accel_pdata) {
		KMSGERR(&client->dev, "platform data is NULL. Abort.\n");
		return -EINVAL;
	}
	accel_pdata->poll_interval = 10;
	accel_pdata->min_interval = 10;
	//accel_pdata->g_range = 0;
	accel_pdata->accel_g_range = 0;
	accel_pdata->accel_res = 0;
#if KIONIX_ENABLE_POLLING_MODE
	accel_pdata->accel_irq_use_drdy = 0;
#else
	accel_pdata->accel_irq_use_drdy = 1;
#endif
	accel_pdata->accel_direction = 6;
	printk("liuyufeng gsensor start\n");
	acceld = kzalloc(sizeof(*acceld), GFP_KERNEL);
	if (acceld == NULL) {
		KMSGERR(&client->dev, \
			"failed to allocate memory for module data. Abort.\n");
		return -ENOMEM;
	}
	pr_err("liuyufeng gsensor kionix_verify \n");
	acceld->client = client;
	acceld->accel_pdata = *accel_pdata;

	i2c_set_clientdata(client, acceld);

	err = kionix_accel_power_on(acceld);
	if (err < 0)
		goto err_free_mem;

	if (accel_pdata->init) {
		err = accel_pdata->init();
		if (err < 0)
			goto err_accel_pdata_power_off;
	}
	printk("liuyufeng gsensor kionix_verify start,i2c_addr: 0x%x!\n",client->addr);
	err = kionix_verify(acceld);
	if (err < 0) {
		KMSGERR(&acceld->client->dev, "%s: kionix_verify returned err = %d. Abort.\n", __func__, err);
		goto err_accel_pdata_exit;
	}

	/* Setup group specific configuration and function callback */
	switch (err) {
		case KIONIX_ACCEL_WHO_AM_I_KXTE9:
			acceld->accel_group = KIONIX_ACCEL_GRP1;
			acceld->accel_registers = kzalloc(sizeof(u8)*accel_grp1_regs_count, GFP_KERNEL);
			if (acceld->accel_registers == NULL) {
				KMSGERR(&client->dev, \
					"failed to allocate memory for accel_registers. Abort.\n");
				goto err_accel_pdata_exit;
			}
			acceld->accel_drdy = 0;
			acceld->kionix_accel_report_accel_data	= kionix_accel_grp1_report_accel_data;
			acceld->kionix_accel_update_odr			= kionix_accel_grp1_update_odr;
			acceld->kionix_accel_power_on_init		= kionix_accel_grp1_power_on_init;
			acceld->kionix_accel_operate			= kionix_accel_grp1_operate;
			acceld->kionix_accel_standby			= kionix_accel_grp1_standby;
			acceld->kionix_accel_wufe				= NULL;
			
			break;
		case KIONIX_ACCEL_WHO_AM_I_KXTF9:
		case KIONIX_ACCEL_WHO_AM_I_KXTI9_1001:
		case KIONIX_ACCEL_WHO_AM_I_KXTIK_1004:
		case KIONIX_ACCEL_WHO_AM_I_KXTJ9_1005:
			if(err == KIONIX_ACCEL_WHO_AM_I_KXTIK_1004)
				acceld->accel_group = KIONIX_ACCEL_GRP3;
			else
				acceld->accel_group = KIONIX_ACCEL_GRP2;
			acceld->accel_registers = kzalloc(sizeof(u8)*accel_grp2_regs_count, GFP_KERNEL);
			if (acceld->accel_registers == NULL) {
				KMSGERR(&client->dev, \
					"failed to allocate memory for accel_registers. Abort.\n");
				goto err_accel_pdata_exit;
			}
			switch(acceld->accel_pdata.accel_res) {
				case KIONIX_ACCEL_RES_6BIT:
				case KIONIX_ACCEL_RES_8BIT:
					acceld->accel_registers[accel_grp2_ctrl_reg1] |= ACCEL_GRP2_RES_8BIT;
					break;
				case KIONIX_ACCEL_RES_12BIT:
				case KIONIX_ACCEL_RES_16BIT:
				default:
					acceld->accel_registers[accel_grp2_ctrl_reg1] |= ACCEL_GRP2_RES_12BIT;
					break;
			}
			if(acceld->accel_pdata.accel_irq_use_drdy && client->irq) {
				acceld->accel_registers[accel_grp2_int_ctrl] |= ACCEL_GRP2_IEN | ACCEL_GRP2_IEA;
				acceld->accel_registers[accel_grp2_ctrl_reg1] |= ACCEL_GRP2_DRDYE;
				acceld->accel_drdy = 1;
			}
			else
				acceld->accel_drdy = 0;
			kionix_accel_grp2_update_g_range(acceld);
			acceld->kionix_accel_report_accel_data	= kionix_accel_grp2_report_accel_data;
			acceld->kionix_accel_update_odr			= kionix_accel_grp2_update_odr;
			acceld->kionix_accel_power_on_init		= kionix_accel_grp2_power_on_init;
			acceld->kionix_accel_operate			= kionix_accel_grp2_operate;
			acceld->kionix_accel_standby			= kionix_accel_grp2_standby;
			acceld->kionix_accel_wufe				= NULL;
			break;
		case KIONIX_ACCEL_WHO_AM_I_KXTJ9_1007:
		case KIONIX_ACCEL_WHO_AM_I_KXCJ9_1008:
		case KIONIX_ACCEL_WHO_AM_I_KXTJ2_1009:
		case KIONIX_ACCEL_WHO_AM_I_KXTJ3_1057:
		case KIONIX_ACCEL_WHO_AM_I_KXCJK_1013:
			//printk("wangdong gsensor kionix_verify err:%d!\n",err);
			if(err == (KIONIX_ACCEL_WHO_AM_I_KXTJ2_1009 | KIONIX_ACCEL_WHO_AM_I_KXTJ3_1057))
				acceld->accel_group = KIONIX_ACCEL_GRP5;
			else if(err == KIONIX_ACCEL_WHO_AM_I_KXCJK_1013)
				acceld->accel_group = KIONIX_ACCEL_GRP6;
			else
				acceld->accel_group = KIONIX_ACCEL_GRP4;
			acceld->accel_registers = kzalloc(sizeof(u8)*accel_grp4_regs_count, GFP_KERNEL);
			if (acceld->accel_registers == NULL) {
				KMSGERR(&client->dev, \
					"failed to allocate memory for accel_registers. Abort.\n");
				goto err_accel_pdata_exit;
			}
			switch(acceld->accel_pdata.accel_res) {
				case KIONIX_ACCEL_RES_6BIT:
				case KIONIX_ACCEL_RES_8BIT:
					acceld->accel_registers[accel_grp4_ctrl_reg1] |= ACCEL_GRP4_RES_8BIT;
					break;
				case KIONIX_ACCEL_RES_12BIT:
				case KIONIX_ACCEL_RES_16BIT:
				default:
					acceld->accel_registers[accel_grp4_ctrl_reg1] |= ACCEL_GRP4_RES_12BIT;
					break;
			}
			//printk("wangdong L2479 gsensor irq num :%d;accel_irq_use_drdy:%d!\n",client->irq,acceld->accel_pdata.accel_irq_use_drdy);
			if(acceld->accel_pdata.accel_irq_use_drdy && client->irq) {
				//printk("wangdong L2407 gsensor irq num err:%d!\n",client->irq);
				acceld->accel_registers[accel_grp4_int_ctrl] |= ACCEL_GRP4_IEN | ACCEL_GRP4_IEA;
				acceld->accel_registers[accel_grp4_ctrl_reg1] |= ACCEL_GRP4_DRDYE;
				acceld->accel_drdy = 1;
			}
			else
				acceld->accel_drdy = 0;
			kionix_accel_grp4_update_g_range(acceld);
			acceld->kionix_accel_report_accel_data	= kionix_accel_grp4_report_accel_data;
			acceld->kionix_accel_update_odr			= kionix_accel_grp4_update_odr;
			acceld->kionix_accel_power_on_init		= kionix_accel_grp4_power_on_init;
			acceld->kionix_accel_operate			= kionix_accel_grp4_operate;
			acceld->kionix_accel_standby			= kionix_accel_grp4_standby;
			acceld->kionix_accel_wufe				= NULL;
			break;
		case KIONIX_ACCEL_WHO_AM_I_KX023:
        case KIONIX_ACCEL_WHO_AM_I_KX022:
			acceld->accel_registers = kzalloc(sizeof(u8)*accel_grp7_regs_count, GFP_KERNEL);
			if (acceld->accel_registers == NULL) {
				KMSGERR(&client->dev, \
					"failed to allocate memory for accel_registers. Abort.\n");
				goto err_accel_pdata_exit;
			}
			switch(acceld->accel_pdata.accel_res) {
				case KIONIX_ACCEL_RES_6BIT:
				case KIONIX_ACCEL_RES_8BIT:
					acceld->accel_registers[accel_grp7_ctrl_reg1] |= ACCEL_GRP7_RES_8BIT;
					break;
				case KIONIX_ACCEL_RES_12BIT:
				case KIONIX_ACCEL_RES_16BIT:
				default:
					acceld->accel_registers[accel_grp7_ctrl_reg1] |= ACCEL_GRP7_RES_16BIT;
					break;
			}
			if(acceld->accel_pdata.accel_irq_use_drdy && client->irq) {				
				acceld->accel_registers[accel_grp7_int_ctrl] |= ACCEL_GRP7_IEN | ACCEL_GRP7_IEA;
				acceld->accel_registers[accel_grp7_ctrl_reg1] |= ACCEL_GRP7_DRDYE;
				acceld->accel_drdy = 1;				
			}
			else
				acceld->accel_drdy = 0;
			kionix_accel_grp7_update_g_range(acceld);
			acceld->kionix_accel_report_accel_data	= kionix_accel_grp7_report_accel_data;
			acceld->kionix_accel_update_odr			= kionix_accel_grp7_update_odr;
			acceld->kionix_accel_power_on_init		= kionix_accel_grp7_power_on_init;
			acceld->kionix_accel_operate			= kionix_accel_grp7_operate;
			acceld->kionix_accel_standby			= kionix_accel_grp7_standby;
			acceld->kionix_accel_wufe				= kionix_accel_grp7_wufe;
			break;
		default:
			KMSGERR(&acceld->client->dev, \
					"%s: unsupported device, who am i = %d. Abort.\n", __func__, err);
			goto err_accel_pdata_exit;
	}

	err = kionix_accel_setup_input_device(acceld);
	if (err)
		goto err_free_accel_registers;

	atomic_set(&acceld->accel_suspended, 0);
	atomic_set(&acceld->accel_suspend_continue, 1);
	atomic_set(&acceld->accel_enabled, 1);
	atomic_set(&acceld->accel_input_event, 1);
	atomic_set(&acceld->accel_enable_resume, 0);

	mutex_init(&acceld->mutex_earlysuspend);
	mutex_init(&acceld->mutex_resume);
	rwlock_init(&acceld->rwlock_accel_data);

	acceld->poll_interval = 10;//acceld->accel_pdata.poll_interval;
	acceld->poll_delay = msecs_to_jiffies(acceld->poll_interval);
	acceld->kionix_accel_update_odr(acceld, acceld->poll_interval);
	kionix_accel_update_direction(acceld);

	printk("liuyufeng gsensor proc_mkdir start !\n");
	/*
	proc_dir = proc_mkdir("sensors", NULL);
	if (proc_dir == NULL)
		KMSGERR(&client->dev, "failed to create /proc/sensors\n");
	else {
		//proc_entry = create_proc_entry( "accelinfo", 0644, proc_dir);
		proc_entry = proc_create("accelinfo", 0644, proc_dir, NULL);
		if (proc_entry == NULL)
			KMSGERR(&client->dev, "failed to create /proc/cpu/accelinfo\n");
	}*/

	//acceld->accel_workqueue = create_workqueue("Kionix Accel Workqueue");
	//INIT_DELAYED_WORK(&acceld->accel_work, kionix_accel_work);
	//init_waitqueue_head(&acceld->wqh_suspend);
	
	init_waitqueue_head(&acceld->wqh_suspend);
	hrtimer_init(&acceld->accel_timer,CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	acceld->accel_timer.function = acc_hrtimer_callback;

	init_waitqueue_head(&acceld->accel_wq);
	acceld->accel_wkp_flag = 0;//0
	acceld->accel_task = kthread_run(accel_poll_thread, acceld,
						"kionix_accel");

						

	

	node = of_find_matching_node(node, of_kionix_accel_match);
	if (node) {
		gpiopin = of_get_named_gpio(node, "int-gpio", 0);
		if (gpiopin < 0)
			printk("------------get gpio filed -----------\n");
		ret = gpio_request(gpiopin, "int-gpio");
		if(ret)
			printk("------------request gpio filed -----------\n");
		gpio_direction_input(gpiopin);
		gpio_set_debounce(gpiopin, headsetdebounce); //???
	}
						
						
	if (acceld->accel_drdy) {
	
	/*	irq = gpio_to_irq(gpiopin);
		if (irq < 0) {
			err = irq;
			printk("%s Unable to get irq number, error %d\n",
				__func__, err);
			return err;
		}

		err = devm_request_any_context_irq(&acceld->client->dev, irq,
							 kionix_accel_isr,  IRQF_TRIGGER_FALLING | IRQF_ONESHOT, KIONIX_ACCEL_IRQ, acceld);
		if (err < 0) {
			printk("%s Unable to claim; error %d\n",
				__func__, err);
			return err;
		}*/
			
		err = request_threaded_irq(client->irq, NULL, kionix_accel_isr, \
					   IRQF_TRIGGER_RISING | IRQF_ONESHOT, \
					   KIONIX_ACCEL_IRQ, acceld);			
		if (err) {
			KMSGERR(&acceld->client->dev, "%s: request_threaded_irq returned err = %d\n", __func__, err);
			KMSGERR(&acceld->client->dev, "%s: running in software polling mode instead\n", __func__);
			acceld->accel_drdy = 0;
		}
		KMSGINF(&acceld->client->dev, "running in hardware interrupt mode\n");	
	} else {
		KMSGINF(&acceld->client->dev, "running in software polling mode\n");
		err = kionix_polling_mode_setup(acceld);

	}

	printk("liuyufeng gsensor power_on_init start !\n");
	
	err = acceld->kionix_accel_power_on_init(acceld);
	if (err) {
		KMSGERR(&acceld->client->dev, "%s: kionix_accel_power_on_init returned err = %d. Abort.\n", __func__, err);
		goto err_free_irq;
	}

	//&sdata->accel_input_dev->dev.kobj	
	//err = sysfs_create_group(&client->dev.kobj, &kionix_accel_attribute_group);
	err = sysfs_create_group(&acceld->input_dev->dev.kobj, &kionix_accel_attribute_group);
	if (err) {
		KMSGERR(&acceld->client->dev, "%s: sysfs_create_group returned err = %d. Abort.\n", __func__, err);
		goto err_free_irq;
	}

#ifdef    CONFIG_HAS_EARLYSUSPEND
	/* The higher the level, the earlier it resume, and the later it suspend */
	acceld->early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 50;
	acceld->early_suspend.suspend = kionix_accel_earlysuspend_suspend;
	acceld->early_suspend.resume = kionix_accel_earlysuspend_resume;
	register_early_suspend(&acceld->early_suspend);
#endif /* CONFIG_HAS_EARLYSUSPEND */

	return 0;

err_free_irq:
	kthread_stop(acceld->accel_task);
	if (acceld->accel_drdy)
	{
		free_irq(client->irq, acceld);
	}
	else
	{
		kionix_polling_mode_exit(acceld);
	}
	//destroy_workqueue(acceld->accel_workqueue);
	input_unregister_device(acceld->input_dev);
err_free_accel_registers:
	kfree(acceld->accel_registers);
err_accel_pdata_exit:
	if (accel_pdata->exit)
		accel_pdata->exit();
err_accel_pdata_power_off:
	kionix_accel_power_off(acceld);
err_free_mem:
	kfree(acceld);
	return err;
}

static int kionix_accel_remove(struct i2c_client *client)
{
	struct kionix_accel_driver *acceld = i2c_get_clientdata(client);

	kthread_stop(acceld->accel_task);
#ifdef    CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&acceld->early_suspend);
#endif /* CONFIG_HAS_EARLYSUSPEND */
	sysfs_remove_group(&client->dev.kobj, &kionix_accel_attribute_group);
	if (acceld->accel_drdy)
		free_irq(client->irq, acceld);
	//destroy_workqueue(acceld->accel_workqueue);
	input_unregister_device(acceld->input_dev);
	kfree(acceld->accel_registers);
	if (acceld->accel_pdata.exit)
		acceld->accel_pdata.exit();
	kionix_accel_power_off(acceld);
	kfree(acceld);

	return 0;
}



static void kionix_accel_shutdown(struct i2c_client *client)
{
	int err;
	struct kionix_accel_driver *acceld = i2c_get_clientdata(client);

#if 1
	err = i2c_smbus_write_byte_data(acceld->client,0x7f, 0x00);
	//sleep(0.03);
	mdelay(30);

	err = i2c_smbus_write_byte_data(acceld->client,0x1D, 0x00);
	//sleep(0.03);
	mdelay(30);

	err = i2c_smbus_write_byte_data(acceld->client,0x1D, 0x80);
	//sleep(0.1);
	mdelay(100);
#endif


	return ;
}


//static SIMPLE_DEV_PM_OPS(kionix_accel_pm_ops, kionix_accel_earlysuspend_suspend, kionix_accel_earlysuspend_resume);
static const struct dev_pm_ops kionix_accel_pm_ops = {
	.suspend = kionix_accel_earlysuspend_suspend,
	.resume = kionix_accel_earlysuspend_resume,
};


static const struct i2c_device_id kionix_accel_id[] = {
	{ KIONIX_ACCEL_NAME, 0 },
	{ },
};

MODULE_DEVICE_TABLE(i2c, kionix_accel_id);

static struct i2c_driver kionix_accel_driver = {
	.driver = {
		.name	= KIONIX_ACCEL_NAME,
		//.pm =&kionix_accel_pm_ops,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(of_kionix_accel_match),	
	},
	.probe		= kionix_accel_probe,
	.remove		= kionix_accel_remove,
	.shutdown   = kionix_accel_shutdown,
	.id_table	= kionix_accel_id,
};
module_i2c_driver(kionix_accel_driver);

MODULE_DESCRIPTION("Kionix accelerometer driver");
MODULE_AUTHOR("Kuching Tan <kuchingtan@kionix.com>");
MODULE_LICENSE("GPL");
