#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>
#define TIME_STR_LEN 9

typedef struct Time {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    char string[TIME_STR_LEN];
} Time;


void setBinaryClockFormat();
uint8_t getCurrentSeconds();
uint8_t getCurrentMinutes();
uint8_t getCurrentHours();
void getCurrentTime(Time* toReturn);

#endif