/* 
 * drivers/input/touchscreen/ft5x0x_ts.c
 *
 * FocalTech ft5x0x TouchScreen driver. 
 *
 * Copyright (c) 2010  Focal tech Ltd.
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
 * VERSION      	DATE			AUTHOR        Note
 *    1.0		  2010-01-05			WenFS    only support mulititouch	Wenfs 2010-10-01
 *    2.0          2011-09-05                   Duxx      Add touch key, and project setting update, auto CLB command
 *    3.0		  2011-09-09			Luowj   
 *
 */

#include <linux/i2c.h>
#include <linux/input.h>
#include "ft5x06_ts.h"
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>   
#include <mach/irqs.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>

#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <mach/regs-irq.h>
#include <linux/regulator/consumer.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>

#define FEATURE_TW_TOUCH_POWER_SEQ

#if defined(FEATURE_TW_TOUCH_POWER_SEQ)
#include <linux/io.h>
#include <plat/gpio-core.h>
#include <plat/gpio-cfg.h>
#include <plat/gpio-cfg-helpers.h>
#endif

//#include <asm/jzsoc.h>

static struct i2c_client *this_client;

#define CONFIG_FT5X0X_MULTITOUCH 1
#define CONFIG_SUPPORT_FTS_CTP_UPG

//#define T9_SIS_POINT_90_ROTATE

static struct regulator *touch_ldo;

int check_touch_power=0;
int check_used_touch_ldo=0;

struct ts_event {
    u16 au16_x[CFG_MAX_TOUCH_POINTS];              //x coordinate
    u16 au16_y[CFG_MAX_TOUCH_POINTS];              //y coordinate
    u8  au8_touch_event[CFG_MAX_TOUCH_POINTS];     //touch event:  0 -- down; 1-- contact; 2 -- contact
    u8  au8_finger_id[CFG_MAX_TOUCH_POINTS];       //touch ID
	u16	pressure;
    u8  touch_point;
};


struct ft5x0x_ts_data {
	struct input_dev	*input_dev;
	struct ts_event		event;
	struct work_struct 	pen_event_work;
	struct workqueue_struct *ts_workqueue;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend	early_suspend;
#endif
	struct mutex device_mode_mutex;   /* Ensures that only one function can specify the Device Mode at a time. */
};

//register address
#define FT5x06_REG_FW_VER 0xA6
// 38 : 24 
#define FT5x0x_TX_NUM	28
#define FT5x0x_RX_NUM   16
//u16 g_RawData[FT5x0x_TX_NUM][FT5x0x_RX_NUM];

static u8 ft5x0x_enter_factory(struct ft5x0x_ts_data *ft5x0x_ts);
static u8 ft5x0x_enter_work(struct ft5x0x_ts_data *ft5x0x_ts);

unsigned char touch_last_key_code; // TOUCH KEY PRESS/RELEASE

/* ******************************************/
//#define FEATURE_TOUCH_FW_VER_0X11

//#define FEATURE_TOUCH_FW_VER_0X13

#define FEATURE_TOUCH_FW_VER_0X14

//#define FEATURE_TOUCH_FW_VER_0X15

//#define FEATURE_TOUCH_FW_VER_0X70

//#define FEATURE_TOUCH_FW_VER_0X71

//#define FEATURE_TOUCH_FW_VER_0X31 //SKPOS

#define FEATURE_TOUCH_ALL_FILE_UPDATE
/* ******************************************/
#if defined(FEATURE_TOUCH_ALL_FILE_UPDATE)
int check_suspend=0;
extern void set_touch_autoCal(int Setvalue);
//void touch_s3c_i2c5_set_platdata(struct s3c2410_platform_i2c *pd, int check_value);
extern void s3c_i2c5_force_stop(void);
//extern void t10s_i2c_clockrate(int i2c_num);
extern int s3c24xx_i2c_set(int num, int slave, int setfreq);
#endif

extern void set_touch_ic_check(int value);

#if CFG_SUPPORT_TOUCH_KEY
int tsp_keycodes[CFG_NUMOFKEYS] ={

        KEY_MENU,
        KEY_HOME,
        KEY_BACK,
        KEY_SEARCH
};

char *tsp_keyname[CFG_NUMOFKEYS] ={

        "Menu",
        "Home",
        "Back",
        "Search"
};

static bool tsp_keystatus[CFG_NUMOFKEYS];
#endif

#ifdef CONFIG_LEDS_T10
extern uint8_t t10_led_timer_set;
extern void t10_led_timer_on(int sec);
#endif

#define FIXED_TEMPORARY_SUSPEND_BUG
#ifdef  FIXED_TEMPORARY_SUSPEND_BUG
u8    suspend_flag;
#endif

#define FEATURE_TW_TOUCH_EMERGENCY_OFF

#if defined(FEATURE_TOUCH_FW_VER_0X11)
#define FEATURE_TW_SYSTEM_SLEEP_WAKEUP
struct timer_list emer_timer;
#endif

#if defined(FEATURE_TW_SYSTEM_SLEEP_WAKEUP)
int touch_emergency = 1;
int wake_touch_count =0;
int old_x_touch = 0;
int old_y_touch = 0;
int usr_shot_touch_count = 0;
int check_emer_timer =0;

void clear_emer(void)
{

    usr_shot_touch_count = 0;
    touch_emergency = 0;
    wake_touch_count = 0;            
    check_emer_timer=0;
    printk("[TSP] clear emergency \n");
}
extern int ft5x0x_read_reg(u8 addr, u8 *pdata);

static void emer_timer_func(void)
{
	unsigned char ver;
//	printk("[FocalTech] %s()\n", __func__);

	//ft5x0x_read_reg(FT5X0X_REG_FIRMID, &ver);
    printk(" emergency  touch verion =%x \n", ver);
    init_timer(&emer_timer);
    emer_timer.function = &clear_emer;
    emer_timer.data = NULL;
    emer_timer.expires = jiffies + 1 * HZ;
    add_timer(&emer_timer);
    check_emer_timer=1;
}
#endif

#if defined(FEATURE_TW_TOUCH_EMERGENCY_OFF)
int touch_emergency_off=0;

int check_touch_emergency_off=0;

void t10_emergency_touch_off(void)
{
    touch_emergency_off=1; 

}
EXPORT_SYMBOL(t10_emergency_touch_off);
#endif

int focal_gpio_slp_cfgpin(unsigned int pin, unsigned int config)
{
	struct s3c_gpio_chip *chip = s3c_gpiolib_getchip(pin);
	void __iomem *reg;
	unsigned long flags;
	int offset;
	u32 con;
	int shift;

	reg = chip->base + 0x10;

	offset = pin - chip->chip.base;
	shift = offset * 2;

	local_irq_save(flags);

	con = __raw_readl(reg);
	con &= ~(3 << shift);
	con |= config << shift;
	__raw_writel(con, reg);

	local_irq_restore(flags);
	return 0;
}

int focal_gpio_slp_setpull_updown(unsigned int pin, unsigned int config)
{
	struct s3c_gpio_chip *chip = s3c_gpiolib_getchip(pin);
	void __iomem *reg;
	unsigned long flags;
	int offset;
	u32 con;
	int shift;

	reg = chip->base + 0x14;

	offset = pin - chip->chip.base;
	shift = offset * 2;

	local_irq_save(flags);

	con = __raw_readl(reg);
	con &= ~(3 << shift);
	con |= config << shift;
	__raw_writel(con, reg);

	local_irq_restore(flags);

	return 0;
}

void focaltech_interrupt_low_gpio(void)
{
    // Touch_int gpx0_4
	focal_gpio_slp_cfgpin(EXYNOS4_GPX0(4), S3C_GPIO_INPUT);
	focal_gpio_slp_setpull_updown(EXYNOS4_GPX0(4), S3C_GPIO_PULL_DOWN);
}


void focaltech_interrupt_init_gpio(void)
{
    // Touch_int gpx0_4
	focal_gpio_slp_cfgpin(EXYNOS4_GPX0(4), S3C_GPIO_INPUT);
	focal_gpio_slp_setpull_updown(EXYNOS4_GPX0(4), S3C_GPIO_PULL_UP);
}

int focaltech_touch_reset(int onoff)
{
	int err;
	// touch gpb4
	err = gpio_request(EXYNOS4_GPB(4), "focal_touch_reset");
	if (err<0) {
		printk("[FocalTech] Error (L:%d), %s() - gpio_request(focal_touch_reset) failed (err=%d)\n", __LINE__, __func__, err);
		return err;
	}else {
		s3c_gpio_setpull(EXYNOS4_GPB(4), S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(EXYNOS4_GPB(4), S3C_GPIO_OUTPUT);

		gpio_set_value(EXYNOS4_GPB(4), onoff);
    }		
	
	gpio_free(EXYNOS4_GPB(4));
	return 0;
}
EXPORT_SYMBOL(focaltech_touch_reset);

void focaltech_touch_on(void)
{
//	regulator_enable(t9_touch_ldo);
	printk("[FocalTech] TS_POWER ON\n");
	//s3c_i2c5_force_stop();
   mdelay(10);

	//focaltech_interrupt_low_gpio();
#if defined(FEATURE_TW_TOUCH_POWER_SEQ)
   focaltech_touch_reset(0);
#endif
	mdelay(200);

    regulator_enable(touch_ldo);
	//Power On sequence need dealy	
	//msleep(300);

#if defined(FEATURE_TW_TOUCH_POWER_SEQ)
   mdelay(20);
   focaltech_touch_reset(1);
	focaltech_interrupt_init_gpio();
	mdelay(410);
#else	
	focaltech_touch_reset();
#endif
	check_touch_emergency_off=0;

	check_touch_power=1;

	printk("[FocalTech] INIT RESET ON\n");
}


void focaltech_touch_off(void)
{
    //int err;
	if(check_touch_power==1)    
	{
			printk("[FocalTech] TS_POWER OFF\n");

			//TS_RESET low

		   focaltech_touch_reset(0);

		//	printk("[FocalTech] TS_RESET Low\n");
#if defined(FEATURE_TW_TOUCH_POWER_SEQ)
		   msleep(1);
			focaltech_interrupt_low_gpio();
#endif

			if (regulator_is_enabled(touch_ldo))
				regulator_disable(touch_ldo);
		    check_touch_emergency_off=1;
		check_touch_power=0;				
	}
}

/***********************************************************************************************
Name	:	ft5x0x_i2c_rxdata 

Input	:	*rxdata
                     *length

Output	:	ret

function	:	

***********************************************************************************************/
static int ft5x0x_i2c_rxdata(char *rxdata, int length)
{
	int ret;

	struct i2c_msg msgs[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= rxdata,
		},
		{
			.addr	= this_client->addr,
			.flags	= I2C_M_RD,
			.len	= length,
			.buf	= rxdata,
		},
	};

//	printk("[FocalTech] %s()\n", __func__);

    //msleep(1);
	ret = i2c_transfer(this_client->adapter, msgs, 2);
	if (ret < 0)
		pr_err("msg %s i2c read error: %d\n", __func__, ret);
	
	return ret;
}
/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static int ft5x0x_i2c_txdata(char *txdata, int length)
{
	int ret;

	struct i2c_msg msg[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= length,
			.buf	= txdata,
		},
	};

//	printk("[FocalTech] %s()\n", __func__);

   	//msleep(1);
	ret = i2c_transfer(this_client->adapter, msg, 1);
	if (ret < 0)
		pr_err("%s i2c write error: %d\n", __func__, ret);

	return ret;
}

/***********************************************************************************************
Name	:	 ft5x0x_write_reg

Input	:	addr -- address
                     para -- parameter

Output	:	

function	:	write register of ft5x0x

***********************************************************************************************/
static int ft5x0x_write_reg(u8 addr, u8 para)
{
    u8 buf[3];
    int ret = -1;

	if(check_suspend==1)
	   return 500;
//    printk("[FocalTech] %s()\n", __func__);
    buf[0] = addr;
    buf[1] = para;
    ret = ft5x0x_i2c_txdata(buf, 2);
    if (ret < 0) {
        pr_err("write reg failed! %#x ret: %d", buf[0], ret);
        return -1;
    }
    
    return 0;
}


/***********************************************************************************************
Name	:	ft5x0x_read_reg 

Input	:	addr
                     pdata

Output	:	

function	:	read register of ft5x0x

***********************************************************************************************/
static int ft5x0x_read_reg(u8 addr, u8 *pdata)
{
	int ret;
	u8 buf[2];
	struct i2c_msg msgs[2];
	
	if(check_suspend==1)
	   return 500;
	
//	printk("[FocalTech] %s()\n", __func__);

	buf[0] = addr;    //register address
	
	msgs[0].addr = this_client->addr;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = buf;
	msgs[1].addr = this_client->addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = 1;
	msgs[1].buf = buf;

	ret = i2c_transfer(this_client->adapter, msgs, 2);
	if (ret < 0)
		pr_err("msg %s i2c read error: %d\n", __func__, ret);

	*pdata = buf[0];
	return ret;
  
}


/***********************************************************************************************
Name	:	 ft5x0x_read_fw_ver

Input	:	 void
                     

Output	:	 firmware version 	

function	:	 read TP firmware version

***********************************************************************************************/
static unsigned char ft5x0x_read_fw_ver(void)
{
	unsigned char ver;
//	printk("[FocalTech] %s()\n", __func__);

	ft5x0x_read_reg(FT5X0X_REG_FIRMID, &ver);
	return(ver);
}

#ifdef CONFIG_SUPPORT_FTS_CTP_UPG
typedef enum
{
    ERR_OK,
    ERR_MODE,
    ERR_READID,
    ERR_ERASE,
    ERR_STATUS,
    ERR_ECC,
    ERR_DL_ERASE_FAIL,
    ERR_DL_PROGRAM_FAIL,
    ERR_DL_VERIFY_FAIL
}E_UPGRADE_ERR_TYPE;

typedef unsigned char         FTS_BYTE;     //8 bit
typedef unsigned short        FTS_WORD;    //16 bit
typedef unsigned int          FTS_DWRD;    //16 bit
typedef unsigned char         FTS_BOOL;    //8 bit

typedef struct _FTS_CTP_PROJECT_SETTING_T
{
    unsigned char uc_i2C_addr;             //I2C slave address (8 bit address)
    unsigned char uc_io_voltage;           //IO Voltage 0---3.3v;	1----1.8v
    unsigned char uc_panel_factory_id;     //TP panel factory ID
}FTS_CTP_PROJECT_SETTING_T;

#define FTS_NULL                0x0
#define FTS_TRUE                0x01
#define FTS_FALSE              0x0

// 0x72 firmware change
#define I2C_CTPM_ADDRESS       0x70


void delay_qt_ms(unsigned long  w_ms)
{
//    unsigned long i;
//    unsigned long j;

    mdelay(w_ms);
}


/*
[function]: 
    callback: read data from ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[out]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    FTS_TRUE     :success;
    FTS_FALSE    :fail;
*/
FTS_BOOL i2c_read_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    int ret;
    
//    printk("[FocalTech] %s()\n", __func__);
    
    ret=i2c_master_recv(this_client, pbt_buf, dw_lenth);

    if(ret<=0)
    {
        printk("[FTS]i2c_read_interface error\n");
        return FTS_FALSE;
    }
  
    return FTS_TRUE;
}

/*
[function]: 
    callback: write data to ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[in]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    FTS_TRUE     :success;
    FTS_FALSE    :fail;
*/
FTS_BOOL i2c_write_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    int ret;

//    printk("[FocalTech] %s()\n", __func__);

    ret=i2c_master_send(this_client, pbt_buf, dw_lenth);
    if(ret<=0)
    {
        //printk("[FTS]i2c_write_interface error line = %d, ret = %d\n", __LINE__, ret);
        return FTS_FALSE;
    }

    return FTS_TRUE;
}

/*
[function]: 
    send a command to ctpm.
[parameters]:
    btcmd[in]        :command code;
    btPara1[in]    :parameter 1;    
    btPara2[in]    :parameter 2;    
    btPara3[in]    :parameter 3;    
    num[in]        :the valid input parameter numbers, if only command code needed and no parameters followed,then the num is 1;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
FTS_BOOL cmd_write(FTS_BYTE btcmd,FTS_BYTE btPara1,FTS_BYTE btPara2,FTS_BYTE btPara3,FTS_BYTE num)
{
    FTS_BYTE write_cmd[4] = {0};

//    printk("[FocalTech] %s()\n", __func__);

    write_cmd[0] = btcmd;
    write_cmd[1] = btPara1;
    write_cmd[2] = btPara2;
    write_cmd[3] = btPara3;
    return i2c_write_interface(I2C_CTPM_ADDRESS, write_cmd, num);
}

/*
[function]: 
    write data to ctpm , the destination address is 0.
[parameters]:
    pbt_buf[in]    :point to data buffer;
    bt_len[in]        :the data numbers;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
FTS_BOOL byte_write(FTS_BYTE* pbt_buf, FTS_DWRD dw_len)
{
//    printk("[FocalTech] %s()\n", __func__);
    
    return i2c_write_interface(I2C_CTPM_ADDRESS, pbt_buf, dw_len);
}

/*
[function]: 
    read out data from ctpm,the destination address is 0.
[parameters]:
    pbt_buf[out]    :point to data buffer;
    bt_len[in]        :the data numbers;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
FTS_BOOL byte_read(FTS_BYTE* pbt_buf, FTS_BYTE bt_len)
{
//    printk("[FocalTech] %s()\n", __func__);
    return i2c_read_interface(I2C_CTPM_ADDRESS, pbt_buf, bt_len);
}


/*
[function]: 
    burn the FW to ctpm.
[parameters]:(ref. SPEC)
    pbt_buf[in]    :point to Head+FW ;
    dw_lenth[in]:the length of the FW + 6(the Head length);    
    bt_ecc[in]    :the ECC of the FW
[return]:
    ERR_OK        :no error;
    ERR_MODE    :fail to switch to UPDATE mode;
    ERR_READID    :read id fail;
    ERR_ERASE    :erase chip fail;
    ERR_STATUS    :status error;
    ERR_ECC        :ecc error.
*/


#define    FTS_PACKET_LENGTH        128

#define FTS_ALL_PACKET_LENGTH        256

static unsigned char CTPM_FW[]=
{
#if defined(FEATURE_TOUCH_FW_VER_0X11)
	#include "focal_bin/ft_app_0x11.i"
#elif defined(FEATURE_TOUCH_FW_VER_0X13)
	#include "focal_bin/ft_app_0x13.i"
#elif defined(FEATURE_TOUCH_FW_VER_0X14)
	#include "focal_bin/ft_app_0x14.i"
#elif defined(FEATURE_TOUCH_FW_VER_0X15)
	#include "focal_bin/ft_app_0x15.i"
#elif defined(FEATURE_TOUCH_FW_VER_0X31)
	#include "focal_bin/ft_app_0x31.i"
#elif defined(FEATURE_TOUCH_FW_VER_0X70)
	#include "focal_bin/ft_app_0x70.i"
#elif defined(FEATURE_TOUCH_FW_VER_0X71)
	#include "focal_bin/ft_app_0x71.i"
#else
// Touch Firmware Version 0x11
	#include "focal_bin/ft_app.i"
#endif

};

static unsigned char ALL_CTPM_FW[]=
{
#if defined(FEATURE_TOUCH_FW_VER_0X11)
	#include "focal_bin/ft5606_all_0x11.i"
#elif defined(FEATURE_TOUCH_FW_VER_0X13)
	#include "focal_bin/ft5606_all_0x13.i"
#elif defined(FEATURE_TOUCH_FW_VER_0X14)
	#include "focal_bin/ft5606_all_0x14.i"
#elif defined(FEATURE_TOUCH_FW_VER_0X15)
	#include "focal_bin/ft5606_all_0x15.i"	
#elif defined(FEATURE_TOUCH_FW_VER_0X31)
	#include "focal_bin/ft5606_all_0x31.i"	
#elif defined(FEATURE_TOUCH_FW_VER_0X70)
	#include "focal_bin/ft5606_all_0x70.i"
#elif defined(FEATURE_TOUCH_FW_VER_0X71)
	#include "focal_bin/ft5606_all_0x71.i"
#else
// Touch Firmware Version 0x11
	#include "focal_bin/ft5606_all_0x13.i"
#endif
};

int t10s_focaltech_touch_reset(int rest_time)
{
	int err=0;
 	err = gpio_request(EXYNOS4_GPB(4), "TOUCH_RESET");
	if (err) {
		printk("[FocalTech] Error (L:%d), %s() - gpio_request(GPIO_TS_RESET) failed (err=%d)\n", __LINE__, __func__, err);
		return err;
	}
    gpio_direction_output(EXYNOS4_GPB(4), 1);
    gpio_set_value(EXYNOS4_GPB(4), 0);
    delay_qt_ms(1);
    gpio_set_value(EXYNOS4_GPB(4), 1);
    gpio_free(EXYNOS4_GPB(4));
    delay_qt_ms(rest_time);
    return 0;
}

E_UPGRADE_ERR_TYPE  fts_ctpm_fw_upgrade(FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    FTS_BYTE reg_val[2] = {0,};
    FTS_DWRD i = 0;

    FTS_DWRD  packet_number;
    FTS_DWRD  j;
    FTS_DWRD  temp;
    FTS_DWRD  lenght;
    FTS_BYTE  packet_buf[FTS_PACKET_LENGTH + 6];
    FTS_BYTE  auc_i2c_write_buf[10];
    FTS_BYTE bt_ecc;
    int      i_ret;

    printk("[FocalTech] %s()\n", __func__);
#if 1
    /*********Step 1:Reset  CTPM *****/
    /*write 0xaa to register 0xfc*/
#if 1 
	t10s_focaltech_touch_reset(5);
#else
    ft5x0x_write_reg(0xfc,0xaa);
    delay_qt_ms(50);
     /*write 0x55 to register 0xfc*/
    ft5x0x_write_reg(0xfc,0x55);
    printk("[FTS] Step 1: Reset CTPM test\n");
   
    delay_qt_ms(30);  //set delay 10ms for 5606
#endif

    /*********Step 2:Enter upgrade mode *****/
    printk("[FTS] Step 2:Enter upgrade mode\n");
    auc_i2c_write_buf[0] = 0x55;
    auc_i2c_write_buf[1] = 0xaa;
    do
    {
        i ++;
        i_ret = ft5x0x_i2c_txdata(auc_i2c_write_buf, 2);
        delay_qt_ms(5);
    }while(i_ret <= 0 && i < 5 );

    /*********Step 3:check READ-ID***********************/ 
    delay_qt_ms(1000);   // set addtional delay for 5606
    cmd_write(0x90,0x00,0x00,0x00,4);
    byte_read(reg_val,2);

    if (reg_val[0] == 0x79 && reg_val[1] == 0x6) // changing 0x03 as 0x06 for 5606
    {
        printk("[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
    }
    else
    {
        printk("reg_val[0] = 0x%x , reg_val[1] = 0x%x\n",reg_val[0],reg_val[1]);
        return ERR_READID;
        //i_is_new_protocol = 1;
    }

    cmd_write(0xcd,0x0,0x00,0x00,1);
    byte_read(reg_val,1);
    printk("[FTS] bootloader version = 0x%x\n", reg_val[0]);

     /*********Step 4:erase app and panel paramenter area ********************/
    cmd_write(0x61,0x00,0x00,0x00,1);  //erase app area
    delay_qt_ms(1500);
    cmd_write(0x63,0x00,0x00,0x00,1);  //erase panel parameter area
    delay_qt_ms(100);
    printk("[FTS] Step 4: erase. \n");

    /*********Step 5:write firmware(FW) to ctpm flash*********/
    bt_ecc = 0;

    printk("[FTS] Step 5: start upgrade. \n");
    dw_lenth = dw_lenth - 8;
    packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
    packet_buf[0] = 0xbf;
    packet_buf[1] = 0x00;
    for (j=0;j<packet_number;j++)
    {
        temp = j * FTS_PACKET_LENGTH;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;
        lenght = FTS_PACKET_LENGTH;
        packet_buf[4] = (FTS_BYTE)(lenght>>8);
        packet_buf[5] = (FTS_BYTE)lenght;

        for (i=0;i<FTS_PACKET_LENGTH;i++)
        {
            packet_buf[6+i] = pbt_buf[j*FTS_PACKET_LENGTH + i]; 
            bt_ecc ^= packet_buf[6+i];
        }
        
        byte_write(&packet_buf[0],FTS_PACKET_LENGTH + 6);
        delay_qt_ms(FTS_PACKET_LENGTH/6 + 1);
        if ((j * FTS_PACKET_LENGTH % 1024) == 0)
        {
              printk("[FTS] upgrade the 0x%x th byte.\n", ((unsigned int)j) * FTS_PACKET_LENGTH);
        }
    }

    if ((dw_lenth) % FTS_PACKET_LENGTH > 0)
    {
        temp = packet_number * FTS_PACKET_LENGTH;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;

        temp = (dw_lenth) % FTS_PACKET_LENGTH;
        packet_buf[4] = (FTS_BYTE)(temp>>8);
        packet_buf[5] = (FTS_BYTE)temp;

        for (i=0;i<temp;i++)
        {
            packet_buf[6+i] = pbt_buf[ packet_number*FTS_PACKET_LENGTH + i]; 
            bt_ecc ^= packet_buf[6+i];
        }

        byte_write(&packet_buf[0],temp+6);    
        delay_qt_ms(20);
    }

    //send the last six byte
    for (i = 0; i<6; i++)
    {
        temp = 0x6ffa + i;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;
        temp =1;
        packet_buf[4] = (FTS_BYTE)(temp>>8);
        packet_buf[5] = (FTS_BYTE)temp;
        packet_buf[6] = pbt_buf[ dw_lenth + i]; 
        bt_ecc ^= packet_buf[6];

        byte_write(&packet_buf[0],7);  
        delay_qt_ms(20);
    }

    /*********Step 6: read out checksum***********************/
    /*send the opration head*/
    cmd_write(0xcc,0x00,0x00,0x00,1);
    byte_read(reg_val,1);
    printk("[FTS] Step 6:  ecc read 0x%x, new firmware 0x%x. \n", reg_val[0], bt_ecc);
    if(reg_val[0] != bt_ecc)
    {
        return ERR_ECC;
    }

    /*********Step 7: reset the new FW***********************/
    cmd_write(0x07,0x00,0x00,0x00,1);

    msleep(300);  //make sure CTP startup normally
#else
    dw_lenth = dw_lenth - 8;
    packet_number = (dw_lenth) / FTS_ALL_PACKET_LENGTH;
    for (j=0;j<packet_number;j++)
    {
        temp = j * FTS_ALL_PACKET_LENGTH;

        for (i=0;i<FTS_ALL_PACKET_LENGTH;i++)
        {
			printk("0x%2x,",  pbt_buf[j*FTS_ALL_PACKET_LENGTH + i]);
			if (((i+1)%16) == 0)
				printk("\r\n");
        }
        
    }

#endif    
    return ERR_OK;
}
#if defined(FEATURE_TOUCH_ALL_FILE_UPDATE)
E_UPGRADE_ERR_TYPE  fts_ctpm_all_file_fw_upgrade(FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    FTS_DWRD i = 0;
    FTS_DWRD  packet_number;
    FTS_DWRD  j;
    FTS_DWRD  temp;
    FTS_DWRD  lenght;
    FTS_BYTE  packet_buf[FTS_ALL_PACKET_LENGTH + 6];
    FTS_BYTE  auc_i2c_write_buf[16];
    FTS_BYTE bt_ecc;
    int      i_ret;
	FTS_BOOL write_ret=FTS_FALSE;
		
    printk("[FocalTech] %s()\n", __func__);

    /*********Step 1:Reset  CTPM *****/
    /*write 0xaa to register 0xfc*/
   // Reset Low
	gpio_request_one(EXYNOS4_GPB(4), GPIOF_OUT_INIT_LOW, "GPB4");
	gpio_set_value(EXYNOS4_GPB(4), 0);
   msleep(1);
	focaltech_interrupt_low_gpio();
    regulator_disable(touch_ldo);
	msleep(150);

	regulator_enable(touch_ldo);
   delay_qt_ms(1);   
	gpio_set_value(EXYNOS4_GPB(4), 1);
	gpio_free(EXYNOS4_GPB(4));
	delay_qt_ms(50);
    /*********Step 2:Enter upgrade mode *****/
    printk("[FTS] Step 2:Enter debug mode\n");
	memset(auc_i2c_write_buf, 0x00, sizeof(auc_i2c_write_buf));

    auc_i2c_write_buf[0] = 0x55;
    auc_i2c_write_buf[1] = 0xaa;   
	do
    {
        i ++;
        i_ret = ft5x0x_i2c_txdata(auc_i2c_write_buf, 2);
        delay_qt_ms(50);//delay_qt_ms(5);
    }while(i_ret <= 0 && i < 5 );

    printk("[FTS] Step2 result  i=%d \n",i);

    /*********Step 3:FT5606_WriteEnable***********************/ 
    printk("[FTS] Step 3:Enter FT5606_WriteEnable mode\n");
	msleep(100);

	memset(auc_i2c_write_buf, 0x00, sizeof(auc_i2c_write_buf));
	auc_i2c_write_buf[0] = 0x09;
	auc_i2c_write_buf[1] = 0xf6;
	auc_i2c_write_buf[2] = 0xfd;
	auc_i2c_write_buf[3] = 0x0d;
	i_ret = byte_write(auc_i2c_write_buf, 4);
	msleep(1);

	memset(auc_i2c_write_buf, 0x00, sizeof(auc_i2c_write_buf));
	auc_i2c_write_buf[0] = 0x09;
	auc_i2c_write_buf[1] = 0xf6;
	auc_i2c_write_buf[2] = 0xfe;
	auc_i2c_write_buf[3] = 0x84;
	i_ret = byte_write(auc_i2c_write_buf, 4);

	msleep(1);	
	memset(auc_i2c_write_buf, 0x00, sizeof(auc_i2c_write_buf));
	auc_i2c_write_buf[0] = 0x09;
	auc_i2c_write_buf[1] = 0xf6;
	auc_i2c_write_buf[2] = 0xfd;
	auc_i2c_write_buf[3] = 0x0f;
	i_ret = byte_write(auc_i2c_write_buf, 4);

	msleep(1);	
	memset(auc_i2c_write_buf, 0x00, sizeof(auc_i2c_write_buf));
	auc_i2c_write_buf[0] = 0x09;
	auc_i2c_write_buf[1] = 0xf6;
	auc_i2c_write_buf[2] = 0xfe;
	auc_i2c_write_buf[3] = 0x28;
	i_ret = byte_write(auc_i2c_write_buf, 4);
    /*********Step 4:EraseFlash***********************/ 
    printk("[FTS] Step 4:EraseFlash \n");
	delay_qt_ms(100);

	memset(auc_i2c_write_buf, 0x00, sizeof(auc_i2c_write_buf));
	auc_i2c_write_buf[0] = 0x09;
	auc_i2c_write_buf[1] = 0xF6;
	auc_i2c_write_buf[2] = 0xF3;
	auc_i2c_write_buf[3] = 0x00;

	auc_i2c_write_buf[4] = 0x07;
	auc_i2c_write_buf[5] = 0xF8;
	auc_i2c_write_buf[6] = 0x79;
	auc_i2c_write_buf[7] = 0x00;
	auc_i2c_write_buf[8] = 0x00;
#if	1
	auc_i2c_write_buf[9] = 0x05; 
	auc_i2c_write_buf[10] = 0x00;
	auc_i2c_write_buf[11] = 0x54;
	auc_i2c_write_buf[12] = 0x55;
   byte_write(auc_i2c_write_buf,13);   
	delay_qt_ms(50);
#else
	auc_i2c_write_buf[9] = 0x06; 
	auc_i2c_write_buf[10] = 0x00;
	auc_i2c_write_buf[11] = 0x14;
	auc_i2c_write_buf[12] = 0x54;
	auc_i2c_write_buf[13] = 0x55;
   byte_write(auc_i2c_write_buf,14);   
	delay_qt_ms(40);
#endif 
	memset(auc_i2c_write_buf, 0x00, sizeof(auc_i2c_write_buf));
	auc_i2c_write_buf[0] = 0x15;
	i_ret = byte_write(auc_i2c_write_buf, 1);
    delay_qt_ms(5);   

	memset(auc_i2c_write_buf, 0x00, sizeof(auc_i2c_write_buf));
	auc_i2c_write_buf[0] = 0x14;
	auc_i2c_write_buf[1] = 0x00;
	i_ret = byte_write(auc_i2c_write_buf, 2);
    delay_qt_ms(100);   
    /*********Step 5:write firmware(FW) to ctpm flash*********/
	memset(packet_buf, 0x00, sizeof(packet_buf));

    bt_ecc = 0;
    printk("[FTS] Step 5: start upgrade. write\n");
    //dw_lenth = dw_lenth - 8;
    packet_number = (dw_lenth) / FTS_ALL_PACKET_LENGTH;
    packet_buf[0] = 0x03;
    packet_buf[1] = 0xfc;

    lenght = FTS_ALL_PACKET_LENGTH;
    packet_buf[4] = (unsigned char)(lenght>>8);
    packet_buf[5] = (unsigned char)lenght;

	for (j=0; j<packet_number; j++)
	{
		temp = j * FTS_ALL_PACKET_LENGTH;
		packet_buf[2] = (unsigned char)(temp>>8);
		packet_buf[3] = (unsigned char)temp;

		lenght = FTS_ALL_PACKET_LENGTH - 1;
		packet_buf[4] = (unsigned char)(lenght>>8);
		packet_buf[5] = (unsigned char)lenght;

		for (i=0;i<FTS_ALL_PACKET_LENGTH;i++)
		{
			packet_buf[6+i] = pbt_buf[j*FTS_ALL_PACKET_LENGTH + i]; 
			//bt_ecc ^= packet_buf[6+i];
			//printk("0x%2x,", packet_buf[6+i]);
			//if (((i+1)%16) == 0)
			//printk("\r\n");
		}

		byte_write(&packet_buf[0],FTS_ALL_PACKET_LENGTH + 6);
		delay_qt_ms(100);//delay_qt_ms(50);  
#if 0
		if ((j * FTS_PACKET_LENGTH % 1024) == 0)
		{
			printk("[FTS] upgrade the 0x%x th byte.\r\n", ((unsigned int)j) * FTS_PACKET_LENGTH);
		}
#endif
		printk("%d Bytes Write...\r\n", (j+1)*FTS_ALL_PACKET_LENGTH);
	}

	if ((dw_lenth % FTS_ALL_PACKET_LENGTH) > 0)
	{
		temp = packet_number * FTS_ALL_PACKET_LENGTH;
		packet_buf[2] = (unsigned char)(temp>>8);
		packet_buf[3] = (unsigned char)temp;

		temp = (dw_lenth % FTS_ALL_PACKET_LENGTH) - 1;
		packet_buf[4] = (unsigned char)(temp>>8);
		packet_buf[5] = (unsigned char)temp;

		for (i=0; i<temp+1; i++)
		{
			packet_buf[6+i] = pbt_buf[ packet_number*FTS_ALL_PACKET_LENGTH + i]; 
		}

		write_ret=byte_write(&packet_buf[0], temp+6+1);
		if(write_ret==FTS_FALSE)
		{
			printk("[FTS] Step 5: write fail >>> \n");
			return ERR_DL_PROGRAM_FAIL;
		}
		delay_qt_ms(100);//delay_qt_ms(20);
	}
	
#if	0
	printk("[FTS] Step 5: start upgrade. read\n");
	memset(packet_buf, 0x00, sizeof(packet_buf));
	
	packet_number = dw_lenth / FTS_ALL_PACKET_LENGTH;
	auc_i2c_write_buf[0] = 0x02;
	auc_i2c_write_buf[1] = 0xFD;

	temp = 0;
	auc_i2c_write_buf[2] = (unsigned char)(temp>>8);
	auc_i2c_write_buf[3] = (unsigned char)temp;
	lenght = 15;
	auc_i2c_write_buf[4] = (unsigned char)(lenght>>8);
	auc_i2c_write_buf[5] = (unsigned char)lenght;

	byte_write(&auc_i2c_write_buf[0], 6);

	byte_read(&packet_buf[0], 16);

	for (j=0; j<packet_number; j++)
	{
		temp = j * FTS_ALL_PACKET_LENGTH;
		auc_i2c_write_buf[2] = (unsigned char)(temp>>8);
		auc_i2c_write_buf[3] = (unsigned char)temp;
		lenght = FTS_ALL_PACKET_LENGTH - 1;
		auc_i2c_write_buf[4] = (unsigned char)(lenght>>8);
		auc_i2c_write_buf[5] = (unsigned char)lenght;
		byte_write(&auc_i2c_write_buf[0], 6);

		byte_read(&packet_buf[0], FTS_ALL_PACKET_LENGTH);

		printk("0x%x 0x%x 0x%x 0x%x 0x%x 0x%x:\r\n", auc_i2c_write_buf[0], auc_i2c_write_buf[1], auc_i2c_write_buf[2]
		   , auc_i2c_write_buf[3], auc_i2c_write_buf[4], auc_i2c_write_buf[5]);
		for (i=0; i<FTS_ALL_PACKET_LENGTH; i++)
		{

			printk("0x%x\t", packet_buf[i]);
			if (((i+1)%16) == 0)
			   printk("\r\n");
		}

	}
	memset(packet_buf, 0x00, sizeof(packet_buf));

    if ((dw_lenth % FTS_ALL_PACKET_LENGTH) > 0)
    {
           auc_i2c_write_buf[0] = 0x02;
           auc_i2c_write_buf[1] = 0xFD;

        temp = packet_number * FTS_ALL_PACKET_LENGTH;
        auc_i2c_write_buf[2] = (unsigned char)(temp>>8);
        auc_i2c_write_buf[3] = (unsigned char)temp;

        temp = (dw_lenth % FTS_ALL_PACKET_LENGTH) - 1;
        auc_i2c_write_buf[4] = (unsigned char)(temp>>8);
        auc_i2c_write_buf[5] = (unsigned char)temp;
		byte_write(&auc_i2c_write_buf[0], 6);
		byte_read(&packet_buf[0], temp);
        for (i=0; i<temp; i++)
        {
			printk("0x%x\t", packet_buf[i]);
			if (((i+1)%16) == 0)
				printk("\r\n");
        }
    }
#endif
	msleep(1000);  //make sure CTP startup normally
	cmd_write(0x0b,0xf4,0x00,0x00,2);
	msleep(300);  //make sure CTP startup normally
	cmd_write(0x07,0x00,0x00,0x00,1);
	msleep(300);  //make sure CTP startup normally
	focaltech_touch_reset(0);
	msleep(5);
	focaltech_touch_reset(1);		
	msleep(300);		
	return ERR_OK;
}
#endif

static int fts_Get_RawData(u16 RawData[][FT5x0x_RX_NUM])
{
	int retval  = 0;
    	int i       = 0;
  //  	u16 dataval = 0x0000;
    	u8  devmode = 0x00;
    	u8  rownum  = 0x00;

    	u8 read_buffer[FT5x0x_RX_NUM * 2];
	u8 read_len = 0;
	//u8 write_buffer[2];
	struct ft5x0x_ts_data * ft5x0x_ts =  i2c_get_clientdata(this_client);
    struct i2c_msg msgs[1];

	printk("[FocalTech] %s()\n", __func__);

	if(ft5x0x_enter_factory(ft5x0x_ts)<0)
	{
		pr_err("%s ERROR: could not enter factory mode", __FUNCTION__);
		retval = -1;
		goto error_return;
	}
	//scan
	if(ft5x0x_read_reg(0x00, &devmode)<0)
	{
		pr_err("%s %d ERROR: could not read register 0x00", __FUNCTION__, __LINE__);
		retval = -1;
		goto error_return;
	}
	devmode |= 0x80;
	if(ft5x0x_write_reg(0x00, devmode)<0)
	{
		pr_err("%s %d ERROR: could not read register 0x00", __FUNCTION__, __LINE__);
		retval = -1;
		goto error_return;
	}
	msleep(20);
	if(ft5x0x_read_reg(0x00, &devmode)<0)
	{
		pr_err("%s %d ERROR: could not read register 0x00", __FUNCTION__, __LINE__);
		retval = -1;
		goto error_return;
	}
	if(0x00 != (devmode&0x80))
	{
		pr_err("%s %d ERROR: could not scan", __FUNCTION__, __LINE__);
		retval = -1;
		goto error_return;
	}
	pr_info("Read rawdata .......\n");
	for(rownum=0; rownum<FT5x0x_TX_NUM; rownum++)
	{
		memset(read_buffer, 0x00, (FT5x0x_RX_NUM * 2));

		if(ft5x0x_write_reg(0x01, rownum)<0)
		{
			pr_err("%s ERROR:could not write rownum", __FUNCTION__);
			retval = -1;
			goto error_return;
		}
		msleep(1);
		read_len = FT5x0x_RX_NUM * 2;
		if(ft5x0x_write_reg(0x10, read_len)<0)
		{
			pr_err("%s ERROR:could not write rownum", __FUNCTION__);
			retval = -1;
			goto error_return;
		}

        msgs[0].addr	= this_client->addr;
        msgs[0].flags	= 1;
        msgs[0].len	= FT5x0x_RX_NUM * 2;
        msgs[0].buf	= read_buffer;

		retval = i2c_transfer(this_client->adapter, msgs, 1);
		if (retval < 0) 
		{
			pr_err("%s ERROR:Could not read row %u raw data", __FUNCTION__, rownum);
			retval = -1;
			goto error_return;
		}
		for(i=0; i<FT5x0x_RX_NUM; i++)
		{
			RawData[rownum][i] = (read_buffer[i<<1]<<8) + read_buffer[(i<<1)+1];
		}
	}
error_return:
	if(ft5x0x_enter_work(ft5x0x_ts)<0)
	{
		pr_err("%s ERROR:could not enter work mode ", __FUNCTION__);
		retval = -1;
	}
	return retval;
}

int fts_ctpm_auto_clb(void)
{
	unsigned char uc_temp;
	unsigned char i ;
	int ret_value=0;

	printk("[FTS] start auto CLB.\n");
	msleep(100);//msleep(200);
	ret_value=ft5x0x_write_reg(0, 0x40);  

	if(ret_value ==500)
	{
		printk("[FTS] start auto CLB ft5x0x_write_reg(0, 0x40) err  \n");		
		goto error;
	}

	delay_qt_ms(100);   //make sure already enter factory mode
	ret_value=ft5x0x_write_reg(2, 0x4);  //write command to start calibration

	if(ret_value ==500)
	{
		printk("[FTS] start auto CLB ft5x0x_write_reg(2, 0x4) err  \n");		
		goto error;		
	}
	
	delay_qt_ms(300);
	for(i=0;i<10;i++)//for(i=0;i<100;i++)
	{
		ret_value=ft5x0x_read_reg(0,&uc_temp);
		if(ret_value ==500)
		{
			printk("[FTS] start auto CLB ft5x0x_write_reg(2, 0x4) err  \n");		
			goto error;		
		}		
		if ( ((uc_temp&0x70)>>4) == 0x0)  //return to normal mode, calibration finish
		{
			printk("[FTS]  calibration finish   OK.\n");
			break;
		}
		delay_qt_ms(200);
		printk("[FTS] waiting calibration %d\n",i);

	}
	printk("[FTS] calibration OK.\n");

	msleep(100);    //msleep(300);
	ret_value=ft5x0x_write_reg(0, 0x40);  //goto factory mode
	if(ret_value ==500)
	{
		printk("[FTS] start auto CLB ft5x0x_write_reg(0, 0x40) err  \n");		
		goto error;		
	}
	delay_qt_ms(100);   //make sure already enter factory mode
	ret_value=ft5x0x_write_reg(2, 0x5);  //store CLB result
	if(ret_value ==500)
	{
		printk("[FTS] start auto CLB ft5x0x_write_reg(2, 0x5) err  \n");		
		goto error;		
	}
	delay_qt_ms(400);
	ret_value=ft5x0x_write_reg(0, 0x0); //return to normal mode 
	if(ret_value ==500)
	{
		printk("[FTS] start auto CLB ft5x0x_write_reg(0, 0) err  \n");		
	}
	ret_value=ft5x0x_write_reg(0, 0x0); //return to normal mode 
	msleep(400);//msleep(300);

	//ret_value=ft5x0x_write_reg(0, 0x0); //return to normal mode 
	printk("[FTS] store CLB result OK.\n");
	focaltech_touch_reset(0);
	msleep(5);
	focaltech_touch_reset(1);		
	msleep(400);		
	return 0;

error:
	return -1;	
}
#if defined(FEATURE_TOUCH_ALL_FILE_UPDATE)
int fts_ctpm_fw_upgrade_with_i_file(int type_i)
#else
int fts_ctpm_fw_upgrade_with_i_file(void)
#endif
{
   FTS_BYTE*     pbt_buf = FTS_NULL;
   int i_ret=0;
    
    printk("[FocalTech] %s()\n", __func__);

    //=========FW upgrade========================*/
#if defined(FEATURE_TOUCH_ALL_FILE_UPDATE)
	if(type_i==0)  // I file update
	{
	   pbt_buf = CTPM_FW;
	   /*call the upgrade function*/
	   i_ret =  fts_ctpm_fw_upgrade(pbt_buf,sizeof(CTPM_FW));
	}
	else // all  file update
	{
	   pbt_buf = ALL_CTPM_FW;
		i_ret=fts_ctpm_all_file_fw_upgrade(pbt_buf,sizeof(ALL_CTPM_FW));
	}
#else
   pbt_buf = CTPM_FW;
   /*call the upgrade function*/
   i_ret =  fts_ctpm_fw_upgrade(pbt_buf,sizeof(CTPM_FW));
#endif
   if (i_ret != 0)
   {
       printk("[FTS] upgrade failed i_ret = %d.\n", i_ret);
       //error handling ...
       //TBD
   }
   else
   {
       printk("[FTS] upgrade successfully.\n");
       fts_ctpm_auto_clb();  //start auto CLB
   }
   return i_ret;
}

unsigned char fts_ctpm_get_i_file_ver(void)
{
    unsigned int ui_sz;
//    printk("[FocalTech] %s()\n", __func__);
    ui_sz = sizeof(CTPM_FW);
    if (ui_sz > 2)
    {
        return CTPM_FW[ui_sz - 2];
    }
    else
    {
        //TBD, error handling?
        return 0xff; //default value
    }
}

#define    FTS_SETTING_BUF_LEN        128

//update project setting
//only update these settings for COB project, or for some special case
int fts_ctpm_update_project_setting(void)
{
    unsigned char uc_i2c_addr;             //I2C slave address (8 bit address)
    unsigned char uc_io_voltage;           //IO Voltage 0---3.3v;	1----1.8v
    unsigned char uc_panel_factory_id;     //TP panel factory ID

    unsigned char buf[FTS_SETTING_BUF_LEN];
    FTS_BYTE reg_val[2] = {0};
    FTS_BYTE  auc_i2c_write_buf[10];
    FTS_BYTE  packet_buf[FTS_SETTING_BUF_LEN + 6];
    FTS_DWRD i = 0;
    int      i_ret;

    uc_i2c_addr = 0x70;
    uc_io_voltage = 0x0;
    uc_panel_factory_id = 0x5a;

    /*********Step 1:Reset  CTPM *****/
    /*write 0xaa to register 0xfc*/
    ft5x0x_write_reg(0xfc,0xaa);
    delay_qt_ms(50);
     /*write 0x55 to register 0xfc*/
    ft5x0x_write_reg(0xfc,0x55);
    printk("[FTS] Step 1: Reset CTPM test\n");
   
    delay_qt_ms(30);   

    /*********Step 2:Enter upgrade mode *****/
    auc_i2c_write_buf[0] = 0x55;
    auc_i2c_write_buf[1] = 0xaa;
    do
    {
        i ++;
        i_ret = ft5x0x_i2c_txdata(auc_i2c_write_buf, 2);
        delay_qt_ms(5);
    }while(i_ret <= 0 && i < 5 );

    /*********Step 3:check READ-ID***********************/        
    cmd_write(0x90,0x00,0x00,0x00,4);
    byte_read(reg_val,2);
    if (reg_val[0] == 0x79 && reg_val[1] == 0x3)
    {
        printk("[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
    }
    else
    {
        return ERR_READID;
    }

    cmd_write(0xcd,0x0,0x00,0x00,1);
    byte_read(reg_val,1);
    printk("bootloader version = 0x%x\n", reg_val[0]);


    /* --------- read current project setting  ---------- */
    //set read start address
    buf[0] = 0x3;
    buf[1] = 0x0;
    buf[2] = 0x78;
    buf[3] = 0x0;
    byte_write(buf, 4);
    byte_read(buf, FTS_SETTING_BUF_LEN);
    
    printk("[FTS] old setting: uc_i2c_addr = 0x%x, uc_io_voltage = %d, uc_panel_factory_id = 0x%x\n",
        buf[0],  buf[2], buf[4]);
    for (i = 0; i < FTS_SETTING_BUF_LEN; i++)
    {
        if (i % 16 == 0)     printk("\n");
        printk("0x%x, ", buf[i]);
        
    }
    printk("\n");

     /*--------- Step 4:erase project setting --------------*/
    cmd_write(0x62,0x00,0x00,0x00,1);
    delay_qt_ms(100);
   
    /*----------  Set new settings ---------------*/
    buf[0] = uc_i2c_addr;
    buf[1] = ~uc_i2c_addr;
    buf[2] = uc_io_voltage;
    buf[3] = ~uc_io_voltage;
    buf[4] = uc_panel_factory_id;
    buf[5] = ~uc_panel_factory_id;
    packet_buf[0] = 0xbf;
    packet_buf[1] = 0x00;
    packet_buf[2] = 0x78;
    packet_buf[3] = 0x0;
    packet_buf[4] = 0;
    packet_buf[5] = FTS_SETTING_BUF_LEN;
    for (i = 0; i < FTS_SETTING_BUF_LEN; i++)
    {
        packet_buf[6 + i] = buf[i];
        if (i % 16 == 0)     printk("\n");
        printk("0x%x, ", buf[i]);
    }
    printk("\n");
    byte_write(&packet_buf[0],FTS_SETTING_BUF_LEN + 6);
    delay_qt_ms(100);

    /********* reset the new FW***********************/
    cmd_write(0x07,0x00,0x00,0x00,1);

    msleep(200);

    return 0;
    
}



#if CFG_SUPPORT_AUTO_UPG

int fts_ctpm_auto_upg(void)
{
    unsigned char uc_host_fm_ver;
    unsigned char uc_tp_fm_ver;
    int           i_ret;

    uc_tp_fm_ver = ft5x0x_read_fw_ver();
    uc_host_fm_ver = fts_ctpm_get_i_file_ver();
#if 0
	if(1)
#else
#if 0
    if(uc_tp_fm_ver>=0x50)
    {
        printk("[FTS] Focal Test Version %x ==>0xf   uc_host_fm_ver=%x \n", uc_tp_fm_ver, uc_host_fm_ver);
        uc_tp_fm_ver=0x0f;
    }
#endif    
    if ( uc_tp_fm_ver == 0xa6  ||   //the firmware in touch panel maybe corrupted
         uc_tp_fm_ver < uc_host_fm_ver || //the firmware in host flash is new, need upgrade
         uc_tp_fm_ver==0x10  // 0x10 => 0x0f
        )
#endif        
    {
        msleep(100);
        printk("[FTS] uc_tp_fm_ver = 0x%x, uc_host_fm_ver = 0x%x\n",
            uc_tp_fm_ver, uc_host_fm_ver);
#if defined(FEATURE_TOUCH_ALL_FILE_UPDATE)
        i_ret = fts_ctpm_fw_upgrade_with_i_file(1);    
#else
        i_ret = fts_ctpm_fw_upgrade_with_i_file();    
#endif
        if (i_ret == 0)
        {
            msleep(300);
            //uc_host_fm_ver = fts_ctpm_get_i_file_ver();
			  uc_tp_fm_ver = ft5x0x_read_fw_ver();
            printk("[FTS] upgrade to new version 0x%x\n", uc_tp_fm_ver);
        }
        else
        {
            printk("[FTS] upgrade failed ret=%d.\n", i_ret);
        }
    }
	else
	{
	   printk("[FTS] Not upgrade  \n");
		//fts_ctpm_auto_clb();
	}
    return 0;
}

#endif

#endif


/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static void ft5x0x_ts_release(void)
{
	struct ft5x0x_ts_data *data = i2c_get_clientdata(this_client);
//	printk("[FocalTech] %s()\n", __func__);
	if (touch_last_key_code > 0) {
		input_report_key(data->input_dev, touch_last_key_code, 0);
		touch_last_key_code = 0;
    }
	input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
	input_report_key(data->input_dev, BTN_TOUCH, 0);
	input_sync(data->input_dev);
}


//read touch point information
static int ft5x0x_read_data(void)
{
	struct ft5x0x_ts_data *data = i2c_get_clientdata(this_client);
	struct ts_event *event = &data->event;
	u8 buf[CFG_POINT_READ_BUF] = {0};
	int ret = -1;
	int i;

//	printk("[FocalTech] %s()\n", __func__);
	ret = ft5x0x_i2c_rxdata(buf, CFG_POINT_READ_BUF);
	if (ret < 0) {
		printk("%s read_data i2c_rxdata failed: %d\n", __func__, ret);
		return ret;
	}
	memset(event, 0, sizeof(struct ts_event));
	event->touch_point = buf[2] & 0x07; //0x07 for 5-point,0x0F for 10-point feature

//	printk("[FocalTech] %s(num = %d)\n", __func__, event->touch_point);
	if (event->touch_point > CFG_MAX_TOUCH_POINTS)
	{
		event->touch_point = CFG_MAX_TOUCH_POINTS;
	}

	for (i = 0; i < event->touch_point; i++)
	{
		event->au16_x[i] = (s16)(buf[3 + 6*i] & 0x0F)<<8 | (s16)buf[4 + 6*i];
		event->au16_y[i] = (s16)(buf[5 + 6*i] & 0x0F)<<8 | (s16)buf[6 + 6*i];
		event->au8_touch_event[i] = buf[0x3 + 6*i] >> 6;
		event->au8_finger_id[i] = (buf[5 + 6*i])>>4;
	}

	event->pressure = 200;

	return 0;
}

/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/

#if CFG_SUPPORT_TOUCH_KEY
/* 
KEY Mapping (Y AREA)
	SEARCH	: 682 ~ 710
	BACK	: 721 ~ 740
	HOME	: 757 ~ 780
	MENU	: 790 ~ 800
*/
//#define TOUCH_KEY_SEARCH_BND_L          682
#define TOUCH_KEY_SEARCH_BND_L          677
#define TOUCH_KEY_SEARCH_BND_H          710
#define TOUCH_KEY_BACK_BND_L            721
#define TOUCH_KEY_BACK_BND_H            740
#define TOUCH_KEY_HOME_BND_L            757
#define TOUCH_KEY_HOME_BND_H            780
#define TOUCH_KEY_MENU_BND_L            790
#define TOUCH_KEY_MENU_BND_H            810


int ft5x0x_touch_key_process(struct input_dev *dev, int x, int y, int touch_event)
{
    int i;
    int key_id;
    
//    printk("[FocalTech] %s()\n", __func__);
    if ( y < TOUCH_KEY_MENU_BND_H && y > TOUCH_KEY_MENU_BND_L)
    {
        key_id = 0;	
    }
    else if ( y < TOUCH_KEY_HOME_BND_H && y > TOUCH_KEY_HOME_BND_L)
    {
        key_id = 1;	
    }
    
    else if ( y < TOUCH_KEY_BACK_BND_H &&y > TOUCH_KEY_BACK_BND_L)
    {
        key_id = 2;
    }  
    else if (y < TOUCH_KEY_SEARCH_BND_H && y > TOUCH_KEY_SEARCH_BND_L)
    {
        key_id = 3;
    }
    else
    {
        key_id = 0xf;

		printk("[FocalTech] [x = %4d, y = %4d] \n", x, y);
    }

    for(i = 0; i <CFG_NUMOFKEYS; i++ )
    {
    #if 0
        if(tsp_keystatus[i])
        {
            input_report_key(dev, tsp_keycodes[i], 0);
      
//            printk("[FTS] %s key is release. Keycode : %d\n", tsp_keyname[i], tsp_keycodes[i]);

            tsp_keystatus[i] = KEY_RELEASE;
        }
        else
    #endif
        if( key_id == i )
        {
            if( touch_event == 0)                                  // detect
            {
                if (touch_last_key_code != tsp_keycodes[i]) {
                    touch_last_key_code = tsp_keycodes[i];
                    input_report_key(dev, tsp_keycodes[i], 1);
                    //                printk( "[FTS] %s key is pressed. Keycode : %d\n", tsp_keyname[i], tsp_keycodes[i]);
                    tsp_keystatus[i] = KEY_PRESS;
                }
            }
        }
    }
    return 0;
    
}    
#endif

#if defined(FEATURE_TW_SYSTEM_SLEEP_WAKEUP)
int check_interrupt_x_y(int x,int y)
{
  //pattern y point 
  // (old_y_touch <582 || old_y_touch>1165)

  if((x>770 && x <1080)&&(y >230 && y<587))
  {
      // pattern point
      printk("[FocalTech] [ pattern point  x = %4d] [  y = %4d] \n",x, y);      
      return 0;
  }
  else
  {
      if((x>165 && x <460)&&(y >230 && y<587))
      {
        printk("[FocalTech] [ pattern point  x = %4d] [  y = %4d] \n",x, y);      
        return 0;
      }
      else
        printk("[FocalTech] [ No point  x = %4d] [  y = %4d] \n",x, y);      
      return 1;        
  }
   return 0;
}

void ft5x0x_emergency(int x, int y)
{

#if 1
//  int tmp_x=0, tmp_y=0;
    //printk("[FocalTech] [ old_y_touch x = %4d] [ old_y_touch y = %4d] \n",old_x_touch, old_y_touch);

	//if (old_y_touch <980 || old_y_touch>1120) 
    //if (old_y_touch <582 || old_y_touch>1165) 	
    if(check_interrupt_x_y(x,y)==1)
   {
		wake_touch_count++;
		//printk("wake_touch_count = [%d]\n", wake_touch_count);
		if (wake_touch_count == 4) {
			disable_irq_nosync(IRQ_EINT(4));	

			printk("wake_touch_count : HIT !!!============usr_shot_touch_count =%d \n",usr_shot_touch_count);
			ft5x0x_ts_release();

			focaltech_touch_reset(0);
			msleep(10);
			focaltech_touch_reset(1);			
			enable_irq(IRQ_EINT(4));
			wake_touch_count = 0;
//			old_x_touch = 0;
//			old_y_touch = 0;
			usr_shot_touch_count = 0;
			touch_emergency = 0;
          if(check_emer_timer==1)
             del_timer(&emer_timer);
			return ; 
		}
	}
   else 
   {
        printk("usr_shot_touch_count : touch_emergency cnt =%d wake_touch_count=%d\n", usr_shot_touch_count,wake_touch_count);
        //wake_touch_count = 0;
        usr_shot_touch_count++;
        if (usr_shot_touch_count ==30) 
        {
            printk("usr_shot_touch_count : touch_emergency 0 ===========\n");
            usr_shot_touch_count = 0;
            touch_emergency = 0;
//            old_x_touch = 0;
//            old_y_touch = 0;
            wake_touch_count = 0;     
            if(check_emer_timer==1)            
                del_timer(&emer_timer);            
        }          
   }

   	//old_x_touch = x;
	//old_y_touch = y;
#else
	//printk("++++ [FocalTech] ft5x0x_emergency : [x = %4d, y = %4d], wake_touch_count [%d]... \n", x, y, wake_touch_count);
	if ((old_x_touch == x)&&(old_y_touch == y)) {
		wake_touch_count++;
		//printk("wake_touch_count = [%d]\n", wake_touch_count);
		if (wake_touch_count == 20) {
			disable_irq_nosync(IRQ_EINT(4));	

			printk("wake_touch_count : HIT !!!============\n");
			ft5x0x_ts_release();

			gpio_request(S5PV210_GPJ2(6), "TOUCH_RESET");	
			gpio_direction_output(S5PV210_GPJ2(6), 1);
		
			gpio_set_value(S5PV210_GPJ2(6), 0);	
			msleep(10);	
			gpio_set_value(S5PV210_GPJ2(6), 1);
			gpio_free(S5PV210_GPJ2(6));
			msleep(10);

			enable_irq(IRQ_EINT(4));
			wake_touch_count = 0;
			old_x_touch = 0;
			old_y_touch = 0;
			usr_shot_touch_count = 0;
			touch_emergency = 0;
			return ; 
		}
	}
   else 
   {
        wake_touch_count = 0;
        usr_shot_touch_count++;

        if (usr_shot_touch_count == 15) 
        {
            printk("usr_shot_touch_count : touch_emergency 0 ===========\n");
            usr_shot_touch_count = 0;
            touch_emergency = 0;
            old_x_touch = 0;
            old_y_touch = 0;
        }          
   }
	old_x_touch = x;
	old_y_touch = y;
#endif

}
#endif

static void ft5x0x_report_value(void)
{
	struct ft5x0x_ts_data *data = i2c_get_clientdata(this_client);
	struct ts_event *event = &data->event;
	int i;

#ifdef T9_SIS_POINT_90_ROTATE
	int tmp_x, tmp_y;
#endif

#if defined(FEATURE_TW_TOUCH_EMERGENCY_OFF)
    if(touch_emergency_off==1)
    {
        check_touch_emergency_off=1;
      	disable_irq_nosync(IRQ_EINT(4));

#if defined(FEATURE_TW_SYSTEM_SLEEP_WAKEUP)
        if(check_emer_timer==1)
          del_timer(&emer_timer);
#endif

    	free_irq(IRQ_EINT(4), data);

    	focaltech_touch_off();

    	cancel_work_sync(&data->pen_event_work);
    	flush_workqueue(data->ts_workqueue);

#ifdef CONFIG_LEDS_T10
        t10_led_timer_on(0);
#endif
    	printk("==touch_emergency_off  kimsh touch end =\n");

        return ;
     }
#endif   

	//	printk("[FocalTech] %s()\n", __func__);
	//	printk("[FocalTech] count = %d\n", event->touch_point);
	for (i  = 0; i < event->touch_point; i++)
	{
		if (event->au16_x[i] < SCREEN_MAX_X && event->au16_y[i] < SCREEN_MAX_Y)
		// LCD view area
		{
			if (touch_last_key_code > 0) {
				//TOUCH KEY RELEASE
				input_report_key(data->input_dev, touch_last_key_code, 0);
				touch_last_key_code = 0;
			}

#ifdef T9_SIS_POINT_90_ROTATE
			tmp_x = event->au16_x[i];
			tmp_y = event->au16_y[i];
			//#6087 Portrait Mode인 상태에서 하단에서 상단으로 Drag시 터치 잘 인식되지 않음
			if(tmp_y ==0)
				event->au16_x[i] =  abs(SCREEN_MAX_Y-1);
			else
				event->au16_x[i] =  abs(tmp_y - SCREEN_MAX_Y);
			//event->au16_x[i] = SCREEN_MAX_Y - tmp_y;
			//#6087 Portrait Mode인 상태에서 하단에서 상단으로 Drag시 터치 잘 인식되지 않음
			if(tmp_x ==0)
				event->au16_y[i] =1;
			else
				event->au16_y[i] = tmp_x;

			//printk("[FocalTech] [x = %4d, y = %4d] \n", event->au16_x[i], event->au16_y[i]);
			input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->au16_x[i]);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->au16_y[i]);
#else
#ifdef T9_SIS_POINT_90_ROTATE
			tmp_x = event->au16_x[i];
			tmp_y = event->au16_y[i];
			event->au16_x[i] = SCREEN_MAX_Y - tmp_y;
			event->au16_y[i] = tmp_x;
			//		printk("[FocalTech] [x = %4d, y = %4d] \n", event->au16_x[i], event->au16_y[i]);
			input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->au16_x[i]);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->au16_y[i]);
#else
			input_report_abs(data->input_dev, ABS_MT_POSITION_X, event->au16_x[i]);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y, event->au16_y[i]);
#endif
#endif
			//printk("[FocalTech] [x = %4d, y = %4d] \n", event->au16_x[i], event->au16_y[i]);

#if defined(FEATURE_TW_SYSTEM_SLEEP_WAKEUP)
			if (touch_emergency) 
				ft5x0x_emergency(event->au16_x[i], event->au16_y[i]);		
#endif
			input_report_abs(data->input_dev, ABS_MT_WIDTH_MAJOR, 1);
			input_report_abs(data->input_dev, ABS_MT_TRACKING_ID, event->au8_finger_id[i]);
			input_report_key(data->input_dev, BTN_TOUCH, 1);
			if (event->au8_touch_event[i]== 0 || event->au8_touch_event[i] == 2)
			{
				input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, event->pressure);
			}
			else
			{
				input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR, 0);
			}
		}
		else //maybe the touch key area
		{
#if CFG_SUPPORT_TOUCH_KEY
			if (event->au16_x[i] >= SCREEN_MAX_X)
			{
				ft5x0x_touch_key_process(data->input_dev, event->au16_x[i], event->au16_y[i], event->au8_touch_event[i]);
			}
#endif
		}

		input_mt_sync(data->input_dev);
	}
	input_sync(data->input_dev);

	if (event->touch_point == 0) {
		ft5x0x_ts_release();
		return ; 
	}

	//	input_sync(data->input_dev);

}	/*end ft5x0x_report_value*/


/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static void ft5x0x_ts_pen_irq_work(struct work_struct *work)
{

	int ret = -1;

//	printk("[FocalTech] %s()\n", __func__);
#ifdef CONFIG_LEDS_T10
	t10_led_timer_on(t10_led_timer_set);
#endif
	ret = ft5x0x_read_data();	
	if (ret == 0) {	
		ft5x0x_report_value();
	}

	enable_irq(IRQ_EINT(4));
}
/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static irqreturn_t ft5x0x_ts_interrupt(int irq, void *dev_id)
{
	struct ft5x0x_ts_data *ft5x0x_ts = dev_id;

#ifdef  FIXED_TEMPORARY_SUSPEND_BUG
    if (suspend_flag == 1) {
        suspend_flag = 0;
        return IRQ_HANDLED;
    }
#endif
	//printk("[FocalTech] %s()\n", __func__);
	//disable_irq(IRQ_EINT(6));
	disable_irq_nosync(IRQ_EINT(4));
	//printk("-------------------------INT______-\n");
	
	if (!work_pending(&ft5x0x_ts->pen_event_work)) {
		queue_work(ft5x0x_ts->ts_workqueue, &ft5x0x_ts->pen_event_work);
	}

	return IRQ_HANDLED;
}
#ifdef CONFIG_HAS_EARLYSUSPEND
/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static void ft5x0x_ts_suspend(struct early_suspend *handler)
{
	struct ft5x0x_ts_data *ts;
	ts =  container_of(handler, struct ft5x0x_ts_data, early_suspend);

#if defined(FEATURE_TOUCH_ALL_FILE_UPDATE)
	check_suspend=1;
	set_touch_autoCal(0);
#endif

#if defined(FEATURE_TW_SYSTEM_SLEEP_WAKEUP)
	if(check_emer_timer==1)
	  del_timer(&emer_timer);
#endif
	printk("==ft5x0x_ts_suspend=\n");
	free_irq(IRQ_EINT(4), ts);

	focaltech_touch_off();

	cancel_work_sync(&ts->pen_event_work);
	flush_workqueue(ts->ts_workqueue);

#ifdef CONFIG_LEDS_T10
	t10_led_timer_on(0);
#endif

#ifdef  FIXED_TEMPORARY_SUSPEND_BUG
	suspend_flag = 1;
#endif

#if defined(FEATURE_TW_SYSTEM_SLEEP_WAKEUP)
  	touch_emergency = 0;
	printk("==touch_emergency - OFF\n");
#endif
//	disable_irq(this_client->irq);
//	disable_irq(IRQ_EINT(4));
//	cancel_work_sync(&ts->pen_event_work);
//	flush_workqueue(ts->ts_workqueue);
	// ==set mode ==, 
//    	ft5x0x_set_reg(FT5X0X_REG_PMODE, PMODE_HIBERNATE);
}
/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static void ft5x0x_ts_resume(struct early_suspend *handler)
{
    	int err;

    	struct ft5x0x_ts_data *ts;
    	ts =  container_of(handler, struct ft5x0x_ts_data, early_suspend);

    	printk("==ft5x0x_ts_resume=\n");

    	focaltech_touch_on();

    	err = request_irq(IRQ_EINT(4), ft5x0x_ts_interrupt, IRQF_TRIGGER_FALLING, "ft5x0x_ts", ts);
	if (err < 0) {
		printk("ft5x0x_ts_resume: request irq failed\n");
	} else {
		irq_set_irq_type(IRQ_EINT(4), IRQ_TYPE_EDGE_FALLING);
	}
	
#if defined(FEATURE_TW_SYSTEM_SLEEP_WAKEUP)
	touch_emergency = 1;
	printk("==touch_emergency - ON\n");
#endif
#if defined(FEATURE_TW_SYSTEM_SLEEP_WAKEUP)
	emer_timer_func();
#endif

#if defined(FEATURE_TOUCH_ALL_FILE_UPDATE)
	check_suspend=0;
#endif
	// wake the mode
//	__gpio_as_output(GPIO_FT5X0X_WAKE);		
//	__gpio_clear_pin(GPIO_FT5X0X_WAKE);		//set wake = 0,base on system
//	 msleep(100);
//	__gpio_set_pin(GPIO_FT5X0X_WAKE);			//set wake = 1,base on system
//	msleep(100);
//	enable_irq(this_client->irq);
//	enable_irq(IRQ_EINT(4));
}
#endif  //CONFIG_HAS_EARLYSUSPEND

/* sysfs */

static u8 ft5x0x_enter_factory(struct ft5x0x_ts_data *ft5x0x_ts)
{
	u8 regval;
	printk("[FocalTech] %s()\n", __func__);
	flush_workqueue(ft5x0x_ts->ts_workqueue);
	disable_irq_nosync(IRQ_EINT(4));
	ft5x0x_write_reg(0x00, 0x40);  //goto factory mode
    	delay_qt_ms(100);   //make sure already enter factory mode
	if(ft5x0x_read_reg(0x00, &regval)<0)
		pr_err("%s ERROR: could not read register\n", __FUNCTION__);
	else
	{
		if((regval & 0x70) != 0x40)
		{
			pr_err("%s() - ERROR: The Touch Panel was not put in Factory Mode. The Device Mode register contains 0x%02X\n", __FUNCTION__, regval);
			return -1;
		}
	}
	return 0;
}
static u8 ft5x0x_enter_work(struct ft5x0x_ts_data *ft5x0x_ts)
{
	u8 regval;
	printk("[FocalTech] %s()\n", __func__);
   	 ft5x0x_write_reg(0x00, 0x00); //return to normal mode 
   	 msleep(100);
	
	if(ft5x0x_read_reg(0x00, &regval)<0)
		pr_err("%s ERROR: could not read register\n", __FUNCTION__);
	else
	{
		if((regval & 0x70) != 0x00)
		{
			pr_err("%s() - ERROR: The Touch Panel was not put in Work Mode. The Device Mode register contains 0x%02X\n", __FUNCTION__, regval);
			enable_irq(IRQ_EINT(4));
			return -1;
		}
	}
	enable_irq(IRQ_EINT(4));
	return 0;
}

static int ft5x0x_GetFirmwareSize(char * firmware_name)
{
	struct file* pfile = NULL;
	struct inode *inode;
	unsigned long magic; 
	off_t fsize = 0; 
	char filepath[128];memset(filepath, 0, sizeof(filepath));

	printk("[FocalTech] %s()\n", __func__);
	sprintf(filepath, "/mnt/sdcard/focal_bin/%s", firmware_name);
	pr_info("filepath=%s\n", filepath);
	if(NULL == pfile){
		pfile = filp_open(filepath, O_RDONLY, 0);
	}
	if(IS_ERR(pfile)){
		pr_err("error occured while opening file %s.\n", filepath);
		return -1;
	}
	inode=pfile->f_dentry->d_inode; 
	magic=inode->i_sb->s_magic;
	fsize=inode->i_size; 
	filp_close(pfile, NULL);
	return fsize;
}
static int ft5x0x_ReadFirmware(char * firmware_name, unsigned char * firmware_buf)
{
	struct file* pfile = NULL;
	struct inode *inode;
	unsigned long magic; 
	off_t fsize; 
	char filepath[128];
	loff_t pos;

	mm_segment_t old_fs;

	memset(filepath, 0, sizeof(filepath));
	printk("[FocalTech] %s()\n", __func__);
	sprintf(filepath, "/mnt/sdcard/focal_bin/%s", firmware_name);
	printk("filepath=%s\n", filepath);
	if(NULL == pfile){
		pfile = filp_open(filepath, O_RDONLY, 0);
	}
	if(IS_ERR(pfile)){
		pr_err("error occured while opening file %s.\n", filepath);
		return -1;
	}
	inode=pfile->f_dentry->d_inode; 
	magic=inode->i_sb->s_magic;
	fsize=inode->i_size; 
	//char * buf;
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;

	vfs_read(pfile, firmware_buf, fsize, &pos);

	filp_close(pfile, NULL);
	set_fs(old_fs);
	return 0;
}

int fts_ctpm_fw_upgrade_with_app_file(char * firmware_name)
{
	FTS_BYTE*     pbt_buf = FTS_NULL;
	int i_ret; u8 fwver;
	int fwsize = ft5x0x_GetFirmwareSize(FIRMWARE_NAME);
	printk("[FocalTech] %s() fwsize=%d firmware_name=%s \n", __func__,fwsize,FIRMWARE_NAME);
	if(fwsize <= 0)
	{
		pr_err("%s ERROR:Get firmware size failed\n", __FUNCTION__);
		return -1;
	}
	//=========FW upgrade========================*/
	pbt_buf = (unsigned char *) kmalloc(fwsize+1,GFP_ATOMIC);
	if(ft5x0x_ReadFirmware(FIRMWARE_NAME, pbt_buf))
	{
		pr_err("%s() - ERROR: request_firmware failed\n", __FUNCTION__);
		kfree(pbt_buf);
		return -1;
	}

	set_touch_autoCal(1);
    msleep(50);  //clock change    
	s3c24xx_i2c_set(5,0x38,100);	
    msleep(50);  //clock change
	
	/*call the upgrade function*/
#if defined(FEATURE_TOUCH_ALL_FILE_UPDATE)
		i_ret=fts_ctpm_all_file_fw_upgrade(pbt_buf,fwsize);
#else
		i_ret =  fts_ctpm_fw_upgrade(pbt_buf, fwsize);
#endif
	
	if (i_ret != 0)
	{
		pr_err("%s() - ERROR:[FTS] upgrade failed i_ret = %d.\n",__FUNCTION__,  i_ret);
		//error handling ...
		//TBD
	}
	else
	{
		pr_info("[FTS] upgrade successfully.\n");
		if(ft5x0x_read_reg(FT5x06_REG_FW_VER, &fwver)>=0)
			pr_info("the new fw ver is 0x%02x\n", fwver);
		fts_ctpm_auto_clb();  //start auto CLB
	}

	msleep(50);  //clock change
	s3c24xx_i2c_set(5,0x38,300);	
	msleep(50);  //clock change
	printk(" [tsp] touch firmware update end...  \n ");
	set_touch_autoCal(0);		
	
	kfree(pbt_buf);
	return i_ret;
}
#if defined(FEATURE_TOUCH_ALL_FILE_UPDATE)
#if 0
static ssize_t ft5x0x_enable_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct ft5x0x_ts_data *data = NULL;
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	ssize_t num_read_chars = 0;
	//u8	   fwver = 0;

	data = (struct ft5x0x_ts_data *) i2c_get_clientdata( client );
	
	printk("[FocalTech] %s()\n", __func__);

#if 0
	mutex_lock(&data->device_mode_mutex);
	if(ft5x0x_read_reg(FT5x06_REG_FW_VER, &fwver) < 0)
		num_read_chars = snprintf(buf, PAGE_SIZE, "get tp fw version fail!\n");
	else
		num_read_chars = snprintf(buf, PAGE_SIZE, "%02X\n", fwver);
#endif

	mutex_unlock(&data->device_mode_mutex);
	return num_read_chars;
}

static ssize_t ft5x0x_enable_store(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct ft5x0x_ts_data *data = NULL;
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	ssize_t num_read_chars = 0;
	//u8	   fwver = 0;
	int return_val=0;
	
	data = (struct ft5x0x_ts_data *) i2c_get_clientdata( client );
	
	printk("[FocalTech] %s()\n", __func__);
	mutex_lock(&data->device_mode_mutex);

	set_touch_autoCal(1);
	if(check_suspend==1)
	{
		num_read_chars = snprintf(buf, PAGE_SIZE, "auto cal fail l!\n");
	}
	else
	{
		disable_irq(IRQ_EINT(4));
		return_val=fts_ctpm_auto_clb();
		if(return_val==0)
		{
			printk("[FocalTech] %s() autocal pass\n", __func__);
			num_read_chars = snprintf(buf, PAGE_SIZE, "auto cal pass l!\n");
		}
		else
		{
			printk("[FocalTech] %s() autocal fail \n", __func__);
			num_read_chars = snprintf(buf, PAGE_SIZE, "auto cal fail l!\n");
		}
		if(check_suspend==0)
		{
			msleep(300);
			enable_irq(IRQ_EINT(4));
			printk("[FocalTech] %s() autocal pass  \n", __func__);
		}
	}
	set_touch_autoCal(0);	
	
	mutex_unlock(&data->device_mode_mutex);
	return num_read_chars;
}
#endif
#endif

static ssize_t ft5x0x_tpfwver_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct ft5x0x_ts_data *data = NULL;
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	ssize_t num_read_chars = 0;
	u8	   fwver = 0;

	data = (struct ft5x0x_ts_data *) i2c_get_clientdata( client );
	
	printk("[FocalTech] %s()\n", __func__);
	mutex_lock(&data->device_mode_mutex);
	if(ft5x0x_read_reg(FT5x06_REG_FW_VER, &fwver) < 0)
		num_read_chars = snprintf(buf, PAGE_SIZE, "get tp fw version fail!\n");
	else
		num_read_chars = snprintf(buf, PAGE_SIZE, "0x%02X\n", fwver);

	mutex_unlock(&data->device_mode_mutex);
	return num_read_chars;
}

static ssize_t ft5x0x_tpfwver_store(struct device *dev,
					struct device_attribute *attr,
						const char *buf, size_t count)
{
	printk("[FocalTech] %s()\n", __func__);
	/* place holder for future use */
	return -EPERM;
}

static ssize_t ft5x0x_tprwreg_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	printk("[FocalTech] %s()\n", __func__);
	/* place holder for future use */
	return -EPERM;
}

static ssize_t ft5x0x_tprwreg_store(struct device *dev,
					struct device_attribute *attr,
						const char *buf, size_t count)
{
	struct ft5x0x_ts_data *data = NULL;
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	ssize_t num_read_chars = 0;
	int retval;
	u16 wmreg=0;u8 regaddr=0xff,regvalue=0xff;
	u8 valbuf[5];

	data = (struct ft5x0x_ts_data *) i2c_get_clientdata( client );

	printk("[FocalTech] %s()\n", __func__);
	memset(valbuf, 0, sizeof(valbuf));
	mutex_lock(&data->device_mode_mutex);
	num_read_chars = count - 1;

	if(num_read_chars!=2)
	{
		if(num_read_chars!=4)
		{
			pr_info("please input 2 or 4 character\n");
			goto error_return;
		}
	}

	memcpy(valbuf, buf, num_read_chars);
	retval = strict_strtoul(valbuf, 16, (long *)&wmreg);
	//pr_info("valbuf=%s wmreg=%x\n", valbuf, wmreg);
	if (0 != retval)
	{
		pr_err("%s() - ERROR: Could not convert the given input to a number. The given input was: \"%s\"\n", __FUNCTION__, buf);
		goto error_return;
	}

	if(2 == num_read_chars)
	{
		//read register
		regaddr = wmreg;
		if(ft5x0x_read_reg(regaddr, &regvalue) < 0)
			pr_err("Could not read the register(0x%02x)\n", regaddr);
		else
			pr_info("the register(0x%02x) is 0x%02x\n", regaddr, regvalue);
	}
	else
	{
		regaddr = wmreg>>8;
		regvalue = wmreg;
		if(ft5x0x_write_reg(regaddr, regvalue)<0)
			pr_err("Could not write the register(0x%02x)\n", regaddr);
		else
			pr_err("Write 0x%02x into register(0x%02x) successful\n", regvalue, regaddr);
	}
error_return:
	mutex_unlock(&data->device_mode_mutex);

	return count;
}


static ssize_t ft5x0x_fwupdate_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	/* place holder for future use */
    return -EPERM;
}
//upgrade from *.i
static ssize_t ft5x0x_fwupdate_store(struct device *dev,
					struct device_attribute *attr,
						const char *buf, size_t count)
{
	struct ft5x0x_ts_data *data = NULL;

	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	//	ssize_t num_read_chars = 0;
	u8 uc_host_fm_ver;
	int i_ret;

	data = (struct ft5x0x_ts_data *) i2c_get_clientdata( client );
	mutex_lock(&data->device_mode_mutex);

	disable_irq(IRQ_EINT(4));

#if defined(FEATURE_TOUCH_ALL_FILE_UPDATE)
    i_ret = fts_ctpm_fw_upgrade_with_i_file(0); // I file update
#else
    i_ret = fts_ctpm_fw_upgrade_with_i_file();
#endif

    if (i_ret == 0)
    {
        msleep(300);
        uc_host_fm_ver = fts_ctpm_get_i_file_ver();
        pr_info("%s [FTS] upgrade to new version 0x%x\n", __FUNCTION__, uc_host_fm_ver);
    }
    else
    {
        pr_err("%s ERROR:[FTS] upgrade failed ret=%d.\n", __FUNCTION__, i_ret);
    }
	enable_irq(IRQ_EINT(4));

	//error_return:
	mutex_unlock(&data->device_mode_mutex);

	return count;
}

static ssize_t ft5x0x_fwupgradeapp_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	/* place holder for future use */
    return -EPERM;
}
//upgrade from app.bin
static ssize_t ft5x0x_fwupgradeapp_store(struct device *dev,
					struct device_attribute *attr,
						const char *buf, size_t count)
{
	struct ft5x0x_ts_data *data = NULL;

	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
//	ssize_t num_read_chars = 0;
	char fwname[128];

	data = (struct ft5x0x_ts_data *) i2c_get_clientdata( client );

	memset(fwname, 0, sizeof(fwname));
	sprintf(fwname, "%s", buf);
	fwname[count-1] = '\0';

	mutex_lock(&data->device_mode_mutex);
	disable_irq(IRQ_EINT(4));

	fts_ctpm_fw_upgrade_with_app_file(fwname);

	enable_irq(IRQ_EINT(4));

// error_return:
	mutex_unlock(&data->device_mode_mutex);

	return count;
}

static ssize_t ft5x0x_rawdata_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct ft5x0x_ts_data *data = NULL;

	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	ssize_t num_read_chars = 0;
	int i=0, j=0;
	u16	RawData[FT5x0x_TX_NUM][FT5x0x_RX_NUM];

	data = (struct ft5x0x_ts_data *) i2c_get_clientdata( client );

	mutex_lock(&data->device_mode_mutex);

	if(fts_Get_RawData(RawData)<0)
		sprintf(buf, "%s", "could not get rawdata\n");
	else
	{
		for(i=0; i<FT5x0x_TX_NUM; i++)
		{
			for(j=0; j<FT5x0x_RX_NUM; j++)
			{
				num_read_chars += sprintf(&(buf[num_read_chars]), "%u ", RawData[i][j]);
			}
			buf[num_read_chars-1] = '\n';
		}
	}
// error_return:
	mutex_unlock(&data->device_mode_mutex);	
	return num_read_chars;
}
//upgrade from app.bin
static ssize_t ft5x0x_rawdata_store(struct device *dev,
					struct device_attribute *attr,
						const char *buf, size_t count)
{

	return -EPERM;
}


/* sysfs */
//static DEVICE_ATTR(rawbase, S_IRUGO|S_IWUSR, ft5x0x_rawbase_show, ft5x0x_rawbase_store);

#if defined(FEATURE_TOUCH_ALL_FILE_UPDATE)
//static DEVICE_ATTR(touchcal, S_IRUGO|S_IWUSR|S_IWGRP |S_IWOTH, ft5x0x_enable_show, ft5x0x_enable_store);
//static DEVICE_ATTR(touchcal, S_IRUGO|S_IWUSR|S_IWGRP, ft5x0x_enable_show, ft5x0x_enable_store);
#endif
static DEVICE_ATTR(ftstpfwver, S_IRUGO|S_IWUSR, ft5x0x_tpfwver_show, ft5x0x_tpfwver_store);
//upgrade from *.i
static DEVICE_ATTR(ftsfwupdate, S_IRUGO|S_IWUSR, ft5x0x_fwupdate_show, ft5x0x_fwupdate_store);
static DEVICE_ATTR(ftstprwreg, S_IRUGO|S_IWUSR, ft5x0x_tprwreg_show, ft5x0x_tprwreg_store);
//upgrade from app.bin 
static DEVICE_ATTR(ftsfwupgradeapp,S_IRUGO|S_IWUSR|S_IWGRP, ft5x0x_fwupgradeapp_show, ft5x0x_fwupgradeapp_store);
static DEVICE_ATTR(ftsrawdatashow, S_IRUGO|S_IWUSR, ft5x0x_rawdata_show, ft5x0x_rawdata_store);

static struct attribute *ft5x0x_attributes[] = {
#if defined(FEATURE_TOUCH_ALL_FILE_UPDATE)
	//&dev_attr_touchcal.attr,
#endif	
	&dev_attr_ftstpfwver.attr,
	&dev_attr_ftsfwupdate.attr,
	&dev_attr_ftstprwreg.attr,
	&dev_attr_ftsfwupgradeapp.attr,
	&dev_attr_ftsrawdatashow.attr,
	NULL
};

static struct attribute_group ft5x0x_attribute_group = {
	.attrs = ft5x0x_attributes
};

/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static int 
ft5x0x_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct ft5x0x_ts_data *ft5x0x_ts;
	struct input_dev *input_dev;
	int err = 0;
	unsigned char uc_reg_value; 
    unsigned char uc_host_fm_ver;
  
#if CFG_SUPPORT_TOUCH_KEY
    int i;
#endif
	
	printk("[FTS] ft5x0x_ts_probe, driver version is %s.\n", CFG_FTS_CTP_DRIVER_VERSION);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}

	ft5x0x_ts = kzalloc(sizeof(struct ft5x0x_ts_data), GFP_KERNEL);
	//ft5x0x_ts = kmalloc(sizeof(struct ft5x0x_ts_data), GFP_KERNEL);
	if (!ft5x0x_ts)	{
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}
	//memset(ft5x0x_ts, 0, sizeof(struct ft5x0x_ts_data));

	this_client = client;
	i2c_set_clientdata(client, ft5x0x_ts);

	mutex_init(&ft5x0x_ts->device_mode_mutex);
	INIT_WORK(&ft5x0x_ts->pen_event_work, ft5x0x_ts_pen_irq_work);

	ft5x0x_ts->ts_workqueue = create_singlethread_workqueue(dev_name(&client->dev));
	if (!ft5x0x_ts->ts_workqueue) {
		err = -ESRCH;
		goto exit_create_singlethread;
	}

    uc_reg_value = ft5x0x_read_fw_ver();
	if(uc_reg_value==0xA6)
	{
		err = -ESRCH;
		goto exit_create_singlethread;
	}
	set_touch_ic_check(1);

	err = request_irq(IRQ_EINT(4), ft5x0x_ts_interrupt, IRQF_TRIGGER_FALLING, "ft5x0x_ts", ft5x0x_ts);
	if (err < 0) {
		dev_err(&client->dev, "ft5x0x_probe: request irq failed\n");
		goto exit_irq_request_failed;
	}

#if defined(FEATURE_TW_TOUCH_EMERGENCY_OFF)
    touch_emergency_off=0; 
    check_touch_emergency_off=0;
#endif    

#if defined(FEATURE_TOUCH_ALL_FILE_UPDATE)
	check_suspend=0;
#endif

	disable_irq(IRQ_EINT(4));

	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		dev_err(&client->dev, "failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}

	ft5x0x_ts->input_dev = input_dev;

	set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit);
	set_bit(ABS_MT_POSITION_X, input_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, input_dev->absbit);
	set_bit(ABS_MT_WIDTH_MAJOR, input_dev->absbit);

#ifdef T9_SIS_POINT_90_ROTATE
	input_set_abs_params(input_dev,
			     ABS_MT_POSITION_X, 0, SCREEN_MAX_Y, 0, 0);
	input_set_abs_params(input_dev,
			     ABS_MT_POSITION_Y, 0, SCREEN_MAX_X, 0, 0);
#else
	input_set_abs_params(input_dev,
			     ABS_MT_POSITION_X, 0, SCREEN_MAX_X, 0, 0);
	input_set_abs_params(input_dev,
			     ABS_MT_POSITION_Y, 0, SCREEN_MAX_Y, 0, 0);
#endif
	input_set_abs_params(input_dev,
			     ABS_MT_TOUCH_MAJOR, 0, PRESS_MAX, 0, 0);
	input_set_abs_params(input_dev,
			     ABS_MT_WIDTH_MAJOR, 0, 200, 0, 0);
    input_set_abs_params(input_dev,
			     ABS_MT_TRACKING_ID, 0, 5, 0, 0);


	set_bit(EV_KEY, input_dev->evbit);
	set_bit(EV_ABS, input_dev->evbit);

#if CFG_SUPPORT_TOUCH_KEY
    //setup key code area
    set_bit(EV_SYN, input_dev->evbit);
    set_bit(BTN_TOUCH, input_dev->keybit);
    input_dev->keycode = tsp_keycodes;
    for(i = 0; i < CFG_NUMOFKEYS; i++)
    {
        input_set_capability(input_dev, EV_KEY, ((int*)input_dev->keycode)[i]);
        tsp_keystatus[i] = KEY_RELEASE;
    }
#endif

	input_dev->name		= FT5X0X_NAME;		//dev_name(&client->dev)
	err = input_register_device(input_dev);
	if (err) {
		dev_err(&client->dev,
		"ft5x0x_ts_probe: failed to register input device: %s\n",
		dev_name(&client->dev));
		goto exit_input_register_device_failed;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	printk("==register_early_suspend =\n");
	ft5x0x_ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ft5x0x_ts->early_suspend.suspend = ft5x0x_ts_suspend;
	ft5x0x_ts->early_suspend.resume	= ft5x0x_ts_resume;
	register_early_suspend(&ft5x0x_ts->early_suspend);
#endif

    msleep(150);  //make sure CTP already finish startup process

    uc_host_fm_ver = fts_ctpm_get_i_file_ver();
    //get some register information
    printk("[FTS] ============== Touch Information Start ==============\n");
    printk("[FTS] Firmware version = 0x%x  host version=%x \n", uc_reg_value,uc_host_fm_ver);
    ft5x0x_read_reg(FT5X0X_REG_PERIODACTIVE, &uc_reg_value);
    printk("[FTS] report rate is %dHz.\n", uc_reg_value * 10);
    ft5x0x_read_reg(FT5X0X_REG_THGROUP, &uc_reg_value);
    printk("[FTS] touch threshold is %d.\n", uc_reg_value * 4);
    printk("[FTS] ============== Touch Information End ================\n");

#if 0
#if CFG_SUPPORT_AUTO_UPG
	set_touch_autoCal(1);
    msleep(50);  //clock change    
	s3c24xx_i2c_set(5,0x38,95);	
    msleep(50);  //clock change
    fts_ctpm_auto_upg();

	set_touch_autoCal(0);
    msleep(50);  //clock change    
	s3c24xx_i2c_set(5,0x38,300);	
    msleep(50);  //clock change
#endif    
#endif  

#if CFG_SUPPORT_UPDATE_PROJECT_SETTING
    fts_ctpm_update_project_setting();
#endif
    enable_irq(IRQ_EINT(4));

	//create sysfs
	err = sysfs_create_group(&client->dev.kobj, &ft5x0x_attribute_group);
	if (0 != err)
	{
		dev_err(&client->dev, "%s() - ERROR: sysfs_create_group() failed: %d\n", __FUNCTION__, err);
		sysfs_remove_group(&client->dev.kobj, &ft5x0x_attribute_group);
	}
	else
	{
		printk("ft5x0x:%s() - sysfs_create_group() succeeded.\n", __FUNCTION__);
	}
	printk("[FTS] ==probe over =\n");
	return 0;

exit_input_register_device_failed:
	input_free_device(input_dev);
exit_input_dev_alloc_failed:
//	free_irq(client->irq, ft5x0x_ts);
	free_irq(IRQ_EINT(4), ft5x0x_ts);
exit_irq_request_failed:
//exit_platform_data_null:
	cancel_work_sync(&ft5x0x_ts->pen_event_work);
	destroy_workqueue(ft5x0x_ts->ts_workqueue);
exit_create_singlethread:
	printk("==singlethread error =\n");
	s3c_i2c5_force_stop();
	
	i2c_set_clientdata(client, NULL);
	kfree(ft5x0x_ts);
	set_touch_ic_check(0);
	
exit_alloc_data_failed:
exit_check_functionality_failed:
	return err;
}
/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static int __devexit ft5x0x_ts_remove(struct i2c_client *client)
{
	struct ft5x0x_ts_data *ft5x0x_ts;
	printk("==ft5x0x_ts_remove=\n");

	focaltech_touch_off();

	if(check_used_touch_ldo==1)
		regulator_put(touch_ldo);
	
	ft5x0x_ts = i2c_get_clientdata(client);
	/* early suspend removed */
//	free_irq(client->irq, ft5x0x_ts);
	mutex_destroy(&ft5x0x_ts->device_mode_mutex);
	free_irq(IRQ_EINT(4), ft5x0x_ts);
	input_unregister_device(ft5x0x_ts->input_dev);
	kfree(ft5x0x_ts);
	cancel_work_sync(&ft5x0x_ts->pen_event_work);
	destroy_workqueue(ft5x0x_ts->ts_workqueue);
	i2c_set_clientdata(client, NULL); 
//	del_timer(&test_timer);

	return 0;
}

static const struct i2c_device_id ft5x0x_ts_id[] = {
	{ FT5X0X_NAME, 0 },{ }
};


MODULE_DEVICE_TABLE(i2c, ft5x0x_ts_id);

static void ft5x0x_ts_shutdown(struct i2c_client *client)
{
	struct ft5x0x_ts_data *ft5x0x_ts;
	printk("==ft5x0x_ts_shutdown=\n");
	ft5x0x_ts = i2c_get_clientdata(client);

    if(check_touch_emergency_off==0)
      {
		 disable_irq_nosync(IRQ_EINT(4));
#ifdef CONFIG_LEDS_T10
        t10_led_timer_on(0);
#endif
        printk(" ft5x0x_ts_shutdown \n");

#if defined(FEATURE_TW_SYSTEM_SLEEP_WAKEUP)
        if(check_emer_timer==1)
            del_timer(&emer_timer);
#endif
        free_irq(IRQ_EINT(4), ft5x0x_ts);

        focaltech_touch_off();

        cancel_work_sync(&ft5x0x_ts->pen_event_work);
        flush_workqueue(ft5x0x_ts->ts_workqueue);

#ifdef  FIXED_TEMPORARY_SUSPEND_BUG
        suspend_flag = 0;
#endif

#if defined(FEATURE_TW_SYSTEM_SLEEP_WAKEUP)
        touch_emergency = 0;
        printk("touch_emergency - OFF\n");
#endif
   }
}

static struct i2c_driver ft5x0x_ts_driver = {
	.probe		= ft5x0x_ts_probe,
	.remove		= __devexit_p(ft5x0x_ts_remove),
	.shutdown = ft5x0x_ts_shutdown,
	.id_table	= ft5x0x_ts_id,
	.driver	= {
		.name	= FT5X0X_NAME,
		.owner	= THIS_MODULE,
	},
};

/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static int __init ft5x0x_ts_init(void)
{
	int ret;
	printk("==ft5x0x_ts_init==\n");

	check_used_touch_ldo=0;
	check_touch_power=0;
	
#ifdef  FIXED_TEMPORARY_SUSPEND_BUG
    suspend_flag = 0;
#endif

    touch_ldo = regulator_get(NULL, "vdd_tsp");
	ret=regulator_set_voltage(touch_ldo, 3000000, 3000000);

    focaltech_touch_on();
	check_used_touch_ldo=1;
	ret = i2c_add_driver(&ft5x0x_ts_driver);
	printk("ret=%d\n",ret);

	return ret;
//	return i2c_add_driver(&ft5x0x_ts_driver);
}

/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static void __exit ft5x0x_ts_exit(void)
{
	printk("==ft5x0x_ts_exit==\n");
	i2c_del_driver(&ft5x0x_ts_driver);
}

module_init(ft5x0x_ts_init);
module_exit(ft5x0x_ts_exit);

MODULE_AUTHOR("<wenfs@Focaltech-systems.com>");
MODULE_DESCRIPTION("FocalTech ft5x0x TouchScreen driver");
MODULE_LICENSE("GPL");
