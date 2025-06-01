#include <utils.h>

bool justCtrlMod(KeyStruct* key) {
  return key->md.ctrlPressed && !key->md.altPressed && !key->md.leftShiftPressed && !key->md.rightShiftPressed;
}

void sysFree(void* ptr) {
  return;
}