#ifndef KEYBOARD_H
#define KEYBOARD_H


#include <stdbool.h>
#include <stdint.h>

typedef struct {
  bool leftShiftPressed: 1;
  bool rightShiftPressed: 1;
  bool ctrlPressed: 1;
  bool altPressed: 1;
  bool capsLockActive: 1;
} ModifierKeys;

typedef struct {
  uint8_t code;
  char character;
  ModifierKeys md;
} KeyStruct;


#endif
