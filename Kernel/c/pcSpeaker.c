#include <pcSpeaker.h>

void playSoundForCertainMs(uint32_t nFrequence, int ms){
    playSound(nFrequence);
    sleep(ms);
    noSound();
}
