/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer and Heath Leach
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*static char rcsid[] = "$Id: iimp.c,v 1.6 2002/10/12 20:06:10 dotd Exp $";*/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

void imp_who( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int count;
    char arg[MAX_INPUT_LENGTH];

    one_argument (argument, arg);
    count	= 0;
    buf[0]	= '\0';

    set_pager_color( AT_PLAIN, ch );
    sprintf(buf, "Player       |Trust|User@HostIP");
   strcat(buf, "\n\r");
   strcat(buf,"-------------+-----+-----------------------------------------");
   strcat(buf, "\n\r");
   send_to_pager(buf, ch);

    for ( d = first_descriptor; d; d = d->next )
    {
	if (  get_trust(ch) >= LEVEL_SUPREME
	||   (d->character && can_see( ch, d->character )) )
	{
	    count++;
	    sprintf( buf,
	     " %-12s|   %2d| %s@%-26s ",
                GET_NAME(d->character),
		get_trust(d->character),
                d->user, d->host);
	    strcat(buf, "\n\r");
	    send_to_pager( buf, ch );
	}
      }
    pager_printf( ch, "%d user%s.\n\r", count, count == 1 ? "" : "s" );
    return;
}
