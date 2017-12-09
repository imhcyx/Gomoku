/*
 * cli.c: Implementation of CLI
 *
 */

#include "cli.h"
#include "pai.h"

#define ROLENAME(x) (x == ROLE_BLACK ? "black" : "white")

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
        printf("─●");
        break;
      case 2:
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

static int cli_callback(
    int role,
    int action,
    pos *lastpos,
    pos *newpos,
    board_t board,
    void *userdata
    )
{
  int i;
  char buf[128];
  if (action == ACTION_NONE || action == ACTION_PLACE) {
    while (1) {
      //system("clear");
      cli_display(board);
      printf("%s>", ROLENAME(role));
      fgets(buf, sizeof(buf), stdin);
      /* TODO: commands and EOF  */
      if (buf[0] >= 'A' && buf[0] < 'A' + BOARD_W && isdigit(buf[1])) {
        i = atoi(&buf[1]);
        if (i > 0 && i <= BOARD_H) {
          newpos->x = buf[0] - 'A';
          newpos->y = BOARD_H - i; 
          //printf("%d,%d\n", newpos->x, newpos->y);
          return ACTION_PLACE;
        }
      }
    }
  }
}

int cli_register_player(int role) {
  return pai_register_player(role, cli_callback, 0);
}
