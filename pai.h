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
#define ACTION_NONE 0
#define ACTION_GIVEUP 1
#define ACTION_PLACE 2
#define ACTION_UNPLACE 3

/* Player callback */

/* The callback function should be defined like this:
 * int callback(int role, int action, pos *lastpos, pos *newpos, board_t *board, void *userdata);
 * role: role of self
 * action: the most recent action
 * lastpos: the position recently affected
 * newpos: the position to be affected
 * board: the board for the player
 *        can be modified arbitrarily
 * userdata: user-defined data in pai_register_player
 *
 * Return value: the action made by the player
 *
 * About actions
 * 
 * ACTION_CLEANUP:
 *  Received on exiting. Cannot be used as return value of callback.
 *  All cleanup work must be done. lastpos, newpos, board are unused.
 *
 * ACTION_NONE:
 *  Received when no previous action is avaliable. Cannot be used as return value of callback.
 *  lastpos is unused.
 *
 * ACTION_GIVEUP:
 *  Callback function should return this when the player gives up.
 *  The opponent will receive this with lastpos unused.
 *
 * ACTION_PLACE:
 *  This action can be received or returned. When received, lastpos indicates previously
 *  position placed on; when returned, newpos must be set to the position to place on.
 *
 * ACTION_UNPLACE:
 *  This action can be received or returned. When received, an unplacement by the opponent
 *  is indicated, with lastpos and newpos unused and return value ignored; when returned,
 *  the recent turn is undone with newpos unused, or it is ignored if no undo can be performed.
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
