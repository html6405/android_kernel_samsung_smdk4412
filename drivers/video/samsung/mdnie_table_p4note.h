#ifndef __MDNIE_TABLE_H__
#define __MDNIE_TABLE_H__

#include "mdnie.h"


static struct mdnie_backlight_value backlight_table[3] = {
	{
		.max = 1216,	/* 76% */
		.mid = 679,	/* 39% */
		.low = 16,	/* 1% */
		.dim = 16,	/* 1% */
	}, {
		.max = 1441,
		.mid = 784,
		.low = 16,
		.dim = 16,
	}, {
		.max = 1216,	/* 76% */
		.mid = 679,	/* 39% */
		.low = 16,	/* 1% */
		.dim = 16,	/* 1% */
	}
};

static const unsigned char power_lut[LUT_LEVEL_MAX][LUT_MAX][9] = {
	/* Indoor power look up table */
	{
		{0x42, 0x3d, 0x3E, 0x48, 0x42, 0x3F, 0x3A, 0x37, 0x3F},
		{0x38, 0x3d, 0x34, 0x48, 0x38, 0x35, 0x30, 0x2d, 0x35}
	},
	/* Outdoor power look up table for outdoor 1 (1k~15k) */
	{
		{0x42, 0x47, 0x3E, 0x52, 0x42, 0x3F, 0x3A, 0x37, 0x3F},
		{0x38, 0x3d, 0x34, 0x48, 0x38, 0x35, 0x30, 0x2d, 0x35}
	},
	/* Outdoor power look up table (15k ~) */
	{
		{100, 100, 100, 100, 100, 100, 100, 100, 100},
		{100, 100, 100, 100, 100, 100, 100, 100, 100}
	}
};

static unsigned short tune_camera[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00ac,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0092, 0x0030,	/*DE pe*/
	0x0093, 0x0030,	/*DE pf*/
	0x0094, 0x0030,	/*DE pb*/
	0x0095, 0x0030,	/*DE ne*/
	0x0096, 0x0030,	/*DE nf*/
	0x0097, 0x0030,	/*DE nb*/
	0x0098, 0x1000,	/*DE max ratio*/
	0x0099, 0x0100,	/*DE min ratio*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0890,	/*CC lut r	16 144*/
	0x0022, 0x18a0,	/*CC lut r	32 160*/
	0x0023, 0x30b0,	/*CC lut r	48 176*/
	0x0024, 0x40c0,	/*CC lut r	64 192*/
	0x0025, 0x50d0,	/*CC lut r	80 208*/
	0x0026, 0x60e0,	/*CC lut r	96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_dynamic_ui[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00a8,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1a04,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0894,	/*CC lut r	16 144*/
	0x0022, 0x18a6,	/*CC lut r	32 160*/
	0x0023, 0x28b8,	/*CC lut r	48 176*/
	0x0024, 0x3ac9,	/*CC lut r	64 192*/
	0x0025, 0x4cd9,	/*CC lut r	80 208*/
	0x0026, 0x5ee7,	/*CC lut r	96 224*/
	0x0027, 0x70f4,	/*CC lut r 112 240*/
	0x0028, 0x82ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_dynamic_video[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00ac,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0090, 0x0080,	/*DE egth*/
	0x0092, 0x0030,	/*DE pe*/
	0x0093, 0x0080,	/*DE pf*/
	0x0094, 0x0080,	/*DE pb*/
	0x0095, 0x0030,	/*DE ne*/
	0x0096, 0x0080,	/*DE nf*/
	0x0097, 0x0080,	/*DE nb*/
	0x0098, 0x1000,	/*DE max ratio*/
	0x0099, 0x0100,	/*DE min ratio*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1a04,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0894,	/*CC lut r	16 144*/
	0x0022, 0x18a6,	/*CC lut r	32 160*/
	0x0023, 0x28b8,	/*CC lut r	48 176*/
	0x0024, 0x3ac9,	/*CC lut r	64 192*/
	0x0025, 0x4cd9,	/*CC lut r	80 208*/
	0x0026, 0x5ee7,	/*CC lut r	96 224*/
	0x0027, 0x70f4,	/*CC lut r 112 240*/
	0x0028, 0x82ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_dynamic_gallery[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00ac,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0090, 0x0080,	/*DE egth*/
	0x0092, 0x0030,	/*DE pe*/
	0x0093, 0x0050,	/*DE pf*/
	0x0094, 0x0050,	/*DE pb*/
	0x0095, 0x0030,	/*DE ne*/
	0x0096, 0x0050,	/*DE nf*/
	0x0097, 0x0050,	/*DE nb*/
	0x0098, 0x1000,	/*DE max ratio*/
	0x0099, 0x0100,	/*DE min ratio*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1a04,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0894,	/*CC lut r	16 144*/
	0x0022, 0x18a6,	/*CC lut r	32 160*/
	0x0023, 0x28b8,	/*CC lut r	48 176*/
	0x0024, 0x3ac9,	/*CC lut r	64 192*/
	0x0025, 0x4cd9,	/*CC lut r	80 208*/
	0x0026, 0x5ee7,	/*CC lut r	96 224*/
	0x0027, 0x70f4,	/*CC lut r 112 240*/
	0x0028, 0x82ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_dynamic_vt[] = {
	0x0000, 0x0000,  /*BANK 0*/
	0x0008, 0x0088,	 /*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,  /*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,  /*CS hg ry*/
	0x00b1, 0x1010,  /*CS hg gc*/
	0x00b2, 0x1010,  /*CS hg bm*/
	0x00b3, 0x1c04,  /*CS weight grayTH*/
	0x0000, 0x0001,  /*BANK 1*/
	0x001f, 0x0080,  /*CC chsel strength*/
	0x0020, 0x0000,  /*CC lut r	 0*/
	0x0021, 0x0a88,  /*CC lut r	16 144*/
	0x0022, 0x1499,  /*CC lut r	32 160*/
	0x0023, 0x1eaa,  /*CC lut r	48 176*/
	0x0024, 0x30bb,  /*CC lut r	64 192*/
	0x0025, 0x42cc,  /*CC lut r	80 208*/
	0x0026, 0x54dd,  /*CC lut r	96 224*/
	0x0027, 0x66ee,  /*CC lut r 112 240*/
	0x0028, 0x77ff,  /*CC lut r 128 255*/
	0x00ff, 0x0000,  /*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_standard_ui[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00a8,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0890,	/*CC lut r	16 144*/
	0x0022, 0x18a0,	/*CC lut r	32 160*/
	0x0023, 0x30b0,	/*CC lut r	48 176*/
	0x0024, 0x40c0,	/*CC lut r	64 192*/
	0x0025, 0x50d0,	/*CC lut r	80 208*/
	0x0026, 0x60e0,	/*CC lut r	96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_standard_video[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00ac,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0090, 0x0080,	/*DE egth*/
	0x0092, 0x0030,	/*DE pe*/
	0x0093, 0x0060,	/*DE pf*/
	0x0094, 0x0060,	/*DE pb*/
	0x0095, 0x0030,	/*DE ne*/
	0x0096, 0x0060,	/*DE nf*/
	0x0097, 0x0060,	/*DE nb*/
	0x0098, 0x1000,	/*DE max ratio*/
	0x0099, 0x0100,	/*DE min ratio*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0890,	/*CC lut r	16 144*/
	0x0022, 0x18a0,	/*CC lut r	32 160*/
	0x0023, 0x30b0,	/*CC lut r	48 176*/
	0x0024, 0x40c0,	/*CC lut r	64 192*/
	0x0025, 0x50d0,	/*CC lut r	80 208*/
	0x0026, 0x60e0,	/*CC lut r	96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_standard_gallery[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00ac,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0090, 0x0080,	/*DE egth*/
	0x0092, 0x0030,	/*DE pe*/
	0x0093, 0x0030,	/*DE pf*/
	0x0094, 0x0030,	/*DE pb*/
	0x0095, 0x0030,	/*DE ne*/
	0x0096, 0x0030,	/*DE nf*/
	0x0097, 0x0030,	/*DE nb*/
	0x0098, 0x1000,	/*DE max ratio*/
	0x0099, 0x0100,	/*DE min ratio*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0890,	/*CC lut r	16 144*/
	0x0022, 0x18a0,	/*CC lut r	32 160*/
	0x0023, 0x30b0,	/*CC lut r	48 176*/
	0x0024, 0x40c0,	/*CC lut r	64 192*/
	0x0025, 0x50d0,	/*CC lut r	80 208*/
	0x0026, 0x60e0,	/*CC lut r	96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_standard_vt[] = {
	0x0000, 0x0000,  /*BANK 0*/
	0x0008, 0x0088,	 /*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,  /*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,  /*CS hg ry*/
	0x00b1, 0x1010,  /*CS hg gc*/
	0x00b2, 0x1010,  /*CS hg bm*/
	0x00b3, 0x1c04,  /*CS weight grayTH*/
	0x0000, 0x0001,  /*BANK 1*/
	0x001f, 0x0080,  /*CC chsel strength*/
	0x0020, 0x0000,  /*CC lut r	 0*/
	0x0021, 0x0a88,  /*CC lut r	16 144*/
	0x0022, 0x1499,  /*CC lut r	32 160*/
	0x0023, 0x1eaa,  /*CC lut r	48 176*/
	0x0024, 0x30bb,  /*CC lut r	64 192*/
	0x0025, 0x42cc,  /*CC lut r	80 208*/
	0x0026, 0x54dd,  /*CC lut r	96 224*/
	0x0027, 0x66ee,  /*CC lut r 112 240*/
	0x0028, 0x77ff,  /*CC lut r 128 255*/
	0x00ff, 0x0000,  /*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_movie_ui[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00a0,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00f6,	/*SCR KgWg*/
	0x00ec, 0x00e5,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0890,	/*CC lut r	16 144*/
	0x0022, 0x18a0,	/*CC lut r	32 160*/
	0x0023, 0x30b0,	/*CC lut r	48 176*/
	0x0024, 0x40c0,	/*CC lut r	64 192*/
	0x0025, 0x50d0,	/*CC lut r	80 208*/
	0x0026, 0x60e0,	/*CC lut r	96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_movie_video[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00a0,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0092, 0x0000,	/*DE pe*/
	0x0093, 0x0000,	/*DE pf*/
	0x0094, 0x0000,	/*DE pb*/
	0x0095, 0x0000,	/*DE ne*/
	0x0096, 0x0000,	/*DE nf*/
	0x0097, 0x0000,	/*DE nb*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1004,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00f6,	/*SCR KgWg*/
	0x00ec, 0x00e5,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0890,	/*CC lut r	16 144*/
	0x0022, 0x18a0,	/*CC lut r	32 160*/
	0x0023, 0x30b0,	/*CC lut r	48 176*/
	0x0024, 0x40c0,	/*CC lut r	64 192*/
	0x0025, 0x50d0,	/*CC lut r	80 208*/
	0x0026, 0x60e0,	/*CC lut r	96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_movie_gallery[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00a0,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00f6,	/*SCR KgWg*/
	0x00ec, 0x00e5,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0890,	/*CC lut r	16 144*/
	0x0022, 0x18a0,	/*CC lut r	32 160*/
	0x0023, 0x30b0,	/*CC lut r	48 176*/
	0x0024, 0x40c0,	/*CC lut r	64 192*/
	0x0025, 0x50d0,	/*CC lut r	80 208*/
	0x0026, 0x60e0,	/*CC lut r	96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_movie_vt[] = {
	0x0000, 0x0000,  /*BANK 0*/
	0x0008, 0x0088,	 /*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,  /*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,  /*CS hg ry*/
	0x00b1, 0x1010,  /*CS hg gc*/
	0x00b2, 0x1010,  /*CS hg bm*/
	0x00b3, 0x1c04,  /*CS weight grayTH*/
	0x0000, 0x0001,  /*BANK 1*/
	0x001f, 0x0080,  /*CC chsel strength*/
	0x0020, 0x0000,  /*CC lut r	 0*/
	0x0021, 0x0a88,  /*CC lut r	16 144*/
	0x0022, 0x1499,  /*CC lut r	32 160*/
	0x0023, 0x1eaa,  /*CC lut r	48 176*/
	0x0024, 0x30bb,  /*CC lut r	64 192*/
	0x0025, 0x42cc,  /*CC lut r	80 208*/
	0x0026, 0x54dd,  /*CC lut r	96 224*/
	0x0027, 0x66ee,  /*CC lut r 112 240*/
	0x0028, 0x77ff,  /*CC lut r 128 255*/
	0x00ff, 0x0000,  /*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_dynamic_ui_cabc[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x02a8,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1a04,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0c98,	/*CC lut r	16 144*/
	0x0022, 0x1caa,	/*CC lut r	32 160*/
	0x0023, 0x2cbc,	/*CC lut r	48 176*/
	0x0024, 0x3ecd,	/*CC lut r	64 192*/
	0x0025, 0x50dc,	/*CC lut r	80 208*/
	0x0026, 0x62e9,	/*CC lut r	96 224*/
	0x0027, 0x74f5,	/*CC lut r 112 240*/
	0x0028, 0x86ff,	/*CC lut r 128 255*/
	0x0075, 0x0000,	/*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,	/*dynamic lcd*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_dynamic_video_cabc[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x02ac,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0090, 0x0080,	/*DE egth*/
	0x0092, 0x0030,	/*DE pe*/
	0x0093, 0x0080,	/*DE pf*/
	0x0094, 0x0080,	/*DE pb*/
	0x0095, 0x0030,	/*DE ne*/
	0x0096, 0x0080,	/*DE nf*/
	0x0097, 0x0080,	/*DE nb*/
	0x0098, 0x1000,	/*DE max ratio*/
	0x0099, 0x0100,	/*DE min ratio*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1a04,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0c98,	/*CC lut r	16 144*/
	0x0022, 0x1caa,	/*CC lut r	32 160*/
	0x0023, 0x2cbc,	/*CC lut r	48 176*/
	0x0024, 0x3ecd,	/*CC lut r	64 192*/
	0x0025, 0x50dc,	/*CC lut r	80 208*/
	0x0026, 0x62e9,	/*CC lut r	96 224*/
	0x0027, 0x74f5,	/*CC lut r 112 240*/
	0x0028, 0x86ff,	/*CC lut r 128 255*/
	0x0075, 0x0000,	/*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,	/*dynamic lcd*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_dynamic_gallery_cabc[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x028c,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0090, 0x0080,	/*DE egth*/
	0x0092, 0x0030,	/*DE pe*/
	0x0093, 0x0050,	/*DE pf*/
	0x0094, 0x0050,	/*DE pb*/
	0x0095, 0x0030,	/*DE ne*/
	0x0096, 0x0050,	/*DE nf*/
	0x0097, 0x0050,	/*DE nb*/
	0x0098, 0x1000,	/*DE max ratio*/
	0x0099, 0x0100,	/*DE min ratio*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1a04,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0c98,	/*CC lut r	16 144*/
	0x0022, 0x1caa,	/*CC lut r	32 160*/
	0x0023, 0x2cbc,	/*CC lut r	48 176*/
	0x0024, 0x3ecd,	/*CC lut r	64 192*/
	0x0025, 0x50dc,	/*CC lut r	80 208*/
	0x0026, 0x62e9,	/*CC lut r	96 224*/
	0x0027, 0x74f5,	/*CC lut r 112 240*/
	0x0028, 0x86ff,	/*CC lut r 128 255*/
	0x0075, 0x0000,	/*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,	/*dynamic lcd*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_dynamic_vt_cabc[] = {
	0x0000, 0x0000,  /*BANK 0*/
	0x0008, 0x0288,  /*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,  /*FA cs1 de8 hdr2 fa1*/
	0x0090, 0x0080,  /*DE egth*/
	0x0092, 0x0030,  /*DE pe*/
	0x0093, 0x0080,  /*DE pf*/
	0x0094, 0x0080,  /*DE pb*/
	0x0095, 0x0030,  /*DE ne*/
	0x0096, 0x0080,  /*DE nf*/
	0x0097, 0x0080,  /*DE nb*/
	0x0098, 0x1000,  /*DE max ratio*/
	0x0099, 0x0100,  /*DE min ratio*/
	0x00b0, 0x1010,  /*CS hg ry*/
	0x00b1, 0x1010,  /*CS hg gc*/
	0x00b2, 0x1010,  /*CS hg bm*/
	0x00b3, 0x1c04,  /*CS weight grayTH*/
	0x0000, 0x0001,  /*BANK 1*/
	0x001f, 0x0080,  /*CC chsel strength*/
	0x0020, 0x0000,  /*CC lut r	 0*/
	0x0021, 0x1090,  /*CC lut r	16 144*/
	0x0022, 0x20a0,  /*CC lut r	32 160*/
	0x0023, 0x30b0,  /*CC lut r	48 176*/
	0x0024, 0x40c0,  /*CC lut r	64 192*/
	0x0025, 0x50d0,  /*CC lut r	80 208*/
	0x0026, 0x60e0,  /*CC lut r	96 224*/
	0x0027, 0x70f0,  /*CC lut r 112 240*/
	0x0028, 0x80ff,  /*CC lut r 128 255*/
	0x0075, 0x0000,  /*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,  /*dynamic lcd*/
	0x00ff, 0x0000,  /*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_standard_ui_cabc[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x02a8,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0c96,	/*CC lut r	16 144*/
	0x0022, 0x1ca7,	/*CC lut r	32 160*/
	0x0023, 0x34b8,	/*CC lut r	48 176*/
	0x0024, 0x44c9,	/*CC lut r	64 192*/
	0x0025, 0x54d9,	/*CC lut r	80 208*/
	0x0026, 0x64e7,	/*CC lut r	96 224*/
	0x0027, 0x74f4,	/*CC lut r 112 240*/
	0x0028, 0x85ff,	/*CC lut r 128 255*/
	0x0075, 0x0000,	/*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,	/*dynamic lcd*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_standard_video_cabc[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x02ac,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0090, 0x0080,	/*DE egth*/
	0x0092, 0x0030,	/*DE pe*/
	0x0093, 0x0060,	/*DE pf*/
	0x0094, 0x0060,	/*DE pb*/
	0x0095, 0x0030,	/*DE ne*/
	0x0096, 0x0060,	/*DE nf*/
	0x0097, 0x0060,	/*DE nb*/
	0x0098, 0x1000,	/*DE max ratio*/
	0x0099, 0x0100,	/*DE min ratio*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0c96,	/*CC lut r	16 144*/
	0x0022, 0x1ca7,	/*CC lut r	32 160*/
	0x0023, 0x34b8,	/*CC lut r	48 176*/
	0x0024, 0x44c9,	/*CC lut r	64 192*/
	0x0025, 0x54d9,	/*CC lut r	80 208*/
	0x0026, 0x64e7,	/*CC lut r	96 224*/
	0x0027, 0x74f4,	/*CC lut r 112 240*/
	0x0028, 0x85ff,	/*CC lut r 128 255*/
	0x0075, 0x0000,	/*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,	/*dynamic lcd*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_standard_gallery_cabc[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x02ac,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0090, 0x0080,	/*DE egth*/
	0x0092, 0x0030,	/*DE pe*/
	0x0093, 0x0030,	/*DE pf*/
	0x0094, 0x0030,	/*DE pb*/
	0x0095, 0x0030,	/*DE ne*/
	0x0096, 0x0030,	/*DE nf*/
	0x0097, 0x0030,	/*DE nb*/
	0x0098, 0x1000,	/*DE max ratio*/
	0x0099, 0x0100,	/*DE min ratio*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0c96,	/*CC lut r	16 144*/
	0x0022, 0x1ca7,	/*CC lut r	32 160*/
	0x0023, 0x34b8,	/*CC lut r	48 176*/
	0x0024, 0x44c9,	/*CC lut r	64 192*/
	0x0025, 0x54d9,	/*CC lut r	80 208*/
	0x0026, 0x64e7,	/*CC lut r	96 224*/
	0x0027, 0x74f4,	/*CC lut r 112 240*/
	0x0028, 0x85ff,	/*CC lut r 128 255*/
	0x0075, 0x0000,	/*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,	/*dynamic lcd*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_standard_vt_cabc[] = {
	0x0000, 0x0000,  /*BANK 0*/
	0x0008, 0x0288,  /*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,  /*FA cs1 de8 hdr2 fa1*/
	0x0090, 0x0080,  /*DE egth*/
	0x0092, 0x0030,  /*DE pe*/
	0x0093, 0x0080,  /*DE pf*/
	0x0094, 0x0080,  /*DE pb*/
	0x0095, 0x0030,  /*DE ne*/
	0x0096, 0x0080,  /*DE nf*/
	0x0097, 0x0080,  /*DE nb*/
	0x0098, 0x1000,  /*DE max ratio*/
	0x0099, 0x0100,  /*DE min ratio*/
	0x00b0, 0x1010,  /*CS hg ry*/
	0x00b1, 0x1010,  /*CS hg gc*/
	0x00b2, 0x1010,  /*CS hg bm*/
	0x00b3, 0x1c04,  /*CS weight grayTH*/
	0x0000, 0x0001,  /*BANK 1*/
	0x001f, 0x0080,  /*CC chsel strength*/
	0x0020, 0x0000,  /*CC lut r	 0*/
	0x0021, 0x1090,  /*CC lut r	16 144*/
	0x0022, 0x20a0,  /*CC lut r	32 160*/
	0x0023, 0x30b0,  /*CC lut r	48 176*/
	0x0024, 0x40c0,  /*CC lut r	64 192*/
	0x0025, 0x50d0,  /*CC lut r	80 208*/
	0x0026, 0x60e0,  /*CC lut r	96 224*/
	0x0027, 0x70f0,  /*CC lut r 112 240*/
	0x0028, 0x80ff,  /*CC lut r 128 255*/
	0x0075, 0x0000,  /*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,  /*dynamic lcd*/
	0x00ff, 0x0000,  /*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_movie_ui_cabc[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x02a0,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00f6,	/*SCR KgWg*/
	0x00ec, 0x00e5,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0c96,	/*CC lut r	16 144*/
	0x0022, 0x1ca7,	/*CC lut r	32 160*/
	0x0023, 0x34b8,	/*CC lut r	48 176*/
	0x0024, 0x44c9,	/*CC lut r	64 192*/
	0x0025, 0x54d9,	/*CC lut r	80 208*/
	0x0026, 0x64e7,	/*CC lut r	96 224*/
	0x0027, 0x74f4,	/*CC lut r 112 240*/
	0x0028, 0x85ff,	/*CC lut r 128 255*/
	0x0075, 0x0000,	/*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,	/*dynamic lcd*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_movie_video_cabc[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x02a0,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0092, 0x0000,	/*DE pe*/
	0x0093, 0x0000,	/*DE pf*/
	0x0094, 0x0000,	/*DE pb*/
	0x0095, 0x0000,	/*DE ne*/
	0x0096, 0x0000,	/*DE nf*/
	0x0097, 0x0000,	/*DE nb*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1004,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00f6,	/*SCR KgWg*/
	0x00ec, 0x00e5,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0c96,	/*CC lut r	16 144*/
	0x0022, 0x1ca7,	/*CC lut r	32 160*/
	0x0023, 0x34b8,	/*CC lut r	48 176*/
	0x0024, 0x44c9,	/*CC lut r	64 192*/
	0x0025, 0x54d9,	/*CC lut r	80 208*/
	0x0026, 0x64e7,	/*CC lut r	96 224*/
	0x0027, 0x74f4,	/*CC lut r 112 240*/
	0x0028, 0x85ff,	/*CC lut r 128 255*/
	0x0075, 0x0000,	/*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,	/*dynamic lcd*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_movie_gallery_cabc[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x02a0,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00f6,	/*SCR KgWg*/
	0x00ec, 0x00e5,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0c96,	/*CC lut r	16 144*/
	0x0022, 0x1ca7,	/*CC lut r	32 160*/
	0x0023, 0x34b8,	/*CC lut r	48 176*/
	0x0024, 0x44c9,	/*CC lut r	64 192*/
	0x0025, 0x54d9,	/*CC lut r	80 208*/
	0x0026, 0x64e7,	/*CC lut r	96 224*/
	0x0027, 0x74f4,	/*CC lut r 112 240*/
	0x0028, 0x85ff,	/*CC lut r 128 255*/
	0x0075, 0x0000,	/*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,	/*dynamic lcd*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_movie_vt_cabc[] = {
	0x0000, 0x0000,  /*BANK 0*/
	0x0008, 0x0288,  /*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,  /*FA cs1 de8 hdr2 fa1*/
	0x0090, 0x0080,  /*DE egth*/
	0x0092, 0x0030,  /*DE pe*/
	0x0093, 0x0080,  /*DE pf*/
	0x0094, 0x0080,  /*DE pb*/
	0x0095, 0x0030,  /*DE ne*/
	0x0096, 0x0080,  /*DE nf*/
	0x0097, 0x0080,  /*DE nb*/
	0x0098, 0x1000,  /*DE max ratio*/
	0x0099, 0x0100,  /*DE min ratio*/
	0x00b0, 0x1010,  /*CS hg ry*/
	0x00b1, 0x1010,  /*CS hg gc*/
	0x00b2, 0x1010,  /*CS hg bm*/
	0x00b3, 0x1c04,  /*CS weight grayTH*/
	0x0000, 0x0001,  /*BANK 1*/
	0x001f, 0x0080,  /*CC chsel strength*/
	0x0020, 0x0000,  /*CC lut r	 0*/
	0x0021, 0x1090,  /*CC lut r	16 144*/
	0x0022, 0x20a0,  /*CC lut r	32 160*/
	0x0023, 0x30b0,  /*CC lut r	48 176*/
	0x0024, 0x40c0,  /*CC lut r	64 192*/
	0x0025, 0x50d0,  /*CC lut r	80 208*/
	0x0026, 0x60e0,  /*CC lut r	96 224*/
	0x0027, 0x70f0,  /*CC lut r 112 240*/
	0x0028, 0x80ff,  /*CC lut r 128 255*/
	0x0075, 0x0000,  /*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,  /*dynamic lcd*/
	0x00ff, 0x0000,  /*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_dynamic_browser[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00a8,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1a04,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0894,	/*CC lut r	16 144*/
	0x0022, 0x18a6,	/*CC lut r	32 160*/
	0x0023, 0x28b8,	/*CC lut r	48 176*/
	0x0024, 0x3ac9,	/*CC lut r	64 192*/
	0x0025, 0x4cd9,	/*CC lut r	80 208*/
	0x0026, 0x5ee7,	/*CC lut r	96 224*/
	0x0027, 0x70f4,	/*CC lut r 112 240*/
	0x0028, 0x82ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_dynamic_ebook[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00a8,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1a04,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0894,	/*CC lut r	16 144*/
	0x0022, 0x18a6,	/*CC lut r	32 160*/
	0x0023, 0x28b8,	/*CC lut r	48 176*/
	0x0024, 0x3ac9,	/*CC lut r	64 192*/
	0x0025, 0x4cd9,	/*CC lut r	80 208*/
	0x0026, 0x5ee7,	/*CC lut r	96 224*/
	0x0027, 0x70f4,	/*CC lut r 112 240*/
	0x0028, 0x82ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_standard_browser[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00a8,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0890,	/*CC lut r	16 144*/
	0x0022, 0x18a0,	/*CC lut r	32 160*/
	0x0023, 0x30b0,	/*CC lut r	48 176*/
	0x0024, 0x40c0,	/*CC lut r	64 192*/
	0x0025, 0x50d0,	/*CC lut r	80 208*/
	0x0026, 0x60e0,	/*CC lut r	96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_standard_ebook[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00a8,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0890,	/*CC lut r	16 144*/
	0x0022, 0x18a0,	/*CC lut r	32 160*/
	0x0023, 0x30b0,	/*CC lut r	48 176*/
	0x0024, 0x40c0,	/*CC lut r	64 192*/
	0x0025, 0x50d0,	/*CC lut r	80 208*/
	0x0026, 0x60e0,	/*CC lut r	96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_movie_browser[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00a0,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00f6,	/*SCR KgWg*/
	0x00ec, 0x00e5,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0890,	/*CC lut r	16 144*/
	0x0022, 0x18a0,	/*CC lut r	32 160*/
	0x0023, 0x30b0,	/*CC lut r	48 176*/
	0x0024, 0x40c0,	/*CC lut r	64 192*/
	0x0025, 0x50d0,	/*CC lut r	80 208*/
	0x0026, 0x60e0,	/*CC lut r	96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_movie_ebook[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00a0,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00f6,	/*SCR KgWg*/
	0x00ec, 0x00e5,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0890,	/*CC lut r	16 144*/
	0x0022, 0x18a0,	/*CC lut r	32 160*/
	0x0023, 0x30b0,	/*CC lut r	48 176*/
	0x0024, 0x40c0,	/*CC lut r	64 192*/
	0x0025, 0x50d0,	/*CC lut r	80 208*/
	0x0026, 0x60e0,	/*CC lut r	96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_auto_ui[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00a8,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0890,	/*CC lut r	16 144*/
	0x0022, 0x18a0,	/*CC lut r	32 160*/
	0x0023, 0x30b0,	/*CC lut r	48 176*/
	0x0024, 0x40c0,	/*CC lut r	64 192*/
	0x0025, 0x50d0,	/*CC lut r	80 208*/
	0x0026, 0x60e0,	/*CC lut r	96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_auto_video[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00ac,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0090, 0x0080,	/*DE egth*/
	0x0092, 0x0030,	/*DE pe*/
	0x0093, 0x0060,	/*DE pf*/
	0x0094, 0x0060,	/*DE pb*/
	0x0095, 0x0030,	/*DE ne*/
	0x0096, 0x0060,	/*DE nf*/
	0x0097, 0x0060,	/*DE nb*/
	0x0098, 0x1000,	/*DE max ratio*/
	0x0099, 0x0100,	/*DE min ratio*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0890,	/*CC lut r	16 144*/
	0x0022, 0x18a0,	/*CC lut r	32 160*/
	0x0023, 0x30b0,	/*CC lut r	48 176*/
	0x0024, 0x40c0,	/*CC lut r	64 192*/
	0x0025, 0x50d0,	/*CC lut r	80 208*/
	0x0026, 0x60e0,	/*CC lut r	96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_auto_gallery[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00ac,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0090, 0x0080,	/*DE egth*/
	0x0092, 0x0030,	/*DE pe*/
	0x0093, 0x0030,	/*DE pf*/
	0x0094, 0x0030,	/*DE pb*/
	0x0095, 0x0030,	/*DE ne*/
	0x0096, 0x0030,	/*DE nf*/
	0x0097, 0x0030,	/*DE nb*/
	0x0098, 0x1000,	/*DE max ratio*/
	0x0099, 0x0100,	/*DE min ratio*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0890,	/*CC lut r	16 144*/
	0x0022, 0x18a0,	/*CC lut r	32 160*/
	0x0023, 0x30b0,	/*CC lut r	48 176*/
	0x0024, 0x40c0,	/*CC lut r	64 192*/
	0x0025, 0x50d0,	/*CC lut r	80 208*/
	0x0026, 0x60e0,	/*CC lut r	96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_auto_vt[] = {
	0x0000, 0x0000,  /*BANK 0*/
	0x0008, 0x0088,	 /*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,  /*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,  /*CS hg ry*/
	0x00b1, 0x1010,  /*CS hg gc*/
	0x00b2, 0x1010,  /*CS hg bm*/
	0x00b3, 0x1c04,  /*CS weight grayTH*/
	0x0000, 0x0001,  /*BANK 1*/
	0x001f, 0x0080,  /*CC chsel strength*/
	0x0020, 0x0000,  /*CC lut r	 0*/
	0x0021, 0x0a88,  /*CC lut r	16 144*/
	0x0022, 0x1499,  /*CC lut r	32 160*/
	0x0023, 0x1eaa,  /*CC lut r	48 176*/
	0x0024, 0x30bb,  /*CC lut r	64 192*/
	0x0025, 0x42cc,  /*CC lut r	80 208*/
	0x0026, 0x54dd,  /*CC lut r	96 224*/
	0x0027, 0x66ee,  /*CC lut r 112 240*/
	0x0028, 0x77ff,  /*CC lut r 128 255*/
	0x00ff, 0x0000,  /*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_auto_browser[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00a8,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0890,	/*CC lut r	16 144*/
	0x0022, 0x18a0,	/*CC lut r	32 160*/
	0x0023, 0x30b0,	/*CC lut r	48 176*/
	0x0024, 0x40c0,	/*CC lut r	64 192*/
	0x0025, 0x50d0,	/*CC lut r	80 208*/
	0x0026, 0x60e0,	/*CC lut r	96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_auto_ebook[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00a8,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0890,	/*CC lut r	16 144*/
	0x0022, 0x18a0,	/*CC lut r	32 160*/
	0x0023, 0x30b0,	/*CC lut r	48 176*/
	0x0024, 0x40c0,	/*CC lut r	64 192*/
	0x0025, 0x50d0,	/*CC lut r	80 208*/
	0x0026, 0x60e0,	/*CC lut r	96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_auto_camera[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x00ac,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0092, 0x0030,	/*DE pe*/
	0x0093, 0x0030,	/*DE pf*/
	0x0094, 0x0030,	/*DE pb*/
	0x0095, 0x0030,	/*DE ne*/
	0x0096, 0x0030,	/*DE nf*/
	0x0097, 0x0030,	/*DE nb*/
	0x0098, 0x1000,	/*DE max ratio*/
	0x0099, 0x0100,	/*DE min ratio*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0890,	/*CC lut r	16 144*/
	0x0022, 0x18a0,	/*CC lut r	32 160*/
	0x0023, 0x30b0,	/*CC lut r	48 176*/
	0x0024, 0x40c0,	/*CC lut r	64 192*/
	0x0025, 0x50d0,	/*CC lut r	80 208*/
	0x0026, 0x60e0,	/*CC lut r	96 224*/
	0x0027, 0x70f0,	/*CC lut r 112 240*/
	0x0028, 0x80ff,	/*CC lut r 128 255*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_dynamic_browser_cabc[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x0288,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1a04,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0c98,	/*CC lut r	16 144*/
	0x0022, 0x1caa,	/*CC lut r	32 160*/
	0x0023, 0x2cbc,	/*CC lut r	48 176*/
	0x0024, 0x3ecd,	/*CC lut r	64 192*/
	0x0025, 0x50dc,	/*CC lut r	80 208*/
	0x0026, 0x62e9,	/*CC lut r	96 224*/
	0x0027, 0x74f5,	/*CC lut r 112 240*/
	0x0028, 0x86ff,	/*CC lut r 128 255*/
	0x0075, 0x0000,	/*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,	/*dynamic lcd*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_dynamic_ebook_cabc[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x0288,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1a04,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0c98,	/*CC lut r	16 144*/
	0x0022, 0x1caa,	/*CC lut r	32 160*/
	0x0023, 0x2cbc,	/*CC lut r	48 176*/
	0x0024, 0x3ecd,	/*CC lut r	64 192*/
	0x0025, 0x50dc,	/*CC lut r	80 208*/
	0x0026, 0x62e9,	/*CC lut r	96 224*/
	0x0027, 0x74f5,	/*CC lut r 112 240*/
	0x0028, 0x86ff,	/*CC lut r 128 255*/
	0x0075, 0x0000,	/*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,	/*dynamic lcd*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_standard_browser_cabc[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x02a8,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0c96,	/*CC lut r	16 144*/
	0x0022, 0x1ca7,	/*CC lut r	32 160*/
	0x0023, 0x34b8,	/*CC lut r	48 176*/
	0x0024, 0x44c9,	/*CC lut r	64 192*/
	0x0025, 0x54d9,	/*CC lut r	80 208*/
	0x0026, 0x64e7,	/*CC lut r	96 224*/
	0x0027, 0x74f4,	/*CC lut r 112 240*/
	0x0028, 0x85ff,	/*CC lut r 128 255*/
	0x0075, 0x0000,	/*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,	/*dynamic lcd*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_standard_ebook_cabc[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x02a8,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0c96,	/*CC lut r	16 144*/
	0x0022, 0x1ca7,	/*CC lut r	32 160*/
	0x0023, 0x34b8,	/*CC lut r	48 176*/
	0x0024, 0x44c9,	/*CC lut r	64 192*/
	0x0025, 0x54d9,	/*CC lut r	80 208*/
	0x0026, 0x64e7,	/*CC lut r	96 224*/
	0x0027, 0x74f4,	/*CC lut r 112 240*/
	0x0028, 0x85ff,	/*CC lut r 128 255*/
	0x0075, 0x0000,	/*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,	/*dynamic lcd*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_movie_browser_cabc[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x02a0,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00f6,	/*SCR KgWg*/
	0x00ec, 0x00e5,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0c96,	/*CC lut r	16 144*/
	0x0022, 0x1ca7,	/*CC lut r	32 160*/
	0x0023, 0x34b8,	/*CC lut r	48 176*/
	0x0024, 0x44c9,	/*CC lut r	64 192*/
	0x0025, 0x54d9,	/*CC lut r	80 208*/
	0x0026, 0x64e7,	/*CC lut r	96 224*/
	0x0027, 0x74f4,	/*CC lut r 112 240*/
	0x0028, 0x85ff,	/*CC lut r 128 255*/
	0x0075, 0x0000,	/*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,	/*dynamic lcd*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_movie_ebook_cabc[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x02a0,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00f6,	/*SCR KgWg*/
	0x00ec, 0x00e5,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0c96,	/*CC lut r	16 144*/
	0x0022, 0x1ca7,	/*CC lut r	32 160*/
	0x0023, 0x34b8,	/*CC lut r	48 176*/
	0x0024, 0x44c9,	/*CC lut r	64 192*/
	0x0025, 0x54d9,	/*CC lut r	80 208*/
	0x0026, 0x64e7,	/*CC lut r	96 224*/
	0x0027, 0x74f4,	/*CC lut r 112 240*/
	0x0028, 0x85ff,	/*CC lut r 128 255*/
	0x0075, 0x0000,	/*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,	/*dynamic lcd*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_auto_ui_cabc[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x02a8,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0c96,	/*CC lut r	16 144*/
	0x0022, 0x1ca7,	/*CC lut r	32 160*/
	0x0023, 0x34b8,	/*CC lut r	48 176*/
	0x0024, 0x44c9,	/*CC lut r	64 192*/
	0x0025, 0x54d9,	/*CC lut r	80 208*/
	0x0026, 0x64e7,	/*CC lut r	96 224*/
	0x0027, 0x74f4,	/*CC lut r 112 240*/
	0x0028, 0x85ff,	/*CC lut r 128 255*/
	0x0075, 0x0000,	/*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,	/*dynamic lcd*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_auto_video_cabc[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x02ac,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0090, 0x0080,	/*DE egth*/
	0x0092, 0x0030,	/*DE pe*/
	0x0093, 0x0060,	/*DE pf*/
	0x0094, 0x0060,	/*DE pb*/
	0x0095, 0x0030,	/*DE ne*/
	0x0096, 0x0060,	/*DE nf*/
	0x0097, 0x0060,	/*DE nb*/
	0x0098, 0x1000,	/*DE max ratio*/
	0x0099, 0x0100,	/*DE min ratio*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0c96,	/*CC lut r	16 144*/
	0x0022, 0x1ca7,	/*CC lut r	32 160*/
	0x0023, 0x34b8,	/*CC lut r	48 176*/
	0x0024, 0x44c9,	/*CC lut r	64 192*/
	0x0025, 0x54d9,	/*CC lut r	80 208*/
	0x0026, 0x64e7,	/*CC lut r	96 224*/
	0x0027, 0x74f4,	/*CC lut r 112 240*/
	0x0028, 0x85ff,	/*CC lut r 128 255*/
	0x0075, 0x0000,	/*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,	/*dynamic lcd*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_auto_gallery_cabc[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x02ac,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x0090, 0x0080,	/*DE egth*/
	0x0092, 0x0030,	/*DE pe*/
	0x0093, 0x0030,	/*DE pf*/
	0x0094, 0x0030,	/*DE pb*/
	0x0095, 0x0030,	/*DE ne*/
	0x0096, 0x0030,	/*DE nf*/
	0x0097, 0x0030,	/*DE nb*/
	0x0098, 0x1000,	/*DE max ratio*/
	0x0099, 0x0100,	/*DE min ratio*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0c96,	/*CC lut r	16 144*/
	0x0022, 0x1ca7,	/*CC lut r	32 160*/
	0x0023, 0x34b8,	/*CC lut r	48 176*/
	0x0024, 0x44c9,	/*CC lut r	64 192*/
	0x0025, 0x54d9,	/*CC lut r	80 208*/
	0x0026, 0x64e7,	/*CC lut r	96 224*/
	0x0027, 0x74f4,	/*CC lut r 112 240*/
	0x0028, 0x85ff,	/*CC lut r 128 255*/
	0x0075, 0x0000,	/*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,	/*dynamic lcd*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_auto_vt_cabc[] = {
	0x0000, 0x0000,  /*BANK 0*/
	0x0008, 0x0288,  /*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,  /*FA cs1 de8 hdr2 fa1*/
	0x0090, 0x0080,  /*DE egth*/
	0x0092, 0x0030,  /*DE pe*/
	0x0093, 0x0080,  /*DE pf*/
	0x0094, 0x0080,  /*DE pb*/
	0x0095, 0x0030,  /*DE ne*/
	0x0096, 0x0080,  /*DE nf*/
	0x0097, 0x0080,  /*DE nb*/
	0x0098, 0x1000,  /*DE max ratio*/
	0x0099, 0x0100,  /*DE min ratio*/
	0x00b0, 0x1010,  /*CS hg ry*/
	0x00b1, 0x1010,  /*CS hg gc*/
	0x00b2, 0x1010,  /*CS hg bm*/
	0x00b3, 0x1c04,  /*CS weight grayTH*/
	0x0000, 0x0001,  /*BANK 1*/
	0x001f, 0x0080,  /*CC chsel strength*/
	0x0020, 0x0000,  /*CC lut r	 0*/
	0x0021, 0x1090,  /*CC lut r	16 144*/
	0x0022, 0x20a0,  /*CC lut r	32 160*/
	0x0023, 0x30b0,  /*CC lut r	48 176*/
	0x0024, 0x40c0,  /*CC lut r	64 192*/
	0x0025, 0x50d0,  /*CC lut r	80 208*/
	0x0026, 0x60e0,  /*CC lut r	96 224*/
	0x0027, 0x70f0,  /*CC lut r 112 240*/
	0x0028, 0x80ff,  /*CC lut r 128 255*/
	0x0075, 0x0000,  /*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,  /*dynamic lcd*/
	0x00ff, 0x0000,  /*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_auto_browser_cabc[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x02a8,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00fd,	/*SCR KgWg*/
	0x00ec, 0x00fc,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0c96,	/*CC lut r	16 144*/
	0x0022, 0x1ca7,	/*CC lut r	32 160*/
	0x0023, 0x34b8,	/*CC lut r	48 176*/
	0x0024, 0x44c9,	/*CC lut r	64 192*/
	0x0025, 0x54d9,	/*CC lut r	80 208*/
	0x0026, 0x64e7,	/*CC lut r	96 224*/
	0x0027, 0x74f4,	/*CC lut r 112 240*/
	0x0028, 0x85ff,	/*CC lut r 128 255*/
	0x0075, 0x0000,	/*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,	/*dynamic lcd*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

static unsigned short tune_auto_ebook_cabc[] = {
	0x0000, 0x0000,	/*BANK 0*/
	0x0008, 0x02a8,	/*Dither8 UC4 ABC2 CP1 | CC8 MCM4 SCR2 SCC1 | CS8 DE4 DNR2 HDR1*/
	0x0030, 0x0000,	/*FA cs1 de8 hdr2 fa1*/
	0x00b0, 0x1010,	/*CS hg ry*/
	0x00b1, 0x1010,	/*CS hg gc*/
	0x00b2, 0x1010,	/*CS hg bm*/
	0x00b3, 0x1804,	/*CS weight grayTH*/
	0x00e1, 0xff00,	/*SCR RrCr*/
	0x00e2, 0x00ff,	/*SCR RgCg*/
	0x00e3, 0x00ff,	/*SCR RbCb*/
	0x00e4, 0x00ff,	/*SCR GrMr*/
	0x00e5, 0xff00,	/*SCR GgMg*/
	0x00e6, 0x00ff,	/*SCR GbMb*/
	0x00e7, 0x00ff,	/*SCR BrYr*/
	0x00e8, 0x00ff,	/*SCR BgYg*/
	0x00e9, 0xff00,	/*SCR BbYb*/
	0x00ea, 0x00ff,	/*SCR KrWr*/
	0x00eb, 0x00f6,	/*SCR KgWg*/
	0x00ec, 0x00e5,	/*SCR KbWb*/
	0x0000, 0x0001,	/*BANK 1*/
	0x001f, 0x0080,	/*CC chsel strength*/
	0x0020, 0x0000,	/*CC lut r	 0*/
	0x0021, 0x0c96,	/*CC lut r	16 144*/
	0x0022, 0x1ca7,	/*CC lut r	32 160*/
	0x0023, 0x34b8,	/*CC lut r	48 176*/
	0x0024, 0x44c9,	/*CC lut r	64 192*/
	0x0025, 0x54d9,	/*CC lut r	80 208*/
	0x0026, 0x64e7,	/*CC lut r	96 224*/
	0x0027, 0x74f4,	/*CC lut r 112 240*/
	0x0028, 0x85ff,	/*CC lut r 128 255*/
	0x0075, 0x0000,	/*CABC dgain*/
	0x0076, 0x0000,
	0x0077, 0x0000,
	0x0078, 0x0000,
	0x007f, 0x0002,	/*dynamic lcd*/
	0x00ff, 0x0000,	/*Mask Release*/
	END_SEQ, 0x0000
};

struct mdnie_tuning_info tuning_table[CABC_MAX][MODE_MAX][OUTDOOR_MAX][SCENARIO_MAX] = {
	{
		{
			{
			    {"dynamic_ui",		tune_dynamic_ui},
			    {"dynamic_video",	tune_dynamic_video},
			    {"dynamic_video",	tune_dynamic_video},
			    {"dynamic_video",	tune_dynamic_video},
			    {"camera",		tune_camera},
			    {"dynamic_ui",		tune_dynamic_ui},
			    {"dynamic_gallery",	tune_dynamic_gallery},
			    {"dynamic_vt",		tune_dynamic_vt},
			    {"dynamic_browser",	tune_dynamic_browser},
			    {"dynamic_ebook",	tune_dynamic_ebook},
			    {"email",		tune_dynamic_ui}
		    },
		    {   //outdoor
			    {"dynamic_ui",		tune_dynamic_ui},
			    {"dynamic_video",	tune_dynamic_video},
			    {"dynamic_video",	tune_dynamic_video},
			    {"dynamic_video",	tune_dynamic_video},
			    {"camera",		tune_camera},
			    {"dynamic_ui",		tune_dynamic_ui},
			    {"dynamic_gallery",	tune_dynamic_gallery},
			    {"dynamic_vt",		tune_dynamic_vt},
			    {"dynamic_browser",	tune_dynamic_browser},
			    {"dynamic_ebook",	tune_dynamic_ebook},
			    {"email",		tune_dynamic_ui}
			 }
		},
		{
			{
			    {"standard_ui",		tune_standard_ui},
			    {"standard_video",	tune_standard_video},
			    {"standard_video",	tune_standard_video},
			    {"standard_video",	tune_standard_video},
			    {"camera",		tune_camera},
			    {"standard_ui",		tune_standard_ui},
			    {"standard_gallery",	tune_standard_gallery},
			    {"standard_vt",		tune_standard_vt},
			    {"standard_browser",	tune_standard_browser},
			    {"standard_ebook",	tune_standard_ebook},
			    {"email",		tune_standard_ui}
		    },
		    {   //outdoor
			    {"standard_ui",		tune_standard_ui},
			    {"standard_video",	tune_standard_video},
			    {"standard_video",	tune_standard_video},
			    {"standard_video",	tune_standard_video},
			    {"camera",		tune_camera},
			    {"standard_ui",		tune_standard_ui},
			    {"standard_gallery",	tune_standard_gallery},
			    {"standard_vt",		tune_standard_vt},
			    {"standard_browser",	tune_standard_browser},
		    	{"standard_ebook",	tune_standard_ebook},
			    {"email",		tune_standard_ui}
			}
		},
		{
		    {
			    {"movie_ui",		tune_movie_ui},
			    {"movie_video",		tune_movie_video},
			    {"movie_video",		tune_movie_video},
			    {"movie_video",		tune_movie_video},
			    {"camera",		tune_camera},
			    {"movie_ui",		tune_movie_ui},
			    {"movie_gallery",	tune_movie_gallery},
			    {"movie_vt",		tune_movie_vt},
			    {"movie_browser",	tune_movie_browser},
			    {"movie_ebook",		tune_movie_ebook},
			    {"email",		tune_movie_ui}
		    },
		    {   //outdoor
			    {"movie_ui",		tune_movie_ui},
			    {"movie_video",		tune_movie_video},
			    {"movie_video",		tune_movie_video},
			    {"movie_video",		tune_movie_video},
			    {"camera",		tune_camera},
			    {"movie_ui",		tune_movie_ui},
			    {"movie_gallery",	tune_movie_gallery},
			    {"movie_vt",		tune_movie_vt},
			    {"movie_browser",	tune_movie_browser},
			    {"movie_ebook",		tune_movie_ebook},
			    {"email",		tune_movie_ui}
			}
		},
		{
		    {
			    {"auto_ui",		tune_auto_ui},
			    {"auto_video",		tune_auto_video},
			    {"auto_video",		tune_auto_video},
			    {"auto_video",		tune_auto_video},
			    {"auto_camera",		tune_auto_camera},
			    {"auto_ui",		tune_auto_ui},
			    {"auto_gallery",	tune_auto_gallery},
			    {"auto_vt",		tune_auto_vt},
			    {"auto_browser",	tune_auto_browser},
			    {"auto_ebook",		tune_auto_ebook},
			    {"email",		tune_auto_ui}
		    },
		    {   //outdoor
			    {"auto_ui",		tune_auto_ui},
			    {"auto_video",		tune_auto_video},
			    {"auto_video",		tune_auto_video},
			    {"auto_video",		tune_auto_video},
			    {"auto_camera",		tune_auto_camera},
			    {"auto_ui",		tune_auto_ui},
			    {"auto_gallery",	tune_auto_gallery},
			    {"auto_vt",		tune_auto_vt},
			    {"auto_browser",	tune_auto_browser},
			    {"auto_ebook",		tune_auto_ebook},
			    {"email",		tune_auto_ui}
			}
		},
		{
		    {
			    {"dynamic_ui_cabc",		tune_dynamic_ui_cabc},
			    {"dynamic_video_cabc",		tune_dynamic_video_cabc},
                {"dynamic_video_cabc",		tune_dynamic_video_cabc},
                {"dynamic_video_cabc",		tune_dynamic_video_cabc},
			    {"camera",			tune_camera},
			    {"dynamic_ui_cabc",		tune_dynamic_ui_cabc},
			    {"dynamic_gallery_cabc",	tune_dynamic_gallery_cabc},
			    {"dynamic_vt_cabc",		tune_dynamic_vt_cabc},
			    {"dynamic_browser_cabc",	tune_dynamic_browser_cabc},
			    {"dynamic_ebook_cabc",		tune_dynamic_ebook_cabc},
			    {"email",			tune_dynamic_ui_cabc}
		    },
		    {   //outdoor
			    {"dynamic_ui_cabc",		tune_dynamic_ui_cabc},
			    {"dynamic_video_cabc",		tune_dynamic_video_cabc},
			    {"dynamic_video_cabc",		tune_dynamic_video_cabc},
			    {"dynamic_video_cabc",		tune_dynamic_video_cabc},
			    {"camera",			tune_camera},
			    {"dynamic_ui_cabc",		tune_dynamic_ui_cabc},
			    {"dynamic_gallery_cabc",	tune_dynamic_gallery_cabc},
			    {"dynamic_vt_cabc",		tune_dynamic_vt_cabc},
			    {"dynamic_browser_cabc",	tune_dynamic_browser_cabc},
			    {"dynamic_ebook_cabc",		tune_dynamic_ebook_cabc},
			    {"email",			tune_dynamic_ui_cabc}
			}
		},
		{
		    {
			    {"standard_ui_cabc",		tune_standard_ui_cabc},
			    {"standard_video_cabc",		tune_standard_video_cabc},
			    {"standard_video_cabc",		tune_standard_video_cabc},
			    {"standard_video_cabc",		tune_standard_video_cabc},
			    {"camera",			tune_camera},
			    {"standard_ui_cabc",		tune_standard_ui_cabc},
			    {"standard_gallery_cabc",	tune_standard_gallery_cabc},
			    {"standard_vt_cabc",		tune_standard_vt_cabc},
			    {"standard_browser_cabc",	tune_standard_browser_cabc},
			    {"standard_ebook_cabc",		tune_standard_ebook_cabc},
			    {"email",			tune_standard_ui_cabc}
		    },
		    {   //outdoor
			    {"standard_ui_cabc",		tune_standard_ui_cabc},
			    {"standard_video_cabc",		tune_standard_video_cabc},
			    {"standard_video_cabc",		tune_standard_video_cabc},
			    {"standard_video_cabc",		tune_standard_video_cabc},
			    {"camera",			tune_camera},
			    {"standard_ui_cabc",		tune_standard_ui_cabc},
			    {"standard_gallery_cabc",	tune_standard_gallery_cabc},
		    	{"standard_vt_cabc",		tune_standard_vt_cabc},
			    {"standard_browser_cabc",	tune_standard_browser_cabc},
			    {"standard_ebook_cabc",		tune_standard_ebook_cabc},
			    {"email",			tune_standard_ui_cabc}
			}
		},
		{
		    {
			    {"movie_ui_cabc",		tune_movie_ui_cabc},
			    {"movie_video_cabc",		tune_movie_video_cabc},
			    {"movie_video_cabc",		tune_movie_video_cabc},
			    {"movie_video_cabc",		tune_movie_video_cabc},
			    {"camera",			tune_camera},
		    	{"movie_ui_cabc",		tune_movie_ui_cabc},
			    {"movie_gallery_cabc",		tune_movie_gallery_cabc},
		    	{"movie_vt_cabc",		tune_movie_vt_cabc},
		    	{"movie_browser_cabc",		tune_movie_browser_cabc},
		    	{"movie_ebook_cabc",		tune_movie_ebook_cabc},
			    {"email",			tune_movie_ui_cabc}
		    },
		    {  //outdoor
			    {"movie_ui_cabc",		tune_movie_ui_cabc},
			    {"movie_video_cabc",		tune_movie_video_cabc},
			    {"movie_video_cabc",		tune_movie_video_cabc},
		    	{"movie_video_cabc",		tune_movie_video_cabc},
		    	{"camera",			tune_camera},
		    	{"movie_ui_cabc",		tune_movie_ui_cabc},
			    {"movie_gallery_cabc",		tune_movie_gallery_cabc},
		    	{"movie_vt_cabc",		tune_movie_vt_cabc},
		    	{"movie_browser_cabc",		tune_movie_browser_cabc},
			    {"movie_ebook_cabc",		tune_movie_ebook_cabc},
			    {"email",			tune_movie_ui_cabc}
			}
		},
		{
		    {
			    {"auto_ui_cabc",		tune_auto_ui_cabc},
			    {"auto_video_cabc",		tune_auto_video_cabc},
			    {"auto_video_cabc",		tune_auto_video_cabc},
			    {"auto_video_cabc",		tune_auto_video_cabc},
			    {"auto_camera",			tune_auto_camera},
			    {"auto_ui_cabc",		tune_auto_ui_cabc},
			    {"auto_gallery_cabc",		tune_auto_gallery_cabc},
			    {"auto_vt_cabc",		tune_auto_vt_cabc},
			    {"auto_browser_cabc",		tune_auto_browser_cabc},
		        {"auto_ebook_cabc",		tune_auto_ebook_cabc},
			    {"email",			tune_auto_ui_cabc}
		    },
		    {   //outdoor
			    {"auto_ui_cabc",		tune_auto_ui_cabc},
			    {"auto_video_cabc",		tune_auto_video_cabc},
			    {"auto_video_cabc",		tune_auto_video_cabc},
			    {"auto_video_cabc",		tune_auto_video_cabc},
			    {"auto_camera",			tune_auto_camera},
			    {"auto_ui_cabc",		tune_auto_ui_cabc},
			    {"auto_gallery_cabc",		tune_auto_gallery_cabc},
			    {"auto_vt_cabc",		tune_auto_vt_cabc},
			    {"auto_browser_cabc",		tune_auto_browser_cabc},
			    {"auto_ebook_cabc",		tune_auto_ebook_cabc},
			    {"email",			tune_auto_ui_cabc}
			}
		}
	}
};

#endif /* __MDNIE_TABLE_H__ */
