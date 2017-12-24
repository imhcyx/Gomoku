/*
 * main.c: The main routine
 *
 */

#include "cli.h"
#include "pai.h"

int main(int argc, const char *argv[]) {
  int winner;
#if 0
  extern void cli_testmode();
  cli_testmode();
  return 0;
#endif
  cli_register_player(ROLE_BLACK);
  cli_register_player(ROLE_WHITE);
  winner = pai_start_game();
  switch (winner) {
  case ROLE_BLACK:
    printf("black win\n");
    break;
  case ROLE_WHITE:
    printf("white win\n");
    break;
  }
  return 0;
}
