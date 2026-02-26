/*!
 * @file calPump.ino
 * @brief Calibrate peristaltic pump flowRate and store it in EEPROM
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author JiaLi(zhixin.liu@dfrobot.com)
 * @version V1.0
 * @date 2026-02-10
 * @url https://github.com/cdjq/DFRobot_PeristalticPump_V2
 */

#include "DFRobot_PeristalticPump_V2.h"

#if defined(ESP32)
int servoPin = 17;
#else
int servoPin = 9;
#endif

DFRobot_PeristalticPump_V2 pump(servoPin);

void onCalPumpEvent(eCalPumpEvent_t event, float value)
{
  switch (event) {
    case eCalEventWaitSetCal:
      Serial.println(F("Type \"SETCAL\" to start calibration."));
      break;
    case eCalEventInvalidSetCal:
      Serial.println(F("Invalid command. Type \"SETCAL\" to start calibration."));
      break;
    case eCalEventStart:
      Serial.println(F("Calibration started..."));
      break;
    case eCalEventCountdown:
      Serial.print(F("Calibration count down: "));
      Serial.print((int)value);
      Serial.println(F("s"));
      break;
    case eCalEventStop:
      Serial.println(F("Calibration stop."));
      break;
    case eCalEventWaitVolume:
      Serial.println(F("Input pumped volume (ml):"));
      break;
    case eCalEventInvalidVolume:
      Serial.println(F("Invalid value. Please input a number > 0."));
      break;
    case eCalEventWriteFailed:
      Serial.println(F("Write flowRate to EEPROM failed."));
      break;
    case eCalEventDone:
      Serial.println(String(F("flowRate(ml/s): ")) + String(value, 4));
      break;
    default:
      break;
  }
}

void setup()
{
  Serial.begin(115200);
  pump.begin();
  pump.setCalPumpEventCallback(onCalPumpEvent);

  Serial.println(F("DFROBOT Peristaltic Pump - Calibration"));
  Serial.println(F("Type \"SETCAL\" and send newline to start."));
}

void loop()
{
  static bool doneFlag = false;
  if (doneFlag) {
    while (1) {
      delay(1000);
    }
  }

  if (pump.calPump()) {
    Serial.println(F("Calibration success, pump flow rate is stored in EEPROM."));
  } else {
    Serial.println(F("Calibration failed."));
  }
  doneFlag = true;
}
