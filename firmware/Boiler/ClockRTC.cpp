#include "ClockRTC.h"

iarduino_RTC ClockRTC::watch(RTC_DS1307);

uint8_t ClockRTC::clock_get_hours() {
  return uint8_t(ClockRTC::watch.Hours);
}

uint8_t ClockRTC::clock_get_day_of_week(){
  return uint8_t(ClockRTC::watch.weekday);
}

char* ClockRTC::clock_get_time(const char* fmt) {
  return ClockRTC::watch.gettime(fmt);
}

void ClockRTC::clock_set_time(uint64_t* datetime, uint8_t* timezone){
  ClockRTC::watch.settimeUnix(uint32_t((*datetime) + (*timezone * 3600)));
}
