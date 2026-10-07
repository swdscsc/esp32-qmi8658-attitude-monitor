# 基于 ESP32-S3 与 QMI8658 的三轴姿态采集与实时显示系统

使用 ESP32-S3 通过 I2C 读取 QMI8658 六轴姿态传感器的加速度/陀螺仪原始数据，
换算为 X/Y/Z 三轴倾角，并使用 LVGL 在 320×240 IPS 屏上以红/绿/蓝三色实时刷新显示。

本项目是 ESP32 嵌入式实训的阶段性综合作品，覆盖 **I2C 驱动编写 → 传感器寄存器配置 →
原始数据换算 → LVGL 图形界面 → 图片取模显示** 的完整链路。

---

## 硬件平台

| 项目 | 参数 |
| --- | --- |
| MCU | ESP32-S3 |
| 姿态传感器 | QMI8658（六轴 IMU，本项目使用加速度计 + 陀螺仪） |
| 显示屏 | 320×240 IPS LCD，SPI 接口，RGB565（16 bit） |
| 框架 | ESP-IDF + FreeRTOS |
| 图形库 | LVGL 8.3（通过 `esp_lvgl_port` 组件引入） |

## 接线表

### QMI8658（I2C）

| ESP32-S3 | QMI8658 | 说明 |
| --- | --- | --- |
| GPIO41 | SCL | I2C 时钟线，100 kHz，启用内部上拉 |
| GPIO42 | SDA | I2C 数据线，启用内部上拉 |
| 3V3 | VCC | — |
| GND | GND | — |

> 从机地址 `0x6A`。代码中使用 ESP32 内部上拉电阻（`GPIO_PULLUP_ENABLE`），
> 若通信不稳定可改为外部 4.7 kΩ 上拉。

### LCD（SPI）

| 信号 | GPIO | 备注 |
| --- | --- | --- |
| MOSI | GPIO13 | SPI3_HOST |
| CLK | GPIO12 | 像素时钟 80 MHz |
| CS | GPIO46 | — |
| DC | GPIO10 | 命令/数据切换 |
| RST | GPIO42 | — |
| 背光 | GPIO3 | LEDC 通道 0 调光 |

## 软件结构

```
esp32-qmi8658-attitude-monitor/
├── main/
│   ├── main.c              # 应用入口：初始化各外设 + 数据显示主循环
│   ├── idf_component.yml   # LVGL / esp_lvgl_port 依赖声明
│   └── CMakeLists.txt
├── components/
│   ├── QMI/qmi.c|.h        # ★ QMI8658 I2C 驱动（寄存器读写、初始化、角度换算）
│   ├── LCD/lcd.c|.h        # SPI LCD 驱动 + LVGL 移植启动
│   ├── LCD/pic.h           # 取模后的背景图片点阵数组
│   ├── LED/led.c|.h        # LED 驱动
│   └── KEY/key.c|.h        # 按键扫描
├── docs/                   # 开发笔记与问题记录
├── CMakeLists.txt
└── dependencies.lock
```

## 关键实现说明

### 1. QMI8658 初始化流程（`components/QMI/qmi.c`）

初始化严格按数据手册的时序要求进行：

1. **设备探测**：读 `WHO_AM_I`（0x00），直到读回 `0x05` 才继续，避免上电未稳定导致误判
2. **软复位**：写 `RESET`（0x60）为 `0xB0`，延时等待复位完成
3. **配置寄存器**：
   - `CTRL1`(0x02) ← `0x40`：开启地址自动递增，便于连续读取
   - `CTRL7`(0x08) ← `0x03`：同时使能加速度计与陀螺仪
   - `CTRL2`(0x03) ← `0x95`：加速度计量程与输出速率配置
   - `CTRL3`(0x04) ← `0xD5`：陀螺仪量程与输出速率配置

### 2. 数据读取与角度换算

```c
// 读状态寄存器，确认数据就绪后再取 12 字节（6 × int16）
qmi8658_register_read(QMI8658_STATUS0, &status, 1);
if (status & 0x03) {
    qmi8658_register_read(QMI8658_AX_L, (uint8_t*)buf, 12);
}

// 用 atan 反解倾角，57.29578 = 180/π
temp = (float)p->acc_x / sqrt(acc_y * acc_y + acc_z * acc_z);
p->AngleX = atan(temp) * 57.29578f;
```

先查 `STATUS0` 再读数据，避免在传感器尚未完成一次采样时读到无效值。

### 3. LVGL 显示（`main/main.c`）

屏幕上分三个标签显示三轴角度，分别用红、绿、蓝区分，与坐标轴对应：

| 轴 | 屏幕位置 | 颜色 |
| --- | --- | --- |
| X | (40, 110) | 红 `0xFF0000` |
| Y | (140, 110) | 绿 `0x00FF00` |
| Z | (240, 110) | 蓝 `0x0000FF` |

背景图来自 `pic.h` 中取模生成的点阵数组，通过 `lcd_show_pic()` 直接刷到 GRAM。

主循环以 `vTaskDelay(pdMS_TO_TICKS(10))` 控制节奏，刷新周期约 **10 ms**。

## 编译与烧录

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash monitor
```

> 依赖的 LVGL / `esp_lvgl_port` / `esp_lcd_touch_ft5x06` 由 ESP-IDF 组件管理器
> 依据 `main/idf_component.yml` 自动拉取，无需手动下载。

## 开发中遇到的问题

见 [`docs/遇到的问题与解决.md`](docs/遇到的问题与解决.md)。

## 后续优化方向

- [ ] **将主循环重构为 FreeRTOS 多任务架构**：拆分为采集任务、解算任务、UI 刷新任务，
      通过队列传递数据，避免采集被 UI 渲染阻塞（当前版本在 `app_main` 主循环中顺序执行）
- [ ] 引入互补滤波 / 卡尔曼滤波融合加速度计与陀螺仪数据，抑制动态抖动
- [ ] 通过 WiFi 将姿态数据上报云端，实现远程监测
- [ ] 用 `esp_lcd_touch_ft5x06` 驱动触摸屏，增加交互界面

---

## License

MIT
