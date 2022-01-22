#ifndef _CLOCK_RTC_H_
#define _CLOCK_RTC_H_

#include <Arduino.h>
#include <iarduino_RTC.h>
#include <Wire.h>

class ClockRTC
{
  public:
    static iarduino_RTC *watch;
    static uint8_t clock_get_hours();
    static uint8_t clock_get_day_of_week();
    static char* clock_get_time(const char* fmt);
    
    ClockRTC();
    void clock_set_time (uint64_t* datetime, uint8_t* timezone);
};

#endif
