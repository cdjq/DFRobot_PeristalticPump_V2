# DFRobot_PeristalticPump_V2
- [英文版](./README.md)

DFRobot 蠕动泵 Arduino 库，兼容 Arduino 与 ESP32 平台。

![svg]

## 产品链接（www.dfrobot.com）

    SKU：SEN0

## 目录

* [概述](#概述)
* [库安装](#库安装)
* [方法](#方法)
* [兼容性](#兼容性y)
* [历史](#历史)
* [创作者](#创作者)

## 功能
- Arduino 平台使用 `Servo`，ESP32 平台使用 `ESP32Servo`
- 支持转速控制（`0~180`，`90` 为停止）
- 支持串口校准流量并存储到 EEPROM
- 支持基于流量的定量出液

## 库安装
使用前下载库文件（https://github.com/cdjq/DFRobot_PeristalticPump_V2）粘贴到\Arduino\libraries目录下，然后打开examples文件夹，运行该文件夹下的demo。如果你需要使用任何ESP32 驱动该设备，请在库管理中搜索下载ESP32Servo。

## 方法
```cpp
/**
 * @fn begin
 * @brief 初始化泵。
 * @return 无
*/
void begin(void);

/**
 * @fn updatePumpStatus
 * @brief 以非阻塞方式更新泵任务状态。
 * @n     必须在 loop() 中持续调用。
 * @return 无
*/
void updatePumpStatus(void);

/**
 * @fn stopPump
 * @brief 立即停止泵。
 * @return 无
*/
void stopPump(void);

/**
 * @fn setPumpRun
 * @brief 以固定转速和时长启动一个非阻塞运行任务
 * @param speed: 0~180。
 * @n     0：逆时针满速转动
 * @n     90：泵停止
 * @n     180：顺时针满速转动
 * @param runTime: 泵运行时间（单位：毫秒）
 * @return true：任务启动成功；false：失败（通常为任务忙）
 * @n     注意事项：
 * @n     1. 这是非阻塞接口，必须在 loop() 中持续调用 updatePumpStatus()。
 * @n     2. 若当前已有任务在运行，本接口会返回 false。
 * @n     3. 可通过 stopPump() 强制停止当前任务。
*/
bool setPumpRun(uint8_t speed, unsigned long runTime);

/**
 * @fn calPump
 * @brief 校准流量 flowRate 并写入 EEPROM。
 * @return true：flowRate 写入成功；false：失败
 * @n     注意事项：
 * @n     1. 这是阻塞接口，校准过程中会等待串口输入。
 * @n     2. 在串口监视器输入 "SETCAL" 开始校准。
 * @n     3. 泵会顺时针满速运行 15 秒后停止。
 * @n     4. 输入实测的泵出体积（单位：ml）。
 * @n     5. flowRate = volume / 15s，timerPump() 和 volumePump() 会使用该值。
 * @n     6. 本函数内部不直接打印提示；可通过 setCalPumpEventCallback() 输出提示信息。
*/
bool calPump();

/**
 * @fn timerPump
 * @brief 启动一个非阻塞的定时泵液任务。
 * @param time: 泵运行时间（单位：毫秒）
 * @param volume: 输出指针，用于返回预计算泵液体积（单位：ml）
 * @return true：任务启动成功；false：失败
 * @n     液体体积 = FlowRate * time
 * @n     注意事项：
 * @n     1. volume 不能为 NULL。
 * @n     2. 需要有效的校准数据（flowRate）。
 * @n     3. 若当前已有任务在运行，本接口会返回 false。
 * @n     4. 这是非阻塞接口，必须在 loop() 中持续调用 updatePumpStatus()。
*/
bool timerPump(unsigned long time, float *volume);

/**
 * @fn volumePump
 * @brief 启动一个非阻塞的定量泵液任务。
 * @param volume: 目标泵液体积（单位：ml）
 * @param runTime: 输出指针，用于返回预计运行时间（单位：s）
 * @return true：任务启动成功；false：失败
 * @n     预计运行时间 = volume / FlowRate
 * @n     注意事项：
 * @n     1. runTime 不能为 NULL。
 * @n     2. volume 必须大于 0。
 * @n     3. 需要有效的校准数据（flowRate）。
 * @n     4. 若当前已有任务在运行，本接口会返回 false。
 * @n     5. 这是非阻塞接口，必须在 loop() 中持续调用 updatePumpStatus()。
*/
bool volumePump(float volume, float *runTime);
```

## 兼容性
| Board       | Work Well | Work Wrong | Untested | Remarks |
| ----------- | :-------: | :--------: | :------: | ------- |
| Arduino Uno |     √     |            |          |         |
| Mega2560    |     √     |            |          |         |
| Leonardo    |     √     |            |          |         |
| ESP32       |     √     |            |          |         |

## 历史
- 2026/2/26 - V1.0.0 版本

## 创作者
Written by JiaLi(zhixinliu@dfrobot.com), 2026. (Welcome to our website)
