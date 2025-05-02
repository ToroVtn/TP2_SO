#include <videoDriver.h>

struct vbe_mode_info_structure {
  uint16_t attributes; // deprecated, only bit 7 should be of interest
                       // to you, and it indicates the mode
                       // supports a linear frame buffer.

  uint8_t window_a;     // deprecated
  uint8_t window_b;     // deprecated
  uint16_t granularity; // deprecated; used while calculating bank numbers
  uint16_t window_size;
  uint16_t segment_a;
  uint16_t segment_b;
  uint32_t win_func_ptr; // deprecated; used to switch banks from protected mode
                         // without returning to real mode

  uint16_t pitch;  // number of bytes per horizontal line
  uint16_t width;  // width in pixels
  uint16_t height; // height in pixels
  uint8_t w_char;  // unused...
  uint8_t y_char;  // ...
  uint8_t planes;
  uint8_t bpp;   // bits per pixel in this mode
  uint8_t banks; // deprecated; total number of banks in this mode
  uint8_t memory_model;
  uint8_t bank_size; // deprecated; size of a bank, almost always 64 KB but may
                     // be 16 KB...
  uint8_t image_pages;
  uint8_t reserved0;

  uint8_t red_mask;
  uint8_t red_position;
  uint8_t green_mask;
  uint8_t green_position;
  uint8_t blue_mask;
  uint8_t blue_position;
  uint8_t reserved_mask;
  uint8_t reserved_position;
  uint8_t direct_color_attributes;

  uint32_t framebuffer; // physical address of the linear frame buffer; write
                        // here to draw to the screen
  uint32_t off_screen_mem_off;
  uint16_t off_screen_mem_size; // size of memory in the framebuffer but not
                                // being displayed on the screen
  uint8_t reserved1[206];
} __attribute__((packed));

static RGBColor bgColor = {0};
static RGBColor fontColor = {0};
RGBColor cursorColor = {0};

typedef struct vbe_mode_info_structure *VBEInfoPtr;

VBEInfoPtr VBE_mode_info = (VBEInfoPtr)0x0000000000005C00;

#define MAX_FONT_SIZE 4
static int fontSize = 1;
static int fontCols, fontRows;
static int charSeparation = 3;
static int cursorCol = 0;
static int cursorRow = 0;

static uint32_t uintToBase(uint64_t value, char * buffer, uint32_t base);

void setFontGridValues() {
  fontCols = VBE_mode_info->width / (ASCII_BF_WIDTH * fontSize + charSeparation);
  fontRows = VBE_mode_info->height / (ASCII_BF_HEIGHT * fontSize + charSeparation);
}
int getFontSize() {
  return fontSize;
}

int getScreenWidth() {
  return VBE_mode_info->width;
}
int getScreenHeight() {
  return VBE_mode_info->height;
}
int getCharSeparation() {
  return charSeparation;
}

void printPixel(int x, int y, RGBColor color) {
  RGBColor *framebuffer = (RGBColor *)VBE_mode_info->framebuffer;
  uint64_t offset = x + (y * VBE_mode_info->pitch / (VBE_mode_info->bpp / 8));
  framebuffer[offset] = color;
}

void fillRectangle(int x, int y, int width, int height, RGBColor color) {
  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      printPixel(x + j, y + i, color);
    }
  }
}

int setFontSize(int fs) {
  if (1 <= fs && fs <= MAX_FONT_SIZE) {
    fontSize = fs;
    setFontGridValues();
  }
  return fontSize;
}

void printCharXY(int x, int y, char c, int fontSize) {
  if (c < ASCII_BF_MIN || c > ASCII_BF_MAX) return;
  c -= ASCII_BF_MIN;
  for (int i = 0; i < ASCII_BF_HEIGHT; ++i) {
    for (int j = 0; j < ASCII_BF_WIDTH; ++j) {
      if (asciiBitFields[c][i * ASCII_BF_WIDTH + j] != 0) {
        fillRectangle(x + j*fontSize, y + i*fontSize, fontSize, fontSize, fontColor);
      }
    }
  }
}

void colRowToXY(int* col, int* row) {
  *col *= ASCII_BF_WIDTH * fontSize + charSeparation;
  *row *= ASCII_BF_HEIGHT * fontSize + charSeparation;
}

void printChar(int col, int row, char c) {
  int x = col;
  int y = row;
  colRowToXY(&x, &y);
  printCharXY(x, y, c, fontSize);
}

void eraseChar() {
  if (cursorPrev() != 0) return;
  int x = cursorCol;
  int y = cursorRow;
  colRowToXY(&x, &y);
  for (int i = 0; i < ASCII_BF_HEIGHT; ++i) {
    for (int j = 0; j < ASCII_BF_WIDTH; ++j) {
      fillRectangle(x + j*fontSize, y + i*fontSize, fontSize, fontSize, bgColor);
    }
  }
}

// 0 if was able to print char, 1 othwerwise (if cursor reached end of screen).
int printNextChar(char c) {
  if (c == '\b') {
    eraseChar();
  } else if (c == '\n') {
    eraseCursor();
    ++cursorRow;
    cursorCol = 0;
  } else {
    if (!cursorHasNext()) {
      return 1;
    }
    printChar(cursorCol, cursorRow, c);
    int endOfScreen = cursorNext();
    if (endOfScreen) {
      return endOfScreen;
    }
  }
  return 0;
}

void printNextString(const char * str){
    for(int i = 0; str[i] != 0; i++){
        printNextChar(str[i]);
    }
}

void printNextBase(uint64_t value, uint32_t base){
    char buffer[getScreenWidth()];
    uintToBase(value, buffer, base);
    printNextString(buffer);
}

void printNextDec(uint64_t value){
  printNextBase(value, 10);
}

void printNextHex(uint64_t value){
  printNextString("0x");
  printNextBase(value, 16);
}

void printNextHexWithPadding(uint64_t value){
  static int padding = 16;
  char buffer[getScreenWidth()];
  int digits = uintToBase(value, buffer, 16);
  printNextString("0x");
  for(int i = 0 + digits; i < padding; i++){
    printNextChar('0');
  }
	printNextBase(value, 16);
}

void moveCursor(int col, int row) {
  if (col < 0 || row < 0 || col >= fontCols || row >= fontRows) return;
  cursorCol = col;
  cursorRow = row;
}

int cursorHasNext() {
  return (cursorCol < fontCols) && (cursorRow < fontRows);
}

// Return 0 if cursor moved normally, 1 if end of screen (and cursor is reset so start of last line).
int cursorNext() {
  eraseCursor();
  if (++cursorCol >= fontCols) {
    if (++cursorRow >= fontRows) {
      cursorRow = fontRows - 1;
      cursorCol = fontCols;
      return 1;
    }
    cursorCol = 0;
  }
  printCursor();
  return 0;
}

// Return 0 if cursor moved normally, 1 if start of screen.
int cursorPrev() {
  eraseCursor();
  if (--cursorCol < 0) {
    if (--cursorRow < 0) {
      cursorRow = 0;
      cursorCol = 0;
      return 1;
    }
    cursorCol = fontCols - 1;
  }
  printCursor();
  return 0;
}

void printCursorOfColor(RGBColor color) {
  if (cursorCol <= 0) return;
  int x = cursorCol;
  int y = cursorRow;
  colRowToXY(&x, &y);
  fillRectangle(x - 2, y, fontSize, (ASCII_BF_HEIGHT - 3) * fontSize, color);
}
void printCursor() {
  printCursorOfColor(cursorColor);
}
void eraseCursor() {
  printCursorOfColor(bgColor);
}

void setRGBColor(RGBColor* color, uint32_t hexColor) {
  color->blue = hexColor & 0xFF;
  color->green = (hexColor >> 8) & 0xFF;
  color->red = (hexColor >> 16) & 0xFF;
}

void setColor(ColorType c, uint32_t hexColor) {
  switch (c) {
    case BACKGROUND:
      setRGBColor(&bgColor, hexColor);
      break;
    case FONT:
      setRGBColor(&fontColor, hexColor);
      break;
    case CURSOR:
      setRGBColor(&cursorColor, hexColor);
      break;
  }
}

void clearScreen() {
  fillRectangle(0, 0, VBE_mode_info->width, VBE_mode_info->height, bgColor);
  moveCursor(0, 0);
}


static uint32_t uintToBase(uint64_t value, char * buffer, uint32_t base)
{
	char *p = buffer;
	char *p1, *p2;
	uint32_t digits = 0;

	//Calculate characters for each digit
	do
	{
		uint32_t remainder = value % base;
		*p++ = (remainder < 10) ? remainder + '0' : remainder + 'A' - 10;
		digits++;
	}
	while (value /= base);

	// Terminate string in buffer.
	*p = 0;

	//Reverse string in buffer.
	p1 = buffer;
	p2 = p - 1;
	while (p1 < p2)
	{
		char tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}

	return digits;
}