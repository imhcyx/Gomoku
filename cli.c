/*
 * cli.c: Implementation of CLI
 *
 */

#include "cli.h"
#include "pai.h"

/* display the board */
static void display(board_t board) {
  int i, j;
  printf("   ┌");
  for (i=0; i<BOARD_W; i++)
    printf("─┬");
  printf("─┐\n");
  for (i=0; i<BOARD_H; i++) {
    printf(" %2d├", BOARD_H - i);
    for (j=0; j<BOARD_W; j++) {
      switch (board[i][j]) {
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

int cli_register_player(int role) {
  
}
