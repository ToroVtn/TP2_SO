#include "syscalls.h"
#include <arrayUtils.h>
#include <circularHistoryBuffer.h>
#include <shellUtils.h>


extern uint8_t bss;

CircularHistoryBuffer commandHistory;
int32_t currentCommandIdx = 0;

int32_t commandReturnCode = 0;
static int32_t currentPromptLen = 0;

Array currentCommand;

Array commands;

void freeArrayPtr(Array* ele) {
  freeArray(*ele);
}

int32_t compareArgv(Array* argv1, Array* argv2) {
  int32_t argc = arrayLen(*argv1);
  int32_t cmp = argc - arrayLen(*argv2);
  if (cmp == 0) cmp = equalsArray(*argv1, *argv2) == 0;
  return cmp;
}

int32_t shell() {
  setShellColors(0xC0CAF5, 0x1A1B26, 0xFFFF11);
  clearScreen();

  currentCommand = initArray(sizeof(char), 100, NULL, NULL);
  commandHistory = initCHB(sizeof(Array), MAX_HISTORY_LEN, (ElementDestructor)freeArrayPtr, (CompareEleFn)compareArgv);
  commands = initArray(sizeof(ShellCommand), 100, NULL, NULL);

  

  addCommand("help", "List all commands and their descriptions.", commandHelp);
  addCommand("echo", "Print all arguments.", commandEcho);
  addCommand("$?", "Print previous command return code.", commandGetReturnCode);
  addCommand("realTime", "Get current time", commandRealTime);
  addCommand("keyInfo", "Get pressed key info. Exit with ctrl+c.", commandGetKeyInfo);
  addCommand("rand", "Generate random numbers.", commandRand);
  addCommand("layout", "Get or set current layout. \n    Available flags: --help, --list", commandLayout);
  addCommand("setColors", "Set font and background colors.", commandSetColors);
  addCommand("sysInfo", "Get some system information.", commandSysInfo);
  addCommand("getRegisters", "Get the values of the saved registers.\n" "    Available flags: --help",
      commandGetRegisters
  );
  ;
  addCommand("snake", "Play the snake game.\n", commandSnake);
  addCommand("zeroDivisionError", "Test the zero division error.", commandZeroDivisionError);
  addCommand("invalidOpcodeError", "Test the invalid opcode error.", commandInvalidOpcodeError);
  addCommand("ps", "Prints the current running processes.", commandPs);
  
  addCommand("kill", "Kill process by specified pid.", commandKill);
  addCommand("nice", "Change the priority of a process with a specified pid", commandNice);
  addCommand("loop", "Prints its pid every <arg> seconds ", commandLoop);
  
  addCommand("block", "Blocks the specified process", commandBlock);
  addCommand("unblock", "Unblocks the specified process", commandUnBlock);
  addCommand("testPipe", "Test the pipe system with a writer and a reader process.", commandTestPipes);
  addCommand("testPriority", "Test the process priority system, 0 for small wait, 1 for long wait.", commandTestPriority);
  addCommand("testMM", "Test Memory manager.", commandTestMM);
  addCommand("memState", "Get the memory state of the current process or a specified pid.", commandGetMemoryState);

  const char* argv[1] = {"help"};
  sysWaitPid(sysCreateProcess(1, argv, commandHelp));

  newPrompt();

  KeyStruct key;
  while (true) {
    
    if(!getKey(&key)) {
     
      sysHalt();
      continue;
    }
    
    if (key.md.ctrlPressed) {
      switch (toLower(key.character)) {
      case '+':
        incFont();
        break;
      case '-':
        decFont();
        break;
      case 'l':
        clearScreenKeepCommand();
        break;
      case 'w':
        deleteWord();
        break;
      case 'k':
        historyPrev();
        break;
      case 'j':
        historyNext();
        break;
      }
    } else {
      if (key.character == '\n') {
        printChar(key.character);
        commandReturnCode = parseCommand();
        newPrompt();
        emptyArray(currentCommand);
      } else if (key.character == '\b') {
        if (arrayLen(currentCommand) > 0) {
          printChar(key.character);
          popArray(currentCommand);
        }
      } else if (key.character == '\t') {
        autocomplete();
      } else {
        printChar(key.character);
        pushToArray(currentCommand, &key.character);
      }
    }
  }

  return 1;
}

void clearLine() {
  int32_t i = arrayLen(currentCommand);
  while (i--) printChar('\b');
  emptyArray(currentCommand);
}

void historyCopy(Array argv) {
  clearLine();
  int32_t argc = arrayLen(argv);
  for (int32_t i = 0; i < argc; ++i) {
    const char* arg = arrayData(*(Array*)getAtArrayIdx(argv, i));
    for (int32_t i = 0; arg[i] != 0; ++i) {
      printChar(arg[i]);
      pushToArray(currentCommand, arg + i);
    }
    if (i < argc - 1) {
      char c = ' ';
      printChar(c);
      pushToArray(currentCommand, &c);
    }
  }
}
void historyPrev() {
  Array* argv = readPrevFromCHB(commandHistory);
  if (argv == NULL) return;
  historyCopy(*argv);
}
void historyNext() {
  Array* argv = readNextFromCHB(commandHistory);
  if (argv == NULL) {
    clearLine();
    readRestFromCHB(commandHistory);
    return;
  }
  historyCopy(*argv);
}

int32_t getCurrentChar() {
  char* c = getAtArrayIdx(currentCommand, -1);
  if (c == NULL) return 0;
  return *c;
}
void deleteWord() {
  char* wordSeps = " -.:,;";
  char c = getCurrentChar();
  if (c == 0) return;
  if (!strContains(wordSeps, c)) {
    do {
      printChar('\b');
      popArray(currentCommand);
    } while ((c = getCurrentChar()) != 0 && !strContains(wordSeps, c));
  } else {
    do {
      printChar('\b');
      popArray(currentCommand);
    } while ((c = getCurrentChar()) != 0 && strContains(wordSeps, c));
  }
}

void setShellColors(uint32_t fontColor, uint32_t bgColor, uint32_t cursorColor) {
  setFontColor(fontColor);
  setBgColor(bgColor);
  setCursorColor(cursorColor);
}

static const char* const prompt = " > ";
static const char* const errorPrompt = " >! ";
void newPrompt() {
  const char* currentPrompt;
  if (commandReturnCode == 0) {
    currentPrompt = prompt;
    currentPromptLen = 3;
  } else {
    currentPrompt = errorPrompt;
    currentPromptLen = 4;
  }
  (void)printString(currentPrompt);
}

void clearScreenKeepCommand() {
  clearScreen();
  newPrompt();
  const char* cc = arrayData(currentCommand);
  int32_t len = arrayLen(currentCommand);
  for (int32_t i = 0; i < len; ++i) {
    printChar(cc[i]);
  }
}

void incFont() {
  setFontSize(systemInfo.fontSize + 1);
  clearScreenKeepCommand();
}
void decFont() {
  setFontSize(systemInfo.fontSize - 1);
  clearScreenKeepCommand();
}

void addCommand(char* name, char* description, ShellFunction function) {
  ShellCommand newCommand = {.name = name, .description = description, .function = function};
  pushToArray(commands, &newCommand);
}
ShellFunction getCommand(const char* name) {
  for (int32_t i = 0; i < arrayLen(commands); ++i) {
    ShellCommand* command = getAtArrayIdx(commands, i);
    if (strcmp(name, command->name) == 0) return command->function;
  }
  return NULL;
}

void autocomplete() {
  int32_t matchCount = 0, matchIdx = 0, len = 0;
  const char* cc = arrayData(currentCommand);
  int32_t ccLen = arrayLen(currentCommand);
  for (int32_t i = 0; i < arrayLen(commands); ++i) {
    char* command = ((ShellCommand*)getAtArrayIdx(commands, i))->name;
    bool match = true;
    int32_t k = 0;
    for (int32_t j = 0; j < ccLen && command[k] != 0 && match; ++j, ++k) {
      if (cc[j] == ' ') return;
      else if (cc[j] != command[k]) match = false;
    }
    if (match && command[k] != 0) {
      ++matchCount;
      if (matchCount > 1) return;
      matchIdx = i;
      len = k;
    }
  }
  if (matchCount == 0) return;
  char* match = ((ShellCommand*)getAtArrayIdx(commands, matchIdx))->name;
  for (int32_t i = len; match[i] != 0; ++i) {
    printChar(match[i]);
    pushToArray(currentCommand, match + i);
  }
}

ShellFunction verifyCommand(Array argv) {
  Array arg = *(Array*)getAtArrayIdx(argv, 0);
  const char* argv0 = arrayData(arg);
  ShellFunction command = getCommand(argv0);
  if (command == NULL) {
    printf("%s: %s\n", CommandResultStrings[COMMAND_NOT_FOUND], argv0);
    // argv shouldn't be freed because I'm saving invalid commands to history too.
    // freeArray(argv);
    return NULL;
  }
  return command;
}

void setArgsNullTerminaor(Array argv) {
  int32_t argc = arrayLen(argv);
  for (int32_t i = 0; i < argc; ++i) {
    char end = 0;
    pushToArray(*(Array*)getAtArrayIdx(argv, i), &end);
  }
}

void setRealArgv(int32_t argc, const char* realArgv[argc], Array argv) {
  for (int32_t i = 0; i < argc; ++i) {
    realArgv[i] = arrayData(*(Array*)getAtArrayIdx(argv, i));
  }
}

int32_t compareArgs(Array* arg1, Array* arg2) {
  int32_t len = arrayLen(*arg1);
  int32_t cmp = len - arrayLen(*arg2);
  if (cmp == 0) {
    cmp = strcmp(arrayData(*arg1), arrayData(*arg2));
  }
  return cmp;
}

ExitCode parseCommand() {
  Array argv = initArray(sizeof(Array), 10, (ElementDestructor)freeArrayPtr, (CompareEleFn)compareArgs);
  Array argv2 = NULL;
  Array currentArgv = argv;
  ShellFunction command;
  ShellFunction command2 = NULL;
  Array arg = NULL;
  const char* cc = arrayData(currentCommand);
  int32_t commandLength = arrayLen(currentCommand);
  bool newWord = true;
  for (int32_t i = 0; i < commandLength; ++i) {
    if (cc[i] == ' ') newWord = true;
    else {
      if (newWord) {
        if (arg != NULL) {
          if (arrayLen(arg) == 1 && *(char*)getAtArrayIdx(arg, 0) == '!') {
            // No ElementDestructor because it will get concatenated with argv in the end.
            argv2 = initArray(sizeof(Array), 10, NULL, NULL);
            currentArgv = argv2;
          }
        }
        arg = initArray(sizeof(char), 30, NULL, NULL);
        pushToArray(currentArgv, &arg);
        newWord = false;
      }
      pushToArray(arg, cc + i);
    }
  }

  int32_t argc = arrayLen(argv);
  if (argc == 0) {
    freeArray(argv);
    return SUCCESS;
  }

  int32_t ret = SUCCESS;
  if (argv2 != NULL) {
    int32_t argc2 = arrayLen(argv2);
    command2 = verifyCommand(argv2);
    if (command2 == NULL) ret = COMMAND_NOT_FOUND;
    else {
      command = verifyCommand(argv);
      if (command != NULL) {
        setArgsNullTerminaor(argv);
        setArgsNullTerminaor(argv2);
        // Dont take the pipe into account. Can't pop it because I still need it for
        // the commandHistory.
        const char* realArgv[--argc];
        const char* realArgv2[argc2];
        setRealArgv(argc, realArgv, argv);
        setRealArgv(argc2, realArgv2, argv2);
        int32_t pipe = sysPipeInit();
        Pipe pipes = {.write = pipe, .read = STDIN, .err = STDERR};
        int32_t pid = sysCreateProcessWithPipeSwap(argc, realArgv, command, pipes);
        pipes.write = STDOUT;
        pipes.read = pipe;
        int32_t pid2 = sysCreateProcessWithPipeSwap(argc2, realArgv2, command2, pipes);
        sysWaitPid(pid);
        char eof = EOF;
        sysWrite(pipe, &eof, 1);
        ret = sysWaitPid(pid2);
        sysDestroyPipe(pipe);
      } else ret = COMMAND_NOT_FOUND;
    }
    concatArray(argv, argv2);
    freeArray(argv2);
  } else {
    // I need this before verifyCommand because that function uses argv0
    // for error message if command is not found.
    setArgsNullTerminaor(argv);
    command = verifyCommand(argv);
    if (command != NULL) {
      const char* realArgv[argc];
      for (int32_t i = 0; i < argc; ++i) {
        realArgv[i] = arrayData(*(Array*)getAtArrayIdx(argv, i));
      }
      if (strcmp(realArgv[argc - 1], "&") == 0) {
        int32_t pid = sysCreateProcess(argc - 1, realArgv, command);
        printf("Running in background '%s', pid: %d\n", realArgv[0], pid);
        ret = SUCCESS;
      } else {
        int32_t pid = sysCreateProcess(argc, realArgv, command);
        ret = sysWaitPid(pid);
      }
    } else ret = COMMAND_NOT_FOUND;
  }
  moveTofrontOrPushCHB(commandHistory, &argv);
  return ret;
}