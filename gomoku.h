/*
 * gomoku.h: General definitions
 *
 */

/* Chessboard dimensions */

#define BOARD_W 15
#define BOARD_H 15

/* Position structure */

typedef struct {
  int x;
  int y;
} pos;

/* Intersection status */

#define I_FREE 0
#define I_BLACK 1
#define I_WHITE 2

