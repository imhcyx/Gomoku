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
        "Commands: play\n"
        "Options:\n"
        "    -b<role>\n"
        "    -w<role>\n"
        "        Specify roles for black(b) and white(w) \n"
        "        role can be p (player) or c (computer)\n",
        argv[0]);
    return 0;
  }
  if (!strcmp(argv[1], "play"))
    cli_init();
  else {
    fprintf(stderr, "Invalid command: %s\n", argv[1]);
    return 1;
  }
  for (i=2; i<argc; i++)
    if (!strcmp(argv[i], "-bp"))
      cli_register_player(ROLE_BLACK);
    else if (!strcmp(argv[i], "-wp"))
      cli_register_player(ROLE_WHITE);
    else if (!strcmp(argv[i], "-bc"))
      ai_register_player(ROLE_BLACK, 0);
    else if (!strcmp(argv[i], "-wc"))
      ai_register_player(ROLE_WHITE, 0);
    else {
      fprintf(stderr, "Invalid option: %s\n", argv[i]);
      return 1;
    }
  return pai_start_game()<0;
}
