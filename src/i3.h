/*
 * Copyright (c) 2000 Fatal Dimensions
 *
 * See the file "LICENSE" or information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/* Ported to Smaug 1.4a by Samson of Alsherok.
 * Consolidated for cross-codebase compatibility by Samson of Alsherok.
 * Modifications and enhancements to the code
 * Copyright (c)2001-2003 Roger Libiez ( Samson )
 * Registered with the United States Copyright Office
 * TX 5-562-404
 */

/* Number of messages to store in the channel history */
#define MAX_I3HISTORY 20

/* Locations of the configuration files */
#define I3_CONFIG_FILE   "../i3/i3.config"
#define I3_CHANNEL_FILE  "../i3/i3.channels"
#define I3_PASSWORD_FILE "../i3/i3.password"
#define I3_UCACHE_FILE   "../i3/i3.ucache"
#define I3_MUDLIST_FILE  "../i3/i3.mudlist"
#define I3_CHANLIST_FILE "../i3/i3.chanlist"
#define I3_BLOCK_FILE    "../i3/i3.blocks"

typedef enum
{
   I3PERM_NOTSET, I3PERM_NONE, I3PERM_MORT, I3PERM_IMM, I3PERM_ADMIN, I3PERM_IMP
} permissions;

/* Ye olde Merc-like command struct */
struct i3cmd_type
{
   char *const name;
   void (*function) ( CHAR_DATA *ch, char *argument );
   int level;
   bool connected;
};

/* Flag macros */
#define I3IS_SET(flag, bit)	((flag) & (bit))
#define I3SET_BIT(var, bit)	((var) |= (bit))
#define I3REMOVE_BIT(var, bit)((var) &= ~(bit))

/* Channel flags, only one so far, but you never know when more might be useful */
#define I3CHAN_LOG	(1 <<  0)

/* Player flags */
#define I3_TELL		(1 <<  0)
#define I3_DENYTELL	(1 <<  1)
#define I3_BEEP		(1 <<  2)
#define I3_DENYBEEP	(1 <<  3)
#define I3_INVIS		(1 <<  4)
#define I3_PRIVACY	(1 <<  5)
#define I3_DENYFINGER	(1 <<  6)
#define I3_AFK		(1 <<  7)
#define I3_COLOR        (1 <<  8)

/* Make sure you've examined the i3cfg.h file and set your information accordingly!
 * If you don't, things below this point will fail - Samson 7-15-03 */
#include "i3cfg.h"

/* You should not need to edit anything below this line if I've done this all correctly. */

/* The current revision of the I3 code */
#define I3DRIVER "AFKMud I3 Driver 2.1"

#define I3PERM(ch)		(CH_I3DATA((ch))->i3perm)
#define I3FLAG(ch)		(CH_I3DATA((ch))->i3flags)
#define FIRST_I3IGNORE(ch)	(CH_I3DATA((ch))->i3first_ignore)
#define LAST_I3IGNORE(ch)	(CH_I3DATA((ch))->i3last_ignore)
#define I3LISTEN(ch)		(CH_I3DATA((ch))->i3_listen)
#define I3DENY(ch)		(CH_I3DATA((ch))->i3_denied)
#define I3REPLY(ch)		(CH_I3DATA((ch))->i3_replyname)
#define I3INVIS(ch)		( I3IS_SET( I3FLAG((ch)), I3_INVIS ) )
#define CH_AFK(ch)		( I3IS_SET( I3FLAG((ch)), I3_AFK ) )
#define I3ISINVIS(ch)		( WIZINVIS((ch)) || I3INVIS((ch)) )

/* Well, you never know. Someone may have decided to utterly redo this for some reason. */
#ifndef MAX_STRING_LENGTH
   #define MAX_STRING_LENGTH 4096
#endif

#ifndef MAX_INPUT_LENGTH
   #define MAX_INPUT_LENGTH 1024
#endif

#ifndef MSL 
   #define MSL MAX_STRING_LENGTH
#endif

#ifndef MIL
   #define MIL MAX_INPUT_LENGTH
#endif

#if !defined( FCLOSE ) && !defined( I3CHRONICLES )
   /* Macro taken from DOTD codebase. Fcloses a file, then nulls its pointer for safety. */
   #define FCLOSE(fp)  fclose(fp); fp=NULL;
#endif

/*
 * Memory allocation macros.
 */
#define I3CREATE(result, type, number)					\
do											\
{											\
    if (!((result) = (type *) calloc ((number), sizeof(type))))	\
    {											\
	i3log( "Malloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
	abort();									\
    }											\
} while(0)

#define I3RECREATE(result,type,number)					\
do											\
{											\
    if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
    {											\
	i3log( "Realloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
	abort();									\
    }											\
} while(0)

#define I3DISPOSE(point) 								\
do											\
{											\
   if((point))									\
   {											\
	free((point));								\
	(point) = NULL;								\
   }											\
} while(0)

/* double-linked list handling macros -Thoric ( From the Smaug codebase ) */
/* Updated by Scion 8/6/1999 */
#define I3LINK(link, first, last, next, prev)                     \
do                                                              	\
{                                                               	\
   if ( !(first) )								\
   {                                           				\
      (first) = (link);				                       	\
      (last) = (link);							    	\
   }											\
   else                                                      	\
      (last)->next = (link);			                       	\
   (link)->next = NULL;			                         	\
   if ((first) == (link))							\
      (link)->prev = NULL;							\
   else										\
      (link)->prev = (last);			                       	\
   (last) = (link);				                       	\
} while(0)

#define I3INSERT(link, insert, first, next, prev)               \
do                                                              \
{                                                               \
   (link)->prev = (insert)->prev;			                \
   if ( !(insert)->prev )                                       \
      (first) = (link);                                         \
   else                                                         \
      (insert)->prev->next = (link);                            \
   (insert)->prev = (link);                                     \
   (link)->next = (insert);                                     \
} while(0)

#define I3UNLINK(link, first, last, next, prev)                   \
do                                                              	\
{                                                               	\
	if ( !(link)->prev )							\
	{			                                    	\
         (first) = (link)->next;			                 	\
	   if ((first))							 	\
	      (first)->prev = NULL;						\
	} 										\
	else										\
	{                                                 		\
         (link)->prev->next = (link)->next;                 	\
	}										\
	if ( !(link)->next ) 							\
	{				                                    \
         (last) = (link)->prev;                 			\
	   if ((last))								\
	      (last)->next = NULL;						\
	} 										\
	else										\
	{                                                    		\
         (link)->next->prev = (link)->prev;                 	\
	}										\
} while(0)

/*
 * Color Alignment Parameters
 */
#ifndef ALIGN_LEFT
   #define ALIGN_LEFT	1
#endif
#ifndef ALIGN_CENTER
   #define ALIGN_CENTER	2
#endif
#ifndef ALIGN_RIGHT
   #define ALIGN_RIGHT	3
#endif

/* Internal structures */
typedef struct I3_channel	I3_CHANNEL;
typedef struct I3_mud		I3_MUD;
typedef struct I3_header	I3_HEADER;
typedef struct I3_ignore	I3_IGNORE;
typedef struct I3_block		I3_BLOCK;
typedef struct ucache_data	UCACHE_DATA;
typedef struct i3_chardata    I3_CHARDATA;

extern char *I3_THISMUD;
extern char *I3_ROUTER_NAME;
extern int I3_socket;

extern I3_CHANNEL *first_I3chan;
extern I3_CHANNEL *last_I3chan;
extern I3_BLOCK *first_i3block;
extern I3_BLOCK *last_i3block;
extern I3_MUD *first_mud;
extern I3_MUD *last_mud;
extern I3_MUD *this_mud;
extern UCACHE_DATA *first_ucache;
extern UCACHE_DATA *last_ucache;

struct ucache_data
{
   UCACHE_DATA *next;
   UCACHE_DATA *prev;
   char *name;
   int gender;
   time_t time;
};

struct I3_ignore
{
   I3_IGNORE *next;
   I3_IGNORE *prev;
   char *name;
};

struct I3_block
{
   I3_BLOCK *next;
   I3_BLOCK *prev;
   char *name;
};

struct i3_chardata
{
   char *i3_replyname;        /* Target for reply - Samson 1-23-01 */
   char *i3_listen;           /* The I3 channels someone is listening to - Samson 1-30-01 */
   char *i3_denied;           /* The I3 channels someone is forbidden to use - Samson 6-16-03 */
   int i3flags;               /* Flag settings such as invis, tell on/off, beep on/off, etc. - Samson 6-30-03 */
   int i3perm;                /* Your permission setting. None, All, Imm, Admin, Imp - Samson 6-25-03 */
   I3_IGNORE *i3first_ignore; /* List of people to ignore stuff from - Samson 2-7-01 */
   I3_IGNORE *i3last_ignore;
};

struct I3_header 
{
    char originator_mudname[256];
    char originator_username[256];
    char target_mudname[256];
    char target_username[256];
};

struct I3_channel 
{
   I3_CHANNEL *next;
   I3_CHANNEL *prev;
   char *local_name;
   char *host_mud;
   char *I3_name;
   char *layout_m;
   char *layout_e;
   int status;
   int i3perm;
   char *history[MAX_I3HISTORY];
   int flags;
};

struct I3_mud 
{
   I3_MUD *next;
   I3_MUD *prev;
   int status;
   char *name;
   char *ipaddress;
   char *mudlib;
   char *base_mudlib;
   char *driver;
   char *mud_type;
   char *open_status;
   char *admin_email;
   char *telnet;
   char *web;
   int  player_port;
   int  imud_tcp_port;
   int  imud_udp_port;
 
   bool tell;
   bool beep;
   bool emoteto;
   bool who;
   bool finger;
   bool locate;
   bool channel;
   bool news;
   bool mail;
   bool file;
   bool auth;
   bool ucache;

   int smtp;
   int ftp;
   int nntp;
   int http;
   int pop3;
   int rcp;
   int amrcp;

   /* only used for this mud */
   char *routerIP;
   char *routerName;
   int  routerPort;
   bool autoconnect;
   int password;
   int mudlist_id;
   int chanlist_id;
   int minlevel;
   int immlevel;
   int adminlevel;
   int implevel;
};

/* External hooks */
void I3_main( bool forced, int mudport, bool isconnected );
bool I3_is_connected( void );
void I3_loop( void );
void I3_shutdown( int delay );
bool I3_command_hook( CHAR_DATA *ch, char *command, char *argument );
void i3init_char( CHAR_DATA *ch );
void i3save_char( CHAR_DATA *ch, FILE *fp );
bool i3load_char( CHAR_DATA *ch, FILE *fp, const char *word );
void free_i3chardata( CHAR_DATA *ch );
int I3_process_who_req( char *s );
void I3_loadmudlist( void );
void I3_loadchanlist( void );
void I3_savemudlist( void );
void I3_savechanlist( void );
void I3_savehistory( void );
