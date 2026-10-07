#ifndef __LED_H
#define __LED_H

#include "driver/gpio.h"

//宏定义
#define    LED_TUN     (gpio_get_level(GPIO_NUM_1)?gpio_set_level(GPIO_NUM_1,0):gpio_set_level(GPIO_NUM_1,1))

//函数声明
void led_init(void);  


#endif

