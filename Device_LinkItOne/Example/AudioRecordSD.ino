/*
  File: AudioRecordSD.ino
  Purpose:
    Record audio from mic and store to SD
  Original:
    The sample come from MediaTek Lab forum, the contribution of zhyena
      http://labs.mediatek.com/forums/posts/list/135.page
*/
#include "vmmm.h"
#include "LTask.h"
#include <LAudio.h>
//#include "vmio.h"
#include <LSD.h>
#include <LStorage.h>
#define Drv LSD

VMINT res = 0;
void cb(VMINT result) {
  res = result;
}
static boolean record_start(void *userData) {
  VMINT16 filename[32];
  vm_record_start("E", "\\", "1", VM_FORMAT_AMR, filename, cb); // drv_name: "C"- Internal flash, "E"-SD, usd vm_get_removable_driver to get the correct drv_name  //vm_record_start((VMSTR)drv, "\\", "1", VM_FORMAT_AMR, filename, cb);
  return true;
}

static boolean record_stop(void* userData) {
  vm_record_stop();
  return true;
}
void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(115200);
  LAudio.begin();
  LAudio.setVolume(6);
  Drv.begin();
}
boolean record_flag = false;
unsigned long start_time, record_time;
void loop() {
  int button = analogRead(A0);
  Serial.println(button);
  if (record_flag == false)
  {
    if (button >= 500)
    {
      LTask.remoteCall(&record_start, NULL);
      record_flag = true;
      digitalWrite(13, HIGH);
      start_time = millis();
    }
  }
  else
  {
    if (button < 500)
    {
      LTask.remoteCall(&record_stop, NULL);
      record_time = millis();

      if (record_time >= start_time)
        record_time -= start_time;
      else
        record_time = 4294967295 - start_time + record_time;
      delay(1000);
      LAudio.playFile(storageSD, "1.amr"); // storageFlash
      delay(1000 + record_time);
      record_flag = false;
      digitalWrite(13, LOW);
    }
  }
}

