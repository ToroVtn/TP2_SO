#include <syscalls.h>
#include <stdlib.h>
#include <time.h>

int testMM(){
    int * array = sysMalloc(sizeof(int) * 8);
    if(array == NULL){
        printf("malloc returned NULL\n");
        return 0;
    }

    setSrand(getMs());
    for (int i=0; i<8; i++){
        array[i] = i+1;
    }
    for (int i=0; i<8; i++){
        printf("%d\n", array[i]);
    }
    return 0;
}