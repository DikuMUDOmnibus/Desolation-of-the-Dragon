/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG (C) 1994, 1995, 1996 by Derek Snider                 |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Narn, Scryn, Haus, Swordbearer,   |   / ' ' \   *
 * Altrag, Grishnakh and Tricops                              |~'~.VxvxV.~'~*
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *			    Main mud header file			    *
 ****************************************************************************/

#ifndef _MUD_H
#define _MUD_H
#endif

#include "global.h"
#include "material.h"

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdarg.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef TIME_WITH_SYS_TIME
#include <time.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if !defined(NOCRYPT) && defined(HAVE_CRYPT_H)
#include <crypt.h>
#endif

#ifdef USE_ASPELL
#include <aspell.h>
#endif

#ifndef MUD_LISTENER
#if defined(COMPRESS) && defined(HAVE_LIBZ)
#include <zlib.h>
#endif
#endif

/* #include <malloc_dbg.h> */

typedef	int				ch_ret;
typedef	int				obj_ret;

/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define const
#define args( list )			( )
#define DECLARE_DO_FUN( fun )		void fun( )
#define DECLARE_SPEC_FUN( fun )		bool fun( )
#define DECLARE_SPELL_FUN( fun )	ch_ret fun( )
#else
#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun
#endif

#define SPECIAL_FUNC(func) \
 bool (func)(void *proc, CMDTYPE *cmd, char *arg, CHAR_DATA *cmd_ch, sh_int type)
#define SFT_UPDATE	1
#define SFT_COMMAND	2
#define SFT_ARGUMENT	3
#define SFT_VIO_UPDATE  4
#define SFT_AGGR_UPDATE 5

/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

#if	!defined(BERR)
#define BERR	 255
#endif

#if	defined(_AIX)
#if	!defined(const)
#define const
#endif
typedef int				sh_int;
typedef int				bool;
#define unix
#else
typedef short    int			sh_int;
#ifndef __cplusplus
typedef unsigned char			bool;
#endif
#endif

#define FCLOSE(fp)  fclose(fp); fp=NULL;

/*
 * Structure types.
 */
typedef struct	affect_data		AFFECT_DATA;
typedef struct	area_data		AREA_DATA;
typedef struct  auction_data            AUCTION_DATA; /* auction data */
typedef struct	ban_data		BAN_DATA;
typedef struct	extracted_char_data	EXTRACT_CHAR_DATA;
typedef struct	char_data		CHAR_DATA;
typedef struct	hunt_hate_fear		HHF_DATA;
typedef struct	fighting_data		FIGHT_DATA;
typedef struct	descriptor_data		DESCRIPTOR_DATA;
typedef struct	exit_data		EXIT_DATA;
typedef struct	extra_descr_data	EXTRA_DESCR_DATA;
typedef struct	help_data		HELP_DATA;
typedef struct	menu_data		MENU_DATA;
typedef struct	mob_index_data		MOB_INDEX_DATA;
typedef struct	note_data		NOTE_DATA;
typedef struct	comment_data		COMMENT_DATA;
typedef struct	board_data		BOARD_DATA;
typedef struct	obj_data		OBJ_DATA;
typedef struct	obj_index_data		OBJ_INDEX_DATA;
typedef struct	pc_data			PC_DATA;
typedef struct	reset_data		RESET_DATA;
typedef struct	map_index_data		MAP_INDEX_DATA;   /* maps */
typedef struct	map_data		MAP_DATA;   /* maps */
typedef struct	room_index_data		ROOM_INDEX_DATA;
typedef struct	shop_data		SHOP_DATA;
typedef struct	repairshop_data		REPAIR_DATA;
typedef struct	time_info_data		TIME_INFO_DATA;
typedef struct	hour_min_sec		HOUR_MIN_SEC;
typedef struct	weather_data		WEATHER_DATA;
typedef	struct	clan_data		CLAN_DATA;
typedef struct  council_data 		COUNCIL_DATA;
typedef struct  tourney_data            TOURNEY_DATA;
typedef struct	mob_prog_data		MPROG_DATA;
typedef struct	mob_prog_act_list	MPROG_ACT_LIST;
typedef	struct	editor_data		EDITOR_DATA;
typedef struct	teleport_data		TELEPORT_DATA;
typedef struct	river_data		RIVER_DATA;
typedef struct	timer_data		TIMER;
typedef struct  godlist_data		GOD_DATA;
typedef struct	system_data		SYSTEM_DATA;
typedef struct	stats_data              STATS_DATA;
typedef	struct	smaug_affect		SMAUG_AFF;
typedef struct  who_data                WHO_DATA;
typedef	struct	skill_type		SKILLTYPE;
typedef	struct	social_type		SOCIALTYPE;
typedef	struct	cmd_type		CMDTYPE;
typedef	struct	alias_type		ALIAS_DATA;
typedef	struct	killed_data		KILLED_DATA;
typedef struct  deity_data		DEITY_DATA;
typedef struct	wizent			WIZENT;
typedef struct	neighbor_data		NEIGHBOR_DATA; /* FB */
typedef struct	extended_bitvector	EXT_BV;
typedef struct  currency_data           CURRENCY_DATA;
typedef struct  currency_index_data     CURR_INDEX_DATA;
typedef struct  christen_data           CHRISTEN_DATA;
typedef struct  dict_entry              DICT_ENTRY;
typedef	struct	lcnv_data		LCNV_DATA;
typedef	struct	lang_data		LANG_DATA;

#include "intro.h"
#include "chess.h"
#include "variables.h"

#define VTRACK 1
#ifdef VTRACK
#include "vtrack.h"
#endif

/*
 * Function types.
 */
typedef	void	DO_FUN		args( ( CHAR_DATA *ch, char *argument ) );
typedef bool	SPEC_FUN \
args( ( void *proc, CMDTYPE *cmd, char *argument, CHAR_DATA *cmd_ch, sh_int type ) );
typedef ch_ret	SPELL_FUN	args( ( int sn, int level, CHAR_DATA *ch, void *vo ) );

#define DUR_CONV	23.333333333333333333333333
#define HIDDEN_TILDE	'*'

#define BV00		(1 <<  0)
#define BV01		(1 <<  1)
#define BV02		(1 <<  2)
#define BV03		(1 <<  3)
#define BV04		(1 <<  4)
#define BV05		(1 <<  5)
#define BV06		(1 <<  6)
#define BV07		(1 <<  7)
#define BV08		(1 <<  8)
#define BV09		(1 <<  9)
#define BV10		(1 << 10)
#define BV11		(1 << 11)
#define BV12		(1 << 12)
#define BV13		(1 << 13)
#define BV14		(1 << 14)
#define BV15		(1 << 15)
#define BV16		(1 << 16)
#define BV17		(1 << 17)
#define BV18		(1 << 18)
#define BV19		(1 << 19)
#define BV20		(1 << 20)
#define BV21		(1 << 21)
#define BV22		(1 << 22)
#define BV23		(1 << 23)
#define BV24		(1 << 24)
#define BV25		(1 << 25)
#define BV26		(1 << 26)
#define BV27		(1 << 27)
#define BV28		(1 << 28)
#define BV29		(1 << 29)
#define BV30		(1 << 30)
#define BV31		(1 << 31)

/*
 * String and memory management parameters.
 */

#define MAX_KEY_HASH		 2048
#define MAX_STRING_LENGTH	 4096  /* buf */
#define MAX_INPUT_LENGTH	 1024  /* arg */
#define MAX_INBUF_SIZE		 1024
#ifndef MUD_LISTENER
#ifdef COMPRESS
#define COMPRESS_BUF_SIZE        1024
#endif
#endif

#define HASHSTR			 /* use string hashing */

#define	MAX_LAYERS		 8	/* maximum clothing layers */
#define MAX_NEST	       100	/* maximum container nesting */

#define MAX_KILLTRACK		25	/* track mob vnums killed */

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MUD_NAME		"Desolation of the Dragon II"

#define MIN_OBJ_RENT		15000

#define MAX_EXP_WORTH	       500000
#define MIN_EXP_WORTH		   20

#define MAX_SKILL		  400
#define MAX_STATS		    7
#define MAX_NPC_CLASS		   26
#define MAX_LEVEL		   75
#define MAX_CLAN		   50
#define MAX_DEITY		   50
#define MAX_CPD			    4   /* Maximum council power level difference */
#define	MAX_HERB		   20

#define LEVEL_SUPREME		   MAX_LEVEL
#define LEVEL_INFINITE		   (MAX_LEVEL - 1)
#define LEVEL_ETERNAL		   (MAX_LEVEL - 2)
#define LEVEL_IMPLEMENTOR	   (MAX_LEVEL - 3)
#define LEVEL_SUB_IMPLEM	   (MAX_LEVEL - 4)
#define LEVEL_ASCENDANT		   (MAX_LEVEL - 5)
#define LEVEL_GREATER		   (MAX_LEVEL - 6)
#define LEVEL_GOD		   (MAX_LEVEL - 7)
#define LEVEL_LESSER		   (MAX_LEVEL - 8)
#define LEVEL_TRUEIMM		   (MAX_LEVEL - 9)
#define LEVEL_DEMI		   (MAX_LEVEL - 10)
#define LEVEL_SAVIOR		   (MAX_LEVEL - 11)
#define LEVEL_CREATOR		   (MAX_LEVEL - 12)
#define LEVEL_ACOLYTE		   (MAX_LEVEL - 13)
#define LEVEL_NEOPHYTE		   (MAX_LEVEL - 14)
#define LEVEL_IMMORTAL		   (MAX_LEVEL - 14)

#define LEVEL_AVATAR		   (MAX_LEVEL - 15)
#define LEVEL_HERO		   (MAX_LEVEL - 15)

#define LEVEL_LOG		    LEVEL_ASCENDANT
#define LEVEL_HIGOD		    LEVEL_GOD

#define LEVEL_LOG_CSET              999

#define PULSE_PER_SECOND            20 /* make sure this is a multiple of 4 for WAIT_STATE */
#define PULSE_VIOLENCE		  ( 3 * PULSE_PER_SECOND)
#define PULSE_SPELL		  PULSE_VIOLENCE
#define PULSE_MOBILE		  ( 4 * PULSE_PER_SECOND)
#define PULSE_TICK		  (70 * PULSE_PER_SECOND)
#define PULSE_AREA		  (60 * PULSE_PER_SECOND)
#define PULSE_AUCTION             (10 * PULSE_PER_SECOND)

#define SPELL_BEATS_PER_ROUND       12 /* do not change without updating skills */


/* player aging stuff */
#define SECS_PER_REAL_HOUR	(60 * 60)

#define SECS_PER_MUD_YEAR	(8 * SECS_PER_REAL_HOUR)
/* 1 mud year = 8 real hours */
#define SECS_PER_MUD_MONTH	(SECS_PER_MUD_YEAR / 17)	/* 1694 */
/* 1 mud month = 28 real minutes */
#define SECS_PER_MUD_DAY	(SECS_PER_MUD_MONTH / 30)	/* 56.5 */
/* 1 mud day = .94 real minutes */
#define SECS_PER_MUD_HOUR	(SECS_PER_MUD_DAY / 24)		/* 2.35 */
/* 1 mud hour = 2.35 real seconds */

/*
 * Stuff for area versions --Shaddai
 */
#define HAS_SPELL_INDEX     -1
#define AREA_VERSION_WRITE 1

/*
 * Command logging types.
 */
typedef enum
{
  LOG_NORMAL, LOG_ALWAYS, LOG_NEVER, LOG_BUILD, LOG_HIGH, LOG_COMM,
  LOG_PC, LOG_HTTPD, LOG_IMC, LOG_IMCDEBUG, LOG_MONITOR, LOG_BUG,
  LOG_DEBUG, LOG_MAGIC, LOG_ALL, LOG_IRC, LOG_LAST
} log_types;
#define LOG_I3 LOG_IMC

/*
 * Log/channel severity
 */
#define SEV_EMERG                  0
#define SEV_ALERT                  10
#define SEV_CRIT                   20
#define SEV_ERR                    30
#define SEV_WARN                   40
#define SEV_NOTICE                 50
#define SEV_INFO                   60
#define SEV_DEBUG                  70
#define SEV_TREET                  80
#define SEV_SPAM                   90
#define SEV_DEFAULT               100


/*
 * Return types for move_char, damage, greet_trigger, etc, etc
 * Added by Thoric to get rid of bugs
 */

typedef enum
{
  rNONE, rCHAR_DIED, rVICT_DIED, rBOTH_DIED, rCHAR_QUIT, rVICT_QUIT,
  rBOTH_QUIT, rSPELL_FAILED,
  rOBJ_SCRAPPED,
  rCHAR_IMMUNE, rVICT_IMMUNE,
  rCHAR_AND_OBJ_EXTRACTED = 128, rERROR = 255, rSTOP
} ret_types;

/* Echo types for echo_to_all */
#define ECHOTAR_ALL	0
#define ECHOTAR_PC	1
#define ECHOTAR_IMM	2
#define ECHOTAR_OUTSIDE 3

/* defines for new do_who */
#define WT_MORTAL       0
#define WT_DEADLY       1
#define WT_IMM          2
#define WT_GROUPED	3
#define WT_GROUPWHO	4

/*
 * Defines for extended bitvectors
 */
#ifndef INTBITS
  #define INTBITS	32
#endif
#define XBM		31	/* extended bitmask   ( INTBITS - 1 )	*/
#define RSV		5	/* right-shift value  ( sqrt(XBM+1) )	*/
#define XBI		4	/* integers in an extended bitvector	*/
#define MAX_BITS	XBI * INTBITS
/*
 * Structure for extended bitvectors -- Thoric
 */
struct extended_bitvector
{
    int		bits[XBI];
};

#ifdef I3
#include "i3.h"
#endif

#ifdef IMC
#include "imc.h"
#endif

/*
 * Tongues / Languages structures
 */

struct lcnv_data
{
    LCNV_DATA *		next;
    LCNV_DATA *		prev;
    char *		old;
    int			olen;
    char *		newstr;
    int			nlen;
};

struct lang_data
{
    LANG_DATA *		next;
    LANG_DATA *		prev;
    char *		name;
    LCNV_DATA *		first_precnv;
    LCNV_DATA *		last_precnv;
    char *		alphabet;
    LCNV_DATA *		first_cnv;
    LCNV_DATA *		last_cnv;
};

/*
 * do_who output structure -- Narn
 */
struct who_data
{
  WHO_DATA *prev;
  WHO_DATA *next;
  char *text;
  int  type;
};

/*
 * Site ban structure.
 */
struct	ban_data
{
    BAN_DATA *	next;
    BAN_DATA *	prev;
    char *	name;
    int		level;
    char *	ban_time;
};

/*
 * Currency stuff
 */
typedef enum
{
    CURR_NONE=0, CURR_GOLD, CURR_SILVER, CURR_BRONZE, CURR_MITHRIL,
    MAX_CURR_TYPE
} currency_types;
#define FIRST_CURR   CURR_NONE+1
#define LAST_CURR    MAX_CURR_TYPE-1
#define DEFAULT_CURR CURR_GOLD

#define GET_MONEY(ch,type)   ((ch)->money[(type)])
#define GET_BALANCE(ch,type) ((ch)->balance[(type)])
/*#define GET_GOLD(ch)         (GET_MONEY((ch),CURR_GOLD))*/

/*
 * Time and weather stuff.
 */
typedef enum
{
  SUN_DARK, SUN_RISE, SUN_LIGHT, SUN_SET
} sun_positions;

typedef enum
{
  SKY_CLOUDLESS, SKY_CLOUDY, SKY_RAINING, SKY_LIGHTNING
} sky_conditions;

typedef enum
{
    TEMP_COLD, TEMP_COOL, TEMP_NORMAL, TEMP_WARM, TEMP_HOT
} temp_conditions;

typedef enum
{
    PRECIP_ARID, PRECIP_DRY, PRECIP_NORMAL, PRECIP_DAMP, PRECIP_WET
} precip_conditions;

typedef enum
{
    WIND_STILL, WIND_CALM, WIND_NORMAL, WIND_BREEZY, WIND_WINDY
} wind_conditions;

#define GET_TEMP_UNIT(weather)   ((weather->temp + 3*weath_unit - 1)/weath_unit)
#define GET_PRECIP_UNIT(weather) ((weather->precip + 3*weath_unit - 1)/weath_unit)
#define GET_WIND_UNIT(weather)   ((weather->wind + 3*weath_unit - 1)/weath_unit)

#define IS_RAINING(weather)      (GET_PRECIP_UNIT(weather)>PRECIP_NORMAL)
#define IS_WINDY(weather)        (GET_WIND_UNIT(weather)>WIND_NORMAL)
#define IS_CLOUDY(weather)       (GET_PRECIP_UNIT(weather)>1)
#define IS_TCOLD(weather)        (GET_TEMP_UNIT(weather)==TEMP_COLD)
#define IS_COOL(weather)         (GET_TEMP_UNIT(weather)==TEMP_COOL)
#define IS_HOT(weather)          (GET_TEMP_UNIT(weather)==TEMP_HOT)
#define IS_WARM(weather)         (GET_TEMP_UNIT(weather)==TEMP_WARM)
#define IS_SNOWING(weather)      (IS_RAINING(weather) && IS_COOL(weather))

struct	time_info_data
{
    int		hour;
    int		day;
    int		month;
    int		year;
    int		sunlight;
};

struct hour_min_sec
{
  int hour;
  int min;
  int sec;
  int manual;
};

#define MAX_CLIMATE	5

struct	weather_data
{
    int 		temp;		/* temperature */
    int			precip;		/* precipitation */
    int			wind;		/* umm... wind */
    int			temp_vector;	/* vectors controlling */
    int			precip_vector;	/* rate of change */
    int			wind_vector;
    int			climate_temp;	/* climate of the area */
    int			climate_precip;
    int			climate_wind;
    NEIGHBOR_DATA *	first_neighbor;	/* areas which affect weather sys */
    NEIGHBOR_DATA *	last_neighbor;
    char *		echo;		/* echo string */
    int			echo_color;	/* color for the echo */
};

struct neighbor_data
{
    NEIGHBOR_DATA *next;
    NEIGHBOR_DATA *prev;
    char *name;
    AREA_DATA *address;
};


/*
 * Structure used to build wizlist
 */
struct	wizent
{
    WIZENT *		next;
    WIZENT *		last;
    char *		name;
    sh_int		level;
};


/*
 * Connected state for a channel.
 */
typedef enum {
    CON_INVALID = -9999,
    CON_GET_NAME = -99,
    CON_GET_OLD_PASSWORD,
    CON_CONFIRM_NEW_NAME,
    CON_GET_NEW_PASSWORD,
    CON_CONFIRM_NEW_PASSWORD,
    CON_GET_NEW_RACE,
    CON_GET_PLANAR,
    CON_GET_NEW_SEX,
    CON_GET_NEW_CLASS,
    CON_SIMPLE_ADVANCED,
    CON_GET_START_TOWN,
    CON_GET_NEW_SCORES,
    CON_GET_DEADLY,
    CON_GET_WANT_RIPANSI,
    CON_GET_INTERFACE,
    CON_PRESS_ENTER,
    CON_READ_MOTD,
    CON_WAIT_1,
    CON_WAIT_2,
    CON_WAIT_3,
    CON_ACCEPTED,
    CON_COPYOVER_RECOVER,
    CON_PLAYING = 0,
    CON_EDITING
} connection_types;

/*
 * Character substates
 */
typedef enum
{
  SUB_NONE, SUB_PAUSE, SUB_PERSONAL_DESC, SUB_OBJ_SHORT, SUB_OBJ_LONG,
  SUB_OBJ_EXTRA, SUB_MOB_LONG, SUB_MOB_DESC, SUB_ROOM_DESC, SUB_ROOM_EXTRA,
  SUB_ROOM_EXIT_DESC, SUB_WRITING_NOTE, SUB_MPROG_EDIT, SUB_HELP_EDIT,
  SUB_WRITING_MAP, SUB_PERSONAL_BIO, SUB_REPEATCMD, SUB_RESTRICTED,
  SUB_DEITYDESC,
  /* timer types ONLY below this point */
  SUB_TIMER_DO_ABORT = 128, SUB_TIMER_CANT_ABORT
} char_substates;

/*
 * Descriptor (channel) structure.
 */
struct	descriptor_data
{
    DESCRIPTOR_DATA *	next;
    DESCRIPTOR_DATA *	prev;
    DESCRIPTOR_DATA *	snoop_by;
    CHAR_DATA *		character;
    CHAR_DATA *		original;
    char *		host;
#define DESCRIPTOR_HAS_SIN_ADDR 1
#ifdef  DESCRIPTOR_HAS_SIN_ADDR
    struct in_addr      sin_addr;
#endif
    int			port;
#ifndef MUD_LISTENER
    int			descriptor;
#endif
    int          	connected;
    int	        	idle;
    sh_int		lines;
    sh_int		scrlen;
    bool		fcommand;
    char		inbuf		[MAX_INBUF_SIZE];
    char		incomm		[MAX_INPUT_LENGTH];
    char		inlast		[MAX_INPUT_LENGTH];
    int			repeat;
    char *		outbuf;
    unsigned long	outsize;
    int			outtop;
    char *		pagebuf;
    unsigned long	pagesize;
    int			pagetop;
    char *		pagepoint;
    char		pagecmd;
    char		pagecolor;
    int			auth_inc;
    int			auth_state;
    char		abuf[ 256 ];
    int			auth_fd;
    char *		user;
    int 		atimes;
    int			newstate;
    unsigned char	prevcolor;
#ifndef MUD_LISTENER
#ifdef COMPRESS
    unsigned char       compressing;
    z_stream *          out_compress;
    unsigned char *     out_compress_buf;
#endif
#endif
#ifdef MXP
    bool                mxp_detected;
#endif
#ifdef MSP
    bool                msp_detected;
#endif
    unsigned int	conn_id;
#ifdef MUD_LISTENER
    unsigned int        uid;
#endif
};


/*
 * Attribute bonus structures.
 */
struct	str_app_type
{
    sh_int	tohit;
    sh_int	todam;
    sh_int	carry;
    sh_int	wield;
};

struct	int_app_type
{
    sh_int	learn;
};

struct	wis_app_type
{
    sh_int	practice;
};

struct	dex_skill_type
{
    sh_int	p_pocket;
    sh_int	p_locks;
    sh_int	traps;
    sh_int	sneak;
    sh_int	hide;
};

struct	dex_app_type
{
    sh_int	reaction;
    sh_int	miss_att;
    sh_int	defensive;
};

struct	con_app_type
{
    sh_int	hitp;
    sh_int	shock;
};

struct	cha_app_type
{
    sh_int	charm;
    sh_int	reaction;
};

struct  lck_app_type
{
    sh_int	luck;
};

/* the old races */
/*
#define RACE_HUMAN	    0
#define RACE_ELF            1
#define RACE_DWARF          2
#define RACE_HALFLING       3
#define RACE_PIXIE          4
#define RACE_VAMPIRE        5
#define RACE_HALF_OGRE      6
#define RACE_HALF_ORC       7
#define RACE_HALF_TROLL     8
#define RACE_HALF_ELF       9
#define RACE_GITH           10
#define	RACE_DRAGON	    31
*/

#define RACE_HALFBREED 0
#define RACE_HUMAN     1
#define RACE_ELF       2
#define RACE_ELVEN     2
#define RACE_DWARF     3
#define RACE_HALFLING  4
#define RACE_GNOME     5
#define RACE_REPTILE  6
#define RACE_SPECIAL  7
#define RACE_LYCANTH  8
#define RACE_DRAGON   9
#define RACE_UNDEAD   10
#define RACE_ORC      11
#define RACE_INSECT   12
#define RACE_ARACHNID 13
#define RACE_DINOSAUR 14
#define RACE_FISH     15
#define RACE_BIRD     16
#define RACE_GIANT    17	/* generic giant more specials down ---V */
#define RACE_PREDATOR 18
#define RACE_PARASITE 19
#define RACE_SLIME    20
#define RACE_DEMON    21
#define RACE_SNAKE    22
#define RACE_HERBIV   23
#define RACE_TREE     24
#define RACE_VEGGIE   25
#define RACE_ELEMENT  26
#define RACE_PLANAR   27
#define RACE_DEVIL    28
#define RACE_GHOST    29
#define RACE_GOBLIN   30
#define RACE_TROLL    31
#define RACE_VEGMAN   32
#define RACE_MFLAYER  33
#define RACE_PRIMATE  34
#define RACE_ENFAN    35
#define RACE_DROW     36
#define RACE_GOLEM    37
#define RACE_SKEXIE   38
#define RACE_TROGMAN  39
#define RACE_PATRYN   40
#define RACE_LABRAT   41
#define RACE_SARTAN   42
#define RACE_TYTAN    43
#define RACE_SMURF    44
#define RACE_ROO      45
#define RACE_HORSE    46
#define RACE_DRAAGDIM 47
#define RACE_ASTRAL   48
#define RACE_GOD      49

#define RACE_GIANT_HILL   50
#define RACE_GIANT_FROST  51
#define RACE_GIANT_FIRE   52
#define RACE_GIANT_CLOUD  53
#define RACE_GIANT_STORM  54
#define RACE_GIANT_STONE  55

#define RACE_DRAGON_RED    56
#define RACE_DRAGON_BLACK  57
#define RACE_DRAGON_GREEN  58
#define RACE_DRAGON_WHITE  59
#define RACE_DRAGON_BLUE   60
#define RACE_DRAGON_SILVER 61
#define RACE_DRAGON_GOLD   62
#define RACE_DRAGON_BRONZE 63
#define RACE_DRAGON_COPPER 64
#define RACE_DRAGON_BRASS  65

#define RACE_VAMPIRE		66
#define RACE_UNDEAD_VAMPIRE	66
#define RACE_LICH		67
#define RACE_UNDEAD_LICH	67
#define RACE_WIGHT		68
#define RACE_UNDEAD_WIGHT	68
#define RACE_GHAST		69
#define RACE_UNDEAD_GHAST	69
#define RACE_SPECTRE		70
#define RACE_UNDEAD_SPECTRE	70
#define RACE_ZOMBIE		71
#define RACE_UNDEAD_ZOMBIE	71
#define RACE_SKELETON		72
#define RACE_UNDEAD_SKELETON	72
#define RACE_GHOUL		73
#define RACE_UNDEAD_GHOUL	73

/* a few pc races */
#define RACE_HALF_ELF     74
#define RACE_HALF_ELVEN   74
#define RACE_HALF_OGRE    75
#define RACE_HALF_ORC     76
#define RACE_HALF_GIANT   77
/* end pc */

#define RACE_LIZARDMAN 78

/* evil pc's */
#define RACE_DARK_DWARF 79
#define RACE_DEEP_GNOME 80
/* end evil */

#define RACE_GNOLL	81

#define RACE_GOLD_ELF	82
#define RACE_GOLD_ELVEN	82
#define RACE_WILD_ELF	83
#define RACE_WILD_ELVEN	83
#define RACE_SEA_ELF	84
#define RACE_SEA_ELVEN	84

/* 10-20-96 Admiral */
#define RACE_TIEFLING   85
#define RACE_AASIMAR    86
#define RACE_SOLAR      87
#define RACE_PLANITAR   88
#define RACE_UNDEAD_SHADOW  89
#define RACE_GIANT_SKELETON 90
#define RACE_NILBOG         91
#define RACE_HOUSERS        92
#define RACE_BAKU           93
#define RACE_BEASTLORD      94
#define RACE_DEVAS          95
#define RACE_POLARIS        96
#define RACE_DEMODAND       97
#define RACE_TARASQUE       98
#define RACE_DIETY          99
#define RACE_DAEMON         100
#define RACE_VAGABOND       101
#define RACE_POKEMON        102

#define RACE_GITHZERAI      103
#define RACE_GITHYANKI      104
#define RACE_BARIAUR        105
#define RACE_MODRON         106
#define RACE_DABUS          107
#define RACE_CRANIUM_RAT    108

#define MAX_RACE            109

typedef enum
{
    CLASS_NONE = -1, CLASS_MAGE, CLASS_CLERIC, CLASS_THIEF, CLASS_WARRIOR,
    CLASS_VAMPIRE, CLASS_DRUID, CLASS_RANGER, CLASS_AMAZON, CLASS_PALADIN,
    CLASS_BARBARIAN, CLASS_PSIONIST, CLASS_ARTIFICER, CLASS_MONK,
    CLASS_NECROMANCER, CLASS_ANTIPALADIN, CLASS_SORCERER, LAST_CLASS
} class_index;

#define FIRST_CLASS             CLASS_MAGE
#define REAL_MAX_CLASS          LAST_CLASS
#ifndef MAX_CLASS
#define MAX_CLASS               LAST_CLASS
#endif

/*
 * Multiclassing Stuff -- Heath multclas.c
 */
int	GetClassLevel		args( ( CHAR_DATA *ch, sh_int cl ) );
bool	OnlyClass		args( ( CHAR_DATA *ch, sh_int cl ) );
int	HowManyClasses		args( ( CHAR_DATA *ch ) );
int	HowManyClassesPlus	args( ( CHAR_DATA *ch ) );
int	BestFightingClass	args( ( CHAR_DATA *ch ) );
int	BestThiefClass		args( ( CHAR_DATA *ch ) );
int	BestMagicClass		args( ( CHAR_DATA *ch ) );
int	GetALevel		args( ( CHAR_DATA *ch, int which ) );
int	GetSecMaxLev		args( ( CHAR_DATA *ch ) );
int	GetThirdMaxLev		args( ( CHAR_DATA *ch ) );
int	GetMaxLevel		args( ( CHAR_DATA *ch ) );
int	GetMinLevel		args( ( CHAR_DATA *ch ) );
int	MIGetMaxLevel		args( ( MOB_INDEX_DATA *ch ) );
int	GetMaxClass		args( ( CHAR_DATA *ch ) );
int	GetAveLevel		args( ( CHAR_DATA *ch ) );
int	GetTotLevel		args( ( CHAR_DATA *ch ) );
void	StartLevels		args( ( CHAR_DATA *ch ) );
char	*GetLevelString		args( ( CHAR_DATA *ch ) );
char	*GetClassString		args( ( CHAR_DATA *ch ) );
char	*GetTitleString		args( ( CHAR_DATA *ch ) );
bool	ShouldSaveSkill		args( ( CHAR_DATA *ch, int skill ) );
bool	CanUseSkill		args( ( CHAR_DATA *ch, int skill ) );
bool	CanUseSkillClass       	args( ( CHAR_DATA *ch, int skill, sh_int cl ) );
int	LowSkLv			args( ( CHAR_DATA *ch, int skill ) );
int	LowSkCl			args( ( CHAR_DATA *ch, int skill ) );
sh_int BestSkCl		args( ( CHAR_DATA *ch, int skill ) );
int	BestSkLv		args( ( CHAR_DATA *ch, int skill ) );
sh_int FirstActive		args( ( CHAR_DATA *ch ) );
sh_int MIFirstActive      	args( ( MOB_INDEX_DATA *ch ) );
void	ClassSpecificStuff	args( ( CHAR_DATA *ch ) );

int     total_memorized         args( ( CHAR_DATA *ch ) );
void    forget_spell            args( ( CHAR_DATA *ch, int sn ) );
int     max_can_memorize        args( ( CHAR_DATA *ch ) );
int     max_can_memorize_spell  args( ( CHAR_DATA *ch, int sn ) );
bool    can_gain_level          args( ( CHAR_DATA *ch, sh_int cl ) );
int     get_numattacks          args( ( CHAR_DATA *ch ) );


/*
 * Class status -- Heath
 */

#define STAT_ACTCLASS    BV00
#define STAT_OLDCLASS    BV01
#define STAT_FALLEN      BV02

#define IS_FALLEN(ch, i)        (IS_SET((ch)->classes[(i)], STAT_FALLEN))
#define SET_FALLEN(ch, i)	(SET_BIT((ch)->classes[(i)], STAT_FALLEN))
#define IS_ACTIVE(ch, i)        (!IS_FALLEN((ch), (i)) && IS_SET((ch)->classes[(i)], STAT_ACTCLASS))
#define SET_ACTIVE(ch, i)	(SET_BIT((ch)->classes[(i)], STAT_ACTCLASS))
#define HAD_CLASS(ch, i)        (IS_SET((ch)->classes[(i)], STAT_OLDCLASS))
#define HAS_CLASS(ch, i)        ((ch)->classes[(i)] != 0)
#define GET_LEVEL(ch, i)        ((ch)->levels[(i)])


/*
 * Practice stuff -- Jesse
 */

#define PRAC_OK			1
#define PRAC_NOCLASS		-1
#define PRAC_NOADEPT		-2
#define PRAC_NOTEACHER		-3
#define PRAC_NOLEVEL		-4
#define PRAC_NOMOBLEVEL		-5
#define PRAC_NOGUILD		-6
#define PRAC_NOSECRET		-7
#define PRAC_NOACTPRAC		-8
#define PRAC_OTHER		-9

sh_int	can_teach_skill( CHAR_DATA *ch, int sn );
sh_int	can_learn_skill( CHAR_DATA *ch, int sn );

/*
 * Interface stuff -- Heath
 */
#define INT_DALE	0
#define INT_MERC	1
#define INT_SMAUG	2
#define INT_DIKU	3
#define INT_DIKUII	4
#define INT_ENVY	5
#define INT_ROM		6
#define INT_IMP         7

#define INT_DEFAULT      INT_DALE

void imp_who		( CHAR_DATA *ch, char *argument );
char *dale_buffered_who	( CHAR_DATA *ch, char *argument );
char *how_good(int percent);
void dale_who		( CHAR_DATA *ch, char *argument );
void dale_score		( CHAR_DATA *ch, char *argument );
void dale_attrib	( CHAR_DATA *ch, char *argument );
void dale_group		( CHAR_DATA *ch, char *argument );
void dale_prac_output   ( CHAR_DATA *ch, CHAR_DATA *is_at_gm );
void smaug_who		( CHAR_DATA *ch, char *argument );
void smaug_score	( CHAR_DATA *ch, char *argument );
void smaug_group	( CHAR_DATA *ch, char *argument );
void smaug_prac_output  ( CHAR_DATA *ch, CHAR_DATA *is_at_gm );
void envy_who		( CHAR_DATA *ch, char *argument );
void envy_score		( CHAR_DATA *ch, char *argument );

/*
 * Languages -- Altrag
 */
#define LANG_COMMON      BV00  /* Human base language */
#define LANG_ELVEN       BV01  /* Elven base language */
#define LANG_DWARVEN     BV02  /* Dwarven base language */
#define LANG_PIXIE       BV03  /* Pixie/Fairy base language */
#define LANG_OGRE        BV04  /* Ogre base language */
#define LANG_ORCISH      BV05  /* Orc base language */
#define LANG_TROLLISH    BV06  /* Troll base language */
#define LANG_RODENT      BV07  /* Small mammals */
#define LANG_INSECTOID   BV08  /* Insects */
#define LANG_MAMMAL      BV09  /* Larger mammals */
#define LANG_REPTILE     BV10  /* Small reptiles */
#define LANG_DRAGON      BV11  /* Large reptiles, Dragons */
#define LANG_SPIRITUAL   BV12  /* Necromancers or undeads/spectres */
#define LANG_MAGICAL     BV13  /* Spells maybe?  Magical creatures */
#define LANG_GOBLIN      BV14  /* Goblin base language */
#define LANG_GOD         BV15  /* Clerics possibly?  God creatures */
#define LANG_ANCIENT     BV16  /* Prelude to a glyph read skill? */
#define LANG_HALFLING    BV17  /* Halfling base language */
#define LANG_CLAN	 BV18  /* Clan language */
#define LANG_GITH	 BV19  /* Gith Language */
#define LANG_GNOMISH     BV20
#define LANG_MINDFLAYER  BV21
#define LANG_AARAKOCRA   BV22
#define LANG_GIANTISH    BV23

#define LANG_UNKNOWN        0  /* Anything that doesnt fit a category */
#define VALID_LANGS    ( LANG_COMMON | LANG_ELVEN | LANG_DWARVEN | LANG_PIXIE  \
    | LANG_OGRE | LANG_ORCISH | LANG_TROLLISH | LANG_GOBLIN \
    | LANG_HALFLING | LANG_GITH | LANG_GNOMISH | LANG_MINDFLAYER \
    | LANG_AARAKOCRA | LANG_GIANTISH)

/* 18 Languages */

/*
 * TO types for act.
 */
#define TO_ROOM		    0
#define TO_NOTVICT	    1
#define TO_VICT		    2
#define TO_CHAR		    3
#define TO_THIRD            4

/*
 * Ansi parsing stuff - Heath
 */

char *ParseAnsiColors(int UsingAnsi, char *txt);

/*
 * Real action "TYPES" for act.
 */
#define DAT_BLACK	    	0
#define DAT_BLOOD	    	1
#define DAT_DGREEN          	2
#define DAT_ORANGE	    	3
#define DAT_DBLUE	    	4
#define DAT_PURPLE	    	5
#define DAT_CYAN	  	6
#define DAT_GREY		7
#define DAT_DGREY	    	8
#define DAT_RED		    	9
#define DAT_GREEN	   	10
#define DAT_YELLOW	   	11
#define DAT_BLUE		12
#define DAT_PINK		13
#define DAT_LBLUE	   	14
#define DAT_WHITE	   	15
#define DAT_BLINK	   	16
#define DAT_PLAIN	   	DAT_GREY
#define DAT_ACTION	   	DAT_GREY
#define DAT_SAY		   	DAT_WHITE
#define DAT_GOSSIP	   	DAT_YELLOW
#define DAT_OOC		        DAT_WHITE
#define DAT_TELL		DAT_PURPLE
#define DAT_HIT		  	DAT_GREY
#define DAT_HITME	  	DAT_GREY
#define DAT_IMMORT	   	DAT_YELLOW
#define DAT_HURT		DAT_PINK
#define DAT_FALLING	   	DAT_GREY
#define DAT_DANGER	   	DAT_RED
#define DAT_MAGIC	   	DAT_WHITE
#define DAT_CONSIDER	   	DAT_GREY
#define DAT_REPORT	   	DAT_LBLUE
#define DAT_POISON	   	DAT_GREY
#define DAT_SOCIAL	   	DAT_GREY
#define DAT_DYING	   	DAT_RED
#define DAT_DEAD		DAT_DGREY
#define DAT_SKILL	   	DAT_GREY
#define DAT_CARNAGE	   	DAT_BLOOD
#define DAT_DAMAGE	   	DAT_GREY
#define DAT_FLEE		DAT_GREY
#define DAT_RMNAME	  	DAT_GREY
#define DAT_RMDESC	   	DAT_GREY
#define DAT_OBJECT	   	DAT_GREY
#define DAT_PERSON	   	DAT_GREY
#define DAT_LIST		DAT_GREY
#define DAT_BYE		   	DAT_GREY
#define DAT_GOLD		DAT_YELLOW
#define DAT_GTELL	   	DAT_DBLUE
#define DAT_NOTE		DAT_GREY
#define DAT_HUNGRY	   	DAT_WHITE
#define DAT_THIRSTY	   	DAT_WHITE
#define	DAT_FIRE		DAT_BLOOD
#define DAT_SOBER	   	DAT_GREY
#define DAT_WEAROFF	   	DAT_GREY
#define DAT_SCORE	   	DAT_PURPLE
#define DAT_SCORE2          	DAT_WHITE
#define DAT_SCORE3          	DAT_YELLOW
#define DAT_SCORE4          	DAT_LBLUE
#define DAT_RESET	   	DAT_GREY
#define DAT_LOG		   	DAT_GREY
#define DAT_DIEMSG	   	DAT_BLOOD
#define DAT_WARTALK		DAT_BLOOD
#define DAT_WHO                 DAT_GREY
#define DAT_WHO2                DAT_DGREY
#define DAT_WHO3                DAT_BLUE
#define DAT_WHO4                DAT_PURPLE
#define DAT_CHESS1		DAT_WHITE
#define DAT_CHESS2		DAT_DGREY
#define DAT_WEATHER		DAT_LBLUE
#define DAT_DIR_NORTH           DAT_GREEN
#define DAT_DIR_SOUTH           DAT_BLUE
#define DAT_DIR_EAST            DAT_YELLOW
#define DAT_DIR_WEST            DAT_PINK
#define DAT_DIR_UP              DAT_LBLUE
#define DAT_DIR_DOWN            DAT_WHITE
#define DAT_DIR_NORTHEAST       DAT_DGREEN
#define DAT_DIR_NORTHWEST       DAT_ORANGE
#define DAT_DIR_SOUTHEAST       DAT_DBLUE
#define DAT_DIR_SOUTHWEST       DAT_PURPLE

typedef enum {
 AT_BLACK, AT_BLOOD, AT_DGREEN, AT_ORANGE, AT_DBLUE, AT_PURPLE, AT_CYAN,
 AT_GREY, AT_DGREY, AT_RED, AT_GREEN, AT_YELLOW, AT_BLUE, AT_PINK,
 AT_LBLUE, AT_WHITE, AT_BLINK, AT_PLAIN, AT_ACTION, AT_SAY, AT_GOSSIP,
 AT_OOC, AT_TELL, AT_HIT, AT_HITME, AT_IMMORT, AT_HURT, AT_FALLING,
 AT_DANGER, AT_MAGIC, AT_CONSIDER, AT_REPORT, AT_POISON, AT_SOCIAL,
 AT_DYING, AT_DEAD, AT_SKILL, AT_CARNAGE, AT_DAMAGE, AT_FLEE, AT_RMNAME,
 AT_RMDESC, AT_OBJECT, AT_PERSON, AT_LIST, AT_BYE, AT_GOLD, AT_GTELL,
 AT_NOTE, AT_HUNGRY, AT_THIRSTY, AT_FIRE, AT_SOBER, AT_WEAROFF,
 AT_SCORE, AT_SCORE2, AT_SCORE3, AT_SCORE4, AT_RESET, AT_LOG,
 AT_DIEMSG, AT_WARTALK, AT_WHO, AT_WHO2, AT_WHO3, AT_WHO4, AT_CHESS1,
 AT_CHESS2, AT_WEATHER, AT_DIR_NORTH, AT_DIR_SOUTH, AT_DIR_EAST,
 AT_DIR_WEST, AT_DIR_UP, AT_DIR_DOWN, AT_DIR_NORTHEAST, AT_DIR_NORTHWEST,
 AT_DIR_SOUTHEAST, AT_DIR_SOUTHWEST,
 MAX_COLOR_TYPE
} color_types;


#define INIT_WEAPON_CONDITION   12
#define MAX_ITEM_IMPACT		30

/*
 * Help table types.
 */
struct	help_data
{
    HELP_DATA *	next;
    HELP_DATA * prev;
    sh_int	level;
    char *	keyword;
    char *	text;
};



/*
 * Shop types.
 */
#define MAX_TRADE	 5

struct	shop_data
{
    SHOP_DATA *	next;			/* Next shop in list		*/
    SHOP_DATA * prev;			/* Previous shop in list	*/
    int		keeper;			/* Vnum of shop keeper mob	*/
    sh_int	buy_type [MAX_TRADE];	/* Item types shop will buy	*/
    sh_int	profit_buy;		/* Cost multiplier for buying	*/
    sh_int	profit_sell;		/* Cost multiplier for selling	*/
    sh_int	open_hour;		/* First opening hour		*/
    sh_int	close_hour;		/* First closing hour		*/
};

#define MAX_FIX		3
#define SHOP_FIX	1
#define SHOP_RECHARGE	2

struct	repairshop_data
{
    REPAIR_DATA * next;			/* Next shop in list		*/
    REPAIR_DATA * prev;			/* Previous shop in list	*/
    int		  keeper;		/* Vnum of shop keeper mob	*/
    sh_int	  fix_type [MAX_FIX];	/* Item types shop will fix	*/
    sh_int	  profit_fix;		/* Cost multiplier for fixing	*/
    sh_int	  shop_type;		/* Repair shop type		*/
    sh_int	  open_hour;		/* First opening hour		*/
    sh_int	  close_hour;		/* First closing hour		*/
};

/* mud prog defines */

#define ERROR_PROG        -1
#define IN_FILE_PROG      -2

typedef enum
{
  ACT_PROG, SPEECH_PROG, RAND_PROG, FIGHT_PROG, DEATH_PROG, HITPRCNT_PROG, 
  ENTRY_PROG, GREET_PROG, ALL_GREET_PROG, GIVE_PROG, BRIBE_PROG, HOUR_PROG, 
  TIME_PROG, WEAR_PROG, REMOVE_PROG, SAC_PROG, LOOK_PROG, EXA_PROG, ZAP_PROG, 
  GET_PROG, DROP_PROG, DAMAGE_PROG, REPAIR_PROG, BIRTH_PROG, SPEECHIW_PROG,
  PULL_PROG, PUSH_PROG, SLEEP_PROG, REST_PROG, LEAVE_PROG, SCRIPT_PROG,
  USE_PROG, QUEST_PROG, COMMAND_PROG, AREA_RESET_PROG, AREA_INIT_PROG,
  NUM_PROG_TYPES
} prog_types;

/*
 * For backwards compatability
 */
#define RDEATH_PROG DEATH_PROG
#define ENTER_PROG  ENTRY_PROG
#define RFIGHT_PROG FIGHT_PROG
#define RGREET_PROG GREET_PROG
#define OGREET_PROG GREET_PROG


/* Mob program structures */
struct  act_prog_data
{
    struct act_prog_data *next;
    void *vo;
};

struct	mob_prog_act_list
{
    MPROG_ACT_LIST * next;
    char *	     buf;
    CHAR_DATA *      ch;
    OBJ_DATA *	     obj;
    void *	     vo;
};

struct	mob_prog_data
{
    MPROG_DATA * next;
    sh_int 	 progtype;
    bool	 triggered;
    int		 resetdelay;
    char *	 arglist;
    char *	 comlist;
};

extern bool	MOBtrigger;

/*
 * Per-class stuff.
 */
struct	class_type
{
    char *	who_name;		/* Name for 'who'		*/
    sh_int	attr_prime;		/* Prime attribute		*/
    int		weapon;			/* First weapon			*/
    int		guild;			/* Vnum of guild room		*/
    sh_int	skill_adept;		/* Maximum skill level		*/
    sh_int	thac0_00;		/* Thac0 for level  0		*/
    sh_int	thac0_32;		/* Thac0 for level 32		*/
    sh_int	hp_min;			/* Min hp gained on leveling	*/
    sh_int	hp_max;			/* Max hp gained on leveling	*/
    sh_int	hp_const_lev;		/* Level for const hp gains	*/
    sh_int	hp_const_add;		/* Hp gained above hp_const_lev */
    bool	fMana;			/* Class gains mana on level	*/
    long	exp_base;		/* Class base exp		*/
    long	exp_power;		/* exp_base*level^exp_power	*/
    char *      attr_string;            /* chargen s i w d co ch l string */
};

/* race dedicated stuff */
struct	race_type
{
    char *      race_name;              /* Race name			*/
    int		affected;		/* Default affect bitvectors	*/
    sh_int	str_plus;		/* Str bonus/penalty		*/
    sh_int	dex_plus;		/* Dex      "			*/
    sh_int	wis_plus;		/* Wis      "			*/
    sh_int	int_plus;		/* Int      "			*/
    sh_int	con_plus;		/* Con      "			*/
    sh_int	cha_plus;		/* Cha      "			*/
    sh_int	lck_plus;		/* Lck 	    "			*/
    sh_int	hit;
    sh_int	mana;
    int		resist;
    int		suscept;
    int		class_restriction;	/* Flags for illegal classes	*/
    int		language;               /* Default racial language      */
    int		is_pc_race;
    int         body_parts;             /* Self explanatory I hope      */
};

typedef enum {
CLAN_PLAIN, CLAN_VAMPIRE, CLAN_WARRIOR, CLAN_DRUID, CLAN_MAGE, CLAN_CELTIC,
CLAN_THIEF, CLAN_CLERIC, CLAN_UNDEAD, CLAN_CHAOTIC, CLAN_NEUTRAL, CLAN_LAWFUL,
CLAN_NOKILL, CLAN_ORDER, CLAN_GUILD } clan_types;

typedef enum { GROUP_CLAN, GROUP_COUNCIL, GROUP_GUILD } group_types;


struct	clan_data
{
    CLAN_DATA * next;		/* next clan in list			*/
    CLAN_DATA * prev;		/* previous clan in list		*/
    char *	filename;	/* Clan filename			*/
    char *	name;		/* Clan name				*/
    char *	motto;		/* Clan motto				*/
    char *	symbol;		/* Clan symbol				*/
    char *	description;	/* A brief description of the clan	*/
    char *	deity;		/* Clan's deity				*/
    char *	leader;		/* Head clan leader			*/
    char *	number1;	/* First officer			*/
    char *	number2;	/* Second officer			*/
    char *      member_list;    /* list of members                      */
    int		pkills;		/* Number of pkills on behalf of clan	*/
    int		pdeaths;	/* Number of pkills against clan	*/
    int		mkills;		/* Number of mkills on behalf of clan	*/
    int		mdeaths;	/* Number of clan deaths due to mobs	*/
    int		illegal_pk;	/* Number of illegal pk's by clan	*/
    int		score;		/* Overall score			*/
    sh_int	clan_type;	/* See clan type defines		*/
    sh_int	favour;		/* Deities favour upon the clan		*/
    sh_int	strikes;	/* Number of strikes against the clan	*/
    sh_int	members;	/* Number of clan members		*/
    sh_int	alignment;	/* Clan's general alignment		*/
    int		board;		/* Vnum of clan board			*/
    int		clanobj1;	/* Vnum of first clan obj (ring)	*/
    int		clanobj2;	/* Vnum of second clan obj (shield)	*/
    int		clanobj3;	/* Vnum of third clan obj (weapon)	*/
    int		recall;		/* Vnum of clan's recall room		*/
    int		storeroom;	/* Vnum of clan's store room		*/
    int		guard1;		/* Vnum of clan guard type 1		*/
    int		guard2;		/* Vnum of clan guard type 2		*/
    sh_int	cl;             /* For guilds				*/
};

struct	council_data
{
    COUNCIL_DATA * next;	/* next council in list			*/
    COUNCIL_DATA * prev;	/* previous council in list		*/
    char *	filename;	/* Council filename			*/
    char *	name;		/* Council name				*/
    char *	description;	/* A brief description of the council	*/
    char *	head;		/* Council head 			*/
    char *	powers;		/* Council powers			*/
    sh_int	members;	/* Number of council members		*/
    int		board;		/* Vnum of council board		*/
    int		meeting;	/* Vnum of council's meeting room	*/
};

struct	deity_data
{
    DEITY_DATA * next;
    DEITY_DATA * prev;
    char *	filename;
    char *	name;
    char *	description;
    sh_int	alignment;
    sh_int	worshippers;
    sh_int	scorpse;
    sh_int	sdeityobj;
    sh_int	savatar;
    sh_int	srecall;
    sh_int	flee;
    sh_int	flee_npcrace;
    sh_int	flee_npcfoe;
    sh_int	kill;
    sh_int	kill_magic;
    sh_int	kill_npcrace;
    sh_int	kill_npcfoe;
    sh_int	sac;
    sh_int	bury_corpse;
    sh_int	aid_spell;
    sh_int	aid;
    sh_int	backstab;
    sh_int	steal;
    sh_int	die;
    sh_int	die_npcrace;
    sh_int	die_npcfoe;
    sh_int	spell_aid;
    sh_int	dig_corpse;
    int		race;
    sh_int	cl;
    int		element;
    int		sex;
    int		avatar;
    int		deityobj;
    int		affected;
    int		npcrace;
    int		npcfoe;
    int		suscept;
};


struct tourney_data
{
    int    open;
    int    low_level;
    int    hi_level;
};

/*
 * Data structure for notes.
 */
struct	note_data
{
    NOTE_DATA *	next;
    NOTE_DATA * prev;
    sh_int      indent;
    char *	sender;
    char *	date;
    char *	to_list;
    char *	subject;
    int         voting;
    char *	yesvotes;
    char *	novotes;
    char *	abstentions;
    char *	text;
};

struct	board_data
{
    BOARD_DATA * next;			/* Next board in list		   */
    BOARD_DATA * prev;			/* Previous board in list	   */
    NOTE_DATA *  first_note;		/* First note on board		   */
    NOTE_DATA *  last_note;		/* Last note on board		   */
    char *	 note_file;		/* Filename to save notes to	   */
    char *	 read_group;		/* Can restrict a board to a       */
    char *	 post_group;		/* council, clan, guild etc        */
    char *	 extra_readers;		/* Can give read rights to players */
    char *       extra_removers;        /* Can give remove rights to players */
    int		 board_obj;		/* Vnum of board object		   */
    sh_int	 num_posts;		/* Number of notes on this board   */
    sh_int	 min_read_level;	/* Minimum level to read a note	   */
    sh_int	 min_post_level;	/* Minimum level to post a note    */
    sh_int	 min_remove_level;	/* Minimum level to remove a note  */
    sh_int	 max_posts;		/* Maximum amount of notes allowed */
    int          type;                  /* Normal board or mail board? */
    sh_int       currtype;
};


/*
 * An affect.
 */
struct	affect_data
{
    AFFECT_DATA *	next;
    AFFECT_DATA *	prev;
    sh_int		type;
    int			duration;
    sh_int		location;
    int			modifier;
    int			bitvector;
};


/*
 * A SMAUG spell
 */
struct	smaug_affect
{
    SMAUG_AFF *		next;
    char *		duration;
    sh_int		location;
    char *		modifier;
    int			bitvector;
};


/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_CITYGUARD	    3060
#define MOB_VNUM_VAMPIRE	    3404
#define MOB_VNUM_ANIMATED_CORPSE    100
#define MOB_VNUM_POLY_WOLF	    10
#define MOB_VNUM_POLY_MIST	    11
#define MOB_VNUM_POLY_BAT	    12
#define MOB_VNUM_POLY_HAWK	    13
#define MOB_VNUM_POLY_CAT	    14
#define MOB_VNUM_POLY_DOVE	    15
#define MOB_VNUM_POLY_FISH	    16

/* Conjure Elemental */
#define MOB_VNUM_FIRE_ELEMENTAL     10
#define MOB_VNUM_WATER_ELEMENTAL    11
#define MOB_VNUM_AIR_ELEMENTAL      12
#define MOB_VNUM_EARTH_ELEMENTAL    13

/* elemental servants */
#define MOB_VNUM_FIRE_SERVANT       40
#define MOB_VNUM_EARTH_SERVANT      41
#define MOB_VNUM_WATER_SERVANT      42
#define MOB_VNUM_WIND_SERVANT       43

/* Cacaodemons */
#define MOB_VNUM_DEMON_TYPE_I       20
#define MOB_VNUM_DEMON_TYPE_II      21
#define MOB_VNUM_DEMON_TYPE_III     22
#define MOB_VNUM_DEMON_TYPE_IV      23
#define MOB_VNUM_DEMON_TYPE_V       24
#define MOB_VNUM_DEMON_TYPE_VI      25

/* Animate xxx */
#define MOB_VNUM_ANIMATE_ROCK       50

#define MOB_VNUM_DUST_DEVIL         60

#define MOB_VNUM_ARMOR_GOLEM        38

/* changestaff mob */
#define MOB_VNUM_CHANGESTAFF_TREE   6110

/* find familiar */
#define MOB_VNUM_FAMILIAR_FIRST     3090
#define MOB_VNUM_FAMILIAR_SECOND    3091
#define MOB_VNUM_FAMILIAR_THIRD     3092
#define MOB_VNUM_FAMILIAR_FOURTH    3093
#define MOB_VNUM_FAMILIAR_FIFTH     3094

/* druid and monk challenge mobs */
#define MOB_VNUM_DRUID_CHALLENGE    600
#define MOB_VNUM_MONK_CHALLENGE     650

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC		 BV00		/* Auto set for mobs	*/
#define ACT_SENTINEL		 BV01		/* Stays in one room	*/
#define ACT_SCAVENGER		 BV02		/* Picks up objects	*/
#define ACT_NICE_THIEF           BV03
#define ACT_ANNOYING             BV04
#define ACT_AGGRESSIVE		 BV05		/* Attacks PC's		*/
#define ACT_STAY_AREA		 BV06		/* Won't leave area	*/
#define ACT_WIMPY		 BV07		/* Flees when hurt	*/
#define ACT_PET			 BV08		/* Auto set for pets	*/
#define ACT_TRAIN		 BV09		/* Can train PC's	*/
#define ACT_PRACTICE		 BV10		/* Can practice PC's	*/
#define ACT_IMMORTAL		 BV11		/* Cannot be killed	*/
#define ACT_DEADLY		 BV12		/* Has a deadly poison  */
#define ACT_CUSTOMSAVES		 BV13           /* mob has custom saves */
#define ACT_META_AGGR		 BV14		/* Extremely aggressive */
#define ACT_GUARDIAN		 BV15		/* Protects master	*/
#define ACT_RUNNING		 BV16		/* Hunts quickly	*/
#define ACT_NOWANDER		 BV17		/* Doesn't wander	*/
#define ACT_MOUNTABLE		 BV18		/* Can be mounted	*/
#define ACT_MOUNTED		 BV19		/* Is mounted		*/
#define ACT_SCHOLAR              BV20           /* Can teach languages  */
#define ACT_SECRETIVE		 BV21		/* actions aren't seen	*/
#define ACT_POLYMORPHED		 BV22		/* Mob is a ch		*/
#define ACT_MOBINVIS		 BV23		/* Like wizinvis	*/
#define ACT_NOASSIST		 BV24		/* Doesn't assist mobs	*/
#define ACT_ILLUSION		 BV25
#define ACT_HUGE		 BV26
#define ACT_GREET		 BV27
#define ACT_TEACHER		 BV28

#define ACT_PROTOTYPE		 BV30		/* A prototype mob	*/
/* 28 acts */

/*
 * ACT2 bits for mobs.
 * Used in #MOBILES.
 */
#define ACT2_MAGE		 BV00
#define ACT2_WARRIOR		 BV01
#define ACT2_CLERIC		 BV02
#define ACT2_THIEF		 BV03
#define ACT2_DRUID		 BV04
#define ACT2_MONK		 BV05
#define ACT2_BARBARIAN		 BV06
#define ACT2_PALADIN		 BV07
#define ACT2_RANGER		 BV08
#define ACT2_PSI		 BV09
#define ACT2_ARTIFICER		 BV10
#define ACT2_VAMPIRE		 BV11
#define ACT2_AMAZON		 BV12
#define ACT2_NECROMANCER         BV13
#define ACT2_ANTIPALADIN         BV14
#define ACT2_MASTER_VAMPIRE      BV15 /* Makes energy drain attacks w/o save */
#define ACT2_PLANAR              BV16 /* same as PLR2_PLANAR - DONT CHANGE! */
/* 13 act2s */

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND		  BV00
#define AFF_INVISIBLE		  BV01
#define AFF_DETECT_EVIL		  BV02
#define AFF_DETECT_INVIS	  BV03
#define AFF_DETECT_MAGIC	  BV04
#define AFF_DETECT_HIDDEN	  BV05
#define AFF_HOLD		  BV06		/* Unused	*/
#define AFF_SANCTUARY		  BV07
#define AFF_FAERIE_FIRE		  BV08
#define AFF_INFRARED		  BV09
#define AFF_CURSE		  BV10
#define AFF_FLAMING		  BV11		/* Unused	*/
#define AFF_POISON		  BV12
#define AFF_PROTECT		  BV13
#define AFF_PARALYSIS		  BV14
#define AFF_SNEAK		  BV15
#define AFF_HIDE		  BV16
#define AFF_SLEEP		  BV17
#define AFF_CHARM		  BV18
#define AFF_FLYING		  BV19
#define AFF_PASS_DOOR		  BV20
#define AFF_FLOATING		  BV21
#define AFF_TRUESIGHT		  BV22
#define AFF_DETECTTRAPS		  BV23
#define AFF_SCRYING	          BV24
#define AFF_FIRESHIELD	          BV25
#define AFF_SHOCKSHIELD	          BV26
#define AFF_HAUS1                 BV27
#define AFF_ICESHIELD  		  BV28
#define AFF_POSSESS		  BV29
#define AFF_BERSERK		  BV30
#define AFF_AQUA_BREATH		  BV31

/* 31 aff's */

/*
 * Bits for 'affected_by2'.
 * Used in #MOBILES.
 */
#define AFF2_ANIMAL_INVIS	  BV00
#define AFF2_HEAT_STUFF		  BV01
#define AFF2_LIFE_PROT		  BV02
#define AFF2_DRAGON_RIDE	  BV03
#define AFF2_GROWTH		  BV04
#define AFF2_TREE_TRAVEL	  BV05
#define AFF2_TRAVELLING           BV06
#define AFF2_SILENCE		  BV07
#define AFF2_TELEPATHY		  BV08
#define AFF2_ETHEREAL		  BV09
#define AFF2_BEACON               BV10 /* Makes visible to all */

/*
 * Resistant Immune Susceptible flags
 */
#define RIS_FIRE		  BV00
#define RIS_COLD		  BV01
#define RIS_ELECTRICITY		  BV02
#define RIS_ENERGY		  BV03
#define RIS_BLUNT		  BV04
#define RIS_PIERCE		  BV05
#define RIS_SLASH		  BV06
#define RIS_ACID		  BV07
#define RIS_POISON		  BV08
#define RIS_DRAIN		  BV09
#define RIS_SLEEP		  BV10
#define RIS_CHARM		  BV11
#define RIS_HOLD		  BV12
#define RIS_NONMAGIC		  BV13
#define RIS_PLUS1		  BV14
#define RIS_PLUS2		  BV15
#define RIS_PLUS3		  BV16
#define RIS_PLUS4		  BV17
#define RIS_PLUS5		  BV18
#define RIS_PLUS6		  BV19
#define RIS_MAGIC		  BV20
#define RIS_PARALYSIS		  BV21
#define RIS_GOOD                  BV22
#define RIS_EVIL                  BV23
#define RIS_HOLY		  BV24
#define RIS_UNHOLY		  BV25
#define RIS_PSYCHIC               BV26
/* 27 RIS's*/

/*
 * Attack types
 */
#define ATCK_BITE		  BV00
#define ATCK_CLAWS		  BV01
#define ATCK_TAIL		  BV02
#define ATCK_STING		  BV03
#define ATCK_PUNCH		  BV04
#define ATCK_KICK		  BV05
#define ATCK_TRIP		  BV06
#define ATCK_BASH		  BV07
#define ATCK_STUN		  BV08
#define ATCK_GOUGE		  BV09
#define ATCK_BACKSTAB		  BV10
#define ATCK_FEED		  BV11
#define ATCK_DRAIN		  BV12
#define ATCK_FIREBREATH		  BV13
#define ATCK_FROSTBREATH	  BV14
#define ATCK_ACIDBREATH		  BV15
#define ATCK_LIGHTNBREATH	  BV16
#define ATCK_GASBREATH		  BV17
#define ATCK_POISON		  BV18
#define ATCK_NASTYPOISON	  BV19
#define ATCK_GAZE		  BV20
#define ATCK_BLINDNESS		  BV21
#define ATCK_CAUSESERIOUS	  BV22
#define ATCK_EARTHQUAKE		  BV23
#define ATCK_CAUSECRITICAL	  BV24
#define ATCK_CURSE		  BV25
#define ATCK_FLAMESTRIKE	  BV26
#define ATCK_HARM		  BV27
#define ATCK_FIREBALL		  BV28
#define ATCK_COLORSPRAY		  BV29
#define ATCK_WEAKEN		  BV30
#define ATCK_SPIRALBLAST	  BV31
/* 32 USED! DO NOT ADD MORE! SB */

/*
 * Defense types
 */
#define DFND_PARRY		  BV00
#define DFND_DODGE		  BV01
#define DFND_HEAL		  BV02
#define DFND_CURELIGHT		  BV03
#define DFND_CURESERIOUS	  BV04
#define DFND_CURECRITICAL	  BV05
#define DFND_DISPELMAGIC	  BV06
#define DFND_DISPELEVIL		  BV07
#define DFND_SANCTUARY		  BV08
#define DFND_FIRESHIELD		  BV09
#define DFND_SHOCKSHIELD	  BV10
#define DFND_SHIELD		  BV11
#define DFND_BLESS		  BV12
#define DFND_STONESKIN		  BV13
#define DFND_TELEPORT		  BV14
#define DFND_MONSUM1		  BV15
#define DFND_MONSUM2		  BV16
#define DFND_MONSUM3		  BV17
#define DFND_MONSUM4		  BV18
#define DFND_DISARM		  BV19
#define DFND_ICESHIELD 		  BV20
#define DFND_GRIP		  BV21
/* 21 def's */

/*
 * Body parts
 */
#define PART_HEAD		  BV00
#define PART_ARMS		  BV01
#define PART_LEGS		  BV02
#define PART_HEART		  BV03
#define PART_BRAINS		  BV04
#define PART_GUTS		  BV05
#define PART_HANDS		  BV06
#define PART_FEET		  BV07
#define PART_FINGERS		  BV08
#define PART_EAR		  BV09
#define PART_EYE		  BV10
#define PART_LONG_TONGUE	  BV11
#define PART_EYESTALKS		  BV12
#define PART_TENTACLES		  BV13
#define PART_FINS		  BV14
#define PART_WINGS		  BV15
#define PART_TAIL		  BV16
#define PART_SCALES		  BV17
/* for combat */
#define PART_CLAWS		  BV18
#define PART_FANGS		  BV19
#define PART_HORNS		  BV20
#define PART_TUSKS		  BV21
#define PART_TAILATTACK		  BV22
#define PART_SHARPSCALES	  BV23
#define PART_BEAK		  BV24

#define PART_HAUNCH		  BV25
#define PART_HOOVES		  BV26
#define PART_PAWS		  BV27
#define PART_FORELEGS		  BV28
#define PART_FEATHERS		  BV29
#define PART_CHEST                BV30
#define PART_STOMACH              BV31
/*
 * Autosave flags
 */
#define SV_DEATH		  BV00
#define SV_KILL			  BV01
#define SV_PASSCHG		  BV02
#define SV_DROP			  BV03
#define SV_PUT			  BV04
#define SV_GIVE			  BV05
#define SV_AUTO			  BV06
#define SV_ZAPDROP		  BV07
#define SV_AUCTION		  BV08
#define SV_GET			  BV09
#define SV_RECEIVE		  BV10
#define SV_IDLE			  BV11
#define SV_BACKUP		  BV12

/*
 * Pipe flags
 */
#define PIPE_TAMPED		  BV01
#define PIPE_LIT		  BV02
#define PIPE_HOT		  BV03
#define PIPE_DIRTY		  BV04
#define PIPE_FILTHY		  BV05
#define PIPE_GOINGOUT		  BV06
#define PIPE_BURNT		  BV07
#define PIPE_FULLOFASH		  BV08
#define PIPE_SINGLE_USE		  BV09
#define PIPE_BONG		  BV10

/*
 * Skill/Spell flags	The minimum BV *MUST* be 11!
 */
#define SF_WATER		  BV11
#define SF_EARTH		  BV12
#define SF_AIR			  BV13
#define SF_ASTRAL		  BV14
#define SF_AREA			  BV15  /* is an area spell		*/
#define SF_DISTANT		  BV16  /* affects something far away	*/
#define SF_REVERSE		  BV17
#define SF_SAVE_HALF_DAMAGE	  BV18  /* save for half damage		*/
#define SF_SAVE_NEGATES		  BV19  /* save negates affect		*/
#define SF_ACCUMULATIVE		  BV20  /* is accumulative		*/
#define SF_RECASTABLE		  BV21  /* can be refreshed		*/
#define SF_NOSCRIBE		  BV22  /* cannot be scribed		*/
#define SF_NOBREW		  BV23  /* cannot be brewed		*/
#define SF_GROUPSPELL		  BV24  /* only affects group members	*/
#define SF_OBJECT		  BV25	/* directed at an object	*/
#define SF_CHARACTER		  BV26  /* directed at a character	*/
#define SF_SECRETSKILL		  BV27	/* hidden unless learned	*/
#define SF_PKSENSITIVE		  BV28	/* much harder for plr vs. plr	*/
#define SF_STOPONFAIL		  BV29	/* stops spell on first failure */
#define SF_VERBALIZE_SKILL        BV30  /* verbalizes skill for monks   */
#define SF_FORGE                  BV31  /* skill requires forge         */

typedef enum { SS_NONE, SS_POISON_DEATH, SS_ROD_WANDS, SS_PARA_PETRI,
	       SS_BREATH, SS_SPELL_STAFF } spell_save_types;

#define ALL_BITS		INT_MAX
#define SDAM_MASK		ALL_BITS & ~(BV00 | BV01 | BV02)
#define SACT_MASK		ALL_BITS & ~(BV03 | BV04 | BV05)
#define SCLA_MASK		ALL_BITS & ~(BV06 | BV07 | BV08)
#define SPOW_MASK		ALL_BITS & ~(BV09 | BV10)

typedef enum { SD_NONE, SD_FIRE, SD_COLD, SD_ELECTRICITY, SD_ENERGY, SD_ACID,
	       SD_POISON, SD_DRAIN } spell_dam_types;

typedef enum { SA_NONE, SA_CREATE, SA_DESTROY, SA_RESIST, SA_SUSCEPT,
               SA_DIVINATE, SA_OBSCURE, SA_CHANGE } spell_act_types;

typedef enum { SP_NONE, SP_MINOR, SP_GREATER, SP_MAJOR } spell_power_types;

typedef enum { SC_NONE, SC_LUNAR, SC_SOLAR, SC_TRAVEL, SC_SUMMON,
	       SC_LIFE, SC_DEATH, SC_ILLUSION, SC_CURSE } spell_class_types;

/*
 * Sex.
 * Used in #MOBILES.
 */
typedef enum { SEX_NEUTRAL, SEX_MALE, SEX_FEMALE } sex_types;

typedef enum {
  TRAP_TYPE_POISON_GAS = 1, TRAP_TYPE_POISON_DART,    TRAP_TYPE_POISON_NEEDLE,
  TRAP_TYPE_POISON_DAGGER,  TRAP_TYPE_POISON_ARROW,   TRAP_TYPE_BLINDNESS_GAS,
  TRAP_TYPE_SLEEPING_GAS,   TRAP_TYPE_FLAME,	      TRAP_TYPE_EXPLOSION,
  TRAP_TYPE_ACID_SPRAY,	    TRAP_TYPE_ELECTRIC_SHOCK, TRAP_TYPE_BLADE,
  TRAP_TYPE_SEX_CHANGE,     TRAP_TYPE_FLOOD_ROOM
} trap_types;

#define MAX_TRAPTYPE		   TRAP_TYPE_SEX_CHANGE

#define TRAP_ROOM      		   BV00
#define TRAP_OBJ	      	   BV01
#define TRAP_ENTER_ROOM		   BV02
#define TRAP_LEAVE_ROOM		   BV03
#define TRAP_OPEN		   BV04
#define TRAP_CLOSE		   BV05
#define TRAP_GET		   BV06
#define TRAP_PUT		   BV07
#define TRAP_PICK		   BV08
#define TRAP_UNLOCK		   BV09
#define TRAP_N			   BV10
#define TRAP_S			   BV11
#define TRAP_E	      		   BV12
#define TRAP_W	      		   BV13
#define TRAP_U	      		   BV14
#define TRAP_D	      		   BV15
#define TRAP_EXAMINE		   BV16
#define TRAP_NE			   BV17
#define TRAP_NW			   BV18
#define TRAP_SE			   BV19
#define TRAP_SW			   BV20

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_TAN_BAG                14
#define OBJ_VNUM_DEED                   15
#define OBJ_VNUM_FIND_FOOD              21
#define OBJ_VNUM_FIND_WATER          27025
#define OBJ_VNUM_TAN_SHIELD             67
#define OBJ_VNUM_TAN_JACKET             68
#define OBJ_VNUM_TAN_BOOTS              69
#define OBJ_VNUM_TAN_GLOVES             70
#define OBJ_VNUM_TAN_LEGGINGS           71
#define OBJ_VNUM_TAN_SLEEVES            72
#define OBJ_VNUM_TAN_HELMET             73

#define OBJ_VNUM_MONEY_ONE              74
#define OBJ_VNUM_MONEY_SOME	        75

#define OBJ_VNUM_CORPSE_NPC             76
#define OBJ_VNUM_CORPSE_PC              77
#define OBJ_VNUM_SEVERED_HEAD	        78
#define OBJ_VNUM_TORN_HEART	        79
#define OBJ_VNUM_SLICED_ARM	        80
#define OBJ_VNUM_SLICED_LEG             81
#define OBJ_VNUM_SPILLED_GUTS	        82
#define OBJ_VNUM_BLOOD		        83
#define OBJ_VNUM_BLOODSTAIN             84
#define OBJ_VNUM_SCRAPS	                85

#define OBJ_VNUM_MUSHROOM	        86
#define OBJ_VNUM_LIGHT_BALL             87
#define OBJ_VNUM_SPRING                 88

#define OBJ_VNUM_SLICE                  89
#define OBJ_VNUM_SHOPPING_BAG	        90

#define OBJ_VNUM_FIRE	                91
#define OBJ_VNUM_TRAP	                92
#define OBJ_VNUM_PORTAL	                93

#define OBJ_VNUM_BLACK_POWDER           94
#define OBJ_VNUM_SCROLL_SCRIBING        95
#define OBJ_VNUM_FLASK_BREWING          96
#define OBJ_VNUM_NOTE		        97
#define OBJ_VNUM_QUILL		        98

/* Academy eq */
#define OBJ_VNUM_SCHOOL_MACE         10315
#define OBJ_VNUM_SCHOOL_DAGGER       10312
#define OBJ_VNUM_SCHOOL_SWORD        10313
#define OBJ_VNUM_SCHOOL_VEST         10308
#define OBJ_VNUM_SCHOOL_SHIELD       10310
#define OBJ_VNUM_SCHOOL_BANNER       10311

/* Conjure Elemental */
#define OBJ_VNUM_RED_STONE            5233
#define OBJ_VNUM_PALE_BLUE_STONE      5230
#define OBJ_VNUM_GREY_STONE           5239
#define OBJ_VNUM_CLEAR_STONE          5243

/* Cacaodemon */
#define OBJ_VNUM_DEMON_TYPE_I         5105
#define OBJ_VNUM_DEMON_TYPE_II       21014
#define OBJ_VNUM_DEMON_TYPE_III       1101
#define OBJ_VNUM_DEMON_TYPE_IV        5113
#define OBJ_VNUM_DEMON_TYPE_V         5107
#define OBJ_VNUM_DEMON_TYPE_VI       27002

/* Major creation */
#define OBJ_VNUM_MAJORC_BOOTS        18258
#define OBJ_VNUM_MAJORC_LEGGINGS     18214
#define OBJ_VNUM_MAJORC_SLEEVES      18215
#define OBJ_VNUM_MAJORC_HELMET       18213
#define OBJ_VNUM_MAJORC_BREAST       18209
#define OBJ_VNUM_MAJORC_GLOVES       18212

/* Minor creation */
#define OBJ_VNUM_MINORC_LONG_SWORD    3022
#define OBJ_VNUM_MINORC_SHIELD        3042
#define OBJ_VNUM_MINORC_RAFT          3060
#define OBJ_VNUM_MINORC_BAG           3032
#define OBJ_VNUM_MINORC_WATER_BARREL  6013
#define OBJ_VNUM_MINORC_BREAD         3010

/* Greater creation ?? */
/*#define OBJ_VNUM_GREATERC_*/

/*
 * Item types.
 * Used in #OBJECTS.
 */
typedef enum
{
  ITEM_NONE, ITEM_LIGHT, ITEM_SCROLL, ITEM_WAND, ITEM_STAFF, ITEM_WEAPON,
  ITEM_FIREWEAPON, ITEM_MISSILE, ITEM_TREASURE, ITEM_ARMOR, ITEM_POTION,
  ITEM_WORN, ITEM_FURNITURE, ITEM_TRASH, ITEM_OLDTRAP, ITEM_CONTAINER,
  ITEM_NOTE, ITEM_DRINK_CON, ITEM_KEY, ITEM_FOOD, ITEM_MONEY, ITEM_PEN,
  ITEM_BOAT, ITEM_CORPSE_NPC, ITEM_CORPSE_PC, ITEM_FOUNTAIN, ITEM_PILL,
  ITEM_BLOOD, ITEM_BLOODSTAIN, ITEM_SCRAPS, ITEM_PIPE, ITEM_HERB_CON,
  ITEM_HERB, ITEM_INCENSE, ITEM_FIRE, ITEM_BOOK, ITEM_SWITCH, ITEM_LEVER,
  ITEM_PULLCHAIN, ITEM_BUTTON, ITEM_DIAL, ITEM_RUNE, ITEM_RUNEPOUCH,
  ITEM_MATCH, ITEM_TRAP, ITEM_MAP, ITEM_PORTAL, ITEM_PAPER,
  ITEM_TINDER, ITEM_LOCKPICK, ITEM_SPIKE, ITEM_DISEASE, ITEM_OIL, ITEM_FUEL,
  ITEM_BLAH1, ITEM_BLAH2, ITEM_MISSILE_WEAPON, ITEM_PROJECTILE, ITEM_QUIVER,
  ITEM_SHOVEL, ITEM_SALVE, ITEM_BOARD, ITEM_COOK, ITEM_KEYRING, ITEM_ODOR,
  ITEM_POKEBALL, ITEM_ROCK, ITEM_STONE, ITEM_FORGE, ITEM_MATERIAL
} item_types;

#define MAX_ITEM_TYPE		     ITEM_MATERIAL

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW		BV00
#define ITEM_HUM		BV01
#define ITEM_DARK		BV02
#define ITEM_LOYAL		BV03
#define ITEM_EVIL		BV04
#define ITEM_INVIS		BV05
#define ITEM_MAGIC		BV06
#define ITEM_NODROP		BV07
#define ITEM_BLESS		BV08
#define ITEM_ANTI_GOOD		BV09
#define ITEM_ANTI_EVIL		BV10
#define ITEM_ANTI_NEUTRAL	BV11
#define ITEM_NOREMOVE		BV12
#define ITEM_INVENTORY		BV13
#define ITEM_ANTI_MAGE		BV14
#define ITEM_ANTI_THIEF	        BV15
#define ITEM_ANTI_WARRIOR	BV16
#define ITEM_ANTI_CLERIC	BV17
#define ITEM_ORGANIC		BV18
#define ITEM_METAL		BV19
#define ITEM_DONATION		BV20
#define ITEM_CLANOBJECT		BV21
#define ITEM_CLANCORPSE		BV22
#define ITEM_ANTI_VAMPIRE	BV23
#define ITEM_ANTI_DRUID	        BV24
#define ITEM_HIDDEN		BV25
#define ITEM_POISONED		BV26
#define ITEM_COVERING		BV27
#define ITEM_DEATHROT		BV28
#define ITEM_BURRIED		BV29	/* item is underground */
#define ITEM_PROTOTYPE		BV30
/* 31 extra_flag's */

/* Extra flags 2 - extra_flags2 */
#define ITEM2_MINERAL		BV00
#define ITEM2_BRITTLE		BV01
#define ITEM2_RESISTANT		BV02
#define ITEM2_IMMUNE		BV03
#define ITEM2_ANTI_MEN		BV04
#define ITEM2_ANTI_WOMEN	BV05
#define ITEM2_ANTI_NEUTER	BV06
#define ITEM2_ANTI_SUN		BV07
#define ITEM2_ANTI_BARBARIAN	BV08
#define ITEM2_ANTI_RANGER	BV09
#define ITEM2_ANTI_PALADIN	BV10
#define ITEM2_ANTI_PSI		BV11
#define ITEM2_ANTI_MONK		BV12
#define ITEM2_ANTI_ARTIFICER	BV13
#define ITEM2_ANTI_AMAZON	BV14
#define ITEM2_ANTI_NECROMANCER	BV15
#define ITEM2_ANTI_APALADIN     BV16

#define ITEM2_ONLY_CLASS	BV20
#define ITEM2_NO_CLONE		BV21
#define ITEM2_RENT		BV22
#define ITEM2_AUDIO		BV23
#define ITEM2_STORED_ITEM       BV24 /* Stored Items -- Heath 8-14-98 */
/* 15 extra_flag2's */

/* Magic flags - extra extra_flags for objects that are used in spells */
#define ITEM_RETURNING		BV00
#define ITEM_BACKSTABBER  	BV01
#define ITEM_BANE		BV02
#define ITEM_LOYAL		BV03
#define ITEM_HASTE		BV04
#define ITEM_DRAIN		BV05
#define ITEM_LIGHTNING_BLADE  	BV06

/* Lever/dial/switch/button/pullchain flags */
#define TRIG_UP			BV00
#define TRIG_UNLOCK		BV01
#define TRIG_LOCK		BV02
#define TRIG_D_NORTH		BV03
#define TRIG_D_SOUTH		BV04
#define TRIG_D_EAST		BV05
#define TRIG_D_WEST		BV06
#define TRIG_D_UP		BV07
#define TRIG_D_DOWN		BV08
#define TRIG_DOOR		BV09
#define TRIG_CONTAINER		BV10
#define TRIG_OPEN		BV11
#define TRIG_CLOSE		BV12
#define TRIG_PASSAGE		BV13
#define TRIG_OLOAD		BV14
#define TRIG_MLOAD		BV15
#define TRIG_TELEPORT		BV16
#define TRIG_TELEPORTALL	BV17
#define TRIG_TELEPORTPLUS	BV18
#define TRIG_DEATH		BV19
#define TRIG_CAST		BV20
#define TRIG_FAKEBLADE		BV21
#define TRIG_RAND4		BV22
#define TRIG_RAND6		BV23
#define TRIG_TRAPDOOR		BV24
#define TRIG_ANOTHEROOM		BV25
#define TRIG_USEDIAL		BV26
#define TRIG_ABSOLUTEVNUM	BV27
#define TRIG_SHOWROOMDESC	BV28
#define TRIG_AUTORETURN		BV29

#define TELE_SHOWDESC		BV00
#define TELE_TRANSALL		BV01
#define TELE_TRANSALLPLUS	BV02


/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		BV00
#define ITEM_WEAR_FINGER	BV01
#define ITEM_WEAR_NECK		BV02
#define ITEM_WEAR_BODY		BV03
#define ITEM_WEAR_HEAD		BV04
#define ITEM_WEAR_LEGS		BV05
#define ITEM_WEAR_FEET		BV06
#define ITEM_WEAR_HANDS		BV07
#define ITEM_WEAR_ARMS		BV08
#define ITEM_WEAR_SHIELD	BV09
#define ITEM_WEAR_ABOUT		BV10
#define ITEM_WEAR_WAIST		BV11
#define ITEM_WEAR_WRIST		BV12
#define ITEM_WIELD		BV13
#define ITEM_HOLD		BV14
#define ITEM_DUAL_WIELD		BV15
#define ITEM_WEAR_EARS		BV16
#define ITEM_WEAR_EYES		BV17
#define ITEM_MISSILE_WIELD	BV18
#define ITEM_WEAR_BACK		BV19
#define ITEM_WEAR_NOSE		BV20
#define ITEM_WEAR_ANKLE		BV21

/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
typedef enum
{
  APPLY_NONE, APPLY_STR, APPLY_DEX, APPLY_INT, APPLY_WIS, APPLY_CON,
  APPLY_SEX, APPLY_CLASS, APPLY_LEVEL, APPLY_AGE, APPLY_HEIGHT, APPLY_WEIGHT,
  APPLY_MANA, APPLY_HIT, APPLY_MOVE, APPLY_GOLD, APPLY_EXP, APPLY_AC,
  APPLY_HITROLL, APPLY_DAMROLL, APPLY_SAVING_POISON, APPLY_SAVING_ROD,
  APPLY_SAVING_PARA, APPLY_SAVING_BREATH, APPLY_SAVING_SPELL, APPLY_CHA,
  APPLY_AFFECT, APPLY_RESISTANT, APPLY_IMMUNE, APPLY_SUSCEPTIBLE,
  APPLY_WEAPONSPELL, APPLY_LCK, APPLY_BACKSTAB, APPLY_PICK, APPLY_TRACK,
  APPLY_STEAL, APPLY_SNEAK, APPLY_HIDE, APPLY_PALM, APPLY_DETRAP, APPLY_DODGE,
  APPLY_SF, APPLY_SCAN, APPLY_GOUGE, APPLY_SEARCH, APPLY_MOUNT, APPLY_DISARM,
  APPLY_KICK, APPLY_PARRY, APPLY_BASH, APPLY_STUN, APPLY_PUNCH, APPLY_CLIMB,
  APPLY_GRIP, APPLY_SCRIBE, APPLY_BREW, APPLY_WEARSPELL, APPLY_REMOVESPELL,
  APPLY_EMOTION, APPLY_MENTALSTATE, APPLY_STRIPSN, APPLY_REMOVE, APPLY_DIG,
  APPLY_FULL, APPLY_THIRST, APPLY_DRUNK, APPLY_BLOOD, APPLY_HIT_REGEN,
  APPLY_MANA_REGEN, APPLY_MOVE_REGEN, APPLY_ANTIMAGIC, APPLY_AFF2,
  APPLY_ROOMFLAG, APPLY_SECTORTYPE, APPLY_ROOMLIGHT, APPLY_TELEVNUM,
  APPLY_TELEDELAY, APPLY_COOK, APPLY_BLAH, APPLY_RACE, APPLY_HITNDAM,
  APPLY_SAVING_ALL, APPLY_EAT_SPELL, APPLY_RACE_SLAYER, APPLY_ALIGN_SLAYER,
  APPLY_FINDTRAP, APPLY_NUMATTACKS, APPLY_BARENUMDIE, APPLY_BARESIZDIE,
  APPLY_NUMMEM, APPLY_IMMUNESPELL, APPLY_ABSORB,
  MAX_APPLY_TYPE
} apply_types;

#define REVERSE_APPLY		   1000

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		      1
#define CONT_PICKPROOF		      2
#define CONT_CLOSED		      4
#define CONT_LOCKED		      8

/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO		      1
#define ROOM_VNUM_HELL		      2
#define ROOM_VNUM_POLY		      3
#define ROOM_VNUM_CHAT		   1000
#define ROOM_VNUM_TEMPLE	     98
#define ROOM_VNUM_SCHOOL          29751
#define ROOM_START_GOOD		   3294
#define ROOM_START_EVIL		   7517
#define ROOM_START_NEUTRAL        13513
#define ROOM_AUTH_START                ROOM_VNUM_SCHOOL

#define ROOM_VNUM_ASTRAL_ENTRANCE  2701

#define VNUM_START_SCRATCH          850
#define VNUM_END_SCRATCH            899

#define ROOM_VNUM_START_SIGIL     100000
#define ROOM_VNUM_START_HEBER     101000
#define ROOM_VNUM_START_OVERGAARD 102000

/*
 * Room flags.           Holy cow!  Talked about stripped away..
 * Used in #ROOMS.       Those merc guys know how to strip code down.
 *			 Lets put it all back... ;)
 */

#define ROOM_DARK		BV00
#define ROOM_DEATH		BV01
#define ROOM_NO_MOB		BV02
#define ROOM_INDOORS		BV03
#define ROOM_LAWFUL		BV04
#define ROOM_NEUTRAL		BV05
#define ROOM_CHAOTIC		BV06
#define ROOM_NO_MAGIC		BV07
#define ROOM_TUNNEL		BV08
#define ROOM_PRIVATE		BV09
#define ROOM_SAFE		BV10
#define ROOM_SOLITARY		BV11
#define ROOM_PET_SHOP		BV12
#define ROOM_NO_RECALL		BV13
#define ROOM_DONATION		BV14
#define ROOM_NODROPALL		BV15
#define ROOM_SILENCE		BV16
#define ROOM_LOGSPEECH		BV17
#define ROOM_NODROP		BV18
#define ROOM_CLANSTOREROOM	BV19
#define ROOM_NO_SUMMON		BV20
#define ROOM_NO_ASTRAL		BV21
#define ROOM_TELEPORT		BV22
#define ROOM_TELESHOWDESC	BV23
#define ROOM_NOFLOOR		BV24
#define ROOM_RECEPTION		BV25
#define ROOM_BANK		BV26
#define ROOM_RIV_SRC		BV27
#define ROOM_ARENA		BV28
#define ROOM_NOMISSILE          BV29
#define ROOM_PROTOTYPE	     	BV30
#define ROOM_ORPHANED           BV31

#define DONATION_ROOM()		(get_room_index(40))
#define IS_DONATION(room)	(DONATION_ROOM() && \
				IS_SET(room->room_flags,ROOM_DONATION))

/*
 * Directions.
 * Used in #ROOMS.
 */
typedef enum
{
  DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST, DIR_UP, DIR_DOWN,
  DIR_NORTHEAST, DIR_NORTHWEST, DIR_SOUTHEAST, DIR_SOUTHWEST, DIR_SOMEWHERE,
  DIR_RESERVED1, DIR_RESERVED2, DIR_RESERVED3, DIR_RESERVED4,
  DIR_ARBITRARY
} dir_types;

#define MAX_REXITS	        20     /* Maximum exits allowed in 1 room */
#define LAST_NORMAL_DIR         DIR_SOMEWHERE
#define DIR_PORTAL		DIR_SOMEWHERE	/* portal direction	  */


/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR		  BV00
#define EX_CLOSED		  BV01
#define EX_LOCKED		  BV02
#define EX_SECRET		  BV03
#define EX_SWIM			  BV04
#define EX_PICKPROOF		  BV05
#define EX_FLY			  BV06
#define EX_CLIMB		  BV07
#define EX_DIG			  BV08
#define EX_RES1                   BV09	/* are these res[1-4] important? */
#define EX_NOPASSDOOR		  BV10
#define EX_HIDDEN		  BV11
#define EX_PASSAGE		  BV12
#define EX_PORTAL 		  BV13
#define EX_RES2			  BV14
#define EX_RES3			  BV15
#define EX_xCLIMB		  BV16
#define EX_xENTER		  BV17
#define EX_xLEAVE		  BV18
#define EX_xAUTO		  BV19
#define EX_RES4	  		  BV20
#define EX_xSEARCHABLE		  BV21
#define EX_BASHED                 BV22
#define EX_BASHPROOF              BV23
#define EX_NOMOB		  BV24
#define EX_WINDOW		  BV25
#define EX_xLOOK		  BV26
#define EX_NOFLY		  BV27
#define MAX_EXFLAG		  28

/*
 * Sector types.
 * Used in #ROOMS.
 */
typedef enum
{
  SECT_INSIDE, SECT_CITY, SECT_FIELD, SECT_FOREST, SECT_HILLS, SECT_MOUNTAIN,
  SECT_WATER_SWIM, SECT_WATER_NOSWIM, SECT_UNDERWATER, SECT_AIR, SECT_DESERT,
  SECT_DUNNO, SECT_OCEANFLOOR, SECT_UNDERGROUND, SECT_TREE, SECT_FIRE,
  SECT_QUICKSAND, SECT_ETHER, SECT_GLACIER, SECT_EARTH,
  SECT_MAX
} sector_types;

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
typedef enum
{
  WEAR_NONE = -1, WEAR_LIGHT = 0, WEAR_FINGER_L, WEAR_FINGER_R, WEAR_NECK_1,
  WEAR_NECK_2, WEAR_BODY, WEAR_HEAD, WEAR_LEGS, WEAR_FEET, WEAR_HANDS,
  WEAR_ARMS, WEAR_SHIELD, WEAR_ABOUT, WEAR_WAIST, WEAR_WRIST_L, WEAR_WRIST_R,
  WEAR_WIELD, WEAR_HOLD, WEAR_DUAL_WIELD, WEAR_EAR_L, WEAR_EAR_R,
  WEAR_EYES, WEAR_MISSILE_WIELD, WEAR_BACK, WEAR_NOSE, WEAR_ANKLE_L,
  WEAR_ANKLE_R, MAX_WEAR
} wear_locations;

/* Board Types */
typedef enum { BOARD_NOTE, BOARD_MAIL } board_types;

/* Auth Flags */
#define FLAG_WRAUTH		      1
#define FLAG_AUTH		      2

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
typedef enum
{
  COND_DRUNK, COND_FULL, COND_THIRST, COND_BLOODTHIRST, MAX_CONDS
} conditions;

#define MAX_COND_VAL	98

/*
 * Positions.
 */
typedef enum
{
  POS_DEAD, POS_MORTAL, POS_INCAP, POS_STUNNED, POS_SLEEPING,
  POS_RESTING, POS_SITTING, POS_FIGHTING, POS_STANDING, POS_MOUNTED,
  POS_SHOVE, POS_DRAG, POS_MEDITATING, MAX_POSITION
} positions;

/*
 * ACT bits for players.
 */
#define PLR_IS_NPC		      BV00	/* Don't EVER set.	*/
#define PLR_BOUGHT_PET		      BV01
#define PLR_SHOVEDRAG		      BV02
#define PLR_AUTOEXIT		      BV03
#define PLR_AUTOLOOT		      BV04
#define PLR_AUTOSAC                   BV05
#define PLR_BLANK		      BV06
#define PLR_OUTCAST 		      BV07
#define PLR_BRIEF		      BV08
#define PLR_COMBINE		      BV09
#define PLR_PROMPT		      BV10
#define PLR_TELNET_GA		      BV11

#define PLR_HOLYLIGHT		   BV12
#define PLR_NO_OOC          	   BV13
#define PLR_ROOMVNUM		   BV14

#define	PLR_SILENCE		   BV15
#define PLR_NO_EMOTE		   BV16
#define PLR_ATTACKER		   BV17
#define PLR_NO_TELL		   BV18
#define PLR_LOG			   BV19
#define PLR_DENY		   BV20
#define PLR_FREEZE		   BV21
#define PLR_THIEF	           BV22
#define PLR_KILLER	           BV23
#define PLR_LITTERBUG	           BV24
#define PLR_ANSI	           BV25
#define PLR_RIP		           BV26
#define PLR_NICE	           BV27
#define PLR_FLEE	           BV28
#define PLR_AUTOGOLD               BV29
#define PLR_AUTOMAP                BV30
#define PLR_AFK                    BV31

/*
 * ACT2 bits for players.
 */
#define PLR2_AUTOASSIST		   BV00
#define PLR2_BUSY		   BV01
#define PLR2_AFK_BUFFER		   BV02
#define PLR2_MONI_AFK		   BV03
#define PLR2_DIED                  BV04
#if 1 /* disable autogain, but don't re-use bit */
#define PLR2_AUTOGAIN              BV05
#endif
#define PLR2_MXP                   BV06
#define PLR2_MSP                   BV07

#define PLR2_PLANAR                BV16 /* same as ACT2_PLANAR - DONT CHANGE */

/* Bits for pc_data->flags. */
#define PCFLAG_R1                  BV00
#define PCFLAG_DEADLY              BV01
#define PCFLAG_UNAUTHED		   BV02
#define PCFLAG_NORECALL            BV03
#define PCFLAG_NOINTRO             BV04
#define PCFLAG_GAG		   BV05
#define PCFLAG_RETIRED             BV06
#define PCFLAG_GUEST               BV07
#define PCFLAG_NOSUMMON		   BV08
#define PCFLAG_PAGERON		   BV09
#define PCFLAG_NOTITLE             BV10
#define PCFLAG_WEB		   BV11

typedef enum
{
  TIMER_NONE, TIMER_RECENTFIGHT, TIMER_SHOVEDRAG, TIMER_DO_FUN,
TIMER_APPLIED, TIMER_PKILLED } timer_types;

struct timer_data
{
    TIMER  *	prev;
    TIMER  *	next;
    DO_FUN *	do_fun;
    int		value;
    sh_int	type;
    sh_int	count;
};


/*
 * Channel bits.
 */
#define	CHANNEL_AUCTION		   BV00
#define	CHANNEL_CHAT		   BV01
#define	CHANNEL_QUEST		   BV02
#define	CHANNEL_IMMTALK		   BV03
#define	CHANNEL_MUSIC		   BV04
#define	CHANNEL_ASK		   BV05
#define	CHANNEL_SHOUT		   BV06
#define	CHANNEL_GOSSIP		   BV07
#define CHANNEL_MONITOR		   BV08
#define CHANNEL_LOG		   BV09
#define CHANNEL_HIGHGOD		   BV10
#define CHANNEL_CLAN		   BV11
#define CHANNEL_BUILD		   BV12
#define CHANNEL_WHISPER		   BV13
#define CHANNEL_AVTALK		   BV14
#define CHANNEL_PRAY		   BV15
#define CHANNEL_COUNCIL 	   BV16
#define CHANNEL_GUILD              BV17
#define CHANNEL_COMM		   BV18
#define CHANNEL_TELLS		   BV19
#define CHANNEL_ORDER              BV20
#define CHANNEL_NEWBIE             BV21
#define CHANNEL_WARTALK            BV22
#define CHANNEL_LOGPC		   BV23
#define CHANNEL_HTTPD		   BV24
#define	CHANNEL_OOC		   BV25
#define CHANNEL_IMC		   BV26
#define CHANNEL_BUG                BV27
#define CHANNEL_DEBUG              BV28
#define CHANNEL_MAGIC              BV29
#define CHANNEL_IMCDEBUG           BV30
#define CHANNEL_IRC                BV31


/* Area defines - Scryn 8/11
 *
 */
#define AREA_DELETED		   BV00
#define AREA_LOADED                BV01

/* Area flags - Narn Mar/96 */
#define AFLAG_NOPKILL               BV00
#define AFLAG_ARENA		    BV01
#define AFLAG_DARK                  BV02

#define AFLAG_RESET_BOOT	    BV29 /* area loads/resets at boot */
#define AFLAG_INITIALIZED	    BV30 /* area is reset/should be reset */
#define AFLAG_MODIFIED		    BV31 /* area has been modified */

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
/* mod_data marker */
struct	mob_index_data
{
    MOB_INDEX_DATA *	next;
    MOB_INDEX_DATA *	next_sort;
    SPEC_FUN *		spec_fun;
    SHOP_DATA *		pShop;
    REPAIR_DATA *	rShop;
    MPROG_DATA *	mudprogs;
    EXT_BV     		progtypes;
    char *		player_name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    int			ivnum;
    sh_int		count;
    sh_int		killed;
    sh_int		sex;
    sh_int              levels[REAL_MAX_CLASS];
    sh_int              classes[REAL_MAX_CLASS];
    int			act;
    int			act2;
    int			affected_by;
    int			affected_by2;
    sh_int		alignment;
    sh_int		mobthac0;		/* Unused */
    sh_int		ac;
    sh_int		hitnodice;
    sh_int		hitsizedice;
    sh_int		hitplus;
    sh_int		damnodice;
    sh_int		damsizedice;
    sh_int		damplus;
    sh_int              antimagicp;  /*anti-magic percentage */
    int			numattacks;
    int                 money[MAX_CURR_TYPE];
    int                 balance[MAX_CURR_TYPE];
    int			gold2;
    int                 bank2;
    int			exp;
    int			xflags;
    int			resistant;
    int			immune;
    int			susceptible;
    int			absorb;
    int			attacks;
    int			defenses;
    int			speaks;
    int 		speaking;
    sh_int		position;
    sh_int		defposition;
    sh_int		height;
    sh_int		weight;
    sh_int		race;
    sh_int		hitroll;
    sh_int		damroll;
    sh_int		perm_str;
    sh_int		perm_int;
    sh_int		perm_wis;
    sh_int		perm_dex;
    sh_int		perm_con;
    sh_int		perm_cha;
    sh_int		perm_lck;
    sh_int		saving_poison_death;
    sh_int		saving_wand;
    sh_int		saving_para_petri;
    sh_int		saving_breath;
    sh_int		saving_spell_staff;
    AREA_DATA *		area;
};


struct hunt_hate_fear
{
    char *		name;
    CHAR_DATA *		who;
    time_t              start_time;
};

struct fighting_data
{
    CHAR_DATA *		who;
    sh_int		align;
    sh_int		duration;
    sh_int		timeskilled;
};

#define MAX_EDIT_LINES          98
#define MAX_EDIT_LINE_LENGTH    167
struct	editor_data
{
    sh_int		numlines;
    sh_int		on_line;
    sh_int		size;
    char		line[MAX_EDIT_LINES][MAX_EDIT_LINE_LENGTH];
};

struct	extracted_char_data
{
    EXTRACT_CHAR_DATA *	next;
    CHAR_DATA *		ch;
    ROOM_INDEX_DATA *	room;
    ch_ret		retcode;
    bool		extract;
};

/*
 * One character (PC or NPC).
 * (Shouldn't most of that build interface stuff use substate, dest_buf,
 * spare_ptr and tempnum?  Seems a little redundant)
 */
/* char_data marker */
struct	char_data
{
    CHAR_DATA *		next;
    CHAR_DATA *		prev;
    CHAR_DATA *		next_in_room;
    CHAR_DATA *		prev_in_room;
    CHAR_DATA *		master;
    CHAR_DATA *		leader;
    FIGHT_DATA *	fighting;
    CHAR_DATA *		reply;
    CHAR_DATA *		switched;
    CHAR_DATA *		mount;
    HHF_DATA *		hunting;
    HHF_DATA *		fearing;
    HHF_DATA *		hating;
    SPEC_FUN *		spec_fun;
    MPROG_ACT_LIST *	mpact;
    int			mpactnum;
    sh_int		mpscriptpos;
    MOB_INDEX_DATA *	pIndexData;
    DESCRIPTOR_DATA *	desc;
    AFFECT_DATA *	first_affect;
    AFFECT_DATA *	last_affect;
    NOTE_DATA *		pnote;
    NOTE_DATA *		comments;
    OBJ_DATA *		first_carrying;
    OBJ_DATA *		last_carrying;
    ROOM_INDEX_DATA *	in_room;
    ROOM_INDEX_DATA *	was_in_room;
    ROOM_INDEX_DATA *	idle_room;
    PC_DATA *		pcdata;
    DO_FUN *		last_cmd;
    DO_FUN *		prev_cmd;   /* mapping */
    void *		dest_buf;
    void *		spare_ptr;
    int			tempnum;
    EDITOR_DATA *	editor;
    TIMER	*	first_timer;
    TIMER	*	last_timer;
    char *		name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    char *		intro_descr;
    int                 vnum;
    sh_int		num_fighting;
    sh_int		substate;
    sh_int		sex;
    sh_int		race;
    sh_int		trust;
    sh_int              levels[REAL_MAX_CLASS];
    sh_int              classes[REAL_MAX_CLASS];
    int			played;
    sh_int              age_bonus;
    time_t		logon;
    time_t		save_time;
    sh_int		timer;
    sh_int		wait;
    sh_int		fight_wait;
    sh_int		hit;
    sh_int		max_hit;
    sh_int		hit_regen;
    sh_int		mana;
    sh_int		max_mana;
    sh_int		mana_regen;
    sh_int		move;
    sh_int		max_move;
    sh_int		move_regen;
    sh_int		practice;
    sh_int              antimagicp;  /*anti-magic percentage */
    sh_int		spellfail;
    int			numattacks;
    int                 money[MAX_CURR_TYPE];
    int                 balance[MAX_CURR_TYPE];
    int			gold2;
    int                 bank2;
    int			exp;
    int 		act;
    int 		act2;
    int			affected_by;
    int			affected_by2;
    int			carry_weight;
    int			carry_number;
    int			xflags;
    int			resistant;
    int			immune;
    int			susceptible;
    int			absorb;
    int			attacks;
    int			defenses;
    int			speaks;
    int			speaking;
    sh_int		saving_poison_death;
    sh_int		saving_wand;
    sh_int		saving_para_petri;
    sh_int		saving_breath;
    sh_int		saving_spell_staff;
    sh_int		alignment;
    sh_int		barenumdie;
    sh_int		baresizedie;
    sh_int		mobthac0;
    sh_int		hitroll;
    sh_int		damroll;
    sh_int		hitplus;
    sh_int		damplus;
    sh_int		position;
    sh_int		defposition;
    sh_int		height;
    sh_int		weight;
    sh_int		armor;
    sh_int		wimpy;
    int			deaf;
    sh_int		colors[MAX_COLOR_TYPE];
    sh_int		perm_str;
    sh_int		perm_int;
    sh_int		perm_wis;
    sh_int		perm_dex;
    sh_int		perm_con;
    sh_int		perm_cha;
    sh_int		perm_lck;
    sh_int		mod_str;
    sh_int		mod_int;
    sh_int		mod_wis;
    sh_int		mod_dex;
    sh_int		mod_con;
    sh_int		mod_cha;
    sh_int		mod_lck;
    sh_int		mental_state;		/* simplified */
    sh_int		emotional_state;	/* simplified */
    int			pagelen;                        /* BUILD INTERFACE */
    sh_int		inter_page;                     /* BUILD INTERFACE */
    sh_int		inter_type;                     /* BUILD INTERFACE */
    char  		*inter_editing;                 /* BUILD INTERFACE */
    int			inter_editing_vnum;             /* BUILD INTERFACE */
    sh_int		inter_substate;                 /* BUILD INTERFACE */
    int			retran;
    int			regoto;
    sh_int		mobinvis;	/* Mobinvis level SB */
    sh_int		cmd_recurse;
    int                 unum;
    VARC_DATA           *vars;
    int                 last_killed_by;
    INTRO_DATA          *first_intro;
    INTRO_DATA          *last_intro;
};


struct killed_data
{
    int			vnum;
    char		count;
};

/*
 * Data which only PC's have.
 */
struct	pc_data
{
    CLAN_DATA *		clan;
    COUNCIL_DATA * 	council;
    AREA_DATA *		area;
    DEITY_DATA *	deity;
    char *		homepage;
    char *		clan_name;
    char * 		council_name;
    char *		deity_name;
    char *		pwd;
    char *		bamfin;
    char *		bamfout;
    char *              rank;
    char *		title;
    char *		bestowments;	/* Special bestowed commands	   */
    int                 flags;		/* Whether the player is deadly and whatever else we add.      */
    int			pkills;		/* Number of pkills on behalf of clan */
    int			pdeaths;	/* Number of times pkilled (legally)  */
    int			mkills;		/* Number of mobs killed		   */
    int			mdeaths;	/* Number of deaths due to mobs       */
    int			illegal_pk;	/* Number of illegal pk's committed   */
    time_t              outcast_time;	/* The time at which the char was outcast */
    time_t              restore_time;	/* The last time the char did a restore all */
    int			r_range_lo;	/* room range */
    int			r_range_hi;
    int			m_range_lo;	/* mob range  */
    int			m_range_hi;
    int			o_range_lo;	/* obj range  */
    int			o_range_hi;
    sh_int		wizinvis;	/* wizinvis level */
    sh_int		min_snoop;	/* minimum snoop level */
    sh_int		condition	[MAX_CONDS];
    sh_int		learned		[MAX_SKILL];
    sh_int		memorized      	[MAX_SKILL];
    sh_int              interface;
    KILLED_DATA		killed		[MAX_KILLTRACK];
    sh_int		quest_number;	/* current *QUEST BEING DONE* DON'T REMOVE! */
    sh_int		quest_curr;	/* current number of quest points */
    int			quest_accum;	/* quest points accumulated in players life */
    sh_int		favor;		/* deity favor */
    int			auth_state;
    time_t		release_date;	/* Auto-helling.. Altrag */
    char *		helled_by;
    char *		bio;		/* Personal Bio */
    char *		authed_by;	/* what crazy imm authed this name ;) */
    SKILLTYPE *		special_skills[5]; /* personalized skills/spells */
    char *		prompt;		/* User config prompts */
    char *		subprompt;	/* Substate prompt */
    sh_int		pagerlen;	/* For pager (NOT menus) */
    bool		openedtourney;
    int			home;
    ALIAS_DATA *	first_alias;
    ALIAS_DATA *	last_alias;
    GAME_BOARD_DATA *	game_board;
    sh_int              log_severity[LOG_LAST];
    sh_int              afk_log_severity[LOG_LAST];
    time_t              time_created;
    time_t              time_immortal;
    time_t              time_to_die;
    bool                inc_times_played;
    sh_int              times_played;

#ifdef I3
    I3_CHARDATA         *i3chardata;
#endif

#ifdef VTRACK
    VTRACK_DATA         *vtrack;
#endif

#ifdef IMC
    IMC_CHARDATA        *imcchardata;
#endif
};



/*
 * Liquids.
 */
#define LIQ_WATER        0
#define LIQ_MAX		18

struct	liq_type
{
    char *	liq_name;
    char *	liq_color;
    sh_int	liq_affect[3];
};

/*
 * Damage types from the attack_table[]
 */
typedef enum
{
   DAM_HIT, DAM_CLEAVE, DAM_STAB, DAM_SLASH, DAM_WHIP, DAM_CLAW,
   DAM_BLAST, DAM_POUND, DAM_CRUSH, DAM_SMASH, DAM_BITE, DAM_PIERCE,
   DAM_STING, DAM_BOLT, DAM_ARROW, DAM_DART, DAM_STONE, DAM_PEA,
   DAM_MAX_TYPE
} damage_types;

/*
 * Extra description data for a room or object.
 */
struct	extra_descr_data
{
    EXTRA_DESCR_DATA *next;	/* Next in list                     */
    EXTRA_DESCR_DATA *prev;	/* Previous in list                 */
    char *keyword;              /* Keyword in look/examine          */
    char *description;          /* What to see                      */
};



/*
 * Prototype for an object.
 */
struct	obj_index_data
{
    OBJ_INDEX_DATA *	next;
    OBJ_INDEX_DATA *	next_sort;
    EXTRA_DESCR_DATA *	first_extradesc;
    EXTRA_DESCR_DATA *	last_extradesc;
    AFFECT_DATA *	first_affect;
    AFFECT_DATA *	last_affect;
    MPROG_DATA *	mudprogs;               /* objprogs */
    EXT_BV     		progtypes;              /* objprogs */
    char *		name;
    char *		short_descr;
    char *		description;
    char *		action_desc;
    int			ivnum;
    sh_int		item_type;
    int			extra_flags;
    int			extra_flags2;
    int			magic_flags; /*Need more bitvectors for spells - Scryn*/
    int			wear_flags;
    sh_int		count;
    sh_int		weight;
    int			cost;
    int			value	[6];
/*    int			serial;*/
    sh_int		layers;
    int			rent;			/* Unused */
    SPEC_FUN *		spec_fun;
    AREA_DATA *		area;
    sh_int		currtype;
};


/*
 * One object.
 */
struct  obj_data
{
    OBJ_DATA *		next;
    OBJ_DATA *		prev;
    OBJ_DATA *		next_content;
    OBJ_DATA *		prev_content;
    OBJ_DATA *		first_content;
    OBJ_DATA *		last_content;
    OBJ_DATA *		in_obj;
    CHAR_DATA *		carried_by;
    EXTRA_DESCR_DATA *	first_extradesc;
    EXTRA_DESCR_DATA *	last_extradesc;
    AFFECT_DATA *	first_affect;
    AFFECT_DATA *	last_affect;
    OBJ_INDEX_DATA *	pIndexData;
    ROOM_INDEX_DATA *	in_room;
    char *		name;
    char *		short_descr;
    char *		description;
    char *		action_desc;
    char *		nickname;
    char *		last_carried_by;
    int                 vnum;
    sh_int		item_type;
    sh_int		mpscriptpos;
    int			extra_flags;
    int			extra_flags2;
    int			magic_flags; /*Need more bitvectors for spells - Scryn*/
    int			wear_flags;
    MPROG_ACT_LIST *	mpact;		/* mudprogs */
    int			mpactnum;	/* mudprogs */
    sh_int		wear_loc;
    sh_int		weight;
    int			cost;
    sh_int		timer;
    int			value	[6];
    sh_int		count;		/* support for object grouping */
    int			serial;		/* serial number	       */
    int			rent;
    SPEC_FUN *		spec_fun;
    sh_int		currtype;
    CHRISTEN_DATA       *christened;
    int                 unum;
    VARC_DATA           *vars;
};


/*
 * Exit data.
 */
struct	exit_data
{
    EXIT_DATA *		prev;		/* previous exit in linked list	*/
    EXIT_DATA *		next;		/* next exit in linked list	*/
    EXIT_DATA *		rexit;		/* Reverse exit pointer		*/
    ROOM_INDEX_DATA *	to_room;	/* Pointer to destination room	*/
    char *		keyword;	/* Keywords for exit or door	*/
    char *		description;	/* Description of exit		*/
    int			vnum;		/* Vnum of room exit leads to	*/
    int			rvnum;		/* Vnum of room in opposite dir	*/
    int			exit_info;	/* door states & other flags	*/
    int			key;		/* Key vnum			*/
    sh_int		vdir;		/* Physical "direction"		*/
    sh_int           rdir;           /* Reverse "direction"          */
    sh_int		distance;	/* how far to the next room	*/
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'H': hide an object
 *   'B': set a bitvector
 *   'T': trap an object
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct	reset_data
{
    RESET_DATA *	next;
    RESET_DATA *	prev;
    char		command;
    int			extra;
    int			arg1;
    int			arg2;
    int			arg3;
};

/* Constants for arg2 of 'B' resets. */
#define	BIT_RESET_DOOR			0
#define BIT_RESET_OBJECT		1
#define BIT_RESET_MOBILE		2
#define BIT_RESET_ROOM			3
#define BIT_RESET_TYPE_MASK		0xFF	/* 256 should be enough */
#define BIT_RESET_DOOR_THRESHOLD	8
#define BIT_RESET_DOOR_MASK		0xFF00	/* 256 should be enough */
#define BIT_RESET_SET			BV30
#define BIT_RESET_TOGGLE		BV31
#define BIT_RESET_FREEBITS	  0x3FFF0000	/* For reference */

typedef enum
{
    PLANE_NORMAL, PLANE_ASTRAL, PLANE_DESERT, PLANE_ARTIC,
    PLANE_UNDERGROUND, PLANE_FIRE, PLANE_WATER, PLANE_AIR, PLANE_EARTH,
    PLANE_PRIME, PLANE_ETHEREAL, PLANE_OTHER, PLANE_MOUNT_CELESTIA,
    PLANE_BYTOPIA, PLANE_ELYSIUM, PLANE_BEASTLANDS, PLANE_ARBOREA,
    PLANE_PANDEMONIUM, PLANE_ABYSS, PLANE_CARCERI, PLANE_HADES,
    PLANE_GEHENNA, PLANE_BAATOR, PLANE_ACHERON, PLANE_MECHANUS,
    PLANE_ARCADIA, PLANE_ASGARD, PLANE_UNIQUE, PLANE_GRAY_WASTE,
    PLANE_LIMBO, PLANE_OUTLANDS,
    PLANE_LAST
} plane_const;

#define FIRST_PLANE     PLANE_NORMAL
#define LAST_PLANE      PLANE_LAST
#define MAX_PLANES      PLANE_LAST

/*
 * Area definition.
 */
struct	area_data
{
    AREA_DATA *		next;
    AREA_DATA *		prev;
    AREA_DATA *		next_sort;
    AREA_DATA *		prev_sort;
    RESET_DATA *	first_reset;
    RESET_DATA *	last_reset;
    char *		name;
    char *		filename;
    int                 flags;
    sh_int              status;  /* h, 8/11 */
    sh_int		age;
    sh_int		nplayer;
    sh_int		reset_frequency;
    int			low_r_vnum;
    int			hi_r_vnum;
    int			low_o_vnum;
    int			hi_o_vnum;
    int			low_m_vnum;
    int			hi_m_vnum;
    int			low_soft_range;
    int			hi_soft_range;
    int			low_hard_range;
    int			hi_hard_range;
    char *		author; /* Scryn */
    char *              resetmsg; /* Rennard */
    char *              comment;
    RESET_DATA *	last_mob_reset;
    RESET_DATA *	last_obj_reset;
    sh_int		max_players;
    int			mkills;
    int			mdeaths;
    int			pkills;
    int			pdeaths;
    int			looted[MAX_CURR_TYPE];
    int			illegal_pk;
    int			high_economy[MAX_CURR_TYPE];
    int			low_economy[MAX_CURR_TYPE];
    WEATHER_DATA *	weather; /* FB */
    sh_int     		currvnum;
    CURR_INDEX_DATA *   currindex;
    sh_int              plane;
    sh_int              area_version;
};



/*
 * Load in the gods building data. -- Altrag
 */
struct	godlist_data
{
    GOD_DATA *		next;
    GOD_DATA *		prev;
    int			level;
    int			low_r_vnum;
    int			hi_r_vnum;
    int			low_o_vnum;
    int			hi_o_vnum;
    int			low_m_vnum;
    int			hi_m_vnum;
};


struct log_channel_def
{
    char *name;
    int channel;
    sh_int level;
    unsigned int num_logs;
};
/*
 * Used to keep track of system settings and statistics		-Thoric
 */
struct	system_data
{
    int		maxplayers;		/* Maximum players this boot   */
    int		alltimemax;		/* Maximum players ever	  */
    int		total_logins;		/* # logins since 04/16/00 */
    char *	time_of_max;		/* Time of max ever */
    int         longest_uptime;
    bool	NO_NAME_RESOLVING;	/* Hostnames are not resolved  */
    bool    	DENY_NEW_PLAYERS;	/* New players cannot connect  */
    bool	WAIT_FOR_AUTH;		/* New players must be auth'ed */
    bool	specials_enabled;
    sh_int	read_all_mail;		/* Read all player mail(was 54)*/
    sh_int	read_mail_free;		/* Read mail for free (was 51) */
    sh_int	write_mail_free;	/* Write mail for free(was 51) */
    sh_int	take_others_mail;	/* Take others mail (was 54)   */
    sh_int	muse_level;		/* Level of muse channel */
    sh_int	think_level;		/* Level of think channel LEVEL_HIGOD*/
    sh_int	build_level;		/* Level of build channel LEVEL_BUILD*/
    sh_int	log_level;		/* Level of log channel LEVEL LOG*/
    sh_int	level_modify_proto;	/* Level to modify prototype stuff LEVEL_LESSER */
    sh_int	level_override_private;	/* override private flag */
    sh_int	level_mset_player;	/* Level to mset a player */
    sh_int	stun_plr_vs_plr;	/* Stun mod player vs. player */
    sh_int	stun_regular;		/* Stun difficult */
    sh_int	dam_plr_vs_plr;		/* Damage mod player vs. player */
    sh_int	dam_plr_vs_mob;		/* Damage mod player vs. mobile */
    sh_int	dam_mob_vs_plr;		/* Damage mod mobile vs. player */
    sh_int	dam_mob_vs_mob;		/* Damage mod mobile vs. mobile */
    sh_int	level_getobjnotake;     /* Get objects without take flag */
    sh_int      level_forcepc;          /* The level at which you can use force on players. */
    sh_int	max_sn;			/* Max skills */
    char       *guild_overseer;         /* Pointer to char containing the name of the */
    char       *guild_advisor;		/* guild overseer and advisor. */
    int		save_flags;		/* Toggles for saving conditions */
    sh_int	save_frequency;		/* How old to autosave someone */
    bool	intro_disabled;
    sh_int      percent_aggr;           /* How aggressive are aggies */
    struct log_channel_def logdefs[LOG_LAST];
};


struct stats_data
{
    long        bytes_in;
    long        bytes_out;
    long        boot_bytes_in;
    long        boot_bytes_out;
    long        comp_bytes_in;
    long        comp_bytes_out;
    long        comp_boot_bytes_in;
    long        comp_boot_bytes_out;
};

/*
 * Room type.
 */
struct	room_index_data
{
    ROOM_INDEX_DATA *	next;
    ROOM_INDEX_DATA *	next_sort;
    CHAR_DATA *		first_person;
    CHAR_DATA *		last_person;
    OBJ_DATA *		first_content;
    OBJ_DATA *		last_content;
    EXTRA_DESCR_DATA *	first_extradesc;
    EXTRA_DESCR_DATA *	last_extradesc;
    AREA_DATA *		area;
    EXIT_DATA *		first_exit;
    EXIT_DATA *		last_exit;
    AFFECT_DATA *	first_affect;
    AFFECT_DATA *	last_affect;
    char *		name;
    MAP_DATA *		map;                 /* maps */
    char *		description;
    int			vnum;
    int			room_flags;
    MPROG_ACT_LIST *	mpact;               /* mudprogs */
    int			mpactnum;            /* mudprogs */
    MPROG_DATA *	mudprogs;            /* mudprogs */
    sh_int		mpscriptpos;
    EXT_BV              progtypes;           /* mudprogs */
    sh_int		light;
    sh_int		sector_type;
    int			tele_vnum;
    sh_int		tele_delay;
    sh_int		tunnel;		     /* max people that will fit */
    int			elevation;
    sh_int		liquid;
    RIVER_DATA *	river;
    SPEC_FUN *		spec_fun;
    CURR_INDEX_DATA *   currindex;
    sh_int     		currvnum;
};

/*
 * Delayed teleport type.
 */
struct	teleport_data
{
    TELEPORT_DATA *	next;
    TELEPORT_DATA *	prev;
    ROOM_INDEX_DATA *	room;
    sh_int		timer;
};

/*
 * Flowing rivers by Jesse
 */
struct	river_data
{
    sh_int depth;	/* Depth of water */
    sh_int speed;	/* Speed of water */
    EXIT_DATA *to;	/* Direction to which the water flows */
};

void calc_river_path(ROOM_INDEX_DATA *rp, sh_int liq);

#define IS_RIVER(rm)		(rm && rm->river)
#define IS_RIVER_SOURCE(rm)	(IS_SET(rm->room_flags,ROOM_RIV_SRC))
#define RIVER_LIQUID(rm)	(rm->liquid)
#define ROOM_ELEVATION(rm)	(rm->elevation)
#define RIVER_SPEED(rm)		(rm->river->speed)
#define RIVER_DEPTH(rm)		(rm->river->depth)
#define RIVER_TO(rm)		(rm->river->to)
#define MAX_ELEVATION		32000

/*
 * Types of skill numbers.  Used to keep separate lists of sn's
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000  /* allows for 1000 skills/spells */
#define TYPE_HERB		     2000  /* allows for 1000 attack types  */
#define TYPE_PERSONAL		     3000  /* allows for 1000 herb types    */

/*
 *  Target types.
 */
typedef enum
{
  TAR_IGNORE, TAR_CHAR_OFFENSIVE, TAR_CHAR_DEFENSIVE, TAR_CHAR_SELF,
  TAR_OBJ_INV, TAR_OBJ_ROOM
} target_types;

typedef enum
{
  SKILL_UNKNOWN, SKILL_SPELL, SKILL_SKILL, SKILL_WEAPON, SKILL_TONGUE,
  SKILL_HERB, SKILL_LORE, SKILL_PSISPELL, SKILL_MAXTYPE
} skill_types;



struct timerset
{
  int num_uses;
  struct timeval total_time;
  struct timeval min_time;
  struct timeval max_time;
};



/*
 * Skills include spells as a particular case.
 */
struct	skill_type
{
    char *	name;			/* Name of skill		*/
    sh_int      skill_level[REAL_MAX_CLASS];	/* Level needed by class	*/
    sh_int      skill_adept[REAL_MAX_CLASS];	/* Max attainable % in this skill */
    SPELL_FUN *	spell_fun;		/* Spell pointer (for spells)	*/
    DO_FUN *	skill_fun;		/* Skill pointer (for skills)	*/
    sh_int	target;			/* Legal targets		*/
    sh_int	minimum_position;	/* Position for caster / user	*/
    sh_int	slot;			/* Slot for #OBJECT loading	*/
    sh_int	min_mana;		/* Minimum mana used		*/
    sh_int      class_mana[REAL_MAX_CLASS]; /* Mana used by class       */
    sh_int	beats;			/* Rounds required to use skill	*/
    sh_int      class_beats[REAL_MAX_CLASS]; /* Rounds required by class */
    char *	noun_damage;		/* Damage message		*/
    char *	msg_off;		/* Wear off message		*/
    char *	msg_off_room;		/* Wear off room message	*/
    char *	msg_off_soon;		/* Wear off soon message	*/
    char *	msg_off_soon_room;	/* Wear off soon room message 	*/
    sh_int	guild;			/* Which guild the skill belongs to */
    sh_int	min_level;		/* Minimum level to be able to cast */
    sh_int	type;			/* Spell/Skill/Weapon/Tongue	*/
    int		flags;			/* extra stuff			*/
    sh_int      dam_type;
    sh_int      act_type;
    sh_int      power_type;
    sh_int      class_type;
    sh_int      save_type;
    char *	hit_char;		/* Success message to caster	*/
    char *	hit_vict;		/* Success message to victim	*/
    char *	hit_room;		/* Success message to room	*/
    char *	miss_char;		/* Failure message to caster	*/
    char *	miss_vict;		/* Failure message to victim	*/
    char *	miss_room;		/* Failure message to room	*/
    char *	die_char;		/* Victim death msg to caster	*/
    char *	die_vict;		/* Victim death msg to victim	*/
    char *	die_room;		/* Victim death msg to room	*/
    char *	imm_char;		/* Victim immune msg to caster	*/
    char *	imm_vict;		/* Victim immune msg to victim	*/
    char *	imm_room;		/* Victim immune msg to room	*/
    char *	abs_char;		/* Victim absorb msg to caster	*/
    char *	abs_vict;		/* Victim absorb msg to victim	*/
    char *	abs_room;		/* Victim absorb msg to room	*/
    char *      corpse_string;          /* String added to corpse       */
    char *	dice;			/* Dice roll			*/
    int         corpse_stage;           /* Stage of decay               */
    int		value;			/* Misc value			*/
    char	difficulty;		/* Difficulty of casting/learning */
    SMAUG_AFF *	affects;		/* Spell affects, if any	*/
    char *	components;		/* Spell components, if any	*/
    char *	teachers;		/* Skill requires a special teacher */
    char	participants;		/* # of required participants	*/
    struct	timerset	userec;	/* Usage record			*/
    char *      part_start_char;        /* participant messages */
    char *      part_start_room;
    char *      part_end_char;
    char *      part_end_vict;
    char *      part_end_room;
    char *      part_end_caster;
    char *      part_miss_char;
    char *      part_miss_room;
    char *      part_abort_char;        /* participant messages */
};


struct  auction_data
{
    OBJ_DATA  * item;   /* a pointer to the item */
    CHAR_DATA * seller; /* a pointer to the seller - which may NOT quit */
    CHAR_DATA * buyer;  /* a pointer to the buyer - which may NOT quit */
    int         bet;    /* last bet - or 0 if noone has bet anything */
    sh_int      currtype;
    sh_int      going;  /* 1,2, sold */
    sh_int      pulse;  /* how many pulses (.25 sec) until another call-out ? */
    int 	starting;
};


/*
 * Utility macros.
 */
#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)	((var) ^= (bit))
#define CH(d)			((d)->original ? (d)->original : (d)->character)

#define IS_AREA_FLAG(var, bit)         IS_SET((var)->flags, (bit))
#define SET_AREA_FLAG(var, bit )       SET_BIT((var)->flags, (bit))
#define REMOVE_AREA_FLAG(var, bit)     REMOVE_BIT((var)->flags, (bit))
#define IS_AREA_STATUS(var, bit)       IS_SET((var)->status, (bit))
#define SET_AREA_STATUS(var, bit)      SET_BIT((var)->status, (bit))
#define REMOVE_AREA_STATUS(var, bit)   REMOVE_BIT((var)->status, (bit))
#define IS_ROOM_FLAG(var, bit)         IS_SET((var)->room_flags, (bit))
#define SET_ROOM_FLAG(var, bit)        SET_BIT((var)->room_flags, (bit))
#define REMOVE_ROOM_FLAG(var, bit)     REMOVE_BIT((var)->room_flags, (bit))
#define IS_ACT_FLAG(var, bit)          IS_SET((var)->act, (bit))
#define SET_ACT_FLAG(var, bit)         SET_BIT((var)->act, (bit))
#define REMOVE_ACT_FLAG(var, bit)      REMOVE_BIT((var)->act, (bit))
#define IS_ACT2_FLAG(var, bit)         IS_SET((var)->act2, (bit))
#define SET_ACT2_FLAG(var, bit)        SET_BIT((var)->act2, (bit))
#define REMOVE_ACT2_FLAG(var, bit)     REMOVE_BIT((var)->act2, (bit))
#define IS_PLR_FLAG(var, bit)          IS_SET((var)->act, (bit))
#define SET_PLR_FLAG(var, bit)         SET_BIT((var)->act, (bit))
#define REMOVE_PLR_FLAG(var, bit)      REMOVE_BIT((var)->act, (bit))
#define IS_PLR2_FLAG(var, bit)         IS_SET((var)->act2, (bit))
#define SET_PLR2_FLAG(var, bit)        SET_BIT((var)->act2, (bit))
#define REMOVE_PLR2_FLAG(var, bit)     REMOVE_BIT((var)->act2, (bit))
#define IS_SAVE_FLAG(var, bit)         IS_SET((var).save_flags, (bit))
#define SET_SAVE_FLAG(var, bit)        SET_BIT((var).save_flags, (bit))
#define REMOVE_SAVE_FLAG(var, bit)     REMOVE_BIT((var).save_flags, (bit))
#define IS_PC_FLAG(var, bit)           IS_SET((var)->pcdata->flags, (bit))
#define SET_PC_FLAG(var, bit)          SET_BIT((var)->pcdata->flags, (bit))
#define REMOVE_PC_FLAG(var, bit)       REMOVE_BIT((var)->pcdata->flags, (bit))
#define IS_EXIT_FLAG(var, bit)         IS_SET((var)->exit_info, (bit))
#define SET_EXIT_FLAG(var, bit)        SET_BIT((var)->exit_info, (bit))
#define REMOVE_EXIT_FLAG(var, bit)     REMOVE_BIT((var)->exit_flags, (bit))

#define IS_IMMUNE(ch, ris)	       IS_SET((ch)->immune, (ris))
#define IS_RESIS(ch, ris)	       IS_SET((ch)->resistant, (ris))
#define IS_SUSCEP(ch, ris)	       IS_SET((ch)->susceptible, (ris))

/*
 * Macros for accessing virtually unlimited bitvectors.		-Thoric
 *
 * Note that these macros use the bit number rather than the bit value
 * itself -- which means that you can only access _one_ bit at a time
 *
 * This code uses an array of integers
 */

/*
 * The functions for these prototypes can be found in misc.c
 * They are up here because they are used by the macros below
 */
bool	ext_is_empty		args( ( EXT_BV *bits ) );
void	ext_clear_bits		args( ( EXT_BV *bits ) );
int	ext_has_bits		args( ( EXT_BV *var, EXT_BV *bits) );
bool	ext_same_bits		args( ( EXT_BV *var, EXT_BV *bits) );
void	ext_set_bits		args( ( EXT_BV *var, EXT_BV *bits) );
void	ext_remove_bits		args( ( EXT_BV *var, EXT_BV *bits) );
void	ext_toggle_bits		args( ( EXT_BV *var, EXT_BV *bits) );

/*
 * Here are the extended bitvector macros:
 */
#define xIS_SET(var, bit)	((var).bits[(bit) >> RSV] & 1 << ((bit) & XBM))
#define xSET_BIT(var, bit)	((var).bits[(bit) >> RSV] |= 1 << ((bit) & XBM))
#define xSET_BITS(var, bit)	(ext_set_bits(&(var), &(bit)))
#define xREMOVE_BIT(var, bit)	((var).bits[(bit) >> RSV] &= ~(1 << ((bit) & XBM)))
#define xREMOVE_BITS(var, bit)	(ext_remove_bits(&(var), &(bit)))
#define xTOGGLE_BIT(var, bit)	((var).bits[(bit) >> RSV] ^= 1 << ((bit) & XBM))
#define xTOGGLE_BITS(var, bit)	(ext_toggle_bits(&(var), &(bit)))
#define xCLEAR_BITS(var)	(ext_clear_bits(&(var)))
#define xIS_EMPTY(var)		(ext_is_empty(&(var)))
#define xHAS_BITS(var, bit)	(ext_has_bits(&(var), &(bit)))
#define xSAME_BITS(var, bit)	(ext_same_bits(&(var), &(bit)))

/*
 * Memory allocation macros.
 */

#define CREATE(result, type, number)				\
do								\
{								\
    if (!((result) = (type *) calloc ((number), sizeof(type))))	\
    { perror("malloc failure"); abort(); }			\
} while(0)

#define RECREATE(result,type,number)				\
do								\
{								\
  if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
	{ perror("realloc failure"); abort(); }			\
} while(0)


#define DISPOSE(point) 						\
do								\
{								\
  if (!(point))							\
  {								\
	bug( "Freeing null pointer in %s at line %d.", __FILE__, __LINE__ ); \
	fprintf( stderr, "DISPOSEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else free((point));						\
  (point) = NULL;							\
} while(0)							\

#ifdef HASHSTR
#define STRALLOC(point)		str_alloc((point))
#define QUICKLINK(point)	quick_link((point))
#define QUICKMATCH(p1, p2)	(int) (p1) == (int) (p2)
#define STRFREE(point)						\
do								\
{								\
  if (!(point))							\
  {								\
	bug( "Freeing null pointer in %s at line %d.", __FILE__, __LINE__ ); \
	fprintf( stderr, "STRFREEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else if (str_free((point))==-1) 				\
    fprintf( stderr, "STRFREEing bad pointer in %s, line %d\n", __FILE__, __LINE__ ); \
  (point) = NULL;							\
} while(0)
#else
#define STRALLOC(point)		str_dup((point))
#define QUICKLINK(point)	str_dup((point))
#define QUICKMATCH(p1, p2)	strcmp((p1), (p2)) == 0
#define STRFREE(point)						\
do								\
{								\
  if (!(point))							\
  {								\
	bug( "Freeing null pointer in %s at line %d.", __FILE__, __LINE__ ); \
	fprintf( stderr, "STRFREEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else free((point));						\
  (point) = NULL;							\
} while(0)
#endif

/* double-linked list handling macros -Thoric */

#define LINK(link, first, last, next, prev)			\
do								\
{								\
    if ( !(first) )						\
      (first)			= (link);			\
    else							\
      (last)->next		= (link);			\
    (link)->next		= NULL;				\
    (link)->prev		= (last);			\
    (last)			= (link);			\
} while(0)

#define INSERT(link, insert, first, next, prev)			\
do								\
{								\
    (link)->prev		= (insert)->prev;		\
    if ( !(insert)->prev )					\
      (first)			= (link);			\
    else							\
      (insert)->prev->next	= (link);			\
    (insert)->prev		= (link);			\
    (link)->next		= (insert);			\
} while(0)

#define UNLINK(link, first, last, next, prev)			\
do								\
{								\
    if ( !(link)->prev )					\
      (first)			= (link)->next;			\
    else							\
      (link)->prev->next	= (link)->next;			\
    if ( !(link)->next )					\
      (last)			= (link)->prev;			\
    else							\
      (link)->next->prev	= (link)->prev;			\
} while(0)


#define CHECK_LINKS(first, last, next, prev, type)		\
do {								\
  type *ptr, *pptr = NULL;					\
  if ( !(first) && !(last) )					\
    break;							\
  if ( !(first) )						\
  {								\
    bug( "CHECK_LINKS: last with NULL first!  %s.",		\
        #first );					\
    for ( ptr = (last); ptr->prev; ptr = ptr->prev );		\
    (first) = ptr;						\
  }								\
  else if ( !(last) )						\
  {								\
    bug( "CHECK_LINKS: first with NULL last!  %s.",		\
        #first );					\
    for ( ptr = (first); ptr->next; ptr = ptr->next );		\
    (last) = ptr;						\
  }								\
  if ( (first) )						\
  {								\
    for ( ptr = (first); ptr; ptr = ptr->next )			\
    {								\
      if ( ptr->prev != pptr )					\
      {								\
        bug( "CHECK_LINKS(%s): %p:->prev != %p.  Fixing.",	\
            #first, ptr, pptr );			\
        ptr->prev = pptr;					\
      }								\
      if ( ptr->prev && ptr->prev->next != ptr )		\
      {								\
        bug( "CHECK_LINKS(%s): %p:->prev->next != %p.  Fixing.",\
            #first, ptr, ptr );			\
        ptr->prev->next = ptr;					\
      }								\
      pptr = ptr;						\
    }								\
    pptr = NULL;						\
  }								\
  if ( (last) )							\
  {								\
    for ( ptr = (last); ptr; ptr = ptr->prev )			\
    {								\
      if ( ptr->next != pptr )					\
      {								\
        bug( "CHECK_LINKS (%s): %p:->next != %p.  Fixing.",	\
            #first, ptr, pptr );			\
        ptr->next = pptr;					\
      }								\
      if ( ptr->next && ptr->next->prev != ptr )		\
      {								\
        bug( "CHECK_LINKS(%s): %p:->next->prev != %p.  Fixing.",\
            #first, ptr, ptr );			\
        ptr->next->prev = ptr;					\
      }								\
      pptr = ptr;						\
    }								\
  }								\
} while(0)


#define CHECK_SUBRESTRICTED(ch)					\
do								\
{								\
    if ( (ch)->substate == SUB_RESTRICTED )			\
    {								\
	send_to_char( "You cannot use this command from within another command.\n\r", ch );	\
	return;							\
    }								\
} while(0)


/*
 * Character macros.
 */
#define GET_AMAGICP(ch)         ((ch)->antimagicp)
#define GET_INTF(ch)		(IS_NPC((ch)) ? INT_DEFAULT : ((ch)->pcdata->interface))
#define GET_MANA(ch)            ((ch)->mana)
#define GET_MAX_MANA(ch)        (mana_limit(ch))
#define GET_HIT(ch)             ((ch)->hit)
#define GET_MAX_HIT(ch)         (hit_limit(ch))
#define GET_MOVE(ch)            ((ch)->move)
#define GET_MAX_MOVE(ch)        (move_limit(ch))
#define GET_EXP(ch)             ((ch)->exp)

#define GET_OLD_COND(ch, i)         ((ch)->pcdata->condition[(i)])

/*
 * Hunger and thirst were annoying for all parties involved, so it's gone -Garil
 */
#ifdef EAT_TO_LIVE
#define GET_COND GET_OLD_COND
#else
#define GET_COND(ch, i)         (((i)==COND_FULL || (i)==COND_THIRST) ? MAX_COND_VAL-10 : GET_OLD_COND((ch), (i)))
#endif

#define GET_PRACS(ch)           ((ch)->practice)
#define GET_ALIGN(ch)           ((ch)->alignment)
#define GET_POS(ch)             ((ch)->position)
#define GET_RANK(ch)            ((ch)->pcdata->rank)
#define GET_NAME(ch)            ((ch)->name)
#define GET_TITLE(ch)           ((ch)->pcdata->title)
#define GET_BLOOD(ch)           ((ch)->pcdata->condition[COND_BLOODTHIRST])
#define GET_MAX_BLOOD(ch)       (HAS_CLASS((ch), CLASS_VAMPIRE) ? \
                                (10 + GET_LEVEL((ch), CLASS_VAMPIRE)) : \
                                (GetMaxLevel((ch)) +10))
#define GET_TIME_PLAYED(ch)     (((ch)->played + (current_time - (ch)->logon)) / 3600)
#define GET_RACE(ch)            ((ch)->race)

#define NO_ANSI(s)              (ParseAnsiColors(0, s))

#define IS_NPC(ch)		(IS_SET((ch)->act, ACT_IS_NPC))
#define IS_IMMORTAL(ch)		((!IS_NPC(ch) && get_trust((ch)) >= LEVEL_IMMORTAL) || \
				(IS_NPC(ch) && IS_SET(ch->act,ACT_IMMORTAL)))
#define IS_HERO(ch)		(get_trust((ch)) >= LEVEL_HERO)
#define IS_AFFECTED(ch, sn)	(IS_SET((ch)->affected_by, (sn)))
#define SET_AFFECTED(ch, sn)	(SET_BIT((ch)->affected_by, (sn)))
#define REMOVE_AFFECTED(ch, sn)	(REMOVE_BIT((ch)->affected_by, (sn)))
#define IS_AFFECTED2(ch, sn)	(IS_SET((ch)->affected_by2, (sn)))
#define SET_AFFECTED2(ch, sn)	(SET_BIT((ch)->affected_by2, (sn)))
#define REMOVE_AFFECTED2(ch, sn) (REMOVE_BIT((ch)->affected_by2, (sn)))
#define HAS_BODYPART(ch, part)	((ch)->xflags == 0 || IS_SET((ch)->xflags, (part)))

#define CAN_CAST(ch)		(TRUE)

#define IS_VAMPIRE(ch)		(!IS_NPC(ch)				    \
				&& ((ch)->race==RACE_VAMPIRE		    \
                                ||  HAS_CLASS((ch), CLASS_VAMPIRE)))
#define IS_GOOD(ch)		((ch)->alignment >= 350)
#define IS_EVIL(ch)		((ch)->alignment <= -350)
#define IS_NEUTRAL(ch)		(!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)		((ch)->position > POS_SLEEPING)
#define GET_AC(ch)		((ch)->armor				    \
				    + ( IS_AWAKE(ch)			    \
				    ? dex_app[get_curr_dex(ch)].defensive   \
				    : 0 )				    \
				    + VAMP_AC(ch))
#define GET_HITROLL(ch)		((ch)->hitroll				    \
				    +str_app[get_curr_str(ch)].tohit	    \
				    +(2-(abs((ch)->mental_state)/10)))
#define GET_DAMROLL(ch)		((ch)->damroll                              \
				    +str_app[get_curr_str(ch)].todam	    \
				    +(((ch)->mental_state > 5		    \
				    &&(ch)->mental_state < 15) ? 1 : 0) )

#define IS_OUTSIDE(ch)		(!IS_SET(				    \
				    (ch)->in_room->room_flags,		    \
				    ROOM_INDOORS))

#define IS_UNDERGROUND(ch)      (ch->in_room->sector_type == SECT_UNDERGROUND)

#define NO_WEATHER_SECT(sect)  (  sect == SECT_INSIDE || 	           \
				  sect == SECT_UNDERWATER ||               \
                                  sect == SECT_OCEANFLOOR ||               \
                                  sect == SECT_UNDERGROUND )

#define IS_DRUNK(ch, drunk)     (number_percent() < \
			        ( (ch)->pcdata->condition[COND_DRUNK] \
				* 2 / (drunk) ) )

#define IS_CLANNED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    \
				&& (ch)->pcdata->clan->clan_type != CLAN_ORDER  \
				&& (ch)->pcdata->clan->clan_type != CLAN_GUILD)

#define IS_ORDERED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    \
				&& (ch)->pcdata->clan->clan_type == CLAN_ORDER)

#define IS_GUILDED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    \
				&& (ch)->pcdata->clan->clan_type == CLAN_GUILD)

#define IS_DEADLYCLAN(ch)	(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    \
				&& (ch)->pcdata->clan->clan_type != CLAN_NOKILL) \
				&& (ch)->pcdata->clan->clan_type != CLAN_ORDER)  \
				&& (ch)->pcdata->clan->clan_type != CLAN_GUILD)

#define IS_DEVOTED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->deity)

#define IS_PKILL(ch)            ((ch)->pcdata && IS_SET( (ch)->pcdata->flags, PCFLAG_DEADLY ))

#define IS_WEB(ch)              ((ch)->pcdata && IS_SET( (ch)->pcdata->flags, PCFLAG_WEB ))

#define IS_SILENCED(ch)         ((!IS_NPC((ch)) && IS_PLR_FLAG((ch), PLR_SILENCE)) || \
                                IS_AFFECTED2((ch), AFF2_SILENCE))

#define CAN_PKILL(ch)           (IS_PKILL(ch) && GetMaxLevel(ch) >= 5 && get_age( ch ) >= 18 )

#define WAIT_STATE(ch, npulse)  ((ch)->wait = (GetMaxLevel(ch)<MAX_LEVEL-10)?UMAX((ch)->wait,(int)(npulse)):0)
#define WAIT_FIGHT(ch, npulse)  ((ch)->fight_wait = \
				UMAX((ch)->fight_wait, (npulse)))

#define IS_VALID_SN(sn)		( (sn) >=0 && (sn) < MAX_SKILL		     \
				&& skill_table[(sn)]			     \
				&& skill_table[(sn)]->name		     )
/*				&& skill_table[(sn)]->spell_fun!=spell_null
				&& skill_table[(sn)]->skill_fun!=skill_notfound )
*/
#define IS_VALID_HERB(sn)	( (sn) >=0 && (sn) < MAX_HERB		     \
				&& herb_table[(sn)]			     \
				&& herb_table[(sn)]->name )

#define SPELL_FLAG(skill, flag)	( IS_SET((skill)->flags, (flag)) )

#if 0
#define SPELL_DAMAGE(skill)	( ((skill)->flags     ) & 7 )
#define SPELL_ACTION(skill)	( ((skill)->flags >> 3) & 7 )
#define SPELL_CLASS(skill)	( ((skill)->flags >> 6) & 7 )
#define SPELL_POWER(skill)	( ((skill)->flags >> 9) & 3 )
#define SPELL_SAVE(skill)	( (skill)->save_type )
#define SET_SDAM(skill, val)	( (skill)->flags =  ((skill)->flags & SDAM_MASK) + ((val) & 7) )
#define SET_SACT(skill, val)	( (skill)->flags =  ((skill)->flags & SACT_MASK) + (((val) & 7) << 3) )
#define SET_SCLA(skill, val)	( (skill)->flags =  ((skill)->flags & SCLA_MASK) + (((val) & 7) << 6) )
#define SET_SPOW(skill, val)	( (skill)->flags =  ((skill)->flags & SPOW_MASK) + (((val) & 3) << 9) )
#define SET_SSAV(skill, val)	( (skill)->save_type = (val) )
#else
#define SPELL_DAMAGE(skill)	( (skill)->dam_type )
#define SPELL_ACTION(skill)	( (skill)->act_type )
#define SPELL_CLASS(skill)	( (skill)->class_type )
#define SPELL_POWER(skill)	( (skill)->power_type )
#define SPELL_SAVE(skill)	( (skill)->save_type )
#define SET_SDAM(skill, val)	( (skill)->dam_type = (val) )
#define SET_SACT(skill, val)	( (skill)->act_type = (val) )
#define SET_SCLA(skill, val)	( (skill)->class_type = (val) )
#define SET_SPOW(skill, val)	( (skill)->power_type = (val) )
#define SET_SSAV(skill, val)	( (skill)->save_type = (val) )
#endif

/* Retired and guest imms. */
#define IS_RETIRED(ch) (ch->pcdata && IS_SET(ch->pcdata->flags,PCFLAG_RETIRED))
#define IS_GUEST(ch)   (ch->pcdata && IS_SET(ch->pcdata->flags,PCFLAG_GUEST))

/* RIS by gsn lookups. -- Altrag.
   Will need to add some || stuff for spells that need a special GSN. */

#define IS_FIRE(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_FIRE )
#define IS_COLD(dt)		( IS_VALID_SN(dt) &&                         \
                                SPELL_DAMAGE(skill_table[(dt)]) == SD_COLD )
#define IS_ACID(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ACID )
#define IS_ELECTRICITY(dt)	( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ELECTRICITY )
#define IS_ENERGY(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ENERGY )
#define IS_DRAIN(dt)		( IS_VALID_SN(dt) &&			     \
        			SPELL_DAMAGE(skill_table[(dt)]) == SD_DRAIN )
#define IS_POISON(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_POISON )


#define NOT_AUTHED(ch)		(!IS_NPC(ch) && ch->pcdata->auth_state <= 3  \
			      && IS_SET(ch->pcdata->flags, PCFLAG_UNAUTHED) )

#define IS_WAITING_FOR_AUTH(ch) (!IS_NPC(ch) && ch->desc		     \
			      && ch->pcdata->auth_state == 1		     \
			      && IS_SET(ch->pcdata->flags, PCFLAG_UNAUTHED) )

#define GET_CON_STATE(ch)       (ch->desc?ch->desc->connected:CON_INVALID)

/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)	   (IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)     (IS_SET((obj)->extra_flags, (stat)))
#define SET_OBJ_STAT(obj,stat)     (SET_BIT((obj)->extra_flags, (stat)))
#define REMOVE_OBJ_STAT(obj,stat)  (REMOVE_BIT((obj)->extra_flags, (stat)))
#define IS_OBJ_STAT2(obj, stat)	   (IS_SET((obj)->extra_flags2, (stat)))
#define SET_OBJ_STAT2(obj,stat)    (SET_BIT((obj)->extra_flags2, (stat)))
#define REMOVE_OBJ_STAT2(obj,stat) (REMOVE_BIT((obj)->extra_flags2, (stat)))

/*
 * MudProg macros.
 */
#define HAS_PROG(who, prog)        (xIS_SET((who)->progtypes, (prog)))
#define IS_PROG_TYPE(what, prog)   ((what)->progtype == (prog))
#define SET_PROG_TYPE(what, prog)  ((what)->progtype = (prog))

/*
 * Description macros.
 */
#if 0
/* see intro.c */
#define PERS(ch, looker)	( can_see( (looker), (ch) ) ?		\
				( IS_NPC(ch) ? (ch)->short_descr	\
				: (ch)->name ) : "someone" )
#endif

/*
 * command flags
 */
#define CMD_NORMAL         BV00
#define CMD_EXPERIMENTAL   BV01
#define CMD_BUILDING       BV02
#define CMD_INFORMATIONAL  BV03
#define CMD_IMC            BV04
#define CMD_UTILITY        BV05
#define CMD_MAINTENANCE    BV06
#define CMD_CREATION       BV07
#define CMD_DELETION       BV08
#define CMD_MODIFICATION   BV09
#define CMD_COMMUNICATION  BV10
#define CMD_SYSTEM         BV11

/*
 * Structure for a command in the command lookup table.
 */
struct	cmd_type
{
    CMDTYPE *		next;
    char *		name;
    DO_FUN *		do_fun;
    sh_int		position;
    sh_int		level;
    sh_int		log;
    int                 flags;
    struct		timerset	userec;
};

#define ALIAS_REASSIGN  BV00
struct	alias_type
{
    ALIAS_DATA *	next;
    ALIAS_DATA *	prev;
    int                 flags;
    char *		name;
    char * 		cmd;
};

/*
 * Structure for a social in the socials table.
 */
struct	social_type
{
    SOCIALTYPE *	next;
    sh_int              position;
    char *              mob_response;
    char *		name;
    char *		char_no_arg;
    char *		others_no_arg;
    char *		char_found;
    char *		others_found;
    char *		vict_found;
    char *		char_auto;
    char *		others_auto;
    char * 		not_found;
};

/*
 * Global constants.
 */
extern  time_t last_restore_all_time;
extern  time_t boot_time;  /* this should be moved down */
extern  HOUR_MIN_SEC * set_boot_time;
extern  struct  tm *new_boot_time;
extern  time_t new_boot_time_t;

extern	const	struct	str_app_type	str_app		[26];
extern	const	struct	int_app_type	int_app		[26];
extern	const	struct	wis_app_type	wis_app		[26];
extern	const	struct	dex_app_type	dex_app		[26];
extern	const	struct	con_app_type	con_app		[26];
extern	const	struct	cha_app_type	cha_app		[26];
extern  const	struct	lck_app_type	lck_app		[26];

extern	const	struct	race_type	race_table	[MAX_RACE];
extern	const	struct	liq_type	liq_table	[LIQ_MAX];
extern	char *				attack_table	[DAM_MAX_TYPE];
extern	char *				attack_table_plural [DAM_MAX_TYPE];
extern	char *				body_location	[];
extern	char *				body_location_hit [];

extern const int mat_costmul[];
extern const int mat_hitplus[];
extern const int mat_damplus[];
extern const int mat_acplus[];
extern const char *mat_name[];
extern const char *mat_qname[];

extern	char *	const	skill_tname	[];
extern	sh_int	const	movement_loss	[SECT_MAX];
extern	char *	const	dir_name_str	[LAST_NORMAL_DIR+1];
extern	char *	const	where_name	[];
extern	const	sh_int	rev_dir		[LAST_NORMAL_DIR+1];
extern	const	int	trap_door	[];
extern	char *	const	ex_flags	[];
extern	char *	const	r_flags		[];
extern	char *	const	w_flags		[];
extern	char *	const	o_flags		[];
extern	char *	const	o2_flags	[];
extern	char *	const	mag_flags	[];
extern	char *	const	a_flags		[];
extern	char *	const	a2_flags	[];
extern	char *	const	o_types		[];
extern	char *	const	a_types		[];
extern	char *	const	act_flags	[];
extern	char *	const	act2_flags	[];
extern	char *	const	plr_flags	[];
extern	char *	const	plr2_flags	[];
extern	char *	const	pc_flags	[];
extern	char *	const	trap_flags	[];
extern	char *	const	wear_locs	[];
extern	char *	const	ris_flags	[];
extern	char *	const	trig_flags	[];
extern	char *	const	part_flags	[];
extern	sh_int	const	RacialMax	[MAX_RACE][REAL_MAX_CLASS];
extern	char *	const	npc_class	[];
extern	char *	const	pc_class	[];
extern	char *	const	short_pc_class	[];
extern	char *	const	defense_flags	[];
extern	char *	const	attack_flags	[];
extern	char *	const	area_flags	[];
extern	char *	const	color_table	[];
extern	int	const	lang_array      [];
extern	char *	const	lang_names      [];
extern	char *	const	sect_types	[];
extern	char *	const	position_types	[MAX_POSITION];

extern	char *	const	temp_settings	[]; /* FB */
extern	char *	const	precip_settings	[];
extern	char *	const	wind_settings	[];
extern	char *	const	preciptemp_msg	[6][6];
extern	char *	const	windtemp_msg	[6][6];
extern	char *	const	precip_msg	[];
extern	char *	const	wind_msg	[];

extern  char *  const   curr_types      [];
extern  char *  const   cap_curr_types  [];

extern  char *  const   race_exit_msgs[MAX_RACE];

extern  char *  const   plane_names[PLANE_LAST];

extern  char *  const   channel_names[32];

extern  char *	const	command_flags	[];

/*
 * Intervention (do_intervene)
 */

#define SYS_ECLIPSE   BV00
#define SYS_NOPORTAL  BV01
#define SYS_NOASTRAL  BV02
#define SYS_NOSUMMON  BV03
#define SYS_NOKILL    BV04
#define SYS_NOMAGIC   BV05
#define SYS_FREEXP    BV06

extern  char *	const	system_flags	[];
extern  int SystemFlags;
#define IS_SYSTEMFLAG(flag) IS_SET(SystemFlags, (flag))

/*
 * Global variables.
 */
extern	int	numobjsloaded;
extern	int	nummobsloaded;
extern	int	physicalobjects;
extern	int	num_descriptors;
extern	struct	system_data		sysdata;
extern	int	top_sn;
extern	int	top_vroom;
extern	int	top_herb;

extern		CMDTYPE		  *	command_hash	[126];

extern		struct class_type *     class_table	[REAL_MAX_CLASS];
extern		char *			title_table	[REAL_MAX_CLASS]
							[MAX_LEVEL+1]
							[2];

extern		SKILLTYPE	  *	skill_table	[MAX_SKILL];
extern		SOCIALTYPE	  *	social_index	[27];
extern		CHAR_DATA	  *	cur_char;
extern		ROOM_INDEX_DATA	  *	cur_room;
extern		bool			cur_char_died;
extern		ch_ret			global_retcode;
extern		SKILLTYPE	  *	herb_table	[MAX_HERB];

extern		HELP_DATA	  *	first_help;
extern		HELP_DATA	  *	last_help;
extern		SHOP_DATA	  *	first_shop;
extern		SHOP_DATA	  *	last_shop;
extern		REPAIR_DATA	  *	first_repair;
extern		REPAIR_DATA	  *	last_repair;

extern		BAN_DATA	  *	first_ban;
extern		BAN_DATA	  *	last_ban;
extern		CHAR_DATA	  *	first_char;
extern		CHAR_DATA	  *	last_char;
extern		DESCRIPTOR_DATA   *	first_descriptor;
extern		DESCRIPTOR_DATA   *	last_descriptor;
extern		BOARD_DATA	  *	first_board;
extern		BOARD_DATA	  *	last_board;
extern		OBJ_DATA	  *	first_object;
extern		OBJ_DATA	  *	last_object;
extern		CLAN_DATA	  *	first_clan;
extern		CLAN_DATA	  *	last_clan;
extern 		COUNCIL_DATA 	  *	first_council;
extern		COUNCIL_DATA	  * 	last_council;
extern		DEITY_DATA	  *	first_deity;
extern		DEITY_DATA	  *	last_deity;
extern		AREA_DATA	  *	first_area;
extern		AREA_DATA	  *	last_area;
extern		AREA_DATA	  *	first_build;
extern		AREA_DATA	  *	last_build;
extern		AREA_DATA	  *	first_asort;
extern		AREA_DATA	  *	last_asort;
extern		AREA_DATA	  *	first_bsort;
extern		AREA_DATA	  *	last_bsort;

extern		LANG_DATA	  *	first_lang;
extern		LANG_DATA	  *	last_lang;
/*
extern		GOD_DATA	  *	first_imm;
extern		GOD_DATA	  *	last_imm;
*/
extern		TELEPORT_DATA	  *	first_teleport;
extern		TELEPORT_DATA	  *	last_teleport;
extern		OBJ_DATA	  *	save_equipment[MAX_WEAR][MAX_LAYERS];
extern		CHAR_DATA	  *	quitting_char;
extern		CHAR_DATA	  *	loading_char;
extern		CHAR_DATA	  *	saving_char;
extern		OBJ_DATA	  *	all_obj;

extern		time_t			current_time;
extern		bool			fLogAll;
extern		char			log_buf		[];
extern		char			bug_buf		[];
extern		TIME_INFO_DATA		time_info;
extern		WEATHER_DATA		weather_info;

extern          AUCTION_DATA      *     auction;
extern		struct act_prog_data *	mob_act_list;

extern		int			weath_unit;
extern		int			rand_factor;
extern		int			climate_factor;
extern		int			neigh_factor;
extern		int			max_vector;

extern          CHAR_DATA         *     tagged_it;

#if 0

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
#define ACMD DECLARE_DO_FUN
#define TABLES_C_ALL
#include "tables.h"
#undef TABLES_C_ALL
#undef ACMD

/*
 * Spell functions.
 * Defined in magic.c.
 */
#define ACMD DECLARE_SPELL_FUN
#define MAGIC_C_ALL
#include "magic.h"
#undef MAGIC_C_ALL
#undef ACMD

#else

DECLARE_DO_FUN(skill_notfound);
DECLARE_DO_FUN(do_smaug_skill);

DECLARE_SPELL_FUN(spell_notfound);
DECLARE_SPELL_FUN(spell_null);
DECLARE_SPELL_FUN(spell_smaug);

#endif

/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if	defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif


/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#define PLAYER_DIR	"../player/"	/* Player files			*/
#define BACKUP_DIR	"../backup/"    /* Backup Player files		*/
#define GOD_DIR		"../gods/"	/* God Info Dir			*/
#define BOARD_DIR	"../boards/"	/* Board data dir		*/
#define CLAN_DIR	"../clans/"	/* Clan data dir		*/
#define COUNCIL_DIR  	"../councils/"  /* Council data dir		*/
#define GUILD_DIR       "../guilds/"    /* Guild data dir               */
#define DEITY_DIR	"../deity/"	/* Deity data dir		*/
#define BUILD_DIR       "../building/"  /* Online building save dir     */
#define SYSTEM_DIR	"../system/"	/* Main system files		*/
#define PROG_DIR	"mudprogs/"	/* MUDProg files		*/
#define CORPSE_DIR	"../corpses/"	/* Corpses			*/
#define NULL_FILE	"/dev/null"	/* To reserve one stream	*/
#define	CLASS_DIR	"../classes/"	/* Classes			*/
#define LOG_DIR		"../log/"	/* Logs				*/
#define PLAYER_LOG_DIR 	"../plog/"	/* Player Logs	       		*/
#define ROOM_LOG_DIR 	"../rlog/"	/* Room Logs	       		*/
#define CHAN_LOG_DIR 	"../clog/"	/* Channel Logs	       		*/
#define EMAIL_QUEUE_DIR "../mailqueue/" /* EMail queue dir              */

#define AREA_LIST	"area.lst"	/* List of areas		*/
#define BAN_LIST        "ban.lst"       /* List of bans                 */
#define CLAN_LIST	"clan.lst"	/* List of clans		*/
#define COUNCIL_LIST	"council.lst"	/* List of councils		*/
#define GUILD_LIST      "guild.lst"     /* List of guilds               */
#define GOD_LIST	"gods.lst"	/* List of gods			*/
#define DEITY_LIST	"deity.lst"	/* List of deities		*/
#define	CLASS_LIST	"class.lst"	/* List of classes		*/

#define BOARD_FILE	"boards.txt"		/* For bulletin boards  */

#define SHUTDOWN_FILE	SYSTEM_DIR "shutdown.txt" /* For 'shutdown'     */
#define RIPSCREEN_FILE	SYSTEM_DIR "mudrip.rip"
#define RIPTITLE_FILE	SYSTEM_DIR "mudtitle.rip"
#define ANSITITLE_FILE	SYSTEM_DIR "mudtitle.ans"
#define ASCTITLE_FILE	SYSTEM_DIR "mudtitle.asc"
#define BOOTLOG_FILE	SYSTEM_DIR "boot.txt"	  /* Boot up error file	 */
#define BUG_FILE	SYSTEM_DIR "bugs.txt"	  /* For bug( )*/
#define USER_BUG_FILE	SYSTEM_DIR "pcbugs.txt"	  /* For 'bug'*/
#define IDEA_FILE	SYSTEM_DIR "ideas.txt"	  /* For 'idea'		 */
#define TYPO_FILE	SYSTEM_DIR "typos.txt"	  /* For 'typo'		 */
#define LOG_FILE	SYSTEM_DIR "log.txt"	  /* For talking in logged rooms */
#define WIZLIST_FILE	SYSTEM_DIR "WIZLIST"	  /* Wizlist		 */
#define WHO_FILE	SYSTEM_DIR "WHO"	  /* Who output file	 */
#define WEBWHO_FILE	SYSTEM_DIR "WEBWHO"	  /* WWW Who output file */
#define REQUEST_PIPE	SYSTEM_DIR "REQUESTS"	  /* Request FIFO	 */
#define SKILL_FILE	SYSTEM_DIR "skills.dat"   /* Skill table	 */
#define HERB_FILE	SYSTEM_DIR "herbs.dat"	  /* Herb table		 */
#define SOCIAL_FILE	SYSTEM_DIR "socials.dat"  /* Socials		 */
#define POSE_FILE	SYSTEM_DIR "poses.dat"    /* Poses      	 */
#define COMMAND_FILE	SYSTEM_DIR "commands.dat" /* Commands		 */
#define USAGE_FILE	SYSTEM_DIR "usage.txt"    /* How many people are on
 						     every half hour - trying to
						     determine best reboot time */
#define LASTLOG_FILE    SYSTEM_DIR "last.log"	  /* for 'last'		 */
#define COPYOVER_FILE	SYSTEM_DIR "copyover.dat" /* for warm reboots	 */
#define EXE_FILE	"../src/dotd"		  /* executable path	 */
#define CHRISTEN_FILE   SYSTEM_DIR "christen.dat" /* obj christen data   */
#define TONGUE_FILE	SYSTEM_DIR "tongues.dat"  /* Tongue tables	 */

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define SF	SPEC_FUN
#define BD	BOARD_DATA
#define CL	CLAN_DATA
#define EDD	EXTRA_DESCR_DATA
#define RD	RESET_DATA
#define ED	EXIT_DATA
#define	ST	SOCIALTYPE
#define	CO	COUNCIL_DATA
#define DE	DEITY_DATA
#define SK	SKILLTYPE

/* act_comm.c */
bool	circle_follow	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	too_many_followers args( ( CHAR_DATA *ch ) );
void	add_follower	args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void	stop_follower	args( ( CHAR_DATA *ch ) );
void	die_follower	args( ( CHAR_DATA *ch ) );
bool	is_same_group	args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
bool	is_same_race_align args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
void	send_rip_screen args( ( CHAR_DATA *ch ) );
void	send_rip_title	args( ( CHAR_DATA *ch ) );
void	send_ansi_title args( ( CHAR_DATA *ch ) );
void	send_ascii_title args( ( CHAR_DATA *ch ) );
void	to_channel	args( ( const char *argument, sh_int log_type,
				sh_int level, sh_int severity ) );
void  	talk_auction    args( ( char *argument ) );
int     knows_language  args( ( CHAR_DATA *ch, int language,
				CHAR_DATA *cch ) );
bool    can_learn_lang  args( ( CHAR_DATA *ch, int language ) );
int     countlangs      args( ( int languages ) );
char *	translate	args( ( int percent, const char *in, const char *name) );
char *	myobj		args( ( OBJ_DATA *obj ) );
char *	obj_short	args( ( OBJ_DATA *obj ) );

/* ansi.c */
char *	color_str	args( ( sh_int AType, CHAR_DATA *ch ) );
char *	def_color_str	args( ( sh_int AType ) );
char *	atcode_color_str args( ( sh_int AType ) );
char *	uncolorify	args( ( const char *arg ) );
int strlen_color args( ( const char *str ) );
char *	center_str_color  args( ( const char *str, int width ) );


/* act_info.c */
int	get_door	args( ( char *arg ) );
char *	format_obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch,
				    bool fShort ) );
void	show_list_to_char	args( ( OBJ_DATA *list, CHAR_DATA *ch,
				    bool fShort, bool fShowNothing ) );
char *ArmorDesc(int a);
char *RollDesc(int a);

/* act_move.c */
void	clear_vrooms	args( ( void ) );
ED *	find_door	args( ( CHAR_DATA *ch, char *arg, bool quiet ) );
ED *	get_exit	args( ( ROOM_INDEX_DATA *room, sh_int dir ) );
ED *	get_exit_to	args( ( ROOM_INDEX_DATA *room, sh_int dir, int vnum ) );
ED *	get_exit_num	args( ( ROOM_INDEX_DATA *room, sh_int count ) );
char *  exit_name       args( ( EXIT_DATA *pexit ) );
char *  dir_name        args( ( sh_int dir ) );
ch_ret	move_char	args( ( CHAR_DATA *ch, EXIT_DATA *pexit, int fall ) );
void	teleport	args( ( CHAR_DATA *ch, int room, int flags ) );
sh_int	encumbrance	args( ( CHAR_DATA *ch, sh_int move ) );
bool	will_fall	args( ( CHAR_DATA *ch, int fall ) );
bool	will_drown      args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room ) );

/* act_obj.c */

obj_ret	damage_obj	args( ( OBJ_DATA *obj ) );
sh_int	get_obj_resistance args( ( OBJ_DATA *obj ) );
void    save_clan_storeroom args( ( CHAR_DATA *ch, CLAN_DATA *clan ) );
void    obj_fall  	args( ( OBJ_DATA *obj, bool through ) );
bool    remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
int	item_ego	args( ( OBJ_DATA *obj ) );
int	char_ego	args( ( CHAR_DATA *ch ) );
bool    ItemSave        args( ( OBJ_DATA *obj, int sn) );

/* act_wiz.c */
RID *	find_location	args( ( CHAR_DATA *ch, char *arg ) );
void	echo_to_all	args( ( sh_int AT_COLOR, char *argument,
				sh_int tar ) );
void   	get_reboot_string args( ( void ) );
struct tm *update_time  args( ( struct tm *old_time ) );
void	free_social	args( ( SOCIALTYPE *social ) );
void	add_social	args( ( SOCIALTYPE *social ) );
void	free_command	args( ( CMDTYPE *command ) );
void	unlink_command	args( ( CMDTYPE *command ) );
void	add_command	args( ( CMDTYPE *command ) );
void	close_area	args( ( AREA_DATA *pArea, bool build ) );

/* boards.c */
void	load_boards	args( ( void ) );
BD *	get_board	args( ( OBJ_DATA *obj ) );
void	free_note	args( ( NOTE_DATA *pnote ) );

/* build.c */
char *	flag_string	args( ( int bitvector, char * const flagarray[] ) );
int     get_mpflag	args( ( char *flag ) );
sh_int get_dir      	args( ( char *txt  ) );
char *	strip_cr	args( ( char *str  ) );
char *	strip_crlf	args( ( char *str  ) );
char *	strip_lf	args( ( char *str  ) );

/* clans.c */
CL *	get_clan	args( ( char *name ) );
void	load_clans	args( ( void ) );
void	save_clan	args( ( CLAN_DATA *clan ) );

CO *	get_council	args( ( char *name ) );
void	load_councils	args( ( void ) );
void 	save_council	args( ( COUNCIL_DATA *council ) );

/* deity.c */
DE *	get_deity	args( ( char *name ) );
void	load_deity	args( ( void ) );
void	save_deity	args( ( DEITY_DATA *deity ) );

/* comm.c */
bool	check_parse_name args( ( char *name ) );
void	close_socket	args( ( DESCRIPTOR_DATA *dclose, bool force ) );
void	write_to_buffer	args( ( DESCRIPTOR_DATA *d, const char *txt,
				unsigned int length ) );
void	write_to_pager	args( ( DESCRIPTOR_DATA *d, const char *txt,
				unsigned int length ) );
void	send_to_room	args( ( const char *txt, ROOM_INDEX_DATA *rm ) );
void	send_to_area	args( ( const char *txt, AREA_DATA *area ) );
void	send_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void	send_to_char_color	args( ( const char *txt, CHAR_DATA *ch ) );
void	send_to_pager	args( ( const char *txt, CHAR_DATA *ch ) );
void	send_to_pager_color	args( ( const char *txt, CHAR_DATA *ch ) );
void	set_char_color  args( ( sh_int AType, CHAR_DATA *ch ) );
void	set_pager_color	args( ( sh_int AType, CHAR_DATA *ch ) );
void	ch_printf	args( ( CHAR_DATA *ch, char *fmt, ... ) )__attribute__((format(printf,2,3)));
void	paint   	args( ( sh_int AType, CHAR_DATA *ch, char *fmt, ... ) )__attribute__((format(printf,3,4)));
void	pager_printf	args( (CHAR_DATA *ch, char *fmt, ...) )__attribute__((format(printf,2,3)));
void	log_printf_plus	args( (sh_int log_type, sh_int level, sh_int severity, char *fmt, ...) )__attribute__((format(printf,4,5)));
void	act		args( ( sh_int AType, const char *format, CHAR_DATA *ch,
			    const void *arg1, const void *arg2, int type ) );
void    copyover_recover	args( (void) );
char    *sec_to_hms     args( ( time_t time_val ) );
char    *sec_to_hms_short       args( ( time_t time_val ) );


/* reset.c */
RD  *	make_reset	args( ( char letter, int extra, int arg1, int arg2, int arg3 ) );
RD  *	add_reset	args( ( AREA_DATA *tarea, char letter, int extra, int arg1, int arg2, int arg3 ) );
RD  *	place_reset	args( ( AREA_DATA *tarea, char letter, int extra, int arg1, int arg2, int arg3 ) );
void	reset_area	args( ( AREA_DATA * pArea ) );

/* db.c */
void	boot_log	args( ( const char *str, ... ) )__attribute__((format(printf,1,2)));
void	show_file	args( ( CHAR_DATA *ch, char *filename ) );
char *	str_dup		args( ( char const *str ) );
void	boot_db		args( ( bool fCopyOver ) );
void	area_update	args( ( void ) );
void	add_char	args( ( CHAR_DATA *ch ) );
CD *	create_mobile	args( ( int vnum ) );
OD *	create_object	args( ( int vnum ) );
void	SetDefaultColor	args( ( CHAR_DATA *ch ) );
int	def_color	args( ( sh_int i ) );
CHAR_DATA *new_char     args( ( void ) );
OBJ_DATA *new_obj       args( ( void ) );
void	clear_char	args( ( CHAR_DATA *ch ) );
void	free_char	args( ( CHAR_DATA *ch ) );
char *	get_extra_descr	args( ( const char *name, EXTRA_DESCR_DATA *ed ) );
MID *	get_mob_index	args( ( int vnum ) );
bool    mob_exists_index args( ( int vnum ) );
OID *	get_obj_index	args( ( int vnum ) );
bool    obj_exists_index args( ( int vnum ) );
RID *	get_room_index	args( ( int vnum ) );
bool    room_exists_index args( ( int vnum ) );
bool    is_other_plane  args( ( ROOM_INDEX_DATA *r1, ROOM_INDEX_DATA *r2 ) );
char	fread_letter	args( ( FILE *fp ) );
int	fread_number	args( ( FILE *fp ) );
char *	fread_string	args( ( FILE *fp ) );
char *	fread_string_nohash args( ( FILE *fp ) );
void	fread_to_eol	args( ( FILE *fp ) );
char *	fread_word	args( ( FILE *fp ) );
char *	fread_line	args( ( FILE *fp ) );
int	number_fuzzy	args( ( int number ) );
int	number_range	args( ( int from, int to ) );
int	number_percent	args( ( void ) );
sh_int	number_door	args( ( void ) );
int	number_bits	args( ( int width ) );
int	number_mm	args( ( void ) );
int	dice		args( ( int number, int size ) );
int	interpolate	args( ( int level, int value_00, int value_32 ) );
void	smash_tilde	args( ( char *str ) );
void	hide_tilde	args( ( char *str ) );
char *	show_tilde	args( ( char *str ) );
bool	str_cmp		args( ( const char *astr, const char *bstr ) );
bool	str_prefix	args( ( const char *astr, const char *bstr ) );
bool	str_infix	args( ( const char *astr, const char *bstr ) );
bool	str_suffix	args( ( const char *astr, const char *bstr ) );
char *	capitalize	args( ( const char *str ) );
char *	strlower	args( ( const char *str ) );
char *	strupper	args( ( const char *str ) );
char *  aoran		args( ( const char *str ) );
void	append_file	args( ( CHAR_DATA *ch, char *file, char *str ) );
void	append_to_file	args( ( char *file, char *str ) );
void	low_bug		args( ( const char *str, ... ) )__attribute__((format(printf,1,2)));
#define bug low_bug
void	log_string_plus	args( ( const char *str, sh_int log_type, sh_int level, sh_int severity ) );
RID *	make_room	args( ( int vnum ) );
OID *	make_object	args( ( int vnum, int cvnum, char *name ) );
MID *	make_mobile	args( ( int vnum, int cvnum, char *name ) );
ED  *	make_exit	args( ( ROOM_INDEX_DATA *pRoomIndex, ROOM_INDEX_DATA *to_room, sh_int door ) );
void	add_help	args( ( HELP_DATA *pHelp ) );
void	fix_area_exits	args( ( AREA_DATA *tarea ) );
AREA_DATA *create_area  args( ( char *filename ) );
void	load_area_file	args( ( AREA_DATA *tarea ) );
void	load_area_demand args( ( int vnum ) );
void	randomize_exits	args( ( ROOM_INDEX_DATA *room, sh_int maxdir ) );
void	make_wizlist	args( ( void ) );
void	tail_chain	args( ( void ) );
bool    delete_room     args( ( ROOM_INDEX_DATA *room ) );
bool    delete_obj      args( ( OBJ_INDEX_DATA *obj ) );
bool    delete_mob      args( ( MOB_INDEX_DATA *mob ) );
/* Functions to add to sorting lists. -- Altrag */
/*void	mob_sort	args( ( MOB_INDEX_DATA *pMob ) );
void	obj_sort	args( ( OBJ_INDEX_DATA *pObj ) );
void	room_sort	args( ( ROOM_INDEX_DATA *pRoom ) );*/
void	sort_area	args( ( AREA_DATA *pArea, bool proto ) );
AREA_DATA *get_room_area args( (int vnum) );
AREA_DATA *get_mob_area args( (int vnum) );
AREA_DATA *get_obj_area args( (int vnum) );


/* build.c */
void	start_editing	args( ( CHAR_DATA *ch, char *data ) );
void	stop_editing	args( ( CHAR_DATA *ch ) );
void	edit_buffer	args( ( CHAR_DATA *ch, char *argument ) );
char *	copy_buffer	args( ( CHAR_DATA *ch ) );
bool	can_rmodify	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room ) );
bool	can_omodify	args( ( CHAR_DATA *ch, OBJ_DATA *obj  ) );
bool	can_mmodify	args( ( CHAR_DATA *ch, CHAR_DATA *mob ) );
bool	can_medit	args( ( CHAR_DATA *ch, MOB_INDEX_DATA *mob ) );
void	free_reset	args( ( AREA_DATA *are, RESET_DATA *res ) );
void	free_area	args( ( AREA_DATA *are ) );
void	assign_area	args( ( CHAR_DATA *ch ) );
EDD *	SetRExtra	args( ( ROOM_INDEX_DATA *room, char *keywords ) );
bool	DelRExtra	args( ( ROOM_INDEX_DATA *room, char *keywords ) );
EDD *	SetOExtra	args( ( OBJ_DATA *obj, char *keywords ) );
bool	DelOExtra	args( ( OBJ_DATA *obj, char *keywords ) );
EDD *	SetOExtraProto	args( ( OBJ_INDEX_DATA *obj, char *keywords ) );
bool	DelOExtraProto	args( ( OBJ_INDEX_DATA *obj, char *keywords ) );
void	fold_area	args( ( AREA_DATA *tarea, char *filename, bool install ) );
int	get_otype	args( ( char *type ) );
int	get_atype	args( ( char *type ) );
int	get_aflag	args( ( char *flag ) );
int	get_a2flag	args( ( char *flag ) );
int	get_oflag	args( ( char *flag ) );
int	get_o2flag	args( ( char *flag ) );
int	get_magflag	args( ( char *flag ) );
int	get_wflag	args( ( char *flag ) );
int	get_actflag	args( ( char *flag ) );
int	get_act2flag	args( ( char *flag ) );
int	get_plrflag	args( ( char *flag ) );
int	get_plr2flag	args( ( char *flag ) );
sh_int	get_postype	args( ( char *type ) );
sh_int get_classtype args( ( char *type ) );
void	init_area_weather args(( void ) );
void	save_weatherdata args( ( void ) );

/* fight.c */
#define max_fight(ch) (8)
void	violence_update	args( ( void ) );
ch_ret	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
ch_ret	multi_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
ch_ret	projectile_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield,
			    OBJ_DATA *projectile, sh_int dist ) );
sh_int	ris_damage	args( ( CHAR_DATA *ch, sh_int dam, int ris ) );
ch_ret	damage		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			    int dt ) );
void	update_pos	args( ( CHAR_DATA *victim ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	stop_fighting	args( ( CHAR_DATA *ch, bool fBoth ) );
void	free_fight	args( ( CHAR_DATA *ch ) );
CD *	who_fighting	args( ( CHAR_DATA *ch ) );
void	check_killer	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	check_attacker	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	death_cry	args( ( CHAR_DATA *ch ) );
void	stop_hunting	args( ( CHAR_DATA *ch ) );
void	stop_hating	args( ( CHAR_DATA *ch ) );
void	stop_fearing	args( ( CHAR_DATA *ch ) );
void	start_hunting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	start_hating	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	start_fearing	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_hunting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_hating	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_fearing	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_safe		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	legal_loot	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
sh_int	VAMP_AC		args( ( CHAR_DATA *ch ) );
bool    check_illegal_pk args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    raw_kill        args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
bool	in_arena	args( ( CHAR_DATA *ch ) );
int     CalcThaco       args( ( CHAR_DATA *ch ) );
int     HitOrMiss       args( ( CHAR_DATA *ch, CHAR_DATA *victim, int calc_thaco ) );
CHAR_DATA *race_align_hatee args( ( CHAR_DATA *ch ) );
int	lorebonus	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int sn ) );
/* makeobjs.c */
void	make_corpse	args( ( CHAR_DATA *ch, CHAR_DATA *killer, int dt ) );
void	make_blood	args( ( CHAR_DATA *ch ) );
void	make_bloodstain args( ( CHAR_DATA *ch ) );
void	make_scraps	args( ( OBJ_DATA *obj, bool quiet ) );
void	make_fire	args( ( ROOM_INDEX_DATA *in_room, sh_int timer) );
OD *	make_trap	args( ( int v0, int v1, int v2, int v3 ) );
OD *	create_money	args( ( int amount, int type ) );

/* misc.c */
bool    actiondesc      args( ( CHAR_DATA *ch, OBJ_DATA *obj, void *vo ) );

/* deity.c */
void adjust_favor	args( ( CHAR_DATA *ch, int field, int mod ) );

/* mud_comm.c */
char *	mprog_type_to_name	args( ( sh_int type ) );

/* mud_prog.c */
#ifdef DUNNO_STRSTR
char *  strstr                  args ( (const char *s1, const char *s2 ) );
#endif

bool	mprog_wordlist_check    args ( ( char * arg, CHAR_DATA *mob,
                			CHAR_DATA* actor, OBJ_DATA* object,
					void* vo, sh_int type ) );
void	mprog_percent_check     args ( ( CHAR_DATA *mob, CHAR_DATA* actor,
                                         OBJ_DATA* object, void* vo,
                                         sh_int type ) );
void	mprog_act_trigger       args ( ( char* buf, CHAR_DATA* mob,
                                         CHAR_DATA* ch, OBJ_DATA* obj,
                                         void* vo ) );
void	mprog_bribe_trigger     args ( ( CHAR_DATA* mob, CHAR_DATA* ch,
                                         int amount ) );
void	mprog_entry_trigger     args ( ( CHAR_DATA* mob ) );
void	mprog_give_trigger      args ( ( CHAR_DATA* mob, CHAR_DATA* ch,
                                         OBJ_DATA* obj ) );
void	mprog_greet_trigger     args ( ( CHAR_DATA* mob ) );
void    mprog_fight_trigger     args ( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_hitprcnt_trigger  args ( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_death_trigger     args ( ( CHAR_DATA *killer, CHAR_DATA* mob ) );
void    mprog_birth_trigger     args ( ( CHAR_DATA *maker, CHAR_DATA* mob ) );
void    mprog_random_trigger    args ( ( CHAR_DATA* mob ) );
void    mprog_speech_trigger    args ( ( char* txt, CHAR_DATA* mob ) );
void    mprog_script_trigger    args ( ( CHAR_DATA *mob ) );
void    mprog_hour_trigger      args ( ( CHAR_DATA *mob ) );
void    mprog_time_trigger      args ( ( CHAR_DATA *mob ) );
bool    mprog_command_trigger   args ( ( char *txt, CHAR_DATA *mob, CHAR_DATA *actor ) );
void    mprog_quest_trigger     args ( ( CHAR_DATA *mob, CHAR_DATA *ch ) );
void    mprog_leave_trigger     args ( ( CHAR_DATA *ch ) );

void    progbug                 args( ( char *str, CHAR_DATA *mob ) );
void	rset_supermob		args( ( ROOM_INDEX_DATA *room) );
void	release_supermob	args( ( void ) );

/* player.c */
void	set_title	args( ( CHAR_DATA *ch, char *title ) );
bool	IsHumanoid	args( ( CHAR_DATA *ch ) );
bool	IsRideable	args( ( CHAR_DATA *ch ) );
bool	IsAnimal	args( ( CHAR_DATA *ch ) );
bool	IsVeggie	args( ( CHAR_DATA *ch ) );
bool	IsUndead	args( ( CHAR_DATA *ch ) );
bool	IsLycanthrope	args( ( CHAR_DATA *ch ) );
bool	IsDiabolic	args( ( CHAR_DATA *ch ) );
bool	IsReptile	args( ( CHAR_DATA *ch ) );
bool	IsPerson	args( ( CHAR_DATA *ch ) );
bool	IsGiantish	args( ( CHAR_DATA *ch ) );
bool	IsSmall		args( ( CHAR_DATA *ch ) );
bool	IsGiant		args( ( CHAR_DATA *ch ) );
bool	IsExtraPlanar	args( ( CHAR_DATA *ch ) );
bool	IsOther		args( ( CHAR_DATA *ch ) );
bool	IsGodly		args( ( CHAR_DATA *ch ) );
bool	IsDragon	args( ( CHAR_DATA *ch ) );
bool	IsGoodSide	args( ( CHAR_DATA *ch ) );
bool	IsBadSide	args( ( CHAR_DATA *ch ) );
bool	IsNeutralSide	args( ( CHAR_DATA *ch ) );
int	race_bodyparts	args( ( CHAR_DATA *ch ) );
int     EqWBits         args( ( CHAR_DATA *ch, int bit ) );
int     EqWBits2        args( ( CHAR_DATA *ch, int bit ) );
void    update_speaks   args( ( CHAR_DATA *ch ) );
char *  get_class_name	args( ( CHAR_DATA *ch ) );
char *  get_race_name 	args( ( CHAR_DATA *ch ) );


/* skills.c */
bool	check_skill		args( ( CHAR_DATA *ch, char *command, char *argument ) );
void	learn_from_success	args( ( CHAR_DATA *ch, int sn ) );
void	learn_from_failure	args( ( CHAR_DATA *ch, int sn ) );
int	check_parry		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam ) );
int	check_dodge		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam ) );
bool 	check_grip		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	disarm			args( ( CHAR_DATA *ch, CHAR_DATA *victim) );
void	trip			args( ( CHAR_DATA *ch, CHAR_DATA *victim) );
bool	mob_fire		args( ( CHAR_DATA *ch, char *name ) );
CD *	scan_for_victim		args( ( CHAR_DATA *ch, EXIT_DATA *pexit,
					 char *name ) );


/* handler.c */
int	get_exp		args( ( CHAR_DATA *ch ) );
int	get_exp_worth	args( ( CHAR_DATA *ch ) );
long	exp_level	args( ( CHAR_DATA *ch, sh_int level, sh_int cl) );
sh_int	get_trust	args( ( CHAR_DATA *ch ) );
sh_int	get_age		args( ( CHAR_DATA *ch ) );
sh_int	get_age_month	args( ( CHAR_DATA *ch ) );
sh_int	get_age_day	args( ( CHAR_DATA *ch ) );
sh_int	get_age_hour	args( ( CHAR_DATA *ch ) );
sh_int	MaxDexForRace	args( ( CHAR_DATA *ch ) );
sh_int	MaxIntForRace	args( ( CHAR_DATA *ch ) );
sh_int	MaxWisForRace	args( ( CHAR_DATA *ch ) );
sh_int	MaxConForRace	args( ( CHAR_DATA *ch ) );
sh_int	MaxChrForRace	args( ( CHAR_DATA *ch ) );
sh_int	MaxStrForRace	args( ( CHAR_DATA *ch ) );
sh_int	get_curr_str	args( ( CHAR_DATA *ch ) );
sh_int	get_curr_int	args( ( CHAR_DATA *ch ) );
sh_int	get_curr_wis	args( ( CHAR_DATA *ch ) );
sh_int	get_curr_dex	args( ( CHAR_DATA *ch ) );
sh_int	get_curr_con	args( ( CHAR_DATA *ch ) );
sh_int	get_curr_cha	args( ( CHAR_DATA *ch ) );
sh_int  get_curr_lck	args( ( CHAR_DATA *ch ) );
bool	can_take_proto	args( ( CHAR_DATA *ch ) );
int	can_carry_n	args( ( CHAR_DATA *ch ) );
int	carry_n         args( ( CHAR_DATA *ch ) );
int	can_carry_w	args( ( CHAR_DATA *ch ) );
int	carry_w  	args( ( CHAR_DATA *ch ) );
bool	is_name		args( ( const char *str, char *namelist ) );
bool	is_name_prefix	args( ( const char *str, char *namelist ) );
bool	nifty_is_name	args( ( char *str, char *namelist ) );
bool	nifty_is_name_prefix args( ( char *str, char *namelist ) );
void	affect_modify	args( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
void	affect_to_char	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_remove	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_strip	args( ( CHAR_DATA *ch, int sn ) );
AFFECT_DATA *is_affected	args( ( CHAR_DATA *ch, int sn ) );
void	affect_join	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	char_from_room	args( ( CHAR_DATA *ch ) );
void	char_to_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
OD *	obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void	obj_from_char	args( ( OBJ_DATA *obj ) );
int	apply_ac	args( ( OBJ_DATA *obj, int iWear ) );
OD *	get_eq_char	args( ( CHAR_DATA *ch, int iWear ) );
void    add_obj_affects args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
void	equip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void	unequip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int	count_obj_list	args( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void	obj_from_room	args( ( OBJ_DATA *obj ) );
OD *	obj_to_room	args( ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
OD *	obj_to_obj	args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void	obj_from_obj	args( ( OBJ_DATA *obj ) );
void	extract_obj	args( ( OBJ_DATA *obj ) );
void	extract_exit	args( ( ROOM_INDEX_DATA *room, EXIT_DATA *pexit ) );
void	extract_room	args( ( ROOM_INDEX_DATA *room ) );
void	clean_room	args( ( ROOM_INDEX_DATA *room ) );
void	clean_obj	args( ( OBJ_INDEX_DATA *obj ) );
void	clean_mob	args( ( MOB_INDEX_DATA *mob ) );
void	clean_resets	args( ( AREA_DATA *tarea ) );
void	recall_char	args( ( CHAR_DATA *ch ) );
void	extract_char	args( ( CHAR_DATA *ch, bool fPull ) );
CD *	get_char_room	args( ( CHAR_DATA *ch, char *argument ) );
CD *	get_char_world	args( ( CHAR_DATA *ch, char *argument ) );
CD *	get_char_area	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_type	args( ( OBJ_INDEX_DATA *pObjIndexData ) );
OD *	get_obj_list	args( ( CHAR_DATA *ch, char *argument,
			    OBJ_DATA *list ) );
OD *	get_obj_list_rev args( ( CHAR_DATA *ch, char *argument,
			    OBJ_DATA *list ) );
OD *	get_obj_carry	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_wear	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_here	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_world	args( ( CHAR_DATA *ch, char *argument ) );
int	get_obj_number	args( ( OBJ_DATA *obj ) );
int	get_obj_weight	args( ( OBJ_DATA *obj ) );
bool	room_is_inside  args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	room_is_dark	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	room_is_private	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	can_see		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_see_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	can_drop_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char *	item_type_name	args( ( OBJ_DATA *obj ) );
char *	affect_loc_name	args( ( sh_int location ) );
ch_ret	check_for_trap	args( ( CHAR_DATA *ch, OBJ_DATA *obj, int flag ) );
ch_ret	check_room_for_traps args( ( CHAR_DATA *ch, int flag ) );
bool	is_trapped	args( ( OBJ_DATA *obj ) );
OD *	get_trap	args( ( OBJ_DATA *obj ) );
ch_ret	spring_trap     args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
void	name_stamp_stats args( ( CHAR_DATA *ch ) );
void	fix_char	args( ( CHAR_DATA *ch ) );
void	showaffect	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	set_cur_obj	args( ( OBJ_DATA *obj ) );
bool	obj_extracted	args( ( OBJ_DATA *obj ) );
void	set_cur_char	args( ( CHAR_DATA *ch ) );
bool	real_char_died	args( ( CHAR_DATA *ch, char *file, int line ) );
#define char_died(ch) real_char_died((ch), __FILE__, __LINE__)
void	add_timer	args( ( CHAR_DATA *ch, sh_int type, sh_int count, DO_FUN *fun, int value ) );
TIMER * get_timerptr	args( ( CHAR_DATA *ch, sh_int type ) );
sh_int	get_timer	args( ( CHAR_DATA *ch, sh_int type ) );
void	extract_timer	args( ( CHAR_DATA *ch, TIMER *timer ) );
void	remove_timer	args( ( CHAR_DATA *ch, sh_int type ) );
bool	in_soft_range	args( ( CHAR_DATA *ch, AREA_DATA *tarea ) );
bool	in_hard_range	args( ( CHAR_DATA *ch, AREA_DATA *tarea ) );
bool	chance  	args( ( CHAR_DATA *ch, sh_int percent ) );
bool 	chance_attrib	args( ( CHAR_DATA *ch, sh_int percent, sh_int attrib ) );
OD *	clone_object	args( ( OBJ_DATA *obj ) );
void	split_obj	args( ( OBJ_DATA *obj, int num ) );
void	separate_obj	args( ( OBJ_DATA *obj ) );
bool	empty_obj	args( ( OBJ_DATA *obj, OBJ_DATA *destobj,
				ROOM_INDEX_DATA *destroom ) );
OD *	find_obj	args( ( CHAR_DATA *ch, char *argument,
				bool carryonly ) );
bool	ms_find_obj	args( ( CHAR_DATA *ch ) );
void	worsen_mental_state args( ( CHAR_DATA *ch, int mod ) );
void	better_mental_state args( ( CHAR_DATA *ch, int mod ) );
void	boost_economy	args( ( AREA_DATA *tarea, int gold, int type ) );
void	lower_economy	args( ( AREA_DATA *tarea, int gold, int type ) );
void	economize_mobgold args( ( CHAR_DATA *mob ) );
bool	economy_has	args( ( AREA_DATA *tarea, int gold, int type ) );
void	add_kill	args( ( CHAR_DATA *ch, CHAR_DATA *mob ) );
int	times_killed	args( ( CHAR_DATA *ch, CHAR_DATA *mob ) );
void	free_aliases	args( ( CHAR_DATA *ch ) );

AREA_DATA *get_area	args( ( char *name ) ); /* FB */
int unum_arg   args( ( char *argument ) );
int vnum_arg   args( ( char *argument ) );

/* interp.c */
bool	check_pos	args( ( CHAR_DATA *ch, sh_int position ) );
void	interpret	args( ( CHAR_DATA *ch, char *argument ) );
bool	is_number	args( ( char *arg ) );
int	number_argument	args( ( char *argument, char *arg ) );
char *	one_argument	args( ( char *argument, char *arg_first ) );
char *	one_argument2	args( ( char *argument, char *arg_first ) );
char *	one_argumentx	args( ( char *argument, char *arg_first, char cEnd ) );
char *  spacetodash     args( ( char *argument ) );
ST *	find_social	args( ( char *command ) );
ST *	find_social_exact args( ( char *command ) );
CMDTYPE *find_command	args( ( char *command ) );
CMDTYPE *find_command_exact args( ( char *command ) );
void	hash_commands	args( ( void ) );
void	start_timer	args( ( struct timeval *start_time ) );
time_t	end_timer	args( ( struct timeval *start_time ) );
void	send_timer	args( ( struct timerset *vtime, CHAR_DATA *ch ) );
void	update_userec	args( ( struct timeval *time_used,
				struct timerset *userec ) );

/* magic.c */
bool	process_spell_components args( ( CHAR_DATA *ch, int sn ) );
int	ch_slookup	args( ( CHAR_DATA *ch, const char *name ) );
int	find_spell	args( ( CHAR_DATA *ch, const char *name, bool know ) );
int	find_skill	args( ( CHAR_DATA *ch, const char *name, bool know ) );
int	find_weapon	args( ( CHAR_DATA *ch, const char *name, bool know ) );
int	find_tongue	args( ( CHAR_DATA *ch, const char *name, bool know ) );
int	find_lore	args( ( CHAR_DATA *ch, const char *name, bool know ) );
int	skill_lookup	args( ( const char *name ) );
int	herb_lookup	args( ( const char *name ) );
int	personal_lookup	args( ( CHAR_DATA *ch, const char *name ) );
int	slot_lookup	args( ( int slot ) );
int	bsearch_skill	args( ( const char *name, int first, int top ) );
int	bsearch_skill_exact     args( ( const char *name, int first, int top ) );
bool	saves_poison_death	args( ( int level, CHAR_DATA *victim ) );
bool	saves_wand		args( ( int level, CHAR_DATA *victim ) );
bool	saves_para_petri	args( ( int level, CHAR_DATA *victim ) );
bool	saves_breath		args( ( int level, CHAR_DATA *victim ) );
bool	saves_spell_staff	args( ( int level, CHAR_DATA *victim ) );
ch_ret	obj_cast_spell	args( ( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj ) );
int	dice_parse	args( ( CHAR_DATA *ch, int level, char *dicestr ) );
SK *	get_skilltype	args( ( int sn ) );
int	get_skill_tname	args( ( char *tname ) );
void    switch_stuff    args( ( CHAR_DATA *ch, CHAR_DATA *oc ) );
void    stop_memorizing args( ( CHAR_DATA *ch ) );
void    spell_lag       args( ( CHAR_DATA *ch, int sn ) );

/* request.c */
void	init_request_pipe	args( ( void ) );
void	check_requests		args( ( void ) );

/* save.c */
/* object saving defines for fread/write_obj. -- Altrag */
#define OS_CARRY	0
#define OS_CORPSE	1
void	save_char_obj	args( ( CHAR_DATA *ch ) );
bool	load_char_obj	args( ( DESCRIPTOR_DATA *d, char *name, bool preload, ROOM_INDEX_DATA *PetRoom, bool loadpets ) );
void	set_alarm	args( ( long seconds ) );
void	requip_char	args( ( CHAR_DATA *ch ) );
void    fwrite_obj      args( ( CHAR_DATA *ch,  OBJ_DATA  *obj, FILE *fp,
				int iNest, sh_int os_type ) );
void	fread_obj	args( ( CHAR_DATA *ch,  FILE *fp, sh_int os_type ) );
void	de_equip_char	args( ( CHAR_DATA *ch ) );
void	re_equip_char	args( ( CHAR_DATA *ch ) );
void	show_rent_list	args( ( CHAR_DATA *ch ) );
int	calc_rent	args( ( CHAR_DATA *ch ) );

/* shops.c */

/* mspecial.c */
SF *	m_spec_lookup	args( ( const char *name ) );
char *	m_lookup_spec	args( ( SPEC_FUN *special ) );

/* ospecial.c */
SF *	o_spec_lookup	args( ( const char *name ) );
char *	o_lookup_spec	args( ( SPEC_FUN *special ) );

/* rspecial.c */
SF *	r_spec_lookup	args( ( const char *name ) );
char *	r_lookup_spec	args( ( SPEC_FUN *special ) );

/* tables.c */
char *	spell_name	args( ( SPELL_FUN *spell ) );
char *	skill_name	args( ( DO_FUN *skill ) );
void	load_skill_table args( ( void ) );
void	save_skill_table args( ( void ) );
void	sort_skill_table args( ( void ) );
void	sort_skill_table_sn args( ( void ) );
void	load_socials	args( ( void ) );
void	load_poses	args( ( void ) );
void	save_socials	args( ( void ) );
void	load_commands	args( ( void ) );
void	save_commands	args( ( void ) );
SPELL_FUN *spell_function args( ( char *name ) );
DO_FUN *skill_function  args( ( char *name ) );
void	write_class_file args( ( sh_int cl ) );
void	save_classes	args( ( void ) );
void	load_classes	args( ( void ) );
void	load_herb_table	args( ( void ) );
void	save_herb_table	args( ( void ) );
void	load_tongues	args( ( void ) );

/* track.c */
void	found_prey	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	hunt_victim	args( ( CHAR_DATA *ch) );

/* update.c */
int	hit_limit	args( ( CHAR_DATA *ch ) );
int	mana_limit	args( ( CHAR_DATA *ch ) );
int	move_limit	args( ( CHAR_DATA *ch ) );
int	hit_gain	args( ( CHAR_DATA *ch ) );
int	mana_gain	args( ( CHAR_DATA *ch ) );
int	move_gain	args( ( CHAR_DATA *ch ) );
void	advance_level	args( ( CHAR_DATA *ch, sh_int cl ) );
void	gain_exp	args( ( CHAR_DATA *ch, int gain ) );
void	gain_condition	args( ( CHAR_DATA *ch, int iCond, int value ) );
void	update_handler	args( ( void ) );
void	reboot_check	args( ( time_t reset ) );
#if 0
void    reboot_check    args( ( char *arg ) );
#endif
void	remove_portal	args( ( OBJ_DATA *portal ) );
void	weather_update	args( ( void ) );
void	init_update	args( ( void ) );

/* hashstr.c */
char *	str_alloc	args( ( const char *str ) );
char *	quick_link	args( ( char *str ) );
int	str_free	args( ( char *str ) );
void	show_hash	args( ( int count ) );
char *	hash_stats	args( ( void ) );
char *	check_hash	args( ( char *str ) );
void	hash_dump	args( ( int hash ) );
void	show_high_hash	args( ( int top ) );
void    hash_check_sanity args( ( void ) );

/* miml.c */
void    miml_to_char	args( ( char *txt, CHAR_DATA *ch ) );


#undef	SK
#undef	CO
#undef	ST
#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef	BD
#undef	CL
#undef	EDD
#undef	RD
#undef	ED

/*
 *
 *  New Build Interface Stuff Follows
 *
 */


/*
 *  Data for a menu page
 */
struct	menu_data
{
    char		*sectionNum;
    char		*charChoice;
    int			x;
    int			y;
    char		*outFormat;
    void		*data;
    int			ptrType;
    int			cmdArgs;
    char		*cmdString;
};

extern		MENU_DATA		room_page_a_data[];
extern		MENU_DATA		room_page_b_data[];
extern		MENU_DATA		room_page_c_data[];
extern		MENU_DATA		room_help_page_data[];

extern		MENU_DATA		mob_page_a_data[];
extern		MENU_DATA		mob_page_b_data[];
extern		MENU_DATA		mob_page_c_data[];
extern		MENU_DATA		mob_page_d_data[];
extern		MENU_DATA		mob_page_e_data[];
extern		MENU_DATA		mob_page_f_data[];
extern		MENU_DATA		mob_help_page_data[];

extern		MENU_DATA		obj_page_a_data[];
extern		MENU_DATA		obj_page_b_data[];
extern		MENU_DATA		obj_page_c_data[];
extern		MENU_DATA		obj_page_d_data[];
extern		MENU_DATA		obj_page_e_data[];
extern		MENU_DATA		obj_help_page_data[];

extern		MENU_DATA		control_page_a_data[];
extern		MENU_DATA		control_help_page_data[];

extern	const   char    room_page_a[];
extern	const   char    room_page_b[];
extern	const   char    room_page_c[];
extern	const   char    room_help_page[];

extern	const   char    obj_page_a[];
extern	const   char    obj_page_b[];
extern	const   char    obj_page_c[];
extern	const   char    obj_page_d[];
extern	const   char    obj_page_e[];
extern	const   char    obj_help_page[];

extern	const   char    mob_page_a[];
extern	const   char    mob_page_b[];
extern	const   char    mob_page_c[];
extern	const   char    mob_page_d[];
extern	const   char    mob_page_e[];
extern	const   char    mob_page_f[];
extern	const   char    mob_help_page[];
extern	const   char *  npc_sex[3];
extern	const   char *  ris_strings[];

extern	const   char    control_page_a[];
extern	const   char    control_help_page[];

#define SH_INT 1
#define INT 2
#define CHAR 3
#define STRING 4
#define SPECIAL 5


#define NO_PAGE    0
#define MOB_PAGE_A 1
#define MOB_PAGE_B 2
#define MOB_PAGE_C 3
#define MOB_PAGE_D 4
#define MOB_PAGE_E 5
#define MOB_PAGE_F 17
#define MOB_HELP_PAGE 14
#define ROOM_PAGE_A 6
#define ROOM_PAGE_B 7
#define ROOM_PAGE_C 8
#define ROOM_HELP_PAGE 15
#define OBJ_PAGE_A 9
#define OBJ_PAGE_B 10
#define OBJ_PAGE_C 11
#define OBJ_PAGE_D 12
#define OBJ_PAGE_E 13
#define OBJ_HELP_PAGE 16
#define CONTROL_PAGE_A 18
#define CONTROL_HELP_PAGE 19

#define NO_TYPE   0
#define MOB_TYPE  1
#define OBJ_TYPE  2
#define ROOM_TYPE 3
#define CONTROL_TYPE 4

#define SUB_NORTH DIR_NORTH
#define SUB_EAST  DIR_EAST
#define SUB_SOUTH DIR_SOUTH
#define SUB_WEST  DIR_WEST
#define SUB_UP    DIR_UP
#define SUB_DOWN  DIR_DOWN
#define SUB_NE    DIR_NORTHEAST
#define SUB_NW    DIR_NORTHWEST
#define SUB_SE    DIR_SOUTHEAST
#define SUB_SW    DIR_SOUTHWEST

/*
 * defines for use with this get_affect function
 */

#define RIS_000		BV00
#define RIS_R00		BV01
#define RIS_0I0		BV02
#define RIS_RI0		BV03
#define RIS_00S		BV04
#define RIS_R0S		BV05
#define RIS_0IS		BV06
#define RIS_RIS		BV07

#define GA_AFFECTED	BV09
#define GA_RESISTANT	BV10
#define GA_IMMUNE	BV11
#define GA_SUSCEPTIBLE	BV12
#define GA_RIS          BV30



/*
 *   Map Structures
 */

struct  map_data	/* contains per-room data */
{
  int vnum;		/* which map this room belongs to */
  int x;		/* horizontal coordinate */
  int y;		/* vertical coordinate */
  char entry;		/* code that shows up on map */
};


struct  map_index_data
{
  MAP_INDEX_DATA  *next;
  int 		  vnum;  		  /* vnum of the map */
  int             map_of_vnums[49][81];   /* room vnums aranged as a map */
};


MAP_INDEX_DATA *get_map_index(int vnum);
void            init_maps(void);


/*
 * mudprograms stuff
 */

extern	CHAR_DATA *supermob;

/*
 * MUD_PROGS START HERE
 * (object stuff)
 */
void oprog_speech_trigger( char *txt, CHAR_DATA *ch );
void oprog_random_trigger( OBJ_DATA *obj );
void oprog_wear_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
bool oprog_use_trigger( CHAR_DATA *ch, OBJ_DATA *obj,
                        CHAR_DATA *vict, OBJ_DATA *targ, void *vo );
void oprog_remove_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_sac_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_damage_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_repair_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_drop_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_zap_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_quest_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_fight_trigger( CHAR_DATA *ch, OBJ_DATA *weapon, CHAR_DATA *vch );
char *oprog_type_to_name( int type );

void oprog_greet_trigger( CHAR_DATA *ch );
void oprog_get_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_examine_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_pull_trigger( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_push_trigger( CHAR_DATA *ch, OBJ_DATA *obj );

void rprog_look_trigger( char *txt, CHAR_DATA *ch );
void rprog_leave_trigger( CHAR_DATA *ch );
void rprog_enter_trigger( CHAR_DATA *ch );
void rprog_sleep_trigger( CHAR_DATA *ch );
void rprog_rest_trigger( CHAR_DATA *ch );
void rprog_rfight_trigger( CHAR_DATA *ch );
void rprog_death_trigger( CHAR_DATA *killer, CHAR_DATA *ch );
void rprog_speech_trigger( char *txt, CHAR_DATA *ch );
void rprog_random_trigger( CHAR_DATA *ch );
void rprog_time_trigger( CHAR_DATA *ch );
void rprog_hour_trigger( CHAR_DATA *ch );
void rprog_quest_trigger( CHAR_DATA *mob );
bool rprog_command_trigger( char *txt, CHAR_DATA *ch );
void rprog_area_reset_trigger( ROOM_INDEX_DATA *room );
void rprog_area_init_trigger( ROOM_INDEX_DATA *room );

char *rprog_type_to_name( int type );

void aprog_init_trigger( AREA_DATA *area );
void aprog_reset_trigger( AREA_DATA *area );

#define OPROG_ACT_TRIGGER
#ifdef OPROG_ACT_TRIGGER
void oprog_act_trigger( char *buf, OBJ_DATA *mobj, CHAR_DATA *ch,
			OBJ_DATA *obj, void *vo );
#endif
#define RPROG_ACT_TRIGGER
#ifdef RPROG_ACT_TRIGGER
void rprog_act_trigger( char *buf, ROOM_INDEX_DATA *room, CHAR_DATA *ch,
			OBJ_DATA *obj, void *vo );
#endif


#define GET_ADEPT(ch,sn)    (skill_table[(sn)]->skill_adept[BestSkCl(ch, sn)])
#define LEARNED(ch,sn)	    (IS_NPC(ch) ? 80 : URANGE(0, ch->pcdata->learned[sn], 101))
#define MEMORIZED(ch,sn)    (IS_NPC(ch) ? 5  : URANGE(0, ch->pcdata->memorized[sn], 500))


#define MENU						\
"\n\rWelcome to Desolation of the Dragon MUD\n\r"	\
"0) Exit from Desolation of the Dragon\n\r"		\
"1) Enter the game at DOTD\n\r"				\
"2) Enter description\n\r"				\
"3) Read the background story\n\r"			\
"4) Change password\n\r"				\
"5) Enter somewhere else\n\r"				\
"K) Kill this character!\n\r\n\r"			\
"   Make your choice: "

#if defined(USE_DB) || defined(START_DB)
#include "sql.h"
#endif


#ifdef USE_BOA
void send_to_web(const char *txt, CHAR_DATA *ch);
#endif

void mud_message(CHAR_DATA *ch, int channel, char *arg);
void mud_recv_message(void);

struct dict_entry
{
    char *word;
    char *definition;
    int match;
    DICT_ENTRY *next;
};
int num_words_in_dict(char *word);
DICT_ENTRY *get_words_in_dict(char *word);
void free_dict_entry(DICT_ENTRY *dict);
