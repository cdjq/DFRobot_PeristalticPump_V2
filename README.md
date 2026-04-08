# DFRobot_PeristalticPump_V2
- [中文版](./README_CN.md)

DFRobot peristaltic pump Arduino library, compatible with Arduino and ESP32 platforms.

![svg]

## Product Link (www.dfrobot.com)

    SKU: DFR1261

## Table of Contents

* [Overview](#overview)
* [Library Installation](#library-installation)
* [Methods](#methods)
* [Compatibility](#compatibility)
* [History](#history)
* [Credits](#credits)

## Overview
Gravity peristaltic pump Arduino library, compatible with Arduino and ESP32 platforms.

## Features
- Uses `Servo` on Arduino platforms and `ESP32Servo` on ESP32
- Supports speed control (`0~180`, `90` means stop)
- Supports serial flow-rate calibration and stores the value to EEPROM
- Supports fixed-volume pumping based on calibrated flow rate

## Library Installation
Before using, download the library file (https://github.com/DFRobot/DFRobot_PeristalticPump_V2) and paste it into the \Arduino\libraries directory. Then open the examples folder and run the demo in that folder. If you need to use any ESP32 to drive the device, search for and download ESP32Servo in the Library Manager.

## Methods
```cpp
/**
 * @fn begin
 * @brief Initialize the pump.
 * @return None
*/
void begin(void);

/**
 * @fn updatePumpStatus
 * @brief Update pump state in non-blocking mode.
 * @n     Call this function repeatedly in loop().
 * @return None
*/
void updatePumpStatus(void);

/**
 * @fn stopPump
 * @brief Stop pump immediately.
 * @return None
*/
void stopPump(void);

/**
 * @fn setPumpRun
 * @brief Start a non-blocking run task with fixed speed and duration.
 * @param speed: 0~180.
 * @n     0: Rotate counterclockwise at full speed
 * @n     90: Pump stop
 * @n     180: Rotate clockwise at full speed
 * @param runTime: Pump operating time (unit: ms)
 * @return true: task started successfully; false: failed (usually task busy)
 * @n     Notes:
 * @n     1. Non-blocking API. You must call updatePumpStatus() repeatedly in loop().
 * @n     2. If a task is already running, this API returns false.
 * @n     3. You can force stop by calling stopPump().
*/
bool setPumpRun(uint8_t speed, unsigned long runTime);

/**
 * @fn calPump
 * @brief Calibrate pump flowRate and save it to EEPROM.
 * @return true: flowRate written successfully; false: failed
 * @n     Notes:
 * @n     1. This is a blocking API. It waits for serial input during calibration.
 * @n     2. Input "SETCAL" in Serial Monitor to start calibration.
 * @n     3. The pump runs clockwise at full speed for 15 seconds, then stops.
 * @n     4. Input the measured pumped volume (unit: ml).
 * @n     5. flowRate = volume / 15s, and this value is used by timerPump() and volumePump().
 * @n     6. This function does not print by itself. Register setCalPumpEventCallback() for prompts/logs.
 * @n     7. Before calibration, prime the tubing by running setPumpRun.ino until it is fully filled with water.
 * @n     8. Skipping the priming step can lead to inaccurate calibration results.
*/
bool calPump();

/**
 * @fn timerPump
 * @brief Start a non-blocking timed pumping task.
 * @param time: Pump operating time (unit: ms)
 * @param volume: Output pointer for precalculated pumped volume (unit: ml)
 * @return true: task started successfully; false: failed
 * @n     Pumped volume = FlowRate * time
 * @n     Notes:
 * @n     1. volume must not be NULL.
 * @n     2. Calibration data (flowRate) must be valid.
 * @n     3. If another task is running, this API returns false.
 * @n     4. Non-blocking API. You must call updatePumpStatus() repeatedly in loop().
*/
bool timerPump(unsigned long time, float *volume);

/**
 * @fn volumePump
 * @brief Start a non-blocking fixed-volume pumping task.
 * @param volume: Target liquid volume to pump (unit: ml)
 * @param runTime: Output pointer for expected run time (unit: s)
 * @return true: task started successfully; false: failed
 * @n     Expected run time = volume / FlowRate
 * @n     Notes:
 * @n     1. runTime must not be NULL.
 * @n     2. volume must be greater than 0.
 * @n     3. Calibration data (flowRate) must be valid.
 * @n     4. If another task is running, this API returns false.
 * @n     5. Non-blocking API. You must call updatePumpStatus() repeatedly in loop().
*/
bool volumePump(float volume, float *runTime);

/**
 * @fn setPumpRunDoneCallback
 * @brief Register callback for setPumpRun task completion.
 * @param cb: Callback function pointer.
 * @n     Pass NULL to disable callback.
 * @return None
 * @n     Notes:
 * @n     1. Triggered when a setPumpRun task finishes (normal timeout or stopPump()).
 * @n     2. If setPumpRun() fails to start, callback will not be triggered.
*/
void setPumpRunDoneCallback(PumpRunDoneCallback cb);

/**
 * @fn setTimerPumpCallback
 * @brief Register progress callback for timerPump task.
 * @param cb: Callback function pointer.
 * @n     Prototype: void cb(float volume, bool finished)
 * @n     volume: current pumped volume (ml)
 * @n     finished: false during running, true when task finishes
 * @param periodMs: Callback period (unit: ms), default 50.
 * @return None
 * @n     Notes:
 * @n     1. If periodMs is 0, it will be forced to 1.
 * @n     2. Pass NULL to disable callback.
 * @n     3. Effective only for timerPump() task.
*/
void setTimerPumpCallback(TimerPumpCallback cb, unsigned long periodMs = 50);

/**
 * @fn setVolumePumpCallback
 * @brief Register progress callback for volumePump task.
 * @param cb: Callback function pointer.
 * @n     Prototype: void cb(float volume, bool finished)
 * @n     volume: current pumped volume (ml)
 * @n     finished: false during running, true when task finishes
 * @param periodMs: Callback period (unit: ms), default 50.
 * @return None
 * @n     Notes:
 * @n     1. If periodMs is 0, it will be forced to 1.
 * @n     2. Pass NULL to disable callback.
 * @n     3. Effective only for volumePump() task.
*/
void setVolumePumpCallback(VolumePumpCallback cb, unsigned long periodMs = 50);

/**
 * @fn setCalPumpEventCallback
 * @brief Register event callback for calPump procedure.
 * @param cb: Callback function pointer.
 * @n     Prototype: void cb(eCalPumpEvent_t event, float value)
 * @n     event: current calibration event
 * @n     value: event value (countdown/time/flowRate, depending on event)
 * @return None
 * @n     Notes:
 * @n     1. Used to output prompts/progress/results for calPump().
 * @n     2. Pass NULL to disable callback.
*/
void setCalPumpEventCallback(CalPumpEventCallback cb);
```

## Compatibility
| Board       | Work Well | Work Wrong | Untested | Remarks |
| ----------- | :-------: | :--------: | :------: | ------- |
| Arduino Uno |     √     |            |          |         |
| Mega2560    |     √     |            |          |         |
| Leonardo    |     √     |            |          |         |
| ESP32       |     √     |            |          |         |

## History
- 2026/2/26 - Version 1.0.0 released.

## Credits
Written by JiaLi(zhixinliu@dfrobot.com), 2026. (Welcome to our website)
