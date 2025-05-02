#include <registers.h>

static char registersNames[REGISTER_QUANTITY][STR_LEN] = {"RIP", "RAX", "RBX", "RCX", "RDX", "RBP", "RSP", "RDI", "RSI",
                                                          "R08", "R09", "R10", "R11", "R12", "R13", "R14", "R15"};

Register exceptionRegisters[REGISTER_QUANTITY];
static void updateRegisters(Register * registers, uint64_t * registersValues);
    

void getRegisters(Register * registers){
    
    uint64_t * registersValues = getRegistersValues();   // obtengo los valores en hexa
    
    updateRegisters(registers, registersValues);
}

Register * getExceptionRegisters(){
    uint64_t * registersValues = getExceptionRegistersValues();   // obtengo los valores en hexa
    
    updateRegisters(exceptionRegisters, registersValues);
    return exceptionRegisters;
}

static void updateRegisters(Register * registersToUpdate, uint64_t * registersValues){
    for(int i = 0; i < REGISTER_QUANTITY; i++){
        for(int j = 0; j < STR_LEN; j++){
            registersToUpdate[i].name[j] = registersNames[i][j]; // debería almacenar el 0/, hago esto porque no me deja hacer
        }                                                // asignación directa, así que hago un strCopy manual
        registersToUpdate[i].value = registersValues[i];
    }
}