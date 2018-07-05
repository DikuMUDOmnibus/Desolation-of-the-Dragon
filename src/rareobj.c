/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

#include "rareobj.h"

RARE_OBJ_DATA *first_rare;
RARE_OBJ_DATA *last_rare;

void free_rare_obj(RARE_OBJ_DATA *rare)
{
    STRFREE(rare->name);
    STRFREE(rare->owner);

    UNLINK(rare, first_rare, last_rare, next, prev);
    DISPOSE(rare);
}

void free_rare_objs()
{
    RARE_OBJ_DATA *rare;

    while ((rare = first_rare))
	free_rare_obj(rare);
}

bool is_rare_obj(OBJ_DATA *obj)
{
    if (obj->rent >= MIN_OBJ_RENT)
        return TRUE;
    return FALSE;
}

int count_objs_for_reset(OBJ_INDEX_DATA *pObj)
{
    RARE_OBJ_DATA *rare;
    OBJ_DATA *obj;
    int count = 0;

    if (pObj->rent < MIN_OBJ_RENT)
	return pObj->count;

    for (rare = first_rare; rare; rare = rare->next)
        if (rare->vnum == pObj->ivnum)
	    count++;

    for (obj = first_object; obj; obj = obj->next)
    {
	if (obj_extracted(obj))
	    continue;
        if (obj->pIndexData == pObj)
	{
	    if (!obj->carried_by || 
	        (obj->carried_by &&
	         IS_NPC(obj->carried_by)))
		count++;
	}
    }

    log_printf_plus(LOG_DEBUG, LEVEL_LOG_CSET, SEV_TREET,
                    "cofr: rare obj: %s vnum: %d count: %d",
                    pObj->name, pObj->ivnum, count);

    return count;
}

RARE_OBJ_DATA *add_rare_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
    RARE_OBJ_DATA *rare;

    CREATE(rare, RARE_OBJ_DATA, 1);

    rare->owner        = STRALLOC(ch->name);
    rare->name         = STRALLOC(obj->name);
    rare->vnum         = obj->pIndexData->ivnum;
    rare->rent         = obj->rent;
    rare->time_added   = current_time;

    LINK(rare, first_rare, last_rare, next, prev);

    return rare;
}

void add_rare_obj_nest(CHAR_DATA *ch, OBJ_DATA *obj, sh_int iNest)
{
    if (iNest >= MAX_NEST)
    {
        bug( "add_rare_obj: iNest hit MAX_NEST %d", iNest );
        return;
    }

    if (obj->prev_content)
        add_rare_obj_nest(ch, obj->prev_content, iNest);

    if (is_rare_obj(obj))
        add_rare_obj(ch, obj);

    if (obj->first_content)
        add_rare_obj_nest(ch, obj->last_content, iNest + 1);
}


/* delete all rare objs owned by user */
bool delete_char_rare_obj(CHAR_DATA *ch)
{
    RARE_OBJ_DATA *rare, *rare_next;
    bool update = FALSE;

    if (IS_NPC(ch))
        return FALSE;

    for (rare = first_rare; rare; rare = rare_next)
    {
        rare_next = rare->next;

        if (str_cmp(rare->owner, ch->name))
            continue;

        free_rare_obj(rare);
        update = TRUE;
    }

    return update;
}

void update_char_rare_obj(CHAR_DATA *ch)
{
    bool update;

    update = delete_char_rare_obj(ch);

    /* add all rare objs owned by user */
    if (ch->first_carrying)
    {
        add_rare_obj_nest(ch, ch->last_carrying, 0);
        update = TRUE;
    }

    if (update)
        save_rare_objs();
}

void fwrite_rare(FILE *fp, RARE_OBJ_DATA *rare)
{
    fprintf(fp, "%d %d %ld\n%s~\n%s~\n",
            rare->vnum,
            rare->rent,
            (long int)rare->time_added,
            rare->owner,
            rare->name);
}

void save_rare_objs(void)
{
    RARE_OBJ_DATA *rare;
    FILE *fp;

    if (!(fp = fopen(SYSTEM_DIR "rareobj.dat", "w")))
    {
        bug( "save_rare_objs: unable to open rareobj.dat" );
        return;
    }

    for (rare = first_rare; rare; rare = rare->next)
        fwrite_rare(fp, rare);

    fprintf(fp, "$\n");

    FCLOSE(fp);
}

void load_rare_objs(void)
{
    RARE_OBJ_DATA *rare;
    FILE *fp;
    char *ln;

    if (!(fp = fopen(SYSTEM_DIR "rareobj.dat", "r")))
    {
        bug( "load_rare_objs: unable to open rareobj.dat" );
        return;
    }

    first_rare = last_rare = NULL;

    while (!feof(fp))
    {
        ln = fread_line(fp);

        if (*ln == '$')
            break;

        CREATE(rare, RARE_OBJ_DATA, 1);

        sscanf(ln, "%d %d %ld",
               &rare->vnum, &rare->rent, (long *)&rare->time_added);
        rare->owner = fread_string(fp);
        rare->name  = fread_string(fp);

        LINK(rare, first_rare, last_rare, next, prev);
    }

    FCLOSE(fp);
}

void do_rareobjs(CHAR_DATA *ch, char *argument)
{
    RARE_OBJ_DATA *rare;
    char arg[MAX_INPUT_LENGTH], *lastowner;
    int count = 0, x;

    if (!argument || !*argument)
    {
        send_to_char("Usage:\n\r  rareobjs summary\n\r  rareobjs list [char]\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg);

    if (!str_prefix(arg, "summary"))
    {
        OBJ_INDEX_DATA *pObj;

        pager_printf(ch, "%-6s %-7s %-25.25s\n\r",
                     "[Vnum]", "[Count]", "[Name]");
        for (x = 0; x < 100000; x++)
            for (rare = first_rare; rare; rare = rare->next)
            {
                if (rare->vnum != x)
                    continue;

                if (!(pObj = get_obj_index(rare->vnum)))
                    continue;

                pager_printf(ch, "%-6d %-7d %-25.25s\n\r",
                             rare->vnum,
                             count_objs_for_reset(pObj),
                             rare->name);
                break;
            }

        return;
    }

    lastowner = NULL;
    if (!str_prefix(arg, "list"))
    {
        pager_printf(ch, "%-15.15s%-6.6s %-6.6s %-25.25s %s\n\r",
                     "[Owner]", "[Vnum]", "[Rent]", "[Object]", "[Time]");
        for (rare = first_rare; rare; rare = rare->next)
        {
            if (argument && *argument && str_cmp(rare->owner, argument))
                continue;

            pager_printf(ch, "%-15.15s%-6d %-6d %-25.25s %s\r",
                         lastowner!=rare->owner?rare->owner:"",
                         rare->vnum,
                         rare->rent,
                         rare->name,
                         lastowner!=rare->owner?ctime( &rare->time_added ):"\n");
            lastowner = rare->owner;
            count++;
        }
        pager_printf(ch, "%d rare objects.\n\r", count);
        return;
    }

    do_rareobjs(ch, NULL);
}
