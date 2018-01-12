/*
 * ai.c: Implementation of AI
 *
 */

#include "ai.h"

/* use pai_time */
#include "pai.h"

/* debug flag */
#define AI_DEBUG GOMOKU_DEBUG

/*
 * Scoring
 *
 * The board is divided to 572 groups with five points each in a line.
 * Pieces in each group is counted to give corresponding scores.
 *
 */

/* scores and piece counts of a group */
typedef struct {
  /* black & white */
  int score[2]; /* part of board score */
  int scorep[2]; /* part of point score */
  int npiece[2];
} group_score;

/* group status and scores of the board */
typedef struct {
  /* scores of groups in 4 directions */
  group_score vertical[BOARD_W-4][BOARD_H];
  group_score horizontal[BOARD_W][BOARD_H-4];
  group_score backslash[BOARD_W-4][BOARD_H-4];
  group_score slash[BOARD_W-4][BOARD_H-4];
  /* point scores */
  int scores[2][BOARD_W][BOARD_H]; /* black & white */
  /* board scores */
  int totalscore[2]; /* black & white */
} board_score;

/* line directions */
/* usage: linearr[direction][0 for x, 1 for y] */
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

/* parameters for game tree searching with alpha beta cutting */
#define ALPHABETA_WIDTH 20
#define ALPHABETA_DEPTH 8

/* max search depth */
#define MAX_DEPTH 20

/* length of maxpos buffer */
#define MAXPOS_LEN 64

/* number of parallel threads */
#define PARALLEL_THREADS 4

/* max execution time for searching (in milliseconds) */
#define MAX_TIME 14500

/* parameters for threads in negamax_parallel */
typedef struct {
  /* inter-thread shared variables */
  /* when *signaled is nonzero, stop searching */
  int *signaled; /* *signal indicates timeout, read only */
  pos *result; /* *result stores point with max score, synced */
  int *alpha; /* *alpha stores alpha value of root node, synced */
  pthread_mutex_t *mutex; /* mutex for sync, read only */
  /* private variables */
  int move; /* current move count */
  int depth; /* search depth */
  int width; /* search width */
  int role; /* current role id */
  int npos; /* length of maxpos */
  pos maxpos[MAXPOS_LEN]; /* points to be searched */
  int *scores[MAXPOS_LEN]; /* used to return position scores */
  HASHVALUE hash; /* current hash value */
  board_t board; /* current board */
  board_score bs; /* current board scores */
} negamax_param;

/* parameters for signal thread */
typedef struct {
  unsigned long long inittime; /* initial time, read only */
  int *signaled; /* timeout flag, set by signal thread */
  int *exit; /* exit flag, set by main thread */
} signal_param;

/* score by the count of pieces */
/* ns: num of pieces of my side */
/* no: num of pieces of opponent's side */
/* if neg, negative score is given for opponent's pieces */
static inline int score_by_count(int ns, int no, int neg) {
  /* pieces of both sides exist */
  if (ns && no)
    return SCORE_PO;
  /* only pieces of my side */
  else if (ns)
    switch (ns) {
      case 1: return SCORE_S1;
      case 2: return SCORE_S2;
      case 3: return SCORE_S3;
      case 4: return SCORE_S4;
      case 5: return SCORE_S5;
    }
  /* only opponent's pieces */
  else if (no)
    /* give negative score */
    if (neg)
      switch (no) {
        case 1: return -SCORE_O1;
        case 2: return -SCORE_O2;
        case 3: return -SCORE_O3;
        case 4: return -SCORE_O4;
        case 5: return -SCORE_O5;
      }
    /* otherwise */
    else
      switch (no) {
        case 1: return SCORE_O1;
        case 2: return SCORE_O2;
        case 3: return SCORE_O3;
        case 4: return SCORE_O4;
        case 5: return SCORE_O5;
      }
  /* no pieces */
  else
    return SCORE_VO;
  return 0;
}

/* score all groups on board using board_score struct */
/* board: current board */
/* bscore: struct to store scores */
static void score_board_by_struct(board_t board, board_score *bscore) {
  group_score *gs; /* pointer to a group_score field in board_score */
  int sb = 0, sw = 0; /* score of black & white */
  int line; /* line 0~3  */
  int i, x0, y0, x, y, nb, nw; /* iteration variables */
  /* clear fields */
  memset(bscore, 0, sizeof(board_score));
  /* iterate each line */
  for (line=0; line<4; line++)
    /* iterate each group horizontally */
    for (x0=0; x0<groupdim[line][0]; x0++)
      /* iterate each group vertically */
      for (y0=0; y0<groupdim[line][1]; y0++) {
        nb = nw = 0;
        /* iterate each point in the group */
        for (
            i=0, x=x0+groupdim[line][2], y=y0+groupdim[line][3];
            i<5; i++, x+=linearr[line][0], y+=linearr[line][1]
            )
          /* count pieces */
          if (board[x][y] == I_BLACK)
            nb++;
          else if (board[x][y] == I_WHITE)
            nw++;
        /* judge direction */
        switch (line) {
          case 0: gs = &bscore->vertical[x0][y0]; break;
          case 1: gs = &bscore->horizontal[x0][y0]; break;
          case 2: gs = &bscore->backslash[x0][y0]; break;
          case 3: gs = &bscore->slash[x0][y0]; break;
        }
        /* record piece counts */
        gs->npiece[0] = nb;
        gs->npiece[1] = nw;
        /* record scores of board and group scores */
        bscore->totalscore[0] += (gs->score[0] = score_by_count(nb, nw, 1));
        bscore->totalscore[1] += (gs->score[1] = score_by_count(nw, nb, 1));
        /* calculate point scores */
        sb = gs->scorep[0] = score_by_count(nb, nw, 0);
        sw = gs->scorep[1] = score_by_count(nw, nb, 0);
        /* update point scores */
        for (
            i=0, x=x0+groupdim[line][2], y=y0+groupdim[line][3];
            i<5; i++, x+=linearr[line][0], y+=linearr[line][1]
            )
        {
          bscore->scores[0][x][y] += sb;
          bscore->scores[1][x][y] += sw;
        }
      }
}

/* update board_score struct by difference */
/* bscore: current board scores */
/* newpos: new piece */
/* role: role of new piece */
/* remove: place (0) or remove (nonzero) */
static void score_struct_delta(board_score *bscore, pos *newpos, int role, int remove) {
  group_score *gs; /* pointer to a group_score field in board_score */
  int db, dw; /* delta of score of black & white */
  int x0, y0, k, x, y, i; /* iteration variables */
  int line; /* line 0~3 */
  /* iterate each line */
  for (line=0; line<4; line++)
    /* iterate each group containing newpos */
    for (
        x0=newpos->x-groupdim[line][2], y0=newpos->y-groupdim[line][3], k=0;
        k<5; x0-=linearr[line][0], y0-=linearr[line][1], k++
        )
      /* valid groups */
      if (x0>=0 && x0<groupdim[line][0] &&
          y0>=0 && y0<groupdim[line][1])
      {
        /* judge direction */
        switch (line) {
          case 0: gs = &bscore->vertical[x0][y0]; break;
          case 1: gs = &bscore->horizontal[x0][y0]; break;
          case 2: gs = &bscore->backslash[x0][y0]; break;
          case 3: gs = &bscore->slash[x0][y0]; break;
        }
        /* process board scores */
        /* acquire old values */
        db = gs->score[0];
        dw = gs->score[1];
        /* update piece count */
        gs->npiece[role] += remove?-1:1;
        /* calculate differences */
        db = (gs->score[0] = score_by_count(gs->npiece[0], gs->npiece[1], 1)) - db;
        dw = (gs->score[1] = score_by_count(gs->npiece[1], gs->npiece[0], 1)) - dw;
        /* apply differences to total scores */
        bscore->totalscore[0] += db;
        bscore->totalscore[1] += dw;
        /* process point scores */
        /* acquire old values */
        db = gs->scorep[0];
        dw = gs->scorep[1];
        /* calculate differences */
        db = (gs->scorep[0] = score_by_count(gs->npiece[0], gs->npiece[1], 0)) - db;
        dw = (gs->scorep[1] = score_by_count(gs->npiece[1], gs->npiece[0], 0)) - dw;
        /* apply differences to points in the group */
        for (
            i=0, x=x0+groupdim[line][2], y=y0+groupdim[line][3];
            i<5; i++, x+=linearr[line][0], y+=linearr[line][1]
            )
        {
          bscore->scores[0][x][y] += db;
          bscore->scores[1][x][y] += dw;
        }
      }
}

/* find up to num points with highest scores */
/* scores: scores of each points on board */
/* posarr is used to receive up to num points with scores in descending order */
/* return value is the actual number of points received */
static int find_max_points(int scores[BOARD_W][BOARD_H], board_t board, int role, pos *posarr, int num) {
  int i, j, k; /* iteration variables */
  int maxscores[64] = {0}; /* record of max scores */
  int score; /* score of a point */
  pos p; /* position */
  int n = 0; /* record of num of actually obtained points  */
  /* iterate each point */
  for (i=0; i<BOARD_W; i++)
    for (j=0; j<BOARD_H; j++) {
      score = scores[i][j];
      /* if the score is greater than the minimum in record*/
      if (score>maxscores[num-1] && board[i][j] == I_FREE) {
        p.x = i;
        p.y = j;
        /* if the position is not banned, add to list */
        if (role == ROLE_WHITE || !checkban(board, &p)) {
          /* insertion sort */
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
  /* normalize return value */
  if (n>num) n = num;
  return n;
}

/* game tree searching with alpha beta cutting */
static int alphabeta(
    HASHVALUE hash, /* hash value of current board */
    int move, /* current move count */
    int role, /* current role */
    int depth, /* max recursion depth */
    int width, /* max search width */
    int alpha, /* alpha value */
    int beta, /* beta value */
    board_t board, /* current board */
    board_score *bscore,
    pos *newpos, /* newest position */
    int *signaled /* if signaled, stop searching */
    )
{

  int i, n, t;
  /* this is alpha node by default */
  hash_type type = hash_alpha;
  pos maxpos[MAXPOS_LEN];

  /* judge if lose */
  i = judge(board, newpos);
  /* if lose, return negative infinity */
  if (i>=0 && i!=role)
    return -SCORE_INF;

  /* leaf node, return score of the board */
  if (depth<=0)
    return bscore->totalscore[role];

  /* look up hash table */
  /* if node already calculated, return stored value */
  if (hashtable_lookup(hash, move, depth, alpha, beta, &t))
    return t;

  /* find points with highest scores */
  n = find_max_points(bscore->scores[role], board, role, maxpos, width);

  /* search on these n points recursively */
  for (i=0; i<n; i++) {

    /* update scores by difference */
    score_struct_delta(bscore, &maxpos[i], role, 0);

    /* place new piece and calculate hash by difference */
    hash = hash_board_apply_delta(hash, board, maxpos[i].x, maxpos[i].y, role+1, 0);

    do {

      /* PVS search */
      if (i>1 && alpha+1<beta) {
        /* probe with beta = alpha+1 */
        t = -alphabeta(hash, move+1, role^1, depth-1, width, -alpha-1, -alpha, board, bscore, &maxpos[i], signaled);
        if (t<=alpha || t>=beta) {
          /* no need to search further */
          break;
        }
      }
    
      /* recursive search */
      t = -alphabeta(hash, move+1, role^1, depth-1, width, -beta, -alpha, board, bscore, &maxpos[i], signaled);

    } while (0);

    /* remove new piece and calculate hash by difference */
    hash = hash_board_apply_delta(hash, board, maxpos[i].x, maxpos[i].y, role+1, 1);

    /* revert scores */
    score_struct_delta(bscore, &maxpos[i], role, 1);
    
    /* time out, stop searching */
    if (signaled && *signaled)
      return 0;

    /* update alpha */
    if (t>alpha) {
      type = hash_exact;
      alpha = t;
    }

    /* beta cutting */
    if (alpha>=beta) {
      type = hash_beta;
      break;
    }

  }

  /* store value to hash table */
  hashtable_store(hash, move, depth, type, alpha);

  /* return alpha value as score of node */
  return alpha;

}

/* thread routine for negamax_parallel */
static void* negamax_thread_routine(void *parameter) {

  negamax_param *param = parameter;
  int i, t;
  int beta = SCORE_INF;
  HASHVALUE hash = param->hash;

#if AI_DEBUG
  /* print depth for debug */
  fprintf(stderr, "depth: %d\n", param->depth);
#endif

  for (i=0; i<param->npos; i++) {

    /* update scores by difference */
    score_struct_delta(&param->bs, &param->maxpos[i], param->role, 0);

    /* place new piece and calculate hash by difference */
    hash = hash_board_apply_delta(hash, param->board, param->maxpos[i].x, param->maxpos[i].y, param->role+1, 0);

    do {

      /* PVS search */
      if (i>1 && *param->alpha+1<beta) {
        /* probe with beta = alpha+1 */
        t = -alphabeta(hash, param->move+1, param->role^1, param->depth-1, param->width, -*param->alpha-1, -*param->alpha, param->board, &param->bs, &param->maxpos[i], param->signaled);
        if (t<=*param->alpha || t>=beta) {
          /* no need to search further */
          break;
        }
      }
    
      /* recursive search */
      t = -alphabeta(hash, param->move+1, param->role^1, param->depth-1, param->width, -beta, -*param->alpha, param->board, &param->bs, &param->maxpos[i], param->signaled);

    } while (0);

    /* remove new piece and calculate hash by difference */
    hash = hash_board_apply_delta(hash, param->board, param->maxpos[i].x, param->maxpos[i].y, param->role+1, 1);

    /* revert scores */
    score_struct_delta(&param->bs, &param->maxpos[i], param->role, 1);

    /* time out, stop searching */
    if (param->signaled && *param->signaled) {
      *param->scores[i] = -SCORE_INF;
      break;
    }

    /* save score */
    *param->scores[i] = t;

#if AI_DEBUG
    /* print scores for debug */
    fprintf(stderr, "score (%d,%d): %d\n", param->maxpos[i].x, param->maxpos[i].y, t);
#endif

    /* synchronize access to shared variables */
    pthread_mutex_lock(param->mutex);

    /* update alpha */
    if (t>*param->alpha) {
      *param->alpha = t;
      *param->result = param->maxpos[i];
    }

    /* finish synchronization */
    pthread_mutex_unlock(param->mutex);

    /* beta cutting */
    if (*param->alpha>=beta)
      break;

  }

  return 0;

}

/* timing thread */
static void* signal_thread_routine(void *parameter) {
  signal_param *param = parameter;
  unsigned long long inittime = param->inittime;
  while (pai_time() - inittime < MAX_TIME) {
    /* search already finished, stop timing */
    if (*param->exit) return 0;
    usleep(100000);
  }
  /* set signaled state */
  *param->signaled = 1;
  return 0;
}

/* wrapper of alphabeta */
/* find the optimal position using alphabeta (multi-threaded) */
static int negamax_parallel(
    int move, /* current move count */
    int role, /* current role */
    int depth, /* search depth */
    int width, /* search width */
    board_t board, /* current board */
    pos *result /* pointer to receive the optimal position */
    )
{

  HASHVALUE hash;
  board_score bs;
  /* initial alpha and beta values */
  int alpha = -SCORE_INF, beta = SCORE_INF;
  int i, j, k; /* iteration variables */
  int n, t;
  pos maxpos[MAXPOS_LEN]; /* points with max scores */
  int scores[MAXPOS_LEN]; /* max scores */
  pos tmppos; /* temp variable for sorting */
  int tmpscore; /* temp variable for sorting */
  negamax_param param[PARALLEL_THREADS]; /* searching thread parameters */
  pthread_t tid[PARALLEL_THREADS]; /* searching thread ids */
  pthread_mutex_t mutex; /* mutex for sync */
  int signaled = 0; /* timeout flag */
  int exit = 0; /* exit flag */
  signal_param sparam; /* signal thread parameter */
  pthread_t stid; /* signal thread id */
  unsigned long long inittime = pai_time(); /* initial time */

  /* set up signal thread */
  sparam.inittime = inittime;
  sparam.signaled = &signaled;
  sparam.exit = &exit;
  pthread_create(&stid, 0, signal_thread_routine, &sparam);

  /* calculate scores */
  score_board_by_struct(board, &bs);

  /* calculate hash value of the current board */
  hash = hash_board(board);

  /* find points with the highest scores */
  n = find_max_points(bs.scores[role], board, role, maxpos, width);

  /* preset result to current optimal position in case of no result produced by search */
  *result = maxpos[0];

  /* initialize mutex */
  pthread_mutex_init(&mutex, 0);

  /* search until time out or max loop count exceeded or width zeroed */
  while (!signaled && depth<=MAX_DEPTH && width>0) {

    /* initialize parameters */
    for (i=0; i<PARALLEL_THREADS; i++) {
      param[i].signaled = &signaled;
      param[i].result = result;
      param[i].alpha = &alpha;
      param[i].mutex = &mutex;
      param[i].move = move;
      param[i].depth = depth;
      param[i].width = width;
      param[i].role = role;
      param[i].npos = 0;
      param[i].hash = hash;
      memcpy(param[i].board, board, sizeof(board_t));
      memcpy(&param[i].bs, &bs, sizeof(board_score));
    }

    /* assign tasks */
    for (i=0; i<n; i++) {
      t = i % PARALLEL_THREADS;
      param[t].maxpos[param[t].npos] = maxpos[i];
      param[t].scores[param[t].npos] = &scores[i];
      param[t].npos++;
    }

    /* fork */
    for (i=0; i<PARALLEL_THREADS; i++) {
      pthread_create(&tid[i], 0, negamax_thread_routine, &param[i]);
    }

    /* join */
    for (i=0; i<PARALLEL_THREADS; i++) {
      pthread_join(tid[i], 0);
    }

    /* sort points with score descending */
    for (j=1; j<n; j++) {
      if (scores[j]>scores[j-1]) {
        tmpscore = scores[j];
        tmppos = maxpos[j];
        for (k=j; k>0&&tmpscore>scores[k-1]; k--) {
          scores[k] = scores[k-1];
          maxpos[k] = maxpos[k-1];
        }
        scores[k] = tmpscore;
        maxpos[k] = tmppos;
      }
    }

    /* continue search only when 1/5 of max time is remaining */
    if (pai_time()-inittime>MAX_TIME/5)
      break;
 
    /* increase depth & decrease num of points to be searched */
    depth += 2;
    n -= 3;

  }

  /* destroy mutex */
  pthread_mutex_destroy(&mutex);

  /* inform signal thread to exit */
  exit = 1;
  pthread_join(stid, 0);

  /* return alpha value as score of node */
  return alpha;

}

/* callback of AI, using game tree searching */
/* see pai.h for specification */
static int ai_callback(
    int role,
    int action,
    int move,
    pos *newpos,
    board_t board,
    void *userdata
    )

{
  int i, n;
  pos maxpos[64];
  int piece = role+1;
  int maxscore = 0;
  /* okay to place */
  if (action == ACTION_NONE || action == ACTION_PLACE) {
    switch (move) {
      /* first and second move */
      case 0:
      case 1:
        /* attempt to place on the center */
        newpos->x = (BOARD_W-1)/2;
        newpos->y = (BOARD_H-1)/2;
        /* center occupied, place on diagonal neighbor */
        if (board[newpos->x][newpos->y] != I_FREE) {
          n = 0;
          for (i=0; i<4; i++) {
            maxpos[n].x = (BOARD_W-1)/2+linearr[2+i/2][0]*(i%2?1:-1);
            maxpos[n].y = (BOARD_H-1)/2+linearr[2+i/2][1]*(i%2?1:-1);
            if (board[maxpos[n].x][maxpos[n].y] == I_FREE)
              n++;
          }
          /* randomly choose one */
          i = rand() % n;
          *newpos = maxpos[i];
        }
        return ACTION_PLACE;
      default:
        /* call negamax searching function for optimal position */
        negamax_parallel(move, role, ALPHABETA_DEPTH, ALPHABETA_WIDTH, board, newpos);
        return ACTION_PLACE;
    }
  }
  /* cleanup work */
  else if (action == ACTION_CLEANUP) {
    /* finalize hash table */
    hashtable_fini();
  }
  return 0;
}

/* register an AI player */
/* prototype in ai.h */
int ai_register_player(int role, int aitype) {
  hashtable_init();
  return pai_register_player(role, ai_callback, 0, 1);
}
