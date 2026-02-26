/*!
 * @file DFRobot_PeristalticPump_V2.h
 * @brief Define the basic structure of the DFRobot_PeristalticPump_V2 class
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author JiaLi(zhixin.liu@dfrobot.com)
 * @version V1.0
 * @date 2026-02-10
 * @url https://github.com/cdjq/DFRobot_PeristalticPump_V2
 */
#ifndef __DFROBOT_PERISTALTICPUMP_V2_H__
#define __DFROBOT_PERISTALTICPUMP_V2_H__

#include <Arduino.h>

#if defined(ESP32)
#include "ESP32Servo.h"
#else
#include "Servo.h"
#endif
#include "EEPROM.h"

#define FLOW_RATE_ADDRESS  0x24    //EEPROM address for flowrate, for more pump need to add more address.
#define PUMP_SPEED_ADDRESS 0x28    //EEPROM address for speed, for more pump need to add more address.
#define CALIBRATION_TIME   15      //when Calibration pump running time, unit secend

#define STOP_SERVO 90    //servo stop position

typedef enum {
  eTaskNone = 0,
  eTaskSetPumpRun,
  eTaskTimerPump,
  eTaskVolumePump
} ePumpTaskType_t;

typedef enum {
  eCalEventWaitSetCal = 0,
  eCalEventInvalidSetCal,
  eCalEventStart,
  eCalEventCountdown,
  eCalEventStop,
  eCalEventWaitVolume,
  eCalEventInvalidVolume,
  eCalEventWriteFailed,
  eCalEventDone
} eCalPumpEvent_t;

typedef struct {
  ePumpTaskType_t type;
  uint8_t         speed;
  unsigned long   startMs;
  unsigned long   durationMs;
  unsigned long   lastProgressCbMs;
  float           flowRateSnapshot;
  float           targetVolume;
} sTaskRuntime_t;

class DFRobot_PeristalticPump_V2 {
public:
  typedef void (*PumpRunDoneCallback)(void);
  typedef void (*TimerPumpCallback)(float volume, bool finished);
  typedef void (*VolumePumpCallback)(float volume, bool finished);
  typedef void (*CalPumpEventCallback)(eCalPumpEvent_t event, float value);

  DFRobot_PeristalticPump_V2(int pin = 9);
  ~DFRobot_PeristalticPump_V2() {};

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
   */
  bool calPump(void);

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

  void setPumpRunDoneCallback(PumpRunDoneCallback cb);
  void setTimerPumpCallback(TimerPumpCallback cb, unsigned long periodMs = 50);
  void setVolumePumpCallback(VolumePumpCallback cb, unsigned long periodMs = 50);
  void setCalPumpEventCallback(CalPumpEventCallback cb);

private:
  void          runPumpBlocking(uint8_t speed, unsigned long runTime);
  bool          writeData(uint16_t addr, uint8_t *data, size_t len);
  bool          readData(uint16_t addr, uint8_t *data, size_t len);
  uint8_t       readCalData(void);
  bool          loadCalData(void);
  bool          writeCalData(float flowRate);
  float         waitVolumeInput(void);
  bool          isTaskRunning(void) const;
  bool          isProgressTask(ePumpTaskType_t task) const;
  unsigned long getProgressPeriodMs(ePumpTaskType_t task) const;
  void          callProgressCallback(ePumpTaskType_t task, float volume, bool finished);
  void          emitCalPumpEvent(eCalPumpEvent_t event, float value) const;
  bool          startTask(ePumpTaskType_t task, uint8_t speed, unsigned long durationMs, float flowRateSnapshot, float targetVolume);
  unsigned long taskElapsedMs(unsigned long nowMs) const;
  float         calcTaskVolume(unsigned long elapsedMs) const;
  void          finishTask(ePumpTaskType_t finishedTask, float finalVolume);
  void          clearTask(void);

  int                  _pin;
  float                _flowRate;
  sTaskRuntime_t       _task;
  uint32_t             _setTaskCount;
  uint32_t             _doneTaskCount;
  PumpRunDoneCallback  _runDoneCb;
  TimerPumpCallback    _timerPumpCb;
  unsigned long        _timerPumpCbPeriodMs;
  VolumePumpCallback   _volumePumpCb;
  unsigned long        _volumePumpCbPeriodMs;
  CalPumpEventCallback _calPumpCb;
#if defined(ESP32)
  bool _eepromInited;
#endif
  Servo _servo;

#if defined(ESP32)
  static const int EEPROM_SIZE = 64;
#endif
};
#endif
