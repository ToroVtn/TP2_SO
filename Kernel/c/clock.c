#include <clock.h>

void getCurrentTime(Time* toReturn){
    toReturn->hours = getCurrentHours();
    toReturn->minutes = getCurrentMinutes();
    toReturn->seconds = getCurrentSeconds();
    
    (toReturn->string)[TIME_STR_LEN-1] = 0;
    (toReturn->string)[0] = ((toReturn->hours) / 10) + '0';
    (toReturn->string)[1] = ((toReturn->hours) % 10) + '0';
    (toReturn->string)[2] = ':';
    (toReturn->string)[3] = ((toReturn->minutes) / 10) + '0';
    (toReturn->string)[4] = ((toReturn->minutes) % 10) + '0';
    (toReturn->string)[5] = ':';
    (toReturn->string)[6] = ((toReturn->seconds) / 10) + '0';
    (toReturn->string)[7] = ((toReturn->seconds) % 10) + '0';
}