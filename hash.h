/*
 * hash.h: Definitions of Hash Table
 *
 */

#ifndef HASH_H
#define HASH_H

#include "gomoku.h"

/* compressed board for less storage */
typedef char deflate_t[64];

/* hash value type (64-bit integer) */
typedef uint64_t HASHVALUE;

/* functions for board_t and deflate_t conversion */

/* deflate board_t to deflate_t */
void deflate_board(deflate_t, board_t);
/* inflate deflate_t to board_t */
void inflate_board(board_t, deflate_t);
/* apply difference to deflate_t */
void deflate_delta(deflate_t, int, int, int);

/* hashing functions */

/* calculate hash value by board_t */
HASHVALUE hash_board(board_t);
/* calculate hash value by deflate_t */
HASHVALUE hash_deflate(deflate_t);
/* apply difference and calculate hash value by board_t */
HASHVALUE hash_board_apply_delta(HASHVALUE, board_t, int, int, int, int);
/* apply difference and calculate hash value by deflate_t */
HASHVALUE hash_deflate_apply_delta(HASHVALUE, deflate_t, int, int, int, int);

/* hash table node types */
typedef enum {
  hash_exact,
  hash_alpha,
  hash_beta
} hash_type;

/* hash table functions */

/* initialize hash table */
void hashtable_init();
/* finalize hash table */
void hashtable_fini();
/* store value to hash table (thread-safe) */
void hashtable_store(HASHVALUE, int, hash_type, int);
/* look up hash table (thread-safe) */
int hashtable_lookup(HASHVALUE, int, int, int, int*);

/* get specified piece by deflate_t */
#define PIECE_BY_DEFLATE(def, x, y) \
  ((def)[(x*BOARD_H+j)/4]>>((i*BOARD_H+j)%4)*2&3)

#endif /* HASH_H */
