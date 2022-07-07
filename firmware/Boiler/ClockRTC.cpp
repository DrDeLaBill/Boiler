#include "ClockRTC.h"

iarduino_RTC ClockRTC::watch(RTC_DS1307);

ClockRTC::ClockRTC() {
  ClockRTC::watch.begin();
}

uint8_t ClockRTC::clock_get_hours() {
  return ClockRTC::watch.Hours;
}

uint8_t ClockRTC::clock_get_day_of_week(){
  return uint8_t(((ClockRTC::watch.gettimeUnix() / 86400) + 3) % 7);
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
