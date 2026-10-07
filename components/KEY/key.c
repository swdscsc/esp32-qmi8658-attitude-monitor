#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//配置按键连接的芯片引脚 
//IO8 --- 输入模式
void key_init(void)
{
    //定义结构体变量
    gpio_config_t gpioconfig = {
        .intr_type = 0,                         //中断配置 --- 关闭中断
        .mode = GPIO_MODE_INPUT,                //模式配置 --- 输入模式
        .pin_bit_mask = 1ULL << GPIO_NUM_8,     //引脚配置 --- IO8引脚
        .pull_down_en = 0,                      //下拉电阻 --- 关闭下拉
        .pull_up_en = 1,                        //上拉电阻 --- 关闭上拉
    };
    gpio_config(&gpioconfig);
}

//判断按键有没有按下
//返回值：1 表示按键  0  表示按键松开  uint8_t = unsigned char
uint8_t key_scan(void)
{
    static uint8_t key_flag = 1;

    //判断按键按下  判断IO8是否为低电平
    if((gpio_get_level(GPIO_NUM_8) == 0) && (key_flag == 1))      
    {
        //延时消抖 --- 延时15ms
        vTaskDelay(pdMS_TO_TICKS(15)); 
        //再次判断按键是否按下
        if(gpio_get_level(GPIO_NUM_8) == 0)
        {
            key_flag = 0;
            return 1;
        }
    }

    //判断按键松开  判断IO8是否为高电平
    if(gpio_get_level(GPIO_NUM_8) == 1)
    {
        key_flag = 1;
    }

    return 0;
}






