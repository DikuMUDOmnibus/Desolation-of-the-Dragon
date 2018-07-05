/****************************************************************************
 *                   ^     +----- |  / ^     ^ |     | +-\                  *
 *                  / \    |      | /  |\   /| |     | |  \                 *
 *                 /   \   +---   |<   | \ / | |     | |  |                 *
 *                /-----\  |      | \  |  v  | |     | |  /                 *
 *               /       \ |      |  \ |     | +-----+ +-/                  *
 ****************************************************************************
 * AFKMud Copyright 1997-2002 Alsherok. Contributors: Samson, Dwip, Whir,   *
 * Cyberfox, Karangi, Rathian, Cam, Raine, and Tarl.                        *
 *                                                                          *
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag,        *
 * Blodkai, Haus, Narn, Scryn, Swordbearer, Tricops, Gorog, Rennard,        *
 * Grishnakh, Fireblade, and Nivek.                                         *
 *                                                                          *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                      *
 *                                                                          *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen,      *
 * Michael Seifert, and Sebastian Hammer.                                   *
 ****************************************************************************
 *                         Intermud-3 Network Module                        *
 ****************************************************************************/

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
 * Copyright (c)2001-2002 Roger Libiez ( Samson )
 * Registered with the United States Copyright Office
 * TX 5-562-404
 */

#include "i3cfg.h"

/* You should not need to edit anything below this line if I've done this all correctly. */

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

#ifndef FCLOSE
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
	perror("malloc failure");						\
	fprintf(stderr, "Malloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
	abort();									\
    }											\
} while(0)

#define I3RECREATE(result,type,number)					\
do											\
{											\
    if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
    {											\
	perror("realloc failure");						\
	fprintf(stderr, "Realloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
	abort();									\
    }											\
} while(0)

#define I3DISPOSE(point) 								\
do											\
{											\
  if (!(point))									\
  {											\
	i3bug( "Freeing null pointer %s:%d", __FILE__, __LINE__ ); \
	fprintf( stderr, "DISPOSEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }											\
  else										\
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
   if (first == link)								\
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

// internal structures
typedef struct I3_channel	I3_CHANNEL;
typedef struct I3_mud		I3_MUD;
typedef struct I3_stats		I3_STATS;
typedef struct I3_header	I3_HEADER;
typedef struct I3_ignore	I3_IGNORE;
typedef struct ucache_data	UCACHE_DATA;
typedef struct i3_chardata    I3_CHARDATA;

extern char *I3_THISMUD;
extern char *I3_ROUTER_NAME;
extern int I3_socket;

extern I3_CHANNEL *first_I3chan;
extern I3_CHANNEL *last_I3chan;
extern I3_MUD *first_mud;
extern I3_MUD *last_mud;
extern I3_MUD *this_mud;
extern UCACHE_DATA *first_ucache;
extern UCACHE_DATA *last_ucache;
extern I3_STATS I3_stats;

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

struct i3_chardata
{
   char *		i3_replyname; /* Target for reply - Samson 1-23-01 */
   char *		i3_listen; /* The I3 channels someone is listening to - Samson 1-30-01 */
   bool		i3invis; /* Invisible to I3? - Samson 2-7-01 */
   I3_IGNORE	*i3first_ignore; /* List of people to ignore stuff from - Samson 2-7-01 */
   I3_IGNORE	*i3last_ignore;
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
   int local_level;
   char *history[20];
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

   // only used for this mud
   char *routerIP;
   char *routerName;
   int  routerPort;
   bool autoconnect;
   int password;
   int mudlist_id;
   int chanlist_id;
   int minlevel;
   int adminlevel;
};

struct I3_stats 
{
    int count_tell_commands;
    int count_tell;
    int count_beep_commands;
    int count_beep;
    int count_emoteto_commands;
    int count_emoteto;
    int count_who_commands;
    int count_who_req;
    int count_who_reply;
    int count_finger_commands;
    int count_finger_req;
    int count_finger_reply;
    int count_locate_commands;
    int count_locate_req;
    int count_locate_reply;
    int count_chanlist_reply;
    int count_channel_m_commands;
    int count_channel_m;
    int count_channel_e_commands;
    int count_channel_e;
    int count_channel_t_commands;
    int count_channel_t;
    int count_channel_add;
    int count_channel_remove;
    int count_channel_admin;
    int count_channel_filter_req;
    int count_channel_filter_reply;
    int count_channel_who_commands;
    int count_channel_who_req;
    int count_channel_who_reply;
    int count_channel_listen;
    int count_chan_user_req;
    int count_chan_user_reply;
    int count_news_read_req;
    int count_news_post_req;
    int count_news_grplist_req;
    int count_mail;
    int count_mail_ack;
    int count_file_list_req;
    int count_file_list_reply;
    int count_file_put;
    int count_file_get_req;
    int count_file_get_reply;
    int count_auth_mud_req;
    int count_auth_mud_reply;
    int count_ucache_update;
    int count_error;
    int count_startup_req_3;
    int count_startup_reply;
    int count_shutdown;
    int count_mudlist;
    int count_oob_req;
    int count_oob_begin;
    int count_oob_end;

    int count_unknown;
    int count_total;
};

/* Channel flags, only one so far, but you never know when more might be useful */
#define I3CHAN_LOG 1

/* External hooks */
void I3_main( bool forced, int mudport, bool isconnected );
bool I3_is_connected( void );
void I3_loop( void );
void I3_shutdown( int delay );
bool I3_command_hook( CHAR_DATA *ch, char *command, char *argument );
void i3init_char( CHAR_DATA *ch );
void i3save_char( CHAR_DATA *ch, FILE *fp );
bool i3load_char( CHAR_DATA *ch, FILE *fp, char *word );
void free_i3chardata( CHAR_DATA *ch );
int I3_process_who_req( char *s );
void I3_loadmudlist( void );
void I3_loadchanlist( void );
void I3_savemudlist( void );
void I3_savechanlist( void );
