/*
 * judge.h: Definitions of judgement and ban checking
 *
 */

#ifndef JUDGE_H
#define JUDGE_H

#include "gomoku.h"

int judge(board_t board, pos *newpos);
int checkban(board_t board, pos *newpos);

#endif /* JUDGE_H */
