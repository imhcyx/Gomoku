/*
 * hash.c: Implementation of Hash Table
 *
 */

#include "hash.h"
#include "zobrist.h"

typedef struct _hash_node {
  struct _hash_node *next;
  HASHVALUE hash;
  int depth;
  hash_type type;
  int value;
} hash_node;

#define HASHBIN_PRIME 97

typedef struct {
  hash_node *node[HASHBIN_PRIME]; /* subscript = hash % HASHBIN_PRIME */
  int count[HASHBIN_PRIME];
} hash_bin;

/* subscript = high byte of hash */
static hash_bin m_hashbin[256];
static int m_init = 0;

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

HASHVALUE hash_board(board_t board) {
  int i, j, piece;
  int index;
  HASHVALUE value = 0;
  for (i=0; i<BOARD_W; i++) 
    for (j=0; j<BOARD_H; j++) {
      piece = board[i][j];
      if (piece) {
        index = i*BOARD_H+j;
        value ^= zobrist[index*piece];
      }
    }
  return value;
}

HASHVALUE hash_deflate(deflate_t def) {
  int i, piece;
  HASHVALUE value = 0;
  for (i=0; i<BOARD_W*BOARD_H; i++) {
    piece = def[i/4]>>(i%4)*2&3;
    if (piece)
      value ^= zobrist[i*piece];
  }
  return value;
}

HASHVALUE hash_board_delta(HASHVALUE oldvalue, board_t board, int newx, int newy, int piece, int remove) {
  int index;
  index = newx*BOARD_H+newy;
  board[newx][newy] = remove ? I_FREE : piece;
  return oldvalue ^ zobrist[index*piece];
}

HASHVALUE hash_deflate_delta(HASHVALUE oldvalue, deflate_t def, int newx, int newy, int piece, int remove) {
  int index;
  char mask1, mask2;
  index = newx*BOARD_H+newy;
  mask1 = ~(3<<(index%4)*2);
  mask2 = remove ? piece<<(index%4)*2 : 0;
  def[index/4] = def[index/4]&mask1|mask2;
  return oldvalue ^ zobrist[index*piece];
}

void hashtable_init() {
  if (!m_init) {
    memset(m_hashbin, 0, sizeof(m_hashbin));
    m_init = 1;
  }
}

void hashtable_fini() {
  int i, j;
  hash_node *node, *next;
  if (!m_init) return;
  for (i=0; i<256; i++)
    for (j=0; j<HASHBIN_PRIME; j++) {
      node = m_hashbin[i].node[j];
      /* test only */
      //fprintf(stderr, "bin[%d][%d]: %d\n", i, j, m_hashbin[i].count[j]);
      while (node) {
        next = node->next;
        free(node);
        node = next;
      }
    }
  m_init = 0;
}

void hashtable_store(HASHVALUE hash, int depth, hash_type type, int value) {
  int i, j;
  hash_node *node;
  i = hash >> (sizeof(HASHVALUE)-1)*8;
  j = hash % HASHBIN_PRIME;
  /* TODO: handle error for malloc */
  node = malloc(sizeof(hash_node));
  node->hash = hash;
  node->depth = depth;
  node->type = type;
  node->value = value;
  node->next = m_hashbin[i].node[j];
  m_hashbin[i].node[j] = node;
  m_hashbin[i].count[j]++;
}

int hashtable_lookup(HASHVALUE hash, int depth, int alpha, int beta, int *pvalue) {
  int i, j;
  hash_node *node;
  i = hash >> (sizeof(HASHVALUE)-1)*8;
  j = hash % HASHBIN_PRIME;
  if (!(node = m_hashbin[i].node[j]))
    return 0;
  while (node) {
    if (node->hash == hash &&
        node->depth >= depth) {
      if (node->type == hash_exact ||
          node->type == hash_alpha && node->value <= alpha ||
          node->type == hash_beta && node->value >= beta) {
        *pvalue = node->value;
        return 1;
      }
    }
    node = node->next;
  }
  return 0;
}
