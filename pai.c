/*
 * pai.c: Implementation of Player Abstract Interface
 *
 */

#include "pai.h"

typedef struct {
  pai_player *player;
  int timeout;
  PAI_PLAYER_CALLBACK callback;
} playerinfo;

playerinfo g_player[ROLE_MAX] = {0}; /* ROLE_BLACK, ROLE_WHITE */

int pai_register_player(int role, PAI_PLAYER_CALLBACK callback) {
  if (role <= 0 || role >= ROLE_MAX) return 0;
  g_player[role].callback = callback;
  return 1;
}

int pai_start_game() {
  if (!g_player[ROLE_BLACK].callback ||
      !g_player[ROLE_WHITE].callback)
    return 0;
  /* TODO */
  return 1;
}

int pai_do_step(pos *newpos) {
  /* TODO */
  return 1;
}

void pai_undo_step() {
  /* TODO */
}

