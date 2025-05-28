#include <syscalls.h>
#include <stdlib.h>
#include <time.h>

int testMM(){
    int * array = sysBuddyMalloc(sizeof(int) * 8);
    setSrand(getMs());
    for (int i=0; i<8; i++){
        array[i] = rand();
    }
    for (int i=0; i<8; i++){
        printf("%d\n", array[i]);
    }
    return 0;
}