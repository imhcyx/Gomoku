/*
 * pai.c: Implementation of Player Abstract Interface
 *
 */

#include "pai.h"
#include "judge.h"

#define ROLE2ISTATUS(x) (x == ROLE_BLACK ? I_BLACK : I_WHITE)

static PAI_PLAYER_CALLBACK m_callback[ROLE_MAX];
static void *m_userdata[ROLE_MAX];

static PAI_DISPLAY_CALLBACK m_dcallback;

static board_t m_board;

/* position record */
#define RECORD_MAX 256
pos record_pos[RECORD_MAX];

static char *m_msg;

/* -1 if no winner, otherwise the role number */
//static int judge();

int pai_register_player(int role, PAI_PLAYER_CALLBACK callback, void *userdata)
{
  if (role < 0 || role >= ROLE_MAX) return 0;
  m_callback[role] = callback;
  m_userdata[role] = userdata;
  return 1;
}

int pai_start_game()
{
  
  int running;
  int move;
  int role, winner;
  int action;
  pos newpos, newest;
  
  /* check callback pointers */
  
  if (!m_callback[ROLE_WHITE] ||
      !m_callback[ROLE_BLACK])
    return -1;
  
  /* initialize stuffs */
  
  memset(m_board, 0, sizeof(m_board));
  running = 1;
  move = 0;
  role = ROLE_BLACK;
  action = 0;

  /* run the game */
  
  while (1) {
    
    if (m_dcallback) m_dcallback(m_board, &newest, m_msg);
    m_msg = 0;

    if (!running) break;

    role = move % 2;
    action = m_callback[role](role, action, move, &newpos, m_board, m_userdata[role]);

    switch (action) {

    case ACTION_GIVEUP:
      
      /* player giving up, exit the game */
      winner = (move+1)%2;
      running = 0;
      break;

    case ACTION_PLACE:

      /* check availability */
      if (newpos.x < 0 || newpos.x >= BOARD_W ||
          newpos.y < 0 || newpos.y >= BOARD_H) {
        
        m_msg = "invalid coordinate, retrying";
        break;

      }

      if (m_board[newpos.x][newpos.y] == I_FREE) {

        /* check bans */
        if (role == ROLE_BLACK && checkban(m_board, &newpos)) {
          m_msg = "position banned, retrying";
          break;
        }

        /* do placement */
        m_board[newpos.x][newpos.y] = ROLE2ISTATUS(role);
        newest = newpos;

        /* record step */
        record_pos[move] = newpos;

      }

      else {

        m_msg = "position occupied, retrying";
        break;

      }

      /* judge */
      if ((winner = judge(m_board, &newpos)) >= 0) {
        running = 0;
      }

#if 0
    s
      /* for debug */
      extern int count_open_4(board_t, pos*, int);
      extern int count_dash_4(board_t, pos*, int);
      extern int count_open_3(board_t, pos*, int);
      if (role == ROLE_BLACK) {
        printf("count_open_4: %d,%d,%d,%d\n",
           count_open_4(m_board, &newpos, 0),
           count_open_4(m_board, &newpos, 1),
           count_open_4(m_board, &newpos, 2),
           count_open_4(m_board, &newpos, 3)
           );
        printf("count_dash_4: %d,%d,%d,%d\n",
           count_dash_4(m_board, &newpos, 0),
           count_dash_4(m_board, &newpos, 1),
           count_dash_4(m_board, &newpos, 2),
           count_dash_4(m_board, &newpos, 3)
           );
        printf("count_open_3: %d,%d,%d,%d\n",
           count_open_3(m_board, &newpos, 0),
           count_open_3(m_board, &newpos, 1),
           count_open_3(m_board, &newpos, 2),
           count_open_3(m_board, &newpos, 3)
           );
      }
#endif

      /* change the role */
      move++;
      break;

    case ACTION_UNPLACE:

      /* do unplacement from step record */
      if (move>=2) {
        move--;
        m_board[record_pos[move].x][record_pos[move].y] = I_FREE;
        move--;
        m_board[record_pos[move].x][record_pos[move].y] = I_FREE;
        if (move>0) newest = record_pos[move-1];
        m_msg = "mos recent turn has been undone";
      }
      else {
        m_msg = "unable to undo";
      }

      /* continue placing */
      action = ACTION_PLACE;

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
  m_dcallback = callback;
}
