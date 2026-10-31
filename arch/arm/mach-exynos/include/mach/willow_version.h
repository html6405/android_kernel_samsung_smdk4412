#ifndef __WILLOW_VERSION_H__
#define __WILLOW_VERSION_H__
/*
 * --------------------------------------------------
 * VER | ADC2_HW_VER0 | ADC1_HW_VER0 | ADC0_HW_VER0 |
 * --------------------------------------------------
 * WS1 |       0      |       0      |       0      | //0
 * --------------------------------------------------
 * ES1 |       0      |       0      |       1      | //1
 * --------------------------------------------------
 * ES2 |       0      |       1      |       0      | //2
 * --------------------------------------------------
 * PP  |       1      |       0      |       0      | //4
 * --------------------------------------------------
 */

typedef enum {
	WILLOW_HW_DVT = 0,
	WILLOW_HW_MVT = 1,
	WILLOW_HW_PP = 2,
	WILLOW_HW_MP = 3,
	WILLOW_HW_MP2 = 4,
	WILLOW_HW_RESERVED0 = 5,
	WILLOW_HW_RESERVED1 = 6,
	WILLOW_HW_RESERVED2 = 7,
	WILLOW_HW_UNKNOWN = 8,
} WILLOW_HW_VERSION;

extern WILLOW_HW_VERSION g_willow_hw_version;

#endif /*__WILLOW_VERSION_H__*/
