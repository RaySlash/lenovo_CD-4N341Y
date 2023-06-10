#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/spinlock.h>

#include <asm/uaccess.h>
#include <linux/gpio.h>

struct tm1604_platform_data{
	int clk_gpio;
	int dat_gpio;
};

static int clk_gpio_num = -1;
static int dat_gpio_num = -1;

void DIN(unsigned char d)
{
  if(gpio_is_valid(dat_gpio_num))
  {
      if(d)
          gpio_set_value(dat_gpio_num, 1);
      else
          gpio_set_value(dat_gpio_num, 0);
  }
}

void CLK(unsigned char d)
{
  if(gpio_is_valid(clk_gpio_num))
  {
      if(d)
          gpio_set_value(clk_gpio_num, 1);
      else
          gpio_set_value(clk_gpio_num, 0);
  }
}


//********************************************//
//***************子函数***********************//
//********************************************//
void delay(void)//几us的延时
{
    //unsigned char i = 2;
    //while(i--);
    udelay(2);
}
void delay_ms(unsigned int ms)//大约ms级的延时
{
    //unsigned int i,j;
    //for(j=0;j<ms;j++)
    //    for(i=0;i<1000;i++);
    msleep(ms);
}

void start(void)									//开始信号
{
    DIN(1);//DIN = 1;
    CLK(1);//CLK = 1;
    delay();
    DIN(0);//DIN = 0;
    delay();
}

void stop(void)									//结束信号
{
	DIN(0);//DIN = 0;
	delay();
	CLK(1);//CLK = 1;
	delay();
	DIN(1);//DIN = 1;
	delay();
}

void write_data(unsigned char dat)             //写数据
{
	unsigned char i;
	for(i=0;i<8;i++)
	{
		CLK(0);//CLK = 0;
		delay();
		DIN(dat & 0x01);//DIN = dat & 0x01;   //低位先传
		CLK(1);//CLK = 1;
		delay();
		dat >>= 1;
	}
  delay();
}

void display(unsigned char dat)								//写显示
{
    unsigned char i;
    start();
    write_data(0x40);                   // 数据设置：普通模式，地址自加，写数据到显示寄存器
    stop();
    start();
    write_data(0xc0);                   //地址设置 ：00H
    for(i=0;i<16;i++)                   //写传输数据
    {
        write_data(dat);
    }
    stop();
    start();
    write_data(0x88);                   //显示控制：开显示，辉度1/16，
    stop();
}

void display_ex(unsigned char data[16], unsigned char gray_lvl)
{
    unsigned char i;
    start();
    write_data(0x40);                   // 数据设置：普通模式，地址自加，写数据到显示寄存器
    stop();
    start();
    write_data(0xc0);                   //地址设置 ：00H
    for(i=0;i<16;i++)                   //写传输数据
    {
      write_data(data[i]);
    }
    stop();
    start();
    write_data(gray_lvl);                   //显示控制：开显示，辉度1/16，
    stop();
}


static ssize_t leds1640_debug_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    unsigned char data[16] = {0};
    unsigned char gray = 0x00;

    if(count == 17)
    {
      memcpy(data, buf, 16);
      gray = buf[16];
      #if 0 //debug show
      printk("--leds1640 data %02x %02x %02x %02x  %02x %02x %02x %02x  %02x %02x %02x %02x  %02x %02x %02x %02x\r\n", \
            data[0], data[1], data[2], data[3], \
            data[4], data[5], data[6], data[7], \
            data[8], data[9], data[10], data[11], \
            data[12], data[13], data[14], data[15]);
      printk("--leds1640 gray 0x%02x\r\n", gray);
      #endif
      display_ex(data, gray);
    }
    return count;
}

static ssize_t leds1640_debug_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", buf);
}

static DEVICE_ATTR(leds1640, 0644, leds1640_debug_show, leds1640_debug_store);


static int tm1604_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct device_node *np = dev->of_node;
    struct tm1604_platform_data *priv;
    int  ret;

    priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
    if (!priv)
    {
        dev_err(dev, "leds1640 failed to allocate state\n");
        return -ENOMEM;
    }

    //printk("<4>tm1604_probe()%d\r\n", __LINE__);

    platform_set_drvdata(pdev, priv);

    priv->clk_gpio = of_get_named_gpio(np, "clk-gpio", 0);
    if (!gpio_is_valid(priv->clk_gpio)) {
        ret = -EINVAL;
    }
    ret = devm_gpio_request_one(dev, priv->clk_gpio, GPIOF_OUT_INIT_HIGH, "tm1640_clk_gpio");
    if (ret < 0)
        return ret;

    //printk("<4>tm1604_probe()%d\r\n", __LINE__);

    priv->dat_gpio = of_get_named_gpio(np, "dat-gpio", 0);
    if (!gpio_is_valid(priv->dat_gpio)) {
        ret = -EINVAL;
    }
    ret = devm_gpio_request_one(dev, priv->dat_gpio, GPIOF_OUT_INIT_HIGH, "tm1640_dat_gpio");
    if (ret < 0)
        return ret;

    clk_gpio_num =  priv->clk_gpio;
    dat_gpio_num =  priv->dat_gpio;

    //printk("<4>tm1604_probe()%d\r\n", __LINE__);
    display(0x00);

    ret = device_create_file(&pdev->dev, &dev_attr_leds1640);
    if (ret)
    {
        dev_err(dev, "error creating sysfs files: leds1640\n");
        return ret;
    }

    return 0;
}

static int tm1604_remove(struct platform_device *pdev)
{
    struct tm1604_platform_data *pGpiodata = platform_get_drvdata(pdev);

    devm_kfree(&pdev->dev, pGpiodata);

    device_remove_file(&pdev->dev, &dev_attr_leds1640);

    return 0;
}

static void tm1604_shutdown(struct platform_device *pdev)
{
    struct tm1604_platform_data *pGpiodata = platform_get_drvdata(pdev);

    if(pGpiodata == NULL)
    {
        return;
    }

    if (gpio_is_valid(pGpiodata->clk_gpio))
        gpio_set_value(pGpiodata->clk_gpio, 1);

    if (gpio_is_valid(pGpiodata->dat_gpio))
        gpio_set_value(pGpiodata->dat_gpio, 1);

    return ;
}

static const struct of_device_id tm1604_of_match[] = {
	{ .compatible = "amlogic, tm1640", },
	{ },
};

#ifdef CONFIG_PM_SLEEP
static int tm1604_suspend(struct device *dev)
{
#if 0
    struct platform_device *pdev = container_of(dev, struct platform_device, dev);
    struct tm1604_platform_data *pGpiodata = platform_get_drvdata(pdev);

    if(pGpiodata == NULL)
    {
        return -1;
    }

    if (gpio_is_valid(PGpiodata->clk_gpio))
        gpio_set_value(pGpiodata->clk_gpio, 0);

    if (gpio_is_valid(pGpiodata->dat_gpio))
        gpio_set_value(pGpiodata->dat_gpio, 0);

#endif
	return 0;
}

static int tm1604_resume(struct device *dev)
{
#if 0
    struct platform_device *pdev = container_of(dev, struct platform_device, dev);
    struct tm1604_platform_data *pGpiodata = platform_get_drvdata(pdev);

    if(pGpiodata == NULL)
    {
        return -1;
    }

    if (gpio_is_valid(pGpiodata->clk_gpio))
        gpio_set_value(pGpiodata->clk_gpio, 1);

    if (gpio_is_valid(pGpiodata->dat_gpio))
        gpio_set_value(pGpiodata->dat_gpio, 1);

#endif
	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(tm1604_pm_ops, tm1604_suspend, tm1604_resume);

static struct platform_driver tm1604_device_driver = {
	.probe		= tm1604_probe,
	.remove		= tm1604_remove,
	.shutdown = tm1604_shutdown,
	.driver		= {
		.name	= "tm1640",
		.pm	= &tm1604_pm_ops,
		.of_match_table = of_match_ptr(tm1604_of_match),
	}
};

static int __init tm1604_init(void)
{
	return platform_driver_register(&tm1604_device_driver);
}

static void __exit tm1604_exit(void)
{
	platform_driver_unregister(&tm1604_device_driver);
}


late_initcall(tm1604_init);
module_exit(tm1604_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tm1640 tangyw@tcl.com");
MODULE_DESCRIPTION("tonly tm1640 driver");
