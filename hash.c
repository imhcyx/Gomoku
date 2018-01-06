/*
 * hash.c: Implementation of Hash Table
 *
 */

#include "hash.h"

/* Zobrist's hashing method is used here */
#include "zobrist.h"

/* hash table node definition */
typedef struct _hash_node {
  struct _hash_node *next;
  HASHVALUE hash;
  int depth;
  hash_type type;
  int value;
} hash_node;

/* chains in a hash bin (prime is preferred) */
#define HASHBIN_PRIME 97

/* hash bin definition */
typedef struct {
  /* chains */
  hash_node *node[HASHBIN_PRIME]; /* subscript = hash % HASHBIN_PRIME */
  /* mutex for chains */
  pthread_mutex_t mutex[HASHBIN_PRIME];
  /* performance counter */
  int count[HASHBIN_PRIME]; 
} hash_bin;

/* hash bins */
static hash_bin m_hashbin[256]; /* subscript = high byte of hash */
/* initialized state */
static int m_init = 0;

/* convert board_t to deflate_t */
void deflate_board(deflate_t def, board_t board) {
  int i, j;
  memset(def, 0, 64);
  for (i=0; i<BOARD_W; i++)
    for (j=0; j<BOARD_H; j++)
      def[(i*BOARD_H+j)/4] |= board[i][j]<<((i*BOARD_H+j)%4)*2;
}

/* convert deflate_t to board_t */
void inflate_board(board_t board, deflate_t def) {
  int i, j;
  for (i=0; i<BOARD_W; i++)
    for (j=0; j<BOARD_H; j++)
      board[i][j] = def[(i*BOARD_H+j)/4]>>((i*BOARD_H+j)%4)*2&3;
}

/* hash by board_t */
HASHVALUE hash_board(board_t board) {
  int i, j, piece;
  int index;
  HASHVALUE value = 0;
  /* iterate all points */
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

/* hash by deflate_t */
HASHVALUE hash_deflate(deflate_t def) {
  int i, piece;
  HASHVALUE value = 0;
  /* iterate all points */
  for (i=0; i<BOARD_W*BOARD_H; i++) {
    piece = def[i/4]>>(i%4)*2&3;
    if (piece)
      value ^= zobrist[i*piece];
  }
  return value;
}

/* apply delta and hash by board_t */
HASHVALUE hash_board_apply_delta(HASHVALUE oldvalue, board_t board, int newx, int newy, int piece, int remove) {
  int index;
  index = newx*BOARD_H+newy;
  board[newx][newy] = remove ? I_FREE : piece;
  return oldvalue ^ zobrist[index*piece];
}

/* apply delta and hash by deflate_t */
HASHVALUE hash_deflate_apply_delta(HASHVALUE oldvalue, deflate_t def, int newx, int newy, int piece, int remove) {
  int index;
  char mask1, mask2;
  index = newx*BOARD_H+newy;
  mask1 = ~(3<<(index%4)*2);
  mask2 = remove ? piece<<(index%4)*2 : 0;
  def[index/4] = def[index/4]&mask1|mask2;
  return oldvalue ^ zobrist[index*piece];
}

/* initialize hash table */
void hashtable_init() {
  int i, j;
  /* if not initialized */
  if (!m_init) {
    /* clear structures */
    memset(m_hashbin, 0, sizeof(m_hashbin));
    /* initialize mutexes */
    for (i=0; i<256; i++)
      for (j=0; j<HASHBIN_PRIME; j++)
        pthread_mutex_init(&m_hashbin[i].mutex[j], 0);
    m_init = 1;
  }
}

/* finalize hash table */
void hashtable_fini() {
  int i, j;
  hash_node *node, *next;
  /* return if finalized */
  if (!m_init) return;
  /* iterate all chains in all bins */
  for (i=0; i<256; i++)
    for (j=0; j<HASHBIN_PRIME; j++) {
      /* get root node */
      node = m_hashbin[i].node[j];
#if 0
      /* test only */
      fprintf(stderr, "bin[%d][%d]: %d\n", i, j, m_hashbin[i].count[j]);
#endif
      /* free all nodes */
      while (node) {
        next = node->next;
        free(node);
        node = next;
      }
      /* destroy mutexes */
      pthread_mutex_destroy(&m_hashbin[i].mutex[j]);
    }
  /* clear state */
  m_init = 0;
}

/* store value to hash table */
/* thread-safe */
void hashtable_store(HASHVALUE hash, int depth, hash_type type, int value) {
  int i, j;
  hash_node *node;
  /* calculate bin and chain number */
  i = hash >> (sizeof(HASHVALUE)-1)*8;
  j = hash % HASHBIN_PRIME;
  /* lock chain */
  pthread_mutex_lock(&m_hashbin[i].mutex[j]);
  /* allocate node */
  node = malloc(sizeof(hash_node));
  /* error in malloc */
  if (!node) return;
  /* fill stuffs */
  node->hash = hash;
  node->depth = depth;
  node->type = type;
  node->value = value;
  /* insert as root node */
  node->next = m_hashbin[i].node[j];
  m_hashbin[i].node[j] = node;
  /* update counter */
  m_hashbin[i].count[j]++;
  /* unlock chain */
  pthread_mutex_unlock(&m_hashbin[i].mutex[j]);
}

/* look up value in hash table */
/* thread-safe */
int hashtable_lookup(HASHVALUE hash, int depth, int alpha, int beta, int *pvalue) {
  int i, j;
  int result;
  hash_node *node;
  /* calculate bin and chain number */
  i = hash >> (sizeof(HASHVALUE)-1)*8;
  j = hash % HASHBIN_PRIME;
  /* lock chain */
  pthread_mutex_lock(&m_hashbin[i].mutex[j]);
  /* is empty chain */
  if (!(node = m_hashbin[i].node[j])) {
    result = 0;
    goto _exit;
  }
  /* linear search */
  while (node) {
    if (node->hash == hash &&
        node->depth >= depth) {
      /* limit result by parameters */
      if (node->type == hash_exact ||
          node->type == hash_alpha && node->value <= alpha ||
          node->type == hash_beta && node->value >= beta) {
        /* found */
        *pvalue = node->value;
        result = 1;
        goto _exit;
      }
    }
    node = node->next;
  }
  /* not found */
  result = 0;
_exit:
  /* unlock chain */
  pthread_mutex_unlock(&m_hashbin[i].mutex[j]);
  return result;
}
