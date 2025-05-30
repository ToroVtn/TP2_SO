

#include <shell.h>
#include <stdlib.h>
#include <syscalls.h>
#include <sysinfo.h>

#include <snake.h>

int main() {
  getSysInfo();

  // shell();

  while (true) {
    char* argv[] = {"shell", "test arg 1", "test arg 2"};
    int pid = sysCreateProcess(sizeof(argv) / sizeof(argv[0]), argv, shell);
    sysWaitPid(pid);
  }

  return 1;
}
