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

/* score for self */
#define SCORE_S1 35
#define SCORE_S2 800
#define SCORE_S3 15000
#define SCORE_S4 800000
/* score for opponent */
#define SCORE_O1 15
#define SCORE_O2 400
#define SCORE_O3 1800
#define SCORE_O4 100000
/* void */
#define SCORE_VO 7
/* polluted */
#define SCORE_PO 0

int ai_register_player(int role);

#endif /* AI_H */
