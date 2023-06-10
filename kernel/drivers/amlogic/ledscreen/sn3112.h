#ifndef _LINUX_LED_SN3112_H__
#define _LINUX_LED_SN3112_H__

#define SN3112_LED_NUM 12//36
#define MAX_BRIGHT_LEVEL  0xFF
#define MIN_BRIGHT_LEVEL  0x00

#define LED_I2C_CHANNEL0 0
#define LED_I2C_CHANNEL1 1

//sn3112 led private struct
struct led_sn3112 {
	struct led_classdev	led_cdev;//common led characteristic
	struct i2c_client	*client;//chip i2c
	spinlock_t		lock;

	enum led_brightness	brightness;
	int			led_num;	/* 0 .. 8 potentially */
	char		name[10];
    int         chip_num;
};

struct led_sn3112_priv_data {
	struct i2c_client	*client;//chip i2c
	int gpio;
	struct led_sn3112 *drvdata;
	struct regulator *supply;
};

struct platform_sn3112_data {
	u8 max_current;    
};

enum SN3112_UIB {
	SN3112_1,
	SN3112_2,
};

#endif	/* _LINUX_LED_SN3112_H__ */
