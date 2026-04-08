/*!
 * @file setPumpRun.ino
 * @brief This example drives a peristaltic pump to run.
 * @copyright Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author JiaLi(zhixin.liu@dfrobot.com)
 * @version V1.0
 * @date 2026-02-10
 * @url https://github.com/DFRobot/DFRobot_PeristalticPump_V2
 */

#include "DFRobot_PeristalticPump_V2.h"

/* For continuous rotation servo:
 * speed = 0   : full speed counterclockwise
 * speed = 90  : stop
 * speed = 180 : full speed clockwise
 */
#if defined(ESP32)
int servoPin = 17;
#else
int servoPin = 9;
#endif

DFRobot_PeristalticPump_V2 pump(servoPin);

int setPumpRunFlag = 0;

void onPumpRunDone()
{
  // Task finished. This demo keeps one-shot behavior, so the flag is not reset.
  // setPumpRunFlag = 0;
  Serial.println(F("Pump run task finished!"));
}

void setup()
{
  Serial.begin(115200);
  pump.begin();
  // set pump run done callback function
  pump.setPumpRunDoneCallback(onPumpRunDone);

  Serial.println(F("setPumpRun callback demo."));
  Serial.println(F("One-shot task: run clockwise for 5000ms."));
}

void loop()
{
  // Update pump status; this must be called repeatedly in loop().
  pump.updatePumpStatus();

  if (setPumpRunFlag == 0) {
    setPumpRunFlag = 1;

    // run clockwise for 5000ms
    /* Note:
     * 1. setPumpRun() can be called repeatedly.
     *    If a task is still running, this call returns false.
     *    After the current task finishes, it returns true and starts a new task.
     * 2. The pump can be forcibly stopped at any time by calling stopPump().
     */
    if (pump.setPumpRun(180, 5000)) {
      Serial.println(F("Run clockwise task started!"));
    } else {
      Serial.println(F("The task of setting the water pump to operate failed!"));
    }
  }
  
}
