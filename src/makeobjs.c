/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider             |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops  |~'~.VxvxV.~'~*
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *			Specific object creation module			    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: makeobjs.c,v 1.13 2003/03/07 04:12:55 dotd Exp $";*/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


/*
 * Make a fire.
 */
void make_fire(ROOM_INDEX_DATA *in_room, sh_int timer)
{
    OBJ_DATA *fire;

    fire = create_object( OBJ_VNUM_FIRE );
    if (!fire) return;
    fire->timer = number_fuzzy(timer);
    obj_to_room( fire, in_room );
    return;
}

/*
 * Make a trap.
 */
OBJ_DATA *make_trap(int v0, int v1, int v2, int v3)
{
    OBJ_DATA *trap;

    trap = create_object( OBJ_VNUM_TRAP );
    if (!trap) return NULL;
    trap->timer = 0;
    trap->value[0] = v0;
    trap->value[1] = v1;
    trap->value[2] = v2;
    trap->value[3] = v3;
    return trap;
}


/*
 * Turn an object into scraps.		-Thoric
 */
void make_scraps( OBJ_DATA *obj, bool quiet )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA  *scraps, *tmpobj = NULL;
    CHAR_DATA *ch = NULL;

    separate_obj( obj );
    scraps	= create_object( OBJ_VNUM_SCRAPS );
    if (!scraps) return;
    scraps->timer = number_range( 5, 15 );

    /* don't make scraps of scraps of scraps of ... */
    if ( obj->vnum == OBJ_VNUM_SCRAPS )
    {
        STRFREE( scraps->short_descr );
        scraps->short_descr = STRALLOC( "some debris" );
        STRFREE( scraps->description );
        scraps->description = STRALLOC( "Bits of debris lie on the ground here." );
    }
    else
    {
        sprintf( buf, scraps->short_descr, obj->short_descr );
        STRFREE( scraps->short_descr );
        scraps->short_descr = STRALLOC( buf );
        sprintf( buf, scraps->description, obj->short_descr );
        STRFREE( scraps->description );
        scraps->description = STRALLOC( buf );
    }

    if ( obj->carried_by )
    {
        if (!quiet)
            act( AT_OBJECT, "$p falls to the ground in scraps!",
                 obj->carried_by, obj, NULL, TO_CHAR );
        if ( obj == get_eq_char( obj->carried_by, WEAR_WIELD )
             &&  (tmpobj = get_eq_char( obj->carried_by, WEAR_DUAL_WIELD)) != NULL )
            tmpobj->wear_loc = WEAR_WIELD;

        obj_to_room( scraps, obj->carried_by->in_room);
    }
    else
        if ( obj->in_room )
        {
            if ( !quiet && (ch = obj->in_room->first_person ) != NULL )
            {
                act( AT_OBJECT, "$p is reduced to little more than scraps.",
                     ch, obj, NULL, TO_ROOM );
                act( AT_OBJECT, "$p is reduced to little more than scraps.",
                     ch, obj, NULL, TO_CHAR );
            }
            obj_to_room( scraps, obj->in_room);
        }
    if ( (obj->item_type == ITEM_CONTAINER
          ||   obj->item_type == ITEM_CORPSE_PC) && obj->first_content )
    {
        if ( !quiet && ch && ch->in_room )
        {
            act( AT_OBJECT, "The contents of $p fall to the ground.",
                 ch, obj, NULL, TO_ROOM );
            act( AT_OBJECT, "The contents of $p fall to the ground.",
                 ch, obj, NULL, TO_CHAR );
        }
        if ( obj->carried_by )
            empty_obj( obj, NULL, obj->carried_by->in_room );
        else
            if ( obj->in_room )
                empty_obj( obj, NULL, obj->in_room );
            else
                if ( obj->in_obj )
                    empty_obj( obj, obj->in_obj, NULL );
    }
    extract_obj( obj );
}


/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA *ch, CHAR_DATA *killer, int dt )
{
    char buf[MAX_STRING_LENGTH], buf2[MAX_INPUT_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;
    int x=0;

    if ( IS_NPC(ch) )
    {
	name		= ch->short_descr;
        corpse		= create_object( OBJ_VNUM_CORPSE_NPC );
        if (!corpse)
            return;
	corpse->timer	= 6;
        for (x=1;x<MAX_CURR_TYPE;x++)
            if ( GET_MONEY(ch,x) )
            {
                ch->in_room->area->looted[x] += GET_MONEY(ch,x);

                obj = create_money( GET_MONEY(ch,x), x );
                snprintf(buf, 127, "make_corpse %d %s", ch->vnum, ch->name);
                obj->last_carried_by = str_dup(buf);

                obj_to_obj( obj, corpse );
                GET_MONEY(ch,x) = 0;
            }

/* Cannot use these!  They are used.
	corpse->value[0] = (int)ch->pIndexData->vnum;
	corpse->value[1] = (int)GET_MAX_HIT(ch);
*/
/*	Using corpse cost to cheat, since corpses not sellable */
        corpse->cost     = (-(int)ch->vnum);
        corpse->currtype = DEFAULT_CURR;
        corpse->value[2] = corpse->timer;
    }
    else
    {
	name		= ch->name;
        corpse		= create_object( OBJ_VNUM_CORPSE_PC );
	if (!corpse) return;
	corpse->timer	= 40;
        corpse->value[2] = (int)(corpse->timer/8);
        for (x=1;x<MAX_CURR_TYPE;x++)
            if ( GET_MONEY(ch,x) )
            {
                obj = create_money( GET_MONEY(ch,x), x );
                snprintf(buf, 127, "make_corpse %d %s", 0, ch->name);
                obj->last_carried_by = str_dup(buf);

                obj_to_obj( obj, corpse );
                GET_MONEY(ch,x) = 0;
            }
        if ( IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
	  SET_BIT( corpse->extra_flags, ITEM_CLANCORPSE );
	/* Pkill corpses get save timers, in ticks (approx 70 seconds)
	   This should be anough for the killer to type 'get all corpse'. */
	if ( !IS_NPC(ch) && !IS_NPC(killer) )
	  corpse->value[3] = 1;
	else
	  corpse->value[3] = 0;
    }
    corpse->value[4] = GET_RACE(ch);
    corpse->value[5] = GetMaxLevel(ch);
    corpse->rent = 0;

    if (IsUndead(ch))
    {
        STRFREE( corpse->name );
        corpse->name        = STRALLOC( "dust pile bones" );
        STRFREE( corpse->short_descr );
        corpse->short_descr = STRALLOC( "a pile of dust and bones" );
        STRFREE( corpse->description );
        corpse->description = STRALLOC( "A pile of dust and bones is here." );
    }
    else
    {
        /* Added corpse name - make locate easier , other skills */
        sprintf( buf, "corpse %s", name );
        STRFREE( corpse->name );
        corpse->name = STRALLOC( buf );

        sprintf( buf, corpse->short_descr, name );
        STRFREE( corpse->short_descr );
        corpse->short_descr = STRALLOC( buf );

        if (IS_VALID_SN(dt) &&
            skill_table[dt]->corpse_string &&
            skill_table[dt]->corpse_string[0] != '\0')
        {
            sprintf( buf2, skill_table[dt]->corpse_string, name );
            if (!IS_AFFECTED(ch, AFF_FLYING))
                sprintf(buf, "The %s lying here.", buf2);
            else
                sprintf(buf, "The %s floating in the air.", buf2);
        }
        else
            sprintf( buf, corpse->description, name );

        STRFREE( corpse->description );
        corpse->description = STRALLOC( buf );
    }

    for ( obj = ch->first_carrying; obj; obj = obj_next )
    {
	obj_next = obj->next_content;
        if (!IS_OBJ_STAT2(obj, ITEM2_STORED_ITEM))
        {
	  obj_from_char( obj );
	  if ( IS_OBJ_STAT( obj, ITEM_INVENTORY )
	    || IS_OBJ_STAT( obj, ITEM_DEATHROT ) )
	      extract_obj( obj );
	  else
	      obj_to_obj( obj, corpse );
        }
   }

    obj_to_room( corpse, ch->in_room );
    return;
}



void make_blood( CHAR_DATA *ch )
{
	OBJ_DATA *obj;

	obj		= create_object( OBJ_VNUM_BLOOD );
	if (!obj) return;
	obj->timer	= number_range( 2, 4 );
	obj->value[1]   = number_range( 3, UMIN(5, GetAveLevel(ch)) );
	obj_to_room( obj, ch->in_room );
}


void make_bloodstain( CHAR_DATA *ch )
{
	OBJ_DATA *obj;

	obj		= create_object( OBJ_VNUM_BLOODSTAIN );
	if (!obj) return;
	obj->timer	= number_range( 1, 2 );
	obj_to_room( obj, ch->in_room );
}


/*
 * make some coinage
 */
OBJ_DATA *create_money( int amount, int type )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    if ( amount <= 0 )
    {
	bug( "Create_money: zero or negative money %d.", amount );
	amount = 1;
    }

    if (type < FIRST_CURR || type > LAST_CURR)
    {
        bug( "Create_money: invalid currency type %d.", type );
        amount = 1;
        type = CURR_GOLD;
    }

    if ( amount >= 10000 )
    {
        log_printf_plus(LOG_DEBUG, LEVEL_IMMORTAL, SEV_DEBUG,
                        "create_money: creating %d of type:%d",
                        amount, type );
    }

    if ( amount == 1 )
    {
	obj = create_object( OBJ_VNUM_MONEY_ONE );
        sprintf( buf, obj->name, curr_types[type] );
        STRFREE( obj->name );
        obj->name = STRALLOC( buf );
        sprintf( buf, obj->short_descr, curr_types[type] );
        STRFREE( obj->short_descr );
        obj->short_descr = STRALLOC( buf );
        sprintf( buf, obj->description, curr_types[type] );
        STRFREE( obj->description );
        obj->description  = STRALLOC( buf );
        obj->value[2]    = type;
    }
    else
    {
	obj = create_object( OBJ_VNUM_MONEY_SOME );
        sprintf( buf, obj->name, curr_types[type] );
        STRFREE( obj->name );
        obj->name = STRALLOC( buf );
        sprintf( buf, obj->short_descr, amount, curr_types[type] );
        STRFREE( obj->short_descr );
        obj->short_descr = STRALLOC( buf );
        sprintf( buf, obj->description, curr_types[type] );
        STRFREE( obj->description );
        obj->description  = STRALLOC( buf );
        obj->value[0]    = amount;
        obj->value[2]    = type;
    }

    return obj;
}

