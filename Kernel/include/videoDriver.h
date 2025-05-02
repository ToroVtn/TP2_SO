#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H

#include <asciiBitFields.h>
#include <stdint.h>

typedef struct {
  uint8_t blue;
  uint8_t green;
  uint8_t red;
} RGBColor;

void printPixel(int x, int y, RGBColor color);
void fillRectangle(int x, int y, int width, int height, RGBColor color);

int getFontSize();
int getScreenWidth();
int getScreenHeight();
int getCharSeparation();

typedef enum {
  BACKGROUND = 0,
  FONT,
  CURSOR,
} ColorType;
void setColor(ColorType c, uint32_t hexColor);

void setFontGridValues();
int setFontSize(int fs);
void printCharXY(int x, int y, char c, int fontSize);
void printChar(int col, int row, char c);
int printNextChar(char c);
void printNextString(const char * str);
void printNextBase(uint64_t value, uint32_t base);
void printNextDec(uint64_t value);
void printNextHex(uint64_t value);
void printNextHexWithPadding(uint64_t value);
void moveCursor(int col, int row);
int cursorHasNext();
int cursorNext();
int cursorPrev();
void printCursor();
void eraseCursor();
void clearScreen();

#endif
