#include <stdlib.h>
#include <syscalls.h>
#include <sysinfo.h>

// Por alguna razón sin esto gdb la flashea con la dirección de systemInfo ¯\_()_/¯
const int asdf = 1;

SystemInfo systemInfo;

void setFontGridValues() {
  systemInfo.fontCols = systemInfo.screenWidth/(systemInfo.charWidth*systemInfo.fontSize + systemInfo.charSeparation);
  systemInfo.fontRows = systemInfo.screenHeight/(systemInfo.charHeight*systemInfo.fontSize + systemInfo.charSeparation);
}

void getSysInfo() {
  sysInfo(&systemInfo);
  setFontGridValues();
}

void setFontSize(int size) {
  systemInfo.fontSize = sysSetFontSize(size);
  setFontGridValues();
}

void setLayout(KbLayout layoutCode) {
  systemInfo.layout = layoutCode;
  sysSetLayout(layoutCode);
}
