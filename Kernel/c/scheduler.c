#include "../include/scheduler.h"


typedef struct PCBnode {
    ProcessControlBlock* pcb;
    ProcessControlBlock* waitingForMe;

}