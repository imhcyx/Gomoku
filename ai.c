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

#define MAXPOINTS 40

static inline int score_by_count(int ns, int no, int neg) {
  if (ns && no)
    return SCORE_PO;
  else if (ns)
    switch (ns) {
      case 1: return SCORE_S1;
      case 2: return SCORE_S2;
      case 3: return SCORE_S3;
      case 4: return SCORE_S4;
    }
  else if (no)
    if (neg)
      switch (no) {
        case 1: return -SCORE_O1;
        case 2: return -SCORE_O2;
        case 3: return -SCORE_O3;
        case 4: return -SCORE_O4;
      }
    else
      switch (no) {
        case 1: return SCORE_O1;
        case 2: return SCORE_O2;
        case 3: return SCORE_O3;
        case 4: return SCORE_O4;
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
  int score = -1;
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

int find_max_points(int scores[BOARD_W][BOARD_H], board_t board, int role, pos posarr[MAXPOINTS]) {
  int i, j, k;
  int maxscores[MAXPOINTS] = {0};
  int score;
  pos p;
  int result = 0;
  for (i=0; i<BOARD_W; i++)
    for (j=0; j<BOARD_H; j++) {
      score = scores[i][j];
      if (score>maxscores[MAXPOINTS-1]) {
        p.x = i;
        p.y = j;
        if (role == ROLE_WHITE || !checkban(board, &p)) {
          result = 1;
          for (k=MAXPOINTS-1; score>maxscores[k-1] && k>0; k--) {
            maxscores[k] = maxscores[k-1];
            posarr[k] = posarr[k-1];
          }
          maxscores[k] = score;
          posarr[k] = p;
        }
      }
    }
  return result;
}

static int ai_callback(
    int role,
    int action,
    int move,
    pos *newpos,
    board_t board,
    void *userdata
    )

{
  int scores[BOARD_W][BOARD_H];
  pos maxpos[MAXPOINTS];
  int piece = role+1;
  if (action == ACTION_NONE || action == ACTION_PLACE) {
    switch (move) {
      case 0:
        newpos->x = (BOARD_W-1)/2;
        newpos->y = (BOARD_H-1)/2;
        return ACTION_PLACE;
      default:
        score_all_points(scores, board, piece);
        /* if all groups are polluted, find_max_points will fail */
        if (find_max_points(scores, board, role, maxpos))
          *newpos = maxpos[0];
        else
          find_max_point(scores, board, role, &newpos->x, &newpos->y);
        return ACTION_PLACE;
    }
  }
  return 0;
}

int ai_register_player(int role) {
  return pai_register_player(role, ai_callback, 0, 1);
}