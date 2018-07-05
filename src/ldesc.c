/******************************************************
            Desolation of the Dragon MUD II
      (C) 2001-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/
#include "mud.h"
#include "ldesc.h"

LDESC *first_ld=NULL;
LDESC *last_ld=NULL;

DESCRIPTOR_DATA *first_ld_desc;
DESCRIPTOR_DATA *last_ld_desc;

DEFINE_PROCESS_CMD(process_usercmd);
DEFINE_PROCESS_CMD(process_nosuchuser);
DEFINE_PROCESS_CMD(process_userdiscon);
DEFINE_PROCESS_CMD(process_newuser);

LDESC_MSG_HANDLER recv_message_handlers[TOTAL_MESSAGE_TYPES] =
{
    { process_usercmd },
    { NULL },
    { process_nosuchuser },
    { process_userdiscon },
    { process_newuser }
};

DESCRIPTOR_DATA *get_descr_by_uid(unsigned int uid)
{
    DESCRIPTOR_DATA *d;

    for (d = first_descriptor; d; d = d->next)
        if (d->uid == uid)
            return d;

    return NULL;
}

DEFINE_PROCESS_CMD(process_usercmd)
{
    DESCRIPTOR_DATA *d = get_descr_by_uid(uid);

    if (!d)
    {
        char tbuf[32];
        snprintf(tbuf, 32, "%d %d ", MESSAGE_NOSUCHUSER, uid);
        debug_printf(10, "process_usercmd: MESSAGE_NOSUCHUSER %d\n", uid);
        send_message_to_ldesc(ld, tbuf, strlen(tbuf));
        return;
    }

    debug_printf(15, "process_usercmd: len: %d [%s]\n", strlen(data), data);

    strcat(d->inbuf+strlen(d->inbuf), data);
    strcat(d->inbuf+strlen(d->inbuf), "\n\r");
}

DEFINE_PROCESS_CMD(process_nosuchuser)
{
    DESCRIPTOR_DATA *d = get_descr_by_uid(uid);

    if (!d)
        return;

    debug_printf(15, "process_nosuchuser: len: %d [%s]\n", strlen(data), data);

    close_socket(d, TRUE);
}

DEFINE_PROCESS_CMD(process_userdiscon)
{
    DESCRIPTOR_DATA *d = get_descr_by_uid(uid);
    int action;

    if (!d)
        return;

    action = atoi(data);
    debug_printf(15, "process_userdiscon: uid: %d, data: %s, action: %d\n", uid, data, action);

    switch (action)
    {
    case MESSAGE_USERDISCON_DONTCLOSE:
        close_socket(d, FALSE);
        break;
    case MESSAGE_USERDISCON_CLOSESOCKET:
        close_socket(d, TRUE);
        break;
    }
}

DEFINE_PROCESS_CMD(process_newuser)
{
    DESCRIPTOR_DATA *dnew=NULL;
    char tbuf[128];

    CREATE(dnew, DESCRIPTOR_DATA, 1);
    init_descriptor(dnew, uid);
    LINK( dnew, first_descriptor, last_descriptor, next, prev );
    dnew->host = STRALLOC( "nowhere" );

    snprintf(tbuf, sizeof(tbuf), "%d %d What is your name? ", MESSAGE_SENDTOUSER, uid );

    send_message_to_ldesc(ld, tbuf, strlen(tbuf));
}

void process_listener_connection(int fd)
{
    static struct timeval null_time;
    fd_set in_set, out_set, exc_set;
    int maxdesc = fd;
    LDESC *ld, *ld_next;

    if (!fd)
        return;

    FD_ZERO( &in_set  );
    FD_ZERO( &out_set );
    FD_ZERO( &exc_set );
    FD_SET( fd, &in_set );

    for ( ld = first_ld; ld; ld = ld->next )
    {
        maxdesc = UMAX(maxdesc, ld->descriptor);
        FD_SET( ld->descriptor, &in_set  );
        FD_SET( ld->descriptor, &out_set );
        FD_SET( ld->descriptor, &exc_set );
    }

    if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) == -1 )
    {
        perror( "process_listener_connections: select" );
        close(fd);
        return;
    }

    if ( FD_ISSET( fd, &exc_set ) )
    {
        close(fd);
        return;
    }

    if ( FD_ISSET( fd, &in_set ) )
    {
        struct sockaddr_un sock;
        int desc;
        unsigned int size = sizeof(sock);

        if ( ( desc = accept( fd, (struct sockaddr *) &sock, &size) ) == -1 )
        {
            perror( "process_listener_connections: accept" );
            return;
        }

        if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
        {
            perror( "process_listener_connections: fcntl: FNDELAY" );
            return;
        }

        ld = (LDESC *)calloc(1,sizeof(LDESC));
        init_ldesc(ld);
        ld->descriptor = desc;

        LINK(ld, first_ld, last_ld, next, prev );

        debug_printf(10, "process_listener_connections: connected to listener on fd %d\n", ld->descriptor);
    }

    for (ld = first_ld; ld; ld = ld_next)
    {
        ld_next = ld->next;
        if (!process_ldesc(ld))
        {
            debug_printf(10, "process_listener_connections: disconnected from listener\n");
            UNLINK(ld, first_ld, last_ld, next, prev );
            free(ld);
        }
    }
}

#ifndef MUD_LISTENER
bool ldwrite_to_descriptor( DESCRIPTOR_DATA *d, char *txt, int length )
#else
bool write_to_descriptor( DESCRIPTOR_DATA *d, char *txt, int length )
#endif
{
    char tbuf[1024*16];

    if (!first_ld)
        return TRUE;

    snprintf(tbuf, sizeof(tbuf), "%d %d %s", MESSAGE_SENDTOUSER, d->uid, txt );

    send_message_to_ldesc(first_ld, tbuf, strlen(tbuf));

    return TRUE;
}

void listener_close_socket(DESCRIPTOR_DATA *d)
{
    char tbuf[32];

    if (!first_ld)
        return;

    snprintf(tbuf, sizeof(tbuf), "%d %d %d", MESSAGE_USERDISCON, d->uid, MESSAGE_USERDISCON_CLOSESOCKET );

    send_message_to_ldesc(first_ld, tbuf, strlen(tbuf));
}

#if 0
void new_descriptor( int new_desc )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
    BAN_DATA *pban;
    struct sockaddr_in sock;
    struct hostent *from;
    int desc;
    unsigned int size;

    set_alarm( 20 );
    size = sizeof(sock);
    if ( check_bad_desc( new_desc ) )
    {
        set_alarm( 0 );
        return;
    }
    set_alarm( 20 );
    if ( ( desc = accept( new_desc, (struct sockaddr *) &sock, &size) ) < 0 )
    {
        perror( "New_descriptor: accept" );
        sprintf(log_buf, "New_descriptor: accept returned <0");
        log_string_plus( log_buf, LOG_COMM, sysdata.log_level, SEV_ERR );
        log_string_plus( log_buf, LOG_BUG, sysdata.log_level, SEV_NOTICE );
        set_alarm( 0 );
        return;
    }
    if ( check_bad_desc( new_desc ) )
    {
        set_alarm( 0 );
        return;
    }
#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

    set_alarm( 20 );
    if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 )
    {
        perror( "New_descriptor: fcntl: FNDELAY" );
        set_alarm( 0 );
        return;
    }
    if ( check_bad_desc( new_desc ) )
        return;
    CREATE( dnew, DESCRIPTOR_DATA, 1 );

    init_descriptor(dnew, desc );
    dnew->port = ntohs(sock.sin_port);


    strcpy( buf, inet_ntoa( sock.sin_addr ) );

    if ( sysdata.NO_NAME_RESOLVING )
    {
        dnew->host = STRALLOC( buf );
        sprintf( log_buf, "Sock.sinaddr: %s, port %d.",
                 dnew->host, dnew->port );
    }
    else
    {
        from = gethostbyaddr( (char *) &sock.sin_addr,
                              sizeof(sock.sin_addr), AF_INET );
        dnew->host = STRALLOC( (char *)( from ? from->h_name : buf) );
        sprintf( log_buf, "Sock.sinaddr: %s(%s), port %d.",
                 buf, dnew->host, dnew->port );
    }
    log_string_plus( log_buf, LOG_COMM, sysdata.log_level, SEV_NOTICE );

    for ( pban = first_ban; pban; pban = pban->next )
    {
        /* This used to use str_suffix, but in order to do bans by the
         first part of the ip, ie "ban 207.136.25" str_prefix must
         be used. -- Narn
         */
        if ( (!str_prefix( pban->name, dnew->host ) ||
              !str_suffix( pban->name, dnew->host )) &&
             pban->level >= LEVEL_SUPREME )
        {
            write_to_descriptor( dnew, "Your site has been banned from this MUD.\n\r", 0 );
            free_desc( dnew );
            set_alarm( 0 );
            return;
        }
    }

    /*
     * Init descriptor data.
     */

    if ( !last_descriptor && first_descriptor )
    {
        DESCRIPTOR_DATA *d;

        bug( "New_descriptor: last_desc is NULL, but first_desc is not! ...fixing" );
        for ( d = first_descriptor; d; d = d->next )
            if ( !d->next )
                last_descriptor = d;
    }

    LINK( dnew, first_descriptor, last_descriptor, next, prev );

#ifdef COMPRESS
    write_to_buffer(dnew, eor_on_str, 0);
    write_to_buffer(dnew, compress2_on_str, 0);
    write_to_buffer(dnew, compress_on_str, 0);
#endif

    write_to_buffer(dnew, eor_on_str, 0);
    write_to_buffer(dnew, mxp_on_str, 0);

    write_to_buffer(dnew, eor_on_str, 0);
    write_to_buffer(dnew, msp_on_str, 0);

    /*
     * Send the greeting.
     */
    {
        extern char * help_greeting;
        char filename[64];
        FILE *fp;
        char buf[MAX_STRING_LENGTH];
        int c;
        int num = 0;

        if ( help_greeting && *help_greeting )
        {
            if (help_greeting[0] == '.' )
                write_to_buffer( dnew, help_greeting+1, 0 );
            else
                write_to_buffer( dnew, help_greeting  , 0 );
        }

        sprintf(filename, "../system/login%d", number_range(1,6));
        if ( (fp = fopen( filename, "r" )) != NULL )
        {
            while ( !feof(fp) )
            {
                while ((buf[num]=fgetc(fp)) != EOF
                       &&      buf[num] != '\n'
                       &&      buf[num] != '\r'
                       &&      num < (MAX_STRING_LENGTH-2))
                    num++;
                c = fgetc(fp);
                if ( (c != '\n' && c != '\r') || c == buf[num] )
                    ungetc(c, fp);
                buf[num++] = '\n';
                buf[num++] = '\r';
                buf[num  ] = '\0';
                write_to_buffer( dnew, buf, num );
                num = 0;
            }
            FCLOSE(fp);
        }
    }

    write_to_buffer( dnew, "What is your name? ", 0);

    start_auth( dnew ); /* Start username authorization */

    if ( ++num_descriptors > sysdata.maxplayers )
        sysdata.maxplayers = num_descriptors;
    sysdata.total_logins++;
    if ( sysdata.maxplayers > sysdata.alltimemax )
    {
        if ( sysdata.time_of_max )
            DISPOSE(sysdata.time_of_max);
        sprintf(buf, "%24.24s", ctime(&current_time));
        sysdata.time_of_max = str_dup(buf);
        sysdata.alltimemax = sysdata.maxplayers;
        sprintf( log_buf, "Broke all-time maximum player record: %d", sysdata.alltimemax );
        log_string_plus( log_buf, LOG_COMM, sysdata.log_level, SEV_INFO );
        log_string_plus( log_buf,LOG_MONITOR, LEVEL_IMMORTAL, SEV_NOTICE );
    }
    if ( sysdata.longest_uptime < (current_time - boot_time) )
        sysdata.longest_uptime = current_time - boot_time;
    save_sysdata( sysdata );
    set_alarm(0);
    return;
}
#endif

