/*
 * hash.c: Implementation of Hash Table
 *
 */

#include "hash.h"

void deflate_board(deflate_t def, board_t board) {
  int i, j;
  memset(def, 0, 64);
  for (i=0; i<BOARD_W; i++)
    for (j=0; j<BOARD_H; j++)
      def[(i*BOARD_H+j)/4] |= board[i][j]<<((i*BOARD_H+j)%4)*2;
}

void inflate_board(board_t board, deflate_t def) {
  int i, j;
  for (i=0; i<BOARD_W; i++)
    for (j=0; j<BOARD_H; j++)
      board[i][j] = def[(i*BOARD_H+j)/4]>>((i*BOARD_H+j)%4)*2&3;
}


void deflate_delta(deflate_t def, delta *d) {
  int i, j, index;
  char mask1, mask2;
  i = d->newpos.x;
  j = d->newpos.y;
  index = i*BOARD_H+j;
  mask1 = ~(3<<(index%4)*2);
  mask2 = d->piece<<(index%4)*2;
  def[index/4] = def[index/4]&mask1|mask2;
}
