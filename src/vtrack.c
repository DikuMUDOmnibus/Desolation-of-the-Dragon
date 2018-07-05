/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2003  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/


#include <stdio.h>
#include "mud.h"

extern int top_room;
extern int top_mob_index;
extern int top_obj_index;

static VTRACK_DATA *new_vtrack(int vnum)
{
    VTRACK_DATA *vt;

    CREATE(vt, VTRACK_DATA, 1);

    vt->vnum  = vnum;
    vt->flags = 0;
    vt->next  = NULL;

    return vt;
}

void free_vtracks(CHAR_DATA *ch)
{
    VTRACK_DATA *vt, *vt_next;

    if (IS_NPC(ch))
        return;

    vt = ch->pcdata->vtrack;

    while (vt)
    {
        vt_next = vt->next;
        DISPOSE(vt);
        vt = vt_next;
    }

    ch->pcdata->vtrack = NULL;
}

static VTRACK_DATA *find_vtrack_vnum(CHAR_DATA *ch, int vnum)
{
    VTRACK_DATA *vt;

    vt = ch->pcdata->vtrack;

    while (vt)
    {
        if (vt->vnum == vnum)
            return vt;
        vt = vt->next;
    }

    return NULL;
}

void vtrack_add(CHAR_DATA *ch, int vnum, int flags)
{
    VTRACK_DATA *vt;

    if (IS_NPC(ch) || vnum <= 0)
        return;

    if ((vt = find_vtrack_vnum(ch, vnum)))
    {
        SET_BIT(vt->flags, flags);
        return;
    }

    vt = new_vtrack(vnum);
    vt->flags = flags;

    vt->next = ch->pcdata->vtrack;
    ch->pcdata->vtrack = vt;
}

void fwrite_vtracks(CHAR_DATA *ch, FILE *fp)
{
    VTRACK_DATA *vt;

    if (!ch || IS_NPC(ch) || !ch->pcdata->vtrack)
        return;

    fprintf(fp, "#VTRACK\n");
    for (vt = ch->pcdata->vtrack; vt; vt = vt->next)
        fprintf(fp, "%d %d\n", vt->vnum, vt->flags);
    fprintf(fp, "0\n\n");
}

void fread_vtracks(CHAR_DATA *ch, FILE *fp)
{
    char *line;
    int x1, x2;

    while (!feof(fp))
    {
        line = fread_line( fp );

        x1=x2=0;
        sscanf( line, "%d %d",
                &x1, &x2 );
        if (x1 == 0)
            break;

        vtrack_add(ch, x1, x2);
    }

}

int vtrack_count(CHAR_DATA *ch, int flag)
{
    VTRACK_DATA *vt;
    int count = 0;

    if (IS_NPC(ch) || !ch->pcdata->vtrack)
        return 0;

    for (vt = ch->pcdata->vtrack; vt; vt = vt->next)
        if (IS_SET(vt->flags, flag))
            count++;

    return count;
}

float vtrack_percent(CHAR_DATA *ch, int flag)
{
    int total, count;

    switch (flag)
    {
    case VTRACK_ROOM:
        total = top_room;
        break;
    case VTRACK_OBJ:
        total = top_obj_index;
        break;
    case VTRACK_MOB:
    case VTRACK_MOBKILL:
        total = top_mob_index;
        break;
    default:
        bug("vtrack_percent: flag %d not handled by case", flag);
        return 0;
        break;
    }

    if (total == 0)
        return 0;

    count = vtrack_count(ch, flag);

    return ((float)count*100.0/(float)total);
}
