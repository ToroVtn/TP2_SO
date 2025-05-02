#include <utils.h>
#include <shell.h>
#include <stdlib.h>

bool justCtrlMod(KeyStruct* key) {
  return 
    key->md.ctrlPressed &&
    !key->md.altPressed &&
    !key->md.leftShiftPressed &&
    !key->md.rightShiftPressed;
}
