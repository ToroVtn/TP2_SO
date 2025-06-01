#ifndef STDLIB_H
#define STDLIB_H

#include <float.h>
#include <keyboard.h>
#include <stdint.h>

#define NULL (void*)0
#define EOF -1

#define MAX_PADDING_DIGITS 2

#define TO_LOWER(c) ((c >= 'A' && c <= 'Z') ? (c + 'a' - 'A') : c)
#define IS_HEX_LETTER(c) ('a' <= TO_LOWER(c) && TO_LOWER(c) <= 'f')
#define IS_DIGIT(c) ('0' <= c && c <= '9')

// Should be larger than the real max amount of characters
// of fontCols*fontRows = 2117 for smallest font size.
#define SCREEN_BUFFER_SIZE 30000
extern char screenBuffer[SCREEN_BUFFER_SIZE];
extern int screenBufWriteIdx;
extern int screenBufReadIdx;

int getKey(KeyStruct* key);
char getChar();
void printChar(char c);
void clearScreen();
void repaint();
void printString(const char* s);
void puts(const char* s);
uint32_t uintToBase(unsigned long value, char* buffer, uint32_t base);
uint32_t intToBase(long value, char* buffer, uint32_t base);
int strcmp(const char*, const char*);
int strFindChar(const char* s, char c);
unsigned int strlen(char* s);
int printf(const char* fmt, ...);
int strToInt(char* s);
void printKey(KeyStruct* key);
void setSrand(unsigned int seed);
unsigned int rand();
// double normalizedRand();
unsigned int randBetween(int min, int max);
void printStringXY(int x, int y, char* s, int fontSize, int charsPerRow);
int strTrimStartOffset(const char* s);
bool strContains(const char* s, const char c);
char toLower(char c);
long pow(int x, int n);

#endif