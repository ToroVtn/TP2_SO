#ifndef SHELL_H
#define SHELL_H

#define MAX_ARG_COUNT 20
#define MAX_ARG_LEN 50 + 1

extern int currentCommandIdx;

typedef enum {
  SUCCESS = 0,
  TOO_MANY_ARGUMENTS,
  ARGUMENT_TOO_LONG,
  COMMAND_NOT_FOUND,
  MISSING_ARGUMENTS,
  ILLEGAL_ARGUMENT,
  OUT_OF_BOUNDS,
} CommandResult;
static const char* const CommandResultStrings[] = {
  [SUCCESS] = "Success",
  [TOO_MANY_ARGUMENTS] = "Too many arguments",
  [ARGUMENT_TOO_LONG]  = "Argument too long",
  [COMMAND_NOT_FOUND]  = "Command not found",
  [MISSING_ARGUMENTS]  = "Missing arguments",
  [ILLEGAL_ARGUMENT]  = "Illegal argument",
  [OUT_OF_BOUNDS]  = "Argument of bounds",
};

int shell();

#endif
