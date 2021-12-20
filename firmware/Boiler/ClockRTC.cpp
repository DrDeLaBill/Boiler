#include "clock_rtc.h"

ClockRTC::ClockRTC() {
  this->watch.begin();
}

uint8_t ClockRTC::clock_get_hours() {
    return this->watch.Hours;
}

uint8_t ClockRTC::clock_get_day_of_week(){
    return ((this->watch.gettimeUnix() / 86400) + 3) % 7;
}

char* ClockRTC::clock_get_time(const char* fmt) {
    return this->watch.gettime(fmt);
}

void ClockRTC::clock_set_time(uint64_t* datetime, uint8_t* timezone){
  this->watch.settimeUnix(*datetime + (*timezone * 3600));
}
