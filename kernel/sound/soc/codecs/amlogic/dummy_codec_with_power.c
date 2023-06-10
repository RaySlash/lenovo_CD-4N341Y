/*
 * sound/soc/codecs/amlogic/dummy_codec.c
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


#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/tlv.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <linux/of.h>
#include <linux/amlogic/aml_gpio_consumer.h>

struct dummy_codec_private {
	struct snd_soc_codec codec;
	int power_enum_value;
	int power_pin;
	int inited;
	struct delayed_work pop_work;
};

#define DUMMY_CODEC_RATES		(SNDRV_PCM_RATE_8000_192000)
#define DUMMY_CODEC_FORMATS		(SNDRV_PCM_FMTBIT_S16_LE |\
		SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE)

static int dummy_codec_pcm_hw_params(struct snd_pcm_substream *substream,
				     struct snd_pcm_hw_params *params,
				     struct snd_soc_dai *dai)
{
	return 0;
}

static int dummy_codec_set_dai_fmt(struct snd_soc_dai *codec_dai,
				   unsigned int fmt)
{
	return 0;
}

static int dummy_codec_mute(struct snd_soc_dai *dai, int mute)
{
	return 0;
}

static int dummy_get_power(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct snd_soc_codec *codec = snd_soc_component_to_codec(component);
	struct dummy_codec_private *dummy = snd_soc_codec_get_drvdata(codec);

	ucontrol->value.integer.value[0] = dummy->power_enum_value;

	return 0;
}

static int dummy_set_power(struct snd_kcontrol *kcontrol,
				   struct snd_ctl_elem_value *ucontrol)
{
	int ret = -1;
	struct snd_soc_component *component = snd_kcontrol_chip(kcontrol);
	struct snd_soc_codec *codec = snd_soc_component_to_codec(component);
	struct dummy_codec_private *dummy = snd_soc_codec_get_drvdata(codec);

	dummy->power_enum_value = ucontrol->value.integer.value[0];

	pr_info(">>>%s  %d\n", __func__, dummy->power_enum_value);

	if (dummy->power_enum_value == 1) {
		if (dummy->inited == 0) {
			schedule_delayed_work(&dummy->pop_work, msecs_to_jiffies(280));
			dummy->inited = 1;
			return 0;
		}
		ret = gpio_direction_output(dummy->power_pin, 1);
		if (ret < 0) {
			pr_err("%s power on fail\n", __func__);
		}
	} else {
		ret = gpio_direction_output(dummy->power_pin, 0);
		if (ret < 0) {
			pr_err("%s power off fail!\n", __func__);
		}
	}

	return 0;
}

static void pop_work_events(struct work_struct *work)
{
	int ret;
	struct delayed_work *d_work = to_delayed_work(work);
	struct dummy_codec_private *dummy  = container_of(d_work, struct dummy_codec_private, pop_work);

	ret = gpio_direction_output(dummy->power_pin, 1);
	if (ret < 0) {
		pr_err("%s power on fail\n", __func__);
	}

	pr_info("%s, %d\n", __func__, __LINE__);
}


static void dummy_codec_shutdown(struct snd_pcm_substream *substream,
						struct snd_soc_dai *dai)
{
	struct snd_soc_codec *codec = dai->codec;
	struct dummy_codec_private *dummy  = snd_soc_codec_get_drvdata(codec);

	dummy->inited = 0;

	pr_info("%s, %d\n", __func__, __LINE__);
}

static int dummy_codec_prepare(struct snd_pcm_substream *substream,
    struct snd_soc_dai *dai)
{
	struct snd_soc_codec *codec = dai->codec;
	struct dummy_codec_private *dummy  = snd_soc_codec_get_drvdata(codec);
	pr_info("%s  inited:%d\n", __func__, dummy->inited);

	return 0;
}

static const struct snd_soc_dapm_widget dummy_codec_dapm_widgets[] = {
	/* Output Side */
	/* DACs */
	SND_SOC_DAPM_DAC("Left DAC", "HIFI Playback",
			 SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_DAC("Right DAC", "HIFI Playback",
			 SND_SOC_NOPM, 7, 0),

	/* Output Lines */
	SND_SOC_DAPM_OUTPUT("LOUTL"),
	SND_SOC_DAPM_OUTPUT("LOUTR"),

};


static const struct snd_kcontrol_new dummy_snd_controls[] = {
	SOC_SINGLE_BOOL_EXT("Dummy Power", 0,
		   dummy_get_power, dummy_set_power),
};

static const struct snd_soc_dapm_route dummy_codec_dapm_routes[] = {

	{"LOUTL", NULL, "Left DAC"},
	{"LOUTR", NULL, "Right DAC"},
};

static struct snd_soc_dai_ops dummy_codec_ops = {
	.hw_params = dummy_codec_pcm_hw_params,
	.set_fmt = dummy_codec_set_dai_fmt,
	.digital_mute = dummy_codec_mute,
	.shutdown = dummy_codec_shutdown,
	.prepare = dummy_codec_prepare,
};

struct snd_soc_dai_driver dummy_codec_with_power_dai = {
	.name = "dummy",
	.id = 1,
	.playback = {
		     .stream_name = "HIFI Playback",
		     .channels_min = 1,
		     .channels_max = 32,
		     .rates = DUMMY_CODEC_RATES,
		     .formats = DUMMY_CODEC_FORMATS,
		     },
	.capture = {
		    .stream_name = "HIFI Capture",
		    .channels_min = 1,
		    .channels_max = 32,
		    .rates = DUMMY_CODEC_RATES,
		    .formats = DUMMY_CODEC_FORMATS,
		    },
	.ops = &dummy_codec_ops,
};

static int dummy_codec_probe(struct snd_soc_codec *codec)
{
	return 0;
}

static int dummy_codec_remove(struct snd_soc_codec *codec)
{
	return 0;
};

struct snd_soc_codec_driver soc_codec_dev_dummy_codec_with_power = {
	.probe = dummy_codec_probe,
	.remove = dummy_codec_remove,
	.component_driver = {
		.dapm_widgets = dummy_codec_dapm_widgets,
		.controls = dummy_snd_controls,
		.num_controls = ARRAY_SIZE(dummy_snd_controls),
		.num_dapm_widgets = ARRAY_SIZE(dummy_codec_dapm_widgets),
		.dapm_routes = dummy_codec_dapm_routes,
		.num_dapm_routes = ARRAY_SIZE(dummy_codec_dapm_routes),
	}
};

#ifdef CONFIG_OF
static const struct of_device_id amlogic_codec_dt_match[] = {
	{.compatible = "amlogic, aml_dummy_codec_with_power",
	 },
	{},
};
#else
#define amlogic_codec_dt_match NULL
#endif

static int dummy_codec_platform_probe(struct platform_device *pdev)
{
	struct dummy_codec_private *dummy_codec;
	int ret;

	dummy_codec = kzalloc(sizeof(struct dummy_codec_private), GFP_KERNEL);
	if (dummy_codec == NULL)
		return -ENOMEM;

	platform_set_drvdata(pdev, dummy_codec);
	
	dummy_codec->inited = 0;
	dummy_codec->power_pin = of_get_named_gpio(pdev->dev.of_node, "power_pin", 0);
	if (dummy_codec->power_pin < 0) {
		pr_err("%s fail to get power_pin pin from dts!\n", __func__);
		ret = -1;
	} else {
		pr_info("%s power_pin = %d!\n", __func__, dummy_codec->power_pin);
		devm_gpio_request_one(&pdev->dev, dummy_codec->power_pin,
					    GPIOF_OUT_INIT_LOW,
					    "dummy_power_pin");
	}

	ret = snd_soc_register_codec(&pdev->dev, &soc_codec_dev_dummy_codec_with_power,
				     &dummy_codec_with_power_dai, 1);

	if (ret < 0)
		kfree(dummy_codec);

	INIT_DELAYED_WORK(&dummy_codec->pop_work, pop_work_events);

	return ret;
}

static int dummy_codec_platform_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	kfree(platform_get_drvdata(pdev));
	return 0;
}

static struct platform_driver dummy_codec_platform_driver = {
	.driver = {
		   .name = "dummy_with_power",
		   .owner = THIS_MODULE,
		   .of_match_table = amlogic_codec_dt_match,
		   },
	.probe = dummy_codec_platform_probe,
	.remove = dummy_codec_platform_remove,
};

static int __init dummy_codec_init(void)
{
	return platform_driver_register(&dummy_codec_platform_driver);
}

static void __exit dummy_codec_exit(void)
{
	platform_driver_unregister(&dummy_codec_platform_driver);
}

module_init(dummy_codec_init);
module_exit(dummy_codec_exit);

MODULE_AUTHOR("AMLogic, Inc.");
MODULE_DESCRIPTION("ASoC dummy_codec driver");
MODULE_LICENSE("GPL");
