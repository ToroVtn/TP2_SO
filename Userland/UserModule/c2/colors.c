#include <colors.h>
#include <syscalls.h>

RGBColor bgColor = {0};
RGBColor fontColor = {0};
RGBColor cursorColor = {0};
RGBColor fillColor = {0};
RGBColor strokeColor = {0};
void setRGBColor(RGBColor* color, uint32_t hexColor) {
  color->blue = hexColor & 0xFF;
  color->green = (hexColor >> 8) & 0xFF;
  color->red = (hexColor >> 16) & 0xFF;
}

void setBgColor(uint32_t hexColor) {
  sysSetColor(BACKGROUND, hexColor);
  setRGBColor(&bgColor, hexColor);
}
void setFontColor(uint32_t hexColor) {
  sysSetColor(FONT, hexColor);
  setRGBColor(&fontColor, hexColor);
}
void setCursorColor(uint32_t hexColor) {
  sysSetColor(CURSOR, hexColor);
  setRGBColor(&cursorColor, hexColor);
}

void setFillColor(uint32_t hexColor) {
  setRGBColor(&fillColor, hexColor);
}
void setStrokeColor(uint32_t hexColor) {
  setRGBColor(&strokeColor, hexColor);
}
