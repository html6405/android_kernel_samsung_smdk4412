/*
 * isl29023.c - Intersil isl29023  ALS & Proximity Driver
 *
 * By Intersil Corp
 * Michael DiGioia
 *
 * Based on isl29020.c
 *	by Kalhan Trisal <kalhan.trisal@intel.com>
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/kernel.h>
#include <linux/hwmon.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/sysfs.h>
//#include <linux/pm_runtime.h>
#include <linux/isl29023.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/input.h>

/* Insmod parameters */
//I2C_CLIENT_INSMOD_1(isl29023);

static int last_mod;
#define delay_to_jiffies(d) ((d)?msecs_to_jiffies(d):1)

#define DEBUG 0

static DEFINE_MUTEX(mutex);

static int is_device = 1;

static int isl_set_range(struct i2c_client *client, int range)
{
	int ret_val;

	ret_val = i2c_smbus_read_byte_data(client, REG_CMD_2);
	if (ret_val < 0)
		return -EINVAL;
	ret_val &= ~SENSOR_RANGE_MASK;	/*reset the bit */
	ret_val |= range;
	ret_val = i2c_smbus_write_byte_data(client, REG_CMD_2, ret_val);
#if DEBUG
   printk(KERN_INFO MODULE_NAME ": %s isl29023 set_range call, \n", __func__);
#endif
	if (ret_val < 0)
		return ret_val;
	return range;
}

static int isl_set_mod(struct i2c_client *client, int mod)
{
	int ret, val, freq;

	switch (mod) {
	case ISL_MOD_POWERDOWN:
		goto setmod;
	case ISL_MOD_ALS_CONT:
		freq = 0;
		break;
	default:
		return -EINVAL;
	}

	/* set IR frequency */
	val = i2c_smbus_read_byte_data(client, REG_CMD_2);
	if (val < 0)
		return -EINVAL;
	val &= ~IR_FREQ_MASK;
	if (freq)
		val |= IR_FREQ_MASK;
	ret = i2c_smbus_write_byte_data(client, REG_CMD_2, val);
	if (ret < 0)
		return -EINVAL;

setmod:
	/* set operation mod */
	val = i2c_smbus_read_byte_data(client, REG_CMD_1);
	if (val < 0)
		return -EINVAL;
	val &= ~ISL_MOD_MASK;
	val |= (mod << 5);
	ret = i2c_smbus_write_byte_data(client, REG_CMD_1, val);
	if (ret < 0)
		return -EINVAL;

	if (mod != ISL_MOD_POWERDOWN)
		last_mod = mod;

	return mod;

}

static int isl_get_res(struct i2c_client *client)
{
	int val;
	val = i2c_smbus_read_byte_data(client, REG_CMD_2);

	if (val < 0)
		return -EINVAL;

	val &= ISL_RES_MASK;
	val >>= 2;

	switch (val) {
	case 0:
		return 65536;
	case 1:
		return 4096;
	case 2:
		return 256;
	case 3:
		return 16;
	default:
		return -EINVAL;
	}
}

static int isl_get_mod(struct i2c_client *client)
{
	int val;

	val = i2c_smbus_read_byte_data(client, REG_CMD_1);
	if (val < 0)
		return -EINVAL;
	return val >> 5;
}

static ssize_t
isl_sensing_range_show(struct device *dev,
		       struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	int val;

	mutex_lock(&mutex);
//	pm_runtime_get_sync(dev);
	val = i2c_smbus_read_byte_data(client, REG_CMD_2);
//	pm_runtime_put_sync(dev);
	mutex_unlock(&mutex);
#if DEBUG
	dev_dbg(dev, "%s: range: 0x%.2x\n", __func__, val);
#endif
	if (val < 0)
		return val;
	return sprintf(buf, "%d000\n", 1 << (2 * (val & 3)));
}

struct isl29023_data {
	int  last;       /* last measured data */
	struct i2c_client *client;
	struct input_dev *input;
	struct delayed_work work;
};

static ssize_t
isl_output_data_show(struct device *dev,
		     struct device_attribute *attr, char *buf)
{
	struct input_dev *input = to_input_dev(dev);
	struct isl29023_data *isl29023 = input_get_drvdata(input);
#if DEBUG
  printk("isl_output_data_show val = %d  \n",isl29023->last);
#endif
	return sprintf(buf, "%d\n", isl29023->last);
}

static ssize_t
isl_sensing_range_store(struct device *dev,
			struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	unsigned int ret_val;
	unsigned long val;

	if (strict_strtoul(buf, 10, &val))
		return -EINVAL;

	switch (val) {
	case 1000:
		val = 0;
		break;
	case 4000:
		val = 1;
		break;
	case 16000:
		val = 2;
		break;
	case 64000:
		val = 3;
		break;
	default:
		return -EINVAL;
	}

	mutex_lock(&mutex);
//	pm_runtime_get_sync(dev);
	ret_val = isl_set_range(client, val);
//	pm_runtime_put_sync(dev);
	mutex_unlock(&mutex);

	if (ret_val < 0)
		return ret_val;
	return count;
}

static ssize_t
isl_enable_show(struct device *dev,
		     struct device_attribute *attr, char *buf)
		     
{
    struct isl29023_data *isl29023 = dev_get_drvdata(dev);
    int val;
    int temp_val = 1;
    
    if(!is_device)
    {
        printk("[LIGHT] NO DEVICE is_device = %d  \n",is_device);
        return;
    }
  
    mutex_lock(&mutex);
    val = isl_get_mod(isl29023->client);
    mutex_unlock(&mutex);
  
    if (val < 0)
      return val;
    
    switch (val) {
    case ISL_MOD_POWERDOWN:
      return sprintf(buf, "%d\n", ISL_MOD_POWERDOWN);
    case ISL_MOD_ALS_CONT:
      return sprintf(buf, "%d\n", temp_val);
    default:
      return -EINVAL;
    }

}

static ssize_t
isl_enable_store(struct device *dev,
		      struct device_attribute *attr,
		      const char *buf, size_t count)
{
	struct isl29023_data *isl29023 = dev_get_drvdata(dev);
	int ret_val;
	unsigned long val;

	if (strict_strtoul(buf, 10, &val))
		return -EINVAL;

    //printk("[LIGHT]isl_enable_store val = %ld  \n",val);
    if(!is_device)
    {
        printk("[LIGHT] NO DEVICE is_device = %d  \n",is_device);
        return;
    }

    if(val == 1)
	    val = 5;  //ISL_MOD_ALS_CONT
    mutex_lock(&mutex);
	ret_val = isl_set_mod(isl29023->client, val);
	mutex_unlock(&mutex);

	if (ret_val < 0)
		return ret_val;

   if(val){
    schedule_delayed_work(&isl29023->work, delay_to_jiffies(3000) + 1);
#ifdef FEATURE_BACKLIGHT_ONOFF_LOG
    printk("[LIGHT] Power-Up \n");
#endif
   }
  else{
    cancel_delayed_work_sync(&isl29023->work);
#ifdef FEATURE_BACKLIGHT_ONOFF_LOG
    printk("[LIGHT] Power-Down \n");
#endif
   }

    return count;
}


static DEVICE_ATTR(range, S_IRUGO | S_IWUSR,
		   isl_sensing_range_show, isl_sensing_range_store);
static DEVICE_ATTR(data, S_IRUGO, isl_output_data_show, NULL);
static DEVICE_ATTR(enable, S_IRUGO | S_IWUSR,
		   isl_enable_show, isl_enable_store);


static struct attribute *mid_att_isl[] = {
	&dev_attr_range.attr,
	&dev_attr_data.attr,
	&dev_attr_enable.attr,
	NULL
};

static struct attribute_group m_isl_gr = {
//	.name = "isl29023",
	.attrs = mid_att_isl
};

static int isl_set_default_config(struct i2c_client *client)
{
	int ret;
	ret = i2c_smbus_write_byte_data(client, REG_CMD_1, 0xE0);
	if (ret < 0)
		return -EINVAL;
	ret = i2c_smbus_write_byte_data(client, REG_CMD_2, 0x03); //0xc3 -> 0x03
	if (ret < 0)
		return -EINVAL;
 #if DEBUG 
    printk(KERN_INFO MODULE_NAME ": %s isl29023 set_default_config call, \n", __func__);
#endif
	return 0;
}

static int isl29023_input_init(struct isl29023_data *isl29023)
{
	struct input_dev *dev;
	int err;

	dev = input_allocate_device();
	if (!dev) {
		return -ENOMEM;
	}
	dev->name = "light";
	dev->id.bustype = BUS_I2C;

	input_set_capability(dev, EV_ABS, ABS_MISC);
	input_set_abs_params(dev, ABS_X, 0, 1000, 0, 0);
	input_set_drvdata(dev, isl29023);

	err = input_register_device(dev);
	if (err < 0) {
		input_free_device(dev);
		return err;
	}
	isl29023->input = dev;

	return 0;
}
static int  isl29023_measure(struct  isl29023_data * isl29023)
{
  struct i2c_client *client =  isl29023->client;
  int ret_val, val, mod;
  unsigned long int max_count;
  int output = 0;
  int temp;

  mutex_lock(&mutex);
//  pm_runtime_get_sync(&client->dev);

  temp = i2c_smbus_read_byte_data(client, REG_DATA_MSB);
  if (temp < 0)
    goto err_exit;
  ret_val = i2c_smbus_read_byte_data(client, REG_DATA_LSB);
  if (ret_val < 0)
    goto err_exit;
  ret_val |= temp << 8;
  
#if DEBUG
  dev_dbg(dev, "%s: Data: %04x\n", __func__, ret_val);
#endif
//	printk("%s: Data: %04x\n", __func__, ret_val);
	
  mod = isl_get_mod(client);
  switch (mod) {
  case ISL_MOD_ALS_CONT:
  case ISL_MOD_ALS_ONCE:
  case ISL_MOD_IR_ONCE:
  case ISL_MOD_IR_CONT:
    output = ret_val;
    break;
  case ISL_MOD_PS_CONT:
  case ISL_MOD_PS_ONCE:
    val = i2c_smbus_read_byte_data(client, REG_CMD_2);
    if (val < 0)
      goto err_exit;
    max_count = isl_get_res(client);
    output = (((1 << (2 * (val & SENSOR_RANGE_MASK))) * 1000)
        * ret_val) / max_count;
    break;
  default:
    goto err_exit;
  }
  
//  pm_runtime_put_sync(&client->dev);
  mutex_unlock(&mutex);
//  return sprintf(buf, "%ld\n", output);

#if DEBUG
  printk("=== isl29023_measure  ==== output = %d \n ",output);
#endif
  return output;

err_exit:
  printk("=== isl29023_measure error  === \n ");
//  pm_runtime_put_sync(&client->dev);
  mutex_unlock(&mutex);
  return -EINVAL;
}

static void isl29023_work_func(struct work_struct *work)
{
	int last_data;
	struct isl29023_data *isl29023 = container_of((struct delayed_work *)work,struct isl29023_data, work);

	last_data = isl29023_measure(isl29023);

	input_report_abs(isl29023->input, ABS_X, last_data);
	input_sync(isl29023->input);
  
	isl29023->last = last_data;
  schedule_delayed_work(&isl29023->work, delay_to_jiffies(3000) + 1);
}

static int
isl29023_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int res;
	int err;
	int mode_check;
  
	struct isl29023_data *isl29023;
	printk("[LIGHT] Isl29023 Probe \n");

	isl29023 = kzalloc(sizeof(struct isl29023_data), GFP_KERNEL);
	if (!isl29023) {
		return -EINVAL;
	}

    //printk(KERN_INFO MODULE_NAME ": %s isl29023 probe call, ID= %s\n", __func__, id->name);
#if 1
	err = isl29023_input_init(isl29023);
	if (err < 0) {
		return -EINVAL;
	}
 	err = sysfs_create_group(&isl29023->input->dev.kobj, &m_isl_gr);
	if (err < 0) {
		return -EINVAL;
	}
#endif
	res = isl_set_default_config(client);
	if (res < 0) {
		//pr_warn("isl29023: set default config failed!!\n");
     printk(KERN_INFO MODULE_NAME ": %s isl29023 set default config failed\n", __func__);

     is_device = 0;
     //
		return -EINVAL;
	}

  i2c_set_clientdata(client, isl29023);
  isl29023->client = client;
  
  /* setup driver interfaces */
    INIT_DELAYED_WORK(&isl29023->work, isl29023_work_func);


  last_mod = 0;
  mode_check = isl_set_mod(client, ISL_MOD_ALS_CONT);
  if (mode_check)
    schedule_delayed_work(&isl29023->work, delay_to_jiffies(3000) + 1);
  else
    cancel_delayed_work_sync(&isl29023->work);

  
#if 0 //Ĩ�� �ҷ����� ���� Ÿ ������ �������� ���� ���� �۾�.
	err = isl29023_input_init(isl29023);
	if (err < 0) {
		return -EINVAL;
	}
  
	err = sysfs_create_group(&isl29023->input->dev.kobj, &m_isl_gr);
	if (err < 0) {
		return -EINVAL;
	}
#endif
  
//  pm_runtime_enable(&client->dev);

#if DEBUG
	dev_dbg(&client->dev, "isl29023 probe succeed!\n");
#endif

	return res;
}

static int isl29023_remove(struct i2c_client *client)
{
  struct isl29023_data *isl29023 = i2c_get_clientdata(client);

	sysfs_remove_group(&isl29023->input->dev.kobj, &m_isl_gr);
 	kfree(isl29023);
//	__pm_runtime_disable(&client->dev, false);
   printk(KERN_INFO MODULE_NAME ": %s isl29023 remove call, \n", __func__);
	return 0;
}

static struct i2c_device_id isl29023_id[] = {
	{"isl29023", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, isl29023_id);


static struct i2c_driver isl29023_driver = {
	.driver = {
	    .name = "isl29023",
   		.owner = THIS_MODULE,
		   },
	.probe = isl29023_probe,
	.remove = isl29023_remove,
	.id_table = isl29023_id,
};

static int __init sensor_isl29023_init(void)
{
#if DEBUG
   printk(KERN_INFO MODULE_NAME ": %s isl29023 init call, \n", __func__);
#endif
	return i2c_add_driver(&isl29023_driver);
}

static void __exit sensor_isl29023_exit(void)
{
 printk(KERN_INFO MODULE_NAME ": %s isl29023 exit call \n", __func__);
	i2c_del_driver(&isl29023_driver);
}

module_init(sensor_isl29023_init);
module_exit(sensor_isl29023_exit);

MODULE_AUTHOR("mdigioia");
MODULE_ALIAS("isl29023 ALS/PS");
MODULE_DESCRIPTION("Intersil isl29023 ALS/PS Driver");
MODULE_LICENSE("GPL v2");


