/*
 * cli.c: Implementation of CLI
 *
 */

#include "cli.h"
#include "pai.h"
#include "hash.h" /* for test mode */

#define ROLENAME(x) (x == ROLE_BLACK ? "black" : "white")

/* display the board */
static void cli_display(board_t board, pos *newest, char *msg, unsigned long long time) {
  int i, j;
#if !GOMOKU_DEBUG
  system("clear");
#endif
  printf("\n");
  /* iterate each row */
  for (i=0; i<BOARD_H; i++) {
    /* print row numbers */
    printf(" %2d ", BOARD_H - i);
    /* iterate each column */
    for (j=0; j<BOARD_W; j++) {
      switch (board[j][i]) {
      case 1: /* black */
        /* use triangle for the newest piece */
        if (newest && newest->x == j && newest->y == i)
          printf("▲");
        else
          printf("●");
        /* use - to align */
        if (j!=BOARD_W-1)
          printf("─");
        break;
      case 2:
        /* use triangle for the newest piece */
        if (newest && newest->x == j && newest->y == i)
          printf("△");
        else
          printf("○");
        /* use - to align */
        if (j!=BOARD_W-1)
          printf("─");
        break;
      default:
        /* draw borders */
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
  /* print column identifiers */
  printf("   ");
  for (i=0; i<BOARD_W; i++)
    printf(" %c", 'A' + i);
  printf("\n");
  /* print the newest position */
  if (newest)
    printf("new: %c%d\n", 'A'+newest->x, BOARD_H-newest->y);
  else
    printf("\n");
  /* print error message if any */
  /* otherwise print processing time of AI */
  if (msg)
    printf("%s\n", msg);
  else
    printf("\n");
  printf("elapsed time: %.3lfs\n", time / 1000.0);
}

/* 
 * parse user-input coordinates
 * valid coordinates:
 * A1 a1 1A 1a (2 chars)
 * A10 a10 10A 10a (3 chars)
 */

/* 0 on failure, non-0 on success  */
static int cli_parse_coordinate(char *str, pos *p) {
  int i, j;
  /* 3 chars */
  if (strlen(str) == 3) {
    /* ADD pattern */
    if (isalpha(str[0]) &&
        isdigit(str[1]) &&
        isdigit(str[2])) {
      i = str[0] - (isupper(str[0])?'A':'a');
      j = 10 * (str[1]-'0') + (str[2]-'0');
    }
    /* DDA pattern */
    else if (isdigit(str[0]) &&
        isdigit(str[1]) &&
        isalpha(str[2])) {
      i = str[2] - (isupper(str[2])?'A':'a');
      j = 10 * (str[0]-'0') + (str[1]-'0');
    }
    else
      return 0;
  }
  /* 2 chars */
  else if (strlen(str) == 2) {
    /* AD pattern */
    if (isalpha(str[0]) && isdigit(str[1])) {
      i = str[0] - (isupper(str[0])?'A':'a');
      j = str[1] - '0';
    }
    /* DA pattern */
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

/* callback for PAI interface */
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

      /* print error message if any */
      if (pmsg)
        printf("%s\n", pmsg);

      /* prompt for input */
      printf("%s>", ROLENAME(role));
      fgets(buf, sizeof(buf), stdin);
      buf[strlen(buf)-1] = '\0';

#if 0
      /* (debug) command "s": print scores for all positions */
      if (!strcmp(buf, "s")) {
        int scores[BOARD_W][BOARD_H];
        int x, y;
        extern void score_all_points(int [BOARD_W][BOARD_H], board_t, int);
        /* score all points */
        score_all_points(scores, board, role+1);
        /* print all scores in a matrix */
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

        result = ACTION_PLACE;
        break;

      }
      else {
        /* unrecognized command */
        pmsg = "unrecognized command";
      }

    }

  }

  return result;

}

int cli_init() {
  /* register display callback */
  pai_register_display(cli_display);
  return 1;
}

int cli_register_player(int role) {
  /* register role to use CLI */
  return pai_register_player(role, cli_callback, 0, 0);
}

/* run test mode */
void cli_testmode() {
  board_t board = {0};
  deflate_t deflate;
  pos p;
  char buf[16];
  FILE *file;
  extern int judge(board_t,pos*);
  extern int checkban(board_t,pos*);
  while (1) {
    /* display the board */
    cli_display(board, 0, 0, 0);
    printf("command>");
    /* read command */
    fgets(buf, sizeof(buf), stdin);
    buf[strlen(buf)-1] = '\0';
    switch (buf[0]) {
      /* place black piece */
      case 'b':
        printf("pos:");
        /* read coordinate */
        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf)-1] = '\0';
        /* parse and check */
        if (cli_parse_coordinate(buf, &p) &&
            p.x >= 0 && p.x < BOARD_W &&
            p.y >= 0 && p.y < BOARD_H)
          board[p.x][p.y] = I_BLACK;
        else
          printf("invalid\n");
        break;
      /* place white piece */
      case 'w':
        printf("pos:");
        /* read coordinate */
        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf)-1] = '\0';
        if (cli_parse_coordinate(buf, &p) &&
        /* parse and check */
            p.x >= 0 && p.x < BOARD_W &&
            p.y >= 0 && p.y < BOARD_H)
          board[p.x][p.y] = I_WHITE;
        else
          printf("invalid\n");
        break;
      /* clear piece */
      case 'c':
        printf("pos:");
        /* read coordinate */
        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf)-1] = '\0';
        if (cli_parse_coordinate(buf, &p) &&
        /* parse and check */
            p.x >= 0 && p.x < BOARD_W &&
            p.y >= 0 && p.y < BOARD_H)
          board[p.x][p.y] = I_FREE;
        else
          printf("invalid\n");
        break;
      /* judge */
      case 'j':
        printf("pos:");
        fgets(buf, sizeof(buf), stdin);
        /* read coordinate */
        buf[strlen(buf)-1] = '\0';
        if (cli_parse_coordinate(buf, &p) &&
        /* parse and check */
            p.x >= 0 && p.x < BOARD_W &&
            p.y >= 0 && p.y < BOARD_H)
          printf("judge: %d\n", judge(board, &p));
        else
          printf("invalid\n");
        break;
      /* checkban */
      case 'f': 
        printf("pos:");
        /* read coordinate */
        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf)-1] = '\0';
        if (cli_parse_coordinate(buf, &p) &&
        /* parse and check */
            p.x >= 0 && p.x < BOARD_W &&
            p.y >= 0 && p.y < BOARD_H)
          printf("checkban: %d\n", checkban(board, &p));
        else
          printf("invalid\n");
        break;
      /* save board to file */
      case 's':
        printf("filename:");
        /* read filename */
        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf)-1] = '\0';
        /* open file */
        file = fopen(buf, "wb");
        if (!file) {
          printf("failed\n");
          break;
        }
        /* deflate the board */
        deflate_board(deflate, board);
        /* write to file */
        fwrite(deflate, 1, sizeof(deflate), file);
        fclose(file);
        break;
      /* load board from file */
      case 'l':
        printf("filename:");
        /* read filename */
        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf)-1] = '\0';
        /* open file */
        file = fopen(buf, "rb");
        if (!file) {
          printf("failed\n");
          break;
        }
        /* read from file */
        fread(deflate, 1, sizeof(deflate), file);
        fclose(file);
        /* inflate the board */
        inflate_board(board, deflate);
        break;
      /* quit */
      case 'q':
        return;
    }
  }
}
