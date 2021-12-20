#ifndef _CLOCK_RTC_H_
#define _CLOCK_RTC_H_

#include <Arduino.h>
#include <Wire.h>

void clock_init();
uint8_t clock_get_hours();
uint8_t clock_get_day_of_week();
char* clock_get_time(const char* fmt);
void clock_set_time (uint64_t* datetime, uint8_t* timezone);









#endif
