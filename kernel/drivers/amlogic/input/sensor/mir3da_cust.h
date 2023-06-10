/* For PLATFORM STANDARD
 *
 * mir3da.h - Linux kernel modules for 3-Axis Accelerometer
 *
 * Copyright (C) 2011-2013 MiraMEMS Sensing Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __MIR3DA_STANDARD_H__
#define __MIR3DA_STANDARD_H__
	 
#include <linux/ioctl.h>

/* driver version info*/
#define DRI_VER                                 "1.0"

#define MIR3DA_I2C_ADDR		                    0x27//0x26<-> SD0=GND;0x27<-> SD0=High
#define MIR3DA_DIRECTION_NUM					0   //0 ~ 7

#define MIR3DA_ACC_IOCTL_BASE                   88
#define IOCTL_INDEX_BASE                        0x00
#define KERNEL4

#define MIR3DA_ACC_IOCTL_SET_DELAY              _IOW(MIR3DA_ACC_IOCTL_BASE, IOCTL_INDEX_BASE, int)
#define MIR3DA_ACC_IOCTL_GET_DELAY              _IOR(MIR3DA_ACC_IOCTL_BASE, IOCTL_INDEX_BASE+1, int)
#define MIR3DA_ACC_IOCTL_SET_ENABLE             _IOW(MIR3DA_ACC_IOCTL_BASE, IOCTL_INDEX_BASE+2, int)
#define MIR3DA_ACC_IOCTL_GET_ENABLE             _IOR(MIR3DA_ACC_IOCTL_BASE, IOCTL_INDEX_BASE+3, int)
#define MIR3DA_ACC_IOCTL_SET_G_RANGE            _IOW(MIR3DA_ACC_IOCTL_BASE, IOCTL_INDEX_BASE+4, int)
#define MIR3DA_ACC_IOCTL_GET_G_RANGE            _IOR(MIR3DA_ACC_IOCTL_BASE, IOCTL_INDEX_BASE+5, int)

#define MIR3DA_ACC_IOCTL_GET_COOR_XYZ           _IOW(MIR3DA_ACC_IOCTL_BASE, IOCTL_INDEX_BASE+22, int)
#define MIR3DA_ACC_IOCTL_CALIBRATION            _IOW(MIR3DA_ACC_IOCTL_BASE, IOCTL_INDEX_BASE+23, int)
#define MIR3DA_ACC_IOCTL_UPDATE_OFFSET     	    _IOW(MIR3DA_ACC_IOCTL_BASE, IOCTL_INDEX_BASE+24, int)

enum {
	MIR3DA_ACCEL = 0,
	MIR3DA_SIGN_M,
	MIR3DA_STEP_D,
	MIR3DA_STEP_C,
	MIR3DA_TILT,
	MIR3DA_SENSORS_NUMB,
};

#define INPUT_EVENT_TYPE		EV_MSC
#define INPUT_EVENT_X			MSC_SERIAL
#define INPUT_EVENT_Y			MSC_PULSELED
#define INPUT_EVENT_Z			MSC_GESTURE
#define INPUT_EVENT_TIME_MSB	MSC_SCAN
#define INPUT_EVENT_TIME_LSB	MSC_MAX
#define INPUT_EVENT_SC			MSC_SERIAL


#endif /* !__MIR3DA_STANDARD_H__ */


