/*
 * judge.h: Definitions of judgement and ban checking
 * 
 * functions in this module are guaranteed to be thread-safe
 *
 */

#ifndef JUDGE_H
#define JUDGE_H

#include "gomoku.h"

/*
 * judge: judge if any player has won
 *
 * Parameters:
 *    board: the chess board
 *    newpos: the position most recently placed on
 *
 * Return value:
 *    the role id of the winner, or -1 if no winner
 */

int judge(board_t board, pos *newpos);

/*
 * checkban: check if a position is banned for black
 *
 * Parameters:
 *    board: the chess board
 *    newpos: the position to be checked
 *
 * Return value:
 *    nonzero if newpos is banned, otherwise 0
 */

int checkban(board_t board, pos *newpos);

#endif /* JUDGE_H */
