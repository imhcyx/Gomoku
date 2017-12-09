/*
 * pai.c: Implementation of Player Abstract Interface
 *
 */

#include "pai.h"

#define CHANGEROLE(x) x = (x == ROLE_BLACK ? ROLE_WHITE : ROLE_BLACK)
#define ROLE2ISTATUS(x) (x == ROLE_BLACK ? I_BLACK : I_WHITE)

static PAI_PLAYER_CALLBACK m_callback[ROLE_MAX];
static void *m_userdata[ROLE_MAX];

static board_t m_board;
/* to avoid access to m_board, define isolated boards for each player */
static board_t m_playerboard[ROLE_MAX];

/* -1 if no winner, otherwise the role number */
static int judge() {
  /* TODO: implement this function in a new module */
  return -1;
}

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
  int role, winner;
  int action;
  pos lastpos, newpos;
  
  /* check callback pointers */
  
  if (!m_callback[ROLE_WHITE] ||
      !m_callback[ROLE_BLACK])
    return -1;
  
  /* initialize stuffs */
  
  memset(m_board, 0, sizeof(m_board));
  running = 1;
  role = ROLE_BLACK;
  action = 0;
  
  /* run the game */
  
  while (running) {
    
    memcpy(&m_playerboard[role], &m_board, sizeof(board_t));
    action = m_callback[role](role, action, &lastpos, &newpos, m_playerboard[role], m_userdata[role]);
    memcpy(&lastpos, &newpos, sizeof(pos));
    
    switch (action) {

    case ACTION_GIVEUP:
      
      /* player giving up, exit the game */
      CHANGEROLE(role);
      winner = role;
      running = 0;
      break;

    case ACTION_PLACE:

      /* check availability */
      
      if (m_board[newpos.x][newpos.y] == I_FREE) {

        /* do placement */
        m_board[newpos.x][newpos.y] = ROLE2ISTATUS(role);

        /* TODO: record step */

      }

      else {

        /* we should not get here */
        fprintf(stderr, "pai: invalid placement on (%d,%d) by %d\n", newpos.x, newpos.y, role);
        break;

      }

      /* TODO: check bans */

      /* judge */
      if ((winner = judge()) >= 0)
        running = 0;

      /* change the role */
      CHANGEROLE(role);
      break;

    case ACTION_UNPLACE:

      /* TODO: do unplacement from step record */

      /* inform opponent without changing the role */
      CHANGEROLE(role);
      memcpy(&m_playerboard[role], &m_board, sizeof(board_t));
      m_callback[role](role, action, 0, 0, m_playerboard[role], m_userdata[role]);
      CHANGEROLE(role);
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
