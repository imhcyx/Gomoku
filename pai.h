/*
 * pai.h: Player Abstract Interface (PAI) definitions
 *
 */

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

/* Player structure */

typedef struct {

  /* Black or white */
  int role;

  /* Maximum execution time for the callback */
  /* (in milliseconds). 0 means no limits. */
  int timeout;

  /* User-defined data for the callback */
  void *userdata;

} pai_player;

/* Player callback */

/* The callback function should be defined like this:
 * void callback(pai_player *player, pos *newpos, void *userdata);
 * player: the player structure
 * newpos: the intersection most recently placed on
 *         by the opponent
 * userdata: user-defined data in pai_register_player
 *
 * Note: if no placement is done in a callback, the player
 *       is considered to give up this game.
 *
 */

typedef void (*PAI_PLAYER_CALLBACK)(pai_player*, pos*, void*);

/* Public functions */

/*
 * pai_register_player: Register a player
 * 
 * role: black or white
 * callback: the callback function
 *
 * Return value: nonzero on success, zero on failure
 *
 */

int pai_register_player(int role, PAI_PLAYER_CALLBACK callback);

/* TODO: set timeout */

/*
 * pai_start_game: Start the game
 *
 * Return value: nonzero on success, zero on failure
 *
 */

int pai_start_game();

/*
 * pai_do_step: Place a piece on specified position
 * (Can only be called in the callback)
 * Return value: nonzero on success, zero on failure
 *               (Typically on multiple placement in
 *               a turn)
 *
 */

int pai_do_step(pos *newpos);

/*
 * pai_undo_step: Undo a placement
 * (Can only be called in the callback)
 */

void pai_undo_step();

