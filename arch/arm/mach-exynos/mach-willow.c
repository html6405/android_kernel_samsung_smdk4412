/* linux/arch/arm/mach-exynos/mach-willow.c
 *
 * Copyright (c) 2012 Thinkware Co., Ltd.
 *		http://www.Thinkware.co.kr
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/platform_device.h>
#include <linux/serial_core.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/clk.h>
#include <linux/lcd.h>
#include <linux/gpio.h>
#include <linux/gpio_keys.h>
#include <linux/i2c.h>
#include <linux/pwm_backlight.h>
#include <linux/input.h>
#include <linux/mmc/host.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/max8649.h>
#include <linux/regulator/fixed.h>
#include <linux/mfd/wm8994/pdata.h>
#include <linux/mfd/max8997.h>
#include <linux/mfd/max77686.h>
#include <linux/v4l2-mediabus.h>
#include <linux/memblock.h>
#include <linux/delay.h>
#if defined(CONFIG_S5P_MEM_CMA)
#include <linux/cma.h>
#endif
#ifdef CONFIG_ANDROID_PMEM
#include <linux/android_pmem.h>
#endif
#include <linux/notifier.h>
#include <linux/reboot.h>
#ifdef CONFIG_HAPTIC_ISA1200
#include <linux/i2c/isa1200.h>
#endif
#include <asm/mach/arch.h>
#include <asm/mach-types.h>

#include <plat/regs-serial.h>
#include <plat/exynos4.h>
#include <plat/cpu.h>
#include <plat/clock.h>
#include <plat/keypad.h>
#include <plat/devs.h>
#include <plat/fb.h>
#include <plat/fb-s5p.h>
#include <plat/fb-core.h>
#include <plat/regs-fb-v4.h>
#include <plat/backlight.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-adc.h>
#include <plat/adc.h>
#include <plat/iic.h>
#include <plat/pd.h>
#include <plat/sdhci.h>
#include <plat/mshci.h>
#include <plat/ehci.h>
#include <plat/usbgadget.h>
#include <plat/s3c64xx-spi.h>
#if defined(CONFIG_VIDEO_FIMC)
#include <plat/fimc.h>
#elif defined(CONFIG_VIDEO_SAMSUNG_S5P_FIMC)
#include <plat/fimc-core.h>
#include <media/s5p_fimc.h>
#endif
#if defined(CONFIG_VIDEO_FIMC_MIPI)
#include <plat/csis.h>
#elif defined(CONFIG_VIDEO_S5P_MIPI_CSIS)
#include <plat/mipi_csis.h>
#endif
#include <plat/tvout.h>
#include <plat/media.h>
#include <plat/regs-srom.h>
#include <plat/sysmmu.h>
#include <plat/tv-core.h>
#if defined(CONFIG_VIDEO_SAMSUNG_S5P_MFC) || defined(CONFIG_VIDEO_MFC5X)
#include <plat/s5p-mfc.h>
#endif

#include <media/exynos_flite.h>
#include <media/exynos_fimc_is.h>
#include <video/platform_lcd.h>
#include <media/m5mo_platform.h>
#include <media/m5mols.h>
#include <plat/system-reset.h>
#include <asm/hardware/gic.h>
#include <mach/board_rev.h>
#include <mach/map.h>
#include <mach/spi-clocks.h>
#include <mach/exynos-ion.h>
#include <mach/regs-pmu.h>
#ifdef CONFIG_EXYNOS4_DEV_DWMCI
#include <mach/dwmci.h>
#endif
#ifdef CONFIG_EXYNOS_CONTENT_PATH_PROTECTION
#include <mach/secmem.h>
#endif
#include <mach/dev.h>
#include <mach/ppmu.h>
#ifdef CONFIG_FB_S5P_MIPI_DSIM
#include <mach/mipi_ddi.h>
#include <mach/dsim.h>
#include <../../../drivers/video/samsung/s3cfb.h>
#endif
#include <plat/fimg2d.h>
#include <mach/dev-sysmmu.h>

#ifdef CONFIG_VIDEO_SAMSUNG_S5P_FIMC
#include <plat/fimc-core.h>
#include <media/s5p_fimc.h>
#endif

#ifdef CONFIG_VIDEO_JPEG_V2X
#include <plat/jpeg.h>
#endif

#include <linux/mfd/s5m87xx/s5m-core.h>
#include <linux/mfd/s5m87xx/s5m-pmic.h>

#if defined(CONFIG_EXYNOS_THERMAL)
#include <mach/tmu.h>
#endif

#ifdef CONFIG_INPUT_L3G4200D_GYR
#include <linux/l3g4200d_gyr.h>
#endif
#ifdef CONFIG_BATTERY_MAX17040
#include <linux/max17040_battery.h>
#include <linux/power/max8903_charger.h>
extern bool usb_is_connected;
extern int dc_is_connected;
#endif

#ifdef CONFIG_ISL29023
#include <linux/isl29023.h>
#endif

#include <mach/gpio.h>
#if defined(CONFIG_TOUCHSCREEN_WILLOW_MXT1664S)
#include <linux/i2c/1664s_driver.h>
#endif
#include <linux/platform_data/usb3503.h>

/* wifi */
extern int brcm_wlan_init(void);

#define WILLOW_BOOT_NORMAL			1
#define WILLOW_BOOT_RECOVERY		2
#define WILLOW_BOOT_HOTKEY_UPDATE	3
#define WILLOW_BOOT_FACTORYTEST_L	4
#define WILLOW_BOOT_FACTORYTEST_H	5
#define WILLOW_BOOT_FASTBOOT      6
#define REG_INFORM4            (S5P_INFORM4)

#include <linux/i2c-gpio.h>

#ifdef CONFIG_VIDEO_MT9M113
#include <media/mt9m113_platform.h>
#define CONFIG_ITU_A
#undef  CAM_ITU_CH_B
#endif

#ifdef CONFIG_VIDEO_AS0260
#include <media/as0260_platform.h>
#undef  CONFIG_ITU_A
#undef  CAM_ITU_CH_B
#define CONFIG_CSI_D
#endif

#if defined(CONFIG_TOUCHSCREEN_WILLOW_FT5X06) || defined(CONFIG_TOUCHSCREEN_WILLOW_MXT1664S)
// focal 1, atmel 2
int touch_ic_check =0;
#define ATMEL_1664S_DEBUG
#ifdef ATMEL_1664S_DEBUG
#define ATMEL_log(fmt, arg...) 	printk(fmt, ##arg)
#else
#define ATMEL_log(fmt, arg...)
#endif
#endif

#ifdef CONFIG_MACH_WILLOW
#include <mach/willow_version.h>
WILLOW_HW_VERSION g_willow_hw_version = WILLOW_HW_UNKNOWN;

void willow_check_hw_version( void )
{
	static int ADC0_HW = 0; // GPL2_2, GPIO_HW_VERSION_0
	static int ADC1_HW = 0; // GPL2_1, GPIO_HW_VERSION_1
	static int ADC2_HW = 0; // GPL2_0, GPIO_HW_VERSION_2 //RESERVERD e.g. LCD

	char *str_version[] = {
		"WILLOW_HW_DVT",
		"WILLOW_HW_MVT",
		"WILLOW_HW_PP",
		"WILLOW_HW_MP",
		"WILLOW_HW_MP2",
		"WILLOW_HW_RESERVED0",
		"WILLOW_HW_RESERVED1",
		"WILLOW_HW_RESERVED2",
		"WILLOW_HW_UNKNOWN"
	};

	/* Set GPL2[0],[1],[2] Control Register to input */
	s3c_gpio_cfgpin(GPIO_HW_VERSION_0, S3C_GPIO_SFN(0x0));
	s3c_gpio_cfgpin(GPIO_HW_VERSION_1, S3C_GPIO_SFN(0x0));
	s3c_gpio_cfgpin(GPIO_HW_VERSION_2, S3C_GPIO_SFN(0x0));

	/* Disable Pullup/Pulldown */
	s3c_gpio_setpull(GPIO_HW_VERSION_0, S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(GPIO_HW_VERSION_1, S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(GPIO_HW_VERSION_2, S3C_GPIO_PULL_NONE);

	/* Read GPL2[0],[1],[2] Data Register
	 * --------------------------------------------------
	 * VER | ADC2_HW_VER0 | ADC1_HW_VER0 | ADC0_HW_VER0 |
	 * --------------------------------------------------
	 * DVT |       0      |       0      |       0      | //0
	 * --------------------------------------------------
	 * MVT |       0      |       0      |       1      | //1
	 * --------------------------------------------------
	 * PP  |       0      |       1      |       0      | //2
	 * --------------------------------------------------
	 * MP  |       0      |       1      |       1      | //3
	 * --------------------------------------------------
	 * MP2 |       1      |       0      |       0      | //4
 	 * --------------------------------------------------
	 * R0  |       1      |       0      |       1      | //5
 	 * --------------------------------------------------
	 * R1  |       1      |       1      |       0      | //6
	 * --------------------------------------------------
	 * R2  |       1      |       1      |       1      | //7
	 * --------------------------------------------------
	 */
	ADC0_HW = gpio_get_value(GPIO_HW_VERSION_0) << 0; //bit0
	printk("ADC0_HW[0x%01X]\n", ADC0_HW);
	ADC1_HW = gpio_get_value(GPIO_HW_VERSION_1) << 1; //bit1
	printk("ADC1_HW[0x%01X]\n", ADC1_HW);
	ADC2_HW = gpio_get_value(GPIO_HW_VERSION_2) << 2; //bit2
	printk("ADC2_HW[0x%01X]\n", ADC2_HW);

	g_willow_hw_version = ADC2_HW | ADC1_HW | ADC0_HW;
	printk("g_willow_hw_version[0x%01X]\n", g_willow_hw_version);

	printk("WILLOW HW_VERSION: [%s]\n", str_version[g_willow_hw_version]);
}

WILLOW_HW_VERSION willow_get_hw_version( void )
{
	static int ADC0_HW = 0; // GPL2_2, GPIO_HW_VERSION_0
	static int ADC1_HW = 0; // GPL2_1, GPIO_HW_VERSION_1
	static int ADC2_HW = 0; // GPL2_0, GPIO_HW_VERSION_2 //RESERVERD e.g. LCD

	ADC0_HW = gpio_get_value(GPIO_HW_VERSION_0) << 0; //bit0
	ADC1_HW = gpio_get_value(GPIO_HW_VERSION_1) << 1; //bit1
	ADC2_HW = gpio_get_value(GPIO_HW_VERSION_2) << 2; //bit2

	return g_willow_hw_version = ADC2_HW | ADC1_HW | ADC0_HW;
}
EXPORT_SYMBOL(willow_get_hw_version);
#endif /* CONFIG_MACH_WILLOW */

/* Following are default values for UCON, ULCON and UFCON UART registers */
#define WILLOW_UCON_DEFAULT	(S3C2410_UCON_TXILEVEL |	\
				 S3C2410_UCON_RXILEVEL |	\
				 S3C2410_UCON_TXIRQMODE |	\
				 S3C2410_UCON_RXIRQMODE |	\
				 S3C2410_UCON_RXFIFO_TOI |	\
				 S3C2443_UCON_RXERR_IRQEN)

#define WILLOW_ULCON_DEFAULT	S3C2410_LCON_CS8

#define WILLOW_UFCON_DEFAULT	(S3C2410_UFCON_FIFOMODE |	\
				 S5PV210_UFCON_TXTRIG4 |	\
				 S5PV210_UFCON_RXTRIG4)

static struct s3c2410_uartcfg willow_uartcfgs[] __initdata = {
	[0] = {
		.hwport		= 0,
		.flags		= 0,
		.ucon		= WILLOW_UCON_DEFAULT,
		.ulcon		= WILLOW_ULCON_DEFAULT,
		.ufcon		= WILLOW_UFCON_DEFAULT,
	},
	[1] = {
		.hwport		= 1,
		.flags		= 0,
		.ucon		= WILLOW_UCON_DEFAULT,
		.ulcon		= WILLOW_ULCON_DEFAULT,
		.ufcon		= WILLOW_UFCON_DEFAULT,
	},
	[2] = {
		.hwport		= 2,
		.flags		= 0,
		.ucon		= WILLOW_UCON_DEFAULT,
		.ulcon		= WILLOW_ULCON_DEFAULT,
		.ufcon		= WILLOW_UFCON_DEFAULT,
	},
	[3] = {
		.hwport		= 3,
		.flags		= 0,
		.ucon		= WILLOW_UCON_DEFAULT,
		.ulcon		= WILLOW_ULCON_DEFAULT,
		.ufcon		= WILLOW_UFCON_DEFAULT,
	},
};

void s3c_setup_uart_cfg_gpio(unsigned char port)
{
	switch (port) {
	case 0:
		s3c_gpio_cfgpin(GPIO_BT_RXD, S3C_GPIO_SFN(GPIO_BT_RXD_AF));
		s3c_gpio_setpull(GPIO_BT_RXD, S3C_GPIO_PULL_UP);
		s3c_gpio_cfgpin(GPIO_BT_TXD, S3C_GPIO_SFN(GPIO_BT_TXD_AF));
		s3c_gpio_setpull(GPIO_BT_TXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_BT_CTS, S3C_GPIO_SFN(GPIO_BT_CTS_AF));
		s3c_gpio_setpull(GPIO_BT_CTS, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_BT_RTS, S3C_GPIO_SFN(GPIO_BT_RTS_AF));
		s3c_gpio_setpull(GPIO_BT_RTS, S3C_GPIO_PULL_NONE);
		break;
	case 1:
		s3c_gpio_cfgpin(GPIO_GPS_RXD, S3C_GPIO_SFN(GPIO_GPS_RXD_AF));
		s3c_gpio_setpull(GPIO_GPS_RXD, S3C_GPIO_PULL_UP);
		s3c_gpio_cfgpin(GPIO_GPS_TXD, S3C_GPIO_SFN(GPIO_GPS_TXD_AF));
		s3c_gpio_setpull(GPIO_GPS_TXD, S3C_GPIO_PULL_NONE);
		break;
	case 2:
		s3c_gpio_cfgpin(GPIO_AP_RXD, S3C_GPIO_SFN(GPIO_AP_RXD_AF));
		s3c_gpio_setpull(GPIO_AP_RXD, S3C_GPIO_PULL_UP);
		s3c_gpio_cfgpin(GPIO_AP_TXD, S3C_GPIO_SFN(GPIO_AP_TXD_AF));
		s3c_gpio_setpull(GPIO_AP_TXD, S3C_GPIO_PULL_NONE);
		break;
	case 3:
		s3c_gpio_cfgpin(GPIO_TEST_RXD, S3C_GPIO_SFN(GPIO_TEST_RXD_AF));
		s3c_gpio_setpull(GPIO_TEST_RXD, S3C_GPIO_PULL_UP);
		s3c_gpio_cfgpin(GPIO_TEST_TXD, S3C_GPIO_SFN(GPIO_TEST_TXD_AF));
		s3c_gpio_setpull(GPIO_TEST_TXD, S3C_GPIO_PULL_NONE);
		break;
	default:
		break;
	}
}
EXPORT_SYMBOL(s3c_setup_uart_cfg_gpio);

#ifdef CONFIG_EXYNOS_MEDIA_DEVICE
struct platform_device exynos_device_md0 = {
	.name = "exynos-mdev",
	.id = -1,
};
#endif

#ifdef CONFIG_SEC_WATCHDOG_RESET
static struct platform_device watchdog_reset_device = {
	.name = "watchdog-reset",
	.id = -1,
};
#endif

//#define WRITEBACK_ENABLED

#if defined(CONFIG_VIDEO_FIMC) || defined(CONFIG_VIDEO_SAMSUNG_S5P_FIMC)
/*
 * External camera reset
 * Because the most of cameras take i2c bus signal, so that
 * you have to reset at the boot time for other i2c slave devices.
 * This function also called at fimc_init_camera()
 * Do optimization for cameras on your platform.
*/
#if defined(CONFIG_ITU_A) 
static int smdk4x12_cam0_reset(int dummy)
{
#if  0
	int err;
	/* Camera A */
	err = gpio_request(EXYNOS4_GPX1(2), "GPX1");
	if (err)
		printk(KERN_ERR "#### failed to request GPX1_2 ####\n");

	s3c_gpio_setpull(EXYNOS4_GPX1(2), S3C_GPIO_PULL_NONE);
	gpio_direction_output(EXYNOS4_GPX1(2), 0);
	gpio_direction_output(EXYNOS4_GPX1(2), 1);
	gpio_free(EXYNOS4_GPX1(2));
#endif
	return 0;
}
#endif
#endif

#ifdef CONFIG_VIDEO_FIMC

/*  MT9M113 Camera driver configuration */

#ifdef CONFIG_VIDEO_MT9M113
void mt9m113_stanby(void)
{
	int err;

	err = gpio_request(EXYNOS4212_GPM1(5), "GPM1_5"); //stnby
	if (err)
		printk(KERN_ERR "#### failed to request GPM1_5 stnby ####\n");
	
		gpio_direction_output(EXYNOS4212_GPM1(5), 1); // stnby
		mdelay(300);		
		gpio_direction_output(EXYNOS4212_GPM1(5), 0); // stnby	
		mdelay(50);		

}
EXPORT_SYMBOL(mt9m113_stanby);

static int mt9m113_power_en(int onoff)
{
	int err;
	/* Camera A */
	// Vdd_cam_io 1.8 vdd_cam 2.8v
	struct regulator *camera_vio = regulator_get(NULL, "vdd_cam_io");
	struct regulator *camera_vdd = regulator_get(NULL, "vdd_cam");
	
	err = gpio_request(EXYNOS4212_GPM1(4), "GPM1_4"); //reset
	if (err)
		printk(KERN_ERR "#### failed to request GPM1_4 ####\n");

	err = gpio_request(EXYNOS4212_GPM1(5), "GPM1_5"); //stnby
	if (err)
		printk(KERN_ERR "#### failed to request GPM1_5 ####\n");

	s3c_gpio_cfgpin(EXYNOS4212_GPM1(4), S3C_GPIO_SFN(1));
	s3c_gpio_setpull(EXYNOS4212_GPM1(4), S3C_GPIO_PULL_NONE);
	s3c_gpio_cfgpin(EXYNOS4212_GPM1(5), S3C_GPIO_SFN(1));
	s3c_gpio_setpull(EXYNOS4212_GPM1(5), S3C_GPIO_PULL_NONE);

	if (onoff) {
		//gpio_direction_output(EXYNOS4212_GPM1(5), 1); // stnby
		//mdelay(10);		
		//gpio_direction_output(EXYNOS4212_GPM1(5), 0); // stnby
		mdelay(10);		
		gpio_direction_output(EXYNOS4212_GPM1(5), 1); // stnby
		mdelay(10);		
		gpio_direction_output(EXYNOS4212_GPM1(5), 0); // stnby
		
		regulator_enable(camera_vdd);
		mdelay(50);
		regulator_enable(camera_vio);
		mdelay(10);
		
		//gpio_direction_output(EXYNOS4212_GPM1(5), 1); // stnby

		gpio_direction_output(EXYNOS4212_GPM1(4), 0);  //reset
		msleep(50);
		gpio_direction_output(EXYNOS4212_GPM1(4), 1);
		msleep(100);
		
		//gpio_direction_output(EXYNOS4212_GPM1(5), 1); // stnby
		//mdelay(10);		
		//gpio_direction_output(EXYNOS4212_GPM1(5), 0); // stnby		
		
	} else {
		gpio_direction_output(EXYNOS4212_GPM1(5), 1); // stnby
		msleep(50);	
		if (regulator_is_enabled(camera_vdd))
			regulator_disable(camera_vdd);
		msleep(50);		
		if (regulator_is_enabled(camera_vio))
			regulator_disable(camera_vio);
		msleep(50);
		gpio_direction_output(EXYNOS4212_GPM1(4), 0);  //reset
	}
	regulator_put(camera_vdd);
	regulator_put(camera_vio);
	gpio_free(EXYNOS4212_GPM1(4));
	gpio_free(EXYNOS4212_GPM1(5));
	msleep(10);

	printk("mt9m113_power_en %d \n", onoff);
	return 0;
}

void mt9m113_gpio_init(void)
{
	/* i2c scl, sda */
	s3c_gpio_cfgpin(EXYNOS4212_GPM4(1), S3C_GPIO_INPUT);
	s3c_gpio_setpull(EXYNOS4212_GPM4(1), S3C_GPIO_PULL_UP);
	s3c_gpio_cfgpin(EXYNOS4212_GPM4(0), S3C_GPIO_INPUT);
	s3c_gpio_setpull(EXYNOS4212_GPM4(0), S3C_GPIO_PULL_UP);
}

static struct mt9m113_platform_data mt9m113_plat = {
	.default_width = WILLOW_PREVIEW_MIN_W,//480,
	.default_height = WILLOW_PREVIEW_MIN_H,
	.max_width = WILLOW_PREVIEW_MAX_W,//960,
	.max_height =WILLOW_PREVIEW_MAX_H,
	.pixelformat = V4L2_PIX_FMT_UYVY,
	.freq = 24000000,
	.is_mipi = 0,
};
static struct i2c_board_info mt9m113_i2c_info = {
	I2C_BOARD_INFO("MT9M113", 0x78),
	.platform_data = &mt9m113_plat,
};

static struct s3c_platform_camera mt9m113 = {

	.id		= CAMERA_PAR_A,
	.clk_name	= "sclk_cam0",
	.cam_power	= mt9m113_power_en,
	.i2c_busnum = 9,
	.type		= CAM_TYPE_ITU,
	.fmt		= ITU_601_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_CBYCRY,
	.info		= &mt9m113_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_UYVY,
	.srclk_name	= "xusbxti",

	.clk_rate	= 24000000,
	.line_length	= WILLOW_PREVIEW_MAX_W,
	.width		= WILLOW_PREVIEW_MAX_W,
	.height		= WILLOW_PREVIEW_MAX_H,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= WILLOW_PREVIEW_MAX_W,
		.height	= WILLOW_PREVIEW_MAX_H,
	},

	.mipi_lanes	= 0,
	.mipi_settle	= 0,
	.mipi_align	= 0,

	/* Polarity */
	.inv_pclk	= 0,
	.inv_vsync	= 0,
	.inv_href	= 0,
	.inv_hsync	= 0,
	.use_isp	= 0,
	.initialized	= 0,
};

#endif

#ifdef CONFIG_VIDEO_AS0260

void as0260_i2c_gpio_init(void)
{
	/* i2c scl, sda */
	s3c_gpio_cfgpin(GPIO_CAMERA_SCL, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_CAMERA_SCL, S3C_GPIO_PULL_UP);
	s3c_gpio_cfgpin(GPIO_CAMERA_SDA, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_CAMERA_SDA, S3C_GPIO_PULL_UP);
}
static void __init as0260_camera_config(void)
{
	int ret=0;
	//CAM_SHUTDOWN  Low Setting GPM1_5 
	ret = gpio_request(GPIO_CAMERA_SHUTDOWN, "GPM1_5");
	if (ret)
		printk(KERN_ERR "#### failed to request GPM1_5 ####\n");

	s3c_gpio_setpull(GPIO_CAMERA_SHUTDOWN, S3C_GPIO_PULL_NONE);
	gpio_direction_output(GPIO_CAMERA_SHUTDOWN, 0);
	gpio_free(GPIO_CAMERA_SHUTDOWN);	
}

int as0260_camera_reset(void)
{
	int err;
	mdelay(50);

	err = gpio_request(GPIO_CAMERA_RESET, "GPM1_4");
	if (err)
		printk(KERN_ERR "#### failed to request GPM1_4 ####\n");

	printk("AS0260 CAMERA_RESET START___________\n");

	s3c_gpio_setpull(GPIO_CAMERA_RESET, S3C_GPIO_PULL_NONE);
	s3c_gpio_cfgpin(GPIO_CAMERA_RESET, S3C_GPIO_OUTPUT);

	gpio_set_value(GPIO_CAMERA_RESET, 1);
	mdelay(10);  // 10
	gpio_set_value(GPIO_CAMERA_RESET, 0);
	mdelay(5);   // 5
	gpio_set_value(GPIO_CAMERA_RESET, 1);	
	mdelay(30);  // 35
	gpio_free(GPIO_CAMERA_RESET);
	
	printk("AS0260 CAMERA_RESET END ____________\n");
	return 0;
}
EXPORT_SYMBOL(as0260_camera_reset);


int as0260_camera_reset_ctrl(int ctrl)
{
	int err;
	mdelay(50);

	err = gpio_request(GPIO_CAMERA_RESET, "GPM1_4");
	if (err)
		printk(KERN_ERR "#### failed to request GPM1_4 ####\n");

	printk("AS0260 CAMERA_RESET START___________\n");

	s3c_gpio_setpull(GPIO_CAMERA_RESET, S3C_GPIO_PULL_NONE);
	s3c_gpio_cfgpin(GPIO_CAMERA_RESET, S3C_GPIO_OUTPUT);


	gpio_set_value(GPIO_CAMERA_RESET, ctrl);
	gpio_free(GPIO_CAMERA_RESET);
	
	printk("AS0260 CAMERA_RESET END ____________\n");
	return 0;
}

int as0260_power_ctrl(int ctrl)
{
	int err=0;

/* 	Camera Power UP Seq 
	1. Reset High GPM1_4
	2. VDDC  vdd_cam_core ldo21
	3. mdelay (50)~200 
	4. VDD_IO  vdd_cam_io ldo5
	5. AVDD     vdd_cam ldo17
*/
	struct regulator *camera_vddc = regulator_get(NULL, "vdd_cam_core");    
	struct regulator *camera_vddi = regulator_get(NULL, "vdd_cam_io");
	struct regulator *camera_vdda = regulator_get(NULL, "vdd_cam");

	printk("[AS0260] _____ as0260_power_ctrl =%d \n",ctrl);
	if(ctrl==1)
	{
		err = gpio_request(EXYNOS4212_GPM1(4), "GPM1_4");
		if (err)
			printk(KERN_ERR "#### failed to request GPM1_4 ####\n");

		printk("#### AS0260 CAMERA_RESET####\n");

		s3c_gpio_setpull(GPIO_CAMERA_RESET, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_CAMERA_RESET, S3C_GPIO_OUTPUT);

		gpio_set_value(GPIO_CAMERA_RESET, 1);
		gpio_free(GPIO_CAMERA_RESET);		

		mdelay(10);

		err=regulator_set_voltage(camera_vddc, 1850000, 1850000);
		if (err)
			printk("[AS0260] _____ camera_vddc enable  regulator_set_voltage err ..... \n");

		err=	regulator_enable(camera_vddc);
		if (err)
			printk("[AS0260] _____ as0260_power_ctrl enable  camera_vddc err ..... \n");

		mdelay(150);  // 150

		err=regulator_set_voltage(camera_vddi, 1850000, 1850000);
		if (err)
			printk("[AS0260] _____ camera_vddi enable  regulator_set_voltage err ..... \n");
		
		err=	regulator_enable(camera_vddi);
		if (err)
			printk("[AS0260] _____ as0260_power_ctrl enable  camera_vddi err ..... \n");
		
		mdelay(35);  // 30

		err=regulator_set_voltage(camera_vdda, 2850000, 2850000);
		if (err)
			printk("[AS0260] _____ camera_vdda enable  regulator_set_voltage err ..... \n");
		
		err=	regulator_enable(camera_vdda);
		if (err)
			printk("[AS0260] _____ as0260_power_ctrl enable  camera_vdda err ..... \n");

		//mdelay(70);  //50

	} else {
		if (regulator_is_enabled(camera_vdda))
			regulator_disable(camera_vdda);

		mdelay(150);		
		if (regulator_is_enabled(camera_vddi))
			regulator_disable(camera_vddi);
		mdelay(30);
		
		if (regulator_is_enabled(camera_vddc))
			regulator_disable(camera_vddc);

		mdelay(20);	

		as0260_camera_reset_ctrl(0);
		
	}
	regulator_put(camera_vddc);
	regulator_put(camera_vddi);
	regulator_put(camera_vdda);

	return 0;
}

///#define MCLK_20MHZ
#define MCLK_22MHZ
//#define MCLK_23MHZ
//#define MCLK_48MHZ

//#define FEATURE_CAMERA_MIPI_1_LANE

static struct as0260_platform_data as0260_plat = {
	.default_width =1920,//640, //1920,
	.default_height =1080,// 480,//1080,
	.max_width = WILLOW_PREVIEW_MAX_W,//960,
	.max_height =WILLOW_PREVIEW_MAX_H,
	.pixelformat = V4L2_PIX_FMT_UYVY,

#if defined(MCLK_20MHZ)
	.freq = 20000000,  
#elif defined(MCLK_22MHZ)
	.freq = 22000000,  //23000000
#elif defined(MCLK_23MHZ)
	.freq = 23200000,  //23000000
#elif defined(MCLK_48MHZ)
	.freq = 24000000,
#else
	.freq = 24000000,
#endif	
	.is_mipi = 1,
	
};

static struct i2c_board_info as0260_i2c_info = {
	I2C_BOARD_INFO("AS0260", 0x90>>1),
	.platform_data = &as0260_plat,
};

static struct s3c_platform_camera as0260= {
	.id		= CAMERA_CSI_D,
	.clk_name	= "sclk_cam1",
	.i2c_busnum = 9,
	.cam_power	= as0260_power_ctrl,
	.type		= CAM_TYPE_MIPI,
	.fmt		= MIPI_CSI_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_YCBYCR,
	.info		= &as0260_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_UYVY,
	.srclk_name	= "xusbxti",
#if defined(MCLK_22MHZ)
	.clk_rate = 22000000, 
#elif defined(MCLK_23MHZ)
	.clk_rate = 23600000, 
#elif defined(MCLK_48MHZ)
	.clk_rate = 48000000,
#else
	.clk_rate = 24000000,
#endif	
	.line_length	= 1920,
	.width		= 1920,//1920,
	.height		= 1080,//,1080,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= 1920,//1920,
		.height	= 1080,//1080,
	},
#if defined(FEATURE_CAMERA_MIPI_1_LANE)
	.mipi_lanes	= 1,
	.mipi_settle = 12,
	.mipi_align = 24,
#else
	.mipi_lanes	= 2,
	.mipi_settle	= 12,
	.mipi_align	= 32,
#endif
	/* Polarity */
	.inv_pclk	= 1, // 0 
	.inv_vsync	= 1, // 1 

	.inv_href	= 0,
	.inv_hsync	= 0,
	.use_isp	= false,
	.initialized	= 0,
};
#endif

#if defined(CONFIG_VIDEO_MT9M113) ||defined(CONFIG_VIDEO_AS0260) 
static struct i2c_gpio_platform_data i2c9_platdata = {
	.scl_pin =GPIO_CAMERA_SCL,
	.sda_pin = GPIO_CAMERA_SDA,
	.udelay = 2,  //250Mhz
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.scl_is_output_only = 0,
};

static struct platform_device s3c_device_i2c9= {
	.name = "i2c-gpio",
	.id = 9,
	.dev.platform_data = &i2c9_platdata,
};

#if ! defined(CONFIG_VIDEO_AS0260) 
static struct i2c_board_info i2c_devs9[] __initdata = {
#if defined(CONFIG_VIDEO_MT9M113) 
	{
		I2C_BOARD_INFO("MT9M113", (0x78 >> 1)),
	},
#endif	
};
#endif

#endif

/* Interface setting */
static struct s3c_platform_fimc fimc_plat = {
#ifdef CONFIG_ITU_A
	.default_cam	= CAMERA_PAR_A,
#endif
#ifdef CONFIG_CSI_D
	.default_cam	= CAMERA_CSI_D,
#endif
	.camera		= {
#ifdef CONFIG_VIDEO_MT9M113	
		&mt9m113,
#endif	
#ifdef CONFIG_VIDEO_AS0260
		&as0260,
#endif
	},
	.hw_ver		= 0x51,
};
#endif /* CONFIG_VIDEO_FIMC */

#ifdef CONFIG_FB_S5P
#ifdef CONFIG_FB_S5P_LTN101AL03
static struct s3c_platform_fb ltn101al03_fb_data __initdata = {
	.hw_ver = 0x70,
	.clk_name = "sclk_lcd",
	.nr_wins = 5,
	.default_win = CONFIG_FB_S5P_DEFAULT_WINDOW,
	.swap = FB_SWAP_HWORD | FB_SWAP_WORD,
};
#endif
#endif

#ifdef CONFIG_BATTERY_MAX17040
void max8903_gpio_init(void)
{
	// nCHG_EN
	s3c_gpio_cfgpin(nCHG_EN , S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(nCHG_EN, S3C_GPIO_PULL_NONE);

	// nCHARGING
	s3c_gpio_cfgpin(nCHARGING , S3C_GPIO_INPUT);
	s3c_gpio_setpull(nCHARGING, S3C_GPIO_PULL_UP);

	// nDC_OK
	s3c_gpio_cfgpin(nDC_OK , S3C_GPIO_INPUT);
	s3c_gpio_setpull(nDC_OK, S3C_GPIO_PULL_UP);

	// nUSB_OK
	s3c_gpio_cfgpin(nUSB_OK , S3C_GPIO_INPUT);
	s3c_gpio_setpull(nUSB_OK, S3C_GPIO_PULL_NONE);

	// nBAT_FLT
	s3c_gpio_cfgpin(nBAT_FLT , S3C_GPIO_INPUT);
	s3c_gpio_setpull(nBAT_FLT, S3C_GPIO_PULL_UP);
}

static int max8903g_charger_enable(void)
{
	gpio_set_value(nCHG_EN, 0);
	return gpio_get_value(nCHARGING) ? 0 : 1;
}

static void max8903g_charger_disable(void)
{
	gpio_set_value(nCHG_EN, 1);
}

static int max8903g_charger_done(void)
{
	return gpio_get_value(nCHARGING) ? 1 : 0;
}

static int max8903g_charger_online(void)
{
	gpio_set_value(nCHG_EN, 0);
	return gpio_get_value(nDC_OK) ? 0 : 1;
}

static int max8903g_battery_online(void)
{
	/*think that  battery always is inserted */
	return 1;
}

static struct max8903_pdata max8903_platform_data = {
	.cen = nCHG_EN,	/* Charger Enable input */
	.dok = nDC_OK,	/* DC(Adapter) Power OK output */
	.uok = nUSB_OK,	/* USB Power OK output */
	.chg = nCHARGING,	/* Charger status output */
	.flt = nBAT_FLT,	/* Fault output */
	//.dcm = 0,	/* Current-Limit Mode input (1: DC, 2: USB) */
	//.usus = 0,	/* USB Suspend Input (1: suspended) */
	.dc_valid = true,
	.usb_valid = true,
	.charger_online = NULL,
};

static struct platform_device willow_charger = {
	.name = "max8903-charger",
	.id = 1,
	.dev = {
		.platform_data = &max8903_platform_data,
	},
};

static struct max17040_platform_data max17040_platform_data = {
	.rep      = 0,
	.chg_en_gpio = nCHG_EN,
	.charger_enable = max8903g_charger_enable,
	.charger_online = max8903g_charger_online,
	.battery_online = max8903g_battery_online,
	.charger_done = max8903g_charger_done,
	.charger_disable = max8903g_charger_disable,
	//.rcomp_value = 0xD700,
};
#endif

static void willow_power_off(void)
{
	if (dc_is_connected ||usb_is_connected) {
		// reboot
		writel(2, S5P_INFORM6);
		arch_reset('r', NULL);
	} else {
		// shutdown
		pr_info("%s: set PS_HOLD low\n", __func__);
		writel(readl(S5P_PS_HOLD_CONTROL) & 0xFFFFFEFF, S5P_PS_HOLD_CONTROL);
		while(1);
	}
}

static void willow_pm_restart(char mode, const char *cmd)
{
  if (cmd!=0){
	  printk("%s : %s\n",__func__,cmd);
    if(strncmp(cmd,"recovery",7)==0){
      writel(WILLOW_BOOT_RECOVERY, S5P_INFORM5);
    }
    else if(strncmp(cmd,"hotkey_update",13)==0){
      writel(WILLOW_BOOT_HOTKEY_UPDATE, S5P_INFORM5);
    }
    else if(strncmp(cmd,"FACTORYTEST_L",13)==0){
      writel(WILLOW_BOOT_FACTORYTEST_L, S5P_INFORM5);
    }
    else if(strncmp(cmd,"FACTORYTEST_H",13)==0){
      writel(WILLOW_BOOT_FACTORYTEST_H, S5P_INFORM5);
    }
    else if(strncmp(cmd,"fastboot",8)==0){
      writel(WILLOW_BOOT_FASTBOOT, S5P_INFORM5);
    }
    //unknown reboot command, anyway set register to normal boot
    else
      writel(WILLOW_BOOT_NORMAL, S5P_INFORM5);
  }
  else
    writel(WILLOW_BOOT_NORMAL, S5P_INFORM5);

	/*
	 * Code from t10s gingerbrad kernel 'arch/arm/kernel/process.c.'
	 * If INFORM6 has '3', bootloader will not enter the charger mode.
	 */
	writel(3, S5P_INFORM6);
	mdelay(50);

	arch_reset(mode, cmd);
}
static int exynos4_notifier_call(struct notifier_block *this,
					unsigned long code, void *_cmd)
{
	int mode = 0;

	if ((code == SYS_RESTART) && _cmd)
		if (!strcmp((char *)_cmd, "recovery"))
			mode = 0xf;

	__raw_writel(mode, REG_INFORM4);

	return NOTIFY_DONE;
}

static struct notifier_block exynos4_reboot_notifier = {
	.notifier_call = exynos4_notifier_call,
};

#ifdef CONFIG_EXYNOS4_DEV_DWMCI
static void exynos_dwmci_cfg_gpio(int width)
{
	unsigned int gpio;

	for (gpio = EXYNOS4_GPK0(0); gpio < EXYNOS4_GPK0(2); gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
		s5p_gpio_set_drvstr(gpio, S5P_GPIO_DRVSTR_LV2);
	}

	switch (width) {
	case 8:
		for (gpio = EXYNOS4_GPK1(3); gpio <= EXYNOS4_GPK1(6); gpio++) {
			s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(4));
			s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
			s5p_gpio_set_drvstr(gpio, S5P_GPIO_DRVSTR_LV2);
		}
	case 4:
		for (gpio = EXYNOS4_GPK0(3); gpio <= EXYNOS4_GPK0(6); gpio++) {
			s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
			s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
			s5p_gpio_set_drvstr(gpio, S5P_GPIO_DRVSTR_LV2);
		}
		break;
	case 1:
		gpio = EXYNOS4_GPK0(3);
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
		s5p_gpio_set_drvstr(gpio, S5P_GPIO_DRVSTR_LV2);
	default:
		break;
	}
}

static struct dw_mci_board exynos_dwmci_pdata __initdata = {
	.num_slots		= 1,
	.quirks			= DW_MCI_QUIRK_BROKEN_CARD_DETECTION | DW_MCI_QUIRK_HIGHSPEED,
	.bus_hz			= 100 * 1000 * 1000,
	.caps			= MMC_CAP_UHS_DDR50 | MMC_CAP_1_8V_DDR |
				MMC_CAP_8_BIT_DATA | MMC_CAP_CMD23,
	.fifo_depth		= 0x80,
	.detect_delay_ms	= 200,
	.hclk_name		= "dwmci",
	.cclk_name		= "sclk_dwmci",
	.cfg_gpio		= exynos_dwmci_cfg_gpio,
};
#endif

static DEFINE_MUTEX(notify_lock);

#define DEFINE_MMC_CARD_NOTIFIER(num) \
static void (*hsmmc##num##_notify_func)(struct platform_device *, int state); \
static int ext_cd_init_hsmmc##num(void (*notify_func)( \
					struct platform_device *, int state)) \
{ \
	mutex_lock(&notify_lock); \
	WARN_ON(hsmmc##num##_notify_func); \
	hsmmc##num##_notify_func = notify_func; \
	mutex_unlock(&notify_lock); \
	return 0; \
} \
static int ext_cd_cleanup_hsmmc##num(void (*notify_func)( \
					struct platform_device *, int state)) \
{ \
	mutex_lock(&notify_lock); \
	WARN_ON(hsmmc##num##_notify_func != notify_func); \
	hsmmc##num##_notify_func = NULL; \
	mutex_unlock(&notify_lock); \
	return 0; \
}

#ifdef CONFIG_S3C_DEV_HSMMC3
	DEFINE_MMC_CARD_NOTIFIER(3)
#endif

/*
 * call this when you need sd stack to recognize insertion or removal of card
 * that can't be told by SDHCI regs
 */
void mmc_force_presence_change(struct platform_device *pdev)
{
	void (*notify_func)(struct platform_device *, int state) = NULL;
	mutex_lock(&notify_lock);
#ifdef CONFIG_S3C_DEV_HSMMC3
	if (pdev == &s3c_device_hsmmc3)
		notify_func = hsmmc3_notify_func;
#endif

	if (notify_func)
		notify_func(pdev, 1);
	else
		pr_warn("%s: called for device with no notifier\n", __func__);
	mutex_unlock(&notify_lock);
}
EXPORT_SYMBOL_GPL(mmc_force_presence_change);

#ifdef CONFIG_S3C_DEV_HSMMC
static struct s3c_sdhci_platdata willow_hsmmc0_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_INTERNAL,
	.clk_type		= S3C_SDHCI_CLK_DIV_EXTERNAL,
#ifdef CONFIG_EXYNOS4_SDHCI_CH0_8BIT
	.max_width		= 8,
	.host_caps		= MMC_CAP_8_BIT_DATA,
#endif
};
#endif

#ifdef CONFIG_S3C_DEV_HSMMC1
static struct s3c_sdhci_platdata willow_hsmmc1_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_INTERNAL,
	.clk_type		= S3C_SDHCI_CLK_DIV_EXTERNAL,
};
#endif

#ifdef CONFIG_S3C_DEV_HSMMC2
static struct s3c_sdhci_platdata willow_hsmmc2_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_INTERNAL,
	.clk_type		= S3C_SDHCI_CLK_DIV_EXTERNAL,
#ifdef CONFIG_EXYNOS4_SDHCI_CH2_8BIT
	.max_width		= 8,
	.host_caps		= MMC_CAP_8_BIT_DATA,
#endif
};
static struct s3c_sdhci_platdata willow_hsmmc2_pdata_MP __initdata = {
	.cd_type		= S3C_SDHCI_CD_GPIO,
	.ext_cd_gpio		= EXYNOS4_GPX0(1),
	.ext_cd_gpio_invert	= true,
	.clk_type		= S3C_SDHCI_CLK_DIV_EXTERNAL,
#ifdef CONFIG_EXYNOS4_SDHCI_CH2_8BIT
	.max_width		= 8,
	.host_caps		= MMC_CAP_8_BIT_DATA,
#endif
};
#endif

#ifdef CONFIG_S3C_DEV_HSMMC3
static struct s3c_sdhci_platdata willow_hsmmc3_pdata __initdata = {
/* new code for brm4334 */
	.cd_type		= S3C_SDHCI_CD_EXTERNAL,
	.clk_type		= S3C_SDHCI_CLK_DIV_EXTERNAL,
	.pm_flags = S3C_SDHCI_PM_IGNORE_SUSPEND_RESUME,
	.ext_cd_init = ext_cd_init_hsmmc3,
	.ext_cd_cleanup = ext_cd_cleanup_hsmmc3,
};
#endif

#ifdef CONFIG_EXYNOS4_DEV_MSHC
static struct s3c_mshci_platdata exynos4_mshc_pdata __initdata = {
	.cd_type		= S3C_MSHCI_CD_PERMANENT,
	.fifo_depth		= 0x80,
#if defined(CONFIG_EXYNOS4_MSHC_8BIT) && \
	defined(CONFIG_EXYNOS4_MSHC_DDR)
	.max_width		= 8,
	.host_caps		= MMC_CAP_8_BIT_DATA | MMC_CAP_1_8V_DDR |
				  MMC_CAP_UHS_DDR50,
#elif defined(CONFIG_EXYNOS4_MSHC_8BIT)
	.max_width		= 8,
	.host_caps		= MMC_CAP_8_BIT_DATA,
#elif defined(CONFIG_EXYNOS4_MSHC_DDR)
	.host_caps		= MMC_CAP_1_8V_DDR | MMC_CAP_UHS_DDR50,
#endif
	.int_power_gpio		= GPIO_XMMC0_CDn,
};
#endif

#ifdef CONFIG_USB_EHCI_S5P
static struct s5p_ehci_platdata willow_ehci_pdata;

static void __init willow_ehci_init(void)
{
	struct s5p_ehci_platdata *pdata = &willow_ehci_pdata;

	s5p_ehci_set_platdata(pdata);
}
#endif

#ifdef CONFIG_USB_OHCI_S5P
static struct s5p_ohci_platdata willow_ohci_pdata;

static void __init willow_ohci_init(void)
{
	struct s5p_ohci_platdata *pdata = &willow_ohci_pdata;

	s5p_ohci_set_platdata(pdata);
}
#endif

/* USB GADGET */
#ifdef CONFIG_USB_GADGET
static struct s5p_usbgadget_platdata willow_usbgadget_pdata;

static void __init willow_usbgadget_init(void)
{
	struct s5p_usbgadget_platdata *pdata = &willow_usbgadget_pdata;

	s5p_usbgadget_set_platdata(pdata);
}
#endif

#ifdef CONFIG_MFD_MAX77686
/* max77686 */
static struct regulator_consumer_supply max77686_buck1[] = {
		REGULATOR_SUPPLY("vdd_mif", NULL),
		REGULATOR_SUPPLY("vdd_mif", "exynos4412-busfreq"),
};

static struct regulator_consumer_supply max77686_buck2 =
REGULATOR_SUPPLY("vdd_arm", NULL);

static struct regulator_consumer_supply max77686_buck3[] = {
		REGULATOR_SUPPLY("vdd_int", NULL),
		REGULATOR_SUPPLY("vdd_int", "exynoss4412-busfreq"),
};

static struct regulator_consumer_supply max77686_buck4[] = {
		REGULATOR_SUPPLY("vdd_g3d", NULL),
		REGULATOR_SUPPLY("vdd_g3d", "mali_dev.0"),
};

static struct regulator_consumer_supply max77686_buck6 =
REGULATOR_SUPPLY("vdd_buck6", NULL);

static struct regulator_consumer_supply max77686_buck7 =
REGULATOR_SUPPLY("vdd_in235", NULL);

static struct regulator_consumer_supply max77686_buck8 =
REGULATOR_SUPPLY("vmmc", NULL);

static struct regulator_consumer_supply __initdata max77686_ldo1_consumer =
REGULATOR_SUPPLY("vdd_alive", NULL);

static struct regulator_consumer_supply __initdata max77686_ldo2_consumer =
REGULATOR_SUPPLY("vdd_m12", NULL);

static struct regulator_consumer_supply __initdata max77686_ldo3_consumer[] = {
		REGULATOR_SUPPLY("vdd_io", NULL),
		REGULATOR_SUPPLY("vdd_bt_wifi_io", NULL),
};

static struct regulator_consumer_supply __initdata max77686_ldo4_consumer[] = {
		REGULATOR_SUPPLY("vdd_sd", NULL),
		REGULATOR_SUPPLY("vdd_mmc012", NULL),
};

static struct regulator_consumer_supply __initdata max77686_ldo5_consumer =
REGULATOR_SUPPLY("vdd_cam_io", NULL);

static struct regulator_consumer_supply __initdata max77686_ldo6_consumer =
REGULATOR_SUPPLY("vdd_tsp_1v8", NULL);

static struct regulator_consumer_supply __initdata max77686_ldo7_consumer =
REGULATOR_SUPPLY("vdd_pll", NULL);

static struct regulator_consumer_supply __initdata max77686_ldo8_consumer =
REGULATOR_SUPPLY("vdd_mipi_1v0", NULL);

static struct regulator_consumer_supply __initdata max77686_ldo9_consumer =
REGULATOR_SUPPLY("vdd_gps_1v8", NULL);

static struct regulator_consumer_supply __initdata max77686_ldo10_consumer =
REGULATOR_SUPPLY("vdd_mipi_1v8", NULL);

static struct regulator_consumer_supply __initdata max77686_ldo11_consumer =
REGULATOR_SUPPLY("vdd_abb", NULL);

static struct regulator_consumer_supply __initdata max77686_ldo12_consumer =
REGULATOR_SUPPLY("vdd_uotg_3v3", NULL);

static struct regulator_consumer_supply __initdata max77686_ldo13_consumer =
REGULATOR_SUPPLY("vdd_c2c_1v8", NULL);

static struct regulator_consumer_supply __initdata max77686_ldo14_consumer =
REGULATOR_SUPPLY("vdd_mmc01_1v8", NULL);

static struct regulator_consumer_supply __initdata max77686_ldo15_consumer =
REGULATOR_SUPPLY("vdd_uotg_1v0", NULL);

static struct regulator_consumer_supply __initdata max77686_ldo16_consumer =
REGULATOR_SUPPLY("vdd_hsic", NULL);

static struct regulator_consumer_supply __initdata max77686_ldo17_consumer =
REGULATOR_SUPPLY("vdd_cam", NULL);

static struct regulator_consumer_supply __initdata max77686_ldo18_consumer =
REGULATOR_SUPPLY("vdd_gps_2v8", NULL);

#if 0 //reset timming bug
static struct regulator_consumer_supply __initdata max77686_ldo20_consumer =
REGULATOR_SUPPLY("vdd_mmc_1v8", NULL);
#endif

static struct regulator_consumer_supply __initdata max77686_ldo21_consumer =
REGULATOR_SUPPLY("vdd_cam_core", NULL);

static struct regulator_consumer_supply __initdata max77686_ldo22_consumer =
REGULATOR_SUPPLY("vdd_bt_wifi", NULL);

static struct regulator_consumer_supply __initdata max77686_ldo23_consumer =
REGULATOR_SUPPLY("vdd_lcd", NULL);

static struct regulator_consumer_supply __initdata max77686_ldo24_consumer =
REGULATOR_SUPPLY("vdd_sensor", NULL);

static struct regulator_consumer_supply max77686_ldo25_consumer[] = {
	REGULATOR_SUPPLY("DCVDD", "1-001a"),
	REGULATOR_SUPPLY("DBVDD", "1-001a"),
	REGULATOR_SUPPLY("AVDD1", "1-001a"),
	REGULATOR_SUPPLY("AVDD2", "1-001a"),
};

static struct regulator_consumer_supply __initdata max77686_ldo26_consumer =
REGULATOR_SUPPLY("vdd_tsp", NULL);

static struct regulator_consumer_supply __initdata max77686_32khcp_consumer =
REGULATOR_SUPPLY("lpo", "bcm4334_bluetooth");

static struct regulator_consumer_supply __initdata max77686_enp32khz_consumer =
REGULATOR_SUPPLY("gps_32khz",NULL);

#define REGULATOR_INIT(_ldo, _name, _min_uV, _max_uV, _always_on, _ops_mask, \
		       _disabled)					\
	static struct regulator_init_data _ldo##_init_data = {		\
		.constraints = {					\
			.name = _name,					\
			.min_uV = _min_uV,				\
			.max_uV = _max_uV,				\
			.always_on	= _always_on,			\
			.boot_on	= _always_on,			\
			.apply_uV	= 1,				\
			.valid_ops_mask = _ops_mask,			\
			.state_mem = {					\
				.disabled	= _disabled,		\
				.enabled	= !(_disabled),		\
			}						\
		},							\
		.num_consumer_supplies = ARRAY_SIZE(_ldo##_supply),	\
		.consumer_supplies = &_ldo##_supply[0],			\
	};

static struct regulator_init_data max77686_buck1_data = {
	.constraints = {
		.name = "vdd_mif range",
		.min_uV = 850000,
		.max_uV = 1100000,
		.boot_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
			REGULATOR_CHANGE_STATUS,
		.state_mem = {
			.disabled	= 1,
			.enabled 	= 0,
		},
	},
	.num_consumer_supplies = ARRAY_SIZE(max77686_buck1),
	.consumer_supplies = max77686_buck1,
};

static struct regulator_init_data max77686_buck2_data = {
	.constraints = {
		.name = "vdd_arm range",
		.min_uV = 850000,
		.max_uV = 1500000,
		.boot_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
				REGULATOR_CHANGE_STATUS,
		.state_mem = {
			.disabled	= 1,
			.enabled 	= 0,
		},
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &max77686_buck2,
};

static struct regulator_init_data max77686_buck3_data = {
	.constraints = {
		.name = "vdd_int range",
		.min_uV = 850000,
		.max_uV = 1150000,
		.boot_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
				REGULATOR_CHANGE_STATUS,
		.state_mem = {
			.disabled	= 1,
			.enabled 	= 0,
		},
	},
	.num_consumer_supplies = ARRAY_SIZE(max77686_buck3),
	.consumer_supplies = max77686_buck3,
};

static struct regulator_init_data max77686_buck4_data = {
	.constraints = {
		.name = "vdd_g3d range",
		.min_uV = 850000,
		.max_uV = 1100000,
		.boot_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies = ARRAY_SIZE(max77686_buck4),
	.consumer_supplies = max77686_buck4,
};

static struct regulator_init_data max77686_buck6_data = {
	.constraints = {
		.name = "vdd buck6",
		.min_uV = 2000000,
		.max_uV = 2000000,
		.apply_uV	= 1,
		.boot_on = 1,
		.state_mem = {
			.disabled	= 1,
			.enabled 	= 0,
		},
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &max77686_buck6,
};

static struct regulator_init_data max77686_buck7_data = {
	.constraints = {
		.name = "vdd_in235 range",
		.min_uV = 3300000,
		.max_uV = 3300000,
		.apply_uV	= 1,
		.boot_on = 1,
		.state_mem = {
			.disabled	= 1,
			.enabled 	= 0,
		},
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &max77686_buck7,
};

static struct regulator_init_data max77686_buck8_data = {
	.constraints = { // temporarily boot_on
		.name = "vdd_emmc_2v8 range",
		.min_uV = 2800000,
		.max_uV = 2800000,
		.boot_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
					  REGULATOR_CHANGE_STATUS,
		.state_mem = {
			.disabled	= 1,
			.enabled 	= 0,
		},
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &max77686_buck8,
};

static struct regulator_init_data __initdata max77686_ldo1_data = {
	.constraints	= {
		.name		= "vdd_alive range",
		.min_uV		= 1000000,
		.max_uV		= 1000000,
		.apply_uV	= 1,
		.always_on	= 1,
		.boot_on 	= 1,
		.state_mem	= {
			.disabled	= 0,
			.enabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo1_consumer,
};

static struct regulator_init_data __initdata max77686_ldo2_data = {
	.constraints	= {
		.name		= "vdd_m12 range",
		.min_uV		= 1200000,
		.max_uV		= 1200000,
		.apply_uV	= 1,
		.boot_on 	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo2_consumer,
};

static struct regulator_init_data __initdata max77686_ldo3_data = {
	.constraints	= {
		.name		= "vdd_io/vdd_bt_wifi_io range",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.boot_on 	= 1,
		.state_mem	= {
			.disabled	= 0,
			.enabled	= 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(max77686_ldo3_consumer),
	.consumer_supplies	= max77686_ldo3_consumer,
};

static struct regulator_init_data __initdata max77686_ldo4_data = {
	.constraints	= {
		.name		= "vdd_sd/vdd_mmc012 range",
		.min_uV		= 1200000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.boot_on 	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
					  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 2800000,
			.disabled	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(max77686_ldo4_consumer),
	.consumer_supplies	= max77686_ldo4_consumer,
};

static struct regulator_init_data __initdata max77686_ldo5_data = {
	.constraints	= {
		.name		= "vdd_cam_io range",
		.min_uV		= 1800000,
		.max_uV		= 1900000,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,
		.state_mem	= {
			.disabled	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo5_consumer,
};

static struct regulator_init_data __initdata max77686_ldo6_data = {
	.constraints	= {
		.name		= "vdd_tsp_1v8 range",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo6_consumer,
};

static struct regulator_init_data __initdata max77686_ldo7_data = {
	.constraints	= {
		.name		= "vdd_pll range",
		.min_uV		= 1100000,
		.max_uV		= 1100000,
		.apply_uV	= 1,
		.boot_on 	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled 	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo7_consumer,
};

static struct regulator_init_data __initdata max77686_ldo8_data = {
	.constraints	= {
		.name		= "vdd_mipi_1v0 range",
		.min_uV		= 1000000,
		.max_uV		= 1000000,
		.apply_uV	= 1,
		.boot_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo8_consumer,
};

static struct regulator_init_data __initdata max77686_ldo9_data = {
	.constraints	= {
		.name		= "vdd_gps_1v8 range",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo9_consumer,
};

static struct regulator_init_data __initdata max77686_ldo10_data = {
	.constraints	= { //VDDQ_PRE,VDD_ADC
		.name		= "vdd_mipi_1v8 range",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.boot_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo10_consumer,
};

static struct regulator_init_data __initdata max77686_ldo11_data = {
	.constraints	= {
		.name		= "vdd_abb_1v8 range",
		.min_uV		= 1950000,
		.max_uV		= 1950000,
		.apply_uV	= 1,
		.boot_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo11_consumer,
};

static struct regulator_init_data __initdata max77686_ldo12_data = {
	.constraints	= {
		.name		= "vdd_uotg_3v3 range",
		.min_uV		= 3300000,
		.max_uV		= 3300000,
		.apply_uV	= 1,
		.boot_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo12_consumer,
};

static struct regulator_init_data __initdata max77686_ldo13_data = {
	.constraints	= {
		.name		= "vdd_c2c range",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.boot_on 	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo13_consumer,
};

static struct regulator_init_data __initdata max77686_ldo14_data = {
	.constraints	= {
		.name		= "vdd_mmc01_1v8 range",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.boot_on 	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo14_consumer,
};

static struct regulator_init_data __initdata max77686_ldo15_data = {
	.constraints	= {
		.name		= "vdd_uotg_1v0 range",
		.min_uV		= 1000000,
		.max_uV		= 1000000,
		.apply_uV	= 1,
		.boot_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo15_consumer,
};

static struct regulator_init_data __initdata max77686_ldo16_data = {
	.constraints	= {
		.name		= "vdd_hsic range",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.boot_on 	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo16_consumer,
};

static struct regulator_init_data __initdata max77686_ldo17_data = {
	.constraints	= {
		.name		= "vdd_cam range",
		.min_uV		= 2800000,
#ifdef CONFIG_VIDEO_MT9M113
		.max_uV		= 2800000,
#else
		.max_uV		= 2900000,
#endif
		.apply_uV	= 1,
#ifdef CONFIG_VIDEO_MT9M113
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
#else
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,
#endif
		.state_mem	= {
			.disabled	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo17_consumer,
};

static struct regulator_init_data __initdata max77686_ldo18_data = {
	.constraints	= {
		.name		= "vdd_gps_2v8 range",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo18_consumer,
};

#if 0  //reset timming bug
static struct regulator_init_data __initdata max77686_ldo20_data = {
	.constraints	= {
		.name		= "vdd_mmc_1v8 range",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.boot_on 	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo20_consumer,
};
#endif

static struct regulator_init_data __initdata max77686_ldo21_data = {
	.constraints	= {
		.name		= "vdd_cam_core range",
		.min_uV		= 1800000,
		.max_uV		= 1900000,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,
		.state_mem	= {
			.disabled	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo21_consumer,
};

static struct regulator_init_data __initdata max77686_ldo22_data = {
	.constraints	= {
		.name		= "vdd_bt_wifi range",
		.min_uV		= 3300000,
		.max_uV		= 3300000,
		.apply_uV	= 1,
		.boot_on 	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 0,
			.enabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo22_consumer,
};

static struct regulator_init_data __initdata max77686_ldo23_data = {
	.constraints	= {
		.name		= "vdd_lcd range",
		.min_uV		= 3300000,
		.max_uV		= 3300000,
		.apply_uV	= 1,
		.boot_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo23_consumer,
};

static struct regulator_init_data __initdata max77686_ldo24_data = {
	.constraints	= {
		.name		= "vdd_sensor range",
		.min_uV		= 3000000,
		.max_uV		= 3000000,
		.boot_on	= 1,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo24_consumer,
};

static struct regulator_init_data __initdata max77686_ldo25_data = {
	.constraints	= {
		.name		= "vdd_audio range",
		.min_uV		= 3300000,
		.max_uV		= 3300000,
		.apply_uV	= 1,
		.boot_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(max77686_ldo25_consumer),
	.consumer_supplies	= max77686_ldo25_consumer,
};

static struct regulator_init_data __initdata max77686_ldo26_data = {
	.constraints	= {
		.name		= "vdd_tsp range",
		.min_uV		= 3000000,
		.max_uV		= 3300000,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV		= 3000000,
			.disabled	= 1,
			.enabled	= 0,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo26_consumer,
};

static struct regulator_init_data max77686_enp32khz_data = {
	.constraints = {
		.name = "32KHZ_PMIC",
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem = {
			.enabled	= 1,
			.disabled	= 0,
		},
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &max77686_enp32khz_consumer,
};

static struct regulator_init_data max77686_32khcp_data = {
	.constraints = {
		.name = "32KHCP_PMIC",
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem = {
			.enabled	= 1,
			.disabled	= 0,
		},
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &max77686_32khcp_consumer,
};

static struct max77686_regulator_data max77686_regulators[] = {
	{MAX77686_BUCK1, &max77686_buck1_data,},
	{MAX77686_BUCK2, &max77686_buck2_data,},
	{MAX77686_BUCK3, &max77686_buck3_data,},
	{MAX77686_BUCK4, &max77686_buck4_data,},
	{MAX77686_BUCK6, &max77686_buck6_data,},
	{MAX77686_BUCK7, &max77686_buck7_data,},
	{MAX77686_BUCK8, &max77686_buck8_data,},
	{MAX77686_LDO1, &max77686_ldo1_data,},
	{MAX77686_LDO2, &max77686_ldo2_data,},
	{MAX77686_LDO3, &max77686_ldo3_data,},
	{MAX77686_LDO4, &max77686_ldo4_data,},
	{MAX77686_LDO5, &max77686_ldo5_data,},
	{MAX77686_LDO6, &max77686_ldo6_data,},
	{MAX77686_LDO7, &max77686_ldo7_data,},
	{MAX77686_LDO8, &max77686_ldo8_data,},
	{MAX77686_LDO9, &max77686_ldo9_data,},
	{MAX77686_LDO10, &max77686_ldo10_data,},
	{MAX77686_LDO11, &max77686_ldo11_data,},
	{MAX77686_LDO12, &max77686_ldo12_data,},
	{MAX77686_LDO13, &max77686_ldo13_data,},
	{MAX77686_LDO14, &max77686_ldo14_data,},
	{MAX77686_LDO15, &max77686_ldo15_data,},
	{MAX77686_LDO16, &max77686_ldo16_data,},
	{MAX77686_LDO17, &max77686_ldo17_data,},
	{MAX77686_LDO18, &max77686_ldo18_data,},
//	{MAX77686_LDO20, &max77686_ldo20_data,},
	{MAX77686_LDO21, &max77686_ldo21_data,},
	{MAX77686_LDO22, &max77686_ldo22_data,},
	{MAX77686_LDO23, &max77686_ldo23_data,},
	{MAX77686_LDO24, &max77686_ldo24_data,},
	{MAX77686_LDO25, &max77686_ldo25_data,},
	{MAX77686_LDO26, &max77686_ldo26_data,},
	{MAX77686_EN32KHZ_CP, &max77686_32khcp_data,},
//	{MAX77686_P32KH, &max77686_enp32khz_data,},
};

struct max77686_opmode_data max77686_opmode_data[MAX77686_REG_MAX] = {
	[MAX77686_LDO1] = {MAX77686_LDO1, MAX77686_OPMODE_LP},
	[MAX77686_LDO2] = {MAX77686_LDO2, MAX77686_OPMODE_STANDBY},
	[MAX77686_LDO3] = {MAX77686_LDO3, MAX77686_OPMODE_NORMAL},
	[MAX77686_LDO4] = {MAX77686_LDO4, MAX77686_OPMODE_LP},
	[MAX77686_LDO5] = {MAX77686_LDO5, MAX77686_OPMODE_NORMAL},
	[MAX77686_LDO6] = {MAX77686_LDO6, MAX77686_OPMODE_STANDBY},
	[MAX77686_LDO7] = {MAX77686_LDO7, MAX77686_OPMODE_STANDBY},
	[MAX77686_LDO8] = {MAX77686_LDO8, MAX77686_OPMODE_STANDBY},
	[MAX77686_LDO9] = {MAX77686_LDO9, MAX77686_OPMODE_LP},
	[MAX77686_LDO10] = {MAX77686_LDO10, MAX77686_OPMODE_LP},
	[MAX77686_LDO11] = {MAX77686_LDO11, MAX77686_OPMODE_STANDBY},
	[MAX77686_LDO12] = {MAX77686_LDO12, MAX77686_OPMODE_STANDBY},
	[MAX77686_LDO13] = {MAX77686_LDO13, MAX77686_OPMODE_LP},
	[MAX77686_LDO14] = {MAX77686_LDO14, MAX77686_OPMODE_STANDBY},
	[MAX77686_LDO15] = {MAX77686_LDO15, MAX77686_OPMODE_STANDBY},
	[MAX77686_LDO16] = {MAX77686_LDO16, MAX77686_OPMODE_STANDBY},
	[MAX77686_LDO17] = {MAX77686_LDO17, MAX77686_OPMODE_LP},
	[MAX77686_LDO18] = {MAX77686_LDO18, MAX77686_OPMODE_LP},
//	[MAX77686_LDO20] = {MAX77686_LDO20, MAX77686_OPMODE_LP},
	[MAX77686_LDO21] = {MAX77686_LDO21, MAX77686_OPMODE_LP},
	[MAX77686_LDO22] = {MAX77686_LDO22, MAX77686_OPMODE_NORMAL},
	[MAX77686_LDO23] = {MAX77686_LDO23, MAX77686_OPMODE_LP},
	[MAX77686_LDO24] = {MAX77686_LDO24, MAX77686_OPMODE_LP},
	[MAX77686_LDO25] = {MAX77686_LDO25, MAX77686_OPMODE_LP},
	[MAX77686_LDO26] = {MAX77686_LDO26, MAX77686_OPMODE_LP},
	[MAX77686_BUCK1] = {MAX77686_BUCK1, MAX77686_OPMODE_STANDBY},
	[MAX77686_BUCK2] = {MAX77686_BUCK2, MAX77686_OPMODE_STANDBY},
	[MAX77686_BUCK3] = {MAX77686_BUCK3, MAX77686_OPMODE_STANDBY},
	[MAX77686_BUCK4] = {MAX77686_BUCK4, MAX77686_OPMODE_STANDBY},
	[MAX77686_BUCK6] = {MAX77686_BUCK6, MAX77686_OPMODE_NORMAL},
	[MAX77686_BUCK7] = {MAX77686_BUCK7, MAX77686_OPMODE_NORMAL},
	[MAX77686_BUCK8] = {MAX77686_BUCK8, MAX77686_OPMODE_NORMAL},
};

static struct max77686_platform_data exynos4_max77686_info = {
	.num_regulators = ARRAY_SIZE(max77686_regulators),
	.regulators = max77686_regulators,
	.irq_gpio	= GPIO_PMIC_IRQ,
	.irq_base	= IRQ_BOARD_PMIC_START,
	.wakeup		= 1,

	.opmode_data = max77686_opmode_data,
	.ramp_rate = MAX77686_RAMP_RATE_27MV,

	.buck234_gpio_dvs = {
			/* Use DVS2 register of each bucks to supply stable power
			 * after sudden reset */
			{PMIC_SET2, 1},
			{PMIC_SET3, 0},
			{PMIC_SET4, 0},
		},
	.buck234_gpio_selb = {
			PMIC_DVS1,
			PMIC_DVS2,
			PMIC_DVS3,
		},

	.buck2_voltage[0] = 1300000,	/* 1.3V */
	.buck2_voltage[1] = 1000000,	/* 1.0V */
	.buck2_voltage[2] = 950000,	/* 0.95V */
	.buck2_voltage[3] = 900000,	/* 0.9V */
	.buck2_voltage[4] = 1000000,	/* 1.0V */
	.buck2_voltage[5] = 1000000,	/* 1.0V */
	.buck2_voltage[6] = 950000,	/* 0.95V */
	.buck2_voltage[7] = 900000,	/* 0.9V */

	.buck3_voltage[0] = 1037500,	/* 1.0375V */
	.buck3_voltage[1] = 1000000,	/* 1.0V */
	.buck3_voltage[2] = 950000,	/* 0.95V */
	.buck3_voltage[3] = 900000,	/* 0.9V */
	.buck3_voltage[4] = 1000000,	/* 1.0V */
	.buck3_voltage[5] = 1000000,	/* 1.0V */
	.buck3_voltage[6] = 950000,	/* 0.95V */
	.buck3_voltage[7] = 900000,	/* 0.9V */

	.buck4_voltage[0] = 1100000,	/* 1.1V */
	.buck4_voltage[1] = 1000000,	/* 1.0V */
	.buck4_voltage[2] = 950000,	/* 0.95V */
	.buck4_voltage[3] = 900000,	/* 0.9V */
	.buck4_voltage[4] = 1000000,	/* 1.0V */
	.buck4_voltage[5] = 1000000,	/* 1.0V */
	.buck4_voltage[6] = 950000,	/* 0.95V */
	.buck4_voltage[7] = 900000,	/* 0.9V */
};
#endif

#ifdef CONFIG_VIDEO_S5P_MIPI_CSIS
static struct regulator_consumer_supply mipi_csi_fixed_voltage_supplies[] = {
	REGULATOR_SUPPLY("mipi_csi", "s5p-mipi-csis.0"),
	REGULATOR_SUPPLY("mipi_csi", "s5p-mipi-csis.1"),
};

static struct regulator_init_data mipi_csi_fixed_voltage_init_data = {
	.constraints = {
		.always_on = 1,
	},
	.num_consumer_supplies	= ARRAY_SIZE(mipi_csi_fixed_voltage_supplies),
	.consumer_supplies	= mipi_csi_fixed_voltage_supplies,
};

static struct fixed_voltage_config mipi_csi_fixed_voltage_config = {
	.supply_name	= "DC_5V",
	.microvolts	= 5000000,
	.gpio		= -EINVAL,
	.init_data	= &mipi_csi_fixed_voltage_init_data,
};

static struct platform_device mipi_csi_fixed_voltage = {
	.name		= "reg-fixed-voltage",
	.id		= 3,
	.dev		= {
		.platform_data	= &mipi_csi_fixed_voltage_config,
	},
};
#endif

#ifdef CONFIG_VIDEO_M5MOLS
static struct regulator_consumer_supply m5mols_fixed_voltage_supplies[] = {
	REGULATOR_SUPPLY("core", NULL),
	REGULATOR_SUPPLY("dig_18", NULL),
	REGULATOR_SUPPLY("d_sensor", NULL),
	REGULATOR_SUPPLY("dig_28", NULL),
	REGULATOR_SUPPLY("a_sensor", NULL),
	REGULATOR_SUPPLY("dig_12", NULL),
};

static struct regulator_init_data m5mols_fixed_voltage_init_data = {
	.constraints = {
		.always_on = 1,
	},
	.num_consumer_supplies	= ARRAY_SIZE(m5mols_fixed_voltage_supplies),
	.consumer_supplies	= m5mols_fixed_voltage_supplies,
};

static struct fixed_voltage_config m5mols_fixed_voltage_config = {
	.supply_name	= "CAM_SENSOR",
	.microvolts	= 1800000,
	.gpio		= -EINVAL,
	.init_data	= &m5mols_fixed_voltage_init_data,
};

static struct platform_device m5mols_fixed_voltage = {
	.name		= "reg-fixed-voltage",
	.id		= 4,
	.dev		= {
		.platform_data	= &m5mols_fixed_voltage_config,
	},
};
#endif

#ifdef CONFIG_MFD_MAX77686
static struct i2c_board_info i2c_devs0[] __initdata = {
	{
		I2C_BOARD_INFO("max77686", (0x12 >> 1)),
		.platform_data	= &exynos4_max77686_info,
	},
};
#endif

static struct i2c_board_info i2c_devs1[] __initdata = {
	{
		I2C_BOARD_INFO("wm8985", 0x1a),
	},
};

#ifdef CONFIG_JACK_MGR
#include <linux/sec_jack.h>

static struct sec_jack_zone sec_jack_zones[] = {
  {
    /* adc <= 2000, unstable zone, default to 3pole if it stays
     * in this range for a half second (20ms delays, 25 samples)
     */
    .adc_high = 2000,
    .delay_ms = 20,
    .check_count = 25,
    .jack_type = SEC_HEADSET_3POLE,
  },
  {
    /* 2000 < adc, 4 pole zone, default to 4pole if it
     * stays in this range for 200ms (20ms delays, 10 samples)
     */
    .adc_high = 0x7fffffff,
    .delay_ms = 20,
    .check_count = 10,
    .jack_type = SEC_HEADSET_4POLE,
  },
};

static struct sec_jack_buttons_zone sec_jack_buttons_zones[] = {
  {
        /* 100 < adc <= 300 - Media Key */
        .adc_high = 300,
        .adc_low = 100,
        .code = KEY_MEDIA,
  },
  {
        /* 500 < adc <= 800 - Volume Up Key */
        .adc_high = 800,
        .adc_low = 500,
        .code = KEY_VOLUMEUP,
  },
  {
        /* 1100 < adc <= 1600 - Volume Down Key */
        .adc_high = 1600,
        .adc_low = 1100,
        .code = KEY_VOLUMEDOWN,
  }
};

static struct sec_jack_buttons_zone sec_jack_buttons_zones_mp2[] = {
  {
        /* 20 < adc <= 100 - Media Key */
        .adc_high = 100,
        .adc_low = 20,
        .code = KEY_MEDIA,
  },
  {
        /* 300 < adc <= 400 - Volume Up Key */
        .adc_high = 400,
        .adc_low = 300,
        .code = KEY_VOLUMEUP,
  },
  {
        /* 800 < adc <= 900 - Volume Down Key */
        .adc_high = 900,
        .adc_low = 800,
        .code = KEY_VOLUMEDOWN,
  }
};

extern int jack_mgr_get_adc_data(void);
static int sec_jack_get_adc_value(void)
{
	return jack_mgr_get_adc_data();
}

static void sec_jack_set_micbias_state(bool on, int jack_type)
{
    //TODO: micbias control if need.
}

struct sec_jack_platform_data willow_jack_pdata = {
  .set_micbias_state = sec_jack_set_micbias_state,
  .get_adc_value = sec_jack_get_adc_value,
  .zones = sec_jack_zones,
  .num_zones = ARRAY_SIZE(sec_jack_zones),
//  .buttons_zones = sec_jack_buttons_zones,
//  .num_buttons_zones = ARRAY_SIZE(sec_jack_buttons_zones),
  .det_gpio = GPIO_JACK_DET,
  .send_end_gpio = GPIO_REMOTE_KEY_INT,
  .det_active_high = 1,
  .send_end_active_high = 0,
};

static struct platform_device willow_device_jack = {
  .name = "jack_mgr",
  .id			  = 1, /* will be used also for gpio_event id */
  .dev.platform_data	= &willow_jack_pdata,
};
#endif /* CONFIG_JACK_MGR */

static void sensor_gpio_init(void){
#if 1 
    int err;
#endif
	// Magnetic Sensor
	gpio_direction_input(EXYNOS4_GPX2(7));
	s3c_gpio_setpull(EXYNOS4_GPX2(7), S3C_GPIO_PULL_NONE);

    // GYRO_SENSOR_DRDY
    err = gpio_request(EXYNOS4_GPX3(1), "gyro_dr");
    if(err) {
        printk(KERN_ERR "failed to request GYRO_SENSOR_DRDY\n");
    }
    gpio_direction_input(EXYNOS4_GPX3(1));
    s3c_gpio_setpull(EXYNOS4_GPX3(1), S3C_GPIO_PULL_DOWN);
    gpio_free(EXYNOS4_GPX3(1));

    // GYRO_SENSOR_INT
    err = gpio_request(EXYNOS4_GPX2(3), "GYRO_INT");
    if(err) {
        printk(KERN_ERR "failed to request GYRO_SENSOR_INT\n");
    }
    gpio_direction_input(EXYNOS4_GPX2(3));
    s3c_gpio_setpull(EXYNOS4_GPX2(3), S3C_GPIO_PULL_DOWN);
    gpio_free(EXYNOS4_GPX2(3));

	// Accelerometer Sensor
#if 0
	err = gpio_request(EXYNOS4_GPX3(6), "ACC_INT");
    if(err) {
        printk(KERN_ERR "failed to request ACC_INT\n");
    }
#endif 
    gpio_direction_input(EXYNOS4_GPX3(6));
    s3c_gpio_setpull(EXYNOS4_GPX3(6), S3C_GPIO_PULL_DOWN);
    gpio_free(EXYNOS4_GPX3(6));
}

static struct i2c_board_info i2c_devs2_DVT[] __initdata = {
#ifdef CONFIG_VIDEO_TVOUT
	{
		I2C_BOARD_INFO("s5p_ddc", (0x74 >> 1)),
	},
#endif
#ifdef CONFIG_INPUT_YAS_ACCELEROMETER
    {	//DVT
        I2C_BOARD_INFO("accelerometer", 0x38),
    },
#endif
};

static struct i2c_board_info i2c_devs2_MVT[] __initdata = {
#ifdef CONFIG_VIDEO_TVOUT
	{
		I2C_BOARD_INFO("s5p_ddc", (0x74 >> 1)),
	},
#endif
};

static struct i2c_board_info i2c_devs3[] __initdata = {
	{
#if defined(CONFIG_BATTERY_MAX17040)
		I2C_BOARD_INFO("max17040", (0x6D >> 1)),
		.platform_data = &max17040_platform_data,
#endif
	},
};

static struct i2c_board_info i2c_devs4_DVT[] __initdata = {
#ifdef CONFIG_INPUT_YAS_MAGNETOMETER
	{
        I2C_BOARD_INFO("geomagnetic", 0x2e),
	},
#endif
};

static struct i2c_board_info i2c_devs4_MVT[] __initdata = {
#ifdef CONFIG_INPUT_ALPS
	{
		I2C_BOARD_INFO("accsns_i2c", 0x38),
	}
#endif
};

#if defined(CONFIG_TOUCHSCREEN_WILLOW_MXT1664S) || defined(CONFIG_TOUCHSCREEN_WILLOW_FT5X06)
int get_touch_ic_check(void)
{
	return touch_ic_check;
}
EXPORT_SYMBOL(get_touch_ic_check);

void set_touch_ic_check(int value )
{
	touch_ic_check = value;
}
EXPORT_SYMBOL(set_touch_ic_check);
#endif

#ifdef CONFIG_TOUCHSCREEN_WILLOW_MXT1664S

int touch_bootst_ctrl(int onoff)
{
	int err;
	// touch gpb4

	err = gpio_request(GPIO_TOUCH_BOOTST_EN, "tsp_bootst");
	if (err<0) {
		printk("[touch_bootst_ctrl] Error (L:%d), %s() - gpio_request(tsp_bootst) failed (err=%d)\n", __LINE__, __func__, err);
		return err;
	}else {
		gpio_direction_output(GPIO_TOUCH_BOOTST_EN, onoff);
    }		
	
	gpio_free(GPIO_TOUCH_BOOTST_EN);

	return 0;
}
EXPORT_SYMBOL(touch_bootst_ctrl);

extern int focaltech_touch_reset(int onoff);

int touch_reset(int onoff)
{
	int err;

	err = gpio_request(GPIO_TOUCH_RESET, "EXYNOS4_GPB(4)");
	if (err<0) {
		printk("[touch_reset] Error (L:%d), %s() - gpio_request(EXYNOS4_GPB(4)) failed (err=%d)\n", __LINE__, __func__, err);
		return err;
	}else {
		gpio_direction_output(GPIO_TOUCH_RESET, onoff);
    }		
	gpio_free(GPIO_TOUCH_RESET);

	return 0;
}

void touch_i2c_init(void)
{
		s3c_gpio_cfgall_range(EXYNOS4_GPB(2), 2,
			S3C_GPIO_SFN(3), S3C_GPIO_PULL_UP);
}

void touch_interrupt_init(void)
{
	int gpio;

	/* TOUCH_INT: XEINT_4 */
	gpio = GPIO_TOUCH_INT;
	gpio_request(gpio, "TOUCH_INT");
	s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(0xf));
	s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
}

void touch_interrupt_low_gpio(void)
{
    // Touch_int gpx0_4
	int gpio;

	gpio = GPIO_TOUCH_INT;
	gpio_request(gpio, "TOUCH_INT");
	s3c_gpio_cfgpin(gpio, S3C_GPIO_INPUT);
	s3c_gpio_setpull(gpio, S3C_GPIO_PULL_DOWN);
}

bool g_activepen_mode = 0;
EXPORT_SYMBOL(g_activepen_mode);

void touch_on(void)
{
	int err=0;
	struct regulator *tsp_vdd1v8 = regulator_get(NULL, "vdd_tsp_1v8");    
	struct regulator *tsp_vdd3v3 = regulator_get(NULL, "vdd_tsp");

	/* ------------ Power ON ------------
	1. Reset Low
	2. VDD 1.8   vdd_tsp_1v8 (LDO6)
	3. AVDD 2.8V  ==> TSP3.0 enable  (vdd_tsp)LDO26
	4. XVDD  GPM3_3
	5. mdelay(1)
	6. Reset High
	7. HW Reset Time 300 <
	*/
	
	ATMEL_log("[ATMEL] TS_POWER ON___________\n");
	/* touch reset pin */
	s3c_gpio_cfgpin(GPIO_TOUCH_RESET, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_TOUCH_RESET, S3C_GPIO_PULL_NONE);
	gpio_set_value(GPIO_TOUCH_RESET, 0);

	/* touch xvdd en pin */
	s3c_gpio_cfgpin(GPIO_TOUCH_BOOTST_EN, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_TOUCH_BOOTST_EN, S3C_GPIO_PULL_NONE);
	gpio_set_value(GPIO_TOUCH_BOOTST_EN, 0);

	//touch_bootst_ctrl(0); // xvdd off 
	//touch_reset(0);
	//msleep(50);

	ATMEL_log("[ATMEL] tsp_vdd1v8 ON___________\n");
	err=	regulator_enable(tsp_vdd1v8);
	if (err)
		ATMEL_log("[ATMEL] _____ tsp_vdd1v8 err ..... \n");
	regulator_put(tsp_vdd1v8);

	//err=regulator_set_voltage(tsp_vdd3v3, 3300000, 3300000);
	//if (err)
		//ATMEL_log("[ATMEL] _____ regulator_set_voltage tsp_vdd3v3 err ..... \n");

	ATMEL_log("[ATMEL] tsp_vdd3v3 ON___________\n");
	err=	regulator_enable(tsp_vdd3v3);
	if (err)
		ATMEL_log("[ATMEL] _____ tsp_vdd3v8 err ..... \n");
	regulator_put(tsp_vdd3v3);

	/* enable touch xvdd */
	if (!g_activepen_mode)
		gpio_set_value(GPIO_TOUCH_BOOTST_EN, 1);
	else
		gpio_set_value(GPIO_TOUCH_BOOTST_EN, 0);

	/* reset ic */
	mdelay(1);
	gpio_set_value(GPIO_TOUCH_RESET, 1);
	
	s3c_gpio_cfgpin(GPIO_TOUCH_INT, S3C_GPIO_SFN(0xf));
	s3c_gpio_setpull(GPIO_TOUCH_INT, S3C_GPIO_PULL_NONE);

	/* HW RESET Time  */
	msleep(300);

}

void touch_off(void)
{
	//int err=0;
	struct regulator *tsp_vdd1v8 = regulator_get(NULL, "vdd_tsp_1v8");    
	struct regulator *tsp_vdd3v3 = regulator_get(NULL, "vdd_tsp");

   /*	    ------------ Power OFF ------------
	1. AVDD 2.8V  ==> TSP3.0 enable 
	2. VDD 1.8 
	3. mdelay(10);
	4. touch interrupt pin 
	5. touch reset pin
	6. touch xvdd en pin 
	*/

	/* touch interrupt pin */
	s3c_gpio_cfgpin(GPIO_TOUCH_INT, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_TOUCH_INT, S3C_GPIO_PULL_NONE);
   
	/* touch reset pin */
	s3c_gpio_cfgpin(GPIO_TOUCH_RESET, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_TOUCH_RESET, S3C_GPIO_PULL_NONE);
	gpio_set_value(GPIO_TOUCH_RESET, 0);
   
	/* touch xvdd en pin */
	s3c_gpio_cfgpin(GPIO_TOUCH_BOOTST_EN, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_TOUCH_BOOTST_EN, S3C_GPIO_PULL_NONE);
	gpio_set_value(GPIO_TOUCH_BOOTST_EN, 0);

	mdelay(10);

	ATMEL_log("[ATMEL] TS_POWER OFF ___________\n");
	
	if (regulator_is_enabled(tsp_vdd3v3))
		regulator_force_disable(tsp_vdd3v3);
	regulator_put(tsp_vdd3v3);		

	if (regulator_is_enabled(tsp_vdd1v8))
		regulator_force_disable(tsp_vdd1v8);
	regulator_put(tsp_vdd1v8);	
}

int atmel1664_gpio_chg_get(void)
{
	return gpio_get_value(GPIO_TOUCH_INT);
}
	

static struct mxt_platform_data atmel1664_touch_platform_data = {
	.max_finger_touches = 10,
	.min_x=0,
	.max_x=1279,
	.min_y=0,
	.max_y=799,
	.min_z=0,
	.max_z=255,  //ABS_MT_TOUCH_MAJOR
	.min_w=0,
	.max_w=200,  //WIDTH_MAJOR
	.gpio_read_done = EXYNOS4_GPX0(4),	
	.power_on=touch_on,
	.power_off=touch_off,
	.boot_address	=0x4a,
};
#endif

static struct i2c_board_info i2c_devs5_DVT[] __initdata = {
#ifdef CONFIG_TOUCHSCREEN_WILLOW_FT5X06
	{
        I2C_BOARD_INFO("ft5x0x_ts", (0x70>>1)),
	},
#endif
};

static struct i2c_board_info i2c_devs5_MVT[] __initdata = {
#ifdef CONFIG_TOUCHSCREEN_WILLOW_MXT1664S
	{
		I2C_BOARD_INFO("atmel_1664", 0x4a),
		.platform_data = &atmel1664_touch_platform_data,
		.irq		= IRQ_EINT(4),
	},
#endif 
};

#ifdef CONFIG_INPUT_L3G4200D_GYR
static struct l3g4200d_gyr_platform_data l3g4200d_gyr_pdata = {
	.axis_map_x = 1,
	.axis_map_y = 0,
	.axis_map_z = 2,
	.negate_x = 0,
	.negate_y = 0,
	.negate_z = 1,
};

static struct i2c_board_info i2c_devs6[] __initdata = {
	{
        I2C_BOARD_INFO("l3g4200d_gyr", 0x68),
        .platform_data = &l3g4200d_gyr_pdata,
	},
};
#endif


#ifdef CONFIG_ISL29023
static struct i2c_gpio_platform_data i2c7_platdata = {
	.sda_pin = EXYNOS4_GPD0(2),
	.scl_pin = EXYNOS4_GPD0(3),
	.udelay = 5, //100Khz
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.scl_is_output_only = 0,
};

static struct platform_device willow_device_i2c7 = {
	.name = "i2c-gpio",
	.id = 7,
	.dev.platform_data = &i2c7_platdata,
};

static struct isl29023_i2c_platform_data isl29023_pdata = {
	.irq_gpio = GPIO_LSENSOR_INT,
};

static struct i2c_board_info i2c_devs7_DVT[] __initdata = {
	{
		I2C_BOARD_INFO("isl29023", 0x44),
		//.irq = IRQ_EINT_GROUP(19,4),
		.platform_data = &isl29023_pdata,
	},
};

static struct i2c_board_info i2c_devs7_MVT[] __initdata = {
	{
		I2C_BOARD_INFO("isl29023", 0x44),
		//.irq = IRQ_EINT_GROUP(19,4),
		.platform_data = &isl29023_pdata,
	},
#ifdef CONFIG_INPUT_ALPS
	{
		I2C_BOARD_INFO("hscd_i2c", 0x0c),
	},
#endif
};

#endif

#ifdef CONFIG_HAPTIC_ISA1200
static int isa1200_power(int vreg_on)
{
	int ret = 0;

	ret = gpio_request(GPIO_VIB_PWR_EN, "vibrator-ldo-en");
	if (ret < 0) {
		printk(KERN_ERR "[%s] gpio_request fail! ret : %d\n", __func__, ret);
		return ret;
	}

	if ( vreg_on ) {
		gpio_set_value(GPIO_VIB_PWR_EN, 1);
		mdelay(10);
	} else {
		gpio_set_value(GPIO_VIB_PWR_EN, 0);
		mdelay(10);
	}

	ret = gpio_get_value(GPIO_VIB_PWR_EN);
	gpio_free(GPIO_VIB_PWR_EN);
	printk(KERN_DEBUG "GPIO_VIB_PWR_EN: %s\n", ret ? "ON" : "OFF");

	return !ret;
}

static struct isa1200_platform_data isa1200_pdata = {
	.name = "vibrator",
	.power_on = isa1200_power,
	.pwm_ch_id = 0, /*channel id*/
	/*gpio to enable haptic*/
	.hap_en_gpio = GPIO_VIB_EN,
	.max_timeout = 15000,
};

static void isa1200_init(void)
{
	int ret = 0;

	ret = gpio_request(GPIO_VIB_PWR_EN, "vibrator-ldo-en");
	if (ret < 0) {
		printk(KERN_ERR "[%s] gpio_request fail! ret : %d\n", __func__, ret);
		return;
	}

	gpio_direction_output(GPIO_VIB_PWR_EN, 0);
	s3c_gpio_cfgpin(GPIO_VIB_PWM, (2));
	gpio_free(GPIO_VIB_PWR_EN);

	s3c_gpio_setpull(GPIO_VIB_PWM, S3C_GPIO_PULL_DOWN);
	s3c_gpio_setpull(GPIO_VIB_PWR_EN, S3C_GPIO_PULL_DOWN);

	return;
}
#endif

#ifdef CONFIG_USBHUB_USB3503
#if USB3503_I2C_CONTROL
static int (*usbhub_set_mode)(struct usb3503_hubctl *, int);
static struct usb3503_hubctl *usbhub_ctl;

static int usb3503_hub_handler(void (*set_mode)(void), void *ctl)
{
    if (!set_mode || !ctl)
        return -EINVAL;

    usbhub_set_mode = (int (*)(struct usb3503_hubctl *, int))set_mode;
    usbhub_ctl = (struct usb3503_hubctl *)ctl;

    return 0;
}
#endif

static int __init usb3503_init(void)
{
    int err;

    err = gpio_request_one(GPIO_USB_DOCK_DET, GPIOF_IN, "USB_DOCK_DET");
    if (err) {
        printk(KERN_ERR "ERR: fail to request gpio %s\n", "USB_DOCK_DET");
        return -1;
    }
    s3c_gpio_cfgpin(GPIO_USB_DOCK_DET, S3C_GPIO_SFN(0xF));
    s3c_gpio_setpull(GPIO_USB_DOCK_DET, S3C_GPIO_PULL_UP);
    gpio_free(GPIO_USB_DOCK_DET);

    err = gpio_request(GPIO_USB_HUB_INT, "USB_HUB_INT");
    if (err) {
        printk(KERN_ERR "ERR: fail to request gpio %s\n", "USB_HUB_INT");
    } else {
        gpio_direction_output(GPIO_USB_HUB_INT, 0);
        s3c_gpio_setpull(GPIO_USB_HUB_INT, S3C_GPIO_PULL_UP);
    }

    err = gpio_request(GPIO_USB_HUB_CONNECT, "USB_HUB_CONNECT");
    if (err) {
        printk(KERN_ERR "ERR: fail to request gpio %s\n", "USB_HUB_CONNECT");
    } else {
        gpio_direction_output(GPIO_USB_HUB_CONNECT, 1);
        s3c_gpio_setpull(GPIO_USB_HUB_CONNECT, S3C_GPIO_PULL_UP);
    }

    //  moved to machine_init
    //    err = gpio_request(GPIO_USB_BOOT_EN, "USB_BOOT_EN");
    //    if (err) {
    //        printk(KERN_ERR "ERR: fail to request gpio %s\n", "USB_BOOT_EN");
    //    } else {
    //        /* GPIO_USB_BOOT_EN, TBD */
    //        gpio_direction_output(GPIO_USB_BOOT_EN, 1);
    //        s3c_gpio_setpull(GPIO_USB_BOOT_EN, S3C_GPIO_PULL_NONE);
    //    }

    err = gpio_request(GPIO_USB_HUB_RST, "HUB_RST");
    if (err) {
        printk(KERN_ERR "ERR: fail to request gpio %s\n", "HUB_RST");
    } else {
        gpio_direction_output(GPIO_USB_HUB_RST, 0);
        s3c_gpio_setpull(GPIO_USB_HUB_RST, S3C_GPIO_PULL_NONE);
    }

    gpio_direction_input(GPIO_USB_HUB_INT);
    return 0;
}

static int usb3503_reset_n(int val)
{
    if (val) {
    	gpio_set_value(GPIO_USB_HUB_RST, 0);
		gpio_direction_output(GPIO_USB_BOOT_EN, 1);
		gpio_direction_output(GPIO_USB_HUB_CONNECT, 1);
		msleep(20);
		gpio_set_value(GPIO_USB_HUB_RST, !!val);
		msleep(10);
		gpio_direction_input(GPIO_USB_HUB_INT);
    }else{
    	gpio_direction_output(GPIO_USB_HUB_INT, 0);
    	gpio_set_value(GPIO_USB_HUB_RST, 0);
    }
    return 0;
}

#if USB3503_I2C_CONTROL
static int host_port_enable(int port, int enable)
{
    int err;

    if (enable) {
        err = usbhub_set_mode(usbhub_ctl, USB3503_MODE_HUB);
        if (err < 0) {
            printk(KERN_ERR "ERR: hub on fail\n");
            goto exit;
        }
        err = s5p_ehci_port_control(&s5p_device_ehci, port, 1);
        if (err < 0) {
            printk(KERN_ERR "ERR: port(%d) enable fail\n", port);
            goto exit;
        }
    } else {
        err = usbhub_set_mode(usbhub_ctl, USB3503_MODE_STANDBY);
        if (err < 0) {
            printk(KERN_ERR "ERR: hub off fail\n");
            goto exit;
        }
        err = s5p_ehci_port_control(&s5p_device_ehci, port, 0);
        if (err < 0) {
            printk(KERN_ERR "ERR: port(%d) enable fail\n", port);
            goto exit;
        }
    }

exit:
    return err;
}
#endif

static struct usb3503_platform_data usb3503_pdata = {
    .reset_n = usb3503_reset_n,
    .usb_doc_det = GPIO_USB_DOCK_DET,
#if USB3503_I2C_CONTROL
    .initial_mode = USB3503_MODE_STANDBY,
    .register_hub_handler = usb3503_hub_handler,
    .port_enable = host_port_enable,
#endif
};
#endif

#if defined(CONFIG_HAPTIC_ISA1200) || defined(CONFIG_USBHUB_USB3503)
static  struct  i2c_gpio_platform_data  i2c8_platdata = {
        .sda_pin                = EXYNOS4_GPB(6),
        .scl_pin                = EXYNOS4_GPB(7),
        .udelay                 = 5,    // 100KHz
        .sda_is_open_drain      = 0,
        .scl_is_open_drain      = 0,
        .scl_is_output_only     = 0,
};

struct platform_device s3c_device_i2c8 = {
        .name                   = "i2c-gpio",
        .id                     = 8,
        .dev.platform_data      = &i2c8_platdata,
};

static struct i2c_board_info i2c_devs8[] __initdata = {
#if defined(CONFIG_HAPTIC_ISA1200)
	{
		I2C_BOARD_INFO("isa1200", (0x91 >> 1)),
		.platform_data = &isa1200_pdata,
	},
#endif
#if defined(CONFIG_USBHUB_USB3503)
    {
        I2C_BOARD_INFO(USB3503_I2C_NAME, 0x08),
        .platform_data = &usb3503_pdata,
    },
#endif
};
#endif

#ifdef CONFIG_ANDROID_PMEM
static struct android_pmem_platform_data pmem_pdata = {
	.name		= "pmem",
	.no_allocator	= 1,
	.cached		= 0,
	.start		= 0,
	.size		= 0
};

static struct android_pmem_platform_data pmem_gpu1_pdata = {
	.name		= "pmem_gpu1",
	.no_allocator	= 1,
	.cached		= 0,
	.start		= 0,
	.size		= 0,
};

static struct platform_device pmem_device = {
	.name	= "android_pmem",
	.id	= 0,
	.dev	= {
		.platform_data = &pmem_pdata
	},
};

static struct platform_device pmem_gpu1_device = {
	.name	= "android_pmem",
	.id	= 1,
	.dev	= {
		.platform_data = &pmem_gpu1_pdata
	},
};

static void __init android_pmem_set_platdata(void)
{
#if defined(CONFIG_S5P_MEM_CMA)
	pmem_pdata.size = CONFIG_ANDROID_PMEM_MEMSIZE_PMEM * SZ_1K;
	pmem_gpu1_pdata.size = CONFIG_ANDROID_PMEM_MEMSIZE_PMEM_GPU1 * SZ_1K;
#endif
}
#endif

#ifdef CONFIG_BATTERY_SAMSUNG
static struct platform_device samsung_device_battery = {
	.name	= "samsung-fake-battery",
	.id	= -1,
};
#endif

static struct gpio_keys_button willow_button[] = {
	{
		.code               = KEY_POWER,
        .gpio               = WILLOW_POWER_KEY,
		.active_low         = 1,
       	.desc               = "gpio-keys: KEY_POWER",
		.type               = EV_KEY,
        .wakeup             = 1,
        .debounce_interval	= 1,
	},
	{
		.code               = KEY_VOLUMEDOWN,
		.gpio               = WILLOW_VOLUM_DOWN,
		.active_low         = 1,
		.desc               = "gpio-keys: KEY_VOLUMEDOWN",
		.type               = EV_KEY,
		.debounce_interval	= 1,
	},
	{
		.code               = KEY_VOLUMEUP,
		.gpio               = WILLOW_VOLUM_UP,
		.active_low         = 1,
		.desc               = "gpio-keys: KEY_VOLUMEUP" ,
		.type               = EV_KEY,
		.debounce_interval	= 1,
	},
};

static struct gpio_keys_platform_data willow_gpiokeys_platform_data = {
	.buttons		= willow_button,
	.nbuttons		= ARRAY_SIZE(willow_button),
};

static struct platform_device willow_gpio_keys = {
	.name	= "gpio-keys",
	.dev	= {
		.platform_data = &willow_gpiokeys_platform_data,
	},
};

static void willow_gpio_key_cfg(void)
{
	s3c_gpio_cfgpin(WILLOW_POWER_KEY, S3C_GPIO_SFN(0x0));
	s3c_gpio_setpull(WILLOW_POWER_KEY, S3C_GPIO_PULL_UP);
	s3c_gpio_cfgpin(WILLOW_VOLUM_DOWN, S3C_GPIO_SFN(0x0));
	s3c_gpio_setpull(WILLOW_VOLUM_DOWN, S3C_GPIO_PULL_UP);
	s3c_gpio_cfgpin(WILLOW_VOLUM_UP, S3C_GPIO_SFN(0x0));
	s3c_gpio_setpull(WILLOW_VOLUM_UP, S3C_GPIO_PULL_UP);
}

static void willow_gpio_pmint_cfg(void)
{
	s3c_gpio_cfgpin(GPIO_PMIC_IRQ, S3C_GPIO_SFN(0xF));
	s3c_gpio_setpull(GPIO_PMIC_IRQ, S3C_GPIO_PULL_UP);
}

static void willow_gpio_i2c_cfg(void)
{
	//GSENSOR
	s3c_gpio_setpull(EXYNOS4_GPA0(6), S3C_GPIO_PULL_UP);
	s3c_gpio_setpull(EXYNOS4_GPA0(7), S3C_GPIO_PULL_UP);

}

#ifdef CONFIG_WAKEUP_ASSIST
static struct platform_device wakeup_assist_device = {
	.name   = "wakeup_assist",
};
#endif

#ifdef CONFIG_VIDEO_FIMG2D
static struct fimg2d_platdata fimg2d_data __initdata = {
	.hw_ver = 0x41,
	.parent_clkname = "mout_g2d0",
	.clkname = "sclk_fimg2d",
	.gate_clkname = "fimg2d",
	.clkrate = 201 * 1000000,	/* 200 Mhz */
};
#endif

#ifdef CONFIG_USB_EXYNOS_SWITCH
static struct s5p_usbswitch_platdata willow_usbswitch_pdata;

static void __init willow_usbswitch_init(void)
{
	struct s5p_usbswitch_platdata *pdata = &willow_usbswitch_pdata;
	int err;

	pdata->gpio_host_detect = EXYNOS4_GPX1(7); /* low active */
	err = gpio_request_one(pdata->gpio_host_detect, GPIOF_IN, "HOST_DETECT");
	if (err) {
		printk(KERN_ERR "failed to request gpio_host_detect\n");
		return;
	}

	s3c_gpio_cfgpin(pdata->gpio_host_detect, S3C_GPIO_SFN(0xF));
	s3c_gpio_setpull(pdata->gpio_host_detect, S3C_GPIO_PULL_NONE);
	gpio_free(pdata->gpio_host_detect);

	pdata->gpio_device_detect = EXYNOS4_GPX2(6); /* low active */
	err = gpio_request_one(pdata->gpio_device_detect, GPIOF_IN, "DEVICE_DETECT");
	if (err) {
		printk(KERN_ERR "failed to request gpio_device_detect\n");
		return;
	}

	s3c_gpio_cfgpin(pdata->gpio_device_detect, S3C_GPIO_SFN(0xF));
	s3c_gpio_setpull(pdata->gpio_device_detect, S3C_GPIO_PULL_NONE);
	gpio_free(pdata->gpio_device_detect);

	if (1) /* Willow DVT has not gpio_host_vbus */
		pdata->gpio_host_vbus = 0;
	else {
		pdata->gpio_host_vbus = EXYNOS4_GPL2(0);
		err = gpio_request_one(pdata->gpio_host_vbus, GPIOF_OUT_INIT_LOW, "HOST_VBUS_CONTROL");
		if (err) {
			printk(KERN_ERR "failed to request gpio_host_vbus\n");
			return;
		}

		s3c_gpio_setpull(pdata->gpio_host_vbus, S3C_GPIO_PULL_NONE);
		gpio_free(pdata->gpio_host_vbus);
	}

	s5p_usbswitch_set_platdata(pdata);
}
#endif

#ifdef CONFIG_BUSFREQ_OPP
/* BUSFREQ to control memory/bus*/
static struct device_domain busfreq;
#endif

static struct platform_device exynos4_busfreq = {
	.id = -1,
	.name = "exynos-busfreq",
};

/* Bluetooth */
#ifdef CONFIG_BT_BCM4334
static struct platform_device bcm4334_bluetooth_device = {
	.name = "bcm4334_bluetooth",
	.id = -1,
};
#endif

static struct platform_device *willow_devices[] __initdata = {
#ifdef CONFIG_ANDROID_PMEM
	&pmem_device,
	&pmem_gpu1_device,
#endif
#ifdef CONFIG_SEC_WATCHDOG_RESET
	&watchdog_reset_device,
#endif
	&s3c_device_adc,
	/* Samsung Power Domain */
	&exynos4_device_pd[PD_MFC],
	&exynos4_device_pd[PD_G3D],
	&exynos4_device_pd[PD_LCD0],
	&exynos4_device_pd[PD_CAM],
	&exynos4_device_pd[PD_TV],
	&exynos4_device_pd[PD_GPS],
	&exynos4_device_pd[PD_GPS_ALIVE],
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_IS
	&exynos4_device_pd[PD_ISP],
#endif
#ifdef CONFIG_FB_MIPI_DSIM
	&s5p_device_mipi_dsim,
#endif
/* mainline fimd */
#ifdef CONFIG_FB_S3C
	&s5p_device_fimd0,
#ifdef defined(CONFIG_LCD_LTN101AL03)
	//&willow_lcd_ltn101al03,
#endif
#endif
#ifdef CONFIG_FB_S5P
	&s3c_device_fb,
#endif
	&s3c_device_wdt,
	&s3c_device_rtc,
#ifdef CONFIG_MFD_MAX77686
	&s3c_device_i2c0,
#endif
	&s3c_device_i2c1,
	&s3c_device_i2c2,
	&s3c_device_i2c3,
	&s3c_device_i2c4,
	&s3c_device_i2c5,
	&s3c_device_i2c6,
#ifdef CONFIG_ISL29023
	&willow_device_i2c7,
#endif
#if defined(CONFIG_HAPTIC_ISA1200) || defined(CONFIG_USBHUB_USB3503)
#ifdef CONFIG_HAPTIC_ISA1200
	&s3c_device_timer[0],
#endif
	&s3c_device_i2c8,
#endif
#if defined(CONFIG_VIDEO_MT9M113) || defined(CONFIG_VIDEO_AS0260) 
	&s3c_device_i2c9,
#endif	
#ifdef CONFIG_USB_EHCI_S5P
	&s5p_device_ehci,
#endif
#ifdef CONFIG_USB_OHCI_S5P
	&s5p_device_ohci,
#endif
#ifdef CONFIG_USB_GADGET
	&s3c_device_usbgadget,
#endif
#ifdef CONFIG_USB_ANDROID_RNDIS
	&s3c_device_rndis,
#endif
#ifdef CONFIG_USB_ANDROID
	&s3c_device_android_usb,
	&s3c_device_usb_mass_storage,
#endif
#ifdef CONFIG_S3C_DEV_HSMMC
	&s3c_device_hsmmc0,
#endif
#ifdef CONFIG_S3C_DEV_HSMMC1
	&s3c_device_hsmmc1,
#endif
#ifdef CONFIG_S3C_DEV_HSMMC2
	&s3c_device_hsmmc2,
#endif
#ifdef CONFIG_S3C_DEV_HSMMC3
	&s3c_device_hsmmc3,
#endif
#ifdef CONFIG_EXYNOS4_DEV_MSHC
	&s3c_device_mshci,
#endif
#ifdef CONFIG_EXYNOS4_DEV_DWMCI
	&exynos_device_dwmci,
#endif
#ifdef CONFIG_SND_SAMSUNG_AC97
	&exynos_device_ac97,
#endif
#ifdef CONFIG_SND_SAMSUNG_I2S
	&exynos_device_i2s0,
#endif
#ifdef CONFIG_SND_SAMSUNG_PCM
	&exynos_device_pcm1,
#endif
#ifdef CONFIG_SND_SAMSUNG_SPDIF
	&exynos_device_spdif,
#endif
#if defined(CONFIG_SND_SAMSUNG_RP) || defined(CONFIG_SND_SAMSUNG_ALP)
	&exynos_device_srp,
#endif
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_IS
	&exynos4_device_fimc_is,
#endif
#ifdef CONFIG_VIDEO_TVOUT
	&s5p_device_tvout,
	&s5p_device_cec,
	&s5p_device_hpd,
#endif
#ifdef CONFIG_VIDEO_EXYNOS_TV
	&s5p_device_i2c_hdmiphy,
	&s5p_device_hdmi,
	&s5p_device_sdo,
	&s5p_device_mixer,
	&s5p_device_cec,
#endif
#if defined(CONFIG_VIDEO_FIMC)
	&s3c_device_fimc0,
	&s3c_device_fimc1,
	&s3c_device_fimc2,
	&s3c_device_fimc3,
/* CONFIG_VIDEO_SAMSUNG_S5P_FIMC is the feature for mainline */
#elif defined(CONFIG_VIDEO_SAMSUNG_S5P_FIMC)
	&s5p_device_fimc0,
	&s5p_device_fimc1,
	&s5p_device_fimc2,
	&s5p_device_fimc3,
#endif
#if defined(CONFIG_VIDEO_FIMC_MIPI)
	&s3c_device_csis0,
	&s3c_device_csis1,
#elif defined(CONFIG_VIDEO_S5P_MIPI_CSIS)
	&s5p_device_mipi_csis0,
	&s5p_device_mipi_csis1,
#endif
#ifdef CONFIG_VIDEO_S5P_MIPI_CSIS
	&mipi_csi_fixed_voltage,
#endif
#ifdef CONFIG_VIDEO_M5MOLS
	&m5mols_fixed_voltage,
#endif

#if defined(CONFIG_VIDEO_MFC5X) || defined(CONFIG_VIDEO_SAMSUNG_S5P_MFC)
	&s5p_device_mfc,
#endif
#ifdef CONFIG_BATTERY_MAX17040
	&willow_charger,
#endif
#ifdef CONFIG_S5P_SYSTEM_MMU
	&SYSMMU_PLATDEV(g2d_acp),
	&SYSMMU_PLATDEV(fimc0),
	&SYSMMU_PLATDEV(fimc1),
	&SYSMMU_PLATDEV(fimc2),
	&SYSMMU_PLATDEV(fimc3),
	&SYSMMU_PLATDEV(jpeg),
	&SYSMMU_PLATDEV(mfc_l),
	&SYSMMU_PLATDEV(mfc_r),
	&SYSMMU_PLATDEV(tv),
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_IS
	&SYSMMU_PLATDEV(is_isp),
	&SYSMMU_PLATDEV(is_drc),
	&SYSMMU_PLATDEV(is_fd),
	&SYSMMU_PLATDEV(is_cpu),
#endif
#endif /* CONFIG_S5P_SYSTEM_MMU */
#ifdef CONFIG_ION_EXYNOS
	&exynos_device_ion,
#endif
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_LITE
	&exynos_device_flite0,
	&exynos_device_flite1,
#endif
#ifdef CONFIG_VIDEO_FIMG2D
	&s5p_device_fimg2d,
#endif
#ifdef CONFIG_EXYNOS_MEDIA_DEVICE
	&exynos_device_md0,
#endif
#ifdef CONFIG_VIDEO_JPEG_V2X
	&s5p_device_jpeg,
#endif
	&samsung_asoc_dma,
	&samsung_asoc_idma,
#ifdef CONFIG_BATTERY_SAMSUNG
	&samsung_device_battery,
#endif
	&samsung_device_keypad,
#ifdef CONFIG_JACK_MGR
	&willow_device_jack,
#endif
#ifdef CONFIG_WAKEUP_ASSIST
	&wakeup_assist_device,
#endif
	&willow_gpio_keys,
#ifdef CONFIG_EXYNOS_THERMAL
	&exynos_device_tmu,
#endif
#ifdef CONFIG_BT_BCM4334
	&bcm4334_bluetooth_device,
#endif
#ifdef CONFIG_S5P_DEV_ACE
	&s5p_device_ace,
#endif
	&exynos4_busfreq,
};

#ifdef CONFIG_EXYNOS_THERMAL
/* below temperature base on the celcius degree */
struct tmu_data exynos_tmu_data __initdata = {
	.ts = {
		.stop_throttle  = 82,
		.start_throttle = 85,
		.stop_warning  = 102,
		.start_warning = 105,
		.start_tripping = 110, /* temp to do tripping */
		.start_hw_tripping = 113,	/* temp to do hw_trpping*/
		.stop_mem_throttle = 80,
		.start_mem_throttle = 85,

		.stop_tc = 13,
		.start_tc = 10,
	},
	.cpulimit = {
		.throttle_freq = 800000,
		.warning_freq = 200000,
	},
	.temp_compensate = {
		.arm_volt = 925000, /* vdd_arm in uV for temperature compensation */
		.bus_volt = 900000, /* vdd_bus in uV for temperature compensation */
		.g3d_volt = 900000, /* vdd_g3d in uV for temperature compensation */
	},
	.efuse_value = 55,
	.slope = 0x10008802,
	.mode = 0,
};
#endif

#if defined(CONFIG_VIDEO_TVOUT)
static struct s5p_platform_hpd hdmi_hpd_data __initdata = {

};
static struct s5p_platform_cec hdmi_cec_data __initdata = {

};
#endif

#ifdef CONFIG_VIDEO_EXYNOS_HDMI_CEC
static struct s5p_platform_cec hdmi_cec_data __initdata = {

};
#endif


#ifdef CONFIG_VIDEO_EXYNOS_FIMC_LITE
static void __set_flite_camera_config(struct exynos_platform_flite *data,
					u32 active_index, u32 max_cam)
{
	data->active_cam_index = active_index;
	data->num_clients = max_cam;
}

static void __init smdk4x12_set_camera_flite_platdata(void)
{
	//int flite0_cam_index = 0;
	int flite1_cam_index = 0;

#ifdef CONFIG_VIDEO_AS0260
	exynos_flite1_default_data.cam[flite1_cam_index++] = &as0260;
#endif

//	__set_flite_camera_config(&exynos_flite0_default_data, 0, flite0_cam_index);
	__set_flite_camera_config(&exynos_flite1_default_data, 0, flite1_cam_index);
}
#endif

#if defined(CONFIG_CMA)
static void __init exynos4_cma_region_reserve(
			struct cma_region *regions_normal,
			struct cma_region *regions_secure)
{
	struct cma_region *reg;
	phys_addr_t paddr_last = 0xFFFFFFFF;

	for (reg = regions_normal; reg->size != 0; reg++) {
		phys_addr_t paddr;

		if (!IS_ALIGNED(reg->size, PAGE_SIZE)) {
			pr_err("S5P/CMA: size of '%s' is NOT page-aligned\n",
								reg->name);
			reg->size = PAGE_ALIGN(reg->size);
		}


		if (reg->reserved) {
			pr_err("S5P/CMA: '%s' alread reserved\n", reg->name);
			continue;
		}

		if (reg->alignment) {
			if ((reg->alignment & ~PAGE_MASK) ||
				(reg->alignment & ~reg->alignment)) {
				pr_err("S5P/CMA: Failed to reserve '%s': "
						"incorrect alignment 0x%08x.\n",
						reg->name, reg->alignment);
				continue;
			}
		} else {
			reg->alignment = PAGE_SIZE;
		}

		if (reg->start) {
			if (!memblock_is_region_reserved(reg->start, reg->size)
			    && (memblock_reserve(reg->start, reg->size) == 0))
				reg->reserved = 1;
			else
				pr_err("S5P/CMA: Failed to reserve '%s'\n",
								reg->name);

			continue;
		}

		paddr = memblock_find_in_range(0, MEMBLOCK_ALLOC_ACCESSIBLE,
						reg->size, reg->alignment);
		if (paddr != MEMBLOCK_ERROR) {
			if (memblock_reserve(paddr, reg->size)) {
				pr_err("S5P/CMA: Failed to reserve '%s'\n",
								reg->name);
				continue;
			}

			reg->start = paddr;
			reg->reserved = 1;
		} else {
			pr_err("S5P/CMA: No free space in memory for '%s'\n",
								reg->name);
		}

		if (cma_early_region_register(reg)) {
			pr_err("S5P/CMA: Failed to register '%s'\n",
								reg->name);
			memblock_free(reg->start, reg->size);
		} else {
			paddr_last = min(paddr, paddr_last);
		}
	}

	if (regions_secure && regions_secure->size) {
		size_t size_secure = 0;
		size_t align_secure, size_region2, aug_size, order_region2;

		for (reg = regions_secure; reg->size != 0; reg++)
			size_secure += reg->size;

		reg--;

		/* Entire secure regions will be merged into 2
		 * consecutive regions. */
		align_secure = 1 <<
			(get_order((size_secure + 1) / 2) + PAGE_SHIFT);
		/* Calculation of a subregion size */
		size_region2 = size_secure - align_secure;
		order_region2 = get_order(size_region2) + PAGE_SHIFT;
		if (order_region2 < 20)
			order_region2 = 20; /* 1MB */
		order_region2 -= 3; /* divide by 8 */
		size_region2 = ALIGN(size_region2, 1 << order_region2);

		aug_size = align_secure + size_region2 - size_secure;
		if (aug_size > 0)
			reg->size += aug_size;

		size_secure = ALIGN(size_secure, align_secure);

		if (paddr_last >= memblock.current_limit) {
			paddr_last = memblock_find_in_range(0,
					MEMBLOCK_ALLOC_ACCESSIBLE,
					size_secure, reg->alignment);
		} else {
			paddr_last -= size_secure;
			paddr_last = round_down(paddr_last, align_secure);
		}

		if (paddr_last) {
			while (memblock_reserve(paddr_last, size_secure))
				paddr_last -= align_secure;

			do {
				reg->start = paddr_last;
				reg->reserved = 1;
				paddr_last += reg->size;

				if (cma_early_region_register(reg)) {
					memblock_free(reg->start, reg->size);
					pr_err("S5P/CMA: "
					"Failed to register secure region "
					"'%s'\n", reg->name);
				} else {
					size_secure -= reg->size;
				}
			} while (reg-- != regions_secure);

			if (size_secure > 0)
				memblock_free(paddr_last, size_secure);
		} else {
			pr_err("S5P/CMA: Failed to reserve secure regions\n");
		}
	}
}

static void __init exynos4_reserve_mem(void)
{
	static struct cma_region regions[] = {
#ifdef CONFIG_ANDROID_PMEM_MEMSIZE_PMEM
		{
			.name = "pmem",
			.size = CONFIG_ANDROID_PMEM_MEMSIZE_PMEM * SZ_1K,
			.start = 0,
		},
#endif
#ifdef CONFIG_ANDROID_PMEM_MEMSIZE_PMEM_GPU1
		{
			.name = "pmem_gpu1",
			.size = CONFIG_ANDROID_PMEM_MEMSIZE_PMEM_GPU1 * SZ_1K,
			.start = 0,
		},
#endif
#ifndef CONFIG_VIDEOBUF2_ION
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_TV
		{
			.name = "tv",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_TV * SZ_1K,
			.start = 0
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_JPEG
		{
			.name = "jpeg",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_JPEG * SZ_1K,
			.start = 0
		},
#endif
#ifdef CONFIG_AUDIO_SAMSUNG_MEMSIZE_SRP
		{
			.name = "srp",
			.size = CONFIG_AUDIO_SAMSUNG_MEMSIZE_SRP * SZ_1K,
			.start = 0,
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMG2D
		{
			.name = "fimg2d",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMG2D * SZ_1K,
			.start = 0
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMD
		{
			.name = "fimd",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMD * SZ_1K,
			.start = 0
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC0
		{
			.name = "fimc0",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC0 * SZ_1K,
			.start = 0
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC2
		{
			.name = "fimc2",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC2 * SZ_1K,
			.start = 0
		},
#endif
#if !defined(CONFIG_EXYNOS_CONTENT_PATH_PROTECTION) && \
	defined(CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC3)
		{
			.name = "fimc3",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC3 * SZ_1K,
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC1
#ifndef CONFIG_USE_FIMC_CMA
		{
			.name = "fimc1",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC1 * SZ_1K,
			.start = 0
		},
#endif
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC_NORMAL
		{
			.name = "mfc-normal",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC_NORMAL * SZ_1K,
			{ .alignment = 1 << 17 },
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC1
		{
			.name = "mfc1",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC1 * SZ_1K,
			{ .alignment = 1 << 17 },
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC0
		{
			.name = "mfc0",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC0 * SZ_1K,
			{ .alignment = 1 << 17 },
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC
		{
			.name = "mfc",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC * SZ_1K,
			{ .alignment = 1 << 17 },
		},
#endif
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_IS
		{
			.name = "fimc_is",
			.size = CONFIG_VIDEO_EXYNOS_MEMSIZE_FIMC_IS * SZ_1K,
			{
				.alignment = 1 << 26,
			},
			.start = 0
		},
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_IS_BAYER
		{
			.name = "fimc_is_isp",
			.size = CONFIG_VIDEO_EXYNOS_MEMSIZE_FIMC_IS_ISP * SZ_1K,
			.start = 0
		},
#endif
#endif
#if !defined(CONFIG_EXYNOS_CONTENT_PATH_PROTECTION) && \
	defined(CONFIG_VIDEO_SAMSUNG_S5P_MFC)
		{
			.name		= "b2",
			.size		= 32 << 20,
			{ .alignment	= 128 << 10 },
		},
		{
			.name		= "b1",
			.size		= 32 << 20,
			{ .alignment	= 128 << 10 },
		},
		{
			.name		= "fw",
			.size		= 1 << 20,
			{ .alignment	= 128 << 10 },
		},
#endif
#else /* !CONFIG_VIDEOBUF2_ION */
#ifdef CONFIG_FB_S5P
#error CONFIG_FB_S5P is defined. Select CONFIG_FB_S3C, instead
#endif
		{
			.name	= "ion",
			.size	= CONFIG_ION_EXYNOS_CONTIGHEAP_SIZE * SZ_1K,
		},
#endif /* !CONFIG_VIDEOBUF2_ION */
		{
			.size = 0
		},
	};
#ifdef CONFIG_EXYNOS_CONTENT_PATH_PROTECTION
	static struct cma_region regions_secure[] = {
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC3
		{
			.name = "fimc3",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC3 * SZ_1K,
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMD_VIDEO
		{
			.name = "video",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMD_VIDEO * SZ_1K,
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC_SECURE
		{
			.name = "mfc-secure",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC_SECURE * SZ_1K,
			.start = 0x5C100000,
		},
#endif
		{
			.name = "sectbl",
			.size = SZ_1M,
			.start = 0x5C000000,
		},
		{
			.size = 0
		},
	};
#else /* !CONFIG_EXYNOS_CONTENT_PATH_PROTECTION */
	struct cma_region *regions_secure = NULL;
#endif
	static const char map[] __initconst =
		"android_pmem.0=pmem;android_pmem.1=pmem_gpu1;"
		"s3cfb.0/fimd=fimd;exynos4-fb.0/fimd=fimd;"
#ifdef CONFIG_EXYNOS_CONTENT_PATH_PROTECTION
		"s3cfb.0/video=video;exynos4-fb.0/video=video;"
#endif
		"s3c-fimc.0=fimc0;s3c-fimc.1=fimc1;s3c-fimc.2=fimc2;s3c-fimc.3=fimc3;"
		"exynos4210-fimc.0=fimc0;exynos4210-fimc.1=fimc1;exynos4210-fimc.2=fimc2;exynos4210-fimc.3=fimc3;"
#ifdef CONFIG_VIDEO_MFC5X
		"s3c-mfc/A=mfc0,mfc-secure;"
		"s3c-mfc/B=mfc1,mfc-normal;"
		"s3c-mfc/AB=mfc;"
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_S5P_MFC
		"s5p-mfc/f=fw;"
		"s5p-mfc/a=b1;"
		"s5p-mfc/b=b2;"
#endif
		"samsung-rp=srp;"
		"s5p-jpeg=jpeg;"
		"exynos4-fimc-is/f=fimc_is;"
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_IS_BAYER
		"exynos4-fimc-is/i=fimc_is_isp;"
#endif
		"s5p-mixer=tv;"
		"s5p-fimg2d=fimg2d;"
		"ion-exynos=ion,fimd,fimc0,fimc1,fimc2,fimc3,fw,b1,b2;"
#ifdef CONFIG_EXYNOS_CONTENT_PATH_PROTECTION
		"s5p-smem/video=video;"
		"s5p-smem/sectbl=sectbl;"
#endif
		"s5p-smem/mfc=mfc0,mfc-secure;"
		"s5p-smem/fimc=fimc3;"
		"s5p-smem/mfc-shm=mfc1,mfc-normal;";

	cma_set_defaults(NULL, map);

	exynos4_cma_region_reserve(regions, regions_secure);
}
#else
static inline void exynos4_reserve_mem(void)
{
}
#endif /* CONFIG_CMA */

#ifdef CONFIG_BACKLIGHT_PWM
/* LCD Backlight data */
static struct samsung_bl_gpio_info willow_bl_gpio_info = {
	.no = BACKLIGHT_PWM_GPIO,
	.func = S3C_GPIO_SFN(2),
};

static struct platform_pwm_backlight_data willow_bl_data = {
	.pwm_id = 1,
	.max_brightness = 255,
	.dft_brightness = 255,
	.lth_brightness = 17,
	.pwm_period_ns  = 62500,
};
#endif

static void __init willow_map_io(void)
{
	clk_xusbxti.rate = 24000000;
	s5p_init_io(NULL, 0, S5P_VA_CHIPID);
	s3c24xx_init_clocks(24000000);
	s3c24xx_init_uarts(willow_uartcfgs, ARRAY_SIZE(willow_uartcfgs));

	exynos4_reserve_mem();
    sensor_gpio_init();
}

static void __init exynos_sysmmu_init(void)
{
	ASSIGN_SYSMMU_POWERDOMAIN(fimc0, &exynos4_device_pd[PD_CAM].dev);
	ASSIGN_SYSMMU_POWERDOMAIN(fimc1, &exynos4_device_pd[PD_CAM].dev);
	ASSIGN_SYSMMU_POWERDOMAIN(fimc2, &exynos4_device_pd[PD_CAM].dev);
	ASSIGN_SYSMMU_POWERDOMAIN(fimc3, &exynos4_device_pd[PD_CAM].dev);
	ASSIGN_SYSMMU_POWERDOMAIN(jpeg, &exynos4_device_pd[PD_CAM].dev);
	ASSIGN_SYSMMU_POWERDOMAIN(mfc_l, &exynos4_device_pd[PD_MFC].dev);
	ASSIGN_SYSMMU_POWERDOMAIN(mfc_r, &exynos4_device_pd[PD_MFC].dev);
	ASSIGN_SYSMMU_POWERDOMAIN(tv, &exynos4_device_pd[PD_TV].dev);
#ifdef CONFIG_VIDEO_FIMG2D
	sysmmu_set_owner(&SYSMMU_PLATDEV(g2d_acp).dev, &s5p_device_fimg2d.dev);
#endif
#if defined(CONFIG_VIDEO_SAMSUNG_S5P_MFC) || defined(CONFIG_VIDEO_MFC5X)
	sysmmu_set_owner(&SYSMMU_PLATDEV(mfc_l).dev, &s5p_device_mfc.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(mfc_r).dev, &s5p_device_mfc.dev);
#endif
#if defined(CONFIG_VIDEO_FIMC)
	sysmmu_set_owner(&SYSMMU_PLATDEV(fimc0).dev, &s3c_device_fimc0.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(fimc1).dev, &s3c_device_fimc1.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(fimc2).dev, &s3c_device_fimc2.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(fimc3).dev, &s3c_device_fimc3.dev);
#elif defined(CONFIG_VIDEO_SAMSUNG_S5P_FIMC)
	sysmmu_set_owner(&SYSMMU_PLATDEV(fimc0).dev, &s5p_device_fimc0.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(fimc1).dev, &s5p_device_fimc1.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(fimc2).dev, &s5p_device_fimc2.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(fimc3).dev, &s5p_device_fimc3.dev);
#endif
#ifdef CONFIG_VIDEO_EXYNOS_TV
	sysmmu_set_owner(&SYSMMU_PLATDEV(tv).dev, &s5p_device_mixer.dev);
#endif
#ifdef CONFIG_VIDEO_TVOUT
	sysmmu_set_owner(&SYSMMU_PLATDEV(tv).dev, &s5p_device_tvout.dev);
#endif
#ifdef CONFIG_VIDEO_JPEG_V2X
	sysmmu_set_owner(&SYSMMU_PLATDEV(jpeg).dev, &s5p_device_jpeg.dev);
#endif
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_IS
	ASSIGN_SYSMMU_POWERDOMAIN(is_isp, &exynos4_device_pd[PD_ISP].dev);
	ASSIGN_SYSMMU_POWERDOMAIN(is_drc, &exynos4_device_pd[PD_ISP].dev);
	ASSIGN_SYSMMU_POWERDOMAIN(is_fd, &exynos4_device_pd[PD_ISP].dev);
	ASSIGN_SYSMMU_POWERDOMAIN(is_cpu, &exynos4_device_pd[PD_ISP].dev);

	sysmmu_set_owner(&SYSMMU_PLATDEV(is_isp).dev,
						&exynos4_device_fimc_is.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(is_drc).dev,
						&exynos4_device_fimc_is.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(is_fd).dev,
						&exynos4_device_fimc_is.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(is_cpu).dev,
						&exynos4_device_fimc_is.dev);
#endif
}

static void __init willow_machine_init(void)
{
	arm_pm_restart = willow_pm_restart;
	pm_power_off = willow_power_off;

	/* IP4791CZ12 Enable */
	gpio_request(GPIO_HDMI_PWR_EN, "HDMI_PWR_EN");
	gpio_direction_output(GPIO_HDMI_PWR_EN, 1);
	s3c_gpio_setpull(GPIO_HDMI_PWR_EN, S3C_GPIO_PULL_NONE);

	/* VDD_5V0_EN Enable */
	gpio_request(GPIO_USB_BOOT_EN, "USB_BOOT_EN");
	gpio_direction_output(GPIO_USB_BOOT_EN, 1);
	s3c_gpio_setpull(GPIO_USB_BOOT_EN, S3C_GPIO_PULL_NONE);

	willow_config_gpio_table();
	willow_check_hw_version();
#ifdef CONFIG_BATTERY_MAX17040
	max8903_gpio_init();
#endif

#if defined(CONFIG_EXYNOS_DEV_PD) && defined(CONFIG_PM_RUNTIME)
	exynos_pd_disable(&exynos4_device_pd[PD_MFC].dev);
	exynos_pd_disable(&exynos4_device_pd[PD_G3D].dev);
	//exynos_pd_disable(&exynos4_device_pd[PD_LCD0].dev);
	exynos_pd_disable(&exynos4_device_pd[PD_CAM].dev);
	exynos_pd_disable(&exynos4_device_pd[PD_TV].dev);
	exynos_pd_disable(&exynos4_device_pd[PD_GPS].dev);
	exynos_pd_disable(&exynos4_device_pd[PD_GPS_ALIVE].dev);
	exynos_pd_disable(&exynos4_device_pd[PD_ISP].dev);
#elif defined(CONFIG_EXYNOS_DEV_PD)
	/*
	 * These power domains should be always on
	 * without runtime pm support.
	 */
	exynos_pd_enable(&exynos4_device_pd[PD_MFC].dev);
	exynos_pd_enable(&exynos4_device_pd[PD_G3D].dev);
	exynos_pd_enable(&exynos4_device_pd[PD_LCD0].dev);
	exynos_pd_enable(&exynos4_device_pd[PD_CAM].dev);
	exynos_pd_enable(&exynos4_device_pd[PD_TV].dev);
	exynos_pd_enable(&exynos4_device_pd[PD_GPS].dev);
	exynos_pd_enable(&exynos4_device_pd[PD_GPS_ALIVE].dev);
	exynos_pd_enable(&exynos4_device_pd[PD_ISP].dev);
#endif

#ifdef CONFIG_MFD_MAX77686
	s3c_i2c0_set_platdata(NULL);
	i2c_register_board_info(0, i2c_devs0, ARRAY_SIZE(i2c_devs0));
#endif
	s3c_i2c1_set_platdata(NULL);
	i2c_register_board_info(1, i2c_devs1, ARRAY_SIZE(i2c_devs1));

	s3c_i2c2_set_platdata(NULL);
	if(willow_get_hw_version() == WILLOW_HW_DVT)
		i2c_register_board_info(2, i2c_devs2_DVT, ARRAY_SIZE(i2c_devs2_DVT));
	else
		i2c_register_board_info(2, i2c_devs2_MVT, ARRAY_SIZE(i2c_devs2_MVT));

	s3c_i2c3_set_platdata(NULL);
	i2c_register_board_info(3, i2c_devs3, ARRAY_SIZE(i2c_devs3));

	s3c_i2c4_set_platdata(NULL);
	if(willow_get_hw_version() == WILLOW_HW_DVT)
		i2c_register_board_info(4, i2c_devs4_DVT, ARRAY_SIZE(i2c_devs4_DVT));
	else
		i2c_register_board_info(4, i2c_devs4_MVT, ARRAY_SIZE(i2c_devs4_MVT));

	s3c_i2c5_set_platdata(NULL);
	if(willow_get_hw_version() == WILLOW_HW_DVT)
		i2c_register_board_info(5, i2c_devs5_DVT, ARRAY_SIZE(i2c_devs5_DVT));
	else
		i2c_register_board_info(5, i2c_devs5_MVT, ARRAY_SIZE(i2c_devs5_MVT));

	s3c_i2c6_set_platdata(NULL);
	i2c_register_board_info(6, i2c_devs6, ARRAY_SIZE(i2c_devs6));

	if(willow_get_hw_version() == WILLOW_HW_DVT)
		i2c_register_board_info(7, i2c_devs7_DVT, ARRAY_SIZE(i2c_devs7_DVT));
	else
		i2c_register_board_info(7, i2c_devs7_MVT, ARRAY_SIZE(i2c_devs7_MVT));

#if defined(CONFIG_HAPTIC_ISA1200) || defined(CONFIG_USBHUB_USB3503)
#ifdef CONFIG_HAPTIC_ISA1200
	isa1200_init();
#endif
#ifdef CONFIG_USBHUB_USB3503
	if(willow_get_hw_version() >= WILLOW_HW_MVT)
		usb3503_init();
#endif
	i2c_register_board_info(8, i2c_devs8, ARRAY_SIZE(i2c_devs8));
#endif

#ifdef CONFIG_VIDEO_MT9M113
	mt9m113_gpio_init();
#elif defined(CONFIG_VIDEO_AS0260)
	as0260_i2c_gpio_init();
#endif

	//i2c_register_board_info(9, i2c_devs9,ARRAY_SIZE(i2c_devs9));

#if defined(CONFIG_FB_S5P_MIPI_DSIM)
	mipi_fb_init();
#endif
#ifdef CONFIG_ANDROID_PMEM
	android_pmem_set_platdata();
#endif
#ifdef CONFIG_FB_S3C
	dev_set_name(&s5p_device_fimd0.dev, "s3cfb.0");
	clk_add_alias("lcd", "exynos4-fb.0", "lcd", &s5p_device_fimd0.dev);
	clk_add_alias("sclk_fimd", "exynos4-fb.0", "sclk_fimd", &s5p_device_fimd0.dev);
	s5p_fb_setname(0, "exynos4-fb");
#if defined(CONFIG_LCD_AMS369FG06) || defined(CONFIG_LCD_LMS501KF03)
	spi_register_board_info(spi_board_info, ARRAY_SIZE(spi_board_info));
#endif
	s5p_fimd0_set_platdata(&smdk4x12_lcd0_pdata);
#ifdef CONFIG_FB_MIPI_DSIM
	s5p_device_mipi_dsim.dev.parent = &exynos4_device_pd[PD_LCD0].dev;
#endif
#ifdef CONFIG_EXYNOS_DEV_PD
	s5p_device_fimd0.dev.parent = &exynos4_device_pd[PD_LCD0].dev;
#endif
#endif
#ifdef CONFIG_FB_S5P
#ifdef CONFIG_FB_S5P_LMS501KF03
	spi_register_board_info(spi_board_info, ARRAY_SIZE(spi_board_info));
	s3cfb_set_platdata(&lms501kf03_data);
#else
	s3cfb_set_platdata(NULL);
#endif
#ifdef CONFIG_FB_S5P_MIPI_DSIM
	s5p_device_dsim.dev.parent = &exynos4_device_pd[PD_LCD0].dev;
#endif
#ifdef CONFIG_FB_S5P_LTN101AL03
	s3cfb_set_platdata(&ltn101al03_fb_data);
#endif
#ifdef CONFIG_EXYNOS_DEV_PD
	s3c_device_fb.dev.parent = &exynos4_device_pd[PD_LCD0].dev;
#endif
#endif
#ifdef CONFIG_USB_EHCI_S5P
	willow_ehci_init();
#endif
#ifdef CONFIG_USB_OHCI_S5P
	willow_ohci_init();
#endif
#ifdef CONFIG_USB_GADGET
	willow_usbgadget_init();
#endif
#ifdef CONFIG_USB_EXYNOS_SWITCH
	willow_usbswitch_init();
#endif
#ifdef CONFIG_ISL29023
	s3c_gpio_setpull(GPIO_LSENSOR_INT, S3C_GPIO_PULL_UP);
#endif
#ifdef CONFIG_JACK_MGR
	struct sec_jack_platform_data *jack_mgr_pdata = willow_device_jack.dev.platform_data;
	int is_jack_mgr_mp2 = 0;
	if(willow_get_hw_version() > WILLOW_HW_MVT){
		s3c_gpio_setpull(GPIO_REMOTE_KEY_INT, S3C_GPIO_PULL_DOWN);
		if(willow_get_hw_version() > WILLOW_HW_MP)
			is_jack_mgr_mp2 = 1;
	}else{
		s3c_gpio_setpull(GPIO_REMOTE_KEY_INT, S3C_GPIO_PULL_UP);
	}
	if(is_jack_mgr_mp2){
		jack_mgr_pdata->buttons_zones = sec_jack_buttons_zones_mp2;
		jack_mgr_pdata->num_buttons_zones = ARRAY_SIZE(sec_jack_buttons_zones_mp2);
	}else{
		jack_mgr_pdata->buttons_zones = sec_jack_buttons_zones;
		jack_mgr_pdata->num_buttons_zones = ARRAY_SIZE(sec_jack_buttons_zones);
	}
#endif

	samsung_bl_set(&willow_bl_gpio_info, &willow_bl_data);

#ifdef CONFIG_EXYNOS4_DEV_DWMCI
	exynos_dwmci_set_platdata(&exynos_dwmci_pdata);
#endif

#ifdef CONFIG_VIDEO_EXYNOS_FIMC_IS
	exynos4_fimc_is_set_platdata(NULL);
#ifdef CONFIG_EXYNOS_DEV_PD
	exynos4_device_fimc_is.dev.parent = &exynos4_device_pd[PD_ISP].dev;
#endif
#endif
#ifdef CONFIG_S3C_DEV_HSMMC
	s3c_sdhci0_set_platdata(&willow_hsmmc0_pdata);
#endif
#ifdef CONFIG_S3C_DEV_HSMMC1
	s3c_sdhci1_set_platdata(&willow_hsmmc1_pdata);
#endif
#ifdef CONFIG_S3C_DEV_HSMMC2
	if(willow_get_hw_version() >= WILLOW_HW_MP)
		s3c_sdhci2_set_platdata(&willow_hsmmc2_pdata_MP);
	else
		s3c_sdhci2_set_platdata(&willow_hsmmc2_pdata);
#endif
#ifdef CONFIG_S3C_DEV_HSMMC3
	s3c_sdhci3_set_platdata(&willow_hsmmc3_pdata);
#endif
#ifdef CONFIG_EXYNOS4_DEV_MSHC
	s3c_mshci_set_platdata(&exynos4_mshc_pdata);
#endif
#if defined(CONFIG_VIDEO_EXYNOS_TV) && defined(CONFIG_VIDEO_EXYNOS_HDMI)
	dev_set_name(&s5p_device_hdmi.dev, "exynos4-hdmi");
	clk_add_alias("hdmi", "s5p-hdmi", "hdmi", &s5p_device_hdmi.dev);
	clk_add_alias("hdmiphy", "s5p-hdmi", "hdmiphy", &s5p_device_hdmi.dev);

	s5p_tv_setup();

	/* setup dependencies between TV devices */
	s5p_device_hdmi.dev.parent = &exynos4_device_pd[PD_TV].dev;
	s5p_device_mixer.dev.parent = &exynos4_device_pd[PD_TV].dev;

	s5p_i2c_hdmiphy_set_platdata(NULL);
#ifdef CONFIG_VIDEO_EXYNOS_HDMI_CEC
	s5p_hdmi_cec_set_platdata(&hdmi_cec_data);
#endif
#endif
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_LITE
	smdk4x12_set_camera_flite_platdata();
	s3c_set_platdata(&exynos_flite0_default_data,
			sizeof(exynos_flite0_default_data), &exynos_device_flite0);
	s3c_set_platdata(&exynos_flite1_default_data,
			sizeof(exynos_flite1_default_data), &exynos_device_flite1);
#ifdef CONFIG_EXYNOS_DEV_PD
	exynos_device_flite0.dev.parent = &exynos4_device_pd[PD_ISP].dev;
	exynos_device_flite1.dev.parent = &exynos4_device_pd[PD_ISP].dev;
#endif
#endif
#ifdef CONFIG_EXYNOS_THERMAL
	exynos_tmu_set_platdata(&exynos_tmu_data);
#endif
#ifdef CONFIG_VIDEO_FIMC
	s3c_fimc0_set_platdata(&fimc_plat);
	s3c_fimc1_set_platdata(&fimc_plat);
	s3c_fimc2_set_platdata(&fimc_plat);
	s3c_fimc3_set_platdata(NULL);
#ifdef CONFIG_EXYNOS_DEV_PD
	s3c_device_fimc0.dev.parent = &exynos4_device_pd[PD_CAM].dev;
	s3c_device_fimc1.dev.parent = &exynos4_device_pd[PD_CAM].dev;
	s3c_device_fimc2.dev.parent = &exynos4_device_pd[PD_CAM].dev;
	s3c_device_fimc3.dev.parent = &exynos4_device_pd[PD_CAM].dev;
#ifdef CONFIG_EXYNOS_CONTENT_PATH_PROTECTION
	secmem.parent = &exynos4_device_pd[PD_CAM].dev;
#endif
#endif
#ifdef CONFIG_VIDEO_FIMC_MIPI
	s3c_csis0_set_platdata(NULL);
	s3c_csis1_set_platdata(NULL);
#ifdef CONFIG_EXYNOS_DEV_PD
	s3c_device_csis0.dev.parent = &exynos4_device_pd[PD_CAM].dev;
	s3c_device_csis1.dev.parent = &exynos4_device_pd[PD_CAM].dev;
#endif
#endif

#if defined(CONFIG_ITU_A) || defined(CONFIG_CSI_C) \
	|| defined(CONFIG_S5K3H1_CSI_C) || defined(CONFIG_S5K3H2_CSI_C) \
	|| defined(CONFIG_S5K6A3_CSI_C)
	smdk4x12_cam0_reset(1);
#endif
#if defined(CONFIG_ITU_B) || defined(CONFIG_CSI_D) \
	|| defined(CONFIG_S5K3H1_CSI_D) || defined(CONFIG_S5K3H2_CSI_D) \
	|| defined(CONFIG_S5K6A3_CSI_D) || defined(CONFIG_VIDEO_AS0260)
	as0260_camera_config();
	//as0260_power_ctrl(0);
	//smdk4x12_cam1_reset(1);
	as0260_camera_reset_ctrl(0);
#endif
#endif /* CONFIG_VIDEO_FIMC */

#ifdef CONFIG_VIDEO_SAMSUNG_S5P_FIMC
	smdk4x12_camera_config();
	smdk4x12_subdev_config();

	dev_set_name(&s5p_device_fimc0.dev, "s3c-fimc.0");
	dev_set_name(&s5p_device_fimc1.dev, "s3c-fimc.1");
	dev_set_name(&s5p_device_fimc2.dev, "s3c-fimc.2");
	dev_set_name(&s5p_device_fimc3.dev, "s3c-fimc.3");

	clk_add_alias("fimc", "exynos4210-fimc.0", "fimc", &s5p_device_fimc0.dev);
	clk_add_alias("sclk_fimc", "exynos4210-fimc.0", "sclk_fimc",
			&s5p_device_fimc0.dev);
	clk_add_alias("fimc", "exynos4210-fimc.1", "fimc", &s5p_device_fimc1.dev);
	clk_add_alias("sclk_fimc", "exynos4210-fimc.1", "sclk_fimc",
			&s5p_device_fimc1.dev);
	clk_add_alias("fimc", "exynos4210-fimc.2", "fimc", &s5p_device_fimc2.dev);
	clk_add_alias("sclk_fimc", "exynos4210-fimc.2", "sclk_fimc",
			&s5p_device_fimc2.dev);
	clk_add_alias("fimc", "exynos4210-fimc.3", "fimc", &s5p_device_fimc3.dev);
	clk_add_alias("sclk_fimc", "exynos4210-fimc.3", "sclk_fimc",
			&s5p_device_fimc3.dev);

	s3c_fimc_setname(0, "exynos4210-fimc");
	s3c_fimc_setname(1, "exynos4210-fimc");
	s3c_fimc_setname(2, "exynos4210-fimc");
	s3c_fimc_setname(3, "exynos4210-fimc");
	/* FIMC */
	s3c_set_platdata(&s3c_fimc0_default_data,
			 sizeof(s3c_fimc0_default_data), &s5p_device_fimc0);
	s3c_set_platdata(&s3c_fimc1_default_data,
			 sizeof(s3c_fimc1_default_data), &s5p_device_fimc1);
	s3c_set_platdata(&s3c_fimc2_default_data,
			 sizeof(s3c_fimc2_default_data), &s5p_device_fimc2);
	s3c_set_platdata(&s3c_fimc3_default_data,
			 sizeof(s3c_fimc3_default_data), &s5p_device_fimc3);
#ifdef CONFIG_EXYNOS_DEV_PD
	s5p_device_fimc0.dev.parent = &exynos4_device_pd[PD_CAM].dev;
	s5p_device_fimc1.dev.parent = &exynos4_device_pd[PD_CAM].dev;
	s5p_device_fimc2.dev.parent = &exynos4_device_pd[PD_CAM].dev;
	s5p_device_fimc3.dev.parent = &exynos4_device_pd[PD_CAM].dev;
#endif
#ifdef CONFIG_VIDEO_S5P_MIPI_CSIS
	dev_set_name(&s5p_device_mipi_csis0.dev, "s3c-csis.0");
	dev_set_name(&s5p_device_mipi_csis1.dev, "s3c-csis.1");
	clk_add_alias("csis", "s5p-mipi-csis.0", "csis",
			&s5p_device_mipi_csis0.dev);
	clk_add_alias("sclk_csis", "s5p-mipi-csis.0", "sclk_csis",
			&s5p_device_mipi_csis0.dev);
	clk_add_alias("csis", "s5p-mipi-csis.1", "csis",
			&s5p_device_mipi_csis1.dev);
	clk_add_alias("sclk_csis", "s5p-mipi-csis.1", "sclk_csis",
			&s5p_device_mipi_csis1.dev);
	dev_set_name(&s5p_device_mipi_csis0.dev, "s5p-mipi-csis.0");
	dev_set_name(&s5p_device_mipi_csis1.dev, "s5p-mipi-csis.1");

	s3c_set_platdata(&s5p_mipi_csis0_default_data,
			sizeof(s5p_mipi_csis0_default_data), &s5p_device_mipi_csis0);
	s3c_set_platdata(&s5p_mipi_csis1_default_data,
			sizeof(s5p_mipi_csis1_default_data), &s5p_device_mipi_csis1);
#ifdef CONFIG_EXYNOS_DEV_PD
	s5p_device_mipi_csis0.dev.parent = &exynos4_device_pd[PD_CAM].dev;
	s5p_device_mipi_csis1.dev.parent = &exynos4_device_pd[PD_CAM].dev;
#endif
#endif
#if defined(CONFIG_ITU_A) || defined(CONFIG_CSI_C) \
	|| defined(CONFIG_S5K3H1_CSI_C) || defined(CONFIG_S5K3H2_CSI_C) \
	|| defined(CONFIG_S5K6A3_CSI_C)
	smdk4x12_cam0_reset(1);
#endif
#if defined(CONFIG_ITU_B) || defined(CONFIG_CSI_D) \
	|| defined(CONFIG_S5K3H1_CSI_D) || defined(CONFIG_S5K3H2_CSI_D) \
	|| defined(CONFIG_S5K6A3_CSI_D) || defined(CONFIG_VIDEO_AS0260)
	//smdk4x12_cam0_reset(1);
	//smdk4x12_camera_config();
	//as0260_power_ctrl(0);
#endif
#endif

#if defined(CONFIG_VIDEO_TVOUT)
	s5p_hdmi_hpd_set_platdata(&hdmi_hpd_data);
	s5p_hdmi_cec_set_platdata(&hdmi_cec_data);
#ifdef CONFIG_EXYNOS_DEV_PD
	s5p_device_tvout.dev.parent = &exynos4_device_pd[PD_TV].dev;
	exynos4_device_pd[PD_TV].dev.parent = &exynos4_device_pd[PD_LCD0].dev;
#endif
#endif

#ifdef CONFIG_VIDEO_JPEG_V2X
#ifdef CONFIG_EXYNOS_DEV_PD
	s5p_device_jpeg.dev.parent = &exynos4_device_pd[PD_CAM].dev;
	exynos4_jpeg_setup_clock(&s5p_device_jpeg.dev, 160000000);
#endif
#endif

#ifdef CONFIG_ION_EXYNOS
	exynos_ion_set_platdata();
#endif

#if defined(CONFIG_VIDEO_MFC5X) || defined(CONFIG_VIDEO_SAMSUNG_S5P_MFC)
#ifdef CONFIG_EXYNOS_DEV_PD
	s5p_device_mfc.dev.parent = &exynos4_device_pd[PD_MFC].dev;
#endif
	if (soc_is_exynos4412() && samsung_rev() >= EXYNOS4412_REV_1_0)
		exynos4_mfc_setup_clock(&s5p_device_mfc.dev, 200 * MHZ);
	else
		exynos4_mfc_setup_clock(&s5p_device_mfc.dev, 267 * MHZ);
#endif

#if defined(CONFIG_VIDEO_SAMSUNG_S5P_MFC)
	dev_set_name(&s5p_device_mfc.dev, "s3c-mfc");
	clk_add_alias("mfc", "s5p-mfc", "mfc", &s5p_device_mfc.dev);
	s5p_mfc_setname(&s5p_device_mfc, "s5p-mfc");
#endif

#ifdef CONFIG_VIDEO_FIMG2D
	s5p_fimg2d_set_platdata(&fimg2d_data);
#endif

	brcm_wlan_init();

	exynos_sysmmu_init();

	platform_add_devices(willow_devices, ARRAY_SIZE(willow_devices));

#ifdef CONFIG_FB_S3C
	exynos4_fimd0_setup_clock(&s5p_device_fimd0.dev, "mout_mpll_user",
				800 * MHZ);
#endif
#ifdef CONFIG_BUSFREQ_OPP
	dev_add(&busfreq, &exynos4_busfreq.dev);
	ppmu_init(&exynos_ppmu[PPMU_DMC0], &exynos4_busfreq.dev);
	ppmu_init(&exynos_ppmu[PPMU_DMC1], &exynos4_busfreq.dev);
	ppmu_init(&exynos_ppmu[PPMU_CPU], &exynos4_busfreq.dev);
#endif

	willow_gpio_key_cfg();
	willow_gpio_pmint_cfg();
	willow_gpio_i2c_cfg();

	register_reboot_notifier(&exynos4_reboot_notifier);
}

MACHINE_START(WILLOW, "WILLOW")
	/* Maintainer: KooHyun Yu <koohyun@thinkware.co.kr>,<koohyun.yu@gmail.com> */
	.atag_offset	= S5P_PA_SDRAM + 0x100,
	.init_irq	= exynos4_init_irq,
	.map_io		= willow_map_io,
	.handle_irq	= gic_handle_irq,
	.init_machine	= willow_machine_init,
	.timer		= &exynos4_timer,
MACHINE_END
