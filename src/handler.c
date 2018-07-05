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
 *		        Main structure manipulation module		    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: handler.c,v 1.79 2004/04/06 22:00:09 dotd Exp $";*/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "mud.h"
#include "gsn.h"
#include "recylist.h"
#include "christen.h"
#include "currency.h"
#include "quest.h"
#include "irc.h"

DECLARE_DO_FUN(do_revert);
DECLARE_DO_FUN(do_return);

extern int              char_affects;
extern int		top_exit;
extern int		top_ed;
extern int		top_affect;
extern int		cur_qobjs;
extern int		cur_qchars;
extern CHAR_DATA *	gch_prev;
extern OBJ_DATA  *	gobj_prev;

CHAR_DATA	*cur_char;
ROOM_INDEX_DATA	*cur_room;
bool		 cur_char_died;
ch_ret		 global_retcode;

OBJ_DATA *group_object( OBJ_DATA *obj1, OBJ_DATA *obj2 );
bool	flush_buffer		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );

/*
 * Return how much exp a char has
 */
int get_exp( CHAR_DATA *ch )
{
    return ch->exp;
}

/*
 * Calculate roughly how much experience a character is worth
 */
int get_exp_worth( CHAR_DATA *ch )
{
    int expw;

    expw = GetAveLevel(ch) * GetAveLevel(ch) * GetAveLevel(ch) * 5;
    expw += ch->max_hit;
    expw -= (ch->armor-50) * 2;
    expw += ( ch->barenumdie * ch->baresizedie + GET_DAMROLL(ch) ) * 50;
    expw += GET_HITROLL(ch) * GetAveLevel(ch) * 10;
    if ( IS_AFFECTED(ch, AFF_SANCTUARY) )
        expw += (int)((float)expw * 1.5);
    if ( IS_AFFECTED(ch, AFF_FIRESHIELD) )
        expw += (int)((float)expw * 1.2);
    if ( IS_AFFECTED(ch, AFF_SHOCKSHIELD) )
        expw += (int)((float)expw * 1.2);
    expw = URANGE( MIN_EXP_WORTH, expw, MAX_EXP_WORTH );

    return expw;
}

double get_exp_base( CHAR_DATA *ch, class_index cl )
{
    if ( ch && IS_NPC(ch) )
        return (267.79888);

    return ((double)class_table[cl]->exp_base / 100000);
}

double get_exp_power( CHAR_DATA *ch, class_index cl )
{
    if ( ch && IS_NPC(ch) )
        return (3.2227);

    return ((double)class_table[cl]->exp_power / 100000);
}

/*								-Thoric
 * Return how much experience is required for ch to get to a certain level
 */
long exp_level( CHAR_DATA *ch, sh_int level, sh_int cl )
{
    int lvl;
    long exp, x;

    lvl = UMAX(0, level - 1);

    if ( lvl == 1 )
        return(1);

    if ( lvl > LEVEL_HERO )
        return(2110000000);

    if ( lvl > LEVEL_HERO-10 )
        exp = (long)((1.01119501933 * pow(10, -17)) * pow(lvl, 14.8001095083));
    else
        exp = (long)(get_exp_base(ch, cl) * pow(lvl, get_exp_power(ch, cl)));

    for (x=1000000000;x>1000;x/=10)
	if (exp > x)
	    break;
    x/=100;

    return(((exp+x)/x)*x);
}

/*
 * Get what level ch is based on exp
 */
/*
 sh_int level_exp( CHAR_DATA *ch, int exp, sh_int ch_class )
 {
 int x, lastx, y, tmp;

 x = LEVEL_SUPREME;
 lastx = x;
 y = 0;
 while ( !y )
 {
 tmp = exp_level(ch, x, ch_class);
 lastx = x;
 if ( tmp > exp )
 x /= 2;
 else
 if (lastx != x )
 x += ( x / 2 );
 else
 y = x;
 }
 if ( y < 1 )
 y = 1;
 if ( y > LEVEL_SUPREME )
 y = LEVEL_SUPREME;
 return y;
 }
 */
/*
 * Retrieve a character's trusted level for permission checking.
 */
sh_int get_trust( CHAR_DATA *ch )
{
    if ( ch->desc && ch->desc->original )
        ch = ch->desc->original;

    if ( ch->trust != 0 )
        return ch->trust;

    if ( IS_NPC(ch) && GetMaxLevel(ch) >= LEVEL_AVATAR )
        return LEVEL_AVATAR;

    if ( GetMaxLevel(ch) >= LEVEL_NEOPHYTE && IS_RETIRED( ch ) )
        return LEVEL_NEOPHYTE;

    return GetMaxLevel(ch);
}


/*
 * Retrieve a character's age.
 */
sh_int get_age( CHAR_DATA *ch )
{
    return ch->age_bonus + 17 + ( ch->played + (current_time - ch->logon) ) / SECS_PER_MUD_YEAR;
}

sh_int get_age_month(CHAR_DATA *ch)
{
    return ((ch->played + (current_time-ch->logon)) % SECS_PER_MUD_YEAR) / SECS_PER_MUD_MONTH;
}

sh_int get_age_day(CHAR_DATA *ch)
{
    return (((ch->played + (current_time-ch->logon)) % SECS_PER_MUD_YEAR) % SECS_PER_MUD_MONTH) / SECS_PER_MUD_DAY;
}

sh_int get_age_hour(CHAR_DATA *ch)
{
    return ((((ch->played + (current_time-ch->logon)) % SECS_PER_MUD_YEAR) % SECS_PER_MUD_MONTH) % SECS_PER_MUD_DAY) / SECS_PER_MUD_HOUR;
}


sh_int
MaxDexForRace(CHAR_DATA *ch)
{
    if (GetMaxLevel(ch) >= LEVEL_HIGOD || IS_NPC(ch))
        return (25);
    if (IsDragon(ch))
        return (20);

    switch (GET_RACE(ch)) {
    case RACE_VAGABOND:
        return (25);
        break;
    case RACE_ELVEN:
    case RACE_WILD_ELF:
    case RACE_GOLD_ELF:
    case RACE_HALFLING:
        return (19);
        break;
    case RACE_SKEXIE:
    case RACE_DROW:
        return (20);
        break;
    case RACE_DWARF:
    case RACE_HALF_OGRE:
        return (17);
        break;
    case RACE_MFLAYER:
    case RACE_HALF_GIANT:
        return (16);
        break;
    default:
        return (18);
        break;
    }
}

sh_int
MaxIntForRace(CHAR_DATA *ch)
{
    if (GetMaxLevel(ch) >= LEVEL_HIGOD || IS_NPC(ch))
        return (25);
    if (IsDragon(ch))
        return (20);

    switch (GET_RACE(ch)) {
    case RACE_VAGABOND:
        return (25);
        break;
    case RACE_MFLAYER:
        return (20);
        break;
    case RACE_GOLD_ELF:
    case RACE_GNOME:
        return (19);
        break;
    case RACE_HALF_GIANT:
    case RACE_HALF_OGRE:
        return (17);
        break;

    default:
        return (18);
        break;
    }
}

sh_int
MaxWisForRace(CHAR_DATA *ch)
{
    if (GetMaxLevel(ch) >= LEVEL_HIGOD || IS_NPC(ch))
        return (25);
    if (IsDragon(ch))
        return (20);

    switch (GET_RACE(ch)) {
    case RACE_VAGABOND:
        return (25);
        break;
    case RACE_MFLAYER:
        return (19);
        break;
    case RACE_GOLD_ELF:
    case RACE_WILD_ELF:
    case RACE_GNOME:
    case RACE_HALF_GIANT:
        return (17);
        break;
    default:
        return (18);
        break;
    }
}

sh_int
MaxConForRace(CHAR_DATA *ch)
{
    if (GetMaxLevel(ch) >= LEVEL_HIGOD || IS_NPC(ch))
        return (25);
    if (IsDragon(ch))
        return (20);

    switch (GET_RACE(ch)) {
    case RACE_HALF_ORC:
    case RACE_DWARF:
    case RACE_HALF_OGRE:
        return (19);
        break;
    case RACE_ELVEN:
    case RACE_GOLD_ELF:
    case RACE_SEA_ELF:
    case RACE_DROW:
        return (17);
        break;
    case RACE_MFLAYER:
        return (16);
        break;
    default:
        return (18);
        break;
    }
}

sh_int
MaxChaForRace(CHAR_DATA *ch)
{
    if (GetMaxLevel(ch) >= LEVEL_HIGOD || IS_NPC(ch))
        return (25);
    if (IsDragon(ch))
        return (20);

    switch (GET_RACE(ch)) {
    case RACE_VAGABOND:
        return (20);
        break;
    case RACE_HALF_ORC:
    case RACE_ORC:
    case RACE_DROW:
    case RACE_DWARF:
    case RACE_DARK_DWARF:
        return (17);
        break;
    default:
        return (18);
        break;
    }
}

sh_int MaxStrForRace(CHAR_DATA *ch)
{
    if (IS_IMMORTAL(ch) || IS_NPC(ch))
        return (25);

    if (IsDragon(ch))
        return (21);

    switch (GET_RACE(ch))
    {
    case RACE_GIANT:
    case RACE_GIANT_HILL:
    case RACE_GIANT_FROST:
    case RACE_GIANT_FIRE:
    case RACE_GIANT_CLOUD:
    case RACE_GIANT_STORM:
    case RACE_GIANT_STONE:
    case RACE_HALF_GIANT:
        return (22);
        break;

    case RACE_TROLL:
    case RACE_VAMPIRE:
    case RACE_HALF_OGRE:
    case RACE_HALF_ORC:
    case RACE_ORC:
        return (21);
        break;

    case RACE_HALFLING:
    case RACE_GOBLIN:
    case RACE_GNOME:
    case RACE_DEEP_GNOME:
        return (17);
        break;

    case RACE_VAGABOND:
    case RACE_MFLAYER:
        return (16);
        break;
    default:
        return (19);
        break;
    }
}

/*
 * Retrieve character's current strength.
 */
sh_int get_curr_str( CHAR_DATA *ch )
{
    sh_int max;

    if (IS_IMMORTAL(ch)) return(25);

    if ( IS_NPC(ch) )
        max = 25;
    else
        max = MaxStrForRace(ch);

    return URANGE( 3, ch->perm_str + ch->mod_str, max );
}



/*
 * Retrieve character's current intelligence.
 */
sh_int get_curr_int( CHAR_DATA *ch )
{
    sh_int max;

    if (IS_IMMORTAL(ch)) return(25);

    if ( IS_NPC(ch) )
        max = 25;
    else
        max = MaxIntForRace(ch);

    return URANGE( 3, ch->perm_int + ch->mod_int, max );
}



/*
 * Retrieve character's current wisdom.
 */
sh_int get_curr_wis( CHAR_DATA *ch )
{
    sh_int max;

    if (IS_IMMORTAL(ch)) return(25);

    if ( IS_NPC(ch) )
        max = 25;
    else
        max = MaxWisForRace(ch);

    return URANGE( 3, ch->perm_wis + ch->mod_wis, max );
}



/*
 * Retrieve character's current dexterity.
 */
sh_int get_curr_dex( CHAR_DATA *ch )
{
    sh_int max;

    if (IS_IMMORTAL(ch)) return(25);

    if ( IS_NPC(ch) )
        max = 25;
    else
        max = MaxDexForRace(ch);

    return URANGE( 3, ch->perm_dex + ch->mod_dex, max );
}



/*
 * Retrieve character's current constitution.
 */
sh_int get_curr_con( CHAR_DATA *ch )
{
    sh_int max;

    if (IS_IMMORTAL(ch)) return(25);

    if ( IS_NPC(ch) )
        max = 25;
    else
        max = MaxConForRace(ch);

    return URANGE( 3, ch->perm_con + ch->mod_con, max );
}

/*
 * Retrieve character's current charisma.
 */
sh_int get_curr_cha( CHAR_DATA *ch )
{
    sh_int max;

    if (IS_IMMORTAL(ch)) return(25);

    if ( IS_NPC(ch) )
        max = 25;
    else
        max = MaxChaForRace(ch);

    return URANGE( 3, ch->perm_cha + ch->mod_cha, max );
}

/*
 * Retrieve character's current luck.
 */
sh_int get_curr_lck( CHAR_DATA *ch )
{
    sh_int max;
    if (IS_IMMORTAL(ch)) return(25);

    if ( IS_NPC(ch) )
        max = 25;
    else
        max = 20;

    return URANGE( 3, ch->perm_lck + ch->mod_lck, max );
}


/*
 * Retrieve a character's carry capacity.
 * Vastly reduced (finally) due to containers		-Thoric
 */
int can_carry_n( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && IS_IMMORTAL(ch) )
        return 1000;

    return(5+get_curr_dex(ch)/2+GetMaxLevel(ch)/2);
}

int carry_n( CHAR_DATA *ch)
{
    return ch->carry_number;
}


/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && IS_IMMORTAL(ch) )
        return 100000000;

    return str_app[get_curr_str(ch)].carry;
}


int carry_w( CHAR_DATA *ch)
{
    int weight = ch->carry_weight;
    int i;

    for (i = 0; i < MAX_CURR_TYPE; i++)
        weight += money_weight(GET_MONEY(ch, i), i);

    return weight;
}


/*
 * See if a player/mob can take a piece of prototype eq		-Thoric
 */
bool can_take_proto( CHAR_DATA *ch )
{
    if ( IS_IMMORTAL(ch) )
        return TRUE;
    else
        if ( IS_NPC(ch) && IS_SET(ch->act, ACT_PROTOTYPE) )
            return TRUE;
        else
            return FALSE;
}


/*
 * See if a string is one of the names of an object.
 */
bool is_name( const char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    for ( ; ; )
    {
        namelist = one_argument( namelist, name );
        if ( name[0] == '\0' )
            return FALSE;
        if ( !str_cmp( str, name ) )
            return TRUE;
    }
}

bool is_name_prefix( const char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    for ( ; ; )
    {
        namelist = one_argument( namelist, name );
        if ( name[0] == '\0' )
            return FALSE;
        if ( !str_prefix( str, name ) )
            return TRUE;
    }
}

/*
 * See if a string is one of the names of an object.		-Thoric
 * Treats a dash as a word delimiter as well as a space
 */
bool is_name2( const char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    for ( ; ; )
    {
        namelist = one_argument2( namelist, name );
        if ( name[0] == '\0' )
            return FALSE;
        if ( !str_cmp( str, name ) )
            return TRUE;
    }
}

bool is_name2_prefix( const char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    for ( ; ; )
    {
        namelist = one_argument2( namelist, name );
        if ( name[0] == '\0' )
            return FALSE;
        if ( !str_prefix( str, name ) )
            return TRUE;
    }
}

/*								-Thoric
 * Checks if str is a name in namelist supporting multiple keywords
 */
bool nifty_is_name( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    if ( !str || str[0] == '\0' || (str[0] == '-' && str[1] == '\0') ||
         !namelist || namelist[0] == '\0' )
        return FALSE;

    for ( ; ; )
    {
        str = one_argument2( str, name );
        if ( name[0] == '\0' )
            return TRUE;
        if ( !is_name2( name, namelist ) )
            return FALSE;
    }
}

bool nifty_is_name_prefix( char *str, char *namelist )
{
    char name[MAX_INPUT_LENGTH];

    if ( !str || str[0] == '\0' || (str[0] == '-' && str[1] == '\0') )
        return FALSE;

    for ( ; ; )
    {
        str = one_argument2( str, name );
        if ( name[0] == '\0' )
            return TRUE;
        if ( !is_name2_prefix( name, namelist ) )
            return FALSE;
    }
}


void room_affect( ROOM_INDEX_DATA *pRoomIndex, AFFECT_DATA *paf, bool fAdd )
{
    if ( fAdd )
    {
        switch(paf->location)
        {
        case APPLY_ROOMFLAG:
        case APPLY_SECTORTYPE:
            break;
        case APPLY_ROOMLIGHT:
            pRoomIndex->light += paf->modifier;
            break;
        case APPLY_TELEVNUM:
        case APPLY_TELEDELAY:
            break;
        }
    }
    else
    {
        switch(paf->location)
        {
        case APPLY_ROOMFLAG:
        case APPLY_SECTORTYPE:
            break;
        case APPLY_ROOMLIGHT:
            pRoomIndex->light -= paf->modifier;
            break;
        case APPLY_TELEVNUM:
        case APPLY_TELEDELAY:
            break;
        }
    }
}

/*
 * Modify a skill (hopefully) properly			-Thoric
 *
 * On "adding" a skill modifying affect, the value set is unimportant
 * upon removing the affect, the skill it enforced to a proper range.
 */
void modify_skill( CHAR_DATA *ch, int sn, int mod, bool fAdd )
{
    if ( !IS_NPC(ch) )
    {
        if ( fAdd )
            ch->pcdata->learned[sn] += mod;
        else
            ch->pcdata->learned[sn] = URANGE(0, LEARNED(ch, sn) + mod,
                                             GET_ADEPT(ch, sn));
    }
}


/*
 * Apply or remove an affect to a character.
 */
void affect_modify( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd )
{
    OBJ_DATA *wield = NULL;
    int mod;
    struct skill_type *skill;
    ch_ret retcode;

    mod = paf->modifier;

    if ( fAdd )
    {
        SET_BIT( ch->affected_by, paf->bitvector );
    }
    else
    {
        REMOVE_BIT( ch->affected_by, paf->bitvector );
        /*
         * might be an idea to have a duration removespell which returns
         * the spell after the duration... but would have to store
         * the removed spell's information somewhere...		-Thoric
         */
        if ( (paf->location % REVERSE_APPLY) == APPLY_REMOVESPELL )
            return;
        switch( paf->location % REVERSE_APPLY )
        {
        case APPLY_AFFECT:        REMOVE_BIT( ch->affected_by, mod );	 return;
        case APPLY_AFF2:          REMOVE_BIT( ch->affected_by2, mod ); return;
        case APPLY_RESISTANT:     REMOVE_BIT( ch->resistant, mod );	 return;
        case APPLY_IMMUNE:        REMOVE_BIT( ch->immune, mod );	 return;
        case APPLY_SUSCEPTIBLE:   REMOVE_BIT( ch->susceptible, mod );	 return;
        case APPLY_ABSORB:        REMOVE_BIT( ch->absorb, mod );	 return;
        case APPLY_WEARSPELL:	    /* affect only on wear */		 return;
        case APPLY_REMOVE:	    SET_BIT( ch->affected_by, mod );	 return;
        }
        mod = 0 - mod;
    }

    switch ( paf->location % REVERSE_APPLY )
    {
    default:
        bug( "Affect_modify: unknown location %d.", paf->location );
        return;

    case APPLY_NONE:						break;
    case APPLY_STR:           ch->mod_str		+= mod;	break;
    case APPLY_DEX:           ch->mod_dex		+= mod;	break;
    case APPLY_INT:           ch->mod_int		+= mod;	break;
    case APPLY_WIS:           ch->mod_wis		+= mod;	break;
    case APPLY_CON:	      ch->mod_con		+= mod;	break;
    case APPLY_CHA:	      ch->mod_cha		+= mod; break;
    case APPLY_LCK:	      ch->mod_lck		+= mod; break;
    case APPLY_SEX:
        ch->sex = (ch->sex+mod) % 3;
        if ( ch->sex < 0 )
            ch->sex += 2;
        ch->sex = URANGE( 0, ch->sex, 2 );
        break;

    case APPLY_CLASS:						break;
    case APPLY_LEVEL:						break;
    case APPLY_GOLD:						break;
    case APPLY_EXP:						break;

    case APPLY_SF:            ch->spellfail		+= mod; break;
    case APPLY_RACE:          ch->race                  += mod;
                              ch->xflags  = race_bodyparts(ch); break;
    case APPLY_NUMATTACKS:    ch->numattacks            += mod; break;
    case APPLY_BARENUMDIE:    ch->barenumdie            += mod; break;
    case APPLY_BARESIZDIE:    ch->baresizedie           += mod; break;
    case APPLY_AGE:	      ch->age_bonus		+= mod;	break;
    case APPLY_HEIGHT:	      ch->height		+= mod;	break;
    case APPLY_WEIGHT:	      ch->weight		+= mod;	break;
    case APPLY_MANA:          ch->max_mana		+= mod;	break;
    case APPLY_HIT:           ch->max_hit		+= mod;	break;
    case APPLY_MOVE:          ch->max_move		+= mod;	break;
    case APPLY_MANA_REGEN:    ch->mana_regen		+= mod; break;
    case APPLY_HIT_REGEN:     ch->hit_regen		+= mod;	break;
    case APPLY_MOVE_REGEN:    ch->move_regen		+= mod;	break;
    case APPLY_ANTIMAGIC:     ch->antimagicp            += mod; break;
    case APPLY_AC:            ch->armor			+= mod;	break;
    case APPLY_HITROLL:       ch->hitroll		+= mod;	break;
    case APPLY_DAMROLL:       ch->damroll		+= mod;	break;
    case APPLY_HITNDAM:       ch->hitroll               += mod;
    ch->damroll               += mod; break;
    case APPLY_SAVING_POISON: ch->saving_poison_death	+= mod;	break;
    case APPLY_SAVING_ROD:    ch->saving_wand		+= mod;	break;
    case APPLY_SAVING_PARA:   ch->saving_para_petri	+= mod;	break;
    case APPLY_SAVING_BREATH: ch->saving_breath		+= mod;	break;
    case APPLY_SAVING_SPELL:  ch->saving_spell_staff	+= mod;	break;
    case APPLY_SAVING_ALL:    ch->saving_poison_death   += mod;
    ch->saving_para_petri     += mod;
    ch->saving_breath         += mod;
    ch->saving_spell_staff    += mod;
    ch->saving_wand           += mod; break;
    case APPLY_AFFECT:        SET_BIT( ch->affected_by, mod );	break;
    case APPLY_AFF2:          SET_BIT( ch->affected_by2, mod );	break;
    case APPLY_RESISTANT:     SET_BIT( ch->resistant, mod );	break;
    case APPLY_IMMUNE:        SET_BIT( ch->immune, mod );	break;
    case APPLY_SUSCEPTIBLE:   SET_BIT( ch->susceptible, mod );	break;
    case APPLY_ABSORB:        SET_BIT( ch->absorb, mod );	break;
    case APPLY_WEAPONSPELL:	/* see fight.c */		break;
    case APPLY_REMOVE:	      REMOVE_BIT(ch->affected_by, mod);	break;

    case APPLY_FULL:
        if ( !IS_NPC(ch) )
            ch->pcdata->condition[COND_FULL] =
                URANGE( 0, GET_COND(ch, COND_FULL) + mod, MAX_COND_VAL );
        break;

    case APPLY_THIRST:
        if ( !IS_NPC(ch) )
            ch->pcdata->condition[COND_THIRST] =
                URANGE( 0, GET_COND(ch, COND_THIRST) + mod, MAX_COND_VAL );
        break;

    case APPLY_DRUNK:
        if ( !IS_NPC(ch) )
            ch->pcdata->condition[COND_DRUNK] =
                URANGE( 0, GET_COND(ch, COND_DRUNK) + mod, MAX_COND_VAL );
        break;

    case APPLY_BLOOD:
        if ( !IS_NPC(ch) )
            ch->pcdata->condition[COND_BLOODTHIRST] =
                URANGE( 0, GET_BLOOD(ch) + mod, GET_MAX_BLOOD(ch) );
        break;

    case APPLY_MENTALSTATE:
        ch->mental_state	= URANGE(-100, ch->mental_state + mod, 100);
        break;
    case APPLY_EMOTION:
        ch->emotional_state	= URANGE(-100, ch->emotional_state + mod, 100);
        break;

    case APPLY_STRIPSN:
        if ( IS_VALID_SN(mod) )
            affect_strip( ch, mod );
        else
            bug( "affect_modify: APPLY_STRIPSN invalid sn %d", mod );
        break;

        /* spell cast upon wear/removal of an object	-Thoric */
    case APPLY_WEARSPELL:
    case APPLY_REMOVESPELL:
        if ( IS_SET(ch->in_room->room_flags, ROOM_NO_MAGIC)
             ||   IS_SET(ch->immune, RIS_MAGIC)
             ||   saving_char == ch		/* so save/quit doesn't trigger */
             ||   loading_char == ch )	/* so loading doesn't trigger */
            return;

        mod = abs(mod);
        if ( IS_VALID_SN(mod)
             &&  (skill=skill_table[mod]) != NULL
             &&   skill->type == SKILL_SPELL
             &&   skill->spell_fun )
            if ( (retcode=(*skill->spell_fun) ( mod, GetMaxLevel(ch), ch,
                                                ch )) == rCHAR_DIED
                 ||   char_died(ch) )
                return;
        break;


        /* skill apply types	-Thoric */

    case APPLY_PALM:	/* not implemented yet */		break;
    case APPLY_TRACK:	modify_skill(ch, gsn_hunt,	mod, fAdd);	break;
    case APPLY_HIDE:	modify_skill(ch, gsn_hide, 	mod, fAdd);	break;
    case APPLY_STEAL:	modify_skill(ch, gsn_steal, 	mod, fAdd);	break;
    case APPLY_SNEAK:	modify_skill(ch, gsn_sneak, 	mod, fAdd);	break;
    case APPLY_PICK:	modify_skill(ch, gsn_pick_lock, mod, fAdd);	break;
    case APPLY_BACKSTAB:modify_skill(ch, gsn_backstab, 	mod, fAdd);	break;
    case APPLY_DETRAP:	modify_skill(ch, gsn_detrap,    mod, fAdd);	break;
    case APPLY_FINDTRAP:modify_skill(ch, gsn_find_traps,mod, fAdd);	break;
    case APPLY_DODGE:	modify_skill(ch, gsn_dodge, 	mod, fAdd);	break;
    case APPLY_SCAN:	modify_skill(ch, gsn_spot, 	mod, fAdd);	break;
    case APPLY_GOUGE:	modify_skill(ch, gsn_gouge, 	mod, fAdd);	break;
    case APPLY_SEARCH:	modify_skill(ch, gsn_search, 	mod, fAdd);	break;
    case APPLY_DIG:	modify_skill(ch, gsn_dig, 	mod, fAdd);	break;
    case APPLY_MOUNT:	modify_skill(ch, gsn_mount, 	mod, fAdd);	break;
    case APPLY_DISARM:	modify_skill(ch, gsn_disarm, 	mod, fAdd);	break;
    case APPLY_KICK:	modify_skill(ch, gsn_kick, 	mod, fAdd);	break;
    case APPLY_PARRY:	modify_skill(ch, gsn_parry, 	mod, fAdd);	break;
    case APPLY_BASH:	modify_skill(ch, gsn_bash, 	mod, fAdd);	break;
    case APPLY_STUN:	modify_skill(ch, gsn_stun, 	mod, fAdd);	break;
    case APPLY_PUNCH:	modify_skill(ch, gsn_punch, 	mod, fAdd);	break;
    case APPLY_CLIMB:	modify_skill(ch, gsn_climb, 	mod, fAdd);	break;
    case APPLY_GRIP:	modify_skill(ch, gsn_grip, 	mod, fAdd);	break;
    case APPLY_SCRIBE:	modify_skill(ch, gsn_scribe, 	mod, fAdd);	break;
    case APPLY_BREW:	modify_skill(ch, gsn_brew, 	mod, fAdd);	break;
    case APPLY_COOK:	modify_skill(ch, gsn_cook,	mod, fAdd);	break;

    case APPLY_NUMMEM:
        if (!IS_NPC(ch) && !fAdd)
        {
            if (paf->duration ||
                total_memorized(ch) >= max_can_memorize(ch) ||
                MEMORIZED(ch, paf->modifier) >= max_can_memorize_spell(ch, paf->modifier))
                break;
            if (!paf->next || paf->next->type != paf->type)
                send_to_char("You close your spell book, and commit another spell to memory.\n\r", ch);
            if (number_percent()<(get_curr_int(ch)*3))
                ch->pcdata->learned[paf->modifier] =
                    UMIN( GET_ADEPT(ch, paf->modifier), LEARNED(ch, paf->modifier) + 1);
            ch->pcdata->memorized[paf->modifier]++;
        }
        break;
    /*
     *  Applys that dont generate effects on pcs
     */

    case APPLY_EAT_SPELL:
    case APPLY_IMMUNESPELL:
    case APPLY_ALIGN_SLAYER:
    case APPLY_RACE_SLAYER:
        break;

        /*
         * Room apply types
         */
    case APPLY_ROOMFLAG:
    case APPLY_SECTORTYPE:
    case APPLY_ROOMLIGHT:
    case APPLY_TELEVNUM:
        break;

    }



    /*
     * Check for weapon wielding.
     * Guard against recursion (for weapons with affects).
     */
    if ( !IS_NPC( ch )
         &&   saving_char != ch
         && ( wield = get_eq_char( ch, WEAR_WIELD ) ) != NULL
         &&   get_obj_weight(wield) > str_app[get_curr_str(ch)].wield )
    {
        static int depth;

        if ( depth == 0 )
        {
            depth++;
            act( AT_ACTION, "You are too weak to wield $p any longer.",
                 ch, wield, NULL, TO_CHAR );
            act( AT_ACTION, "$n stops wielding $p.", ch, wield, NULL, TO_ROOM );
            unequip_char( ch, wield );
            depth--;
        }
    }

    return;
}



/*
 * Give an affect to a char.
 */
void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_new;

    if ( !ch )
    {
        bug( "Affect_to_char: NULL ch!" );
        return;
    }

    if ( !paf )
    {
        bug( "Affect_to_char: NULL paf!" );
        return;
    }

    if ( paf->duration == 0 )
    {
        bug( "Affect_to_char: %s duration: %d, sn: %d",
             GET_NAME(ch), paf->duration, paf->type );
        return;
    }

    CREATE( paf_new, AFFECT_DATA, 1 );
    LINK( paf_new, ch->first_affect, ch->last_affect, next, prev );
    paf_new->type	= paf->type;
    paf_new->duration	= paf->duration;
    paf_new->location	= paf->location;
    paf_new->modifier	= paf->modifier;
    paf_new->bitvector	= paf->bitvector;
    char_affects++;

    affect_modify( ch, paf_new, TRUE );
    return;
}


/*
 * Remove an affect from a char.
 */
void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    if ( !ch->first_affect )
    {
        bug( "Affect_remove: no affect." );
        return;
    }

    affect_modify( ch, paf, FALSE );

    UNLINK( paf, ch->first_affect, ch->last_affect, next, prev );
    char_affects--;
    DISPOSE( paf );
    return;
}

void free_aliases( CHAR_DATA *ch )
{
    ALIAS_DATA *pal;

    if (!ch || !ch->pcdata)
        return;

    while ((pal=ch->pcdata->first_alias))
    {
	UNLINK(pal, ch->pcdata->first_alias, ch->pcdata->last_alias, next, prev);
        if (pal->name)
            DISPOSE(pal->name);
        if (pal->cmd)
            DISPOSE(pal->cmd);
        DISPOSE( pal );
    }
}

/*
 * Strip all affects of a given sn.
 */
void affect_strip( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;

    for ( paf = ch->first_affect; paf; paf = paf_next )
    {
        paf_next = paf->next;
        if ( paf->type == sn )
            affect_remove( ch, paf );
    }

    return;
}



/*
 * Return true if a char is affected by a spell.
 */
AFFECT_DATA *is_affected( CHAR_DATA *ch, int sn )
{
    AFFECT_DATA *paf;

    for ( paf = ch->first_affect; paf; paf = paf->next )
        if ( paf->type == sn )
            return paf;

    return NULL;
}


/*
 * Add or enhance an affect.
 * Limitations put in place by Thoric, they may be high... but at least
 * they're there :)
 */
void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    AFFECT_DATA *paf_old;

    for ( paf_old = ch->first_affect; paf_old; paf_old = paf_old->next )
        if ( paf_old->type == paf->type )
        {
            paf->duration = UMIN( 1000000, paf->duration + paf_old->duration );
            if ( paf->modifier )
                paf->modifier = UMIN( 5000, paf->modifier + paf_old->modifier );
            else
                paf->modifier = paf_old->modifier;
            affect_remove( ch, paf_old );
            break;
        }

    affect_to_char( ch, paf );
    return;
}


/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    AFFECT_DATA *paf;

    if ( !ch->in_room )
    {
        bug( "Char_from_room: NULL." );
        return;
    }

    if ( !IS_NPC(ch) )
        --ch->in_room->area->nplayer;

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
         &&   obj->item_type == ITEM_LIGHT
         &&   obj->value[2] != 0
         &&   ch->in_room->light > 0 )
        --ch->in_room->light;

    /*
     * Character's affect on the room
     */
    for ( paf = ch->first_affect; paf; paf = paf->next )
        room_affect(ch->in_room, paf, FALSE);

    /*
     * Room's affect on the character
     */
    if ( !char_died(ch) )
    {
        for ( paf = ch->in_room->first_affect; paf; paf = paf->next )
            affect_modify(ch, paf, FALSE);

        if ( char_died(ch) )	/* could die from removespell, etc */
            return;
    }

    UNLINK( ch, ch->in_room->first_person, ch->in_room->last_person,
            next_in_room, prev_in_room );
    ch->was_in_room  = ch->in_room;
    ch->in_room      = NULL;
    ch->next_in_room = NULL;
    ch->prev_in_room = NULL;

    if ( !IS_NPC(ch)
         &&   get_timer( ch, TIMER_SHOVEDRAG ) > 0 )
        remove_timer( ch, TIMER_SHOVEDRAG );

    return;
}


/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
    OBJ_DATA *obj;
    AFFECT_DATA *paf;

    if ( !ch )
    {
        bug( "Char_to_room: NULL ch!" );
        return;
    }
    if ( !pRoomIndex )
    {
        char buf[MAX_STRING_LENGTH];

        sprintf( buf, "Char_to_room: %s -> NULL room!  Putting char in limbo (%d)",
                 ch->name, ROOM_VNUM_LIMBO );
        bug( buf, 0 );
        /* This used to just return, but there was a problem with crashing
         and I saw no reason not to just put the char in limbo. -Narn */
        pRoomIndex = get_room_index( ROOM_VNUM_LIMBO );
    }

    ch->in_room		= pRoomIndex;
    LINK( ch, pRoomIndex->first_person, pRoomIndex->last_person,
          next_in_room, prev_in_room );

    if ( !IS_NPC(ch) )
    {
        if ( ++ch->in_room->area->nplayer > ch->in_room->area->max_players )
            ch->in_room->area->max_players = ch->in_room->area->nplayer;
        if (!IS_SET(ch->in_room->area->flags, AFLAG_INITIALIZED) &&
            !IS_IMMORTAL(ch))
        {
            SET_BIT(ch->in_room->area->flags, AFLAG_INITIALIZED);
            aprog_init_trigger(ch->in_room->area);
            reset_area(ch->in_room->area);
            log_printf_plus(LOG_MONITOR, UMAX(GetMaxLevel(ch), LEVEL_LOG_CSET), SEV_NOTICE,
                            "%s initialized %s", GET_NAME(ch), ch->in_room->area->name);
        }
        if (IS_PLR2_FLAG(ch, PLR2_DIED) && !IS_IMMORTAL(ch))
            REMOVE_PLR2_FLAG(ch, PLR2_DIED);

#ifdef VTRACK
        vtrack_add_room(ch, ch->in_room->vnum);
#endif

        if (IS_ROOM_FLAG(ch->in_room, ROOM_ORPHANED) && !IS_IMMORTAL(ch))
            bug("%s in orphaned room %d", GET_NAME(ch), ch->in_room->vnum);
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
         &&   obj->item_type == ITEM_LIGHT
         &&   obj->value[2] != 0 )
        ++ch->in_room->light;

    /*
     * Room's effect on the character
     */
    if ( !char_died(ch) )
    {
        for ( paf = pRoomIndex->first_affect; paf; paf = paf->next )
            affect_modify(ch, paf, TRUE);

        if ( char_died(ch) )	/* could die from a wearspell, etc */
            return;
    }

    /*
     * Character's effect on the room
     */
    for ( paf = ch->first_affect; paf; paf = paf->next )
        room_affect(pRoomIndex, paf, TRUE);


    if ( !IS_NPC(ch)
         &&    IS_SET(ch->in_room->room_flags, ROOM_SAFE)
         &&    get_timer(ch, TIMER_SHOVEDRAG) <= 0 )
        add_timer( ch, TIMER_SHOVEDRAG, 10, NULL, 0 );  /*-30 Seconds-*/

    /*
     * Delayed Teleport rooms					-Thoric
     * Should be the last thing checked in this function
     */
    if ( IS_SET( ch->in_room->room_flags, ROOM_TELEPORT )
         &&	 ch->in_room->tele_delay > 0 )
    {
        TELEPORT_DATA *tele;

        for ( tele = first_teleport; tele; tele = tele->next )
            if ( tele->room == pRoomIndex )
                return;

        CREATE( tele, TELEPORT_DATA, 1 );
        LINK( tele, first_teleport, last_teleport, next, prev );
        tele->room		= pRoomIndex;
        tele->timer		= pRoomIndex->tele_delay;
    }
    if ( !ch->was_in_room )
        ch->was_in_room = ch->in_room;
    return;
}

/*
 * Give an obj to a char.
 */
OBJ_DATA *obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
    OBJ_DATA *otmp;
    OBJ_DATA *oret = obj;
    bool skipgroup, grouped;
    int oweight = get_obj_weight( obj );
    int onum = get_obj_number( obj );
    int wear_loc = obj->wear_loc;
    int extra_flags = obj->extra_flags;

    skipgroup = FALSE;
    grouped = FALSE;

    if (IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
    {
        if (!IS_IMMORTAL( ch )
            && (IS_NPC(ch) && !IS_SET(ch->act, ACT_PROTOTYPE)) )
            return obj_to_room( obj, ch->in_room );
    }

    if ( loading_char == ch )
    {
        int x,y;
        for ( x = 0; x < MAX_WEAR; x++ )
            for ( y = 0; y < MAX_LAYERS; y++ )
                if ( save_equipment[x][y] == obj )
                {
                    skipgroup = TRUE;
                    break;
                }
    }

    if ( !skipgroup )
        for ( otmp = ch->first_carrying; otmp; otmp = otmp->next_content )
            if ( (oret=group_object( otmp, obj )) == otmp )
            {
                grouped = TRUE;
                break;
            }

    if ( !grouped )
    {
        LINK( obj, ch->first_carrying, ch->last_carrying, next_content, prev_content );
        obj->carried_by	 		= ch;
        obj->in_room	 		= NULL;
        obj->in_obj	 		= NULL;

        if (!obj->last_carried_by)
        {
            char buf[128];
            snprintf(buf, 127, "obj_to_char %d %s", ch->vnum, ch->name);
            obj->last_carried_by = str_dup(buf);
        }
    }
    if (wear_loc == WEAR_NONE)
    {
        ch->carry_number	+= onum;
        ch->carry_weight	+= oweight;
    }
    else
        if ( !IS_SET(extra_flags, ITEM_MAGIC) )
            ch->carry_weight	+= oweight;

#ifdef VTRACK
    vtrack_add_obj(ch, oret ? oret->vnum : obj->vnum);
#endif

    return (oret ? oret : obj);
}



/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA *obj )
{
    CHAR_DATA *ch;
    char buf[128];

    if ( ( ch = obj->carried_by ) == NULL )
    {
        bug( "Obj_from_char: null ch." );
        return;
    }

    if ( obj->wear_loc != WEAR_NONE )
        unequip_char( ch, obj );

    /* obj may drop during unequip... */
    if ( !obj->carried_by )
        return;

    UNLINK( obj, ch->first_carrying, ch->last_carrying, next_content, prev_content );

    if ( IS_OBJ_STAT( obj, ITEM_COVERING ) && obj->first_content )
        empty_obj( obj, NULL, NULL );

    obj->in_room	 = NULL;
    obj->carried_by	 = NULL;
    ch->carry_number	-= get_obj_number( obj );
    ch->carry_weight	-= get_obj_weight( obj );

    if (obj->last_carried_by)
        DISPOSE(obj->last_carried_by);
    snprintf(buf, 127, "obj_from_char %d %s", ch->vnum, ch->name);
    obj->last_carried_by = str_dup(buf);

    return;
}


/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( OBJ_DATA *obj, int iWear )
{
    if ( obj->item_type != ITEM_ARMOR )
        return 0;

    switch ( iWear )
    {
    case WEAR_BODY:	return 3 * obj->value[0];
    case WEAR_HEAD:	return 2 * obj->value[0];
    case WEAR_LEGS:	return 2 * obj->value[0];
    case WEAR_FEET:	return     obj->value[0];
    case WEAR_HANDS:	return     obj->value[0];
    case WEAR_ARMS:	return     obj->value[0];
    case WEAR_SHIELD:	return     obj->value[0];
    case WEAR_FINGER_L:	return     obj->value[0];
    case WEAR_FINGER_R: return     obj->value[0];
    case WEAR_ANKLE_L:	return     obj->value[0];
    case WEAR_ANKLE_R:	return     obj->value[0];
    case WEAR_NECK_1:	return     obj->value[0];
    case WEAR_NECK_2:	return     obj->value[0];
    case WEAR_ABOUT:	return 2 * obj->value[0];
    case WEAR_WAIST:	return     obj->value[0];
    case WEAR_BACK:	return     obj->value[0];
    case WEAR_WRIST_L:	return     obj->value[0];
    case WEAR_WRIST_R:	return     obj->value[0];
    case WEAR_HOLD:	return     obj->value[0];
    case WEAR_EYES:	return	   obj->value[0];
    case WEAR_NOSE:	return	   obj->value[0];
    }

    return 0;
}



/*
 * Find a piece of eq on a character.
 * Will pick the top layer if clothing is layered.		-Thoric
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
    OBJ_DATA *obj, *maxobj = NULL;

    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
        if ( obj->wear_loc == iWear )
        {
            if ( !obj->pIndexData->layers )
                return obj;
            else if ( !maxobj ||
                      obj->pIndexData->layers > maxobj->pIndexData->layers )
                maxobj = obj;
        }

    return maxobj;
}


void add_obj_affects(CHAR_DATA *ch, OBJ_DATA *obj)
{
    AFFECT_DATA *paf;

    /*
    for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
        affect_modify( ch, paf, TRUE );
    */
    for ( paf = obj->first_affect; paf; paf = paf->next )
        affect_modify( ch, paf, TRUE );

}

/*
 * Equip a char with an obj.
 */
void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear )
{
    OBJ_DATA	*otmp;

    if ( obj->carried_by != ch )
    {
	bug( "Equip_char: obj(%s) is not carried by char(%s)", obj->name, GET_NAME(ch));
	return;
    }

    if ( (otmp=get_eq_char( ch, iWear )) != NULL
         &&   (!otmp->pIndexData->layers || !obj->pIndexData->layers) )
    {
        bug( "Equip_char: already equipped (%d).", iWear );
        return;
    }

    separate_obj(obj);	/* just in case */
    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
         ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
         ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
    {
        /*
         * Thanks to Morgenes for the bug fix here!
         */
        if ( loading_char != ch )
        {
            act( AT_MAGIC, "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
            act( AT_MAGIC, "$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM );
        }
        if ( obj->carried_by )
            obj_from_char( obj );
        obj_to_room( obj, ch->in_room );
        oprog_zap_trigger( ch, obj);
        if ( IS_SET(sysdata.save_flags, SV_ZAPDROP) && !char_died(ch) )
            save_char_obj( ch );
        return;
    }

    ch->armor      	-= apply_ac( obj, iWear );
    obj->wear_loc	 = iWear;

    ch->carry_number	-= get_obj_number( obj );
    if ( IS_SET( obj->extra_flags, ITEM_MAGIC ) )
        ch->carry_weight  -= get_obj_weight( obj );

    add_obj_affects(ch, obj);

    if ( obj->item_type == ITEM_LIGHT
         &&   obj->value[2] != 0
         &&   ch->in_room )
        ++ch->in_room->light;

    return;
}



/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
    AFFECT_DATA *paf;

    if ( obj->wear_loc == WEAR_NONE )
    {
        bug( "Unequip_char: already unequipped." );
        return;
    }

    ch->carry_number	+= get_obj_number( obj );
    if ( IS_SET( obj->extra_flags, ITEM_MAGIC ) )
        ch->carry_weight  += get_obj_weight( obj );

    ch->armor		+= apply_ac( obj, obj->wear_loc );
    obj->wear_loc	 = -1;

    /*
    for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
        affect_modify( ch, paf, FALSE );
    */

    if ( obj->carried_by )
        for ( paf = obj->first_affect; paf; paf = paf->next )
            affect_modify( ch, paf, FALSE );

    if ( !obj->carried_by )
        return;

    if ( obj->item_type == ITEM_LIGHT
         &&   obj->value[2] != 0
         &&   ch->in_room
         &&   ch->in_room->light > 0 )
        --ch->in_room->light;

    return;
}



/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list )
{
    OBJ_DATA *obj;
    int nMatch;

    nMatch = 0;
    for ( obj = list; obj; obj = obj->next_content )
        if ( obj->pIndexData == pObjIndex )
            nMatch++;

    return nMatch;
}



/*
 * Move an obj out of a room.
 */
void	write_corpses	args( ( CHAR_DATA *ch, char *name ) );

int falling;

void obj_from_room( OBJ_DATA *obj )
{
    ROOM_INDEX_DATA *in_room;
    AFFECT_DATA *paf;

    if ( ( in_room = obj->in_room ) == NULL )
    {
        bug( "obj_from_room: NULL." );
        return;
    }

    for ( paf = obj->first_affect; paf; paf = paf->next )
        room_affect(in_room, paf, FALSE);

    /*
    for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
        room_affect(in_room, paf, FALSE);
    */

    UNLINK( obj, in_room->first_content, in_room->last_content,
            next_content, prev_content );

    if ( IS_OBJ_STAT( obj, ITEM_COVERING ) && obj->first_content )
        empty_obj( obj, NULL, obj->in_room );

    if (obj->item_type == ITEM_FIRE)
        obj->in_room->light -= obj->count;

    obj->carried_by   = NULL;
    obj->in_obj	      = NULL;
    obj->in_room      = NULL;
    if ( obj->vnum == OBJ_VNUM_CORPSE_PC && falling == 0 )
        write_corpses( NULL, obj->short_descr+14 );
    return;
}


/*
 * Move an obj into a room.
 */
OBJ_DATA *obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *to_pRoomIndex )
{
    ROOM_INDEX_DATA *pRoomIndex;
    OBJ_DATA *otmp, *oret;
    sh_int count = obj->count;
    sh_int item_type = obj->item_type;
    AFFECT_DATA *paf;

    if (IS_DONATION(to_pRoomIndex))
        pRoomIndex = DONATION_ROOM();
    else
        pRoomIndex = to_pRoomIndex;

    for ( paf = obj->first_affect; paf; paf = paf->next )
        room_affect(pRoomIndex, paf, TRUE);

    /*
    for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
        room_affect(pRoomIndex, paf, TRUE);
    */

    for ( otmp = pRoomIndex->first_content; otmp; otmp = otmp->next_content )
        if ( (oret=group_object( otmp, obj )) == otmp )
        {
            if (item_type == ITEM_FIRE)
                pRoomIndex->light += count;
            return oret;
        }

    LINK( obj, pRoomIndex->first_content, pRoomIndex->last_content,
          next_content, prev_content );
    obj->in_room				= pRoomIndex;
    obj->carried_by				= NULL;
    obj->in_obj					= NULL;
    if (item_type == ITEM_FIRE)
        pRoomIndex->light += count;
    falling++;
    obj_fall( obj, FALSE );
    falling--;
    if ( obj->vnum == OBJ_VNUM_CORPSE_PC && falling == 0 )
        write_corpses( NULL, obj->short_descr+14 );
    return obj;
}



/*
 * Move an object into an object.
 */
OBJ_DATA *obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to )
{
    OBJ_DATA *otmp, *oret;

    if ( obj == obj_to )
    {
        bug( "Obj_to_obj: trying to put object inside itself: vnum %d", obj->vnum );
        return obj;
    }
    /* Big carry_weight bug fix here by Thoric */
    if ( obj->carried_by != obj_to->carried_by )
    {
        if ( obj->carried_by )
            obj->carried_by->carry_weight -= get_obj_weight( obj );
        if ( obj_to->carried_by )
            obj_to->carried_by->carry_weight += get_obj_weight( obj );
    }

    for ( otmp = obj_to->first_content; otmp; otmp = otmp->next_content )
        if ( (oret=group_object( otmp, obj )) == otmp )
            return oret;

    LINK( obj, obj_to->first_content, obj_to->last_content,
          next_content, prev_content );
    obj->in_obj				 = obj_to;
    obj->in_room			 = NULL;
    obj->carried_by			 = NULL;

    return obj;
}


/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_from;

    if ( ( obj_from = obj->in_obj ) == NULL )
    {
        bug( "Obj_from_obj: null obj_from." );
        return;
    }

    UNLINK( obj, obj_from->first_content, obj_from->last_content,
            next_content, prev_content );

    if ( IS_OBJ_STAT( obj, ITEM_COVERING ) && obj->first_content )
        empty_obj( obj, obj->in_obj, NULL );

    obj->in_obj       = NULL;
    obj->in_room      = NULL;
    obj->carried_by   = NULL;

    for ( ; obj_from; obj_from = obj_from->in_obj )
        if ( obj_from->carried_by )
            obj_from->carried_by->carry_weight -= get_obj_weight( obj );

    return;
}



/*
 * Extract an obj from the world.
 */
void extract_obj( OBJ_DATA *obj )
{
    OBJ_DATA *obj_content;

    if ( obj_extracted(obj) )
    {
        bug( "extract_obj: obj %d already extracted!", obj->vnum );
        return;
    }

    /*separate_obj(obj);*/

    if ( obj->item_type == ITEM_PORTAL )
        remove_portal( obj );

    while ( ( obj_content = obj->last_content ) != NULL )
    {
        obj_from_obj(obj_content);
        if (obj->in_room)
            obj_to_room(obj_content, obj->in_room);
        else if (obj->carried_by)
            obj_to_char(obj_content, obj->carried_by);
        else if (obj->in_obj)
        {
            obj_to_obj(obj_content, obj->in_obj);
            bug( "extract_obj: obj %d in obj", obj->vnum );
        }
        else
        {
            obj_to_room(obj_content, get_room_index(ROOM_VNUM_LIMBO));
            bug( "extract_obj: obj_content %d extracted to limbo!", obj_content->vnum );
        }

    }

    if ( obj->carried_by )
        obj_from_char( obj );
    else if ( obj->in_room )
        obj_from_room( obj );
    else if ( obj->in_obj )
        obj_from_obj( obj );

    if ( obj->carried_by )
        bug( "extract_obj: obj->carried_by" );
    if ( obj->in_room )
        bug( "extract_obj: obj->in_room" );
    if ( obj->in_obj )
        bug( "extract_obj: obj->in_obj" );


    {
        AFFECT_DATA *paf;
        AFFECT_DATA *paf_next;

        for ( paf = obj->first_affect; paf; paf = paf_next )
        {
            paf_next    = paf->next;
            DISPOSE( paf );
        }
        obj->first_affect = obj->last_affect = NULL;
    }

    {
        EXTRA_DESCR_DATA *ed;
        EXTRA_DESCR_DATA *ed_next;

        for ( ed = obj->first_extradesc; ed; ed = ed_next )
        {
            ed_next = ed->next;
            STRFREE( ed->description );
            STRFREE( ed->keyword     );
            DISPOSE( ed );
        }
        obj->first_extradesc = obj->last_extradesc = NULL;
    }

    if (obj->name)
	STRFREE(obj->name);
    if (obj->short_descr)
	STRFREE(obj->short_descr);
    if (obj->description)
	STRFREE(obj->description);
    if (obj->action_desc)
	STRFREE(obj->action_desc);
    if (obj->last_carried_by)
        DISPOSE(obj->last_carried_by);

    if (obj->christened)
    {
        bug("extracting christened obj: %s", get_christen_name(obj));
        obj->christened = NULL;
    }

    if ( obj->vars )
        free_variables( obj->vars );

    if ( obj == gobj_prev )
        gobj_prev		= obj->prev;

    obj->pIndexData->count -= obj->count;
    numobjsloaded -= obj->count;
    --physicalobjects;

    UNLINK( obj, first_object, last_object, next, prev );
    DISPOSE(obj);
    return;
}


void recall_char( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *location = NULL;

    char_from_room( ch );

    if ( !IS_NPC(ch) )
    {
	if ( ch->pcdata->home )
	{
            load_area_demand( ch->pcdata->home );
	    location = get_room_index( ch->pcdata->home );
	}
	if ( !location && ch->pcdata->clan )
	{
            load_area_demand( ch->pcdata->clan->recall );
	    location = get_room_index( ch->pcdata->clan->recall );
	}
    }

    if ( !location )
    {
	if ( IsBadSide(ch) )
	{
            load_area_demand( ROOM_START_EVIL );
	    location = get_room_index( ROOM_START_EVIL );
	}
	else
	{
            load_area_demand( ROOM_START_GOOD );
	    location = get_room_index( ROOM_START_GOOD );
	}
    }

    if ( !location )
    {
	load_area_demand( ROOM_VNUM_ASTRAL_ENTRANCE );
	location = get_room_index( ROOM_VNUM_ASTRAL_ENTRANCE );
    }

    if ( !location )
    {
	load_area_demand( ROOM_VNUM_LIMBO );
	location = get_room_index( ROOM_VNUM_LIMBO );
    }

    if ( !location )
    {
	bug("recall_char: no room to recall to!");
        abort();
    }

    char_to_room( ch, location );
}



/*
 * Extract a char from the world.
 */
void extract_char( CHAR_DATA *ch, bool fPull )
{
    CHAR_DATA *wch;
    OBJ_DATA *obj, *prev_obj;
    char buf[MAX_STRING_LENGTH];

    if ( !ch )
    {
        bug( "Extract_char: NULL ch." );
        return;		/* who removed this line? */
    }

    if ( !ch->in_room )
    {
        bug( "Extract_char: NULL room." );
        return;
    }

    if ( ch == supermob )
    {
        bug( "Extract_char: ch == supermob!" );
        return;
    }

    if ( char_died(ch) )
    {
        sprintf( buf, "extract_char: %s already died!", ch->name );
        bug( buf, 0 );
        return;
    }
    if ( ch == cur_char )
        cur_char_died = TRUE;

    if ( gch_prev == ch )
        gch_prev = ch->prev;

    if ( fPull && !IS_SET(ch->act, ACT_POLYMORPHED))
        die_follower( ch );

    stop_fighting( ch, TRUE );

    if ( ch->mount )
    {
        REMOVE_BIT( ch->mount->act, ACT_MOUNTED );
        ch->mount = NULL;
        ch->position = POS_STANDING;
    }

    if ( IS_NPC(ch) && IS_SET( ch->act, ACT_MOUNTED ) )
        for ( wch = first_char; wch; wch = wch->next )
            if ( wch->mount == ch )
            {
                wch->mount = NULL;
                wch->position = POS_STANDING;
            }


    REMOVE_BIT( ch->act, ACT_MOUNTED );


    prev_obj = ch->last_carrying;

    while ( (obj = prev_obj) )
    {
        prev_obj = obj->prev_content;
        extract_obj( obj );
    }

    if ( !fPull )
    {
        recall_char( ch );
        /*
         if ( ( wch = get_char_room( ch, "healer" ) ) != NULL )
         {
         act( AT_MAGIC, "$n mutters a few incantations, waves $s hands and points $s finger.",
         wch, NULL, NULL, TO_ROOM );
         act( AT_MAGIC, "$n appears from some strange swirling mists!", ch, NULL, NULL, TO_ROOM );
         sprintf(buf, "Welcome back to the land of the living, %s",
         capitalize( ch->name ) );
         do_say( wch, buf );
         }
         else
         act( AT_MAGIC, "$n appears from some strange swirling mists!", ch, NULL, NULL, TO_ROOM );
         */
        ch->position = POS_RESTING;

        if (ch->desc)
            flush_buffer(ch->desc, FALSE);

        return;
    }

#ifdef IRC
    irc_logoff(ch);
#endif

    char_from_room( ch );

    if ( IS_NPC(ch) )
    {
        --ch->pIndexData->count;
        --nummobsloaded;
    }

    if ( ch->desc && ch->desc->original && IS_SET(ch->act, ACT_POLYMORPHED))
        do_revert( ch, "" );

    if ( ch->desc && ch->desc->original )
        do_return( ch, "" );

    for ( wch = first_char; wch; wch = wch->next )
        if ( wch->reply == ch )
            wch->reply = NULL;

    UNLINK( ch, first_char, last_char, next, prev );

    if ( ch->desc )
    {
        if ( ch->desc->character != ch )
            bug( "Extract_char: char's descriptor points to another char" );
        else
        {
            ch->desc->character = NULL;
            close_socket( ch->desc, FALSE );
            ch->desc = NULL;
        }
    }

    free_char(ch);

    return;
}


/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int number, count, vnum, unum;

    if (!argument || argument[0] == '\0')
        return NULL;

    if ( !str_cmp( argument, "self" ) )
        return ch;

    /* check the room for an unum */
    if ((unum=unum_arg(argument)))
    {
        for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
            if ( can_see( ch, rch ) && rch->unum == unum )
                return rch;
    }

    number = number_argument( argument, arg );

    if ( get_trust(ch) >= LEVEL_SAVIOR && is_number( arg ) )
        vnum = atoi( arg );
    else
        vnum = -1;

    count  = 0;

    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
        if ( can_see( ch, rch ) &&
             (nifty_is_name( arg, rch->name ) ||
              nifty_is_name( arg, PERS(rch, ch) ) ||
              vnum == rch->vnum) )
        {
            if ( number == 0 && !IS_NPC(rch) )
                return rch;
            else
                if ( ++count == number )
                    return rch;
        }

    if ( vnum != -1 )
        return NULL;

    /* If we didn't find an exact match, run through the list of characters
     again looking for prefix matching, ie gu == guard.
     Added by Narn, Sept/96
     */
    count  = 0;
    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
        if ( !can_see( ch, rch ) ||
             (!nifty_is_name_prefix( arg, PERS(rch, ch) ) &&
              !nifty_is_name_prefix( arg, rch->name )) )
            continue;
        if ( number == 0 && !IS_NPC(rch) )
            return rch;
        else
            if ( ++count == number )
                return rch;
    }

    return NULL;
}




/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    int number, count, vnum, unum;

    if (!argument || argument[0] == '\0' || !ch)
        return NULL;

    if ( !str_cmp( argument, "self" ) )
        return ch;

    /* check the world for an unum */
    if ((unum=unum_arg(argument)))
    {
        for ( wch = first_char; wch; wch = wch->next )
            if ( can_see( ch, wch ) && wch->unum == unum )
                return wch;
    }

    number = number_argument( argument, arg );
    count  = 0;

    /*
     * Allow reference by vnum for saints+			-Thoric
     */
    if ( get_trust(ch) >= LEVEL_SAVIOR && is_number( arg ) )
        vnum = atoi( arg );
    else
        vnum = -1;

    /* check the room for an exact match */
    for ( wch = ch->in_room->first_person; wch; wch = wch->next_in_room )
        if ( can_see( ch, wch ) &&
             (nifty_is_name( arg, wch->name ) ||
              vnum == wch->vnum) )
        {
            if ( number == 0 && !IS_NPC(wch) )
                return wch;
            else
                if ( ++count == number )
                    return wch;
        }

    count = 0;

    /* check the world for an exact match */
    for ( wch = first_char; wch; wch = wch->next )
        if ( can_see( ch, wch ) &&
             (nifty_is_name( arg, wch->name ) ||
              vnum == wch->vnum) )
        {
            if ( number == 0 && !IS_NPC(wch) )
                return wch;
            else
                if ( ++count == number )
                    return wch;
        }

    count = 0;

    /* check the room for an exact match - with intro desc */
    for ( wch = ch->in_room->first_person; wch; wch = wch->next_in_room )
        if ( can_see( ch, wch ) &&
             (nifty_is_name( arg, PERS(wch, ch) ) ||
              nifty_is_name( arg, wch->name ) ||
              vnum == wch->vnum) )
        {
            if ( number == 0 && !IS_NPC(wch) )
                return wch;
            else
                if ( ++count == number )
                    return wch;
        }

    count = 0;

    /* check the world for an exact match - with intro desc */
    for ( wch = first_char; wch; wch = wch->next )
        if ( can_see( ch, wch ) &&
             (nifty_is_name( arg, PERS(wch, ch) ) ||
              nifty_is_name( arg, wch->name ) ||
              vnum == wch->vnum) )
        {
            if ( number == 0 && !IS_NPC(wch) )
                return wch;
            else
                if ( ++count == number )
                    return wch;
        }

    count = 0;

    /* bail out if looking for a vnum match */
    if ( vnum != -1 )
        return NULL;

    /*
     * If we didn't find an exact match, check the room for
     * for a prefix match, ie gu == guard.
     * Added by Narn, Sept/96
     */
    count  = 0;
    for ( wch = ch->in_room->first_person; wch; wch = wch->next_in_room )
    {
        if ( !can_see( ch, wch ) ||
             (!nifty_is_name_prefix( arg, PERS(wch, ch) ) &&
              !nifty_is_name_prefix( arg, wch->name )) )
            continue;
        if ( number == 0 && !IS_NPC(wch) )
            return wch;
        else
            if ( ++count == number )
                return wch;
    }

    /*
     * If we didn't find a prefix match in the room, run through the full list
     * of characters looking for prefix matching, ie gu == guard.
     * Added by Narn, Sept/96
     */
    count  = 0;
    for ( wch = first_char; wch; wch = wch->next )
    {
        if ( !can_see( ch, wch ) ||
             (!nifty_is_name_prefix( arg, PERS(wch, ch) ) &&
              !nifty_is_name_prefix( arg, wch->name )) )
            continue;
        if ( number == 0 && !IS_NPC(wch) )
            return wch;
        else
            if ( ++count == number )
                return wch;
    }

    return NULL;
}

CHAR_DATA *get_char_area( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *wch;
    int number, count, vnum, unum;

    if (!argument || !ch)
        return NULL;

    if ( !str_cmp( argument, "self" ) )
        return ch;

    /* check the area for an unum */
    if ((unum = unum_arg(argument)))
    {
        for ( wch = first_char; wch; wch = wch->next )
            if ( can_see( ch, wch ) &&
                 ch->in_room->area == wch->in_room->area &&
                 wch->unum == unum )
                return wch;
    }

    number = number_argument( argument, arg );
    count  = 0;

    /*
     * Allow reference by vnum for saints+			-Thoric
     */
    if ( get_trust(ch) >= LEVEL_SAVIOR && is_number( arg ) )
        vnum = atoi( arg );
    else
        vnum = -1;

    /* check the room for an exact match */
    for ( wch = ch->in_room->first_person; wch; wch = wch->next_in_room )
        if ( can_see( ch, wch ) &&
             (nifty_is_name( arg, PERS(wch, ch) ) ||
              nifty_is_name( arg, wch->name ) ||
              vnum == wch->vnum) )
        {
            if ( number == 0 && !IS_NPC(wch) )
                return wch;
            else
                if ( ++count == number )
                    return wch;
        }

    count = 0;

    /* check the zone for an exact match */
    for ( wch = first_char; wch; wch = wch->next )
        if ( can_see( ch, wch ) &&
             (ch->in_room->area == wch->in_room->area) &&
             (nifty_is_name( arg, PERS(wch, ch) ) ||
              nifty_is_name( arg, wch->name ) ||
              vnum == wch->vnum) )
        {
            if ( number == 0 && !IS_NPC(wch) )
                return wch;
            else
                if ( ++count == number )
                    return wch;
        }

    /* bail out if looking for a vnum match */
    if ( vnum != -1 )
        return NULL;

    /*
     * If we didn't find an exact match, check the room for
     * for a prefix match, ie gu == guard.
     * Added by Narn, Sept/96
     */
    count  = 0;
    for ( wch = ch->in_room->first_person; wch; wch = wch->next_in_room )
    {
        if ( !can_see( ch, wch ) ||
             (!nifty_is_name_prefix( arg, PERS(wch, ch) ) &&
              !nifty_is_name_prefix( arg, wch->name )) )
            continue;
        if ( number == 0 && !IS_NPC(wch) )
            return wch;
        else
            if ( ++count == number )
                return wch;
    }

    /*
     * If we didn't find a prefix match in the room, run through the full list
     * of characters looking for prefix matching, ie gu == guard.
     * Added by Narn, Sept/96
     */
    count  = 0;
    for ( wch = first_char; wch; wch = wch->next )
    {
        if ( !can_see( ch, wch ) ||
             (ch->in_room->area != wch->in_room->area) ||
             (!nifty_is_name_prefix( arg, PERS(wch, ch) ) &&
              !nifty_is_name_prefix( arg, wch->name )) )
            continue;
        if ( number == 0 && !IS_NPC(wch) )
            return wch;
        else
            if ( ++count == number )
                return wch;
    }

    return NULL;
}



/*
 * Find some object with a given index data.
 * Used by area-reset 'P', 'T' and 'H' commands.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex )
{
    OBJ_DATA *obj;

    for ( obj = last_object; obj; obj = obj->prev )
        if ( obj->pIndexData == pObjIndex )
            return obj;

    return NULL;
}


/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, char *argument, OBJ_DATA *list )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number, unum;
    int count;

    if ((unum = unum_arg(argument)))
    {
        for ( obj = list; obj; obj = obj->next_content )
            if ( can_see_obj( ch, obj ) && obj->unum == unum )
                return obj;
    }

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = list; obj; obj = obj->next_content )
        if ( can_see_obj( ch, obj ) && nifty_is_name( arg, obj->name ) )
            if ( (count += obj->count) >= number )
                return obj;

    /* If we didn't find an exact match, run through the list of objects
     again looking for prefix matching, ie swo == sword.
     Added by Narn, Sept/96
     */
    count = 0;
    for ( obj = list; obj; obj = obj->next_content )
        if ( can_see_obj( ch, obj ) && nifty_is_name_prefix( arg, obj->name ) )
            if ( (count += obj->count) >= number )
                return obj;

    return NULL;
}

/*
 * Find an obj in a list...going the other way			-Thoric
 */
OBJ_DATA *get_obj_list_rev( CHAR_DATA *ch, char *argument, OBJ_DATA *list )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number, unum;
    int count;

    if ((unum = unum_arg(argument)))
    {
        for ( obj = list; obj; obj = obj->prev_content )
            if ( can_see_obj( ch, obj ) && obj->unum == unum )
                return obj;
    }

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = list; obj; obj = obj->prev_content )
        if ( can_see_obj( ch, obj ) && nifty_is_name( arg, obj->name ) )
            if ( (count += obj->count) >= number )
                return obj;

    /* If we didn't find an exact match, run through the list of objects
     again looking for prefix matching, ie swo == sword.
     Added by Narn, Sept/96
     */
    count = 0;
    for ( obj = list; obj; obj = obj->prev_content )
        if ( can_see_obj( ch, obj ) && nifty_is_name_prefix( arg, obj->name ) )
            if ( (count += obj->count) >= number )
                return obj;

    return NULL;
}



/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number, count, vnum, unum;

    if ((unum = unum_arg(argument)))
    {
        for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
            if ( can_see_obj( ch, obj ) && obj->unum == unum )
                return obj;
    }

    number = number_argument( argument, arg );

    if ( get_trust(ch) >= LEVEL_SAVIOR && is_number( arg ) )
        vnum = atoi( arg );
    else
        vnum = -1;

    count  = 0;
    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
        if ( obj->wear_loc == WEAR_NONE
             &&   can_see_obj( ch, obj )
             &&  (nifty_is_name( arg, obj->name ) || obj->vnum == vnum) )
            if ( (count += obj->count) >= number )
                return obj;

    if ( vnum != -1 )
        return NULL;

    /* If we didn't find an exact match, run through the list of objects
     again looking for prefix matching, ie swo == sword.
     Added by Narn, Sept/96
     */
    count = 0;
    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
        if ( obj->wear_loc == WEAR_NONE
             &&   can_see_obj( ch, obj )
             &&   nifty_is_name_prefix( arg, obj->name ) )
            if ( (count += obj->count) >= number )
                return obj;

    return NULL;
}



/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number, count, vnum, unum;

    if ((unum = unum_arg(argument)))
    {
        for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
            if ( obj->wear_loc != WEAR_NONE && can_see_obj( ch, obj ) && obj->unum == unum )
                return obj;
    }

    number = number_argument( argument, arg );

    if ( get_trust(ch) >= LEVEL_SAVIOR && is_number( arg ) )
        vnum = atoi( arg );
    else
        vnum = -1;

    count  = 0;
    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
        if ( obj->wear_loc != WEAR_NONE
             &&   can_see_obj( ch, obj )
             &&  (nifty_is_name( arg, obj->name ) || obj->vnum == vnum) )
            if ( ++count == number )
                return obj;

    if ( vnum != -1 )
        return NULL;

    /* If we didn't find an exact match, run through the list of objects
     again looking for prefix matching, ie swo == sword.
     Added by Narn, Sept/96
     */
    count = 0;
    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
        if ( obj->wear_loc != WEAR_NONE
             &&   can_see_obj( ch, obj )
             &&   nifty_is_name_prefix( arg, obj->name ) )
            if ( ++count == number )
                return obj;

    return NULL;
}



/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;

    if ( ( obj = get_obj_carry( ch, argument ) ) != NULL )
        return obj;

    if ( ( obj = get_obj_wear( ch, argument ) ) != NULL )
        return obj;

    if ( IS_DONATION(ch->in_room) )
        obj = get_obj_list_rev( ch, argument, DONATION_ROOM()->last_content );
    else
        obj = get_obj_list_rev( ch, argument, ch->in_room->last_content );
    if ( obj )
        return obj;

    return NULL;
}



/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int number, count, vnum, unum;

    if ( ( obj = get_obj_here( ch, argument ) ) != NULL )
        return obj;

    if ((unum = unum_arg(argument)))
    {
        for ( obj = first_object; obj; obj = obj->next )
            if ( can_see_obj( ch, obj ) && obj->unum == unum )
                return obj;
    }

    number = number_argument( argument, arg );

    /*
     * Allow reference by vnum for saints+			-Thoric
     */
    if ( get_trust(ch) >= LEVEL_SAVIOR && is_number( arg ) )
        vnum = atoi( arg );
    else
        vnum = -1;

    count  = 0;
    for ( obj = first_object; obj; obj = obj->next )
        if ( can_see_obj( ch, obj ) && (nifty_is_name( arg, obj->name )
                                        ||   vnum == obj->vnum) )
            if ( (count += obj->count) >= number )
                return obj;

    /* bail out if looking for a vnum */
    if ( vnum != -1 )
        return NULL;

    /* If we didn't find an exact match, run through the list of objects
     again looking for prefix matching, ie swo == sword.
     Added by Narn, Sept/96
     */
    count  = 0;
    for ( obj = first_object; obj; obj = obj->next )
        if ( can_see_obj( ch, obj ) && nifty_is_name_prefix( arg, obj->name ) )
            if ( (count += obj->count) >= number )
                return obj;

    return NULL;
}


/*
 * How mental state could affect finding an object		-Thoric
 * Used by get/drop/put/quaff/recite/etc
 * Increasingly freaky based on mental state and drunkeness
 */
bool ms_find_obj( CHAR_DATA *ch )
{
    int ms = ch->mental_state;
    int drunk = IS_NPC(ch) ? 0 : ch->pcdata->condition[COND_DRUNK];
    char *t;

    /*
     * we're going to be nice and let nothing weird happen unless
     * you're a tad messed up
     */
    drunk = UMAX( 1, drunk );
    if ( abs(ms) + (drunk/3) < 30 )
        return FALSE;
    if ( (number_percent() + (ms < 0 ? 15 : 5))> abs(ms)/2 + drunk/4 )
        return FALSE;
    if ( ms > 15 )	/* range 1 to 20 */
        switch( number_range( UMAX(1, (ms/5-15)), (ms+4) / 5 ) )
        {
        default:
        case  1: t="As you reach for it, you forgot what it was...\n\r";					break;
        case  2: t="As you reach for it, something inside stops you...\n\r";				break;
        case  3: t="As you reach for it, it seems to move out of the way...\n\r";				break;
        case  4: t="You grab frantically for it, but can't seem to get a hold of it...\n\r";		break;
        case  5: t="It disappears as soon as you touch it!\n\r";						break;
        case  6: t="You would if it would stay still!\n\r";							break;
        case  7: t="Whoa!  It's covered in blood!  Ack!  Ick!\n\r";						break;
        case  8: t="Wow... trails!\n\r";									break;
        case  9: t="You reach for it, then notice the back of your hand is growing something!\n\r";		break;
        case 10: t="As you grasp it, it shatters into tiny shards which bite into your flesh!\n\r";		break;
        case 11: t="What about that huge dragon flying over your head?!?!?\n\r";				break;
        case 12: t="You stratch yourself instead...\n\r";							break;
        case 13: t="You hold the universe in the palm of your hand!\n\r";					break;
        case 14: t="You're too scared.\n\r";								break;
        case 15: t="Your mother smacks your hand... 'NO!'\n\r";						break;
        case 16: t="Your hand grasps the worse pile of revoltingness than you could ever imagine!\n\r";	break;
        case 17: t="You stop reaching for it as it screams out at you in pain!\n\r";			break;
        case 18: t="What about the millions of burrow-maggots feasting on your arm?!?!\n\r";		break;
        case 19: t="That doesn't matter anymore... you've found the true answer to everything!\n\r";	break;
        case 20: t="A supreme entity has no need for that.\n\r";						break;
        }
    else
    {
        int sub = URANGE(1, abs(ms)/2 + drunk, 60);
        switch( number_range( 1, sub/10 ) )
        {
        default:
        case  1: t="In just a second...\n\r";				break;
        case  2: t="You can't find that...\n\r";					break;
        case  3: t="It's just beyond your grasp...\n\r";				break;
        case  4: t="...but it's under a pile of other stuff...\n\r";		break;
        case  5: t="You go to reach for it, but pick your nose instead.\n\r";	break;
        case  6: t="Which one?!?  I see two... no three...\n\r";			break;
        }
    }
    send_to_char( t, ch );
    return TRUE;
}


/*
 * Generic get obj function that supports optional containers.	-Thoric
 * currently only used for "eat" and "quaff".
 */
OBJ_DATA *find_obj( CHAR_DATA *ch, char *argument, bool carryonly )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj = NULL;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( !str_cmp( arg2, "from" )
         &&   argument[0] != '\0' )
        argument = one_argument( argument, arg2 );

    if ( arg2[0] == '\0' )
    {
        if ( carryonly && ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
        {
            send_to_char( "You do not have that item.\n\r", ch );
            return NULL;
        }
        else
            if ( !carryonly && ( obj = get_obj_here( ch, arg1 ) ) == NULL )
            {
                act( AT_PLAIN, "I see no $T here.", ch, NULL, arg1, TO_CHAR );
                return NULL;
            }
        return obj;
    }
    else
    {
        OBJ_DATA *container = NULL;

        if ( carryonly
             && ( container = get_obj_carry( ch, arg2 ) ) == NULL
             && ( container = get_obj_wear( ch, arg2 ) ) == NULL )
        {
            send_to_char( "You do not have that item.\n\r", ch );
            return NULL;
        }
        if ( !carryonly && ( container = get_obj_here( ch, arg2 ) ) == NULL )
        {
            act( AT_PLAIN, "I see no $T here.", ch, NULL, arg2, TO_CHAR );
            return NULL;
        }

        if ( !IS_OBJ_STAT(container, ITEM_COVERING )
             &&    IS_SET(container->value[1], CONT_CLOSED) )
        {
            act( AT_PLAIN, "The $d is closed.", ch, NULL, container->name, TO_CHAR );
            return NULL;
        }

        obj = get_obj_list( ch, arg1, container->first_content );
        if ( !obj )
            act( AT_PLAIN, IS_OBJ_STAT(container, ITEM_COVERING) ?
                 "I see nothing like that beneath $p." :
                 "I see nothing like that in $p.",
                 ch, container, NULL, TO_CHAR );
        return obj;
    }
    return NULL;
}

int get_obj_number( OBJ_DATA *obj )
{
    return obj->count;
}


/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA *obj )
{
    int weight;

    if ( IS_OBJ_STAT2(obj, ITEM2_STORED_ITEM) )
        return 0;

    if ( obj->item_type == ITEM_MONEY )
        return money_weight(obj->value[0], obj->value[2]);

    weight = obj->count * obj->weight;
    for ( obj = obj->first_content; obj; obj = obj->next_content )
        weight += get_obj_weight( obj );

    return weight;
}

bool room_is_inside( ROOM_INDEX_DATA *pRoomIndex)
{
    if ( !pRoomIndex )
    {
        bug( "room_is_inside: NULL pRoomIndex" );
        return TRUE;
    }

    if (IS_ROOM_FLAG(pRoomIndex, ROOM_INDOORS) ||
        pRoomIndex->sector_type == SECT_INSIDE)
        return TRUE;

    return FALSE;
}


/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex )
{
    if ( !pRoomIndex )
    {
        bug( "room_is_dark: NULL pRoomIndex" );
        return TRUE;
    }

    if (IS_SYSTEMFLAG(SYS_ECLIPSE))
        return TRUE;

    if (IS_AREA_FLAG(pRoomIndex->area,AFLAG_DARK))
        return TRUE;

    if ( pRoomIndex->light > 0 )
        return FALSE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_DARK) )
        return TRUE;

    if ( pRoomIndex->sector_type == SECT_INSIDE
         ||   pRoomIndex->sector_type == SECT_CITY )
        return FALSE;

    if ( time_info.sunlight == SUN_SET
         ||   time_info.sunlight == SUN_DARK )
        return TRUE;

    return FALSE;
}



/*
 * True if room is private.
 */
bool room_is_private( ROOM_INDEX_DATA *pRoomIndex )
{
    CHAR_DATA *rch;
    int count;

    if ( !pRoomIndex )
    {
        bug( "room_is_private: NULL pRoomIndex" );
        return FALSE;
    }

    count = 0;
    for ( rch = pRoomIndex->first_person; rch; rch = rch->next_in_room )
        count++;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)  && count >= 3 )
        return TRUE;

    if ( IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1 )
        return TRUE;

    return FALSE;
}



/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch )
    {
        if ( IS_AFFECTED(victim, AFF_INVISIBLE)
             ||   IS_AFFECTED(victim, AFF_HIDE)
             ||   (IS_AFFECTED2(victim, AFF2_ANIMAL_INVIS) && IsAnimal(ch))
             ||   (victim->pcdata && victim->pcdata->wizinvis) )
            return FALSE;
        else
            return TRUE;
    }

    if ( ch == victim )
        return TRUE;

    if ( IS_AFFECTED2(victim, AFF2_BEACON) )
        return TRUE;

    if ( !IS_NPC(victim)
         &&   get_trust( ch ) < victim->pcdata->wizinvis )
        return FALSE;

    /* SB */
    if ( IS_NPC(victim)
         &&   IS_SET(victim->act, ACT_MOBINVIS)
         &&   get_trust( ch ) < victim->mobinvis )
        return FALSE;

    if ( !IS_IMMORTAL(ch) && !IS_NPC(victim) && !victim->desc
         &&    get_timer(victim, TIMER_RECENTFIGHT) > 0
         &&  (!victim->switched || !IS_AFFECTED(victim->switched, AFF_POSSESS)) )
        return FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) )
        return TRUE;

    /* The miracle cure for blindness? -- Altrag */
    if ( !IS_AFFECTED(ch, AFF_TRUESIGHT) )
    {
        if ( IS_AFFECTED(ch, AFF_BLIND) )
            return FALSE;

        if ( room_is_dark( ch->in_room ) && !IS_AFFECTED(ch, AFF_INFRARED) )
            return FALSE;

        if ( IS_AFFECTED(victim, AFF_INVISIBLE)
             &&  !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
            return FALSE;

        if ( IS_AFFECTED(victim, AFF_HIDE)
             &&   !IS_AFFECTED(ch, AFF_DETECT_HIDDEN)
             &&   !victim->fighting)
            return FALSE;
    }

    /* Redone by Narn to let newbie council members see pre-auths. */
    if( NOT_AUTHED( victim ) )
    {
        if( NOT_AUTHED( ch ) || IS_IMMORTAL( ch ) || IS_NPC( ch ) )
            return TRUE;

        if( ch->pcdata->council && !str_cmp( ch->pcdata->council->name, "Newbie Council" ) )
            return TRUE;

        return FALSE;
    }

    /* Commented out for who list purposes
     if (!NOT_AUTHED(victim) && NOT_AUTHED(ch) && !IS_IMMORTAL(victim)
     && !IS_NPC(victim))
     return FALSE;*/
    return TRUE;
}



/*
 * True if char can see obj.
 */
bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !ch || !obj )
        return FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) )
        return TRUE;

    if ( IS_OBJ_STAT( obj, ITEM_BURRIED ) )
        return FALSE;

    /*
     *   PC's can't see items in storage -- Heath
     */

    if ( IS_OBJ_STAT2( obj, ITEM2_STORED_ITEM ) )
        return FALSE;

    if ( IS_AFFECTED( ch, AFF_TRUESIGHT ) )
        return TRUE;

    if ( IS_AFFECTED( ch, AFF_BLIND ) )
        return FALSE;

    if ( IS_OBJ_STAT(obj, ITEM_HIDDEN) )
        return FALSE;

    if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
        return TRUE;

    if ( room_is_dark( ch->in_room ) && !IS_AFFECTED(ch, AFF_INFRARED) )
        return FALSE;

    if ( IS_OBJ_STAT(obj, ITEM_INVIS)
         &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
        return FALSE;

    return TRUE;
}



/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
    if ( !IS_OBJ_STAT(obj, ITEM_NODROP) )
        return TRUE;

    if ( !IS_NPC(ch) && GetMaxLevel(ch) >= LEVEL_IMMORTAL )
        return TRUE;

    if ( IS_NPC(ch) && ch->vnum == 3 )
        return TRUE;

    return FALSE;
}


/*
 * Return ascii name of an item type.
 */
char *item_type_name( OBJ_DATA *obj )
{
    if ( obj->item_type < 1 || obj->item_type > MAX_ITEM_TYPE )
    {
        bug( "Item_type_name: vnum: %d. unknown type %d.",
             obj->vnum, obj->item_type );
        return "(unknown)";
    }

    return o_types[obj->item_type];
}


/*
 * Return ascii name of an affect location.
 */
char *affect_loc_name( sh_int location )
{
    switch ( location )
    {
    case MAX_APPLY_TYPE:
    case APPLY_NONE:		return "none";
    case APPLY_BLAH:		return "blah";
    case APPLY_STR:		return "strength";
    case APPLY_DEX:		return "dexterity";
    case APPLY_INT:		return "intelligence";
    case APPLY_WIS:		return "wisdom";
    case APPLY_CON:		return "constitution";
    case APPLY_CHA:		return "charisma";
    case APPLY_LCK:		return "luck";
    case APPLY_SEX:		return "sex";
    case APPLY_CLASS:		return "class";
    case APPLY_LEVEL:		return "level";
    case APPLY_AGE:		return "age";
    case APPLY_MANA:		return "mana";
    case APPLY_HIT:		return "hp";
    case APPLY_MOVE:		return "moves";
    case APPLY_MANA_REGEN:	return "mana regen";
    case APPLY_HIT_REGEN:	return "hp regen ";
    case APPLY_MOVE_REGEN:	return "move regen";
    case APPLY_ANTIMAGIC:       return "antimagic";
    case APPLY_GOLD:		return "gold";
    case APPLY_EXP:		return "experience";
    case APPLY_AC:		return "armor class";
    case APPLY_HITROLL:		return "hit roll";
    case APPLY_DAMROLL:		return "damage roll";
    case APPLY_SAVING_POISON:	return "save vs poison";
    case APPLY_SAVING_ROD:	return "save vs rod";
    case APPLY_SAVING_PARA:	return "save vs paralysis";
    case APPLY_SAVING_BREATH:	return "save vs breath";
    case APPLY_SAVING_SPELL:	return "save vs spell";
    case APPLY_HEIGHT:		return "height";
    case APPLY_WEIGHT:		return "weight";
    case APPLY_AFFECT:		return "affected_by";
    case APPLY_AFF2:		return "affected_by2";
    case APPLY_RESISTANT:	return "resistant";
    case APPLY_IMMUNE:		return "immune";
    case APPLY_SUSCEPTIBLE:	return "susceptible";
    case APPLY_BACKSTAB:	return "backstab";
    case APPLY_PICK:		return "pick";
    case APPLY_TRACK:		return "track";
    case APPLY_STEAL:		return "steal";
    case APPLY_SNEAK:		return "sneak";
    case APPLY_HIDE:		return "hide";
    case APPLY_PALM:		return "palm";
    case APPLY_DETRAP:		return "detrap";
    case APPLY_FINDTRAP:      	return "findtrap";
    case APPLY_DODGE:		return "dodge";
    case APPLY_SF:		return "spellfail";
    case APPLY_SCAN:		return "scan";
    case APPLY_GOUGE:		return "gouge";
    case APPLY_SEARCH:		return "search";
    case APPLY_MOUNT:		return "mount";
    case APPLY_DISARM:		return "disarm";
    case APPLY_KICK:		return "kick";
    case APPLY_PARRY:		return "parry";
    case APPLY_BASH:		return "bash";
    case APPLY_STUN:		return "stun";
    case APPLY_PUNCH:		return "punch";
    case APPLY_CLIMB:		return "climb";
    case APPLY_GRIP:		return "grip";
    case APPLY_SCRIBE:		return "scribe";
    case APPLY_BREW:		return "brew";
    case APPLY_WEAPONSPELL:	return "weapon spell";
    case APPLY_WEARSPELL:	return "wear spell";
    case APPLY_REMOVESPELL:	return "remove spell";
    case APPLY_MENTALSTATE:	return "mental state";
    case APPLY_EMOTION:		return "emotional state";
    case APPLY_STRIPSN:		return "dispel";
    case APPLY_REMOVE:		return "remove";
    case APPLY_DIG:		return "dig";
    case APPLY_FULL:		return "hunger";
    case APPLY_THIRST:		return "thirst";
    case APPLY_DRUNK:		return "drunk";
    case APPLY_BLOOD:		return "blood";
    case APPLY_ROOMFLAG:	return "roomflag";
    case APPLY_SECTORTYPE:	return "sectortype";
    case APPLY_ROOMLIGHT:	return "roomlight";
    case APPLY_TELEVNUM:	return "teleport vnum";
    case APPLY_TELEDELAY:	return "teleport delay";
    case APPLY_COOK:            return "cook";
    case APPLY_RACE:            return "race";
    case APPLY_NUMATTACKS:      return "numattacks";
    case APPLY_BARENUMDIE:      return "barenumdie";
    case APPLY_BARESIZDIE:      return "baresizdie";
    case APPLY_HITNDAM:         return "hit-n-dam";
    case APPLY_SAVING_ALL:      return "all saves";
    case APPLY_EAT_SPELL:       return "eat spell";
    case APPLY_RACE_SLAYER:     return "race slayer";
    case APPLY_ALIGN_SLAYER:    return "alignment slayer";
    case APPLY_NUMMEM:          return "number memorized";
    case APPLY_IMMUNESPELL:     return "immune spell";
    case APPLY_ABSORB:		return "absorb";
    }

    bug( "Affect_location_name: unknown location %d.", location );
    return "(unknown)";
}


/*
 * Set off a trap (obj) upon character (ch)			-Thoric
 */
ch_ret spring_trap( CHAR_DATA *ch, OBJ_DATA *obj )
{
    int dam;
    int typ;
    int lev;
    char *txt;
    char buf[MAX_STRING_LENGTH];
    ch_ret retcode;

    typ = obj->value[1];
    lev = obj->value[2];

    retcode = rNONE;

    switch(typ)
    {
    default:
        txt = "hit by a trap";					break;
    case TRAP_TYPE_POISON_GAS:
        txt = "surrounded by a green cloud of gas";		break;
    case TRAP_TYPE_POISON_DART:
        txt = "hit by a dart";					break;
    case TRAP_TYPE_POISON_NEEDLE:
        txt = "pricked by a needle";				break;
    case TRAP_TYPE_POISON_DAGGER:
        txt = "stabbed by a dagger";				break;
    case TRAP_TYPE_POISON_ARROW:
        txt = "struck with an arrow";				break;
    case TRAP_TYPE_BLINDNESS_GAS:
        txt = "surrounded by a red cloud of gas";		break;
    case TRAP_TYPE_SLEEPING_GAS:
        txt = "surrounded by a yellow cloud of gas";		break;
    case TRAP_TYPE_FLAME:
        txt = "struck by a burst of flame";			break;
    case TRAP_TYPE_EXPLOSION:
        txt = "hit by an explosion";				break;
    case TRAP_TYPE_ACID_SPRAY:
        txt = "covered by a spray of acid";			break;
    case TRAP_TYPE_ELECTRIC_SHOCK:
        txt = "suddenly shocked";				break;
    case TRAP_TYPE_BLADE:
        txt = "sliced by a razor sharp blade";			break;
    case TRAP_TYPE_SEX_CHANGE:
        txt = "surrounded by a mysterious aura";		break;
    case TRAP_TYPE_FLOOD_ROOM:
        txt = "caught up in a flood";           		break;
    }

    dam = number_range( obj->value[2], obj->value[2] * 2);
    sprintf( buf, "You are %s!", txt );
    act( AT_HITME, buf, ch, NULL, NULL, TO_CHAR );
    sprintf( buf, "$n is %s.", txt );
    act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
    --obj->value[0];
    if ( obj->value[0] <= 0 )
        extract_obj( obj );
    switch(typ)
    {
    default:
    case TRAP_TYPE_POISON_DART:
    case TRAP_TYPE_POISON_NEEDLE:
    case TRAP_TYPE_POISON_DAGGER:
    case TRAP_TYPE_POISON_ARROW:
        /* hmm... why not use spell_poison() here? */
        retcode = obj_cast_spell( gsn_poison, lev, ch, ch, NULL );
        if ( retcode == rNONE )
            retcode = damage( ch, ch, dam, TYPE_UNDEFINED );
        break;
    case TRAP_TYPE_POISON_GAS:
        retcode = obj_cast_spell( gsn_poison, lev, ch, ch, NULL );
        break;
    case TRAP_TYPE_BLINDNESS_GAS:
        retcode = obj_cast_spell( gsn_blindness, lev, ch, ch, NULL );
        break;
    case TRAP_TYPE_SLEEPING_GAS:
        retcode = obj_cast_spell( gsn_sleep, lev, ch, ch, NULL );
        break;
    case TRAP_TYPE_ACID_SPRAY:
        retcode = obj_cast_spell( gsn_acid_blast, lev, ch, ch, NULL );
        break;
    case TRAP_TYPE_SEX_CHANGE:
        retcode = obj_cast_spell( skill_lookup("change sex"), lev, ch, ch, NULL );
        break;
    case TRAP_TYPE_FLAME:
    case TRAP_TYPE_EXPLOSION:
        retcode = obj_cast_spell( gsn_fireball, lev, ch, ch, NULL );
        break;
    case TRAP_TYPE_ELECTRIC_SHOCK:
    case TRAP_TYPE_BLADE:
        retcode = damage( ch, ch, dam, TYPE_UNDEFINED );
        break;
    case TRAP_TYPE_FLOOD_ROOM:
        retcode = rNONE;
        break;
    }
    return retcode;
}

/*
 * Check an object for a trap					-Thoric
 */
ch_ret check_for_trap( CHAR_DATA *ch, OBJ_DATA *obj, int flag )
{
    OBJ_DATA *check;
    ch_ret    retcode;

    if ( !obj->first_content )
        return rNONE;

    retcode = rNONE;

    for ( check = obj->first_content; check; check = check->next_content )
        if ( check->item_type == ITEM_TRAP
             &&   IS_SET(check->value[3], flag) )
        {
            retcode = spring_trap( ch, check );
            if ( retcode != rNONE )
                return retcode;
        }
    return retcode;
}

/*
 * Check the room for a trap					-Thoric
 */
ch_ret check_room_for_traps( CHAR_DATA *ch, int flag )
{
    OBJ_DATA *check;
    ch_ret    retcode;

    retcode = rNONE;

    if ( !ch )
        return rERROR;
    if ( !ch->in_room || !ch->in_room->first_content )
        return rNONE;

    for ( check = ch->in_room->first_content; check; check = check->next_content )
    {
        if ( check->item_type == ITEM_TRAP
             &&   IS_SET(check->value[3], flag) )
        {
            retcode = spring_trap( ch, check );
            if ( retcode != rNONE )
                return retcode;
        }
    }
    return retcode;
}

/*
 * return TRUE if an object contains a trap			-Thoric
 */
bool is_trapped( OBJ_DATA *obj )
{
    OBJ_DATA *check;

    if ( !obj->first_content )
        return FALSE;

    for ( check = obj->first_content; check; check = check->next_content )
        if ( check->item_type == ITEM_TRAP )
            return TRUE;

    return FALSE;
}

/*
 * If an object contains a trap, return the pointer to the trap	-Thoric
 */
OBJ_DATA *get_trap( OBJ_DATA *obj )
{
    OBJ_DATA *check;

    if ( !obj->first_content )
        return NULL;

    for ( check = obj->first_content; check; check = check->next_content )
        if ( check->item_type == ITEM_TRAP )
            return check;

    return NULL;
}

/*
 * Remove an exit from a room					-Thoric
 */
void extract_exit( ROOM_INDEX_DATA *room, EXIT_DATA *pexit )
{
    UNLINK( pexit, room->first_exit, room->last_exit, next, prev );
    if ( pexit->rexit )
        pexit->rexit->rexit = NULL;
    STRFREE( pexit->keyword );
    STRFREE( pexit->description );
    DISPOSE( pexit );
}

/*
 * Remove a room
 */
void extract_room( ROOM_INDEX_DATA *room )
{
    bug( "extract_room: not implemented" );
    /*
     (remove room from hash table)
     clean_room( room )
     DISPOSE( room );
     */
    return;
}

/*
 * clean out a room (leave list pointers intact )		-Thoric
 */
void clean_room( ROOM_INDEX_DATA *room )
{
    EXTRA_DESCR_DATA	*ed, *ed_next;
    EXIT_DATA		*pexit, *pexit_next;

    STRFREE( room->description );
    STRFREE( room->name );
    for ( ed = room->first_extradesc; ed; ed = ed_next )
    {
        ed_next = ed->next;
        STRFREE( ed->description );
        STRFREE( ed->keyword );
        DISPOSE( ed );
        top_ed--;
    }
    room->first_extradesc	= NULL;
    room->last_extradesc		= NULL;
    for ( pexit = room->first_exit; pexit; pexit = pexit_next )
    {
        pexit_next = pexit->next;
        STRFREE( pexit->keyword );
        STRFREE( pexit->description );
        DISPOSE( pexit );
        top_exit--;
    }
    room->first_exit = NULL;
    room->last_exit = NULL;
    room->room_flags = 0;
    room->sector_type = 0;
    room->light = 0;
}

/*
 * clean out an object (index) (leave list pointers intact )	-Thoric
 */
void clean_obj( OBJ_INDEX_DATA *obj )
{
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    EXTRA_DESCR_DATA *ed;
    EXTRA_DESCR_DATA *ed_next;
    MPROG_DATA *mprog, *mprog_next;

    STRFREE( obj->name );
    STRFREE( obj->short_descr );
    STRFREE( obj->description );
    STRFREE( obj->action_desc );

    xCLEAR_BITS(obj->progtypes);

    for ( mprog = obj->mudprogs; mprog; mprog = mprog_next )
    {
        mprog_next = mprog->next;
        STRFREE( mprog->arglist );
        STRFREE( mprog->comlist );
        DISPOSE( mprog );
    }

    obj->item_type		= 0;
    obj->extra_flags	= 0;
    obj->wear_flags		= 0;
    obj->count		= 0;
    obj->weight		= 0;
    obj->cost		= 0;
    obj->rent		= 0;
    obj->currtype       = DEFAULT_CURR;
    obj->value[0]		= 0;
    obj->value[1]		= 0;
    obj->value[2]		= 0;
    obj->value[3]		= 0;
    obj->value[4]		= 0;
    obj->value[5]		= 0;
    for ( paf = obj->first_affect; paf; paf = paf_next )
    {
        paf_next    = paf->next;
        DISPOSE( paf );
        top_affect--;
    }
    obj->first_affect	= NULL;
    obj->last_affect	= NULL;
    for ( ed = obj->first_extradesc; ed; ed = ed_next )
    {
        ed_next		= ed->next;
        STRFREE( ed->description );
        STRFREE( ed->keyword     );
        DISPOSE( ed );
        top_ed--;
    }
    obj->first_extradesc	= NULL;
    obj->last_extradesc	= NULL;
}

/*
 * clean out a mobile (index) (leave list pointers intact )	-Thoric
 */
void clean_mob( MOB_INDEX_DATA *mob )
{
    MPROG_DATA *mprog, *mprog_next;
    int i;

    STRFREE( mob->player_name );
    STRFREE( mob->short_descr );
    STRFREE( mob->long_descr  );
    STRFREE( mob->description );
    mob->spec_fun	= NULL;
    mob->pShop	= NULL;
    mob->rShop	= NULL;
    xCLEAR_BITS(mob->progtypes);

    for ( mprog = mob->mudprogs; mprog; mprog = mprog_next )
    {
        mprog_next = mprog->next;
        STRFREE( mprog->arglist );
        STRFREE( mprog->comlist );
        DISPOSE( mprog );
    }
    mob->count	 = 0;	   mob->killed		= 0;
    mob->sex	 = 0;

    for (i = 0; i < MAX_CLASS; ++i)
        mob->levels[i] = 0;

    mob->act2	 = 0;	   mob->affected_by2	= 0;
    mob->act	 = 0;	   mob->affected_by	= 0;
    mob->alignment	 = 0;	   mob->mobthac0	= 0;
    mob->ac		 = 0;	   mob->hitnodice	= 0;
    mob->hitsizedice = 0;	   mob->hitplus		= 0;
    mob->damnodice	 = 0;	   mob->damsizedice	= 0;
    mob->damplus	 = 0;
    GET_EXP(mob)	 = 0;	   mob->position	= 0;
    mob->defposition = 0;	   mob->height		= 0;
    mob->weight	 = 0;	/* mob->vnum		= 0;	*/
    for (i=0;i<MAX_CURR_TYPE;i++)
        GET_MONEY(mob,i) = 0;
}

extern int top_reset;

/*
 * Remove all resets from an area				-Thoric
 */
void clean_resets( AREA_DATA *tarea )
{
    RESET_DATA *pReset, *pReset_next;

    for ( pReset = tarea->first_reset; pReset; pReset = pReset_next )
    {
        pReset_next = pReset->next;
        DISPOSE( pReset );
        --top_reset;
    }
    tarea->first_reset	= NULL;
    tarea->last_reset	= NULL;
}


/*
 * "Roll" players stats based on the character name		-Thoric
 */
void name_stamp_stats( CHAR_DATA *ch )
{
    unsigned int x;
    int a, b, c;

    for ( x = 0; x < strlen(ch->name); x++ )
    {
        c = ch->name[x] + x;
        b = c % 14;
        a = (c % 1) + 1;
        switch (b)
        {
        case  0:
            ch->perm_str = UMIN( 18, ch->perm_str + a );
            break;
        case  1:
            ch->perm_dex = UMIN( 18, ch->perm_dex + a );
            break;
        case  2:
            ch->perm_wis = UMIN( 18, ch->perm_wis + a );
            break;
        case  3:
            ch->perm_int = UMIN( 18, ch->perm_int + a );
            break;
        case  4:
            ch->perm_con = UMIN( 18, ch->perm_con + a );
            break;
        case  5:
            ch->perm_cha = UMIN( 18, ch->perm_cha + a );
            break;
        case  6:
            ch->perm_lck = UMIN( 18, ch->perm_lck + a );
            break;
        case  7:
            ch->perm_str = UMAX(  9, ch->perm_str - a );
            break;
        case  8:
            ch->perm_dex = UMAX(  9, ch->perm_dex - a );
            break;
        case  9:
            ch->perm_wis = UMAX(  9, ch->perm_wis - a );
            break;
        case 10:
            ch->perm_int = UMAX(  9, ch->perm_int - a );
            break;
        case 11:
            ch->perm_con = UMAX(  9, ch->perm_con - a );
            break;
        case 12:
            ch->perm_cha = UMAX(  9, ch->perm_cha - a );
            break;
        case 13:
            ch->perm_lck = UMAX(  9, ch->perm_lck - a );
            break;
        }
    }
}

/*
 * "Fix" a character's stats					-Thoric
 */
void fix_char( CHAR_DATA *ch )
{
    AFFECT_DATA *aff;
    OBJ_DATA *carry[MAX_LEVEL*200];
    OBJ_DATA *obj;
    int x, ncarry;

    de_equip_char( ch );

    ncarry = 0;
    while ( (obj=ch->first_carrying) != NULL )
    {
        carry[ncarry++]  = obj;
        obj_from_char( obj );
    }

    for ( aff = ch->first_affect; aff; aff = aff->next )
        affect_modify( ch, aff, FALSE );

    ch->affected_by	= race_table[ch->race].affected;
    ch->mental_state	= -10;
    GET_HIT(ch)		= UMAX( 1, GET_HIT(ch)  );
    GET_MANA(ch)	= UMAX( 1, GET_MANA(ch) );
    GET_MOVE(ch)	= UMAX( 1, GET_MOVE(ch) );
    ch->armor		= 100;
    ch->mod_str		= 0;
    ch->mod_dex		= 0;
    ch->mod_wis		= 0;
    ch->mod_int		= 0;
    ch->mod_con		= 0;
    ch->mod_cha		= 0;
    ch->mod_lck		= 0;
    ch->damroll		= 0;
    ch->hitroll		= 0;
    ch->alignment	= URANGE( -1000, ch->alignment, 1000 );
    ch->saving_breath	= 0;
    ch->saving_wand	= 0;
    ch->saving_para_petri = 0;
    ch->saving_spell_staff = 0;
    ch->saving_poison_death = 0;

    ch->carry_weight	= 0;
    ch->carry_number	= 0;

    ClassSpecificStuff(ch);

    for ( aff = ch->first_affect; aff; aff = aff->next )
        affect_modify( ch, aff, TRUE );

    for ( x = 0; x < ncarry; x++ )
        obj_to_char( carry[x], ch );

    re_equip_char( ch );
}


/*
 * Show an affect verbosely to a character			-Thoric
 */
void showaffect( CHAR_DATA *ch, AFFECT_DATA *paf )
{
    char buf[MAX_STRING_LENGTH];
    int x;

    if ( !paf )
    {
        bug( "showaffect: NULL paf" );
        return;
    }
    if ( paf->location != APPLY_NONE && paf->modifier != 0 )
    {
        switch( paf->location )
        {
        default:
            sprintf( buf, "Affects %s by %d.\n\r",
                     affect_loc_name( paf->location ), paf->modifier );
            break;
        case APPLY_AFFECT:
            sprintf( buf, "Affects %s by",
                     affect_loc_name( paf->location ) );
            for ( x = 0; x < 32 ; x++ )
                if ( IS_SET( paf->modifier, 1 << x ) )
                {
                    strcat( buf, " " );
                    strcat( buf, a_flags[x] );
                }
            strcat( buf, "\n\r" );
            break;
        case APPLY_AFF2:
            sprintf( buf, "Affects %s by",
                     affect_loc_name( paf->location ) );
            for ( x = 0; x < 32 ; x++ )
                if ( IS_SET( paf->modifier, 1 << x ) )
                {
                    strcat( buf, " " );
                    strcat( buf, a2_flags[x] );
                }
            strcat( buf, "\n\r" );
            break;
        case APPLY_EAT_SPELL:
            sprintf( buf, "Casts eat spell %d '%s'\n\r",
                     paf->modifier,
                     IS_VALID_SN(paf->modifier) ? skill_table[paf->modifier]->name
                     : "unknown" );
            break;
        case APPLY_WEAPONSPELL:
            sprintf( buf, "Casts weapon spell %d '%s'\n\r",
                     paf->modifier,
                     IS_VALID_SN(paf->modifier) ? skill_table[paf->modifier]->name
                     : "unknown" );
            break;
        case APPLY_WEARSPELL:
            sprintf( buf, "Casts wear spell %d '%s'\n\r",
                     paf->modifier,
                     IS_VALID_SN(paf->modifier) ? skill_table[paf->modifier]->name
                     : "unknown" );
            break;
        case APPLY_REMOVESPELL:
            sprintf( buf, "Casts remove spell %d '%s'\n\r",
                     paf->modifier,
                     IS_VALID_SN(paf->modifier) ? skill_table[paf->modifier]->name
                     : "unknown" );
            break;
        case APPLY_IMMUNESPELL:
            sprintf( buf, "Immune to spell %d '%s'\n\r",
                     paf->modifier,
                     IS_VALID_SN(paf->modifier) ? skill_table[paf->modifier]->name
                     : "unknown" );
            break;
        case APPLY_ALIGN_SLAYER:
            sprintf( buf, "Alignment Slayer\n\r");
            break;
        case APPLY_RACE_SLAYER:
            sprintf( buf, "Double Damage vs. any %s\n\r",
                     race_table[paf->modifier].race_name );
            break;
        case APPLY_RESISTANT:
        case APPLY_IMMUNE:
        case APPLY_SUSCEPTIBLE:
        case APPLY_ABSORB:
            sprintf( buf, "Affects %s by",
                     affect_loc_name( paf->location ) );
            for ( x = 0; x < 32 ; x++ )
                if ( IS_SET( paf->modifier, 1 << x ) )
                {
                    strcat( buf, " " );
                    strcat( buf, ris_flags[x] );
                }
            strcat( buf, "\n\r" );
            break;
        }
        send_to_char( buf, ch );
    }
}


/*
 * Check the recently extracted object queue for obj		-Thoric
 */
bool obj_extracted( OBJ_DATA *obj )
{
    return FALSE;
}

/*
 * Set the current global character to ch			-Thoric
 */
void set_cur_char( CHAR_DATA *ch )
{
    cur_char	   = ch;
    cur_char_died  = FALSE;
    cur_room	   = ch->in_room;
    global_retcode = rNONE;
}

/*
 * Check to see if ch died recently				-Thoric
 */
bool real_char_died( CHAR_DATA *ch, char *file, int line )
{
    if (!ch)
    {
	bug("char_died (%s:%d): NULL ch", file, line);
	return TRUE;
    }

    return FALSE;
}

/*
 * Add a timer to ch						-Thoric
 * Support for "call back" time delayed commands
 */
void add_timer( CHAR_DATA *ch, sh_int type, sh_int count, DO_FUN *fun, int value )
{
    TIMER *timer;

    for ( timer = ch->first_timer; timer; timer = timer->next )
        if ( timer->type == type )
        {
            timer->count  = count;
            timer->do_fun = fun;
            timer->value	 = value;
            break;
        }
    if ( !timer )
    {
        CREATE( timer, TIMER, 1 );
        timer->count	= count;
        timer->type	= type;
        timer->do_fun	= fun;
        timer->value	= value;
        LINK( timer, ch->first_timer, ch->last_timer, next, prev );
    }
}

TIMER *get_timerptr( CHAR_DATA *ch, sh_int type )
{
    TIMER *timer;

    for ( timer = ch->first_timer; timer; timer = timer->next )
        if ( timer->type == type )
            return timer;
    return NULL;
}

sh_int get_timer( CHAR_DATA *ch, sh_int type )
{
    TIMER *timer;

    if ( (timer = get_timerptr( ch, type )) != NULL )
        return timer->count;
    else
        return 0;
}

void extract_timer( CHAR_DATA *ch, TIMER *timer )
{
    if ( !timer )
    {
        bug( "extract_timer: NULL timer" );
        return;
    }

    UNLINK( timer, ch->first_timer, ch->last_timer, next, prev );
    DISPOSE( timer );
    return;
}

void remove_timer( CHAR_DATA *ch, sh_int type )
{
    TIMER *timer;

    for ( timer = ch->first_timer; timer; timer = timer->next )
        if ( timer->type == type )
            break;

    if ( timer )
        extract_timer( ch, timer );
}

bool in_soft_range( CHAR_DATA *ch, AREA_DATA *tarea )
{
    if ( IS_IMMORTAL(ch) )
        return TRUE;
    else
        if ( IS_NPC(ch) )
            return TRUE;
        else
            if ( GetMaxLevel(ch) >= tarea->low_soft_range ||
                 GetMaxLevel(ch) <= tarea->hi_soft_range )
                return TRUE;
            else
                return FALSE;
}

bool in_hard_range( CHAR_DATA *ch, AREA_DATA *tarea )
{
    if ( IS_IMMORTAL(ch) )
        return TRUE;
    else
        if ( IS_NPC(ch) )
            return TRUE;
        else
            if ( tarea->low_hard_range == 0 && tarea->hi_hard_range == 0 )
                return TRUE;
            else
                if ( GetMaxLevel(ch) >= tarea->low_hard_range &&
                     GetMaxLevel(ch) <= tarea->hi_hard_range )
                    return TRUE;
                else
                    return FALSE;
}


/*
 * Scryn, standard luck check 2/2/96
 */
bool chance( CHAR_DATA *ch, sh_int percent )
{
    /*  sh_int clan_factor, ms;*/
    sh_int deity_factor, ms;

    if (!ch)
    {
        bug("Chance: null ch!");
        return FALSE;
    }

    /* Code for clan stuff put in by Narn, Feb/96.  The idea is to punish clan
     members who don't keep their alignment in tune with that of their clan by
     making it harder for them to succeed at pretty much everything.  Clan_factor
     will vary from 1 to 3, with 1 meaning there is no effect on the player's
     change of success, and with 3 meaning they have half the chance of doing
     whatever they're trying to do.

     Note that since the neutral clannies can only be off by 1000 points, their
     maximum penalty will only be half that of the other clan types.

     if ( IS_CLANNED( ch ) )
     clan_factor = 1 + abs( ch->alignment - ch->pcdata->clan->alignment ) / 1000;
     else
     clan_factor = 1;
     */
    /* Mental state bonus/penalty:  Your mental state is a ranged value with
     * zero (0) being at a perfect mental state (bonus of 10).
     * negative values would reflect how sedated one is, and
     * positive values would reflect how stimulated one is.
     * In most circumstances you'd do best at a perfectly balanced state.
     */

    if ( IS_DEVOTED( ch ) )
        deity_factor = ch->pcdata->favor / -200;
    else
        deity_factor = 0;

    ms = 10 - abs(ch->mental_state);

    if ( (number_percent() - get_curr_lck(ch) + 13 - ms) + deity_factor <=
         percent )
        return TRUE;
    else
        return FALSE;
}

bool chance_attrib( CHAR_DATA *ch, sh_int percent, sh_int attrib )
{
    /* Scryn, standard luck check + consideration of 1 attrib 2/2/96*/
    sh_int  deity_factor;

    if (!ch)
    {
        bug("Chance: null ch!");
        return FALSE;
    }

    if ( IS_DEVOTED( ch ) )
        deity_factor = ch->pcdata->favor / -200;
    else
        deity_factor = 0;

    if (number_percent() - get_curr_lck(ch) + 13 - attrib + 13 + deity_factor <= percent )
        return TRUE;
    else
        return FALSE;

}


/*
 * Make a simple clone of an object (no extras...yet)		-Thoric
 */
OBJ_DATA *clone_object( OBJ_DATA *obj )
{
    OBJ_DATA *clone;

    clone = new_obj();
    clone->pIndexData	= obj->pIndexData;
    clone->vnum         = obj->vnum;
    clone->name		= QUICKLINK( obj->name );
    clone->short_descr	= QUICKLINK( obj->short_descr );
    clone->description	= QUICKLINK( obj->description );
    clone->action_desc	= QUICKLINK( obj->action_desc );
    clone->item_type	= obj->item_type;
    clone->extra_flags	= obj->extra_flags;
    clone->extra_flags2	= obj->extra_flags2;
    clone->magic_flags	= obj->magic_flags;
    clone->wear_flags	= obj->wear_flags;
    clone->wear_loc	= obj->wear_loc;
    clone->weight	= obj->weight;
    clone->cost		= obj->cost;
    clone->currtype     = obj->currtype;
    clone->rent		= obj->rent;
    clone->timer	= obj->timer;
    clone->value[0]	= obj->value[0];
    clone->value[1]	= obj->value[1];
    clone->value[2]	= obj->value[2];
    clone->value[3]	= obj->value[3];
    clone->value[4]	= obj->value[4];
    clone->value[5]	= obj->value[5];
    clone->count	= 1;
    ++obj->pIndexData->count;
    ++numobjsloaded;
    ++physicalobjects;
    LINK( clone, first_object, last_object, next, prev );
    return clone;
}

/*
 * If possible group obj2 into obj1				-Thoric
 * This code, along with clone_object, obj->count, and special support
 * for it implemented throughout handler.c and save.c should show improved
 * performance on MUDs with players that hoard tons of potions and scrolls
 * as this will allow them to be grouped together both in memory, and in
 * the player files.
 */
OBJ_DATA *group_object( OBJ_DATA *obj1, OBJ_DATA *obj2 )
{
    if ( !obj1 || !obj2 )
        return NULL;
    if ( obj1 == obj2 )
        return obj1;

    if ( obj1->pIndexData == obj2->pIndexData
         /*
          &&	!obj1->pIndexData->mudprogs
          &&  !obj2->pIndexData->mudprogs
          */
         &&   QUICKMATCH( obj1->name,	obj2->name )
         &&   QUICKMATCH( obj1->short_descr,	obj2->short_descr )
         &&   QUICKMATCH( obj1->description,	obj2->description )
         &&   QUICKMATCH( obj1->action_desc,	obj2->action_desc )
         &&   obj1->vnum	        == obj2->vnum
         &&   obj1->item_type	        == obj2->item_type
         &&   obj1->extra_flags	        == obj2->extra_flags
         &&   obj1->extra_flags2	== obj2->extra_flags2
         &&   obj1->magic_flags 	== obj2->magic_flags
         &&   obj1->wear_flags          == obj2->wear_flags
         &&   obj1->wear_loc		== obj2->wear_loc
         &&   obj1->weight       	== obj2->weight
         &&   obj1->cost	        == obj2->cost
         &&   obj1->rent	        == obj2->rent
         &&   obj1->currtype     	== obj2->currtype
         &&   obj1->timer		== obj2->timer
         &&   obj1->value[0]		== obj2->value[0]
         &&   obj1->value[1]		== obj2->value[1]
         &&   obj1->value[2]		== obj2->value[2]
         &&   obj1->value[3]		== obj2->value[3]
         &&   obj1->value[4]		== obj2->value[4]
         &&   obj1->value[5]		== obj2->value[5]
         &&  !obj1->first_extradesc     && !obj2->first_extradesc
         &&  !obj1->first_affect	&& !obj2->first_affect
         &&  !obj1->first_content	&& !obj2->first_content
         &&  !obj1->christened          && !obj2->christened
         &&  !obj1->spec_fun            && !obj2->spec_fun)
    {
        obj1->count += obj2->count;
        obj1->pIndexData->count += obj2->count;	/* to be decremented in */
        numobjsloaded += obj2->count;		/* extract_obj */
        extract_obj( obj2 );
        return obj1;
    }
    return obj2;
}

/*
 * Split off a grouped object					-Thoric
 * decreased obj's count to num, and creates a new object containing the rest
 */
void split_obj( OBJ_DATA *obj, int num )
{
    int count = obj->count;
    OBJ_DATA *rest;

    if ( count <= num || num == 0 )
        return;

    rest = clone_object(obj);
    --obj->pIndexData->count;	/* since clone_object() ups this value */
    --numobjsloaded;
    rest->count = obj->count - num;
    obj->count = num;

    if ( obj->carried_by )
    {
        LINK( rest, obj->carried_by->first_carrying,
              obj->carried_by->last_carrying,
              next_content, prev_content );
        rest->carried_by	 	= obj->carried_by;
        rest->in_room	 		= NULL;
        rest->in_obj	 		= NULL;
    }
    else
        if ( obj->in_room )
        {
            LINK( rest, obj->in_room->first_content, obj->in_room->last_content,
                  next_content, prev_content );
            rest->carried_by	 	= NULL;
            rest->in_room	 		= obj->in_room;
            rest->in_obj	 		= NULL;
        }
        else
            if ( obj->in_obj )
            {
                LINK( rest, obj->in_obj->first_content, obj->in_obj->last_content,
                      next_content, prev_content );
                rest->in_obj			 = obj->in_obj;
                rest->in_room			 = NULL;
                rest->carried_by		 = NULL;
            }
}

void separate_obj( OBJ_DATA *obj )
{
    split_obj( obj, 1 );
}

/*
 * Empty an obj's contents... optionally into another obj, or a room
 */
bool empty_obj( OBJ_DATA *obj, OBJ_DATA *destobj, ROOM_INDEX_DATA *destroom )
{
    OBJ_DATA *otmp, *otmp_next;
    CHAR_DATA *ch = obj->carried_by;
    bool movedsome = FALSE;

    if ( !obj )
    {
        bug( "empty_obj: NULL obj" );
        return FALSE;
    }
    if ( destobj || (!destroom && !ch && (destobj = obj->in_obj) != NULL) )
    {
        for ( otmp = obj->first_content; otmp; otmp = otmp_next )
        {
            otmp_next = otmp->next_content;
            if ( destobj->item_type == ITEM_CONTAINER
                 &&   get_obj_weight( otmp ) + get_obj_weight( destobj )
                 > destobj->value[0] )
                continue;
            obj_from_obj( otmp );
            obj_to_obj( otmp, destobj );
            movedsome = TRUE;
        }
        return movedsome;
    }
    if ( destroom || (!ch && (destroom = obj->in_room) != NULL) )
    {
        for ( otmp = obj->first_content; otmp; otmp = otmp_next )
        {
            otmp_next = otmp->next_content;
            if ( ch && HAS_PROG(otmp->pIndexData, DROP_PROG) && otmp->count > 1 )
            {
                separate_obj( otmp );
                obj_from_obj( otmp );
                if ( !otmp_next )
                    otmp_next = obj->first_content;
            }
            else
                obj_from_obj( otmp );
            otmp = obj_to_room( otmp, destroom );
            if ( ch )
            {
                quest_trigger_objplace(ch, otmp, destroom);
                oprog_drop_trigger( ch, otmp );		/* mudprogs */
                if ( char_died(ch) )
                    ch = NULL;
            }
            movedsome = TRUE;
        }
        return movedsome;
    }
    if ( ch )
    {
        for ( otmp = obj->first_content; otmp; otmp = otmp_next )
        {
            otmp_next = otmp->next_content;
            obj_from_obj( otmp );
            obj_to_char( otmp, ch );
            movedsome = TRUE;
        }
        return movedsome;
    }
    bug( "empty_obj: could not determine a destination for vnum %d",
         obj->vnum );
    return FALSE;
}

/*
 * Improve mental state						-Thoric
 */
void better_mental_state( CHAR_DATA *ch, int mod )
{
    int c = URANGE( 0, abs(mod), 20 );
    int con = get_curr_con(ch);

    if (IS_IMMORTAL(ch))
        ch->mental_state = 0;

    c += number_percent() < con ? 1 : 0;

    if ( ch->mental_state < 0 )
        ch->mental_state = URANGE( -100, ch->mental_state + c, 0 );
    else
        if ( ch->mental_state > 0 )
            ch->mental_state = URANGE( 0, ch->mental_state - c, 100 );
}

/*
 * Deteriorate mental state					-Thoric
 */
void worsen_mental_state( CHAR_DATA *ch, int mod )
{
    int c   = URANGE( 0, abs(mod), 20 );
    int con = get_curr_con(ch);

    if (IS_IMMORTAL(ch))
        ch->mental_state = 0;

    c -= number_percent() < con ? 1 : 0;
    if ( c < 1 )
        return;

    if ( ch->mental_state < 0 )
        ch->mental_state = URANGE( -100, ch->mental_state - c, 100 );
    else
        if ( ch->mental_state > 0 )
            ch->mental_state = URANGE( -100, ch->mental_state + c, 100 );
        else
            ch->mental_state -= c;
}


/*
 * Add gold to an area's economy                                -Thoric
 */
void boost_economy( AREA_DATA *tarea, int gold, int type )
{
    while ( gold >= 1000000000 )
    {
        ++tarea->high_economy[type];
        gold -= 1000000000;
    }
    tarea->low_economy[type] += gold;
    while ( tarea->low_economy[type] >= 1000000000 )
    {
        ++tarea->high_economy[type];
        tarea->low_economy[type] -= 1000000000;
    }
}

/*
 * Take gold from an area's economy                             -Thoric
 */
void lower_economy( AREA_DATA *tarea, int gold, int type )
{
    while ( gold >= 1000000000 )
    {
        --tarea->high_economy[type];
        gold -= 1000000000;
    }
    tarea->low_economy[type] -= gold;
    while ( tarea->low_economy[type] < 0 )
    {
        --tarea->high_economy[type];
        tarea->low_economy[type] += 1000000000;
    }
}

/*
 * Check to see if economy has at least this much gold             -Thoric
 */
bool economy_has( AREA_DATA *tarea, int gold, int type )
{
    int hasgold = ((tarea->high_economy[type] > 0) ? 1 : 0) * 1000000000
        + tarea->low_economy[type];

    if ( hasgold >= gold )
        return TRUE;
    return FALSE;
}

/*
 * Used in db.c when resetting a mob into an area                   -Thoric
 * Makes sure mob doesn't get more than 10% of that area's gold,
 * and reduces area economy by the amount of gold given to the mob
 */
void economize_mobgold( CHAR_DATA *mob )
{
    AREA_DATA *tarea;
    int gold, type;

    for (type=0;type<MAX_CURR_TYPE;type++)
    {
        /* make sure it isn't way too much */
        GET_MONEY(mob,type) = UMIN( GET_MONEY(mob,type), GetMaxLevel(mob) * GetMaxLevel(mob) * 400 );
        if ( !mob->in_room )
            return;
        tarea = mob->in_room->area;

        gold = ((tarea->high_economy[type] > 0) ? 1 : 0) * 1000000000 + tarea->low_economy[type];
        GET_MONEY(mob,type) = URANGE( 0, GET_MONEY(mob,type), gold / 10 );
        if ( GET_MONEY(mob,type) )
            lower_economy( tarea, GET_MONEY(mob,type), type );
    }
}


/*
 * Add another notch on that there belt... ;)
 * Keep track of the last so many kills by vnum			-Thoric
 */
void add_kill( CHAR_DATA *ch, CHAR_DATA *mob )
{
    int x;
    int vnum;
    sh_int track;

    if ( IS_NPC(ch) )
    {
        bug( "add_kill: trying to add kill to npc" );
        return;
    }
    if ( !IS_NPC(mob) )
    {
        bug( "add_kill: trying to add kill non-npc" );
        return;
    }
    vnum = mob->vnum;
    track = URANGE( 2, ((GetMaxLevel(ch)+3) * MAX_KILLTRACK)/LEVEL_AVATAR, MAX_KILLTRACK );
    for ( x = 0; x < track; x++ )
        if ( ch->pcdata->killed[x].vnum == vnum )
        {
            if ( ch->pcdata->killed[x].count < 50 )
                ++ch->pcdata->killed[x].count;
            return;
        }
        else
            if ( ch->pcdata->killed[x].vnum == 0 )
                break;
    memmove( (char *) ch->pcdata->killed+sizeof(KILLED_DATA),
             ch->pcdata->killed, (track-1) * sizeof(KILLED_DATA) );
    ch->pcdata->killed[0].vnum  = vnum;
    ch->pcdata->killed[0].count = 1;
    if ( track < MAX_KILLTRACK )
        ch->pcdata->killed[track].vnum = 0;
}

/*
 * Return how many times this player has killed this mob	-Thoric
 * Only keeps track of so many (MAX_KILLTRACK), and keeps track by vnum
 */
int times_killed( CHAR_DATA *ch, CHAR_DATA *mob )
{
    int x;
    int vnum;
    sh_int track;

    if ( IS_NPC(ch) )
    {
        bug( "times_killed: ch is not a player" );
        return 0;
    }
    if ( !IS_NPC(mob) )
    {
        bug( "add_kill: mob is not a mobile" );
        return 0;
    }

    vnum = mob->vnum;
    track = URANGE( 2, ((GetMaxLevel(ch)+3) * MAX_KILLTRACK)/LEVEL_AVATAR, MAX_KILLTRACK );
    for ( x = 0; x < track; x++ )
        if ( ch->pcdata->killed[x].vnum == vnum )
            return ch->pcdata->killed[x].count;
        else
            if ( ch->pcdata->killed[x].vnum == 0 )
                break;
    return 0;
}

/*
 * returns area with name matching input string
 * Last Modified : July 21, 1997
 * Fireblade
 */
AREA_DATA *get_area(char *name)
{
    AREA_DATA *pArea;

    if(!name)
    {
        bug("get_area: NULL input string.");
        return NULL;
    }

    for(pArea = first_area; pArea; pArea = pArea->next)
    {
        if(nifty_is_name(name, pArea->name))
            break;
    }

    if(!pArea)
    {
        for(pArea = first_build; pArea; pArea = pArea->next)
        {
            if(nifty_is_name(name, pArea->name))
                break;
        }
    }

    return pArea;
}

int unum_arg(char *argument)
{
    if (argument && *argument == 'u' && is_number(argument+1))
        return atoi(argument+1);

    return -1;
}

int vnum_arg(char *argument)
{
    if (argument && *argument == 'v' && is_number(argument+1))
        return atoi(argument+1);

    return -1;
}
