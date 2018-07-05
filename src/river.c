/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*static char rcsid[] = "$Id: river.c,v 1.7 2003/09/09 19:43:23 dotd Exp $";*/

#include <stdio.h>
#include <string.h>

#include "mud.h"

#define ENABLE_RIVERS

char buf[MAX_STRING_LENGTH];

int lowest_exit(ROOM_INDEX_DATA *rp)
{
  EXIT_DATA *ex;
  int min = MAX_ELEVATION+1;

  for (ex = rp->first_exit; ex; ex = ex->next)
    min = UMIN(min, ROOM_ELEVATION(ex->to_room));

  if (min < 0 || min == MAX_ELEVATION+1)
    return(-1);

  return(min);
}

EXIT_DATA *find_best_exit(ROOM_INDEX_DATA *rp)
{
    EXIT_DATA *ex;
    int min = -1;

    if (!rp || !rp->first_exit || (min=lowest_exit(rp))==-1)
	return(NULL);

    for (ex = rp->first_exit; ex; ex = ex->next)
	if (ROOM_ELEVATION(ex->to_room) == min &&
	    !IS_RIVER(ex->to_room))
	    return(ex);

    return(NULL);
}

void calc_river_path(ROOM_INDEX_DATA *rp, sh_int liq)
{
    EXIT_DATA *ex;
    char buf2[MAX_INPUT_LENGTH];

    if (!rp)
    {
	log_string_plus("!rp in calc_river_path:river.c", LOG_BUG, LEVEL_LOG_CSET, SEV_ERR);
	return;
    }

    sprintf(buf2, "%d ", rp->vnum);
    strcat(buf, buf2);

    if (!rp->river)
      CREATE(rp->river, RIVER_DATA, 1);
    RIVER_SPEED(rp) = 1;
    RIVER_DEPTH(rp) = 1;
    RIVER_LIQUID(rp) = liq;

    if ((ex = find_best_exit(rp))!=NULL)
    {
	RIVER_TO(rp) = ex;
	calc_river_path(ex->to_room, liq);
    }
    else
    {
	RIVER_DEPTH(rp) = UMAX(1, lowest_exit(rp) - ROOM_ELEVATION(rp));
	RIVER_SPEED(rp) = 0;
    }
}

char *river_room_desc(ROOM_INDEX_DATA *rp)
{
  char buf2[MAX_STRING_LENGTH];

#ifndef ENABLE_RIVERS
return(NULL);
#endif

  if (!IS_RIVER(rp))
    return(NULL);

  if (RIVER_SPEED(rp) < 0) {
    log_string_plus("-RIVER_SPEED in river_room_desc:river.c", LOG_BUG, LEVEL_LOG_CSET, SEV_ERR);
    return(NULL);
  }

  if (RIVER_SPEED(rp) == 0 || !RIVER_TO(rp))
    sprintf(buf2, "The %s is calm.\n\r",
		liq_table[RIVER_LIQUID(rp)].liq_name);
  else if (RIVER_SPEED(rp) >= 150)
    sprintf(buf2, "The %s falls in torents to the %s.\n\r",
		liq_table[RIVER_LIQUID(rp)].liq_name,
		exit_name((RIVER_TO(rp))));
  else if (RIVER_SPEED(rp) >= 75)
    sprintf(buf2, "The %s foams and froths as it flows to the %s.\n\r",
		liq_table[RIVER_LIQUID(rp)].liq_name,
		exit_name((RIVER_TO(rp))));
  else if (RIVER_SPEED(rp) >= 50)
    sprintf(buf2, "The %s is rushes at great speed to the %s.\n\r",
		liq_table[RIVER_LIQUID(rp)].liq_name,
		exit_name((RIVER_TO(rp))));
  else if (RIVER_SPEED(rp) >= 20)
    sprintf(buf2, "The %s is flowing rapidly to the %s.\n\r",
		liq_table[RIVER_LIQUID(rp)].liq_name,
		exit_name((RIVER_TO(rp))));
  else if (RIVER_SPEED(rp) >= 10)
    sprintf(buf2, "The %s is flowing smoothly to the %s.\n\r",
		liq_table[RIVER_LIQUID(rp)].liq_name,
		exit_name((RIVER_TO(rp))));
  else if (RIVER_SPEED(rp) >= 5)
    sprintf(buf2, "The %s is gently flowing to the %s.\n\r",
		liq_table[RIVER_LIQUID(rp)].liq_name,
		exit_name((RIVER_TO(rp))));
  else
    sprintf(buf2, "The %s quietly flows to the %s.\n\r",
		liq_table[RIVER_LIQUID(rp)].liq_name,
		exit_name((RIVER_TO(rp))));

  return(str_dup(buf2));
}
