#include <registers.h>

uint64_t registerValues[REGISTER_QUANTITY] = {0};
uint64_t exceptionRegisterValues[REGISTER_QUANTITY] = {0};

static char registerNames[REGISTER_QUANTITY][STR_LEN] = {"RIP", "RAX", "RBX", "RCX", "RDX", "RBP", "RSP", "RDI", "RSI",
                                                         "R08", "R09", "R10", "R11", "R12", "R13", "R14", "R15"};

uint64_t* getRegisterValues() {
  return registerValues;
}

uint64_t* getExceptionRegisterValues() {
  return exceptionRegisterValues;
}

void getRegisters(Register regs[REGISTER_QUANTITY]) {
  for (int i = 0; i < REGISTER_QUANTITY; i++) {
    for (int j = 0; j < STR_LEN; j++) {
      // debería almacenar el 0/, hago esto porque no me deja hacer
      // asignación directa, así que hago un strCopy manual
      regs[i].name[j] = registerNames[i][j];
    }
    regs[i].value = registerValues[i];
  }
}

void printGenericRegisters(uint64_t regs[REGISTER_QUANTITY]) {
  for (int i = 0; i < REGISTER_QUANTITY; i++) {
    printNextString(registerNames[i]);
    printNextChar(' ');
    printNextHexWithPadding(regs[i]);
    printNextChar(' ');
    // Para que entren 3 registros por línea.
    if (i % 3 == 0) printNextChar('\n');
  }
  printNextChar('\n');
}

void printRegisters() {
  printGenericRegisters(registerValues);
}

void printExceptionRegisters() {
  printGenericRegisters(exceptionRegisterValues);
}