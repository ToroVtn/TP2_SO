#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  bool leftShiftPressed : 1;
  bool rightShiftPressed : 1;
  bool ctrlPressed : 1;
  bool altPressed : 1;
  bool capsLockActive : 1;
} ModifierKeys;

typedef struct {
  uint8_t code;
  char character;
  ModifierKeys md;
} KeyStruct;

enum ModKeyCodes {
  CAPS_LOCK = 0x3A,
  LEFT_SHIFT = 0x2A,
  RIGHT_SHIFT = 0x36,
  CTRL = 0x1D,
  ALT = 0x38,
};
#endif