/*
 * pai.h: Player Abstract Interface (PAI) definitions
 *
 */

#ifndef PAI_H
#define PAI_H

#include "gomoku.h"

/*
 * About Player Abstract Interface
 * Participants are abstracted as a player, which
 * can be a user, an AI, or a remote computer.
 * A player can place a piece in a turn and undo it
 * as many times as he wants.
 *
 */

/* Role constants */

#define ROLE_BLACK 0
#define ROLE_WHITE 1
#define ROLE_MAX   2

/* Action constants */

#define ACTION_CLEANUP -1 /* only received on exiting */
#define ACTION_GIVEUP 1
#define ACTION_PLACE 2
#define ACTION_UNPLACE 3

/* Player callback */

/* The callback function should be defined like this:
 * int callback(int role, int action, pos *lastpos, pos *newpos, board_t *board, void *userdata);
 * role: role of self
 * action: the most recent action
           0 if no previous action is available
 * lastpos: the position recently affected
 * newpos: the position to be affected
 * board: the board for the player
 *        can be modified arbitrarily
 * userdata: user-defined data in pai_register_player
 *
 * Return value: the action by the player
 *
 * Note:
 * 1. if the action made by the opponent is ACTION_UNPLACE,
 *    newpos and the return value are ignored.
 * 2. if action is ACTION_CLEANUP, the following 4 arguments are NULL.
 *
 */

typedef int (*PAI_PLAYER_CALLBACK)(int, int, pos*, pos*, board_t*, void*);

/* Public functions */

/*
 * pai_register_player: Register a player
 * 
 * role: black or white
 * callback: the callback function
 * userdata: user-defined data to pass to the callback
 *
 * Return value: nonzero on success, zero on failure
 *
 */

int pai_register_player(int role, PAI_PLAYER_CALLBACK callback, void *userdata);

/*
 * pai_start_game: Start the game
 *
 * Return value: negative on failure, otherwise the winner role
 *
 */

int pai_start_game();

#endif /* PAI_H */