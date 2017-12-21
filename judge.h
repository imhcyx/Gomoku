/*
 * judge.h: Definitions of judgement and ban checking
 *
 */

#include "gomoku.h"

int judge(board_t board, pos *newpos);
int checkban(board_t board, pos *newpos);
