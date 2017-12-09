/*
 * cli.c: Implementation of CLI
 *
 */

#include "cli.h"
#include "pai.h"

#define ROLENAME(x) (x == ROLE_BLACK ? "black" : "white")

static pos newest[2];

/* display the board */
static void cli_display(board_t board) {
  int i, j;
  printf("   ┌");
  for (i=0; i<BOARD_W; i++)
    printf("─┬");
  printf("─┐\n");
  for (i=0; i<BOARD_H; i++) {
    printf(" %2d├", BOARD_H - i);
    for (j=0; j<BOARD_W; j++) {
      switch (board[j][i]) {
      case 1:
        if (newest[0].x == j && newest[0].y == i)
          printf("-▲");
        else
          printf("─●");
        break;
      case 2:
        if (newest[1].x == j && newest[1].y == i)
          printf("-△");
        else
          printf("─○");
        break;
      default:
        printf("─┼");
        break;
      }
    }
    printf("─┤\n");
  }
  printf("   └");
  for (i=0; i<BOARD_W; i++)
    printf("─┴");
  printf("─┘\n");
  printf("    ");
  for (i=0; i<BOARD_W; i++)
    printf(" %c", 'A' + i);
  printf("\n");
}

/* 0 on failure, non-0 on success  */
static int cli_parse_coordinate(char *str, pos *p) {
  int i, j;
  if (strlen(str) == 3) {
    if (isalpha(str[0]) &&
        isdigit(str[1]) &&
        isdigit(str[2])) {
      i = str[0] - (isupper(str[0])?'A':'a');
      j = 10 * (str[1]-'0') + (str[2]-'0');
    }
    else if (isdigit(str[0]) &&
        isdigit(str[1]) &&
        isalpha(str[2])) {
      i = str[2] - (isupper(str[2])?'A':'a');
      j = 10 * (str[0]-'0') + (str[1]-'0');
    }
    else
      return 0;
  }
  else if (strlen(str) == 2) {
    if (isalpha(str[0]) && isdigit(str[1])) {
      i = str[0] - (isupper(str[0])?'A':'a');
      j = str[1] - '0';
    }
    else if (isdigit(str[0]) && isalpha(str[1])) {
      i = str[1] - (isupper(str[1])?'A':'a');
      j = str[0] - '0';
    }
    else
      return 0;
  }
  else
    return 0;
  p->x = i;
  p->y = BOARD_H - j;
  return 1;
}

static int cli_callback(
    int role,
    int action,
    pos *lastpos,
    pos *newpos,
    board_t board,
    void *userdata
    )

{
  int i, result;
  char buf[128];
  char *pmsg = 0;

  if (action == ACTION_NONE || action == ACTION_PLACE) {

    while (1) {

      /* display the board */
      /* system uses fork? making it difficult to debug */
      //system("clear");
      printf("\n");
      cli_display(board);
      if (pmsg)
        printf("%s\n", pmsg);
      else
        printf("\n");

      /* prompt for input */
      printf("%s>", ROLENAME(role));
      fgets(buf, sizeof(buf), stdin);
      buf[strlen(buf)-1] = '\0';

      /* TODO: commands and EOF  */

      /* handle quit  */
      if (!strcmp(buf, "quit")) {
        result = ACTION_GIVEUP;
        break;
      }

      /* try to recognize input as coordinates */
      if (cli_parse_coordinate(buf, newpos)) {

        printf("%d,%d\n", newpos->x, newpos->y);

        /* check validity */
        if (newpos->x < 0 || newpos->x >= BOARD_W ||
            newpos->y < 0 || newpos->y >= BOARD_H) {
          pmsg = "invalid coordinate";
        }
        /* check if occupied*/
        else if (board[newpos->x][newpos->y] == I_FREE) {
          result = ACTION_PLACE;
          newest[role].x = newpos->x;
          newest[role].y = newpos->y;
          break;
        }
        else {
          pmsg = "already occupied";
        }

      }
      else {
        pmsg = "unrecognized command";
      }

    }

  }

  return result;

}

int cli_register_player(int role) {
  return pai_register_player(role, cli_callback, 0);
}
