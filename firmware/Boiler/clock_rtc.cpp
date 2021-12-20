/*
 *
 */ 

#include <Arduino.h>
#include <iarduino_RTC.h>
#include "clock_rtc.h"


iarduino_RTC watch(RTC_DS1307);


void clock_init() {
    watch.begin();
    //watch.settime(44, 21, 16, 1 , 12 , 20);
    //watch.settime(-1, -1, -1, -1, -1, -1, get_day_of_week());
}

uint8_t clock_get_hours() {
    return watch.Hours;
}

uint8_t clock_get_day_of_week(){
    return ((watch.gettimeUnix() / 86400) + 3) % 7;
}

char* clock_get_time(const char* fmt) {
    return watch.gettime(fmt);
}

void clock_set_time (uint64_t* datetime, uint8_t* timezone){
  watch.settimeUnix(*datetime + (*timezone * 3600));
}
