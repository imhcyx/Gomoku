/*
 * ai.c: Implementation of AI
 *
 */

#include "ai.h"

static const int linearr[][2] = {
  1,0,
  0,1,
  1,1,
  1,-1
};

/* group dimensions */
/* width height xoffset yoffset */
static const int groupdim[][4] = {
  BOARD_W-4,  BOARD_H,    0,  0,
  BOARD_W,    BOARD_H-4,  0,  0,
  BOARD_W-4,  BOARD_H-4,  0,  0,
  BOARD_W-4,  BOARD_H-4,  0,  4
};

#define ALPHABETA_WIDTH 15
#define ALPHABETA_DEPTH 8

static inline int score_by_count(int ns, int no, int neg) {
  if (ns && no)
    return SCORE_PO;
  else if (ns)
    switch (ns) {
      case 1: return SCORE_S1;
      case 2: return SCORE_S2;
      case 3: return SCORE_S3;
      case 4: return SCORE_S4;
      case 5: return SCORE_S5;
    }
  else if (no)
    if (neg)
      switch (no) {
        case 1: return -SCORE_O1;
        case 2: return -SCORE_O2;
        case 3: return -SCORE_O3;
        case 4: return -SCORE_O4;
        case 5: return -SCORE_O5;
      }
    else
      switch (no) {
        case 1: return SCORE_O1;
        case 2: return SCORE_O2;
        case 3: return SCORE_O3;
        case 4: return SCORE_O4;
        case 5: return SCORE_O5;
      }
  else
    return SCORE_VO;
}

int score_board(board_t board, int piece) {
  int score = 0;
  int line;
  int i, x0, y0, x, y, ns, no;
  for (line=0; line<4; line++)
    for (x0=0; x0<groupdim[line][0]; x0++)
      for (y0=0; y0<groupdim[line][1]; y0++) {
        ns = no = 0;
        for (
            i=0, x=x0+groupdim[line][2], y=y0+groupdim[line][3];
            i<5; i++, x+=linearr[line][0], y+=linearr[line][1]
            )
          if (board[x][y] == piece)
            ns++;
          else if (board[x][y] != I_FREE)
            no++;
        score += score_by_count(ns, no, 1);
      }
  return score;
}

int score_deflate(deflate_t def, int piece) {
  board_t board;
  inflate_board(board, def);
  return score_board(board, piece);
}

int score_point(board_t board, int x, int y, int piece) {
  int score = 0;
  int line;
  int i0, j0, k0, i, j, k;
  int k0min, k0max;
  int ns, no;
  if (board[x][y] != I_FREE)
    return -1;
  for (line=0; line<4; line++) {
    for (k0min=0, i=x, j=y;
        k0min>-5 && VALID_COORD(i, j);
        k0min--, i-=linearr[line][0], j-=linearr[line][1]);
    k0min++;
    for (k0max=0, i=x, j=y;
        k0max<5 && VALID_COORD(i, j);
        k0max++, i+=linearr[line][0], j+=linearr[line][1]);
    k0max -= 5;
    i0 = x+linearr[line][0]*k0max;
    j0 = y+linearr[line][1]*k0max;
    for (k0=k0max; k0>=k0min; k0--) {
      ns = no = 0;
      for (k=0, i=i0, j=j0;
          k<5;
          k++, i+=linearr[line][0], j+=linearr[line][1])
        if (board[i][j] == piece)
          ns++;
        else if (board[i][j] != I_FREE)
          no++;
      score += score_by_count(ns, no, 0);
      i0 -= linearr[line][0];
      j0 -= linearr[line][1];
    }
  }
  return score;
}

void score_all_points(int scores[BOARD_W][BOARD_H], board_t board, int piece) {
  int i, j;
  for (i=0; i<BOARD_W; i++)
    for (j=0; j<BOARD_H; j++)
      scores[i][j] = score_point(board, i, j, piece);
}

int find_max_point(int scores[BOARD_W][BOARD_H], board_t board, int role, int *x, int *y) {
  int i, j;
  int score = 0;
  pos p;
  int result = 0;
  for (i=0; i<BOARD_W; i++)
    for (j=0; j<BOARD_H; j++)
      if (scores[i][j]>score) {
        p.x = i;
        p.y = j;
        if (role == ROLE_WHITE || !checkban(board, &p)) {
          result = 1;
          score = scores[i][j];
          *x = i;
          *y = j;
        }
      }
  return result;
}

int find_max_points(int scores[BOARD_W][BOARD_H], board_t board, int role, pos *posarr, int num) {
  int i, j, k;
  int maxscores[64] = {0};
  int score;
  pos p;
  int n = 0;
  for (i=0; i<BOARD_W; i++)
    for (j=0; j<BOARD_H; j++) {
      score = scores[i][j];
      if (score>maxscores[num-1]) {
        p.x = i;
        p.y = j;
        if (role == ROLE_WHITE || !checkban(board, &p)) {
          for (k=num-1; score>maxscores[k-1] && k>0; k--) {
            maxscores[k] = maxscores[k-1];
            posarr[k] = posarr[k-1];
          }
          maxscores[k] = score;
          posarr[k] = p;
          n++;
        }
      }
    }
  if (n>num) n = num;
  return n;
}

int alphabeta(
    HASHVALUE hash,
    int role,
    int depth,
    int alpha,
    int beta,
    board_t board,
    pos *newpos
    )
{
  int i, n, t;
  hash_type type = hash_alpha;
  int scores[BOARD_W][BOARD_H];
  pos maxpos[64];
  i = judge(board, newpos);
  if (i>=0 && i!=role)
    return -SCORE_INF;
  if (depth<=0)
    return score_board(board, role+1);
  if (hashtable_lookup(hash, depth, alpha, beta, &t))
    return t; 
  score_all_points(scores, board, role+1);
  n = find_max_points(scores, board, role, maxpos, ALPHABETA_WIDTH);
  for (i=0; i<n; i++) {
    hash = hash_board_delta(hash, board, maxpos[i].x, maxpos[i].y, role+1, 0);
    t = -alphabeta(hash, role^1, depth-1, -beta, -alpha, board, &maxpos[i]);
    hash = hash_board_delta(hash, board, maxpos[i].x, maxpos[i].y, role+1, 1);
    if (t>alpha) {
      type = hash_exact;
      alpha = t;
    }
    if (alpha>=beta) {
      type = hash_beta;
      break;
    }
  }
  hashtable_store(hash, depth, type, alpha);
  return alpha;
}

int negamax(
    int role,
    int depth,
    board_t board,
    pos *result
    )
{
  HASHVALUE hash;
  int alpha = -SCORE_INF, beta = SCORE_INF;
  int i, n, t;
  int scores[BOARD_W][BOARD_H];
  pos maxpos[64];
  if (depth<=0)
    return score_board(board, role+1);
  hash = hash_board(board);
  score_all_points(scores, board, role+1);
  n = find_max_points(scores, board, role, maxpos, ALPHABETA_WIDTH);
  *result = maxpos[0];
  for (i=0; i<n; i++) {
    hash = hash_board_delta(hash, board, maxpos[i].x, maxpos[i].y, role+1, 0);
    t = -alphabeta(hash, role^1, depth-1, -beta, -alpha, board, &maxpos[i]);
    hash = hash_board_delta(hash, board, maxpos[i].x, maxpos[i].y, role+1, 1);
    printf("score (%d,%d): %d\n", maxpos[i].x, maxpos[i].y, t);
    if (t>alpha) {
      alpha = t;
      *result = maxpos[i];
    }
    if (alpha>=beta)
      break;
  }
  return alpha;
}

static int ai_callback1(
    int role,
    int action,
    int move,
    pos *newpos,
    board_t board,
    void *userdata
    )

{
  int i, n;
  int num;
  int scores[BOARD_W][BOARD_H];
  pos maxpos[64];
  int piece = role+1;
  if (action == ACTION_NONE || action == ACTION_PLACE) {
    switch (move) {
      case 0:
      case 1:
        newpos->x = (BOARD_W-1)/2;
        newpos->y = (BOARD_H-1)/2;
        if (board[newpos->x][newpos->y] != I_FREE) {
          n = 0;
          for (i=0; i<4; i++) {
            maxpos[n].x = (BOARD_W-1)/2+linearr[2+i/2][0]*(i%2?1:-1);
            maxpos[n].y = (BOARD_H-1)/2+linearr[2+i/2][1]*(i%2?1:-1);
            if (board[maxpos[n].x][maxpos[n].y] == I_FREE)
              n++;
          }
          i = rand() % n;
          *newpos = maxpos[i];
        }
        return ACTION_PLACE;
      default:
        score_all_points(scores, board, piece);
        num = find_max_points(scores, board, role, maxpos, 40);
        for (n=0; n<num && scores[maxpos[0].x][maxpos[0].y]==scores[maxpos[n].x][maxpos[n].y]; n++);
        i = rand() % n;
        *newpos = maxpos[i];
        return ACTION_PLACE;
    }
  }
  return 0;
}

static int ai_callback2(
    int role,
    int action,
    int move,
    pos *newpos,
    board_t board,
    void *userdata
    )

{
  int i, n;
  int num;
  int scores[BOARD_W][BOARD_H];
  pos maxpos[64], p;
  int piece = role+1;
  int maxscore = 0;
  if (action == ACTION_NONE || action == ACTION_PLACE) {
    switch (move) {
      case 0:
        newpos->x = (BOARD_W-1)/2;
        newpos->y = (BOARD_H-1)/2;
        return ACTION_PLACE;
      case 1:
        score_all_points(scores, board, piece);
        num = find_max_points(scores, board, role, maxpos, 40);
        for (n=0; n<num && scores[maxpos[0].x][maxpos[0].y]==scores[maxpos[n].x][maxpos[n].y]; n++);
        i = rand() % n;
        *newpos = maxpos[i];
        return ACTION_PLACE;
      default:
        n = negamax(role, ALPHABETA_DEPTH, board, &p);
        //printf("score: %d\n", n);
        *newpos = p;
        return ACTION_PLACE;
    }
  }
  else if (action == ACTION_CLEANUP) {
    hashtable_fini();
  }
  return 0;
}

int ai_register_player(int role, int aitype) {
  switch (aitype) {
    case 1:
      return pai_register_player(role, ai_callback1, 0, 1);
    case 2:
      hashtable_init();
      return pai_register_player(role, ai_callback2, 0, 1);
    default:
      return 0;
  }
}
