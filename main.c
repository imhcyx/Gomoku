/*
 * main.c: The main routine
 *
 */

#include "cli.h"
#include "pai.h"

int main(int argc, const char *argv[]) {
  cli_register_player(ROLE_BLACK);
  cli_register_player(ROLE_WHITE);
  printf("%d\n", pai_start_game());
  return 0;
}
