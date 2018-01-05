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

/* usage: dirarr[line][direction][0 for x, 1 for y] */
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

/* patterns for open 4 */
static const char *patopen4[] = {
  "-#****#-"
};

/* patterns for dash 4 */
static const char *patdash4[] = {
  "-**#**-", "-***#*-", "-*#***-", "x****#-", "-#****x"
};

/* patterns for open 3 */
/* Note: patopen3[0] and patopen3[1] are exclusive */
static const char *patopen3[] = {
  "+***#+", "+#***+", "+**#*+", "+*#**+"
};

/* match a specified position on the board with a char pattern (for black only) */
/* return value is nonzero if matched */
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

/* match a line with a pattern (for black only) */
/* initial start should be -5 */
/* if matched, return value is nonzero and *result saves the matched position */
/* to continue the match, set start to previuos *result */
static inline int pat_match(board_t board, pos *newpos, int line, const char *pat, int start, int *result) {
  int i, j;
  int l = strlen(pat);
  pos p;
  /* iterate each start index of the pattern */
  for (i=start; i+l-1<=5; i++, j++)
    /* match each char in the pattern */
    for (j=0;
        char_match(board,
          newpos->x+dirarr[line][0][0]*(i+j),
          newpos->y+dirarr[line][0][1]*(i+j),
          pat[j]);
        j++
        )
      /* j>=l-1, each char is matched */
      if (j>=l-1) {
        /* scan '#' or 'x' */
        for (j=0; j<l; j++) {
          p.x = newpos->x+dirarr[line][0][0]*(i+j);
          p.y = newpos->y+dirarr[line][0][1]*(i+j);
          switch (pat[j]) {
            case '#':
              /* # should not be banned */
              if (checkban(board, &p))
                return 0;
              break;
            case 'x':
              /* x should be barrier or banned */
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

/* count consecutive pieces in a line */
static inline int count_line(board_t board, pos *newpos, int line) {
  int i, n;
  int x, y;
  n = 1;
  /* iterate 2 directions */
  for (i=0; i<2; i++)
    /* iterate each position */
    for (
        x = newpos->x+dirarr[line][i][0],
        y = newpos->y+dirarr[line][i][1];
        VALID_COORD(x, y);
        x += dirarr[line][i][0],
        y += dirarr[line][i][1]
        )
      if (board[x][y] == board[newpos->x][newpos->y])
        n++;
      else
        break;
  return n;
}

/* count open 4 in a line (for black only) */
static inline int count_open_4(board_t board, pos *newpos, int line) {
  int i, start, count;
  count = 0;
  /* match each pattern */
  for (i=0; i<sizeof(patopen4)/sizeof(char*); i++) {
    start = -6;
    /* match until not matched */
    while (pat_match(board, newpos, line, patopen4[i], start+1, &start))
      count++;
  }
  return count;
}

/* count dash 4 in a line (for black only) */
static inline int count_dash_4(board_t board, pos *newpos, int line) {
  int i, start, count;
  count = 0;
  /* match each pattern */
  for (i=0; i<sizeof(patdash4)/sizeof(char*); i++) {
    start = -6;
    /* match until not matched */
    while (pat_match(board, newpos, line, patdash4[i], start+1, &start))
      count++;
  }
  return count;
}

/* count open 3 in a line (for black only) */
static inline int count_open_3(board_t board, pos *newpos, int line) {
  int i, j, result, start, count;
  pos p;
  count = 0;
  /* match each pattern */
  for (i=0; i<sizeof(patopen3)/sizeof(char*); i++) {
    start = -6;
    /* match until not matched */
    while (pat_match(board, newpos, line, patopen3[i], start+1, &start))
      for (j=0; j<strlen(patopen3[i]); j++)
        if (patopen3[i][j] == '#') {
          /* if open 3 pattern is matched, place on # and check if it is open 4 */
          p.x = newpos->x+dirarr[line][0][0]*(start+j);
          p.y = newpos->y+dirarr[line][0][1]*(start+j);
          board[p.x][p.y] = I_BLACK;
          result = count_open_4(board, &p, line);
          board[p.x][p.y] = I_FREE;
          if (result)
            count++;
        }
    /* if patopen[0] is matched, do not match patopen[1] */
    if (i==0 && count)
      i++;
  }
  return count;
}

/* judge if any player has won */
/* newpos indicates the newest placement */
/* return value is the role of winner or -1 if no winner */
int judge(board_t board, pos *newpos) {
  int i, j, n;
  int x, y;
  /* iterate 4 lines  */
  for (i=0; i<4; i++)
    if (board[newpos->x][newpos->y] == I_BLACK &&
        count_line(board, newpos, i) == 5 ||
        board[newpos->x][newpos->y] == I_WHITE &&
        count_line(board, newpos, i) >= 5)
      return board[newpos->x][newpos->y] - 1;
  return -1;
}

/* check if newpos is banned */
/* return value is nonzero if banned */
/* only black may encounter banned position */
/* do not checkban for white */
int checkban(board_t board, pos *newpos) {
  int result;
  int i, lcount;
  int open3count, alive4count;
  result = 0;
  /* occupied position cannot be checkban'ed */
  if (board[newpos->x][newpos->y] != I_FREE) {
    return 0;
  }
  /* tentative placement */
  board[newpos->x][newpos->y] = I_BLACK;
  open3count = 0;
  alive4count = 0;
  for (i=0; i<4; i++) {
    lcount = count_line(board, newpos, i);
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
    /* count open 3 */
    open3count += count_open_3(board, newpos, i);
    /* count alive 4 */
    alive4count += count_open_4(board, newpos, i) + count_dash_4(board, newpos, i);
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
