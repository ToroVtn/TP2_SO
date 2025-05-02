#ifndef DRAW_H
#define DRAW_H

void strokeHorizontalLine(int x, int y, int length, int width);
void strokeVerticalLine(int x, int y, int length, int width);
void strokeRectangle(int x, int y, int width, int height, int strokeWidth);
void strokeRectangleOutward(int x, int y, int width, int height, int strokeWidth);
void fillRectangle(int x, int y, int width, int height);
void clearRectangle(int x, int y, int width, int height);
void fillRectangleWithBorder(int x, int y, int width, int height, int strokeWidth);

#endif
