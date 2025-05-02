#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <layouts.h>
#include <stdbool.h>
#include <stdint.h>

#define KB_BUF_SIZE 20

typedef struct ModifierKeys {
  bool leftShiftPressed: 1;
  bool rightShiftPressed: 1;
  bool ctrlPressed: 1;
  bool altPressed: 1;
  bool capsLockActive: 1;
  // bool altGrPressed: 1;
} ModifierKeys;

typedef struct KeyStruct {
  uint8_t code;
  char key;
  ModifierKeys md;
} KeyStruct;

enum ModKeyCodes {
  CAPS_LOCK = 0x3A,
  LEFT_SHIFT = 0x2A,
  RIGHT_SHIFT = 0x36,
  CTRL = 0x1D,
  ALT = 0x38,
};

extern uint8_t readKeyCode();
void readKeyToBuffer();
int readKbBuffer(KeyStruct buf[], int len);
void setLayout(KbLayout layout);
KbLayout getLayout();

#endif
