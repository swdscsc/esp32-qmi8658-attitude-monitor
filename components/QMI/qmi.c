#include "qmi.h"

static const char *TAG = "qmi8658";
/*
    函数功能：iic引脚初始化
    函数参数：无
    函数返回值：无
    备注： 
*/
esp_err_t IIC_Init(void)
{
    i2c_config_t i2c_conf = {0};

    i2c_conf.master.clk_speed   = 100000;
    i2c_conf.mode               = I2C_MODE_MASTER;     //主模式
    i2c_conf.scl_io_num         = BSP_IIC_SCL;         //时钟信号引脚
    i2c_conf.sda_io_num         = BSP_IIC_SDA;         //数据传输引脚
    i2c_conf.scl_pullup_en      = GPIO_PULLUP_ENABLE;  //内置上拉电阻
    i2c_conf.sda_pullup_en      = GPIO_PULLUP_ENABLE;  //内置上拉电阻
  

    i2c_param_config(I2C_NUM_0, &i2c_conf);

    return i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

}


/*
    函数功能：主机发送数据到从机
    函数参数：无
    函数返回值：无
    备注： 
*/
esp_err_t qmi8658_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    int ret;
    uint8_t write_buf[2] = {reg_addr, data};

    ret = i2c_master_write_to_device(I2C_NUM_0, QMI8658_SENSOR_ADDR, write_buf, sizeof(write_buf), 1000 / portTICK_PERIOD_MS);

    return ret;
}


/*
    函数功能：主机接收从机的数据
    函数参数：无
    函数返回值：无
    备注： 
*/
esp_err_t qmi8658_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(I2C_NUM_0, QMI8658_SENSOR_ADDR, &reg_addr, 1, data, len, 1000 / portTICK_PERIOD_MS);
}



/**********************************姿态传感器*******************************/
/*
    函数功能：qmi配置初始化
    函数参数：无
    函数返回值：无
    备注： 
    发送地址0x6a，找到设备
    发送ID号，验证传感器是否存在
    开启陀螺仪和加速度计
    让加速度计和陀螺仪工作
    读取加速度机和陀螺仪的检测数据--获取到原始数据
    将原始数据转换为角度值
    复位
*/
void Qmi8658_Init(void)
{
    uint8_t ID = 0;

    //发送ID号，检测传感器是否存在
    while (ID != 0x05)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);  //提供稳定读取时间
        qmi8658_register_read(QMI8658_WHO_AM_I, &ID, 1);   //读取ID号

    }
    //成功读取，打印查看
    ESP_LOGI(TAG, "read id successfully");

    //发送地址0x6a，找到设备

    //复位
    qmi8658_register_write_byte(QMI8658_RESET, 0xb0);   //0xb0复位
    vTaskDelay(1000 / portTICK_PERIOD_MS);              //提供稳定复位时间
    qmi8658_register_write_byte(QMI8658_CTRL1, 0x40);   //地址自动递增
    qmi8658_register_write_byte(QMI8658_CTRL7, 0x03);   //开启陀螺仪和加速度计
    qmi8658_register_write_byte(QMI8658_CTRL2, 0x95);   //加速度计工作
    qmi8658_register_write_byte(QMI8658_CTRL3, 0xd5);   //陀螺仪工作
}


/*
    函数功能：读取数据函数
    函数参数：无
    函数返回值：无
    备注： 
*/
void Qmi8658_Read_Acc(t_sQMI8658* p)
{
    uint8_t status = 0, data_ready = 0;
    int16_t buf[6];

    //判断标记位为1 -- 表示有数据
    qmi8658_register_read(QMI8658_STATUS0, &status, 1);
    if(status & 0x03)
        data_ready = 1;
    if(data_ready == 1)
    {
        data_ready = 0;

        qmi8658_register_read(QMI8658_AX_L, (uint8_t*)buf, 12);

        //数据存在，开始将数据处理--存放起来
        p->acc_x = buf[0];
        p->acc_y = buf[1];
        p->acc_z = buf[2];
        p->gyr_x = buf[3];
        p->gyr_y = buf[4];
        p->gyr_z = buf[5]; 

    }
}

//将数据转换为角度
// 获取XYZ轴的倾角值
void qmi8658_fetch_angleFromAcc(t_sQMI8658 *p)
{
    float temp;
    Qmi8658_Read_Acc(p); // 读取加速度和陀螺仪的寄存器值
    // 根据寄存器值 计算倾角值 并把弧度转换成角度
    temp = (float)p->acc_x / sqrt( ((float)p->acc_y * (float)p->acc_y + (float)p->acc_z * (float)p->acc_z) );
    p->AngleX = atan(temp)*57.29578f; // 180/π=57.29578
 
    temp = (float)p->acc_y / sqrt( ((float)p->acc_x * (float)p->acc_x + (float)p->acc_z * (float)p->acc_z) );
    p->AngleY = atan(temp)*57.29578f; // 180/π=57.29578

    temp = sqrt( ((float)p->acc_x * (float)p->acc_x + (float)p->acc_y * (float)p->acc_y) ) / (float)p->acc_z;
    p->AngleZ = atan(temp)*57.29578f; // 180/π=57.29578
}

