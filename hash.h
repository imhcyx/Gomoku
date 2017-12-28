/*
 * hash.h: Definitions of Hash Table
 *
 */

#ifndef HASH_H
#define HASH_H

#include "gomoku.h"

/* compressed board for less storage */
typedef char deflate_t[64];

/* board delta record for difference calculation */
typedef struct {
  pos newpos;
  int piece; /* I_BLACK or I_WHITE */
} delta;

void deflate_board(deflate_t, board_t);
void inflate_board(board_t, deflate_t);
void deflate_delta(deflate_t, delta*);

#define PIECE_BY_DEFLATE(def, x, y) \
  ((def)[(x*BOARD_H+j)/4]>>((i*BOARD_H+j)%4)*2&3)

#endif /* HASH_H */
