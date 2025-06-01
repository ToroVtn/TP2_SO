/* sampleCodeModule.c */

#include <shell.h>
#include <syscalls.h>
#include <sysinfo.h>

#include <snake.h>

int main() {
  getSysInfo();

  while (true) {
    char* argv[] = {"shell", "test arg 1", "test arg 2"};
    int pid = sysCreateProcess(sizeof(argv) / sizeof(argv[0]), argv, shell);
    sysWaitPid(pid);
  }

  return 1;
}