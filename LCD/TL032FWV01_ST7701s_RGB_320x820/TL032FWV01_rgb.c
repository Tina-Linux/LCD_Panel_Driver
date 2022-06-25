/*
 * drivers/video/fbdev/sunxi/disp2/disp/lcd/TL032FWV01_rgb.c
 *
 * Copyright (c) 2018-2021 Allwinnertech Co., Ltd.
 * Author: YuzukiTsuru <gloomyghost@gloomyghost.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

&lcd0 {
	lcd_used            = <1>;

	lcd_driver_name     = "TL032FWV01_rgb";
	lcd_backlight       = <100>;

	lcd_if              = <0>;
	lcd_hv_if           = <0>;

	lcd_x               = <320>;
	lcd_y               = <820>;
	lcd_width           = <24>;
	lcd_height          = <67>;
	lcd_dclk_freq       = <35>;

	lcd_hbp             = <251>;
	lcd_ht              = <576>;
	lcd_hspw            = <5>;
	lcd_vbp             = <100>;
	lcd_vt              = <930>;
	lcd_vspw            = <10>;

	lcd_frm             = <1>;
	lcd_io_phase        = <0x0000>;
	lcd_gamma_en        = <0>;
	lcd_cmap_en         = <0>;
	lcd_hv_clk_phase    = <0>;
	lcd_hv_sync_polarity= <0>;
	lcd_hv_syuv_seq     = <0>;
	lcd_rb_swap         = <0>;

	lcd_gpio_0 = <&pio PD 22 GPIO_ACTIVE_HIGH>; //RST
	lcd_gpio_1 = <&pio PB 10 GPIO_ACTIVE_HIGH>; //CS
	lcd_gpio_2 = <&pio PB 4 GPIO_ACTIVE_HIGH>; //SDA
	lcd_gpio_3 = <&pio PB 6 GPIO_ACTIVE_HIGH>; //SCK
	lcd_bl_en  = <&pio PB 5 GPIO_ACTIVE_HIGH>; //bl
	pinctrl-0  = <&rgb18_pins_a>;
	pinctrl-1  = <&rgb18_pins_b>;
};
 */
#include "TL032FWV01_rgb.h"
#include "default_panel.h"

//s32 sunxi_lcd_gpio_set_value(u32 screen_id, u32 io_index, u32 value)

#define TL032FWV01_spi_scl_1 sunxi_lcd_gpio_set_value(0, 3, 1)
#define TL032FWV01_spi_scl_0 sunxi_lcd_gpio_set_value(0, 3, 0)
#define TL032FWV01_spi_sdi_1 sunxi_lcd_gpio_set_value(0, 2, 1)
#define TL032FWV01_spi_sdi_0 sunxi_lcd_gpio_set_value(0, 2, 0)
#define TL032FWV01_spi_cs_1 sunxi_lcd_gpio_set_value(0, 1, 1)
#define TL032FWV01_spi_cs_0 sunxi_lcd_gpio_set_value(0, 1, 0)
#define TL032FWV01_spi_reset_1 sunxi_lcd_gpio_set_value(0, 0, 1)
#define TL032FWV01_spi_reset_0 sunxi_lcd_gpio_set_value(0, 0, 0)

static void LCD_power_on(u32 sel);
static void LCD_power_off(u32 sel);
static void LCD_bl_open(u32 sel);
static void LCD_bl_close(u32 sel);

static void LCD_panel_init(u32 sel);
static void LCD_panel_exit(u32 sel);

static void LCD_cfg_panel_info(struct panel_extend_para *info)
{
	u32 i = 0, j = 0;
	u32 items;
	u8 lcd_gamma_tbl[][2] = {
		/* {input value, corrected value} */
		{ 0, 0 },     { 15, 15 },   { 30, 30 },   { 45, 45 },
		{ 60, 60 },   { 75, 75 },   { 90, 90 },   { 105, 105 },
		{ 120, 120 }, { 135, 135 }, { 150, 150 }, { 165, 165 },
		{ 180, 180 }, { 195, 195 }, { 210, 210 }, { 225, 225 },
		{ 240, 240 }, { 255, 255 },
	};

	u32 lcd_cmap_tbl[2][3][4] = {
		{
			{ LCD_CMAP_G0, LCD_CMAP_B1, LCD_CMAP_G2, LCD_CMAP_B3 },
			{ LCD_CMAP_B0, LCD_CMAP_R1, LCD_CMAP_B2, LCD_CMAP_R3 },
			{ LCD_CMAP_R0, LCD_CMAP_G1, LCD_CMAP_R2, LCD_CMAP_G3 },
		},
		{
			{ LCD_CMAP_B3, LCD_CMAP_G2, LCD_CMAP_B1, LCD_CMAP_G0 },
			{ LCD_CMAP_R3, LCD_CMAP_B2, LCD_CMAP_R1, LCD_CMAP_B0 },
			{ LCD_CMAP_G3, LCD_CMAP_R2, LCD_CMAP_G1, LCD_CMAP_R0 },
		},
	};

	items = sizeof(lcd_gamma_tbl) / 2;
	for (i = 0; i < items - 1; i++) {
		u32 num = lcd_gamma_tbl[i + 1][0] - lcd_gamma_tbl[i][0];

		for (j = 0; j < num; j++) {
			u32 value = 0;

			value = lcd_gamma_tbl[i][1] +
				((lcd_gamma_tbl[i + 1][1] -
				  lcd_gamma_tbl[i][1]) *
				 j) / num;
			info->lcd_gamma_tbl[lcd_gamma_tbl[i][0] + j] =
				(value << 16) + (value << 8) + value;
		}
	}
	info->lcd_gamma_tbl[255] = (lcd_gamma_tbl[items - 1][1] << 16) +
				   (lcd_gamma_tbl[items - 1][1] << 8) +
				   lcd_gamma_tbl[items - 1][1];

	memcpy(info->lcd_cmap_tbl, lcd_cmap_tbl, sizeof(lcd_cmap_tbl));
}

static s32 LCD_open_flow(u32 sel)
{
	printk("=====================LCD_open_flow\n");
	/* open lcd power, and delay 50ms */
	LCD_OPEN_FUNC(sel, LCD_power_on, 20);
	/* open lcd power, than delay 200ms */
	LCD_OPEN_FUNC(sel, LCD_panel_init, 20);
	/* open lcd controller, and delay 100ms */
	LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 100);
	/* open lcd backlight, and delay 0ms */
	LCD_OPEN_FUNC(sel, LCD_bl_open, 0);

	return 0;
}

static s32 LCD_close_flow(u32 sel)
{
	/* close lcd backlight, and delay 0ms */
	LCD_CLOSE_FUNC(sel, LCD_bl_close, 0);
	/* close lcd controller, and delay 0ms */
	LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 0);
	/* open lcd power, than delay 200ms */
	LCD_CLOSE_FUNC(sel, LCD_panel_exit, 200);
	/* close lcd power, and delay 500ms */
	LCD_CLOSE_FUNC(sel, LCD_power_off, 500);

	return 0;
}

static void LCD_power_on(u32 sel)
{
	printk("=====================LCD_power_on\n");
	/* config lcd_power pin to open lcd power0 */
	sunxi_lcd_power_enable(sel, 0);
	sunxi_lcd_pin_cfg(sel, 1);
}

static void LCD_power_off(u32 sel)
{
	sunxi_lcd_pin_cfg(sel, 0);
	/* config lcd_power pin to close lcd power0 */
	sunxi_lcd_power_disable(sel, 0);
}

static void LCD_bl_open(u32 sel)
{
	printk("=====================LCD_bl_open\n");
	sunxi_lcd_pwm_enable(sel);
	sunxi_lcd_backlight_enable(sel);
}

static void LCD_bl_close(u32 sel)
{
	/* config lcd_bl_en pin to close lcd backlight */
	sunxi_lcd_backlight_disable(sel);
	sunxi_lcd_pwm_disable(sel);
}

//three line 9bit mode
static void LCD_WRITE_DATA(u32 value)
{
	u32 i;
	TL032FWV01_spi_cs_0;
	TL032FWV01_spi_sdi_1;
	TL032FWV01_spi_scl_0;
	sunxi_lcd_delay_us(10);
	TL032FWV01_spi_scl_1;
	for (i = 0; i < 8; i++) {
		sunxi_lcd_delay_us(10);
		if (value & 0x80)
			TL032FWV01_spi_sdi_1;
		else
			TL032FWV01_spi_sdi_0;
		value <<= 1;
		sunxi_lcd_delay_us(10);
		TL032FWV01_spi_scl_0;
		TL032FWV01_spi_scl_1;
	}
	sunxi_lcd_delay_us(10);
	TL032FWV01_spi_cs_1;
}

static void LCD_WRITE_COMMAND(u32 value)
{
	u32 i;
	TL032FWV01_spi_cs_0;
	TL032FWV01_spi_sdi_0;
	TL032FWV01_spi_scl_0;
	sunxi_lcd_delay_us(10);
	TL032FWV01_spi_scl_1;
	for (i = 0; i < 8; i++) {
		sunxi_lcd_delay_us(10);
		if (value & 0x80)
			TL032FWV01_spi_sdi_1;
		else
			TL032FWV01_spi_sdi_0;
		TL032FWV01_spi_scl_0;
		sunxi_lcd_delay_us(10);
		TL032FWV01_spi_scl_1;
		value <<= 1;
	}
	sunxi_lcd_delay_us(10);
	TL032FWV01_spi_cs_1;
}

static void LCD_panel_init(u32 sel)
{
	printk("=====================LCD_panel_init\n");
	LCD_WRITE_COMMAND(0x11);
	sunxi_lcd_delay_ms(20); //sunxi_lcd_delay_ms 20ms

	LCD_WRITE_COMMAND(0xFF);
	LCD_WRITE_DATA(0x77);
	LCD_WRITE_DATA(0x01);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x13);

	LCD_WRITE_COMMAND(0xEF);
	LCD_WRITE_DATA(0x08);

	LCD_WRITE_COMMAND(0xFF);
	LCD_WRITE_DATA(0x77);
	LCD_WRITE_DATA(0x01);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x10);

	LCD_WRITE_COMMAND(0xC0);
	LCD_WRITE_DATA(0xE5);
	LCD_WRITE_DATA(0x02);

	LCD_WRITE_COMMAND(0xC1);
	LCD_WRITE_DATA(0x0C);
	LCD_WRITE_DATA(0x0A);

	LCD_WRITE_COMMAND(0xC2);
	LCD_WRITE_DATA(0x07);
	LCD_WRITE_DATA(0x0F);

	LCD_WRITE_COMMAND(0xC3);
	LCD_WRITE_DATA(0x02);

    // LCD_WRITE_COMMAND(0xCD);
    // LCD_WRITE_DATA(0x18);        //0F 08-OK  D0-D18      

	LCD_WRITE_COMMAND(0xCD);
	LCD_WRITE_DATA(0x08);

	LCD_WRITE_COMMAND(0xB0);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x08);
	LCD_WRITE_DATA(0x51);
	LCD_WRITE_DATA(0x0D);
	LCD_WRITE_DATA(0xCE);
	LCD_WRITE_DATA(0x06);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x08);
	LCD_WRITE_DATA(0x08);
	LCD_WRITE_DATA(0x1D);
	LCD_WRITE_DATA(0x02);
	LCD_WRITE_DATA(0xD0);
	LCD_WRITE_DATA(0x0F);
	LCD_WRITE_DATA(0x6F);
	LCD_WRITE_DATA(0x36);
	LCD_WRITE_DATA(0x3F);

	LCD_WRITE_COMMAND(0xB1);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x10);
	LCD_WRITE_DATA(0x4F);
	LCD_WRITE_DATA(0x0C);
	LCD_WRITE_DATA(0x11);
	LCD_WRITE_DATA(0x05);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x07);
	LCD_WRITE_DATA(0x07);
	LCD_WRITE_DATA(0x1F);
	LCD_WRITE_DATA(0x05);
	LCD_WRITE_DATA(0xD3);
	LCD_WRITE_DATA(0x11);
	LCD_WRITE_DATA(0x6E);
	LCD_WRITE_DATA(0x34);
	LCD_WRITE_DATA(0x3F);

	LCD_WRITE_COMMAND(0xFF);
	LCD_WRITE_DATA(0x77);
	LCD_WRITE_DATA(0x01);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x11);

	LCD_WRITE_COMMAND(0xB0);
	LCD_WRITE_DATA(0x4D);

	LCD_WRITE_COMMAND(0xB1);
	LCD_WRITE_DATA(0x1C);

	LCD_WRITE_COMMAND(0xB2);
	LCD_WRITE_DATA(0x87);

	LCD_WRITE_COMMAND(0xB3);
	LCD_WRITE_DATA(0x80);

	LCD_WRITE_COMMAND(0xB5);
	LCD_WRITE_DATA(0x47);

	LCD_WRITE_COMMAND(0xB7);
	LCD_WRITE_DATA(0x85);

	LCD_WRITE_COMMAND(0xB8);
	LCD_WRITE_DATA(0x21);

	LCD_WRITE_COMMAND(0xB9);
	LCD_WRITE_DATA(0x10);

	LCD_WRITE_COMMAND(0xC1);
	LCD_WRITE_DATA(0x78);

	LCD_WRITE_COMMAND(0xC2);
	LCD_WRITE_DATA(0x78);

	LCD_WRITE_COMMAND(0xD0);
	LCD_WRITE_DATA(0x88);

	sunxi_lcd_delay_ms(100);

	LCD_WRITE_COMMAND(0xE0);
	LCD_WRITE_DATA(0x80);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x02);

	LCD_WRITE_COMMAND(0xE1);
	LCD_WRITE_DATA(0x04);
	LCD_WRITE_DATA(0xA0);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x05);
	LCD_WRITE_DATA(0xA0);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x60);
	LCD_WRITE_DATA(0x60);

	LCD_WRITE_COMMAND(0xE2);
	LCD_WRITE_DATA(0x30);
	LCD_WRITE_DATA(0x30);
	LCD_WRITE_DATA(0x60);
	LCD_WRITE_DATA(0x60);
	LCD_WRITE_DATA(0x3C);
	LCD_WRITE_DATA(0xA0);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x3D);
	LCD_WRITE_DATA(0xA0);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);

	LCD_WRITE_COMMAND(0xE3);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x33);
	LCD_WRITE_DATA(0x33);

	LCD_WRITE_COMMAND(0xE4);
	LCD_WRITE_DATA(0x44);
	LCD_WRITE_DATA(0x44);

	LCD_WRITE_COMMAND(0xE5);
	LCD_WRITE_DATA(0x06);
	LCD_WRITE_DATA(0x3E);
	LCD_WRITE_DATA(0xA0);
	LCD_WRITE_DATA(0xA0);
	LCD_WRITE_DATA(0x08);
	LCD_WRITE_DATA(0x40);
	LCD_WRITE_DATA(0xA0);
	LCD_WRITE_DATA(0xA0);
	LCD_WRITE_DATA(0x0A);
	LCD_WRITE_DATA(0x42);
	LCD_WRITE_DATA(0xA0);
	LCD_WRITE_DATA(0xA0);
	LCD_WRITE_DATA(0x0C);
	LCD_WRITE_DATA(0x44);
	LCD_WRITE_DATA(0xA0);
	LCD_WRITE_DATA(0xA0);

	LCD_WRITE_COMMAND(0xE6);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x33);
	LCD_WRITE_DATA(0x33);

	LCD_WRITE_COMMAND(0xE7);
	LCD_WRITE_DATA(0x44);
	LCD_WRITE_DATA(0x44);

	LCD_WRITE_COMMAND(0xE8);
	LCD_WRITE_DATA(0x07);
	LCD_WRITE_DATA(0x3F);
	LCD_WRITE_DATA(0xA0);
	LCD_WRITE_DATA(0xA0);
	LCD_WRITE_DATA(0x09);
	LCD_WRITE_DATA(0x41);
	LCD_WRITE_DATA(0xA0);
	LCD_WRITE_DATA(0xA0);
	LCD_WRITE_DATA(0x0B);
	LCD_WRITE_DATA(0x43);
	LCD_WRITE_DATA(0xA0);
	LCD_WRITE_DATA(0xA0);
	LCD_WRITE_DATA(0x0D);
	LCD_WRITE_DATA(0x45);
	LCD_WRITE_DATA(0xA0);
	LCD_WRITE_DATA(0xA0);

	LCD_WRITE_COMMAND(0xEB);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x01);
	LCD_WRITE_DATA(0x4E);
	LCD_WRITE_DATA(0x4E);
	LCD_WRITE_DATA(0xEE);
	LCD_WRITE_DATA(0x44);
	LCD_WRITE_DATA(0x00);

	LCD_WRITE_COMMAND(0xED);
	LCD_WRITE_DATA(0xFF);
	LCD_WRITE_DATA(0xFF);
	LCD_WRITE_DATA(0x04);
	LCD_WRITE_DATA(0x56);
	LCD_WRITE_DATA(0x72);
	LCD_WRITE_DATA(0xFF);
	LCD_WRITE_DATA(0xFF);
	LCD_WRITE_DATA(0xFF);
	LCD_WRITE_DATA(0xFF);
	LCD_WRITE_DATA(0xFF);
	LCD_WRITE_DATA(0xFF);
	LCD_WRITE_DATA(0x27);
	LCD_WRITE_DATA(0x65);
	LCD_WRITE_DATA(0x40);
	LCD_WRITE_DATA(0xFF);
	LCD_WRITE_DATA(0xFF);

	LCD_WRITE_COMMAND(0xEF);
	LCD_WRITE_DATA(0x10);
	LCD_WRITE_DATA(0x0D);
	LCD_WRITE_DATA(0x04);
	LCD_WRITE_DATA(0x08);
	LCD_WRITE_DATA(0x3F);
	LCD_WRITE_DATA(0x1F);

	LCD_WRITE_COMMAND(0xFF);
	LCD_WRITE_DATA(0x77);
	LCD_WRITE_DATA(0x01);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x13);

	LCD_WRITE_COMMAND(0xE8);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x0E);

	LCD_WRITE_COMMAND(0xFF);
	LCD_WRITE_DATA(0x77);
	LCD_WRITE_DATA(0x01);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);

	LCD_WRITE_COMMAND(0x11);

	sunxi_lcd_delay_ms(120);

	LCD_WRITE_COMMAND(0xFF);
	LCD_WRITE_DATA(0x77);
	LCD_WRITE_DATA(0x01);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x13);

	LCD_WRITE_COMMAND(0xE8);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x0C);

	sunxi_lcd_delay_ms(10);

	LCD_WRITE_COMMAND(0xE8);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);

	LCD_WRITE_COMMAND(0xFF);
	LCD_WRITE_DATA(0x77);
	LCD_WRITE_DATA(0x01);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);
	LCD_WRITE_DATA(0x00);

	LCD_WRITE_COMMAND(0x3A);
	LCD_WRITE_DATA(0x66);

	LCD_WRITE_COMMAND(0x36);
	LCD_WRITE_DATA(0x00);

	LCD_WRITE_COMMAND(0x35);
	LCD_WRITE_DATA(0x00);

	LCD_WRITE_COMMAND(0x29);
	return;
}

static void LCD_panel_exit(u32 sel)
{
	return;
}

/* sel: 0:lcd0; 1:lcd1 */
static s32 LCD_user_defined_func(u32 sel, u32 para1, u32 para2, u32 para3)
{
	return 0;
}

struct __lcd_panel TL032FWV01_rgb_panel = {
	/* panel driver name, must mach the lcd_drv_name in sys_config.fex */
	.name = "TL032FWV01_rgb",
	.func =
		{
			.cfg_panel_info = LCD_cfg_panel_info,
			.cfg_open_flow = LCD_open_flow,
			.cfg_close_flow = LCD_close_flow,
			.lcd_user_defined_func = LCD_user_defined_func,
		},
};
