/* sampleCodeModule.c */

#include <colors.h>
#include <draw.h>
#include <utils.h>
#include <shell.h>
#include <shellCommands.h>
#include <snake.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <sysinfo.h>
#include <syscalls.h>

int currentCommandIdx = 0;

static int commandReturnCode = 0;

void newPrompt();
void incFont();
void decFont();
void addCommand(char* name, char* description, ShellFunction function);
void setShellColor();
CommandResult parseCommand();

int shell() {
  setShellColor();
  clearScreen();

  addCommand("help", "List all commands and their descriptions.", commandHelp);
  addCommand("echo", "Print all arguments.", commandEcho);
  addCommand("$?", "Print previous command return code.", commandGetReturnCode);
  addCommand("realTime", "Get current time", commandRealTime);
  addCommand("keyInfo", "Get pressed key info. Exit with ctrl+c.", commandGetKeyInfo);
  addCommand("rand", "Generate random numbers.", commandRand);
  addCommand("layout", "Get or set current layout. \n    Available flags: --help, --list", commandLayout);
  addCommand("setColors", "Set font and background colors.", commandSetColors);
  addCommand("sysInfo", "Get some system information.", commandSysInfo);
  addCommand("getRegisters", "Get the values of the saved registers. \n    Available flags: --help", commandGetRegisters);
  addCommand("snake", "Play snake.", commandSnake);
  addCommand("zeroDivisionError", "Test the zero division error", commandZeroDivisionError);
  addCommand("invalidOpcodeError", "Test the invalid opcode error", commandInvalidOpcodeError);
  commandHelp();
  newPrompt();

  KeyStruct key;
  while (1) {
    sysHalt();
    if (getKey(&key) != EOF) {
      if (key.md.ctrlPressed) {
        switch (key.character) {
          case '+':
            incFont();
            break;
          case '-':
            decFont();
            break;
          case 'l':
            clearScreen();
            screenBufWriteIdx = 0;
            screenBufReadIdx = 0;
            currentCommandIdx = 0;
            newPrompt();
            break;
        }
      } else {
        if (key.character == '\n') {
          printChar(key.character);
          commandReturnCode = parseCommand();
          newPrompt();
        } else if (key.character == '\b') {
          if (screenBufWriteIdx != currentCommandIdx) {
            printChar(key.character);
          }
        } else {
          printChar(key.character);
        }
      }
    }
  };

  return 1;
}

void setShellColor() {
  setBgColor(0x1A1B26);
  setFontColor(0xC0CAF5);
  setCursorColor(0xFFFF11);
}

void incFont() {
  setFontSize(systemInfo.fontSize + 1);
  repaint();
}
void decFont() {
  setFontSize(systemInfo.fontSize - 1);
  repaint();
}

static char* prompt = " > ";
static char* errorPrompt = " >! ";
void newPrompt() {
  char* currentPrompt = (commandReturnCode == 0) ? prompt : errorPrompt;
  printString(currentPrompt);
  currentCommandIdx = screenBufWriteIdx;
}

static ShellCommand commands[50];
static int commandCount = 0;
void addCommand(char* name, char* description, ShellFunction function) {
  ShellCommand newCommand = {.name = name, .description = description, .function = function};
  commands[commandCount++] = newCommand;
}
ShellFunction getCommand(char* name) {
  for (int i = 0; i < commandCount; ++i) {
    if (strcmp(name, commands[i].name) == 0) return commands[i].function;
  }
  return NULL;
}

CommandResult parseCommand() {
  // Upto MAX_ARG_COUNT arguments (including the actual command)
  // of MAX_COMMAND_LEN characters each (+1 for null termination).
  char argv[MAX_ARG_COUNT][MAX_ARG_LEN];
  int i = currentCommandIdx;
  int len = 0;
  int argc = 0;
  ShellFunction command;
  while (argc < MAX_ARG_COUNT) {
    if (screenBuffer[i] != ' ' && screenBuffer[i] != '\n' && len < MAX_ARG_LEN) {
      argv[argc][len++] = screenBuffer[i];
      i = (i + 1) % SCREEN_BUFFER_SIZE;
    } else {
      argv[argc][len] = 0;
      if (len == MAX_ARG_LEN) {
        printf("%s: %s...\n", CommandResultStrings[ARGUMENT_TOO_LONG], argv[argc]);
        return ARGUMENT_TOO_LONG;
      }
      if (argc == 0) {
        if (argv[0][0] == 0) return 0;
        command = getCommand(argv[0]);
        if (command == NULL) {
          printf("%s: %s\n", CommandResultStrings[COMMAND_NOT_FOUND], argv[0]);
          return COMMAND_NOT_FOUND;
        }
      }
      ++argc;
      len = 0;
      if (screenBuffer[i] == '\n') {
        return command(argc, argv);
      }
      i = (i + 1) % SCREEN_BUFFER_SIZE;
      while (screenBuffer[i] == ' ');
    }
  }
  puts(CommandResultStrings[TOO_MANY_ARGUMENTS]);
  return TOO_MANY_ARGUMENTS;
}

CommandResult commandEcho(int argc, char argv[argc][MAX_ARG_LEN]) {
  // Starts at 1 because first arg is the command name
  for (int i = 1; i < argc; ++i) {
    printf("%s ", argv[i]);
  }
  printChar('\n');
  return SUCCESS;
}

CommandResult commandGetReturnCode() {
  printf("%s - Code: %d\n", CommandResultStrings[commandReturnCode], commandReturnCode);
  return SUCCESS;
}

CommandResult commandRealTime(){
  Time currentTime;
  sysGetCurrentTime(&currentTime);
  printf("%s\n", currentTime.string);
  return SUCCESS;
}

CommandResult commandHelp() {
  printString("Available commands:\n");
  for (int i = 0; i < commandCount; ++i) {
    printf("\t- %s: %s\n", commands[i].name, commands[i].description);
  }
  return SUCCESS;
}

CommandResult commandGetKeyInfo() {
  KeyStruct key;
  while (1) {
    sysHalt();
    if (getKey(&key) != EOF) {
      if (justCtrlMod(&key) && key.character == 'c') return SUCCESS;
      else {
        printKey(&key);
      }
    }
  }
  return SUCCESS;
}

CommandResult commandRand(int argc, char argv[argc][MAX_ARG_LEN]) {
  static bool randInitialized = false;
  if (!randInitialized) {
    setSrand(sysGetTicks());
    randInitialized = true;
  }
  if (argc < 3) {
    puts("Usage:");
    printf("\t\t%s <min> <max> [count]\n", argv[0]);
    printf("Where all arguments are integers and count is optional.\n");
    return MISSING_ARGUMENTS;
  }
  int min = strToInt(argv[1]);
  int max = strToInt(argv[2]);
  if (max < min) {
    puts("Error: min can't be greater than max");
    return ILLEGAL_ARGUMENT;
  }
  int count = (argc > 3) ? strToInt(argv[3]) : 1;
  while (count--) {
    printf("%d%s", randBetween(min, max), (count == 0) ? "" : ", ");
  }
  printf("\n");
  return SUCCESS;
}

CommandResult commandLayout(int argc, char (*argv)[MAX_ARG_LEN]) {
  if (argc == 1) {
    printf("Current layout: %s - %d\n", LayoutStrings[systemInfo.layout], systemInfo.layout);
    return SUCCESS;
  }
  if (strcmp(argv[1], "--help") == 0) {
    printf("Usage\n");
    printf("\t\t%s [option] [layout]\n", argv[0]);
    printf("Options:\n");
    printf("\t\t--help    print this help message.\n");
    printf("\t\t--list    list all available layouts.\n");
    printf("If `layout` is not included then get current layout. If `layout` included then set system layout.\n");
    printf("`layout` should be a valid code. To see valid values for the"
           "`layout` argument use the --list option\n");
  } else if (strcmp(argv[1], "--list") == 0) {
    printf("- %s: %d\n", LayoutStrings[QWERTY_LATAM], QWERTY_LATAM);
    printf("- %s: %d\n", LayoutStrings[QWERTY_US], QWERTY_US);
  } else {
    int code = strToInt(argv[1]);
    if (code != QWERTY_LATAM && code != QWERTY_US) {
      printf("Layout not available: %s\n", argv[1]);
      return ILLEGAL_ARGUMENT;
    }
    setLayout(code);
    printf("Layout set to %s\n", LayoutStrings[code]);
  }
  return SUCCESS;
}

CommandResult commandSetColors(int argc, char (*argv)[MAX_ARG_LEN]) {
  if (argc < 4) {
    puts("Usage:");
    printf("\t\t%s <fontColor> <backgroundColor> <cursorColor>\n", argv[0]);
    printf("Where all arguments should be hex colors.\n");
    return MISSING_ARGUMENTS;
  }
  int fontColor = strToInt(argv[1]);
  int bgColor = strToInt(argv[2]);
  int cursorColor = strToInt(argv[3]);
  setFontColor(fontColor);
  setBgColor(bgColor);
  setCursorColor(cursorColor);
  repaint();
  return SUCCESS;
}

CommandResult commandSysInfo() {
  printf("screenWidth: %d\n", systemInfo.screenWidth);
  printf("screenHeight: %d\n", systemInfo.screenHeight);
  printf("charWidth: %d\n", systemInfo.charWidth);
  printf("charHeight: %d\n", systemInfo.charHeight);
  printf("layout: %d\n", systemInfo.layout);
  printf("fontSize: %d\n", systemInfo.fontSize);
  printf("charSeparation: %d\n", systemInfo.charSeparation);
  printf("fontCols: %d\n", systemInfo.fontCols);
  printf("fontRows: %d\n", systemInfo.fontRows);
  return SUCCESS;
}

CommandResult commandGetRegisters(int argc, char argv[argc][MAX_ARG_LEN]) {
  if (argc >= 2 && strcmp(argv[1], "--help") == 0) {
    puts("Usage:");
    printf("\t\t%s\n", argv[0]);
    printf(
      "You can save the values of the registers at any time by pressing F1 "
      "and by running this command without this flag it will print the saved "
      "values of the registers.\n"
    );
    return SUCCESS;
  }
  Register * registers;
  sysGetRegisters(registers);
  for(int i = 0; i < REGISTER_QUANTITY; i++){
    printf("%s %x ", registers[i].name, registers[i].value);
    if(i % 3 == 0) // so it has enough space to print all 3 registers in a line
      printf("\n");
  }
  printf("\n");
  printf("For more info add --help to the command\n");
  return SUCCESS;
}

CommandResult commandSnake(int argc, char argv[argc][MAX_ARG_LEN]) {
  if (argc < 3) {
    puts("Usage:");
    printf("\t\t%s <playerCount> <player1Name> [player2Name]\n", argv[0]);
    printf("Where playerCount can be 1 or 2.\n");
    printf("Player 1 moves with wasd, player 2 with ijkl. Other keybinds are:\n");
    printf(" ctrl + r: reset game\n");
    printf(" ctrl + x: lose game\n");
    printf(" ctrl + c: exit game\n");
    return MISSING_ARGUMENTS;
  }
  int playerCount = strToInt(argv[1]);
  if (playerCount != 1 && playerCount != 2) {
    printf("Invalid player count: %s\n", argv[1]);
    return ILLEGAL_ARGUMENT;
  } else if (playerCount == 2 && argc < 4) {
    printf("Player 2 name missing\n");
    return MISSING_ARGUMENTS;
  }
  snake(playerCount > 1, argv[2], argv[3]);
  setShellColor();
  repaint();
  return SUCCESS;
}
CommandResult commandZeroDivisionError(){
  int i = 4/0;
}
