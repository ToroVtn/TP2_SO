#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>
#include <timer.h>
#include <videoDriver.h>

#define REGISTER_QUANTITY 17
#define STR_LEN 4

typedef struct Register {
  uint64_t value;
  char name[STR_LEN];
} Register;

void printRegisters();
void printExceptionRegisters();
void getRegisters(Register registers[REGISTER_QUANTITY]);
uint64_t* getRegisterValues();
uint64_t* getExceptionRegisterValues();

#endif