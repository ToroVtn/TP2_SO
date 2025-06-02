#ifndef SHELL_COMMANDS_H
#define SHELL_COMMANDS_H

#include <circularBuffer.h>
#include <colors.h>
#include <draw.h>
#include <snake.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <syscalls.h>
#include <sysinfo.h>
#include <utils.h>

#define MAX_ARG_COUNT 30
#define MAX_ARG_LEN 50
#define MAX_COMMAND_COUNT 50
#define MAX_HISTORY_LEN 50

typedef enum {
  SUCCESS = 0,
  TOO_MANY_ARGUMENTS,
  ARGUMENT_TOO_LONG,
  COMMAND_NOT_FOUND,
  MISSING_ARGUMENTS,
  ILLEGAL_ARGUMENT,
  OUT_OF_BOUNDS,
} ExitCode;
static const char* const CommandResultStrings[] = {
    "Success",           "Too many arguments", "Argument too long",  "Command not found",
    "Missing arguments", "Illegal argument",   "Argument of bounds",
};

typedef void (*ShellFunction)(int argc, char* [argc]);



typedef struct ShellCommand {
  char* name;
  char* description;
  ShellFunction function;
  // CommandArgument arguments[MAX_ARG_COUNT];
  // CommandOption options[];
} ShellCommand;

void newPrompt();
void incFont();
void decFont();
void clearLine();
void addCommand(char* name, char* description, ShellFunction function);
void setShellColors(uint32_t fontColor, uint32_t bgColor, uint32_t cursorColor);
void autocomplete();
void deleteWord();
void historyPush();
void historyPrev();
void historyNext();
void resetHistoryCurrentVals();
// void parseCommandOpts(int argc, char argv[argc][MAX_ARG_LEN], int flagCount, CommandOption flags[]);

ExitCode parseCommand();
void commandEcho(int argc, char* argv[argc]);
void commandGetReturnCode();
void commandRealTime();
void commandHelp();
void commandGetKeyInfo();
void commandRand(int argc, char* argv[argc]);
void commandLayout(int argc, char* argv[argc]);
void commandSetColors(int argc, char* argv[argc]);
void commandSysInfo();
void commandGetRegisters(int argc, char* argv[argc]);
void commandSnake(int argc, char* argv[argc]);
void commandTest();
void commandZeroDivisionError();
void commandInvalidOpcodeError();
void commandPs();

#endif