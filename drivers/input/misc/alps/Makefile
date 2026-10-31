############################################################################
#
# Makefile
#
# Copyright (C) 2011-2012 ALPS ELECTRIC CO., LTD. All Rights Reserved.
#
# This software is licensed under the terms of the GNU General Public
# License version 2, as published by the Free Software Foundation, and
# may be copied, distributed, and modified under those terms.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
############################################################################

# Debug Option
#EXTRA_CFLAGS += -DALPS_DEBUG

# Select Build Module
#MODULES = alps-input.o hscdtd004a_i2c.o bma150_i2c.o
#MODULES = alps-input.o hscdtd004a_i2c.o mma8450_i2c.o
#MODULES = alps-input.o hscdtd004a_i2c.o kxtf9_i2c.o
MODULES = alps-input.o hscdtd007a_i2c.o bma150_i2c.o

# Kernel Root Directory
#ROOTDIR = $(ANDROID_R)/kernel-beagleboard-xm
#ROOTDIR = $(MY_WORK)/kernel
ROOTDIR = $('pwd'../../../..)

##############################################

# Select input device range for accelerometer sensor
ifeq ($(findstring bma150, $(MODULES)), bma150)
    EXTRA_CFLAGS += -DACC_10BIT
else
    EXTRA_CFLAGS += -DACC_12BIT
endif

# Select input device range for geomagnetic sensor
ifeq ($(findstring hscdtd004a, $(MODULES)), hscdtd004a)
    EXTRA_CFLAGS += -DMAG_13BIT
else
    EXTRA_CFLAGS += -DMAG_15BIT
endif

$(info ALPS IF Driver build option : $(EXTRA_CFLAGS))

##############################################

ifneq ($(KERNELRELEASE), )
   
#obj-m := $(MODULES)
obj-y := $(MODULES)

else

include $(ROOTDIR).config

all: modules

modules:
	make -C $(ROOTDIR) M=${shell pwd} modules

clean:
	-rm -f *.[oas] *.ko *.mod.c .*.d .*.tmp .*.cmd *.symvers
	-rm -rf .tmp_versions modules.order

endif
