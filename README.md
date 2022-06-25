# LCD_Panel_Driver

Used to store the LCD screen driver used by Tina Linux

## Directory Structure

```
├─BRIDGE                                         # signal conversion bridge
|  ├─ICN6202_MIPI-LVDS                           # Chipname_input-output
│  └─... 
├─DRIVER_DATASHEET                               # Stores the data sheet of the common driver chip
└─LCD
    ├─BH040I-01_ST7701s_RGB_480x480              # Specific Panel Driver
    ├─d310t9362v1_ST7701s_MIPI_800x480           # PanelName_DriverName_DriverInterface_Resolution
    ├─e1918am3_RM67162_MIPI_240x536
    ├─Generic_ST7701s_MIPI                       # Generic Panel Driver
    ├─Generic_ST7789V_i80                        # Generic_DriverName_DriverInterface
    └─...
```

## Information

Please note that these drivers are all used by the Linux Kernel, and some modifications need to be made to use Uboot. details as follows:

Take `st7701s_rgb` as an example:

```diff
diff --git a/st7701s_rgb.c b/st7701s_rgb.c
--- a/st7701s_rgb.c
+++ b/st7701s_rgb.c
@@ -90,7 +90,7 @@ static void LCD_bl_close(u32 sel);
 static void LCD_panel_init(u32 sel);
 static void LCD_panel_exit(u32 sel);
 
-static void LCD_cfg_panel_info(struct panel_extend_para *info)
+static void LCD_cfg_panel_info(panel_extend_para *info)
 {
 	u32 i = 0, j = 0;
 	u32 items;

@@ -513,7 +511,7 @@ static s32 LCD_user_defined_func(u32 sel, u32 para1, u32 para2, u32 para3)
 	return 0;
 }
 
-struct __lcd_panel st7701s_rgb_panel = {
+__lcd_panel_t st7701s_rgb_panel = {
 	/* panel driver name, must mach the lcd_drv_name in sys_config.fex */
 	.name = "st7701s_rgb",
 	.func = {
	
diff --git a/st7701s_rgb.h b/st7701s_rgb.h
--- a/st7701s_rgb.h
+++ b/st7701s_rgb.h
@@ -13,6 +13,6 @@
 
 #include "panels.h"
 
-extern struct __lcd_panel st7701s_rgb_panel;
+extern  __lcd_panel_t st7701s_rgb_panel;
 
 #endif
 
```



