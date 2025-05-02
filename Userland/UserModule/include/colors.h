#ifndef COLORS_H
#define COLORS_H

#include <stdint.h>

typedef enum {
  BACKGROUND = 0,
  FONT,
  CURSOR,
} FontColors;

typedef struct {
  uint8_t blue;
  uint8_t green;
  uint8_t red;
} RGBColor;

extern RGBColor bgColor;
extern RGBColor fontColor;
extern RGBColor fillColor;
extern RGBColor strokeColor;

void setBgColor(uint32_t hexColor);
void setFontColor(uint32_t hexColor);
void setCursorColor(uint32_t hexColor);
void setFillColor(uint32_t hexColor);
void setStrokeColor(uint32_t hexColor);

#endif
