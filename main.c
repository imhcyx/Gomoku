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
        "    -b<role>\n"
        "    -w<role>\n"
        "        Specify roles for black(b) and white(w) \n"
        "        role can be p (player), c1 (AI 1) or c2 (AI 2)\n",
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
    if (!strcmp(argv[i], "-bp"))
      cli_register_player(ROLE_BLACK);
    else if (!strcmp(argv[i], "-wp"))
      cli_register_player(ROLE_WHITE);
    else if (!strcmp(argv[i], "-bc1"))
      ai_register_player(ROLE_BLACK, 1);
    else if (!strcmp(argv[i], "-wc1"))
      ai_register_player(ROLE_WHITE, 1);
    else if (!strcmp(argv[i], "-bc2"))
      ai_register_player(ROLE_BLACK, 2);
    else if (!strcmp(argv[i], "-wc2"))
      ai_register_player(ROLE_WHITE, 2);
    else {
      fprintf(stderr, "Invalid option: %s\n", argv[i]);
      return 1;
    }
  return pai_start_game()<0;
}
