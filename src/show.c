/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer and Heath Leach
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*static char rcsid[] = "$Id: show.c,v 1.65 2004/03/30 16:16:04 dotd Exp $";*/

#include <string.h>

#include "mud.h"

DECLARE_DO_FUN(do_ofind);
DECLARE_DO_FUN(do_mfind);
DECLARE_DO_FUN(do_rfind);
DECLARE_DO_FUN(do_bug);
DECLARE_DO_FUN(do_idea);
DECLARE_DO_FUN(do_typo);

extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];

extern struct timerset userec_violence;
extern struct timerset userec_area;
extern struct timerset userec_mobile;
extern struct timerset userec_spell;
extern struct timerset userec_aggr;
extern struct timerset userec_objact;
extern struct timerset userec_act;

extern int top_room_vnum;

void find_entrances( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *pRoomIndex, *dest_room;
    EXIT_DATA *pexit;
    int hash;
    bool found = FALSE;

    dest_room = ch->in_room;

    send_to_pager("Entrances into this room:\n\r", ch);

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pRoomIndex = room_index_hash[hash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
	{
            if ( pRoomIndex->tele_vnum == dest_room->vnum )
            {
		pager_printf(ch, " Teleport  from %-5d '%s'\n\r",
                             pRoomIndex->vnum,
                             pRoomIndex->name);
                found = TRUE;
            }
            for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
            {
                if ( pexit->to_room == dest_room )
                {
                    pager_printf(ch, " %-9.9s from %-5d '%s'\n\r",
				 exit_name(pexit), pRoomIndex->vnum,
				 pRoomIndex->name);
                    found = TRUE;
                }
            }
	}
    if (!found)
        send_to_pager("None.\n\r", ch);
    found = FALSE;

    send_to_pager("\n\rEntrances into this area:\n\r", ch);

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pRoomIndex = room_index_hash[hash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
	{
            if ( pRoomIndex->area == dest_room->area )
                continue;

            if ( pRoomIndex->tele_vnum == dest_room->vnum )
            {
                pager_printf(ch, " Teleport  from %-5d '%s'\n\r",
                             pRoomIndex->vnum,
                             pRoomIndex->name);
                found = TRUE;
            }
            for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
            {
                if ( pexit->to_room->area == dest_room->area )
                {
                    pager_printf(ch, " %-9.9s to %-5d from %-5d '%s'\n\r",
				 exit_name(pexit),
                                 pexit->to_room->vnum,
                                 pRoomIndex->vnum,
                                 pRoomIndex->name);
                    found = TRUE;
                }
            }
	}

    if (!found)
        send_to_pager("None.\n\r", ch);
}

void find_exits( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *pRoomIndex, *dest_room;
    EXIT_DATA *pexit;
    int hash;
    bool found = FALSE;

    dest_room = ch->in_room;

    send_to_pager("\n\rExits from this area:\n\r", ch);

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pRoomIndex = room_index_hash[hash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
            for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
            {
                if ( pRoomIndex->area == dest_room->area &&
                     pexit->to_room->area != dest_room->area )
                {
		    pager_printf(ch, " %-9.9s from %c%-5d to %c%-5d '%s'\n\r",
				 exit_name(pexit),
				 IS_ROOM_FLAG(pRoomIndex, ROOM_ORPHANED)?'O':' ',
                                 pRoomIndex->vnum,
				 IS_ROOM_FLAG(pexit->to_room, ROOM_ORPHANED)?'O':' ',
                                 pexit->to_room->vnum,
                                 pexit->to_room->name);
                    found = TRUE;
                }
            }

    if (!found)
        send_to_pager("None.\n\r", ch);
}

void find_unused_objects( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObjIndex;
    RESET_DATA *pReset;
    AREA_DATA *pArea = ch->in_room->area;
    bool found = FALSE;
    int hash, num=0;

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pObjIndex = obj_index_hash[hash];
              pObjIndex;
              pObjIndex = pObjIndex->next )
        {
            if (pObjIndex->area != pArea)
                continue;
            found = FALSE;
            for ( pReset = pArea->first_reset; pReset; pReset = pReset->next )
            {
                switch( pReset->command )
                {
                case 'P':
                    if (pReset->arg3 == pObjIndex->ivnum)
                    {
                        found = TRUE;
                        break;
                    }
                case 'G':
                case 'E':
                case 'O':
                    if (pReset->arg1 == pObjIndex->ivnum)
                    {
                        found = TRUE;
                        break;
                    }
                    break;
                default:
                    continue;
                    break;
                }
            }
            if (!found)
            {
                pager_printf( ch, "[%5d] %s\n\r",
                              pObjIndex->ivnum,
                              capitalize( pObjIndex->short_descr ) );
                num++;
            }
        }
    pager_printf( ch, "Number of matches: %d\n\r", num);
}

void find_unused_mobiles( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMobIndex;
    RESET_DATA *pReset;
    AREA_DATA *pArea = ch->in_room->area;
    bool found = FALSE;
    int hash, num=0;

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pMobIndex = mob_index_hash[hash];
              pMobIndex;
              pMobIndex = pMobIndex->next )
        {
            if (pMobIndex->area != pArea)
                continue;
            found = FALSE;
            for ( pReset = pArea->first_reset; pReset; pReset = pReset->next )
            {
                switch( pReset->command )
                {
                case 'M':
                    if (pReset->arg1 == pMobIndex->ivnum)
                    {
                        found = TRUE;
                        break;
                    }
                    break;
                default:
                    continue;
                    break;
                }
            }
            if (!found)
            {
                pager_printf( ch, "[%5d] %s\n\r",
                              pMobIndex->ivnum,
                              capitalize( pMobIndex->short_descr ) );
                num++;
            }
        }
    pager_printf( ch, "Number of matches: %d\n\r", num);

}

void find_door_for_key( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pexit;
    int keyvnum, num=0;
    int hash;

    if (argument[0] == '\0' || !is_number(argument))
	keyvnum = 0;
    else
	keyvnum = atoi(argument);

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pRoomIndex = room_index_hash[hash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
            for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
                if (keyvnum > 0 && pexit->key == keyvnum)
                {
                    pager_printf(ch, " %-9.9s room %-5d '%s'\n\r",
				 exit_name(pexit),
                                 pRoomIndex->vnum,
                                 pRoomIndex->name);
                    num++;
                }
                else if (keyvnum == 0 && pexit->key != 0 &&
                         pRoomIndex->area == ch->in_room->area)
                {
                    pager_printf(ch, " %-9.9s key %-5d room %-5d '%s'\n\r",
				 exit_name(pexit),
                                 pexit->key,
                                 pRoomIndex->vnum,
                                 pRoomIndex->name);
                    num++;
                }

    pager_printf( ch, "Number of matches: %d\n\r", num);
}


void find_hhf(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *temp_ch, *vch;

    vch = get_char_world(ch, argument);

    for ( temp_ch = first_char; temp_ch; temp_ch = temp_ch->next )
    {
        if (char_died(temp_ch) || !can_see(ch, temp_ch))
            continue;

        if (vch)
        {
            if (is_hating(temp_ch, vch))
                pager_printf( ch, "'%s' u%d hates\n\r", PERS(temp_ch, ch), temp_ch->unum);
            if (is_hunting(temp_ch, vch))
                pager_printf( ch, "'%s' u%d hunts\n\r", PERS(temp_ch, ch), temp_ch->unum);
            if (is_fearing(temp_ch, vch))
                pager_printf( ch, "'%s' u%d fears\n\r", PERS(temp_ch, ch), temp_ch->unum);
        }
        else
        {
            if (temp_ch->hating)
                pager_printf( ch, "'%s' u%d hates '%s' u%d\n\r",
                              PERS(temp_ch, ch), temp_ch->unum,
                              PERS(temp_ch->hating->who, ch), temp_ch->hating->who->unum);
            if (temp_ch->hunting)
                pager_printf( ch, "'%s' u%d hunts '%s' u%d\n\r",
                              PERS(temp_ch, ch), temp_ch->unum,
                              PERS(temp_ch->hunting->who, ch), temp_ch->hunting->who->unum);
            if (temp_ch->fearing)
                pager_printf( ch, "'%s' u%d fears '%s' u%d\n\r",
                              PERS(temp_ch, ch), temp_ch->unum,
                              PERS(temp_ch->fearing->who, ch), temp_ch->fearing->who->unum);
        }
    }

    send_to_pager("Ok.\n\r", ch);
}

void find_teleport(CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *pRoomIndex;
    TELEPORT_DATA *tele;
    int icnt=0, hash;
    bool areaonly;

    areaonly = !str_cmp(argument, "area")?TRUE:FALSE;

    for ( tele = first_teleport; tele; tele = tele->next )
        if (!areaonly || tele->room->area == ch->in_room->area)
        {
            icnt++;
            pager_printf(ch, "active timer [%3d] to [%5d] %s\n\r",
                         tele->timer, tele->room->vnum, tele->room->name );
        }

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pRoomIndex = room_index_hash[hash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
            if ((pRoomIndex->tele_vnum || pRoomIndex->tele_delay) &&
                (!areaonly || pRoomIndex->area == ch->in_room->area))
            {
                icnt++;
                pager_printf(ch, "dormat timer [%3d] to [%5d] from [%5d] %s\n\r",
                             pRoomIndex->tele_delay, pRoomIndex->tele_vnum,
                             pRoomIndex->vnum, pRoomIndex->name );
            }

    pager_printf(ch, "%d teleport matches found.\n\r", icnt);
}

void find_currency_assigned(CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *pRoomIndex;
    int icnt=0, hash;
    bool areaonly;

    areaonly = !str_cmp(argument, "area")?TRUE:FALSE;

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pRoomIndex = room_index_hash[hash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
            if (pRoomIndex->currvnum &&
                (!areaonly || pRoomIndex->area == ch->in_room->area))
            {
                pager_printf(ch, "Room vnum: %-5d  Currency vnum: %-5d  Room: %s\n\r",
                             pRoomIndex->vnum,
                             pRoomIndex->currvnum,
                             pRoomIndex->name);
                icnt++;
            }

    pager_printf(ch, "%d currency matches found.\n\r", icnt);
}

void find_objects_rent(CHAR_DATA *ch, char *argument)
{
    OBJ_INDEX_DATA *pObjIndex;
    int icnt=0, hash;
    bool areaonly;

    areaonly = !str_cmp(argument, "area")?TRUE:FALSE;

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next )
            if ((pObjIndex->rent > MIN_OBJ_RENT || IS_OBJ_STAT2(pObjIndex, ITEM2_RENT)) &&
                (!areaonly || pObjIndex->area == ch->in_room->area))
            {
                pager_printf(ch, "Vnum: %-5d  %cRent: %-9d  Name: %s\n\r",
                             pObjIndex->ivnum,
                             IS_OBJ_STAT2(pObjIndex, ITEM2_RENT)?'*':' ',
                             pObjIndex->rent,
                             pObjIndex->name);
                icnt++;
            }

    pager_printf(ch, "%d rent matches found.\n\r", icnt);
}


void find_portals(CHAR_DATA *ch)
{
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pexit;
    int icnt=0, hash;

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pRoomIndex = room_index_hash[hash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
            for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
            {
                if ( pexit->vdir == DIR_SOMEWHERE )
                {
                    pager_printf(ch, "from %-5d to %-5d '%s'\n\r",
                                 pRoomIndex->vnum,
                                 pexit->to_room->vnum,
                                 pexit->to_room->name);
                    icnt++;
                }
            }
    pager_printf(ch, "%d portal matches found.\n\r", icnt);
}

void find_bigbucks(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *temp_ch;
    int min, icnt=0, x;

    min = atoi(argument);

    if (min <= 0)
        min = 10000;


    for ( temp_ch = first_char; temp_ch; temp_ch = temp_ch->next )
    {
        if (char_died(temp_ch) || !can_see(ch, temp_ch))
            continue;

	for (x=0;x<MAX_CURR_TYPE;x++)
	{
            if (GET_MONEY(temp_ch, x)>=min)
            {
                pager_printf(ch, "%6d %-10.10s '%s'\n\r",
                             GET_MONEY(temp_ch, x),
                             curr_types[x],
                             PERS(temp_ch, ch));
                icnt++;
	    }
            if (GET_BALANCE(temp_ch, x)>=min)
            {
                pager_printf(ch, "%6d %-10.10s (bank) '%s'\n\r",
                             GET_BALANCE(temp_ch, x),
                             curr_types[x],
                             PERS(temp_ch, ch));
                icnt++;
	    }
	}
    }
    pager_printf(ch, "%d bigbucks matches found.\n\r", icnt);
}

void find_quest_obj(CHAR_DATA *ch)
{
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    char buf[MAX_INPUT_LENGTH];
    int icnt=0;
    int hi_vnum, lo_vnum;

    lo_vnum = 50000;
    hi_vnum = 50099;

    for ( obj = first_object; obj; obj = obj->next )
    {
        if (obj_extracted(obj) ||
            obj->vnum < lo_vnum ||
            obj->vnum > hi_vnum)
            continue;

        sprintf(buf, "%3d) u%-6d [%5d] %-24s in ",
                ++icnt, obj->unum, obj->vnum, obj_short(obj));
        if ( obj->carried_by )
            sprintf(buf+strlen(buf), "invent [%5d] %s\n\r",
                    (IS_NPC(obj->carried_by) ? obj->carried_by->vnum
                     : 0), PERS(obj->carried_by, ch));
        else if ( obj->in_room )
            sprintf(buf+strlen(buf), "room   [%5d] %s\n\r",
                    obj->in_room->vnum, obj->in_room->name);
        else if ( obj->in_obj )
            sprintf(buf+strlen(buf), "object [%5d] %s\n\r",
                    obj->in_obj->vnum, obj_short(obj->in_obj));
        else
        {
            bug("find_quest_obj: object doesnt have location!");
            sprintf(buf+strlen(buf), "nowhere??\n\r");
        }
        send_to_pager(buf, ch);
    }

    for ( victim = first_char; victim; victim = victim->next )
    {
        if (char_died(victim) ||
            !IS_NPC(victim) ||
            victim->vnum < lo_vnum ||
            victim->vnum > hi_vnum)
            continue;
        pager_printf( ch, "%3d) u%-6d [%5d] %-24s [%5d] %s\n\r",
                      ++icnt,
                      victim->unum,
                      victim->vnum,
                      PERS(victim, ch),
                      victim->in_room->vnum,
                      victim->in_room->name );
    }
    pager_printf(ch, "%d quest obj matches found.\n\r", icnt);
}

void find_object_spell(CHAR_DATA *ch, char *argument)
{
    OBJ_INDEX_DATA *pObjIndex;
    AFFECT_DATA *aff;
    char *otype;
    int icnt=0, hash, sn;
    bool found=FALSE;

    if ((sn = skill_lookup(argument)) < 0)
    {
        send_to_char("Invalid spell.\n\r", ch);
        return;
    }

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next )
        {
            found = FALSE;

            switch(pObjIndex->item_type)
            {
            case ITEM_POTION:
            case ITEM_PILL:
            case ITEM_SCROLL:
                if (pObjIndex->value[1] == sn ||
                    pObjIndex->value[2] == sn ||
                    pObjIndex->value[3] == sn)
                    found = TRUE;
                break;
            case ITEM_SALVE:
                if (pObjIndex->value[4] == sn &&
                    pObjIndex->value[5] == sn)
                    found = TRUE;
                break;
            case ITEM_STAFF:
                if (pObjIndex->value[3] == sn)
                    found = TRUE;
                break;
            }

            if (!found)
                for ( aff = pObjIndex->first_affect; aff; aff = aff->next )
                    if ( (aff->location == APPLY_EAT_SPELL ||
                          aff->location == APPLY_WEAPONSPELL ||
                          aff->location == APPLY_WEARSPELL ||
                          aff->location == APPLY_REMOVESPELL) &&
                         aff->modifier == sn )
                    {
                        found = TRUE;
                        break;
                    }

            if (!found)
                continue;

            if ( pObjIndex->item_type >= 1 && pObjIndex->item_type <= MAX_ITEM_TYPE )
            {
                otype = o_types[pObjIndex->item_type];
            }
            else
            {
                bug( "object vnum: %d. unknown type %d.",
                     pObjIndex->ivnum, pObjIndex->item_type );
                otype = "(unknown)";
            }


            pager_printf(ch, "Vnum: %-5d  Type: %-10.10s  Name: %s\n\r",
                         pObjIndex->ivnum,
                         otype,
                         pObjIndex->name);
            icnt++;
        }

    pager_printf(ch, "%d object spell matches found.\n\r", icnt);
}

void find_stuff_noarea(CHAR_DATA *ch)
{
    ROOM_INDEX_DATA *pRoomIndex;
    MOB_INDEX_DATA *pMobIndex;
    OBJ_INDEX_DATA *pObjIndex;
    int hash, icnt=0;

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pRoomIndex = room_index_hash[hash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
            if (!pRoomIndex->area)
            {
                pager_printf(ch, "R Vnum: %-5d  Name: %s\n\r",
                             pRoomIndex->vnum,
                             pRoomIndex->name);
                icnt++;
            }

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next )
            if (!pObjIndex->area)
            {
                pager_printf(ch, "O Vnum: %-5d  Name: %s\n\r",
                             pObjIndex->ivnum,
                             pObjIndex->name);
                icnt++;
            }

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pMobIndex = mob_index_hash[hash]; pMobIndex; pMobIndex = pMobIndex->next )
            if (!pMobIndex->area)
            {
                pager_printf(ch, "M Vnum: %-5d  Name: %s\n\r",
                             pMobIndex->ivnum,
                             pMobIndex->short_descr);
                icnt++;
            }

    pager_printf(ch, "%d noarea matches found.\n\r", icnt);
}

void find_stuff_nowhere(CHAR_DATA *ch)
{
    CHAR_DATA *mob;
    OBJ_DATA *obj;
    int icnt=0;

    for ( obj = first_object; obj; obj = obj->next )
    {
        if (!obj->carried_by &&
            !obj->in_room &&
            !obj->in_obj)
	{
        pager_printf(ch, "%c u%-5d Vnum: %-5d  Name: %s\n\r",
                     obj_extracted(obj)?'X':'O',
                     obj->unum,
                     obj->vnum,
                     obj->name?obj->name:"(null)");
        icnt++;
	}
    }

    for ( mob = first_char; mob; mob = mob->next )
    {
        if (char_died(ch) ||
            ch->in_room)
            continue;

        pager_printf(ch, "M u%-5d Vnum: %-5d  Name: %s\n\r",
                     mob->unum,
                     mob->vnum,
                     mob->short_descr);
        icnt++;
    }

    pager_printf(ch, "%d nowhere matches found.\n\r", icnt);
}

void show_update_timers(CHAR_DATA *ch)
{
    send_to_char("Violence update:\n\r", ch);
    send_timer(&userec_violence, ch);
    send_to_char("Area update:\n\r", ch);
    send_timer(&userec_area, ch);
    send_to_char("Mobile update:\n\r", ch);
    send_timer(&userec_mobile, ch);
    send_to_char("Spell update:\n\r", ch);
    send_timer(&userec_spell, ch);
    send_to_char("Aggr update:\n\r", ch);
    send_timer(&userec_aggr, ch);
    send_to_char("Obj Act update:\n\r", ch);
    send_timer(&userec_objact, ch);
    send_to_char("Act update:\n\r", ch);
    send_timer(&userec_act, ch);
}

void show_log_stats(CHAR_DATA *ch)
{
    int x;

    ch_printf(ch, "%-12.12s %-3s %s\n\r",
              "Log Name", "Lev", "Number of Messages");
    for (x=LOG_NORMAL;x<LOG_LAST;x++)
        ch_printf(ch, "%-12.12s %-3d %d\n\r",
                  sysdata.logdefs[x].name,
                  sysdata.logdefs[x].level,
                  sysdata.logdefs[x].num_logs);
}

void show_disconnected(CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *pRoomIndex, *dest_room;
    EXIT_DATA *pexit;
    int ohash, ihash, icnt=0;
    bool found, areaonly=TRUE;

    /* this is so expensive, don't let people lag the mud with it */
    if (!str_cmp(argument, "all") && !str_cmp(GET_NAME(ch), "Garil"))
        areaonly = FALSE;

    for ( ohash = 0; ohash < MAX_KEY_HASH; ohash++ )
        for ( dest_room = room_index_hash[ohash]; dest_room; dest_room = dest_room->next )
        {
            if (areaonly && dest_room->area != ch->in_room->area)
                continue;

            found = FALSE;

            if ( dest_room->first_exit || dest_room->tele_vnum )
                found = TRUE;
            else
            {
                for ( ihash = 0; ihash < MAX_KEY_HASH; ihash++ )
                {
                    for ( pRoomIndex = room_index_hash[ihash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
                    {
                        if ( pRoomIndex->tele_vnum == dest_room->vnum )
                        {
                            found = TRUE;
                            break;
                        }

                        for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
                        {
                            if ( pexit->to_room == dest_room )
                            {
                                found = TRUE;
                                break;
                            }
                        }

                        if (found)
                            break;

                    }
                    if (found)
                        break;
                }
            }

            if (!found)
            {
                pager_printf(ch, "R Vnum: %-5d  Name: %s\n\r",
                             dest_room->vnum,
                             dest_room->name);
                icnt++;
            }
        }
    pager_printf(ch, "%d disconnected matches found.\n\r", icnt);
}

void do_show_timers(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *temp_ch, *vch;
    TIMER *timer;
    int icnt = 0;

    vch = get_char_world(ch, argument);

    pager_printf(ch, "%-30.30s %-7s %-6s %s\n\r",
              "[Name]", "[Count]", "[Type]", "[Value]");
    for ( temp_ch = first_char; temp_ch; temp_ch = temp_ch->next )
    {
        if (vch && vch != temp_ch)
            continue;

        if (!vch && IS_NPC(temp_ch))
            continue;

        if (!temp_ch->first_timer)
            continue;

        for (timer = temp_ch->first_timer; timer; timer = timer->next)
        {
            pager_printf(ch, "%-30.30s %-7d %-6d %d\n\r",
                         GET_NAME(temp_ch),
                         timer->count,
                         timer->type,
                         timer->value
                        );
            icnt++;
        }
    }
    pager_printf(ch, "%d timers matches found.\n\r", icnt);
}

void do_show_progs(CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *pRoomIndex;
    MOB_INDEX_DATA *pMobIndex;
    OBJ_INDEX_DATA *pObjIndex;
    int hash, icnt=0;

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pRoomIndex = room_index_hash[hash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
            if (pRoomIndex->mudprogs)
            {
                pager_printf(ch, "R Vnum: %-5d  Name: %s\n\r",
                             pRoomIndex->vnum,
                             pRoomIndex->name);
                icnt++;
            }

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next )
            if (pObjIndex->mudprogs)
            {
                pager_printf(ch, "O Vnum: %-5d  Name: %s\n\r",
                             pObjIndex->ivnum,
                             pObjIndex->name);
                icnt++;
            }

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pMobIndex = mob_index_hash[hash]; pMobIndex; pMobIndex = pMobIndex->next )
            if (pMobIndex->mudprogs)
            {
                pager_printf(ch, "M Vnum: %-5d  Name: %s\n\r",
                             pMobIndex->ivnum,
                             pMobIndex->short_descr);
                icnt++;
            }

    pager_printf(ch, "%d progs matches found.\n\r", icnt);
}

void do_show_guildmasters(CHAR_DATA *ch, char *argument)
{
    MOB_INDEX_DATA *pMobIndex;
    int hash, icnt = 0, cl, findcl = CLASS_NONE;

    if (argument && *argument != '\0')
        if ((findcl = get_classtype(argument)) == CLASS_NONE)
        {
            send_to_char("Invalid class.\n\r", ch);
            return;
        }
    cl = findcl;

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pMobIndex = mob_index_hash[hash]; pMobIndex; pMobIndex = pMobIndex->next )
            if (IS_ACT_FLAG(pMobIndex, ACT_TEACHER) ||
                IS_ACT_FLAG(pMobIndex, ACT_PRACTICE))
            {
                if (findcl != CLASS_NONE)
                {
                    if (!IS_ACTIVE(pMobIndex, findcl))
                        continue;
                    cl = findcl;
                }
                else
                {
                    for (cl=0; cl<MAX_CLASS; cl++)
                        if (IS_ACTIVE(pMobIndex, cl))
                            break;
                }

                pager_printf(ch, "%c Vnum: %-5d  Level: %2d/%2s  Name: %s\n\r",
                             IS_ACT_FLAG(pMobIndex, ACT_TEACHER)?'T':'P',
                             pMobIndex->ivnum,
                             GET_LEVEL(pMobIndex, cl),
                             short_pc_class[cl],
                             pMobIndex->player_name);
                icnt++;
            }

    pager_printf(ch, "%d guildmasters found.\n\r", icnt);
}

void do_find_mobile_reset( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *mob;
    ROOM_INDEX_DATA *room;
    RESET_DATA *pReset;
    AREA_DATA *pArea;
    char *mname, *rname;
    int hash, vnum=0, num=0;

    if (!argument || argument[0] == '\0')
    {
        send_to_char("Find which mob's resets?\n\r", ch);
        return;
    }

    if (is_number(argument))
        vnum = atoi(argument);
    else
    {
        for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        {
            for ( mob = mob_index_hash[hash];
                  mob;
                  mob = mob->next )
                if ( nifty_is_name( argument, mob->player_name ) )
                {
                    vnum = mob->ivnum;
                    break;
                }
            if (vnum)
                break;
        }
    }

    if (vnum <= 0)
    {
        send_to_char("No such mobile exists.\n\r", ch);
        return;
    }

    for (pArea = first_asort; pArea; pArea = pArea->next_sort)
        for ( pReset = pArea->first_reset; pReset; pReset = pReset->next )
            if (pReset->command == 'M' && pReset->arg1 == vnum)
            {
                if ( !(mob = get_mob_index(pReset->arg1)) )
                    mname = "Mobile: *BAD VNUM*";
                else
                    mname = mob->player_name;
                if ( !(room = get_room_index(pReset->arg3)) )
                    rname = "Room: *BAD VNUM*";
                else
                    rname = room->name;
                pager_printf( ch, "%s (%d) -> %s (%d) [%d]\n\r",
                              mname, pReset->arg1, rname,
                              pReset->arg3, pReset->arg2 );
                num++;
            }

    pager_printf( ch, "Number of matches: %d\n\r", num);
}

void do_find_object_reset( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *obj, *obj2 = NULL, *lastobj = NULL;
    MOB_INDEX_DATA *mob = NULL;
    ROOM_INDEX_DATA *room = NULL;
    RESET_DATA *pReset, *lo_reset = NULL;
    AREA_DATA *pArea;
    char *mname, *rname, *oname;
    int hash, vnum=0, num=0;

    if (!argument || argument[0] == '\0')
    {
        send_to_char("Find which object's resets?\n\r", ch);
        return;
    }

    if (is_number(argument))
        vnum = atoi(argument);
    else
    {
        for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        {
            for ( obj = obj_index_hash[hash];
                  obj;
                  obj = obj->next )
                if ( nifty_is_name( argument, obj->name ) )
                {
                    vnum = obj->ivnum;
                    break;
                }
            if (vnum)
                break;
        }
    }

    if (vnum <= 0)
    {
        send_to_char("No such object exists.\n\r", ch);
        return;
    }

    for (pArea = first_asort; pArea; pArea = pArea->next_sort)
        for ( pReset = pArea->first_reset; pReset; pReset = pReset->next )
        {
            if (pReset->command == 'M')
                mob = get_mob_index(pReset->arg1);

            if (pReset->arg1 != vnum)
                continue;

            if (pReset->command == 'E' ||
                pReset->command == 'G')
            {
                if ( !mob )
                    mname = "* ERROR: NO MOBILE! *";
                else
                    mname = mob->player_name;
                if ( !(obj = get_obj_index(pReset->arg1)) )
                    oname = "Object: *BAD VNUM*";
                else
                    oname = obj->name;
                pager_printf( ch, "%s (%d) -> %s (%s) [%d]\n\r", oname, pReset->arg1, mname,
                              (pReset->command == 'G' ? "carry" : wear_locs[pReset->arg3]),
                              pReset->arg2 );
                lastobj = obj;
                lo_reset = pReset;
                num++;
                continue;
            }
            if (pReset->command == 'O')
            {
                if ( !(obj = get_obj_index(pReset->arg1)) )
                    oname = "Object: *BAD VNUM*";
                else
                    oname = obj->name;
                if ( !(room = get_room_index(pReset->arg3)) )
                    rname = "Room: *BAD VNUM*";
                else
                    rname = room->name;
                pager_printf( ch, "(object) %s (%d) -> %s (%d) [%d]\n\r", oname,
                              pReset->arg1, rname, pReset->arg3, pReset->arg2 );
                if ( !room )
                    obj = NULL;
                lastobj = obj;
                lo_reset = pReset;
                num++;
                continue;
            }
            if (pReset->command == 'P')
            {
                if ( !(obj = get_obj_index(pReset->arg1)) )
                    oname = "Object1: *BAD VNUM*";
                else
                    oname = obj->name;
                obj2 = NULL;
                if ( pReset->arg3 > 0 )
                {
		    obj2 = get_obj_index(pReset->arg3);
                    if (obj2)
			rname = obj2->name;
		    else
			rname = "Object2: *BAD VNUM*";
                    lastobj = obj2;
                }
                else if ( !lastobj )
                    rname = "Object2: *NULL obj*";
                else if ( pReset->extra == 0 )
                {
                    rname = lastobj->name;
                    obj2 = lastobj;
                }
                else
                {
                    int iNest;
                    RESET_DATA *reset;

                    reset = lo_reset->next;
                    for ( iNest = 0; iNest < pReset->extra; iNest++ )
                    {
                        for ( ; reset; reset = reset->next )
                            if ( reset->command == 'O' || reset->command == 'G' ||
                                 reset->command == 'E' || (reset->command == 'P' &&
                                                           !reset->arg3 && reset->extra == iNest) )
                                break;
                        if ( !reset || reset->command != 'P' )
                            break;
                    }
                    if ( !reset )
                        rname = "Object2: *BAD NESTING*";
                    else if ( !(obj2 = get_obj_index(reset->arg1)) )
                        rname = "Object2: *NESTED BAD VNUM*";
                    else
                        rname = obj2->name;
                }
                pager_printf( ch, "(Put) %s (%d) -> %s (%d) [%d] {nest %d}\n\r", oname,
                              pReset->arg1, rname, (obj2 ? obj2->ivnum : pReset->arg3),
                              pReset->arg2, pReset->extra );

            }
        }

    pager_printf( ch, "Number of matches: %d\n\r", num);
}

void do_find_mobs_with_saves( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMobIndex;
    int hash, num=0;

    send_to_pager( "[Vnum ] [Psn Wnd Par Bre Spl] [Name]\n\r", ch );

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pMobIndex = mob_index_hash[hash];
              pMobIndex;
              pMobIndex = pMobIndex->next )
        {
            if (!IS_SET(pMobIndex->act, ACT_CUSTOMSAVES))
                continue;

            pager_printf( ch, "[%5d] [%3d %3d %3d %3d %3d] %s\n\r",
                          pMobIndex->ivnum,
                          pMobIndex->saving_poison_death,
                          pMobIndex->saving_wand,
                          pMobIndex->saving_para_petri,
                          pMobIndex->saving_breath,
                          pMobIndex->saving_spell_staff,
                          capitalize( pMobIndex->short_descr ) );
            num++;
        }
    pager_printf( ch, "Number of matches: %d\n\r", num);
}

void find_object_affect(CHAR_DATA *ch, char *argument)
{
    OBJ_INDEX_DATA *pObjIndex;
    AFFECT_DATA *aff;
    int icnt=0, hash, loc;

    if ((loc = get_atype(argument)) < 1)
    {
        send_to_char("Invalid affect.\n\r", ch);
        return;
    }

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next )
        {
            for ( aff = pObjIndex->first_affect; aff; aff = aff->next )
            {
                if ( aff->location != loc )
                    continue;

                pager_printf(ch, "Vnum: %-5d  Mod: %-9d  Name: %s\n\r",
                             pObjIndex->ivnum,
                             aff->modifier,
                             pObjIndex->name);
                icnt++;
            }
        }

    pager_printf(ch, "%d object affect matches found.\n\r", icnt);
}

void do_find_mobile_bit13( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA *pMobIndex;
    int hash, num=0;

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pMobIndex = mob_index_hash[hash];
              pMobIndex;
              pMobIndex = pMobIndex->next )
        {
            if (!IS_SET(pMobIndex->act, BV13))
                continue;

            pager_printf(ch, "%-6d] %s\n\r",
                         pMobIndex->ivnum,
                         pMobIndex->short_descr);
            num++;
        }
    pager_printf( ch, "Number of matches: %d\n\r", num);
}

#define COUNT_ARRAY_SIZE 128

void show_obj_counts(CHAR_DATA *ch, char *argument)
{
    OBJ_INDEX_DATA *obj_arr[COUNT_ARRAY_SIZE], *pObjIndex;
    int max = INT_MAX, index = -1, hash;
    int x, y;

    if (is_number(argument))
        max = atoi(argument);

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next )
        {
            if (pObjIndex->count > max)
                continue;

            if (index == -1)
            {
                obj_arr[0] = pObjIndex;
                index++;
                continue;
            }

            if (pObjIndex->count > obj_arr[index]->count)
            {
                index = UMIN(index+1, COUNT_ARRAY_SIZE-1);
                for (x = 0; x <= index; x++)
                    if (pObjIndex->count > obj_arr[x]->count)
                    {
                        for (y = index; y > x; y--)
                            obj_arr[y] = obj_arr[y-1];
                        obj_arr[x] = pObjIndex;
                        break;
                    }
            }
        }

    send_to_pager("[#]  [Cnt] [Vnum] [Name]\n\r", ch);
    for (x = 0; x <= index; x++)
        pager_printf(ch, "%3d. %-5d %-6d %s\n\r",
                     x+1,
                     obj_arr[x]->count,
                     obj_arr[x]->ivnum,
                     obj_arr[x]->name);
}

void show_mob_counts(CHAR_DATA *ch, char *argument)
{
    MOB_INDEX_DATA *mob_arr[COUNT_ARRAY_SIZE], *pMobIndex;
    int max = INT_MAX, index = -1, hash;
    int x, y;

    if (is_number(argument))
        max = atoi(argument);

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pMobIndex = mob_index_hash[hash]; pMobIndex; pMobIndex = pMobIndex->next )
        {
            if (pMobIndex->count > max)
                continue;

            if (index == -1)
            {
                mob_arr[0] = pMobIndex;
                index++;
                continue;
            }

            if (pMobIndex->count > mob_arr[index]->count)
            {
                index = UMIN(index+1, COUNT_ARRAY_SIZE-1);
                for (x = 0; x <= index; x++)
                    if (pMobIndex->count > mob_arr[x]->count)
                    {
                        for (y = index; y > x; y--)
                            mob_arr[y] = mob_arr[y-1];
                        mob_arr[x] = pMobIndex;
                        break;
                    }
            }
        }

    send_to_pager("[#]  [Cnt] [Vnum] [Name]\n\r", ch);
    for (x = 0; x <= index; x++)
        pager_printf(ch, "%3d. %-5d %-6d %s\n\r",
                     x+1,
                     mob_arr[x]->count,
                     mob_arr[x]->ivnum,
                     mob_arr[x]->short_descr);
}

void orphan_recurse(ROOM_INDEX_DATA *room)
{
    EXIT_DATA *pexit;

    if (!room || !IS_ROOM_FLAG(room, ROOM_ORPHANED))
        return;

    REMOVE_ROOM_FLAG(room, ROOM_ORPHANED);
    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
        if (pexit->to_room && IS_ROOM_FLAG(pexit->to_room, ROOM_ORPHANED))
            orphan_recurse(pexit->to_room);

    if (room->tele_vnum)
        orphan_recurse(get_room_index(room->tele_vnum));
}

void orphan_room_search(void)
{
    ROOM_INDEX_DATA *start, *pRoomIndex;
    int iHash;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for ( pRoomIndex  = room_index_hash[iHash];
              pRoomIndex;
              pRoomIndex  = pRoomIndex->next )
            SET_ROOM_FLAG(pRoomIndex, ROOM_ORPHANED);
    }

    if ((start = get_room_index(1)))
        orphan_recurse(start);
    if ((start = get_room_index(100)))
        orphan_recurse(start);
    if ((start = get_room_index(2701)))
        orphan_recurse(start);
    if ((start = get_room_index(3294)))
        orphan_recurse(start);
    if ((start = get_room_index(29751)))
        orphan_recurse(start);
    if ((start = get_room_index(36601)))
        orphan_recurse(start);
}

void show_orphaned_rooms(CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *pRoomIndex;
    int vnum, count = 0;

    if (GetMaxLevel(ch) == MAX_LEVEL)
        orphan_room_search();

    for (vnum = 1; vnum < top_room_vnum; vnum++)
    {
        if (!(pRoomIndex = get_room_index(vnum)))
            continue;

        if (IS_ROOM_FLAG(pRoomIndex, ROOM_ORPHANED))
        {
            if (count < 1000)
                pager_printf(ch, "%6d) %s\n\r",
                             pRoomIndex->vnum,
                             pRoomIndex->name);
            count++;
        }
    }
    pager_printf(ch, "%d orphaned rooms (first 1000 displayed).\n\r", count);
}

void do_show( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int count=0;

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Usage:\n\r"
                      "  show objects <name>\n\r"
                      "  show mobiles <name>\n\r"
                      "  show rooms <name>\n\r"
                      "  show modified\n\r"
                      "  show initialized\n\r"
                      "  show entrances\n\r"
                      "  show exits\n\r"
                      "  show unused objects\n\r"
                      "  show unused mobiles\n\r"
                      "  show door <key vnum>\n\r"
                      "  show hhf <name>\n\r"
                      "  show teleport [area]\n\r"
                      "  show currency [area]\n\r"
                      "  show rent [area]\n\r"
                      "  show portals\n\r", ch );
        send_to_char( "  show bigbucks [amount]\n\r"
                      "  show quest\n\r"
                      "  show ospell [spell name]\n\r"
                      "  show noarea\n\r"
                      "  show nowhere\n\r"
                      "  show utimer\n\r"
                      "  show logstats\n\r"
                      "  show disconnected [all]\n\r"
                      "  show bugs\n\r"
                      "  show ideas\n\r"
                      "  show typos\n\r"
                      "  show timers [name]\n\r"
                      "  show progs\n\r"
                      "  show guildmasters [class]\n\r"
                      "  show mreset <mobile>\n\r", ch );
        send_to_char( "  show oreset <object>\n\r"
                      "  show mobsaves\n\r"
                      "  show oaffect\n\r"
                      "  show bit13\n\r"
                      "  show ocount\n\r"
                      "  show mcount\n\r"
                      "  show orphans\n\r",
                      ch );
        return;
    }
    if (!str_prefix(arg, "objects"))
        do_ofind(ch,argument);
    else if (!str_prefix(arg, "mobiles"))
        do_mfind(ch,argument);
    else if (!str_prefix(arg, "rooms"))
        do_rfind(ch,argument);
    else if (!str_prefix(arg, "modified"))
    {
        AREA_DATA *tarea;
        for (tarea = first_area; tarea; tarea = tarea->next)
            if (IS_SET(tarea->flags, AFLAG_MODIFIED))
            {
                sprintf(arg,"%-30.30s| %s\n\r",
                        tarea->name, tarea->filename);
		send_to_char(arg, ch);
                count++;
            }
        ch_printf(ch, "%d areas modified.\n\r", count);
    }
    else if (!str_prefix(arg, "initialized"))
    {
        AREA_DATA *tarea;
        for (tarea = first_area; tarea; tarea = tarea->next)
            if (IS_SET(tarea->flags, AFLAG_INITIALIZED))
            {
                sprintf(arg,"%-24.24s| Age: %2d Reset Frequency: %2d\n\r",
                        tarea->name,
                        tarea->age,
                        tarea->reset_frequency?tarea->reset_frequency:15);
		send_to_char(arg, ch);
                count++;
            }
        ch_printf(ch, "%d areas initialized.\n\r", count);
    }
    else if (!str_prefix(arg, "entrances"))
        find_entrances(ch,argument);
    else if (!str_prefix(arg, "exits"))
        find_exits(ch,argument);
    else if (!str_prefix(arg, "unused"))
    {
        argument = one_argument( argument, arg );
        if (arg[0] == '\0')
            do_show(ch,"");
        else if (!str_prefix(arg, "objects"))
            find_unused_objects(ch,argument);
        else if (!str_prefix(arg, "mobiles"))
            find_unused_mobiles(ch,argument);
    }
    else if (!str_prefix(arg,  "door"))
        find_door_for_key(ch, argument);
    else if (!str_prefix(arg,  "hhf"))
        find_hhf(ch, argument);
    else if (!str_prefix(arg,  "teleport"))
        find_teleport(ch, argument);
    else if (!str_prefix(arg,  "currency"))
        find_currency_assigned(ch, argument);
    else if (!str_prefix(arg,  "rent"))
        find_objects_rent(ch, argument);
    else if (!str_prefix(arg,  "portals"))
        find_portals(ch);
    else if (!str_prefix(arg,  "bigbucks"))
        find_bigbucks(ch, argument);
    else if (!str_prefix(arg,  "quest"))
        find_quest_obj(ch);
    else if (!str_prefix(arg,  "ospell"))
        find_object_spell(ch, argument);
    else if (!str_prefix(arg,  "noarea"))
        find_stuff_noarea(ch);
    else if (!str_prefix(arg,  "nowhere"))
        find_stuff_nowhere(ch);
    else if (!str_prefix(arg,  "utimer"))
        show_update_timers(ch);
    else if (!str_prefix(arg,  "logstats"))
        show_log_stats(ch);
    else if (!str_prefix(arg,  "disconnected"))
        show_disconnected(ch,argument);
    else if (!str_prefix(arg, "bugs"))
        do_bug(ch, NULL);
    else if (!str_prefix(arg, "ideas"))
        do_idea(ch, NULL);
    else if (!str_prefix(arg, "typos"))
        do_typo(ch, NULL);
    else if (!str_prefix(arg, "timers"))
        do_show_timers(ch, argument);
    else if (!str_prefix(arg, "progs"))
        do_show_progs(ch, argument);
    else if (!str_prefix(arg, "guildmasters"))
        do_show_guildmasters(ch, argument);
    else if (!str_prefix(arg, "mreset"))
        do_find_mobile_reset(ch, argument);
    else if (!str_prefix(arg, "oreset"))
        do_find_object_reset(ch, argument);
    else if (!str_prefix(arg, "mobsaves"))
        do_find_mobs_with_saves(ch, argument);
    else if (!str_prefix(arg,  "oaffect"))
        find_object_affect(ch, argument);
    else if (!str_prefix(arg,  "bit13"))
        do_find_mobile_bit13(ch, argument);
    else if (!str_prefix(arg, "ocount"))
        show_obj_counts(ch, argument);
    else if (!str_prefix(arg, "mcount"))
        show_mob_counts(ch, argument);
    else if (!str_prefix(arg, "orphans"))
        show_orphaned_rooms(ch, argument);
    else
        do_show(ch,"");
}
