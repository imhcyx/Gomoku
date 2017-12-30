/*
 * main.c: The main routine
 *
 */

#include "cli.h"
#include "pai.h"
#include "ai.h"

int main(int argc, const char *argv[]) {
  int i;
  int command = 0; /* 1=play 2=test */
  int black = 0, white = 0; /* 1=player 2=computer */
  cli_init();
  if (argc == 1) {
    printf(
        "Usage: %s <command> [options]\n"
        "Commands: play, test\n"
        "Options:\n"
        "    -b<role>\n"
        "    -w<role>\n"
        "        Specify roles for black and white (p for player, c for computer).\n",
        argv[0]);
    return 0;
  }
  if (!strcmp(argv[1], "play"))
    command = 1;
  else if (!strcmp(argv[1], "test"))
    command = 2;
  else {
    fprintf(stderr, "Invalid command: %s\n", argv[1]);
    return 1;
  }
  for (i=2; i<argc; i++)
    if (!strcmp(argv[i], "-bp"))
      black = 1;
    else if (!strcmp(argv[i], "-bc"))
      black = 2;
    else if (!strcmp(argv[i], "-wp"))
      white = 1;
    else if (!strcmp(argv[i], "-wc"))
      white = 2;
    else {
      fprintf(stderr, "Invalid option: %s\n", argv[i]);
      return 1;
    }
  if (command == 1) {
    if (black == 1)
      cli_register_player(ROLE_BLACK);
    else if (black == 2)
      ai_register_player(ROLE_BLACK);
    else {
      fprintf(stderr, "Role for black not specified\n");
      return 1;
    }
    if (white == 1)
      cli_register_player(ROLE_WHITE);
    else if (white == 2)
      ai_register_player(ROLE_WHITE);
    else {
      fprintf(stderr, "Role for white not specified\n");
      return 1;
    }
    return pai_start_game()<0;
  }
  else if (command == 2) {
    cli_testmode();
  }
  return 0;
}
