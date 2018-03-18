/*
 * pai.c: Implementation of Player Abstract Interface
 *
 */

#include "pai.h"
#include "judge.h"

/* use ftime */
#include <sys/timeb.h>

/* convert role id to piece number */
#define ROLE2ISTATUS(x) (x == ROLE_BLACK ? I_BLACK : I_WHITE)

/* player callbacks */
static PAI_PLAYER_CALLBACK m_callback[ROLE_MAX];
/* player userdata for callbacks */
static void *m_userdata[ROLE_MAX];
/* automatically exit when game ends */
static int m_autoexit[ROLE_MAX];
/* display callback */
static PAI_DISPLAY_CALLBACK m_dcallback;

/* the board maintained by PAI */
static board_t m_board;

/* position record */
#define RECORD_MAX 256
pos record_pos[RECORD_MAX];

/* when move count reaches this value, check banned positions */
#define CHECKFREE_THRESHOLD 200

/* check if all free positions are banned */
static int isallbanned(board_t board) {
  int i, j;
  pos p;
  for (i=0; i<BOARD_W; i++)
    for (j=0; j<BOARD_H; j++) {
      p.x = i;
      p.y = j;
      if (board[i][j] == I_FREE && !checkban(board, &p))
        return 0;
    }
  return 1;
}

int pai_register_player(int role, PAI_PLAYER_CALLBACK callback, void *userdata, int autoexit)
{
  /* check role id */
  if (role < 0 || role >= ROLE_MAX) return 0;
  /* save parameters */
  m_callback[role] = callback;
  m_userdata[role] = userdata;
  m_autoexit[role] = autoexit;
  return 1;
}

int pai_start_game()
{
  
  int running; /* running flag */
  int move; /* move count */
  int role; /* current role if */
  int winner; /* winner, -1=none, 2=draw */
  int action; /* action returned by player callback */
  pos newpos, newest;
  char *msg = 0;
  unsigned long long time1, time2;

  /* check callback pointers */
  
  if (!m_callback[ROLE_WHITE] ||
      !m_callback[ROLE_BLACK]) {
    fprintf(stderr, "Error: incomplete role specification\n");
    return -1;
  }
  
  /* initialize stuffs */
  
  memset(m_board, 0, sizeof(m_board));
  running = 1;
  move = 0;
  role = ROLE_BLACK;
  winner = -1;
  action = 0;
  time1 = pai_time();

  /* run the game */
  
  while (running) {
    
    /* call display callback and calculate time */
    time2 = pai_time();
    if (m_dcallback) m_dcallback(m_board, (move?&newest:0), msg, time2-time1);
    time1 = time2;
    msg = 0;

    /* set role id */
    role = move % 2;

    /* if the player is registered with autoexit, exit when game ends */
    if (winner >= 0 && m_autoexit[role]) break;

    /* call player callback */
    action = m_callback[role](role, action, move, &newpos, m_board, m_userdata[role]);

    switch (action) {

    case ACTION_GIVEUP:
      
      /* player giving up, exit the game */
      if (winner < 0) winner = (move+1)%2;
      running = 0;
      break;

    case ACTION_PLACE:

      /* check if there is already winner */
      if (winner >= 0) {
        msg = "only \"undo\" and \"quit\" are available now";
        break;
      }
      
      /* check availability */
      if (newpos.x < 0 || newpos.x >= BOARD_W ||
          newpos.y < 0 || newpos.y >= BOARD_H) {
        
        msg = "invalid coordinate, retrying";
        break;

      }

      if (m_board[newpos.x][newpos.y] == I_FREE) {

        /* check bans */
        if (role == ROLE_BLACK && checkban(m_board, &newpos)) {
          msg = "position banned, retrying";
          break;
        }

        /* do placement */
        m_board[newpos.x][newpos.y] = ROLE2ISTATUS(role);
        newest = newpos;

        /* record step */
        record_pos[move] = newpos;

      }

      else {

        msg = "position occupied, retrying";
        break;

      }

      /* judge */
      if ((winner = judge(m_board, &newpos)) >= 0) {
        msg = winner ? "white win" : "black win";
      }

#if 0 
      extern int score_board(board_t, int);
      printf("black: %8d\n", score_board(m_board, I_BLACK));
      printf("white: %8d\n", score_board(m_board, I_WHITE));
#endif

      /* increase move count */
      move++;

      /* check if the game ends in a draw */
      if (move > BOARD_W*BOARD_H ||
          (move > CHECKFREE_THRESHOLD && isallbanned(m_board))) {
        winner = 2;
        msg = "end in a draw";
      }

      break;

    case ACTION_UNPLACE:

      /* do unplacement from step record */
      if (move>=2) {
        move--;
        m_board[record_pos[move].x][record_pos[move].y] = I_FREE;
        move--;
        m_board[record_pos[move].x][record_pos[move].y] = I_FREE;
        /* restore newest */
        if (move>0) newest = record_pos[move-1];
        msg = "most recent turn has been undone";
      }
      else {
        msg = "unable to undo";
      }

      /* continue placing */
      action = ACTION_PLACE;

      /* reset winner */
      winner = -1;

      break;

    default:

      /* we should not get here */
      fprintf(stderr, "pai: undefined action %d from role %d\n", action, role);
      break;

    }

  }

  /* send cleanup messages */
  for (role = 0; role < ROLE_MAX; role++)
    m_callback[role](role, ACTION_CLEANUP, 0, 0, 0, 0);

  return winner;

}

int pai_register_display(PAI_DISPLAY_CALLBACK callback) {
  /* simply save the callback */
  m_dcallback = callback;
  return 1;
}

/* get time counter in milliseconds */
unsigned long long pai_time() {
  struct timeb t;
  ftime(&t);
  return 1000 * t.time + t.millitm;
}
