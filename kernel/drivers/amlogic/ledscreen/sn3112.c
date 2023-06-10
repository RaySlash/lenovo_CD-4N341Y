#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/init.h>
#include <linux/leds.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/slab.h>

#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/device.h>
#include <linux/regulator/consumer.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define GVA         0x04   //sn3112 first address

#define DATE_REG_FIRST_ADD (0x04)

#define RGB_SWITCH_CMD   0xFF

struct sn3112_private
{
    int init_gpio;
    struct regulator *phy_regulator;
    //int i2c0power_gpio;
	struct delayed_work delay_work;
};

enum rgb_order
{
    ORDER_RGB = 0x00,
    ORDER_RBG,
    ORDER_GRB,
    ORDER_GBR,
    ORDER_BGR,
    ORDER_BRG
};



static int major;
static struct class *class;

static struct i2c_client *sn3112_client;
static struct sn3112_private *is_priv;
static enum rgb_order harware_rgb_order = ORDER_RGB;

static const struct of_device_id of_sn3112_leds_match[] = {
    { .compatible = "tonly,sn3112", },
    {},
};

MODULE_DEVICE_TABLE(of, of_sn3112_leds_match);

static int sn3112_software_power_onoff(struct i2c_client *client, u8 onoff);
static int sn3112_led_ctrl(struct i2c_client *client, u8 *reg, u8 length);


//#define INIT_LED_ON_UBOOT
#ifndef INIT_LED_ON_UBOOT
static int sn3112_pwm_update(struct i2c_client *client);
static int led_sn3112_Init(void);
#endif
ssize_t sn3112_pwm_write(int index, int count, const char* buf);


#define LED_NUMBER   4
#define LED_BUF_SIZE (3*LED_NUMBER)
#define DEFAULT_SPEED    1000

static struct _style {
	int times;		/*1: stop after action once, 0: cycle action*/
	int state_index;
	const char style[6][LED_BUF_SIZE];	/*style data*/
} styleData = {
	2, 0,
	{
		 /*led1    led2      led3     led4   */
		{	0xFF, 0xFF, 0xFF,
			0x90, 0x90, 0x90,
			0x60, 0x60, 0x60,
			0x60, 0x60, 0x60
		}, /*state1*/
		{	0x90, 0x90, 0x90,
			0xFF, 0xFF, 0xFF,
			0x90, 0x90, 0x90,
			0x60, 0x60, 0x60
		}, /*state2*/
		{	0x60, 0x60, 0x60,
			0x90, 0x90, 0x90,
			0xFF, 0xFF, 0xFF,
			0x90, 0x90, 0x90
		}, /*state3*/
		{	0x60, 0x60, 0x60,
			0x60, 0x60, 0x60,
			0x90, 0x90, 0x90,
			0xFF, 0xFF, 0xFF
		}, /*state4*/
		{	0x60, 0x60, 0x60,
			0x90, 0x90, 0x90,
			0xFF, 0xFF, 0xFF,
			0x90, 0x90, 0x90
		}, /*state5*/
		{	0x90, 0x90, 0x90,
			0xFF, 0xFF, 0xFF,
			0x90, 0x90, 0x90,
			0x60, 0x60, 0x60
		}, /*state6*/
	},
};

static void leds_display(struct work_struct *work)
{
	char stage1[12] = { 0x00, 0x00, 0x00,
						0x80, 0x80, 0x80,
						0x80, 0x80, 0x80,
						0x00, 0x00, 0x00};

	char stage2[12] = { 0x80, 0x80, 0x80,
						0x80, 0x80, 0x80,
						0x80, 0x80, 0x80,
						0x80, 0x80, 0x80};

	switch (styleData.times) {
	case 2:
		sn3112_pwm_write(GVA, 12, stage1);
		styleData.times--;
		schedule_delayed_work(&is_priv->delay_work, msecs_to_jiffies(1000));
		break;
	case 1:
		sn3112_pwm_write(GVA, 12, stage2);
		styleData.times--;
		schedule_delayed_work(&is_priv->delay_work, msecs_to_jiffies(1000));
		break;
	case 0:
		sn3112_pwm_write(GVA, 12, styleData.style[styleData.state_index]);

		styleData.state_index++;

		if (styleData.state_index > 5)
			styleData.state_index = 0;

		schedule_delayed_work(&is_priv->delay_work, msecs_to_jiffies(250));
		break;
	default:
		break;
	}
}


static int sn3112_pm_suspend(struct device *dev)
{
    int ret;
    u8 reg[3] = {0x00, 0x00, 0x00};

    ret = sn3112_led_ctrl(sn3112_client, reg, 3);//disable all leds switch
    if (ret) { 
        dev_err(&sn3112_client->dev, "sn3112_pm_suspend write led switch error\n");
        return ret;
    }

    i2c_smbus_write_byte_data(sn3112_client, 0x16, 0x0);

    ret = sn3112_software_power_onoff(sn3112_client, 0);
    if (ret < 0) 
    {
        dev_err(&sn3112_client->dev, "sn3112_pm_resume write onoff error\n");
        return ret;
    }

    if (gpio_is_valid(is_priv->init_gpio))
    {
        gpio_set_value(is_priv->init_gpio, 0);//low
    }

    return 0;
}

static int sn3112_pm_resume(struct device *dev)
{
#if 1//delete by tom,because resume led flash short time.
    int i = 0;
    int ret = 0;
    u8 reg[3] = {0xFF, 0xFF, 0xFF};

    if (gpio_is_valid(is_priv->init_gpio))
    {
        gpio_set_value(is_priv->init_gpio, 1);//set high
    }
    msleep(1);

    //reset register first
    i2c_smbus_write_byte_data(sn3112_client, 0x17, 0x0);
    msleep(1);

    ret = sn3112_software_power_onoff(sn3112_client, 1);
    if (ret < 0) 
    {
        dev_err(&sn3112_client->dev, "sn3112_pm_resume write onoff error\n");
        return ret;
    }

    ret = sn3112_led_ctrl(sn3112_client, reg, 3);//enable all leds switch
    if (ret) {
       dev_err(&sn3112_client->dev, "sn3112_pm_resume write led switch error\n");
       return ret;
    }

    i2c_smbus_write_byte_data(sn3112_client, 0x16, 0x0);

    for(i = 0x04; i <= 0x0f; i++)
    {
        i2c_smbus_write_byte_data(sn3112_client, i, 0xFF);
    }

    i2c_smbus_write_byte_data(sn3112_client, 0x16, 0x0);

#endif
    return 0;
}

static int sn3112_open(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t sn3112_write(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
    int i,ret;
    unsigned char ker_buf[36];

    if (copy_from_user(ker_buf, buf, count))
         return -EFAULT;

    //printk("sn3112_write count=%d  ker_buf[0]=%d\n",count,(int)ker_buf[0]);

    for(i = 0x01; i <= count; i++)
    {
       ret = i2c_smbus_write_byte_data(sn3112_client, i + DATE_REG_FIRST_ADD - 1, ker_buf[i - 1]);
       if (ret < 0)
        return ret;
    }

    ret = i2c_smbus_write_byte_data(sn3112_client, 0x16, 0xaa);

    return ret;
}

static long sn3112_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret = 0;

    switch(cmd)
    {
        case RGB_SWITCH_CMD:
        {
            harware_rgb_order = arg;
            break;
        }
        default:
        {
            ret = -1;
            break;
        }
    }

    return ret;
}

static struct file_operations sn3112_fops = {
    .owner = THIS_MODULE,
    .open  = sn3112_open,
    .write = sn3112_write,
    .unlocked_ioctl = sn3112_ioctl,
};

void sn3112_buff_switch_to_rgb_order(char *buffer, int count)
{
    int i = 0;
    char temp;

    switch(harware_rgb_order)
    {
        case ORDER_RGB:
        {
            break; //no need to switch,because file order is RGB
        }
        case ORDER_RBG:
        {
            for (i = 0; i < count - 1; i++)
            {
                temp = buffer[i + 1];
                buffer[i + 1] = buffer[i + 2];
                buffer[i + 2] = temp;
                i = i + 2; //should + 2 to jump
            }
            break;
        }
        case ORDER_GRB:
        {
            for (i = 0; i < count - 1; i++)
            {
                temp = buffer[i];
                buffer[i] = buffer[i + 1];
                buffer[i + 1] = temp;
                i = i + 2; //should + 2 to jump
            }
            break;
        }
        case ORDER_GBR:
        {
            for (i = 0; i < count - 1; i++)
            {
                temp = buffer[i];
                buffer[i] = buffer[i + 1];
                buffer[i + 1] = buffer[i + 2];
                buffer[i + 2] = temp;
                i = i + 2; //should + 2 to jump
            }
            break;
        }
        case ORDER_BGR:
        {
            for (i = 0; i < count - 1; i++)
            {
                temp = buffer[i];
                buffer[i] = buffer[i + 2];
                buffer[i + 2] = temp;
                i = i + 2; //should + 2 to jump
            }
            break;
        }
        case ORDER_BRG:
        {
            for (i = 0; i < count - 1; i++)
            {
                temp = buffer[i];
                buffer[i] = buffer[i + 2];
                buffer[i + 2] = buffer[i + 1];
                buffer[i + 1] = temp;
                i = i + 2; //should + 2 to jump
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

/*
 * desc: set chip software onoff
 * onoff: on, 1; off 0;
 */
static int sn3112_software_power_onoff(struct i2c_client *client, u8 onoff)
{
    //reg 0x00 SSD bit to 0, shutdown mode
    u8 ctr = onoff & 0x01;
    int ret = 0;

    ret = i2c_smbus_write_byte_data(client, 0x00, ctr);

    if (ret < 0)
    {
        dev_err(&client->dev, "%s i2c_smbus_write_byte_data ERR \r\n", __func__);
        return ret;
    }

    return 0;
}

/*
    13h 14h 15h LED Control Register (OUT1~OUT3)(OUT4~OUT9)(OUT10~OUT12)
    0 LED off
    1 LED on
*/
static int sn3112_led_ctrl(struct i2c_client *client, u8 *reg, u8 length)
{
    int ret = 0;

    if ((reg == NULL) || (length != 3)) {
        dev_err(&client->dev,"sn3112_led_ctrl args err\r\n");
        return -EINVAL;
    }

    ret = i2c_smbus_write_byte_data(client, 0x13, reg[0]);
    if (ret < 0)
    {
        dev_err(&client->dev, "%s i2c_smbus_write_byte_data err\r\n", __func__);
        return ret;
    }

    ret = i2c_smbus_write_byte_data(client, 0x14, reg[1]);
    if (ret < 0)
    {
        dev_err(&client->dev, "%s i2c_smbus_write_byte_data err\r\n", __func__);
        return ret;
    }

    ret = i2c_smbus_write_byte_data(client, 0x15, reg[2]);
    if (ret < 0)
    {
        dev_err(&client->dev, "%s i2c_smbus_write_byte_data err\r\n", __func__);
        return ret;
    }

    return 0;
}

#ifndef INIT_LED_ON_UBOOT
/*
 * 16h PWM Update Register
 */
static int sn3112_pwm_update(struct i2c_client *client)
{
    int ret = i2c_smbus_write_byte_data(client, 0x16, 0xAA);

    if (ret < 0)
    {
        return ret;
    }

    return 0;
}
#endif

ssize_t sn3112_pwm_write(int index, int count, const char* buf)
{
    int i, ret = 0;
    char switch_buffer[12] = {0};

    for (i = 0; i < count; i++)
    {
        switch_buffer[i] = buf[i];
    }

    if (count >= 3) //should more than 3
    {
        sn3112_buff_switch_to_rgb_order(switch_buffer, count);
    }

    for (i = 0; i <= count - 1; i++)
    {
       //printk("sn3112_pwm_write pwm_reg[%d] buf[%d]=0x%x\n",index+i,i,buf[i]);
       ret = i2c_smbus_write_byte_data(sn3112_client, index + i, switch_buffer[i]);
       if (ret < 0)
          return ret;
    }

    ret = i2c_smbus_write_byte_data(sn3112_client, 0x16, 0x0);

    return ret;
}

static ssize_t sn3112_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int i;

    for (i = 0x04; i <= 0x0f; i++)
    {
       i2c_smbus_write_byte_data(sn3112_client, i, 0x80);
    }

    i2c_smbus_write_byte_data(sn3112_client, 0x16, 0x0);

    return 1;
}

static ssize_t sn3112_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int ret;
    //printk("sn3112_store count=%d,dev=%s\n",count,attr->attr.name);

    static int update_by_user = 0;

	if (update_by_user == 0) {
		update_by_user = 1;
		cancel_delayed_work_sync(&is_priv->delay_work);
	}

    if (!strncmp(attr->attr.name, "sn3112_gva", strlen("sn3112_gva")))
    {
        ret = sn3112_pwm_write(GVA, count, buf);
    }
    else
    {
        printk("sn3112_store:invalid device!\n");
        ret = -1;
    }

    return ret;
}

static DEVICE_ATTR(sn3112_gva, 0664, sn3112_show, sn3112_store);

void sn3112_create_platform_devs(struct device *dev)//333
{
    if (device_create_file(dev, &dev_attr_sn3112_gva))
    {
        device_remove_file(dev, &dev_attr_sn3112_gva);
    }

    return;
}

#ifndef INIT_LED_ON_UBOOT
static int led_sn3112_Init(void)
{
    int i, ret = 0;
    u8 reg[3] = {0xFF, 0xFF, 0xFF};
    
    ret = i2c_smbus_write_byte_data(sn3112_client, 0x17, 0xAA);
    if(ret < 0)
    {
        //dev_err(&client->dev,"sn3112_reset_default ERR\r\n");
        dev_err(&sn3112_client->dev, "led_sn3112_Init  write 0x17 error\n");
        return ret;
    }
    msleep(1);

    ret = sn3112_software_power_onoff(sn3112_client, 1);
    if (ret < 0) 
    {
        dev_err(&sn3112_client->dev, "led_sn3112_Init write onoff error\n");
        return ret;
    }

    ret = sn3112_led_ctrl(sn3112_client, reg, 3);//enable all leds switch
    if (ret < 0)
    {
        dev_err(&sn3112_client->dev, "led_sn3112_Init write led switch error\n");
        return ret;
    }

    // init 12 led to 0x00
    for (i = 0x04; i <= 0x0f; i++)
    {
		if ((i > 0x06) && (i < 0x0d)) {
			ret = i2c_smbus_write_byte_data(sn3112_client, i, 0x80);
		} else { 
			ret = i2c_smbus_write_byte_data(sn3112_client, i, 0x00);
		}
        if (ret < 0)
        {
            //dev_err(&client->dev,"sn3112_reset_default ERR\r\n");	
            dev_err(&sn3112_client->dev, "led_sn3112_Init write led %x init error\n", i - 4);
            return ret;
        }
    }

    //update brightness
    ret = sn3112_pwm_update(sn3112_client);
    if (ret < 0)
    {
        //dev_err(&client->dev,"%s:sn3112_pwm_update ERR\r\n", __func__);
        return ret;
    }

    //printk("led_sn3112_Init");
    return ret;
}
#endif

static int sn3112_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
    int ret;
    struct i2c_adapter *adapter;
    const struct of_device_id *of_id;
    struct device *dev = &client->dev;
    struct device_node *np = dev->of_node;

    dev_err(&client->dev, "sn3112_probe \r\n");

    adapter = to_i2c_adapter(client->dev.parent);

    if (!i2c_check_functionality(adapter, I2C_FUNC_I2C)) {
        dev_err(dev, "failed i2c_check_functionality\n");
        return -EIO;
    }

    sn3112_client = client;

    is_priv = devm_kzalloc(dev, sizeof(*is_priv), GFP_KERNEL);
    if (!is_priv)  
        return -ENOMEM;

    i2c_set_clientdata(client, is_priv);
    
    of_id = of_match_device(of_sn3112_leds_match, dev);
    if (!of_id) {
        printk("<song>Unknown device type \r\n");
        return -EINVAL;
    }

    is_priv->phy_regulator = devm_regulator_get(dev, "sn3112-regulator");
	#if 0
    ret = regulator_get_voltage(is_priv->phy_regulator);
    if (ret != 3300000)
    {
        ret = regulator_set_voltage(is_priv->phy_regulator, 3300000, 3300000);
        if (ret != 0) {
            printk("failed to regulator_set_voltage %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
            return ret;
        }
    }
    ret = regulator_enable(is_priv->phy_regulator);
    if (ret != 0) {
        printk("failed to regulator_set_voltage %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return ret;
    }
	#endif
    major = register_chrdev(0, "sn3112", &sn3112_fops);
    class = class_create(THIS_MODULE, "sn3112");
    device_create(class, NULL, MKDEV(major, 0), NULL, "sn3112"); /* /dev/sn3112 */

    sn3112_create_platform_devs(dev);
    //printk("led_sn3112_Init start\n");
    is_priv->init_gpio = of_get_named_gpio(np, "sdb-gpio", 0);
    
    if (!gpio_is_valid(is_priv->init_gpio)) 
    {
        printk("%s get invalid extamp-sdb-gpio %d\n", __func__, is_priv->init_gpio);
        ret = -EINVAL;
    }

    ret = devm_gpio_request_one(dev, is_priv->init_gpio, GPIOF_OUT_INIT_HIGH, "sn3112 init");
    if (ret < 0)
    {
        printk("%s devm_gpio_request_one fail\n", __func__);
        return ret;
    }

    printk("sn3112 is_priv->init_gpio = %d\n", is_priv->init_gpio);

    if (gpio_is_valid(is_priv->init_gpio))
    {
        gpio_set_value(is_priv->init_gpio, 1);//set high
    }

#ifndef INIT_LED_ON_UBOOT
    if ((led_sn3112_Init() < 0)){
        printk("failed to led_sn3112_Init %s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }
#endif
	INIT_DELAYED_WORK(&is_priv->delay_work, leds_display);
	schedule_delayed_work(&is_priv->delay_work, 0);

    dev_err(&client->dev, "sn3112 init success \r\n");
    return 0;
}

static int  sn3112_remove(struct i2c_client *client)
{
    //printk("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    device_destroy(class, MKDEV(major, 0));
    class_destroy(class);
    unregister_chrdev(major, "sn3112");

    return 0;
}

static void sn3112_shutdown(struct i2c_client *client)
{
    int  i; 
    int ret;
    u8 reg[3] = {0x00, 0x00, 0x00};
    //struct device *dev = &client->dev;

    for (i = 0x04; i <= 0x0f; i++)
    {
        i2c_smbus_write_byte_data(sn3112_client, i, 0);
    }
    i2c_smbus_write_byte_data(sn3112_client, 0x16, 0x0);
    
    ret = sn3112_led_ctrl(sn3112_client, reg, 3);//disable all leds switch
    if (ret) {  
        dev_err(&sn3112_client->dev,"led_sn3112_Init  write led switch error\n");
        return;
    }

    i2c_smbus_write_byte_data(sn3112_client, 0x16, 0x0);

    if (gpio_is_valid(is_priv->init_gpio))
    {
        gpio_set_value(is_priv->init_gpio, 0);//set low
    }

/*
    is_priv->phy_regulator = devm_regulator_get(dev, "sn3112-regulator");
    ret = regulator_get_voltage(is_priv->phy_regulator);
    regulator_disable(is_priv->phy_regulator);
*/
}

static const struct i2c_device_id sn3112_id_table[] = {
    {"sn3112", 0},
    { }
};

MODULE_DEVICE_TABLE(i2c, sn3112_id_table);

static struct dev_pm_ops sn3112_i2c_pm = {
    .suspend    = sn3112_pm_suspend,
    .resume     = sn3112_pm_resume,
};

/* 1. ∑÷≈‰/…Ë÷√i2c_driver */
static struct i2c_driver sn3112_driver = {
    .driver = {
        .name   = "sn3112",
        .owner  = THIS_MODULE,
        .of_match_table = of_match_ptr(of_sn3112_leds_match),
        .pm     = &sn3112_i2c_pm,
    },
    .probe      = sn3112_probe,
    .remove     = sn3112_remove,
    .shutdown   = sn3112_shutdown,
    .id_table   = sn3112_id_table,
};

module_i2c_driver(sn3112_driver);

MODULE_DESCRIPTION("miaoshu");
MODULE_AUTHOR("tonly");
MODULE_LICENSE("GPL");
