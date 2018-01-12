/*
 * hash.h: Definitions of Hash Table
 *
 */

#ifndef HASH_H
#define HASH_H

#include "gomoku.h"

/* hash value type (64-bit integer) */
typedef uint64_t HASHVALUE;

/* hashing functions */

/* calculate hash value by board_t */
HASHVALUE hash_board(board_t);
/* apply difference and calculate hash value by board_t */
HASHVALUE hash_board_apply_delta(HASHVALUE, board_t, int, int, int, int);

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
void hashtable_store(HASHVALUE, int, int, hash_type, int);
/* look up hash table (thread-safe) */
int hashtable_lookup(HASHVALUE, int, int, int, int, int*);

#endif /* HASH_H */
