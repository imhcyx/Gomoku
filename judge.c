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

int const dirarr[][2][2] = {
  1,0,-1,0,
  0,1,0,-1,
  1,1,-1,-1,
  1,-1,-1,1
};

#define ITERATE_BY_DIR(vx, vy, newpos,  line, dir) \
  for ( \
    (vx) = (newpos)->x+dirarr[line][dir][0], \
    (vy) = (newpos)->y+dirarr[line][dir][1]; \
    (vx) >= 0 && (vx) < BOARD_W && (vy) >= 0 && (vy) < BOARD_H; \
    (vx) += dirarr[line][dir][0], \
    (vy) += dirarr[line][dir][1] \
    )

#define END2POS(pos, line, dir, end) \
  pos.x = dirarr[line][dir][0] * (end), \
  pos.y = dirarr[line][dir][0] * (end)

/* count pieces in a line */
static int count_line(board_t board, pos *newpos, int line, int allowspaces) {
  int i, n;
  int x, y;
  n = 1;
  /* iterate 2 directions */
  for (i=0; i<2; i++)
    /* iterate each intersection */
    ITERATE_BY_DIR(x, y, newpos, line, i)
      if (board[newpos->x][newpos->y] == board[x][y])
        n++;
      else if (board[newpos->x][newpos->y] != I_FREE || !allowspaces)
        break;
  return n;
}

static int is_open_4(board_t board, pos *newpos, int line) {
  int i, n, x, y, end[2];
  pos p;
  n = 1;
  for (i=0; i<2; i++) {
    end[i] = 0;
    ITERATE_BY_DIR(x, y, newpos, line, i)
      if (board[newpos->x][newpos->y] == board[x][y]) {
        n++;
        end[i]++;
      }
  }
  if (n!=4) return 0;
  END2POS(p, line, 0, end[0]);
  if (board[p.x][p.y] != I_FREE || checkban(board, &p))
    return 0;
  END2POS(p, line, 1, end[1]);
  if (board[p.x][p.y] != I_FREE || checkban(board, &p))
    return 0;
  return 1;
}

static int is_open_3(board_t board, pos *newpos, int line) {}

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
  int open3count, open4count;
  result = 0;
  if (board[newpos->x][newpos->y] != I_FREE) {
    return 0;
  }
  /* tentative placement */
  board[newpos->x][newpos->y] = I_BLACK;
  open3count = 0;
  open4count = 0;
  for (i=1; i<4; i++) {
    lcount = count_line(board, newpos, i, 1);
    /* check long line */
    if (lcount > 5) {
      result = 1;
      goto _exit;
    }
    /* count open 3 */
    if (lcount == 3 && is_open_3(board, newpos, i))
      open3count++;
    /* count open 4 */
    if (lcount == 4 && is_open_4(board, newpos, i))
      open4count++;
  }
  /* check 3-3 & 4-4 */
  if (open3count >= 2 || open4count >= 2) {
    result = 1;
    goto _exit;
  }
_exit:
  /* unplacement */
  board[newpos->x][newpos->y] = I_FREE;
  return result;
}
