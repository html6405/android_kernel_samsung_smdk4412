/* linux/drivers/video/samsung/s3cfb_ltn101al03.c
 *
 * LTN101AL03 1280x800 LCD module driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include "s3cfb.h"

#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <mach/regs-clock.h>
#include <plat/gpio-cfg.h>
#include <plat/fb.h>
#include <linux/io.h>
#include <linux/platform_device.h>

static struct s3cfb_lcd ltn101al03 = {
	.width = 1280,
	.height = 800,
	.p_width = 217,
	.p_height = 136,
	.bpp = 24,
	.freq = 60,

	.timing = {
		.h_fp = 16,
		.h_bp = 64,
		.h_sw = 48,
		.v_fp = 1,
		.v_fpe = 1,
		.v_bp = 12,
		.v_bpe = 1,
		.v_sw = 3,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

/* name should be fixed as 's3cfb_set_lcd_info' */
void s3cfb_set_lcd_info(struct s3cfb_global *ctrl)
{
	ltn101al03.init_ldi = NULL;
	ctrl->lcd = &ltn101al03;
}

//static void ltn101al03_cfg_gpio(struct platform_device *pdev)
//{
//	int i;
//
//	for (i = 0; i < 8; i++) {
//		s3c_gpio_cfgpin(S5PV210_GPF0(i), S3C_GPIO_SFN(2));
//		s3c_gpio_setpull(S5PV210_GPF0(i), S3C_GPIO_PULL_NONE);
//	}
//
//	for (i = 0; i < 8; i++) {
//		s3c_gpio_cfgpin(S5PV210_GPF1(i), S3C_GPIO_SFN(2));
//		s3c_gpio_setpull(S5PV210_GPF1(i), S3C_GPIO_PULL_NONE);
//	}
//
//	for (i = 0; i < 8; i++) {
//		s3c_gpio_cfgpin(S5PV210_GPF2(i), S3C_GPIO_SFN(2));
//		s3c_gpio_setpull(S5PV210_GPF2(i), S3C_GPIO_PULL_NONE);
//	}
//
//	for (i = 0; i < 4; i++) {
//		s3c_gpio_cfgpin(S5PV210_GPF3(i), S3C_GPIO_SFN(2));
//		s3c_gpio_setpull(S5PV210_GPF3(i), S3C_GPIO_PULL_NONE);
//	}
//
//	/* mDNIe SEL: why we shall write 0x2 ? */
//	writel(0x2, S5P_MDNIE_SEL);
//
//	/* drive strength to max */
//	writel(0xffffffff, S5PV210_GPF0_BASE + 0xc);
//	writel(0xffffffff, S5PV210_GPF1_BASE + 0xc);
//	writel(0xffffffff, S5PV210_GPF2_BASE + 0xc);
//	writel(0x000003ff, S5PV210_GPF3_BASE + 0xc);
//}
//
//#define S5PV210_GPD_0_0_TOUT_0  (0x2)
//#define S5PV210_GPD_0_1_TOUT_1  (0x2 << 4)
//#define S5PV210_GPD_0_2_TOUT_2  (0x2 << 8)
//#define S5PV210_GPD_0_3_TOUT_3  (0x2 << 12)
//static int ltn101al03_backlight_on(struct platform_device *pdev)
//{
//	int err;
//
//	//LVDS Power on
//	err = gpio_request(S5PV210_GPJ1(5), "GPJ1");
//	if (err) {
//		printk(KERN_ERR "failed to request GPJ1 for "
//			"lcd backlight control\n");
//		return err;
//	}
//	gpio_direction_output(S5PV210_GPJ1(5), 1);
//	gpio_set_value(S5PV210_GPJ1(5), 1);
//	gpio_free(S5PV210_GPJ1(5));
//
//	//LCD Power on
//	err = gpio_request(S5PV210_GPJ2(7), "GPJ2");
//	if (err) {
//		printk(KERN_ERR "failed to request GPJ2 for "
//			"lcd backlight control\n");
//		return err;
//	}
//	gpio_direction_output(S5PV210_GPJ2(7), 1);
//	gpio_set_value(S5PV210_GPJ2(7), 1);
//	gpio_free(S5PV210_GPJ2(7));
//
//	//LCD PWM on
//	err = gpio_request(S5PV210_GPD0(0), "GPD0");
//	if (err) {
//		printk(KERN_ERR "failed to request GPD0 for "
//			"lcd backlight control\n");
//		return err;
//	}
//	gpio_direction_output(S5PV210_GPD0(0), 1);
//	s3c_gpio_cfgpin(S5PV210_GPD0(0), S5PV210_GPD_0_0_TOUT_0);
//	gpio_free(S5PV210_GPD0(0));
//
//	//LCD On Notify to Charger
//	err = gpio_request(S5PV210_GPH2(3), "GPH2");
//	if (err) {
//		printk(KERN_ERR "failed to request GPH2 for "
//			"lcd backlight control\n");
//		return err;
//	}
//	gpio_direction_output(S5PV210_GPH2(3), 1);
//	gpio_set_value(S5PV210_GPH2(3), 0);
//	gpio_free(S5PV210_GPH2(3));
//
//	return 0;
//}
//
//static int ltn101al03_backlight_off(struct platform_device *pdev, int onoff)
//{
//	int err;
//
//	//LCD PWM off
//	err = gpio_request(S5PV210_GPD0(0), "GPD0");
//	if (err) {
//		printk(KERN_ERR "failed to request GPD0 for "
//				"lcd backlight control\n");
//		return err;
//	}
//	gpio_direction_output(S5PV210_GPD0(0), 0);
//	gpio_free(S5PV210_GPD0(0));
//
//	//LCD Power off
//	err = gpio_request(S5PV210_GPJ2(7), "GPJ2");
//	if (err) {
//		printk(KERN_ERR "failed to request GPJ2 for "
//				"lcd backlight control\n");
//		return err;
//	}
//	gpio_direction_output(S5PV210_GPJ2(7), 1);
//	gpio_set_value(S5PV210_GPJ2(7), 0);
//	gpio_free(S5PV210_GPJ2(7));
//
//	//LVDS Power off
//	err = gpio_request(S5PV210_GPJ1(5), "GPJ1");
//	if (err) {
//		printk(KERN_ERR "failed to request GPJ1 for "
//			"lcd backlight control\n");
//		return err;
//	}
//	gpio_direction_output(S5PV210_GPJ1(5), 1);
//	gpio_set_value(S5PV210_GPJ1(5), 0);
//	gpio_free(S5PV210_GPJ1(5));
//
//	//LCD Off Notify to Charger
//	err = gpio_request(S5PV210_GPH2(3), "GPH2");
//	if (err) {
//		printk(KERN_ERR "failed to request GPH2 for "
//			"lcd backlight control\n");
//		return err;
//	}
//	gpio_direction_output(S5PV210_GPH2(3), 1);
//	gpio_set_value(S5PV210_GPH2(3), 1);
//	gpio_free(S5PV210_GPH2(3));
//
//	return 0;
//}
//
//static int ltn101al03_reset_lcd(struct platform_device *pdev)
//{
//	return 0;
//}
//
//static struct s3c_platform_fb ltn101al03_fb_data __initdata = {
//	.hw_ver	= 0x62,
//	.nr_wins = 5,
//	.default_win = CONFIG_FB_S3C_DEFAULT_WINDOW,
//	.swap = FB_SWAP_WORD | FB_SWAP_HWORD,
//
//	.lcd = &ltn101al03,
//	.cfg_gpio	= ltn101al03_cfg_gpio,
//	.backlight_on	= ltn101al03_backlight_on,
//	.backlight_onoff    = ltn101al03_backlight_off,
//	.reset_lcd	= ltn101al03_reset_lcd,
//};

