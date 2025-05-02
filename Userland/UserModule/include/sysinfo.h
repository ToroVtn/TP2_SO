#ifndef SYSINFO_H
#define SYSINFO_H

typedef enum KbLayout {
  QWERTY_LATAM = 0,
  QWERTY_US,
} KbLayout;
static const char* const LayoutStrings[] = {
  [QWERTY_LATAM] = "QWERTY LATAM",
  [QWERTY_US] = "QWERTY US",
};
#define TIME_STR_LEN 9
#define REGISTER_STR_LEN 4
#define REGISTER_QUANTITY 17

typedef struct Time { 
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    char string[TIME_STR_LEN];
} Time;

typedef struct Register{
    uint64_t value;
    char name[REGISTER_STR_LEN];
} Register;


typedef struct SystemInfo {
  int screenWidth;
  int screenHeight;
  int charWidth;
  int charHeight;
  int layout;
  int fontSize;
  int charSeparation;
  int fontCols;
  int fontRows;
} SystemInfo;

extern SystemInfo systemInfo;

void getSysInfo();
void setFontSize(int size);
void setLayout(KbLayout code);

#endif
