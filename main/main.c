#include <stdio.h>
#include "freertos/FreeRTOS.h"            //系统
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "qmi.h"
#include "pic.h"

void app_main(void)
{
    uint8_t buff[50] = {0};
    uint8_t buffb[50] = {0};
    uint8_t buffc[50] = {0};
    t_sQMI8658 value;
    led_init();
    key_init();
    bsp_lvgl_start();                                              //lcd+lvgl配置  
    IIC_Init();                                                    //IIC配置函数
    Qmi8658_Init();                                                //姿态传感器配置

    //显示图片
    lcd_show_pic(0,0,320,240,gImage_pic);

    //标签
    lv_obj_t *xyd = lv_label_create(lv_scr_act());                 //创建标签
    lv_label_set_text(xyd,"hello xyd!");                           //设置显示内容
    lv_obj_set_pos(xyd,100,50);                                    //设置标签在屏幕上面显示位置
    lv_obj_set_style_text_color(xyd,lv_color_hex(0xff0000),0);     //设置显示文本的字体颜色
    lv_obj_set_style_text_font(xyd,&lv_font_montserrat_24,0);

    //显示数据 x轴角度
    lv_obj_t *a = lv_label_create(lv_scr_act());                 //创建标签
    lv_label_set_text(a,"0");                                    //设置显示内容
    lv_obj_set_pos(a,40,110);                                   //设置标签在屏幕上面显示位置
    lv_obj_set_style_text_color(a,lv_color_hex(0xff0000),0);     //设置显示文本的字体颜色
    lv_obj_set_style_text_font(a,&lv_font_montserrat_24,0);

    //显示数据 y轴角度
    lv_obj_t *b = lv_label_create(lv_scr_act());                 //创建标签
    lv_label_set_text(b,"0");                                    //设置显示内容
    lv_obj_set_pos(b,140,110);                                   //设置标签在屏幕上面显示位置
    lv_obj_set_style_text_color(b,lv_color_hex(0x00ff00),0);     //设置显示文本的字体颜色
    lv_obj_set_style_text_font(b,&lv_font_montserrat_24,0);

    //显示数据 z轴角度
    lv_obj_t *c = lv_label_create(lv_scr_act());                 //创建标签
    lv_label_set_text(c,"0");                                    //设置显示内容
    lv_obj_set_pos(c,240,110);                                   //设置标签在屏幕上面显示位置
    lv_obj_set_style_text_color(c,lv_color_hex(0x0000ff),0);     //设置显示文本的字体颜色
    lv_obj_set_style_text_font(c,&lv_font_montserrat_24,0);

    while (1)    
    { 
        qmi8658_fetch_angleFromAcc(&value);
        sprintf((char *)buff,"%0.2f",value.AngleX);
        lv_label_set_text(a,(char *)buff);
        sprintf((char *)buffb,"%0.2f",value.AngleY);
        lv_label_set_text(b,(char *)buffb);
        sprintf((char *)buffc,"%0.2f",value.AngleZ);
        lv_label_set_text(c,(char *)buffc);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

