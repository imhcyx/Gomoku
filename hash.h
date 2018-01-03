/*
 * hash.h: Definitions of Hash Table
 *
 */

#ifndef HASH_H
#define HASH_H

#include "gomoku.h"

/* compressed board for less storage */
typedef char deflate_t[64];

typedef uint64_t HASHVALUE;

void deflate_board(deflate_t, board_t);
void inflate_board(board_t, deflate_t);
void deflate_delta(deflate_t, int, int, int);

HASHVALUE hash_board(board_t);
HASHVALUE hash_deflate(deflate_t);
HASHVALUE hash_board_delta(HASHVALUE, board_t, int, int, int, int);
HASHVALUE hash_deflate_delta(HASHVALUE, deflate_t, int, int, int, int);

typedef enum {
  hash_exact,
  hash_alpha,
  hash_beta
} hash_type;

void hashtable_init();
void hashtable_fini();
void hashtable_store(HASHVALUE, int, hash_type, int);
int hashtable_lookup(HASHVALUE, int, int, int, int*);

#define PIECE_BY_DEFLATE(def, x, y) \
  ((def)[(x*BOARD_H+j)/4]>>((i*BOARD_H+j)%4)*2&3)

#endif /* HASH_H */
