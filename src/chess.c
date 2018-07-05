/******************************************************
 Desolation of the Dragon MUD II
 (C) 1997-2003  Jesse DeFer
 http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*
 * Chess for SMAUG 1.x          http://dotd.com
 * (C) Jesse DeFer, dotd@dotd.com, dotd.com 4000
 *
 * Installation instructions:
 * 1.  Copy the chess source to your SMAUG src dir
 * 2.  Add it to your makefile to be compiled/linked with SMAUG
 * 3.  Add do_smaug_chess to mud.h, tables.c, and commands.dat
 * 4.  Update SC_COMMANDNAME to the same name you created your command as
 * 5.  Include chess.h in mud.h, below the typedef for bool
 * 6.  Add 'GAME_BOARD_DATA *game_board;' to your pcdata struct (mud.h)
 * 7.  Add free_game(ch->pcdata->game_board); to db.c, free_char(),
 *     before DISPOSE( ch->pcdata );
 * 8.  Add ch->pcdata->game_board = NULL; to save.c (not very important),
 *     to where the rest of the pcdata structures get initialized
 * 9.  Check the defines after these comments
 * 10. Add lines 2-4 of this file to HELP CHESS (the copyright)
 *
 * IMC Installation instructions:
 * AntiFreeze CL-2 and later:
 * 1. init_chess(); to the mud's boot sequence (boot_db is a good choice)
 * Previous to AntiFreeze CL-1:
 * 1. Paste the following into imc.c after "tell" in imc_recv()
 else if (!strcasecmp(p->type, "chess"))
 {
 void imc_recv_chess(imc_char_data *from, PACKET *p);
 imc_recv_chess(&d, p);
 }
 * 2. Uncomment the #define USE_IMC below
 *
 * NOTE: The IMC code doesn't support castling or pawn promotion yet.
 */

/*
 * Todo: add 3 move repitition draw
 *       castling over imc
 */

/* Changes:
 * 0.23:
 *      only show CVS ID to imms, it's just spam for morts
 *      add _syntax argument so we don't duplicate the syntax string
 * 0.24:
 *      added castling
 *      added SC_COMMANDNAME for convienence
 *      rename do_game_board to do_smaug_chess
 * 0.25:
 *      pawn promotion
 * 0.26:
 *      pp bug fixes
 * 0.27:
 *      king movement fixes
 * 0.28:
 *      fixed an instance for a king in checkmate could move
 * 0.29:
 *      ported imc support to IMC2 Antifreeze CL-1 (untested)
 * 0.30:
 *      fixes to IMC2 support
 * 0.31:
 *      Upgrade to IMC2 AntiFreeze CL-2 (this means you)
 */

#include <stdio.h>
#include <string.h>

#include "mud.h"
#define SC_VERSION "0.31"
#define SC_INFO "SMAUG Chess v" SC_VERSION ", by Jesse DeFer, jdefer@dotd.com"
#define SC_CVSID "$Id: chess.c,v 1.37 2004/04/01 02:55:49 dotd Exp $"
#define SC_COMMANDNAME "chess"

#ifndef GET_NAME
#define GET_NAME(ch) (ch)->name
#endif
#define CHESS_NAME(ch,board) (board->type==TYPE_IMC?ch:GET_NAME(ch))

/*#define USE_IMC*/	/* Uncomment for IMC */
/*#define STOCK_COLOR*/	/* Uncomment for non DOTDII codebases*/
/*#define send_to_char send_to_char_color*/ /* Uncomment if required */

#define WHITE_BACKGROUND	"^w"
#define BLACK_BACKGROUND	"^x"
#define WHITE_FOREGROUND	"&W"
#define BLACK_FOREGROUND	"&x"

#ifdef USE_IMC
#include "imcsdk.h"

void imc_send_chess(CHAR_DATA *ch, char *to, char *argument);
#endif

void do_smaug_chess(CHAR_DATA *ch, char *argument);

const char *big_pieces[MAX_PIECES][2] = {
    {
        "%s       ",
        "%s       "
    },
    {
        "%s  (-)  ",
        "%s  -|-  "
    },
    {
        "%s  ###  ",
        "%s  { }  "
    },
    {
        "%s  ###  ",
        "%s  { }  "
    },
    {
        "%s  /-@- ",
        "%s / /   "
    },
    {
        "%s  () + ",
        "%s  {}-| "
    },
    {
        "%s   @   ",
        "%s  /+\\  "
    },
    {
        "%s  ^^^^^^  ",
        "%s  {@}  "
    },
    {
        "%s  [-]  ",
        "%s  -|-  "
    },
    {
        "%s  ###  ",
        "%s  [ ]  "
    },
    {
        "%s  ###  ",
        "%s  [ ]  "
    },
    {
        "%s  /-*- ",
        "%s / /   "
    },
    {
        "%s  [] + ",
        "%s  {}-| "
    },
    {
        "%s   #   ",
        "%s  /+\\  "
    },
    {
        "%s  ^^^^^^  ",
        "%s  [#]  "
    }
};

const char small_pieces[MAX_PIECES+1] = " prrnbqkPRRNBQK";

const char *piece_names[MAX_PIECES] =
{
    "empty",
    "pawn", "rook", "rook", "knight", "bishop", "queen", "king",
    "pawn", "rook", "rook", "knight", "bishop", "queen", "king"
};

static char *print_big_board(CHAR_DATA *ch, GAME_BOARD_DATA *board)
{
    static char retbuf[MAX_STRING_LENGTH*2];
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    char s1[16], s2[16];
    int x,y;

#ifdef STOCK_COLOR
    sprintf(s1,WHITE_FOREGROUND);
    sprintf(s2,BLACK_FOREGROUND);
#else
    sprintf(s1,"%s",atcode_color_str(ch->colors[AT_CHESS1]));
    sprintf(s2,"%s",atcode_color_str(ch->colors[AT_CHESS2]));
#endif

    sprintf(retbuf,WHITE_FOREGROUND "     1      2      3      4      5      6      7      8\n\r");

    for (x=0;x<8;x++)
    {
        strcat(retbuf,"  ");
        for (y=0;y<8;y++)
        {
            sprintf(buf,"%s%s",
                    x%2==0 ? (y%2==0 ? BLACK_BACKGROUND : WHITE_BACKGROUND) : \
                    (y%2==0 ? WHITE_BACKGROUND : BLACK_BACKGROUND),
                    big_pieces[board->board[x][y]][0]);
            sprintf(buf2,buf,IS_WHITE(board->board[x][y]) ? s1 : s2);
            strcat(retbuf,buf2);
        }
        strcat(retbuf, BLACK_BACKGROUND "\n\r");

        sprintf(buf, WHITE_FOREGROUND "%c ", 'A'+x);
        strcat(retbuf,buf);
        for (y=0;y<8;y++)
        {
            sprintf(buf,"%s%s",
                    x%2==0 ? (y%2==0 ? BLACK_BACKGROUND : WHITE_BACKGROUND) : \
                    (y%2==0 ? WHITE_BACKGROUND : BLACK_BACKGROUND),
                    big_pieces[board->board[x][y]][1]);
            sprintf(buf2,buf,IS_WHITE(board->board[x][y]) ? s1 : s2);
            strcat(retbuf,buf2);
        }
        strcat(retbuf, BLACK_BACKGROUND "\n\r");
    }

    return(retbuf);
}

static char *srow(CHAR_DATA *ch, GAME_BOARD_DATA *board, int x, int y)
{
    char buf[MAX_INPUT_LENGTH], s1[16], s2[16];

#ifdef STOCK_COLOR
    sprintf(s1,WHITE_FOREGROUND);
    sprintf(s2,BLACK_FOREGROUND);
#else
    sprintf(s1,"%s",atcode_color_str(ch->colors[AT_CHESS1]));
    sprintf(s2,"%s",atcode_color_str(ch->colors[AT_CHESS2]));
#endif

    sprintf(buf,"%s%s%c",
            x%2==0 ? (y%2==0 ? BLACK_BACKGROUND : WHITE_BACKGROUND) : \
            (y%2==0 ? WHITE_BACKGROUND : BLACK_BACKGROUND),
            board->board[x][y] == NO_PIECE ? "" : \
            ( IS_WHITE(board->board[x][y]) ? s1 : s2 ),
            small_pieces[board->board[x][y]]);

    return(str_dup(buf));
}

static char *print_small_board(CHAR_DATA *ch, GAME_BOARD_DATA *board)
{
    static char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char *a,*b,*c,*d,*e,*f,*g,*h;
    int x;

    sprintf(buf,BLACK_BACKGROUND WHITE_FOREGROUND \
            "   12345678\n\r  +--------+\n\r");

    for (x=0;x<8;x++)
    {
        a=srow(ch,board,x,0); b=srow(ch,board,x,1);
        c=srow(ch,board,x,2); d=srow(ch,board,x,3);
        e=srow(ch,board,x,4); f=srow(ch,board,x,5);
        g=srow(ch,board,x,6); h=srow(ch,board,x,7);
        sprintf(buf2,
                BLACK_BACKGROUND WHITE_FOREGROUND "%c |%s%s%s%s%s%s%s%s" \
                BLACK_BACKGROUND WHITE_FOREGROUND "| %c\n\r",
                'A'+x, a, b, c, d, e, f, g, h, 'A'+x );
        free(a); free(b); free(c); free(d);
        free(e); free(f); free(g); free(h);
        strcat(buf,buf2);
    }

    sprintf(buf2, BLACK_BACKGROUND WHITE_FOREGROUND \
            "  +--------+\n\r   12345678\n\r");
    strcat(buf,buf2);

    return(buf);
}

static void init_board(GAME_BOARD_DATA *board, bool imc)
{
    int x,y;
    for (x=0;x<8;x++)
        for (y=0;y<8;y++)
            board->board[x][y] = 0;
    board->board[0][0] = WHITE_QROOK;
    board->board[0][1] = WHITE_KNIGHT;
    board->board[0][2] = WHITE_BISHOP;
    if (!imc)
    {
	board->board[0][3] = WHITE_QUEEN;
	board->board[0][4] = WHITE_KING;
    }
    else
    {
	board->board[0][4] = WHITE_QUEEN;
	board->board[0][3] = WHITE_KING;
    }
    board->board[0][5] = WHITE_BISHOP;
    board->board[0][6] = WHITE_KNIGHT;
    board->board[0][7] = WHITE_KROOK;
    for (x=0;x<8;x++)
        board->board[1][x] = WHITE_PAWN;
    for (x=0;x<8;x++)
        board->board[6][x] = BLACK_PAWN;
    board->board[7][0] = BLACK_QROOK;
    board->board[7][1] = BLACK_KNIGHT;
    board->board[7][2] = BLACK_BISHOP;
    if (!imc)
    {
	board->board[7][3] = BLACK_QUEEN;
	board->board[7][4] = BLACK_KING;
    }
    else
    {
	board->board[7][4] = BLACK_QUEEN;
	board->board[7][3] = BLACK_KING;
    }
    board->board[7][5] = BLACK_BISHOP;
    board->board[7][6] = BLACK_KNIGHT;
    board->board[7][7] = BLACK_KROOK;
    for (x=0;x<MAX_PIECES;x++)
        board->moved[x] = 0;
    board->player1 = NULL;
    board->player2 = NULL;
    board->turn = NULL;
    board->moves = 0;
    board->type = TYPE_LOCAL;
}

static bool find_piece(GAME_BOARD_DATA *board, int *x, int *y, int piece)
{
    int a,b;

    for (a=0;a<8;a++)
    {
        for (b=0;b<8;b++)
            if (board->board[a][b] == piece)
                break;
        if (board->board[a][b] == piece)
            break;
    }
    *x = a;
    *y = b;
    if (board->board[a][b] == piece)
        return TRUE;
    return FALSE;
}

#define SAME_COLOR(x1,y1,x2,y2)	\
    ((IS_WHITE(board->board[x1][y1]) && IS_WHITE(board->board[x2][y2])) || \
    (IS_BLACK(board->board[x1][y1]) && IS_BLACK(board->board[x2][y2])))

static bool king_in_check(GAME_BOARD_DATA *board, int piece)
{
    int x=0,y=0,l,m;

    if ( piece != WHITE_KING && piece != BLACK_KING )
        return FALSE;

    if (!find_piece(board,&x,&y,piece))
        return FALSE;

    if ( x<0 || y<0 || x>7 || y>7 )
        return FALSE;

    /* pawns */
    if ( IS_WHITE(piece) && x < 7 &&
         (( y > 0 && IS_BLACK(board->board[x+1][y-1]) ) ||
          ( y < 7 && IS_BLACK(board->board[x+1][y+1]) )))
        return TRUE;
    else if ( IS_BLACK(piece) && x > 0 &&
              (( y > 0 && IS_WHITE(board->board[x-1][y-1]) ) ||
               ( y < 7 && IS_WHITE(board->board[x-1][y+1]) )))
        return TRUE;
    /* knights */
    if ( x-2 >= 0 && y-1 >= 0 &&
         ( (board->board[x-2][y-1] == BLACK_KNIGHT && IS_WHITE(board->board[x][y])) ||
           (board->board[x-2][y-1] == WHITE_KNIGHT && IS_BLACK(board->board[x][y])) ))
        return TRUE;
    if ( x-2 >= 0 && y+1 < 8 &&
         ( (board->board[x-2][y+1] == BLACK_KNIGHT && IS_WHITE(board->board[x][y])) ||
           (board->board[x-2][y+1] == WHITE_KNIGHT && IS_BLACK(board->board[x][y])) ))
        return TRUE;

    if ( x-1 >= 0 && y-2 >= 0 &&
         ( (board->board[x-1][y-2] == BLACK_KNIGHT && IS_WHITE(board->board[x][y])) ||
           (board->board[x-1][y-2] == WHITE_KNIGHT && IS_BLACK(board->board[x][y])) ))
        return TRUE;
    if ( x-1 >= 0 && y+2 < 8 &&
         ( (board->board[x-1][y+2] == BLACK_KNIGHT && IS_WHITE(board->board[x][y])) ||
           (board->board[x-1][y+2] == WHITE_KNIGHT && IS_BLACK(board->board[x][y])) ))
        return TRUE;

    if ( x+1 < 8 && y-2 >= 0 &&
         ( (board->board[x+1][y-2] == BLACK_KNIGHT && IS_WHITE(board->board[x][y])) ||
           (board->board[x+1][y-2] == WHITE_KNIGHT && IS_BLACK(board->board[x][y])) ))
        return TRUE;
    if ( x+1 < 8 && y+2 < 8 &&
         ( (board->board[x+1][y+2] == BLACK_KNIGHT && IS_WHITE(board->board[x][y])) ||
           (board->board[x+1][y+2] == WHITE_KNIGHT && IS_BLACK(board->board[x][y])) ))
        return TRUE;

    if ( x+2 < 8 && y-1 >= 0 &&
         ( (board->board[x+2][y-1] == BLACK_KNIGHT && IS_WHITE(board->board[x][y])) ||
           (board->board[x+2][y-1] == WHITE_KNIGHT && IS_BLACK(board->board[x][y])) ))
        return TRUE;
    if ( x+2 < 8 && y+1 < 8 &&
         ( (board->board[x+2][y+1] == BLACK_KNIGHT && IS_WHITE(board->board[x][y])) ||
           (board->board[x+2][y+1] == WHITE_KNIGHT && IS_BLACK(board->board[x][y])) ))
        return TRUE;

    /* horizontal/vertical long distance */
    for (l=x+1;l<8;l++)
        if ( board->board[l][y] != NO_PIECE )
        {
            if ( SAME_COLOR(x,y,l,y) )
                break;
            if ( board->board[l][y] == BLACK_QUEEN || board->board[l][y] == WHITE_QUEEN ||
                 board->board[l][y] == BLACK_QROOK || board->board[l][y] == WHITE_QROOK ||
                 board->board[l][y] == BLACK_KROOK || board->board[l][y] == WHITE_KROOK )
                return TRUE;
            break;
        }
    for (l=x-1;l>=0;l--)
        if ( board->board[l][y] != NO_PIECE )
        {
            if ( SAME_COLOR(x,y,l,y) )
                break;
            if ( board->board[l][y] == BLACK_QUEEN || board->board[l][y] == WHITE_QUEEN ||
                 board->board[l][y] == BLACK_QROOK || board->board[l][y] == WHITE_QROOK ||
                 board->board[l][y] == BLACK_KROOK || board->board[l][y] == WHITE_KROOK )
                return TRUE;
            break;
        }
    for (m=y+1;m<8;m++)
        if ( board->board[x][m] != NO_PIECE )
        {
            if ( SAME_COLOR(x,y,x,m) )
                break;
            if ( board->board[x][m] == BLACK_QUEEN || board->board[x][m] == WHITE_QUEEN ||
                 board->board[x][m] == BLACK_QROOK || board->board[x][m] == WHITE_QROOK ||
                 board->board[x][m] == BLACK_KROOK || board->board[x][m] == WHITE_KROOK )
                return TRUE;
            break;
        }
    for (m=y-1;m>=0;m--)
        if ( board->board[x][m] != NO_PIECE )
        {
            if ( SAME_COLOR(x,y,x,m) )
                break;
            if ( board->board[x][m] == BLACK_QUEEN || board->board[x][m] == WHITE_QUEEN ||
                 board->board[x][m] == BLACK_QROOK || board->board[x][m] == WHITE_QROOK ||
                 board->board[x][m] == BLACK_KROOK || board->board[x][m] == WHITE_KROOK )
                return TRUE;
            break;
        }
    /* diagonal long distance */
    for (l=x+1,m=y+1;l<8 && m<8;l++,m++)
        if ( board->board[l][m] != NO_PIECE )
        {
            if ( SAME_COLOR(x,y,l,m) )
                break;
            if ( board->board[l][m] == BLACK_QUEEN || board->board[l][m] == WHITE_QUEEN ||
                 board->board[l][m] == BLACK_BISHOP || board->board[l][m] == WHITE_BISHOP )
                return TRUE;
            break;
        }
    for (l=x-1,m=y+1;l>=0 && m<8;l--,m++)
        if ( board->board[l][m] != NO_PIECE )
        {
            if ( SAME_COLOR(x,y,l,m) )
                break;
            if ( board->board[l][m] == BLACK_QUEEN || board->board[l][m] == WHITE_QUEEN ||
                 board->board[l][m] == BLACK_BISHOP || board->board[l][m] == WHITE_BISHOP )
                return TRUE;
            break;
        }
    for (l=x+1,m=y-1;l<8 && m>=0;l++,m--)
        if ( board->board[l][m] != NO_PIECE )
        {
            if ( SAME_COLOR(x,y,l,m) )
                break;
            if ( board->board[l][m] == BLACK_QUEEN || board->board[l][m] == WHITE_QUEEN ||
                 board->board[l][m] == BLACK_BISHOP || board->board[l][m] == WHITE_BISHOP )
                return TRUE;
            break;
        }
    for (l=x-1,m=y-1;l>=0 && m>=0;l--,m--)
        if ( board->board[l][m] != NO_PIECE )
        {
            if ( SAME_COLOR(x,y,l,m) )
                break;
            if ( board->board[l][m] == BLACK_QUEEN || board->board[l][m] == WHITE_QUEEN ||
                 board->board[l][m] == BLACK_BISHOP || board->board[l][m] == WHITE_BISHOP )
                return TRUE;
            break;
        }
    return FALSE;
}

static bool king_in_possible_checkmate(GAME_BOARD_DATA *board, int piece)
{
    int x=0,y=0,dx,dy,sk=0;

    if ( piece != WHITE_KING && piece != BLACK_KING )
        return FALSE;

    if (!find_piece(board,&x,&y,piece))
        return FALSE;

    if ( x<0 || y<0 || x>7 || y>7 )
        return FALSE;

    if (!king_in_check(board,board->board[x][y]))
        return FALSE;

    dx = x+1;
    dy = y+1;
    if ( dx < 8 && dy < 8 && board->board[dx][dy] == NO_PIECE )
    {
        sk = board->board[dx][dy] = board->board[x][y];
        board->board[x][y] = NO_PIECE;
        if (!king_in_check(board,sk))
        {
            board->board[x][y] = sk;
            board->board[dx][dy] = NO_PIECE;
            return FALSE;
        }
        board->board[x][y] = sk;
        board->board[dx][dy] = NO_PIECE;
    }
    dx = x-1;
    dy = y+1;
    if ( dx >= 0 && dy < 8 && board->board[dx][dy] == NO_PIECE )
    {
        sk = board->board[dx][dy] = board->board[x][y];
        board->board[x][y] = NO_PIECE;
        if (!king_in_check(board,sk))
        {
            board->board[x][y] = sk;
            board->board[dx][dy] = NO_PIECE;
            return FALSE;
        }
        board->board[x][y] = sk;
        board->board[dx][dy] = NO_PIECE;
    }
    dx = x+1;
    dy = y-1;
    if ( dx < 8 && dy >= 0 && board->board[dx][dy] == NO_PIECE )
    {
        sk = board->board[dx][dy] = board->board[x][y];
        board->board[x][y] = NO_PIECE;
        if (!king_in_check(board,sk))
        {
            board->board[x][y] = sk;
            board->board[dx][dy] = NO_PIECE;
            return FALSE;
        }
        board->board[x][y] = sk;
        board->board[dx][dy] = NO_PIECE;
    }
    dx = x-1;
    dy = y-1;
    if ( dx >= 0 && dy >= 0 && board->board[dx][dy] == NO_PIECE )
    {
        sk = board->board[dx][dy] = board->board[x][y];
        board->board[x][y] = NO_PIECE;
        if (!king_in_check(board,sk))
        {
            board->board[x][y] = sk;
            board->board[dx][dy] = NO_PIECE;
            return FALSE;
        }
        board->board[x][y] = sk;
        board->board[dx][dy] = NO_PIECE;
    }
    dx = x;
    dy = y+1;
    if ( dy < 8 && board->board[dx][dy] == NO_PIECE )
    {
        sk = board->board[dx][dy] = board->board[x][y];
        board->board[x][y] = NO_PIECE;
        if (!king_in_check(board,sk))
        {
            board->board[x][y] = sk;
            board->board[dx][dy] = NO_PIECE;
            return FALSE;
        }
        board->board[x][y] = sk;
        board->board[dx][dy] = NO_PIECE;
    }
    dx = x;
    dy = y-1;
    if ( dy >= 0 && board->board[dx][dy] == NO_PIECE )
    {
        sk = board->board[dx][dy] = board->board[x][y];
        board->board[x][y] = NO_PIECE;
        if (!king_in_check(board,sk))
        {
            board->board[x][y] = sk;
            board->board[dx][dy] = NO_PIECE;
            return FALSE;
        }
        board->board[x][y] = sk;
        board->board[dx][dy] = NO_PIECE;
    }
    dx = x+1;
    dy = y;
    if ( dx < 8 && board->board[dx][dy] == NO_PIECE )
    {
        sk = board->board[dx][dy] = board->board[x][y];
        board->board[x][y] = NO_PIECE;
        if (!king_in_check(board,sk))
        {
            board->board[x][y] = sk;
            board->board[dx][dy] = NO_PIECE;
            return FALSE;
        }
        board->board[x][y] = sk;
        board->board[dx][dy] = NO_PIECE;
    }
    dx = x-1;
    dy = y;
    if ( dx >= 0 && board->board[dx][dy] == NO_PIECE )
    {
        sk = board->board[dx][dy] = board->board[x][y];
        board->board[x][y] = NO_PIECE;
        if (!king_in_check(board,sk))
        {
            board->board[x][y] = sk;
            board->board[dx][dy] = NO_PIECE;
            return FALSE;
        }
        board->board[x][y] = sk;
        board->board[dx][dy] = NO_PIECE;
    }
    return TRUE;
}

static int is_valid_move(CHAR_DATA *ch, GAME_BOARD_DATA *board, int x, int y, int dx, int dy)
{
    if ( dx<0 || dy<0 || dx>7 || dy>7 )
        return MOVE_OFFBOARD;

    if ( board->board[x][y] == NO_PIECE )
        return MOVE_INVALID;

    if ( x == dx && y == dy )
        return MOVE_INVALID;

    if ( IS_WHITE(board->board[x][y]) && board->player1 == ch )
        return MOVE_WRONGCOLOR;
    if ( IS_BLACK(board->board[x][y]) && (board->player2 == ch || !ch) )
        return MOVE_WRONGCOLOR;

    switch (board->board[x][y])
    {
    case WHITE_PAWN:    case BLACK_PAWN:
        if ( IS_WHITE(board->board[x][y]) &&
             dx == x+2 && x == 1 && dy == y &&
             board->board[dx][dy] == NO_PIECE &&
             board->board[x+1][dy] == NO_PIECE )
            return MOVE_OK;
        else if ( IS_BLACK(board->board[x][y]) &&
                  dx == x-2 && x == 6 && dy == y &&
                  board->board[dx][dy] == NO_PIECE &&
                  board->board[x-1][dy] == NO_PIECE )
            return MOVE_OK;
        if ( IS_WHITE(board->board[x][y]) && dx != x+1 )
            return MOVE_INVALID;
        else if ( IS_BLACK(board->board[x][y]) && dx != x-1 )
            return MOVE_INVALID;
        if ( dy != y && dy != y-1 && dy != y+1 )
            return MOVE_INVALID;
        if ( dy == y )
        {
            if ( board->board[dx][dy] == NO_PIECE)
                return MOVE_OK;
            else if ( SAME_COLOR(x,y,dx,dy) )
                return MOVE_SAMECOLOR;
            else
                return MOVE_BLOCKED;
        }
        else
        {
            if ( board->board[dx][dy] == NO_PIECE )
                return MOVE_INVALID;
            else if ( SAME_COLOR(x,y,dx,dy) )
                return MOVE_SAMECOLOR;
            else if ( board->board[dx][dy] != BLACK_KING &&
                      board->board[dx][dy] != WHITE_KING )
                return MOVE_TAKEN;
            else
                return MOVE_INVALID;
        }
        break;
    case WHITE_QROOK:	case BLACK_QROOK:
    case WHITE_KROOK:	case BLACK_KROOK:
        {
            int cnt;

            if ( dx != x && dy != y )
                return MOVE_INVALID;

            if ( dx == x)
            {
                for (cnt = y; cnt != dy; )
                {
                    if ( cnt != y && board->board[x][cnt] != NO_PIECE )
                        return MOVE_BLOCKED;
                    if ( dy > y )
                        cnt++;
                    else
                        cnt--;
                }
            }
            else if ( dy == y)
            {
                for (cnt = x; cnt != dx; )
                {
                    if ( cnt !=x && board->board[cnt][y] != NO_PIECE )
                        return MOVE_BLOCKED;
                    if ( dx > x )
                        cnt++;
                    else
                        cnt--;
                }
            }

            if ( board->board[dx][dy] == NO_PIECE )
                return MOVE_OK;

            if ( !SAME_COLOR(x,y,dx,dy) )
                return MOVE_TAKEN;

            return MOVE_SAMECOLOR;
        }
        break;
    case WHITE_KNIGHT:	case BLACK_KNIGHT:
        if ( (dx == x-2 && dy == y-1) ||
             (dx == x-2 && dy == y+1) ||
             (dx == x-1 && dy == y-2) ||
             (dx == x-1 && dy == y+2) ||
             (dx == x+1 && dy == y-2) ||
             (dx == x+1 && dy == y+2) ||
             (dx == x+2 && dy == y-1) ||
             (dx == x+2 && dy == y+1) )
        {
            if ( board->board[dx][dy] == NO_PIECE )
                return MOVE_OK;
            if ( SAME_COLOR(x,y,dx,dy) )
                return MOVE_SAMECOLOR;
            return MOVE_TAKEN;
        }
        return MOVE_INVALID;
        break;
    case WHITE_BISHOP:	case BLACK_BISHOP:
        {
            int l, m, blocked = FALSE;

            if ( dx == x || dy == y )
                return MOVE_INVALID;

            l = x;
            m = y;

            while ( 1 )
            {
                if ( dx > x )
                    l++;
                else
                    l--;
                if ( dy > y )
                    m++;
                else
                    m--;
                if ( l > 7 || m > 7 || l < 0 || m < 0 )
                    return MOVE_INVALID;
                if ( l == dx && m == dy )
                    break;
                if ( board->board[l][m] != NO_PIECE )
                    blocked = TRUE;
            }
            if ( l != dx || m != dy )
                return MOVE_INVALID;

            if ( blocked )
                return MOVE_BLOCKED;

            if ( board->board[dx][dy] == NO_PIECE )
                return MOVE_OK;

            if ( !SAME_COLOR(x,y,dx,dy) )
                return MOVE_TAKEN;

            return MOVE_SAMECOLOR;
        }
        break;
    case WHITE_QUEEN:	case BLACK_QUEEN:
        {
            int l, m, blocked = FALSE;

            l = x;
            m = y;

            while ( 1 )
            {
                if ( dx > x )
                    l++;
                else if ( dx < x )
                    l--;
                if ( dy > y )
                    m++;
                else if ( dy < y )
                    m--;
                if ( l > 7 || m > 7 || l < 0 || m < 0 )
                    return MOVE_INVALID;
                if ( l == dx && m == dy )
                    break;
                if ( board->board[l][m] != NO_PIECE )
                    blocked = TRUE;
            }
            if ( l != dx || m != dy )
                return MOVE_INVALID;

            if ( blocked )
                return MOVE_BLOCKED;

            if ( board->board[dx][dy] == NO_PIECE )
                return MOVE_OK;

            if ( !SAME_COLOR(x,y,dx,dy) )
                return MOVE_TAKEN;

            return MOVE_SAMECOLOR;
        }
        break;
    case WHITE_KING:	case BLACK_KING:
        {
            int sp,sk;

            if ( dx > x+1 || dx < x-1 || dy > y+1 || dy < y-1 )
                return MOVE_INVALID;

            if ( SAME_COLOR(x,y,dx,dy) )
                return MOVE_SAMECOLOR;

            sk = board->board[x][y];
            sp = board->board[dx][dy];
            board->board[x][y] = NO_PIECE;
            board->board[dx][dy] = sk;

            if (king_in_check(board,sk))
            {
                board->board[x][y] = sk;
                board->board[dx][dy] = sp;
                return MOVE_CHECK;
            }

            board->board[x][y] = sk;
            board->board[dx][dy] = sp;

            if ( board->board[dx][dy] == NO_PIECE )
                return MOVE_OK;

            return MOVE_TAKEN;
        }
        break;
    default:
        bug("is_valid_move():chess.c invaild piece: %d", board->board[x][y]);
        return MOVE_INVALID;
        break;
    }

    bug("is_valid_move():chess.c shouldn't get here");
    return MOVE_OK;
}

#undef SAME_COLOR

static void board_move_stuff( GAME_BOARD_DATA *board )
{
    board->moves++;
    if (board->turn == board->player1)
        board->turn = board->player2;
    else
        board->turn = board->player1;
}

void free_game( GAME_BOARD_DATA *board )
{
    if ( !board )
        return;
#ifdef USE_IMC
    if ( board->type == TYPE_IMC )
    {
        imc_send_chess((CHAR_DATA *)board->player1?:NULL, (char *)board->player2, "stop");
        if (board->player2)
            DISPOSE(board->player2);
    }
#endif
    if ( board->player1 )
    {
        CHAR_DATA *ch = (CHAR_DATA *)board->player1;
        ch_printf(ch, "The game has been stopped at %d total moves.\n\r", board->moves);
        ch->pcdata->game_board = NULL;
    }
    if ( board->player2 )
    {
        CHAR_DATA *ch = (CHAR_DATA *)board->player2;
        ch_printf(ch, "The game has been stopped at %d total moves.\n\r", board->moves);
        ch->pcdata->game_board = NULL;
    }
    board->player1 = NULL;
    board->player2 = NULL;
    DISPOSE(board);
}

#ifdef USE_IMC
void imc_send_chess(CHAR_DATA *ch, char *to, char *argument)
{
    PACKET out;

    if ( !ch || imc_active<IA_UP )
        return;

    if( !strcasecmp( imc_mudof( to ), "*" ) )
	return; /* don't let them do this */

    setdata(&out, imc_getdata(ch));

    imcstrlcpy( out.to, to, IMC_NAME_LENGTH );
    imcstrlcpy( out.type, "chess", IMC_TYPE_LENGTH );
    imc_addkey( &out, "text", argument );

    imc_send(&out);
    imc_freedata(&out);
}

void imc_recv_chess(imc_char_data *from, PACKET *p)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim, *vch;
    char buf[MAX_INPUT_LENGTH];
    char *argument;

    argument = imc_getkey( p, "text", "" );

    if ( !strcmp(p->to, "*") )
        return;

    victim=NULL;
    for ( d=first_descriptor; d; d=d->next )
    {
        if ( d->connected==CON_PLAYING &&
             (vch=d->original ? d->original : d->character)!=NULL &&
             !IS_NPC(vch) )
        {
            if ( !str_cmp(p->to, GET_NAME(vch)) )
            {
                victim=vch;
                break;
            }
            if ( is_name(p->to, GET_NAME(vch)) )
                victim=vch;
        }
    }

    if ( !victim )
    {
        if ( !str_cmp(argument, "stop") )
            return;
        sprintf(buf, "%s is not here.", p->to);
        imc_send_tell(NULL, from->name, buf, 1);
        return;
    }

    if ( !victim->pcdata->game_board )
    {
        if ( !str_cmp(argument, "stop") )
            return;
        sprintf(buf, "%s is not ready to be joined in a game.", p->to);
        imc_send_tell(NULL, from->name, buf, 1);
        imc_send_chess((CHAR_DATA *)victim->pcdata->game_board->player1?:NULL,(char *)from->name,"stop");
        return;
    }

    if ( !str_cmp(argument, "start") )
    {
        if ( victim->pcdata->game_board->player2 != NULL )
        {
            sprintf(buf, "%s is already playing a game.", p->to);
            imc_send_tell(NULL, from->name, buf, 1);
            imc_send_chess((CHAR_DATA *)victim->pcdata->game_board->player1?:NULL,(char *)from->name,"stop");
            return;
        }
        victim->pcdata->game_board->player2 = str_dup(from->name);
        victim->pcdata->game_board->moves = 0;
        victim->pcdata->game_board->type = TYPE_IMC;
        ch_printf(victim, "%s has joined your game.\n\r", from->name);
        imc_send_chess(victim, from->name, "accepted");
        return;
    }
    if ( !str_cmp(argument, "accepted") )
    {
        if ( !victim->pcdata->game_board ||
             victim->pcdata->game_board->player2 == NULL ||
             victim->pcdata->game_board->type != TYPE_IMC ||
             str_cmp((char *)victim->pcdata->game_board->player2,from->name) )
        {
            imc_send_chess((CHAR_DATA *)victim->pcdata->game_board->player1?:NULL,(char *)from->name, "stop");
            return;
        }
        ch_printf(victim,"You have joined %s in a game.\n\r", from->name);
        if (victim->pcdata->game_board->player2)
            DISPOSE(victim->pcdata->game_board->player2);
        victim->pcdata->game_board->player2 = str_dup(from->name);
        victim->pcdata->game_board->moves = 0;
        return;
    }
    if ( !str_cmp(argument, "stop") )
    {
        ch_printf(victim, "%s has stopped the game.\n\r", from->name);
        free_game(victim->pcdata->game_board);
        return;
    }
    if ( !str_cmp(argument, "forfeit") )
    {
        ch_printf(victim, "%s has forfeited the game, you win!\n\r", from->name);
        free_game(victim->pcdata->game_board);
        return;
    }
    if ( !str_cmp(argument, "invalidmove") )
    {
        send_to_char("You have issued an invalid move according to the other mud.\n\r", victim);
        do_smaug_chess(victim, "stop");
        return;
    }
    if ( !str_cmp(argument, "moveok") )
    {
        send_to_char("The other mud has accepted your move.\n\r", victim);
        return;
    }

    if ( !str_prefix("move", argument) )
    {
        char a,b;
        int x,y,dx,dy,ret;
        a=b=' ';
        x=y=dx=dy=-1;
        if (sscanf(argument, "move %c%d %c%d",&a,&y,&b,&dy) != 4 ||
            a<'0' || a>'7' || b<'0' || b>'7' || y<0 || y>7 || dy<0 || dy>7)
        {
            imc_send_chess((CHAR_DATA *)victim->pcdata->game_board->player1?:NULL,(char *)from->name, "invalidmove");
            return;
        }
        x = a - '0';
        dx = b - '0';
        x = (7-x);
        y = (7-y);
        dx = (7-dx);
        dy = (7-dy);
        log_printf_plus(LOG_DEBUG, LEVEL_LOG_CSET, SEV_SPAM, "IMC Chess move: %d, %d -> %d, %d", x,y,dx,dy);
        ret = is_valid_move(NULL,victim->pcdata->game_board,x,y,dx,dy);
        if (ret == MOVE_OK || ret == MOVE_TAKEN)
        {
            GAME_BOARD_DATA *board;
            int piece, destpiece;
            board = victim->pcdata->game_board;
            piece = board->board[x][y];
            destpiece = board->board[dx][dy];
            board->board[dx][dy] = piece;
            board->board[x][y] = NO_PIECE;
            if ( king_in_check(board,IS_WHITE(board->board[dx][dy])?WHITE_KING:BLACK_KING) &&
                 ( board->board[dx][dy]!=WHITE_KING && board->board[dx][dy]!=BLACK_KING ))
            {
                board->board[dx][dy] = destpiece;
		board->board[x][y] = piece;
                /* fall through and send invalidmove */
            }
            else
	    {
		ch_printf(victim, "%s has moved, %c%d (%s) to %c%d (%s)\n\r",
			  from->name,
			  x + 'a', y+1, piece_names[piece],
			  dx + 'a', dy+1, piece_names[destpiece]);

                board_move_stuff(board);
                board->lastx = dx;
                board->lasty = dy;
                board->moved[piece]++;
                board->moved[destpiece]++; /* taken pieces marked as moved */
                imc_send_chess((CHAR_DATA *)board->player1?:NULL,(char *)from->name, "moveok");
                return;
            }
	}
/*	ch_printf(victim, "%s has tried to move, %c%d (%s) to %c%d (%s)\n\r",
		  from->name,
		  x + 'a', y+1, piece_names[victim->pcdata->game_board->board[x][y]],
		  dx + 'a', dy+1, piece_names[victim->pcdata->game_board->board[dx][dy]]);*/
        ch_printf(victim, "%s has made an invalid move, aborting the game.\n\r", from->name);
	log_printf_plus(LOG_IMC, LEVEL_LOG_CSET, SEV_ERR, "IMC Chess invalidmove: ret=%d", ret);
	imc_send_chess((CHAR_DATA *)victim->pcdata->game_board->player1?:NULL,(char *)from->name, "invalidmove");
        return;
    }

    sprintf(log_buf, "Unknown chess command from: %s, %s", from->name, argument);
    log_string_plus(log_buf, LOG_IMC, LEVEL_LOG_CSET, SEV_ERR);
}
#endif

static void board_move_messages(CHAR_DATA *ch, int ret, char *extra)
{
    CHAR_DATA *opp;
    char buf[MAX_INPUT_LENGTH];

    if ( ch == ch->pcdata->game_board->player1 )
        opp = (CHAR_DATA *)ch->pcdata->game_board->player2;
    else
        opp = (CHAR_DATA *)ch->pcdata->game_board->player1;
    /* don't send messages to imc opponent, their local system will display
       the messages to them itself */
#define SEND_TO_OPP(buf,opp) \
    if (opp) \
    { \
        if (ch->pcdata->game_board->type==TYPE_LOCAL) \
        { \
            send_to_char((buf),(opp)); \
            send_to_char("\n\r",(opp)); \
        } \
    }
    switch (ret)
    {
    case MOVE_OK:
        send_to_char("Ok.\n\r", ch);
        sprintf(buf, "%s has moved, %s.", GET_NAME(ch), extra);
        SEND_TO_OPP(buf, opp);
        break;
    case MOVE_CASTLE:
        send_to_char("Ok.\n\r", ch);
        sprintf(buf, "%s has castled %s.", GET_NAME(ch), extra);
        SEND_TO_OPP(buf, opp);
        break;
    case MOVE_PROMOTE:
        send_to_char("Ok.\n\r", ch);
        sprintf(buf, "%s has promoted a pawn to a %s.", GET_NAME(ch), extra);
        SEND_TO_OPP(buf, opp);
        break;
    case MOVE_INVALID:
        send_to_char("Invalid move.\n\r", ch);
        break;
    case MOVE_BLOCKED:
        send_to_char("You are blocked in that direction.\n\r", ch);
        break;
    case MOVE_TAKEN:
        send_to_char("You take the enemy's piece.\n\r", ch);
        sprintf(buf, "%s has moved, %s, and captured one of your pieces!", GET_NAME(ch), extra);
        SEND_TO_OPP(buf, opp);
        break;
    case MOVE_OFFBOARD:
        send_to_char("That move would be off the board.\n\r", ch);
        break;
    case MOVE_SAMECOLOR:
        send_to_char("Your own piece blocks the way.\n\r", ch);
        break;
    case MOVE_CHECK:
        send_to_char("That move would result in a check.\n\r", ch);
        sprintf(buf, "%s has attempted a move that would result in a check.", GET_NAME(ch));
        SEND_TO_OPP(buf, opp);
        break;
    case MOVE_WRONGCOLOR:
        send_to_char("That is not your piece.\n\r", ch);
        break;
    case MOVE_INCHECK:
        send_to_char("Your king would still be in check as a result of that move.\n\r", ch);
        break;
    default:
        bug("board_move_messages():chess.c unknown return value from is_valid_move():chess.c");
        break;
    }
#undef SEND_TO_OPP
}

void do_smaug_chess(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg);

    if ( IS_NPC(ch) )
    {
        send_to_char("NPC's can't be in games.\n\r", ch);
        return;
    }

    if ( !str_cmp(arg, "_syntax") || !str_cmp(arg, "help") || arg[0] == '?' )
    {
        send_to_char(SC_INFO "\n\r", ch);
        if (IS_IMMORTAL(ch))
            send_to_char(SC_CVSID "\n\r", ch);
        send_to_char("Usage:    " SC_COMMANDNAME " <start|stop|join|forfeit>\n\r"
                     "Info:     " SC_COMMANDNAME " <status|board|sboard|pieces>\n\r"
                     "Movement: " SC_COMMANDNAME " <move> <source> <dest> [extra command]\n\r"
                     "Extras:   " SC_COMMANDNAME " <castle|promote>\n\r", ch);
        return;
    }

    if ( !str_cmp(arg, "start") )
    {
        GAME_BOARD_DATA *board;
        if ( ch->pcdata->game_board )
        {
            send_to_char("You are already in a game.\n\r", ch);
            return;
        }
        CREATE(board, GAME_BOARD_DATA, 1);
        init_board(board, FALSE);
        ch->pcdata->game_board = board;
        ch->pcdata->game_board->player1 = ch;
        ch->pcdata->game_board->turn = ch;
        send_to_char("You have started a game.\n\r", ch);
        return;
    }

    if ( !str_cmp(arg, "join") )
    {
        GAME_BOARD_DATA *board=NULL;
        CHAR_DATA *vch;
        char arg2[MAX_INPUT_LENGTH];
        if ( ch->pcdata->game_board )
        {
            send_to_char("You are already in a game.\n\r", ch);
            return;
        }
        argument = one_argument(argument,arg2);
        if ( arg[0] == '\0' )
        {
            send_to_char("Join who?\n\r", ch);
            return;
        }
#ifdef USE_IMC
        if ( strstr( arg2, "@" ) )
        {
            if (!str_cmp(imc_mudof(arg2), imc_siteinfo.localname))
            {
                send_to_char("That is this mud, don't use @.\n\r",ch);
                return;
            }
            send_to_char("Attempting to initiate IMC game...\n\r", ch);
            if (!str_cmp(imc_mudof(arg2), "*"))
            {
                send_to_char("* is not a valid mud name.\n\r",ch);
                return;
            }
            CREATE(board, GAME_BOARD_DATA, 1);
            init_board(board, TRUE);
            board->type = TYPE_IMC;
            board->player1 = (void *)ch;
            board->player2 = (void *)str_dup(arg2);
            board->turn = board->player2;
            board->moves = -1;
            ch->pcdata->game_board = board;
            imc_send_chess(ch,arg2,"start");
            return;
        }
#endif
        if ( !( vch = get_char_world(ch,arg2) ) )
        {
            send_to_char("Cannot find that player.\n\r", ch);
            return;
        }
        if ( IS_NPC(vch) )
        {
            send_to_char("That player is an NPC, and cannot play games.\n\r", ch);
            return;
        }
        board = vch->pcdata->game_board;
        if ( !board )
        {
            send_to_char("That player is not playing a game.\n\r", ch);
            return;
        }
        if ( board->player2 )
        {
            send_to_char("That game already has two players.\n\r", ch);
            return;
        }
        board->player2 = (void *)ch;
        board->turn = board->player2;
        ch->pcdata->game_board = board;
        send_to_char("You have joined a game, it is your turn.\n\r", ch);
        ch_printf((CHAR_DATA *)board->player1, "%s has joined your game.\n\r", GET_NAME(ch));
        return;
    }

    if ( !str_cmp(arg, "pieces") )
    {
        char buf[MAX_INPUT_LENGTH];
        int x;

        send_to_char("White pieces:\n\r", ch);
        for (x=WHITE_PAWN;x<=WHITE_KING;x++)
        {
            if (x == WHITE_KROOK)
                continue;
            ch_printf(ch, "%-7s", piece_names[x]);
        }
        send_to_char("\n\r", ch);
        for (x=WHITE_PAWN;x<=WHITE_KING;x++)
        {
            if (x == WHITE_KROOK)
                continue;
            sprintf(buf, "%s", big_pieces[x][0]);
            ch_printf(ch, buf, "");
        }
        send_to_char("\n\r", ch);
        for (x=WHITE_PAWN;x<=WHITE_KING;x++)
        {
            if (x == WHITE_KROOK)
                continue;
            sprintf(buf, "%s", big_pieces[x][1]);
            ch_printf(ch, buf, "");
        }
        send_to_char("\n\r\n\rBlack pieces:\n\r", ch);

        for (x=BLACK_PAWN;x<=BLACK_KING;x++)
        {
            if (x == BLACK_KROOK)
                continue;
            ch_printf(ch, "%-7s", piece_names[x]);
        }
        send_to_char("\n\r", ch);
        for (x=BLACK_PAWN;x<=BLACK_KING;x++)
        {
            if (x == BLACK_KROOK)
                continue;
            sprintf(buf, "%s", big_pieces[x][0]);
            ch_printf(ch, buf, "");
        }
        send_to_char("\n\r", ch);
        for (x=BLACK_PAWN;x<=BLACK_KING;x++)
        {
            if (x == BLACK_KROOK)
                continue;
            sprintf(buf, "%s", big_pieces[x][1]);
            ch_printf(ch, buf, "");
        }
        send_to_char("\n\r", ch);
 
        return;
    }

    if ( !ch->pcdata->game_board )
    {
        do_smaug_chess(ch, "_syntax");
        return;
    }

    if ( !str_cmp(arg, "stop") )
    {
        free_game(ch->pcdata->game_board);
        return;
    }

    if ( !str_cmp(arg, "forfeit") )
    {
        send_to_char("You forfeit the game, you loose!\n\r", ch);
        free_game(ch->pcdata->game_board);
        return;
    }

    if ( !str_cmp(arg, "status") )
    {
        GAME_BOARD_DATA *board = ch->pcdata->game_board;
        if ( !board->player1 )
            send_to_char("There is no black player.\n\r", ch);
        else if ( board->player1 == ch )
            send_to_char("You are black.\n\r", ch);
        else
            ch_printf(ch, "%s is black.\n\r",
                      GET_NAME((CHAR_DATA *)board->player1));
        if (king_in_possible_checkmate(board,BLACK_KING))
            send_to_char("The black king is possibly checkmated.\n\r", ch);
        else if (king_in_check(board,BLACK_KING))
            send_to_char("The black king is in check.\n\r", ch);
        if ( !board->player2 )
            send_to_char("There is no white player.\n\r", ch);
        else if ( board->player2 == ch )
            send_to_char("You are white.\n\r", ch);
        else
            ch_printf(ch, "%s is white.\n\r",
                      board->type == TYPE_LOCAL ?
                      GET_NAME((CHAR_DATA *)board->player2) :
                      (char *)board->player2);
        if (king_in_possible_checkmate(board,WHITE_KING))
            send_to_char("The white king is possibly checkmated.\n\r", ch);
        else if (king_in_check(board,WHITE_KING))
            send_to_char("The white king is in check.\n\r", ch);
        if ( !board->player2 ||
             !board->player1 )
            return;
        if (board->moves<0)
            send_to_char("The game hasn't started yet.\n\r", ch);
        else
            ch_printf(ch, "%d turns.\n\r", board->moves);
        if ( board->turn == ch )
            send_to_char("It is your turn.\n\r", ch);
        else
        {
            ch_printf(ch, "It is %s's turn.\n\r",
                      board->type == TYPE_LOCAL ?
                      GET_NAME((CHAR_DATA *)board->turn) :
                      (char *)board->turn);
        }
        return;
    }

    if ( !str_prefix(arg, "board") )
    {
        static char *b1;
        b1 = print_big_board(ch, ch->pcdata->game_board);
        send_to_char(b1,ch);
        return;
    }
    if ( !str_prefix(arg, "sboard") )
    {
        send_to_char(print_small_board(ch,ch->pcdata->game_board),ch);
        return;
    }

    if ( !ch->pcdata->game_board->player1 ||
         !ch->pcdata->game_board->player2 )
    {
        send_to_char("There is only 1 player.\n\r", ch);
        return;
    }
    if ( ch->pcdata->game_board->moves < 0 )
    {
        send_to_char("The game hasn't started yet.\n\r", ch);
        return;
    }

    if ( !str_prefix(arg, "promote") )
    {
        GAME_BOARD_DATA *board = ch->pcdata->game_board;
        int piece = board->board[board->lastx][board->lasty];
        char extra[MAX_INPUT_LENGTH];

        if (!((piece == BLACK_PAWN && board->player1 == ch) ||
              (piece == WHITE_PAWN && board->player2 == ch)))
        {
            send_to_char("You can't promote that.\n\r", ch);
            return;
        }

        if ((piece == BLACK_PAWN && board->lastx != 0) ||
            (piece == WHITE_PAWN && board->lastx != 7))
        {
            send_to_char("You can only promote pawns when they have reached theother end of the board.\n\r", ch);
            return;
        }

        if ( !argument || !*argument )
        {
            send_to_char("Promote it to what?\n\r", ch);
            return;
        }

        if ( !str_prefix(argument, "queen") )
        {
            if (IS_WHITE(piece))
                piece = WHITE_QUEEN;
            else
                piece = BLACK_QUEEN;
        }
        else if ( !str_prefix(argument, "bishop") )
        {
            if (IS_WHITE(piece))
                piece = WHITE_BISHOP;
            else
                piece = BLACK_BISHOP;
        }
        else if ( !str_prefix(argument, "knight") )
        {
            if (IS_WHITE(piece))
                piece = WHITE_KNIGHT;
            else
                piece = BLACK_KNIGHT;
        }
        else if ( !str_prefix(argument, "rook") )
        {
            if (IS_WHITE(piece))
                piece = WHITE_KROOK;
            else
                piece = BLACK_KROOK;
        }
        else
        {
            send_to_char("You can't promote it to that.\n\r", ch);
            return;
        }

        board->board[board->lastx][board->lasty] = piece;

        sprintf(extra, "%s (%c%d)",
                piece_names[piece], board->lastx+'a', board->lasty+1);

	board_move_messages(ch, MOVE_PROMOTE, extra);

#ifdef USE_IMC
	/* TODO: IMC support for promote */
	if ( ch->pcdata->game_board->type == TYPE_IMC)
	{
	    sprintf(arg, "promote %d%d %s", board->lastx, board->lasty, argument);
	    imc_send_chess((CHAR_DATA *)ch->pcdata->game_board->player1, (char *)ch->pcdata->game_board->player2, arg);
	}
#endif
        return;
    }

    if ( ch->pcdata->game_board->turn != ch )
    {
        send_to_char("It is not your turn.\n\r", ch);
        return;
    }

    if ( !str_prefix(arg, "castle") )
    {
        GAME_BOARD_DATA *board = ch->pcdata->game_board;
        int myx, rooky, kdy, rdy;

        if ( king_in_check(board,board->player1==ch?BLACK_KING:WHITE_KING) )
        {
            send_to_char("You cannot castle when in check.\n\r", ch);
            return;
        }

        if (board->player1 == ch)
        {
            if (board->moved[BLACK_KING] > 0)
            {
                send_to_char("You cannot castle if you have moved your king before.\n\r", ch);
                return;
            }
            myx = 7;
        }
        else
        {
            if (board->moved[WHITE_KING] > 0)
            {
                send_to_char("You cannot castle if you have moved your king before.\n\r", ch);
                return;
            }
            myx = 0;
        }

        if ( !*argument )
        {
            send_to_char("Usage: " SC_COMMANDNAME " castle <short|kingside|long|queenside>\n\r",ch);
            return;
        }

        if ( !str_prefix(argument, "short") || !str_prefix(argument, "kingside") )
        {
            if ((board->player1 == ch && board->moved[BLACK_KROOK] > 0) ||
                (board->player2 == ch && board->moved[WHITE_KROOK] > 0))
            {
                send_to_char("You cannot castle because you have moved your kingside rook before.\n\r", ch);
                return;
            }
            rooky = 7;
        }
        else if ( !str_prefix(argument, "long") || !str_prefix(argument, "queenside") )
        {
            if ((board->player1 == ch && board->moved[BLACK_QROOK] > 0) ||
                (board->player2 == ch && board->moved[WHITE_QROOK] > 0))
            {
                send_to_char("You cannot castle because you have moved your queenside rook before.\n\r", ch);
                return;
            }
            rooky = 0;
        }
        else
        {
            do_smaug_chess(ch, "castle");
            return;
        }

        if ((rooky == 7 &&
             (board->board[myx][6] != NO_PIECE ||
              board->board[myx][5] != NO_PIECE)) ||
            (rooky == 0 &&
             (board->board[myx][1] != NO_PIECE ||
              board->board[myx][2] != NO_PIECE ||
              board->board[myx][3] != NO_PIECE)))
        {
            send_to_char("There are pieces between the king and rook blocking your castle.\n\r", ch);
            return;
        }

        /* castling succeeded */
        if (board->board[myx][rooky] == BLACK_KROOK ||
            board->board[myx][rooky] == WHITE_KROOK)
        {
            kdy = 6;
            rdy = 5;
        }
        else if (board->board[myx][rooky] == BLACK_QROOK ||
                 board->board[myx][rooky] == WHITE_QROOK)
        {
            kdy = 2;
            rdy = 3;
        }
        else
        {
            bug("do_smaug_chess():chess.c board out of sync, game aborted, please contact maintainer");
            send_to_char("BUG in castling!  Aborting game!  Please contact an immortal.\n\r", ch);
            free_game(board);
            return;
        }

        /* check for 'move across check' rule */
        board->board[myx][rdy] = board->board[myx][4];
        board->board[myx][4] = NO_PIECE;

        if ( king_in_check(board,board->board[myx][rdy]) )
        {
            send_to_char("Your king would move across check if you castled.\n\r", ch);

            board->board[myx][4] = board->board[myx][rdy];
            board->board[myx][rdy] = NO_PIECE;
            return;
        }

        board->board[myx][kdy] = board->board[myx][rdy];
        board->board[myx][rdy] = board->board[myx][rooky];
        board->board[myx][rooky] = NO_PIECE;

        /* check for 'check' after castled */
        if ( king_in_check(board,board->board[myx][kdy]) )
        {
            send_to_char("Your king would be in check if you castled.\n\r", ch);

            board->board[myx][4] = board->board[myx][kdy];
            board->board[myx][kdy] = NO_PIECE;
            board->board[myx][rooky] = board->board[myx][rdy];
            board->board[myx][rdy] = NO_PIECE;
            return;
        }

        board->moved[board->board[myx][kdy]]++;
        board->moved[board->board[myx][rdy]]++;

	board_move_stuff(board);
	board_move_messages(ch, MOVE_CASTLE, rooky==7?"kingside":"queenside");
#ifdef USE_IMC
	/* TODO: IMC support for castle */
	if ( ch->pcdata->game_board->type == TYPE_IMC)
	{
	    sprintf(arg, "castle %s", rooky==7?"kingside":"queenside");
	    imc_send_chess((CHAR_DATA *)ch->pcdata->game_board->player1, (char *)ch->pcdata->game_board->player2, arg);
	}
#endif
    }
    else if ( !str_prefix(arg, "move") )
    {
        char extra[MAX_INPUT_LENGTH];
        char a,b;
        int x,y,dx,dy,ret;

        if ( !*argument )
        {
            send_to_char("Usage: " SC_COMMANDNAME " move <source> <destination>\n\r",ch);
            return;
        }

        if (sscanf(argument,"%c%d %c%d",&a,&y,&b,&dy)!=4)
        {
            send_to_char("Usage: " SC_COMMANDNAME " move <source> <destination>\n\r",ch);
            return;
        }
        argument = one_argument(argument, arg);
        argument = one_argument(argument, arg);

        if ( a < 'a' || a > 'h' || b < 'a' || b > 'h' || y < 1 || y > 8 || dy < 1 || dy > 8 )
        {
            send_to_char("Invalid move, use a-h and 1-8 (example: a4 b4).\n\r", ch);
            return;
        }

        x = a - 'a';
        dx = b - 'a';
        y--;
        dy--;

        extra[0] = '\0';

        ret = is_valid_move(ch,ch->pcdata->game_board,x,y,dx,dy);
        if (ret == MOVE_OK || ret == MOVE_TAKEN)
        {
            GAME_BOARD_DATA *board;
            int piece, destpiece;

            board = ch->pcdata->game_board;
            piece = board->board[x][y];
            destpiece = board->board[dx][dy];
            board->board[dx][dy] = piece;
            board->board[x][y] = NO_PIECE;
            if ( king_in_check(board,IS_WHITE(board->board[dx][dy])?WHITE_KING:BLACK_KING) )
            {
                board->board[dx][dy] = destpiece;
                board->board[x][y] = piece;
                ret = MOVE_INCHECK;
            }
            else
            {
                sprintf(extra, "%c%d (%s) to %c%d (%s)",
                        a, y+1, piece_names[piece],
                        b, dy+1, piece_names[destpiece]);

                board_move_stuff(board);
                board->lastx = dx;
                board->lasty = dy;
                board->moved[piece]++;
                board->moved[destpiece]++; /* taken pieces marked as moved */
#ifdef USE_IMC
                if ( ch->pcdata->game_board->type == TYPE_IMC)
                {
                    sprintf(arg, "move %d%d %d%d", x, y, dx, dy);
                    imc_send_chess((CHAR_DATA *)ch->pcdata->game_board->player1, (char *)ch->pcdata->game_board->player2, arg);
                }
#endif
            }
	    board_move_messages(ch, ret, extra);
        }

        if ( argument && *argument )
        {
            do_smaug_chess(ch, argument);
            return;
        }
        return;
    }

    do_smaug_chess(ch, "_syntax");
}

void init_chess(void)
{
#ifdef USE_IMC
    imc_register_packet_handler("chess", imc_recv_chess);
#endif
}
