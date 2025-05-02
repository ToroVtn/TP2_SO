#ifndef PC_SPEAKER_H
#define PC_SPEAKER_H

#include <stdint.h>
#include <timer.h>

void playSound(uint32_t nFrequence);
void playSoundForCertainMs(uint32_t nFrequence, int ms);
void noSound();

#endif