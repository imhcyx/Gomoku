/*
 * gomoku.h: General definitions
 *
 */

#ifndef GOMOKU_H
#define GOMOKU_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Chessboard definitions */

#define BOARD_W 15
#define BOARD_H 15

typedef char board_t[BOARD_W][BOARD_H];

/* Position structure */

typedef struct {
  int x;
  int y;
} pos;

/* Intersection status */

#define I_FREE 0
#define I_BLACK 1
#define I_WHITE 2

#endif /* GOMOKU_H */