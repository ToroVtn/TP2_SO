#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>
#define REGISTER_QUANTITY 17
#define STR_LEN 4

typedef struct Register{
    uint64_t value;
    char name[STR_LEN];
} Register;

uint64_t * getRegistersValues();

uint64_t * getExceptionRegistersValues();

void getRegisters(Register * registers);

Register * getExceptionRegisters();


#endif