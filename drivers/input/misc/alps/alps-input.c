/* alps-input.c
 *
 * Input device driver for alps sensor
 *
 * Copyright (C) 2011-2012 ALPS ELECTRIC CO., LTD. All Rights Reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/input-polldev.h>
#if 1 /* willow: earlysuspend shim */
#include <linux/earlysuspend.h>
#endif

#include <asm/uaccess.h> 
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include "alps_compass_io.h"

extern int accsns_get_acceleration_data(int *xyz);
extern int hscd_get_magnetic_field_data(int *xyz);
extern void hscd_activate(int flgatm, int flg, int dtime);
extern void accsns_activate(int flgatm, int flg);
extern int hscd_self_test_A(void);
extern int hscd_self_test_B(void);

static DEFINE_MUTEX(alps_lock);

static struct platform_device *pdev;
static struct input_polled_dev *alps_idev;
#if 1 /* willow: earlysuspend shim */
static struct early_suspend alps_early_suspend_handler;
#endif

#define EVENT_TYPE_ACCEL_X          ABS_X
#define EVENT_TYPE_ACCEL_Y          ABS_Y
#define EVENT_TYPE_ACCEL_Z          ABS_Z
#define EVENT_TYPE_ACCEL_STATUS     ABS_WHEEL

#define EVENT_TYPE_MAGV_X           ABS_HAT0X
#define EVENT_TYPE_MAGV_Y           ABS_HAT0Y
#define EVENT_TYPE_MAGV_Z           ABS_BRAKE

#define ALPS_POLL_INTERVAL   100    /* msecs */
#define ALPS_INPUT_FUZZ        0    /* input event threshold */
#define ALPS_INPUT_FLAT        0

#define POLL_STOP_TIME       400    /* (msec) */

static int flgM = 0, flgA = 0;
static int flgSuspend = 0;
static int delay = 200;
static int poll_stop_cnt = 0;

///////////////////////////////////////////////////////////////////////////////
// for I/O Control

static long alps_ioctl(struct file* filp, unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *)arg;
    int ret = -1, tmpval;

    switch (cmd) {
        case ALPSIO_SET_MAGACTIVATE:
            ret = copy_from_user(&tmpval, argp, sizeof(tmpval));
            if (ret) {
                printk("error : alps_ioctl(cmd = ALPSIO_SET_MAGACTIVATE)\n" );
                return -EFAULT;
            }
#ifdef ALPS_DEBUG
            printk("alps_ioctl(cmd = ALPSIO_SET_MAGACTIVATE), flgM = %d\n", tmpval);
#endif
            mutex_lock(&alps_lock);
            flgM = tmpval;
            hscd_activate(1, tmpval, delay);
            mutex_unlock(&alps_lock);
            break;

        case ALPSIO_SET_ACCACTIVATE:
            ret = copy_from_user(&tmpval, argp, sizeof(tmpval));
            if (ret) {
                printk("error : alps_ioctl(cmd = ALPSIO_SET_ACCACTIVATE)\n");
                return -EFAULT;
            }
#ifdef ALPS_DEBUG
            printk("alps_ioctl(cmd = ALPSIO_SET_ACCACTIVATE), flgA = %d\n", tmpval);
#endif
            mutex_lock(&alps_lock);
            flgA = tmpval;
            accsns_activate(1, flgA);
            mutex_unlock(&alps_lock);
            break;

        case ALPSIO_SET_DELAY:
            ret = copy_from_user(&tmpval, argp, sizeof(tmpval));
            if (ret) {
                printk( "error : alps_ioctl(cmd = ALPSIO_SET_DELAY)\n" );
                return -EFAULT;
            }
#ifdef ALPS_DEBUG
            printk("alps_ioctl(cmd = ALPSIO_SET_DELAY)\n");
#endif
            mutex_lock(&alps_lock);
            if (flgM) {
                if      (tmpval <=  10) tmpval =  10;
                else if (tmpval <=  20) tmpval =  20;
                else if (tmpval <=  70) tmpval =  70;
                else                    tmpval = 200;
            }
            else {
                if      (tmpval <=  10) tmpval =  10;
            }
            delay = tmpval;
            poll_stop_cnt = POLL_STOP_TIME / tmpval;
            hscd_activate(1, flgM, delay);
            mutex_unlock(&alps_lock);
#ifdef ALPS_DEBUG
            printk("     delay = %d\n", delay);
#endif
            break;

        case ALPSIO_ACT_SELF_TEST_A:
#ifdef ALPS_DEBUG
               printk("alps_ioctl(cmd = ALPSIO_ACT_SELF_TEST_A)\n");
#endif
            mutex_lock(&alps_lock);
            ret = hscd_self_test_A();
            mutex_unlock(&alps_lock);
#ifdef ALPS_DEBUG
            printk("[HSCD] Self test-A result : %d\n", ret);
#endif
            if (copy_to_user(argp, &ret, sizeof(ret))) {
                printk( "error : alps_ioctl(cmd = ALPSIO_ACT_SELF_TEST_A)\n" );
                return -EFAULT;
            }
            break;

        case ALPSIO_ACT_SELF_TEST_B:
#ifdef ALPS_DEBUG
               printk("alps_ioctl(cmd = ALPSIO_ACT_SELF_TEST_B)\n");
#endif
            mutex_lock(&alps_lock);
            ret = hscd_self_test_B();
            mutex_unlock(&alps_lock);
#ifdef ALPS_DEBUG
            printk("[HSCD] Self test-B result : %d\n", ret);
#endif
            if (copy_to_user(argp, &ret, sizeof(ret))) {
                printk( "error : alps_ioctl(cmd = ALPSIO_ACT_SELF_TEST_B)\n" );
                return -EFAULT;
            }
            break;

        default:
            return -ENOTTY;
    }
    return 0;
}

static int 
alps_io_open( struct inode* inode, struct file* filp )
{
    return 0;
}

static int 
alps_io_release( struct inode* inode, struct file* filp )
{
    return 0;
}

static struct file_operations alps_fops = {
    .owner   = THIS_MODULE,
    .open    = alps_io_open,
    .release = alps_io_release,
    .unlocked_ioctl = alps_ioctl,
};

static struct miscdevice alps_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "alps_compass_io",
    .fops  = &alps_fops,
};


///////////////////////////////////////////////////////////////////////////////
// for input device

static int alps_probe(struct platform_device *dev)
{
    printk(KERN_INFO "alps: alps_probe\n");
    return 0;
}

static int alps_remove(struct platform_device *dev)
{
    printk(KERN_INFO "alps: alps_remove\n");
    return 0;
}

#if 1 /* willow: earlysuspend shim */
static void alps_early_suspend(struct early_suspend *handler)
{
#ifdef ALPS_DEBUG
    printk("alps-input: early_suspend\n");
#endif
    mutex_lock(&alps_lock);
    flgSuspend = 1;
    mutex_unlock(&alps_lock);
}

static void alps_early_resume(struct early_suspend *handler)
{
#ifdef ALPS_DEBUG
    printk("alps-input: early_resume\n");
#endif
    mutex_lock(&alps_lock);
    poll_stop_cnt = POLL_STOP_TIME / delay;
    flgSuspend = 0;
    mutex_unlock(&alps_lock);
}
#endif

static struct platform_driver alps_driver = {
    .driver    = {
        .name  = "alps-input",
        .owner = THIS_MODULE,
    },
    .probe     = alps_probe,
    .remove    = alps_remove,
};

#if 1 /* willow: earlysuspend shim */
static struct early_suspend alps_early_suspend_handler = {
    .suspend = alps_early_suspend,
    .resume  = alps_early_resume,
};
#endif

static void accsns_poll(struct input_dev *idev)
{
    int xyz[3];

    if(accsns_get_acceleration_data(xyz) == 0) {
        input_report_abs(idev, EVENT_TYPE_ACCEL_X, xyz[0]);
        input_report_abs(idev, EVENT_TYPE_ACCEL_Y, xyz[1]);
        input_report_abs(idev, EVENT_TYPE_ACCEL_Z, xyz[2]);
        idev->sync = 0;
        input_event(idev, EV_SYN, SYN_REPORT, 1);
    }
}

static void hscd_poll(struct input_dev *idev)
{
    int xyz[3];

    if(hscd_get_magnetic_field_data(xyz) == 0) {
        input_report_abs(idev, EVENT_TYPE_MAGV_X, xyz[0]);
        input_report_abs(idev, EVENT_TYPE_MAGV_Y, xyz[1]);
        input_report_abs(idev, EVENT_TYPE_MAGV_Z, xyz[2]);
        idev->sync = 0;
        input_event(idev, EV_SYN, SYN_REPORT, 2);
    }
}


static void alps_poll(struct input_polled_dev *dev)
{
    struct input_dev *idev = dev->input;

    mutex_lock(&alps_lock);
    dev->poll_interval = delay;
    if (!flgSuspend) {
        if (poll_stop_cnt-- < 0) {
            poll_stop_cnt = -1;
            if (flgM) hscd_poll(idev);
            if (flgA) accsns_poll(idev);
        }
#ifdef ALPS_DEBUG
        else printk("pollinf stop. delay = %d, poll_stop_cnt = %d\n", delay, poll_stop_cnt);
#endif
    }
    mutex_unlock(&alps_lock);
}

static int __init alps_init(void)
{
    struct input_dev *idev;
    int ret;

    #include <mach/willow_version.h>
    if(g_willow_hw_version >= WILLOW_HW_MVT){
        printk("alps_init: board revision check ok! (upper MVT) \n");
    }else{
        printk("alps_init: Not use! alps input device not used DVT device!\n");
        return -1;
    }

    ret = platform_driver_register(&alps_driver);
    if (ret){
        printk("alps_init: failt to platform_driver_register!\n");
        goto out_region;
    }
    printk(KERN_INFO "alps-init: platform_driver_register\n");

    pdev = platform_device_register_simple("alps_compass", -1, NULL, 0);
    if (IS_ERR(pdev)) {
        ret = PTR_ERR(pdev);
        goto out_driver;
    }
    printk(KERN_INFO "alps-init: platform_device_register_simple\n");

    alps_idev = input_allocate_polled_device();
    if (!alps_idev) {
        ret = -ENOMEM;
        goto out_device;
    }
    printk(KERN_INFO "alps-init: input_allocate_polled_device\n");

    alps_idev->poll = alps_poll;
    alps_idev->poll_interval = ALPS_POLL_INTERVAL;

    /* initialize the input class */
    idev = alps_idev->input;
    idev->name = "alps_compass";
    idev->phys = "alps_compass/input0";
    idev->id.bustype = BUS_HOST;
    idev->dev.parent = &pdev->dev;
    idev->evbit[0] = BIT_MASK(EV_ABS);

#if defined(ACC_10BIT)
    input_set_abs_params(idev, EVENT_TYPE_ACCEL_X,
            -512, 511, ALPS_INPUT_FUZZ, ALPS_INPUT_FLAT);
    input_set_abs_params(idev, EVENT_TYPE_ACCEL_Y,
            -512, 511, ALPS_INPUT_FUZZ, ALPS_INPUT_FLAT);
    input_set_abs_params(idev, EVENT_TYPE_ACCEL_Z,
            -512, 511, ALPS_INPUT_FUZZ, ALPS_INPUT_FLAT);
#elif defined(ACC_12BIT)
    input_set_abs_params(idev, EVENT_TYPE_ACCEL_X,
            -2048, 2047, ALPS_INPUT_FUZZ, ALPS_INPUT_FLAT);
    input_set_abs_params(idev, EVENT_TYPE_ACCEL_Y,
            -2048, 2047, ALPS_INPUT_FUZZ, ALPS_INPUT_FLAT);
    input_set_abs_params(idev, EVENT_TYPE_ACCEL_Z,
            -2048, 2047, ALPS_INPUT_FUZZ, ALPS_INPUT_FLAT);
#endif

#if defined(MAG_15BIT)
    input_set_abs_params(idev, EVENT_TYPE_MAGV_X,
            -16384, 16383, ALPS_INPUT_FUZZ, ALPS_INPUT_FLAT);
    input_set_abs_params(idev, EVENT_TYPE_MAGV_Y,
            -16384, 16383, ALPS_INPUT_FUZZ, ALPS_INPUT_FLAT);
    input_set_abs_params(idev, EVENT_TYPE_MAGV_Z,
            -16384, 16383, ALPS_INPUT_FUZZ, ALPS_INPUT_FLAT);
#elif defined(MAG_13BIT)
    input_set_abs_params(idev, EVENT_TYPE_MAGV_X,
            -4096, 4095, ALPS_INPUT_FUZZ, ALPS_INPUT_FLAT);
    input_set_abs_params(idev, EVENT_TYPE_MAGV_Y,
            -4096, 4095, ALPS_INPUT_FUZZ, ALPS_INPUT_FLAT);
    input_set_abs_params(idev, EVENT_TYPE_MAGV_Z,
            -4096, 4095, ALPS_INPUT_FUZZ, ALPS_INPUT_FLAT);
#endif

    ret = input_register_polled_device(alps_idev);
    if (ret)
        goto out_alc_poll;
    printk(KERN_INFO "alps-init: input_register_polled_device\n");

    ret = misc_register(&alps_device);
    if (ret) {
        printk("alps-init: alps_io_device register failed\n");
        goto out_reg_poll;
    }
    printk("alps-init: misc_register\n");

#if 1 /* willow: earlysuspend shim */
    register_early_suspend(&alps_early_suspend_handler);
    printk("alps-init: early_suspend_register\n");
#endif
    mutex_lock(&alps_lock);
    flgSuspend = 0;
    mutex_unlock(&alps_lock);

    return 0;

out_reg_poll:
    input_unregister_polled_device(alps_idev);
    printk(KERN_INFO "alps-init: input_unregister_polled_device\n");
out_alc_poll:
    input_free_polled_device(alps_idev);
    printk(KERN_INFO "alps-init: input_free_polled_device\n");
out_device:
    platform_device_unregister(pdev);
    printk(KERN_INFO "alps-init: platform_device_unregister\n");
out_driver:
    platform_driver_unregister(&alps_driver);
    printk(KERN_INFO "alps-init: platform_driver_unregister\n");
out_region:
    return ret;
}

static void __exit alps_exit(void)
{
#if 1 /* willow: earlysuspend shim */
    unregister_early_suspend(&alps_early_suspend_handler);
    printk(KERN_INFO "alps-exit: early_suspend_unregister\n");
#endif
    misc_deregister(&alps_device);
    printk(KERN_INFO "alps-exit: misc_deregister\n");
    input_unregister_polled_device(alps_idev);
    printk(KERN_INFO "alps-exit: input_unregister_polled_device\n");
    input_free_polled_device(alps_idev);
    printk(KERN_INFO "alps-exit: input_free_polled_device\n");
    platform_device_unregister(pdev);
    printk(KERN_INFO "alps-exit: platform_device_unregister\n");
    platform_driver_unregister(&alps_driver);
    printk(KERN_INFO "alps-exit: platform_driver_unregister\n");
}

module_init(alps_init);
module_exit(alps_exit);

MODULE_DESCRIPTION("Alps Input Device");
MODULE_AUTHOR("ALPS ELECTRIC CO., LTD.");
MODULE_LICENSE("GPL v2");
