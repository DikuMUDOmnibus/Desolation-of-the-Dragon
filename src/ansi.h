/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer and Heath Leach
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*
**	Ansi color Codes
**	DaleMUD 2.0, msw addtions
**	03/07/94
**
*/

#define CLEAR_CURSOR_TO_END    "\033[0K" /* Clear from cursor to end of line */
#define CLEAR_BEGIN_TO_CURSOR  "\033[1K" /* Clear from begin of line to cursor */
#define CLEAR_LINE             "\033[2K" /* Clear line containing cursor */
#define CLEAR_FROM_CURSOR      "\033[0J" /* Clear screen from cursor */
#define CLEAR_TO_CURSOR        "\033[1J" /* Clear screen to cursor */
#define CLEAR_SCREEN           "\033[2J" /* Clear entire screen */

#define ANSI_BEEP              "\007" /* Annoying ansi beep */
/*
 #define CURSOR_UP(number)      printf("\033[%dA", number)
 #define CURSOR_DOWN(number)    printf("\033[%dB", number)
 #define CURSOR_FORWARD(number) printf("\033[%dC", number)
 #define CURSOR_BACKWARD(number) printf("\033[%dD", number)
 #define CURSOR_POSITION(row, column) printf("\033[%d;%dH", row, column)
*/

#define MOD_NORMAL    "\033[0m"
#define MOD_BOLD      "\033[1m"
#define MOD_FAINT     "\033[2m"
#define MOD_UNDERLINE "\033[4m"
#define MOD_BLINK     "\033[5m"
#define MOD_REVERSE   "\033[7m"

#define FG_BLACK		"\033[0;0;30m"
#define FG_RED			"\033[0;0;31m"
#define FG_GREEN		"\033[0;0;32m"
#define FG_BROWN		"\033[0;0;33m"
#define FG_BLUE			"\033[0;0;34m"
#define FG_MAGENTA		"\033[0;0;35m"
#define FG_CYAN			"\033[0;0;36m"
#define FG_LT_GRAY		"\033[0;0;37m"
#define FG_DK_GRAY		"\033[0;1;30m"
#define FG_LT_RED		"\033[0;1;31m"
#define FG_LT_GREEN		"\033[0;1;32m"
#define FG_YELLOW		"\033[0;1;33m"
#define FG_LT_BLUE		"\033[0;1;34m"
#define FG_LT_MAGENTA		"\033[0;1;35m"
#define FG_LT_CYAN		"\033[0;1;36m"
#define FG_WHITE		"\033[0;1;37m"
#define FG_BLNK_BLACK		"\033[0;0;5;30m"
#define FG_BLNK_RED		"\033[0;0;5;31m"
#define FG_BLNK_GREEN		"\033[0;0;5;32m"
#define FG_BLNK_BROWN		"\033[0;0;5;33m"
#define FG_BLNK_BLUE		"\033[0;0;5;34m"
#define FG_BLNK_MAGENTA		"\033[0;0;5;35m"
#define FG_BLNK_CYAN		"\033[0;0;5;36m"
#define FG_BLNK_LT_GRAY		"\033[0;0;5;37m"
#define FG_BLNK_DK_GRAY		"\033[0;1;5;30m"
#define FG_BLNK_LT_RED		"\033[0;1;5;31m"
#define FG_BLNK_LT_GREEN	"\033[0;1;5;32m"
#define FG_BLNK_YELLOW		"\033[0;1;5;33m"
#define FG_BLNK_LT_BLUE		"\033[0;1;5;34m"
#define FG_BLNK_LT_MAGENTA	"\033[0;1;5;35m"
#define FG_BLNK_LT_CYAN		"\033[0;1;5;36m"
#define FG_BLNK_WHITE		"\033[0;1;5;37m"

#define BK_BLACK      "\033[0;40m"
#define BK_RED        "\033[0;41m"
#define BK_GREEN      "\033[0;42m"
#define BK_BROWN      "\033[0;43m"
#define BK_BLUE       "\033[0;44m"
#define BK_MAGENTA    "\033[0;45m"
#define BK_CYAN       "\033[0;46m"
#define BK_LT_GRAY    "\033[0;47m"

/*
 #define WINDOW(top, bottom) printf("\033[%d;%dr%s", top, bottom, CLEAR_SCREEN)
*/

