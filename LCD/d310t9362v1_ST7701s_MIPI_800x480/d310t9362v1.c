/* drivers/video/sunxi/disp2/disp/lcd/ST7796S.c
 *
 * Copyright (c) 2017 Allwinnertech Co., Ltd.
 * Author: zhengxiaobin <zhengxiaobin@allwinnertech.com>
 *
 * he0801a-068 panel driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
[lcd0]
 * lcd_used            = 1
 *
 * lcd_driver_name     = "d310t9362v1"
 *
 * lcd_bl_0_percent    = 0
 * lcd_bl_40_percent   = 23
 * lcd_bl_100_percent  = 100
 * lcd_backlight       = 88
 *
 * lcd_if              = 4
 * lcd_x               = 480
 * lcd_y               = 640
 * lcd_width           = 36
 * lcd_height          = 65
 * lcd_dclk_freq       = 25
 *
 * lcd_pwm_used        = 1
 * lcd_pwm_ch          = 8
 * lcd_pwm_freq        = 50000
 * lcd_pwm_pol         = 1
 * lcd_pwm_max_limit   = 255
 *
 * lcd_hbp             = 70
 * lcd_ht              = 615
 * lcd_hspw            = 8
 * lcd_vbp             = 30
 * lcd_vt              = 690
 * lcd_vspw            = 10
 *
 * lcd_dsi_if          = 0
 * lcd_dsi_lane        = 2
 * lcd_dsi_format      = 0
 * lcd_dsi_te          = 0
 * lcd_dsi_eotp        = 0
 *
 * lcd_frm             = 0
 * lcd_io_phase        = 0x0000
 * lcd_hv_clk_phase    = 0
 * lcd_hv_sync_polarity= 0
 * lcd_gamma_en        = 0
 * lcd_bright_curve_en = 0
 * lcd_cmap_en         = 0
 *
 * lcdgamma4iep        = 22
 *
 * ;lcd_bl_en           = port:PD09<1><0><default><1>
 * lcd_power            = "vcc-lcd"
 * lcd_pin_power        = "vcc18-dsi"
 * lcd_pin_power1    	= "vcc-pd"
 *
 * ;reset
 * lcd_gpio_0          = port:PD09<1><0><default><1>
*/
#include "d310t9362v1.h"

static void lcd_power_on(u32 sel);
static void lcd_power_off(u32 sel);
static void lcd_bl_open(u32 sel);
static void lcd_bl_close(u32 sel);

static void lcd_panel_init(u32 sel);
static void lcd_panel_exit(u32 sel);

#define panel_reset(sel, val) sunxi_lcd_gpio_set_value(sel, 0, val)

static void lcd_cfg_panel_info(struct panel_extend_para *info)
{
	u32 i = 0, j = 0;
	u32 items;
	u8 lcd_gamma_tbl[][2] = {
	    {0, 0},     {15, 15},   {30, 30},   {45, 45},   {60, 60},
	    {75, 75},   {90, 90},   {105, 105}, {120, 120}, {135, 135},
	    {150, 150}, {165, 165}, {180, 180}, {195, 195}, {210, 210},
	    {225, 225}, {240, 240}, {255, 255},
	};

	u32 lcd_cmap_tbl[2][3][4] = {
	    {
		{LCD_CMAP_G0, LCD_CMAP_B1, LCD_CMAP_G2, LCD_CMAP_B3},
		{LCD_CMAP_B0, LCD_CMAP_R1, LCD_CMAP_B2, LCD_CMAP_R3},
		{LCD_CMAP_R0, LCD_CMAP_G1, LCD_CMAP_R2, LCD_CMAP_G3},
	    },
	    {
		{LCD_CMAP_B3, LCD_CMAP_G2, LCD_CMAP_B1, LCD_CMAP_G0},
		{LCD_CMAP_R3, LCD_CMAP_B2, LCD_CMAP_R1, LCD_CMAP_B0},
		{LCD_CMAP_G3, LCD_CMAP_R2, LCD_CMAP_G1, LCD_CMAP_R0},
	    },
	};

	items = sizeof(lcd_gamma_tbl) / 2;
	for (i = 0; i < items - 1; i++) {
		u32 num = lcd_gamma_tbl[i + 1][0] - lcd_gamma_tbl[i][0];

		for (j = 0; j < num; j++) {
			u32 value = 0;

			value =
			    lcd_gamma_tbl[i][1] +
			    ((lcd_gamma_tbl[i + 1][1] - lcd_gamma_tbl[i][1]) *
			     j) /
				num;
			info->lcd_gamma_tbl[lcd_gamma_tbl[i][0] + j] =
			    (value << 16) + (value << 8) + value;
		}
	}
	info->lcd_gamma_tbl[255] = (lcd_gamma_tbl[items - 1][1] << 16) +
				   (lcd_gamma_tbl[items - 1][1] << 8) +
				   lcd_gamma_tbl[items - 1][1];

	memcpy(info->lcd_cmap_tbl, lcd_cmap_tbl, sizeof(lcd_cmap_tbl));
}

static s32 lcd_open_flow(u32 sel)
{
	printk("=====================lcd_open_flow\n");
	LCD_OPEN_FUNC(sel, lcd_power_on, 10);
	LCD_OPEN_FUNC(sel, lcd_panel_init, 120);
	LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 120);
	LCD_OPEN_FUNC(sel, lcd_bl_open, 0);
	return 0;
}

static s32 lcd_close_flow(u32 sel)
{
	printk("=====================lcd_close_flow\n");
	LCD_CLOSE_FUNC(sel, lcd_bl_close, 0);
	LCD_CLOSE_FUNC(sel, lcd_panel_exit, 200);
	LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 0);
	LCD_CLOSE_FUNC(sel, lcd_power_off, 500);

	return 0;
}

static void lcd_power_on(u32 sel)
{
	printk("=====================lcd_power_on\n");
	sunxi_lcd_pin_cfg(sel, 1);
	sunxi_lcd_power_enable(sel, 0);
	// sunxi_lcd_power_enable(sel, 1);
	sunxi_lcd_delay_ms(50);

	/* reset lcd by gpio */
	panel_reset(sel, 1);
	sunxi_lcd_delay_ms(5);
	panel_reset(sel, 0);
	sunxi_lcd_delay_ms(10);
	panel_reset(sel, 1);
	sunxi_lcd_delay_ms(120);
}

static void lcd_power_off(u32 sel)
{
	printk("=====================lcd_power_off\n");
	sunxi_lcd_pin_cfg(sel, 0);
	sunxi_lcd_delay_ms(20);
	panel_reset(sel, 0);
	sunxi_lcd_delay_ms(5);
	sunxi_lcd_power_disable(sel, 0);
}

static void lcd_bl_open(u32 sel)
{
	printk("=====================lcd_bl_open\n");
	sunxi_lcd_pwm_enable(sel);
	sunxi_lcd_backlight_enable(sel);
}

static void lcd_bl_close(u32 sel)
{
	printk("=====================lcd_bl_close\n");
	sunxi_lcd_backlight_disable(sel);
	sunxi_lcd_pwm_disable(sel);
}


#define REGFLAG_DELAY 0XFC
#define REGFLAG_END_OF_TABLE 0xFD /* END OF REGISTERS MARKER */

struct LCM_setting_table {
	u8 cmd;
	u32 count;
	u8 para_list[32];
};

static struct LCM_setting_table lcm_initialization_setting[] = {
	{0x01, 1, {0x00} },
	{REGFLAG_DELAY, REGFLAG_DELAY, {120} },

	{0x11, 1, {0x00} },
	{REGFLAG_DELAY, REGFLAG_DELAY, {120} },

	{0xff, 5, {0x77, 0x01, 0x00, 0x00, 0x11} },
	{0xd1, 1, {0x11} },
	{0x55, 1, {0xb0} },

	{0xff, 5, {0x77, 0x01, 0x00, 0x00, 0x10} },
	{0xc0, 2, {0x63, 0x00} }, // SCNL = (0x63 + 1) * 8 = 800
	{0xc1, 2, {0x09, 0x02} }, // VFB=0x09  VBF=0x02
	{0xc2, 2, {0x37, 0x08} }, // PCLK= 512 + (0x08 * 16) = 640

	{0xc7, 1, {0x00} }, // x-dir  rotate 0 : 0x00     rotate 180 :0x04

	{0xcc, 1, {0x38} },

	{0xb0, 16, {0x00, 0x11, 0x19, 0x0c, 0x10, 0x06, 0x07, 0x0a, 0x09, 0x22,
		   0x04, 0x10, 0x0e, 0x28, 0x30, 0x1c} },

	{0xb1, 16, {0x00, 0x12, 0x19, 0x0d, 0x10, 0x04, 0x06, 0x07, 0x08, 0x23,
		    0x04, 0x12, 0x11, 0x28, 0x30, 0x1c} },

	{0xff, 5, {0x77, 0x01, 0x00, 0x00, 0x11} }, //  enable  bk fun of  command 2  BK1
	{0xb0, 1, {0x4d} },
	{0xb1, 1, {0x5b} }, // 0x56  0x4a  0x5b
	{0xb2, 1, {0x07} },
	{0xb3, 1, {0x80} },
	{0xb5, 1, {0x47} },
	{0xb7, 1, {0x8a} },
	{0xb8, 1, {0x21} },
	{0xc1, 1, {0x78} },
	{0xc2, 1, {0x78} },
	{0xd0, 1, {0x88} },
	{REGFLAG_DELAY, REGFLAG_DELAY, {100} },

	{0xe0, 3, {0x00, 0x00, 0x02} },
	{0xe1, 11, {0x01, 0xa0, 0x03, 0xa0, 0x02, 0xa0, 0x04, 0xa0, 0x00, 0x44,
		    0x44} },
	{0xe2, 12, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		    0x00, 0x00} },
	{0xe3, 4, {0x00, 0x00, 0x33, 0x33} },
	{0xe4, 2, {0x44, 0x44} },
	{0xe5, 16, {0x01, 0x26, 0xa0, 0xa0, 0x03, 0x28, 0xa0, 0xa0, 0x05, 0x2a,
		    0xa0, 0xa0, 0x07, 0x2c, 0xa0, 0xa0} },
	{0xe6, 4, {0x00, 0x00, 0x33, 0x33} },
	{0xe7, 2, {0x44, 0x44} },
	{0xe8, 16, {0x02, 0x26, 0xa0, 0xa0, 0x04, 0x28, 0xa0, 0xa0, 0x06, 0x2a,
		    0xa0, 0xa0, 0x08, 0x2c, 0xa0, 0xa0} },
	{0xeb, 7, {0x00, 0x01, 0xe4, 0xe4, 0x44, 0x00, 0x40} },
	{0xed, 16, {0xff, 0xf7, 0x65, 0x4f, 0x0b, 0xa1, 0xcf, 0xff, 0xff, 0xfc,
		    0x1a, 0xb0, 0xf4, 0x56, 0x7f, 0xff} },

	{0xff, 5, {0x77, 0x01, 0x00, 0x00, 0x00} },
	{0x36, 1, {0x00} }, // U&D  Y-DIR  rotate 0: 0x00 : rotate 180 :0x10
	{0x3a, 1, {0x55} },
	{0x29, 1, {0x00} },
	{REGFLAG_END_OF_TABLE, REGFLAG_END_OF_TABLE, {} }
};

static void lcd_panel_init(u32 sel)
{
	u32 i = 0;

	sunxi_lcd_dsi_clk_enable(sel);
	sunxi_lcd_delay_ms(100);

	for (i = 0;; i++) {
		if (lcm_initialization_setting[i].cmd == REGFLAG_END_OF_TABLE)
			break;
		else if (lcm_initialization_setting[i].cmd == REGFLAG_DELAY)
			sunxi_lcd_delay_ms(lcm_initialization_setting[i].count);
		else {
			dsi_dcs_wr(0, lcm_initialization_setting[i].cmd,
				   lcm_initialization_setting[i].para_list,
				   lcm_initialization_setting[i].count);
		}
	}
}

static void lcd_panel_exit(u32 sel)
{
	sunxi_lcd_dsi_dcs_write_0para(sel, 0x28);
	sunxi_lcd_delay_ms(80);
	sunxi_lcd_dsi_dcs_write_0para(sel, 0x10);
	sunxi_lcd_delay_ms(50);
}

/*sel: 0:lcd0; 1:lcd1*/
static s32 lcd_user_defined_func(u32 sel, u32 para1, u32 para2, u32 para3)
{
	return 0;
}

struct __lcd_panel d310t9362v1_panel = {
	/* panel driver name, must mach the name of
	 * lcd_drv_name in sys_config.fex
	 */
	.name = "d310t9362v1",
	.func = {
		.cfg_panel_info = lcd_cfg_panel_info,
		.cfg_open_flow = lcd_open_flow,
		.cfg_close_flow = lcd_close_flow,
		.lcd_user_defined_func = lcd_user_defined_func,
	},
};
