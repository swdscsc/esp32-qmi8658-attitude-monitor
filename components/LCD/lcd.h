#ifndef __LCD_H
#define __LCD_H

#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_types.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch_ft5x06.h"
#include "esp_lvgl_port.h"


//引脚宏定义
#define   BSP_LCD_SPI_MOSI    (GPIO_NUM_13)
#define   BSP_LCD_SPI_CLK     (GPIO_NUM_12)
#define   BSP_LCD_SPI_CS      (GPIO_NUM_46)
#define   BSP_LCD_DC          (GPIO_NUM_10)
#define   BSP_LCD_RST         (GPIO_NUM_42)
#define   BSP_LCD_BACKLIGHT   (GPIO_NUM_3)

//spi配置宏定义
#define  BSP_LCD_PIXEL_CLOCK_HZ   (80 * 1000 * 1000)
#define  BSP_LCD_SPI_NUM          (SPI3_HOST)
#define  LCD_CMD_BITS             (8)
#define  LCD_PARAM_BITS           (8)
#define  BSP_LCD_BITS_PER_PIXEL   (16)
#define  LCD_LEDC_CH              (LEDC_CHANNEL_0)

//屏幕分辨率宏定义
#define  BSP_LCD_H_RES            (320)
#define  BSP_LCD_V_RES            (240)
#define  BSP_LCD_DRAW_BUF_HEIGHT  (20)

//函数声明
void lcd_set_color(uint16_t color);
esp_err_t bsp_lcd_init(void);
void bsp_lvgl_start(void);
void lcd_show_pic(int x_start,int y_start,int x_end,int y_end,const unsigned char *gImage);

#endif

