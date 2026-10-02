/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
*
* File Name		: l3g4200d_gyr_sysfs.c
* Authors		: MH - C&I BU - Application Team
*			: Carmine Iascone (carmine.iascone@st.com)
*			: Matteo Dameno (matteo.dameno@st.com)
* Version		: V 1.2.0 sysfs
* Date			: 21/11/2010
* Description		: L3G4200D digital output gyroscope sensor API
*
********************************************************************************
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* THE PRESENT SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES
* OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, FOR THE SOLE
* PURPOSE TO SUPPORT YOUR APPLICATION DEVELOPMENT.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* THIS SOFTWARE IS SPECIFICALLY DESIGNED FOR EXCLUSIVE USE WITH ST PARTS.
*
********************************************************************************
* REVISON HISTORY
*
* VERSION | DATE 	| AUTHORS	     | DESCRIPTION
*
* 1.0	  | 19/11/2010	| Carmine Iascone    | First Release
* 1.2	  | 		| Matteo Dameno	     | FIFO features
*
*******************************************************************************/

#include	<linux/i2c.h>
#include	<linux/mutex.h>
#include	<linux/input.h>
#include	<linux/interrupt.h>
#include	<linux/gpio.h>
#include <linux/slab.h>

#if 1 /* willow: earlysuspend shim */
#include <linux/earlysuspend.h>
#endif

#include <linux/l3g4200d_gyr.h>

#define L3G4200D_GYR_DEV_NAME	"l3g4200d_gyr"

/** Maximum polled-device-reported rot speed value value in dps*/
#define FS_MAX			32768

/* l3g4200d gyroscope registers */
#define WHO_AM_I        0x0F

#define	CTRL_REG1	0x20    /* CTRL REG1 */
#define	CTRL_REG2	0x21    /* CTRL REG2 */
#define	CTRL_REG3	0x22    /* CTRL_REG3 */
#define	CTRL_REG4	0x23    /* CTRL_REG4 */
#define	CTRL_REG5	0x24    /* CTRL_REG5 */
#define	REFERENCE	0x25    /* REFERENCE REG */
#define	FIFO_CTRL_REG	0x2E    /* FIFO CONTROL REGISTER */
#define FIFO_SRC_REG	0x2F    /* FIFO SOURCE REGISTER */
#define	OUT_X_L		0x28    /* 1st AXIS OUT REG of 6 */

#define AXISDATA_REG	OUT_X_L

/* CTRL_REG1 */
#define	PM_OFF		0x00
#define	PM_NORMAL	0x08
#define	ENABLE_ALL_AXES	0x07
#define	BW00		0x00
#define	BW01		0x10
#define	BW10		0x20
#define	BW11		0x30
#define	ODR100		0x00  /* ODR = 100Hz */
#define	ODR200		0x40  /* ODR = 200Hz */
#define	ODR400		0x80  /* ODR = 400Hz */
#define	ODR800		0xC0  /* ODR = 800Hz */


/* CTRL_REG3 bits */
#define	I2_DRDY		0x08
#define	I2_WTM		0x04
#define	I2_OVRUN	0x02
#define	I2_EMPTY	0x01
#define	I2_NONE		0x00
#define	I2_MASK		0x0F

/* CTRL_REG4 bits */
#define	FS_MASK		0x30

/* CTRL_REG5 bits */
#define	FIFO_ENABLE	0x40

/* FIFO_CTRL_REG bits */
#define	FIFO_MODE_MASK		0xE0
#define	FIFO_MODE_BYPASS	0x00
#define	FIFO_MODE_FIFO		0x20
#define	FIFO_MODE_STREAM	0x40
#define	FIFO_MODE_STR2FIFO	0x60
#define	FIFO_MODE_BYPASS2STR	0x80
#define	FIFO_WATERMARK_MASK	0x1F

#define FIFO_STORED_DATA_MASK	0x1F


#define FUZZ			0
#define FLAT			0
#define AUTO_INCREMENT		0x80

/* RESUME STATE INDICES */
#define	RES_CTRL_REG1		0
#define	RES_CTRL_REG2		1
#define	RES_CTRL_REG3		2
#define	RES_CTRL_REG4		3
#define	RES_CTRL_REG5		4
#define	RES_FIFO_CTRL_REG	5
#define	RESUME_ENTRIES		6

/*
#define	RES_INT_CFG1		6
#define	RES_INT_THS1		7
#define	RES_INT_DUR1		8
#define	RES_TEMP_CFG_REG	14
*/

#define DEBUG 0
#define TEST_ATTR 1

/** Registers Contents */
#define WHOAMI_L3G4200D		0x00D3	/* Expected content for WAI register*/

/*
 * L3G4200D gyroscope data
 * brief structure containing gyroscope values for yaw, pitch and roll in
 * signed short
 */

struct l3g4200d_gyr_triple {
	short	x,	/* x-axis angular rate data. */
		y,	/* y-axis angluar rate data. */
		z;	/* z-axis angular rate data. */
};

struct output_rate {
	int poll_rate_ms;
	u8 mask;
};

static const struct output_rate odr_table[] = {

	{	2,	ODR800|BW10},
	{	3,	ODR400|BW01},
	{	5,	ODR200|BW00},
	{	10,	ODR100|BW00},
};

struct l3g4200d_gyr_data {
	struct i2c_client *client;
	struct l3g4200d_gyr_platform_data *pdata;

	struct mutex lock;
	struct delayed_work input_work;

	struct input_dev *input_dev;
	int hw_initialized;
	atomic_t enabled;

	u8 reg_addr;
	u8 resume_state[RESUME_ENTRIES];

	int irq2;
	struct work_struct irq2_work;
	struct workqueue_struct *irq2_work_queue;

	bool polling_enabled;
#if 1 /* willow: earlysuspend shim */
	struct early_suspend early_suspend;
#endif
};

static int l3g4200d_gyr_i2c_read(struct l3g4200d_gyr_data *gyro,
				  u8 *buf, int len)
{
	int err;

	struct i2c_msg msgs[] = {
		{
		 .addr = gyro->client->addr,
		 .flags = gyro->client->flags & I2C_M_TEN,
		 .len = 1,
		 .buf = buf,
		 },
		{
		 .addr = gyro->client->addr,
		 .flags = (gyro->client->flags & I2C_M_TEN) | I2C_M_RD,
		 .len = len,
		 .buf = buf,
		 },
	};

	err = i2c_transfer(gyro->client->adapter, msgs, 2);

	if (err != 2) {
		dev_err(&gyro->client->dev, "read transfer error\n");
		return -EIO;
	}

	return 0;
}

static int l3g4200d_gyr_i2c_write(struct l3g4200d_gyr_data *gyro,
						u8 *buf,
						u8 len)
{
	int err;

	struct i2c_msg msgs[] = {
		{
		 .addr = gyro->client->addr,
		 .flags = gyro->client->flags & I2C_M_TEN,
		 .len = len + 1,
		 .buf = buf,
		 },
	};

	err = i2c_transfer(gyro->client->adapter, msgs, 1);

	if (err != 1) {
		dev_err(&gyro->client->dev, "write transfer error\n");
		return -EIO;
	}

	return 0;
}

static int l3g4200d_gyr_register_write(struct l3g4200d_gyr_data *gyro, u8 *buf,
		u8 reg_address, u8 new_value)
{
	int err;

	/* Sets configuration register at reg_address
	 *  NOTE: this is a straight overwrite  */
	buf[0] = reg_address;
	buf[1] = new_value;
	err = l3g4200d_gyr_i2c_write(gyro, buf, 1);
	if (err < 0)
		return err;

	return err;
}

static int l3g4200d_gyr_register_read(struct l3g4200d_gyr_data *gyro, u8 *buf,
		u8 reg_address)
{

	int err = -1;
	buf[0] = (reg_address);
	err = l3g4200d_gyr_i2c_read(gyro, buf, 1);
	return err;
}

static int l3g4200d_gyr_register_update(struct l3g4200d_gyr_data *gyro, u8 *buf,
		u8 reg_address, u8 mask, u8 new_bit_values)
{
	int err = -1;
	u8 init_val;
	u8 updated_val;
	err = l3g4200d_gyr_register_read(gyro, buf, reg_address);
	if (!(err < 0)) {
		init_val = buf[0];
		updated_val = ((mask & new_bit_values) | ((~mask) & init_val));
		err = l3g4200d_gyr_register_write(gyro, buf, reg_address,
				updated_val);
	}
	return err;
}


static int l3g4200d_gyr_update_watermark(struct l3g4200d_gyr_data *gyro,
								u8 watermark)
{
	int res = 0;
	u8 buf[2];
	u8 new_value;

	mutex_lock(&gyro->lock);
	new_value = (watermark % 0x20);
	res = l3g4200d_gyr_register_update(gyro, buf, FIFO_CTRL_REG,
			 FIFO_WATERMARK_MASK, new_value);
	if (res < 0) {
		pr_err("%s : failed to update watermark\n", __func__);
		return res;
	}
	#if DEBUG
	pr_err("%s : new_value:0x%02x,watermark:0x%02x\n", __func__, new_value,
								watermark);
	#endif
	gyro->resume_state[RES_FIFO_CTRL_REG] =
		((FIFO_WATERMARK_MASK & new_value) |
		(~FIFO_WATERMARK_MASK &
				gyro->resume_state[RES_FIFO_CTRL_REG]));
	gyro->pdata->watermark = new_value;
	mutex_unlock(&gyro->lock);
	return res;
}

static int l3g4200d_gyr_update_fifomode(struct l3g4200d_gyr_data *gyro,
								u8 fifomode)
{
	int res;
	u8 buf[2];
	u8 new_value;

	new_value = fifomode;
	res = l3g4200d_gyr_register_update(gyro, buf, FIFO_CTRL_REG,
					FIFO_MODE_MASK, new_value);
	if (res < 0) {
		pr_err("%s : failed to update fifoMode\n", __func__);
		return res;
	}

	gyro->resume_state[RES_FIFO_CTRL_REG] =
		((FIFO_MODE_MASK & new_value) |
		(~FIFO_MODE_MASK &
				gyro->resume_state[RES_FIFO_CTRL_REG]));
	gyro->pdata->fifomode = new_value;

	return res;
}

static int l3g4200d_gyr_fifo_reset(struct l3g4200d_gyr_data *gyro)
{
	u8 oldmode;
	int res;

	oldmode = gyro->pdata->fifomode;
	res = l3g4200d_gyr_update_fifomode(gyro, FIFO_MODE_BYPASS);
	if (res < 0)
		return res;
	res = l3g4200d_gyr_update_fifomode(gyro, oldmode);
	return res;
}

static int l3g4200d_gyr_fifo_hwenable(struct l3g4200d_gyr_data *gyro,
								u8 enable)
{
	int res;
	u8 buf[2];
	u8 set = 0x00;
	if (enable)
		set = FIFO_ENABLE;
	res = l3g4200d_gyr_register_update(gyro, buf, CTRL_REG5,
			FIFO_ENABLE, set);
	if (res < 0) {
		pr_err("%s : fifo_hw switch to:0x%02x failed\n", __func__, set);
		return res;
	}
	gyro->resume_state[RES_CTRL_REG5] =
		((FIFO_ENABLE & set) |
		(~FIFO_ENABLE & gyro->resume_state[RES_CTRL_REG5]));
	pr_err("%s : fifo_hw set to:0x%02x\n", __func__, set);
	return res;
}

static int l3g4200d_gyr_manage_int2settings(struct l3g4200d_gyr_data *gyro,
								u8 fifomode)
{
	int res;
	u8 buf[2];
	bool enable_fifo_hw;
	bool recognized_mode = true;
	u8 int2bits = I2_NONE;

	switch (fifomode) {
	case FIFO_MODE_FIFO:
		gyro->polling_enabled = false;
		int2bits = (I2_WTM | I2_OVRUN);
		res = l3g4200d_gyr_register_update(gyro, buf, CTRL_REG3,
					I2_MASK, int2bits);
		if (res < 0) {
			pr_err("%s : failed to update CTRL_REG3:0x%02x\n",
				__func__, fifomode);
			goto err_mutex_unlock;
		}
		gyro->resume_state[RES_CTRL_REG3] =
			((I2_MASK & int2bits) |
			(~(I2_MASK) & gyro->resume_state[RES_CTRL_REG3]));
		enable_fifo_hw = true;
		break;

	case FIFO_MODE_BYPASS:
		/*
		if (!gyro->polling_enabled)
			int2bits = I2_DRDY;
		*/
		int2bits = I2_NONE;
		res = l3g4200d_gyr_register_update(gyro, buf, CTRL_REG3,
					I2_MASK, int2bits);
		if (res < 0) {
			pr_err("%s : failed to update to CTRL_REG3:0x%02x\n",
				__func__, fifomode);
			goto err_mutex_unlock;
		}
		gyro->resume_state[RES_CTRL_REG3] =
			((I2_MASK & int2bits) |
			(~I2_MASK & gyro->resume_state[RES_CTRL_REG3]));
		enable_fifo_hw = false;
		break;

	default:
		enable_fifo_hw = false;
		recognized_mode = false;
		res = l3g4200d_gyr_register_update(gyro, buf, CTRL_REG3,
					I2_MASK, I2_NONE);
		if (res < 0) {
			pr_err("%s : failed to update CTRL_REG3:0x%02x\n",
				__func__, fifomode);
			goto err_mutex_unlock;
		}
		gyro->resume_state[RES_CTRL_REG3] =
			((I2_MASK & 0x00) |
			(~I2_MASK & gyro->resume_state[RES_CTRL_REG3]));
		break;

	}
	if (recognized_mode) {
		l3g4200d_gyr_update_fifomode(gyro, fifomode);
		if (res < 0) {
			pr_err("%s : failed to set fifoMode\n", __func__);
			goto err_mutex_unlock;
		}
	}
	res = l3g4200d_gyr_fifo_hwenable(gyro, enable_fifo_hw);

err_mutex_unlock:
	/* mutex_unlock(&gyro->lock); */
	return res;
}

static int l3g4200d_gyr_update_fs_range(struct l3g4200d_gyr_data *gyro,
								u8 new_fs)
{
	int res ;
	u8 buf[2];

	buf[0] = CTRL_REG4;

	res = l3g4200d_gyr_register_update(gyro, buf, CTRL_REG4,
							FS_MASK, new_fs);

	if (res < 0) {
		pr_err("%s : failed to update fs:0x%02x\n",
			__func__, new_fs);
		return res;
	}
	gyro->resume_state[RES_CTRL_REG4] =
		((FS_MASK & new_fs) |
		(~FS_MASK & gyro->resume_state[RES_CTRL_REG4]));

	return res;
}


static int l3g4200d_gyr_update_odr(struct l3g4200d_gyr_data *gyro,
				int poll_interval_ms)
{
	int err = -1;
	int i;
	u8 config[2];

	for (i = ARRAY_SIZE(odr_table) - 1; i >= 0; i--) {
		if (odr_table[i].poll_rate_ms <= poll_interval_ms)
			break;
	}

	config[1] = odr_table[i].mask;
	config[1] |= (ENABLE_ALL_AXES + PM_NORMAL);

	/* If device is currently enabled, we need to write new
	 *  configuration out to it */
	if (atomic_read(&gyro->enabled)) {
		config[0] = CTRL_REG1;
		err = l3g4200d_gyr_i2c_write(gyro, config, 1);
		if (err < 0)
			return err;
		gyro->resume_state[RES_CTRL_REG1] = config[1];
	}


	return err;
}

/* gyroscope data readout */
static int l3g4200d_gyr_get_data(struct l3g4200d_gyr_data *gyro,
			     struct l3g4200d_gyr_triple *data)
{
	int err;
	unsigned char gyro_out[6];
	/* y,p,r hardware data */
	s16 hw_d[3] = { 0 };

	gyro_out[0] = (AUTO_INCREMENT | AXISDATA_REG);

	err = l3g4200d_gyr_i2c_read(gyro, gyro_out, 6);

	if (err < 0)
		return err;

	hw_d[0] = (s16) (((gyro_out[1]) << 8) | gyro_out[0]);
	hw_d[1] = (s16) (((gyro_out[3]) << 8) | gyro_out[2]);
	hw_d[2] = (s16) (((gyro_out[5]) << 8) | gyro_out[4]);
#if DEBUG
      printk(KERN_INFO "gyro_out hw_d[0] = %d hw_d[1] = %d hw_d[2] = %d\n",
              hw_d[0], hw_d[1], hw_d[2]);
#endif

	data->x = ((gyro->pdata->negate_x) ? (-hw_d[gyro->pdata->axis_map_x])
		   : (hw_d[gyro->pdata->axis_map_x]));
	data->y = ((gyro->pdata->negate_y) ? (-hw_d[gyro->pdata->axis_map_y])
		   : (hw_d[gyro->pdata->axis_map_y]));
	data->z = ((gyro->pdata->negate_z) ? (-hw_d[gyro->pdata->axis_map_z])
		   : (hw_d[gyro->pdata->axis_map_z]));

	#if DEBUG
		printk(KERN_INFO "gyro_out: y = %d p = %d r= %d\n",
						data->x, data->y, data->z);
	#endif

	return err;
}

static void l3g4200d_gyr_report_values(struct l3g4200d_gyr_data *gyro,
					struct l3g4200d_gyr_triple *data)
{
	input_report_abs(gyro->input_dev, ABS_X, data->x);
	input_report_abs(gyro->input_dev, ABS_Y, data->y);
	input_report_abs(gyro->input_dev, ABS_Z, data->z);
}

static int l3g4200d_gyr_hw_init(struct l3g4200d_gyr_data *gyro)
{
	int err;
	u8 buf[6];

#if DEBUG
	printk(KERN_INFO "%s hw init\n", L3G4200D_GYR_DEV_NAME);
#endif
	buf[0] = (AUTO_INCREMENT | CTRL_REG1);
	buf[1] = gyro->resume_state[RES_CTRL_REG1];
	buf[2] = gyro->resume_state[RES_CTRL_REG2];
	buf[3] = gyro->resume_state[RES_CTRL_REG3];
	buf[4] = gyro->resume_state[RES_CTRL_REG4];
	buf[5] = gyro->resume_state[RES_CTRL_REG5];
	err = l3g4200d_gyr_i2c_write(gyro, buf, 5);
	if (err < 0)
		return err;

	buf[0] = FIFO_CTRL_REG;
	buf[1] = gyro->resume_state[RES_FIFO_CTRL_REG];
	err = l3g4200d_gyr_i2c_write(gyro, buf, 1);
	if (err < 0)
			return err;

	gyro->hw_initialized = 1;

	return err;
}

static void l3g4200d_gyr_device_power_off(struct l3g4200d_gyr_data *dev_data)
{
	int err;
	u8 buf[2];
#if DEBUG
	pr_info("%s power off\n", L3G4200D_GYR_DEV_NAME);
#endif
	buf[0] = CTRL_REG1;
	buf[1] = PM_OFF;
	err = l3g4200d_gyr_i2c_write(dev_data, buf, 1);
	if (err < 0)
		dev_err(&dev_data->client->dev, "[GYR] soft power off failed\n");

	if (dev_data->pdata->power_off) {
		/* disable_irq_nosync(acc->irq1); */
		disable_irq_nosync(dev_data->irq2);
		dev_data->pdata->power_off();
		dev_data->hw_initialized = 0;
	}

	if (dev_data->hw_initialized)
		/* disable_irq_nosync(acc->irq1); */
		disable_irq_nosync(dev_data->irq2);
		dev_data->hw_initialized = 0;

}

static int l3g4200d_gyr_device_power_on(struct l3g4200d_gyr_data *dev_data)
{
	int err;

	if (dev_data->pdata->power_on) {
		err = dev_data->pdata->power_on();
		if (err < 0)
			return err;
		if (dev_data->pdata->gpio_int2)
			enable_irq(dev_data->irq2);
	}


	if (!dev_data->hw_initialized) {
		err = l3g4200d_gyr_hw_init(dev_data);
		if (err < 0) {
			l3g4200d_gyr_device_power_off(dev_data);
			return err;
		}
	}

	if (dev_data->hw_initialized) {
		/*
		if (dev_data->pdata->gpio_int1) {
			enable_irq(dev_data->irq1);
			printk(KERN_INFO "%s: power on: irq1 enabled\n",
					L3G4200D_GYR_DEV_NAME);
		}*/
		if (dev_data->pdata->gpio_int2) {
			enable_irq(dev_data->irq2);
			printk(KERN_INFO "%s: power on: irq2 enabled\n",
						L3G4200D_GYR_DEV_NAME);
		}
	}

	return 0;
}

static int l3g4200d_gyr_enable(struct l3g4200d_gyr_data *dev_data)
{
	int err;

	if (!atomic_cmpxchg(&dev_data->enabled, 0, 1)) {

		err = l3g4200d_gyr_device_power_on(dev_data);
		if (err < 0) {
			atomic_set(&dev_data->enabled, 0);
			return err;
		}
		if (dev_data->polling_enabled)
			schedule_delayed_work(&dev_data->input_work,
			    msecs_to_jiffies(dev_data->pdata->poll_interval));
	}

	return 0;
}

static int l3g4200d_gyr_disable(struct l3g4200d_gyr_data *dev_data)
{
	if (atomic_cmpxchg(&dev_data->enabled, 1, 0)) {
		cancel_delayed_work_sync(&dev_data->input_work);
		l3g4200d_gyr_device_power_off(dev_data);
	}
	return 0;
}

static ssize_t attr_polling_rate_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	int val;
	struct l3g4200d_gyr_data *gyro = dev_get_drvdata(dev);
	mutex_lock(&gyro->lock);
	val = gyro->pdata->poll_interval;
	mutex_unlock(&gyro->lock);
	return sprintf(buf, "%d\n", val);
}

static ssize_t attr_polling_rate_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t size)
{
	struct l3g4200d_gyr_data *gyro = dev_get_drvdata(dev);
	unsigned long interval_ms;

	if (strict_strtoul(buf, 10, &interval_ms))
		return -EINVAL;
	if (!interval_ms)
		return -EINVAL;
	mutex_lock(&gyro->lock);
	gyro->pdata->poll_interval = interval_ms;
	l3g4200d_gyr_update_odr(gyro, interval_ms);
	mutex_unlock(&gyro->lock);
	return size;
}

static ssize_t attr_range_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct l3g4200d_gyr_data *gyro = dev_get_drvdata(dev);
	u8 val;
	mutex_lock(&gyro->lock);
	val = gyro->pdata->fs_range;
	mutex_unlock(&gyro->lock);
	return sprintf(buf, "0x%02x\n", val);
}

static ssize_t attr_range_store(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf, size_t size)
{
	struct l3g4200d_gyr_data *gyro = dev_get_drvdata(dev);
	unsigned long val;
	bool good_val = false;

	if (strict_strtoul(buf, 16, &val))
		return -EINVAL;
	switch ((u8) val) {
	case L3G4200D_GYR_FS_250DPS:
		good_val = true;
		break;
	case L3G4200D_GYR_FS_500DPS:
		good_val = true;
		break;
	case L3G4200D_GYR_FS_2000DPS:
		good_val = true;
		break;
	default:
		return -EINVAL;
	}
	if (good_val) {
		mutex_lock(&gyro->lock);
		l3g4200d_gyr_update_fs_range(gyro, (u8) val);
		gyro->pdata->fs_range = val;
		mutex_unlock(&gyro->lock);
	}
	return size;
}

static ssize_t attr_enable_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct l3g4200d_gyr_data *gyro = dev_get_drvdata(dev);
	int val = atomic_read(&gyro->enabled);
	return sprintf(buf, "%d\n", val);
}

static ssize_t attr_enable_store(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t size)
{
	struct l3g4200d_gyr_data *gyro = dev_get_drvdata(dev);
	unsigned long val;

	if (strict_strtoul(buf, 10, &val))
		return -EINVAL;

	if (val){
		l3g4200d_gyr_enable(gyro);
		printk(KERN_INFO "[GYR] Power-ON\n");
    }
	else{
		l3g4200d_gyr_disable(gyro);
		printk(KERN_INFO "[GYR] Power-OFF\n");
    }
	return size;
}

static ssize_t attr_polling_mode_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	int val = 0;
	struct l3g4200d_gyr_data *gyro = dev_get_drvdata(dev);
	mutex_lock(&gyro->lock);
	if (gyro->polling_enabled)
		val = 1;
	mutex_unlock(&gyro->lock);
	return sprintf(buf, "%d\n", val);
}

static ssize_t attr_polling_mode_store(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t size)
{
	struct l3g4200d_gyr_data *gyro = dev_get_drvdata(dev);
	unsigned long val;

	if (strict_strtoul(buf, 10, &val))
		return -EINVAL;

	mutex_lock(&gyro->lock);
	if (val) {
		gyro->polling_enabled = true;
		l3g4200d_gyr_manage_int2settings(gyro, FIFO_MODE_BYPASS);
		if (gyro->polling_enabled) {
			schedule_delayed_work(&gyro->input_work,
				msecs_to_jiffies(gyro->pdata->poll_interval));
		}
	} else {
		gyro->polling_enabled = false;
		l3g4200d_gyr_manage_int2settings(gyro, FIFO_MODE_BYPASS);
	}
	mutex_unlock(&gyro->lock);
	return size;
}

static ssize_t attr_watermark_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t size)
{
	struct l3g4200d_gyr_data *gyro = dev_get_drvdata(dev);
	unsigned long watermark;
	int res;

	if (strict_strtoul(buf, 16, &watermark))
		return -EINVAL;

	res = l3g4200d_gyr_update_watermark(gyro, watermark);
	if (res < 0)
		return res;

	return size;
}

static ssize_t attr_watermark_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct l3g4200d_gyr_data *gyro = dev_get_drvdata(dev);
	int val = gyro->pdata->watermark;
	return sprintf(buf, "0x%02x\n", val);
}

static ssize_t attr_fifomode_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t size)
{
	struct l3g4200d_gyr_data *gyro = dev_get_drvdata(dev);
	unsigned long fifomode;
	int res;

	if (strict_strtoul(buf, 16, &fifomode))
		return -EINVAL;

	mutex_lock(&gyro->lock);
	res = l3g4200d_gyr_manage_int2settings(gyro, (u8) fifomode);
	mutex_unlock(&gyro->lock);

	if (res < 0)
		return res;
	return size;
}

static ssize_t attr_fifomode_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct l3g4200d_gyr_data *gyro = dev_get_drvdata(dev);
	u8 val = gyro->pdata->fifomode;
	return sprintf(buf, "0x%02x\n", val);
}

static ssize_t attr_reg_set(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t size)
{
	int rc;
	struct l3g4200d_gyr_data *gyro = dev_get_drvdata(dev);
	u8 x[2];
	unsigned long val;

	if (strict_strtoul(buf, 16, &val))
		return -EINVAL;
	mutex_lock(&gyro->lock);
	x[0] = gyro->reg_addr;
	mutex_unlock(&gyro->lock);
	x[1] = val;
	rc = l3g4200d_gyr_i2c_write(gyro, x, 1);
	return size;
}

static ssize_t attr_reg_get(struct device *dev, struct device_attribute *attr,
				char *buf)
{
	ssize_t ret;
	struct l3g4200d_gyr_data *gyro = dev_get_drvdata(dev);
	int rc;
	u8 data;

	mutex_lock(&gyro->lock);
	data = gyro->reg_addr;
	mutex_unlock(&gyro->lock);
	rc = l3g4200d_gyr_i2c_read(gyro, &data, 1);
	ret = sprintf(buf, "0x%02x\n", data);
	return ret;
}

static ssize_t attr_addr_set(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct l3g4200d_gyr_data *gyro = dev_get_drvdata(dev);
	unsigned long val;

	if (strict_strtoul(buf, 16, &val))
		return -EINVAL;

	mutex_lock(&gyro->lock);

	gyro->reg_addr = val;

	mutex_unlock(&gyro->lock);

	return size;
}

static struct device_attribute attributes[] = {
	__ATTR(pollrate_ms, 0644, attr_polling_rate_show,
						attr_polling_rate_store),
	__ATTR(range, 0644, attr_range_show, attr_range_store),
	__ATTR(enable, 0644, attr_enable_show, attr_enable_store),
//   __ATTR(enable_gyro, 0644, attr_enable_show, attr_enable_store),
	__ATTR(enable_polling, 0644, attr_polling_mode_show,
						attr_polling_mode_store),
	__ATTR(fifo_samples, 0644, attr_watermark_show, attr_watermark_store),
	__ATTR(fifo_mode, 0644, attr_fifomode_show, attr_fifomode_store),
	__ATTR(reg_value, 0600, attr_reg_get, attr_reg_set),
	__ATTR(reg_addr, 0200, NULL, attr_addr_set),
};
#ifdef TEST_ATTR
static DEVICE_ATTR(enable, S_IRUGO|S_IWUSR|S_IWGRP,
		   attr_enable_show, attr_enable_store);
static DEVICE_ATTR(polling, S_IRUGO|S_IWUSR|S_IWGRP,
		   attr_polling_mode_show, attr_polling_mode_store);		   
static DEVICE_ATTR(reg_value, S_IRUGO|S_IWUSR|S_IWGRP,
		   attr_reg_get, attr_reg_set);
static struct attribute *l3g4200d_attributes[] = {
   	&dev_attr_enable.attr,
   	&dev_attr_polling.attr,
   	&dev_attr_reg_value.attr,
   	NULL
};
static struct attribute_group l3g4200d_attributes_group = {
	.attrs = l3g4200d_attributes
};
#endif

static int create_sysfs_interfaces(struct device *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		if (device_create_file(dev, attributes + i))
			goto error;
	return 0;

error:
	for (; i >= 0; i--)
		device_remove_file(dev, attributes + i);
	dev_err(dev, "%s:Unable to create interface\n", __func__);
	return -1;
}

static int remove_sysfs_interfaces(struct device *dev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(attributes); i++)
		device_remove_file(dev, attributes + i);
	return 0;
}

static void report_triple(struct l3g4200d_gyr_data *gyro)
{
	int err;
	struct l3g4200d_gyr_triple data_out;

	err = l3g4200d_gyr_get_data(gyro, &data_out);
	if (err < 0)
		dev_err(&gyro->client->dev, "get_gyroscope_data failed\n");
	else
		l3g4200d_gyr_report_values(gyro, &data_out);
}

static void l3g4200d_gyr_irq2_fifo(struct l3g4200d_gyr_data *gyro)
{
	int err;
	u8 buf[2];
	u8 int_source;
	u8 samples;
	u8 workingmode;
	u8 stored_samples;

	mutex_lock(&gyro->lock);

	workingmode = gyro->pdata->fifomode;
	samples = (gyro->pdata->watermark)+1;

	err = l3g4200d_gyr_register_read(gyro, buf, FIFO_SRC_REG);
	if (err > 0)
		dev_err(&gyro->client->dev, "error reading fifo source reg\n");

	int_source = buf[0];
	#if DEBUG
	pr_debug("%s : FIFO_SRC_REG content:0x%02x\n", __func__, int_source);
	#endif

	stored_samples = int_source & FIFO_STORED_DATA_MASK;

	switch (workingmode) {
	case FIFO_MODE_BYPASS:
	{
		report_triple(gyro);
	}
	case FIFO_MODE_FIFO:
	    #if DEBUG
		pr_debug("%s : samples:%d stored:%d\n", __func__,
						samples, stored_samples);
		#endif
		for (; samples > 0; samples--)
			report_triple(gyro);
		l3g4200d_gyr_fifo_reset(gyro);
		break;
	}
	input_sync(gyro->input_dev);
	mutex_unlock(&gyro->lock);
}

static irqreturn_t l3g4200d_gyr_isr2(int irq, void *dev)
{
	struct l3g4200d_gyr_data *gyro = dev;

	disable_irq_nosync(irq);
	queue_work(gyro->irq2_work_queue, &gyro->irq2_work);

	return IRQ_HANDLED;
}

static void l3g4200d_gyr_irq2_work_func(struct work_struct *work)
{

	struct l3g4200d_gyr_data *gyro =
	container_of(work, struct l3g4200d_gyr_data, irq2_work);
	/* TODO  add interrupt service procedure.
		 ie:l3g4200d_gyr_XXX(gyro); */
	l3g4200d_gyr_irq2_fifo(gyro);

//exit:
	enable_irq(gyro->irq2);
}

static void l3g4200d_gyr_input_work_func(struct work_struct *work)
{
	/* int err; */
	/* struct l3g4200d_gyr_triple data_out; */
	struct l3g4200d_gyr_data *gyro =
		container_of((struct delayed_work *)work,
			 struct l3g4200d_gyr_data, input_work);


	mutex_lock(&gyro->lock);
	report_triple(gyro);
	input_sync(gyro->input_dev);

	if (gyro->polling_enabled) {
		schedule_delayed_work(&gyro->input_work, msecs_to_jiffies(
				gyro->pdata->poll_interval));
	}
	mutex_unlock(&gyro->lock);
}


int l3g4200d_gyr_input_open(struct input_dev *input)
{
	//struct l3g4200d_gyr_data *gyro = input_get_drvdata(input);

	return 0;//l3g4200d_gyr_enable(gyro); //no need enabling gyro at this time.
}

void l3g4200d_gyr_input_close(struct input_dev *dev)
{
	struct l3g4200d_gyr_data *gyro = input_get_drvdata(dev);

	l3g4200d_gyr_disable(gyro);
}

static int l3g4200d_gyr_validate_pdata(struct l3g4200d_gyr_data *gyro)
{
	gyro->pdata->poll_interval = max(gyro->pdata->poll_interval,
			gyro->pdata->min_interval);

	if (gyro->pdata->axis_map_x > 2 ||
	    gyro->pdata->axis_map_y > 2 ||
	    gyro->pdata->axis_map_z > 2) {
		dev_err(&gyro->client->dev,
			"invalid axis_map value x:%u y:%u z%u\n",
			gyro->pdata->axis_map_x,
			gyro->pdata->axis_map_y,
			gyro->pdata->axis_map_z);
		return -EINVAL;
	}

	/* Only allow 0 and 1 for negation boolean flag */
	if (gyro->pdata->negate_x > 1 ||
	    gyro->pdata->negate_y > 1 ||
	    gyro->pdata->negate_z > 1) {
		dev_err(&gyro->client->dev,
			"invalid negate value x:%u y:%u z:%u\n",
			gyro->pdata->negate_x,
			gyro->pdata->negate_y,
			gyro->pdata->negate_z);
		return -EINVAL;
	}

	/* Enforce minimum polling interval */
	if (gyro->pdata->poll_interval < gyro->pdata->min_interval) {
		dev_err(&gyro->client->dev,
			"minimum poll interval violated\n");
		return -EINVAL;
	}
	return 0;
}

#if 0
void l3g4200d_gyr_early_suspend(struct early_suspend *h);
void l3g4200d_gyr_late_resume(struct early_suspend *h);
#endif

static int l3g4200d_gyr_input_init(struct l3g4200d_gyr_data *gyro)
{
	int err;

	INIT_DELAYED_WORK(&gyro->input_work, l3g4200d_gyr_input_work_func);
	gyro->input_dev = input_allocate_device();
	if (!gyro->input_dev) {
		err = -ENOMEM;
		dev_err(&gyro->client->dev,
			"input device allocation failed\n");
		goto err0;
	}

	gyro->input_dev->open = l3g4200d_gyr_input_open;
	gyro->input_dev->close = l3g4200d_gyr_input_close;
	gyro->input_dev->name = "gyroscope";

	gyro->input_dev->id.bustype = BUS_I2C;
	gyro->input_dev->dev.parent = &gyro->client->dev;

	input_set_drvdata(gyro->input_dev, gyro);

	set_bit(EV_ABS, gyro->input_dev->evbit);

	input_set_abs_params(gyro->input_dev, ABS_X, -FS_MAX, FS_MAX, FUZZ,
									FLAT);
	input_set_abs_params(gyro->input_dev, ABS_Y, -FS_MAX, FS_MAX, FUZZ,
									FLAT);
	input_set_abs_params(gyro->input_dev, ABS_Z, -FS_MAX, FS_MAX, FUZZ,
									FLAT);

	err = input_register_device(gyro->input_dev);
	if (err) {
		dev_err(&gyro->client->dev,
			"unable to register input device %s\n",
			gyro->input_dev->name);
		goto err1;
	}

#if 0
	gyro->early_suspend.suspend = l3g4200d_gyr_early_suspend;
	gyro->early_suspend.resume = l3g4200d_gyr_late_resume;
	gyro->early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB;
	register_early_suspend(&gyro->early_suspend);
#endif

	return 0;

err1:
	input_free_device(gyro->input_dev);
err0:
	return err;
}

static void l3g4200d_gyr_input_cleanup(struct l3g4200d_gyr_data *gyro)
{
#if 1 /* willow: earlysuspend shim */
	unregister_early_suspend(&gyro->early_suspend);
#endif
	input_unregister_device(gyro->input_dev);
	input_free_device(gyro->input_dev);
}

static int l3g4200d_gyr_probe(struct i2c_client *client,
					const struct i2c_device_id *devid)
{
	struct l3g4200d_gyr_data *gyro;
//	struct device *dev;

	int err = -1;
#if DEBUG
	pr_info("%s: probe start.\n", L3G4200D_GYR_DEV_NAME);
#endif
    printk(KERN_INFO "[GYR] L3g4200d Probe\n");

	if (client->dev.platform_data == NULL) {
		dev_err(&client->dev, "platform data is NULL. exiting.\n");
		err = -ENODEV;
		goto err0;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "client not i2c capable:1\n");
		err = -ENODEV;
		goto err0;
	}

	gyro = kzalloc(sizeof(*gyro), GFP_KERNEL);
	if (gyro == NULL) {
		dev_err(&client->dev,
			"failed to allocate memory for module data\n");
		err = -ENOMEM;
		goto err0;
	}

	mutex_init(&gyro->lock);
	mutex_lock(&gyro->lock);
	gyro->client = client;

	gyro->pdata = kmalloc(sizeof(*gyro->pdata), GFP_KERNEL);
	if (gyro->pdata == NULL) {
		dev_err(&client->dev,
			"failed to allocate memory for pdata: %d\n", err);
		goto err1;
	}
	memcpy(gyro->pdata, client->dev.platform_data,
						sizeof(*gyro->pdata));

	err = l3g4200d_gyr_validate_pdata(gyro);
	if (err < 0) {
		dev_err(&client->dev, "failed to validate platform data\n");
		goto err1_1;
	}

	i2c_set_clientdata(client, gyro);

	if (gyro->pdata->init) {
		err = gyro->pdata->init();
		if (err < 0) {
			dev_err(&client->dev, "init failed: %d\n", err);
			goto err1_1;
		}
	}


	memset(gyro->resume_state, 0, ARRAY_SIZE(gyro->resume_state));

	gyro->resume_state[RES_CTRL_REG1] = ENABLE_ALL_AXES + PM_NORMAL;
	gyro->resume_state[RES_CTRL_REG2] = 0x00;
	gyro->resume_state[RES_CTRL_REG3] = 0x00;
	gyro->resume_state[RES_CTRL_REG4] = 0x00;
	gyro->resume_state[RES_CTRL_REG5] = 0x00;
	gyro->resume_state[RES_FIFO_CTRL_REG] = 0x00;

	gyro->polling_enabled = true;

	err = l3g4200d_gyr_device_power_on(gyro);
	if (err < 0) {
		dev_err(&client->dev, "power on failed: %d\n", err);
		goto err2;
	}

	atomic_set(&gyro->enabled, 1);

	err = l3g4200d_gyr_update_fs_range(gyro, gyro->pdata->fs_range);
	if (err < 0) {
		dev_err(&client->dev, "update_fs_range failed\n");
		goto err2;
	}

	err = l3g4200d_gyr_update_odr(gyro, gyro->pdata->poll_interval);
	if (err < 0) {
		dev_err(&client->dev, "update_odr failed\n");
		goto err2;
	}

	err = l3g4200d_gyr_input_init(gyro);
	if (err < 0)
		goto err3;

	err = create_sysfs_interfaces(&client->dev);
	if (err < 0) {
		dev_err(&client->dev,
			"%s device register failed\n", L3G4200D_GYR_DEV_NAME);
		goto err4;
	}
#ifdef TEST_ATTR
    err = sysfs_create_group(&gyro->input_dev->dev.kobj, &l3g4200d_attributes_group);
    if(err < 0){
        goto err7;
    }
#endif

	l3g4200d_gyr_device_power_off(gyro);

	/* As default, do not report information */
	atomic_set(&gyro->enabled, 0);


	if (gyro->pdata->gpio_int2) {
		gyro->irq2 = gpio_to_irq(gyro->pdata->gpio_int2);
		printk(KERN_INFO "%s: %s has set irq2 to irq:"
						" %d mapped on gpio:%d\n",
			L3G4200D_GYR_DEV_NAME, __func__, gyro->irq2,
							gyro->pdata->gpio_int2);

		INIT_WORK(&gyro->irq2_work, l3g4200d_gyr_irq2_work_func);
		gyro->irq2_work_queue =
			create_singlethread_workqueue("l3g4200d_gyr_wq2");
		if (!gyro->irq2_work_queue) {
			err = -ENOMEM;
			dev_err(&client->dev, "cannot create "
						"work queue2: %d\n", err);
			goto err5;
		}

		err = request_irq(gyro->irq2, l3g4200d_gyr_isr2,
				IRQF_TRIGGER_HIGH, "l3d4200d_gyr_irq2", gyro);
		if (err < 0) {
			dev_err(&client->dev, "request irq2 failed: %d\n", err);
			goto err6;
		}
		disable_irq_nosync(gyro->irq2);
	}

	mutex_unlock(&gyro->lock);

	#if DEBUG
	pr_info("%s probed: device created successfully\n",
			L3G4200D_GYR_DEV_NAME);
	#endif

	return 0;

/*err7:
	free_irq(gyro->irq2, gyro);
*/
#ifdef TEST_ATTR
err7:
#endif
err6:
	destroy_workqueue(gyro->irq2_work_queue);
err5:
	l3g4200d_gyr_device_power_off(gyro);
	remove_sysfs_interfaces(&client->dev);
err4:
	l3g4200d_gyr_input_cleanup(gyro);
err3:

err2:
	if (gyro->pdata->exit)
		gyro->pdata->exit();
err1_1:
	mutex_unlock(&gyro->lock);
	kfree(gyro->pdata);
err1:
	kfree(gyro);
err0:
	pr_err("%s: Driver Initialization failed\n",
				L3G4200D_GYR_DEV_NAME);
	return err;
}

static int l3g4200d_gyr_remove(struct i2c_client *client)
{
	struct l3g4200d_gyr_data *gyro = i2c_get_clientdata(client);

	#if DEBUG
	pr_info(KERN_INFO "L3G4200D driver removing\n");
	#endif

	/*
	if (gyro->pdata->gpio_int1) {
		free_irq(gyro->irq1, gyro);
		gpio_free(gyro->pdata->gpio_int1);
		destroy_workqueue(gyro->irq1_work_queue);
	}*/
	if (gyro->pdata->gpio_int2) {
		free_irq(gyro->irq2, gyro);
		gpio_free(gyro->pdata->gpio_int2);
		destroy_workqueue(gyro->irq2_work_queue);
	}

	l3g4200d_gyr_input_cleanup(gyro);
	l3g4200d_gyr_device_power_off(gyro);
	remove_sysfs_interfaces(&client->dev);

	kfree(gyro->pdata);
	kfree(gyro);
	return 0;
}

#ifndef CONFIG_HAS_EARLYSUSPEND
static int l3g4200d_gyr_suspend(struct device *dev)
{
	#ifdef CONFIG_SUSPEND
//	struct i2c_client *client = to_i2c_client(dev);
//	struct l3g4200d_gyr_data *gyro = i2c_get_clientdata(client);
	#if DEBUG
	pr_info(KERN_INFO "l3g4200d_gyr_suspend\n");
	#endif
	/* TO DO */
	#endif
	return 0;
}
#else
void l3g4200d_gyr_early_suspend(struct early_suspend *h)
{
	struct l3g4200d_gyr_data *dev_data =
		container_of(h, struct l3g4200d_gyr_data, early_suspend);

	if (atomic_read(&dev_data->enabled)) {
		cancel_delayed_work_sync(&dev_data->input_work);
		l3g4200d_gyr_device_power_off(dev_data);
	}

	return ;
}
#endif

#ifndef CONFIG_HAS_EARLYSUSPEND
static int l3g4200d_gyr_resume(struct device *dev)
{

	#ifdef CONFIG_SUSPEND
//	struct i2c_client *client = to_i2c_client(dev);
//	struct l3g4200d_gyr_data *gyro = i2c_get_clientdata(client);
	#if DEBUG
	pr_info(KERN_INFO "l3g4200d_gyr_resume\n");
	#endif
	/* TO DO */
	#endif
	return 0;
}
#else
void l3g4200d_gyr_late_resume(struct early_suspend *h)
{
	struct l3g4200d_gyr_data *dev_data =
		container_of(h, struct l3g4200d_gyr_data, early_suspend);

	if (atomic_read(&dev_data->enabled)) {
		l3g4200d_gyr_device_power_on(dev_data);
		schedule_delayed_work(&dev_data->input_work, msecs_to_jiffies(dev_data->pdata->poll_interval));
	}

	return ;
}
#endif

static const struct i2c_device_id l3g4200d_gyr_id[] = {
	{ L3G4200D_GYR_DEV_NAME , 0 },
	{},
};

MODULE_DEVICE_TABLE(i2c, l3g4200d_gyr_id);

#ifndef CONFIG_HAS_EARLYSUSPEND
static struct dev_pm_ops l3g4200d_gyr_pm = {
	.suspend = l3g4200d_gyr_suspend,
	.resume = l3g4200d_gyr_resume,
};
#endif

static struct i2c_driver l3g4200d_gyr_driver = {
	.driver = {
			.owner = THIS_MODULE,
			.name = L3G4200D_GYR_DEV_NAME,
    #ifndef CONFIG_HAS_EARLYSUSPEND
			.pm = &l3g4200d_gyr_pm,
    #endif
	},
	.probe = l3g4200d_gyr_probe,
	.remove = __devexit_p(l3g4200d_gyr_remove),
	.id_table = l3g4200d_gyr_id,
//	.suspend = l3g4200d_gyr_suspend,
//	.resume = l3g4200d_gyr_resume,

};

static int __init l3g4200d_gyr_init(void)
{
	#if DEBUG
	pr_info("%s: gyroscope sysfs driver init\n", L3G4200D_GYR_DEV_NAME);
	#endif
	return i2c_add_driver(&l3g4200d_gyr_driver);
}

static void __exit l3g4200d_gyr_exit(void)
{
	#if DEBUG
	pr_info("L3G4200D exit\n");
	#endif
	i2c_del_driver(&l3g4200d_gyr_driver);
	return;
}

module_init(l3g4200d_gyr_init);
module_exit(l3g4200d_gyr_exit);

MODULE_DESCRIPTION("l3g4200d digital gyroscope sysfs driver");
MODULE_AUTHOR("Matteo Dameno, Carmine Iascone, STMicroelectronics");
MODULE_LICENSE("GPL");

