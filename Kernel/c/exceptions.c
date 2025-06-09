#include <exceptions.h>
#include <registers.h>
#include <timer.h>
#include <videoDriver.h>

void zeroDivisionException() {
  clearScreen();
  printNextString("An exception occurred | ExceptionId: 0 - ZeroDivision Error\n\n");
  printExceptionRegisters();
  printNextString("Returning to shell in 10 seconds\n");
  sleep(10000);
  clearScreen();
}

void invalidOpcodeException() {
  clearScreen();
  printNextString("An exception occurred | ExceptionId: 6 - InvalidOpcode Error\n\n");
  printExceptionRegisters();
  printNextString("Returning to shell in 10 seconds\n");
  sleep(10000);
}