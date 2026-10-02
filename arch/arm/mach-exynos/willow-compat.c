/*
 * willow-compat.c - glue for General Mobile E-Tab 4 (willow) vendor drivers
 * ported from the Thinkware 3.0.40 kernel to the LineageOS smdk4412 3.4 kernel.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <mach/map.h>
#include <plat/regs-iic.h>
#include <plat/iic.h>
#include <plat/devs.h>

/* --- touch: i2c5 bus speed switch used by FT5606 / mXT1664S drivers --- */
static struct s3c2410_platform_i2c touch_100k_i2c_data = {
	.flags		= 0,
	.slave_addr	= 0x10,
	.frequency	= 95*1000,
	.sda_delay	= 100,
};

static struct s3c2410_platform_i2c touch_300k_i2c_data = {
	.flags		= 0,
	.slave_addr	= 0x10,
	.frequency	= 200*1000,
	.sda_delay	= 100,
};


void touch_s3c_i2c5_set_platdata(struct s3c2410_platform_i2c *pd, int check_value)
{
	struct s3c2410_platform_i2c *npd;

	pd = check_value ? &touch_300k_i2c_data : &touch_100k_i2c_data;
	pd->bus_num = 5;
	npd = s3c_set_platdata(pd, sizeof(struct s3c2410_platform_i2c),
			       &s3c_device_i2c5);
	if (npd)
		npd->cfg_gpio = s3c_i2c5_cfg_gpio;
}
EXPORT_SYMBOL(touch_s3c_i2c5_set_platdata);

void s3c_i2c5_force_stop(void)
{
	void __iomem *regs;
	struct clk *clk;
	unsigned long iicstat;

	regs = ioremap(S3C_PA_IIC5, SZ_4K);
	if (!regs)
		return;
	clk = clk_get(&s3c_device_i2c5.dev, "i2c");
	if (IS_ERR_OR_NULL(clk)) {
		iounmap(regs);
		return;
	}
	clk_enable(clk);
	iicstat = readl(regs + S3C2410_IICSTAT);
	writel(iicstat & ~S3C2410_IICSTAT_TXRXEN, regs + S3C2410_IICSTAT);
	clk_disable(clk);
	clk_put(clk);
	iounmap(regs);
}
EXPORT_SYMBOL(s3c_i2c5_force_stop);

/* TODO(willow): port per-bus clock change from Thinkware i2c-s3c2410.c.
 * Only used while flashing touch firmware, so a no-op is safe for now. */
int s3c24xx_i2c_set(int num, int slave, int setfreq)
{
	pr_info("willow: s3c24xx_i2c_set(%d, 0x%02x, %d) ignored\n",
		num, slave, setfreq);
	return 0;
}
EXPORT_SYMBOL(s3c24xx_i2c_set);

int check_touch_cal;
void set_touch_autoCal(int value)
{
	check_touch_cal = value;
}
EXPORT_SYMBOL(set_touch_autoCal);

/* --- usb3503 dock hub recovery hooks (TODO(willow): real implementation) --- */
int ehci_power_control(struct device *dev, int force_enable, const char *buf)
{
	return 0;
}
EXPORT_SYMBOL(ehci_power_control);

void udc_power_control(int enable)
{
}
EXPORT_SYMBOL(udc_power_control);

/* --- referenced by fs/proc/cmdline.c (Samsung LPM charging) --- */
unsigned int poweroff_charging;
static int __init willow_get_lpm_state(char *str)
{
	get_option(&str, &poweroff_charging);
	return 1;
}
__setup("poweroff_charging=", willow_get_lpm_state);
