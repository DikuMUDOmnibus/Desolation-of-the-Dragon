/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

#include <netdb.h>

typedef struct irc_user_list IRC_USERLIST;
typedef struct irc_user IRC_USER;
typedef struct irc_channel IRC_CHANNEL;

#define IRC_MAX_NICK_LENGTH	9
#define IRC_MAX_CHANNEL_LENGTH  200
#define IRC_MAX_MESSAGE_LENGTH  512
#define IRC_MAX_OUTPUT_LENGTH   4096

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

/* IRC Channel Modes */
#define IRC_CMODE_OPER			BV00
#define IRC_CMODE_PRIVATE		BV01
#define IRC_CMODE_SECRET		BV02
#define IRC_CMODE_INVITE		BV03
#define IRC_CMODE_TOPIC_OPER		BV04
#define IRC_CMODE_NO_OUTSIDE_MSG	BV05
#define IRC_CMODE_MODERATED		BV06
#define IRC_CMODE_USERLIMIT             BV07
#define IRC_CMODE_BAN                   BV08
#define IRC_CMODE_MODERATOR             BV09
#define IRC_CMODE_KEY                   BV10

/* IRC User Modes */
#define IRC_UMODE_SERVER                BV00
#define IRC_UMODE_INVISIBLE		BV01
#define IRC_UMODE_SERVER_NOTICES	BV02
#define IRC_UMODE_WALLOPS		BV03
#define IRC_UMODE_OPER			BV04

#define IRC_SERVER(u)		(IS_SET((u)->mode, IRC_UMODE_SERVER))
#define IRC_NICK(u)		((u)->nick[0]?(u)->nick:"*")
#define IRC_MUD_USER(u)		((u)->ch)
#define IRC_LOCAL_USER(u)	((u)->descriptor)

#define IRC_USER_MODE(u,flag)   (IS_SET((u)->mode, (flag)))
#define IRC_CHAN_MODE(c,flag)	(IS_SET((c)->mode, (flag)))

typedef enum
{
    IRC_STATE_REG1, IRC_STATE_REG2, IRC_STATE_CONN
} irc_states;

typedef enum
{
    IRC_CMD_UNKNOWN, IRC_CMD_PASS, IRC_CMD_NICK, IRC_CMD_USER,
    IRC_CMD_SERVER, IRC_CMD_OPER, IRC_CMD_QUIT, IRC_CMD_SQUIT,
    IRC_CMD_JOIN, IRC_CMD_PART, IRC_CMD_MODE, IRC_CMD_TOPIC,
    IRC_CMD_NAMES, IRC_CMD_LIST, IRC_CMD_INVITE, IRC_CMD_KICK,
    IRC_CMD_VERSION, IRC_CMD_STATS, IRC_CMD_LINKS, IRC_CMD_TIME,
    IRC_CMD_CONNECT, IRC_CMD_TRACE, IRC_CMD_ADMIN, IRC_CMD_INFO,
    IRC_CMD_PRIVMSG, IRC_CMD_NOTICE, IRC_CMD_WHO, IRC_CMD_WHOIS,
    IRC_CMD_WHOWAS, IRC_CMD_KILL, IRC_CMD_PING, IRC_CMD_PONG,
    IRC_CMD_ERROR,

    IRC_CMD_MAX_COMMANDS
} irc_commands;

struct irc_user_list
{
    IRC_USERLIST *next;
    IRC_USERLIST *prev;
    IRC_USER *user;
};

struct irc_user
{
    CHAR_DATA *ch;
    int descriptor;
    irc_states state;
    unsigned int idle;
    char nick[IRC_MAX_NICK_LENGTH+1];
    char *username;
    char *hostname;
    char *servname;
    char *realname;
    char *inbuf;
    unsigned short int inlen;
    char *outbuf;
    unsigned short int outlen;
    char last_command[IRC_MAX_MESSAGE_LENGTH+1];
    unsigned int mode;
};

struct irc_channel
{
    IRC_CHANNEL *next;
    IRC_CHANNEL *prev;
    char name[IRC_MAX_CHANNEL_LENGTH+1];
    char topic[IRC_MAX_CHANNEL_LENGTH+1];
    unsigned int mode;
    unsigned int numusers;
    unsigned int userlimit;
    IRC_USERLIST *first_user;
    IRC_USERLIST *last_user;
    IRC_USERLIST *first_oper;
    IRC_USERLIST *last_oper;
    IRC_USERLIST *first_speaker;
    IRC_USERLIST *last_speaker;
    char *bans;
    char *invites;
    char *key;
};

extern int irc_socket;

void irc_startup(bool copyover);
void irc_shutdown(void);
void irc_loop(void);

void irc_logon(CHAR_DATA *ch);
void irc_logoff(CHAR_DATA *ch);

void irc_mud_to_channel(CHAR_DATA *ch, char *channel, char *message);
