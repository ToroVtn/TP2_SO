/* sampleCodeModule.c */

#include <shell.h>
#include <stdlib.h>
#include <syscalls.h>
#include <sysinfo.h>

#include <snake.h>

int main() {
  getSysInfo();

  shell();

  return 1;
}
