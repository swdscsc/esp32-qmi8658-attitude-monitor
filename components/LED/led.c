#include "driver/gpio.h"


void led_init(void)       //ctrl + x
{
    //配置IO1引脚
    gpio_config_t gpioconfig = {
        .intr_type = 0,                       //中断开关 --- 关闭中断
        .mode = GPIO_MODE_INPUT_OUTPUT,       //模式配置 --- 输入输出模式
        .pin_bit_mask = 1ULL << GPIO_NUM_1,   //引脚选择 --- IO1引脚
        .pull_down_en = 0,                    //下拉电阻 --- 关闭下拉电阻 --- 初始电平为低电平
        .pull_up_en = 0,                      //上拉电阻 --- 关闭上拉电阻 --- 初始电平为高电平
    };
    gpio_config(&gpioconfig);

    //关闭灯
    gpio_set_level(GPIO_NUM_1,1);
}



 
