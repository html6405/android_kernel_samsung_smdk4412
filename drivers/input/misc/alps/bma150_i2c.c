/* bma150_i2c.c
 *
 * Accelerometer device driver for I2C
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
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/slab.h>
#if 1 /* willow: earlysuspend shim */
#include <linux/earlysuspend.h>
#endif

#define I2C_RETRY_DELAY    5
#define I2C_RETRIES        5

/* Register Name for accsns */
#define ACC_XOUT           0x02
#define ACC_YOUT           0x04
#define ACC_ZOUT           0x06
#define ACC_TEMP           0x08
#define ACC_REG0B          0x0B
#define ACC_REG0A          0x0A
#define ACC_REG14          0x14

#define ACC_DRIVER_NAME    "accsns_i2c"
#define I2C_ACC_ADDR       (0x38)        /* 011 1000    */

static struct i2c_driver accsns_driver;
static struct i2c_client *client_accsns = NULL;
#if 1 /* willow: earlysuspend shim */
static struct early_suspend accsns_early_suspend_handler;
#endif

static atomic_t flgEna;
static atomic_t flgSuspend;

static int accsns_i2c_readm(u8 *rxData, int length)
{
    int err;
    int tries = 0;

    struct i2c_msg msgs[] = {
        {
            .addr  = client_accsns->addr,
            .flags = 0,
            .len   = 1,
            .buf   = rxData,
        },
        {
            .addr  = client_accsns->addr,
            .flags = I2C_M_RD,
            .len   = length,
            .buf   = rxData,
         },
    };

    do {
        err = i2c_transfer(client_accsns->adapter, msgs, 2);
    } while ((err != 2) && (++tries < I2C_RETRIES));

    if (err != 2) {
        dev_err(&client_accsns->adapter->dev, "read transfer error\n");
        err = -EIO;
    } else {
        err = 0;
    }

    return err;
}

static int accsns_i2c_writem(u8 *txData, int length)
{
    int err;
    int tries = 0;
#ifdef ALPS_DEBUG
    int i;
#endif

    struct i2c_msg msg[] = {
        {
            .addr  = client_accsns->addr,
            .flags = 0,
            .len   = length,
            .buf   = txData,
        },
    };

#ifdef ALPS_DEBUG
    printk("[ACC] i2c_writem : ");
    for (i=0; i<length;i++) printk("0X%02X, ", txData[i]);
    printk("\n");
#endif

    do {
        err = i2c_transfer(client_accsns->adapter, msg, 1);
    } while ((err != 1) && (++tries < I2C_RETRIES));

    if (err != 1) {
        dev_err(&client_accsns->adapter->dev, "write transfer error\n");
        err = -EIO;
    } else {
        err = 0;
    }

    return err;
}

int accsns_get_acceleration_data(int *xyz)
{
    int err = -1;
    int i;
    u8 sx[6];

    if (atomic_read(&flgSuspend) == 1) return err;
    sx[0] = ACC_XOUT;
    err = accsns_i2c_readm(sx, 6);
    if (err < 0) return err;
    for (i=0; i<3; i++) {
        xyz[i] = (sx[2 * i] >> 6) | (sx[2 * i + 1] << 2);
        if (xyz[i] & 0x200) xyz[i] = (xyz[i] | 0xFFFFFC00);
    }

#ifdef ALPS_DEBUG
    /*** DEBUG OUTPUT - REMOVE ***/
    printk("Acc_I2C, x:%d, y:%d, z:%d\n", xyz[0], xyz[1], xyz[2]);
    /*** <end> DEBUG OUTPUT - REMOVE ***/
#endif

    return err;
}

void accsns_activate(int flgatm, int flg)
{
    u8 buf[2];

    if (flg != 0) flg = 1;

    buf[0] = ACC_REG14    ; buf[1] = 0;
    accsns_i2c_writem(buf, 2);
    buf[0] = ACC_REG0B    ; buf[1] = 0;
    accsns_i2c_writem(buf, 2);
    buf[0] = ACC_REG0A;
    if (flg == 0) buf[1] = 0x01;
    else          buf[1] = 0x00;
    accsns_i2c_writem(buf, 2);
    mdelay(2);
    if (flgatm) atomic_set(&flgEna, flg);
}

static void accsns_register_init(void)
{
    int d[3];
    u8  buf[2];

#ifdef ALPS_DEBUG
    printk("[ACC] register_init\n");
#endif

    buf[0] = ACC_REG0A;
    buf[1] = 0x02;
    accsns_i2c_writem(buf, 2);
    mdelay(4);

    accsns_activate(0, 1);
    accsns_get_acceleration_data(d);
#ifdef ALPS_DEBUG	
    printk("[ACC] x:%d y:%d z:%d\n",d[0],d[1],d[2]);
#endif
    accsns_activate(0, 0);
}

static int accsns_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
#ifdef ALPS_DEBUG
    printk("[ACC] probe\n");
#endif
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
        dev_err(&client->adapter->dev, "client not i2c capable\n");
        return -ENOMEM;
    }

    client_accsns = client;
    i2c_set_clientdata(client, &client_accsns);
    dev_info(&client->adapter->dev, "detected bam150 accelerometer\n");

    accsns_register_init();

    return 0;
}

static int __devexit accsns_remove(struct i2c_client *client)
{
#ifdef ALPS_DEBUG
    printk("[ACC] remove\n");
#endif
    accsns_activate(0, 0);

#if 1 /* willow: earlysuspend shim */
    unregister_early_suspend(&accsns_early_suspend_handler);
#endif
    kfree(client_accsns);
    return 0;
}

static int accsns_suspend(struct i2c_client *client,pm_message_t mesg)
{
#ifdef ALPS_DEBUG
    printk("[ACC] suspend\n");
#endif
    atomic_set(&flgSuspend, 1);
    accsns_activate(0, 0);

    return 0;
}

static int accsns_resume(struct i2c_client *client)
{
#ifdef ALPS_DEBUG
    printk("[ACC] resume\n");
#endif
    atomic_set(&flgSuspend, 0);
    accsns_activate(0, atomic_read(&flgEna));

    return 0;
}

#if 1 /* willow: earlysuspend shim */
static void accsns_early_suspend(struct early_suspend *handler)
{
#ifdef ALPS_DEBUG
    printk("[ACC] early_suspend\n");
#endif
    accsns_suspend(client_accsns, PMSG_SUSPEND);
}

static void accsns_early_resume(struct early_suspend *handler)
{
#ifdef ALPS_DEBUG
    printk("[ACC] early_resume\n");
#endif
    accsns_resume(client_accsns);
}
#endif

static const struct i2c_device_id accsns_id[] = {
    { ACC_DRIVER_NAME, 0 },
    { }
};

static struct i2c_driver accsns_driver = {
    .probe     = accsns_probe,
    .remove    = accsns_remove,
    .id_table  = accsns_id,
    .driver    = {
        .name  = ACC_DRIVER_NAME,
        .owner      = THIS_MODULE,
    },
#ifndef CONFIG_HAS_EARLYSUSPEND
    .suspend   = accsns_suspend,
    .resume    = accsns_resume,
#endif
};

#if 1 /* willow: earlysuspend shim */
static struct early_suspend accsns_early_suspend_handler = {
    .suspend = accsns_early_suspend,
    .resume  = accsns_early_resume,
};
#endif

static int __init accsns_init(void)
{
    int rc;

#ifdef ALPS_DEBUG
    printk("[ACC] init\n");
#endif
    atomic_set(&flgEna, 0);
    atomic_set(&flgSuspend, 0);
    rc = i2c_add_driver(&accsns_driver);
    if (rc != 0) {
        printk("can't add i2c driver\n");
        rc = -ENOTSUPP;
        return rc;
    }

#if 1 /* willow: earlysuspend shim */
    register_early_suspend(&accsns_early_suspend_handler);
#endif

#ifdef ALPS_DEBUG
    printk("accsns_open end !!!!\n");
#endif

    return rc;
}

static void __exit accsns_exit(void)
{
#ifdef ALPS_DEBUG
    printk("[ACC] exit\n");
#endif
    i2c_del_driver(&accsns_driver);
}

module_init(accsns_init);
module_exit(accsns_exit);

EXPORT_SYMBOL(accsns_get_acceleration_data);
EXPORT_SYMBOL(accsns_activate);

MODULE_DESCRIPTION("Alps Accelerometer Device");
MODULE_AUTHOR("ALPS ELECTRIC CO., LTD.");
MODULE_LICENSE("GPL v2");
