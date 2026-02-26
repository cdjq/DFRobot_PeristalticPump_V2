/*!
 * @file DFRobot_PeristalticPump_V2.cpp
 * @brief The implementation of the DFRobot_PeristalticPump_V2 class
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author JiaLi(zhixin.liu@dfrobot.com)
 * @version V1.0
 * @date 2026-02-10
 * @url https://github.com/DFRobot/DFRobot_PeristalticPump_V2
 */

#include "DFRobot_PeristalticPump_V2.h"

#include <math.h>

DFRobot_PeristalticPump_V2::DFRobot_PeristalticPump_V2(int pin)
{
  this->_pin                   = pin;
  this->_flowRate              = 0.0f;
  this->_task.type             = eTaskNone;
  this->_task.speed            = STOP_SERVO;
  this->_task.startMs          = 0;
  this->_task.durationMs       = 0;
  this->_task.lastProgressCbMs = 0;
  this->_task.flowRateSnapshot = 0.0f;
  this->_task.targetVolume     = 0.0f;
  this->_setTaskCount          = 0;
  this->_doneTaskCount         = 0;
  this->_runDoneCb             = NULL;
  this->_timerPumpCb           = NULL;
  this->_timerPumpCbPeriodMs   = 50;
  this->_volumePumpCb          = NULL;
  this->_volumePumpCbPeriodMs  = 50;
  this->_calPumpCb             = NULL;
#if defined(ESP32)
  this->_eepromInited = false;
#endif
}

bool DFRobot_PeristalticPump_V2::isTaskRunning(void) const
{
  return this->_task.type != eTaskNone;
}

bool DFRobot_PeristalticPump_V2::isProgressTask(ePumpTaskType_t task) const
{
  return (task == eTaskTimerPump) || (task == eTaskVolumePump);
}

unsigned long DFRobot_PeristalticPump_V2::getProgressPeriodMs(ePumpTaskType_t task) const
{
  if (task == eTaskTimerPump) {
    return this->_timerPumpCbPeriodMs;
  } else if (task == eTaskVolumePump) {
    return this->_volumePumpCbPeriodMs;
  }
  return 0;
}

void DFRobot_PeristalticPump_V2::callProgressCallback(ePumpTaskType_t task, float volume, bool finished)
{
  if ((task == eTaskTimerPump) && (this->_timerPumpCb != NULL)) {
    this->_timerPumpCb(volume, finished);
  } else if ((task == eTaskVolumePump) && (this->_volumePumpCb != NULL)) {
    this->_volumePumpCb(volume, finished);
  }
}

void DFRobot_PeristalticPump_V2::emitCalPumpEvent(eCalPumpEvent_t event, float value) const
{
  if (this->_calPumpCb != NULL) {
    this->_calPumpCb(event, value);
  }
}

bool DFRobot_PeristalticPump_V2::startTask(ePumpTaskType_t task, uint8_t speed, unsigned long durationMs, float flowRateSnapshot, float targetVolume)
{
  if (this->isTaskRunning()) {
    return false;
  }

  unsigned long now            = millis();
  this->_task.type             = task;
  this->_task.speed            = speed;
  this->_task.startMs          = now;
  this->_task.durationMs       = durationMs;
  this->_task.lastProgressCbMs = now;
  this->_task.flowRateSnapshot = flowRateSnapshot;
  this->_task.targetVolume     = targetVolume;
  return true;
}

unsigned long DFRobot_PeristalticPump_V2::taskElapsedMs(unsigned long nowMs) const
{
  unsigned long elapsed = nowMs - this->_task.startMs;
  if (elapsed > this->_task.durationMs) {
    elapsed = this->_task.durationMs;
  }
  return elapsed;
}

float DFRobot_PeristalticPump_V2::calcTaskVolume(unsigned long elapsedMs) const
{
  float volume = this->_task.flowRateSnapshot * ((float)elapsedMs / 1000.0f);
  if (volume > this->_task.targetVolume) {
    volume = this->_task.targetVolume;
  }
  return volume;
}

void DFRobot_PeristalticPump_V2::finishTask(ePumpTaskType_t finishedTask, float finalVolume)
{
  if (finishedTask == eTaskSetPumpRun) {
    this->_doneTaskCount++;
    if ((this->_setTaskCount == this->_doneTaskCount) && (this->_runDoneCb != NULL)) {
      this->_runDoneCb();
    }
  } else if (this->isProgressTask(finishedTask)) {
    this->callProgressCallback(finishedTask, finalVolume, true);
  }
}

void DFRobot_PeristalticPump_V2::clearTask(void)
{
  unsigned long now            = millis();
  this->_task.type             = eTaskNone;
  this->_task.speed            = STOP_SERVO;
  this->_task.startMs          = now;
  this->_task.durationMs       = 0;
  this->_task.lastProgressCbMs = now;
  this->_task.flowRateSnapshot = 0.0f;
  this->_task.targetVolume     = 0.0f;
}

void DFRobot_PeristalticPump_V2::updatePumpStatus(void)
{
  if (!this->isTaskRunning()) {
    this->_servo.write(STOP_SERVO);
    return;
  }

  unsigned long now = millis();
  if (now - this->_task.startMs >= this->_task.durationMs) {
    ePumpTaskType_t finishedTask = this->_task.type;
    unsigned long   elapsed      = this->taskElapsedMs(now);
    float           finalVolume  = this->calcTaskVolume(elapsed);
    this->clearTask();
    this->_servo.write(STOP_SERVO);
    this->finishTask(finishedTask, finalVolume);
    return;
  }

  this->_servo.write(this->_task.speed);
  if (this->isProgressTask(this->_task.type)) {
    unsigned long periodMs = this->getProgressPeriodMs(this->_task.type);
    if (now - this->_task.lastProgressCbMs >= periodMs) {
      this->callProgressCallback(this->_task.type, this->calcTaskVolume(this->taskElapsedMs(now)), false);
      this->_task.lastProgressCbMs = now;
    }
  }
}

void DFRobot_PeristalticPump_V2::begin(void)
{
#if defined(ESP32)
  if (!this->_eepromInited) {
    this->_eepromInited = EEPROM.begin(EEPROM_SIZE);
  }
  //ESP32PWM::allocateTimer(0);

  this->_servo.setPeriodHertz(50);
  this->_servo.attach(this->_pin, 500, 2500);
#else
  this->_servo.attach(this->_pin);
#endif
  this->_servo.write(STOP_SERVO);
  this->clearTask();

  this->loadCalData();
}

bool DFRobot_PeristalticPump_V2::setPumpRun(uint8_t speed, unsigned long runTime)
{
  if (this->startTask(eTaskSetPumpRun, speed, runTime, 0.0f, 0.0f)) {
    this->_setTaskCount++;
    return true;
  }
  return false;
}

void DFRobot_PeristalticPump_V2::setPumpRunDoneCallback(PumpRunDoneCallback cb)
{
  this->_runDoneCb = cb;
}

void DFRobot_PeristalticPump_V2::setTimerPumpCallback(TimerPumpCallback cb, unsigned long periodMs)
{
  this->_timerPumpCb = cb;
  if (periodMs == 0) {
    periodMs = 1;
  }
  this->_timerPumpCbPeriodMs = periodMs;
}

void DFRobot_PeristalticPump_V2::setVolumePumpCallback(VolumePumpCallback cb, unsigned long periodMs)
{
  this->_volumePumpCb = cb;
  if (periodMs == 0) {
    periodMs = 1;
  }
  this->_volumePumpCbPeriodMs = periodMs;
}

void DFRobot_PeristalticPump_V2::setCalPumpEventCallback(CalPumpEventCallback cb)
{
  this->_calPumpCb = cb;
}

void DFRobot_PeristalticPump_V2::stopPump(void)
{
  if (this->isTaskRunning()) {
    ePumpTaskType_t stoppedTask = this->_task.type;
    unsigned long   elapsed     = this->taskElapsedMs(millis());
    float           finalVolume = this->calcTaskVolume(elapsed);
    this->clearTask();
    this->finishTask(stoppedTask, finalVolume);
  }
  this->_servo.write(STOP_SERVO);
}

void DFRobot_PeristalticPump_V2::runPumpBlocking(uint8_t speed, unsigned long runTime)
{
  this->_servo.write(speed);
  delay(runTime);
  this->_servo.write(STOP_SERVO);
}

bool DFRobot_PeristalticPump_V2::calPump(void)
{
  this->emitCalPumpEvent(eCalEventWaitSetCal, 0.0f);
  while (Serial.available() > 0) {
    Serial.read();
  }

  while (true) {
    if (Serial.available() > 0) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();
      cmd.toUpperCase();
      if (cmd == "SETCAL") {
        break;
      }
      this->emitCalPumpEvent(eCalEventInvalidSetCal, 0.0f);
    }
    delay(10);
  }

  this->emitCalPumpEvent(eCalEventStart, (float)CALIBRATION_TIME);
  for (int i = CALIBRATION_TIME; i > 0; i--) {
    this->runPumpBlocking(180, 1000);
    this->emitCalPumpEvent(eCalEventCountdown, (float)(i - 1));
  }

  this->emitCalPumpEvent(eCalEventStop, 0.0f);
  this->emitCalPumpEvent(eCalEventWaitVolume, 0.0f);

  float volume = 0.0f;
  while (true) {
    volume = this->waitVolumeInput();
    if (volume > 0.0f) {
      break;
    }
    this->emitCalPumpEvent(eCalEventInvalidVolume, volume);
  }

  this->_flowRate = volume / (float)CALIBRATION_TIME;
  if (!this->writeCalData(this->_flowRate)) {
    this->emitCalPumpEvent(eCalEventWriteFailed, this->_flowRate);
    return false;
  }

  this->emitCalPumpEvent(eCalEventDone, this->_flowRate);
  return true;
}

bool DFRobot_PeristalticPump_V2::timerPump(unsigned long time, float *volume)
{
  if (volume == NULL) {
    return false;
  }

  *volume = 0.0f;
  if (!this->loadCalData()) {
    return false;
  }

  if (this->isTaskRunning()) {
    return false;
  }

  *volume = this->_flowRate * ((float)time / 1000.0f);
  //Serial.println(String(F("Pumped volume(ml): ")) + String(*volume, 4));

  if (!this->startTask(eTaskTimerPump, 0, time, this->_flowRate, *volume)) {
    *volume = 0.0f;
    return false;
  }

  return true;
}

bool DFRobot_PeristalticPump_V2::volumePump(float volume, float *runTime)
{
  if (runTime == NULL) {
    return false;
  }

  *runTime = 0.0f;
  if (!this->loadCalData()) {
    return false;
  }

  if (volume <= 0.0f) {
    return false;
  }

  if (this->isTaskRunning()) {
    return false;
  }

  float runTimeMs = ((float)volume / this->_flowRate) * 1000.0f;
  if (runTimeMs <= 0.0f) {
    return false;
  }

  *runTime = runTimeMs / 1000.0f;
  if (!this->startTask(eTaskVolumePump, 0, (unsigned long)runTimeMs, this->_flowRate, volume)) {
    *runTime = 0.0f;
    return false;
  }

  //Serial.println(String(F("Pump run time(s): ")) + String(*runTime, 2));
  return true;
}

bool DFRobot_PeristalticPump_V2::writeData(uint16_t addr, uint8_t *data, size_t len)
{
  if (data == NULL || len == 0) {
    return false;
  }

#if defined(ESP32)
  if (!this->_eepromInited) {
    return false;
  }
  if ((size_t)addr + len > EEPROM_SIZE) {
    return false;
  }
#endif

  for (size_t i = 0; i < len; i++) {
    EEPROM.write(addr + i, data[i]);
  }

#if defined(ESP32)
  return EEPROM.commit();
#else
  return true;
#endif
}

bool DFRobot_PeristalticPump_V2::readData(uint16_t addr, uint8_t *data, size_t len)
{
  if (data == NULL || len == 0) {
    return false;
  }

#if defined(ESP32)
  if (!this->_eepromInited) {
    return false;
  }
  if ((size_t)addr + len > EEPROM_SIZE) {
    return false;
  }
#endif

  for (size_t i = 0; i < len; i++) {
    data[i] = EEPROM.read(addr + i);
  }

  return true;
}

uint8_t DFRobot_PeristalticPump_V2::readCalData(void)
{
  float flowRate = 0.0f;
  if (!this->readData(FLOW_RATE_ADDRESS, (uint8_t *)&flowRate, sizeof(flowRate))) {
    return 1;
  }
  if (isnan(flowRate) || isinf(flowRate) || flowRate <= 0.0f) {
    return 2;
  }
  //Serial.println(String(F("flowRate(ml/s): ")) + String(flowRate, 4));

  this->_flowRate = flowRate;
  return 0;
}

bool DFRobot_PeristalticPump_V2::loadCalData(void)
{
  uint8_t ret = this->readCalData();
  if (ret == 0) {
    return true;
  } else if (ret == 1) {
    //Serial.println(F("Read flowRate from EEPROM failed."));
  } else if (ret == 2) {
    //Serial.println(F("Uncalibrated flowRate in EEPROM."));
  }
  return false;
}

bool DFRobot_PeristalticPump_V2::writeCalData(float flowRate)
{
  return this->writeData(FLOW_RATE_ADDRESS, (uint8_t *)&flowRate, sizeof(flowRate));
}

float DFRobot_PeristalticPump_V2::waitVolumeInput(void)
{
  while (true) {
    if (Serial.available() > 0) {
      String text = Serial.readStringUntil('\n');
      text.trim();
      if (text.length() == 0) {
        continue;
      }

      return text.toFloat();
    }
    delay(10);
  }
}
