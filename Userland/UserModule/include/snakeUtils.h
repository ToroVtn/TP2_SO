#ifndef SNAKE_UTILS_H
#define SNAKE_UTILS_H

#include <colors.h>
#include <draw.h>
#include <keyboard.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <syscalls.h>
#include <sysinfo.h>

#define SNAKE_MAX_LEN 50
#define NAME_MAX_LEN 15
#define SCORE_MAX_DIGITS 5
#define CHAR_SPACE_UP 2
#define CHAR_SPACE_LEFT 3
#define ESC 27
#define UP -1
#define DOWN 1
#define LEFT -1
#define RIGHT 1
#define STOPPED 0

// typedef struct bodyT{
//     int x, y, len, score, dirx, diry, tail_x, tail_y;
//     int color;
// }bodyT;

typedef struct {
  int x, y;
} Point;

typedef struct {
  Point body[SNAKE_MAX_LEN];
  int len;
  int score;
  int dirX, dirY;
  int color;
  char* name;
  int nameLen;
  int scoreX, scoreY;
} Snake;

typedef struct {
  int rows;
  int cols;
  int cellSize;
  int color;
  int borderWidth;
  int lineWidth;
  int x0, y0;
  int width, height;
} Grid;

void setGrid();
void drawGrid();
void fillGridCell(int col, int row);
void clearGridCell(int col, int row);

void setSnake(Snake* s, int col, int row, uint32_t color, char* name, int scoreX, int scoreY);
void growSnake(Snake* s);
int moveInput();
int specialKeyInput();
void drawSnake(Snake* s);
void moveSnake(Snake* s);
bool pointEquals(Point a, Point b);
bool snakeCollision(Snake*);
bool onSnake(Point p, Snake* s);
void appleGen();
int eaten(Snake* s);
void updateScoreBoard();
void reset();
void changeDirections(Snake *snake, char input, char *movKeys);
void gameOver();

#endif
