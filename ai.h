/*
 * ai.h: Definitions of AI
 *
 */

#ifndef AI_H
#define AI_H

#include "gomoku.h"
#include "pai.h"
#include "hash.h"
#include "judge.h"

#define SCORE_INF 100000000
/* score for self */
#define SCORE_S1 35
#define SCORE_S2 800
#define SCORE_S3 15000
#define SCORE_S4 800000
#define SCORE_S5 10000000
/* score for opponent */
#define SCORE_O1 20
#define SCORE_O2 500
#define SCORE_O3 4000
#define SCORE_O4 300000
#define SCORE_O5 10000000
/* void */
#define SCORE_VO 7
/* polluted */
#define SCORE_PO 1

int ai_register_player(int role, int aitype);

#endif /* AI_H */
