#ifndef INTERRUPTIONS_H
#define INTERRUPTIONS_H

#include <stdint.h>
#include <timer.h>

#pragma pack(push, 1)
typedef struct InterruptionDescriptor {
  uint16_t offset_0_15;
  uint16_t segmentSelector;
  uint8_t empty1;
  uint8_t accessPrivileges;
  uint16_t offset_16_31;
  uint32_t offset_32_63;
  uint32_t empty2;
} InterruptionDescriptor;
#pragma pack(pop)

typedef enum PicMask {
  TIMER_TICK_MASK = 0xFFFE,
  KEYBOARD_MASK = 0xFFFD,
  CASCADE_MASK = 0xFFFB,
  SERIAL_2_4_MASK = 0xFFF7,
  SERIAL_1_3_MASK = 0xFFEF,
  USB_MASK = 0xFFDF,
} PicMask;

typedef enum AccessPrivilege {
  ACS_PRESENT = 0x80, // segmento presente en memoria
  ACS_CSEG = 0x18,    // segmento de codigo
  ACS_DSEG = 0x10,    // segmento de datos
  ACS_READ = 0x02,    // segmento de lectura
  ACS_WRITE = 0x02,   // segmento de escritura
  ACS_IDT = ACS_DSEG,
  ACS_INT_386 = 0x0E, // Interrupt GATE 32 bits
  ACS_INT = (ACS_PRESENT | ACS_INT_386),
  ACS_CODE = (ACS_PRESENT | ACS_CSEG | ACS_READ),
  ACS_DATA = (ACS_PRESENT | ACS_DSEG | ACS_WRITE),
  ACS_STACK = (ACS_PRESENT | ACS_DSEG | ACS_WRITE),
} AccessPrivilege;

typedef void (*InterruptionFunction)();

extern void disableInterruptions();
extern void enableInterruptions();
extern void haltTillNextInterruption();
extern void picMask(uint16_t mask);

void irqDispatcher(uint8_t index);
// void setupIdtEntry(int index, InterruptionFunction irqHandler);
void setupIdtEntry(int index, void* irqHandler);
void loadIdt();

extern void irq00Handler();
extern void irq01Handler();
extern void irq02Handler();
extern void irq03Handler();
extern void irq04Handler();
extern void irq05Handler();
extern void irq06Handler();
extern void irq07Handler();

extern void exception00Handler();
extern void exception01Handler();

#endif
