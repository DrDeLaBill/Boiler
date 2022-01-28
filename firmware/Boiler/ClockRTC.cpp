#include "ClockRTC.h"

iarduino_RTC ClockRTC::watch(RTC_DS1307);

ClockRTC::ClockRTC() {
  ClockRTC::watch.begin();
}

uint8_t ClockRTC::clock_get_hours() {
  return ClockRTC::watch.Hours;
}

uint8_t ClockRTC::clock_get_day_of_week(){
  return ((ClockRTC::watch.gettimeUnix() / 86400) + 3) % 7;
}

char* ClockRTC::clock_get_time(const char* fmt) {
  return ClockRTC::watch.gettime(fmt);
}

void ClockRTC::clock_set_time(uint64_t* datetime, uint8_t* timezone){
  ClockRTC::watch.settimeUnix(*datetime + (*timezone * 3600));
}
