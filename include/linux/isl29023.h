#ifndef _LINUX_ISL29023_H__
#define _LINUX_ISL29023_H__

#include <linux/types.h>

#define MODULE_NAME	"isl29023"

/* registers */
#define isl29023_REG_VENDOR_REV                 0x06
#define isl29023_VENDOR                         1
#define isl29023_VENDOR_MASK                    0x0F
#define isl29023_REV                            4
#define isl29023_REV_SHIFT                      4
#define isl29023_REG_DEVICE                     0x11
#define isl29023_DEVICE                         11

#define REG_CMD_1		    0x00
#define REG_CMD_2		    0x01
#define REG_DATA_LSB		0x02
#define REG_DATA_MSB		0x03
#define ISL_MOD_MASK		0xE0
#define ISL_MOD_POWERDOWN	0
#define ISL_MOD_ALS_ONCE	1
#define ISL_MOD_IR_ONCE		2
#define ISL_MOD_PS_ONCE		3
#define ISL_MOD_RESERVED	4
#define ISL_MOD_ALS_CONT	5
#define ISL_MOD_IR_CONT		6
#define ISL_MOD_PS_CONT		7
#define IR_CURRENT_MASK		0xC0
#define IR_FREQ_MASK		0x30
#define SENSOR_RANGE_MASK	0x03
#define ISL_RES_MASK		0x0C

struct isl29023_i2c_platform_data {
	unsigned int irq_gpio;
};

#endif	/* _LINUX_ISL29030_H__ */
