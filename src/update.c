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
 *			      Regular update module			    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: update.c,v 1.62 2004/04/06 22:00:11 dotd Exp $";*/

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "gsn.h"
#define SPEC SPECIAL_FUNC
#include "mspecial.h"
#undef SPEC

DECLARE_DO_FUN(do_shout);
DECLARE_DO_FUN(do_stand);
DECLARE_DO_FUN(do_emote);
DECLARE_DO_FUN(do_gossip);
DECLARE_DO_FUN(do_quit);
DECLARE_DO_FUN(do_sleep);
DECLARE_DO_FUN(do_help);

/*
 * Local functions.
 */
static void	mobile_update	args( ( void ) );
static void	time_update	args( ( void ) );	/* FB */
static void	char_update	args( ( void ) );
static void	obj_update	args( ( void ) );
static void	room_update	args( ( void ) );
static void	aggr_update	args( ( void ) );
static void	auction_update	args( ( void ) );
static void	char_check	args( ( void ) );
static void	drunk_randoms	args( ( CHAR_DATA *ch ) );
static void	halucinations	args( ( CHAR_DATA *ch ) );
static void	char_pulse_update args( ( CHAR_DATA *ch ) );
void	room_act_update	args( ( void ) );
void	obj_act_update	args( ( void ) );
void	subtract_times	args( ( struct timeval *etime,
			       struct timeval *start_time ) );
void    acro_check      args( ( void ) );

/* weather functions - FB */
static void	adjust_vectors		args( ( WEATHER_DATA *weather) );
static void	get_weather_echo	args( ( WEATHER_DATA *weather) );
static void	get_time_echo		args( ( WEATHER_DATA *weather) );

/*
 * Global Variables
 */

int		pulse_area	= 0;
int		pulse_mobile	= 0;
int		pulse_violence	= 0;
int		pulse_spell	= 0;
int		pulse_point	= 0;
int		pulse_second	= 0;
int		pulse_teleport	= 0;

struct timerset userec_violence;
struct timerset userec_area;
struct timerset userec_mobile;
struct timerset userec_spell;
struct timerset userec_aggr;
struct timerset userec_objact;
struct timerset userec_act;

CHAR_DATA *	gch_prev;
OBJ_DATA *	gobj_prev;

CHAR_DATA *	timechar;

char * corpse_descs[] =
{
    "The corpse of %s is in the last stages of decay.",
    "The corpse of %s is crawling with vermin.",
    "The corpse of %s fills the air with a foul stench.",
    "The corpse of %s is buzzing with flies.",
    "The corpse of %s lies here."
};

extern int      top_exit;
extern char	lastplayercmd[MAX_INPUT_LENGTH*2];
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];

void get_obj(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *obj_container);

/*
 * Advancement stuff.
 */

#define PERCENT_AGGRO sysdata.percent_aggr
#define HAS_CLASSX(ch, i)    (IS_ACTIVE(ch, i) && !HAD_CLASS(ch, i))

int mana_limit(CHAR_DATA *ch)
{
    int max, tmp;

    max = 0;

    if (IS_NPC(ch))
        return(UMAX(100,ch->max_mana));

    if (HAS_CLASSX(ch, CLASS_MAGE)) {
        max += 100;
        max += GET_LEVEL(ch, CLASS_MAGE) * 5;
    }

    if (HAS_CLASSX(ch, CLASS_NECROMANCER)) {
        max += 100;
        max += GET_LEVEL(ch, CLASS_NECROMANCER) * 5;
    }

    if (HAS_CLASSX(ch, CLASS_PSIONIST)) {
        max += 100;
        max += GET_LEVEL(ch, CLASS_PSIONIST) * 5;
    }

    if (HAS_CLASSX(ch, CLASS_PALADIN)) {
        max += 100;
        max += (GET_LEVEL(ch, CLASS_PALADIN)/4) * 5;
    }

    if (HAS_CLASSX(ch, CLASS_RANGER)) {
        max += 100;
        max += (GET_LEVEL(ch, CLASS_RANGER)/4) * 5;
    }

    if (HAS_CLASSX(ch, CLASS_CLERIC)) {
        max += 100;
        max += (GET_LEVEL(ch, CLASS_CLERIC)/3) * 5;
    }

    if (HAS_CLASSX(ch, CLASS_DRUID)) {
        max += 100;
        max += (GET_LEVEL(ch, CLASS_DRUID)/3) * 5;
    }

    if (HAS_CLASSX(ch, CLASS_THIEF)) {
        max += 100;
    }

    if (HAS_CLASSX(ch, CLASS_WARRIOR)) {
        max += 100;
    }

    if (HAS_CLASSX(ch, CLASS_ARTIFICER)) {
        max += 100;
    }

    if (HAS_CLASSX(ch, CLASS_MONK)) {
        max += 100;
    }

    if (HAS_CLASSX(ch, CLASS_ANTIPALADIN)) {
        max += 100;
        max += (GET_LEVEL(ch, CLASS_ANTIPALADIN)/4) * 5;
    }

    if (HAS_CLASSX(ch, CLASS_SORCERER)) {
        max += 100;
    }


    max /= HowManyClasses(ch);

    tmp = 0;

    tmp = get_curr_int(ch)/3;
    tmp += 2;
    tmp = tmp*3;

    max += tmp;
    max += ch->max_mana;

    if (OnlyClass(ch,CLASS_BARBARIAN)) /* 100 mana max for barbs */
    {
        max=100;  /* barbarians only get 100 mana... */
    }

    return(max);
}

#undef HAS_CLASSX

int hit_limit(CHAR_DATA *ch)
{
    int max;

    max = (ch->max_hit);

    return (max);
}


int move_limit(CHAR_DATA *ch)
{
    int max;

    if (IS_NPC(ch)) {
        max = 100 + get_curr_con(ch);
    } else {
        max = 100 + ch->max_move;
    }

    if (GET_RACE(ch) == RACE_DWARF || GET_RACE(ch) == RACE_GNOME)
        max -= 35;
    else if (GET_RACE(ch) == RACE_ELVEN || GET_RACE(ch) == RACE_DROW  ||
             GET_RACE(ch) == RACE_GOLD_ELF || GET_RACE(ch) == RACE_WILD_ELF ||
             GET_RACE(ch) == RACE_SEA_ELF ||
             GET_RACE(ch)== RACE_HALF_ELVEN)
        max += 20;
    else if (IS_ACTIVE(ch,CLASS_BARBARIAN))
        max +=45;  /* barbs get more move ... */
    else if (GET_RACE(ch) == RACE_HALFLING)
        max -= 45 ;
    else if (GET_RACE(ch) == RACE_HALF_GIANT)
        max +=60;
    else if (GET_RACE(ch) == RACE_HALF_OGRE)
        max +=50        ;

    return (max);
}


void advance_level( CHAR_DATA *ch, sh_int cl )
{
    char buf[MAX_STRING_LENGTH];
    int add_hp = 0;
    int add_mana = 0;
    int add_prac = 0;
    int i;
    int can_gain;

    add_mana		= GET_MAX_MANA(ch);

    GET_LEVEL(ch,cl)+=1;

    sprintf( buf, "the %s %s", get_race_name(ch), GetTitleString(ch) );
    set_title( ch, buf );

    /* for duals, set active classes that you've passed in level */
    for (i = 0; i < MAX_CLASS; ++i) {
        if (!IS_ACTIVE(ch, i) && HAD_CLASS(ch, i) && !IS_FALLEN(ch, i) &&
            (GET_LEVEL(ch, i) < GetMaxLevel(ch)))
            SET_BIT(ch->classes[i], STAT_ACTCLASS);
    }
    can_gain = TRUE;

    /* for duals don't gain on less than your inactive max level */
    for (i = 0; i < MAX_CLASS; ++i) {
        if (!IS_ACTIVE(ch, i) && HAD_CLASS(ch, i))
            can_gain = FALSE;
    }

    if (can_gain) {
        if (cl == CLASS_WARRIOR || cl == CLASS_BARBARIAN ||
            cl == CLASS_PALADIN || cl == CLASS_RANGER ||
            cl == CLASS_ANTIPALADIN || cl == CLASS_AMAZON)
            add_hp	= con_app[get_curr_con(ch)].hitp;
        else
            add_hp	= UMIN(con_app[get_curr_con(ch)].hitp, 2);

        if (GET_LEVEL(ch, cl) > class_table[cl]->hp_const_lev)
            add_hp	+= class_table[cl]->hp_const_add;
        else
            add_hp	+= number_range(class_table[cl]->hp_min,
                                        class_table[cl]->hp_max );

        add_hp	/= HowManyClasses(ch);
        add_hp	= UMAX(  1, add_hp   );
        ch->max_hit 	+= add_hp;
    } else {
        add_hp = 0;
    }

    /*
     add_mana	= class_table[cl]->fMana
     ? number_range(2, (2*get_curr_int(ch)+get_curr_wis(ch))/8)
     : 0;
     add_mana    /= HowManyClasses(ch);
     add_mana	= UMAX(  0, add_mana );

     ch->max_mana	+= add_mana;
     */
    add_mana		= GET_MAX_MANA(ch) - add_mana;

    add_prac		= wis_app[get_curr_wis(ch)].practice;
    add_prac		= UMAX(1, add_prac / HowManyClasses(ch));
    ch->practice	+= add_prac;

    if ( !IS_NPC(ch) )
        REMOVE_BIT( ch->act, PLR_BOUGHT_PET );

    if ( GetMaxLevel(ch) == LEVEL_AVATAR )
    {
        DESCRIPTOR_DATA *d;

        sprintf( buf, "%s has just achieved Avatarhood!", ch->name );
        for ( d = first_descriptor; d; d = d->next )
            if ( d->connected == CON_PLAYING && d->character != ch )
            {
                send_to_char( buf,	d->character );
                send_to_char( "\n\r",	d->character );
            }
        do_help( ch, "M_ADVHERO_" );
    }
    if ( GetMaxLevel(ch) < LEVEL_IMMORTAL )
    {
        if ( IS_VAMPIRE(ch) )
            sprintf( buf,
                     "Your gain is: %d/%d hp, %d/%d bp, %d/%d mana %d/%d prac.\n\r",
                     add_hp,	GET_MAX_HIT(ch),
                     1,	        GET_MAX_BLOOD(ch),
                     add_mana,	GET_MAX_MANA(ch),
                     add_prac,	ch->practice
                   );
        else
            sprintf( buf,
                     "Your gain is: %d/%d hp, %d/%d mana, %d/%d prac.\n\r",
                     add_hp,	GET_MAX_HIT(ch),
                     add_mana,	GET_MAX_MANA(ch),
                     add_prac,	ch->practice
                   );
        send_to_char( buf, ch );
    }
    else if (GetMaxLevel(ch) == LEVEL_IMMORTAL)
    {
        interpret(ch, "chan +all");
        interpret(ch, "holylight");
        interpret(ch, "speak all");
        interpret(ch, "rank Immortal");
        interpret(ch, "title the Immortal");
        SET_BIT(ch->act, PLR_LOG);
        interpret(ch, "save");
        interpret(ch, "think Greetings everybody!");
	interpret(ch, "help think");
    }

    ClassSpecificStuff(ch);
}



void gain_exp( CHAR_DATA *ch, int gain )
{
    register int gainlev;
    register int i;

    if ( IS_NPC(ch) || GetMaxLevel(ch) >= LEVEL_AVATAR || in_arena(ch) )
        return;

    gainlev = (int)(gain / HowManyClasses(ch));
    if( gainlev > 0 && GetMaxLevel(ch) < 5) {
        ch_printf(ch, "The Gods increase your learning, causing you to receive %d more exp!\n\r", gainlev);
        gainlev *= 2;
    }
    ch->exp += gainlev;

    for (i = 0; i < MAX_CLASS; ++i)
    {
        if (!can_gain_level(ch, i))
            continue;

        if (IS_ACTIVE(ch, i) && !HAD_CLASS(ch, i))
        {

#ifdef PLR2_AUTOGAIN
            if (IS_SET(ch->act2, PLR2_AUTOGAIN))
#else
            if (GetMaxLevel(ch) < 5)
#endif
            {
                while ( ch->exp >= exp_level(ch, GET_LEVEL(ch, i)+1, i) )
                {
                    if ( GET_LEVEL(ch,i) >= LEVEL_AVATAR )
                        break;
                    ch_printf( ch, "You have obtained a greater standing in the %s class!\n\r", pc_class[i]);
                    advance_level( ch, i );
                }
            }
            else
            {
                ch->exp = UMIN((exp_level(ch, GET_LEVEL(ch, i)+2, i)-1), ch->exp);
            }
        }
    }

    if (IS_ACTIVE(ch, CLASS_PALADIN) && !IS_FALLEN(ch, CLASS_PALADIN) && !IS_GOOD(ch))
    {
        SET_BIT(ch->classes[CLASS_PALADIN], STAT_FALLEN);
        if (!IS_ACTIVE(ch, CLASS_WARRIOR))
        {
            SET_BIT(ch->classes[CLASS_WARRIOR], STAT_ACTCLASS);
            GET_LEVEL(ch, CLASS_WARRIOR) = GET_LEVEL(ch, CLASS_PALADIN);
        }
        send_to_char("You have fallen from grace with the Paladin's Guild.\n\r", ch);
    }

    if (IS_ACTIVE(ch, CLASS_RANGER) && !IS_FALLEN(ch, CLASS_RANGER) && IS_EVIL(ch))
    {
        SET_BIT(ch->classes[CLASS_RANGER], STAT_FALLEN);
        if (!IS_ACTIVE(ch, CLASS_WARRIOR))
        {
            SET_BIT(ch->classes[CLASS_WARRIOR], STAT_ACTCLASS);
            GET_LEVEL(ch, CLASS_WARRIOR) = GET_LEVEL(ch, CLASS_RANGER);
        }
        send_to_char("You have fallen from grace with the Ranger's Guild.\n\r", ch);
    }

    if (IS_ACTIVE(ch, CLASS_ANTIPALADIN) && !IS_FALLEN(ch, CLASS_ANTIPALADIN) && !IS_EVIL(ch))
    {
        SET_BIT(ch->classes[CLASS_ANTIPALADIN], STAT_FALLEN);
        if (!IS_ACTIVE(ch, CLASS_WARRIOR))
        {
            SET_BIT(ch->classes[CLASS_WARRIOR], STAT_ACTCLASS);
            GET_LEVEL(ch, CLASS_WARRIOR) = GET_LEVEL(ch, CLASS_ANTIPALADIN);
        }
        send_to_char("You have fallen from grace with the Anti-Paladin's Guild.\n\r", ch);
    }

    if (IS_ACTIVE(ch, CLASS_DRUID) && !IS_FALLEN(ch, CLASS_DRUID) && !IS_NEUTRAL(ch))
    {
        SET_BIT(ch->classes[CLASS_DRUID], STAT_FALLEN);
        if (!IS_ACTIVE(ch, CLASS_CLERIC))
        {
            SET_BIT(ch->classes[CLASS_CLERIC], STAT_ACTCLASS);
            GET_LEVEL(ch, CLASS_CLERIC) = GET_LEVEL(ch, CLASS_DRUID);
        }
        send_to_char("You have fallen from grace with the Druid's Guild.\n\r", ch);
    }


    return;
}

/*
 * Regeneration stuff.
 */
/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{

    if (age < 15)
        return(p0);                               /* < 15   */
    else if (age <= 29)
        return (int) (p1+(((age-15)*(p2-p1))/15));  /* 15..29 */
    else if (age <= 44)
        return (int) (p2+(((age-30)*(p3-p2))/15));  /* 30..44 */
    else if (age <= 59)
        return (int) (p3+(((age-45)*(p4-p3))/15));  /* 45..59 */
    else if (age <= 79)
        return (int) (p4+(((age-60)*(p5-p4))/20));  /* 60..79 */
    else
        return(p6);                               /* >= 80 */
}

/* manapoint gain pr. game hour */
int mana_gain(struct char_data *ch)
{
    int gain;

    if (IS_NPC(ch)) {
        gain = 8;
    } else {
        gain = graf(get_age(ch),10,10,12,16,22,24,30);
    }

    switch (GET_POS(ch)) {
    case POS_SLEEPING:
    case POS_MEDITATING:
        gain += gain;
        break;
    case POS_RESTING:
        gain+= (gain>>1);  /* Divide by 2 */
        break;
    case POS_SITTING:
        gain += (gain>>2); /* Divide by 4 */
        break;
    }

    gain += ch->mana_regen;

    gain += gain;

    gain += wis_app[get_curr_wis(ch)].practice*2;

    if (IS_AFFECTED(ch,AFF_POISON))
        gain >>= 2;

    if (!IS_NPC(ch))
        if((GET_COND(ch,COND_FULL)==0)||(GET_COND(ch,COND_THIRST)==0))
            gain >>= 2;

    if (GET_RACE(ch) == RACE_ELVEN || GET_RACE(ch) == RACE_GNOME ||
        GET_RACE(ch) == RACE_GOLD_ELF || GET_RACE(ch) == RACE_WILD_ELF ||
        GET_RACE(ch) == RACE_SEA_ELF || GET_RACE(ch) == RACE_SKEXIE ||
        GET_RACE(ch) == RACE_DROW || GET_RACE(ch) == RACE_HALF_ELVEN)
        gain+=2;

    if (!IS_NPC(ch))
    {
        if (GET_COND(ch, COND_DRUNK)>10)
            gain += (gain >> 1);
        else if (GET_COND(ch, COND_DRUNK)>0)
            gain += (gain >> 2);
    }

    if (!(IS_ACTIVE(ch, CLASS_MAGE) ||
          IS_ACTIVE(ch, CLASS_NECROMANCER) ||
          IS_ACTIVE(ch, CLASS_CLERIC) ||
          IS_ACTIVE(ch, CLASS_DRUID) ||
          IS_ACTIVE(ch, CLASS_PSIONIST)))
        gain -=2;

    if (IS_ACTIVE(ch, CLASS_BARBARIAN))
        gain -=2;

    if (is_affected(ch,gsn_meditate))
        gain +=3;

#ifdef MUD_DEBUG
    if (!IS_NPC(ch))
        log_printf_plus(LOG_DEBUG, LEVEL_IMMORTAL, SEV_SPAM+1, "move_gain: %s %d", GET_NAME(ch), gain);
#endif

    return (gain);
}


int hit_gain(struct char_data *ch)
{

    int gain=15, dam;

    if (!IS_NPC(ch))
    {
        if (GET_POS(ch) == POS_FIGHTING)
            gain = 0;
        else
            gain = graf(get_age(ch),10,12,14,16,14,10,8);
    }

    if (GET_RACE(ch) == RACE_DWARF)
        gain += 3;
    if (GET_RACE(ch) == RACE_HALFLING)
        gain += 2;
    if (GET_RACE(ch) == RACE_HALF_GIANT)
        gain += 5;
    if (GET_RACE(ch) == RACE_HALF_OGRE)
        gain += 4;
    if (GET_RACE(ch) == RACE_HALF_ORC)
        gain += 4;
    if (GET_RACE(ch) == RACE_HALF_ELF)
        gain += 2;

    if (OnlyClass(ch, CLASS_BARBARIAN))
        gain += 5;    /* barbs gain hits faster... */

    gain += con_app[get_curr_con(ch)].hitp;
    gain += ch->hit_regen;

    switch (GET_POS(ch))
    {
    case POS_SLEEPING:
        gain += gain;
        break;
    case POS_RESTING:
    case POS_MEDITATING:
        gain += gain/2;
        break;
    case POS_SITTING:
        gain += gain/4;
        break;
    }

    if (IS_AFFECTED(ch,AFF_POISON))
    {
        gain = 0;
        dam = number_range(10,32);
        if (GET_RACE(ch) == RACE_HALFLING)
            dam = number_range(1,20);
        if (is_affected(ch, gsn_slow_poison))
            dam /= 4;
        damage(ch, ch, dam, gsn_poison);

	if (char_died(ch))
	    return(gain);
    }

    if (!IS_NPC(ch))
    {
        if (GET_COND(ch,COND_FULL) == 0 || GET_COND(ch,COND_THIRST) == 0)
            gain >>= 4;
        if (GET_COND(ch, COND_DRUNK)>10)
            gain += (gain >> 1);
        else if (GET_COND(ch, COND_DRUNK)>0)
            gain += (gain >> 2);
    }

    if (!(IS_ACTIVE(ch, CLASS_WARRIOR) ||
          IS_ACTIVE(ch, CLASS_PALADIN) ||
          IS_ACTIVE(ch, CLASS_VAMPIRE) ||
          IS_ACTIVE(ch, CLASS_ANTIPALADIN) ||
          IS_ACTIVE(ch, CLASS_RANGER) ||
          IS_ACTIVE(ch, CLASS_BARBARIAN)))
    {
        gain -= 2;
        if (gain < 0 && !ch->fighting)
            damage(ch,ch,gain*-1,skill_lookup("unknown"));

	if (char_died(ch))
	    return(gain);
    }

#ifdef MUD_DEBUG
    if (!IS_NPC(ch))
        log_printf_plus(LOG_DEBUG, LEVEL_IMMORTAL, SEV_SPAM+1, "hit_gain: %s %d", GET_NAME(ch), gain);
#endif

    return (gain);
}

int move_gain(struct char_data *ch)
/* move gain pr. game hour */
{
    int gain;

    if(IS_NPC(ch)) {
        gain = 22;
        if (IsRideable(ch))
            gain += gain/2;

        /* Neat and fast */
    } else {
        if (GET_POS(ch) != POS_FIGHTING)
            gain = graf(get_age(ch), 15,21,25,28,20,10,3);
        else
            gain = 0;
    }

    switch (GET_POS(ch)) {
    case POS_SLEEPING:
        gain += (gain>>2); /* Divide by 4 */
        break;
    case POS_RESTING:
    case POS_MEDITATING:
        gain+= (gain>>3);  /* Divide by 8 */
        break;
    case POS_SITTING:
        gain += (gain>>4); /* Divide by 16 */
        break;
    }

    gain += ch->move_regen;

    if (GET_RACE(ch) == RACE_DWARF || GET_RACE(ch) == RACE_SKEXIE)
        gain += 4;

    if (GET_RACE(ch) == RACE_HALF_GIANT)
        gain +=6;
    if (GET_RACE(ch) == RACE_HALF_OGRE)
        gain +=5;
    if (GET_RACE(ch) == RACE_HALF_ORC)
        gain +=4;

    if (IS_AFFECTED(ch,AFF_POISON))
        gain >>= 5;

    if (!IS_NPC(ch))
        if((GET_COND(ch,COND_FULL)==0)||(GET_COND(ch,COND_THIRST)==0))
            gain >>= 3;

    if (!(IS_ACTIVE(ch, CLASS_THIEF) ||
          IS_ACTIVE(ch, CLASS_MONK)))
        gain-=2;

#ifdef MUD_DEBUG
    if (!IS_NPC(ch))
        log_printf_plus(LOG_DEBUG, LEVEL_IMMORTAL, SEV_SPAM+1, "mana_gain: %s %d", GET_NAME(ch), gain);
#endif

    return (gain);
}

void gain_condition( CHAR_DATA *ch, int iCond, int value )
{
    int condition;
    ch_ret retcode = 0;

    if ( value == 0 || IS_NPC(ch) || IS_IMMORTAL(ch) || NOT_AUTHED(ch))
        return;

    condition				= GET_COND(ch, iCond);
    if ( iCond == COND_BLOODTHIRST )
        ch->pcdata->condition[iCond]    = URANGE( 0, condition + value,
                                                  GET_MAX_BLOOD(ch) );
    else if (iCond == COND_DRUNK && value > 0 && gsn_drinking)
    {
        int rval;

        if (!(rval = (2 * value * LEARNED(ch, gsn_drinking)) / 100))
            rval = 1 + (number_percent() > LEARNED(ch, gsn_drinking));

        if (number_percent() < 10 &&
            number_percent() < LEARNED(ch, gsn_drinking))
            learn_from_failure(ch, gsn_drinking);

        ch->pcdata->condition[iCond]    = URANGE( 0, condition + (2*rval), MAX_COND_VAL );
    }
    else
        ch->pcdata->condition[iCond]    = URANGE( 0, condition + (2*value), MAX_COND_VAL );

    if ( GET_COND(ch, iCond) == 0 )
    {
        switch ( iCond )
        {
        case COND_FULL:
            if ( !IS_VAMPIRE(ch) )
            {
                send_to_char( "You are STARVING!\n\r",  ch );
                act( AT_HUNGRY, "$n is starved half to death!", ch, NULL, NULL, TO_ROOM);
                worsen_mental_state( ch, 1 );
                retcode = damage(ch, ch, 1, TYPE_UNDEFINED);
            }
            break;

        case COND_THIRST:
            if ( !IS_VAMPIRE(ch) )
            {
                set_char_color( AT_THIRSTY, ch );
                send_to_char( "You are DYING of THIRST!\n\r", ch );
                act( AT_THIRSTY, "$n is dying of thirst!", ch, NULL, NULL, TO_ROOM);
                worsen_mental_state( ch, 1 );
                retcode = damage(ch, ch, 2, TYPE_UNDEFINED);
            }
            break;

        case COND_BLOODTHIRST:
            send_to_char( "You are starved to feast on blood!\n\r", ch );
            act( AT_PLAIN, "$n is suffering from lack of blood!", ch,
                 NULL, NULL, TO_ROOM);
            worsen_mental_state( ch, 2 );
            retcode = damage(ch, ch, GET_MAX_HIT(ch) / 20, TYPE_UNDEFINED);
            break;
        case COND_DRUNK:
            if ( condition != 0 ) {
                set_char_color( AT_SOBER, ch );
                send_to_char( "You are sober.\n\r", ch );
            }
            retcode = rNONE;
            break;
        default:
            bug( "Gain_condition: invalid condition type %d", iCond );
            retcode = rNONE;
            break;
        }
    }

    if ( retcode != rNONE )
        return;

    if ( GET_COND(ch, iCond) <= 3 )
    {
        switch ( iCond )
        {
        case COND_FULL:
            if ( !IS_VAMPIRE(ch) )
            {
                set_char_color( AT_HUNGRY, ch );
                send_to_char( "You are really hungry.\n\r",  ch );
                act( AT_HUNGRY, "You can hear $n's stomach growling.", ch, NULL, NULL, TO_ROOM);
                if ( number_bits(4) == 0 )
                    worsen_mental_state( ch, 1 );
            }
            break;

        case COND_THIRST:
            if ( !IS_VAMPIRE(ch) )
            {
                set_char_color( AT_THIRSTY, ch );
                send_to_char( "You are really thirsty.\n\r", ch );
                if ( number_bits(4) == 0 )
                    worsen_mental_state( ch, 1 );
                act( AT_THIRSTY, "$n looks a little parched.", ch, NULL, NULL, TO_ROOM);
            }
            break;

        case COND_BLOODTHIRST:
            set_char_color( AT_BLOOD, ch );
            send_to_char( "You have a growing need to feast on blood!\n\r", ch );
            act( AT_BLOOD, "$n gets a strange look in $s eyes...", ch,
                 NULL, NULL, TO_ROOM);
            if ( number_bits(4) == 0 )
                worsen_mental_state( ch, 1 );
            break;
        case COND_DRUNK:
            if ( condition != 0 ) {
                set_char_color( AT_SOBER, ch );
                send_to_char( "You are feeling a little less light headed.\n\r", ch );
            }
            break;
        }
    }
    else if ( GET_COND(ch, iCond) <= 6 )
    {
        switch ( iCond )
        {
        case COND_FULL:
            if ( !IS_VAMPIRE(ch) )
            {
                set_char_color( AT_HUNGRY, ch );
                send_to_char( "You are hungry.\n\r",  ch );
            }
            break;

        case COND_THIRST:
            if ( !IS_VAMPIRE(ch) )
            {
                set_char_color( AT_THIRSTY, ch );
                send_to_char( "You are thirsty.\n\r", ch );
            }
            break;

        case COND_BLOODTHIRST:
            set_char_color( AT_BLOOD, ch );
            send_to_char( "You feel an urgent need for blood.\n\r", ch );
            break;
        }
    }
    else if ( GET_COND(ch, iCond) <= 10 )
    {
        switch ( iCond )
        {
        case COND_FULL:
            if ( !IS_VAMPIRE(ch) )
            {
                set_char_color( AT_HUNGRY, ch );
                send_to_char( "You are a mite peckish.\n\r",  ch );
            }
            break;

        case COND_THIRST:
            if ( !IS_VAMPIRE(ch) )
            {
                set_char_color( AT_THIRSTY, ch );
                send_to_char( "You could use a sip of something refreshing.\n\r", ch );
            }
            break;

        case COND_BLOODTHIRST:
            set_char_color( AT_BLOOD, ch );
            send_to_char( "You feel an aching in your fangs.\n\r", ch );
            break;
        }
    }
    return;
}



/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Mud cpu time.
 */
static void mobile_update( void )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *ch;
    EXIT_DATA *pexit;
    int door;
    ch_ret     retcode;

    retcode = rNONE;

    /* Examine all mobs. */
    for ( ch = last_char; ch; ch = gch_prev )
    {
        set_cur_char( ch );
        if ( ch == first_char && ch->prev )
        {
            bug( "mobile_update: first_char->prev != NULL... fixed" );
            ch->prev = NULL;
        }

        gch_prev = ch->prev;

        if ( gch_prev && gch_prev->next != ch )
        {
            sprintf( bug_buf, "FATAL: Mobile_update: %s->prev->next doesn't point to ch.",
                     ch->name );
            bug( bug_buf, 0 );
            bug( "Short-cutting here" );
            gch_prev = NULL;
            ch->prev = NULL;
            do_shout( ch, "Thoric says, 'Prepare for the worst!'" );
        }

        if ( !IS_NPC(ch) )
        {
            drunk_randoms(ch);
            halucinations(ch);
            char_pulse_update(ch);
            continue;
        }

        if ( !ch->in_room
             ||   IS_AFFECTED(ch, AFF_CHARM)
             ||   IS_AFFECTED(ch, AFF_PARALYSIS) )
            continue;

        /* Clean up 'animated corpses' that are not charmed - Scryn */

        if ( ch->vnum == MOB_VNUM_ANIMATED_CORPSE && !IS_AFFECTED(ch, AFF_CHARM) )
        {
            if(ch->in_room->first_person)
                act(AT_MAGIC, "$n returns to the dust from whence $e came.", ch, NULL, NULL, TO_ROOM);

            if(IS_NPC(ch)) /* Guard against purging switched? */
                extract_char(ch, TRUE);
            continue;
        }

        if ( !IS_SET( ch->act, ACT_RUNNING )
             &&   !IS_SET( ch->act, ACT_SENTINEL )
             &&   !ch->fighting && ch->hunting )
        {
            WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
            /* Commented out temporarily to avoid spam - Scryn
             sprintf( log_buf, "%s hunting %s from %s.", ch->name,
             ch->hunting->name,
             ch->in_room->name );
             log_string( log_buf ); */
            hunt_victim( ch );
            continue;
        }

        if ( ch->fighting && (ch->position == POS_SITTING) &&
             number_range(0, 1) )
        {
           do_stand(ch, "");
           continue;
        }

        /* Examine call for special procedure */
        if ( !IS_SET( ch->act, ACT_RUNNING )
             && !IS_SET(ch->act, ACT_PROTOTYPE)
             && sysdata.specials_enabled==TRUE)
        {
            if (ch->spec_fun)
            {
                if ( (*ch->spec_fun) ( ch, NULL, "", NULL, SFT_UPDATE ) )
                    continue;
                if ( char_died(ch) )
                    continue;
            }
            if (IS_ACT2_FLAG(ch, ACT2_MAGE))
                if (spec_mage(ch, NULL, "", NULL, SFT_UPDATE))
                    continue;
            if (IS_ACT2_FLAG(ch, ACT2_CLERIC))
                if (spec_cleric(ch, NULL, "", NULL, SFT_UPDATE))
                    continue;
            if (IS_ACT2_FLAG(ch, ACT2_WARRIOR))
                if (spec_warrior(ch, NULL, "", NULL, SFT_UPDATE))
                    continue;
            if (IS_ACT2_FLAG(ch, ACT2_THIEF))
                if (spec_thief(ch, NULL, "", NULL, SFT_UPDATE))
                    continue;
            if (IS_ACT2_FLAG(ch, ACT2_DRUID))
                if (spec_druid(ch, NULL, "", NULL, SFT_UPDATE))
                    continue;
            if (IS_ACT2_FLAG(ch, ACT2_MONK))
                if (spec_monk(ch, NULL, "", NULL, SFT_UPDATE))
                    continue;
            if (IS_ACT2_FLAG(ch, ACT2_BARBARIAN))
                if (spec_barbarian(ch, NULL, "", NULL, SFT_UPDATE))
                    continue;
            if (IS_ACT2_FLAG(ch, ACT2_PALADIN))
                if (spec_paladin(ch, NULL, "", NULL, SFT_UPDATE))
                    continue;
            if (IS_ACT2_FLAG(ch, ACT2_RANGER))
                if (spec_ranger(ch, NULL, "", NULL, SFT_UPDATE))
                    continue;
            if (IS_ACT2_FLAG(ch, ACT2_PSI))
                if (spec_psi(ch, NULL, "", NULL, SFT_UPDATE))
                    continue;
            if (spec_racial_specifics(ch, NULL, "", NULL, SFT_UPDATE))
                continue;
        }

        /* Check for mudprogram script on mob */
        if ( HAS_PROG( ch->pIndexData, SCRIPT_PROG ) )
        {
            mprog_script_trigger( ch );
            continue;
        }

        if ( ch != cur_char )
        {
            bug( "Mobile_update: ch != cur_char after spec_fun" );
            continue;
        }

        /* That's all for sleeping / busy monster */
        if ( ch->position != POS_STANDING )
            continue;

        if ( IS_SET(ch->act, ACT_MOUNTED ) )
        {
            if ( IS_SET(ch->act, ACT_AGGRESSIVE) )
                do_emote( ch, "snarls and growls." );
            continue;
        }

        if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE )
             &&   IS_SET(ch->act, ACT_AGGRESSIVE) )
            do_emote( ch, "glares around and snarls." );


        /* MOBprogram random trigger */
        if ( ch->in_room->area->nplayer > 0 )
        {
            mprog_random_trigger( ch );
            if ( char_died(ch) )
                continue;
            if ( ch->position < POS_STANDING )
                continue;
        }

        /* MOBprogram hour trigger: do something for an hour */
        mprog_hour_trigger(ch);

        if ( char_died(ch) )
            continue;

        rprog_hour_trigger(ch);
        if ( char_died(ch) )
            continue;

        if ( ch->position < POS_STANDING )
            continue;

        /* Scavenge */
        if ( IS_SET(ch->act, ACT_SCAVENGER)
             &&   ch->in_room->first_content
             &&   number_bits( 2 ) == 0 )
        {
            OBJ_DATA *obj, *obj_next;
            OBJ_DATA *obj_best;
            int max;

            max         = 1;
            obj_best    = NULL;
            for ( obj = ch->in_room->first_content; obj; obj = obj_next )
            {
                obj_next = obj->next_content;
                if (obj->item_type == ITEM_MONEY &&
                    GET_MONEY(ch, obj->value[2]) >= obj->value[0])
                {
                    obj_from_room(obj);
                    extract_obj(obj);
                    continue;
                }
                if ( CAN_WEAR(obj, ITEM_TAKE) && obj->cost > max &&
                     !IS_OBJ_STAT( obj, ITEM_BURRIED ) )
                {
                    obj_best    = obj;
                    max         = obj->cost;
                }
            }

            if ( obj_best )
            {
		separate_obj(obj_best);
		get_obj(ch, obj_best, NULL);
/*
                obj_from_room( obj_best );
                obj_to_char( obj_best, ch );
                act( AT_ACTION, "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
*/
            }
        }

        /* Wander */
        if ( !IS_SET(ch->act, ACT_RUNNING)
             &&   !IS_SET(ch->act, ACT_SENTINEL)
             &&   !IS_SET(ch->act, ACT_PROTOTYPE)
             && ( door = number_bits( 5 ) ) <= 9
             && ( pexit = get_exit(ch->in_room, door) ) != NULL
             &&   pexit->to_room
             &&   !IS_SET(pexit->exit_info, EX_CLOSED)
             &&   !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)
             &&   !IS_SET(pexit->to_room->room_flags, ROOM_DEATH)
             &&   !will_drown(ch, pexit->to_room)
             && ( !IS_SET(ch->act, ACT_STAY_AREA)
                  ||   pexit->to_room->area == ch->in_room->area ) )
        {
            retcode = move_char( ch, pexit, 0 );
            /* If ch changes position due
             to it's or someother mob's
             movement via MOBProgs,
             continue - Kahn */
            if ( char_died(ch) )
                continue;
            if ( retcode != rNONE || IS_SET(ch->act, ACT_SENTINEL)
                 ||    ch->position < POS_STANDING )
                continue;
        }

        /* Flee */
        if ( GET_HIT(ch) < GET_MAX_HIT(ch) / 2
             && ( door = number_bits( 4 ) ) <= 9
             && ( pexit = get_exit(ch->in_room,door) ) != NULL
             &&   pexit->to_room
             &&   !IS_SET(pexit->exit_info, EX_CLOSED)
             &&   !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB) )
        {
            CHAR_DATA *rch;
            bool found;

            found = FALSE;
            for ( rch  = ch->in_room->first_person;
                  rch;
                  rch  = rch->next_in_room )
            {
                if ( is_fearing(ch, rch) && number_bits(5) == 0 )
                {
                    switch( number_bits(2) )
                    {
                    case 0:
                        sprintf( buf, "Get away from me, %s!", rch->name );
                        break;
                    case 1:
                        sprintf( buf, "Leave me be, %s!", rch->name );
                        break;
                    case 2:
                        sprintf( buf, "%s is trying to kill me!  Help!", rch->name );
                        break;
                    default:
                        sprintf( buf, "Someone save me from %s!", rch->name );
                        break;
                    }
                    do_gossip( ch, buf );
                    found = TRUE;
                    break;
                }
            }
            if ( found )
                retcode = move_char( ch, pexit, 0 );
        }
    }

    return;
}


/*
 * Update all chars, including mobs.
 * This function is performance sensitive.
 */
static void char_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_save;
    sh_int save_count = 0;

    ch_save	= NULL;
    for ( ch = last_char; ch; ch = gch_prev )
    {
        /*        if ((ch->desc && ch->desc->connected<0) || GetMaxLevel(ch)==0)
         continue;*/
        if ( ch == first_char && ch->prev )
        {
            bug( "char_update: first_char->prev != NULL... fixed" );
            ch->prev = NULL;
        }
        gch_prev = ch->prev;
        set_cur_char( ch );
        if ( gch_prev && gch_prev->next != ch )
        {
            bug( "char_update: ch->prev->next != ch" );
            return;
        }

        /*
         *  Do a room_prog rand check right off the bat
         *   if ch disappears (rprog might wax npc's), continue
         */
        if(!IS_NPC(ch))
            rprog_random_trigger( ch );

        if( char_died(ch) )
            continue;

        if(IS_NPC(ch))
            mprog_time_trigger(ch);

        if( char_died(ch) )
            continue;

        rprog_time_trigger(ch);

        if( char_died(ch) )
            continue;

        /*
         * See if player should be auto-saved.
         */
        if ( !IS_NPC(ch)
             && ( !ch->desc || ch->desc->connected == CON_PLAYING )
             &&    GetMaxLevel(ch) >= 1
             &&    current_time - ch->save_time > (sysdata.save_frequency*60) )
            ch_save	= ch;
        else
            ch_save	= NULL;

        if ( ch->position >= POS_STUNNED )
        {
            if ( GET_HIT(ch)  < GET_MAX_HIT(ch) )
                GET_HIT(ch) = UMIN(GET_HIT(ch)+hit_gain(ch),
                                   GET_MAX_HIT(ch));

            if (char_died(ch))
                continue;

            if ( GET_MANA(ch) < GET_MAX_MANA(ch) )
                GET_MANA(ch) = UMIN(GET_MANA(ch)+mana_gain(ch),
                                    GET_MAX_MANA(ch));

            if ( GET_MOVE(ch) < GET_MAX_MOVE(ch) )
                GET_MOVE(ch) = UMIN(GET_MOVE(ch)+move_gain(ch),
                                    GET_MAX_MOVE(ch));

            if ( ch->position == POS_STUNNED || ch->position == POS_FIGHTING )
                update_pos( ch );
        }
        else
            GET_HIT(ch) -= 1;

        if ( !IS_NPC(ch) && !IS_IMMORTAL(ch) )
        {
            if ( ++ch->timer >= 120 )
            {
                do_quit( ch, "yes" );
                continue;
            }
            if ( ch->timer >= 60 )
            {
                if ( !ch->idle_room && ch->in_room )
                {
                    ch->idle_room = ch->in_room;
                    if ( ch->fighting )
                        stop_fighting( ch, TRUE );
                    act( AT_ACTION, "$n disappears into the void.",
                         ch, NULL, NULL, TO_ROOM );
                    send_to_char( "You disappear into the void.\n\r", ch );
                    log_printf_plus(LOG_MONITOR, LEVEL_LOG_CSET, SEV_NOTICE, "%s has idled out.", GET_NAME(ch));
                    if ( IS_SET( sysdata.save_flags, SV_IDLE ) )
                        save_char_obj( ch );
                    char_from_room( ch );
                    char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
                }
                continue;
            }
        }

        if ( !IS_NPC(ch) )
        {
            OBJ_DATA *obj;

            if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
                 &&   obj->item_type == ITEM_LIGHT
                 &&   obj->value[2] > 0 )
            {
                if ( --obj->value[2] == 0 && ch->in_room )
                {
                    ch->in_room->light -= obj->count;
                    act( AT_ACTION, "$p goes out.", ch, obj, NULL, TO_ROOM );
                    act( AT_ACTION, "$p goes out.", ch, obj, NULL, TO_CHAR );
                    extract_obj( obj );
                }
            }

            if ( GET_COND(ch, COND_DRUNK) > 8 )
                worsen_mental_state( ch, GET_COND(ch, COND_DRUNK)/8 );
            if ( GET_COND(ch, COND_FULL) > 1 )
            {
                switch( ch->position )
                {
                case POS_MEDITATING:
                case POS_SLEEPING:  better_mental_state( ch, 4 );	break;
                case POS_RESTING:   better_mental_state( ch, 3 );	break;
                case POS_SITTING:
                case POS_MOUNTED:   better_mental_state( ch, 2 );	break;
                case POS_STANDING:  better_mental_state( ch, 1 );	break;
                case POS_FIGHTING:
                    if ( number_bits(2) == 0 )
                        better_mental_state( ch, 1 );
                    break;
                }
            }
            if ( GET_COND(ch, COND_THIRST) > 1 )
            {
                switch( ch->position )
                {
                case POS_SLEEPING:  better_mental_state( ch, 5 );	break;
                case POS_MEDITATING:
                case POS_RESTING:   better_mental_state( ch, 3 );	break;
                case POS_SITTING:
                case POS_MOUNTED:   better_mental_state( ch, 2 );	break;
                case POS_STANDING:  better_mental_state( ch, 1 );	break;
                case POS_FIGHTING:
                    if ( number_bits(2) == 0 )
                        better_mental_state( ch, 1 );
                    break;
                }
            }
            gain_condition( ch, COND_DRUNK,  -1 );
            gain_condition( ch, COND_FULL,   -1 );
            if ( IS_VAMPIRE(ch) && ch->levels[CLASS_VAMPIRE] >= 5 )
            {
                if ( time_info.hour < 21 && time_info.hour > 5 )
                    gain_condition( ch, COND_BLOODTHIRST, -1 );
            }
            if ( ch->in_room )
                switch( ch->in_room->sector_type )
                {
                default:
                    gain_condition( ch, COND_THIRST, -1 );  break;
                case SECT_DESERT:
                    gain_condition( ch, COND_THIRST, -2 );  break;
                case SECT_UNDERWATER:
                case SECT_OCEANFLOOR:
                    if ( number_bits(1) == 0 )
                        gain_condition( ch, COND_THIRST, -1 );  break;
                }

        }

        if ( char_died(ch) )
            continue;
        /*
         * Careful with the damages here,
         *   MUST NOT refer to ch after damage taken,
         *   as it may be lethal damage (on NPC).
         */
        if ( IS_AFFECTED(ch, AFF_POISON) )
        {
            act( AT_POISON, "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
            act( AT_POISON, "You shiver and suffer.", ch, NULL, NULL, TO_CHAR );
            ch->mental_state = URANGE( 20, ch->mental_state
                                       + (IS_PKILL(ch) ? 3 : 4), 100 );
            damage( ch, ch, 6, gsn_poison );
        }
        else
            if ( ch->position == POS_INCAP )
                damage( ch, ch, 1, TYPE_UNDEFINED );
            else
                if ( ch->position == POS_MORTAL )
                    damage( ch, ch, 4, TYPE_UNDEFINED );

        if ( char_died(ch) )
            continue;

        if ( ch->mental_state >= 30 )
        {
            switch( (ch->mental_state+5) / 10 )
            {
            case  3:
                send_to_char( "You feel feverish.\n\r", ch );
                act( AT_ACTION, "$n looks kind of out of it.", ch, NULL, NULL, TO_ROOM );
                break;
            case  4:
                send_to_char( "You do not feel well at all.\n\r", ch );
                act( AT_ACTION, "$n doesn't look too good.", ch, NULL, NULL, TO_ROOM );
                break;
            case  5:
                send_to_char( "You need help!\n\r", ch );
                act( AT_ACTION, "$n looks like $e could use your help.", ch, NULL, NULL, TO_ROOM );
                break;
            case  6:
                send_to_char( "Seekest thou a cleric.\n\r", ch );
                act( AT_ACTION, "Someone should fetch a healer for $n.", ch, NULL, NULL, TO_ROOM );
                break;
            case  7:
                send_to_char( "You feel reality slipping away...\n\r", ch );
                act( AT_ACTION, "$n doesn't appear to be aware of what's going on.", ch, NULL, NULL, TO_ROOM );
                break;
            case  8:
                send_to_char( "You begin to understand... everything.\n\r", ch );
                act( AT_ACTION, "$n starts ranting like a madman!", ch, NULL, NULL, TO_ROOM );
                break;
            case  9:
                send_to_char( "You are ONE with the universe.\n\r", ch );
                act( AT_ACTION, "$n is ranting on about 'the answer', 'ONE' and other mumbo-jumbo...", ch, NULL, NULL, TO_ROOM );
                break;
            case 10:
                send_to_char( "You feel the end is near.\n\r", ch );
                act( AT_ACTION, "$n is muttering and ranting in tongues...", ch, NULL, NULL, TO_ROOM );
                break;
            }
        }

        if ( ch->mental_state <= -30 )
        {
            switch( (abs(ch->mental_state)+5) / 10 )
            {
            case  10:
                if ( ch->position > POS_SLEEPING )
                {
                    if ( (ch->position == POS_STANDING
                          ||    ch->position < POS_FIGHTING)
                         &&    number_percent()+10 < abs(ch->mental_state) )
                        do_sleep( ch, "" );
                    else
                        send_to_char( "You're barely conscious.\n\r", ch );
                }
                break;
            case   9:
                if ( ch->position > POS_SLEEPING )
                {
                    if ( (ch->position == POS_STANDING
                          ||    ch->position < POS_FIGHTING)
                         &&   (number_percent()+20) < abs(ch->mental_state) )
                        do_sleep( ch, "" );
                    else
                        send_to_char( "You can barely keep your eyes open.\n\r", ch );
                }
                break;
            case   8:
                if ( ch->position > POS_SLEEPING )
                {
                    if ( ch->position < POS_SITTING
                         &&  (number_percent()+30) < abs(ch->mental_state) )
                        do_sleep( ch, "" );
                    else
                        send_to_char( "You're extremely drowsy.\n\r", ch );
                }
                break;
            case   7:
                if ( ch->position > POS_RESTING )
                    send_to_char( "You feel very unmotivated.\n\r", ch );
                break;
            case   6:
                if ( ch->position > POS_RESTING )
                    send_to_char( "You feel sedated.\n\r", ch );
                break;
            case   5:
                if ( ch->position > POS_RESTING )
                    send_to_char( "You feel sleepy.\n\r", ch );
                break;
            case   4:
                if ( ch->position > POS_RESTING )
                    send_to_char( "You feel tired.\n\r", ch );
                break;
            case   3:
                if ( ch->position > POS_RESTING )
                    send_to_char( "You could use a rest.\n\r", ch );
                break;
            }
        }

        if ( ch == ch_save && IS_SET( sysdata.save_flags, SV_AUTO )
             &&   ++save_count < 10 )	/* save max of 10 per tick */
            save_char_obj( ch );
    }

    return;
}



/*
 * Update all objs.
 * This function is performance sensitive.
 */
static void obj_update( void )
{
    OBJ_DATA *obj;
    sh_int AT_TEMP;

    for ( obj = last_object; obj; obj = gobj_prev )
    {
        CHAR_DATA *rch;
        char *message;

        separate_obj(obj);

        if ( obj == first_object && obj->prev )
        {
            bug( "obj_update: first_object->prev != NULL... fixed" );
            obj->prev = NULL;
        }
        gobj_prev = obj->prev;
        if ( gobj_prev && gobj_prev->next != obj )
        {
            bug( "obj_update: obj->prev->next != obj" );
            return;
        }

        if ( obj_extracted(obj) )
            continue;

        if ( obj->currtype<FIRST_CURR || obj->currtype>LAST_CURR)
        {
            bug("obj_update: obj u%d has bad currency, fixing.",
                obj->unum);
            obj->currtype = DEFAULT_CURR;
        }

	if ( obj->item_type == ITEM_MONEY &&
	     ( obj->value[0]<=0 || obj->value[0]>50000 ) )
	{
	    bug("obj_update: obj u%d has %d of curr %d",
		obj->unum, obj->value[0], obj->value[2]);
        }

        if ( obj->spec_fun && sysdata.specials_enabled==TRUE)
        {
            if ( (*obj->spec_fun) ( obj, NULL, "", NULL, SFT_UPDATE ) )
                continue;
            if ( obj_extracted(obj) )
                continue;
        }

        if ( obj->carried_by )
            oprog_random_trigger( obj );
        else
            if( obj->in_room && obj->in_room->area->nplayer > 0 )
                oprog_random_trigger( obj );

        if( obj_extracted(obj) )
            continue;

	if ( IS_OBJ_STAT2(obj, ITEM2_ANTI_SUN) )
	{
	    ROOM_INDEX_DATA *troom=NULL;

	    if (obj->in_room)
		troom=obj->in_room;
	    if (obj->carried_by)
		troom=obj->carried_by->in_room;
	    if (troom && NO_WEATHER_SECT(troom->sector_type) &&
		!IS_ROOM_FLAG(troom, ROOM_INDOORS) &&
		!IS_ROOM_FLAG(troom, ROOM_DARK) &&
		time_info.sunlight == SUN_LIGHT)
	    {
		log_printf_plus(LOG_DEBUG, LEVEL_IMMORTAL, SEV_DEBUG, "object destroyed by sunlight: %s, room: %d", obj->name, troom->vnum);
		if (troom->first_person)
		    act( AT_YELLOW, "$p is touched by the sunlight, and disintigrates into dust.", troom->first_person, obj, NULL, TO_ROOM );
		extract_obj(obj);
		continue;
	    }
	}

        if ( obj->item_type == ITEM_PIPE )
        {
            if ( IS_SET( obj->value[3], PIPE_LIT ) )
            {
                if ( --obj->value[1] <= 0 )
                {
                    obj->value[1] = 0;
                    REMOVE_BIT( obj->value[3], PIPE_LIT );
                    REMOVE_BIT( obj->value[3], PIPE_TAMPED );
                    if ( IS_SET( obj->value[3], PIPE_SINGLE_USE ) )
                    {
                        extract_obj(obj);
                        continue;
                    }
                }
                else
                    if ( IS_SET( obj->value[3], PIPE_HOT ) )
                        REMOVE_BIT( obj->value[3], PIPE_HOT );
                    else
                    {
                        if ( IS_SET( obj->value[3], PIPE_GOINGOUT ) )
                        {
                            REMOVE_BIT( obj->value[3], PIPE_LIT );
                            REMOVE_BIT( obj->value[3], PIPE_TAMPED );
                            REMOVE_BIT( obj->value[3], PIPE_GOINGOUT );
                        }
                        else
                            SET_BIT( obj->value[3], PIPE_GOINGOUT );
                    }
                if ( !IS_SET( obj->value[3], PIPE_LIT ) )
                    SET_BIT( obj->value[3], PIPE_FULLOFASH );
            }
            else
                REMOVE_BIT( obj->value[3], PIPE_HOT );
        }

        /* Corpse decay (npc corpses decay at 8 times the rate of pc corpses) - Narn */

        if ( obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_CORPSE_NPC )
        {
            sh_int timerfrac = UMAX(1, obj->timer - 1);
            if ( obj->item_type == ITEM_CORPSE_PC )
                timerfrac = (int)(obj->timer / 8 + 1);

            if ( obj->timer > 0 && obj->value[2] > timerfrac )
            {
                char buf[MAX_STRING_LENGTH];
                char name[MAX_STRING_LENGTH];
                char *bufptr;
                bufptr = one_argument( obj->short_descr, name );
                bufptr = one_argument( bufptr, name );
                bufptr = one_argument( bufptr, name );

                separate_obj(obj);
                obj->value[2] = timerfrac;
                sprintf( buf, corpse_descs[ UMIN( timerfrac - 1, 4 ) ],
                         capitalize( bufptr ) );

                STRFREE( obj->description );
                obj->description = STRALLOC( buf );
            }
        }

        /* don't let inventory decay */
        if ( IS_OBJ_STAT(obj, ITEM_INVENTORY) )
            continue;

        if ( ( obj->timer <= 0 || --obj->timer > 0 ) )
            continue;

        /* if we get this far, object's timer has expired. */

        AT_TEMP = AT_PLAIN;
        switch ( obj->item_type )
        {
        default:
            message = "$p mysteriously vanishes.";
            AT_TEMP = AT_PLAIN;
            break;
        case ITEM_PORTAL:
            message = "$p winks out of existence.";
            /*	   remove_portal(obj);
             obj->item_type = ITEM_TRASH;*/
            /* so extract_obj	 */
            AT_TEMP = AT_MAGIC;			/* doesn't remove_portal */
            break;
        case ITEM_FOUNTAIN:
            message = "$p dries up.";
            AT_TEMP = AT_BLUE;
            break;
        case ITEM_CORPSE_NPC:
            message = "$p decays into dust and blows away.";
            AT_TEMP = AT_OBJECT;
            break;
        case ITEM_CORPSE_PC:
            message = "$p is sucked into a swirling vortex of colors...";
            AT_TEMP = AT_MAGIC;
            break;
        case ITEM_FOOD:
            message = "$p is devoured by a swarm of maggots.";
            AT_TEMP = AT_HUNGRY;
            break;
        case ITEM_BLOOD:
            message = "$p slowly seeps into the ground.";
            AT_TEMP = AT_BLOOD;
            break;
        case ITEM_BLOODSTAIN:
            message = "$p dries up into flakes and blows away.";
            AT_TEMP = AT_BLOOD;
            break;
        case ITEM_SCRAPS:
            message = "$p crumbles and decays into nothing.";
            AT_TEMP = AT_OBJECT;
            break;
        case ITEM_FIRE:
            if (obj->in_room)
                --obj->in_room->light;
            message = "$p burns out.";
            AT_TEMP = AT_FIRE;
        }

        if ( obj->carried_by )
        {
            act( AT_PLAIN, message, obj->carried_by, obj, NULL, TO_CHAR );
        }
        else if ( obj->in_room
                  &&      ( rch = obj->in_room->first_person ) != NULL
                  &&	!IS_OBJ_STAT( obj, ITEM_BURRIED ) )
        {
            act( AT_TEMP, message, rch, obj, NULL, TO_ROOM );
            act( AT_TEMP, message, rch, obj, NULL, TO_CHAR );
        }

        extract_obj( obj );
    }
    return;
}

static void room_update( void )
{
    ROOM_INDEX_DATA *rp;
    register int i;

    for (i = 0; i < MAX_KEY_HASH; i++)
        for (rp = room_index_hash[i]; rp; rp = rp->next)
        {
            if (IS_RIVER(rp))
                DISPOSE(rp->river);
        }

    for (i = 0; i < MAX_KEY_HASH; i++)
        for (rp = room_index_hash[i]; rp; rp = rp->next)
        {
            if (IS_RIVER_SOURCE(rp))
            {
                calc_river_path(rp, RIVER_LIQUID(rp));
            }

            if ( rp->spec_fun && sysdata.specials_enabled==TRUE )
            {
                if ( (*rp->spec_fun) ( rp, NULL, "", NULL, SFT_UPDATE ) )
                    continue;
            }

        }
}

/*
 * Function to check important stuff happening to a player
 * This function should take about 5% of mud cpu time
 */
static void char_check( void )
{
    CHAR_DATA *ch, *ch_next;
    OBJ_DATA *obj;
    EXIT_DATA *pexit;
    static int cnt = 0;
    register int door, retcode;

    cnt = (cnt+1) % 2;

    for ( ch = first_char; ch; ch = ch_next )
    {
        set_cur_char(ch);
        ch_next = ch->next;
        will_fall(ch, 0);

        if ( char_died( ch ) )
            continue;

        if ( IS_NPC( ch ) )
        {
            if ( ch->wait > 0 )
            {
                ch->wait--;
                continue;
            }

            if ( cnt != 0 )
                continue;

            /* running mobs	-Thoric */
            if ( IS_SET(ch->act, ACT_RUNNING) )
            {
                if ( !IS_SET( ch->act, ACT_SENTINEL )
                     &&   !ch->fighting && ch->hunting )
                {
                    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
                    hunt_victim( ch );
                    continue;
                }

                if ( ch->spec_fun && sysdata.specials_enabled==TRUE )
                {
                    if ( (*ch->spec_fun) ( ch, NULL, "", NULL, SFT_UPDATE ) )
                        continue;
                    if ( char_died(ch) )
                        continue;
                }

                if ( !IS_SET(ch->act, ACT_SENTINEL)
                     &&   !IS_SET(ch->act, ACT_PROTOTYPE)
                     && ( door = number_bits( 4 ) ) <= 9
                     && ( pexit = get_exit(ch->in_room, door) ) != NULL
                     &&   pexit->to_room
                     &&   !IS_SET(pexit->exit_info, EX_CLOSED)
                     &&   !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)
                     &&   !IS_SET(pexit->to_room->room_flags, ROOM_DEATH)
                     &&   !will_drown(ch, pexit->to_room)
                     && ( !IS_SET(ch->act, ACT_STAY_AREA)
                          ||   pexit->to_room->area == ch->in_room->area ) )
                {
                    retcode = move_char( ch, pexit, 0 );
                    if ( char_died(ch) )
                        continue;
                    if ( retcode != rNONE || IS_SET(ch->act, ACT_SENTINEL)
                         ||    ch->position < POS_STANDING )
                        continue;
                }
            }
            continue;
        }
        else
        {
            if ( ch->mount
                 &&   ch->in_room != ch->mount->in_room )
            {
                REMOVE_BIT( ch->mount->act, ACT_MOUNTED );
                ch->mount = NULL;
                ch->position = POS_STANDING;
                send_to_char( "No longer upon your mount, you fall to the ground...\n\rOUCH!\n\r", ch );
            }

            if ( ( ch->in_room && ch->in_room->sector_type == SECT_UNDERWATER )
                 || ( ch->in_room && ch->in_room->sector_type == SECT_OCEANFLOOR ) )
            {
                if ( !IS_AFFECTED( ch, AFF_AQUA_BREATH ) )
                {
                    if ( GetMaxLevel(ch) < LEVEL_IMMORTAL )
                    {
                        int dam;

                        /* Changed level of damage at Brittany's request. -- Narn */
                        dam = number_range( GET_MAX_HIT(ch) / 100, GET_MAX_HIT(ch) / 50 );
                        dam = UMAX( 1, dam );
                        if ( number_bits(3) == 0 )
                            send_to_char( "You cough and choke as you try to breathe water!\n\r", ch );
                        damage( ch, ch, dam, TYPE_UNDEFINED );
                    }
                }
            }

            if ( char_died( ch ) )
                continue;

            if ( will_drown(ch, ch->in_room) )
            {
                if (!ch->mount )
                {
                    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
                        if ( obj->item_type == ITEM_BOAT ) break;

                    if ( !obj )
                    {
                        if ( GetMaxLevel(ch) < LEVEL_IMMORTAL )
                        {
                            int mov;
                            int dam;

                            if ( GET_MOVE(ch) > 0 )
                            {
                                mov = number_range( GET_MAX_MOVE(ch) / 20, GET_MAX_MOVE(ch) / 5 );
                                mov = UMAX( 1, mov );

                                if ( GET_MOVE(ch) - mov < 0 )
                                    GET_MOVE(ch) = 0;
                                else
                                    GET_MOVE(ch) -= mov;
                                send_to_char("You are drowning, and quickly becoming tired.\n\r", ch);
                            }
                            else
                            {
                                dam = number_range( GET_MAX_HIT(ch) / 20, GET_MAX_HIT(ch) / 5 );
                                dam = UMAX( 1, dam );

                                if ( number_bits(2) == 0 )
                                    send_to_char( "Struggling with exhaustion, you choke on a mouthful of water.\n\r", ch );
                                damage( ch, ch, dam, TYPE_UNDEFINED );
                            }
                        }
                    }
                }
            }

            /* beat up on link dead players */
            if ( !ch->desc )
            {
                CHAR_DATA *wch, *wch_next;

                for ( wch = ch->in_room->first_person; wch; wch = wch_next )
                {
                    wch_next	= wch->next_in_room;

                    if (!IS_NPC(wch)
                        ||   wch->fighting
                        ||   IS_AFFECTED(wch, AFF_CHARM)
                        ||   !IS_AWAKE(wch)
                        || ( IS_SET(wch->act, ACT_WIMPY) && IS_AWAKE(ch) )
                        ||   !can_see( wch, ch ) )
                        continue;

                    if ( is_hating( wch, ch ) )
                    {
                        found_prey( wch, ch );
                        continue;
                    }

                    if ( (!IS_ACT_FLAG(wch, ACT_META_AGGR) && !IS_ACT_FLAG(wch, ACT_AGGRESSIVE))
                         ||    IS_SET(wch->act, ACT_MOUNTED)
                         ||    IS_SET(wch->in_room->room_flags, ROOM_SAFE ) )
                        continue;

                    global_retcode = multi_hit( wch, ch, TYPE_UNDEFINED );
                }
            }
        }
    }
}


/*
 * Aggress.
 *
 * for each descriptor
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function should take 5% to 10% of ALL mud cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 */
static void aggr_update( void )
{
    DESCRIPTOR_DATA *d, *dnext;
    CHAR_DATA *wch;
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    CHAR_DATA *victim;
    struct act_prog_data *apdtmp;

#ifdef UNDEFD
    /*
     *  GRUNT!  To do
     *
     */
    if ( IS_NPC( wch ) && wch->mpactnum > 0
         && wch->in_room->area->nplayer > 0 )
    {
        MPROG_ACT_LIST * tmp_act, *tmp2_act;
        for ( tmp_act = wch->mpact; tmp_act;
              tmp_act = tmp_act->next )
        {
            oprog_wordlist_check( tmp_act->buf,wch, tmp_act->ch,
                                  tmp_act->obj, tmp_act->vo, ACT_PROG );
            DISPOSE( tmp_act->buf );
        }
        for ( tmp_act = wch->mpact; tmp_act; tmp_act = tmp2_act )
        {
            tmp2_act = tmp_act->next;
            DISPOSE( tmp_act );
        }
        wch->mpactnum = 0;
        wch->mpact    = NULL;
    }
#endif

    /* check mobprog act queue */
    while ( (apdtmp = mob_act_list) != NULL )
    {
        wch = (CHAR_DATA *)mob_act_list->vo;
        if ( !char_died(wch) && wch->mpactnum > 0 )
        {
            MPROG_ACT_LIST * tmp_act;

            while ( (tmp_act = wch->mpact) != NULL )
            {
                if ( tmp_act->obj && obj_extracted(tmp_act->obj) )
                    tmp_act->obj = NULL;
                if ( tmp_act->ch && !char_died(tmp_act->ch) )
                    mprog_wordlist_check( tmp_act->buf, wch, tmp_act->ch,
                                          tmp_act->obj, tmp_act->vo, ACT_PROG );
                wch->mpact = tmp_act->next;
                DISPOSE(tmp_act->buf);
                DISPOSE(tmp_act);
            }
            wch->mpactnum = 0;
            wch->mpact    = NULL;
        }
        mob_act_list = apdtmp->next;
        DISPOSE( apdtmp );
    }


    /*
     * Just check descriptors here for victims to aggressive mobs
     * We can check for linkdead victims to mobile_update	-Thoric
     */
    for ( d = first_descriptor; d; d = dnext )
    {
        dnext = d->next;
        if ( d->connected != CON_PLAYING || (wch=d->character) == NULL )
            continue;

        if ( char_died(wch)
             ||   IS_NPC(wch)
             ||   GetMaxLevel(wch) >= LEVEL_IMMORTAL
             ||  !wch->in_room )
            continue;

        for ( ch = wch->in_room->first_person; ch; ch = ch_next )
        {
            int count;

            ch_next	= ch->next_in_room;

            if ( !IS_NPC(ch)
                 ||   ch->fighting
                 ||   (!IS_SET(ch->act, ACT_AGGRESSIVE) &&
                       !IS_SET(ch->act, ACT_META_AGGR))
                 ||   IS_SET(ch->act, ACT_WIMPY)
                 ||   IS_SET(ch->act, ACT_MOUNTED)
                 ||   IS_SET(ch->in_room->room_flags, ROOM_SAFE )
                 ||   IS_AFFECTED(ch, AFF_CHARM)
                 ||   !IS_AWAKE(ch)
                 ||   !can_see( ch, wch )
                 ||   char_died(ch) )
                continue;

            if ( is_hating( ch, wch ) )
            {
                found_prey( ch, wch );
                continue;
            }

            if ( !IS_SET(ch->act, ACT_META_AGGR) &&
                 number_percent() > PERCENT_AGGRO )
                continue;

            /*
             * check for an aggr spec
             */
            if (ch->spec_fun)
            {
                if ( (*ch->spec_fun) ( ch, NULL, "", NULL, SFT_AGGR_UPDATE ) )
                    continue;
                if (spec_racial_specifics(ch, NULL, "", NULL, SFT_AGGR_UPDATE))
                    continue;
                if ( char_died(ch) )
                    continue;
            }

            /*
             * Ok we have a 'wch' player character and a 'ch' npc aggressor.
             * Now make the aggressor fight a RANDOM pc victim in the room,
             *   giving each 'vch' an equal chance of selection.
             */
            count	= 0;
            victim	= NULL;
            for ( vch = wch->in_room->first_person; vch; vch = vch_next )
            {
                vch_next = vch->next_in_room;

                if ( !IS_NPC(vch)
                     &&   GetMaxLevel(vch) < LEVEL_IMMORTAL
                     &&   ( !IS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch) )
                     &&   can_see( ch, vch ) )
                {
                    if ( number_range( 0, count ) == 0 )
                        victim = vch;
                    count++;
                }
            }

            if ( !victim )
            {
                bug( "Aggr_update: null victim, count %d.", count );
                continue;
            }

            if ( IS_NPC(ch) && IS_SET(ch->attacks, ATCK_BACKSTAB ) )
            {
                OBJ_DATA *obj;

                if ( !ch->mount
                     && (obj = get_eq_char( ch, WEAR_WIELD )) != NULL
                     && obj->value[3] == 11
                     && !victim->fighting
                     && GET_HIT(victim) >= GET_MAX_HIT(victim) )
                {
                    check_attacker( ch, victim );
                    spell_lag(ch, gsn_backstab);
                    if ( !IS_AWAKE(victim)
                         ||   number_percent( )+5 < GetAveLevel(ch) )
                    {
                        global_retcode = multi_hit( ch, victim, gsn_backstab );
                        continue;
                    }
                    else
                    {
                        global_retcode = damage( ch, victim, 0, gsn_backstab );
                        continue;
                    }
                }
            }
            if ( IS_SET(ch->act, ACT_META_AGGR) || PERCENT_AGGRO > 100)
                multi_hit(ch, victim, TYPE_UNDEFINED);
            else
                set_fighting( ch, victim);
        }
    }

    return;
}

/* From interp.c */
bool check_social  args( ( CHAR_DATA *ch, char *command, char *argument ) );

/*
 * drunk randoms	- Tricops
 * (Made part of mobile_update	-Thoric)
 */
static void drunk_randoms( CHAR_DATA *ch )
{
    CHAR_DATA *rvch = NULL;
    CHAR_DATA *vch;
    sh_int drunk;
    sh_int position;

    if ( IS_NPC( ch ) || GET_COND(ch, COND_DRUNK) <= 0 )
        return;

    if ( number_percent() < 30 )
        return;

    drunk = GET_COND(ch, COND_DRUNK);
    position = ch->position;
    ch->position = POS_STANDING;

    if ( number_percent() < (2*drunk / 20) )
        check_social( ch, "burp", "" );
    else
        if ( number_percent() < (2*drunk / 20) )
            check_social( ch, "hiccup", "" );
        else
            if ( number_percent() < (2*drunk / 20) )
                check_social( ch, "drool", "" );
            else
                if ( number_percent() < (2*drunk / 20) )
                    check_social( ch, "fart", "" );
                else
                    if ( drunk > (10+(get_curr_con(ch)/5))
                         &&   number_percent() < ( 2 * drunk / 18 ) )
                    {
                        for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
                            if ( number_percent() < 10 )
				rvch = vch;
                        if (rvch)
			    check_social( ch, "puke", rvch->name );
                        else
			    check_social( ch, "puke", "" );
                    }

    ch->position = position;
    return;
}

static void halucinations( CHAR_DATA *ch )
{
    if ( ch->mental_state >= 30 && number_bits(5 - (ch->mental_state >= 50) - (ch->mental_state >= 75)) == 0 )
    {
        char *t;

        switch( number_range( 1, UMIN(20, (ch->mental_state+5) / 5)) )
        {
        default:
        case  1: t = "You feel very restless... you can't sit still.\n\r";		break;
        case  2: t = "You're tingling all over.\n\r";				break;
        case  3: t = "Your skin is crawling.\n\r";					break;
        case  4: t = "You suddenly feel that something is terribly wrong.\n\r";	break;
        case  5: t = "Those damn little fairies keep laughing at you!\n\r";		break;
        case  6: t = "You can hear your mother crying...\n\r";			break;
        case  7: t = "Have you been here before, or not?  You're not sure...\n\r";	break;
        case  8: t = "Painful childhood memories flash through your mind.\n\r";	break;
        case  9: t = "You hear someone call your name in the distance...\n\r";	break;
        case 10: t = "Your head is pulsating... you can't think straight.\n\r";	break;
        case 11: t = "The ground... seems to be squirming...\n\r";			break;
        case 12: t = "You're not quite sure what is real anymore.\n\r";		break;
        case 13: t = "It's all a dream... or is it?\n\r";				break;
        case 14: t = "They're coming to get you... coming to take you away...\n\r";	break;
        case 15: t = "You begin to feel all powerful!\n\r";				break;
        case 16: t = "You're light as air... the heavens are yours for the taking.\n\r";	break;
        case 17: t = "Your whole life flashes by... and your future...\n\r";	break;
        case 18: t = "You are everywhere and everything... you know all and are all!\n\r";	break;
        case 19: t = "You feel immortal!\n\r";					break;
        case 20: t = "Ahh... the power of a Supreme Entity... what to do...\n\r";	break;
        }
        send_to_char( t, ch );
    }
    return;
}

static void tele_update( void )
{
    TELEPORT_DATA *tele, *tele_next;

    if ( !first_teleport )
        return;

    for ( tele = first_teleport; tele; tele = tele_next )
    {
        tele_next = tele->next;
        if ( --tele->timer <= 0 )
        {
            if ( tele->room->first_person )
            {
                if ( IS_SET( tele->room->room_flags, ROOM_TELESHOWDESC ) )
                    teleport( tele->room->first_person, tele->room->tele_vnum,
                              TELE_SHOWDESC | TELE_TRANSALL );
                else
                    teleport( tele->room->first_person, tele->room->tele_vnum,
                              TELE_TRANSALL );
            }
            UNLINK( tele, first_teleport, last_teleport, next, prev );
            DISPOSE( tele );
        }
    }
}

#if FALSE
/*
 * Write all outstanding authorization requests to Log channel - Gorog
 */
static void auth_update( void )
{
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool first_time = TRUE;         /* so titles are only done once */

    for ( d = first_descriptor; d; d = d->next )
    {
        victim = d->character;
        if ( victim && IS_WAITING_FOR_AUTH(victim) )
        {
            if ( first_time )
            {
                first_time = FALSE;
                strcpy (log_buf, "Pending authorizations:" );
                log_string_plus( log_buf,LOG_MONITOR,LEVEL_IMMORTAL,SEV_CRIT);
            }
            sprintf( log_buf, " %s@%s new %s %s", victim->name,
                     victim->desc->host, race_table[victim->race].race_name,
                     GetClassString(victim) );
            log_string_plus( log_buf,LOG_MONITOR,LEVEL_IMMORTAL,SEV_CRIT);
        }
    }
}
#endif

static void auth_update( void )
{
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    char buf [MAX_INPUT_LENGTH];
    bool found_hit = FALSE;         /* was at least one found? */

    strcpy (log_buf, "Pending authorizations:\n\r" );
    for ( d = first_descriptor; d; d = d->next )
    {
        if ( (victim = d->character) && IS_WAITING_FOR_AUTH(victim) )
        {
            found_hit = TRUE;
            sprintf( buf, " %s@%s new %s %s\n\r", victim->name,
                     victim->desc->host, race_table[victim->race].race_name,
                     GetClassString(victim) );
            strcat (log_buf, buf);
        }
    }
    if (found_hit)
    {
        log_string_plus( log_buf,LOG_MONITOR,LEVEL_IMMORTAL,SEV_CRIT);
    }
}


static void affect_update( CHAR_DATA *ch )
{
    AFFECT_DATA *paf, *paf_next;
    SKILLTYPE *skill;

    for ( paf = ch->first_affect; paf; paf = paf_next )
    {
        paf_next = paf->next;
        if ( paf->duration > 0 )
            paf->duration--;
        else if ( paf->duration < 0 )
            ;
        else if ( paf->duration == 25 )
        {
            skill = get_skilltype(paf->type);
            if (skill->msg_off_soon)
                act( AT_WEAROFF, skill->msg_off_soon, ch, NULL, NULL, TO_CHAR );
            if (skill->msg_off_soon_room)
                act( AT_WEAROFF, skill->msg_off_soon_room, ch, NULL, NULL, TO_ROOM );
        }
        else
        {
            if ( !paf_next ||
                 paf_next->type != paf->type ||
                 paf_next->duration > 0 )
            {
                skill = get_skilltype(paf->type);
                if ( paf->type > 0 && skill && skill->msg_off )
                {
                    set_char_color( AT_WEAROFF, ch );
                    send_to_char( skill->msg_off, ch );
                    send_to_char( "\n\r", ch );
                    if (skill->msg_off_room)
                        act( AT_WEAROFF, skill->msg_off_room, ch, NULL, NULL, TO_ROOM );
                }
            }
            if (paf->type == gsn_possess)
            {
                ch->desc->character = ch->desc->original;
                ch->desc->original = NULL;
                ch->desc->character->desc = ch->desc;
                ch->desc->character->switched = NULL;
                ch->desc = NULL;
            }
            affect_remove( ch, paf );
        }
    }
}

static void spell_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *lst_ch;

    lst_ch = NULL;
    for ( ch = last_char; ch; lst_ch = ch, ch = gch_prev )
    {
        set_cur_char( ch );

        if ( ch == first_char && ch->prev )
        {
            bug( "ERROR: first_char->prev != NULL, fixing..." );
            ch->prev = NULL;
        }

        gch_prev	= ch->prev;

        if ( gch_prev && gch_prev->next != ch )
        {
	    bug( "FATAL: spell_update: %s->prev->next doesn't point to ch.",
                     ch->name );
            bug( "Short-cutting here" );
            ch->prev = NULL;
            gch_prev = NULL;
            do_shout( ch, "Thoric says, 'Prepare for the worst!'" );
        }

        /*
         * See if we got a pointer to someone who recently died...
         * if so, either the pointer is bad... or it's a player who
         * "died", and is back at the healer...
         * Since he/she's in the char_list, it's likely to be the later...
         * and should not already be in another fight already
         */
        if ( char_died(ch) )
            continue;

        /*
         * See if we got a pointer to some bad looking data...
         */
        if ( !ch->in_room || !ch->name )
        {
            log_string_plus( "spell_update: bad ch record!  (Shortcutting.)", LOG_NORMAL, LEVEL_LOG_CSET, SEV_ALERT );
            sprintf( log_buf, "ch: %d  ch->in_room: %d  ch->prev: %d  ch->next: %d",
                     (int) ch, (int) ch->in_room, (int) ch->prev, (int) ch->next );
            log_string_plus( log_buf, LOG_NORMAL, LEVEL_LOG_CSET, SEV_ALERT );
            log_string_plus( lastplayercmd, LOG_NORMAL, LEVEL_LOG_CSET, SEV_ALERT );
            if ( lst_ch )
                sprintf( log_buf, "lst_ch: %d  lst_ch->prev: %d  lst_ch->next: %d",
                         (int) lst_ch, (int) lst_ch->prev, (int) lst_ch->next );
            else
                strcpy( log_buf, "lst_ch: NULL" );
            log_string_plus( log_buf, LOG_NORMAL, LEVEL_LOG_CSET, SEV_ALERT );
            gch_prev = NULL;
            continue;
        }

        if ( char_died(ch) )
            continue;

        /*
         * We need spells that have shorter durations than an hour.
         * So a melee round sounds good to me... -Thoric
         */
        affect_update(ch);
    }
    return;
}

void init_update( void )
{
    memset(&userec_area, 0, sizeof(userec_area));
    memset(&userec_violence, 0, sizeof(userec_violence));
    memset(&userec_spell, 0, sizeof(userec_spell));
    memset(&userec_mobile, 0, sizeof(userec_mobile));
    memset(&userec_aggr, 0, sizeof(userec_aggr));
    memset(&userec_objact, 0, sizeof(userec_objact));
    memset(&userec_act, 0, sizeof(userec_act));
}

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void update_handler( void )
{
    struct timeval start_time;
    struct timeval etime;
#ifdef MUD_DEBUG
    struct timeval time_used;
#endif

    if ( timechar )
    {
        set_char_color(AT_PLAIN, timechar);
        send_to_char( "Starting update timer.\n\r", timechar );
        gettimeofday(&start_time, NULL);
    }

    if ( --pulse_area     <= 0 )
    {
        pulse_area	= PULSE_AREA;
#ifdef MUD_DEBUG
        start_timer(&time_used);
#endif
        area_update	( );
#ifdef MUD_DEBUG
        end_timer(&time_used);
        update_userec(&time_used, &userec_area);
#endif
    }

    if ( --pulse_mobile   <= 0 )
    {
        pulse_mobile	= PULSE_MOBILE;
#ifdef MUD_DEBUG
        start_timer(&time_used);
#endif
        mobile_update  ( );
#ifdef MUD_DEBUG
        end_timer(&time_used);
        update_userec(&time_used, &userec_mobile);
#endif
    }

    if ( --pulse_violence <= 0 )
    {
        pulse_violence	= PULSE_VIOLENCE;
#ifdef MUD_DEBUG
        start_timer(&time_used);
#endif
        violence_update	( );
#ifdef MUD_DEBUG
        end_timer(&time_used);
        update_userec(&time_used, &userec_violence);
#endif
    }

    if ( --pulse_spell    <= 0 )
    {
        pulse_spell     = PULSE_SPELL;
#ifdef MUD_DEBUG
        start_timer(&time_used);
#endif
        spell_update ( );
#ifdef MUD_DEBUG
        end_timer(&time_used);
        update_userec(&time_used, &userec_spell);
#endif
    }

    if ( --pulse_point    <= 0 )
    {
        pulse_point     = number_range( (int)(PULSE_TICK * 0.75), (int)(PULSE_TICK * 1.25) );

        auth_update     ( );			/* Gorog */
        time_update	( );
        weather_update	( );
        char_update	( );
        obj_update	( );
        room_update	( );
        clear_vrooms	( );			/* remove virtual rooms */
    }

    if ( --pulse_second   <= 0 )
    {
        pulse_second	= PULSE_PER_SECOND;

#if USE_MUDMSG
        mud_recv_message();
#endif

	char_check( );
        /*reboot_check( "" ); Disabled to check if its lagging a lot - Scryn*/
        /* Much faster version enabled by Altrag..
         although I dunno how it could lag too much, it was just a bunch
         of comparisons.. */
	reboot_check(0);
        acro_check( );
    }

    if ( auction->item && --auction->pulse <= 0 )
    {
        auction->pulse  = PULSE_AUCTION;
        auction_update( );
    }


    if ( --pulse_teleport <= 0 )
    {
        pulse_teleport  = PULSE_PER_SECOND/10;
        tele_update( );
    }

#ifdef MUD_DEBUG
    start_timer(&time_used);
#endif
    aggr_update( );
#ifdef MUD_DEBUG
    end_timer(&time_used);
    update_userec(&time_used, &userec_aggr);
    start_timer(&time_used);
#endif
    obj_act_update ( );
#ifdef MUD_DEBUG
    end_timer(&time_used);
    update_userec(&time_used, &userec_objact);
    start_timer(&time_used);
#endif
    room_act_update( );
#ifdef MUD_DEBUG
    end_timer(&time_used);
    update_userec(&time_used, &userec_act);
#endif

    if ( timechar )
    {
        gettimeofday(&etime, NULL);
        set_char_color(AT_PLAIN, timechar);
        send_to_char( "Update timing complete.\n\r", timechar );
        subtract_times(&etime, &start_time);
        ch_printf( timechar, "Timing took %ld.%06ld seconds.\n\r",
                   etime.tv_sec, etime.tv_usec );
        timechar = NULL;
    }
    tail_chain( );
    return;
}


void remove_portal( OBJ_DATA *portal )
{
    ROOM_INDEX_DATA *fromRoom, *toRoom;
    CHAR_DATA *ch;
    EXIT_DATA *pexit;
    bool found;

    if ( !portal )
    {
        bug( "remove_portal: portal is NULL" );
        return;
    }

    fromRoom = portal->in_room;
    found = FALSE;
    if ( !fromRoom )
    {
        bug( "remove_portal: portal->in_room is NULL" );
        return;
    }

    for ( pexit = fromRoom->first_exit; pexit; pexit = pexit->next )
        if ( IS_SET( pexit->exit_info, EX_PORTAL ) )
        {
            found = TRUE;
            break;
        }

    if ( !found )
    {
        bug( "remove_portal: portal not found in room %d!", fromRoom->vnum );
        return;
    }

    if ( pexit->vdir != DIR_PORTAL )
        bug( "remove_portal: exit in dir %d != DIR_PORTAL", pexit->vdir );

    if ( ( toRoom = pexit->to_room ) == NULL )
       bug( "remove_portal: toRoom is NULL" );

    extract_exit( fromRoom, pexit );
    /* rendunancy */
    /* send a message to fromRoom */
    /* ch = fromRoom->first_person; */
    /* if(ch!=NULL) */
    /* act( AT_PLAIN, "A magical portal below winks from existence.", ch, NULL, NULL, TO_ROOM ); */

    /* send a message to toRoom */
    if ( toRoom && (ch = toRoom->first_person) != NULL )
        act( AT_PLAIN, "A magical portal above winks from existence.", ch, NULL, NULL, TO_ROOM );

    /* remove the portal obj: looks better to let update_obj do this */
    /* extract_obj(portal);  */

    return;
}

void reboot_check( time_t reset )
{
    static char *tmsg[] =
    { "You feel the ground shake as the end comes near!",
    "Lightning crackles in the sky above!",
    "Crashes of thunder sound across the land!",
    "The sky has suddenly turned midnight black.",
    "You notice the life forms around you slowly dwindling away.",
    "The seas across the realm have turned frigid.",
    "The aura of magic that surrounds the realms seems slightly unstable.",
    "You sense a change in the magical forces surrounding you."
    };
    static const int times[] = { 60, 120, 180, 240, 300, 600, 900, 1800 };
    static const int timesize =
        UMIN(sizeof(times)/sizeof(*times), sizeof(tmsg)/sizeof(*tmsg));
    char buf[MAX_STRING_LENGTH];
    static int trun;
    static bool init;

    if ( !init || reset >= current_time )
    {
        for ( trun = timesize-1; trun >= 0; trun-- )
            if ( reset >= current_time+times[trun] )
                break;
        init = TRUE;
        return;
    }

    if ( (current_time % 1800) == 0 )
    {
        sprintf(buf, "%.24s: %d players", ctime(&current_time), num_descriptors);
        append_to_file(USAGE_FILE, buf);
    }

    if ( new_boot_time_t - boot_time < 60*60*18 &&
         !set_boot_time->manual )
        return;

    if ( new_boot_time_t <= current_time )
    {
        CHAR_DATA *vch;
        extern bool mud_down;

        if ( auction->item )
        {
            sprintf(buf, "Sale of %s has been stopped by mud.",
                    auction->item->short_descr);
            talk_auction(buf);
            obj_to_char(auction->item, auction->seller);
            auction->item = NULL;
            if ( auction->buyer && auction->buyer != auction->seller )
            {
                GET_MONEY(auction->buyer,auction->currtype) += auction->bet;
                send_to_char("Your money has been returned.\n\r", auction->buyer);
            }
        }
        echo_to_all(AT_YELLOW, "You are forced from these realms by a strong "
                    "magical presence\n\ras life here is reconstructed.", ECHOTAR_ALL);

        for ( vch = first_char; vch; vch = vch->next )
            if ( !IS_NPC(vch) )
                save_char_obj(vch);
        mud_down = TRUE;
        return;
    }

    if ( trun != -1 && new_boot_time_t - current_time <= times[trun] )
    {
        echo_to_all(AT_YELLOW, tmsg[trun], ECHOTAR_ALL);
        if ( trun <= 5 )
            sysdata.DENY_NEW_PLAYERS = TRUE;
        --trun;
        return;
    }
    return;
}

#if 0
void reboot_check( char *arg )
{
    char buf[MAX_STRING_LENGTH];
    extern bool mud_down;
    /*struct tm *timestruct;
     int timecheck;*/
    CHAR_DATA *vch;

    /*Bools to show which pre-boot echoes we've done. */
    static bool thirty  = FALSE;
    static bool fifteen = FALSE;
    static bool ten     = FALSE;
    static bool five    = FALSE;
    static bool four    = FALSE;
    static bool three   = FALSE;
    static bool two     = FALSE;
    static bool one     = FALSE;

    /* This function can be called by do_setboot when the reboot time
     is being manually set to reset all the bools. */
    if ( !str_cmp( arg, "reset" ) )
    {
        thirty  = FALSE;
        fifteen = FALSE;
        ten     = FALSE;
        five    = FALSE;
        four    = FALSE;
        three   = FALSE;
        two     = FALSE;
        one     = FALSE;
        return;
    }

    /* If the mud has been up less than 18 hours and the boot time
     wasn't set manually, forget it. */
    /* Usage monitor */

    if ((current_time % 1800) == 0)
    {
        sprintf(buf, "%s: %d players", ctime(&current_time), num_descriptors);
        append_to_file(USAGE_FILE, buf);
    }

    /* Change by Scryn - if mud has not been up 18 hours at boot time - still
     * allow for warnings even if not up 18 hours
     */
    if ( new_boot_time_t - boot_time < 60*60*18
         && set_boot_time->manual == 0 )
    {
        return;
    }
    /*
     timestruct = localtime( &current_time);

     if ( timestruct->tm_hour == set_boot_time->hour
     && timestruct->tm_min  == set_boot_time->min )*/
    if ( new_boot_time_t <= current_time )
    {
        /* Return auction item to seller */
        if (auction->item != NULL)
        {
            sprintf (buf,"Sale of %s has been stopped by mud.",
                     auction->item->short_descr);
            talk_auction (buf);
            obj_to_char (auction->item, auction->seller);
            auction->item = NULL;
            if (auction->buyer != NULL && auction->seller != auction->buyer) /* return money to the buyer */
            {
                GET_MONEY(auction->buyer,auction->currtype) += auction->bet;
                send_to_char ("Your money has been returned.\n\r",auction->buyer);
            }
        }

        sprintf( buf, "You are forced from these realms by a strong magical presence" );
        echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
        sprintf( buf, "as life here is reconstructed." );
        echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );

        /* Save all characters before booting. */
        for ( vch = first_char; vch; vch = vch->next )
        {
            if ( !IS_NPC( vch ) )
                save_char_obj( vch );
        }
        mud_down = TRUE;
    }

    /* How many minutes to the scheduled boot? */
    /*  timecheck = ( set_boot_time->hour * 60 + set_boot_time->min )
     - ( timestruct->tm_hour * 60 + timestruct->tm_min );

     if ( timecheck > 30  || timecheck < 0 ) return;

     if ( timecheck <= 1 ) */
    if ( new_boot_time_t - current_time <= 60 )
    {
        if ( one == FALSE )
        {
            sprintf( buf, "You feel the ground shake as the end comes near!" );
            echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
            one = TRUE;
            sysdata.DENY_NEW_PLAYERS = TRUE;
        }
        return;
    }

    /*  if ( timecheck == 2 )*/
    if ( new_boot_time_t - current_time <= 120 )
    {
        if ( two == FALSE )
        {
            sprintf( buf, "Lightning crackles in the sky above!" );
            echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
            two = TRUE;
            sysdata.DENY_NEW_PLAYERS = TRUE;
        }
        return;
    }

    /*  if ( timecheck == 3 )*/
    if (new_boot_time_t - current_time <= 180 )
    {
        if ( three == FALSE )
        {
            sprintf( buf, "Crashes of thunder sound across the land!" );
            echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
            three = TRUE;
            sysdata.DENY_NEW_PLAYERS = TRUE;
        }
        return;
    }

    /*  if ( timecheck == 4 )*/
    if( new_boot_time_t - current_time <= 240 )
    {
        if ( four == FALSE )
        {
            sprintf( buf, "The sky has suddenly turned midnight black." );
            echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
            four = TRUE;
            sysdata.DENY_NEW_PLAYERS = TRUE;
        }
        return;
    }

    /*  if ( timecheck == 5 )*/
    if( new_boot_time_t - current_time <= 300 )
    {
        if ( five == FALSE )
        {
            sprintf( buf, "You notice the life forms around you slowly dwindling away." );
            echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
            five = TRUE;
            sysdata.DENY_NEW_PLAYERS = TRUE;
        }
        return;
    }

    /*  if ( timecheck == 10 )*/
    if( new_boot_time_t - current_time <= 600 )
    {
        if ( ten == FALSE )
        {
            sprintf( buf, "The seas across the realm have turned frigid." );
            echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
            ten = TRUE;
        }
        return;
    }

    /*  if ( timecheck == 15 )*/
    if( new_boot_time_t - current_time <= 900 )
    {
        if ( fifteen == FALSE )
        {
            sprintf( buf, "The aura of magic which once surrounded the realms seems slightly unstable." );
            echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
            fifteen = TRUE;
        }
        return;
    }

    /*  if ( timecheck == 30 )*/
    if( new_boot_time_t - current_time <= 1800 )
    {
        if ( thirty == FALSE )
        {
            sprintf( buf, "You sense a change in the magical forces surrounding you." );
            echo_to_all( AT_YELLOW, buf, ECHOTAR_ALL );
            thirty = TRUE;
        }
        return;
    }

    return;
}
#endif

/* the auction update*/

static void auction_update (void)
{
    int tax, pay;
    char buf[MAX_STRING_LENGTH];

    switch (++auction->going) /* increase the going state */
    {
    case 1 : /* going once */
    case 2 : /* going twice */
        if (auction->bet > auction->starting)
            sprintf (buf, "%s: going %s for %d.", auction->item->short_descr,
                     ((auction->going == 1) ? "once" : "twice"), auction->bet);
        else
            sprintf (buf, "%s: going %s (bid not received yet).", auction->item->short_descr,
                     ((auction->going == 1) ? "once" : "twice"));

        talk_auction (buf);
        break;

    case 3 : /* SOLD! */
        if (!auction->buyer && auction->bet)
        {
            bug( "Auction code reached SOLD, with NULL buyer, but %d %s bid", auction->bet, curr_types[auction->currtype] );
            auction->bet = 0;
        }
        if (auction->bet > 0 && auction->buyer != auction->seller)
        {
            sprintf (buf, "%s sold to %s for %d.",
                     auction->item->short_descr,
                     IS_NPC(auction->buyer) ? auction->buyer->short_descr : auction->buyer->name,
                     auction->bet);
            talk_auction(buf);

            act(AT_ACTION, "The auctioneer materializes before you, and hands you $p.",
                auction->buyer, auction->item, NULL, TO_CHAR);
            act(AT_ACTION, "The auctioneer materializes before $n, and hands $m $p.",
                auction->buyer, auction->item, NULL, TO_ROOM);

            if ( (carry_w(auction->buyer)
                  +     get_obj_weight( auction->item ))
                 >     can_carry_w( auction->buyer ) )
            {
                act( AT_PLAIN, "$p is too heavy for you to carry with your current inventory.", auction->buyer, auction->item, NULL, TO_CHAR );
                act( AT_PLAIN, "$n is carrying too much to also carry $p, and $e drops it.", auction->buyer, auction->item, NULL, TO_ROOM );
                obj_to_room( auction->item, auction->buyer->in_room );
            }
            else
                obj_to_char( auction->item, auction->buyer );
            pay = (int)((float)auction->bet * 0.9);
            tax = (int)((float)auction->bet * 0.1);
            boost_economy( auction->seller->in_room->area, tax, auction->currtype );
            GET_MONEY(auction->seller,auction->currtype) += pay; /* give him the money, tax 10 % */
            sprintf(buf, "The auctioneer pays you %d %s, charging an auction fee of %d.\n\r", pay, curr_types[auction->currtype], tax);
            send_to_char(buf, auction->seller);
            auction->item = NULL; /* reset item */
            if ( IS_SET( sysdata.save_flags, SV_AUCTION ) )
            {
                save_char_obj( auction->buyer );
                save_char_obj( auction->seller );
            }
        }
        else /* not sold */
        {
            sprintf (buf, "No bids received for %s - object has been removed from auction\n\r.",auction->item->short_descr);
            talk_auction(buf);
            act (AT_ACTION, "The auctioneer appears before you to return $p to you.",
                 auction->seller,auction->item,NULL,TO_CHAR);
            act (AT_ACTION, "The auctioneer appears before $n to return $p to $m.",
                 auction->seller,auction->item,NULL,TO_ROOM);
            if ( (carry_w(auction->seller)
                  +     get_obj_weight( auction->item ))
                 >     can_carry_w( auction->seller ) )
            {
                act( AT_PLAIN, "You drop $p as it is just too much to carry"
                     " with everything else you're carrying.", auction->seller,
                     auction->item, NULL, TO_CHAR );
                act( AT_PLAIN, "$n drops $p as it is too much extra weight"
                     " for $m with everything else.", auction->seller,
                     auction->item, NULL, TO_ROOM );
                obj_to_room( auction->item, auction->seller->in_room );
            }
            else
                obj_to_char (auction->item,auction->seller);
	    tax = (int)((float)auction->item->cost * 0.05);
            boost_economy( auction->seller->in_room->area, tax, auction->currtype );
            sprintf(buf, "The auctioneer charges you an auction fee of %d.\n\r", tax );
            send_to_char(buf, auction->seller);
            if ((GET_MONEY(auction->seller, auction->currtype) - tax) < 0)
                GET_MONEY(auction->seller, auction->currtype) = 0;
            else
                GET_MONEY(auction->seller, auction->currtype) -= tax;
            if ( IS_SET( sysdata.save_flags, SV_AUCTION ) )
                save_char_obj( auction->seller );
        } /* else */
        auction->item = NULL; /* clear auction */
    } /* switch */
} /* func */

void subtract_times(struct timeval *etime, struct timeval *start_time)
{
    etime->tv_sec -= start_time->tv_sec;
    etime->tv_usec -= start_time->tv_usec;
    while ( etime->tv_usec < 0 )
    {
        etime->tv_usec += 1000000;
        etime->tv_sec--;
    }
    return;
}

void char_regen( CHAR_DATA *ch )
{
    if (!IS_NPC(ch) && GET_COND(ch, COND_FULL)<=1)
    {
        send_to_char("Your stomach growls loudly.\n\r",ch);
        return;
    }

    GET_HIT(ch) = UMIN(number_fuzzy(4)+GET_HIT(ch),GET_MAX_HIT(ch));
    if (!IS_NPC(ch))
    {
        gain_condition(ch, COND_FULL, -2);
        ch->alignment = UMAX(GET_ALIGN(ch)-5, -1000);
    }
    send_to_char("Your wounds heal themselves.\n\r", ch);
}

static void char_pulse_update( CHAR_DATA *ch )
{

    if (GET_HIT(ch) >= GET_MAX_HIT(ch))
        return;

    if (GET_RACE(ch) == RACE_TROLL)
        char_regen(ch);
}

/*
 * Function to update weather vectors according to climate
 * settings, random effects, and neighboring areas.
 * Last modified: July 18, 1997
 * - Fireblade
 */
void adjust_vectors(WEATHER_DATA *weather)
{
    NEIGHBOR_DATA *neigh;
    register double dT, dP, dW;

    if(!weather)
    {
        bug("adjust_vectors: NULL weather data.");
        return;
    }

    dT = 0;
    dP = 0;
    dW = 0;

    /* Add in random effects */
    dT += number_range(-rand_factor, rand_factor);
    dP += number_range(-rand_factor, rand_factor);
    dW += number_range(-rand_factor, rand_factor);

    /* Add in climate effects*/
    dT += climate_factor *
        (((weather->climate_temp - 2)*weath_unit) -
         (weather->temp))/weath_unit;
    dP += climate_factor *
        (((weather->climate_precip - 2)*weath_unit) -
         (weather->precip))/weath_unit;
    dW += climate_factor *
        (((weather->climate_wind - 2)*weath_unit) -
         (weather->wind))/weath_unit;


    /* Add in effects from neighboring areas */
    for(neigh = weather->first_neighbor; neigh; neigh = neigh->next)
    {
        /* see if we have the area cache'd already */
        if(!neigh->address)
        {
            /* try and find address for area */
            neigh->address = get_area(neigh->name);

            /* if couldn't find area ditch the neigh */
            if(!neigh->address)
            {
                NEIGHBOR_DATA *temp;
                bug("adjust_weather: "
                    "invalid area name.");
                temp = neigh->prev;
                UNLINK(neigh,
                       weather->first_neighbor,
                       weather->last_neighbor,
                       next,
                       prev);
                STRFREE(neigh->name);
                DISPOSE(neigh);
                neigh = temp;
                continue;
            }
        }

        dT +=(neigh->address->weather->temp -
              weather->temp) / neigh_factor;
        dP +=(neigh->address->weather->precip -
              weather->precip) / neigh_factor;
        dW +=(neigh->address->weather->wind -
              weather->wind) / neigh_factor;
    }

    /* now apply the effects to the vectors */
    weather->temp_vector += (int)dT;
    weather->precip_vector += (int)dP;
    weather->wind_vector += (int)dW;

    /* Make sure they are within the right range */
    weather->temp_vector = URANGE(-max_vector,
                                  weather->temp_vector, max_vector);
    weather->precip_vector = URANGE(-max_vector,
                                    weather->precip_vector, max_vector);
    weather->wind_vector = URANGE(-max_vector,
                                  weather->wind_vector, max_vector);

    return;
}

/*
 * function updates weather for each area
 * Last Modified: July 31, 1997
 * Fireblade
 */
void weather_update()
{
    AREA_DATA *pArea;
    DESCRIPTOR_DATA *d;
    register int limit;

    limit = 3 * weath_unit;

    for(pArea = first_area; pArea;
        pArea = (pArea == last_area) ? first_build : pArea->next)
    {
        /* Apply vectors to fields */
        pArea->weather->temp +=
            pArea->weather->temp_vector;
        pArea->weather->precip +=
            pArea->weather->precip_vector;
        pArea->weather->wind +=
            pArea->weather->wind_vector;

        /* Make sure they are within the proper range */
        pArea->weather->temp = URANGE(-limit,
                                      pArea->weather->temp, limit);
        pArea->weather->precip = URANGE(-limit,
                                        pArea->weather->precip, limit);
        pArea->weather->wind = URANGE(-limit,
                                      pArea->weather->wind, limit);

        /* get an appropriate echo for the area */
        get_weather_echo(pArea->weather);
    }

    for(pArea = first_area; pArea;
        pArea = (pArea == last_area) ? first_build : pArea->next)
    {
        adjust_vectors(pArea->weather);
    }

    /* display the echo strings to the appropriate players */
    for(d = first_descriptor; d; d = d->next)
    {
        WEATHER_DATA *weath;

        if (d->connected == CON_PLAYING &&
            IS_OUTSIDE(d->character) &&
            !NO_WEATHER_SECT(d->character->in_room->sector_type) &&
            IS_AWAKE(d->character))
        {
            weath = d->character->in_room->area->weather;
            if(!weath->echo)
                continue;
            set_char_color(weath->echo_color, d->character);
            ch_printf(d->character, "%s", weath->echo);
        }
    }

    return;
}

/*
 * get weather echo messages according to area weather...
 * stores echo message in weath_data.... must be called before
 * the vectors are adjusted
 * Last Modified: August 10, 1997
 * Fireblade
 */
static void get_weather_echo(WEATHER_DATA *weath)
{
    int n;
    int temp, precip, wind;
    int dT, dP, dW;
    int tindex, pindex, windex;

    /* set echo to be nothing */
    weath->echo = NULL;
    weath->echo_color = AT_GREY;

    /* get the random number */
    n = number_bits(2);

    /* variables for convenience */
    temp = weath->temp;
    precip = weath->precip;
    wind = weath->wind;

    dT = weath->temp_vector;
    dP = weath->precip_vector;
    dW = weath->wind_vector;

    tindex = (temp + 3*weath_unit - 1)/weath_unit;
    pindex = (precip + 3*weath_unit - 1)/weath_unit;
    windex = (wind + 3*weath_unit - 1)/weath_unit;

    /* get the echo string... mainly based on precip */
    switch(pindex)
    {
    case 0:
        if(precip - dP > -2*weath_unit)
        {
            char *echo_strings[4] =
            {
                "The clouds disappear.\n\r",
                "The clouds disappear.\n\r",
                "The sky begins to break through "
                "the clouds.\n\r",
                "The clouds are slowly "
                "evaporating.\n\r"
            };

            weath->echo = echo_strings[n];
            weath->echo_color = AT_WHITE;
        }
        break;

    case 1:
        if(precip - dP <= -2*weath_unit)
        {
            char *echo_strings[4] =
            {
                "The sky is getting cloudy.\n\r",
                "The sky is getting cloudy.\n\r",
                "Light clouds cast a haze over "
                "the sky.\n\r",
                "Billows of clouds spread through "
                "the sky.\n\r"
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_GREY;
        }
        break;

    case 2:
        if(precip - dP > 0)
        {
            if(tindex > 1)
            {
                char *echo_strings[4] =
                {
                    "The rain stops.\n\r",
                    "The rain stops.\n\r",
                    "The rainstorm tapers "
                    "off.\n\r",
                    "The rain's intensity "
                    "breaks.\n\r"
                };
                weath->echo = echo_strings[n];
                weath->echo_color = AT_CYAN;
            }
            else
            {
                char *echo_strings[4] =
                {
                    "The snow stops.\n\r",
                    "The snow stops.\n\r",
                    "The snow showers taper "
                    "off.\n\r",
                    "The snow flakes disappear "
                    "from the sky.\n\r"
                };
                weath->echo = echo_strings[n];
                weath->echo_color = AT_WHITE;
            }
        }
        break;

    case 3:
        if(precip - dP <= 0)
        {
            if(tindex > 1)
            {
                char *echo_strings[4] =
                {
                    "It starts to rain.\n\r",
                    "It starts to rain.\n\r",
                    "A droplet of rain falls "
                    "upon you.\n\r",
                    "The rain begins to "
                    "patter.\n\r"
                };
                weath->echo = echo_strings[n];
                weath->echo_color = AT_CYAN;
            }
            else
            {
                char *echo_strings[4] =
                {
                    "It starts to snow.\n\r",
                    "It starts to snow.\n\r",
                    "Crystal flakes begin to "
                    "fall from the "
                    "sky.\n\r",
                    "Snow flakes drift down "
                    "from the clouds.\n\r"
                };
                weath->echo = echo_strings[n];
                weath->echo_color = AT_WHITE;
            }
        }
        else if(tindex < 2 && temp - dT > -weath_unit)
        {
            char *echo_strings[4] =
            {
                "The temperature drops and the rain "
                    "becomes a light snow.\n\r",
                    "The temperature drops and the rain "
                    "becomes a light snow.\n\r",
                    "Flurries form as the rain freezes.\n\r",
                    "Large snow flakes begin to fall "
                    "with the rain.\n\r"
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_WHITE;
        }
        else if(tindex > 1 && temp - dT <= -weath_unit)
        {
            char *echo_strings[4] =
            {
                "The snow flurries are gradually "
                    "replaced by pockets of rain.\n\r",
                    "The snow flurries are gradually "
                    "replaced by pockets of rain.\n\r",
                    "The falling snow turns to a cold drizzle.\n\r",
                    "The snow turns to rain as the air warms.\n\r"
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_CYAN;
        }
        break;

    case 4:
        if(precip - dP > 2*weath_unit)
        {
            if(tindex > 1)
            {
                char *echo_strings[4] =
                {
                    "The lightning has stopped.\n\r",
                    "The lightning has stopped.\n\r",
                    "The sky settles, and the "
                    "thunder surrenders.\n\r",
                    "The lightning bursts fade as "
                    "the storm weakens.\n\r"
                };
                weath->echo = echo_strings[n];
                weath->echo_color = AT_GREY;
            }
        }
        else if(tindex < 2 && temp - dT > -weath_unit)
        {
            char *echo_strings[4] =
            {
                "The cold rain turns to snow.\n\r",
                "The cold rain turns to snow.\n\r",
                "Snow flakes begin to fall "
                "amidst the rain.\n\r",
                "The driving rain begins to freeze.\n\r"
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_WHITE;
        }
        else if(tindex > 1 && temp - dT <= -weath_unit)
        {
            char *echo_strings[4] =
            {
                "The snow becomes a freezing rain.\n\r",
                "The snow becomes a freezing rain.\n\r",
                "A cold rain beats down on you "
                "as the snow begins to melt.\n\r",
                "The snow is slowly replaced by a heavy "
                "rain.\n\r"
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_CYAN;
        }
        break;

    case 5:
        if(precip - dP <= 2*weath_unit)
        {
            if(tindex > 1)
            {
                char *echo_strings[4] =
                {
                    "Lightning flashes in the "
                        "sky.\n\r",
                        "Lightning flashes in the "
                        "sky.\n\r",
                        "A flash of lightning splits "
                        "the sky.\n\r",
                        "The sky flashes, and the "
                        "ground trembles with "
                        "thunder.\n\r"
                };
                weath->echo = echo_strings[n];
                weath->echo_color = AT_YELLOW;
            }
        }
        else if(tindex > 1 && temp - dT <= -weath_unit)
        {
            char *echo_strings[4] =
            {
                "The sky rumbles with thunder as "
                    "the snow changes to rain.\n\r",
                    "The sky rumbles with thunder as "
                    "the snow changes to rain.\n\r",
                    "The falling turns to freezing rain "
                    "amidst flashes of "
                    "lightning.\n\r",
                    "The falling snow begins to melt as "
                    "thunder crashes overhead.\n\r"
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_WHITE;
        }
        else if(tindex < 2 && temp - dT > -weath_unit)
        {
            char *echo_strings[4] =
            {
                "The lightning stops as the rainstorm "
                    "becomes a blinding "
                    "blizzard.\n\r",
                    "The lightning stops as the rainstorm "
                    "becomes a blinding "
                    "blizzard.\n\r",
                    "The thunder dies off as the "
                    "pounding rain turns to "
                    "heavy snow.\n\r",
                    "The cold rain turns to snow and "
                    "the lightning stops.\n\r"
            };
            weath->echo = echo_strings[n];
            weath->echo_color = AT_CYAN;
        }
        break;

    default:
        bug("echo_weather: invalid precip index");
        weath->precip = 0;
        break;
    }

    return;
}

/*
 * get echo messages according to time changes...
 * some echoes depend upon the weather so an echo must be
 * found for each area
 * Last Modified: August 10, 1997
 * Fireblade
 */
static void get_time_echo(WEATHER_DATA *weath)
{
    int n;
    int pindex;

    n = number_bits(2);
    pindex = (weath->precip + 3*weath_unit - 1)/weath_unit;
    weath->echo = NULL;
    weath->echo_color = AT_GREY;

    switch(time_info.hour)
    {
    case 5:
        {
            char *echo_strings[4] =
            {
                "The day has begun.\n\r",
                "The day has begun.\n\r",
                "The sky slowly begins to glow.\n\r",
                "The sun slowly embarks upon a new day.\n\r"
            };
            time_info.sunlight = SUN_RISE;
            weath->echo = echo_strings[n];
            weath->echo_color = AT_YELLOW;
            break;
        }
    case 6:
        {
            char *echo_strings[4] =
            {
                "The sun rises in the east.\n\r",
                "The sun rises in the east.\n\r",
                "The hazy sun rises over the horizon.\n\r",
                "Day breaks as the sun lifts into the sky.\n\r"
            };
            time_info.sunlight = SUN_LIGHT;
            weath->echo = echo_strings[n];
            weath->echo_color = AT_ORANGE;
            break;
        }
    case 12:
        {
            if(pindex > 0)
            {
                weath->echo = "It's noon.\n\r";
            }
            else
            {
                char *echo_strings[2] =
                {
                    "The intensity of the sun "
                        "heralds the noon hour.\n\r",
                        "The sun's bright rays beat down "
                        "upon your shoulders.\n\r"
                };
                weath->echo = echo_strings[n%2];
            }
            time_info.sunlight = SUN_LIGHT;
            weath->echo_color = AT_WHITE;
            break;
        }
    case 19:
        {
            char *echo_strings[4] =
            {
                "The sun slowly disappears in the west.\n\r",
                "The reddish sun sets past the horizon.\n\r",
                "The sky turns a reddish orange as the sun "
                "ends its journey.\n\r",
                "The sun's radiance dims as it sinks in the "
                "sky.\n\r"
            };
            time_info.sunlight = SUN_SET;
            weath->echo = echo_strings[n];
            weath->echo_color = AT_RED;
            break;
        }
    case 20:
        {
            if(pindex > 0)
            {
                char *echo_strings[2] =
                {
                    "The night begins.\n\r",
                    "Twilight descends around you.\n\r"
                };
                weath->echo = echo_strings[n%2];
            }
            else
            {
                char *echo_strings[2] =
                {
                    "The moon's gentle glow diffuses "
                        "through the night sky.\n\r",
                        "The night sky gleams with "
                        "glittering starlight.\n\r"
                };
                weath->echo = echo_strings[n%2];
            }
            time_info.sunlight = SUN_DARK;
            weath->echo_color = AT_DBLUE;
            break;
        }
    }

    return;
}

/*
 * update the time
 */
static void time_update()
{
    AREA_DATA *pArea;
    DESCRIPTOR_DATA *d;
    WEATHER_DATA *weath;

    switch(++time_info.hour)
    {
    case 5:
    case 6:
    case 12:
    case 19:
    case 20:
        for(pArea = first_area; pArea;
            pArea = (pArea == last_area) ? first_build : pArea->next)
        {
            get_time_echo(pArea->weather);
        }

        for(d = first_descriptor; d; d = d->next)
        {
            if (d->connected == CON_PLAYING &&
                IS_OUTSIDE(d->character) &&
                !NO_WEATHER_SECT(d->character->in_room->sector_type) &&
                IS_AWAKE(d->character))
            {
                weath = d->character->in_room->area->weather;
                if(!weath->echo)
                    continue;
                set_char_color(weath->echo_color,
                               d->character);
                ch_printf(d->character, "%s", weath->echo);
            }
        }
        break;
    case 24:
        time_info.hour = 0;
        time_info.day++;
        break;
    }

    if(time_info.day >= 30)
    {
        time_info.day = 0;
        time_info.month++;
    }

    if(time_info.month >= 17)
    {
        time_info.month = 0;
        time_info.year++;
    }

    return;
}

void do_ticks(CHAR_DATA *ch, char *argument)
{
    ch_printf(ch, "Area:     %d/%d\n\r"
              "Mobile:   %d/%d\n\r"
              "Violence: %d/%d\n\r"
              "Spell:    %d/%d\n\r"
              "Point:    %d/%d(%0.0f-%0.0f)\n\r"
              "Second:   %d/%d\n\r",
              pulse_area, PULSE_AREA,
              pulse_mobile, PULSE_MOBILE,
              pulse_violence, PULSE_VIOLENCE,
              pulse_spell, PULSE_SPELL,
              pulse_point, PULSE_TICK, (PULSE_TICK*0.75), (PULSE_TICK*1.25),
              pulse_second, PULSE_PER_SECOND);
}
