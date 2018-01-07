/*
 * gomoku.h: General definitions
 *
 */

#ifndef GOMOKU_H
#define GOMOKU_H

#define GOMOKU_DEBUG 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdint.h>

/* usleep */
#include <unistd.h>

/* use POSIX threads for parallel searching */
#include <pthread.h>

/* Chessboard definitions */

/* board dimensions */
#define BOARD_W 15
#define BOARD_H 15

/* check if coordinates are valid */
#define VALID_COORD(x, y) ( \
    (x) >= 0 && (x) < BOARD_W && \
    (y) >= 0 && (y) < BOARD_H )

/* board is a 2-dimension array */
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
