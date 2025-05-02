#ifndef NAIVE_CONSOLE_H
#define NAIVE_CONSOLE_H

#include <keyboard.h>
#include <stdbool.h>
#include <stdint.h>

void ncPrint(const char * string);
void ncPrintChar(char character);
void ncNewline();
void ncPrintDec(uint64_t value);
void ncPrintHex(uint64_t value);
void ncPrintBin(uint64_t value);
void ncPrintBase(uint64_t value, uint32_t base);
void ncClear();

void printBool(bool b);
void printKeyStruct(KeyStruct k);
void printBuffer2(KeyStruct buf[], int len);

#endif
