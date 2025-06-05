#include <draw.h>
#include <shell.h>
#include <stdarg.h>
#include <stdlib.h>
#include <syscalls.h>
#include <sysinfo.h>

int getKey(KeyStruct* key) {
  int read = sysRead(key, 1);
  return (read == 0) ? EOF : read;
}

char getChar() {
  KeyStruct key;
  int read = getKey(&key);
  if (read == 0) return EOF;
  else return key.character;
}

char screenBuffer[SCREEN_BUFFER_SIZE];
int screenBufWriteIdx = 0;
int screenBufReadIdx = 0;

void incWriteIdx() {
  screenBufWriteIdx = (screenBufWriteIdx + 1) % SCREEN_BUFFER_SIZE;
}
void decWriteIdx() {
  if (--screenBufWriteIdx < 0) screenBufWriteIdx = SCREEN_BUFFER_SIZE - 1;
}
void incReadIdxBy(int val) {
  screenBufReadIdx = (screenBufReadIdx + val) % SCREEN_BUFFER_SIZE;
}

void clearScreen() {
  sysMoveCursor(0, 0);
  clearRectangle(0, 0, systemInfo.screenWidth, systemInfo.screenHeight);
}

void jumpLine() {
  int i = screenBufReadIdx;
  int length = 0;
  while (/* i != screenBufWriteIdx --> this should always be the case &&  */
         screenBuffer[i] != '\n' && length < systemInfo.fontCols
  ) {
    i = (i + 1) % SCREEN_BUFFER_SIZE;
    ++length;
  }
  if (screenBuffer[i] == '\n') ++length;
  incReadIdxBy(length);
  repaint();
}

void printScreenBuffer() {
  for (int i = screenBufReadIdx; i != screenBufWriteIdx; i = (i + 1) % SCREEN_BUFFER_SIZE) {
    /* int endOfScreen = */ sysWriteCharNext(screenBuffer[i]);
    // if (endOfScreen) jumpLine();
  }
}

void repaint() {
  clearScreen();
  printScreenBuffer();
}

void printChar(char c) {
  int endOfScreen = sysWriteCharNext(c);
  if (c == '\b') {
    decWriteIdx();
  } else {
    screenBuffer[screenBufWriteIdx] = c;
    incWriteIdx();
  }
  if (endOfScreen) jumpLine();
}

void printString(const char* s) {
  for (int i = 0; s[i] != 0; ++i) {
    printChar(s[i]);
  }
}

void puts(const char* s) {
  printString(s);
  sysWriteCharNext('\n');
}

int strcmp(const char* s1, const char* s2) {
  int i = 0;
  for (; s1[i] != 0 && s2[i] != 0; ++i) {
    if (s1[i] < s2[i]) return -1;
    else if (s1[i] > s2[i]) return 1;
  }
  if (s1[i] != 0) return 1;
  else if (s2[i] != 0) return -1;
  else return 0;
}

int strFindChar(const char* s, char c) {
  for (int i = 0; s[i] != 0; ++i) {
    if (s[i] == c) return i;
  }
  return -1;
}

unsigned int strlen(char* s) {
  int len = 0;
  while (s[len++] != 0);
  return len - 1;
}

uint32_t uintToBase(unsigned long value, char* buffer, uint32_t base) {
  char* p = buffer;
  char* p1 = buffer;
  char* p2;
  uint32_t digits = 0;

  do {
    uint32_t remainder = value % base;
    *p++ = (remainder < 10) ? remainder + '0' : remainder + 'A' - 10;
    digits++;
  } while (value /= base);

  *p = 0;

  p2 = p - 1;
  while (p1 < p2) {
    char tmp = *p1;
    *p1 = *p2;
    *p2 = tmp;
    p1++;
    p2--;
  }

  return digits;
}
uint32_t intToBase(long value, char* buffer, uint32_t base) {
  char* p = buffer;
  if (value < 0) {
    *p++ = '-';
    value = -value;
  }
  return uintToBase(value, p, base);
}
void printAsBase(long n, int base) {
  char buf[255];
  intToBase(n, buf, base);
  printString(buf);
}
void printUintAsBase(unsigned long n, int base) {
  char buf[255];
  uintToBase(n, buf, base);
  printString(buf);
}

static char paddingChar;
static int paddingLen = 0;
static int paddingSign = 1;
void printPadding() {
  for (int i = 0; i < paddingLen; ++i) {
    printChar(paddingChar);
  }
  paddingLen = 0;
}
void printStringWithAlignedPadding(const char* s) {
  if (paddingSign > 0) {
    printPadding();
    printString(s);
  } else {
    printString(s);
    printPadding();
  }
}
void printAsBaseWithPadding(long n, int base) {
  char buf[255];
  int digits = intToBase(n, buf, base);
  paddingLen = paddingLen - digits;
  char* s = buf;
  if (n < 0) {
    printChar('-');
    ++s;
    --paddingLen;
  }
  printStringWithAlignedPadding(s);
}
void printUintAsBaseWithPadding(long n, int base) {
  char buf[255];
  int digits = uintToBase(n, buf, base);
  paddingLen = paddingLen - digits;
  char* s = buf;
  printStringWithAlignedPadding(s);
}
void printStringWithPadding(const char* s) {
  for (int i = 0; s[i] != 0 && paddingLen > 0; ++i) {
    --paddingLen;
  }
  printStringWithAlignedPadding(s);
}

// Return 0 on successful print, non 0 on error.
int printf(const char* fmt, ...) {
  va_list p;
  va_start(p, fmt);
  paddingLen = 0;

  int i = 0;
  while (fmt[i] != 0) {
    if (fmt[i] != '%' && paddingLen == 0) {
      printChar(fmt[i]);
    } else {
      if (paddingLen == 0) ++i;
      else if (!strContains("dlxs", fmt[i])) {
        printf("...\nError: Only '%%s', '%%d', '%%l' and '%%x' modifiers accept padding.\n");
        return 1;
      }
      switch (fmt[i]) {
      case '%':
        printChar('%');
        break;
      case 'd':
        printAsBaseWithPadding(va_arg(p, int), 10);
        paddingLen = 0;
        break;
      case 'l':
        if (fmt[i + 1] == 'x') {
          ++i;
          printString("0x");
          printUintAsBaseWithPadding(va_arg(p, long), 16);
        } else {
          printAsBaseWithPadding(va_arg(p, long), 10);
        }
        break;
      case 'f':
        printString("'TODO print float'");
        break;
      case 'x':
        printString("0x");
        printUintAsBaseWithPadding(va_arg(p, int), 16);
        break;
      case 'p':
        paddingLen = 8;
        paddingSign = 1;
        paddingChar = '0';
        printString("0x");
        printUintAsBaseWithPadding(va_arg(p, long), 16);
        break;
      case 'b':
        printString("0b");
        printAsBase(va_arg(p, int), 2);
        break;
      case 's':
        printStringWithPadding(va_arg(p, char*));
        break;
      case 'c':
        printChar(va_arg(p, int));
        break;
      default:
        paddingSign = 1;
        if (fmt[i] == '-') {
          paddingSign = -1;
          ++i;
        }
        if (IS_DIGIT(fmt[i])) {
          paddingChar = ' ';
          if (fmt[i] == '0') {
            paddingChar = '0';
            ++i;
          }
          char nbr[MAX_PADDING_DIGITS + 1];
          int j = 0;
          while (IS_DIGIT(fmt[i]) && j < MAX_PADDING_DIGITS) nbr[j++] = fmt[i++];
          if (IS_DIGIT(fmt[i])) {
            printf("...\nFormat error: \"%s\"\n", fmt);
            printf("Maximum padding of %l exceeded\n", pow(10, MAX_PADDING_DIGITS) - 1);
            return 1;
          }
          nbr[j] = 0;
          paddingLen = strToInt(nbr);
        } else {
          printf("\nUnkown option: %%%c\n", fmt[i]);
          return 1;
        }
      }
    }
    if (paddingLen == 0) ++i;
  }
  return 0;
}

int hexCharToDec(char c) {
  c = TO_LOWER(c);
  if (IS_DIGIT(c)) return c - '0';
  else if (IS_HEX_LETTER(c)) return c - 'a' + 10;
  else return -1;
}
int strToInt(char* s) {
  int base = 10;
  int multiplier = 1;
  if (s[0] == '-') {
    multiplier = -1;
    ++s;
  }
  if (s[0] == '0' && s[1] == 'x') {
    base = 16;
    s += 2;
  }
  int j = strlen(s) - 1;
  int n = 0, k = 1;
  while (j >= 0 && (IS_DIGIT(s[j]) || (base == 16 && IS_HEX_LETTER(s[j])))) {
    n += hexCharToDec(s[j]) * k;
    --j;
    k *= base;
  }
  return n * multiplier;
}

void printKey(KeyStruct* key) {
  // printf(
  //   "{char: %c, code: %d, ctrl: %d, lShift: %d, rShift: %d, capsLock: %d, alt: %d}\n",
  //   key->character,
  //   key->code,
  //   key->md.ctrlPressed,
  //   key->md.leftShiftPressed,
  //   key->md.rightShiftPressed,
  //   key->md.capsLockActive,
  //   key->md.altPressed
  // );
  printf(
      "('%c' | %x)%s%s%s%s%s\n", key->character, key->code, key->md.ctrlPressed ? " + ctrl" : "",
      key->md.leftShiftPressed ? " + l-shift" : "", key->md.rightShiftPressed ? " + r-shift" : "",
      key->md.capsLockActive ? " + capsLock" : "", key->md.altPressed ? " + alt" : ""
  );
}

static unsigned int srand = 0;
void setSrand(unsigned int seed) {
  srand = seed;
}
// Return number between 0 and 1073741823 (0x3FFFFFFF).
unsigned int rand() {
  // Using Borland parameters from https://en.wikipedia.org/wiki/Linear_congruential_generator
  // Seems to always alternate between even and odd numbers which kinda sucks but oh well...
  return srand = ((22695477l * srand + 1) % (2l << 31)) & 0x3FFFFFFF;
}
// Apparently returning floating point values is not allowed, I get a compilation error.
// double normalizedRand() {
//   return (double)rand() / 0x3FFFFFFF;
// }
// unsigned int randBetween(int min, int max) {
//   double nrand = normalizedRand();
//   if (nrand >= 1.0) return max;
//   else return (max - min + 1) * nrand + min;
// }
unsigned int randBetween(int min, int max) {
  return rand() % (max - min + 1) + min;
}

void printStringXY(int x, int y, char* s, int fontSize, int charsPerRow) {
  int col = 0;
  for (int i = 0; s[i] != 0; ++i) {
    sysWriteCharXY(x + i * systemInfo.charWidth * fontSize + systemInfo.charSeparation, y, s[i], fontSize);
    if (charsPerRow && col > charsPerRow) {
      y += systemInfo.charHeight * fontSize + systemInfo.charSeparation;
      col = 0;
      x = 0;
    }
    ++col;
  }
}

int strTrimStartOffset(const char* s) {
  int i = 0;
  while (*(s++) == ' ') ++i;
  return i;
}

bool strContains(const char* s, const char c) {
  for (int i = 0; s[i] != 0; ++i) {
    if (s[i] == c) return true;
  }
  return false;
}

char toLower(char c) {
  return TO_LOWER(c);
}

long pow(int x, int n) {
  // Habría que settear un errno o algo en realidad.
  if (n < 0) return x;
  long res = 1;
  for (int i = 0; i < n; ++i) {
    res *= x;
  }
  return res;
}