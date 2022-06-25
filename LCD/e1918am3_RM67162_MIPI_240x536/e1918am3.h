/* drivers/video/sunxi/disp2/disp/lcd/e1918am3.h
 *
 * Copyright (c) 2021 Allwinnertech Co., Ltd.
 *
 * e1918am3 panel driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef _e1918am3_H
#define _e1918am3_H

#include "panels.h"

extern struct __lcd_panel e1918am3_panel;

extern s32 bsp_disp_get_panel_info(u32 screen_id, struct disp_panel_para *info);

#endif /*End of file*/
