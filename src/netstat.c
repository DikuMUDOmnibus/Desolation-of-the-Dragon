/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2003  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mud.h"

/* Netstat for SMAUG v1.01
 *
 * Works only on Linux because it needs /proc/net/tcp
 * If you don't have a sin_addr var in your descriptor,
 * then this won't work with name resolving turned on
 *
 * To add sin_addr:
 * In mud.h, in your descriptor_data structure, add:
 * struct in_addr sin_addr;
 * in comm.c, in new_descriptor add:
 * memcpy(&dnew->sin_addr, &sock.sin_addr, sizeof(dnew->sin_addr));
 * below the dnew->port = ntohs( sock.sin_addr ); line
 * also remove the #if DESCRIPTOR_HAS_SIN_ADDR lines below
 */

extern int port;
#define MUDPORT port

bool compare_ip(DESCRIPTOR_DATA *d, unsigned int ip)
{
    struct in_addr inp;

#if DESCRIPTOR_HAS_SIN_ADDR
    if (ip == d->sin_addr.s_addr)
        return TRUE;
#endif

    if (inet_aton(d->host, &inp) &&
        ip == inp.s_addr)
        return TRUE;

    return FALSE;
}

void do_netstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    FILE *fp;
    DESCRIPTOR_DATA *d;
    struct in_addr inp;
    int x, st, txq, rxq, tr, retrans;
    unsigned int localip, remoteip;
    unsigned short int localport, remoteport;
    unsigned int when;

    if (argument && !str_cmp(argument, "help"))
    {
        send_to_char("Name           - Players name\n\r"
                     "Remote Address - IP and Host of player\n\r"
                     "RecvQ          - Kernel TCP Receive Buffer\n\r"
                     "SendQ          - Kernel TCP Send Buffer \n\r"
                     "Timer          - TCP Retransmit Timer\n\r"
                     "Retrans        - Number of retransmits\n\r\n\r", ch);
        send_to_char("NOTES: When a player is experiencing network lag, you will likely see\n\r"
                     "       an increase in the SendQ, and non-zero values in Timer and Retrans.\n\r"
                     "       When Timer is non-zero, it will be that many seconds before the\n\r"
                     "       player will receive any data from the MUD.  If the value is very\n\r"
                     "       large (it may grow to over a minute) then the player is severly\n\r"
                     "       lagging and is unlikely to recover.  Non-zero values in Timer however\n\r"
                     "       are not necessarily a bad thing as TCP is designed this way and can\n\r"
                     "       recover.  You should probably never see RecvQ greater than zero.\n\r", ch);
        return;
    }

    if (!(fp = fopen("/proc/net/tcp", "r")))
    {
        send_to_char("Unable to open /proc/net/tcp\n\r", ch);
        return;
    }

    fgets(buf, MAX_INPUT_LENGTH-1, fp);

    ch_printf(ch, "%-20s %-21s %-5s %-5s %-5s %s\n\r",
              "Name", "Remote Address",
              "RecvQ", "SendQ",
              "Timer", "Retrans");

    while (fgets(buf, MAX_INPUT_LENGTH-1, fp))
    {
        sscanf(buf, "%d: %x:%hx %x:%hx %x %x:%x %d:%x %x",
               &x, &localip, &localport, &remoteip, &remoteport,
               &st, &txq, &rxq, &tr, &when, &retrans);

        for (d = first_descriptor; d; d = d->next)
        {
            if (st != 1 ||
                d->port != remoteport ||
                MUDPORT != localport ||
                !compare_ip(d, remoteip))
                continue;

            inp.s_addr = remoteip;

            sprintf(buf, "%s:%d", inet_ntoa(inp), remoteport);

            ch_printf(ch, "%-20s %-21s %-5d %-5d %-5.1f %-2d\n\r",
                      d->original  ? d->original->name  :
                      d->character ? d->character->name : "(none)",
                      buf,
                      rxq, txq,
                      when/100.0, retrans
                     );
        }
    }

    fclose(fp);
}
