/*
 * main.c: The main routine
 *
 */

#include "cli.h"
#include "pai.h"
#include "ai.h"

int main(int argc, const char *argv[]) {
  cli_init();
#if 0
  extern void cli_testmode();
  cli_testmode();
  return 0;
#endif
  if (argc != 3) {
    printf(
        "Usage: %s <role1> <role2>\n"
        "role can be:\n"
        "\tp for player\n"
        "\tc for computer\n",
        argv[0]);
    return 0;
  }
  if (!strcmp(argv[1], "p"))
    cli_register_player(ROLE_BLACK);
  else if (!strcmp(argv[1], "c"))
    ai_register_player(ROLE_BLACK);
  else {
    printf("Invalid option: %s\n", argv[1]);
    return 1;
  }
  if (!strcmp(argv[2], "p"))
    cli_register_player(ROLE_WHITE);
  else if (!strcmp(argv[2], "c"))
    ai_register_player(ROLE_WHITE);
  else {
    printf("Invalid option: %s\n", argv[2]);
    return 1;
  }
  return pai_start_game()<0;
}
