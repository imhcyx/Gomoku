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
 * 
 * int callback(
 *    int role,
 *    int action,
 *    int move,
 *    pos *newpos,
 *    board_t board,
 *    void *userdata
 *    );
 * 
 * role: role of self
 * action: the most recent action
 * move: the count of moves already made
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
 *  All cleanup work must be done. newpos, board are unused.
 *
 * ACTION_NONE:
 *  Received when no previous action is avaliable. Cannot be used as
 *  return value of callback.
 *
 * ACTION_GIVEUP:
 *  Callback function should return this when the player gives up.
 *  The opponent will receive this.
 *
 * ACTION_PLACE:
 *  This action can be received or returned. When returned, newpos must
 *  be set to the position to place on.
 *
 * ACTION_UNPLACE:
 *  This action can only be returned. The most recent turn is
 *  undone with newpos unused, or it is ignored if no undo can be
 *  performed.
 *
 */

typedef int (*PAI_PLAYER_CALLBACK)(int, int, int, pos*, board_t, void*);

/* Display callback */

/* The callback function should be defined like this:
 *
 * void callback(
 *    board_t board,
 *    pos *newest,
 *    char *msg,
 *    unsigned long long time
 *    );
 *
 * board: the board to be displayed
 * newest: newest position (shown as a triangle)
 * msg: error message
 * time: processing time
 *
 */

typedef void (*PAI_DISPLAY_CALLBACK)(board_t, pos*, char*, unsigned long long);

/* Public functions */

/*
 * pai_register_player: Register a player
 * 
 * role: black or white
 * callback: the callback function
 * userdata: user-defined data to pass to the callback
 * autoexit: automatically exit when losing
 *
 * Return value: nonzero on success, zero on failure
 *
 */

int pai_register_player(int role, PAI_PLAYER_CALLBACK callback, void *userdata, int autoexit);

/*
 * pai_start_game: Start the game
 *
 * Return value: negative on failure, otherwise the winner role
 *
 */

int pai_start_game();

/*
 * pai_register_display: Register the display
 *
 * callback: the callback function
 *
 * Return value: nonzero on success, zero on failure
 *
 */

int pai_register_display(PAI_DISPLAY_CALLBACK callback);

/*
 * pai_time: Get time counter in milliseconds
 *
 * Return value: the time counter
 *
 */

unsigned long long pai_time();

#endif /* PAI_H */
