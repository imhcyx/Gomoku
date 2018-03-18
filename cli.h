/*
 * cli.h: Command-Line Interface (CLI) definitions
 *
 */

#ifndef CLI_H
#define CLI_H

#include "gomoku.h"

/*
 * cli_init: initialize CLI
 *
 * Return value:
 *    nonzero for success, otherwise 0
 */

int cli_init();

/*
 * cli_register_player: register a player who uses CLI
 *
 * Parameters:
 *    role: role id of the player
 * 
 * Return value:
 *    nonzero for success, otherwise 0
 */

int cli_register_player(int role);

#endif /* CLI_H */
