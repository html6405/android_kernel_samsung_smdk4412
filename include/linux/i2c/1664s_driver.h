/*
*  Copyright (C) 2010, Samsung Electronics Co. Ltd. All Rights Reserved.
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*/

#ifndef __MXT_H__
#define __MXT_H__
#include <linux/semaphore.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/wakelock.h>
#define MXT_DEV_NAME "Atmel MXT1664S"

/* Feature */
/*#######################################*/
#define TOUCH_BOOSTER                           1
#define USE_SUMSIZE                                     0
#define SYSFS   0
#define FOR_BRINGUP  1
#define UPDATE_ON_PROBE   0
#define READ_FW_FROM_HEADER     0
#define FOR_DEBUGGING_TEST_DOWNLOADFW_BIN 0
#define ITDEV   1
#define SHOW_COORDINATE 0
#define DEBUG_INFO     0

#define FEATURE_TOUCH_PASSIVEPEN
#define FEATURE_TOUCH_ACTIVEPEN
#define FEATURE_TOUCH_CONFIG_UPDATE
#define FEATURE_TOUCH_1ST_POINT
#define FEATURE_TOUCH_DEBUG
#define FEATURE_TOUCH_NOISE

#define S_PEN_USER 00666
/*#######################################*/

/* ATMEL Defined  Start */
#define OBJECT_TABLE_START_ADDRESS      7
#define OBJECT_TABLE_ELEMENT_SIZE       6

#define CMD_RESET_OFFSET                0
#define CMD_BACKUP_OFFSET               1
#define CMD_CALIBRATE_OFFSET    2
#define CMD_REPORTATLL_OFFSET   3
#define CMD_DEBUG_CTRL_OFFSET   4
#define CMD_DIAGNOSTIC_OFFSET   5


#define DETECT_MSG_MASK         0x80
#define PRESS_MSG_MASK                  0x40
#define RELEASE_MSG_MASK                0x20
#define MOVE_MSG_MASK                   0x10
#define AMPLITUDE_MSG_MASK      0x04
#define SUPPRESS_MSG_MASK               0x02

/* Slave addresses */
/* need to check +  */
#define MXT_APP_LOW             0x4a
#define MXT_APP_HIGH            0x4d
#define MXT_BOOT_LOW            0x24 //0x26
#define MXT_BOOT_HIGH           0x25 //0x27
/* need to check -  */

/* FIRMWARE NAME */
#define MXT_FW_NAME                     "tsp_atmel/mXT1664S.fw"
#define MXT_BOOT_VALUE          0xa5
#define MXT_BACKUP_VALUE                0x55

/* Bootloader mode status */
#define MXT_WAITING_BOOTLOAD_CMD        0xc0    /* valid 7 6 bit only */
#define MXT_WAITING_FRAME_DATA  0x80    /* valid 7 6 bit only */
#define MXT_FRAME_CRC_CHECK     0x02
#define MXT_FRAME_CRC_FAIL              0x03
#define MXT_FRAME_CRC_PASS              0x04
#define MXT_APP_CRC_FAIL                0x40    /* valid 7 8 bit only */
#define MXT_BOOT_STATUS_MASK    0x3f

/* Command to unlock bootloader */
#define MXT_UNLOCK_CMD_MSB              0xaa
#define MXT_UNLOCK_CMD_LSB              0xdc

#define ID_BLOCK_SIZE                   7

#define MXT_STATE_INACTIVE              -1
#define MXT_STATE_RELEASE               0
#define MXT_STATE_PRESS         1
#define MXT_STATE_MOVE          2

#define MAX_USING_FINGER_NUM 10

#define MXT_SW_RESET_TIME               300             /* msec */
#define MXT_HW_RESET_TIME               300     /* msec */

enum { 
	RESERVED_T0 = 0,
	RESERVED_T1,
	DEBUG_DELTAS_T2,
	DEBUG_REFERENCES_T3,
	DEBUG_SIGNALS_T4,
	GEN_MESSAGEPROCESSOR_T5,
	GEN_COMMANDPROCESSOR_T6,
	GEN_POWERCONFIG_T7,
	GEN_ACQUISITIONCONFIG_T8,
	TOUCH_MULTITOUCHSCREEN_T9,
	TOUCH_SINGLETOUCHSCREEN_T10,
	TOUCH_XSLIDER_T11,
	TOUCH_YSLIDER_T12,
	TOUCH_XWHEEL_T13,
	TOUCH_YWHEEL_T14,
	TOUCH_KEYARRAY_T15,
	PROCG_SIGNALFILTER_T16,
	PROCI_LINEARIZATIONTABLE_T17,
	SPT_COMMSCONFIG_T18, 
	SPT_GPIOPWM_T19,
	PROCI_GRIPFACESUPPRESSION_T20,
	RESERVED_T21,
	PROCG_NOISESUPPRESSION_T22,
	TOUCH_PROXIMITY_T23,
	PROCI_ONETOUCHGESTUREPROCESSOR_T24,
	SPT_SELFTEST_T25,
	DEBUG_CTERANGE_T26,
	PROCI_TWOTOUCHGESTUREPROCESSOR_T27,
	SPT_CTECONFIG_T28,
	SPT_GPI_T29,
	SPT_GATE_T30,
	TOUCH_KEYSET_T31,
	TOUCH_XSLIDERSET_T32,
	RESERVED_T33,
	GEN_MESSAGEBLOCK_T34,
	SPT_GENERICDATA_T35,
	RESERVED_T36,
	DEBUG_DIAGNOSTIC_T37,
	SPT_USERDATA_T38,
	SPARE_T39,
	PROCI_GRIPSUPPRESSION_T40,
	SPARE_T41,
	PROCI_TOUCHSUPPRESSION_T42,
	SPT_DIGITIZER_T43,
	SPARE_T44,
	SPARE_T45,
	SPT_CTECONFIG_T46,
	PROCI_STYLUS_T47,
	PROCG_NOISESUPPRESSION_T48,
	SPARE_T49,
	SPARE_T50,
	SPARE_T51,
	TOUCH_PROXIMITY_KEY_T52,
	GEN_DATASOURCE_T53,
	SPARE_T54,
	PROCI_ADAPTIVETHRESHOLD_T55,
	PROCI_SHIELDLESS_T56,
	PROCI_EXTRATOUCHSCREENDATA_T57,
	SPT_TIMER_T61=61,
	PROCG_NOISESUPPRESSION_T62=62,
	PROCI_ACTIVESTYLUS_T63=63,
	PROCI_LENSBENDING_T65=65,
	SPT_GOLDENREFERENCES_T66=66,
	SPT_SERIALDATACOMMAND_T68=68,
	RESERVED_T255 = 255,
};

struct mxt_platform_data 
{
	int max_finger_touches;
	int gpio_read_done;
	const u8 *config;
	size_t config_length;
	int min_x;
	int max_x;
	int min_y;
	int max_y;
	int min_z;
	int max_z;
	int min_w;
	int max_w;
	void (*power_on) (void);
	void (*power_off) (void);
	void (*read_ta_status) (bool *);
	int boot_address;
};

enum {
	MXT_PAGE_UP =           0x01,
	MXT_PAGE_DOWN =         0x02,
	MXT_DELTA_MODE =        0x10,
	MXT_REFERENCE_MODE =    0x11,
	MXT_CTE_MODE =          0x31
};

extern struct class *sec_class;

struct object_t {
	u8 object_type;
	u16 i2c_address;
	u8 size;
	u8 instances;
	u8 num_report_ids;
} __packed;

struct finger_info {
	s16 x;
	s16 y;
	s16 z;
	u16 w;
	s8 state;
	int16_t component;
	u16 mcount;     /*add for debug*/
};

struct report_id_map_t {
	u8 object_type;     /*!< Object type. */
	u8 instance;        /*!< Instance number. */
};

#include <linux/hrtimer.h>

struct mxt_data 
{
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct mxt_platform_data *pdata;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
#ifdef FEATURE_TOUCH_1ST_POINT	
	struct hrtimer timer;
#endif
	//struct work_struct work;
	u8 family_id;
	u32 finger_mask;
	int gpio_read_done;
	struct object_t *objects;
	u8 objects_len;
	u8 tsp_version;
	u8 tsp_build;
	u8 tsp_variant;
	u8 finger_type;
	u16 msg_proc;
	u16 cmd_proc;
	u16 msg_object_size;
	u32 x_dropbits; 
	u32 y_dropbits;
	u8 tchthr_batt;
	u8 tchthr_charging;
	u8 calcfg_batt;
	u8 calcfg_charging;
	const u8 *t48_config_batt;
	const u8 *t48_config_chrg;
#if TOUCH_BOOSTER
	struct delayed_work dvfs_dwork;
#endif
	void (*power_on)(void);
	void (*power_off)(void);
	void (*register_cb)(void *);
	void (*read_ta_status)(bool *);
	int num_fingers;
#if ITDEV
	u16 last_read_addr;
	u16 msg_proc_addr;
#endif
	struct finger_info fingers[];
};
 

enum {
	/* use in_kernel mode only for test */
	MXT540E_FIRM_IN_KERNEL = 0,
	MXT540E_FIRM_EXTERNAL,
};

typedef struct {
	uint8_t reset;       /*  Force chip reset             */
	uint8_t backupnv;    /*  Force backup to eeprom/flash */
	uint8_t calibrate;   /*  Force recalibration          */
	uint8_t reportall;   /*  Force all objects to report  */
	uint8_t reserved;
	uint8_t diagnostic;  /*  Controls the diagnostic object */
} __packed gen_commandprocessor_t6_config_t;

typedef struct {
	uint8_t nIDLEACQINT;
	uint8_t nACTVACQINT;
	uint8_t nACTV2IDLETO;
	uint8_t nCFG;
} __packed gen_powerconfig_t7_config_t;

typedef struct {
	uint8_t nCHRGTIME;
	uint8_t nATCHDRIFT;
	uint8_t nTCHDRIFT;
	uint8_t nDRIFTST;
	uint8_t nTCHAUTOCAL;
	uint8_t nSYNC;
	uint8_t nATCHCALST;
	uint8_t nATCHCALSTHR;
	uint8_t nATCHFRCCALTHR;
	uint8_t nATCHFRCCALRATIO;
} __packed gen_acquisitionconfig_t8_config_t;

typedef struct {
	uint8_t nCTRL;
	uint8_t nXORIGIN;
	uint8_t nYORIGIN;
	uint8_t nXSIZE;
	uint8_t nYSIZE;
	uint8_t nAKSCFG;
	uint8_t nBLEN;
	uint8_t nTCHTHR;
	uint8_t nTCHDI;
	uint8_t nORIENT;
	uint8_t nMRGTIMEOUT;
	uint8_t nMOVHYSTI;
	uint8_t nMOVHYSTN;
	uint8_t nMOVFILTER;
	uint8_t nNUMTOUCH;
	uint8_t nMRGHYST;
	uint8_t nMRGTHR;
	uint8_t nAMPHYST;
	uint16_t nXRANGE;
	uint16_t nYRANGE;
	uint8_t nXLOCLIP;
	uint8_t nXHICLIP;
	uint8_t nYLOCLIP;
	uint8_t nYHICLIP;
	uint8_t nXEDGECTRL;
	uint8_t nXEDGEDIST;
	uint8_t nYEDGECTRL;
	uint8_t nYEDGEDIST;
	uint8_t nJUMPLIMIT;
	uint8_t nTCHHYST;
	uint8_t nXPITCH;
	uint8_t nYPITCH;
	uint8_t nNEXTTCHDI;
	uint8_t nCFG;
} __packed touch_multitouchscreen_t9_config_t;

typedef struct {
	uint8_t nCTRL;
	uint8_t nXORIGIN;
	uint8_t nYORIGIN;
	uint8_t nXSIZE;
	uint8_t nYSIZE;
	uint8_t nAKSCFG;
	uint8_t nBLEN;
	uint8_t nTCHTHR;
	uint8_t nTCHDI;
	uint8_t nRESERVED[2];
} __packed touch_keyarray_t15_config_t;

typedef struct {
	uint8_t nCTRL;
	uint8_t nCOMMAND;
} __packed spt_comcconfig_t18_config_t;

/* GPIOPWM Configuration */
typedef struct {
	uint8_t ctrl;             /*  Main configuration field           */
	uint8_t reportmask;       /*  Event mask for generating messages
				      to the host */
	uint8_t dir;              /*  Port DIR register   */
	uint8_t intpullup;        /*  Port pull-up per pin enable register */
	uint8_t out;              /*  Port OUT register*/
	uint8_t wake;             /*  Port wake on change enable register  */
	uint8_t pwm;              /*  Port pwm enable register    */
	uint8_t period;           /*  PWM period (min-max) percentage*/
	uint8_t duty[4];          /*  PWM duty cycles percentage */
	uint8_t trigger[4];       /*  Trigger for GPIO */
} __packed spt_gpiopwm_t19_config_t;

typedef struct {
	uint8_t ctrl;
	uint8_t xlogrip;
	uint8_t xhigrip;
	uint8_t ylogrip;
	uint8_t yhigrip;
	uint8_t maxtchs;
	uint8_t reserved;
	uint8_t szthr1;
	uint8_t szthr2;
	uint8_t shpthr1;
	uint8_t shpthr2;
	uint8_t supextto;
} __packed proci_gripfacesuppression_t20_config_t;


typedef struct {
	uint8_t ctrl;
	uint8_t reserved;
	uint8_t reserved1;
	int16_t gcaful;
	int16_t gcafll;
	uint8_t actvgcafvalid;        /* LCMASK */
	uint8_t noisethr;
	uint8_t reserved2;
	uint8_t freqhopscale;
	uint8_t freq[5u];
	uint8_t idlegcafvalid;        /* LCMASK */
} __packed procg_noisesuppression_t22_config_t;

typedef struct {
	uint8_t nCTRL;
	uint8_t nNUMGEST;
	uint8_t nGESTEN[2];
	uint8_t nPROCESS;
	uint8_t nTAPTO;
	uint8_t nFLICKTO;
	uint8_t nDRAGTO;
	uint8_t nSPRESSTO;
	uint8_t nLPRESSTO;
	uint8_t nREPPRESSTO;
	uint8_t nFLICKTHR[2];
	uint8_t nDRAGTHR[2];
	uint8_t nTAPTHR[2];
	uint8_t nTHROWTHR[2];
} __packed proci_onetouchgestureprocessor_t24;

typedef struct {
	uint8_t nCTRL;
	uint8_t nCMD;
	uint16_t nUPSIGLIM[3];
	uint16_t nLOSIGLIM[3];
	uint16_t nPINDWELLUS;
} __packed spt_selftest_t25_config_t;

typedef struct {
	uint8_t nCTRL;
	uint8_t nNUMGEST;
	uint8_t nRESERVED;
	uint8_t nGESTEN;
	uint8_t nROTATETHR;
	uint16_t nZOOMTHR;
} __packed proci_twotouchgestureprocessor_t27_config_t;

typedef struct {
	uint8_t ctrl;          /*  Ctrl field reserved for future expansion */
	uint8_t cmd;           /*  Cmd field for sending CTE commands */
	uint8_t mode;          /*  LCMASK CTE mode configuration field */
	/*  LCMASK The global gcaf number of averages when idle */
	uint8_t idlegcafdepth;
	/*  LCMASK The global gcaf number of averages when active */
	uint8_t actvgcafdepth;
	int8_t  voltage;
} __packed spt_cteconfig_t28_config_t;

typedef struct {
	uint8_t nMODE;
	uint8_t nPAGE;
	uint8_t nDATA[128];
} __packed debug_diagnostic_t37_config_t;

typedef struct {
	uint8_t nDATA[64];
} __packed spt_userdata_t38_config_t;

typedef struct {
	uint8_t nCTRL;
	uint8_t nXLOGRIP;
	uint8_t nXHIGRIP;
	uint8_t nYLOGRIP;
	uint8_t nYHIGRIP;
} __packed proci_gripsuppression_t40_config_t;

typedef struct {
	uint8_t nCTRL;
	uint8_t nAPPRTHR;
	uint8_t nMAXAPPRAREA;
	uint8_t nMAXTCHAREA;
	uint8_t nSUPSTRENGTH;
	uint8_t nSUPEXTTO;
	uint8_t nMAXNUMTCHS;
	uint8_t nSHAPESTRENGTH;
	uint8_t nSUPDIST;
	uint8_t nDISTHYST;
} __packed proci_touchsuppression_t42_config_t;


typedef struct {
	uint8_t nCTRL;
	uint8_t nHIDIDLERATE;
	uint16_t nXLENGTH;
	uint16_t nYLENGTH;
	uint8_t nRWKRATE;
	uint8_t nHEIGHTSCALE;
	uint8_t nHEIGHTOFFSET;
	uint8_t nWIDTHSCALE;
	uint8_t nWIDTHOFFSET;
	uint8_t nRESERVED;
} __packed spt_digitizer_t43_t;


typedef struct {
	uint8_t nCTRL;
	uint8_t nMODE;
	uint8_t nIDLESYNCSPERX;
	uint8_t nACTVSYNCSPERX;
	uint8_t nADCSPERSYNC;
	uint8_t nPULSESPERADC;
	uint8_t nXSLEW;
	uint16_t nSYNCDELAY;
	uint8_t nXVOLTAGE;
	uint8_t nADCCTRL;
} __packed spt_cteconfig_t46_config_t;

typedef struct {
	uint8_t nCTRL;
	uint8_t nCONTMIN;
	uint8_t nCONTMAX;
	uint8_t nSTABILITY;
	uint8_t nMAXTCHAREA;
	uint8_t nAMPLTHR;
	uint8_t nSTYSHAPE;
	uint8_t nHOVERSUP;
	uint8_t nCONFTHR;
	uint8_t nSYNCSPERX;
	uint8_t nXPOSADJ;
	uint8_t nYPOSADJ;
	uint8_t nCFG;
	uint8_t nRESERVED[7];
	uint8_t nSUPSTYTO;
	uint8_t nMAXNUMSTY;
} __packed proci_stylus_t47_config_t;

typedef struct {
	/*  Reserved RPTAPX RPTFREQ RPTEN ENABLE             */
	uint8_t ctrl;
	/*  Reserved GCMODE                                  */
	uint8_t cfg;
	/*  INCRST INCBIAS Reserved FIXFREQ MFEN NLEN        */
	uint8_t calcfg;
	/*  Base sampling frequency                          */
	uint8_t basefreq;
	/*  Frequency Hopping frequency 0                    */
	uint8_t freq_0;
	/*  Frequency Hopping frequency 1                    */
	uint8_t freq_1;
	/*  Frequency Hopping frequency 2                    */
	uint8_t freq_2;
	/*  Frequency Hopping frequency 3                    */
	uint8_t freq_3;
	/*  Median Filter frequency for second filter frame  */
	uint8_t mffreq_2;
	/*  Median Filter frequency for third filter frame   */
	uint8_t mffreq_3;
	/*  GAIN Reserved                                    */
	uint8_t nlgain;
	/*  Noise line threshold                             */
	uint8_t nlthr;
	/*  Grass cut limit                                  */
	uint8_t gclimit;
	/*  Grass cut valid ADCs                             */
	uint8_t gcactvinvldadcs;
	/*  Grass cut valid threshold                        */
	uint8_t gcidleinvldadcs;
	/*  Grass-cutting source threshold                   */
	uint16_t gcinvalidthr;
	/*  Max ADCs per X line                              */
	uint8_t gcmaxadcsperx;
	uint8_t gclimitmin;
	uint8_t gclimitmax;
	uint16_t gccountmintgt;
	uint8_t mfinvlddiffthr;
	uint16_t mfincadcspxthr;
	uint16_t mferrorthr;
	uint8_t selfreqmax;
	uint8_t reserved9;
	uint8_t reserved10;
	uint8_t reserved11;
	uint8_t reserved12;
	uint8_t reserved13;
	uint8_t reserved14;
	uint8_t blen ;
	uint8_t tchthr ;
	uint8_t tchdi ;
	uint8_t movhysti ;
	uint8_t movhystn ;
	uint8_t movfilter ;
	uint8_t numtouch ;
	uint8_t mrghyst ;
	uint8_t mrgthr ;
	uint8_t xloclip ;
	uint8_t xhiclip ;
	uint8_t yloclip ;
	uint8_t yhiclip ;
	uint8_t xedgectrl ;
	uint8_t xedgedist ;
	uint8_t yedgectrl ;
	uint8_t yedgedist ;
	uint8_t jumplimit ;
	uint8_t tchhyst ;
	uint8_t nexttchdi ;
} __packed procg_noisesuppression_t48_config_t;

typedef struct {
	/** Prox Configuration **/
	/*  ACENABLE LCENABLE Main configuration field           */
	uint8_t ctrl;

	/** Physical Configuration **/
	/*  ACMASK LCMASK Object x start position on matrix  */
	uint8_t xorigin;
	/*  ACMASK LCMASK Object y start position on matrix  */
	uint8_t yorigin;

	uint8_t reserved0;
	uint8_t reserved1;

	uint8_t askcfg;

	uint8_t reserved2;

	/*  Fixed detection threshold   */
	uint16_t fxddthr;

	/*  Fixed detection integration  */
	uint8_t fxddi;
	/*  Acquisition cycles to be averaged */
	uint8_t average;
	/*  Movement nulling rate */
	uint16_t mvnullrate;
	/*  Movement detection threshold */
	uint16_t mvdthr;
} __packed touch_proximity_t52_config_t;

typedef struct {
	uint8_t nCTRL;
	uint8_t nTARGETTHR;
	uint8_t nTHRADJLIM;
	uint8_t nRESETSTEPTIME;
	uint8_t nFORCECHGDIST;
	uint8_t nFORCECHGTIME;
	uint8_t nLOWESTTHR;
} __packed proci_adaptivethreshold_t55_config_t;

typedef struct {
	uint8_t nCTRL ;
	uint8_t nCOMMAND ;
	uint8_t nOPTINT ;
	uint8_t nINTTIME;
	uint8_t nINTDELAY[32];
	uint8_t nMULTICUTGC ;
	uint8_t nGCLIMIT ;
	uint8_t nNCNCL ;
	uint8_t nTOUCHBIAS ;
	uint8_t nBASESCALE ;
	uint8_t nSHIFTLIMIT ;
	uint16_t nYLONOISEMUL ;
	uint16_t nYLONOISEDIV ;
	uint16_t nYHINOISEMUL ;
	uint16_t nYHINOISEDIV ;
	uint8_t nNCNCLMANIDX ;
} __packed proci_shieldless_t56_t;

typedef struct {
	uint8_t nCTRL;
	uint8_t nAREATHR;
	uint8_t nAREAHYST;
} __packed proc_extratouchscreendata_t57_t;

typedef struct {
	uint8_t nCTRL;
	uint8_t nCMD;
	uint8_t nMODE;
	uint8_t nPERIOD;
} __packed spt_timer_t61_t;

typedef struct {
 	uint8_t  nCTRL ;
 	uint8_t  nCALCFG1 ;
 	uint8_t  nCALCFG2 ;
 	uint8_t  nCALCFG3 ;
 	uint8_t  nCFG1 ;
 	uint8_t  nRESERVED1 ;
 	uint8_t  nRESERVED2 ;
 	uint8_t  nBASEFREQ ;
 	uint8_t  nMAXSELFREQ ;
 	uint8_t  nFREQ[5] ;
 	uint8_t  nHOPCNT ;
 	uint8_t  nALTMAXSELFREQ ;
 	uint8_t  nHOPCNTPER ;
 	uint8_t  nHOPEVALTO ;
 	uint8_t  nHOPST ;
 	uint8_t  nNLGAIN ;
 	uint8_t  nMINNLTHR ;
 	uint8_t  nINCNLTHR ;
 	uint8_t  nADCSPERXTHR ;
 	uint8_t  nNLTHRMARGIN ;
 	uint8_t  nMAXADCSPERX ;
 	uint8_t  nACTVADCSVLDNOD ;
 	uint8_t  nIDLEADCSVLDNOD ;
 	uint8_t  nMINGCLIMIT ;
 	uint8_t  nMAXGCLIMIT ;
 	uint8_t  nRESERVED[5] ;

	uint8_t  nBLEN_0 ;
 	uint8_t  nTCHTHR_0 ;
 	uint8_t  nTCHDI_0 ;
 	uint8_t  nMOVHYSTI_0 ;
 	uint8_t  nMOVHYSTN_0 ;
 	uint8_t  nMOVFILTER_0 ;
 	uint8_t  nNUMTOUCH_0 ;
 	uint8_t  nMRGHYST_0 ;
 	uint8_t  nMRGTHR_0 ;
 	uint8_t  nXLOCLIP_0 ;
 	uint8_t  nXHICLIP_0 ;
 	uint8_t  nYLOCLIP_0 ;
 	uint8_t  nYHICLIP_0 ;
 	uint8_t  nXEDGECTRL_0 ;
 	uint8_t  nXEDGEDIST_0 ;
 	uint8_t  nYEDGECTRL_0 ;
 	uint8_t  nYEDGEDIST_0 ;
 	uint8_t  nJUMPLIMIT_0 ;
 	uint8_t  nTCHHYST_0 ;
 	uint8_t  nNEXTTCHDI_0 ;

	uint8_t  nBLEN_1 ;
 	uint8_t  nTCHTHR_1 ;
 	uint8_t  nTCHDI_1 ;
 	uint8_t  nMOVHYSTI_1 ;
 	uint8_t  nMOVHYSTN_1 ;
 	uint8_t  nMOVFILTER_1 ;
 	uint8_t  nNUMTOUCH_1 ;
 	uint8_t  nMRGHYST_1 ;
 	uint8_t  nMRGTHR_1 ;
 	uint8_t  nXLOCLIP_1 ;
 	uint8_t  nXHICLIP_1 ;
 	uint8_t  nYLOCLIP_1 ;
 	uint8_t  nYHICLIP_1 ;
 	uint8_t  nXEDGECTRL_1 ;
 	uint8_t  nXEDGEDIST_1 ;
 	uint8_t  nYEDGECTRL_1 ;
 	uint8_t  nYEDGEDIST_1 ;
 	uint8_t  nJUMPLIMIT_1 ;
 	uint8_t  nTCHHYST_1 ;
 	uint8_t  nNEXTTCHDI_1 ;
} __packed proci_noisesupperssion_t62_t;

typedef struct {
	uint8_t nCTRL  ;
	uint8_t nMAXTCHAREA  ;
	uint8_t nSIGPWR  ;
	uint8_t nSIGRATIO  ;
	uint8_t nSIGCNTMAX  ;
	uint8_t nXADJUST  ;
	uint8_t nYADJUST  ;
	uint8_t nCOMMSFILTER  ;
	uint8_t nDETADCSPERX  ;
	uint8_t nSUPDIST  ;
	uint8_t nSUPDISTHYST  ;
	uint8_t nSUPTO  ;
} __packed proci_activestylues_t63_t;

	
typedef struct {
	uint8_t nCTRL;
	uint8_t nGRADTHR;
	uint16_t nYLONOISEMUL;
	uint16_t nYLONOISEDIV;
	uint16_t nYHINOISEMUL;
	uint16_t nYHINOISEDIV;
	uint8_t nLPFILTCOEF;
	uint16_t nFORCESCALE;
	uint8_t nFORCETHR;
	uint8_t nFORCETHRHYST;
	uint8_t nFORCEDI;
	uint8_t nFORCEHYST;
} __packed proci_lensbending_t65_t;

typedef struct {
	uint8_t nCTRL;
	uint8_t nFCALFAILTHR;
	uint8_t nFCALDRIFTCNT;
	uint8_t nFCALDRIFTCOEF;
	uint8_t nFCALDRIFTLIM;
} __packed spt_goldenreferences_t66_t;

typedef struct {
	uint8_t nCTRL;
	uint8_t nRESERVED[2];
	uint8_t nDATATYPE;
	uint8_t nLENGTH;
	uint8_t nDATA[64]; // temp
	uint8_t nCMD;
	uint8_t nRESERVED1[2];
} __packed spt_serialdatacommand_t68_t;

typedef struct {
	uint16_t upsiglim;              /* LCMASK */
	uint16_t losiglim;              /* LCMASK */
} siglim_t;

/*! = Config Structure = */

/* \brief to save the registers which is for calibration/plam-recovery
 *	in runmode */
struct mxt_runmode_registers_t {
	/* T8 */
	uint8_t t8_atchcalst;
	uint8_t t8_atchcalsthr;
	uint8_t t8_atchfrccalthr;
	uint8_t t8_atchfrccalratio;

	/* T9 */
	/* uint8_t t9_numtouch; */
	/* uint8_t t9_tchthr; */

	/* T42 */
	/* uint8_t t42_maxnumtchs; */
};
#endif