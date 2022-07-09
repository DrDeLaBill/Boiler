#include "ClockRTC.h"

iarduino_RTC ClockRTC::watch(RTC_DS1307);

uint8_t ClockRTC::clock_get_hours() {
  Serial.println(F("Get Hours unix"));
  Serial.println(ClockRTC::watch.Hours);
  return uint8_t(ClockRTC::watch.Hours);
}

uint8_t ClockRTC::clock_get_day_of_week(){
  Serial.println(F("Get day unix"));
  Serial.println(ClockRTC::watch.weekday);
  return uint8_t(ClockRTC::watch.weekday);
}

char* ClockRTC::clock_get_time(const char* fmt) {
  return ClockRTC::watch.gettime(fmt);
}

void ClockRTC::clock_set_time(uint64_t* datetime, uint8_t* timezone){
  Serial.println(F("Setting clock:"));
  Serial.print(*datetime);
  Serial.print(F(" + "));
  Serial.print(*timezone);
  Serial.print(F(" * 3600 = "));
  Serial.println((*datetime) + (*timezone * 3600));
  ClockRTC::watch.settimeUnix(uint32_t((*datetime) + (*timezone * 3600)));
}
