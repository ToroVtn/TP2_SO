#include <naiveConsole.h>

static uint32_t uintToBase(uint64_t value, char * buffer, uint32_t base);

static char buffer[64] = { '0' };
static uint8_t * const video = (uint8_t*)0xB8000;
static uint8_t * currentVideo = (uint8_t*)0xB8000;
static const uint32_t width = 80;
static const uint32_t height = 25 ;

static uint8_t* const maxVideo = (uint8_t*)0xb8000 + 2*80*25;

void ncPrint(const char * string)
{
	int i;

	for (i = 0; string[i] != 0; i++)
		ncPrintChar(string[i]);
}

void ncPrintChar(char character)
{
	*currentVideo = character;
	currentVideo += 2;
  if (currentVideo > maxVideo) {
    ncClear();
  }
}

void ncNewline()
{
	do
	{
		ncPrintChar(' ');
	}
	while((uint64_t)(currentVideo - video) % (width * 2) != 0);
}

void ncPrintDec(uint64_t value)
{
	ncPrintBase(value, 10);
}

void ncPrintHex(uint64_t value)
{
  ncPrint("0x");
	ncPrintBase(value, 16);
}

void ncPrintBin(uint64_t value)
{
	ncPrintBase(value, 2);
}

void ncPrintBase(uint64_t value, uint32_t base)
{
    uintToBase(value, buffer, base);
    ncPrint(buffer);
}

void ncClear()
{
	int i;

	for (i = 0; i < height * width; i++)
		video[i * 2] = ' ';
	currentVideo = video;
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




void printBool(bool b) {
  if (b) ncPrint("True");
  else ncPrint("False");
}

void printKeyStruct(KeyStruct k) {
  if (k.key == 0) {
    ncPrint("No key");
    ncNewline();
    return;
  }
  char key[2] = {k.key, 0};
  ncPrint(key);
  ncPrint(" | ");
  ncPrintHex(k.code);
  ncPrint(": { ");
  ncPrint("l-shift: ");
  printBool(k.md.rightShiftPressed);
  ncPrint(", ");
  ncPrint("r-shift: ");
  printBool(k.md.rightShiftPressed);
  ncPrint(", ");
  ncPrint("ctrl: ");
  printBool(k.md.ctrlPressed);
  ncPrint(", ");
  ncPrint("caps lock: ");
  printBool(k.md.capsLockActive);
  ncPrint(", ");
  ncPrint("alt: ");
  printBool(k.md.altPressed);
  ncPrint(" }");
  ncNewline();
}

void printBuffer2(KeyStruct buf[], int len) {
  for (int i = 0; i < len; ++i) {
    printKeyStruct(buf[i]);
  }
}

