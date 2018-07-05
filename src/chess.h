/******************************************************
            Desolation of the Dragon MUD II
              (C) 1997-2003  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

typedef struct	game_board_data	GAME_BOARD_DATA;

void free_game( GAME_BOARD_DATA *board );
void init_chess( void );

#define NO_PIECE	0

#define WHITE_PAWN	1
#define WHITE_QROOK	2
#define WHITE_KROOK     3
#define WHITE_KNIGHT    4
#define WHITE_BISHOP    5
#define WHITE_QUEEN     6
#define WHITE_KING      7

#define BLACK_PAWN      8
#define BLACK_QROOK     9
#define BLACK_KROOK     10
#define BLACK_KNIGHT    11
#define BLACK_BISHOP	12
#define BLACK_QUEEN	13
#define BLACK_KING	14

#define MAX_PIECES	15

#define IS_WHITE(x) ((x)>=WHITE_PAWN && (x)<=WHITE_KING)
#define IS_BLACK(x) ((x)>=BLACK_PAWN && (x)<=BLACK_KING)

#define MOVE_OK		0
#define MOVE_INVALID	1
#define MOVE_BLOCKED	2
#define MOVE_TAKEN	3
#define MOVE_OFFBOARD	5
#define MOVE_SAMECOLOR	6
#define MOVE_CHECK	8
#define MOVE_WRONGCOLOR	9
#define MOVE_INCHECK	10
#define MOVE_CASTLE     11
#define MOVE_PROMOTE    12

#define TYPE_LOCAL	1
#define TYPE_IMC	2

struct game_board_data
{
    int board[8][8];
    int moved[MAX_PIECES];
    int moves;
    int type;
    void *player1;
    void *player2;
    void *turn;
    int lastx;
    int lasty;
};
