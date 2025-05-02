#include <asciiBitFields.h>
#include <keyboard.h>
#include <sysinfo.h>
#include <videoDriver.h>

void getSysInfo(SystemInfo* si) {
  si->screenWidth = getScreenWidth();
  si->screenHeight = getScreenHeight();
  si->charWidth = ASCII_BF_WIDTH;
  si->charHeight = ASCII_BF_HEIGHT;
  si->layout = getLayout();
  si->fontSize = getFontSize();
  si->charSeparation = getCharSeparation();
}
