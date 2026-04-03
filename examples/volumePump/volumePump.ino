/*!
 * @file volumePump.ino
 * @brief Run the pump by fixed volume (call once)
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author JiaLi(zhixin.liu@dfrobot.com)
 * @version V1.0
 * @date 2026-02-10
 * @url https://github.com/DFRobot/DFRobot_PeristalticPump_V2
 */

#include "DFRobot_PeristalticPump_V2.h"

#if defined(ESP32)
int servoPin = 17;
#else
int servoPin = 9;
#endif

DFRobot_PeristalticPump_V2 pump(servoPin);

uint8_t volumePumpFlag = 1;
float   runTime        = 0.0f;

void onVolumePumpCallback(float volume, bool finished)
{
  if (finished) {
    Serial.print(F("Final volume(ml): "));
    Serial.println(volume, 4);
  } else {
    Serial.print(F("Current volume(ml): "));
    Serial.println(volume, 4);
  }
}

void setup()
{
  Serial.begin(115200);
  pump.begin();
  pump.setVolumePumpCallback(onVolumePumpCallback, 50);
  Serial.println(F("volumePump demo (call once)."));
  Serial.println(F("Run calPump.ino first to store flowRate in EEPROM."));
}

void loop()
{
  // Update pump status; this must be called repeatedly in loop().
  pump.updatePumpStatus();

  /**
   * Note:
   * 1. Use volumePump() to run the pump for a target volume.
   * 2. Parameter 1: target pumped volume in milliliters (ml).
   * 3. Parameter 2: pointer to a float for the expected run time (s).
   * 4. Return true: task started successfully.
   * 5. Return false: start failed.
   *    - A task may already be running. Wait until it finishes, or call stopPump().
   *    - Calibration data may be invalid. Run calPump.ino to recalibrate flow rate.
  */
  if (volumePumpFlag) {
    volumePumpFlag = 0;
    Serial.println(F("Start: volumePump 100 ml"));
    if (pump.volumePump(100, &runTime)) {
      Serial.print(F("Expected run time(s): "));
      Serial.println(runTime, 2);
    } else {
      Serial.println(F("volumePump start failed."));
    }
  }
}
