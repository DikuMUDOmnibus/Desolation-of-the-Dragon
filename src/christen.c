/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/
#include <stdio.h>
#include <string.h>

#include "mud.h"
#include "christen.h"

CHRISTEN_DATA *first_christen;
CHRISTEN_DATA *last_christen;
int last_cvnum;

CHRISTEN_DATA *get_christen(int cvnum)
{
    CHRISTEN_DATA *cd;

    for (cd=first_christen;cd;cd=cd->next)
        if (cd->cvnum == cvnum)
            return cd;

    return NULL;
}

char *get_christen_name(OBJ_DATA *obj)
{
    if (!obj->christened)
        return "";

    return obj->christened->name;
}

CHRISTEN_DATA *new_christen(char *owner, int ovnum, char *cname, int cvnum, time_t when)
{
    CHRISTEN_DATA *cd;
    char *nobody = "nobody";
    char *nothing = "nothing";

    CREATE(cd, CHRISTEN_DATA, 1);

    cd->cvnum = cvnum;
    cd->ovnum = ovnum;
    cd->owner = STRALLOC(owner?owner:nobody);
    cd->name  = str_dup(cname?cname:nothing);
    cd->when  = when;

    LINK(cd, first_christen, last_christen, next, prev);

    return cd;
}

void free_christen(CHRISTEN_DATA *cd)
{
    DISPOSE(cd->name);
    STRFREE(cd->owner);

    UNLINK(cd, first_christen, last_christen, next, prev);
    DISPOSE(cd);
}

void free_christens(void)
{
    CHRISTEN_DATA *cd;

    while ((cd = first_christen))
        free_christen(cd);
}

void save_christens(void)
{
    FILE *fp;
    CHRISTEN_DATA *cd;

    if (!(fp = fopen(CHRISTEN_FILE, "w")))
    {
        bug("Unable to write christen data to " CHRISTEN_FILE);
        return;
    }

    for (cd=first_christen;cd;cd=cd->next)
    {
        fprintf(fp, "%d %d %ld\n%s~\n%s~\n",
                cd->cvnum, cd->ovnum, (long)cd->when,
                cd->owner, cd->name );
    }
    fprintf(fp, "$\n");

    fclose(fp);
}

void load_christens(void)
{
    FILE *fp;
    char *line, *owner, *cname;
    int cvnum, ovnum;
    time_t when;

    last_cvnum = 0;
    first_christen = NULL;
    last_christen = NULL;

    if (!(fp = fopen(CHRISTEN_FILE, "r")))
        return;

    while (!feof(fp))
    {
        line = fread_line(fp);

        if (!*line || *line == '$')
            break;

        sscanf( line, "%d %d %ld", &cvnum, &ovnum, (long *)&when);
        owner = fread_string(fp);
        cname = fread_string(fp);

        last_cvnum = UMAX(last_cvnum, cvnum);

        new_christen(owner, ovnum, cname, cvnum, when);
    }

    fclose(fp);
}


void do_christen(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
    int perc;

    if (IS_NPC(ch))
    {
        send_to_char("You cannot christen objects.\n\r", ch);
        return;
    }

    if (!argument || !*argument)
    {
        send_to_char("Christen what?\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg);

    if ( IS_IMMORTAL(ch) )
    {
        CHRISTEN_DATA *cd=NULL;
        int x=0;

        if ( !str_cmp(arg, "list") )
        {
            for (cd=first_christen;cd;cd=cd->next)
            {
                ch_printf(ch, "%-5d: Obj: %-5d  Owner: %s  Christened: %s\n",
                          cd->cvnum, cd->ovnum, cd->owner, cd->name);
                x++;
            }
            ch_printf(ch, "%d christens found.\n\r", x);
            return;
        }
        if ( !str_cmp(arg, "delete") )
        {
            if ((cd = get_christen(atoi(argument))))
            {
                for ( obj = first_object; obj; obj = obj->next )
                    if ( obj->christened == cd )
                    {
                        obj->christened = NULL;
                        x++;
                    }
                free_christen(cd);
                save_christens();
                ch_printf(ch, "Ok.  (Removed from %d in-game objects)\n\r", x);
                return;
            }

            send_to_char("Not found.\n\r", ch);
            return;
        }
        if ( !str_cmp(arg, "find") )
        {
            for ( obj = first_object; obj; obj = obj->next )
                if ( (cd = obj->christened) )
                {
                    ch_printf(ch, "%-5d: Obj: %-5d  Owner: %s  Christened: %s\n",
                              cd->cvnum, cd->ovnum, cd->owner, cd->name);
                    x++;
                }

            ch_printf(ch, "%d objects found.\n\r", x);
            return;
        }

    }

    if (!argument || !*argument)
    {
        send_to_char("Christen it what?\n\r", ch);
        return;
    }

    if (strlen(argument)>32 && !IS_IMMORTAL(ch))
    {
        send_to_char("That name is too long, try again.\n\r", ch);
        return;
    }

    if (!(obj = get_obj_carry(ch, arg)))
    {
        send_to_char("You don't have that.\n\r", ch);
        return;
    }

    if (obj->christened)
    {
        ch_printf(ch, "That object is already christened \"%s\".",
                  get_christen_name(obj));
        return;
    }

    obj->christened = new_christen(GET_NAME(ch), obj->vnum,
                                   argument, ++last_cvnum, current_time);
    save_christens();

    act(AT_YELLOW, "You hold $p up and loudly proclaim:",
        ch, obj, argument, TO_CHAR);
    act(AT_YELLOW, "$n holds $p up and loudly proclaims:",
        ch, obj, argument, TO_ROOM);

    perc = number_percent();

    if (perc<30)
        strcpy(buf, "'I christen thee, \"$T\"!'");
    else if (perc<50)
        strcpy(buf, "'Forever shall you be known as \"$T\"!'");
    else if (perc<70)
        strcpy(buf, "'From this day forward, you will be \"$T\"!'");
    else if (perc<90)
        strcpy(buf, "'You are now \"$T\", may your enemies tremble at your name!'");
    else
        strcpy(buf, "'$T!'");

    act(AT_YELLOW, buf, ch, obj, argument, TO_CHAR);
    act(AT_YELLOW, buf, ch, obj, argument, TO_ROOM);

    if (number_percent()>95)
    {
        sprintf(log_buf, "do_christen: %s received blessing for %s", GET_NAME(ch), argument);
        log_string_plus(log_buf,LOG_MONITOR,LEVEL_IMMORTAL,SEV_NOTICE);

        act(AT_MAGIC, "$T is struck by a bolt from heaven!", ch, obj, argument, TO_ROOM);
        act(AT_MAGIC, "$T is struck by a bolt from heaven!  You feel it's power grow!", ch, obj, argument, TO_ROOM);

        if ( obj->item_type == ITEM_WEAPON )
        {
            AFFECT_DATA *paf;
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type	= -1;
            paf->duration	= -1;
            paf->location	= APPLY_HITROLL;
            paf->modifier	= GetMaxLevel(ch)/25+1;
            paf->bitvector	= 0;
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
        }
        else if ( obj->item_type == ITEM_ARMOR )
        {
            AFFECT_DATA *paf;
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type	= -1;
            paf->duration	= -1;
            paf->location	= APPLY_AC;
            paf->modifier	= GetMaxLevel(ch)/10+1;
            paf->bitvector	= 0;
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
        }

    }

}
