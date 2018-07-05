/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/
/*
 * recycle list code - Garil 8/14/99
 *
 * Instead of a ton of malloc() and free() calls, call them once and merely move the
 * unused structures to another list, then upon the next creation they merely need
 * to be moved and initialized, possibly prevents memory fragmentation, and calling
 * malloc once is probably faster, who knows?
 */
#include <string.h>

#include "mud.h"

CHAR_DATA *		first_recy_char;
CHAR_DATA *		last_recy_char;
OBJ_DATA *		first_recy_obj;
OBJ_DATA *		last_recy_obj;
int			mobs_in_recycle;
int			objs_in_recycle;
int                     mobs_unrecycled;
int                     mobs_recycled;
int                     objs_unrecycled;
int                     objs_recycled;
unsigned int            unum;

void *recy_char;
void *recy_obj;

void init_recycler(void)
{
    int x;

    log_string_plus("initializing recycler", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO);
    
    first_recy_char = NULL;
    last_recy_char = NULL;
    mobs_in_recycle = objs_in_recycle = 0;
    mobs_unrecycled = mobs_recycled = 0;
    objs_unrecycled = objs_recycled = 0;
    unum = 0;

    CREATE(recy_char, CHAR_DATA, 1000);
    for (x=0; x<1000; x++)
        LINK((CHAR_DATA *)recy_char+(sizeof(CHAR_DATA)*x), first_recy_char, last_recy_char, next, prev);
    mobs_in_recycle = 1000;

    CREATE(recy_obj, OBJ_DATA, 1000);
    for (x=0; x<1000; x++)
        LINK((OBJ_DATA *)recy_obj+(sizeof(OBJ_DATA)*x), first_recy_obj, last_recy_obj, next, prev);
    objs_in_recycle = 1000;
}

void free_recycled(void)
{
    DISPOSE(recy_char);
    DISPOSE(recy_obj);
}

void do_recstat(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *rch;
    OBJ_DATA *robj;
    ch_printf(ch,
              "mobs_in_recycle: %d\n\r"
              "objs_in_recycle: %d\n\r"
              "mobs_unrecycled: %d\n\r"
              "mobs_recycled  : %d\n\r"
              "objs_unrecycled: %d\n\r"
              "objs_recycled  : %d\n\r",
              mobs_in_recycle, objs_in_recycle,
              mobs_unrecycled, mobs_recycled,
              objs_unrecycled, objs_recycled);


    for (rch = first_recy_char; rch; rch=rch->next)
    {
	if (rch->name)
	    bug("RECYCLE: ch with name");
	if (rch->short_descr)
	    bug("RECYCLE: ch with short_descr");
	if (rch->description)
	    bug("RECYCLE: ch with description");
	if (rch->intro_descr)
	    bug("RECYCLE: ch with intro_descr");
    }
    for (robj = first_recy_obj; robj; robj=robj->next)
    {
	if (robj->name)
	    bug("RECYCLE: obj with name");
	if (robj->short_descr)
	    bug("RECYCLE: obj with short_descr");
	if (robj->description)
	    bug("RECYCLE: obj with description");
	if (robj->action_desc)
	    bug("RECYCLE: obj with action_desc");
    }
}

CHAR_DATA *unrecycle_char(void)
{
    CHAR_DATA *ch;

    if ((ch=first_recy_char))
    {
        UNLINK(ch, first_recy_char, last_recy_char, next, prev);
        mobs_in_recycle--;
        mobs_unrecycled++;
	if (ch->trust)
	    bug("unrecycle_char: trust %d", ch->trust);    
    }
    else
        CREATE(ch, CHAR_DATA, 1);
    ch->recycled = FALSE;
    ch->unum     = unum++;

    return ch;
}

void recycle_char(CHAR_DATA *ch)
{
    if (!ch)
        return;

    if (ch->recycled)
    {
        bug("ch already recycled");
        return;
    }
    
/*    UNLINK(ch, first_char, last_char, next, prev);*/
    LINK(ch, first_recy_char, last_recy_char, next, prev);
    free_char(ch);
    memset(ch, 0, sizeof(CHAR_DATA));
    ch->recycled = TRUE;
    mobs_in_recycle++;
    mobs_recycled++;
}

OBJ_DATA *unrecycle_obj(void)
{
    OBJ_DATA *obj;
    
    if ((obj=first_recy_obj))
    {
        UNLINK(obj, first_recy_obj, last_recy_obj, next, prev);
        objs_in_recycle--;
        objs_unrecycled++;
    }
    else
        CREATE(obj, OBJ_DATA, 1);
    obj->recycled = FALSE;
    obj->unum     = unum++;
    
    return obj;
}

void recycle_obj(OBJ_DATA *obj)
{
    if (!obj)
        return;

    if (obj->recycled)
    {
        bug("obj already recycled");
        return;
    }
    
/*    UNLINK(obj, first_obj, last_obj, next, prev);*/
    LINK(obj, first_recy_obj, last_recy_obj, next, prev);
/*    memset(obj, 0, sizeof(OBJ_DATA));*/
    obj->recycled = TRUE;
    objs_in_recycle++;
    objs_recycled++;
}
