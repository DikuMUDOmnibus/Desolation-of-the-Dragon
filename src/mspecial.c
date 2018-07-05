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
 *			   "Special procedure" module			    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: mspecial.c,v 1.69 2003/12/21 17:20:55 dotd Exp $";*/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "gsn.h"

DECLARE_DO_FUN(do_north);
DECLARE_DO_FUN(do_south);
DECLARE_DO_FUN(do_east);
DECLARE_DO_FUN(do_west);
DECLARE_DO_FUN(do_up);
DECLARE_DO_FUN(do_down);
DECLARE_DO_FUN(do_northeast);
DECLARE_DO_FUN(do_northwest);
DECLARE_DO_FUN(do_southeast);
DECLARE_DO_FUN(do_southwest);
DECLARE_DO_FUN(do_gossip);
DECLARE_DO_FUN(do_shout);
DECLARE_DO_FUN(do_cast);
DECLARE_DO_FUN(do_unlock);
DECLARE_DO_FUN(do_open);
DECLARE_DO_FUN(do_close);
DECLARE_DO_FUN(do_lock);
DECLARE_DO_FUN(do_say);
DECLARE_DO_FUN(do_wear);
DECLARE_DO_FUN(do_emote);
DECLARE_DO_FUN(do_bash);
DECLARE_DO_FUN(do_disarm);
DECLARE_DO_FUN(do_punch);
DECLARE_DO_FUN(do_kick);
DECLARE_DO_FUN(do_rescue);
DECLARE_DO_FUN(do_stand);
DECLARE_DO_FUN(do_wake);
DECLARE_DO_FUN(do_get);
DECLARE_DO_FUN(do_kill);
DECLARE_DO_FUN(do_use);
DECLARE_DO_FUN(do_recite);
DECLARE_DO_FUN(do_zap);
DECLARE_DO_FUN(do_buy);
DECLARE_DO_FUN(do_list);
DECLARE_DO_FUN(do_slay);
DECLARE_DO_FUN(do_force);
DECLARE_DO_FUN(do_purge);
DECLARE_DO_FUN(do_quivering_palm);
DECLARE_DO_FUN(do_mset);
DECLARE_DO_FUN(do_copyover);
DECLARE_DO_FUN(do_reboot);
DECLARE_DO_FUN(do_shutdown);

DECLARE_SPELL_FUN(spell_cure_blindness);
DECLARE_SPELL_FUN(spell_cure_poison);
DECLARE_SPELL_FUN(spell_poison);
DECLARE_SPELL_FUN(spell_teleport);

#define SPEC SPECIAL_FUNC
#include "mspecial.h"
#undef SPEC

/* for tracking */
int find_first_step(ROOM_INDEX_DATA *src, ROOM_INDEX_DATA *target, int maxdist );

#define SPEC(func) \
    if (++i==2) {i=0;send_to_pager("\n\r",ch);} pager_printf(ch, "%-38s", #func)
void do_mslist( CHAR_DATA *ch, char *argument )
{
    int i=-1;

    set_pager_color(AT_PLAIN, ch);

#include "mspecial.h"

    send_to_pager( "\n\r", ch );
}
#undef SPEC

#define SPEC(func) \
    if (mob->spec_fun == func && strstr(#func, argument)) \
    { ch_printf(ch, "u%-5d %5d: %s\n\r", mob->unum, mob->vnum, #func); match++; continue; }
void do_msfind( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;
    int match=0;

    if (!argument)
    {
        send_to_char("msfind <name>\n\r", ch);
        return;
    }
    for (mob=first_char;mob;mob=mob->next)
    {
        if (!IS_NPC(mob) || !mob->spec_fun)
            continue;
#include "mspecial.h"
    }
    ch_printf(ch, "%d matches found.\n\r", match);
}
#undef SPEC

/*
 * Given a name, return the appropriate spec fun.
 */
#define SPEC(func) \
    if ( !str_cmp( name, #func ) ) return func
SPEC_FUN *m_spec_lookup( const char *name )
{
#include "mspecial.h"
    return NULL;
}
#undef SPEC

/*
 * Given a pointer, return the appropriate spec fun text.
 */
#define SPEC(func) \
    if ( special == func ) return #func
char *m_lookup_spec( SPEC_FUN *special )
{
#include "mspecial.h"
    return "";
}
#undef SPEC


/* if a spell casting mob is hating someone... try and summon them */
bool summon_if_hating( CHAR_DATA *ch )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    char name[MAX_INPUT_LENGTH];
    bool found = FALSE;

    if ( ch->fighting ||
         ch->fearing ||
         !ch->hating ||
         IS_SET( ch->in_room->room_flags, ROOM_SAFE ) ||
         IS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) ||
         GetMaxLevel(ch) < 20 )
        return FALSE;

    /* if player is close enough to hunt... don't summon */
    if ( ch->hunting )
        return FALSE;

    one_argument( ch->hating->name, name );

    /* make sure the char exists - works even if player quits */
    for (victim = first_char;
         victim;
         victim = victim->next)
    {
        if ( !str_cmp( ch->hating->name, victim->name ) )
        {
            found = TRUE;
            break;
        }
    }

    if ( !found )
        return FALSE;
    if ( ch->in_room == victim->in_room )
        return FALSE;
    if ( !IS_NPC( victim ) )
        sprintf( buf, "summon 0.%s", name );
    else
        sprintf( buf, "summon %s", name );
    do_cast( ch, buf );
    return TRUE;
}



/*
 * Core procedure for dragons.
 */
bool dragon( CHAR_DATA *ch, char *spell_name )
{
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    int sn;

    if ( ch->position != POS_FIGHTING )
        return FALSE;

    for ( victim = ch->in_room->first_person; victim; victim = v_next )
    {
        v_next = victim->next_in_room;
        if ( who_fighting( victim ) == ch && number_bits( 2 ) == 0 )
            break;
    }

    if ( !victim )
        return FALSE;

    if ( ( sn = skill_lookup( spell_name ) ) < 0 )
        return FALSE;
    (*skill_table[sn]->spell_fun) ( sn, BestSkLv(ch, sn), ch, victim );
    return TRUE;
}

SPECIAL_FUNC(spec_dragon)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if ( type != SFT_UPDATE && type != SFT_AGGR_UPDATE )
        return FALSE;

    if ( type == SFT_UPDATE && !who_fighting(ch) )
        return FALSE;

    switch (GET_RACE(ch))
    {
    case RACE_DRAGON:
    case RACE_DRAGON_RED:
        return dragon(ch, "breath of fire");
    case RACE_DRAGON_BLACK:
        return dragon(ch, "breath of acid");
    case RACE_DRAGON_GREEN:
        return dragon(ch, "breath of gas");
    case RACE_DRAGON_WHITE:
        return dragon(ch, "breath of frost");
    case RACE_DRAGON_BLUE:
        return dragon(ch, "breath of lightning");
    case RACE_DRAGON_SILVER:
        if (number_percent() < 75)
            return dragon(ch, "breath of frost");
        return dragon(ch, "breath of gas");
    case RACE_DRAGON_GOLD:
        if (number_percent() < 75)
            return dragon(ch, "breath of fire");
        return dragon(ch, "breath of gas");
    case RACE_DRAGON_BRONZE:
        if (number_percent() < 75)
            return dragon(ch, "breath of lightning");
        return dragon(ch, "breath of gas");
    case RACE_DRAGON_COPPER:
        if (number_percent() < 75)
            return dragon(ch, "breath of acid");
        return dragon(ch, "breath of gas");
    case RACE_DRAGON_BRASS:
        return dragon(ch, "breath of gas");
    }

    return FALSE;
}


SPECIAL_FUNC(spec_cast_adept)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    CHAR_DATA *victim;
    CHAR_DATA *v_next;

    if ( !IS_AWAKE(ch) )
        return FALSE;

    for ( victim = ch->in_room->first_person; victim; victim = v_next )
    {
        v_next = victim->next_in_room;
        if ( victim != ch && can_see( ch, victim ) && number_bits( 1 ) == 0 )
            break;
    }

    if ( !victim )
        return FALSE;

    switch ( number_bits( 3 ) )
    {
    case 0:
        act( AT_MAGIC, "$n utters the word 'ciroht'.", ch, NULL, NULL, TO_ROOM );
        spell_smaug( gsn_armor, GetMaxLevel(ch), ch, victim );
        return TRUE;

    case 1:
        act( AT_MAGIC, "$n utters the word 'sunimod'.", ch, NULL, NULL, TO_ROOM );
        spell_smaug( gsn_bless, GetMaxLevel(ch), ch, victim );
        return TRUE;

    case 2:
        act( AT_MAGIC, "$n utters the word 'suah'.", ch, NULL, NULL, TO_ROOM );
        spell_cure_blindness( gsn_cure_blindness,
                              GetMaxLevel(ch), ch, victim );
        return TRUE;

    case 3:
        act( AT_MAGIC, "$n utters the word 'nran'.", ch, NULL, NULL, TO_ROOM );
        spell_smaug( gsn_cure_light,
                     GetMaxLevel(ch), ch, victim );
        return TRUE;

    case 4:
        act( AT_MAGIC, "$n utters the word 'nyrcs'.", ch, NULL, NULL, TO_ROOM );
        spell_cure_poison( gsn_cure_poison,
                           GetMaxLevel(ch), ch, victim );
        return TRUE;

    case 5:
        act( AT_MAGIC, "$n utters the word 'gartla'.", ch, NULL, NULL, TO_ROOM );
        spell_smaug( gsn_refresh, GetMaxLevel(ch), ch, victim );
        return TRUE;

    }

    return FALSE;
}

SPECIAL_FUNC(spec_cast_mage)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    summon_if_hating( ch );

    if ( ch->position != POS_FIGHTING )
        return FALSE;

    for ( victim = ch->in_room->first_person; victim; victim = v_next )
    {
        v_next = victim->next_in_room;
        if ( who_fighting( victim ) && number_bits( 2 ) == 0 )
            break;
    }

    if ( !victim || victim == ch )
        return FALSE;

    for ( ;; )
    {
        int min_level;

        switch ( number_bits( 4 ) )
        {
        case  0: min_level =  0; spell = "blindness";      break;
        case  1: min_level =  3; spell = "chill touch";    break;
        case  2: min_level =  7; spell = "weaken";         break;
        case  4: min_level = 11; spell = "colour spray";   break;
        case  6: min_level = 13; spell = "energy drain";   break;
        case  7:
        case  8:
        case  9: min_level = 15; spell = "fireball";       break;
        default: min_level = 20; spell = "acid blast";     break;
        }

        if ( GetMaxLevel(ch) >= min_level )
            break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
        return FALSE;
    (*skill_table[sn]->spell_fun) ( sn, GetMaxLevel(ch), ch, victim );
    return TRUE;
}

SPECIAL_FUNC(spec_cast_undead)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *spell;
    int sn;

    summon_if_hating( ch );

    if ( ch->position != POS_FIGHTING )
        return FALSE;

    for ( victim = ch->in_room->first_person; victim; victim = v_next )
    {
        v_next = victim->next_in_room;
        if ( who_fighting( victim ) == ch && number_bits( 2 ) == 0 )
            break;
    }

    if ( !victim || victim == ch )
        return FALSE;

    for ( ;; )
    {
        int min_level;

        switch ( number_bits( 4 ) )
        {
        case  0: min_level =  0; spell = "curse";          break;
        case  1: min_level =  3; spell = "weaken";         break;
        case  2: min_level =  6; spell = "chill touch";    break;
        case  3: min_level =  9; spell = "blindness";      break;
        case  4: min_level = 12; spell = "poison";         break;
        case  5: min_level = 15; spell = "energy drain";   break;
        case  6: min_level = 18; spell = "harm";           break;
        default: min_level = 35; spell = "gate";           break;
        }

        if ( GetMaxLevel(ch) >= min_level )
            break;
    }

    if ( ( sn = skill_lookup( spell ) ) < 0 )
        return FALSE;
    (*skill_table[sn]->spell_fun) ( sn, GetMaxLevel(ch), ch, victim );
    return TRUE;
}

SPECIAL_FUNC(spec_executioner)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    CHAR_DATA *cityguard;
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    char *crime, buf[MAX_INPUT_LENGTH];

    if ( !IS_AWAKE(ch) || ch->fighting )
        return FALSE;

    crime = "";
    for ( victim = ch->in_room->first_person; victim; victim = v_next )
    {
        v_next = victim->next_in_room;

        if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER) )
        { crime = "KILLER"; break; }

        if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF) )
        { crime = "THIEF"; break; }
    }

    if ( !victim )
        return FALSE;

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
        sprintf( buf, "%s is a %s!  As well as a COWARD!",
                 victim->name, crime );
        do_gossip( ch, buf );
        return TRUE;
    }

    sprintf( buf, "%s is a %s!  PROTECT THE INNOCENT!  MORE BLOOOOD!!!",
             victim->name, crime );
    do_shout( ch, buf );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    if ( char_died(ch) )
        return TRUE;

    /* Added log in case of missing cityguard -- Tri */

    if ( !(cityguard = create_mobile( MOB_VNUM_CITYGUARD )) )
    {
        bug( "Missing Cityguard - Vnum:[%d]", MOB_VNUM_CITYGUARD );
        return TRUE;
    }
    char_to_room( cityguard, ch->in_room );

    if ( !(cityguard = create_mobile( MOB_VNUM_CITYGUARD )) )
    {
        bug( "Missing Cityguard - Vnum:[%d]", MOB_VNUM_CITYGUARD );
        return TRUE;
    }
    char_to_room( cityguard, ch->in_room );

    return TRUE;
}

SPECIAL_FUNC(spec_scavenger)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    OBJ_DATA *corpse;
    OBJ_DATA *c_next;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if ( !IS_AWAKE(ch) || type!=SFT_UPDATE )
        return FALSE;

    for ( corpse = ch->in_room->first_content; corpse; corpse = c_next )
    {
        c_next = corpse->next_content;
        if ( corpse->item_type != ITEM_CORPSE_NPC )
            continue;

        act( AT_ACTION, "$n devours a corpse.", ch, NULL, NULL, TO_ROOM );
        for ( obj = corpse->first_content; obj; obj = obj_next )
        {
            obj_next = obj->next_content;
            obj_from_obj( obj );
            obj_to_room( obj, ch->in_room );
        }
        extract_obj( corpse );
        return TRUE;
    }

    return FALSE;
}

SPECIAL_FUNC(spec_guard)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *v_next;
    CHAR_DATA *ech;
    char *crime;
    int max_evil;

    if ( !IS_AWAKE(ch) || ch->fighting )
        return FALSE;

    max_evil = 300;
    ech      = NULL;
    crime    = "";

    for ( victim = ch->in_room->first_person; victim; victim = v_next )
    {
        v_next = victim->next_in_room;

        if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER) )
        { crime = "KILLER"; break; }

        if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF) )
        { crime = "THIEF"; break; }

        if ( victim->fighting
             &&   who_fighting( victim ) != ch
             &&   victim->alignment < max_evil )
        {
            max_evil = victim->alignment;
            ech      = victim;
        }
    }

    if ( victim && IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
        sprintf( buf, "%s is a %s!  As well as a COWARD!",
                 victim->name, crime );
        do_gossip( ch, buf );
        return TRUE;
    }

    if ( victim )
    {
        sprintf( buf, "%s is a %s!  PROTECT THE INNOCENT!!  BANZAI!!",
                 victim->name, crime );
        do_shout( ch, buf );
        multi_hit( ch, victim, TYPE_UNDEFINED );
        return TRUE;
    }

    if ( ech )
    {
        act( AT_GOSSIP, "$n screams 'PROTECT THE INNOCENT!!  BANZAI!!",
             ch, NULL, NULL, TO_ROOM );
        multi_hit( ch, ech, TYPE_UNDEFINED );
        return TRUE;
    }

    return FALSE;
}

SPECIAL_FUNC(spec_janitor)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    OBJ_DATA *trash;
    OBJ_DATA *trash_next;
    char buf[MAX_INPUT_LENGTH];

    if ( !IS_AWAKE(ch) || type!=SFT_UPDATE)
        return FALSE;

    for ( trash = ch->in_room->first_content; trash; trash = trash_next )
    {
        trash_next = trash->next_content;
        if ( !IS_SET( trash->wear_flags, ITEM_TAKE )
             ||    IS_OBJ_STAT( trash, ITEM_BURRIED ) )
            continue;

        if ( trash->item_type == ITEM_MONEY )
        {
            obj_from_room( trash );
            extract_obj( trash );
            return TRUE;
        }
        if ( trash->item_type == ITEM_DRINK_CON
             ||   trash->item_type == ITEM_TRASH
             ||   trash->cost < 100
             ||  (trash->vnum == OBJ_VNUM_SHOPPING_BAG
                  &&  !trash->first_content) )
        {
            act( AT_ACTION, "$n picks up some trash.", ch, NULL, NULL, TO_ROOM );
            obj_from_room( trash );
            obj_to_char( trash, ch );
            return TRUE;
        }
        if ( trash->cost >= 100 )
        {
            sprintf(buf, "donate all.%s", spacetodash(trash->name));
            obj_from_room( trash );
            obj_to_char( trash, ch );
            interpret(ch, buf);
            return TRUE;
        }

    }

    return FALSE;
}



SPECIAL_FUNC(spec_mayor)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    static const char open_path[] =
        "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

    static const char close_path[] =
        "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

    static const char *path;
    static int pos;
    static bool move;

    if ( !move )
    {
        if ( time_info.hour ==  6 )
        {
            path = open_path;
            move = TRUE;
            pos  = 0;
        }

        if ( time_info.hour == 20 )
        {
            path = close_path;
            move = TRUE;
            pos  = 0;
        }
    }

    if ( ch->fighting )
        return spec_cleric( ch, cmd, arg, cmd_ch, type );
    if ( !move || ch->position < POS_SLEEPING )
        return FALSE;

    switch ( path[pos] )
    {
    case '0':
    case '1':
    case '2':
    case '3':
        move_char( ch, get_exit( ch->in_room, path[pos] - '0' ), 0 );
        break;

    case 'W':
        ch->position = POS_STANDING;
        act( AT_ACTION, "$n awakens and groans loudly.", ch, NULL, NULL, TO_ROOM );
        break;

    case 'S':
        ch->position = POS_SLEEPING;
        act( AT_ACTION, "$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM );
        break;

    case 'a':
        act( AT_SAY, "$n says 'Hello Honey!'", ch, NULL, NULL, TO_ROOM );
        break;

    case 'b':
        act( AT_SAY, "$n says 'What a view!  I must do something about that dump!'",
             ch, NULL, NULL, TO_ROOM );
        break;

    case 'c':
        act( AT_SAY, "$n says 'Vandals!  Youngsters have no respect for anything!'",
             ch, NULL, NULL, TO_ROOM );
        break;

    case 'd':
        act( AT_SAY, "$n says 'Good day, citizens!'", ch, NULL, NULL, TO_ROOM );
        break;

    case 'e':
        act( AT_SAY, "$n says 'I hereby declare the town of Darkhaven open!'",
             ch, NULL, NULL, TO_ROOM );
        break;

    case 'E':
        act( AT_SAY, "$n says 'I hereby declare the town of Darkhaven closed!'",
             ch, NULL, NULL, TO_ROOM );
        break;

    case 'O':
        do_unlock( ch, "gate" );
        do_open( ch, "gate" );
        break;

    case 'C':
        do_close( ch, "gate" );
        do_lock( ch, "gate" );
        break;

    case '.' :
        move = FALSE;
        break;
    }

    pos++;
    return FALSE;
}

SPECIAL_FUNC(spec_snake)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    CHAR_DATA *victim = NULL;

    if ( type!=SFT_UPDATE || ch->position != POS_FIGHTING ||
         (victim = who_fighting(ch)) == NULL ||
         number_percent() > 2 * GetAveLevel(ch) )
        return FALSE;

    act(AT_HIT, "You bite $N!",  ch, NULL, victim, TO_CHAR);
    act(AT_ACTION, "$n bites $N!",  ch, NULL, victim, TO_NOTVICT);
    act(AT_POISON, "$n bites you!", ch, NULL, victim, TO_VICT);
    spell_poison(gsn_poison, GetAveLevel(ch), ch, victim);
    return TRUE;
}


SPECIAL_FUNC(spec_thief)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    CHAR_DATA *victim;
    CHAR_DATA *v_next;

    if ( type!=SFT_UPDATE || ch->position != POS_STANDING )
        return FALSE;

    for ( victim = ch->in_room->first_person; victim; victim = v_next )
    {
        v_next = victim->next_in_room;

        if ( IS_NPC(victim)
             ||   GetMaxLevel(victim) >= LEVEL_IMMORTAL
             ||   number_bits( 2 ) != 0
             ||   !can_see( ch, victim ) )	/* Thx Glop */
            continue;

        if ( IS_AWAKE(victim) && number_range( 0, GetMaxLevel(ch) ) == 0 )
        {
            act( AT_ACTION, "You discover $n's hands in your sack of money!",
                 ch, NULL, victim, TO_VICT );
            act( AT_ACTION, "$N discovers $n's hands in $S sack of money!",
                 ch, NULL, victim, TO_NOTVICT );
            return TRUE;
        }
        else
        {
            int money, maxmoney, type=CURR_GOLD;
            for (money=0;money<10;money++)
            {
                type = number_range(FIRST_CURR, LAST_CURR);
                if (GET_MONEY(victim,type))
                    break;
            }
            if (!GET_MONEY(victim,type))
                return FALSE;

            maxmoney = GetMaxLevel(ch) * 1000;
            money = GET_MONEY(victim,type)
                * number_range( 1, URANGE(2, GetMaxLevel(ch)/4, 10) ) /100;
            GET_MONEY(ch,type)     += 9 * money / 10;
            GET_MONEY(victim,type) -= money;
            if ( GET_MONEY(ch,type) > maxmoney )
            {
                boost_economy( ch->in_room->area, GET_MONEY(ch,type) - maxmoney/2, type );
                GET_MONEY(ch,type) = maxmoney/2;
            }
            log_printf_plus( LOG_DEBUG, LEVEL_LOG_CSET, SEV_DEBUG,
                             "spec_thief: %s stole %d %s from %s",
                             GET_NAME(ch), money, curr_types[type], GET_NAME(victim));
            return TRUE;
        }
    }

    return FALSE;
}

void submit(CHAR_DATA *ch, CHAR_DATA *t)
{
    char buf[MAX_INPUT_LENGTH];

    switch(number_range(1,8))
    {
    case 1:
        sprintf(buf, "bow %s", GET_NAME(t));
        interpret(ch, buf);
        break;
    case 2:
        sprintf(buf, "smile %s", GET_NAME(t));
        interpret(ch, buf);
        break;
    case 3:
        sprintf(buf, "wink %s", GET_NAME(t));
        interpret(ch, buf);
        break;
    case 4:
        sprintf(buf, "wave %s", GET_NAME(t));
        interpret(ch, buf);
        break;
    default:
        act(AT_PLAIN,"$n nods $s head at you.", ch, 0, t, TO_VICT);
        act(AT_PLAIN,"$n nods $s head at $N.", ch, 0, t, TO_NOTVICT);
        break;
    }
}

void sayhello(CHAR_DATA *ch, CHAR_DATA *t)
{
    char buf[MAX_INPUT_LENGTH];

    if (number_percent()>75)
        return;

    sprintf(buf, "BUG PLEASE REPORT: sayhello");

    if (IsBadSide(ch))
    {
        switch(number_range(1,10))
        {
        case 1:
            do_say(ch, "Hey doofus, go get a life!");
            break;
        case 2:
            if (t->sex == SEX_FEMALE)
                do_say(ch, "Get lost ...... witch!");
            else
                do_say(ch, "Are you talking to me, punk?");
            break;
        case 3:
            do_say(ch, "May the road you travel be cursed!");
            break;
        case 4:
            if (t->sex == SEX_FEMALE)
                sprintf(buf, "Make way!  Make way for me, %s!", PERS(t,ch));
            else
                sprintf(buf, "Make way!  Make way for me, %s!", PERS(t,ch));
            do_say(ch, buf);
            break;
        case 5:
            do_say(ch, "May the evil godling Ixzuul grin evily at you.");
            break;
        case 6:
            do_say(ch, "Not you again...");
            break;
        case 7:
            do_say(ch, "You are always welcome here, great one, now go clean the stables.");
            break;
        case 8:
            do_say(ch, "Ya know, those smugglers are men after my own heart...");
            break;
        case 9:
            if (time_info.hour > 6 && time_info.hour < 12)
                sprintf(buf, "It's morning, %s, do you know where your brains are?", PERS(t,ch));
            else if (time_info.hour >=12 && time_info.hour < 20)
                sprintf(buf, "It's afternoon, %s, do you know where your parents are?", PERS(t,ch));
            else if (time_info.hour >= 20 && time_info.hour <= 24)
                sprintf(buf, "It's evening, %s, do you know where your kids are?", PERS(t,ch));
            else
                sprintf(buf, "Up for a midnight stroll, %s?\n", PERS(t,ch));
            do_say(ch, buf);
            break;
        case 10:
            {
                char buf2[80];
                if (time_info.hour < 6)
                    strcpy(buf2,"evening");
                else if (time_info.hour < 12)
                    strcpy(buf2, "morning");
                else if (time_info.hour < 20)
                    strcpy(buf2, "afternoon");
                else
                    strcpy(buf2, "evening");

                if (IS_CLOUDY(ch->in_room->area->weather))
                    sprintf(buf, "Nice %s to go for a walk, %s, I hate it.", buf2, PERS(t,ch));
                else if (IS_RAINING(ch->in_room->area->weather))
                    sprintf(buf, "I hope %s's rain never clers up.. don't you %s?", buf2, PERS(t,ch));
                else if (IS_SNOWING(ch->in_room->area->weather))
                    sprintf(buf, "What a wonderfully miserable %s, %s!", buf2, PERS(t,ch));
                else
                    sprintf(buf, "Such a terrible %s, don't you think?", buf2);
                do_say(ch, buf);
                break;
            }
        }
    }
    else
    {
        switch(number_range(1,10))
        {
        case 1:
            do_say(ch, "Greetings, adventurer!");
            break;
        case 2:
            if (t->sex == SEX_FEMALE)
                do_say(ch, "Good day, Milady.");
            else
                do_say(ch, "Good day, Lord.");
            break;
        case 3:
            if (t->sex == SEX_FEMALE)
                do_say(ch, "Pleasant journey, Mistress.");
            else
                do_say(ch, "Pleasant journey, Master.");
            break;
        case 4:
            if (t->sex == SEX_FEMALE)
                sprintf(buf, "Make way!  Make way for the lady %s!", PERS(t,ch));
            else
                sprintf(buf, "Make way!  Make way for the lord %s!", PERS(t,ch));
            do_say(ch, buf);
            break;
        case 5:
            do_say(ch, "May the prophet smile upon you.");
            break;
        case 6:
            do_say(ch, "It is a pleasure to see you again.");
            break;
        case 7:
            do_say(ch, "You are always welcome here, great one!");
            break;
        case 8:
            do_say(ch, "My lord bids you greetings.");
            break;
        case 9:
            if (time_info.hour > 6 && time_info.hour < 12)
                sprintf(buf, "Good morning to ye, %s.", PERS(t,ch));
            else if (time_info.hour >=12 && time_info.hour < 20)
                sprintf(buf, "Good afternoon, %s.", PERS(t,ch));
            else if (time_info.hour >= 20 && time_info.hour <= 24)
                sprintf(buf, "Pleasant evening, %s.", PERS(t,ch));
            else
                sprintf(buf, "Up for a midnight stroll, %s?", PERS(t,ch));
            do_say(ch, buf);
            break;
        case 10:
            {
                char buf2[160];
                if (time_info.hour < 6)
                    strcpy(buf2,"evening");
                else if (time_info.hour < 12)
                    strcpy(buf2, "morning");
                else if (time_info.hour < 20)
                    strcpy(buf2, "afternoon");
                else
                    strcpy(buf2, "evening");
                if (IS_CLOUDY(ch->in_room->area->weather))
                    sprintf(buf, "Nice %s to go for a walk, %s.", buf2, PERS(t,ch));
                else if (IS_RAINING(ch->in_room->area->weather))
                    sprintf(buf, "I hope %s's rain clears up.. don't you %s?", buf2, PERS(t,ch));
                else if (IS_SNOWING(ch->in_room->area->weather))
                    sprintf(buf, "How can you be out on such a miserable %s, %s!", buf2, PERS(t,ch));
                else
                    sprintf(buf, "Such a pleasant %s, don't you think?", buf2);
                do_say(ch, buf);
                break;
            }
        }
    }
}

bool greet_people(CHAR_DATA *ch)
{
    CHAR_DATA *tch;

    if (!IS_NPC(ch) || !IS_SET(ch->act,ACT_GREET))
        return FALSE;

    for (tch=ch->in_room->first_person; tch; tch = tch->next_in_room)
    {
        if (!IS_NPC(tch) && can_see(ch,tch) && number_range(1,1000)<=GetMaxLevel(tch))
        {
            submit(ch, tch);
            sayhello(ch, tch);
            return TRUE;
        }
    }
    return FALSE;
}

bool callforhelp(CHAR_DATA *ch, SPEC_FUN *spec)
{
    CHAR_DATA *vch;
    sh_int count=0;

    if (!who_fighting(ch))
        return FALSE;

    for (vch=first_char;vch && count<=2;vch=vch->next)
    {
        if (ch!=vch && !vch->hunting && !vch->fighting && !char_died(vch) &&
            vch->in_room!=ch->in_room && spec == vch->spec_fun)
        {
            if (IS_NPC(ch) && IS_NPC(vch) &&
                ch->vnum!=vch->vnum &&
                number_percent()<5)
                do_shout(vch, "Fear not, I shall assist you!");
            start_hating(vch,who_fighting(ch));
            if (!IS_ACT_FLAG(vch, ACT_SENTINEL))
                start_hunting(vch,who_fighting(ch));
            count++;
        }
    }

    if (count>0)
        return TRUE;

    return FALSE;
}

SPECIAL_FUNC(spec_GenericCityguard)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc, *hatee, *vch;
    char buf[MAX_INPUT_LENGTH];

    if (!IS_AWAKE(ch))
        return FALSE;

    if (type!=SFT_UPDATE)
        return FALSE;

    vch = who_fighting(ch);

    if (vch && (vch->spec_fun == ch->spec_fun || IS_IMMORTAL(vch)))
    {
        stop_fighting(ch, TRUE);
        do_say(ch, "Pardon me, I didn't mean to attack you!");
        return TRUE;
    }

    if (vch)
    {
        if (number_percent()<6)
        {
            do_shout(ch, "To me, my fellows!  I need thy aid!");
            callforhelp(ch,ch->spec_fun);
            return TRUE;
        }
        return spec_warrior(proc,cmd,arg,cmd_ch,type);
    }

    if ((hatee=race_align_hatee(ch))!=NULL)
    {
        if (!IS_AFFECTED(hatee, AFF_CHARM))
        {
            do_say(ch, "Die, foul creature!");
            set_fighting(ch,hatee);
            multi_hit(ch,hatee,TYPE_UNDEFINED);
            return TRUE;
        }

        for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
            if (hatee->master == vch)
                break;
        if (vch &&
            hatee->master == vch &&
            number_percent() > 90)
        {
            sprintf(buf, "point %s", spacetodash(hatee->name));
            interpret(ch, buf);
            sprintf(buf, "%s, we don't like their kind here, please take it elsewhere.", PERS(vch,ch));
            do_say(ch, buf);
            return TRUE;
        }
    }

    if (greet_people(ch))
        return TRUE;

    if (ch->first_carrying)
    {
        OBJ_DATA *obj;

        for (obj = ch->first_carrying; obj; obj = obj->next_content)
            if (obj->pIndexData->area != ch->pIndexData->area)
            {
                sprintf(buf, "donate %s", spacetodash(obj->name));
                interpret(ch, buf);
                return TRUE;
            }

        if (number_percent()<5)
        {
            do_wear(ch, "all");
            return TRUE;
        }
    }

    return FALSE;
}

SPECIAL_FUNC(spec_SilverstoneCityguard)
{    return(spec_GenericCityguard(proc,cmd,arg,cmd_ch,type));}
SPECIAL_FUNC(spec_MalariaCityguard)
{    return(spec_GenericCityguard(proc,cmd,arg,cmd_ch,type));}
SPECIAL_FUNC(spec_HorsemenCityguard)
{    return(spec_GenericCityguard(proc,cmd,arg,cmd_ch,type));}
SPECIAL_FUNC(spec_WoodElfCityguard)
{    return(spec_GenericCityguard(proc,cmd,arg,cmd_ch,type));}
SPECIAL_FUNC(spec_MidgaardCityguard)
{    return(spec_GenericCityguard(proc,cmd,arg,cmd_ch,type));}
SPECIAL_FUNC(spec_AtlantisCityguard)
{    return(spec_GenericCityguard(proc,cmd,arg,cmd_ch,type));}
SPECIAL_FUNC(spec_NewThalosCityguard)
{    return(spec_GenericCityguard(proc,cmd,arg,cmd_ch,type));}
SPECIAL_FUNC(spec_PrydainCityguard)
{    return(spec_GenericCityguard(proc,cmd,arg,cmd_ch,type));}
SPECIAL_FUNC(spec_MordilniaCityguard)
{    return(spec_GenericCityguard(proc,cmd,arg,cmd_ch,type));}

SPECIAL_FUNC(spec_GenericCityguardHateUndead)
{    return(spec_GenericCityguard(proc,cmd,arg,cmd_ch,type));}

SPECIAL_FUNC(spec_troguard)
{    return(spec_GenericCityguard(proc,cmd,arg,cmd_ch,type));}

bool GenericCitizen(void *proc, SPEC_FUN *spec, int type)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (type!=SFT_UPDATE || !IS_AWAKE(ch))
        return FALSE;

    if (who_fighting(ch))
        if (!callforhelp(ch,spec))
        {
            do_say(ch, "Alas, I am alone!");
            return TRUE;
        }

    greet_people(ch);

    return TRUE;
}

SPECIAL_FUNC(spec_GenericCitizen)
{    return(GenericCitizen(proc,spec_GenericCityguard,type));}
SPECIAL_FUNC(spec_SilverstoneCitizen)
{    return(GenericCitizen(proc,spec_GenericCityguard,type));}
SPECIAL_FUNC(spec_MalariaCitizen)
{    return(GenericCitizen(proc,spec_MalariaCityguard,type));}
SPECIAL_FUNC(spec_HorsemenCitizen)
{    return(GenericCitizen(proc,spec_HorsemenCityguard,type));}
SPECIAL_FUNC(spec_WoodElfCitizen)
{    return(GenericCitizen(proc,spec_WoodElfCityguard,type));}
SPECIAL_FUNC(spec_MidgaardCitizen)
{    return(GenericCitizen(proc,spec_MidgaardCityguard,type));}
SPECIAL_FUNC(spec_AtlantisCitizen)
{    return(GenericCitizen(proc,spec_AtlantisCityguard,type));}
SPECIAL_FUNC(spec_NewThalosCitizen)
{    return(GenericCitizen(proc,spec_NewThalosCityguard,type));}
SPECIAL_FUNC(spec_PyrdainCitizen)
{    return(GenericCitizen(proc,spec_PrydainCityguard,type));}
SPECIAL_FUNC(spec_MordilniaCitizen)
{    return(GenericCitizen(proc,spec_MordilniaCityguard,type));}

bool BlockedCmd(CHAR_DATA *ch, CHAR_DATA *vch, CMDTYPE *cmd, int room, DO_FUN *bcmd, int ch_class)
{
    if (ch->in_room->vnum==room && cmd->do_fun==bcmd && !HAS_CLASS(vch,ch_class))
    {
        act(AT_PLAIN,"$n shakes $s head at you and blocks your way.", ch, 0, vch, TO_VICT);
        act(AT_PLAIN,"$n shakes $s head at $N and blocks $S way.", ch, 0, vch, TO_NOTVICT);
        return TRUE;
    }
    return FALSE;
}

SPECIAL_FUNC(spec_GenericGuildguard)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (type!=SFT_COMMAND)
        return FALSE;

    switch (ch->in_room->vnum)
    {
    case 13532: /* 13519 */
        return(BlockedCmd(ch, cmd_ch, cmd, 13532, do_south, CLASS_THIEF));
    case 13512: /* 13143 */
        return(BlockedCmd(ch, cmd_ch, cmd, 13512, do_south, CLASS_CLERIC));
    case 13526: /* 13154 */
        return(BlockedCmd(ch, cmd_ch, cmd, 13526, do_south, CLASS_WARRIOR));
    case 13525: /* 13153 */
        return(BlockedCmd(ch, cmd_ch, cmd, 13525, do_north, CLASS_MAGE));

    case 18266: /* 18210 */
        return(BlockedCmd(ch, cmd_ch, cmd, 18266, do_west, CLASS_MAGE));
    case 18276: /* 18211 */
        return(BlockedCmd(ch, cmd_ch, cmd, 18276, do_east, CLASS_CLERIC));
    case 18272: /* 18212 */
        return(BlockedCmd(ch, cmd_ch, cmd, 18272, do_north, CLASS_THIEF));
    case 18256: /* 18213 */
        return(BlockedCmd(ch, cmd_ch, cmd, 18256, do_north, CLASS_WARRIOR));

    case 16110: /* 16107 */
        return(BlockedCmd(ch, cmd_ch, cmd, 16110, do_west, CLASS_WARRIOR));
    case 16115: /* 16108 */
        return(BlockedCmd(ch, cmd_ch, cmd, 16115, do_east, CLASS_MAGE));
    case 16117: /* 16109 */
        return(BlockedCmd(ch, cmd_ch, cmd, 16117, do_west, CLASS_THIEF));
    case 16126: /* 16110 */
        return(BlockedCmd(ch, cmd_ch, cmd, 16116, do_east, CLASS_CLERIC));
/*
    case 3017:
        return(BlockedCmd(ch, cmd_ch, cmd, 3017, do_south, CLASS_MAGE));
    case 3004:
        return(BlockedCmd(ch, cmd_ch, cmd, 3004, do_north, CLASS_CLERIC));
    case 3027:
        return(BlockedCmd(ch, cmd_ch, cmd, 3027, do_east, CLASS_THIEF));
    case 3021:
        return(BlockedCmd(ch, cmd_ch, cmd, 3021, do_east, CLASS_WARRIOR));
*/
    case 3282: /* 3209 */
        return(BlockedCmd(ch, cmd_ch, cmd, 3282, do_south, CLASS_MAGE));
    case 3281: /* 3210 */
        return(BlockedCmd(ch, cmd_ch, cmd, 3281, do_north, CLASS_CLERIC));
    case 3289: /* 3211 */
        return(BlockedCmd(ch, cmd_ch, cmd, 3289, do_east, CLASS_THIEF));
    case 3288: /* 3212 */
        return(BlockedCmd(ch, cmd_ch, cmd, 3288, do_east, CLASS_WARRIOR));
    case 3341: /* 3213 */
        return(BlockedCmd(ch, cmd_ch, cmd, 3341, do_south, CLASS_PALADIN));
    case 3371: /* 3243 */
        return(BlockedCmd(ch, cmd_ch, cmd, 3371, do_south, CLASS_ARTIFICER));
    }

    return FALSE;
}

SPECIAL_FUNC(spec_cleric)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    CHAR_DATA *victim;
    char *spell=NULL, buf[MAX_INPUT_LENGTH];
    int sn, lspell, healperc, level, position;

    if (type!=SFT_UPDATE ||
        IS_SET(ch->in_room->room_flags,ROOM_NO_MAGIC) ||
        ch->wait)
        return FALSE;

    level = GetMaxLevel(ch);

    if (!(victim=who_fighting(ch)))
    {
        victim=ch;
        if (GET_HIT(ch) < GET_MAX_HIT(ch)-10) {
            if (level >= 20) {
                spell="heal";
            } else if (level > 12) {
                spell="cure critical";
            } else if (level > 8) {
                spell="cure serious";
            } else {
                spell="cure light";
            }
        } else if (!is_affected(ch,gsn_armor)) {
            spell="armor";
        } else if (!is_affected(ch,gsn_bless)) {
            spell="bless";
        } else if (!is_affected(ch,gsn_aid)) {
            spell="aid";
        } else if (!IS_AFFECTED(ch,AFF_DETECT_MAGIC)) {
            spell="detect magic";
        } else if (!IS_RESIS(ch, RIS_EVIL) && IS_GOOD(ch)) {
            spell="protection from evil";
        } else if (is_affected(ch,gsn_poison)) {
            spell="remove poison";
        } else if (is_affected(ch,gsn_blindness)) {
            spell="cure blind";
        } else if (GET_MOVE(ch) < GET_MAX_MOVE(ch)/2) {
            spell="refresh";
        } else if (GetMaxLevel(ch) > 24) {
            if (!is_affected(ch,gsn_protection_from_fire)) {
                spell="protection from fire";
            } else if (!is_affected(ch,gsn_protection_from_cold)) {
                spell="protection from cold";
            } else if (!is_affected(ch,gsn_protection_from_energy)) {
                spell="protection from energy";
            } else if (!is_affected(ch,gsn_protection_from_electricity)) {
                spell="protection from electricity";
            } else if (!is_affected(ch,gsn_sanctuary) && !IS_AFFECTED(ch, AFF_SANCTUARY)) {
                spell="sanctuary";
            } else if (is_affected(ch,gsn_curse)) {
                spell="remove curse";
            } else if (is_affected(ch,gsn_paralyze)) {
                spell="remove paralysis";
            } else if (summon_if_hating(ch))
                return TRUE;
            else
                return FALSE;
        }
    }
    else
    {
        lspell = number_range(0,GetMaxLevel(ch));
        lspell += GetMaxLevel(ch)/5;
        lspell = UMAX(1, UMIN(GetMaxLevel(ch), lspell));

        if (GET_HIT(ch) < GET_MAX_HIT(ch)/2)
            healperc=7;
        else if (GET_HIT(ch) < GET_MAX_HIT(ch)/4)
            healperc=5;
        else if (GET_HIT(ch) < GET_MAX_HIT(ch)/8)
            healperc=3;
        else
            healperc=7;

        if (number_range(1,healperc+2)>3) {
            if (IS_OUTSIDE(ch) && (ch->in_room->area->weather->precip>0) && (lspell >= 15) &&
                (number_range(0,5)==0)) {
                spell="call lightning";
            } else {
                switch(lspell) {
                case 1:
                case 2:
                case 3:
                    spell="cause light";
                    break;
                case 4:
                case 5:
                case 6:
                    spell="blindness";
                    break;
                case 7:
                    spell="dispel magic";
                    break;
                case 8:
                    spell="poison";
                    break;
                case 9:
                case 10:
                    spell="cause critical";
                    break;
                case 11:
                    if (!IS_SET(victim->immune, RIS_FIRE)) {
                        spell="flamestrike";
                    } else {
                        spell="cause critical";
                    }
                    break;
                case 13:
                case 14:
                case 15:
                case 16: {
                    if (!IS_SET(victim->immune, RIS_FIRE)) {
                        spell="flamestrike";
                    } else if (IS_AFFECTED(victim, AFF_SANCTUARY) &&
                               ( GetMaxLevel(ch) > GetMaxLevel(victim))) {
                        spell="dispel magic";
                    } else {
                        spell="cause critic";
                    }
                    break;
                }
                case 17:
                case 18:
                case 19:
                default:
                    spell="harm";
                    break;
                }
            }
        } else {
            victim=ch;
            if (IS_AFFECTED(ch, AFF_BLIND) && (lspell >= 4) & (number_range(0,3)==0)) {
                spell="cure blind";
            } else if (IS_AFFECTED(ch, AFF_CURSE) && (lspell >= 6) && (number_range(0,6)==0)) {
                spell="remove curse";
            } else if (IS_AFFECTED(ch, AFF_POISON) && (lspell >= 5) && (number_range(0,6)==0)) {
                spell="remove poison";
            } else {
                switch(lspell) {
                case 1:
                case 2:
                    spell="armor";
                    break;
                case 3:
                case 4:
                case 5:
                    spell="cure light";
                    break;
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                    spell="cure serious";
                    break;
                case 11:
                case 12:
                case 13:
                case 14:
                case 15:
                case 16:
                    spell="cure critical";
                    break;
                case 17:
                case 18:
                    spell="heal";
                    break;
                default:
                    spell="sanctuary";
                    break;
                }
            }
        }

    }

    if (!spell)
        return FALSE;

    if (!(sn = skill_lookup(spell)))
    {
        bug("spec_cleric: spell not found %s", spell);
        return FALSE;
    }

    position = ch->position;
    ch->position = POS_STANDING;

    sprintf(buf, "'%s'", spell);
    do_cast(ch, buf);

    ch->position = position;

    return TRUE;
}

SPECIAL_FUNC(spec_mage)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    CHAR_DATA *victim, *cast_on = NULL;
    char *spell=NULL, buf[MAX_INPUT_LENGTH];
    int sn, lspell, level, position;

    if (type!=SFT_UPDATE ||
        IS_SET(ch->in_room->room_flags,ROOM_NO_MAGIC) ||
        ch->wait)
        return FALSE;

    level = GetMaxLevel(ch);

    if (!(victim=who_fighting(ch)))
    {
        victim=ch;
        cast_on=ch;
        if (!is_affected(ch,gsn_shield)) {
            spell="shield";
        } else if (!is_affected(ch,gsn_strength)) {
            spell="strength";
        } else if (!is_affected(ch,gsn_protection_from_evil)) {
            spell="protection from evil";
        } else if (!is_affected(ch,gsn_darkness)) {
            spell="darkness";
        } else if (GetMaxLevel(ch) > 24) {
            if (!is_affected(ch,gsn_armor)) {
                spell="armor";
            } else if (!is_affected(ch,gsn_stone_skin)) {
                spell="stone skin";
            } else if (!is_affected(ch,gsn_minor_invulnerability)) {
                spell="minor invulnerability";
            } else if (!is_affected(ch,gsn_major_invulnerability)) {
                spell="major invulnerability";
            } else if (!is_affected(ch,gsn_fireshield) && !IS_AFFECTED(ch, AFF_FIRESHIELD)) {
                spell="fireshield";
            }
        } else if (summon_if_hating(ch))
            return TRUE;
        else
            return FALSE;
    }
    else
    {
        lspell = number_range(0,GetMaxLevel(ch));
        lspell += GetMaxLevel(ch)/5;
        lspell = UMAX(1, UMIN(GetMaxLevel(ch), lspell));

        if (IS_AFFECTED(ch, AFF_BLIND) && lspell > 15) {
            cast_on=ch;
            spell="cure blind";
        } else if ((IS_AFFECTED(victim, AFF_SANCTUARY) || IS_AFFECTED(victim, AFF_FIRESHIELD)) &&
                   lspell > 10 && GetMaxLevel(ch) > GetMaxLevel(victim)) {
            cast_on=victim;
            spell="dispel magic";
        } else if (GET_HIT(ch) < GET_MAX_HIT(ch)/4 && lspell > 20 &&
                   !IS_ACT_FLAG(ch, ACT_AGGRESSIVE)) {
            cast_on=ch;
            spell="teleport";
        } else if (GET_HIT(ch) > GET_MAX_HIT(ch)/2 &&
                   !IS_ACT_FLAG(ch, ACT_AGGRESSIVE) && number_percent()>50 &&
                   GetMaxLevel(ch) > GetMaxLevel(victim)) {
            cast_on=victim;
            /* non-damaging case */

            if (lspell>8 && lspell<50 && number_percent()<15) {
                spell="web";
            } else if (lspell>5 && lspell<10 && number_percent()<15) {
                spell="weakness";
            } else if (lspell>5 && lspell<10 && number_percent()<13) {
                spell="armor";
            } else if (lspell>12 && lspell<20 && number_percent()<13) {
                spell="curse";
            } else if (lspell>10 && lspell<20 && number_percent()<17) {
                spell="blind";
            } else if (lspell>5 && lspell<40 && number_percent()<17) {
                spell="charm monster";
            }
            else
            {
                /* stuff here */
                switch(lspell) {
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                    spell="monsum one";
                    break;
                case 11:
                case 12:
                case 13:
                    spell="monsum two";
                    break;
                case 14:
                case 15:
                    spell="monsum three";
                    break;
                case 16:
                case 17:
                case 18:
                    spell="monsum four";
                    break;
                case 19:
                case 20:
                case 21:
                case 22:
                    spell="monsum five";
                    break;
                case 23:
                case 24:
                case 25:
                    spell="monsum six";
                    break;
                case 26:
                default:
                    spell="monsum seven";
                    break;
                }
            }
        } else {
            /* damaging case */
            cast_on=victim;
            switch(lspell) {
            case 1:
            case 2:
                spell="magic missile";
                break;
            case 3:
            case 4:
            case 5:
                spell="shocking grasp";
                break;
            case 6:
            case 7:
            case 8:
                if (ch->num_fighting <= 2)
                    spell="web";
                else
                    spell="burning hands";
                break;
            case 9:
            case 10:
                if (ch->num_fighting <= 2)
                    spell="acid blast";
                else
                    spell="ice storm";
                break;
            case 11:
            case 12:
            case 13:
                spell="lightning bolt";
                break;
            case 14:
            case 15:
                spell="teleport";
                break;
            case 16:
            case 17:
            case 18:
            case 19:
                spell="slow";
                break;
            case 20:
            case 21:
            case 22:
                if (IS_EVIL(ch) && !IS_SET(ch->act2, ACT2_MASTER_VAMPIRE))
                {
                    spell="energy drain";
                    break;
                }
            case 23:
            case 24:
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:
                if (ch->num_fighting <= 2)
                    spell="colour spray";
                else
                    spell="cone of cold";
                break;
            case 30:
            case 31:
            case 32:
            case 33:
            case 34:
            case 35:
                spell="fireball";
                break;
            case 36:
            case 37:
                spell="chain lightning";
                break;
            case 38:
                spell="feeblemind";
                break;
            case 39:
                spell="paralyze";
                break;
            case 40:
            case 41:
                if (ch->num_fighting <= 2)
                    spell="meteor swarm";
                else
                    spell="incendiary cloud";
                break;
            default:
                if (ch->num_fighting <= 2)
                    spell="disintegrate";
                else
                    spell="incendiary cloud";
                break;
            }
        }
    }

    if (!spell)
        return FALSE;

    if (!(sn = skill_lookup(spell)))
    {
        bug("spec_mage: spell not found %s", spell);
        return FALSE;
    }

    position = ch->position;
    ch->position = POS_STANDING;

    sprintf(buf, "'%s' %s", spell, cast_on==ch?"self":cast_on?GET_NAME(cast_on):"");
    do_cast(ch, buf);

    ch->position = position;

    return TRUE;
}

SPECIAL_FUNC(spec_troll_thrower)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    CHAR_DATA *vict;

    if (type==SFT_COMMAND)
    {
        if (cmd->do_fun == do_north)
        {
            do_emote(ch, "growls fiercly.");
            return TRUE;
        }
        return FALSE;
    }

    if (!(vict=who_fighting(ch)) || !get_exit(vict->in_room, DIR_EAST))
        return FALSE;

    act(AT_PLAIN, "$n picks you up and throws you east.", ch, NULL, vict, TO_VICT);
    act(AT_PLAIN, "$n picks $N up and throws $S east.", ch, NULL, vict, TO_NOTVICT);
    move_char(vict, get_exit(vict->in_room, DIR_EAST), 0);

    return TRUE;
}


bool StandUp(CHAR_DATA *ch)
{
    if (ch->wait)
        return FALSE;

    if (GET_POS(ch)<=POS_STUNNED || GET_POS(ch)>=POS_FIGHTING)
        return FALSE;

    if (GET_HIT(ch) > (GET_MAX_HIT(ch) / 2))
        act(AT_PLAIN, "$n quickly stands up.", ch, NULL, NULL, TO_ROOM);
    else if (GET_HIT(ch) > (GET_MAX_HIT(ch) / 6))
        act(AT_PLAIN, "$n slowly stands up.", ch, NULL, NULL, TO_ROOM);
    else
        act(AT_PLAIN, "$n gets to $s feet very slowly.", ch, NULL, NULL, TO_ROOM);

    if (who_fighting(ch))
        GET_POS(ch)=POS_FIGHTING;
    else
        GET_POS(ch)=POS_STANDING;

    return TRUE;
}


void MakeNiftyAttack(CHAR_DATA *ch)
{
    CHAR_DATA *fighting;
    int num;

    if (ch->wait)
        return;

    if (GET_POS(ch)!=POS_FIGHTING || !(fighting=who_fighting(ch)))
        return;

    num = number_range(1,4);

    if (num <= 2)
        do_bash(ch, GET_NAME(fighting));
    else if (num == 3)
    {
        if (get_eq_char(ch,WEAR_WIELD) && get_eq_char(fighting,WEAR_WIELD))
            do_disarm(ch, GET_NAME(fighting));
        else
            do_punch(ch, GET_NAME(fighting));
    }
    else
        do_kick(ch, GET_NAME(fighting));

}

bool FighterMove(CHAR_DATA *ch)
{
    CHAR_DATA *cfriend, *foe;

    if (ch->wait)
        return FALSE;

    if (!(foe=who_fighting(ch)))
        return FALSE;

    if (!(cfriend=who_fighting(foe)))
        return FALSE;

    if (GET_RACE(cfriend) == GET_RACE(ch) && GET_HIT(cfriend) < GET_HIT(ch))
        do_rescue(ch, GET_NAME(cfriend));
    else
        MakeNiftyAttack(ch);

    return TRUE;
}

bool FindABetterWeapon(CHAR_DATA *ch)
{
    return FALSE;
}

SPECIAL_FUNC(spec_warrior)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (type!=SFT_UPDATE || !IS_AWAKE(ch))
        return FALSE;

    if (who_fighting(ch))
    {
        if (StandUp(ch))
            return TRUE;

        if (FighterMove(ch))
            return TRUE;

        if (FindABetterWeapon(ch))
            return TRUE;
    }

    return FALSE;
}
SPECIAL_FUNC(spec_ranger)
{ return(spec_warrior(proc,cmd,arg,cmd_ch,type)); }
SPECIAL_FUNC(spec_barbarian)
{ return(spec_warrior(proc,cmd,arg,cmd_ch,type)); }
SPECIAL_FUNC(spec_paladin)
{ return(spec_warrior(proc,cmd,arg,cmd_ch,type)); }

/*
 SPECIAL_FUNC(spec_ranger)
 {
 CHAR_DATA *ch = (CHAR_DATA *)proc;

 if (number_bits(10)==1)
 bug("%d: I'm a ranger!", ch->pIndexData->vnum);

 return FALSE;
 }
 SPECIAL_FUNC(spec_barbarian)
 {
 CHAR_DATA *ch = (CHAR_DATA *)proc;

 if (number_bits(10)==1)
 bug("%d: I'm a barbarian!", ch->pIndexData->vnum);

 return FALSE;
 }
 SPECIAL_FUNC(spec_paladin)
 {
 CHAR_DATA *ch = (CHAR_DATA *)proc;

 if (number_bits(10)==1)
 bug("%d: I'm a paladin!", ch->pIndexData->vnum);

 return FALSE;
 }
 */

#define APU             0
#define NED             1
#define MOE             2
#define MAX_SUPERSHOP   3

struct supershop_items {
    int container;
    int contains;
    int howman;
    int flags;
};

#define SSFLG_OVER21       1
#define SSFLG_SNICKER      2

static struct supershop_items apu_supershop[] =
{
    {3280, 0, 0, 0},
    {3281, 0, 0, 0},
    {3282, 0, 0, 0},
    {3283, 0, 0, 0},
    {3284, 0, 0, 0},
    {3285, 0, 0, 0},
    {3286, 0, 0, 0},
    {3287, 0, 0, 0},
    {  -1,-1,-1,-1},

};
static struct supershop_items ned_supershop[] =
{
    {3258, 0, 0, 0},
    {3259, 0, 0, 0},
    {3260, 0, 0, 0},
    {3261, 0, 0, 0},
    {3262, 0, 0, 0},
    {  -1,-1,-1,-1}
};
static struct supershop_items moe_supershop[] =
{
    {3264, 0, 0, 1 },
    {3266, 0, 0, 1 },
    {3268, 0, 0, 1 },
    {3270, 0, 0, 3 },
    {3272, 0, 0, 3 },
    {3274, 0, 0, 3 },
    {3279, 0, 0, 1 },
    {3276, 0, 0, 0 },
    {3277, 0, 0, 0 },
    {3278, 0, 0, 0 },
    {3275, 0, 0, 0 },
    {3263, 3264, 6, 1 }, /* sixpack */
    {3265, 3266, 6, 1 }, /* sixpack */
    {3267, 3268, 6, 1 }, /* sixpack */
    {3269, 3270, 6, 3 }, /* sixpack */
    {3271, 3272, 6, 3 }, /* sixpack */
    {3273, 3274, 6, 3 }, /* sixpack */
    {  -1,-1,-1,-1 }
};

#define SOLD_SUPERSHOP(x) x==APU?apu_supershop:(x==NED?ned_supershop:(x==MOE?moe_supershop:NULL))

struct _sst_type {
    sh_int close;
    sh_int open;
    sh_int lastcall;
};

static struct _sst_type supershop_times[MAX_SUPERSHOP] =
{
    { 0 ,  4, 23 },
    { 20,  7, 19 },
    { 2 , 14,  1 }
};

#define SS_OPENTIME(x)      supershop_times[x].open
#define SS_CLOSETIME(x)     supershop_times[x].close
#define SS_LASTCALLTIME(x)  supershop_times[x].lastcall

#define SS_CLOSED     0
#define SS_OPEN       1
#define SS_LASTCALL   2

static sh_int supershop_status[MAX_SUPERSHOP] =
{ SS_OPEN, SS_OPEN, SS_OPEN };

#define SS_STATUS(x)  supershop_status[x]

static char *kill_speech[MAX_SUPERSHOP] =
{
    "You little bastard...",
    "Oh Lord, please save me!",
    "Listen to me you little punk, I'm gonna pull your guts out through your mouth and use 'em to string my guitar!"
};
static char *kill_act_vict[MAX_SUPERSHOP] =
{
    "$n pulls out a wand and aims it at you!",
    "$n prays to his God who smites you.  OUCH!",
    "$n grabs you by the neck."
};
static char *kill_act_room[MAX_SUPERSHOP] =
{
    "$n pulls out a wand and aims it at $N.",
    "$n prays to his God who smites $N.",
    "$n grabs $N by the neck."
};
static char *cast_speech[MAX_SUPERSHOP] =
{
    "Silly customer!  You offend my cultural heritage by doing that!",
    "Heathen sorcerer!  I'm going to have to ask you to leave my shop.",
    "Hey.. HEY!  The only magic we practice here is voodoo!"
};
static char *buyclosed_speech[MAX_SUPERSHOP] =
{
    "I cannot sell you things while we are closed.",
    "Sorry neighbor, but we're closed!",
    "Get lost, I'm closed."
};
struct _ss_msgs {
    char *gossip;
    char *shout;
    char *say;
};
static struct _ss_msgs open_speech[MAX_SUPERSHOP] =
{
    {
        "Welcome customers, I am open!",
        "Welcome customers!",
        "Welcome, I'm opening the shop now!"
    },
    {
        "Come on over, we're open for business!",
        "We're open for business!",
        "Wow, did you spend all night in here... you must be exhausted!"
    },
    {
        "Moe's is open for business!",
        "For the best beer in the land, come to Moe's!",
        "We're open for breakfast, come on in!",
    }
};
static struct _ss_msgs close_speech[MAX_SUPERSHOP] =
{
    {
        "Closing for the first time ever!",
        "Closing for the first time ever!",
        "Thank you, come again!"
    },
    {
        "I'm closing up for the night, see you bright and early tomorrow!",
        "The store is closing, see you bright and early tormorrow!",
        "Thanks for coming, and come again!"
    },
    {
        "I'm closed, come back tomorrow.",
        "No more beer for any of you tonight, you'll have to wait till tomorrow.",
        "Get lost, you lousy punks!"
    }
};
static struct _ss_msgs lastcall_speech[MAX_SUPERSHOP] =
{
    {
        "Thank you, come again!",
        "Ohh, but I must close, it is a religious holiday!",
        "Closing for the first time ever!"
    },
    {
        "Come back another time!",
        "The store is closing, come back tomorrow!",
        "Time to close up!"
    },
    {
        "All right you pukes, 1 hour till closing time.",
        "Last call for beer before Moe's closes!",
        "Allright, who's been sucking coins out of the lovetester machine!"
    }
};

bool spec_super_shopkeeper(SPECIAL_FUNC(funcp), void *proc, CMDTYPE *cmd, char *argument, CHAR_DATA *cmd_ch, sh_int type)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    int keeper = -1;

    if ( !IS_AWAKE(ch) )
        return FALSE;

    if (funcp==spec_apu)
        keeper = APU;
    else if (funcp==spec_ned_flanders)
        keeper = NED;
    else if (funcp==spec_moe_bar)
        keeper = MOE;

    if (keeper==-1)
        return FALSE;

    if ( type == SFT_UPDATE )
    {
        if (SS_STATUS(keeper)==SS_CLOSED && time_info.hour == SS_OPENTIME(keeper))
        {
            SS_STATUS(keeper)=SS_OPEN;
            do_unlock(ch, "door");
            do_open(ch, "door");
            if (number_percent()>50) {
                if (*open_speech[keeper].gossip)
                    do_gossip(ch,open_speech[keeper].gossip);
            } else if (number_percent()>90) {
                if (*open_speech[keeper].shout)
                    do_shout(ch,open_speech[keeper].shout);
            } else
            if (*open_speech[keeper].say)
                do_say(ch,open_speech[keeper].say);
            return TRUE;
        }
        if (SS_STATUS(keeper)==SS_OPEN && time_info.hour == SS_LASTCALLTIME(keeper))
        {
            SS_STATUS(keeper)=SS_LASTCALL;
            if (number_percent()>50) {
                if (*lastcall_speech[keeper].gossip)
                    do_gossip(ch,lastcall_speech[keeper].gossip);
            } else if (number_percent()>90) {
                if (*lastcall_speech[keeper].shout)
                    do_shout(ch,lastcall_speech[keeper].shout);
            } else
            if (*lastcall_speech[keeper].say)
                do_say(ch,lastcall_speech[keeper].say);
            return TRUE;
        }
        if (SS_STATUS(keeper)==SS_LASTCALL && time_info.hour == SS_CLOSETIME(keeper))
        {
            SS_STATUS(keeper)=SS_CLOSED;
            if (number_percent()>50) {
                if (*close_speech[keeper].gossip)
                    do_gossip(ch,close_speech[keeper].gossip);
            } else if (number_percent()>90) {
                if (*close_speech[keeper].shout)
                    do_shout(ch,close_speech[keeper].shout);
            } else
            if (*close_speech[keeper].say)
                do_say(ch,close_speech[keeper].say);
            do_close(ch, "door");
            do_lock(ch, "door");
            return TRUE;
        }
        return FALSE;
    }

    if ( ch == cmd_ch )
        return FALSE;

    if ( type == SFT_COMMAND )
    {
        if (cmd->do_fun==do_kill ||
            cmd->do_fun==do_bash ||
            cmd->do_fun==do_kick)
        {
            do_say(ch,kill_speech[keeper]);
            if (!IS_IMMORTAL(cmd_ch))
            {
                int hitsleft;
                hitsleft = dice(2,6) + 6;
                if (hitsleft < GET_HIT(cmd_ch) ) {
                    act(AT_PLAIN, kill_act_room[keeper], ch, NULL, cmd_ch, TO_NOTVICT);
                    act(AT_PLAIN, kill_act_vict[keeper], ch, NULL, cmd_ch, TO_VICT);
                    GET_HIT(cmd_ch) = hitsleft;
                }
                GET_POS(cmd_ch) = POS_SITTING;
            }
            return TRUE;
        }
        if (cmd->do_fun==do_cast ||
            cmd->do_fun==do_use ||
            cmd->do_fun==do_zap ||
            cmd->do_fun==do_recite)
        {
            do_say(ch,cast_speech[keeper]);
            return TRUE;
        }

        if (SS_STATUS(keeper)==SS_CLOSED &&
            (cmd->do_fun==do_buy || cmd->do_fun==do_list))
        {
            do_say(ch,buyclosed_speech[keeper]);
            return TRUE;
        }

        if (cmd->do_fun==do_buy)
        {
            struct supershop_items *scan;
            OBJ_INDEX_DATA *pobj1=NULL, *pobj2;
            OBJ_DATA *obj1;
            char arg[MAX_INPUT_LENGTH];
            int cost=0,x;

            one_argument(argument, arg);
            if (arg[0] == '\0')
            {
                act(AT_SAY, "[$n] says 'Yes, but what do you want to buy?'", ch, NULL, cmd_ch, TO_VICT);
                return TRUE;
            }
            for (scan = SOLD_SUPERSHOP(keeper); scan->container>=0; scan++)
            {
                pobj1 = get_obj_index(scan->container);
                if (!pobj1)
                    continue;
                if (nifty_is_name(arg, pobj1->name))
                    break;
            }
            if (!pobj1 || !nifty_is_name(arg, pobj1->name))
            {
                act(AT_SAY, "[$n] says 'I don't have that.'", ch, NULL, cmd_ch, TO_VICT);
                return TRUE;
            }
            pobj2 = scan->contains ? get_obj_index(scan->contains) : NULL;
            cost = (pobj2 ? (scan->howman * pobj2->cost) : 0)
                + pobj1->cost;
            cost *= 9;
            cost /=10;
            cost++;
            if (GET_MONEY(cmd_ch,pobj1->currtype)<cost)
            {
                act(AT_SAY, "[$n] says 'You don't have enough money.'", ch, NULL, cmd_ch, TO_VICT);
                return TRUE;
            }
            obj1 = create_object(pobj1->ivnum);
            if (!obj1)
            {
                act(AT_SAY, "[$n] says 'I don't have that.'", ch, NULL, cmd_ch, TO_VICT);
                bug("spec_super_shopkeeper: create_object(pobj1) failed");
                return TRUE;
            }
            if (pobj2)
            {
                for (x=0;x<scan->howman;x++)
                {
                    OBJ_DATA *obj2;
                    obj2 = create_object(pobj2->ivnum);
                    obj2 = obj_to_obj(obj2, obj1);
                }
            }
            obj1 = obj_to_char(obj1, cmd_ch);
            GET_MONEY(cmd_ch,pobj1->currtype) -= cost;
            act(AT_PLAIN,"$N buys a $p from $n.", ch, obj1, cmd_ch, TO_NOTVICT);
            act(AT_PLAIN,"You buy a $p from $n.", ch, obj1, cmd_ch, TO_VICT);
            return TRUE;
        }

        if (cmd->do_fun==do_list)
        {
            struct supershop_items *scan;
            OBJ_INDEX_DATA *pobj1, *pobj2;
            int cost=0;

            act(AT_SAY, "[$n] says 'We have the following items:'", ch, NULL, cmd_ch, TO_VICT);
            act(AT_SAY, "$n says something to $N.", ch, NULL, cmd_ch, TO_NOTVICT);
            for (scan = SOLD_SUPERSHOP(keeper); scan->container>=0; scan++)
            {
                pobj1 = get_obj_index(scan->container);
                if (!pobj1)
                    continue;
                pobj2 = scan->contains ? get_obj_index(scan->contains) : NULL;
                cost = (pobj2 ? (scan->howman * pobj2->cost) : 0)
                    + pobj1->cost;
                cost *= 9;
                cost /=10;
                cost++;
                ch_printf(cmd_ch, "%s for %d gold coins.\n\r", pobj1->short_descr, cost);
            }
            return TRUE;
        }
        return FALSE;
    }

    return FALSE;
}

SPECIAL_FUNC(spec_apu)
{ return(spec_super_shopkeeper(spec_apu,proc,cmd,arg,cmd_ch,type)); }
SPECIAL_FUNC(spec_ned_flanders)
{ return(spec_super_shopkeeper(spec_ned_flanders,proc,cmd,arg,cmd_ch,type)); }
SPECIAL_FUNC(spec_moe_bar)
{ return(spec_super_shopkeeper(spec_moe_bar,proc,cmd,arg,cmd_ch,type)); }

SPECIAL_FUNC(spec_monk)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc, *vict;

    if (type!=SFT_UPDATE || !(vict = who_fighting(ch)) || ch->wait)
        return FALSE;

    if (GET_POS(ch) < POS_FIGHTING)
    {
        interpret(ch, "spring");
        return TRUE;
    }

    if (GET_HIT(ch) < GET_MAX_HIT(ch)/20 &&
        GetMaxLevel(vict) > GetMaxLevel(ch))
    {
        interpret(ch, "flee");
        return TRUE;
    }

    if (GetMaxLevel(ch)>30 &&
        number_percent() > 50 &&
        GetMaxLevel(vict) <= GetMaxLevel(ch) &&
        GET_MAX_HIT(vict) < 2*GET_MAX_HIT(ch) &&
        !is_affected(ch, gsn_quivering_palm))
    {
        do_quivering_palm(ch, GET_NAME(vict));
        return TRUE;
    }

    if (get_eq_char(vict, WEAR_WIELD))
    {
        do_disarm(ch, GET_NAME(vict));
        return TRUE;
    }

    do_kick(ch, GET_NAME(vict));

    return TRUE;
}

SPECIAL_FUNC(spec_TreeThrowerMob)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    CHAR_DATA *vict;

    if (type==SFT_COMMAND)
    {
        if (cmd->do_fun == do_north)
        {
	    act(AT_PLAIN, "The trees rustle and block your way.", cmd_ch, NULL, NULL, TO_CHAR);
	    act(AT_PLAIN, "The trees rustle and block $n's way.", cmd_ch, NULL, NULL, TO_ROOM);
            return TRUE;
        }
        return FALSE;
    }

    if ((vict=who_fighting(ch)) && get_exit(vict->in_room, DIR_SOUTH))
    {
        act(AT_PLAIN, "$n picks you up and throws you south.", ch, NULL, vict, TO_VICT);
        act(AT_PLAIN, "$n picks $N up and throws $S south.", ch, NULL, vict, TO_NOTVICT);
        move_char(vict, get_exit(vict->in_room, DIR_EAST), 0);
        return TRUE;
    }

    return FALSE;
}

SPECIAL_FUNC(spec_druid)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    CHAR_DATA *victim;
    char *spell=NULL, buf[MAX_INPUT_LENGTH];
    int sn, lspell, healperc, level, position;

    if (type!=SFT_UPDATE ||
        IS_SET(ch->in_room->room_flags,ROOM_NO_MAGIC) ||
        ch->wait)
        return FALSE;

    level = GetMaxLevel(ch);

    if (!(victim=who_fighting(ch)))
    {
        victim=ch;
        if (GET_HIT(ch) < GET_MAX_HIT(ch)-10) {
            if (level >= 20) {
                spell="heal";
            } else if (level > 12) {
                spell="cure critical";
            } else if (level > 8) {
                spell="cure serious";
            } else {
                spell="cure light";
            }
        } else if (!is_affected(ch,gsn_armor)) {
            spell="armor";
        } else if (!is_affected(ch,gsn_bless)) {
            spell="bless";
        } else if (!is_affected(ch,gsn_aid)) {
            spell="aid";
        } else if (!IS_AFFECTED(ch,AFF_DETECT_MAGIC)) {
            spell="detect magic";
        } else if (!IS_RESIS(ch, RIS_EVIL) && IS_GOOD(ch)) {
            spell="protection from evil";
        } else if (is_affected(ch,gsn_poison)) {
            spell="remove poison";
        } else if (is_affected(ch,gsn_blindness)) {
            spell="cure blind";
        } else if (GET_MOVE(ch) < GET_MAX_MOVE(ch)/2) {
            spell="refresh";
        } else if (GetMaxLevel(ch) > 24) {
            if (!is_affected(ch,gsn_protection_from_fire)) {
                spell="protection from fire";
            } else if (!is_affected(ch,gsn_protection_from_cold)) {
                spell="protection from cold";
            } else if (!is_affected(ch,gsn_protection_from_energy)) {
                spell="protection from energy";
            } else if (!is_affected(ch,gsn_protection_from_electricity)) {
                spell="protection from electricity";
            } else if (is_affected(ch,gsn_curse)) {
                spell="remove curse";
            } else if (is_affected(ch,gsn_paralyze)) {
                spell="remove paralysis";
            } else if (summon_if_hating(ch))
                return TRUE;
        }
    }
    else
    {
        lspell = number_range(0,GetMaxLevel(ch));
        lspell += GetMaxLevel(ch)/5;
        lspell = UMAX(1, UMIN(GetMaxLevel(ch), lspell));

        if (GET_HIT(ch) < GET_MAX_HIT(ch)/2)
            healperc=7;
        else if (GET_HIT(ch) < GET_MAX_HIT(ch)/4)
            healperc=5;
        else if (GET_HIT(ch) < GET_MAX_HIT(ch)/8)
            healperc=3;
        else
            healperc=7;

        if (number_range(1,healperc+2)>3) {
            if (IS_OUTSIDE(ch) && (ch->in_room->area->weather->precip>0) && (lspell >= 15) &&
                (number_range(0,5)==0)) {
                spell="call lightning";
            } else {
                switch(lspell) {
                case 1:
                case 2:
                case 3:
                    spell="cause light";
                    break;
                case 4:
                case 5:
                case 6:
                    spell="blindness";
                    break;
                case 7:
                    spell="dispel magic";
                    break;
                case 8:
                    spell="poison";
                    break;
                case 9:
                case 10:
                    spell="cause critical";
                    break;
                case 11:
                    spell="flamestrike";
                    break;
                case 13:
                case 14:
                case 15:
                case 16: {
                    if (!IS_SET(victim->immune, RIS_FIRE)) {
                        spell="flamestrike";
                    } else if (IS_AFFECTED(victim, AFF_SANCTUARY) &&
                               ( GetMaxLevel(ch) > GetMaxLevel(victim))) {
                        spell="dispel magic";
                    } else {
                        spell="cause critic";
                    }
                    break;
                }
                case 17:
                case 18:
                case 19:
                default:
                    spell="harm";
                    break;
                }
            }
        } else {
            victim=ch;
            if (IS_AFFECTED(ch, AFF_BLIND) && (lspell >= 4) & (number_range(0,3)==0)) {
                spell="cure blind";
            } else if (IS_AFFECTED(ch, AFF_CURSE) && (lspell >= 6) && (number_range(0,6)==0)) {
                spell="remove curse";
            } else if (IS_AFFECTED(ch, AFF_POISON) && (lspell >= 5) && (number_range(0,6)==0)) {
                spell="remove poison";
            } else {
                switch(lspell) {
                case 1:
                case 2:
                    spell="armor";
                    break;
                case 3:
                case 4:
                case 5:
                    spell="cure light";
                    break;
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                    spell="cure serious";
                    break;
                case 11:
                case 12:
                case 13:
                case 14:
                case 15:
                case 16:
                    spell="cure critical";
                    break;
                case 17:
                case 18:
                    spell="heal";
                    break;
                default:
                    spell="sanctuary";
                    break;
                }
            }
        }

    }

    if (!spell)
        return FALSE;

    if (!(sn = skill_lookup(spell)))
    {
        bug("spec_druid: spell not found %s", spell);
        return FALSE;
    }

    position = ch->position;
    ch->position = POS_STANDING;

    sprintf(buf, "'%s'", spell);
    do_cast(ch, buf);

    ch->position = position;

    return TRUE;
}

#define CS_WHITE 0
#define CS_BLACK 1

int side = CS_WHITE;  /* to avoid having to pass side with each function call */

#define CS_IS_BLACK(piece)  (IS_NPC(piece) && (piece)->vnum >= 1400 && (piece)->vnum <= 1415)
#define CS_IS_WHITE(piece)  (IS_NPC(piece) && (piece)->vnum >= 1448 && (piece)->vnum <= 1463)
#define CS_IS_PIECE(piece)  ((CS_IS_WHITE(piece)) || (CS_IS_BLACK(piece)))
#define CS_IS_ENEMY(piece)  (side?CS_IS_WHITE(piece):CS_IS_BLACK(piece))
#define CS_IS_FRIEND(piece) (side?CS_IS_BLACK(piece):CS_IS_WHITE(piece))
#define CS_ON_BOARD(room)   ((room)->vnum >= 1400 && (room)->vnum <= 1463)
#define CS_FORWARD          (side?DIR_SOUTH:DIR_NORTH)
#define CS_BACK             (side?DIR_NORTH:DIR_SOUTH)
#define CS_LEFT             (side?DIR_EAST:DIR_WEST)
#define CS_RIGHT            (side?DIR_WEST:DIR_EAST)

ROOM_INDEX_DATA *forward_square(ROOM_INDEX_DATA *room)
{
    EXIT_DATA *dest;

    if (!room)
        return NULL;

    if ((dest = get_exit(room, CS_FORWARD)) && CS_ON_BOARD(dest->to_room))
        return dest->to_room;

    return NULL;
}

ROOM_INDEX_DATA *back_square(ROOM_INDEX_DATA *room)
{
    EXIT_DATA *dest;

    if (!room)
        return NULL;

    if ((dest = get_exit(room, CS_BACK)) && CS_ON_BOARD(dest->to_room))
        return dest->to_room;

    return NULL;
}

ROOM_INDEX_DATA *left_square(ROOM_INDEX_DATA *room)
{
    EXIT_DATA *dest;

    if (!room)
        return NULL;

    if ((dest = get_exit(room, CS_LEFT)) && CS_ON_BOARD(dest->to_room))
        return dest->to_room;

    return NULL;

}

ROOM_INDEX_DATA *right_square(ROOM_INDEX_DATA *room)
{
    EXIT_DATA *dest;

    if (!room)
        return NULL;

    if ((dest = get_exit(room, CS_RIGHT)) && CS_ON_BOARD(dest->to_room))
        return dest->to_room;

    return NULL;
}

ROOM_INDEX_DATA *forward_left_square(ROOM_INDEX_DATA *room)
{
    return left_square(forward_square(room));
}

ROOM_INDEX_DATA *forward_right_square(ROOM_INDEX_DATA *room)
{
    return right_square(forward_square(room));
}

ROOM_INDEX_DATA *back_right_square(ROOM_INDEX_DATA *room)
{
    return right_square(back_square(room));
}

ROOM_INDEX_DATA *back_left_square(ROOM_INDEX_DATA *room)
{
    return left_square(back_square(room));
}

CHAR_DATA *square_contains_enemy(ROOM_INDEX_DATA *square)
{
    CHAR_DATA *i;

    for (i = square->first_person; i; i = i->next_in_room)
        if (CS_IS_ENEMY(i))
            return i;

    return NULL;
}

int square_contains_friend(ROOM_INDEX_DATA *square)
{
    CHAR_DATA *i;

    for (i = square->first_person; i; i = i->next_in_room)
        if (CS_IS_FRIEND(i))
            return TRUE;

    return FALSE;
}

int square_empty(ROOM_INDEX_DATA *square)
{
    CHAR_DATA *i;

    for (i = square->first_person; i; i = i->next_in_room)
        if (CS_IS_PIECE(i))
            return FALSE;

    return TRUE;
}

SPECIAL_FUNC(spec_chess_game)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    ROOM_INDEX_DATA *rp = NULL, *crp = ch->in_room;
    CHAR_DATA *ep = NULL;
    int move_dir = 0, move_amount = 0, move_found = FALSE;
    int c = 0;

    if (type!=SFT_UPDATE || !IS_AWAKE(ch))
        return FALSE;

    /* keep original fighter() spec_proc for kings and knights */
    if (ch->fighting)
    {
        switch (ch->vnum)
        {
        case 1401: case 1404: case 1406: case 1457: case 1460: case 1462:
            return spec_warrior(proc, cmd, arg, cmd_ch, type);
        default:
            return FALSE;
        }
    }

    if (!crp || !CS_ON_BOARD(crp))
        return FALSE;

    if (side == CS_WHITE && CS_IS_BLACK(ch))
        return FALSE;

    if (side == CS_BLACK && CS_IS_WHITE(ch))
        return FALSE;

    if (number_range(0,15))
        return FALSE;

    switch (ch->vnum)
    {
    case 1408: case 1409: case 1410: case 1411:  /* black pawns */
    case 1412: case 1413: case 1414: case 1415:
    case 1448: case 1449: case 1450: case 1451:  /* white pawns */
    case 1452: case 1453: case 1454: case 1455:
        move_dir = number_range(0,3);
        switch (move_dir)
        {
        case 0: rp = forward_left_square(crp);  break;
        case 1: rp = forward_right_square(crp); break;
        case 2: rp = forward_square(crp);       break;
        case 3:
            if (ch->in_room->vnum == ch->vnum)
            {
                rp = forward_square(crp);
                if (rp && square_empty(rp))
                {
                    crp = rp;
                    rp = forward_square(crp);
                }
            }
        }

        if (rp && !square_contains_friend(rp))
        {
            ep = square_contains_enemy(rp);
            if ((move_dir <= 1 && ep) || (move_dir > 1 && !ep))
                move_found = TRUE;
        }
        break;

    case 1400:  /* black rooks */
    case 1407:
    case 1456:  /* white rooks */
    case 1463:
        move_dir = number_range(0,3);
        move_amount = number_range(1,7);
        for (c = 0; c < move_amount; c++)
        {
            switch(move_dir)
            {
            case 0: rp = forward_square(crp);  break;
            case 1: rp = back_square(crp);     break;
            case 2: rp = right_square(crp);    break;
            case 3: rp = left_square(crp);
            }
            if (rp && !square_contains_friend(rp))
            {
                move_found = TRUE;
                if ((ep = square_contains_enemy(rp)))
                    c = move_amount;
                else
                    crp = rp;
            }
            else
            {
                c = move_amount;
                rp = crp;
            }
        }
        break;

    case 1401:  /* black knights */
    case 1406:
    case 1457:  /* white knights */
    case 1462:
        move_dir = number_range(0,7);
        switch(move_dir)
        {
        case 0: rp = forward_left_square(forward_square(crp));  break;
        case 1: rp = forward_right_square(forward_square(crp)); break;
        case 2: rp = forward_right_square(right_square(crp));   break;
        case 3: rp = back_right_square(right_square(crp));      break;
        case 4: rp = back_right_square(back_square(crp));       break;
        case 5: rp = back_left_square(back_square(crp));        break;
        case 6: rp = back_left_square(left_square(crp));        break;
        case 7: rp = forward_left_square(left_square(crp));     break;
        }
        if (rp && !square_contains_friend(rp))
        {
            move_found = TRUE;
            ep = square_contains_enemy(rp);
        }
        break;

    case 1402:  /* black bishops */
    case 1405:
    case 1458:  /* white bishops */
    case 1461:
        move_dir = number_range(0,3);
        move_amount = number_range(1,7);
        for (c = 0; c < move_amount; c++)
        {
            switch(move_dir)
            {
            case 0: rp = forward_left_square(crp);  break;
            case 1: rp = forward_right_square(crp); break;
            case 2: rp = back_right_square(crp);    break;
            case 3: rp = back_left_square(crp);     break;
            }
            if (rp && !square_contains_friend(rp))
            {
                move_found = TRUE;
                if ((ep = square_contains_enemy(rp)))
                    c = move_amount;
                else
                    crp = rp;
            }
            else
            {
                c = move_amount;
                rp = crp;
            }
        }
        break;

    case 1403:  /* black queen */
    case 1459:  /* white queen */
        move_dir = number_range(0,7);
        move_amount = number_range(1,7);
        for (c = 0; c < move_amount; c++)
        {
            switch(move_dir)
            {
            case 0: rp = forward_left_square(crp);  break;
            case 1: rp = forward_square(crp);       break;
            case 2: rp = forward_right_square(crp); break;
            case 3: rp = right_square(crp);         break;
            case 4: rp = back_right_square(crp);    break;
            case 5: rp = back_square(crp);          break;
            case 6: rp = back_left_square(crp);     break;
            case 7: rp = left_square(crp);
            }
            if (rp && !square_contains_friend(rp))
            {
                move_found = TRUE;
                if ((ep = square_contains_enemy(rp)))
                    c = move_amount;
                else
                    crp = rp;
            }
            else
            {
                c = move_amount;
                rp = crp;
            }
        }
        break;

    case 1404:  /* black king */
    case 1460:  /* white king */
        move_dir = number_range(0,7);
        switch (move_dir)
        {
        case 0: rp = forward_left_square(crp);  break;
        case 1: rp = forward_square(crp);       break;
        case 2: rp = forward_right_square(crp); break;
        case 3: rp = right_square(crp);         break;
        case 4: rp = back_right_square(crp);    break;
        case 5: rp = back_square(crp);          break;
        case 6: rp = back_left_square(crp);     break;
        case 7: rp = left_square(crp);
        }
        if (rp && !square_contains_friend(rp))
        {
            move_found = TRUE;
            ep = square_contains_enemy(rp);
        }
        break;
    }

    if (move_found && rp)
    {
        do_emote(ch, "leaves the room.");
        char_from_room(ch);
        char_to_room(ch, rp);
        do_emote(ch, "has arrived.");

        if (ep)
        {
            if (side)
                switch(number_range(0,3))
                {
                case 0:
                    do_emote(ch, "grins evilly and says, 'ONLY EVIL shall rule!'");
                    break;
                case 1:
                    do_emote(ch, "leers cruelly and says, 'You will die now!'");
                    break;
                case 2:
                    do_emote(ch, "issues a bloodcurdling scream.");
                    break;
                case 3:
                    do_emote(ch, "glares with black anger.");
                }
            else
                switch(number_range(0,3))
                {
                case 0:
                    do_emote(ch, "glows an even brighter pristine white.");
                    break;
                case 1:
                    do_emote(ch, "chants a prayer and begins battle.");
                    break;
                case 2:
                    do_emote(ch, "says, 'Black shall lose!");
                    break;
                case 3:
                    do_emote(ch, "shouts, 'For the Flame! The Flame!'");
                }
            multi_hit(ch, ep, TYPE_UNDEFINED);
        }
        side = (side + 1) % 2;
        return TRUE;
    }
    return FALSE;
}

#define SAM_WAITING 0
#define SAM_SHOUTGREET 1
#define SAM_LOVESHOUT 2
#define SAM_NOTHING 9
SPECIAL_FUNC(spec_annoy_morbus)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    CHAR_DATA *vict = NULL;
    char vname[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];

    if (type == SFT_COMMAND && GetMaxLevel(cmd_ch)<(MAX_LEVEL-1))
    {
        if (cmd->do_fun == do_slay ||
            cmd->do_fun == do_kill)
        {
            sprintf(buf, "Help HELP!!!  I'm being attacked by %s!!", PERS(cmd_ch,ch));
            do_shout(ch, buf);
            do_emote(ch, "happily hops around screaming like a stuck pig!");
            return TRUE;
        }
        if (cmd->do_fun == do_force)
        {
            sprintf(buf, "%s just tried to force me to do something naughty!  That's not nice!", PERS(cmd_ch,ch));
            do_shout(ch, buf);
            return TRUE;
        }
        if (cmd->do_fun == do_purge)
        {
            sprintf(buf, "I love you happy fuzzy, do you love me too?");
            do_shout(cmd_ch, buf);
            ch->tempnum = SAM_LOVESHOUT;
            return TRUE;
        }
        if (cmd->do_fun == do_cast)
        {
            do_say(ch, "oooOOOooo MAGIC!!!  Can I see???");
            do_emote(ch, "hops happily!");
            act(AT_MAGIC, "$n disrupts your spell.", ch, NULL, cmd_ch, TO_VICT);
            act(AT_MAGIC, "$n disrupts $N's spell.", ch, NULL, cmd_ch, TO_NOTVICT);
            return TRUE;
	}
	if (cmd->do_fun == do_copyover ||
	    cmd->do_fun == do_reboot ||
	    cmd->do_fun == do_shutdown)
	{
            do_shout(ch, "Woohoo!");
            do_emote(ch, "hops happily!");
            return TRUE;
	}
	if (cmd->do_fun == do_mset &&
	    (!str_cmp(arg, "spec") || !str_cmp(arg, "name")))
	{
            do_say(ch, "Awwww, you think I'm special!");
            do_emote(ch, "hops happily!");
            return TRUE;
	}
	if (number_percent()>90)
	{
            act(AT_MAGIC, "$n happily bumps into you.", ch, NULL, cmd_ch, TO_VICT);
            act(AT_MAGIC, "$n happily bumps into $N.", ch, NULL, cmd_ch, TO_NOTVICT);
	    return TRUE;
	}
        return FALSE;
    }

    if (type != SFT_UPDATE)
        return FALSE;

    one_argument(ch->name, vname);

    for (vict = first_char; vict; vict = vict->next)
        if (!IS_NPC(vict) && GET_CON_STATE(vict) == CON_PLAYING &&
            !str_cmp(GET_NAME(vict), vname))
            break;

    if (!vict || str_cmp(GET_NAME(vict), vname))
    {
        if (number_range(1,10) == 1)
        {
            sprintf(buf, "Damn, I cant find %s anywhere!", vname);
            do_say(ch, buf);
        }
        ch->tempnum = SAM_WAITING;
        return TRUE;
    }

    if (ch->tempnum == SAM_WAITING)
    {
        sprintf(buf, "%s! You've come home to me!", vname);
        do_shout(ch, buf);
        ch->tempnum = SAM_SHOUTGREET;
        return TRUE;
    }
    else if (ch->tempnum == SAM_LOVESHOUT)
    {
        do_shout(ch, "Yes, I do love you, with all my fuzzy little happy heart!");
        ch->tempnum = SAM_NOTHING;
        return TRUE;
    }

    if (GetMaxLevel(vict) > MAX_LEVEL-1)
    {
        if (!number_range(0,29))
        {
            sprintf(buf, "Some fool thinks im gonna play with %s.", PERS(vict,ch));
            do_shout(ch, buf);
        }
        return TRUE;
    }

    if (GET_HIT(ch) < (GET_MAX_HIT(ch)/2))
    {
        GET_HIT(ch) = GET_MAX_HIT(ch);
        act(AT_PLAIN, "$n looks much healthier now.", ch, NULL, NULL, TO_ROOM);
        return TRUE;
    }

    if (ch->in_room != vict->in_room)
    {
        sprintf(buf, "$n takes off to find %s.", PERS(vict,ch));
        act(AT_PLAIN, buf, ch, NULL, NULL, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, vict->in_room);
        sprintf(buf, "$n has arrived, ready to get %s!", PERS(vict,ch));
        act(AT_PLAIN, buf, ch, NULL, vict, TO_NOTVICT);
        sprintf(buf, "You feel strange, but the feeling fades.");
        act(AT_PLAIN, buf, ch, NULL, vict, TO_VICT);
        return TRUE;
    }

    if (GET_POS(vict) == POS_SLEEPING)
    {
        sprintf(buf, "Shhhhhh, %s is sleeping...", PERS(vict,ch));
        do_say(ch, buf);
        do_emote(ch, "puts his finger to his mouth.");
    }


    switch (number_range(1, 35))
    {
    case 1:
        sprintf(buf,"shout Mmmmmmmmm forbidden donut, uhhhhhhh!");
        interpret(vict, buf);
        return TRUE;
    case 2:
        sprintf(buf, "shout Nine out of Ten immortals agree! Happy Fuzzies Kick Ass!");
        interpret(vict, buf);
        return TRUE;
    case 3:
        sprintf(buf, "shout Wow! How about this lag... aint it great!");
        interpret(vict, buf);
        return TRUE;
    case 4:
        sprintf(buf, "fart");
        interpret(vict, buf);
        return TRUE;
    case 5:
        sprintf(buf, "shout I love playing with %s!", PERS(vict,ch));
        interpret(ch, buf);
        return TRUE;
    case 6:
        sprintf(buf, "shout I love my Happy Fuzzy, do you love my Happy Fuzzy too?");
        interpret(vict, buf);
        return TRUE;
    default:
        return FALSE;
    }
}


SPECIAL_FUNC(spec_blink)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    int sn = 0;

    if (type!=SFT_VIO_UPDATE || !IS_AWAKE(ch) || !(sn = skill_lookup("teleport")))
        return FALSE;

    if (GET_HIT(ch) < (GET_MAX_HIT(ch) / 3))
    {
        act(AT_MAGIC, "You blink.", ch, NULL, NULL, TO_CHAR);
        act(AT_MAGIC, "$n blinks.", ch, NULL, NULL, TO_ROOM);
        spell_teleport(sn, BestSkLv(ch, sn), ch, ch );
        return TRUE;
    }

    return FALSE;
}

bool generic_touch_caster(CHAR_DATA *ch, CHAR_DATA *tar, int sn)
{
    SKILLTYPE *skill = NULL;

    if (tar->in_room != ch->in_room ||
        IS_IMMUNE(ch, RIS_EVIL) ||
        IS_RESIS(ch, RIS_EVIL) ||
        IS_AFFECTED(tar, AFF_SANCTUARY) ||
        HitOrMiss(ch, tar, CalcThaco(ch)) ||
        IS_AFFECTED(tar, AFF_PARALYSIS) ||
        char_died(tar) ||
        !(skill=get_skilltype(sn)))
        return FALSE;

    act(AT_DANGER, "$n touches $N!",  ch, NULL, tar, TO_NOTVICT);
    act(AT_DANGER, "$n touches you!", ch, NULL, tar, TO_VICT);
    (*skill->spell_fun) ( sn, BestSkLv(ch, sn), ch, tar );

    return TRUE;
}

SPECIAL_FUNC(spec_ghoul)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (type != SFT_VIO_UPDATE || !IS_AWAKE(ch))
        return FALSE;

    return generic_touch_caster(ch, cmd_ch, gsn_paralyze);
}

SPECIAL_FUNC(spec_CarrionCrawler)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    CHAR_DATA *tar =  cmd_ch;

    if (!IS_AWAKE(ch))
        return FALSE;

    if (type==SFT_VIO_UPDATE ||
        (type==SFT_UPDATE && (tar = race_align_hatee(ch))))
        return generic_touch_caster(ch, tar, gsn_paralyze);

    return FALSE;
}

SPECIAL_FUNC(spec_vampire)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (type != SFT_VIO_UPDATE || !IS_AWAKE(ch))
        return FALSE;

    return generic_touch_caster(ch, cmd_ch, gsn_energy_drain);
}

SPECIAL_FUNC(spec_wraith)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    CHAR_DATA *tar = cmd_ch;

    if (!IS_AWAKE(ch))
        return FALSE;

    if (type==SFT_VIO_UPDATE ||
        (type==SFT_UPDATE && (tar = race_align_hatee(ch))))
        return generic_touch_caster(ch, tar, gsn_energy_drain);

    return FALSE;
}

SPECIAL_FUNC(spec_shadow)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (type != SFT_VIO_UPDATE || !IS_AWAKE(ch) )
        return FALSE;

    return generic_touch_caster(ch, cmd_ch, number_percent()>50?gsn_chill_touch:gsn_weakness);
}

SPECIAL_FUNC(spec_geyser)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (type != SFT_UPDATE || !IS_AWAKE(ch) || number_range(0,6))
        return FALSE;

    /*cast_geyser( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, 0, 0);*/
    send_to_area("The volcano rumbles slightly.\n\r", ch->in_room->area);

    return TRUE;
}

SPECIAL_FUNC(spec_tormentor)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (type != SFT_COMMAND)
        return FALSE;

    if (IS_IMMORTAL(cmd_ch) || IS_NPC(cmd_ch))
        return FALSE;

    act(AT_DYING, "You savagely beat $N.", ch, NULL, cmd_ch, TO_CHAR);
    act(AT_DYING, "$n savagely beats you.", ch, NULL, cmd_ch, TO_VICT);
    act(AT_DYING, "$n savagely beats $N.", ch, NULL, cmd_ch, TO_NOTVICT);
    return TRUE;
}

SPECIAL_FUNC(spec_Drow)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    int sn = 0;

    if (type == SFT_VIO_UPDATE)
    {
        if (!is_affected(cmd_ch,gsn_darkness) && number_percent()>25)
        {
            act(AT_MAGIC, "$n drops a globe of darkness around $N.", ch, NULL, cmd_ch, TO_NOTVICT);
            send_to_char("Suddenly, you plunge into darkness and cannot see!\n\r", cmd_ch);
            sn = gsn_darkness;
        }
        else if (!is_affected(cmd_ch,gsn_faerie_fire) && number_percent()>50)
        {
            act(AT_MAGIC, "$n points at $N, who is outlined by small flames.", ch, NULL, cmd_ch, TO_NOTVICT);
            act(AT_MAGIC, "$n points at you, small flames ignite all over you.", ch, NULL, cmd_ch, TO_VICT);
            if (number_percent()<10)
                send_to_char("Strangely, the flames don't hurt.", ch);
            sn = gsn_faerie_fire;
        }

        if (sn)
        {
            (*skill_table[sn]->spell_fun) ( sn, BestSkLv(ch, sn), ch, cmd_ch );
            return TRUE;
        }
        return FALSE;
    }

    if (type == SFT_UPDATE)
    {
        if ((IS_UNDERGROUND(ch) ||
            room_is_dark(ch->in_room)) &&
            !is_affected(ch,gsn_darkness))
        {
            act(AT_MAGIC, "$n uses $s innate powers of darkness.", ch, NULL, NULL, TO_ROOM);
            sn = gsn_darkness;
        }
        else if (ch->hunting && !is_affected(ch,gsn_fly))
        {
            act(AT_MAGIC, "$n uses $s innate powers of levitation.", ch, NULL, NULL, TO_ROOM);
            sn = gsn_fly;
        }

        if (sn)
        {
            (*skill_table[sn]->spell_fun) ( sn, BestSkLv(ch, sn), ch, ch );
            return TRUE;
        }
        return FALSE;
    }

    return FALSE;
}

SPECIAL_FUNC(spec_banana)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (type != SFT_COMMAND)
        return FALSE;

    if (cmd->do_fun == do_north ||
        cmd->do_fun == do_south ||
        cmd->do_fun == do_east ||
        cmd->do_fun == do_west ||
        cmd->do_fun == do_northeast ||
        cmd->do_fun == do_northwest ||
        cmd->do_fun == do_southeast ||
        cmd->do_fun == do_southwest ||
        cmd->do_fun == do_up ||
        cmd->do_fun == do_down)
    {
        if (GET_POS(cmd_ch) != POS_STANDING ||
            IS_IMMORTAL(cmd_ch) ||
            saves_para_petri(GetMaxLevel(cmd_ch), cmd_ch))
            return FALSE;

        act(AT_DYING, "$N steps on you and falls to the ground.", ch, NULL, cmd_ch, TO_CHAR);
        act(AT_DYING, "As you try to leave, you slip on a banana.", ch, NULL, cmd_ch, TO_VICT);
        act(AT_DYING, "$N tries to leave, but slips on a banana and falls.", ch, NULL, cmd_ch, TO_NOTVICT);
        cmd_ch->position = POS_SITTING;
        return TRUE;
    }

    return FALSE;
}

#define NN_LOOSE  0
#define NN_FOLLOW 1
#define NN_STOP   2
SPECIAL_FUNC(spec_NudgeNudge)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    CHAR_DATA *vch, *vict = NULL;

    if (type!=SFT_UPDATE || !IS_AWAKE(ch) || ch->fighting)
        return FALSE;

    if ( !ch->master )
        ch->tempnum = NN_LOOSE;

    switch(ch->tempnum)
    {
    case NN_LOOSE:
        for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
            if ( !IS_NPC(vch) && number_bits(1) == 0 )
            {
                vict = vch;
                break;
            }
        if (!vict)
            return FALSE;
        if ( circle_follow( ch, vict ) )
            return FALSE;
        if ( ch->master )
            stop_follower( ch );
        add_follower( ch, vict );

        ch->tempnum = NN_FOLLOW;

        do_say(ch, "Good Evenin' Squire!");
        act(AT_PLAIN, "$n nudges you.", ch, 0, ch->master, TO_VICT);
        act(AT_PLAIN, "$n nudges $N.", ch, 0, ch->master, TO_NOTVICT);
        break;
    case NN_FOLLOW:
        switch(number_range(0,20))
        {
        case 0:
            do_say(ch, "Is your wife a goer?  Know what I mean, eh?");
            act(AT_PLAIN, "$n nudges you.", ch, 0, ch->master, TO_VICT);
            act(AT_PLAIN, "$n nudges $N.", ch, 0, ch->master, TO_NOTVICT);
            break;
        case 1:
            act(AT_PLAIN, "$n winks at you.", ch, 0, ch->master, TO_VICT);
            act(AT_PLAIN, "$n winks at you.", ch, 0, ch->master, TO_VICT);
            act(AT_PLAIN, "$n winks at $N.", ch, 0, ch->master, TO_NOTVICT);
            act(AT_PLAIN, "$n winks at $N.", ch, 0, ch->master, TO_NOTVICT);
            act(AT_PLAIN, "$n nudges you.", ch, 0, ch->master, TO_VICT);
            act(AT_PLAIN, "$n nudges $N.", ch, 0, ch->master, TO_NOTVICT);
            act(AT_PLAIN, "$n nudges you.", ch, 0, ch->master, TO_VICT);
            act(AT_PLAIN, "$n nudges $N.", ch, 0, ch->master, TO_NOTVICT);
            do_say(ch, "Say no more!  Say no MORE!");
            break;
        case 2:
            do_say(ch, "You been around, eh?");
            do_say(ch, "...I mean you've ..... done it, eh?");
            act(AT_PLAIN, "$n nudges you.", ch, 0, ch->master, TO_VICT);
            act(AT_PLAIN, "$n nudges $N.", ch, 0, ch->master, TO_NOTVICT);
            act(AT_PLAIN, "$n nudges you.", ch, 0, ch->master, TO_VICT);
            act(AT_PLAIN, "$n nudges $N.", ch, 0, ch->master, TO_NOTVICT);
            break;
        case 3:
            do_say(ch, "A nod's as good as a wink to a blind bat, eh?");
            act(AT_PLAIN, "$n nudges you.", ch, 0, ch->master, TO_VICT);
            act(AT_PLAIN, "$n nudges $N.", ch, 0, ch->master, TO_NOTVICT);
            act(AT_PLAIN, "$n nudges you.", ch, 0, ch->master, TO_VICT);
            act(AT_PLAIN, "$n nudges $N.", ch, 0, ch->master, TO_NOTVICT);
            break;
        case 4:
            do_say(ch, "You're WICKED, eh!  WICKED!");
            act(AT_PLAIN, "$n winks at you.", ch, 0, ch->master, TO_VICT);
            act(AT_PLAIN, "$n winks at you.", ch, 0, ch->master, TO_VICT);
            act(AT_PLAIN, "$n winks at $N.", ch, 0, ch->master, TO_NOTVICT);
            act(AT_PLAIN, "$n winks at $N.", ch, 0, ch->master, TO_NOTVICT);
            break;
        case 5:
            do_say(ch, "Wink. Wink.");
            break;
        case 6:
            do_say(ch, "Nudge. Nudge.");
            break;
        case 7:
        case 8:
            ch->tempnum = NN_STOP;
            return FALSE;
            break;
        default:
            return FALSE;
            break;
        }
        break;
    case NN_STOP:
        /*
         **  Stop following
         */
        do_say(ch, "Evening, Squire");
        stop_follower(ch);
        ch->tempnum = NN_LOOSE;
        break;
    default:
        ch->tempnum = NN_LOOSE;
        return FALSE;
        break;
    }

    return TRUE;
}

bool first_ringie_in_room(CHAR_DATA *ch)
{
    CHAR_DATA *vch, *first;

    if (!IS_NPC(ch))
        return FALSE;

    first = ch;

    for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
    {
        if (!IS_NPC(vch) || ch->pIndexData != vch->pIndexData)
            continue;

        if (vch->unum < first->unum)
            first = vch;
    }

    if (first == ch)
        return TRUE;

    return FALSE;
}

SPECIAL_FUNC(spec_Ringwraith)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc, *victim;
    OBJ_DATA *ring, *obj;
    ROOM_INDEX_DATA *ringroom = NULL;
    char buf[MAX_INPUT_LENGTH];

    if (ch->fighting)
    {
        if (GET_POS(ch)<POS_FIGHTING)
            do_stand(ch, "");
        else
            spec_wraith(proc,cmd,arg,cmd_ch,type);

        return FALSE;
    }

    if (type!=SFT_UPDATE || !IS_AWAKE(ch))
        return FALSE;

    if (!first_ringie_in_room(ch))
        return FALSE;

    if (!(ring = get_obj_world(ch, "one-ring")))
        return FALSE;

    obj = ring->in_obj;
    while (obj && obj->in_obj)
        obj = obj->in_obj;
    if (!obj)
        obj = ring;
    if (obj->in_room)
        ringroom = obj->in_room;
    else if (obj->carried_by)
        ringroom = obj->carried_by->in_room;

    if (!ringroom)
    {
        bug("spec_Ringwraith: one ring is nowhere");
        return FALSE;
    }

        /* track the ring */
    if (ch->in_room != ringroom)
    {
        EXIT_DATA *pexit;
        int dir;

        if ((dir = find_first_step(ch->in_room, ringroom, 500))<0)
            return FALSE;

        if ( (pexit=get_exit(ch->in_room, dir)) == NULL )
        {
            bug( "spec_Ringwraith: lost exit?" );
            return FALSE;
        }

        move_char(ch, pexit, FALSE);

        if (char_died(ch))
            return TRUE;

        if (ch->in_room == ringroom && number_percent()<10)
            interpret(ch, "cackle");

        return TRUE;
    }

    /* the ring is in the same room! */
    if ((victim = obj->carried_by))
    {
        if (victim==ch)
        {
            obj_from_char( ring );
            extract_obj( ring );
            interpret(ch, "grin");
            return TRUE;
        }
        if (victim->spec_fun == ch->spec_fun)
            return FALSE;

        switch (ch->tempnum)
        {
        case 0:
            do_wake(ch, GET_NAME(victim));
            sprintf(buf, "%s, give me The Ring.", PERS(victim, ch));
            do_say(ch, buf);
            if ( IS_ROOM_FLAG( ch->in_room, ROOM_SILENCE ) )
            {
                sprintf(buf, "poke %s", GET_NAME(victim));
                interpret(ch, buf);
            }
            ch->tempnum++;
            return TRUE;
            break;
        case 1:
        case 2:
            ch->tempnum++;
            return TRUE;
            break;
        case 3:
            if (IS_NPC(victim) && !IS_ACT_FLAG(victim,ACT_POLYMORPHED))
            {
                act(AT_PLAIN, "$N quickly surrenders The Ring to $n.", ch, NULL, victim, TO_ROOM);
                if (ring->carried_by)
                    obj_from_char(ring);
                obj_to_char(ring, ch);
                return TRUE;
            }

            do_wake(ch, GET_NAME(victim));
            sprintf(buf, "%s, give me The Ring.  *NOW*", PERS(victim, ch));
            do_say(ch, buf);
            if ( IS_ROOM_FLAG( ch->in_room, ROOM_SILENCE ) )
            {
                act(AT_PLAIN, "$n pokes you in the ribs very painfully.", ch, NULL, victim, TO_VICT);
                act(AT_PLAIN, "$n pokes $N in the ribs, ouch that looks painful.", ch, NULL, victim, TO_ROOM);
            }

            ch->tempnum++;
            return TRUE;
            break;
        case 4:
        case 5:
            ch->tempnum++;
            return TRUE;
            break;
        default:
            if (is_safe(ch, victim))
            {
                sprintf(buf, "You can't stay here forever, %s", PERS(victim, ch));
                do_say(ch, buf);
                if ( IS_ROOM_FLAG( ch->in_room, ROOM_SILENCE ) )
                    interpret(ch, "growl");
                return TRUE;
            }

            do_say(ch, "I guess I'll just have to get it myself.");
            one_hit( ch, victim, TYPE_UNDEFINED );
            ch->tempnum = 0;
            return TRUE;
            break;
        }
        return FALSE;
    }
    else if (ring->in_obj)
    {
        obj_from_obj(ring);
        obj_to_char(ring, ch);
        act(AT_PLAIN, "$n gets the One Ring.", ch, NULL, NULL, TO_ROOM);
        ch->tempnum = 0;
        return TRUE;
    }
    else if (ring->in_room)
    {
        do_get(ch, "one-ring");
        ch->tempnum = 0;
        return TRUE;
    }
    else
    {
        bug("spec_Ringwraith: a One Ring was completely disconnected!?");
    }
    ch->tempnum = 0;
    return FALSE;
}
