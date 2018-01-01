/*
 * main.c: The main routine
 *
 */

#include "cli.h"
#include "pai.h"
#include "ai.h"

int main(int argc, const char *argv[]) {
  int i;
  srand(time(0));
  if (argc == 1) {
    printf(
        "Usage: %s <command> [options]\n"
        "Commands: play, test\n"
        "Options:\n"
        "    -p<role>\n"
        "    -c1<role>\n"
        "    -c2<role>\n"
        "        Specify roles for black(b) and white(w) (-p for player, -c for computer).\n"
        "        aitype can be 1 or 2\n",
        argv[0]);
    return 0;
  }
  if (!strcmp(argv[1], "play"))
    cli_init();
  else if (!strcmp(argv[1], "test")) {
    cli_init();
    cli_testmode();
    return 0;
  }
  else {
    fprintf(stderr, "Invalid command: %s\n", argv[1]);
    return 1;
  }
  for (i=2; i<argc; i++)
    if (!strcmp(argv[i], "-pb"))
      cli_register_player(ROLE_BLACK);
    else if (!strcmp(argv[i], "-pw"))
      cli_register_player(ROLE_WHITE);
    else if (!strcmp(argv[i], "-c1b"))
      ai_register_player(ROLE_BLACK, 1);
    else if (!strcmp(argv[i], "-c1w"))
      ai_register_player(ROLE_WHITE, 1);
    else if (!strcmp(argv[i], "-c2b"))
      ai_register_player(ROLE_BLACK, 2);
    else if (!strcmp(argv[i], "-c2w"))
      ai_register_player(ROLE_WHITE, 2);
    else {
      fprintf(stderr, "Invalid option: %s\n", argv[i]);
      return 1;
    }
  return pai_start_game()<0;
}
