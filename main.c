/*
 * main.c: The main routine
 *
 */

#include "cli.h"
#include "pai.h"
#include "ai.h"

int main(int argc, const char *argv[]) {
  int winner;
  cli_init();
#if 0
  extern void cli_testmode();
  cli_testmode();
  return 0;
#endif
  cli_register_player(ROLE_BLACK);
  ai_register_player(ROLE_WHITE);
  winner = pai_start_game();
  switch (winner) {
  case ROLE_BLACK:
    printf("winner: black\n");
    break;
  case ROLE_WHITE:
    printf("winner: white\n");
    break;
  }
  return 0;
}
