/*
 * max8903_charger.c - Maxim 8903 USB/Adapter Charger Driver
 *
 * Copyright (C) 2011 Samsung Electronics
 * MyungJoo Ham <myungjoo.ham@samsung.com>
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/power_supply.h>
#include <linux/platform_device.h>
#include <linux/power/max8903_charger.h>
#include <linux/delay.h>
#include <plat/gpio-cfg.h>

//#define SUPPORT_USB_STATE
#define FULL_VCELL 4250000
#define FULL_SOC 98
#define LOW_SOC 1
#define WORK_DELAY 1000

#ifdef CONFIG_BATTERY_MAX17040
#define HAVE_FUEL_GAUGE
#endif

#define BATTERY_DEBUG
#ifdef BATTERY_DEBUG
#include <linux/proc_fs.h>
#endif

#ifdef HAVE_FUEL_GAUGE
extern int fg_read_soc(void);
extern int fg_read_vcell(void);
#endif

#ifdef CONFIG_TOUCHSCREEN_ATMEL_MXT1664S
#include <linux/i2c/1664s_driver.h>
#endif
#ifdef FEATURE_TOUCH_NOISE
extern void atm1664_power_noise(bool power_detect);
#endif

bool usb_is_connected = false;
bool dc_is_connected = false;
EXPORT_SYMBOL(usb_is_connected);
EXPORT_SYMBOL(dc_is_connected);

bool g_update_need = true;

struct max8903_data {
	struct max8903_pdata *pdata;
	struct device *dev;

	struct power_supply battery; // battery
	struct power_supply usb; // usb charger
	struct power_supply adapter; // dc adapter

	struct delayed_work	work;

	bool fault; // battery fault
	bool usb_in;
	bool ta_in;
};

static enum power_supply_property battery_charger_props[] = {
	POWER_SUPPLY_PROP_STATUS, /* Charger status output */
	POWER_SUPPLY_PROP_HEALTH, /* Fault or OK */
	POWER_SUPPLY_PROP_ONLINE,
#ifdef HAVE_FUEL_GAUGE
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
#endif	
	POWER_SUPPLY_PROP_TECHNOLOGY,
};

static enum power_supply_property usb_charger_props[] = {
	POWER_SUPPLY_PROP_ONLINE, /* External power source */
};

static enum power_supply_property adapter_charger_props[] = {
	POWER_SUPPLY_PROP_ONLINE, /* External power source */
};

#ifdef BATTERY_DEBUG
static bool g_debug_enable = false;
static bool g_force_soc_use = false;
static int  g_force_soc_val = 100;

void print_status_table(void)
{
	printk("\n------------------------------------------\n");
	printk("|     | ta_in(nDC_OK) | usb_in(nUSB_OK)  |\n");
	printk("------------------------------------------\n");
	printk("| NC  | H(fals)       | L(true)          |\n");
	printk("| AC  | L(true)       | H(fals)          |\n");
	printk("| USB | L(true)       | L(true)          |\n");
	printk("------------------------------------------\n\n");
}

ssize_t battery_debug_enable_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	print_status_table();
	printk("[BATTERY_DEBUG]   g_debug_enable[%sABLE]\n", g_debug_enable ? "EN" : "DIS");

	return 0;
}

ssize_t battery_debug_enable_write(struct file *file, const char __user *buffer, size_t count, loff_t * ppos)
{
	char *endp;

	print_status_table();
	g_debug_enable = (u32)(simple_strtoul(buffer, &endp, 10));
	printk("[BATTERY_DEBUG]   g_debug_enable[%sABLE]\n", g_debug_enable ? "EN" : "DIS");

	return (count + endp - buffer);
}

static const struct file_operations battery_debug_proc_fops = {
	.read = battery_debug_enable_read,
	.write = battery_debug_enable_write,
};
#endif

static int battery_get_property(struct power_supply *battery,
		enum power_supply_property psp,
		union power_supply_propval *val)
{
	struct max8903_data *data = container_of(battery,
			struct max8903_data, battery);

	switch (psp) {
		case POWER_SUPPLY_PROP_ONLINE:
			val->intval = 1; //always
			dev_dbg(data->dev, "%s: ONLINE=%d\n", __func__, val->intval);
			break;
		case POWER_SUPPLY_PROP_STATUS:
			val->intval = POWER_SUPPLY_STATUS_UNKNOWN;
			if (data->pdata->chg) {
#ifdef SUPPORT_USB_STATE
				if (gpio_get_value(data->pdata->chg) == 0) { // low is charging
					if(fg_read_soc() >= FULL_SOC || fg_read_vcell() >= FULL_VCELL) {
						val->intval = POWER_SUPPLY_STATUS_FULL;
						gpio_set_value(data->pdata->cen, 1); // off
					} else if(data->ta_in || data->usb_in) {
						val->intval = POWER_SUPPLY_STATUS_CHARGING;
					} else {
						val->intval = POWER_SUPPLY_STATUS_UNKNOWN;
					}
				} else { // not charging
					if(fg_read_soc() < FULL_SOC || fg_read_vcell() < FULL_VCELL) {
						gpio_set_value(data->pdata->cen, 0); // on
					}

					if (data->usb_in || data->ta_in) 
						val->intval = POWER_SUPPLY_STATUS_NOT_CHARGING;
					else
						val->intval = POWER_SUPPLY_STATUS_DISCHARGING;
				}
#else
				if(gpio_get_value(data->pdata->chg) == 0) { // low is charging
					if((fg_read_soc() >= FULL_SOC || fg_read_vcell() >= FULL_VCELL)
						&& (data->ta_in && !data->usb_in)) {
						val->intval = POWER_SUPPLY_STATUS_FULL;
						gpio_set_value(data->pdata->cen, 1); // off
					} else if(data->ta_in) {
						if(data->usb_in) {
							val->intval = POWER_SUPPLY_STATUS_NOT_CHARGING;
						} else {
							val->intval = POWER_SUPPLY_STATUS_CHARGING;
						}
					} else {
						val->intval = POWER_SUPPLY_STATUS_UNKNOWN;
					}

				} else { // not charging
					if(fg_read_soc() < FULL_SOC && fg_read_vcell() < FULL_VCELL) {
						gpio_set_value(data->pdata->cen, 0); // on
					}

					val->intval = POWER_SUPPLY_STATUS_DISCHARGING;
				}
#endif
			}
			break;
		case POWER_SUPPLY_PROP_HEALTH:
			val->intval = POWER_SUPPLY_HEALTH_GOOD;
			if (data->fault)
				val->intval = POWER_SUPPLY_HEALTH_UNSPEC_FAILURE;
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_NOW:
			val->intval = fg_read_vcell();
			break;
		case POWER_SUPPLY_PROP_CAPACITY:
			val->intval = fg_read_soc();
			if (val->intval >= FULL_SOC) {
				val->intval = 100;
			}
#ifdef BATTERY_DEBUG
			if (g_force_soc_use) {
				val->intval = g_force_soc_val;
			}
#endif
			break;
		case POWER_SUPPLY_PROP_TECHNOLOGY:
			val->intval = POWER_SUPPLY_TECHNOLOGY_LIPO;
			break;
		default:
			return -EINVAL;
	}
	return 0;
}

#ifdef BATTERY_DEBUG
static int battery_set_property(struct power_supply *battery,
		enum power_supply_property psp,
		const union power_supply_propval *val)
{

	switch (psp) {
		case POWER_SUPPLY_PROP_CAPACITY:
			g_force_soc_use = true;

			if (0 <= val->intval && 100 >= val->intval)
			{
				g_force_soc_val = val->intval;
			} else {
				g_force_soc_use = false;
			}

			printk("[BATTERY_DEBUG]   FORCE[%sABLE] SOC[%d] \n",
				g_force_soc_use ? "EN" : "DIS",
				g_force_soc_use ? g_force_soc_val : fg_read_soc());
			power_supply_changed(battery);

			break;

		default:
			return -EINVAL;
	}

	return 0;
}
#endif

static int usb_get_property(struct power_supply *usb, enum power_supply_property psp, union power_supply_propval *val)
{
	struct max8903_data *data = container_of(usb,
			struct max8903_data, usb);

	switch (psp) {
		case POWER_SUPPLY_PROP_ONLINE:
			val->intval = data->ta_in ? (data->usb_in ? 1 : 0) : 0;//JSK//data->usb_in ? 1 : 0;
			break;
		default:
			return -EINVAL;
	}
	return 0;
}

static int adapter_get_property(struct power_supply *adapter, enum power_supply_property psp, union power_supply_propval *val)
{
	struct max8903_data *data = container_of(adapter,
			struct max8903_data, adapter);

	switch (psp) {
		case POWER_SUPPLY_PROP_ONLINE:
			val->intval = data->ta_in ? (data->usb_in ? 0 : 1) : 0;
			break;
		default:
			return -EINVAL;
	}
	return 0;
}

static irqreturn_t max8903_dcin(int irq, void *_data)
{
	struct max8903_data *data = _data;
	struct max8903_pdata *pdata = data->pdata;
	struct power_supply *battery = &data->battery;
	struct power_supply *usb = &data->usb;
	struct power_supply *adapter = &data->adapter;

	/*-----------------------------------------
	 *     | ta_in(nDC_OK) | usb_in(nUSB_OK)  |
	 *-----------------------------------------
	 * NC  | H(fals)       | L(true)          |
	 * AC  | L(true)       | H(fals)          |
	 * USB | L(true)       | L(true)          |
	 *----------------------------------------*/
	int ta_in, usb_in;

	ta_in = gpio_get_value(pdata->dok) ? 0 : 1;
	msleep(250);
	usb_in = gpio_get_value(pdata->uok) ? 0 : 1;

#ifdef BATTERY_DEBUG
	if (g_debug_enable) {
		printk("[BATTERY] %s() dc=[%s], usb=[%s]\n", __FUNCTION__, ta_in ? "L" : "H", usb_in ? "L" : "H");
	}
#endif

	if (ta_in == data->ta_in && usb_in == data->usb_in)
		return IRQ_HANDLED;

	data->ta_in = ta_in;
	data->usb_in = usb_in;

	if ( ta_in && usb_in ) {
		usb_is_connected = true;
		dc_is_connected = false;
	} else if ( ta_in ) {
		usb_is_connected = false;
		dc_is_connected = true;
	} else {
		usb_is_connected = false;
		dc_is_connected = false;
	}

	/* Set Current-Limit-Mode 1:DC 0:USB */
#ifdef CTRL_DCM
	if (pdata->dcm)
		gpio_set_value(pdata->dcm, ta_in ? 1 : 0);
#endif

	/* Charger Enable / Disable (cen is negated) */
	if (pdata->cen)
		gpio_set_value(pdata->cen, ta_in ? 0 :
				(data->usb_in ? 0 : 1));

	dev_dbg(data->dev, "TA(DC-IN) Charger %s.\n", ta_in ?
			"Connected" : "Disconnected");

#if 0
	old_type = data->psy.type;

	if (data->ta_in)
		data->psy.type = POWER_SUPPLY_TYPE_MAINS;
	else if (data->usb_in)
		data->psy.type = POWER_SUPPLY_TYPE_USB;
	else
		data->psy.type = POWER_SUPPLY_TYPE_BATTERY;

	if (old_type != data->psy.type)
		power_supply_changed(&data->psy);
#else
#ifdef SUPPORT_USB_STATE
	if(data->ta_in)
		power_supply_changed(adapter);
	else if(data->usb_in)
		power_supply_changed(usb);
	else
		power_supply_changed(battery);
#else
	if(data->ta_in && data->usb_in)
		power_supply_changed(usb);
	else if(data->ta_in)
		power_supply_changed(adapter);
	else
		power_supply_changed(battery);
#endif
#endif

#ifdef FEATURE_TOUCH_NOISE
		atm1664_power_noise(ta_in); 	
#endif

	return IRQ_HANDLED;
}

#ifdef SUPPORT_USB_STATE
static irqreturn_t max8903_usbin(int irq, void *_data)
{
	struct max8903_data *data = _data;
	struct max8903_pdata *pdata = data->pdata;
	struct power_supply *battery = &data->battery;
	struct power_supply *usb = &data->usb;
	struct power_supply *adapter = &data->adapter;

	bool usb_in;
	//	enum power_supply_type old_type;

	usb_in = gpio_get_value(pdata->uok) ? 0 : 1;

	if (usb_in == data->usb_in)
		return IRQ_HANDLED;

	data->usb_in = usb_in;

	/* Do not touch Current-Limit-Mode */

	/* Charger Enable / Disable (cen is negated) */
	if (pdata->cen)
		gpio_set_value(pdata->cen, usb_in ? 0 :
				(data->ta_in ? 0 : 1));

	dev_dbg(data->dev, "USB Charger %s.\n", usb_in ?
			"Connected" : "Disconnected");

#if 0
	old_type = data->psy.type;

	if (data->ta_in)
		data->psy.type = POWER_SUPPLY_TYPE_MAINS;
	else if (data->usb_in)
		data->psy.type = POWER_SUPPLY_TYPE_USB;
	else
		data->psy.type = POWER_SUPPLY_TYPE_BATTERY;

	if (old_type != data->psy.type)
		power_supply_changed(&data->psy);
#else
	if(data->ta_in)
		power_supply_changed(adapter);
	else if(data->usb_in)
		power_supply_changed(usb);
	else
		power_supply_changed(battery);
#endif

	return IRQ_HANDLED;
}
#else
void usb_connection_report(bool connected)
{
	struct max8903_data *data;
	struct power_supply *battery;
	struct power_supply *usb;
	struct power_supply *adapter;

	usb = power_supply_get_by_name(CHARGER_USB_NAME);

	if(usb) {
		data = container_of(usb, struct max8903_data, usb);

		if(data) {
			if(data->usb_in == connected) 
				return;

			battery = &data->battery;
			adapter = &data->adapter;

			data->usb_in = connected;

			if(data->ta_in && data->usb_in)
				power_supply_changed(usb);
			else if(data->ta_in)
				power_supply_changed(adapter);
			else
				power_supply_changed(battery);
		}
	}
}

EXPORT_SYMBOL(usb_connection_report);
#endif

static irqreturn_t max8903_fault(int irq, void *_data)
{
	struct max8903_data *data = _data;
	struct max8903_pdata *pdata = data->pdata;
	bool fault;

	fault = gpio_get_value(pdata->flt) ? false : true;

	if (fault == data->fault)
		return IRQ_HANDLED;

	data->fault = fault;

	if (fault)
		dev_err(data->dev, "Charger suffers a fault and stops.\n");
	else
		dev_err(data->dev, "Charger recovered from a fault.\n");

	return IRQ_HANDLED;
}

static bool charger_online(void)
{
	struct max8903_data *data;
	struct power_supply *usb;

	usb = power_supply_get_by_name(CHARGER_USB_NAME);

	if(usb) {
		data = container_of(usb, struct max8903_data, usb);

		if(data) {
			return data->ta_in || data->usb_in;
		}
	}

	return false;
}

static void max8903_work(struct work_struct *work)
{
	struct max8903_data* data;
	static int msg_update_cnt =0;
	static int old_soc = 0;
	int soc = fg_read_soc();

	data = container_of(work, struct max8903_data, work.work);

#ifdef SUPPORT_USB_STATE
	if(!data->usb_in && !data->ta_in)
		power_supply_changed(&data->battery);
	else if(data->ta_in)
		power_supply_changed(&data->adapter);
	else if(data->usb_in)
		power_supply_changed(&data->usb);
#else
	if(old_soc != soc)
		g_update_need = true;

	if(g_update_need) {
#ifdef BATTERY_DEBUG
		if ( g_debug_enable ) {
			printk("[*NOTIFY] old_soc=%d soc=%d \n", old_soc , soc);
		}
#endif
		g_update_need = false;
		old_soc = soc;
		if(data->ta_in && data->usb_in)
			power_supply_changed(&data->usb);
		else if(data->ta_in)
			power_supply_changed(&data->adapter);
		else
			power_supply_changed(&data->battery);
	}
#endif

	if ( msg_update_cnt > 58 ) {
		printk("[BATTERY] old_soc=%d soc=%d, vcell=%duV, dc=%s, usb=%s\n", old_soc, soc, fg_read_vcell(), data->ta_in ? "true" : "false", data->usb_in ? "true" : "false");
		msg_update_cnt = 0;
	} else {
		msg_update_cnt++;
	}

#ifdef BATTERY_DEBUG
	if ( g_debug_enable ) {
		printk("[BATTERY] old_soc=%d soc=%d, vcell=%duV, dc=%s, usb=%s\n", old_soc, soc, fg_read_vcell(), data->ta_in ? "true" : "false", data->usb_in ? "true" : "false");
	}
#endif

	schedule_delayed_work(&data->work, msecs_to_jiffies(WORK_DELAY));
}

static int max8903_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct max8903_data *data = platform_get_drvdata(pdev);
	struct max8903_pdata *pdata = data->pdata;

	g_update_need = false;
	cancel_delayed_work(&data->work);

	if(pdata->dok) {
		int irq = gpio_to_irq(pdata->dok);
		enable_irq_wake(irq);
	}
 
	return 0;
}

static int max8903_resume(struct platform_device* pdev)
{
	struct max8903_data *data = platform_get_drvdata(pdev);
	struct max8903_pdata *pdata = data->pdata;

	g_update_need = true;
	schedule_delayed_work(&data->work, msecs_to_jiffies(WORK_DELAY));

	if(pdata->dok) {
		int irq = gpio_to_irq(pdata->dok);
		disable_irq_wake(irq);
	}

	max8903_work(&data->work.work);

	return 0;
}

static __devinit int max8903_probe(struct platform_device *pdev)
{
	struct max8903_data *data;
	struct power_supply *battery;
	struct power_supply *usb;
	struct power_supply *adapter;
	struct device *dev = &pdev->dev;
	struct max8903_pdata *pdata = pdev->dev.platform_data;
	int ret = 0;
	int gpio;
	int ta_in = 0;
	int usb_in = 0;

	data = kzalloc(sizeof(struct max8903_data), GFP_KERNEL);
	if (data == NULL) {
		dev_err(dev, "Cannot allocate memory.\n");
		return -ENOMEM;
	}

	battery = &data->battery;
	usb = &data->usb;
	adapter = &data->adapter;

	pdata->charger_online = charger_online;

	data->pdata = pdata;
	data->dev = dev;
	platform_set_drvdata(pdev, data);

	if (pdata->dc_valid == false && pdata->usb_valid == false) {
		dev_err(dev, "No valid power sources.\n");
		ret = -EINVAL;
		goto err;
	}

	if (pdata->dc_valid) {
#ifdef CTRL_DCM
		if (pdata->dok && gpio_is_valid(pdata->dok) &&
				pdata->dcm && gpio_is_valid(pdata->dcm)) {
			gpio = pdata->dok; /* PULL_UPed Interrupt */
			ta_in = gpio_get_value(gpio) ? 0 : 1;

			gpio = pdata->dcm; /* Output */
			gpio_set_value(gpio, ta_in);
		} else {
			dev_err(dev, "When DC is wired, DOK and DCM should"
					" be wired as well.\n");
			ret = -EINVAL;
			goto err;
		}
	} else {
		if (pdata->dcm) {
			if (gpio_is_valid(pdata->dcm))
				gpio_set_value(pdata->dcm, 0);
			else {
				dev_err(dev, "Invalid pin: dcm.\n");
				ret = -EINVAL;
				goto err;
			}
		}
#else
		gpio = pdata->dok; /* PULL_UPed Interrupt */
		ta_in = gpio_get_value(gpio) ? 0 : 1;
#endif
	}

	if (pdata->usb_valid) {
		if (pdata->uok && gpio_is_valid(pdata->uok)) {
			gpio = pdata->uok;
			usb_in = gpio_get_value(gpio) ? 0 : 1;
		} else {
			dev_err(dev, "When USB is wired, UOK should be wired."
					"as well.\n");
			ret = -EINVAL;
			goto err;
		}
	}
	//printk("%s() ta_in[%s] usb_in[%s]\n", __FUNCTION__, ta_in ? "L" : "H", usb_in ? "L" : "H");

	if (pdata->cen) {
		if (gpio_is_valid(pdata->cen)) {
			gpio_set_value(pdata->cen, (ta_in || usb_in) ? 0 : 1);
		} else {
			dev_err(dev, "Invalid pin: cen.\n");
			ret = -EINVAL;
			goto err;
		}
	}

	if (pdata->chg) {
		if (!gpio_is_valid(pdata->chg)) {
			dev_err(dev, "Invalid pin: chg.\n");
			ret = -EINVAL;
			goto err;
		}
	}

	if (pdata->flt) {
		if (!gpio_is_valid(pdata->flt)) {
			dev_err(dev, "Invalid pin: flt.\n");
			ret = -EINVAL;
			goto err;
		}
	}

#ifdef CTRL_USUS
	if (pdata->usus) {
		if (!gpio_is_valid(pdata->usus)) {
			dev_err(dev, "Invalid pin: usus.\n");
			ret = -EINVAL;
			goto err;
		}
	}
#endif

	data->fault = false;
	data->ta_in = ta_in;
	data->usb_in = usb_in;

	if ( ta_in && usb_in ) {
		usb_is_connected = true;
		dc_is_connected = false;
	} else if ( ta_in ) {
		usb_is_connected = false;
		dc_is_connected = true;
	} else {
		usb_is_connected = false;
		dc_is_connected = false;
	}

	battery->name = CHARGER_BATTERY_NAME;//"charger-battery";
	battery->type = POWER_SUPPLY_TYPE_BATTERY;
	battery->get_property = battery_get_property;
#ifdef BATTERY_DEBUG
	battery->set_property = battery_set_property;
#endif
	battery->properties = battery_charger_props;
	battery->num_properties = ARRAY_SIZE(battery_charger_props);

	usb->name = CHARGER_USB_NAME;//"charger-usb";
	usb->type = POWER_SUPPLY_TYPE_USB;
	usb->get_property = usb_get_property;
	usb->properties = usb_charger_props;
	usb->num_properties = ARRAY_SIZE(usb_charger_props);

	adapter->name = CHARGER_ADAPTER_NAME;//"charger-adapter";
	adapter->type = POWER_SUPPLY_TYPE_MAINS;
	adapter->get_property = adapter_get_property;
	adapter->properties = adapter_charger_props;
	adapter->num_properties = ARRAY_SIZE(adapter_charger_props);

	ret = power_supply_register(dev, battery);
	if (ret) {
		dev_err(dev, "failed: power supply register (battery).\n");
		goto err;
	}

	ret = power_supply_register(dev, usb);
	if (ret) {
		dev_err(dev, "failed: power supply register (usb).\n");
		goto err;
	}

	ret = power_supply_register(dev, adapter);
	if (ret) {
		dev_err(dev, "failed: power supply register (adapter).\n");
		goto err;
	}

	if (pdata->dc_valid) {
		ret = request_threaded_irq(gpio_to_irq(pdata->dok),
				NULL, max8903_dcin,
				IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
				"MAX8903 DC IN", data);
		if (ret) {
			dev_err(dev, "Cannot request irq %d for DC (%d)\n",
					gpio_to_irq(pdata->dok), ret);
			goto err_psy;
		}
	}

#ifdef SUPPORT_USB_STATE
	if (pdata->usb_valid) {
		ret = request_threaded_irq(gpio_to_irq(pdata->uok),
				NULL, max8903_usbin,
				IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
				"MAX8903 USB IN", data);
		if (ret) {
			dev_err(dev, "Cannot request irq %d for USB (%d)\n",
					gpio_to_irq(pdata->uok), ret);
			goto err_dc_irq;
		}
	}
#endif

	if (pdata->flt) {
		ret = request_threaded_irq(gpio_to_irq(pdata->flt),
				NULL, max8903_fault,
				IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
				"MAX8903 Fault", data);
		if (ret) {
			dev_err(dev, "Cannot request irq %d for Fault (%d)\n",
					gpio_to_irq(pdata->flt), ret);
			goto err_usb_irq;
		}
	}

	INIT_DELAYED_WORK_DEFERRABLE(&data->work, max8903_work);

	schedule_delayed_work(&data->work, msecs_to_jiffies(WORK_DELAY));

#ifdef BATTERY_DEBUG
	proc_create("battery_debug", S_IRWXUGO, NULL, &battery_debug_proc_fops);
#endif

	return 0;

err_usb_irq:
#ifdef SUPPORT_USB_STATE
	if (pdata->usb_valid)
		free_irq(gpio_to_irq(pdata->uok), data);
err_dc_irq:
	if (pdata->dc_valid)
		free_irq(gpio_to_irq(pdata->dok), data);
#endif
err_psy:
	power_supply_unregister(battery);
	power_supply_unregister(usb);
	power_supply_unregister(adapter);
err:
	kfree(data);
	return ret;
}

static __devexit int max8903_remove(struct platform_device *pdev)
{
	struct max8903_data *data = platform_get_drvdata(pdev);

	if (data) {
		struct max8903_pdata *pdata = data->pdata;

		if (pdata->flt)
			free_irq(gpio_to_irq(pdata->flt), data);
#ifdef SUPPORT_USB_STATE
		if (pdata->usb_valid)
			free_irq(gpio_to_irq(pdata->uok), data);
#endif
		if (pdata->dc_valid)
			free_irq(gpio_to_irq(pdata->dok), data);

		power_supply_unregister(&data->battery);
		power_supply_unregister(&data->usb);
		power_supply_unregister(&data->adapter);

		kfree(data);
	}

	return 0;
}

static struct platform_driver max8903_driver = {
	.probe	= max8903_probe,
	.remove	= __devexit_p(max8903_remove),
	.suspend = max8903_suspend,
	.resume = max8903_resume,
	.driver = {
		.name	= "max8903-charger",
		.owner	= THIS_MODULE,
	},
};

static int __init max8903_init(void)
{
	return platform_driver_register(&max8903_driver);
}
module_init(max8903_init);

static void __exit max8903_exit(void)
{
	platform_driver_unregister(&max8903_driver);
}
module_exit(max8903_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MAX8903 Charger Driver");
MODULE_AUTHOR("MyungJoo Ham <myungjoo.ham@samsung.com>");
MODULE_ALIAS("max8903-charger");
