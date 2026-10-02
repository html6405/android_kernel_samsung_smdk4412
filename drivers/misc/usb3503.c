/*
 * drivers/misc/usb3503.c - usb3503 usb hub driver
 */
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/platform_data/usb3503.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/regulator/consumer.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/pm_runtime.h>
#include <plat/devs.h>
#include <plat/ehci.h>
#include <linux/switch.h>
#include <linux/wakelock.h>
#include <plat/adc.h>
#include <plat/gpio-cfg.h>

#if USB3503_DOCK_SWITCH
struct switch_dev switch_dock_detection = {
	.name = "dock",
};
#endif

#if DOCK_LINEOUT_JACK_SWITCH
struct switch_dev switch_dock_lineout_jack_detection = {
	.name = "usb_audio",
};
#endif

#if USE_WAKELOCK_CONTROL
static struct wake_lock dock_wakelock;
#endif

struct s3c_adc_client *dock_adc_client;

#define DOCK_SWITCH_DETACH			0
#define DOCK_SWITCH_ATTACH			1
#define DOCK_SWITCH_ATTACH_VALUE		5

#if HUB_UNIDENTIFIED_RECOVERY
void usb_host_pwr_enable(struct usb3503_hubctl *hc, int enable, int force_change);
extern void udc_power_control(int enable);
extern int ehci_power_control(struct device *dev, int force_enable,const char *buf);
static atomic_t dock_hub_connected = ATOMIC_INIT(0);

void dock_hub_status(int is_dock)
{
	atomic_set(&dock_hub_connected, is_dock);
}
int check_dock_hub(void)
{
	return atomic_read(&dock_hub_connected);
}
static void unidentified_recovery_work_func(struct work_struct *work)
{
	struct usb3503_hubctl *hc = container_of(work, struct usb3503_hubctl, recovery_work);

	if(!check_dock_hub())
	{
		pr_err(HUB_TAG "Start unidentified_recovery!...\n");
		udc_power_control(0);
		ehci_power_control(NULL, 0, NULL);
		msleep(20);
		ehci_power_control(NULL, 1, NULL);
		usb_host_pwr_enable(hc, 1, 1);
		udc_power_control(1);
	}
}
#endif


#if USB3503_DOCK_SWITCH
void change_dock_switch_state(int conn)
{
	int curstat  = switch_get_state(&switch_dock_detection);
	if(curstat==DOCK_SWITCH_ATTACH_VALUE) curstat = DOCK_SWITCH_ATTACH;
	if(conn == curstat){
		pr_info(HUB_TAG "[%s] same status!! (%d) \n",__func__,conn);
		return;
	}
	if(conn){
		switch_set_state(&switch_dock_detection, DOCK_SWITCH_ATTACH_VALUE);
	}else if(curstat && !conn){
		switch_set_state(&switch_dock_detection, DOCK_SWITCH_DETACH);
	}
}
EXPORT_SYMBOL(change_dock_switch_state);
#endif

#if USB3503_I2C_CONTROL
static int usb3503_register_write(struct i2c_client *i2c_dev, char reg,
	char data)
{
	int ret;
	char buf[2];
	struct i2c_msg msg[] = {
		{
			.addr = i2c_dev->addr,
			.flags = 0,
			.len = 2,
			.buf = buf,
		},
	};

	buf[0] = reg;
	buf[1] = data;

	ret = i2c_transfer(i2c_dev->adapter, msg, 1);
	if (ret < 0)
		pr_err(HUB_TAG "%s: reg: %x data: %x write failed\n",
		__func__, reg, data);

	return ret;
}

static int usb3503_register_read(struct i2c_client *i2c_dev, char reg,
	char *data)
{
	int ret;
	struct i2c_msg msgs[] = {
		{
			.addr = i2c_dev->addr,
			.flags = 0,
			.len = 1,
			.buf = &reg,
		},
		{
			.addr = i2c_dev->addr,
			.flags = I2C_M_RD,
			.len = 1,
			.buf = data,
		},
	};

	ret = i2c_transfer(i2c_dev->adapter, msgs, 2);
	if (ret < 0)
		pr_err(HUB_TAG "%s: reg: %x read failed\n", __func__, reg);

	return ret;
}

static int reg_write(struct i2c_client *i2c_dev, char reg, char req, int retry)
{
	int cnt = retry, err;
	char data = 0;

	pr_debug(HUB_TAG "%s: write %02X, data: %02x\n", __func__, reg, req);
	do {
		err = usb3503_register_write(i2c_dev, reg, req);
		if (err < 0) {
			pr_err(HUB_TAG "%s: usb3503_register_write failed"
					" - retry(%d)", __func__, cnt);
			continue;
		}

		err = usb3503_register_read(i2c_dev, reg, &data);
		if (err < 0)
			pr_err(HUB_TAG "%s: usb3503_register_read failed"
					" - retry(%d)", __func__, cnt);
	} while (data != req && cnt--);

	pr_info(HUB_TAG "%s: write %02X, req:%02x, val:%02x\n", __func__, reg,
		req, data);

	return err;
}

static int reg_update(struct i2c_client *i2c_dev, char reg, char req, int retry)
{
	int cnt = retry, err;
	char data;

	pr_debug(HUB_TAG "%s: update %02X, data: %02x\n", __func__, reg, req);
	do {
		err = usb3503_register_read(i2c_dev, reg, &data);
		if (err < 0) {
			pr_err(HUB_TAG "%s: usb3503_register_read failed"
					" - retry(%d)", __func__, cnt);
			continue;
		}

		pr_debug(HUB_TAG "%s: read %02X, data: %02x\n", __func__, reg,
			data);
		if ((data & req) == req) {
			pr_debug(HUB_TAG "%s: aleady set data: %02x\n",
				__func__, data);
			break;
		}
		err = usb3503_register_write(i2c_dev, reg, data | req);
		if (err < 0)
			pr_err(HUB_TAG "%s: usb3503_register_write failed"
					" - retry(%d)", __func__, cnt);
	} while (cnt--);

	pr_info(HUB_TAG "%s: update %02X, req:%02x, val:%02x\n", __func__, reg,
		req, data);
	return err;
}

static int reg_clear(struct i2c_client *i2c_dev, char reg, char req, int retry)
{
	int cnt = retry, err;
	char data;

	pr_debug(HUB_TAG "%s: clear %X, data %x\n", __func__, reg, req);
	do {
		err = usb3503_register_read(i2c_dev, reg, &data);
		if (err < 0)
			goto exit;
		pr_debug(HUB_TAG "%s: read %02X, data %02x\n", __func__, reg,
			data);
		if (!(data & req)) {
			pr_err(HUB_TAG "%s: aleady cleared data = %02x\n",
				__func__, data);
			break;
		}
		err = usb3503_register_write(i2c_dev, reg, data & ~req);
		if (err < 0)
			goto exit;
	} while (cnt--);
exit:
	pr_info(HUB_TAG "%s: clear %02X, req:%02x, val:%02x\n", __func__, reg,
		req, data);
	return err;
}
#endif

int check_dock_adc_value(void)
{
	int i;
	int adc, adc_sum = 0;
	int adc_buff[ADC_SAMPLING_NUM] = {0};
	int adc_min = 0, adc_max = 0;

	if (IS_ERR(dock_adc_client)) {
		return DOCK_ADC_VALUE_MAX-1;
	}

	for(i = 0; i < ADC_SAMPLING_NUM; i++)
	{
		adc_buff[i] = s3c_adc_read(dock_adc_client, WILLOW_DOCK_ADC_ID);
		pr_debug(HUB_TAG "[dock] CNT : %d. ADC value = 0x%03x (%d)\n", i, adc_buff[i],adc_buff[i]);		
		adc_sum +=adc_buff[i];
		if(i == 0)
		{
			adc_min = adc_buff[0];
			adc_max = adc_buff[0];
		}
		else
		{
			if(adc_max < adc_buff[i])
				adc_max = adc_buff[i];
			else if(adc_min > adc_buff[i])
				adc_min = adc_buff[i];
		}

		if(i < ADC_SAMPLING_NUM - 1)
			msleep(20);
	}

	adc = (adc_sum - adc_max - adc_min)/(ADC_SAMPLING_NUM-2);
	pr_info(HUB_TAG "[%s] DOCK : ADC average value = 0x%03x (%d) !!\n",__func__, adc, adc);
	return adc;
}

void usb_host_pwr_enable(struct usb3503_hubctl *hc, int enable, int force_change){

	if(!force_change && hc->host_pwr_enabled == enable) return;
	hc->host_pwr_enabled  = enable;

	if(enable){
#ifdef CONFIG_USB_EHCI_S5P
		hc->reset_n(0);
		s5p_ehci_port_control(&s5p_device_ehci, 2, 0);
		hc->reset_n(1);
		s5p_ehci_port_control(&s5p_device_ehci, 2, 1);
#endif
#ifdef CONFIG_USB_EHCI_S5P
		pm_runtime_get_sync(&s5p_device_ehci.dev);
#endif
#ifdef CONFIG_USB_OHCI_S5P
		pm_runtime_get_sync(&s5p_device_ohci.dev);
#endif
	}else{
#ifdef CONFIG_USB_EHCI_S5P
		hc->reset_n(0);
		s5p_ehci_port_control(&s5p_device_ehci, 2, 0);
#endif
#ifdef CONFIG_USB_OHCI_S5P
		pm_runtime_put(&s5p_device_ohci.dev);
#endif
#ifdef CONFIG_USB_EHCI_S5P
		pm_runtime_put(&s5p_device_ehci.dev);
#endif
	}
}

static void usb3503_change_status(struct usb3503_hubctl *hc, int force_detached) {
	hc->new_dock_status = gpio_get_value(hc->usb_doc_det);
	pr_debug(HUB_TAG "[%s] cur : %d, new : %d\n",__func__,hc->cur_dock_status,hc->new_dock_status);

	if (force_detached)
		hc->cur_dock_status = DOCK_STATE_DETACHED;
	else if (hc->cur_dock_status == hc->new_dock_status){
		//pr_info("[%s] same status!!!\n",__func__);
		return;
	}
	else
		hc->cur_dock_status = hc->new_dock_status;

	if (hc->cur_dock_status == DOCK_STATE_ATTACHED) {
		//pr_info(HUB_TAG"[%s] DOCK_STATE_ATTACHED @@@ \n",__func__);
		usb_host_pwr_enable(hc, 1, 0);
		int adc =  check_dock_adc_value();
		if(adc < DOCK_ADC_VALUE_MAX){
			//dock
			hc->is_dock = 1;
#if HUB_UNIDENTIFIED_RECOVERY
			schedule_delayed_work(&hc->recovery_work,   msecs_to_jiffies(UNIDENTIFY_CHECK_TIME));
#endif

#if USE_WAKELOCK_CONTROL
			wake_lock(&dock_wakelock);
#endif

#if USB3503_DOCK_SWITCH
			change_dock_switch_state(DOCK_SWITCH_ATTACH);
#endif
		}else{
			//others
			hc->is_dock = 0;
		}
	}
	else if(hc->cur_dock_status == DOCK_STATE_DETACHED)
	{
		//pr_info(HUB_TAG "[%s] STATE_DETACHED ### (%d)\n",__func__,hc->is_dock);
		if(hc->is_dock == 1){
			hc->is_dock = 0;
#if HUB_UNIDENTIFIED_RECOVERY
			dock_hub_status(0);
			cancel_delayed_work_sync(&hc->recovery_work);
#endif

#if USE_WAKELOCK_CONTROL
			wake_unlock(&dock_wakelock);
#endif

#if USB3503_DOCK_SWITCH
			change_dock_switch_state(DOCK_SWITCH_DETACH);
#endif

#if DOCK_LINEOUT_JACK_SWITCH
			switch_set_state(&switch_dock_lineout_jack_detection, 0);
#endif
		}
	}
	else
	{
		pr_err(HUB_TAG "[%s] invalid state !!! \n",__func__);
	}
}

static void usb3503_dock_worker(struct work_struct *work)
{
	struct usb3503_hubctl *hc = container_of(work, struct usb3503_hubctl, dock_work);

	usb3503_change_status(hc, 0);
	enable_irq(hc->dock_irq);
}

static irqreturn_t usb3503_dock_irq_thread(int irq, void *data) {
	struct usb3503_hubctl *hc = data;

	disable_irq_nosync(hc->dock_irq);
	queue_work(hc->workqueue, &hc->dock_work);

	return IRQ_HANDLED;
}

void check_dock_lineout_jack_status(struct usb3503_hubctl *hc, bool force_check)
{
	hc->new_lineout_jack_status = gpio_get_value(hc->lineout_jack_det);

	pr_info(HUB_TAG "[%s] ## cur (%d) new (%d)\n",__func__,hc->cur_lineout_jack_status,hc->new_lineout_jack_status);

	if(!force_check && hc->cur_lineout_jack_status == hc->new_lineout_jack_status){
		pr_debug("[%s] same status!!!\n",__func__);
		return;
	}

	hc->cur_lineout_jack_status = hc->new_lineout_jack_status;

	if(hc->cur_lineout_jack_status == DOCK_STATE_ATTACHED)
	{
		pr_debug(HUB_TAG "[%s] LINEOUT JACK DOCK_STATE_ATTACHED @\n",__func__);
#if DOCK_LINEOUT_JACK_SWITCH
		//android  : BIT_USB_HEADSET_ANLG (0x1)
		switch_set_state(&switch_dock_lineout_jack_detection, 0x01);
#endif
	}
	else if(hc->cur_lineout_jack_status == DOCK_STATE_DETACHED)
	{
		pr_debug(HUB_TAG "[%s] LINEOUT JACK DOCK_STATE_DETACHED #\n",__func__);
#if DOCK_LINEOUT_JACK_SWITCH
		switch_set_state(&switch_dock_lineout_jack_detection, 0);
#endif
	}
	else
	{
		pr_err(HUB_TAG "[%s] invalid state !!! \n",__func__);
	}

}

void dock_lineout_jack_intr_handle(struct work_struct *work)
{
	struct usb3503_hubctl *hc = container_of(work, struct usb3503_hubctl, lineout_jack_work);
	check_dock_lineout_jack_status(hc, 0);
	enable_irq(hc->lineout_jack_irq);
}

irqreturn_t dock_lineout_jack_thread(int irq, void *data)
{
	struct usb3503_hubctl *hc = (struct usb3503_hubctl *)data;

	disable_irq_nosync(hc->lineout_jack_irq);
	queue_work(hc->lineout_jack_workqueue, &hc->lineout_jack_work);

	return IRQ_HANDLED;
}

#if USB3503_I2C_CONTROL
static int usb3503_set_mode(struct usb3503_hubctl *hc, int mode)
{
	int err = 0;
	struct i2c_client *i2c_dev = hc->i2c_dev;

	pr_info(HUB_TAG "%s: mode = %d\n", __func__, mode);

	switch (mode) {
	case USB3503_MODE_HUB:
		hc->reset_n(1);

		/* SP_ILOCK: set connect_n, config_n for config */
		err = reg_write(i2c_dev, SP_ILOCK_REG,
			(SPILOCK_CONNECT_N | SPILOCK_CONFIG_N), 3);
		if (err < 0) {
			pr_err(HUB_TAG "SP_ILOCK write fail err = %d\n", err);
			goto exit;
		}
#ifdef USB3503_ES_VER
/* ES version issue
 * USB3503 can't PLL power up under cold circumstance, so enable
 * the Force suspend clock bit
 */
		err = reg_update(i2c_dev, CFGP_REG, CFGP_CLKSUSP, 1);
		if (err < 0) {
			pr_err(HUB_TAG "CFGP update fail err = %d\n", err);
			goto exit;
		}
#endif
		/* PDS : Port1,3 Disable For Self Powered Operation */
		err = reg_update(i2c_dev, PDS_REG, (PDS_PORT1 | PDS_PORT3), 1);
		if (err < 0) {
			pr_err(HUB_TAG "PDS update fail err = %d\n", err);
			goto exit;
		}
		/* CFG1 : SELF_BUS_PWR -> Self-Powerd operation */
		err = reg_update(i2c_dev, CFG1_REG, CFG1_SELF_BUS_PWR, 1);
		if (err < 0) {
			pr_err(HUB_TAG "CFG1 update fail err = %d\n", err);
			goto exit;
		}
		/* SP_LOCK: clear connect_n, config_n for hub connect */
		err = reg_clear(i2c_dev, SP_ILOCK_REG,
			(SPILOCK_CONNECT_N | SPILOCK_CONFIG_N), 1);
		if (err < 0) {
			pr_err(HUB_TAG "SP_ILOCK clear err = %d\n", err);
			goto exit;
		}
		hc->mode = mode;

		/* Should be enable the HSIC port1 */

		break;

	case USB3503_MODE_STANDBY:
		hc->reset_n(0);
		hc->mode = mode;
		break;

	default:
		pr_err(HUB_TAG "%s: Invalid mode %d\n", __func__, mode);
		err = -EINVAL;
		goto exit;
		break;
	}
exit:
	return err;
}

/* sysfs for control */
static ssize_t mode_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct usb3503_hubctl *hc = dev_get_drvdata(dev);

	if (hc->mode == USB3503_MODE_HUB)
		return sprintf(buf, "%s", "hub");
	else if (hc->mode == USB3503_MODE_STANDBY)
		return sprintf(buf, "%s", "standby");

	return 0;
}

static ssize_t mode_store(
		struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
	struct usb3503_hubctl *hc = dev_get_drvdata(dev);

	if (!strncmp(buf, "hub", 3)) {
		if (hc->port_enable)
			hc->port_enable(1, 1);
		pr_debug(HUB_TAG "mode set to hub\n");
	} else if (!strncmp(buf, "standby", 7)) {
		if (hc->port_enable)
			hc->port_enable(1, 0);
		pr_debug(HUB_TAG "mode set to standby\n");
	}
	return size;
}
static DEVICE_ATTR(mode, 0664, mode_show, mode_store);
#endif

int usb3503_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct usb3503_hubctl *hc = i2c_get_clientdata(client);

	usb_host_pwr_enable(hc, 0, 0);

#if HUB_UNIDENTIFIED_RECOVERY
	cancel_delayed_work_sync(&hc->recovery_work);
#endif

	pr_debug(HUB_TAG "suspended\n");

	return 0;
}

int usb3503_resume(struct i2c_client *client)
{
	struct usb3503_hubctl *hc = i2c_get_clientdata(client);

	if(gpio_get_value(hc->usb_doc_det)==DOCK_STATE_ATTACHED){
		pr_debug(HUB_TAG "[%s] dock is connected. usb_host_pwr_enable(1)!\n",__func__);
		usb_host_pwr_enable(hc, 1, 0);
	}

#if USB3503_I2C_CONTROL
	if (hc->mode == USB3503_MODE_HUB)
		usb3503_set_mode(hc, USB3503_MODE_HUB);

	pr_debug(HUB_TAG "resume mode=%s", (hc->mode == USB3503_MODE_HUB) ?
		"hub" : "standny");
#endif

	return 0;
}

int usb3503_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err = 0;
	struct usb3503_hubctl *hc;
	struct usb3503_platform_data *pdata;

	pr_info(HUB_TAG "%s:%d\n", __func__, __LINE__);

	hc = kzalloc(sizeof(struct usb3503_hubctl), GFP_KERNEL);
	if (!hc) {
		pr_err(HUB_TAG "private data alloc fail\n");
		err = -ENOMEM;
		goto err_kzalloc;
	}

#if USB3503_DOCK_SWITCH
	err = switch_dev_register(&switch_dock_detection);
	if (err < 0) {
		pr_err(HUB_TAG "%s : Failed to register switch device (switch_dock_detection)\n", __func__);
		goto err_switch_dev_register1;
	}
#endif

#if DOCK_LINEOUT_JACK_SWITCH
	if (switch_dev_register(&switch_dock_lineout_jack_detection)) {
		pr_err(HUB_TAG "%s : Failed to register switch device (switch_dock_lineout_jack_detection) \n", __func__);
		goto err_switch_dev_register2;
	}
#endif

#if USE_WAKELOCK_CONTROL
	wake_lock_init(&dock_wakelock, WAKE_LOCK_SUSPEND, "dock_wakelock");
#endif

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit1;
	}

	pdata = client->dev.platform_data;
	if (pdata == NULL) {
		pr_err(HUB_TAG "device's platform data is NULL!\n");
		err = -ENODEV;
		goto exit1;
	}

	hc->cur_dock_status = DOCK_STATE_UNKNOWN;
	hc->new_dock_status = DOCK_STATE_UNKNOWN;
	hc->usb_doc_det = pdata->usb_doc_det;

	hc->cur_lineout_jack_status = DOCK_STATE_UNKNOWN;
	hc->new_lineout_jack_status = DOCK_STATE_UNKNOWN;
	hc->lineout_jack_det = GPIO_LINEOUT_DET_N;

	hc->i2c_dev = client;
	hc->reset_n = pdata->reset_n;
#if USB3503_I2C_CONTROL
	hc->port_enable = pdata->port_enable;
	if (pdata->initial_mode) {
		usb3503_set_mode(hc, pdata->initial_mode);
		hc->mode = pdata->initial_mode;
	}

	if (pdata->register_hub_handler)
		pdata->register_hub_handler((void (*)(void))usb3503_set_mode,
			(void *)hc);
#endif

	dock_adc_client = s3c_adc_register((struct platform_device *)client, NULL, NULL, 0);
	if (IS_ERR(dock_adc_client)) {
		pr_err("%s  : failed to register dock_adc_client!n",__func__);
	}

#if HUB_UNIDENTIFIED_RECOVERY
	INIT_DELAYED_WORK(&hc->recovery_work, unidentified_recovery_work_func);
#endif

	hc->workqueue = create_singlethread_workqueue(USB3503_I2C_NAME);
	INIT_WORK(&hc->dock_work, usb3503_dock_worker);

	hc->dock_irq = gpio_to_irq(pdata->usb_doc_det);
	if (!hc->dock_irq) {
		pr_err(HUB_TAG "Failed to get USB_DOCK_DET IRQ\n");
		err = -ENODEV;
		goto err_usb_doc_det;
	}
	err = request_irq(hc->dock_irq, usb3503_dock_irq_thread,
			IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING|IRQF_ONESHOT, "USB_DOCK_DET", hc);
	if (err) {
		pr_err(HUB_TAG "Failed to allocate an USB_DOCK_DET interrupt(%d)\n",
				hc->dock_irq);
		goto err_usb_doc_det;
	}

	hc->lineout_jack_workqueue = create_singlethread_workqueue("lineout_jack_workqueue");
	INIT_WORK(&hc->lineout_jack_work, dock_lineout_jack_intr_handle);
	err = gpio_request(hc->lineout_jack_det, "GPIO_LINEOUT_DET_N");
	if (err < 0) {
		pr_err(HUB_TAG "gpio_request failed for GPIO_DOCK_DET_N \n");
		goto err_lineout_jack_det_req;
	}
	s3c_gpio_cfgpin(hc->lineout_jack_det, S3C_GPIO_SFN(0xf));
	s3c_gpio_setpull(hc->lineout_jack_det, S3C_GPIO_PULL_NONE);
	//pr_info(HUB_TAG "[dock] is %s\n",	previous_dock_lineout_jack_state ? "removed/not connected" : "connected");
	hc->lineout_jack_irq = gpio_to_irq(hc->lineout_jack_det);
	if (!hc->lineout_jack_irq) {
		pr_err(HUB_TAG "Failed to get GPIO_LINEOUT_DET_N IRQ\n");
		err = -ENODEV;
		goto err_lineout_jack_det;
	}
	err = request_irq(hc->lineout_jack_irq, dock_lineout_jack_thread,
		IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING|IRQF_ONESHOT, "Dock lineout jack Detected", hc);
	if (err < 0){
		pr_err(HUB_TAG "Failed to allocate an GPIO_LINEOUT_DET_N interrupt(%d)\n", hc->lineout_jack_irq);
		goto err_lineout_jack_det;
	}

	i2c_set_clientdata(client, hc);

#if USB3503_I2C_CONTROL
	err = device_create_file(&client->dev, &dev_attr_mode);
	pr_info(HUB_TAG "%s: probed on  %s mode\n", __func__,
		(hc->mode == USB3503_MODE_HUB) ? "hub" : "standby");
#endif

	return 0;

err_lineout_jack_det:
	gpio_free(hc->lineout_jack_det);
err_lineout_jack_det_req:
	cancel_work_sync(&hc->lineout_jack_work);
err_usb_doc_det:
	gpio_free(hc->usb_doc_det);
	cancel_work_sync(&hc->dock_work);
#if HUB_UNIDENTIFIED_RECOVERY
	cancel_delayed_work_sync(&hc->recovery_work);
#endif
exit1:
#if USE_WAKELOCK_CONTROL
	wake_lock_destroy(&dock_wakelock);
#endif
#if DOCK_LINEOUT_JACK_SWITCH
	switch_dev_unregister(&switch_dock_lineout_jack_detection);
#endif
err_switch_dev_register2:
#if USB3503_DOCK_SWITCH
	switch_dev_unregister(&switch_dock_detection);
#endif
err_switch_dev_register1:
	kfree(hc);
err_kzalloc:
	return err;
}

static int usb3503_remove(struct i2c_client *client)
{
	struct usb3503_hubctl *hc = i2c_get_clientdata(client);

#if USE_WAKELOCK_CONTROL
	wake_lock_destroy(&dock_wakelock);
#endif

	pr_debug(HUB_TAG "%s\n", __func__);
#if USB3503_DOCK_SWITCH
	switch_dev_unregister(&switch_dock_detection);
#endif

#if DOCK_LINEOUT_JACK_SWITCH
	switch_dev_unregister(&switch_dock_lineout_jack_detection);
#endif

	gpio_free(hc->usb_doc_det);
	free_irq(hc->dock_irq, hc->i2c_dev);
	cancel_work_sync(&hc->dock_work);

	gpio_free(hc->lineout_jack_det);
	free_irq(hc->lineout_jack_irq, hc->i2c_dev);
	cancel_work_sync(&hc->dock_work);

#if HUB_UNIDENTIFIED_RECOVERY
	cancel_delayed_work_sync(&hc->recovery_work);
#endif

	if (!IS_ERR(dock_adc_client))
		s3c_adc_release(dock_adc_client);
	kfree(hc);

	return 0;
}

static const struct i2c_device_id usb3503_id[] = {
	{ USB3503_I2C_NAME, 0 },
	{ }
};

static struct i2c_driver usb3503_driver = {
	.probe = usb3503_probe,
	.remove = usb3503_remove,
	.suspend = usb3503_suspend,
	.resume = usb3503_resume,
	.id_table = usb3503_id,
	.driver = {
		.name = USB3503_I2C_NAME,
	},
};

static int __init usb3503_init(void)
{
	pr_info(HUB_TAG "USB HUB driver init\n");
	return i2c_add_driver(&usb3503_driver);
}

static void __exit usb3503_exit(void)
{
	pr_info(HUB_TAG "USB HUB driver exit\n");
	i2c_del_driver(&usb3503_driver);
}
late_initcall(usb3503_init);
module_exit(usb3503_exit);

MODULE_DESCRIPTION("USB3503 USB HUB driver");
MODULE_LICENSE("GPL");
