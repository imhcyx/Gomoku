/*
 * judge.c: Implementation of judgement and ban checking
 *
 */

#include "judge.h"

/*
 * direction specifications
 * lines & directions:
 *                1    1          0
 *                *     *        *
 *   0: 1***0 1:  *  2:  *  3:  *
 *                *       *    *
 *                0        0  1
 *
 *
 */

static const int dirarr[][2][2] = {
  1,0,-1,0,
  0,1,0,-1,
  1,1,-1,-1,
  1,-1,-1,1
};

/*
 * patterns:
 *   *  = piece(black)
 *   +  = free
 *   #  = free & to be placed on (should not be banned)
 *   x  = barrier (white, border, banned)
 *   -  = non piece (free, white, border)
 *
 * open 4: -#****#-
 * dash 4: -**#**-
 *         -***#*-
 *         x****#-
 * open 3: +***#+
 *         +**#*+
 *         (and then become open 4)
 *
 */

#define PAT_IS_PIECE(board, x, y) ( \
    (x) >= 0 && (x) < BOARD_W && \
    (y) >= 0 && (y) < BOARD_H && \
    board[x][y] == I_BLACK)

#define PAT_IS_FREE(board, x, y) ( \
    (x) >= 0 && (x) < BOARD_W && \
    (y) >= 0 && (y) < BOARD_H && \
    board[x][y] == I_FREE)

/* if free, checkban later */
#define PAT_IS_BARRIER(board, x, y) ( \
    (x) < 0 || (x) >= BOARD_W || \
    (y) < 0 || (x) >= BOARD_H || \
    board[x][y] != I_BLACK)

static const char *patopen4[] = {
  "-#****#-"
};

static const char *patdash4[] = {
  "-**#**-", "-***#*-", "-*#***-", "x****#-", "-#****x"
};

static const char *patopen3[] = {
  "+***#+", "+#***+", "+**#*+", "+*#**+"
};

#define ITERATE_BY_DIR(vx, vy, newpos, line, dir) \
  for ( \
    (vx) = (newpos)->x+dirarr[line][dir][0], \
    (vy) = (newpos)->y+dirarr[line][dir][1]; \
    (vx) >= 0 && (vx) < BOARD_W && (vy) >= 0 && (vy) < BOARD_H; \
    (vx) += dirarr[line][dir][0], \
    (vy) += dirarr[line][dir][1] \
    )

#define END2POS(newpos, line, dir, end, pos) ( \
  pos.x = newpos->x+dirarr[line][dir][0]*(end), \
  pos.y = newpos->y+dirarr[line][dir][1]*(end))

#define END2I(board, newpos, line, dir, end) \
  (board \
    [newpos->x+dirarr[line][dir][0]*(end)] \
    [newpos->y+dirarr[line][dir][1]*(end)] \
  )

static inline int char_match(board_t board, int x, int y, char pattern) {
  switch (pattern) {
    case '*':
      if (PAT_IS_PIECE(board, x, y))
        return 1;
      return 0;
    case '+':
    case '#':
      if (PAT_IS_FREE(board, x, y))
        return 1;
      return 0;
    case 'x':
    case '-':
      if (PAT_IS_BARRIER(board, x, y))
        return 1;
      return 0;
  }
  return 0;
}

static int pat_match(board_t board, pos *newpos, int line, const char *pat, int *result) {
  int i, j;
  int l = strlen(pat);
  pos p;
  for (i=-5; i+l-1<=5; i++, j++)
    for (j=0;
        char_match(board,
          newpos->x+dirarr[line][0][0]*(i+j),
          newpos->y+dirarr[line][0][1]*(i+j),
          pat[j]);
        j++
        )
      if (j>=l-1) {
        for (j=0; j<l; j++) {
          p.x = newpos->x+dirarr[line][0][0]*(i+j);
          p.y = newpos->y+dirarr[line][0][1]*(i+j);
          switch (pat[j]) {
            case '#':
              if (checkban(board, &p))
                return 0;
              break;
            case 'x':
              if (p.x>=0 && p.x<BOARD_W &&
                  p.y>=0 && p.y<BOARD_H &&
                  board[p.x][p.y] != I_WHITE &&
                  !checkban(board, &p))
                return 0;
              break;
          }
        }
        if (result) *result = i;
        return 1;
      }
  return 0;
}

/* count pieces in a line */
static inline int count_line(board_t board, pos *newpos, int line, int allowedspaces) {
  int i, n;
  int x, y;
  n = 1;
  /* iterate 2 directions */
  for (i=0; i<2; i++)
    /* iterate each intersection */
    ITERATE_BY_DIR(x, y, newpos, line, i)
      if (board[x][y] == board[newpos->x][newpos->y])
        n++;
      else if (board[x][y] == I_FREE && allowedspaces)
        allowedspaces--;
      else
        break;
  return n;
}

/*static*/ int is_open_4(board_t board, pos *newpos, int line) {
  int i;
  for (i=0; i<sizeof(patopen4)/sizeof(char*); i++)
    if (pat_match(board, newpos, line, patopen4[i], 0))
      return 1;
  return 0;
}

/*static*/ int is_dash_4(board_t board, pos *newpos, int line) {
  int i;
  for (i=0; i<sizeof(patdash4)/sizeof(char*); i++)
    if (pat_match(board, newpos, line, patdash4[i], 0))
      return 1;
  return 0;
}

/*static*/ int is_open_3(board_t board, pos *newpos, int line) {
  int i, j, result;
  pos p;
  for (i=0; i<sizeof(patopen3)/sizeof(char*); i++)
    if (pat_match(board, newpos, line, patopen3[i], &result))
      for (j=0; j<strlen(patopen3[i]); j++)
        if (patopen3[i][j] == '#') {
          p.x = newpos->x+dirarr[line][0][0]*(result+j);
          p.y = newpos->y+dirarr[line][0][1]*(result+j);
          board[p.x][p.y] = I_BLACK;
          result = is_open_4(board, &p, line);
          board[p.x][p.y] = I_FREE;
          if (result)
            return 1;
        }
  return 0;
}

int judge(board_t board, pos *newpos) {
  int i, j, n;
  int x, y;
  /* iterate 4 lines  */
  for (i=0; i<4; i++)
    if (count_line(board, newpos, i, 0) == 5)
      return board[newpos->x][newpos->y] - 1;
  return -1;
}

int checkban(board_t board, pos *newpos) {
  int result;
  int i, lcount;
  int open3count, alive4count;
  result = 0;
  if (board[newpos->x][newpos->y] != I_FREE) {
    return 0;
  }
  /* tentative placement */
  board[newpos->x][newpos->y] = I_BLACK;
  open3count = 0;
  alive4count = 0;
  for (i=0; i<4; i++) {
    lcount = count_line(board, newpos, i, 0);
    /* 5 reached, ban is no longer valid */
    if (lcount == 5) {
      result = 0;
      goto _exit;
    }
    /* check overline */
    if (lcount > 5) {
      result = 1;
      goto _exit;
    }
    lcount = count_line(board, newpos, i, 1);
    /* count open 3 */
    if (lcount == 3 && is_open_3(board, newpos, i))
      open3count++;
    /* count alive 4 */
    if (lcount == 4 && (is_open_4(board, newpos, i) || is_dash_4(board, newpos, i)))
      alive4count++;
  }
  /* check 3-3 & 4-4 */
  if (open3count >= 2 || alive4count >= 2) {
    result = 1;
    goto _exit;
  }
_exit:
  /* unplacement */
  board[newpos->x][newpos->y] = I_FREE;
  return result;
}
