/*
 * cli.h: Command-Line Interface (CLI) definitions
 *
 */

#ifndef CLI_H
#define CLI_H

#include "gomoku.h"

int cli_init();
int cli_register_player(int role);
void cli_testmode();

#endif /* CLI_H */
