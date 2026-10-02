/*
 *  Copyright (C) 2009 Samsung Electronics
 *  Minkyu Kang <mk7.kang@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __MAX17040_BATTERY_H_
#define __MAX17040_BATTERY_H_

/*
 * TODO: the parts below should be cleared and modified properly with
 * a precise h/w naming scheme.
 *
 */
struct max8903_output_desc {
	/* Configuration parameters */
	int gpio;
	int active_low;
	char *desc;
	int wakeup;		/* configure the button as a wake-up source */
	int debounce_interval;	/* debounce ticks interval in msecs */
	bool can_disable;
  int enable_int;
  int irq;
};
/*
 * should be cleared as the struct above came from T10/9S to make
 * the current code to be compiled.
 */


struct max17040_platform_data {
  /*
   * TODO: need to be cleared,
   * and came from the old code.
   * refer to the TODO above
   */
  struct max8903_output_desc *output_desc;
  int nOutputs;
  int chg_en_gpio;
	unsigned int rep:1;		/* enable input subsystem auto repeat */
  /*
   * end TODO:
   */
	int (*battery_online)(void);
	int (*charger_online)(void);
	int (*charger_enable)(void);
//	int (*power_supply_register)(struct device *parent,
//			struct power_supply *psy);
//	void (*power_supply_unregister)(struct power_supply *psy);

  /*
   * TODO: need to be cleared,
   * and came from the old code.
   * refer to the TODO above
   */
	u16 rcomp_value;
	int (*charger_done)(void);
	void (*charger_disable)(void);
  /*
   * end TODO:
   */

};

#endif
