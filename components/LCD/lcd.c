#include "lcd.h"

#define    TAG    "LCD"

static esp_lcd_panel_handle_t panel_handle = NULL;
static esp_lcd_panel_io_handle_t io_handle = NULL;

//lcd背光灯配置
void bsp_lcd_backlight_init(void)
{
    //配置IO1引脚
    gpio_config_t gpioconfig = {
        .intr_type = 0,                       //中断开关 --- 关闭中断
        .mode = GPIO_MODE_OUTPUT,             //模式配置 --- 输出模式
        .pin_bit_mask = 1ULL << BSP_LCD_BACKLIGHT, //引脚选择 --- IO3引脚
        .pull_down_en = 0,                    //下拉电阻 --- 关闭下拉电阻 --- 初始电平为低电平
        .pull_up_en = 0,                      //上拉电阻 --- 关闭上拉电阻 --- 初始电平为高电平
    };
    gpio_config(&gpioconfig);

    //打开背光灯
    gpio_set_level(GPIO_NUM_3,1);
}

// 液晶屏初始化
esp_err_t bsp_display_new(void)
{
    esp_err_t ret = ESP_OK;

    //屏幕引脚和空间进行配置
    const spi_bus_config_t buscfg = {
        .sclk_io_num = BSP_LCD_SPI_CLK,                   //配置时钟线引脚
        .mosi_io_num = BSP_LCD_SPI_MOSI,                  //配置数据线引脚 --- 输出（显示）
        .miso_io_num = GPIO_NUM_NC,                       //配置数据线引脚 --- 输入（触摸）
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = BSP_LCD_H_RES * BSP_LCD_V_RES * sizeof(uint16_t),             
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(BSP_LCD_SPI_NUM, &buscfg, SPI_DMA_CH_AUTO), TAG, "SPI init");

    //配置spi通信控制引脚
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = BSP_LCD_DC,                       //数据命令选择线配置     选择发送数据还是发送命令
        .cs_gpio_num = BSP_LCD_SPI_CS,                   //片选线配置             通信开关
        .pclk_hz = BSP_LCD_PIXEL_CLOCK_HZ,               //时钟频率               80MHz
        .lcd_cmd_bits = LCD_CMD_BITS,                    //命令位                 8bit
        .lcd_param_bits = LCD_PARAM_BITS,                //数据位                 8bit
        .spi_mode = 3,                                   //模式配置               
        .trans_queue_depth = 10,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)BSP_LCD_SPI_NUM, &io_config, &io_handle), err, TAG, "io");


    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = BSP_LCD_RST,                   //复位引脚配置
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,      //颜色数据格式配置
        .bits_per_pixel = BSP_LCD_BITS_PER_PIXEL,        //配置颜色数据  ---  16bit
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle), err, TAG, "panel");

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);

    // ====================== 320×240 全屏关键 ======================
    esp_lcd_panel_set_gap(panel_handle, 0, 0);                   //设置屏幕偏移  ---  设置屏幕的起始点
    esp_lcd_panel_invert_color(panel_handle, true);              //颜色反转
    esp_lcd_panel_swap_xy(panel_handle, true);                   //开启xy轴的互换  ---  横屏显示
    esp_lcd_panel_mirror(panel_handle, true, false);             //开启x轴的镜像，关闭y轴的镜像

    bsp_lcd_backlight_init();                                    //配置背光灯，打开背光灯

    return ret;

err:
    if (panel_handle) esp_lcd_panel_del(panel_handle);
    if (io_handle) esp_lcd_panel_io_del(io_handle);
    spi_bus_free(BSP_LCD_SPI_NUM);
    return ret;
}


//设置屏幕颜色
void lcd_set_color(uint16_t color)
{
    uint16_t *buf = heap_caps_malloc(BSP_LCD_H_RES * sizeof(uint16_t),MALLOC_CAP_SPIRAM);   //优化DMA内存，DMA加速刷屏
    if(!buf)               //错误检测
    {
        return ;
    }
    for(int i=0;i<BSP_LCD_H_RES;i++)
    {
        buf[i] = color;
    }
    for(int y=0;y<BSP_LCD_V_RES;y++)
    {
        esp_lcd_panel_draw_bitmap(panel_handle,0,y,BSP_LCD_H_RES,y+1,buf);
    }
    heap_caps_free(buf);
}

//屏幕配置函数
//把屏幕变成白色屏幕
esp_err_t bsp_lcd_init(void)
{
    esp_err_t ret = bsp_display_new();
    lcd_set_color(0xffff);
    esp_lcd_panel_disp_on_off(panel_handle,true);
    return ret;
}


//屏幕初始化
static lv_disp_t *bsp_display_lcd_init(void)
{
    bsp_display_new();
    lcd_set_color(0xffff);
    esp_lcd_panel_disp_on_off(panel_handle,true);

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = BSP_LCD_H_RES * BSP_LCD_DRAW_BUF_HEIGHT,
        .double_buffer = true,
        .hres = BSP_LCD_H_RES,
        .vres = BSP_LCD_V_RES,
        .monochrome = false,
        .rotation = {
            .mirror_x = true,
            .mirror_y = false,
            .swap_xy = true,
        },
        .flags = {
            .buff_spiram = true,
        },
    };
    return lvgl_port_add_disp(&disp_cfg);
}

//lvgl配置函数
static lv_disp_t *disp;
void bsp_lvgl_start(void)
{
    lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&lvgl_cfg);
    disp = bsp_display_lcd_init();
}


//显示图片函数
void lcd_show_pic(int x_start,int y_start,int x_end,int y_end,const unsigned char *gImage)
{
    size_t pixels_byte_size = (x_end - x_start) * (y_end - y_start) * 2;
    uint16_t *pixels = (uint16_t *)heap_caps_malloc(pixels_byte_size,MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    if(pixels == NULL)
    {
        ESP_LOGE(TAG,"MEMORY FOR BITMAP IS NOT ENOUGH!");
        return ;
    }
    memcpy(pixels,gImage,pixels_byte_size);
    esp_lcd_panel_draw_bitmap(panel_handle,x_start,y_start,x_end,y_end,(uint16_t *)pixels);
    heap_caps_free(pixels);
}








