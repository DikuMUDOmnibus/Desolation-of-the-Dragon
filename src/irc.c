/******************************************************
 Desolation of the Dragon MUD II
 (C) 1997-2002  Jesse DeFer
 http://www.dotd.com  dotd@dotd.com
 ******************************************************/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "mud.h"
#include "irc.h"

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

int irc_socket;
fd_set irc_in_set;
fd_set irc_out_set;
fd_set irc_exc_set;

IRC_USERLIST *irc_first_user;
IRC_USERLIST *irc_last_user;

IRC_CHANNEL *irc_first_chan;
IRC_CHANNEL *irc_last_chan;

void irc_do_quit(IRC_USER *u, char *parameters);

IRC_CHANNEL *irc_new_channel(char *name);

void irc_startup(bool copyover)
{
    IRC_CHANNEL *c;
    struct sockaddr_in sa;
    int x = 1;

    if (copyover)
    {
        /* handle copyover */
    }

    if ((irc_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("irc_startup: socket");
        exit(1);
    }

    if (setsockopt(irc_socket, SOL_SOCKET, SO_REUSEADDR, (void *)&x, sizeof(x)) == -1)
    {
        perror("irc_startup: setsockopt, SO_REUSEADDR");
        exit(1);
    }

    memset(&sa, '\0', sizeof(sa));
    sa.sin_family       = AF_INET;
    sa.sin_port         = htons(6667);
    sa.sin_addr.s_addr  = htonl(INADDR_ANY);

    if (bind(irc_socket, (struct sockaddr *)&sa, sizeof(sa)) == -1)
    {
        perror("irc_startup: bind");
        exit(1);
    }

    if (listen(irc_socket, 10) == -1)
    {
        perror("irc_startup: listen");
        exit(1);
    }

    irc_first_user = irc_last_user = NULL;
    irc_first_chan = irc_last_chan = NULL;

    if ((c = irc_new_channel("#OOC")))
    {
        strcpy(c->topic, "Out of Character Chat");
    }
    if ((c = irc_new_channel("#newbie")))
    {
        strcpy(c->topic, "Newbie Help/Chat");
    }
    if ((c = irc_new_channel("#think")))
    {
        strcpy(c->topic, "Immortals Only");
        SET_BIT(c->mode, IRC_CMODE_SECRET);
    }

    log_printf_plus( LOG_IRC, LEVEL_LOG_CSET, SEV_INFO,
                     "IRC Server initialized");
}

IRC_USERLIST *irc_get_userlist(IRC_USERLIST *first, IRC_USER *u)
{
    IRC_USERLIST *ul;
    for (ul = first; ul; ul = ul->next)
	if (ul->user == u)
	    return ul;

    return NULL;
}

void irc_send_to_user(IRC_USER *u, char *message)
{
    int len, buflen;

    if (!message || *message == '\0')
        return;

    if (IRC_MUD_USER(u))
    {
        send_to_char(message, u->ch);
        return;
    }

    len = strlen(message);
    buflen = strlen(u->outbuf);

    while (len + buflen > u->outlen)
    {
        u->outlen += IRC_MAX_MESSAGE_LENGTH;
        u->outbuf = realloc(u->outbuf, u->outlen);
    }

    strncat(u->outbuf, message, IRC_MAX_MESSAGE_LENGTH);
}

void irc_user_printf(IRC_USER *u, char *fmt, ...)
{
    char buf[IRC_MAX_MESSAGE_LENGTH];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf)-1, fmt, args);
    va_end(args);

    irc_send_to_user(u, buf);
}

void irc_reply_to_user(IRC_USER *u, int replycode, char *fmt, ...)
{
    char buf[IRC_MAX_MESSAGE_LENGTH];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf)-1, fmt, args);
    va_end(args);

    irc_user_printf(u, ":%s %d %s %s",
                    "irc.dotd.com",
                    replycode,
                    IRC_NICK(u),
                    buf);
}

void irc_send_to_all(char *message)
{
    IRC_USERLIST *ul;

    for (ul = irc_first_user; ul; ul = ul->next)
        irc_send_to_user(ul->user, message);
}

void irc_send_to_servers(char *message)
{
    IRC_USERLIST *ul;

    for (ul = irc_first_user; ul; ul = ul->next)
        if (IRC_SERVER(ul->user))
            irc_send_to_user(ul->user, message);
}

void irc_send_to_channel_raw(IRC_CHANNEL *c, char *fmt, ...)
{
    IRC_USERLIST *ul;
    char buf[IRC_MAX_MESSAGE_LENGTH+1];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf)-1, fmt, args);
    va_end(args);

    for (ul = c->first_user; ul; ul = ul->next)
    {
        if (IRC_MUD_USER(ul->user))
            continue;
        irc_send_to_user(ul->user, buf);
    }
}

void irc_send_to_channel(IRC_CHANNEL *c, IRC_USER *from, char *command, bool sendtomud, bool sendtouser, char *fmt, ...)
{
    IRC_USERLIST *ul;
    char buf[IRC_MAX_MESSAGE_LENGTH+1];
    char mbuf[IRC_MAX_MESSAGE_LENGTH+1];
    int x, y;
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf)-1, fmt, args);
    va_end(args);

    if (sendtomud)
    {
	if (buf[0] == ':')
	    x = 1;
	else
            x = 0;
        y = 0;
        for (;x<strlen(buf);x++)
            if (buf[x] == '\n' || buf[x] == '\r')
                break;
            else
                mbuf[y++] = buf[x];
        mbuf[y] = '\0';
    }

    for (ul = c->first_user; ul; ul = ul->next)
    {
        if (!sendtouser && ul->user == from)
            continue;
        if (IRC_MUD_USER(ul->user))
        {
            if (sendtomud && mbuf[0] != '\0' && !IRC_MUD_USER(from))
                irc_user_printf(ul->user, "&G[&W%s&G] %ss '%s&G'&w\n\r",
                                IRC_NICK(from), c->name+1, mbuf);
            continue;
        }
        irc_user_printf(ul->user, ":%s %s %s %s",
                        IRC_NICK(from), command, c->name, buf);
    }
}

IRC_USERLIST *irc_chan_user(IRC_CHANNEL *c, IRC_USER *u)
{
    return irc_get_userlist(c->first_user, u);
}
IRC_USERLIST *irc_chan_oper(IRC_CHANNEL *c, IRC_USER *u)
{
    return irc_get_userlist(c->first_oper, u);
}
IRC_USERLIST *irc_chan_speaker(IRC_CHANNEL *c, IRC_USER *u)
{
    return irc_get_userlist(c->first_speaker, u);
}

void irc_remove_channel_user(IRC_CHANNEL *c, IRC_USER *u)
{
    IRC_USERLIST *ul;

    if ((ul = irc_chan_user(c, u)))
    {
        UNLINK(ul, c->first_user, c->last_user, next, prev);
        DISPOSE(ul);
        c->numusers--;
        irc_send_to_channel(c, u, "PART", FALSE, TRUE, "\n\r");
    }
    if ((ul = irc_chan_oper(c, u)))
    {
        UNLINK(ul, c->first_oper, c->last_oper, next, prev);
        DISPOSE(ul);
    }
    if ((ul = irc_chan_speaker(c, u)))
    {
        UNLINK(ul, c->first_speaker, c->last_speaker, next, prev);
        DISPOSE(ul);
    }
}

void irc_remove_user(IRC_USER *u)
{
    IRC_CHANNEL *c;
    IRC_USERLIST *ul;

    for (ul = irc_first_user; ul; ul = ul->next)
	if (ul->user == u)
	{
	    UNLINK(ul, irc_first_user, irc_last_user, next, prev);
	    DISPOSE(ul);
	    break;
	}
    for (c = irc_first_chan; c; c = c->next)
        irc_remove_channel_user(c, u);

    if (u->descriptor)
    {
        FD_CLR(u->descriptor, &irc_in_set);
        FD_CLR(u->descriptor, &irc_out_set);
        close(u->descriptor);
    }

    if (u->username)
        DISPOSE(u->username);
    if (u->hostname)
        DISPOSE(u->hostname);
    if (u->servname)
        DISPOSE(u->servname);
    if (u->realname)
        DISPOSE(u->realname);

    DISPOSE(u->inbuf);
    DISPOSE(u->outbuf);
    DISPOSE(u);
}

void irc_disconnect_user(IRC_USER *u)
{
    irc_remove_user(u);
}

void irc_shutdown(void)
{
    IRC_USERLIST *ul, *ul_next;

    for (ul = irc_first_user; ul; ul = ul_next)
    {
        ul_next = ul->next;
        irc_remove_user(ul->user);
    }

    if (close(irc_socket) == -1)
        perror("irc_shutdown: close");
    irc_socket = 0;

    log_printf_plus( LOG_IRC, LEVEL_LOG_CSET, SEV_INFO,
                     "IRC Server stopped");
}

bool irc_valid_char(int c)
{
    if (isprint(c))
	return TRUE;
    if (c >= 'A' && c <= 'Z')
        return TRUE;
    if (c >= 'a' && c <= 'z')
        return TRUE;
    if (c >= '0' && c <= '9')
        return TRUE;
    if (c == ']' ||
        c == '[' ||
        c == '}' ||
        c == '{' ||
        c == '\\' ||
        c == '|' ||
        c == '`' ||
        c == '^' ||
        c == ' ' ||
        c == '\n' ||
        c == '\r')
        return TRUE;
    return FALSE;
}

int irc_lower(int c)
{
    if (c >= 'A' && c <= 'Z')
        return c + 'a' - 'A';
    if (c == '{')
        return '[';
    if (c == '}')
        return ']';
    if (c == '|')
        return '\\';
    return c;
}

int irc_str_cmp(char *astr, char *bstr)
{
    int x;

    if (strlen(astr) != strlen(bstr))
        return 1;

    for (x=0;x<strlen(astr);x++)
        if (irc_lower(astr[x]) != irc_lower(bstr[x]))
            return 1;

    return 0;
}

bool irc_mask_match(char *astr, char *bstr)
{
    if (!str_cmp(astr, bstr))
	return TRUE;

    return FALSE;
}

IRC_USER *irc_get_user(IRC_USERLIST *first, char *nick)
{
    IRC_USERLIST *ul;

    for (ul = first; ul; ul = ul->next)
        if (!irc_str_cmp(ul->user->nick, nick))
            return ul->user;

    return NULL;
}

IRC_USER *irc_get_user_by_ch(IRC_USERLIST *first, CHAR_DATA *ch)
{
    IRC_USERLIST *ul;

    if (!ch)
        return NULL;

    for (ul = first; ul; ul = ul->next)
        if (ul->user->ch == ch)
            return ul->user;

    return NULL;
}

IRC_CHANNEL *irc_get_channel(char *name)
{
    IRC_CHANNEL *c;

    for (c = irc_first_chan; c; c = c->next)
        if (!irc_str_cmp(c->name, name))
            return c;

    return NULL;
}

bool irc_link_user(IRC_USERLIST **first, IRC_USERLIST **last, IRC_USER *u)
{
    IRC_USERLIST *ul;

    if (irc_get_user(*first, u->nick))
        return FALSE;

    CREATE(ul, IRC_USERLIST, 1);
    ul->user = u;
    LINK(ul, (*first), (*last), next, prev);
    return TRUE;
}

IRC_USER *irc_new_user(int descriptor)
{
    IRC_USER *u;

    CREATE(u, IRC_USER, 1);
    u->ch = NULL;
    u->descriptor = descriptor;
    u->nick[0] = '*';
    u->nick[1] = '\0';
    u->username = NULL;
    u->hostname = NULL;
    u->servname = NULL;
    u->realname = NULL;
    CREATE(u->inbuf, char, IRC_MAX_MESSAGE_LENGTH);
    u->inbuf[0] = '\0';
    u->inlen = IRC_MAX_MESSAGE_LENGTH;
    CREATE(u->outbuf, char, IRC_MAX_MESSAGE_LENGTH);
    u->outbuf[0] = '\0';
    u->outlen = IRC_MAX_MESSAGE_LENGTH;
    u->last_command[0] = '\0';
    u->mode = 0;

    if (!irc_link_user(&irc_first_user, &irc_last_user, u))
    {
        log_printf_plus( LOG_IRC, LEVEL_LOG_CSET, SEV_INFO,
                         "irc_new_user: %s already linked", IRC_NICK(u));
    }

    return u;
}

IRC_CHANNEL *irc_new_channel(char *name)
{
    IRC_CHANNEL *c;

    CREATE(c, IRC_CHANNEL, 1);
    strncpy(c->name, name, IRC_MAX_CHANNEL_LENGTH);
    c->topic[0] = '\0';
    c->mode = IRC_CMODE_NO_OUTSIDE_MSG|IRC_CMODE_TOPIC_OPER;
    c->numusers = 0;
    c->userlimit = 0;
    c->first_user = NULL;
    c->last_user = NULL;
    c->first_oper = NULL;
    c->last_oper = NULL;
    c->first_speaker = NULL;
    c->last_speaker = NULL;
    c->bans = NULL;
    c->invites = NULL;
    c->key = NULL;

    LINK(c, irc_first_chan, irc_last_chan, next, prev);

    return c;
}

void irc_add_channel_user(IRC_CHANNEL *c, IRC_USER *u)
{
    if (!irc_link_user(&c->first_user, &c->last_user, u))
    {
        log_printf_plus( LOG_IRC, LEVEL_LOG_CSET, SEV_INFO,
                         "irc_add_channel_user: %s already linked", IRC_NICK(u));
    }
    else
        c->numusers++;
    irc_send_to_channel(c, u, "JOIN", FALSE, TRUE, "\n\r");
}
void irc_add_channel_oper(IRC_CHANNEL *c, IRC_USER *u)
{
    if (!irc_link_user(&c->first_oper, &c->last_oper, u))
    {
        log_printf_plus( LOG_IRC, LEVEL_LOG_CSET, SEV_INFO,
                         "irc_add_channel_oper: %s already linked", IRC_NICK(u));
    }
}
void irc_add_channel_speaker(IRC_CHANNEL *c, IRC_USER *u)
{
    if (!irc_link_user(&c->first_speaker, &c->last_speaker, u))
    {
        log_printf_plus( LOG_IRC, LEVEL_LOG_CSET, SEV_INFO,
                         "irc_add_channel_speaker: %s already linked", IRC_NICK(u));
    }
}

IRC_USER *irc_new_sock_user(int descriptor, struct sockaddr_in *sa)
{
    IRC_USER *u;

    u = irc_new_user(descriptor);
    u->state = IRC_STATE_REG1;
    u->hostname = str_dup(inet_ntoa(sa->sin_addr));
    u->servname = str_dup("irc.dotd.com");
    return u;
}

IRC_USER *irc_new_mud_user(CHAR_DATA *ch)
{
    char nick[IRC_MAX_NICK_LENGTH+1];
    IRC_USER *u;

    u = irc_new_user(0);
    u->ch = ch;
    u->state = IRC_STATE_CONN;
    strncpy(nick, GET_NAME(ch), IRC_MAX_NICK_LENGTH);
    u->hostname = str_dup(ch->desc->host);
    u->servname = str_dup("irc.dotd.com");
    if ((irc_get_user(irc_first_user, nick)))
    {
        irc_remove_user(u);
        return NULL;
    }
    strcpy(u->nick, nick);
    return u;
}

void irc_accept(void)
{
    IRC_USERLIST *ul;
    IRC_USER *u;
    struct sockaddr_in sa;
    static struct timeval tvzero;
    int maxdesc, desc, size;

    FD_ZERO(&irc_in_set);
    FD_ZERO(&irc_out_set);
    FD_ZERO(&irc_exc_set);

    FD_SET(irc_socket, &irc_in_set);
    maxdesc = irc_socket;

    for (ul = irc_first_user; ul; ul = ul->next)
    {
	u = ul->user;
        if (!IRC_LOCAL_USER(u))
            continue;
        maxdesc = UMAX(maxdesc, u->descriptor);
        FD_SET(u->descriptor, &irc_in_set);
        FD_SET(u->descriptor, &irc_out_set);
        FD_SET(u->descriptor, &irc_exc_set);
    }

    if (select(maxdesc+1, &irc_in_set, &irc_out_set, &irc_exc_set, &tvzero) < 0)
    {
        perror("irc_accept: select");
        exit(1);
    }

    if (FD_ISSET(irc_socket, &irc_exc_set))
    {
        fprintf(stderr, "exception on controlling socket");
        FD_CLR(irc_socket, &irc_in_set);
        FD_CLR(irc_socket, &irc_out_set);
    }
    else if (FD_ISSET(irc_socket, &irc_in_set))
    {
        size = sizeof(sa);
        if ((desc = accept(irc_socket, (struct sockaddr *) &sa, &size)) == -1)
        {
            perror("irc_accept: accept");
            return;
        }
        if (fcntl(desc, F_SETFL, FNDELAY) == -1)
        {
            perror("irc_accept: fcntl");
            return;
        }
        irc_new_sock_user(desc, &sa);
    }
}

void irc_read(IRC_USER *u)
{
    char buf[IRC_MAX_MESSAGE_LENGTH];
    int len, pos, nread;

    if (!IRC_LOCAL_USER(u))
        return;

    if ((nread = read(u->descriptor, buf, IRC_MAX_MESSAGE_LENGTH)) == -1)
    {
        if (errno == EWOULDBLOCK)
            return;
        perror("irc_read: read");
        irc_remove_user(u);
        return;
    }

    if (nread == 0)
    {
        log_printf_plus( LOG_IRC, LEVEL_LOG_CSET, SEV_INFO,
                         "irc_read: EOF received, closing link to %s",
                         IRC_NICK(u));
        irc_do_quit(u, ":EOF encountered on read");
        return;
    }

    if (u->inlen + nread > 8192)
    {
        log_printf_plus( LOG_IRC, LEVEL_LOG_CSET, SEV_INFO,
                         "irc_read: input buffer to long, closing link to %s",
                         IRC_NICK(u));
        irc_do_quit(u, ":Input buffer overflowed");
        return;
    }

    len = strlen(u->inbuf);
    while (len + nread > u->inlen)
    {
        u->inlen += IRC_MAX_MESSAGE_LENGTH;
        u->inbuf = realloc(u->inbuf, u->inlen);
    }

    for (pos = 0; pos < nread; pos++)
        if (irc_valid_char(buf[pos]))
            u->inbuf[len++] = buf[pos];
    u->inbuf[len] = '\0';
}

void irc_write(IRC_USER *u)
{
    int len, pos, written = 0, block;

    if (!IRC_LOCAL_USER(u))
        return;

    len = strlen(u->outbuf);
    if (len == 0)
        return;

    for (pos = 0; pos < len; pos += written)
    {
        block = UMIN(len - pos, 4096);
        if ((written = write(u->descriptor, u->outbuf + pos, block)) == -1)
        {
            perror("irc_write: write");
            irc_remove_user(u);
            return;
        }
    }
    u->outbuf[0] = '\0';
}


irc_commands irc_command_name_to_type(char *command)
{
    if (!command || *command == '\0')
        return IRC_CMD_UNKNOWN;

    switch (tolower(*command))
    {
    case 'a':
        if (!str_cmp(command, "ADMIN"))           return IRC_CMD_ADMIN;
        break;

    case 'c':
        if (!str_cmp(command, "CONNECT"))         return IRC_CMD_CONNECT;
        break;

    case 'e':
        if (!str_cmp(command, "ERROR"))           return IRC_CMD_ERROR;
        break;

    case 'i':
        if (!str_cmp(command, "INFO"))            return IRC_CMD_INFO;
        if (!str_cmp(command, "INVITE"))          return IRC_CMD_INVITE;
        break;

    case 'j':
        if (!str_cmp(command, "JOIN"))            return IRC_CMD_JOIN;
        break;

    case 'k':
        if (!str_cmp(command, "KICK"))            return IRC_CMD_KICK;
        if (!str_cmp(command, "KILL"))            return IRC_CMD_KILL;
        break;

    case 'l':
        if (!str_cmp(command, "LINKS"))           return IRC_CMD_LINKS;
        if (!str_cmp(command, "LIST"))            return IRC_CMD_LIST;
        break;

    case 'm':
        if (!str_cmp(command, "MODE"))            return IRC_CMD_MODE;
        break;

    case 'n':
        if (!str_cmp(command, "NAMES"))           return IRC_CMD_NAMES;
        if (!str_cmp(command, "NICK"))            return IRC_CMD_NICK;
        if (!str_cmp(command, "NOTICE"))          return IRC_CMD_NOTICE;
        break;

    case 'o':
        if (!str_cmp(command, "OPER"))            return IRC_CMD_OPER;
        break;

    case 'p':
        if (!str_cmp(command, "PART"))            return IRC_CMD_PART;
        if (!str_cmp(command, "PASS"))            return IRC_CMD_PASS;
        if (!str_cmp(command, "PING"))            return IRC_CMD_PING;
        if (!str_cmp(command, "PONG"))            return IRC_CMD_PONG;
        if (!str_cmp(command, "PRIVMSG"))         return IRC_CMD_PRIVMSG;
        break;

    case 'q':
        if (!str_cmp(command, "QUIT"))            return IRC_CMD_QUIT;
        break;

    case 's':
        if (!str_cmp(command, "SERVER"))          return IRC_CMD_SERVER;
        if (!str_cmp(command, "STATS"))           return IRC_CMD_STATS;
        if (!str_cmp(command, "SQUIT"))           return IRC_CMD_SQUIT;
        break;

    case 't':
        if (!str_cmp(command, "TIME"))            return IRC_CMD_TIME;
        if (!str_cmp(command, "TOPIC"))           return IRC_CMD_TOPIC;
        if (!str_cmp(command, "TRACE"))           return IRC_CMD_TRACE;
        break;

    case 'u':
        if (!str_cmp(command, "USER"))            return IRC_CMD_USER;
        break;

    case 'v':
        if (!str_cmp(command, "VERSION"))         return IRC_CMD_VERSION;
        break;

    case 'w':
        if (!str_cmp(command, "WHO"))             return IRC_CMD_WHO;
        if (!str_cmp(command, "WHOIS"))           return IRC_CMD_WHOIS;
        if (!str_cmp(command, "WHOWAS"))          return IRC_CMD_WHOWAS;
        break;
    }

    return IRC_CMD_UNKNOWN;
}

bool irc_needmoreparams(IRC_USER *u, char *parameters)
{
    if (!parameters || parameters[0] == '\0')
    {
        irc_reply_to_user(u, 461, "%s :Not enough parameters\n\r",
                       u->last_command);
        return TRUE;
    }
    return FALSE;
}

IRC_CHANNEL *irc_checkchannel(IRC_USER *u, char *parameters)
{
    IRC_CHANNEL *c;
    char name[IRC_MAX_CHANNEL_LENGTH];

    one_argument(parameters, name);

    if (!(c = irc_get_channel(name)))
    {
        irc_reply_to_user(u, 403, "%s :No such channel\n\r", parameters);
        return NULL;
    }

    return c;
}

void irc_do_unfinished(IRC_USER *u, char *parameters)
{
    log_printf_plus( LOG_IRC, LEVEL_LOG_CSET, SEV_INFO,
                     "irc_do_unfinished: %s sent %s %s",
                     IRC_NICK(u), u->last_command, parameters);
}

void irc_do_pass(IRC_USER *u, char *parameters)
{
}

void irc_send_motd(IRC_USER *u)
{
    irc_reply_to_user(u, 375, ":- dotd.com Message of the day -\n\r");
    irc_reply_to_user(u, 372, ":- Welcome!\n\r");
    irc_reply_to_user(u, 376, ":End of /MOTD command.\n\r");
}

void irc_do_nick(IRC_USER *u, char *parameters)
{
    IRC_CHANNEL *c;
    IRC_USER *anotheruser;
    char buf[IRC_MAX_MESSAGE_LENGTH];
    char nick[IRC_MAX_NICK_LENGTH+1];
    int x, len;

    if (!parameters || *parameters == '\0')
    {
        irc_reply_to_user(u, 431, ":No nickname given\n\r");
        return;
    }

    len = UMIN(strlen(parameters), IRC_MAX_NICK_LENGTH);
    for (x=0; x<len; x++)
        nick[x] = parameters[x];
    nick[x] = '\0';

    if ((anotheruser = irc_get_user(irc_first_user, nick)))
    {
        irc_reply_to_user(u, 433, ":Nickname is already in use\n\r");
        return;
    }

    if (u->nick[0] != '*')
    {
        sprintf(buf, ":%s NICK %s\n\r", u->nick, nick);

        irc_send_to_user(u, buf);

        /* fixme: will send duplicate nicks to people in
         * multiple channels with user */
        for (c = irc_first_chan; c; c = c->next)
            if (irc_chan_user(c, u))
                irc_send_to_channel(c, u, "NICK", FALSE, FALSE, "%s\n\r", nick);
    }
    else
	sprintf(buf, "NICK %s\n\r", nick);
    irc_send_to_servers(buf);

    strcpy(u->nick, nick);

    if (u->state == IRC_STATE_CONN)
	return;

    if (++u->state == IRC_STATE_CONN)
    {
        log_printf_plus( LOG_IRC, LEVEL_LOG_CSET, SEV_INFO,
                         "New user: %s(%s)@%s",
                         IRC_NICK(u),
                         u->username?u->username:"*",
                         u->hostname?u->hostname:"*");
        irc_send_motd(u);
    }
}

void irc_do_user(IRC_USER *u, char *parameters)
{
    char username[IRC_MAX_MESSAGE_LENGTH];
    char hostname[IRC_MAX_MESSAGE_LENGTH];
    char servname[IRC_MAX_MESSAGE_LENGTH];

    if (u->state == IRC_STATE_CONN)
    {
        irc_reply_to_user(u, 462, ":You may not reregister\n\r");
        return;
    }

    if (irc_needmoreparams(u, parameters))
        return;

    parameters = one_argument(parameters, username);

    if (irc_needmoreparams(u, parameters))
        return;

    parameters = one_argument(parameters, hostname);

    if (irc_needmoreparams(u, parameters))
        return;

    parameters = one_argument(parameters, servname);

    if (irc_needmoreparams(u, parameters))
        return;

    u->username = str_dup(username);
    if (!u->hostname)
        u->hostname = str_dup(hostname);
    if (!u->servname)
        u->servname = str_dup(servname);
    u->realname = str_dup(parameters);

    if (++u->state == IRC_STATE_CONN)
    {
        log_printf_plus( LOG_IRC, LEVEL_LOG_CSET, SEV_INFO,
                         "New user: %s(%s)@%s",
                         IRC_NICK(u),
                         u->username?u->username:"*",
                         u->hostname?u->hostname:"*");
        irc_send_motd(u);
    }
}

void irc_do_server(IRC_USER *u, char *parameters)
{

}

void irc_do_quit(IRC_USER *u, char *parameters)
{
    char buf[IRC_MAX_MESSAGE_LENGTH];

    sprintf(buf, ":%s QUIT %s\n\r",
            IRC_NICK(u), parameters);

    log_printf_plus( LOG_IRC, LEVEL_LOG_CSET, SEV_INFO,
                     "User quitting: %s", IRC_NICK(u));

    irc_disconnect_user(u);

    irc_send_to_servers(buf);
}

void irc_do_list(IRC_USER *u, char *parameters)
{
    IRC_CHANNEL *c;

    irc_reply_to_user(u, 321, "Channel :Users  Name\n\r");
    for (c = irc_first_chan; c; c = c->next)
    {
	if (IRC_CHAN_MODE(c, IRC_CMODE_SECRET) && !irc_chan_user(c, u))
	    continue;
        irc_reply_to_user(u, 322, "%s %d :%s\n\r",
                        c->name, c->numusers,
			IRC_CHAN_MODE(c, IRC_CMODE_PRIVATE)?"Prv":c->topic);
    }
    irc_reply_to_user(u, 323, ":End of /LIST\n\r", u->nick);
}

void irc_send_topic(IRC_USER *u, IRC_CHANNEL *c)
{
    if (c->topic[0] == '\0')
        irc_reply_to_user(u, 331, "%s :No topic is set\n\r", c->name);
    else
        irc_reply_to_user(u, 332, "%s :%s\n\r", c->name, c->topic);
}

void irc_send_names(IRC_USER *u, IRC_CHANNEL *c)
{
    char buf[IRC_MAX_MESSAGE_LENGTH];
    IRC_USERLIST *ul;

    buf[0] = '\0';

    for (ul = c->first_user; ul; ul = ul->next)
    {
        if (strlen(buf)+32 > IRC_MAX_MESSAGE_LENGTH)
        {
            irc_reply_to_user(u, 353, "%s\n\r", buf);
            buf[0] = '\0';
        }

        if (buf[0] == '\0')
            sprintf(buf, "= %s :", c->name);

        if (irc_chan_oper(c, ul->user))
            strcat(buf, "@");
        strcat(buf, IRC_NICK(ul->user));
        strcat(buf, " ");
    }

    if (buf[0] != '\0')
        irc_reply_to_user(u, 353, "%s\n\r", buf);
    irc_reply_to_user(u, 366, "%s :End of /NAMES list\n\r", c->name);
}

void irc_do_join(IRC_USER *u, char *parameters)
{
    IRC_CHANNEL *c;
    char channels[IRC_MAX_MESSAGE_LENGTH];
    char name[IRC_MAX_CHANNEL_LENGTH];
    char *s;

    if (irc_needmoreparams(u, parameters))
        return;

    parameters = one_argument(parameters, channels);

    s = channels;
    while (1)
    {
        s = one_argumentx(s, name, ',');

        if (name[0] == '\0')
            break;

        if (!(c = irc_checkchannel(u, name)))
            continue;

        if (irc_chan_user(c, u))
            continue;

        irc_add_channel_user(c, u);
        irc_send_topic(u, c);
        irc_send_names(u, c);
    }
}

void irc_do_part(IRC_USER *u, char *parameters)
{
    IRC_CHANNEL *c;
    char channels[IRC_MAX_MESSAGE_LENGTH];
    char name[IRC_MAX_CHANNEL_LENGTH];
    char *s;

    if (irc_needmoreparams(u, parameters))
        return;

    parameters = one_argument(parameters, channels);

    s = channels;
    while (1)
    {
        s = one_argumentx(s, name, ',');

        if (name[0] == '\0')
            break;

        if (!(c = irc_checkchannel(u, name)))
            continue;

        if (!irc_chan_user(c, u))
        {
            irc_reply_to_user(u, 442, "%s :You're not on that channel\n\r", c->name);
            continue;
        }

        irc_remove_channel_user(c, u);
    }
}

unsigned int irc_chan_flag(char mode)
{
    switch (mode)
    {
    case 'o': return IRC_CMODE_OPER;
    case 'p': return IRC_CMODE_PRIVATE;
    case 's': return IRC_CMODE_SECRET;
    case 'i': return IRC_CMODE_INVITE;
    case 't': return IRC_CMODE_TOPIC_OPER;
    case 'n': return IRC_CMODE_NO_OUTSIDE_MSG;
    case 'm': return IRC_CMODE_MODERATED;
    case 'l': return IRC_CMODE_USERLIMIT;
    case 'b': return IRC_CMODE_BAN;
    case 'v': return IRC_CMODE_MODERATOR;
    case 'k': return IRC_CMODE_KEY;
    }
    return 0;
}

unsigned int irc_user_flag(char mode)
{
    switch (mode)
    {
    case 'i': return IRC_UMODE_INVISIBLE;
    case 's': return IRC_UMODE_SERVER_NOTICES;
    case 'w': return IRC_UMODE_WALLOPS;
    case 'o': return IRC_UMODE_OPER;
    }
    return 0;
}

void irc_do_mode(IRC_USER *u, char *parameters)
{
    IRC_CHANNEL *destc;
    IRC_USER *destu;
    char arg[IRC_MAX_CHANNEL_LENGTH];
    char arg2[IRC_MAX_CHANNEL_LENGTH];
    char modestr[IRC_MAX_CHANNEL_LENGTH];
    char extrastr[IRC_MAX_CHANNEL_LENGTH];
    bool setflag;
    unsigned short int pos, mpos, x;
    unsigned int flag;

    if (irc_needmoreparams(u, parameters))
        return;

    parameters = one_argument(parameters, arg);

    destc = NULL;
    if (!(destu = irc_get_user(irc_first_user, arg)))
        destc = irc_get_channel(arg);
    if (!destu && !destc)
    {
        irc_reply_to_user(u, 401, "%s :No such nick/channel\n\r", arg);
        return;
    }

    if (destu && destu != u)
    {
        if (!IRC_USER_MODE(u, IRC_UMODE_OPER))
        {
            irc_reply_to_user(u, 502, ":Cant change mode for other users\n\r");
            return;
        }
    }

    if (!parameters || parameters[0] == '\0')
    {
        modestr[0] = '\0';
        if (destu)
        {
            if (IRC_USER_MODE(destu, IRC_UMODE_INVISIBLE))
                strcat(modestr, "i");
            if (IRC_USER_MODE(destu, IRC_UMODE_SERVER_NOTICES))
                strcat(modestr, "s");
            if (IRC_USER_MODE(destu, IRC_UMODE_WALLOPS))
                strcat(modestr, "w");
            if (IRC_USER_MODE(destu, IRC_UMODE_OPER))
                strcat(modestr, "o");
            irc_reply_to_user(u, 221, "+%s\n\r", IRC_NICK(destu), modestr);
        }
        else if (destc)
        {
            if (IRC_CHAN_MODE(destc, IRC_CMODE_PRIVATE))
                strcat(modestr, "p");
            if (IRC_CHAN_MODE(destc, IRC_CMODE_SECRET))
                strcat(modestr, "s");
            if (IRC_CHAN_MODE(destc, IRC_CMODE_INVITE))
                strcat(modestr, "i");
            if (IRC_CHAN_MODE(destc, IRC_CMODE_TOPIC_OPER))
                strcat(modestr, "t");
            if (IRC_CHAN_MODE(destc, IRC_CMODE_NO_OUTSIDE_MSG))
                strcat(modestr, "n");
            if (IRC_CHAN_MODE(destc, IRC_CMODE_MODERATED))
                strcat(modestr, "m");
            irc_reply_to_user(u, 324, "%s +%s\n\r", destc->name, modestr);
        }
        return;
    }

    if (destc && !irc_chan_oper(destc, u))
    {
        irc_reply_to_user(u, 482, "%s :You're not channel operator\n\r", destc->name);
        return;
    }

    parameters = one_argument(parameters, arg);

    pos = 0;
    setflag = TRUE;
    if (arg[0] == '-')
    {
        pos++;
        setflag = FALSE;
    }
    else if (arg[0] == '+')
    {
        pos++;
        setflag = TRUE;
    }

    if (setflag)
        modestr[0] = '+';
    else
        modestr[0] = '-';
    mpos = 1;
    extrastr[0] = '\0';
    for (x = pos; x < strlen(arg); x++)
    {
        flag = 0;
        if (destu)
            flag = irc_user_flag(arg[x]);
        else if (destc)
            flag = irc_chan_flag(arg[x]);

        if (!flag)
        {
            irc_reply_to_user(u, 472, "%c :is unknown mode char to me\n\r", arg[x]);
            continue;
        }

        if (destu)
        {
            modestr[mpos++] = arg[x];
            if (setflag && !IRC_USER_MODE(destu, flag) &&
                flag != IRC_UMODE_OPER)
                SET_BIT(destu->mode, flag);
            else if (!setflag && IRC_USER_MODE(destu, flag))
                REMOVE_BIT(destu->mode, flag);
            else
                mpos--;
        }
        else if (destc)
        {
            modestr[mpos++] = arg[x];
            if (flag == IRC_CMODE_USERLIMIT)
            {
                if (irc_needmoreparams(u, parameters))
                    continue;
                parameters = one_argument(parameters, arg2);

                destc->userlimit = atoi(arg2);
                sprintf(extrastr+strlen(extrastr), "%d ", destc->userlimit);
            }
            else if (flag == IRC_CMODE_BAN)
            {
                if (irc_needmoreparams(u, parameters))
                    continue;
                parameters = one_argument(parameters, arg2);
                /* finish me */
            }
            else if (flag == IRC_CMODE_MODERATOR)
            {
                if (irc_needmoreparams(u, parameters))
                    continue;
                parameters = one_argument(parameters, arg2);
                /* finish me */
            }
            else if (flag == IRC_CMODE_KEY)
            {
                if (irc_needmoreparams(u, parameters))
                    continue;
                parameters = one_argument(parameters, arg2);

                if (destc->key)
                    DISPOSE(destc->key);
                destc->key = str_dup(arg2);
                sprintf(extrastr+strlen(extrastr), "%s ", destc->key);
            }
            else if (setflag && !IRC_CHAN_MODE(destc, flag))
                SET_BIT(destc->mode, flag);
            else if (!setflag && IRC_CHAN_MODE(destc, flag))
                REMOVE_BIT(destc->mode, flag);
            else
                mpos--;

        }

    }
    modestr[mpos] = '\0';

}

void irc_do_topic(IRC_USER *u, char *parameters)
{
    IRC_CHANNEL *c;
    char name[IRC_MAX_CHANNEL_LENGTH];

    if (irc_needmoreparams(u, parameters))
        return;

    parameters = one_argument(parameters, name);

    if (!(c = irc_checkchannel(u, name)))
        return;

    if (!parameters || parameters[0] == '\0')
    {
        irc_send_topic(u, c);
        return;
    }

    if (!irc_chan_user(c, u))
    {
        irc_reply_to_user(u, 442, "%s :You're not on that channel\n\r", c->name);
        return;
    }

    if (!irc_chan_oper(c, u))
    {
        irc_reply_to_user(u, 482, "%s :You're not channel operator\n\r", c->name);
        return;
    }

    strncpy(c->topic, parameters, IRC_MAX_CHANNEL_LENGTH);
}

void irc_do_names(IRC_USER *u, char *parameters)
{
    IRC_CHANNEL *c;
    char channels[IRC_MAX_MESSAGE_LENGTH];
    char name[IRC_MAX_CHANNEL_LENGTH];
    char *s;

    if (irc_needmoreparams(u, parameters))
        return;

    parameters = one_argument(parameters, channels);

    s = channels;
    while (1)
    {
        s = one_argumentx(s, name, ',');

        if (name[0] == '\0')
            break;

        if (!(c = irc_get_channel(name)))
        {
            irc_reply_to_user(u, 366, "%s :End of /NAMES list\n\r", name);
            continue;
        }

        if (IRC_CHAN_MODE(c, IRC_CMODE_SECRET) && !irc_chan_user(c, u))
        {
            irc_reply_to_user(u, 366, "%s :End of /NAMES list\n\r", name);
            continue;
        }

        irc_send_names(u, c);
    }
}


void irc_do_privmsg(IRC_USER *u, char *parameters)
{
    IRC_USER *destu;
    IRC_CHANNEL *destc;
    char receivers[IRC_MAX_CHANNEL_LENGTH];
    char *r;
    char arg[IRC_MAX_CHANNEL_LENGTH];
    bool notice = FALSE;

    notice = !str_cmp(u->last_command, "NOTICE")?TRUE:FALSE;

    if (!parameters || parameters[0] == '\0' || parameters[0] == ':')
    {
	if (!notice)
	    irc_reply_to_user(u, 411, ":No recipient given (%s)\n\r",
			      u->last_command);
	return;
    }

    parameters = one_argument(parameters, receivers);

    if (!parameters || parameters[0] == '\0')
    {
	if (!notice)
	    irc_reply_to_user(u, 412, ":No text to send\n\r");
	return;
    }

    r = receivers;
    while (1)
    {
        r = one_argumentx(r, arg, ',');
	if (arg[0] == '\0')
	    break;

        destc = NULL;
        if (!(destu = irc_get_user(irc_first_user, arg)))
        {
            if ((destc = irc_get_channel(arg)))
            {
                if ((!irc_chan_user(destc, u) &&
                     IRC_CHAN_MODE(destc, IRC_CMODE_NO_OUTSIDE_MSG)) ||
                    (!irc_chan_oper(destc, u) &&
                     !irc_chan_speaker(destc, u) &&
                     IRC_CHAN_MODE(destc, IRC_CMODE_MODERATED)))
                {
                    irc_reply_to_user(u, 404, "%s :Cannot send to channel\n\r",
                                      destc->name);
                    continue;
                }
            }
        }

        if (!destu && !destc)
        {
            if (arg[0] == '#') /* host mask */
            {
/*                continue;*/
            }
            else if (arg[0] == '$') /* server mask */
            {
/*                continue;*/
            }

            if (!notice)
                irc_reply_to_user(u, 401, "%s :No such nick/channel\n\r", arg);
            continue;
        }

        if (destu)
        {
            if (IRC_MUD_USER(destu))
                irc_user_printf(destu, "&G%s irctells you '%s&G'&w\n\r",
                                IRC_NICK(u),
                                *parameters==':'?parameters+1:parameters);
            else
                irc_user_printf(destu, ":%s PRIVMSG %s %s\n\r",
                                IRC_NICK(u), IRC_NICK(destu), parameters);
        }
        else if (destc)
        {
            irc_send_to_channel(destc, u, "PRIVMSG", TRUE, FALSE, "%s\n\r",
                                parameters);
        }
    }
}

void irc_do_who(IRC_USER *u, char *parameters)
{
   IRC_USERLIST *ul;
   IRC_USER *destu;
   char arg[IRC_MAX_MESSAGE_LENGTH];
   bool oper = FALSE;

   parameters = one_argument(parameters, arg);

   if (parameters && parameters[0] == 'o')
	oper = TRUE;

    for (ul = irc_first_user; ul; ul = ul->next)
    {
	destu = ul->user;
	if (destu->state != IRC_STATE_CONN)
	    continue;
	if (oper && !IRC_USER_MODE(destu, IRC_UMODE_OPER))
	    continue;
        if (irc_mask_match(arg, IRC_NICK(destu)))
            irc_reply_to_user(u, 352, "<channel> <user> %s %s %s <H|G>[*][@|+] :<hopcount> <real name>\n\r",
			      destu->hostname,
			      "irc.dotd.com",
			      IRC_NICK(destu)
			      );
    }
    irc_reply_to_user(u, 315, "%s :End of /WHO list\n\r", arg);
}
void irc_do_whois(IRC_USER *u, char *parameters)
{
}
void irc_do_whowas(IRC_USER *u, char *parameters)
{
}

void irc_do_ping(IRC_USER *u, char *parameters)
{
    IRC_USER *destu;
    char arg[IRC_MAX_MESSAGE_LENGTH];

    if (!parameters || parameters[0] == '\0')
    {
	irc_reply_to_user(u, 409, ":No origin specified\n\r");
	return;
    }

    one_argument(parameters, arg);

    if (!(destu = irc_get_user(irc_first_user, arg)))
    {

	return;
    }

    irc_user_printf(destu, ":%s PING %s\n\r",
                    IRC_NICK(u), parameters);
}
void irc_do_pong(IRC_USER *u, char *parameters)
{
}


typedef void (*irc_func)(IRC_USER *u, char *);

irc_func irc_command_pointers[IRC_CMD_MAX_COMMANDS] =
{
    irc_do_unfinished,
    irc_do_pass,
    irc_do_nick,
    irc_do_user,
    irc_do_server,
    irc_do_unfinished,
    irc_do_quit,
    irc_do_unfinished,
    irc_do_join,
    irc_do_part,
    irc_do_mode,
    irc_do_topic,
    irc_do_names,
    irc_do_list,
    irc_do_unfinished,
    irc_do_unfinished,
    irc_do_unfinished,
    irc_do_unfinished,
    irc_do_unfinished,
    irc_do_unfinished,
    irc_do_unfinished,
    irc_do_unfinished,
    irc_do_unfinished,
    irc_do_unfinished,
    irc_do_privmsg,
    irc_do_privmsg,
    irc_do_who,
    irc_do_whois,
    irc_do_whowas,
    irc_do_unfinished,
    irc_do_ping,
    irc_do_pong,
    irc_do_unfinished
};


void irc_interpret(IRC_USER *u, char *message)
{
    char prefix[IRC_MAX_MESSAGE_LENGTH], command[IRC_MAX_MESSAGE_LENGTH];
    irc_commands cmdnum;
    int lev;

    u->idle = 0;

    if (!message || *message == '\0')
        return;

    message = one_argument(message, prefix);

    if (prefix[0] == ':')
        message = one_argument(message, command);
    else
    {
        strcpy(command, prefix);
        prefix[0] = '\0';
    }

    cmdnum = irc_command_name_to_type(command);

    if (cmdnum == IRC_CMD_UNKNOWN)
    {
        log_printf_plus( LOG_IRC, LEVEL_LOG_CSET, SEV_INFO,
                         "Unknown command %s from %s",
                         command, IRC_NICK(u));
        return;
    }

    if (u->ch)
        lev = UMIN(GetMaxLevel(u->ch)+1, MAX_LEVEL);
    else
        lev = LEVEL_LOG_CSET;

    log_printf_plus( LOG_IRC, lev, SEV_SPAM+9,
                     "%s: %s %s",
                     IRC_NICK(u), command, message);

    strcpy(u->last_command, command);

    if (u->state != IRC_STATE_CONN &&
        cmdnum != IRC_CMD_NICK &&
        cmdnum != IRC_CMD_USER)
    {
        irc_reply_to_user(u, 451, ":You have not registered\n\r");
        return;
    }

    (irc_command_pointers[cmdnum])(u, message);
}

void irc_input_handler(IRC_USER *u)
{
    char message[IRC_MAX_MESSAGE_LENGTH+1];
    int pos, len, start = 0;

    /* don't do anything if no inbuf */
    if (u->inbuf[0] == '\0')
        return;

    len = strlen(u->inbuf);

    /* skip initial crlf's */
    while (start < len)
        if (u->inbuf[start] == '\n' ||
            u->inbuf[start] == '\r')
            start++;
        else
            break;

    /* exit if nothing left or no crlf's */
    if (u->inbuf[start] == '\0' ||
        (!strchr(u->inbuf+start, '\n') &&
         !strchr(u->inbuf+start, '\r')))
        return;

    /* pos winds up being the end of the str */
    for (pos = start; pos < len; pos++)
        if (u->inbuf[pos] == '\n' ||
            u->inbuf[pos] == '\r')
            break;

    strncpy(message, u->inbuf+start, UMIN(pos-start, IRC_MAX_MESSAGE_LENGTH));
    message[UMIN(pos-start, IRC_MAX_MESSAGE_LENGTH)] = '\0';

    while (pos < len)
        if (u->inbuf[pos] == '\n' ||
            u->inbuf[pos] == '\r')
            pos++;
        else
            break;

    if (len-pos > 0)
    {
        memmove(u->inbuf, u->inbuf+pos, len-pos);
	u->inbuf[len-pos] = '\0';
    }
    else
        u->inbuf[0] = '\0';

    irc_interpret(u, message);
}

void irc_loop(void)
{
    IRC_USERLIST *ul, *ul_next;
    IRC_USER *u;

    if (irc_socket < 1)
        return;

    irc_accept();

    /* Exceptions */
    for (ul = irc_first_user; ul; ul = ul_next)
    {
        ul_next = ul->next;
	u = ul->user;
        u->idle++;
        if (FD_ISSET(u->descriptor, &irc_exc_set))
            irc_remove_user(u);
    }

    /* Input */
    for (ul = irc_first_user; ul; ul = ul_next)
    {
        ul_next = ul->next;
	u = ul->user;
        if (FD_ISSET(u->descriptor, &irc_in_set))
            irc_read(u);
    }

    /* Messages */
    for (ul = irc_first_user; ul; ul = ul_next)
    {
        ul_next = ul->next;
	u = ul->user;
        irc_input_handler(u);
    }

    /* Output */
    for (ul = irc_first_user; ul; ul = ul_next)
    {
        ul_next = ul->next;
	u = ul->user;
        if (FD_ISSET(u->descriptor, &irc_out_set))
            irc_write(u);
    }
}

void do_irc(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    IRC_USERLIST *ul;
    IRC_USER *u;

    if (!argument || argument[0] == '\0')
    {
        send_to_char("Syntax:\n\r"
                     "  irc logon\n\r"
                     "  irc logoff\n\r",
                     ch);
        if (IS_IMMORTAL(ch))
            send_to_char("  irc users\n\r"
                         "  irc kick\n\r"
                         "  irc startup\n\r"
                         "  irc shutdown\n\r",
                         ch);
        return;
    }

    if (!str_cmp(argument, "logoff"))
    {
        irc_logoff(ch);
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(argument, "logon"))
    {
        irc_logon(ch);
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!IS_IMMORTAL(ch))
    {
        do_irc(ch, "");
        return;
    }

    if (!str_cmp(argument, "users"))
    {
        send_to_char("Desc|Con|Idle|Port |Player@Host\n\r"
                     "----+---+----+-----+--------------------------------\n\r",
                     ch);

        for (ul = irc_first_user; ul && (u = ul->user); ul = ul->next)
            ch_printf(ch, " %3d|%3d|%4d|%5d|%-9.9s@%s\n\r",
                      u->descriptor,
                      u->state,
                      u->idle/PULSE_PER_SECOND,
                      0,
                      *u->nick?u->nick:"(no nick)",
                      u->hostname);
        return;
    }

    if (!str_cmp(argument, "startup"))
    {
        irc_startup(FALSE);
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(argument, "shutdown"))
    {
        irc_shutdown();
        send_to_char("Ok.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg);

    if (!argument || argument[0] == '\0')
    {
        do_irc(ch, "");
        return;
    }

    if (!str_cmp(arg, "kick"))
    {
        if ((u = irc_get_user(irc_first_user, argument)))
            irc_remove_user(u);
        send_to_char("Ok.\n\r", ch);
        return;
    }

    do_irc(ch, "");
}

void irc_logon(CHAR_DATA *ch)
{
    IRC_USER *u;
    IRC_CHANNEL *c;

    if (!(u = irc_new_mud_user(ch)))
    {
        send_to_char("Unable to log on.\n\r", ch);
        return;
    }

    if ((c = irc_get_channel("#OOC")))
        irc_add_channel_user(c, u);
    if ((c = irc_get_channel("#newbie")))
        irc_add_channel_user(c, u);
    if (IS_IMMORTAL(ch) && (c = irc_get_channel("#think")))
        irc_add_channel_user(c, u);
}

void irc_logoff(CHAR_DATA *ch)
{
    IRC_USER *u;

    if (!ch)
        return;

    if ((u = irc_get_user_by_ch(irc_first_user, ch)))
        irc_remove_user(u);
}

void irc_mud_to_channel(CHAR_DATA *ch, char *channel, char *message)
{
    char buf[IRC_MAX_MESSAGE_LENGTH+1];
    IRC_USER *u;
    IRC_CHANNEL *c;

    if (!(u = irc_get_user_by_ch(irc_first_user, ch)))
        return;

    if (!(c = irc_get_channel(channel)))
        return;

    if (!irc_chan_user(c, u))
        return;

    sprintf(buf, "PRIVMSG %s :%s", c->name, message);
    irc_interpret(u, buf);
}
