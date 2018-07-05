/******************************************************
            Desolation of the Dragon MUD II
      (C) 2001-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <limits.h>

#ifdef __cplusplus
#include <iostream>
#endif

#ifndef UMAX
#define UMAX(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef UMIN
#define UMIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX_INBUF_SIZE
#define MAX_INBUF_SIZE 1024
#endif

#ifndef LINK
#define LINK(link, first, last, next, prev)			\
{								\
    if ( !(first) )						\
      (first)			= (link);			\
    else							\
      (last)->next		= (link);			\
    (link)->next		= NULL;				\
    (link)->prev		= (last);			\
    (last)			= (link);			\
}
#endif
#ifndef UNLINK
#define UNLINK(link, first, last, next, prev)			\
{								\
    if ( !(link)->prev )					\
      (first)			= (link)->next;			\
    else							\
      (link)->prev->next	= (link)->next;			\
    if ( !(link)->next )					\
      (last)			= (link)->prev;			\
    else							\
      (link)->next->prev	= (link)->prev;			\
}
#endif
#define DEBUG 19

typedef struct ldesc LDESC;

struct ldesc {
    int descriptor;
    char *messages_in;
    int len_in;
    int alloc_in;
    char *messages_out;
    int len_out;
    int alloc_out;
#ifndef LISTENER
    LDESC *next;
    LDESC *prev;
#endif
};

void debug_printf(int level, const char *format, ...) __attribute__((format(printf,2,3)));

void read_message_from_ldesc(LDESC *ld);
int send_message_to_ldesc(LDESC *ld, char *data, int datalen);
void disconnect_ldesc(LDESC *ld);
int process_ldesc(LDESC *ld);
void init_ldesc(LDESC *ld);
void process_message(LDESC *ld, char *data, int datalen);

#ifdef LISTENER
extern int shutdown_process;
void send_to_all(char *str);
extern int conn_number;
#else
extern LDESC *first_ld;
extern LDESC *last_ld;
#endif

/*
 * Low level message format:
 *
 * Byte: 1     2345  6-len
 *       [tag] [len] [data]
 *
 * Minimum message length: 6 bytes
 * Minimum data length: 1 byte
 * Maximum message/data length: none
 *
 */
#define MESSAGE_TAG '\xA1'


/*
 * All messages MUST start with:
 * Format: "%d %d ", type, uid
 * type    the message type, see MESSAGE_XXX defines
 * uid     the unique id of the user, used to keep track of the connection,
 *         the mud must remember this to send in reply messages
 */

#define MESSAGE_USERCMD      0
/*
 * The listener sends what the user types to the mud, via this message type.
 * If the mud doesn't recognize the uid, start the login process
 *
 * Format: "%s", input
 * input   what the user typed
 */
#define MESSAGE_SENDTOUSER   1
/*
 * The mud sends data to the user via this message type.  If the listener
 * doesn't recognize the uid, reply with MESSAGE_NOSUCHUSER
 *
 * Format: "%s", data
 * data    the actual data to send to the user
 */

#define MESSAGE_NOSUCHUSER   2
/*
 * Use the message when the user doesn't exist.  If the mud gets this, it
 * should 'disconnect' the user and stop sending data.  If the listener gets
 * this, it should send a MESSAGE_NEWUSER message.
 *
 * No additional data needed
 */

#define MESSAGE_USERDISCON   3
/*
 * When the listener sends this to the mud, the mud should 'disconnect' the
 * user.  When the mud sends this to the listener, the listener handles the
 * user according to action.  The mud can ignore the value of action.
 *
 * Format: "%s", action
 * action  what to do with the user, see MESSAGE_USERDISCON_xxx
 */
#define MESSAGE_USERDISCON_DONTCLOSE   0
#define MESSAGE_USERDISCON_CLOSESOCKET 1

#define MESSAGE_NEWUSER      4
/*
 * The listener sends this to the mud to initiate a new connection.
 *
 * No additional data needed
 */

#define MESSAGE_FIRST MESSAGE_USERCMD
#define MESSAGE_LAST  MESSAGE_NEWUSER
#define TOTAL_MESSAGE_TYPES MESSAGE_LAST+1

struct ldesc_msg_handler {
    void (*process)(LDESC *ld, unsigned int uid, char *data);
};
typedef struct ldesc_msg_handler LDESC_MSG_HANDLER;

/*
 * You must define the function pointers in this structure to handle
 * the incoming messages.  Message types you wish to ignore should be
 * assigned NULL.
 */
extern LDESC_MSG_HANDLER recv_message_handlers[TOTAL_MESSAGE_TYPES];

#define DEFINE_PROCESS_CMD(func) void (func)(LDESC *ld, unsigned int uid, char *data)
