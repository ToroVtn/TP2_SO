#include <colors.h>
#include <draw.h>
#include <snake.h>
#include <snakeUtils.h>
#include <stdlib.h>
#include <syscalls.h>
#include <sysinfo.h>
#include <time.h>

static bool GAME_OVER, EXIT, MULTIPLAYER, MUTE;
Snake s1 = {0};
Snake s2 = {0};
KeyStruct input;

Grid grid;

void snake(bool multiplayer, char* player1Name, char* player2Name, bool mute) {
  MULTIPLAYER = multiplayer;
  MUTE = mute;
  GAME_OVER = false;
  EXIT = false;
  setGrid();
  int bgColor = 0x262626;
  setBgColor(bgColor);
  setCursorColor(bgColor);
  setStrokeColor(grid.color);
  setFontColor(0xFFFFFF);
  int prevFontSize = systemInfo.fontSize;
  setFontSize(1);
  clearScreen();
  drawGrid();
  setSnake(&s1, grid.cols / 2 - 3, grid.rows / 2, 0x00FF00, player1Name, grid.x0, grid.y0 - systemInfo.charHeight - 10);
  if (MULTIPLAYER) {
    setSnake(
        &s2, grid.cols / 2 + 3, grid.rows / 2, 0x0000FF, player2Name,
        grid.width - (NAME_MAX_LEN + 2 + SCORE_MAX_DIGITS) * (systemInfo.charWidth + systemInfo.charSeparation),
        grid.y0 - systemInfo.charHeight - 10
    );
  }

  appleGen();
  updateScoreBoard();
  while (!EXIT) {
    do {
      sysHalt();
      getKey(&input);
    } while (!specialKeyInput() && !moveInput());
    while (!GAME_OVER && !EXIT && !specialKeyInput()) {
      sleep(55);
      // sysHalt();
      getKey(&input);
      moveInput();
      moveSnake(&s1);
      if (snakeCollision(&s1) || onSnake(&s1, s2.body[0])) {
        gameOver();
      }
      if (MULTIPLAYER) {
        moveSnake(&s2);
        if (snakeCollision(&s2) || onSnake(&s2, s1.body[0])) {
          gameOver();
        }
      }
      if (eaten(&s1) || eaten(&s2)) {
        updateScoreBoard();
      }
    }
  }
  setFontSize(prevFontSize);
  return;
}

void setGrid() {
  grid.cols = 60;
  grid.rows = 40;
  grid.cellSize = 15;
  grid.color = 0x424242;
  grid.borderWidth = 3;
  grid.lineWidth = 1;
  grid.x0 = CHAR_SPACE_LEFT * systemInfo.charWidth + grid.borderWidth;
  grid.y0 = CHAR_SPACE_UP * systemInfo.charHeight + grid.borderWidth + 10;
  grid.width = grid.cols * (grid.cellSize + grid.lineWidth) - grid.lineWidth;
  grid.height = grid.rows * (grid.cellSize + grid.lineWidth) - grid.lineWidth;
}

Point apple = {0};

void drawSnake(Snake* s) {
  setFillColor(s->color);
  for (int i = 0; i < s->len; ++i) {
    fillGridCell(s->body[i].x, s->body[i].y);
  }
}

void eraseSnake(Snake* s) {
  for (int i = 0; i < s->len; ++i) {
    clearGridCell(s->body[i].x, s->body[i].y);
  }
}

void drawGrid() {
  strokeRectangleOutward(grid.x0, grid.y0, grid.width, grid.height, grid.borderWidth);
  for (int i = 1; i < grid.rows; i++) {
    strokeHorizontalLine(grid.x0, grid.y0 + (i - 1) * grid.lineWidth + i * grid.cellSize, grid.width, grid.lineWidth);
  }
  for (int i = 1; i < grid.cols; i++) {
    strokeVerticalLine(grid.x0 + (i - 1) * grid.lineWidth + i * grid.cellSize, grid.y0, grid.height, grid.lineWidth);
  }
}

void fillGridCell(int col, int row) {
  fillRectangle(
      grid.x0 + col * (grid.cellSize + grid.lineWidth), grid.y0 + row * (grid.cellSize + grid.lineWidth), grid.cellSize,
      grid.cellSize
  );
}

void clearGridCell(int col, int row) {
  clearRectangle(
      grid.x0 + col * (grid.cellSize + grid.lineWidth), grid.y0 + row * (grid.cellSize + grid.lineWidth), grid.cellSize,
      grid.cellSize
  );
}

void printSnakeName(Snake* s) {
  s->scoreX = s->nameX;
  s->scoreY = s->nameY;
  printStringXY(s->nameX, s->nameY, s->name, 1, 0);
  s->scoreX += s->nameLen * systemInfo.charWidth + systemInfo.charSeparation;
  printStringXY(s->scoreX, s->scoreY, ": ", 1, 0);
  s->scoreX += 2 * systemInfo.charWidth + systemInfo.charSeparation;
}

void setSnake(Snake* s, int col, int row, uint32_t color, char* name, int nameX, int nameY) {
  s->body[0].x = col;
  s->body[0].y = row;
  s->dirX = 0;
  s->dirY = 0;
  s->len = 1;
  s->color = color;
  s->score = 0;
  s->name = name;
  s->nameLen = strlen(name);
  if (s->nameLen > NAME_MAX_LEN) {
    name[NAME_MAX_LEN] = 0;
    s->nameLen = NAME_MAX_LEN;
  }
  s->nameX = nameX;
  s->nameY = nameY;
  printSnakeName(s);
  drawSnake(s);
}

void growSnake(Snake* s) {
  if (s->len == SNAKE_MAX_LEN) return;
  int l = ++s->len;
  s->body[l - 1].x = s->body[l - 2].x;
  s->body[l - 1].y = s->body[l - 2].y;
  // fillGridCell(s->body[l - 1].x, s->body[l - 1].y);
}

void changeDirections(Snake* snake, char input, char* movKeys) {
  char upKey = movKeys[0];
  char leftKey = movKeys[1];
  char downKey = movKeys[2];
  char rightKey = movKeys[3];

  if ((input == upKey || input == downKey) && (snake->dirY == STOPPED)) {
    snake->dirY = (input == upKey) ? UP : DOWN;
    snake->dirX = STOPPED;
  } else if ((input == leftKey || input == rightKey) && (snake->dirX == STOPPED)) {
    snake->dirY = STOPPED;
    snake->dirX = (input == leftKey) ? LEFT : RIGHT;
  }
}

// Return 1 if special key pressed, 0 otherwise;
bool moveInput() {
  switch (input.character) {
  case 'w':
  case 'a':
  case 's':
  case 'd':
    changeDirections(&s1, input.character, "wasd");
    break;

  case 'i':
  case 'j':
  case 'k':
  case 'l':
    changeDirections(&s2, input.character, "ijkl");
    break;

  default:
    return false;
  }
  input.character = 0;
  return true;
}

bool specialKeyInput() {
  if (input.md.ctrlPressed && input.character != 0) {
    switch (input.character) {
    case 'x':
      gameOver();
      break;

    case 'r':
      reset();
      break;

    case 'c':
      GAME_OVER = true;
      EXIT = true;
      break;

    default:
      return false;
    }
    input.character = 0;
    return true;
  }
  return false;
}

void moveSnake(Snake* s) {
  setFillColor(s->color);
  clearGridCell(s->body[s->len - 1].x, s->body[s->len - 1].y);
  for (int i = s->len - 1; i > 0; i--) {
    s->body[i].x = s->body[i - 1].x;
    s->body[i].y = s->body[i - 1].y;
  }
  s->body[0].x += s->dirX;
  s->body[0].y += s->dirY;
  fillGridCell(s->body[0].x, s->body[0].y);
}

void gameOver() {
  GAME_OVER = true;
  char* message = "Game Over";
  int len = strlen(message);
  int fontSize = 3;
  printStringXY(
      grid.width / 2 - (len / 2) * (systemInfo.charWidth * fontSize + systemInfo.charSeparation), grid.height / 2,
      message, fontSize, 0
  );
  if (!MUTE) sysPlaySound(100, 50);
}

bool pointEquals(Point a, Point b) {
  return a.x == b.x && a.y == b.y;
}

bool snakeCollision(Snake* s) {
  if (s->body[0].x == grid.cols - 1 || s->body[0].x == 0 || s->body[0].y == grid.rows - 1 || s->body[0].y == 0) {
    return true;
  }
  for (int i = s->len; i > 1; i--) {
    if (pointEquals(s->body[0], s->body[i])) {
      return true;
    }
  }
  return false;
}

bool onSnake(Snake* s, Point p) {
  for (int i = 0; i < s->len; i++) {
    if (pointEquals(p, s->body[i])) return true;
  }
  return false;
}

void appleGen() {
  bool regen = false;
  do {
    apple.x = randBetween(1, grid.cols - 2);
    apple.y = randBetween(1, grid.rows - 2);
    for (int i = 0; i < s1.len && !regen; ++i) {
      if (onSnake(&s1, apple) || onSnake(&s2, apple)) {
        apple.x = randBetween(1, grid.cols - 2);
        apple.y = randBetween(1, grid.rows - 2);
      }
    }
  } while (regen);
  setFillColor(0xFF0000);
  fillGridCell(apple.x, apple.y);
}

int eaten(Snake* s) {
  if (pointEquals(apple, s->body[0])) {
    s->score++;
    if (!MUTE) sysPlaySound(600, 50);
    appleGen();
    growSnake(s);
    return 1;
  }
  return 0;
}

void eraseScore(int col, int row, int nameLen) {
  sysMoveCursor(col + nameLen + SCORE_MAX_DIGITS, row);
  for (int i = 0; i < nameLen + SCORE_MAX_DIGITS; ++i) {
    // printf("\b");
  }
  sysMoveCursor(col, row);
}

void printPlayerScore(Snake* s) {
  clearRectangle(
      s->scoreX, s->scoreY, (systemInfo.charWidth + systemInfo.charSeparation) * SCORE_MAX_DIGITS, systemInfo.charHeight
  );
  char score[SCORE_MAX_DIGITS + 1];
  intToBase(s->score, score, 10);
  printStringXY(s->scoreX, s->scoreY, score, 1, 0);
}

void updateScoreBoard() {
  printPlayerScore(&s1);
  if (MULTIPLAYER) printPlayerScore(&s2);
}

void reset() {
  clearScreen();
  drawGrid();
  s1.len = 1;
  s1.body[0].x = grid.cols / 2 - 3;
  s1.body[0].y = grid.rows / 2;
  s1.dirX = STOPPED;
  s1.dirY = STOPPED;
  s1.score = 0;
  drawSnake(&s1);
  printSnakeName(&s1);
  if (MULTIPLAYER) {
    s2.len = 1;
    s2.body[0].x = grid.cols / 2 + 3;
    s2.body[0].y = grid.rows / 2;
    s2.dirX = STOPPED;
    s2.dirY = STOPPED;
    s2.score = 0;
    drawSnake(&s2);
    printSnakeName(&s2);
  }
  updateScoreBoard();
  appleGen();
  GAME_OVER = false;
}