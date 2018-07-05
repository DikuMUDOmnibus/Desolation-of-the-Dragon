/******************************************************
            Desolation of the Dragon MUD II
      (C) 2001-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/
#include "ldesc.h"

void debug_printf(int level, const char *format, ...)
{
#ifdef DEBUG
    char buf[8192];
    va_list va;

    if (level > DEBUG)
        return;

    va_start(va, format);
    vsnprintf(buf, sizeof(buf), format, va);
    va_end(va);

    fprintf(stderr, "%s", buf);
#endif
}

void init_ldesc(LDESC *ld)
{
    ld->descriptor = -1;
    ld->alloc_in = 512;
    ld->messages_in = (char *)malloc(ld->alloc_in);
    ld->alloc_out = 512;
    ld->messages_out = (char *)malloc(ld->alloc_out);
#ifndef LISTENER
    ld->next = ld->prev = NULL;
#endif
}

void disconnect_ldesc(LDESC *ld)
{
    close(ld->descriptor);
    ld->descriptor = -1;
    ld->len_in = 0;
    ld->len_out = 0;
#ifndef LISTENER
    free(ld->messages_in);
    free(ld->messages_out);
#endif
}

void process_message(LDESC *ld, char *data, int datalen)
{
    static char *tbuf = NULL;
    static int tbuf_len = 0;
    int msg, x, spaces=0;
    unsigned int uid;

    if (sscanf(data, "%d %d", &msg, &uid) != 2)
    {
        debug_printf(20, "process_message: unexpected format, ignoring message\n");
        return;
    }

    if (msg < MESSAGE_FIRST || msg > MESSAGE_LAST)
    {
        debug_printf(20, "process_message: message type out of range: %d\n", msg);
        return;
    }

    for (x=0;x<datalen && spaces<2;x++)
        if (data[x]==' ')
            spaces++;

    if (spaces!=2)
    {
        debug_printf(20, "process_message: unexpected format, ignoring message\n");
        return;
    }

    if (tbuf_len < datalen-x+1)
    {
        debug_printf(50, "process_message: increase tbuf from %d to %d\n", tbuf_len, datalen-x+1);
        tbuf_len = datalen-x+1;
        tbuf = (char *)realloc(tbuf, tbuf_len);
    }
    else
        tbuf_len = datalen-x+1;

    debug_printf(40, "process_message: copying %d bytes into tbuf, start: %d, total: %d\n", tbuf_len, x, datalen);

    memcpy(tbuf, (data+x), tbuf_len-1);
    tbuf[tbuf_len-1]='\0';

    if (recv_message_handlers[msg].process)
        recv_message_handlers[msg].process(ld, uid, tbuf);
}

void read_message_from_ldesc(LDESC *ld)
{
    int x=0, len=0;

    if (ld->len_in < 6) /* min length is 6 bytes */
        return;

    if (ld->messages_in[0] != MESSAGE_TAG)
    {
        for (x=0;x<ld->len_in;x++)
            if (ld->messages_in[x] == MESSAGE_TAG)
                break;
        if (ld->messages_in[x] == MESSAGE_TAG)
        {
            int y;

            ld->len_in -= (x+1);

            for (y=0;y<ld->len_in;y++)
                ld->messages_in[y] = ld->messages_in[y+x];
            debug_printf(30, "read_message_from_ldesc: %d bytes before MESSAGE_TAG found, length is now %d\n", x, ld->len_in);
        }
        else
        {
            ld->len_in = 0;
            debug_printf(30, "read_message_from_ldesc: %d bytes with no MESSAGE_TAG, length is now %d\n", x, ld->len_in);
        }
    }

    if (ld->len_in < 6)
        return;

    memcpy(&len, ld->messages_in+1, 4); /* +1 to skip MESSAGE_TAG */
    debug_printf(30, "read_message_from_ldesc: %d byte message found, length is %d\n", len, ld->len_in);

    if (len+5 > ld->len_in) /* don't have complete message yet */
        return;

    process_message(ld, ld->messages_in+5, len);

    ld->len_in -= (len+5);

    for (x=0;x<ld->len_in;x++)
        ld->messages_in[x] = ld->messages_in[x+len+5];

    debug_printf(30, "read_message_from_ldesc: message purged, length now %d\n", ld->len_in);
}

int send_message_to_ldesc(LDESC *ld, char *data, int datalen)
{
    if (datalen<1)
    {
        debug_printf(30, "send_message_to_ldesc: datalen too small: %d\n", datalen);
        return 1;
    }

    if (ld->len_out+5+datalen > ld->alloc_out)
    {
        debug_printf(50, "send_message_to_ldesc: realloc: from %d to %d\n", ld->alloc_out, datalen+5+ld->len_out);
        ld->messages_out = (char *)realloc(ld->messages_out, datalen+5+ld->len_out);
        ld->alloc_out = datalen+5+ld->len_out;
    }

    *(ld->messages_out+ld->len_out) = MESSAGE_TAG;
    ld->len_out+=1;

    memcpy(ld->messages_out+ld->len_out, &datalen, 4);
    ld->len_out+=4;

    memcpy(ld->messages_out+ld->len_out, data, datalen);
    ld->len_out+=datalen;

    debug_printf(30, "send_message_to_ldesc: queued %d bytes, %d/%d bytes in queue\n", datalen+5, ld->len_out, ld->alloc_out);
    return 0;
}

#define MAXREADSIZE 8192
int process_ldesc(LDESC *ld)
{
    static struct timeval null_time;
    fd_set fdset_in, fdset_out, fdset_exc;
    int bytes, desc;
    char buf[MAXREADSIZE];

    desc = ld->descriptor;

    FD_ZERO(&fdset_in);
    FD_ZERO(&fdset_out);
    FD_ZERO(&fdset_exc);
    FD_SET(desc, &fdset_in);
    FD_SET(desc, &fdset_out);
    FD_SET(desc, &fdset_exc);

    if (select(desc+1, &fdset_in, &fdset_out, &fdset_exc, &null_time) == -1)
    {
        perror("select");
        return 0;
    }
    if (FD_ISSET(desc, &fdset_exc))
    {
        disconnect_ldesc(ld);
        return 0;
    }
    if (FD_ISSET(desc, &fdset_in))
    {
        if ((bytes = recv(desc, buf, MAXREADSIZE, 0)) == -1 && errno != EWOULDBLOCK)
        {
            perror("recv");
            disconnect_ldesc(ld);
            return 0;
        }
        if (bytes == 0)
        {
            debug_printf(5, "process_ldesc: read 0 bytes, closing fd %d\n", desc);
            disconnect_ldesc(ld);
	    return 0;
        }
        if (bytes > 0)
        {
            if (bytes+ld->len_in > ld->alloc_in)
            {
                debug_printf(50, "process_ldesc: read: realloc: from %d to %d\n", ld->alloc_in, bytes+ld->len_in);
                ld->messages_in = (char *)realloc(ld->messages_in, bytes+ld->len_in);
                ld->alloc_in = bytes+ld->len_in;
            }
            memcpy(ld->messages_in+ld->len_in, buf, bytes);
            ld->len_in += bytes;
            debug_printf(30, "process_ldesc: read %d bytes, %d(%d) bytes in queue\n", bytes, ld->len_in, ld->alloc_in);
        }
    }
    read_message_from_ldesc(ld);
    if (FD_ISSET(desc, &fdset_out))
    {
        if (ld->len_out>0)
        {
            if ((bytes = write(desc, ld->messages_out, ld->len_out)) == -1)
            {
                perror("recv");
                disconnect_ldesc(ld);
                return 0;
            }
            if (bytes == 0)
            {
                debug_printf(5, "process_ldesc: write 0 bytes, closing fd %d\n", desc);
                disconnect_ldesc(ld);
                return 0;
            }
            if (bytes == ld->len_out)
                ld->len_out = 0;
            else if (bytes > 0)
            {
                int x;

                ld->len_out -= bytes;

                for (x=0;x<ld->len_out;x++)
                    ld->messages_out[x] = ld->messages_out[x+bytes];
            }
            if (bytes > 0)
                debug_printf(30, "process_ldesc: wrote %d bytes, %d(%d) bytes to write\n", bytes, ld->len_in, ld->alloc_out);
        }
    }
    return 1;
}
