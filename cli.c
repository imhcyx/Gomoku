/*
 * cli.c: Implementation of CLI
 *
 */

#include "cli.h"
#include "pai.h"
#include "hash.h" /* for test mode */

#define ROLENAME(x) (x == ROLE_BLACK ? "black" : "white")

/* display the board */
static void cli_display(board_t board, pos *newest, char *msg, clock_t time) {
  int i, j;
  printf("\n");
  for (i=0; i<BOARD_H; i++) {
    printf(" %2d ", BOARD_H - i);
    for (j=0; j<BOARD_W; j++) {
      switch (board[j][i]) {
      case 1:
        if (newest && newest->x == j && newest->y == i)
          printf("▲");
        else
          printf("●");
        if (j!=BOARD_W-1)
          printf("─");
        break;
      case 2:
        if (newest && newest->x == j && newest->y == i)
          printf("△");
        else
          printf("○");
        if (j!=BOARD_W-1)
          printf("─");
        break;
      default:
        if (j==0) {
          if (i==0)
            printf("┌─");
          else if (i==BOARD_H-1)
            printf("└─");
          else
            printf("├─");
        }
        else if (j==BOARD_W-1) {
          if (i==0)
            printf("┐");
          else if (i==BOARD_H-1)
            printf("┘");
          else
            printf("┤");
        }
        else {
          if (i==0)
            printf("┬─");
          else if (i==BOARD_H-1)
            printf("┴─");
          else
            printf("┼─");
        }
        break;
      }
    }
    printf("\n");
  }
  printf("   ");
  for (i=0; i<BOARD_W; i++)
    printf(" %c", 'A' + i);
  printf("\n");
  if (newest)
    printf("new: %c%d\n", 'A'+newest->x, BOARD_H-newest->y);
  else
    printf("\n");
  if (msg)
    printf("%s\n", msg);
  else if (time)
    printf("processing time: %.3lfs\n", time / 1000000.0);
  else
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
    int move,
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

      if (pmsg)
        printf("%s\n", pmsg);

      /* prompt for input */
      printf("%s>", ROLENAME(role));
      fgets(buf, sizeof(buf), stdin);
      buf[strlen(buf)-1] = '\0';

      /* TODO: commands and EOF  */

#if 1
      if (!strcmp(buf, "s")) {
        int scores[BOARD_W][BOARD_H];
        int x, y;
        extern void score_all_points(int [BOARD_W][BOARD_H], board_t, int);
        score_all_points(scores, board, role+1);
        for (y=0; y<BOARD_H; y++) {
          for (x=0; x<BOARD_W; x++)
            printf("%8d ", scores[x][y]);
          printf("\n");
        }
        continue;
      }
#endif

      /* handle quit  */
      if (!strcmp(buf, "quit") || !strcmp(buf, "q")) {
        result = ACTION_GIVEUP;
        break;
      }

      /* handle undo */
      if (!strcmp(buf, "undo") || !strcmp(buf, "u")) {
        result = ACTION_UNPLACE;
        break;
      }

      /* try to recognize input as coordinates */
      if (cli_parse_coordinate(buf, newpos)) {

        //printf("%d,%d\n", newpos->x, newpos->y);

        result = ACTION_PLACE;
        break;

      }
      else {
        pmsg = "unrecognized command";
      }

    }

  }

  return result;

}

int cli_init() {
  pai_register_display(cli_display);
  return 1;
}

int cli_register_player(int role) {
  return pai_register_player(role, cli_callback, 0, 0);
}

void cli_testmode() {
  board_t board = {0};
  deflate_t deflate;
  pos p;
  char buf[16];
  FILE *file;
  extern int judge(board_t,pos*);
  extern int checkban(board_t,pos*);
  while (1) {
    cli_display(board, 0, 0, 0);
    printf("command>");
    fgets(buf, sizeof(buf), stdin);
    buf[strlen(buf)-1] = '\0';
    switch (buf[0]) {
      case 'b':
        printf("pos:");
        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf)-1] = '\0';
        if (cli_parse_coordinate(buf, &p) &&
            p.x >= 0 && p.x < BOARD_W &&
            p.y >= 0 && p.y < BOARD_H)
          board[p.x][p.y] = I_BLACK;
        else
          printf("invalid\n");
        break;
      case 'w':
        printf("pos:");
        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf)-1] = '\0';
        if (cli_parse_coordinate(buf, &p) &&
            p.x >= 0 && p.x < BOARD_W &&
            p.y >= 0 && p.y < BOARD_H)
          board[p.x][p.y] = I_WHITE;
        else
          printf("invalid\n");
        break;
      case 'c':
        printf("pos:");
        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf)-1] = '\0';
        if (cli_parse_coordinate(buf, &p) &&
            p.x >= 0 && p.x < BOARD_W &&
            p.y >= 0 && p.y < BOARD_H)
          board[p.x][p.y] = I_FREE;
        else
          printf("invalid\n");
        break;
      case 'j':
        printf("pos:");
        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf)-1] = '\0';
        if (cli_parse_coordinate(buf, &p) &&
            p.x >= 0 && p.x < BOARD_W &&
            p.y >= 0 && p.y < BOARD_H)
          printf("judge: %d\n", judge(board, &p));
        else
          printf("invalid\n");
        break;
      case 'f': 
        printf("pos:");
        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf)-1] = '\0';
        if (cli_parse_coordinate(buf, &p) &&
            p.x >= 0 && p.x < BOARD_W &&
            p.y >= 0 && p.y < BOARD_H)
          printf("checkban: %d\n", checkban(board, &p));
        else
          printf("invalid\n");
        break;
      case 's':
        printf("filename:");
        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf)-1] = '\0';
        file = fopen(buf, "wb");
        if (!file) {
          printf("failed\n");
          break;
        }
        deflate_board(deflate, board);
        fwrite(deflate, 1, sizeof(deflate), file);
        fclose(file);
        break;
      case 'l':
        printf("filename:");
        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf)-1] = '\0';
        file = fopen(buf, "rb");
        if (!file) {
          printf("failed\n");
          break;
        }
        fread(deflate, 1, sizeof(deflate), file);
        fclose(file);
        inflate_board(board, deflate);
        break;
      case 'q':
        return;
    }
  }
}
