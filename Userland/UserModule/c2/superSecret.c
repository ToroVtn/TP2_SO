#include <shellUtils.h>
#include <syscalls.h>

#define a3f 208
#define b3f 233
#define b3 247
#define c4 261
#define c4s 277
#define e4f 311
#define f4 349
#define a4f 415
#define b4f 466
#define b4 493
#define c5 523
#define c5s 554
#define e5f 622
#define f5 698
#define f5s 740
#define a5f 831
#define rest 0

// extern RGBColor frame1[];
// extern RGBColor frame2[];

void commandSuperSecret() {
  int xxx[] = {b4f, b4f, a4f, a4f, f5,  f5,  e5f, b4f, b4f, a4f, a4f, e5f, e5f, c5s, c5,   b4f, c5s, c5s, c5s, c5s,
               c5s, e5f, c5,  b4f, a4f, a4f, a4f, e5f, c5s, b4f, b4f, a4f, a4f, f5,  f5,   e5f, b4f, b4f, a4f, a4f,
               a5f, c5,  c5s, c5,  b4f, c5s, c5s, c5s, c5s, c5s, e5f, c5,  b4f, a4f, rest, a4f, e5f, c5s, rest};
  int yyy[] = {1, 1, 1, 1, 3, 3, 6, 1, 1, 1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 1, 3, 3, 3, 1, 2, 2, 2, 4, 8, 1,
               1, 1, 1, 3, 3, 6, 1, 1, 1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 1, 3, 3, 3, 1, 2, 2, 2, 4, 8, 4};
  int zzz = 100;

  for (int i = 0; i < sizeof(xxx) / sizeof(*xxx); ++i) {
    sysPlaySound(xxx[i], zzz * yyy[i]);
  }

  // sysPrintImage(0, 0, 250, 212, frame1);

  sysExit(SUCCESS);
}