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
 *			    Battle & death module			    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: fight.c,v 1.103 2004/04/06 22:00:09 dotd Exp $";*/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "gsn.h"
#include "quest.h"

DECLARE_DO_FUN(do_shout);
DECLARE_DO_FUN(do_bash);
DECLARE_DO_FUN(do_stun);
DECLARE_DO_FUN(do_gouge);
DECLARE_DO_FUN(do_bite);
DECLARE_DO_FUN(do_claw);
DECLARE_DO_FUN(do_tail);
DECLARE_DO_FUN(do_sting);
DECLARE_DO_FUN(do_punch);
DECLARE_DO_FUN(do_kick);
DECLARE_DO_FUN(do_get);
DECLARE_DO_FUN(do_split);
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_sacrifice);
DECLARE_DO_FUN(do_recall);
DECLARE_DO_FUN(do_flee);
DECLARE_DO_FUN(do_return);
DECLARE_DO_FUN(do_revert);
DECLARE_DO_FUN(do_help);
DECLARE_DO_FUN(do_variables);

DECLARE_SPELL_FUN(spell_energy_drain);
DECLARE_SPELL_FUN(spell_fire_breath);
DECLARE_SPELL_FUN(spell_frost_breath);
DECLARE_SPELL_FUN(spell_acid_breath);
DECLARE_SPELL_FUN(spell_gas_breath);
DECLARE_SPELL_FUN(spell_lightning_breath);
DECLARE_SPELL_FUN(spell_blindness);
DECLARE_SPELL_FUN(spell_cause_serious);
DECLARE_SPELL_FUN(spell_curse);
DECLARE_SPELL_FUN(spell_flamestrike);
DECLARE_SPELL_FUN(spell_harm);
DECLARE_SPELL_FUN(spell_fireball);
DECLARE_SPELL_FUN(spell_colour_spray);
DECLARE_SPELL_FUN(spell_weaken);
DECLARE_SPELL_FUN(spell_dispel_magic);
DECLARE_SPELL_FUN(spell_dispel_evil);
DECLARE_SPELL_FUN(spell_poison);
DECLARE_SPELL_FUN(spell_earthquake);

extern char		lastplayercmd[MAX_INPUT_LENGTH*2];
extern CHAR_DATA *	gch_prev;
extern const sh_int backstab_mult[MAX_LEVEL+1];
extern const int thaco[REAL_MAX_CLASS][111];

/*
 * Local functions.
 */
static void	dam_message	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
                                        int dt ) );
static void	group_gain	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
static int	align_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim ) );
static void	align_zap	args( ( CHAR_DATA *ch ) );
static int	obj_hitroll	args( ( OBJ_DATA *obj ) );
int             berserkdambonus args( ( CHAR_DATA *ch, int dam ) );
int             BarbarianToHitMagicBonus args( ( CHAR_DATA *ch ) );
bool     life_protection_object args( ( CHAR_DATA *ch ) );
bool	dual_flip = FALSE;


/*
 * Check to see if weapon is poisoned.
 */
static bool is_wielding_poisoned( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) 	)
         &&   (IS_SET( obj->extra_flags, ITEM_POISONED) ) )
        return TRUE;

    return FALSE;

}

/*
 * hunting, hating and fearing code				-Thoric
 */
bool is_hunting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->hunting || ch->hunting->who != victim )
        return FALSE;

    return TRUE;
}

bool is_hating( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->hating || ch->hating->who != victim )
        return FALSE;

    return TRUE;
}

bool is_fearing( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->fearing || ch->fearing->who != victim )
        return FALSE;

    return TRUE;
}

void stop_hunting( CHAR_DATA *ch )
{
    if ( ch->hunting )
    {
        STRFREE( ch->hunting->name );
        DISPOSE( ch->hunting );
        ch->hunting = NULL;
    }
    return;
}

void stop_hating( CHAR_DATA *ch )
{
    if ( ch->hating )
    {
        STRFREE( ch->hating->name );
        DISPOSE( ch->hating );
        ch->hating = NULL;
    }
    return;
}

void stop_fearing( CHAR_DATA *ch )
{
    if ( ch->fearing )
    {
        STRFREE( ch->fearing->name );
        DISPOSE( ch->fearing );
        ch->fearing = NULL;
    }
    return;
}

void start_hunting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->hunting )
        stop_hunting( ch );

    CREATE( ch->hunting, HHF_DATA, 1 );
    ch->hunting->name = QUICKLINK( victim->name );
    ch->hunting->who  = victim;
    ch->hunting->start_time = current_time;
    return;
}

void start_hating( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->hating )
        stop_hating( ch );

    CREATE( ch->hating, HHF_DATA, 1 );
    ch->hating->name = QUICKLINK( victim->name );
    ch->hating->who  = victim;
    ch->hating->start_time = current_time;
    return;
}

void start_fearing( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->fearing )
        stop_fearing( ch );

    CREATE( ch->fearing, HHF_DATA, 1 );
    ch->fearing->name = QUICKLINK( victim->name );
    ch->fearing->who  = victim;
    ch->fearing->start_time = current_time;
    return;
}

int get_dam_by_race( CHAR_DATA *ch )
{
    int num;

    num = number_percent();
    switch(GET_RACE(ch)) {
    case RACE_REPTILE:
        if (num <= 50) {
            return(DAM_CLAW);
        } else {
            return(DAM_BITE);
        }
        break;
    case RACE_LYCANTH:
    case RACE_DRAGON:
    case RACE_DRAGON_RED    :
    case RACE_DRAGON_BLACK  :
    case RACE_DRAGON_GREEN  :
    case RACE_DRAGON_WHITE  :
    case RACE_DRAGON_BLUE   :
    case RACE_DRAGON_SILVER :
    case RACE_DRAGON_GOLD   :
    case RACE_DRAGON_BRONZE :
    case RACE_DRAGON_COPPER :
    case RACE_DRAGON_BRASS  :
    case RACE_PREDATOR:
    case RACE_LABRAT:
        if (num <= 33) {
            return(DAM_BITE);
        } else {
            return(DAM_CLAW);
        }
        break;
    case RACE_INSECT:
        if (num <= 50) {
            return(DAM_BITE);
        } else {
            return(DAM_STING);
        }
        break;
    case RACE_ARACHNID:
    case RACE_DINOSAUR:
    case RACE_FISH:
    case RACE_SNAKE:
        return(DAM_BITE);
        break;
    case RACE_BIRD:
    case RACE_SKEXIE:
        return(DAM_CLAW);
        break;
    case RACE_GIANT:
    case RACE_GIANT_HILL   :
    case RACE_GIANT_FROST  :
    case RACE_GIANT_FIRE   :
    case RACE_GIANT_CLOUD  :
    case RACE_GIANT_STORM  :
    case RACE_GIANT_STONE  :
    case RACE_GOLEM:
        return(DAM_POUND);
        break;
    case RACE_DEMON:
    case RACE_DEVIL:
    case RACE_TROLL:
    case RACE_TROGMAN:
    case RACE_LIZARDMAN:
        return(DAM_CLAW);
        break;
    case RACE_TREE:
        return(DAM_SMASH);
        break;
    case RACE_MFLAYER:
        if (num <= 60) {
            return(DAM_WHIP);
        } else if (num < 80) {
            return(DAM_BITE);
        } else {
            return(DAM_HIT);
        }
        break;
    case RACE_PRIMATE:
        if (num <= 70) {
            return(DAM_HIT);
        } else {
            return(DAM_BITE);
        }
        break;
    case RACE_TYTAN:
        return(DAM_BLAST);
        break;
    default:
        return(DAM_HIT);
    }


}

/*
 * Get the current armor class for a vampire based on time of day
 */
sh_int VAMP_AC( CHAR_DATA * ch )
{
    if ( IS_VAMPIRE( ch ) || IS_OUTSIDE( ch ) )
    {
        switch(time_info.sunlight)
        {
        case SUN_DARK:
            return -8;
        case SUN_RISE:
            return 5;
        case SUN_LIGHT:
            return 10;
        case SUN_SET:
            return 2;
        default:
            return 0;
        }
    }
    else
        return 0;
}

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 * Many hours spent fixing bugs in here by Thoric, as noted by residual
 * debugging checks.  If you never get any of these error messages again
 * in your logs... then you can comment out some of the checks without
 * worry.
 */
void violence_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *lst_ch;
    CHAR_DATA *victim;
    CHAR_DATA *rch, *rch_next;
    TIMER	*timer, *timer_next;
    ch_ret     retcode;
    int	       x, attacktype = 0, cnt;

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
	    bug( "FATAL: violence_update: %s->prev->next doesn't point to ch.", GET_NAME(ch) );
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
            log_string_plus( "violence_update: bad ch record!  (Shortcutting.)", LOG_NORMAL, LEVEL_LOG_CSET, SEV_ALERT );
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

        for ( timer = ch->first_timer; timer; timer = timer_next )
        {
            timer_next = timer->next;
            if ( --timer->count <= 0 )
            {
                if ( timer->type == TIMER_DO_FUN )
                {
                    int tempsub;

                    tempsub = ch->substate;
                    ch->substate = timer->value;
                    (timer->do_fun)( ch, "" );
                    if ( char_died(ch) )
                        break;
                    ch->substate = tempsub;
                }
                if ( timer->count <= 0 )
                    extract_timer( ch, timer );
            }
        }

        if ( char_died(ch) )
            continue;

        /* Spell wearing-off used to be here -Jesse 8-22-97 */

        if ( ( victim = who_fighting( ch ) ) == NULL
             ||   IS_AFFECTED( ch, AFF_PARALYSIS ) )
            continue;

        retcode = rNONE;

        if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE ) )
        {
            sprintf( log_buf, "violence_update: %s fighting %s in a SAFE room.",
                     ch->name, victim->name );
            log_string_plus( log_buf, LOG_BUG, LEVEL_LOG_CSET, SEV_ERR );
            stop_fighting( ch, TRUE );
            continue;
        }

        if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
            retcode = multi_hit( ch, victim, TYPE_UNDEFINED );
        else
            stop_fighting( ch, FALSE );

        if ( retcode == rCHAR_DIED || char_died(ch) )
            continue;

        if ( retcode == rVICT_DIED
             || ( victim = who_fighting( ch ) ) == NULL )
            continue;

        /*
         * Violence triggered specs
         */
        if ( ch->spec_fun && sysdata.specials_enabled==TRUE )
        {
            (*ch->spec_fun) ( ch, NULL, "", victim, SFT_VIO_UPDATE );

            if ( char_died(ch) )
                continue;
        }

        /*
         *  Mob triggers
         */
        rprog_rfight_trigger( ch );
        if ( char_died(ch) )
            continue;
        mprog_hitprcnt_trigger( ch, victim );
        if ( char_died(ch) )
            continue;
        mprog_fight_trigger( ch, victim );
        if ( char_died(ch) )
            continue;
        if ( get_eq_char(ch, WEAR_WIELD) )
        {
            oprog_fight_trigger( ch, get_eq_char(ch, WEAR_WIELD), victim );
            if ( char_died(ch) )
                continue;
        }
        if ( get_eq_char(ch, WEAR_DUAL_WIELD) )
        {
            oprog_fight_trigger( ch, get_eq_char(ch, WEAR_DUAL_WIELD), victim );
            if ( char_died(ch) )
                continue;
        }

        /*
         * NPC special attack flags				-Thoric
         */
        if ( IS_NPC(ch) )
        {
            cnt = 0;
            if ( ch->attacks )
                for ( ;; )
                {
                    if ( cnt++ > 10 )
                    {
                        attacktype = 0;
                        break;
                    }
                    x = number_range( 7, 31 );
                    attacktype = 1 << x;
                    if ( IS_SET( ch->attacks, attacktype ) )
                        break;
                }

            if ( 30 + (GetAveLevel(ch)/4) < number_percent( ) )
                attacktype = 0;
            switch( attacktype )
            {
            case ATCK_BASH:
                do_bash( ch, "" );
                retcode = global_retcode;
                break;
            case ATCK_STUN:
                do_stun( ch, "" );
                retcode = global_retcode;
                break;
            case ATCK_GOUGE:
                do_gouge( ch, "" );
                retcode = global_retcode;
                break;
            case ATCK_FEED:
                do_gouge( ch, "" );
                retcode = global_retcode;
                break;
            case ATCK_DRAIN:
                retcode = spell_energy_drain( gsn_energy_drain,
                                              BestSkLv(ch, gsn_energy_drain), ch, victim );
                break;
            case ATCK_FIREBREATH:
                retcode = spell_fire_breath( skill_lookup( "breath of fire" ),
                                             BestSkLv(ch, skill_lookup("breath of fire")), ch, victim );
                break;
            case ATCK_FROSTBREATH:
                retcode = spell_frost_breath( skill_lookup( "breath of frost" ),
                                              BestSkLv(ch, skill_lookup("breath of frost")), ch, victim );
                break;
            case ATCK_ACIDBREATH:
                retcode = spell_acid_breath( skill_lookup( "breath of acid" ),
                                             BestSkLv(ch, skill_lookup("breath of acid")), ch, victim );
                break;
            case ATCK_LIGHTNBREATH:
                retcode = spell_lightning_breath( skill_lookup( "breath of lightning" ),
                                                  BestSkLv(ch, skill_lookup("breath of lightning")), ch, victim );
                break;
            case ATCK_GASBREATH:
                retcode = spell_gas_breath( skill_lookup( "breath of gas" ),
                                            BestSkLv(ch, skill_lookup("breath of gas")), ch, victim );
                break;
            case ATCK_SPIRALBLAST:
                break;
            case ATCK_POISON:
                retcode = spell_poison( gsn_poison, BestSkLv(ch, gsn_poison), ch, victim );
                break;
            case ATCK_NASTYPOISON:
                break;
            case ATCK_GAZE:
                break;
            case ATCK_BLINDNESS:
                retcode = spell_blindness( gsn_blindness,
                                           BestSkLv(ch, gsn_blindness), ch, victim );
                break;
            case ATCK_CAUSESERIOUS:
                /*
                 retcode = spell_cause_serious( gsn_cause_serious,
                 BestSkLv(ch, gsn_cause_serious), ch, victim );
                 */
                break;
            case ATCK_EARTHQUAKE:
                retcode = spell_earthquake( gsn_earthquake,
                                            BestSkLv(ch, gsn_earthquake), ch, victim );
                break;
            case ATCK_CAUSECRITICAL:
                /*
                 retcode = spell_cause_critical( gsn_cause_critical,
                 BestSkLv(ch, gsn_cause_critical), ch, victim );
                 */
                break;
            case ATCK_CURSE:
                retcode = spell_curse( gsn_curse,
                                       BestSkLv(ch, gsn_curse), ch, victim );
                break;
            case ATCK_FLAMESTRIKE:
                retcode = spell_flamestrike( gsn_flamestrike,
                                             BestSkLv(ch, gsn_flamestrike), ch, victim );
                break;
            case ATCK_HARM:
                retcode = spell_harm( gsn_harm,
                                      BestSkLv(ch, gsn_harm), ch, victim );
                break;
            case ATCK_FIREBALL:
                retcode = spell_fireball( gsn_fireball,
                                          BestSkLv(ch, gsn_fireball), ch, victim );
                break;
            case ATCK_COLORSPRAY:
                retcode = spell_colour_spray( gsn_colour_spray,
                                              BestSkLv(ch, gsn_colour_spray), ch, victim );
                break;
            case ATCK_WEAKEN:
                retcode = spell_weaken( gsn_weaken,
                                        BestSkLv(ch, gsn_weaken), ch, victim );
                break;
            }
            if ( retcode == rCHAR_DIED || (char_died(ch)) )
                continue;

            /*
             * NPC special defense flags				-Thoric
             */
            cnt = 0;
            if ( ch->defenses )
                for ( ;; )
                {
                    if ( cnt++ > 10 )
                    {
                        attacktype = 0;
                        break;
                    }
                    x = number_range( 2, 18 );
                    attacktype = 1 << x;
                    if ( IS_SET( ch->defenses, attacktype ) )
                        break;
                }
            if ( 50 + (GetMaxLevel(ch)/4) < number_percent( ) )
                attacktype = 0;
            switch( attacktype )
            {
            case DFND_CURELIGHT:
                act( AT_MAGIC, "$n mutters a few incantations...and looks a little better.", ch, NULL, NULL, TO_ROOM );
                retcode = spell_smaug( gsn_cure_light,
                                       BestSkLv(ch, gsn_cure_light), ch, ch );
                break;
            case DFND_CURESERIOUS:
                act( AT_MAGIC, "$n mutters a few incantations...and looks a bit better.", ch, NULL, NULL, TO_ROOM );
                retcode = spell_smaug( gsn_cure_serious,
                                       BestSkLv(ch, gsn_cure_serious), ch, ch );
                break;
            case DFND_CURECRITICAL:
                act( AT_MAGIC, "$n mutters a few incantations...and looks a bit healthier.", ch, NULL, NULL, TO_ROOM );
                retcode = spell_smaug( gsn_cure_critical,
                                       BestSkLv(ch, gsn_cure_critical), ch, ch );
                break;
            case DFND_DISPELMAGIC:
                act( AT_MAGIC, "$n mutters a few incantations...and waves $s arms about.", ch, NULL, NULL, TO_ROOM );
                retcode = spell_dispel_magic( gsn_dispel_magic,
                                              BestSkLv(ch, gsn_dispel_magic), ch, victim );
                break;
            case DFND_DISPELEVIL:
                act( AT_MAGIC, "$n mutters a few incantations...and waves $s arms about.", ch, NULL, NULL, TO_ROOM );
                retcode = spell_dispel_evil( gsn_dispel_evil,
                                             BestSkLv(ch, gsn_dispel_evil), ch, victim );
                break;
            case DFND_SANCTUARY:
                if ( !IS_AFFECTED(victim, AFF_SANCTUARY) )
                {
                    act( AT_MAGIC, "$n mutters a few incantations...", ch, NULL, NULL, TO_ROOM );
                    retcode = spell_smaug( gsn_sanctuary,
                                           BestSkLv(ch, gsn_sanctuary), ch, ch );
                }
                else
                    retcode = rNONE;
                break;
            }
            if ( retcode == rCHAR_DIED || (char_died(ch)) )
                continue;
        }
        /*
         * Fun for the whole family!
         */
        for ( rch = ch->in_room->first_person; rch; rch = rch_next )
        {
            rch_next = rch->next_in_room;

            if ( rch->fighting || rch == victim || !IS_AWAKE(rch) )
                continue;

            /*
             * PC's auto-assist others in their group.
             */
            if ( !IS_NPC(ch) )
            {
                if ( IS_AFFECTED(rch, AFF_CHARM) &&
                     rch->master != ch )
                    continue;

                /* PC's check for group, NPC's only check for master */
                if ( !IS_NPC(rch) )
                {
                    if ( !IS_PLR2_FLAG(rch, PLR2_AUTOASSIST) )
                        continue;

                    if ( !is_same_group(rch, ch) ||
                         is_same_group(rch, victim) )
                        continue;
                }
                else
                {
                    if ( !IS_ACT_FLAG(rch, ACT_GUARDIAN) )
                        continue;
                }

                if ( !can_see(rch, victim) ||
                     !can_see(rch, ch) )
                    continue;

                multi_hit( rch, victim, TYPE_UNDEFINED );
                act(AT_PLAIN, "You assist $N!", rch, NULL, ch, TO_CHAR);

                if (char_died(victim) ||
                    char_died(ch) ||
                    char_died(rch))
                    break;

                continue;
            }

            /* PC's should never assist NPC's */
            if ( !IS_NPC(rch) )
                continue;

            /*
             * NPC's assist NPC's of same type
             */
            if ( (IS_AFFECTED(rch, AFF_CHARM) && rch->master!=ch) ||
                 IS_SET(rch->act, ACT_NOASSIST) ||
                 !can_see(rch, ch) ||
                 !can_see(rch, victim) ||
                 is_same_race_align(rch, victim) ||
                 is_same_group(rch, victim) ||
                 number_percent() > 40)
                 continue;

            if ( rch->pIndexData == ch->pIndexData ||
                 is_same_group(rch, ch) ||
                 is_same_race_align(rch, ch) ||
                 rch->master == ch )
            {
                one_hit( rch, victim, TYPE_UNDEFINED );
                if (char_died(victim) ||
                    char_died(ch) ||
                    char_died(rch))
                    break;
            }
        }
    }

    return;
}



/*
 * Do one group of attacks.
 */
ch_ret multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    int     x;
    int     percent;
    ch_ret  retcode;

    if (char_died(ch))
        return rNONE;

    /* add timer if player is attacking another player */
    if ( !IS_NPC(ch) && !IS_NPC(victim) )
        add_timer( ch, TIMER_RECENTFIGHT, 20, NULL, 0 );

    if ( !IS_NPC(ch) && IS_SET( ch->act, PLR_NICE ) && !IS_NPC( victim ) )
        return rNONE;

    if ( (retcode = one_hit( ch, victim, dt )) != rNONE )
        return retcode;
    if ( who_fighting( ch ) != victim || dt == gsn_backstab)
        return rNONE;

    /* Very high percent of hitting compared to percent of going berserk */
    /* 40% or higher is always hit.. don't learn anything here though. */
    /* -- Altrag */
    percent = IS_NPC(ch) ? 100 : (LEARNED(ch, gsn_berserk)*5/2);
    if ( IS_AFFECTED(ch, AFF_BERSERK) && number_percent() < percent )
        if ( (retcode = one_hit( ch, victim, dt )) != rNONE ||
             who_fighting( ch ) != victim )
            return retcode;

    x=get_numattacks(ch);

    x-=1000;

    while (x>=1000)
    {
        retcode = one_hit( ch, victim, dt );
        if ( retcode != rNONE || who_fighting( ch ) != victim )
            return retcode;
        x-=1000;
    }

    if (x>0)
        if ((10*number_percent())<x)
        {
            retcode = one_hit( ch, victim, dt );
            if ( retcode != rNONE || who_fighting( ch ) != victim )
                return retcode;
        }

    if ( get_eq_char( ch, WEAR_DUAL_WIELD ) )
    {
        percent = IS_NPC(ch) ? GetMaxLevel(ch) : LEARNED(ch, gsn_dual_wield);
        if ( number_percent( ) < percent )
        {
            if ( !get_eq_char( ch, WEAR_WIELD ) )
            {
                bug("!WEAR_WIELD in multi_hit in fight.c");
                return rNONE;
            }
            dual_flip = TRUE;
            retcode = one_hit( ch, victim, dt );
            if ( retcode != rNONE || who_fighting( ch ) != victim )
                return retcode;
        }
        else
            learn_from_failure( ch, gsn_dual_wield );
    }

    retcode = rNONE;

    percent = IS_NPC(ch) ? (int) (GetMaxLevel(ch) / 2) : 0;
    if ( number_percent( ) < percent )
        retcode = one_hit( ch, victim, dt );

    if ( retcode == rNONE && !char_died(ch) )
    {
        int move;

        if ( !IS_AFFECTED(ch, AFF_FLYING)
             &&   !IS_AFFECTED(ch, AFF_FLOATING) )
            move = encumbrance( ch, movement_loss[URANGE(0, ch->in_room->sector_type, SECT_MAX-1)] );
        else
            move = encumbrance( ch, 1 );
        if ( GET_MOVE(ch) )
            GET_MOVE(ch) = UMAX( 0, GET_MOVE(ch) - move );
    }

    return retcode;
}


/*
 * Weapon types, haus
 */
static int weapon_prof_bonus_check( CHAR_DATA *ch, OBJ_DATA *wield, int *gsn_ptr )
{
    int bonus;

    bonus = 0;	*gsn_ptr = -1;
    if ( !IS_NPC(ch) && GET_LEVEL(ch, BestFightingClass(ch)) > 5 && wield)
    {
        switch(wield->value[3])
        {
        default:	*gsn_ptr = -1;			break;
        case 0:
        case 12:
        case 10:
        case 6:
        case 3:
        case 1:
        case 11:
        case 2:
        case 4:
        case 5:
        case 7:
        case 8:	break;

        }
        if ( *gsn_ptr != -1 )
            bonus = (int) ((LEARNED(ch, *gsn_ptr) -50)/10);

        /* Reduce weapon bonuses for misaligned clannies.
         if ( IS_CLANNED(ch) )
         {
         bonus = bonus /
         ( 1 + abs( GET_ALIGN(ch) - ch->pcdata->clan->alignment ) / 1000 );
         }*/

        if ( IS_DEVOTED( ch ) )
        {
            bonus = bonus - abs( ch->pcdata->favor ) / -100 ;
        }

    }
    return bonus;
}

/*
 * Calculate the tohit bonus on the object and return RIS values.
 * -- Altrag
 */
static int obj_hitroll( OBJ_DATA *obj )
{
    int tohit = 0;
    AFFECT_DATA *paf;

    /*
    for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
        if ( paf->location == APPLY_HITROLL )
            tohit += paf->modifier;
    */
    for ( paf = obj->first_affect; paf; paf = paf->next )
        if ( paf->location == APPLY_HITROLL ||
             paf->location == APPLY_HITNDAM )
            tohit += paf->modifier;
    return tohit;
}

/*
 * Offensive shield level modifier
 */
/*
 static sh_int off_shld_lvl( CHAR_DATA *ch, CHAR_DATA *victim )
 {
 sh_int lvl;

 if ( !IS_NPC(ch) )
 {
 lvl = UMAX( 1, (GetMaxLevel(ch) - 10) / 2 );
 if ( number_percent() + (GetMaxLevel(victim) - lvl) < 35 )
 return lvl;
 else
 return 0;
 }
 else
 {
 lvl = GetMaxLevel(ch) / 2;
 if ( number_percent() + (GetMaxLevel(victim) - lvl) < 70 )
 return lvl;
 else
 return 0;
 }
 }
 */

/*
 * Hit one guy once.
 */

int one_hit_affects(CHAR_DATA *ch, CHAR_DATA *victim, AFFECT_DATA *first_affect, int dam)
{
    AFFECT_DATA *aff;

    for ( aff = first_affect; aff; aff = aff->next )
    {
        if ( aff->location == APPLY_RACE_SLAYER )
        {
            if ( (aff->modifier == GET_RACE(victim)) ||
                 ((aff->modifier == RACE_UNDEAD) && IsUndead(victim)) ||
                 ((aff->modifier == RACE_DRAGON) && IsDragon(victim)) ||
                 ((aff->modifier == RACE_GIANT) && IsGiantish(victim)) )
            {
                sprintf(log_buf, "%s race slaying %s for %d*2 damage.",
                        GET_NAME(ch), GET_NAME(victim), dam);
                log_string_plus(log_buf,LOG_MONITOR,LEVEL_IMMORTAL,SEV_SPAM+1);
                dam *= 2;
            }
        }
        else if ( aff->location == APPLY_ALIGN_SLAYER )
        {
            switch(aff->modifier)
            {
            case 0: if (IS_EVIL(victim))    dam *= 2; break;
            case 1: if (IS_NEUTRAL(victim)) dam *= 2; break;
            case 2: if (IS_GOOD(victim))    dam *= 2; break;
            default: break;
            }
            sprintf(log_buf, "%s(%d) possibly align slaying %s(%d) for %d*2 damage, align mod %d.",
                    GET_NAME(ch), GET_ALIGN(ch),
                    GET_NAME(victim), GET_ALIGN(victim), dam, aff->modifier);
            log_string_plus(log_buf,LOG_MONITOR,LEVEL_IMMORTAL,SEV_SPAM+1);
        }
    }
    return dam;
}


int WeaponCheck(CHAR_DATA *ch, OBJ_DATA *wield, CHAR_DATA *victim, int dt, int dam)
{
    int plusris = 0, x;

    if ( wield )
    {
        if ( IS_SET( wield->extra_flags, ITEM_MAGIC ) )
            dam = ris_damage( victim, dam, RIS_MAGIC );
        else
            dam = ris_damage( victim, dam, RIS_NONMAGIC );

        /*
         * Handle PLUS1 - PLUS6 ris bits vs. weapon hitroll	-Thoric
         */
        plusris = obj_hitroll( wield );
    }
    else
    {
        OBJ_DATA *gloves;
        int ris = RIS_NONMAGIC;

        if ( IS_ACTIVE(ch, CLASS_BARBARIAN) )
            plusris = UMAX(plusris, BarbarianToHitMagicBonus( ch ));

        /* inate tohit plus based on level, mostly for monks */
        if (GetMaxLevel(ch) > 48)
            plusris = 5;
        else if (GetMaxLevel(ch) > 36)
            plusris = 4;
        else if (GetMaxLevel(ch) > 26)
            plusris = 3;
        else if (GetMaxLevel(ch) > 18)
            plusris = 2;
        else if (GetMaxLevel(ch) > 12)
            plusris = 1;

        /* magic if wearing magic gloves, mostly for monks */
        if ( (gloves = get_eq_char( ch, WEAR_HANDS )) != NULL )
            if ( IS_OBJ_STAT(gloves, ITEM_MAGIC) )
                ris = RIS_MAGIC;

        dam = ris_damage( victim, dam, ris );
    }

    /* check for RIS_PLUSx 					-Thoric */
    if ( dam > 0 )
    {
        int res, abs, imm, sus, mod;

        if ( plusris )
            plusris = RIS_PLUS1 << UMIN(plusris, 7);

        /* initialize values to handle a zero plusris */
        abs = imm = res = -1;  sus = 1;

        /* find high ris */
        for ( x = RIS_PLUS1; x <= RIS_PLUS6; x <<= 1 )
        {
            if ( IS_SET( victim->absorb, x ) )
                abs = x;
            if ( IS_SET( victim->immune, x ) )
                imm = x;
            if ( IS_SET( victim->resistant, x ) )
                res = x;
            if ( IS_SET( victim->susceptible, x ) )
                sus = x;
        }
        mod = 10;
        if ( abs >= plusris )
            mod -= 110;
        if ( imm >= plusris )
            mod -= 10;
        if ( res >= plusris )
            mod -= 2;
        if ( sus <= plusris )
            mod += 2;

        /* check if immune */
        if ( mod <= -100 )
            dam = dam * -1;
        else
        {
            if ( mod <= 0 )
                dam = -1;
            if ( mod != 10 )
                dam = (dam * mod) / 10;
        }
    }

    return dam;
}


ch_ret one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    OBJ_DATA *wield;
    int thac0;
#if SMAUG_THACO
    int victim_ac;
    int thac0_00;
    int thac0_32;
    int i, max;
    int diceroll;
#endif
    int dam, x;
    int attacktype, cnt;
    int	prof_bonus;
    int	prof_gsn;
    ch_ret retcode = 0;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */

    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
        return rVICT_DIED;

    /*
     * Figure out the weapon doing the damage			-Thoric
     */
    if ( (wield = get_eq_char( ch, WEAR_DUAL_WIELD )) != NULL )
    {
        if ( dual_flip == TRUE )
        {
            dual_flip = FALSE;
        }
        else
            wield = get_eq_char( ch, WEAR_WIELD );
    } else
        wield = get_eq_char( ch, WEAR_WIELD );

    prof_bonus = weapon_prof_bonus_check( ch, wield, &prof_gsn );

    if ( ch->fighting		/* make sure fight is already started */
         &&   dt == TYPE_UNDEFINED
         &&   IS_NPC(ch)
         &&   ch->attacks != 0 )
    {
        cnt = 0;
        for ( ;; )
        {
            x = number_range( 0, 6 );
            attacktype = 1 << x;
            if ( IS_SET( ch->attacks, attacktype ) )
                break;
            if ( cnt++ > 16 )
            {
                attacktype = 0;
                break;
            }
        }
        if ( attacktype == ATCK_BACKSTAB )
            attacktype = 0;
        if ( wield && number_percent( ) > 25 )
            attacktype = 0;
        switch ( attacktype )
        {
        default:
            break;
        case ATCK_BITE:
            do_bite( ch, "" );
            retcode = global_retcode;
            break;
        case ATCK_CLAWS:
            do_claw( ch, "" );
            retcode = global_retcode;
            break;
        case ATCK_TAIL:
            do_tail( ch, "" );
            retcode = global_retcode;
            break;
        case ATCK_STING:
            do_sting( ch, "" );
            retcode = global_retcode;
            break;
        case ATCK_PUNCH:
            do_punch( ch, "" );
            retcode = global_retcode;
            break;
        case ATCK_KICK:
            do_kick( ch, "" );
            retcode = global_retcode;
            break;
        case ATCK_TRIP:
            attacktype = 0;
            break;
        }
        if ( attacktype )
            return retcode;
    }


    if ( dt == TYPE_UNDEFINED )
    {
        dt = TYPE_HIT;
        if ( wield && wield->item_type == ITEM_WEAPON )
        {
            dt += wield->value[3];
        } else dt += get_dam_by_race(ch);
    }

#if SMAUG_THACO
    /*
     * Calculate to-hit-armor-ch_class-0 versus armor.
     */
    for (i = 0, thac0 = 21; i < MAX_CLASS; ++i) {
        if ( IS_NPC(ch) )
        {
            thac0_00 = ch->mobthac0;
            thac0_32 =  0;
        }
        else
        {
            thac0_00 = class_table[i]->thac0_00;
            thac0_32 = class_table[i]->thac0_32;
        }

        max = interpolate(GET_LEVEL(ch, i), thac0_00, thac0_32 ) - GET_HITROLL(ch);
        thac0 = UMIN(thac0, max);
    }

    victim_ac = UMAX( -19, (int) (GET_AC(victim) / 10) );

    /* if you can't see what's coming... */
    if ( wield && !can_see_obj( victim, wield) )
        victim_ac += 1;
    if ( !can_see( ch, victim ) )
        victim_ac -= 4;
#endif

#if 0
    /*
     * "learning" between combatients.  Takes the intelligence difference,
     * and multiplies by the times killed to make up a learning bonus
     * given to whoever is more intelligent		-Thoric
     */
    if ( ch->fighting && ch->fighting->who == victim )
    {
        sh_int times = ch->fighting->timeskilled;

        if ( times )
        {
            sh_int intdiff = get_curr_int(ch) - get_curr_int(victim);

            if ( intdiff != 0 )
                victim_ac += (intdiff*times)/10;
        }
    }
#endif

#if SMAUG_THACO
    /* Weapon proficiency bonus */
    victim_ac += prof_bonus;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
        ;

    if ( diceroll == 0
         || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
#else /* dale thaco */
    thac0 = CalcThaco(ch);

    if (!HitOrMiss(ch, victim, thac0))
#endif
    {
        /* Miss. */
        if ( prof_gsn != -1 || prof_gsn == gsn_fireball )
            learn_from_failure( ch, prof_gsn );
        damage( ch, victim, 0, dt );
        tail_chain( );
        return rNONE;
    }

    /*
     * Hit.
     * Calc damage.
     */

    if ( !wield )       /* dice formula fixed by Thoric */
        dam = dice( ch->barenumdie, ch->baresizedie );
    else if ( wield->item_type == ITEM_WEAPON )
        dam = dice( wield->value[1], wield->value[2] );
    else
    {
        dam = dice( ch->barenumdie, ch->baresizedie );
        bug("one_hit: %s wielding %s", GET_NAME(ch), wield->name);
    }
    /*
     * Bonuses.
     */
    dam += GET_DAMROLL(ch);

    if ( prof_bonus )
        dam += prof_bonus / 4;

    /* Auto-poisoning is bonus enough... and no check here for immunity
     if ( wield && IS_SET( wield->extra_flags, ITEM_POISONED ) )
     dam += dam / 4;
     */

    if ( !IS_AWAKE(victim) )
        dam *= 2;
    if ( dt == gsn_backstab && CanUseSkill(ch, dt))
    {
        int mult;
        mult = backstab_mult[BestSkLv(ch, dt)];
        mult += lorebonus(ch, victim, dt);

        dam *= mult;

        log_printf_plus( LOG_DEBUG, LEVEL_LOG_CSET, SEV_DEBUG,
                         "backstab: %s vs %s  dam: %d  bs_mult: %d  mult: %d",
                         GET_NAME(ch), GET_NAME(victim),
                         dam,
                         backstab_mult[GET_LEVEL(ch, CLASS_THIEF)],
                         mult);
    }

    if (is_affected(ch, gsn_berserk) && wield)
        dam = berserkdambonus(ch,dam);

    dam = UMAX(dam, 0);

    dam = WeaponCheck(ch, wield, victim, dt, dam);

    /*
     if ( prof_gsn != -1 )
     {
     if ( dam > 0 )
     learn_from_success( ch, prof_gsn );
     else
     learn_from_failure( ch, prof_gsn );
     }
     */
    /* immune to damage */
    if ( dam == -1 )
    {
        if ( dt >= 0 && dt < top_sn )
        {
            SKILLTYPE *skill = skill_table[dt];
            bool found1 = FALSE, found2 = FALSE, found3 = FALSE;

            if ( skill->imm_char && skill->imm_char[0] != '\0' )
            {
                act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
                found1 = TRUE;
            }
            if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
            {
                act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
                found2 = TRUE;
            }
            if ( skill->imm_room && skill->imm_room[0] != '\0' )
            {
                act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT );
                found3 = TRUE;
            }
            if ( !found1 || !found2 || !found3 )
                log_printf_plus(LOG_MAGIC, LEVEL_LOG_CSET, SEV_DEBUG,
                                "spell %d is missing an immune message", dt);
            if ( found1 || found2 || found3 )
                return rNONE;
        }
        dam = 0;
    }

    if (wield && wield->first_affect)
        dam = one_hit_affects(ch, victim, wield->first_affect, dam);

    if (ch->first_affect)
        dam = one_hit_affects(ch, victim, ch->first_affect, dam);

    if ( (retcode = damage( ch, victim, dam, dt )) != rNONE )
        return retcode;
    if ( char_died(ch) )
        return rCHAR_DIED;
    if ( char_died(victim) )
        return rVICT_DIED;

    retcode = rNONE;
    if ( dam == 0 )
        return retcode;

    /* weapon spells	-Thoric */
    if ( wield
         &&  !IS_SET(victim->immune, RIS_MAGIC)
         &&  !IS_SET(victim->in_room->room_flags, ROOM_NO_MAGIC) )
    {
        AFFECT_DATA *aff;

        /*
        for ( aff = wield->pIndexData->first_affect; aff; aff = aff->next )
            if ( aff->location == APPLY_WEAPONSPELL
                 &&   IS_VALID_SN(aff->modifier)
                 &&   skill_table[aff->modifier]->spell_fun )
                retcode = (*skill_table[aff->modifier]->spell_fun) (aff->modifier, GetMaxLevel(ch)/3, ch, victim );
        if ( retcode != rNONE || char_died(ch) || char_died(victim) )
            return retcode;
        */

        for ( aff = wield->first_affect; aff; aff = aff->next )
            if ( aff->location == APPLY_WEAPONSPELL
                 &&   IS_VALID_SN(aff->modifier)
                 &&   skill_table[aff->modifier]->spell_fun )
                retcode = (*skill_table[aff->modifier]->spell_fun) (aff->modifier, GetMaxLevel(ch)/3, ch, victim );
        if ( retcode != rNONE || char_died(ch) || char_died(victim) )
            return retcode;
    }


    /*
     * magic shields that retaliate				-Thoric
     */
    /*
     * Redone in dale fashion   -Heath, 1-18-98
     */
    if ( IS_AFFECTED( victim, AFF_FIRESHIELD )
         &&  !IS_AFFECTED( ch, AFF_FIRESHIELD ) )
        retcode = damage(victim, ch, dam, gsn_fireball);
    if ( retcode != rNONE || char_died(ch) || char_died(victim) )
        return retcode;

    if ( IS_AFFECTED( victim, AFF_ICESHIELD )
         &&  !IS_AFFECTED( ch, AFF_ICESHIELD ) )
        retcode = damage(victim, ch, dam, gsn_chill_touch);
    if ( retcode != rNONE || char_died(ch) || char_died(victim) )
        return retcode;

    if ( IS_AFFECTED( victim, AFF_SHOCKSHIELD )
         &&  !IS_AFFECTED( ch, AFF_SHOCKSHIELD ) )
        retcode = damage(victim, ch, dam, gsn_lightning_bolt);
    if ( retcode != rNONE || char_died(ch) || char_died(victim) )
        return retcode;

    tail_chain( );
    return retcode;
}

/*
 * Hit one guy with a projectile.
 * Handles use of missile weapons (wield = missile weapon)
 * or thrown items/weapons
 */
ch_ret projectile_hit( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield,
                       OBJ_DATA *projectile, sh_int dist )
{
    int thac0;
#ifdef SMAUG_THACO
    int victim_ac;
    int thac0_00;
    int thac0_32;
    int diceroll;
    int i, max;
#endif
    int plusris;
    int dam;
    int	prof_bonus;
    int	prof_gsn = -1;
    int proj_bonus;
    int dt;
    ch_ret retcode;

    if ( !projectile )
        return rNONE;

    if ( projectile->item_type == ITEM_PROJECTILE
         ||   projectile->item_type == ITEM_WEAPON )
    {
        dt = TYPE_HIT + projectile->value[2];
        proj_bonus = number_range(projectile->value[1], projectile->value[3]);
    }
    else
    {
        dt = TYPE_UNDEFINED;
        proj_bonus = number_range(1, URANGE(2, get_obj_weight(projectile), 100) );
    }

    /*
     * Can't beat a dead char!
     */
    if ( victim->position == POS_DEAD || char_died(victim) )
    {
        extract_obj(projectile);
        return rVICT_DIED;
    }

    if ( wield )
        prof_bonus = weapon_prof_bonus_check( ch, wield, &prof_gsn );
    else
        prof_bonus = 0;

    if ( dt == TYPE_UNDEFINED )
    {
        dt = TYPE_HIT;
        if ( wield && wield->item_type == ITEM_MISSILE_WEAPON )
            dt += wield->value[3];
    }

#if SMAUG_THACO
    /*
     * Calculate to-hit-armor-ch_class-0 versus armor.
     */
    for (i = 0, thac0 = 21; i < MAX_CLASS; ++i) {
        if ( IS_NPC(ch) )
        {
            thac0_00 = ch->mobthac0;
            thac0_32 =  0;
        }
        else
        {
            thac0_00 = class_table[i]->thac0_00;
            thac0_32 = class_table[i]->thac0_32;
        }

        max = interpolate(GET_LEVEL(ch, i), thac0_00, thac0_32 ) - GET_HITROLL(ch);
        thac0 = UMIN(thac0, max);
    }

    victim_ac = UMAX( -19, (int) (GET_AC(victim) / 10) );

    /* if you can't see what's coming... */
    if ( !can_see_obj( victim, projectile) )
        victim_ac += 1;
    if ( !can_see( ch, victim ) )
        victim_ac -= 4;

    /* Weapon proficiency bonus */
    victim_ac += prof_bonus;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
        ;

    if ( diceroll == 0
         || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
#else /* dale thaco */
    thac0 = CalcThaco(ch);

    if (!HitOrMiss(ch, victim, thac0))
#endif
    {
        /* Miss. */
        if ( prof_gsn != -1 )
            learn_from_failure( ch, prof_gsn );

        /* Do something with the projectile */
        if ( number_percent() < 50 )
            extract_obj(projectile);
        else
        {
            if ( projectile->in_obj )
                obj_from_obj(projectile);
            if ( projectile->carried_by )
                obj_from_char(projectile);
            obj_to_room(projectile, victim->in_room);
        }
        damage( ch, victim, 0, dt );
        tail_chain( );
        return rNONE;
    }

    /*
     * Hit.
     * Calc damage.
     */

    if ( !wield )       /* dice formula fixed by Thoric */
        dam = proj_bonus;
    else
        dam = number_range(wield->value[1], wield->value[2]) + (proj_bonus / 10);

    /*
     * Bonuses.
     */
    dam += GET_DAMROLL(ch);

    if ( prof_bonus )
        dam += prof_bonus / 4;

    if ( !IS_AWAKE(victim) )
        dam *= 2;

    if ( dam <= 0 )
        dam = 1;

    plusris = 0;

    /* one_hit does this, should we too? */
/*    dam = WeaponCheck(ch, wield, victim, dam, dt);*/

    if ( IS_OBJ_STAT(projectile, ITEM_MAGIC) )
        dam = ris_damage( victim, dam, RIS_MAGIC );
    else
        dam = ris_damage( victim, dam, RIS_NONMAGIC );

    /*
     * Handle PLUS1 - PLUS6 ris bits vs. weapon hitroll	-Thoric
     */
    if ( wield )
        plusris = obj_hitroll( wield );

    /* check for RIS_PLUSx 					-Thoric */
    if ( dam )
    {
        int x, res, imm, sus, mod;

        if ( plusris )
            plusris = RIS_PLUS1 << UMIN(plusris, 7);

        /* initialize values to handle a zero plusris */
        imm = res = -1;  sus = 1;

        /* find high ris */
        for ( x = RIS_PLUS1; x <= RIS_PLUS6; x <<= 1 )
        {
            if ( IS_SET( victim->immune, x ) )
                imm = x;
            if ( IS_SET( victim->resistant, x ) )
                res = x;
            if ( IS_SET( victim->susceptible, x ) )
                sus = x;
        }
        mod = 10;
        if ( imm >= plusris )
            mod -= 10;
        if ( res >= plusris )
            mod -= 2;
        if ( sus <= plusris )
            mod += 2;

        /* check if immune */
        if ( mod <= 0 )
            dam = -1;
        if ( mod != 10 )
            dam = (dam * mod) / 10;
    }

    if ( prof_gsn != -1 )
    {
        if ( dam > 0 )
            learn_from_success( ch, prof_gsn );
        else
            learn_from_failure( ch, prof_gsn );
    }

    /* immune to damage */
    if ( dam == -1 )
    {
        if ( dt >= 0 && dt < top_sn )
        {
            SKILLTYPE *skill = skill_table[dt];
            bool found1 = FALSE, found2 = FALSE, found3 = FALSE;

            if ( skill->imm_char && skill->imm_char[0] != '\0' )
            {
                act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
                found1 = TRUE;
            }
            if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
            {
                act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
                found2 = TRUE;
            }
            if ( skill->imm_room && skill->imm_room[0] != '\0' )
            {
                act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT );
                found3 = TRUE;
            }
            if ( !found1 || !found2 || !found3 )
                log_printf_plus(LOG_MAGIC, LEVEL_LOG_CSET, SEV_DEBUG,
                                "spell %d is missing an immune message", dt);
            if ( found1 || found2 || found3 )
            {
                if ( number_percent() < 50 )
                    extract_obj(projectile);
                else
                {
                    if ( projectile->in_obj )
                        obj_from_obj(projectile);
                    if ( projectile->carried_by )
                        obj_from_char(projectile);
                    obj_to_room(projectile, victim->in_room);
                }
                return rNONE;
            }
        }
        dam = 0;
    }
    if ( (retcode = damage( ch, victim, dam, dt )) != rNONE )
    {
        extract_obj(projectile);
        return retcode;
    }
    if ( char_died(ch) )
    {
        extract_obj(projectile);
        return rCHAR_DIED;
    }
    if ( char_died(victim) )
    {
        extract_obj(projectile);
        return rVICT_DIED;
    }

    retcode = rNONE;
    if ( dam == 0 )
    {
        if ( number_percent() < 50 )
            extract_obj(projectile);
        else
        {
            if ( projectile->in_obj )
                obj_from_obj(projectile);
            if ( projectile->carried_by )
                obj_from_char(projectile);
            obj_to_room(projectile, victim->in_room);
        }
        return retcode;
    }

    /* weapon spells	-Thoric */
    if ( wield
         &&  !IS_SET(victim->immune, RIS_MAGIC)
         &&  !IS_SET(victim->in_room->room_flags, ROOM_NO_MAGIC) )
    {
        AFFECT_DATA *aff;

        /*
        for ( aff = wield->pIndexData->first_affect; aff; aff = aff->next )
            if ( aff->location == APPLY_WEAPONSPELL
                 &&   IS_VALID_SN(aff->modifier)
                 &&   skill_table[aff->modifier]->spell_fun )
                retcode = (*skill_table[aff->modifier]->spell_fun) ( aff->modifier, GetMaxLevel(ch)/3, ch, victim );
        if ( retcode != rNONE || char_died(ch) || char_died(victim) )
        {
            extract_obj(projectile);
            return retcode;
        }
        */

        for ( aff = wield->first_affect; aff; aff = aff->next )
            if ( aff->location == APPLY_WEAPONSPELL
                 &&   IS_VALID_SN(aff->modifier)
                 &&   skill_table[aff->modifier]->spell_fun )
                retcode = (*skill_table[aff->modifier]->spell_fun) ( aff->modifier, GetMaxLevel(ch)/3, ch, victim );
        if ( retcode != rNONE || char_died(ch) || char_died(victim) )
        {
            extract_obj(projectile);
            return retcode;
        }
    }

    extract_obj(projectile);

    tail_chain( );
    return retcode;
}

/*
 * Calculate damage based on resistances, immunities and suceptibilities
 *					-Thoric
 */
sh_int ris_damage( CHAR_DATA *ch, sh_int dam, int ris )
{
    if ( IS_SET(ch->absorb, ris ) )
    {
        log_printf_plus(LOG_DEBUG, LEVEL_IMMORTAL, SEV_SPAM+2,
                        "ris_damage: %s absorbed %d",
                        GET_NAME(ch), dam);
        return(dam*-1);
    }
    if ( IS_SET(ch->immune, ris ) )
    {
        log_printf_plus(LOG_DEBUG, LEVEL_IMMORTAL, SEV_SPAM+2,
                        "ris_damage: %s immune, no damage",
                        GET_NAME(ch));
        return(-1);
    }
    if ( IS_SET(ch->resistant, ris ) )
    {
        log_printf_plus(LOG_DEBUG, LEVEL_IMMORTAL, SEV_SPAM+2,
                        "ris_damage: %s resistant, half of %d",
                        GET_NAME(ch), dam);
        return(dam/2);
    }
    if ( IS_SET(ch->susceptible, ris ) )
    {
        log_printf_plus(LOG_DEBUG, LEVEL_IMMORTAL, SEV_SPAM+2,
                        "ris_damage: %s susceptible, double of %d",
                        GET_NAME(ch), dam);
        return(dam*2);
    }

    return dam;
}


/*
 * Inflict damage from a hit.
 */
ch_ret damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
    char buf1[MAX_STRING_LENGTH];
    sh_int dameq;
    sh_int maxdam;
    bool npcvict;
    bool loot;
    OBJ_DATA *damobj;
    ch_ret retcode;
    sh_int dampmod;
    CHAR_DATA *gch;

    retcode = rNONE;

    if ( !ch )
    {
        bug( "Damage: null ch!" );
        return rERROR;
    }
    if ( !victim )
    {
        bug( "Damage: null victim!" );
        return rVICT_DIED;
    }

    if ( victim->position == POS_DEAD )
        return rVICT_DIED;

    if (check_illegal_pk(ch, victim))
    {
        stop_fighting( ch, TRUE );
        return rNONE;
    }

    npcvict = IS_NPC(victim);

    if ( dam > 0 && dt != TYPE_UNDEFINED && IS_VALID_SN(dt) )
    {
        int slm, sls;

        slm = skill_table[dt]->skill_level[CLASS_MAGE];
        sls = skill_table[dt]->skill_level[CLASS_SORCERER];

        if ( (slm <= 5 || sls <= 5) &&
             is_affected(victim, gsn_minor_invulnerability) )
            dam = 0;

        if ( ((slm >= 6  && slm <= 10) ||
              (sls >= 6 && sls >= 10)) &&
             is_affected(victim, gsn_major_invulnerability) )
            dam = 0;

        if ( dam == 0 )
        {
            act(AT_MAGIC, "$N's globe deflects your $t.",
                ch, skill_table[dt]->noun_damage, victim, TO_CHAR);
            act(AT_MAGIC, "Your globe deflects $n's $t.",
                ch, skill_table[dt]->noun_damage, victim, TO_VICT);
            act(AT_MAGIC, "A globe around $N deflects $n's $t.",
                ch, skill_table[dt]->noun_damage, victim, TO_NOTVICT);
        }
    }

    /*
     * Check Align types for RIS                                 -Heath ;)
     */

    if ( IS_GOOD(ch) && IS_EVIL(victim) )
        dam = ris_damage(victim, dam, RIS_GOOD);
    else if ( IS_EVIL(ch) && IS_GOOD(victim) )
        dam = ris_damage(victim, dam, RIS_EVIL);

    /*
     * Check damage types for RIS				-Thoric
     */
    if ( dam > 0 && dt != TYPE_UNDEFINED )
    {
        if ( IS_FIRE(dt) )
            dam = ris_damage(victim, dam, RIS_FIRE);
        else if ( IS_COLD(dt) )
            dam = ris_damage(victim, dam, RIS_COLD);
        else if ( IS_ACID(dt) )
            dam = ris_damage(victim, dam, RIS_ACID);
        else if ( IS_ELECTRICITY(dt) )
            dam = ris_damage(victim, dam, RIS_ELECTRICITY);
        else if ( IS_ENERGY(dt) )
            dam = ris_damage(victim, dam, RIS_ENERGY);
        else if ( IS_DRAIN(dt) )
            dam = ris_damage(victim, dam, RIS_DRAIN);
        else if ( dt == gsn_poison || IS_POISON(dt) )
            dam = ris_damage(victim, dam, RIS_POISON);
        else if ( dt == (TYPE_HIT + 7) || dt == (TYPE_HIT + 8) )
            dam = ris_damage(victim, dam, RIS_BLUNT);
        else if ( dt == (TYPE_HIT + 2) || dt == (TYPE_HIT + 11)
                  ||   dt == (TYPE_HIT + 10) || dt == gsn_backstab)
            dam = ris_damage(victim, dam, RIS_PIERCE);
        else if ( dt == (TYPE_HIT + 1) || dt == (TYPE_HIT + 3)
                  ||   dt == (TYPE_HIT + 4) || dt == (TYPE_HIT + 5) )
            dam = ris_damage(victim, dam, RIS_SLASH);

    }

/*
    if ( dam == -1 )
    {
        if ( dt >= 0 && dt < top_sn )
        {
            bool found = FALSE;
            SKILLTYPE *skill = skill_table[dt];

            if ( skill->imm_char && skill->imm_char[0] != '\0' )
            {
                act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
                found = TRUE;
            }
            if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
            {
                act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
                found = TRUE;
            }
            if ( skill->imm_room && skill->imm_room[0] != '\0' )
            {
                act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT );
                found = TRUE;
            }
            if ( found )
                return rNONE;
        }
        dam = 0;
    }
*/

    if ( dam > 0 && npcvict && ch != victim )
    {
        if ( !IS_SET( victim->act, ACT_SENTINEL ) )
        {
            if (!is_hunting(victim, ch))
                start_hunting(victim, ch);
        }

        if (!is_hating(victim, ch))
            start_hating(victim, ch);
    }
    /*
     * Stop up any residual loopholes.
     */

    maxdam = BestSkLv(ch, dt) * 30;
    if ( dt == gsn_backstab )
        maxdam = BestSkLv(ch, dt) * 60;
    if ( dam > maxdam )
    {
        log_printf_plus(LOG_MONITOR, LEVEL_IMMORTAL, SEV_INFO,
                        "damage: %d more than %d points - %s (lvl %d) -> %s",
                        dam, maxdam,
                        GET_NAME(ch),
                        GetMaxLevel(ch),
                        GET_NAME(victim));

        dam = maxdam;
    }

    if ( victim != ch )
    {
        /*
         * Certain attacks are forbidden.
         * Most other attacks are returned.
         */
        if ( is_safe( ch, victim ) )
            return rNONE;
        check_attacker( ch, victim );


        if ( victim->position > POS_STUNNED )
        {
            if ( !victim->fighting && victim->in_room == ch->in_room )
                set_fighting( victim, ch );
            if ( victim->fighting )
              if ( victim->position != POS_SITTING )
                victim->position = POS_FIGHTING;
        }

        if ( victim->position > POS_STUNNED )
        {
            if ( !ch->fighting && victim->in_room == ch->in_room )
                set_fighting( ch, victim );

            /*
             * If victim is charmed, ch might attack victim's master.
             */
            if ( IS_NPC(ch)
                 &&   npcvict
                 &&   IS_AFFECTED(victim, AFF_CHARM)
                 &&   victim->master
                 &&   victim->master->in_room == ch->in_room
                 &&   number_bits( 3 ) == 0 )
            {
                stop_fighting( ch, FALSE );
                retcode = multi_hit( ch, victim->master, TYPE_UNDEFINED );
                return retcode;
            }
        }


        /*
         * More charm stuff.
         */
        if ( victim->master == ch )
            stop_follower( victim );

        /* Pkill stuff.  If a deadly attacks another deadly or is attacked by one,
         * then ungroup any nondealies.  Disabled untill I can figure out the right
         * way to do it.
         */

        /*	{
         sh_int anopc = 0;  * # of (non-pkill) pc in a (ch) *
         sh_int bnopc = 0;  * # of (non-pkill) pc in b (victim) *
         CHAR_DATA *lch;   * leader ch *

         * count the # of non-pkill pc in a ( not including == ch ) *
         for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
         if ( is_same_group( ch, gch ) && !IS_NPC( gch )
         && !IS_PKILL( gch ) && ( ch != gch ) ) anopc++;

         * count the # of non-pkill pc in b ( not including == victim ) *
         for ( gch = victim->in_room->first_person; gch; gch = gch->next_in_room )
         if ( is_same_group( victim, gch ) && !IS_NPC( gch )
         && !IS_PKILL( gch ) && ( victim != gch ) ) bnopc++;


         * only consider disbanding if both groups have 1(+) non-pk pc *
         if ( ( bnopc > 0 ) && ( anopc > 0 ) )
         {
         * look at group a through ch's leader first *
         lch = ch->leader ? ch->leader : ch;
         if ( lch != ch )
         {
         * stop following leader if it isn't pk *
         if ( !IS_NPC(lch) && !IS_PKILL( lch ) )
         stop_follower( ch );
         else
         {
         * disband non-pk members from lch's group if it is pk *
         for ( gch = ch->in_room->first_person; gch;
         gch = gch->next_in_room )
         {
         if ( is_same_group( lch, gch )
         && ( lch != gch )
         && !IS_NPC(gch) && !IS_PKILL( gch ) )
         stop_follower( gch );
         }
         }
         }
         else
         for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
         {
         * ch is leader - disband non-pks from group *
         if ( is_same_group( ch, gch )
         && ( ch != gch )
         && ( !IS_PKILL( gch ) && !IS_NPC( gch ) ) )
         stop_follower( gch );
         }

         * time to look at the victims group through its leader *
         lch = victim->leader ? victim->leader : victim;

         if ( lch != victim )
         {
         * if leader isn't deadly, stop following lch *
         if ( !IS_PKILL( lch ) && !IS_NPC( lch ) )
         stop_follower( victim );
         else
         {
         * lch is pk, disband non-pk's from group *
         for ( gch = victim->in_room->first_person; gch;
         gch = gch->next_in_room )
         {
         if ( is_same_group( lch, gch )
         && ( lch != gch )
         && ( !IS_PKILL( gch ) && !IS_NPC( gch ) ) )
         stop_follower( gch );
         }
         }
         }
         else
         {
         * victim is leader of group - disband non-pks *
         for ( gch = victim->in_room->first_person; gch;
         gch = gch->next_in_room )
         {
         if ( is_same_group( victim, gch )
         && ( victim != gch )
         && !IS_PKILL( gch ) && !IS_NPC( gch ) )
         stop_follower( gch );
         }
         }
         }
         }*/

        {
            sh_int anopc = 0;  /* # of (non-pkill) pc in a (ch) */
            sh_int bnopc = 0;  /* # of (non-pkill) pc in b (victim) */

            /* count the # of non-pkill pc in a ( not including == ch ) */
            for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
                if ( is_same_group( ch, gch ) && !IS_NPC( gch )
                     && !IS_PKILL( gch ) && ( ch != gch ) ) anopc++;

            /* count the # of non-pkill pc in b ( not including == victim ) */
            for ( gch = victim->in_room->first_person; gch; gch = gch->next_in_room )
                if ( is_same_group( victim, gch ) && !IS_NPC( gch )
                     && !IS_PKILL( gch ) && ( victim != gch ) ) bnopc++;


            /* only consider disbanding if both groups have 1(+) non-pk pc,
             or when one participant is pc, and the other group has 1(+)
             pk pc's (in the case that participant is only pk pc in group) */
            if ( ( bnopc > 0 && anopc > 0 )
                 || ( bnopc > 0 && !IS_NPC(ch) )
                 || ( anopc > 0 && !IS_NPC(victim) ) )
            {
                /* Disband from same group first */
                if ( is_same_group(ch, victim) )
                {
                    /* Messages to char and master handled in stop_follower */
                    act( AT_ACTION, "$n disbands from $N's group.",
                         (ch->leader == victim) ? victim : ch, NULL,
                         (ch->leader == victim) ? victim->master : ch->master,
                         TO_NOTVICT );
                    if ( ch->leader == victim )
                        stop_follower(victim);
                    else
                        stop_follower(ch);
                }
                /* if leader isnt pkill, leave the group and disband ch */
                if ( ch->leader != NULL && !IS_NPC(ch->leader) &&
                     !IS_PKILL(ch->leader) )
                {
                    act( AT_ACTION, "$n disbands from $N's group.", ch, NULL,
                         ch->master, TO_NOTVICT );
                    stop_follower( ch );
                }
                else
                {
                    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
                        if ( is_same_group(gch, ch) && !IS_NPC( gch ) &&
                             !IS_PKILL( gch ) && gch != ch )
                        {
                            act( AT_ACTION, "$n disbands from $N's group.", ch, NULL,
                                 gch->master, TO_NOTVICT );
                            stop_follower( gch );
                        }
                }
                /* if leader isnt pkill, leave the group and disband victim */
                if ( victim->leader != NULL && !IS_NPC(victim->leader) &&
                     !IS_PKILL(victim->leader) )
                {
                    act( AT_ACTION, "$n disbands from $N's group.", victim, NULL,
                         victim->master, TO_NOTVICT );
                    stop_follower( victim );
                }
                else
                {
                    for ( gch = victim->in_room->first_person; gch; gch = gch->next_in_room )
                        if ( is_same_group(gch, victim) && !IS_NPC( gch ) &&
                             !IS_PKILL( gch ) && gch != victim )
                        {
                            act( AT_ACTION, "$n disbands from $N's group.", gch, NULL,
                                 gch->master, TO_NOTVICT );
                            stop_follower( gch );
                        }
                }
            }
        }

        /*         for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
         {
         if ( is_same_group( ch, gch )
         && ( IS_PKILL( ch ) != IS_PKILL( gch ) ) )
         {
         stop_follower( ch );
         stop_follower( gch );
         }
         }

         for ( gch = victim->in_room->first_person; gch; gch = gch->next_in_room )
         {
         if ( is_same_group( victim, gch )
         && ( IS_PKILL( victim ) != IS_PKILL( gch ) ) )
         {
         stop_follower( victim );
         stop_follower( gch );
         }
         }
         */
        /*
         * Inviso attacks ... not.
         */

        if ( IS_AFFECTED(ch, AFF_INVISIBLE) )
        {
            affect_strip( ch, gsn_invis );
            affect_strip( ch, gsn_group_invis );
            REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
            act( AT_MAGIC, "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
        }

        /* Take away Hide */
        if ( IS_AFFECTED(ch, AFF_HIDE) )
            REMOVE_BIT(ch->affected_by, AFF_HIDE);

        /*
         * Damage modifiers.
         */
        if ( dam > 0 )
        {
            if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
                dam /= 2;

            if ( IS_AFFECTED(victim, AFF_PROTECT) && IS_EVIL(ch) )
                dam -= (int) (dam / 4);

            if ( dam < 0 )
                dam = 0;
        }

        /*
         * Check for disarm, trip, parry, and dodge.
         */
        if ( dam > 0 && dt >= TYPE_HIT )
        {
            if ( IS_NPC(ch)
                 &&   IS_SET( ch->attacks, DFND_DISARM )
                 &&   GetMaxLevel(ch) > 9
                 &&   number_percent( ) < GetMaxLevel(ch) / 2 )
                disarm( ch, victim );

            if ( IS_NPC(ch)
                 &&   IS_SET( ch->attacks, ATCK_TRIP )
                 &&   GetMaxLevel(ch) > 5
                 &&   number_percent( ) < GetMaxLevel(ch) / 2 )
                trip( ch, victim );

            dam = check_parry( ch, victim, dam );
            dam = check_dodge( ch, victim, dam );
        }

        /*
         * Check control panel settings and modify damage
         */
        if ( IS_NPC(ch) )
        {
            if ( npcvict )
                dampmod = sysdata.dam_mob_vs_mob;
            else
                dampmod = sysdata.dam_mob_vs_plr;
        }
        else
        {
            if ( npcvict )
                dampmod = sysdata.dam_plr_vs_mob;
            else
                dampmod = sysdata.dam_plr_vs_plr;
        }
        if ( dampmod > 0 )
            dam = ( dam * dampmod ) / 100;

    }


    /*
     * Code to handle equipment getting damaged, and also support  -Thoric
     * bonuses/penalties for having or not having equipment where hit
     */
    if (dam > 10 && dt != TYPE_UNDEFINED)
    {
        /* get a random body eq part */
        dameq  = number_range(WEAR_LIGHT, MAX_WEAR-1);
        damobj = get_eq_char(victim, dameq);
        if ( damobj )
        {
            if ( dam > get_obj_resistance(damobj) )
            {
                damage_obj(damobj);
            }
            dam -= 5;  /* add a bonus for having something to block the blow */
        }
        else
            dam += 5;  /* add penalty for bare skin! */
    }

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */

#ifdef MUD_DEBUG
    if (!IS_NPC(ch) || !IS_NPC(victim))
        log_printf_plus(LOG_DEBUG, LEVEL_IMMORTAL, SEV_SPAM+9,
                        "damage: %d points - %s -> %s",
                        dam,
                        GET_NAME(ch),
                        GET_NAME(victim));
#endif

    dam_message( ch, victim, dam, dt );

    if (IS_AFFECTED2(victim, AFF2_LIFE_PROT) && dam >= GET_HIT(victim))
    {
        if (life_protection_object(victim))
            dam = 0;
        REMOVE_BIT(victim->affected_by2, AFF2_LIFE_PROT);
    }

    /* destroy brittle weapons */
    damobj = get_eq_char( ch, WEAR_DUAL_WIELD );
    if (damobj && IS_OBJ_STAT2(damobj, ITEM2_BRITTLE))
        damage_obj(damobj);
    damobj = get_eq_char( ch, WEAR_WIELD );
    if (damobj && IS_OBJ_STAT2(damobj, ITEM2_BRITTLE))
        damage_obj(damobj);

    /* dam is -1 when immune */
    if (dam == -1)
        dam = 0;

    if (dam < 0)
    {
        log_printf_plus(LOG_MONITOR, LEVEL_IMMORTAL, SEV_SPAM+1,
                        "damage: %s healing %s for %d points (absorb?)",
                        GET_NAME(ch),
                        GET_NAME(victim),
                        dam*-1);
    }

    if ( !IS_IMMORTAL(victim) )
        victim->hit = UMIN(victim->hit-dam, GET_MAX_HIT(victim));

    /* Make sure newbies dont die */

    if ( dam > 0 && (dt > TYPE_HIT || dt == gsn_backstab)
         && !IS_AFFECTED( victim, AFF_POISON )
         &&  is_wielding_poisoned( ch )
         && !IS_SET( victim->immune, RIS_POISON )
         && !saves_poison_death( GetMaxLevel(ch), victim ) )
    {
        AFFECT_DATA af;

        af.type      = gsn_poison;
        af.duration  = 20;
        af.location  = APPLY_STR;
        af.modifier  = -2;
        af.bitvector = AFF_POISON;
        affect_join( victim, &af );
        victim->mental_state = URANGE( 20, victim->mental_state + 2, 100 );
    }

    /*
     * Vampire self preservation				-Thoric
     */
    if ( IS_VAMPIRE(victim) )
    {
        if ( dam >= (GET_MAX_HIT(victim) / 10) )	/* get hit hard, lose blood */
            gain_condition(victim, COND_BLOODTHIRST, -1 - (GetMaxLevel(victim) / 20));
        if ( GET_HIT(victim) <= (GET_MAX_HIT(victim) / 8)
             && GET_COND(victim, COND_BLOODTHIRST)>5 )
        {
            gain_condition(victim, COND_BLOODTHIRST,
                           -URANGE(3, GetMaxLevel(victim) / 10, 8) );
            GET_HIT(victim) += URANGE( 4, (GET_MAX_HIT(victim) / 30), 15);
            set_char_color(AT_BLOOD, victim);
            send_to_char("You howl with rage as the beast within stirs!\n\r", victim);
        }
    }

    if ( !npcvict
         &&   get_trust(victim) >= LEVEL_IMMORTAL
         &&   get_trust(ch)     >= LEVEL_IMMORTAL
         &&   GET_HIT(victim) < 1 )
        victim->hit = GET_MAX_HIT(victim);

    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
        act( AT_DYING, "$n is mortally wounded, and will die soon, if not aided.",
             victim, NULL, NULL, TO_ROOM );
        act( AT_DANGER, "You are mortally wounded, and will die soon, if not aided.",
             victim, NULL, NULL, TO_CHAR );
        break;

    case POS_INCAP:
        act( AT_DYING, "$n is incapacitated and will slowly die, if not aided.",
             victim, NULL, NULL, TO_ROOM );
        act( AT_DANGER, "You are incapacitated and will slowly die, if not aided.",
             victim, NULL, NULL, TO_CHAR );
        break;

    case POS_STUNNED:
        if ( !IS_AFFECTED( victim, AFF_PARALYSIS ) )
        {
            act( AT_ACTION, "$n is stunned, but will probably recover.",
                 victim, NULL, NULL, TO_ROOM );
            act( AT_HURT, "You are stunned, but will probably recover.",
                 victim, NULL, NULL, TO_CHAR );
        }
        break;

    case POS_DEAD:
        if ( dt >= 0 && dt < top_sn )
        {
            SKILLTYPE *skill = skill_table[dt];

            if ( skill->die_char && skill->die_char[0] != '\0' )
                act( AT_DIEMSG, skill->die_char, ch, get_eq_char(ch, WEAR_WIELD), victim, TO_CHAR );
            if ( skill->die_vict && skill->die_vict[0] != '\0' )
                act( AT_DIEMSG, skill->die_vict, ch, get_eq_char(ch, WEAR_WIELD), victim, TO_VICT );
            if ( skill->die_room && skill->die_room[0] != '\0' )
                act( AT_DIEMSG, skill->die_room, ch, get_eq_char(ch, WEAR_WIELD), victim, TO_NOTVICT );
        }
        act( AT_DIEMSG, "$n is DEAD!", victim, 0, 0, TO_ROOM );
        act( AT_DIEMSG, "You have been killed!", victim, 0, 0, TO_CHAR );
        break;

    default:
        if ( dam > GET_MAX_HIT(victim) / 4 )
        {
            act( AT_HURT, "That really did HURT!", victim, 0, 0, TO_CHAR );
            if ( number_bits(3) == 0 )
                worsen_mental_state( ch, 1 );
        }
        if ( GET_HIT(victim) < GET_MAX_HIT(victim) / 4 )

        {
            act( AT_DANGER, "You wish that your wounds would stop BLEEDING so much!",
                 victim, 0, 0, TO_CHAR );
            if ( number_bits(2) == 0 )
                worsen_mental_state( ch, 1 );
        }
        break;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    /* lets make NPC's not slaughter PC's */
    /*
     if ( !IS_AWAKE(victim)
     &&   !IS_AFFECTED( victim, AFF_PARALYSIS ) )
     {
     if ( victim->fighting
     &&   victim->fighting->who->hunting
     &&   victim->fighting->who->hunting->who == victim )
     stop_hunting( victim->fighting->who );

     if ( victim->fighting
     &&   victim->fighting->who->hating
     &&   victim->fighting->who->hating->who == victim )
     stop_hating( victim->fighting->who );

     if (!npcvict && IS_NPC(ch))
     stop_fighting( victim, TRUE );
     else
     stop_fighting( victim, FALSE );
     }
     */
    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
        group_gain( ch, victim );

        if ( in_arena(ch) || in_arena(victim) )
        {
            sprintf( log_buf, "%s killed by %s in the arena at %d",
                     victim->name,
                     (IS_NPC(ch) ? ch->short_descr : ch->name),
                     victim->in_room->vnum );
            log_string_plus( log_buf,LOG_MONITOR,LEVEL_IMMORTAL,SEV_NOTICE);
        }
        else if ( !npcvict )
        {
            int loss;

            /*
             * Dying penalty:
             */
            if (GetMaxLevel(victim)>=51)
                loss = (int)((float)GET_EXP(victim) * 0.25);
            else if (GetMaxLevel(victim)>=40)
                loss = (int)((float)GET_EXP(victim) * 0.20);
            else if (GetMaxLevel(victim)>=25)
                loss = (int)((float)GET_EXP(victim) * 0.15);
            else if (GetMaxLevel(victim)>=15)
                loss = (int)((float)GET_EXP(victim) * 0.10);
            else if (GetMaxLevel(victim)>=5)
                loss = (int)((float)GET_EXP(victim) * 0.08);
            else
                loss = (int)((float)GET_EXP(victim) * 0.03);

            /* less loss for being killed by something stronger than you */
            if (GetMaxLevel(ch) > GetMaxLevel(victim))
                loss /= 2;

            /* less loss for being killed twice by the same mob */
            if ( IS_NPC(ch) )
            {
                if ( victim->last_killed_by == ch->vnum )
                    loss = (int)((float)loss * 0.10);
                else
                    victim->last_killed_by = ch->vnum;

            }

            sprintf( log_buf, "%s killed by %s at %d, lost %d/%d exp, dam %d, dt %d",
		    victim->name,
		    (IS_NPC(ch) ? ch->short_descr : ch->name),
		    victim->in_room->vnum,
		    loss, GET_EXP(victim),
		    dam, dt);
            log_string_plus( log_buf, LOG_MONITOR, LEVEL_IMMORTAL, SEV_NOTICE);

            GET_EXP(victim) -= UMIN(GET_EXP(victim), loss);
        }
        else
            if ( !IS_NPC(ch) )		/* keep track of mob vnum killed */
                add_kill( ch, victim );

        check_killer( ch, victim );

        loot = legal_loot( ch, victim );

        if ( ch->in_room != victim->in_room )
            loot = FALSE;

        set_cur_char(victim);
        raw_kill( ch, victim, dt );

        if ( victim == ch )
            return rNONE;

        victim = NULL;

        if ( !IS_NPC(ch) && loot )
        {
            /* Autogold by Scryn 8/12 */
            if ( IS_SET(ch->act, PLR_AUTOGOLD) )
            {
                int init, diff;
                OBJ_DATA *corpse;

                if ((corpse = get_obj_here(ch, "corpse")))
                {
                    OBJ_DATA *mobj, *mobj_next;
                    char mbuf[80];
		    int currtype;

                    for (mobj = corpse->first_content; mobj; mobj=mobj_next)
                    {
			mobj_next = mobj->next_content;
                        if (mobj->item_type != ITEM_MONEY)
                            continue;
			currtype = mobj->value[2];
                        init = GET_MONEY(ch, currtype);
                        sprintf(mbuf, "%s-coins corpse", curr_types[currtype]);
                        do_get( ch, mbuf );
                        diff = GET_MONEY(ch, currtype) - init;
                        if (diff)
                        {
                            sprintf(buf1,"%d %s", diff, curr_types[currtype]);
                            do_split( ch, buf1 );
                        }
                    }
                }
            }
            if ( IS_SET(ch->act, PLR_AUTOLOOT) )
                do_get( ch, "all corpse" );
            else if ( !IS_NPC(ch) && !IS_SET(ch->pcdata->flags, PCFLAG_GAG) )
                do_look( ch, "in corpse" );

            if ( IS_SET(ch->act, PLR_AUTOSAC) )
                do_sacrifice( ch, "corpse" );
        }

        if ( IS_SET( sysdata.save_flags, SV_KILL ) )
            save_char_obj( ch );
        return rVICT_DIED;
    }

    if ( victim == ch )
        return rNONE;

    /*
     * Take care of link dead people.
     */
    if ( !npcvict && !victim->desc
         && !IS_SET( victim->pcdata->flags, PCFLAG_NORECALL ) )
    {
        if ( number_range( 0, victim->wait ) == 0)
        {
            do_recall( victim, "" );
            return rNONE;
        }
    }

    /*
     * Wimp out?
     */
    if ( npcvict && dam > 0 )
    {
        if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 3 ) == 0
               &&   GET_HIT(victim) < GET_MAX_HIT(victim) / 2 )
             ||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master
                    &&     victim->master->in_room != victim->in_room ) )
        {
            start_fearing( victim, ch );
            stop_hunting( victim );
            do_flee( victim, "" );
        }
    }

    if ( !npcvict
         &&   GET_HIT(victim) > 0
         &&   GET_HIT(victim) <= victim->wimpy
         &&   victim->wait == 0 )
        do_flee( victim, "" );
    else
        if ( !npcvict && IS_SET( victim->act, PLR_FLEE ) )
            do_flee( victim, "" );

    tail_chain( );
    return rNONE;
}



bool is_safe( CHAR_DATA *ch, CHAR_DATA *victim )
{
    /* Thx Josh! */
    if ( who_fighting( ch ) == ch )
	return FALSE;

    if ( in_arena( ch ) )
        return FALSE;

    if ( IS_SET( victim->in_room->room_flags, ROOM_SAFE ) )
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "A magical force stops you.\n\r", ch );
        return TRUE;
    }

    if ( GetMaxLevel(ch) > LEVEL_IMMORTAL )
        return FALSE;

    if( !IS_NPC( ch ) && !IS_NPC( victim )
        && ch != victim
        && IS_SET( victim->in_room->area->flags, AFLAG_NOPKILL ) )
    {
        set_char_color( AT_IMMORT, ch );
        send_to_char( "The gods have forbidden player killing in this area.\n\r", ch );
        return TRUE;
    }

    if ( IS_NPC(ch) || IS_NPC(victim) )
        return FALSE;

    if ( get_age( ch ) < 18 || GetMaxLevel(ch) < 5 )
    {
        send_to_char( "You are not yet ready, needing age or experience, if not both.\n\r", ch );
        return TRUE;
    }

    if ( get_age( victim ) < 18 || GetMaxLevel(victim) < 5 )
    {
        send_to_char( "They are yet too young to die.\n\r", ch );
        return TRUE;
    }

    if ( GetMaxLevel(ch) - GetMaxLevel(victim) > 5
         ||   GetMaxLevel(victim) - GetMaxLevel(ch) > 5 )
    {
        set_char_color( AT_IMMORT, ch );
        send_to_char( "The gods do not allow murder when there is such a difference in level.\n\r", ch );
        return TRUE;
    }

    if ( get_timer(victim, TIMER_PKILLED) > 0 )
    {
        set_char_color( AT_GREEN, ch );
        send_to_char( "That character has died within the last 5 minutes.\n\r", ch);
        return TRUE;
    }

    if ( get_timer(ch, TIMER_PKILLED) > 0 )
    {
        set_char_color( AT_GREEN, ch );
        send_to_char( "You have been killed within the last 5 minutes.\n\r", ch );
        return TRUE;
    }

    return FALSE;
}

/*
 * just verify that a corpse looting is legal
 */
bool legal_loot( CHAR_DATA *ch, CHAR_DATA *victim )
{
    /* anyone can loot mobs */
    if ( IS_NPC(victim) )
        return TRUE;
    /* non-charmed mobs can loot anything */
    if ( IS_NPC(ch) && !ch->master )
        return TRUE;
    /* members of different clans can loot too! -Thoric */
    if ( !IS_NPC(ch) && !IS_NPC(victim)
         &&    IS_SET( ch->pcdata->flags, PCFLAG_DEADLY )
         &&    IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) )
        return TRUE;
    return FALSE;
}

/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if (ch == victim)
        return;

    /*
     * NPC's are fair game.
     */
    if ( IS_NPC(victim) )
    {
        if ( !IS_NPC( ch ) )
        {
            int level_ratio;
            level_ratio = URANGE( 1, GetMaxLevel(ch) / GetMaxLevel(victim), 50);
            if ( ch->pcdata->clan )
                ch->pcdata->clan->mkills++;
            ch->pcdata->mkills++;
            ch->in_room->area->mkills++;
            if ( ch->pcdata->deity )
            {
                if ( victim->race == ch->pcdata->deity->npcrace )
                    adjust_favor( ch, 3, level_ratio );
                else
                    if ( victim->race == ch->pcdata->deity->npcfoe )
                        adjust_favor( ch, 17, level_ratio );
                    else
                        adjust_favor( ch, 2, level_ratio );
            }
        }
        return;
    }

    /*
     * Any character in the arena is ok to kill.
     */
    if ( in_arena( ch ) )
        return;

    /*
     * So are killers and thieves.
     */
    if ( IS_SET(victim->act, PLR_KILLER) ||
         IS_SET(victim->act, PLR_THIEF) )
    {
        if ( !IS_NPC( ch ) )
        {
            if ( ch->pcdata->clan )
                ch->pcdata->clan->pkills++;
            ch->pcdata->pkills++;
            ch->in_room->area->pkills++;
        }
        if ( victim->pcdata->clan )
            victim->pcdata->clan->mdeaths++;
        victim->pcdata->mdeaths++;
        victim->in_room->area->mdeaths++;
        return;
    }

    /*
     * Charm-o-rama.
     */
    if ( IS_SET(ch->affected_by, AFF_CHARM) )
    {
        if ( !ch->master )
        {
	    bug( "Check_killer: %s bad AFF_CHARM", GET_NAME(ch) );
            affect_strip( ch, gsn_charm_person );
            REMOVE_BIT( ch->affected_by, AFF_CHARM );
            return;
        }

        /* stop_follower( ch ); */

        check_killer( ch->master, victim );
        return;
    }

    if ( !IS_NPC(ch) &&
         !IS_NPC(victim) &&
         IS_PC_FLAG( ch, PCFLAG_DEADLY ) &&
         IS_PC_FLAG( victim, PCFLAG_DEADLY ) )
    {

        if ( ch->pcdata->clan )
            ch->pcdata->clan->pkills++;
        ch->pcdata->pkills++;
        ch->in_room->area->pkills++;

        GET_HIT(ch)  = GET_MAX_HIT(ch);
        GET_MANA(ch) = GET_MAX_MANA(ch);
        GET_MOVE(ch) = GET_MAX_MOVE(ch);
        if ( ch->pcdata )
            ch->pcdata->condition[COND_BLOODTHIRST] = GET_MAX_BLOOD(ch);
        update_pos(victim);

        if ( victim != ch )
        {
            act( AT_MAGIC, "Bolts of blue energy rise from the corpse, seeping into $n.", ch, victim->name, NULL, TO_ROOM );
            act( AT_MAGIC, "Bolts of blue energy rise from the corpse, seeping into you.", ch, victim->name, NULL, TO_CHAR );
        }

        if ( victim->pcdata->clan )
            victim->pcdata->clan->pdeaths++;
        victim->pcdata->pdeaths++;

        adjust_favor( victim, 11, 1 );
        adjust_favor( ch, 2, 1 );
        add_timer( victim, TIMER_PKILLED, 115, NULL, 0 );
        WAIT_STATE( victim, 3 * PULSE_VIOLENCE );

        return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * And current killers stay as they are.
     */
    if ( IS_NPC(ch) )
    {
        if ( !IS_NPC(victim) )
        {
            int level_ratio;
            if ( victim->pcdata->clan )
                victim->pcdata->clan->mdeaths++;
            victim->pcdata->mdeaths++;
            victim->in_room->area->mdeaths++;

            level_ratio = URANGE( 1, GetMaxLevel(ch) / GetMaxLevel(victim), 50);
            if ( victim->pcdata->deity )
            {
                if ( ch->race == victim->pcdata->deity->npcrace )
                    adjust_favor( victim, 12, level_ratio );
                else
                    if ( ch->race == victim->pcdata->deity->npcfoe )
                        adjust_favor( victim, 15, level_ratio );
                    else
                        adjust_favor( victim, 11, level_ratio );
            }
        }
        return;
    }

    if ( ch->pcdata->clan )
        ch->pcdata->clan->illegal_pk++;
    ch->pcdata->illegal_pk++;
    ch->in_room->area->illegal_pk++;

    if ( victim->pcdata->clan )
        victim->pcdata->clan->pdeaths++;
    victim->pcdata->pdeaths++;
    victim->in_room->area->pdeaths++;

    if ( IS_SET(ch->act, PLR_KILLER) )
        return;

    log_printf_plus(LOG_MONITOR, LEVEL_IMMORTAL, SEV_NOTICE, "check_killer: %s attacking %s, setting KILLER", GET_NAME(ch), GET_NAME(victim));

    set_char_color( AT_WHITE, ch );
    send_to_char( "A strange feeling grows deep inside you, and a tingle goes up your spine...\n\r", ch );
    set_char_color( AT_IMMORT, ch );
    send_to_char( "A deep voice booms inside your head, 'Thou shall now be known as a deadly murderer!!!'\n\r", ch );
    set_char_color( AT_WHITE, ch );
    send_to_char( "You feel as if your soul has been revealed for all to see.\n\r", ch );

    SET_BIT(ch->act, PLR_KILLER);
    if ( IS_SET( ch->act, PLR_ATTACKER) )
        REMOVE_BIT(ch->act, PLR_ATTACKER);
    save_char_obj( ch );
    return;
}

/*
 * See if an attack justifies a ATTACKER flag.
 */
void check_attacker( CHAR_DATA *ch, CHAR_DATA *victim )
{

    /*
     * Made some changes to this function Apr 6/96 to reduce the prolifiration
     * of attacker flags in the realms. -Narn
     */
    /*
     * NPC's are fair game.
     * So are killers and thieves and people in the arena.
     */
    if ( ch == victim ||
         IS_NPC(victim) ||
         IS_SET(victim->act, PLR_KILLER) ||
         IS_SET(victim->act, PLR_THIEF) ||
         in_arena(victim) )
        return;

    /* deadly char check */
    if ( !IS_NPC(ch) &&
         !IS_NPC(victim) &&
         IS_PC_FLAG( ch, PCFLAG_DEADLY ) &&
         IS_PC_FLAG( victim, PCFLAG_DEADLY ) )
        return;

    /*
     * Charm-o-rama.
     */
    if ( IS_SET(ch->affected_by, AFF_CHARM) )
    {
        if ( !ch->master )
        {
	    bug( "Check_attacker: %s bad AFF_CHARM", GET_NAME(ch) );
            affect_strip( ch, gsn_charm_person );
            REMOVE_BIT( ch->affected_by, AFF_CHARM );
            return;
        }

        /* Won't have charmed mobs fighting give the master an attacker
         flag.  The killer flag stays in, and I'll put something in
         do_murder. -Narn */
        /*SET_BIT(ch->master->act, PLR_ATTACKER);*/
        /* stop_follower( ch ); */
        return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * And current killers stay as they are.
     */
    if ( IS_NPC(ch)
         ||   IS_SET(ch->act, PLR_ATTACKER)
         ||   IS_SET(ch->act, PLR_KILLER) )
        return;

    log_printf_plus(LOG_MONITOR, LEVEL_IMMORTAL, SEV_NOTICE, "check_attacker: %s attacking %s, setting ATTACKER", GET_NAME(ch), GET_NAME(victim));

    SET_BIT(ch->act, PLR_ATTACKER);
    save_char_obj( ch );
    return;
}


/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( !victim )
    {
        bug( "update_pos: null victim" );
        return;
    }

    if ( GET_HIT(victim) > 0 )
    {
        if ( victim->position <= POS_STUNNED )
            victim->position = POS_STANDING;
        if ( IS_AFFECTED( victim, AFF_PARALYSIS ) )
            victim->position = POS_STUNNED;
        return;
    }

    if ( IS_NPC(victim) || GET_HIT(victim) <= -11 )
    {
        if ( victim->mount )
        {
            act( AT_ACTION, "$n falls from $N.",
                 victim, NULL, victim->mount, TO_ROOM );
            REMOVE_BIT( victim->mount->act, ACT_MOUNTED );
            victim->mount = NULL;
        }
        victim->position = POS_DEAD;
        return;
    }

    if ( victim->position == POS_FIGHTING && !victim->fighting )
        victim->position = POS_STANDING;

    if ( GET_HIT(victim) <= -6 ) victim->position = POS_MORTAL;
    else if ( GET_HIT(victim) <= -3 ) victim->position = POS_INCAP;
    else                          victim->position = POS_STUNNED;

    if ( victim->position > POS_STUNNED
         &&   IS_AFFECTED( victim, AFF_PARALYSIS ) )
        victim->position = POS_STUNNED;

    if ( victim->mount )
    {
        act( AT_ACTION, "$n falls unconscious from $N.",
             victim, NULL, victim->mount, TO_ROOM );
        REMOVE_BIT( victim->mount->act, ACT_MOUNTED );
        victim->mount = NULL;
    }
    return;
}


/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    FIGHT_DATA *fight;

    if ( ch == victim )
    {
        bug( "Set_fighting: %s fighting self?", GET_NAME(ch) );
        return;
    }

    if ( ch->fighting )
    {
        bug( "Set_fighting: %s -> %s (already fighting %s)",
             GET_NAME(ch), GET_NAME(ch), GET_NAME(ch->fighting->who) );
        return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
        affect_strip( ch, gsn_sleep );
    stop_memorizing(ch);
    stop_memorizing(victim);

    /* Limit attackers -Thoric */
    if ( victim->num_fighting > max_fight(victim) )
    {
        send_to_char( "There are too many people fighting for you to join in.\n\r", ch );
        return;
    }

    CREATE( fight, FIGHT_DATA, 1 );
    fight->who	 = victim;
    fight->align = align_compute( ch, victim );
    if ( !IS_NPC(ch) && IS_NPC(victim) )
        fight->timeskilled = times_killed(ch, victim);
    ch->num_fighting = 1;
    ch->fighting = fight;
    ch->position = POS_FIGHTING;
    victim->num_fighting++;
    if ( victim->switched && IS_AFFECTED(victim->switched, AFF_POSSESS) )
    {
        send_to_char( "You are disturbed!\n\r", victim->switched );
        do_return( victim->switched, "" );
    }
    return;
}


CHAR_DATA *who_fighting( CHAR_DATA *ch )
{
    if ( !ch )
    {
        bug( "who_fighting: null ch" );
        return NULL;
    }
    if ( !ch->fighting )
        return NULL;
    return ch->fighting->who;
}

void free_fight( CHAR_DATA *ch )
{
    if ( !ch )
    {
        bug( "Free_fight: null ch!" );
        return;
    }
    if ( ch->fighting )
    {
        if ( !char_died(ch->fighting->who) )
            --ch->fighting->who->num_fighting;
        DISPOSE( ch->fighting );
    }
    ch->fighting = NULL;
    if ( ch->mount )
        ch->position = POS_MOUNTED;
    else
        ch->position = POS_STANDING;
    /* Berserk wears off after combat. -- Altrag */
    if ( IS_AFFECTED(ch, AFF_BERSERK) )
    {
        affect_strip(ch, gsn_berserk);
        set_char_color(AT_WEAROFF, ch);
        send_to_char(skill_table[gsn_berserk]->msg_off, ch);
        send_to_char("\n\r", ch);
        act( AT_WEAROFF, skill_table[gsn_berserk]->msg_off_room, ch, NULL, NULL, TO_ROOM );
    }
    return;
}


/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;

    free_fight( ch );
    update_pos( ch );

    if ( !fBoth )   /* major short cut here by Thoric */
        return;

    for ( fch = first_char; fch; fch = fch->next )
    {
        if ( who_fighting( fch ) == ch )
        {
            free_fight( fch );
            update_pos( fch );
        }
    }
    return;
}


/*
 * Improved Death_cry contributed by Diavolo.
 * Additional improvement by Thoric (and removal of turds... sheesh!)
 */
void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    EXIT_DATA *pexit;
    int vnum;

    if ( !ch )
    {
        bug( "DEATH_CRY: null ch!" );
        return;
    }

    vnum = 0;
    switch ( number_bits( 4 ) )
    {
    default: msg  = "You hear $n's death cry.";				break;
    case  0: msg  = "$n hits the ground ... DEAD.";			break;
    case  1: msg  = "$n splatters blood on your armor.";		break;
    case  2: if ( HAS_BODYPART(ch, PART_GUTS) )
    {
        msg  = "$n's guts spill all over the ground.";
        vnum = OBJ_VNUM_SPILLED_GUTS;
    }
    else
        msg = "$n collapses lifeless to the ground.";
    break;
    case  3: if ( HAS_BODYPART(ch, PART_HEAD) )
    {
        msg  = "$n's severed head plops on the ground.";
        vnum = OBJ_VNUM_SEVERED_HEAD;
    }
    else
        msg = "You hear $n's death cry.";
    break;
    case  4: if ( HAS_BODYPART(ch, PART_HEART) )
    {
        msg  = "$n's heart is torn from $s chest.";
        vnum = OBJ_VNUM_TORN_HEART;
    }
    else
        msg = "$n collapses lifeless to the ground.";
    break;
    case  5: if ( HAS_BODYPART(ch, PART_ARMS) )
    {
        msg  = "$n's arm is sliced from $s dead body.";
        vnum = OBJ_VNUM_SLICED_ARM;
    }
    else
        msg = "You hear $n's death cry.";
    break;
    case  6: if ( HAS_BODYPART(ch, PART_LEGS) || HAS_BODYPART(ch, PART_FORELEGS) )
    {
        msg  = "$n's leg is sliced from $s dead body.";
        vnum = OBJ_VNUM_SLICED_LEG;
    }
    else
        msg = "$n collapses lifeless to the ground.";
    break;
    }

    act( AT_CARNAGE, msg, ch, NULL, NULL, TO_ROOM );

    if ( vnum )
    {
        char buf[MAX_STRING_LENGTH];
        OBJ_DATA *obj;
        char *name;

        name		= IS_NPC(ch) ? ch->short_descr : ch->name;
        obj		= create_object( vnum );
        if (obj) {
            obj->timer	= number_range( 4, 7 );

            sprintf( buf, obj->short_descr, name );
            STRFREE( obj->short_descr );
            obj->short_descr = STRALLOC( buf );

            sprintf( buf, obj->description, name );
            STRFREE( obj->description );
            obj->description = STRALLOC( buf );

            obj = obj_to_room( obj, ch->in_room );
        }
    }

    if ( IS_NPC(ch) )
        msg = "You hear something's death cry.";
    else
        msg = "You hear someone's death cry.";

    was_in_room = ch->in_room;
    for ( pexit = was_in_room->first_exit; pexit; pexit = pexit->next )
    {
        if ( pexit->to_room
             &&   pexit->to_room != was_in_room )
        {
            ch->in_room = pexit->to_room;
            act( AT_CARNAGE, msg, ch, NULL, NULL, TO_ROOM );
        }
    }
    ch->in_room = was_in_room;

    return;
}



void raw_kill( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    CHAR_DATA *victmp;
    int tpos = 0;

    /*    char buf[MAX_STRING_LENGTH]; */

    if ( !victim )
    {
        bug( "raw_kill: null victim!" );
        return;
    }
    /* backup in case hp goes below 1 */
    if (NOT_AUTHED(victim))
    {
        bug( "raw_kill: killing unauthed" );
        return;
    }

    stop_fighting( victim, TRUE );

    if (IS_SYSTEMFLAG(SYS_NOKILL))
    {
        GET_POS(ch) = POS_STANDING;
        GET_POS(victim) = POS_STANDING;
        GET_HIT(victim) 	= UMAX( 1, GET_HIT(victim)  );
        GET_MANA(victim)	= UMAX( 1, GET_MANA(victim) );
        GET_MOVE(victim)	= UMAX( 1, GET_MOVE(victim) );
        return;
    }

    /* Take care of polymorphed chars */
    if(IS_NPC(victim) && IS_SET(victim->act, ACT_POLYMORPHED))
    {
        char_from_room(victim->desc->original);
        char_to_room(victim->desc->original, victim->in_room);
        victmp = victim->desc->original;
        do_revert(victim, "");
        raw_kill(ch, victmp, dt);
        return;
    }

#ifdef VTRACK
    if (IS_NPC(victim))
        vtrack_add_mobkill(ch, victim->vnum);
#endif

    tpos = GET_POS(ch);

    GET_POS(ch) = POS_STANDING;

    if (IS_NPC(victim))
        quest_trigger_mobkill(ch, victim);

    mprog_death_trigger( ch, victim );
    if ( char_died(victim) )
        return;
    /* death_cry( victim ); */

    rprog_death_trigger( ch, victim );
    if ( char_died(victim) )
        return;

    GET_POS(ch) = tpos;

    make_corpse( victim, ch, dt );
    make_blood( victim );

    if ( IS_NPC(victim) )
    {
        victim->pIndexData->killed++;
        extract_char( victim, TRUE );
        victim = NULL;
        return;
    }

    set_char_color( AT_DIEMSG, victim );

    if (in_arena(victim))
    {
        recall_char(victim);
        ch->position = POS_RESTING;
        do_look( victim, "auto" );
        return;
    }

    do_help(victim, "_DIEMSG_" );

    extract_char( victim, FALSE );
    if ( !victim )
    {
        bug( "oops! raw_kill: extract_char destroyed pc char" );
        return;
    }
    while ( victim->first_affect )
        affect_remove( victim, victim->first_affect );
    victim->affected_by	= race_table[victim->race].affected;
    victim->affected_by2 = 0;
    victim->resistant   = 0;
    victim->susceptible = 0;
    victim->immune      = 0;
    victim->carry_weight= 0;
    victim->armor	= 100;
    victim->mod_str	= 0;
    victim->mod_dex	= 0;
    victim->mod_wis	= 0;
    victim->mod_int	= 0;
    victim->mod_con	= 0;
    victim->mod_cha	= 0;
    victim->mod_lck   	= 0;
    victim->damroll	= 0;
    victim->hitroll	= 0;
    victim->mental_state = -10;
    GET_ALIGN(victim)	= URANGE( -1000, GET_ALIGN(victim), 1000 );
    victim->saving_spell_staff = 0;
    victim->position	= POS_RESTING;
    GET_HIT(victim)	= UMAX( 1, GET_HIT(victim)  );
    GET_MANA(victim)	= UMAX( 1, GET_MANA(victim) );
    GET_MOVE(victim)	= UMAX( 1, GET_MOVE(victim) );
    stop_hunting(ch);
    stop_hating(ch);

    /*
     * Pardon crimes...						-Thoric
     */
    if ( IS_SET( victim->act, PLR_KILLER) )
    {
        REMOVE_BIT( victim->act, PLR_KILLER);
        send_to_char("The gods have pardoned you for your murderous acts.\n\r",victim);
    }
    if ( IS_SET( victim->act, PLR_THIEF) )
    {
        REMOVE_BIT( victim->act, PLR_THIEF);
        send_to_char("The gods have pardoned you for your thievery.\n\r",victim);
    }

    /* for resur/reinc code - Heath */
    SET_BIT(victim->act2, PLR2_DIED);

    victim->pcdata->condition[COND_FULL]   = MAX_COND_VAL;
    victim->pcdata->condition[COND_THIRST] = MAX_COND_VAL;
    if ( IS_VAMPIRE( victim ) )
        victim->pcdata->condition[COND_BLOODTHIRST] = (GetMaxLevel(victim) / 2);

    if ( IS_SET( sysdata.save_flags, SV_DEATH ) )
        save_char_obj( victim );

    do_look( victim, "auto" );
    return;
}


static void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp, max_level = 0, share, diff;
    int members = 0, member_level = 1, npcs = 0;

    /*
     * Monsters don't get kill xp's or alignment changes.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if ( (IS_NPC(ch) && !ch->master) || victim == ch )
        return;

    if ( !IS_NPC(victim) )
        return;

    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
        if ( !is_same_group( gch, ch ) )
            continue;
	if ( IS_NPC( gch ) )
	    npcs++;
	else
	{
	    members++;
	    max_level = UMAX(max_level, GetMaxLevel(gch));
	    member_level += GetAveLevel(gch);
	}
    }

    if ( members == 0 )
    {
        bug( "Group_gain: %s's group has 0 members", GET_NAME(ch) );
        members = 1;
    }

    lch = ch->leader ? ch->leader : ch;

    share = GET_EXP(victim) / member_level;

    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
        if ( !is_same_group( gch, ch ) || IS_NPC( gch ) )
            continue;

        xp = share * GetAveLevel(gch);
        xp *= (100 + members);
        xp /= 100;
        diff = max_level - GetMaxLevel(gch);
        if (diff >= 10)
            xp /= ((diff / 10) + 1);
        /* aggressive calc */
        /* level mod calc */
	if (IS_SYSTEMFLAG(SYS_FREEXP))
	    xp = UMIN(xp, 5000000);
	else if (members > 8)
            xp = UMIN(xp, 2000000);
        else
            xp = UMIN(xp, ((members * 100000) + (npcs * 10000) + 100000));
        xp = UMAX(xp, 0);

        GET_ALIGN(gch) = align_compute( gch, victim );
        sprintf( buf, "%sYou receive %d experience points.\n\r", color_str(AT_PLAIN,gch), xp );
        send_to_char( buf, gch );
        gain_exp( gch, xp ); /* group gain */
        align_zap(ch);
    }

    return;
}

static int align_compute( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int change;

    if (IS_NPC(ch) && !IS_ACT_FLAG(ch,ACT_POLYMORPHED))
        return GET_ALIGN(ch);

    if (in_arena(ch))
        return GET_ALIGN(ch);

    change = GET_ALIGN(victim)/10;

    return (URANGE(-1000, GET_ALIGN(ch)-change, 1000));
}

static void align_zap(CHAR_DATA *ch)
{
    OBJ_DATA *obj, *obj_next;

    for ( obj = ch->first_carrying; obj; obj = obj_next )
    {
        obj_next = obj->next_content;
        if ( obj->wear_loc == WEAR_NONE )
            continue;

        if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
             ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
             ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
        {
            act( AT_MAGIC, "You are zapped by $p.", ch, obj, NULL, TO_CHAR );
            act( AT_MAGIC, "$n is zapped by $p.",   ch, obj, NULL, TO_ROOM );

            obj_from_char( obj );
            obj = obj_to_room( obj, ch->in_room );
            oprog_zap_trigger(ch, obj);  /* mudprogs */
            if ( char_died(ch) )
                return;
        }
    }
}

bool cant_hit_part(int part)
{
    if (part==PART_BRAINS)
        return TRUE;

    if (part==PART_LONG_TONGUE)
        return TRUE;

    if (part==PART_SHARPSCALES)
        return TRUE;

    if (part==PART_TAILATTACK)
        return TRUE;

    if (part==PART_HEART)
        return TRUE;

    return FALSE;
}

int RandomBodyPart(CHAR_DATA *ch)
{
    int i = number_range(0, 31);

    if (!ch)
        return(0);

    if (ch->xflags == 0)
        ch->xflags = race_bodyparts(ch);

    if (ch->xflags == 0)
        return(0);

    while (!HAS_BODYPART(ch, 1<<i) || cant_hit_part(1<<i))
        i = number_range(0,31);

    return(i);
}

static void dam_message( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
    char buf1[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH], buf3[MAX_INPUT_LENGTH];
    struct skill_type *skill = NULL;
    int snum = 0, hitloc = 0;

    static struct dam_weapon_type {
        char *to_room;
        char *to_char;
        char *to_victim;
    } dam_weapons[] = {
        {
            "$n misses $N with a wild swing.",                 /*    0    */
            "You miss $N.",
            "$n misses you with a wild swing."
        },
        {
            "$n bruises $N with $s %s %s.",                       /*  1.. 2  */
            "You bruise $N as you %s $M %s.",
            "$n bruises you as $e %s your %s."
        },
        {
            "$n barely %s $N %s.",                            /*  3.. 4  */
            "You barely %s $N %s.",
            "$n barely %s your %s."
        },
        {
            "$n %s $N %s.",                                   /*  5.. 6  */
            "You %s $N %s.",
            "$n %s your %s."
        },
        {
            "$n %s $N hard %s.",                               /*  7..10  */
            "You %s $N hard %s.",
            "$n %s you hard on your %s."
        },
        {
            "$n %s $N very hard %s.",                         /* 11..14  */
            "You %s $N very hard %s.",
            "$n %s you very hard on your %s."
        },
        {
            "$n %s $N extremely well %s.",                  /* 15..20  */
            "You %s $N extremely well %s.",
            "$n %s you extremely well on your %s."
        },
        {
            "$n ravages $N with $s %s %s.",
            "You ravage $N with your %s %s.",
            "$n ravages you with $s %s on your %s."
        },
        {
            "$n butchers $N with $s %s %s.",
            "You butcher $N with your %s %s.",
            "$n butchers you with $s %s on your %s."
        },
        {
            "$n demolishes $N with $s %s %s.",
            "You demolish $N with your %s %s.",
            "$n demolishes you with $s %s on your %s."
        },
        {
            "$n slaughters $N with $s %s %s.",
            "You slaughter $N with your %s %s.",
            "$n slaughters you with $s %s on your %s."
        },
        {
            "$n pulverizes $N with $s %s %s.",
            "You pulverize $N with your %s %s.",
            "$n pulverizes you with $s %s on your %s."
        },
        {
            "$n massacres $N with $s %s %s.",
            "You massacre $N with your %s %s.",
            "$n massacres you with $s %s on your %s."
        },
        {
            "$n decimates $N with $s %s %s.",
            "You decimate $N with your %s %s.",
            "$n decimates you with $s %s on your %s."
        },
        {
            "$n devastates $N with $s %s %s.",
            "You devastate $N with your %s %s.",
            "$n tears you up with $s %s on your %s."
        },
        {
            "$n extirpates $N with $s %s %s.",
            "You extirpate $N with your %s %s.",
            "$n extirpates you with $s %s on your %s."
        },
        {
            "$n obliterates $N with $s %s %s.",
            "You obliterate $N with your %s %s.",
            "$n obliterates you with $s %s on your %s."
        },
        {
            "$n vaporizes $N with $s %s %s.",
            "You vaporize $N with your %s %s.",
            "$n vaporizes you with $s %s on your %s."
        },
        {
            "$n atomizes $N with $s %s %s.",
            "You atomize $N with your %s %s.",
            "$n atomizes you with $s %s on your %s."
        },
        {
            "$n disembowels $N with $s %s %s.",
            "You disembowel $N with your %s %s.",
            "$n disembowels you with $s %s on your %s."
        },
        {
            "$n eviscerates $N with $s %s %s.",
            "You eviscerate $N with your %s %s.",
            "$n eviscerates you with $s %s on your %s."
        },
        {
            "$n incinerates $N with $s %s %s.",
            "You incinerate $N with your %s %s.",
            "$n incinerates you with $s %s on your %s."
        }
    };

    if ( dt==TYPE_UNDEFINED )
    {
	log_printf_plus(LOG_MAGIC, LEVEL_LOG_CSET, SEV_DEBUG+1,
			"dam_message: undefined dt");
        return;
    }

    if ( dt >=0 && dt < top_sn )
        skill = skill_table[dt];

    buf1[0]=buf2[0]=buf3[0]='\0';

    if ( dt >= TYPE_HIT )
    {
        if (dam <= 0) {
            snum = 0;
        } else if (dam <= 2) {
            snum = 1;
        } else if (dam <= 4) {
            snum = 2;
        } else if (dam <= 10) {
            snum = 3;
        } else if (dam <= 15) {
            snum = 4;
        } else if (dam <= 25) {
            snum = 5;
        } else if (dam <= 30) {
            snum = 6;
        } else if (dam <= 34) {
            snum = 7; /* ravage */
        } else if (dam <= 38) {
            snum = 8; /* butcher */
        } else if (dam <= 41) {
            snum = 9; /* demolish */
        } else if (dam <= 44) {
            snum = 10; /* slaughter */
        } else if (dam <= 47) {
            snum = 11; /* pulverize */
        } else if (dam <= 50) {
            snum = 12; /* massacre */
        } else if (dam <= 53) {
            snum = 13; /* decimate */
        } else if (dam <= 56) {
            snum = 14; /* devastate */
        } else if (dam <= 59) {
            snum = 15; /* expirate */
        } else if (dam <= 62) {
            snum = 16; /* obliterate */
        } else if (dam <= 66) {
            snum = 17; /* vaporize */
        } else if (dam <= 70) {
            snum = 18; /* atomize */
        } else if (dam <= 72) {
            snum = 19; /* disembowel */
        } else if (dam <= 75) {
            snum = 20; /* eviscerate */
        } else {
            snum = 21; /* incinerate */
        }

        if ( dt >= TYPE_HIT + sizeof(attack_table)/sizeof(attack_table[0]) ||
             dt >= TYPE_HIT + sizeof(attack_table_plural)/sizeof(attack_table_plural[0]) )
        {
            bug( "Dam_message: bad dt %d.", dt );
            dt  = TYPE_HIT;
        }

        hitloc = RandomBodyPart(victim);

        if (IS_NPC(victim) && victim->pIndexData->mudprogs)
        {
            char tbuf[MAX_INPUT_LENGTH];
            sprintf(tbuf, "self set hitloc %s", body_location[hitloc]);
            do_variables(victim, tbuf);
        }

        if (snum==0) {
            sprintf(buf1, "%s", dam_weapons[snum].to_room);
            sprintf(buf2, "%s", dam_weapons[snum].to_char);
            sprintf(buf3, "%s", dam_weapons[snum].to_victim);
        } else if (snum>=2 && snum<=6) {
            sprintf(buf1, dam_weapons[snum].to_room,
                    attack_table_plural[dt-TYPE_HIT],
                    body_location_hit[hitloc]);
            sprintf(buf2, dam_weapons[snum].to_char,
                    attack_table[dt-TYPE_HIT],
                    body_location_hit[hitloc]);
            sprintf(buf3, dam_weapons[snum].to_victim,
                    attack_table_plural[dt-TYPE_HIT],
                    body_location[hitloc]);
        } else {
            sprintf(buf1, dam_weapons[snum].to_room,
                    attack_table[dt-TYPE_HIT],
                    body_location_hit[hitloc]);
            sprintf(buf2, dam_weapons[snum].to_char,
                    attack_table[dt-TYPE_HIT],
                    body_location_hit[hitloc]);
            sprintf(buf3, dam_weapons[snum].to_victim,
                    attack_table[dt-TYPE_HIT],
                    body_location[hitloc]);
        }
    }
    else /* spell dam messages */
    {
        if ( skill )
        {
            bool found1 = FALSE, found2 = FALSE, found3 = FALSE;

            if ( dam < -1 )
            {
                if ( skill->abs_char && skill->abs_char[0] != '\0' ) {
                    act( AT_HIT, skill->abs_char, ch, get_eq_char(ch, WEAR_WIELD), victim, TO_CHAR );
                    found1 = TRUE;
                }
                if ( skill->abs_vict && skill->abs_vict[0] != '\0' ) {
                    act( AT_HITME, skill->abs_vict, ch, get_eq_char(ch, WEAR_WIELD), victim, TO_VICT );
                    found2 = TRUE;
                }
                if ( skill->abs_room && skill->abs_room[0] != '\0' ) {
                    act( AT_ACTION, skill->abs_room, ch, get_eq_char(ch, WEAR_WIELD), victim, TO_NOTVICT );
                    found3 = TRUE;
                }
                if ( !found1 || !found2 || !found3 )
                    log_printf_plus(LOG_MAGIC, LEVEL_LOG_CSET, SEV_DEBUG,
                                    "spell %d is missing an absorb message", dt);
            }
            else if ( dam == -1 )
            {
                if ( skill->imm_char && skill->imm_char[0] != '\0' ) {
                    act( AT_HIT, skill->imm_char, ch, get_eq_char(ch, WEAR_WIELD), victim, TO_CHAR );
                    found1 = TRUE;
                }
                if ( skill->imm_vict && skill->imm_vict[0] != '\0' ) {
                    act( AT_HITME, skill->imm_vict, ch, get_eq_char(ch, WEAR_WIELD), victim, TO_VICT );
                    found2 = TRUE;
                }
                if ( skill->imm_room && skill->imm_room[0] != '\0' ) {
                    act( AT_ACTION, skill->imm_room, ch, get_eq_char(ch, WEAR_WIELD), victim, TO_NOTVICT );
                    found3 = TRUE;
                }
                if ( !found1 || !found2 || !found3 )
                    log_printf_plus(LOG_MAGIC, LEVEL_LOG_CSET, SEV_DEBUG,
                                    "spell %d is missing an immune message", dt);
            }
            else if ( dam == 0 )
            {
                if ( skill->miss_char && skill->miss_char[0] != '\0' ) {
                    act( AT_HIT, skill->miss_char, ch, get_eq_char(ch, WEAR_WIELD), victim, TO_CHAR );
                    found1 = TRUE;
                }
                if ( skill->miss_vict && skill->miss_vict[0] != '\0' ) {
                    act( AT_HITME, skill->miss_vict, ch, get_eq_char(ch, WEAR_WIELD), victim, TO_VICT );
                    found2 = TRUE;
                }
                if ( skill->miss_room && skill->miss_room[0] != '\0' ) {
                    act( AT_ACTION, skill->miss_room, ch, get_eq_char(ch, WEAR_WIELD), victim, TO_NOTVICT );
                    found3 = TRUE;
                }
                if ( !found1 || !found2 || !found3 )
                    log_printf_plus(LOG_MAGIC, LEVEL_LOG_CSET, SEV_DEBUG,
                                    "spell %d is missing a miss message", dt);
            }
            else
            {
                if (GET_POS(victim) != POS_DEAD)
                {
                    if ( skill->hit_char && skill->hit_char[0] != '\0' ) {
                        if ( !SPELL_FLAG(skill, SF_AREA) ||
                             (SPELL_FLAG(skill, SF_AREA) &&
                              strstr(skill->hit_char, "$N")) )
                            act( AT_HIT, skill->hit_char, ch, get_eq_char(ch, WEAR_WIELD), victim, TO_CHAR );
                        found1 = TRUE;
                    }
                    if ( skill->hit_vict && skill->hit_vict[0] != '\0' ) {
                        if ( !SPELL_FLAG(skill, SF_AREA) ||
                             (SPELL_FLAG(skill, SF_AREA) &&
                              strstr(skill->hit_vict, "$N")) )
                            act( AT_HITME, skill->hit_vict, ch, get_eq_char(ch, WEAR_WIELD), victim, TO_VICT );
                        found2 = TRUE;
                    }
                    if ( skill->hit_room && skill->hit_room[0] != '\0' ) {
                        if ( !SPELL_FLAG(skill, SF_AREA) ||
                             (SPELL_FLAG(skill, SF_AREA) &&
                              strstr(skill->hit_room, "$N")) )
                            act( AT_ACTION, skill->hit_room, ch, get_eq_char(ch, WEAR_WIELD), victim, TO_NOTVICT );
                        found3 = TRUE;
                    }
                }
                if ( !found1 || !found2 || !found3 )
                    log_printf_plus(LOG_MAGIC, LEVEL_LOG_CSET, SEV_DEBUG,
                                    "spell %d is missing a hit message", dt);

            }

            if ( found1 || found2 || found3 )
                return;

            /* generic spell hit message here */
	    log_printf_plus(LOG_MAGIC, LEVEL_LOG_CSET, SEV_DEBUG+1,
			    "dam_message: generic spell hit message here");

            return;
        }
    }

    act( AT_ACTION, buf1, ch, NULL, victim, TO_NOTVICT );

    if (IS_NPC(ch) || !IS_SET(ch->pcdata->flags, PCFLAG_GAG))
        act( AT_HIT, buf2, ch, NULL, victim, TO_CHAR );

    if (IS_NPC(victim) || !IS_SET(victim->pcdata->flags, PCFLAG_GAG))
        act( AT_HITME, buf3, ch, NULL, victim, TO_VICT );
}

void do_assist( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim, *bob;
    char arg[MAX_INPUT_LENGTH];
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Assist whom?\n\r", ch );
        return;
    }

    if ( ( bob = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    victim = who_fighting(bob);

    if (!victim)
    {
        send_to_char( "They aren't fighting anyone!\n\r", ch );
        return;
    }

    if ( !IS_NPC(victim) )
    {
        if ( !IS_SET(victim->act, PLR_KILLER)
             &&   !IS_SET(victim->act, PLR_THIEF) )
        {
            send_to_char( "You must MURDER a player.\n\r", ch );
            return;
        }
    }

    if ( victim == ch )
    {
        send_to_char( "You hit yourself.  Ouch!\n\r", ch );
        return;
    }

    if ( is_safe( ch, victim ) )
        return;

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim )
    {
        act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( ch->position == POS_FIGHTING )
    {
        send_to_char( "You do the best you can!\n\r", ch );
        return;
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    check_attacker( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}

void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Kill whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "You hit yourself.  Ouch!\n\r", ch );
        return;
    }

    if ( is_safe( ch, victim ) )
        return;

    if (!CAN_PKILL(ch) && !CAN_PKILL(victim))
    {

    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
        if ( ch->master == victim )
        {
            act( AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
            return;
        }
        else if ( !IS_NPC(victim) )
        {
            if ( ch->master )
                SET_BIT(ch->master->act, PLR_ATTACKER);
        }
    }

    if ( ch->position == POS_FIGHTING )
    {
        send_to_char( "You do the best you can!\n\r", ch );
        return;
    }

    if ( !IS_NPC( victim ) && IS_SET( ch->act, PLR_NICE ) )
    {
        send_to_char( "You feel too nice to do that!\n\r", ch );
        return;
    }

    if (check_illegal_pk( ch, victim ))
    {
        send_to_char("You cannot do that.\n\r", ch);
        return;
    }

    if (!IS_NPC (victim) && !in_arena(ch))
    {
        sprintf( log_buf, "%s: murder %s.", ch->name, victim->name );
        log_string_plus( log_buf, LOG_MONITOR, GetMaxLevel(ch), SEV_NOTICE );
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    check_attacker( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}

bool in_arena( CHAR_DATA *ch )
{

    if ( IS_SET( ch->in_room->room_flags, ROOM_ARENA ) )
        return TRUE;

    if ( IS_SET( ch->in_room->area->flags, AFLAG_ARENA ) )
        return TRUE;

    if ( !str_cmp( ch->in_room->area->filename, "arena.are" ) )
        return TRUE;

    if ( ch->in_room->vnum < 29 || ch->in_room->vnum > 43 )
        return FALSE;

    return TRUE;
}

bool check_illegal_pk( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if (!victim || IS_NPC(victim) || IS_NPC(ch) || ch == victim || in_arena(victim))
        return FALSE;

    if (ch->pcdata->clan && victim->pcdata->clan)
        return FALSE;

    if (!is_same_race_align(ch, victim) &&
        GetMaxLevel(ch) - GetMaxLevel(victim) < 10)
        return FALSE;

    log_printf_plus(LOG_MONITOR, LEVEL_IMMORTAL, SEV_NOTICE,
                    "check_illegal_pk: ch: %s, victim: %s",
                    GET_NAME(ch), GET_NAME(victim));

    if (!CAN_PKILL(victim) || !CAN_PKILL(ch) ||
        GetMaxLevel(ch) - GetMaxLevel(victim) > 10)
    {
        sprintf( log_buf, "%s performing illegal pkill on %s at %d",
                 (IS_NPC(ch) ? ch->short_descr : ch->name),
                 victim->name,
                 victim->in_room->vnum );
        log_string_plus(log_buf,LOG_MONITOR,LEVEL_IMMORTAL,SEV_CRIT);
        return TRUE;
    }

    return FALSE;
}

void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    int attempt;
    int los = 0;
    bool retreated = FALSE;
    sh_int door;
    EXIT_DATA *pexit;

    if ( !who_fighting( ch ) )
    {
        if ( ch->position == POS_FIGHTING )
        {
            if ( ch->mount )
                ch->position = POS_MOUNTED;
            else
                ch->position = POS_STANDING;
        }
        send_to_char( "You aren't fighting anyone.\n\r", ch );
        return;
    }

    if ( GET_MOVE(ch) <= 0 )
    {
        send_to_char( "You're too exhausted to flee from combat!\n\r", ch );
        act(AT_FLEE, "$n tries to flee but is too exhausted.", ch, NULL, NULL, TO_ROOM);
        return;
    }

    if (GET_POS(ch) == POS_SITTING || GET_POS(ch) == POS_RESTING)
    {
        GET_MOVE(ch) -= 10;
        act(AT_PLAIN, "$n scrambles madly to $s feet!",
            ch, NULL, NULL, TO_ROOM);
        act(AT_PLAIN, "Panic-stricken, you scramble to your feet.",
            ch, NULL, NULL, TO_CHAR);
        GET_POS(ch) = POS_STANDING;
        WAIT_STATE(ch, PULSE_VIOLENCE);
        return;
    }

    /* No fleeing while stunned. - Narn */
    if ( GET_POS(ch) < POS_FIGHTING )
        return;

    if (is_affected(ch, gsn_berserk))
    {
        send_to_char("You can think of nothing but the battle.\n\r", ch);
        return;
    }

    if (is_affected(ch, gsn_web))
    {
        if (saves_para_petri(GetMaxLevel(ch), ch))
        {
            send_to_char("You are entrapped in sticky webs!\n\r", ch);
            send_to_char("Your struggles only entrap you further!\n\r", ch);
            act(AT_PLAIN, "$n struggles against webs that bind $s.", ch, NULL, NULL, TO_ROOM);
            WAIT_STATE(ch, PULSE_VIOLENCE);
            GET_MOVE(ch) = 0;
        }
        else
        {
            send_to_char("You briefly pull free from the sticky webbing!\n\r", ch);
            act(AT_PLAIN, "$n briefly pulls free from the webs that bind $s.", ch, NULL, NULL, TO_ROOM);
            GET_MOVE(ch) -= 50;
        }
        return;
    }


    if (!IS_NPC(ch))
        retreated = (number_percent() < LEARNED(ch, gsn_retreat)) ? TRUE : FALSE;

    if (!retreated)
        act(AT_FLEE, "$n panics and attempts to flee!", ch, NULL, NULL, TO_ROOM);

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 8; attempt++ )
    {

        door = number_door( );
        if ( ( pexit = get_exit(was_in, door) ) == NULL
             ||   !pexit->to_room
             || ( IS_SET(pexit->exit_info, EX_CLOSED)
                  &&   !IS_AFFECTED( ch, AFF_PASS_DOOR ) )
             || ( IS_NPC(ch)
                  &&   IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB) ) )
            continue;

        affect_strip ( ch, gsn_sneak );
        REMOVE_BIT   ( ch->affected_by, AFF_SNEAK );
        if ( ch->mount && ch->mount->fighting )
            stop_fighting( ch->mount, TRUE );
        move_char( ch, pexit, 0 );
        if ( ( now_in = ch->in_room ) == was_in )
            continue;

        ch->in_room = was_in;
        if (retreated)
        {
            act( AT_FLEE, "$n skillfully retreats from battle.", ch, NULL, NULL, TO_ROOM );
            act( AT_FLEE, "You skillfully retreat from battle.", ch, NULL, NULL, TO_CHAR );
        }
        else
        {
            act( AT_FLEE, "$n flees head over heels!", ch, NULL, NULL, TO_ROOM );
            act( AT_FLEE, "You flee, head over heels!", ch, NULL, NULL, TO_CHAR );
            learn_from_failure(ch, gsn_retreat);
        }
        ch->in_room = now_in;
        act( AT_FLEE, "$n glances around for signs of pursuit.", ch, NULL, NULL, TO_ROOM );

        if ( !IS_NPC(ch) )
        {
            CHAR_DATA *wf = who_fighting(ch);

	    if (!wf)
	    {
		bug("do_flee: something seriously wrong, !who_fighting(%s)", GET_NAME(ch));
		return;
	    }

            if (GetMaxLevel(ch) > 3) {
                if (!retreated ||
                    !HAS_CLASS(ch, CLASS_WARRIOR) ||
                    !HAS_CLASS(ch, CLASS_BARBARIAN) ||
                    !HAS_CLASS(ch, CLASS_PALADIN) ||
                    !HAS_CLASS(ch, CLASS_ANTIPALADIN) ||
                    !HAS_CLASS(ch, CLASS_RANGER))
                {
                    los = GetMaxLevel(ch) +
                        (GetSecMaxLev(ch)/2) +
                        (GetThirdMaxLev(ch)/3);
                    los -= GetMaxLevel(wf) +
                        (GetSecMaxLev(wf)/2) +
                        (GetThirdMaxLev(wf)/3);
                    los *= GetMaxLevel(ch);
                    if (los < 0)
                        los = 1;
                }
            } else
                los = 0;

            if (IS_NPC(wf) && !IS_ACT_FLAG(wf, ACT_SENTINEL))
            {
                if (IS_ACT_FLAG(wf, ACT_META_AGGR))
                {
                    start_hating(wf, ch);
                    start_hunting(wf, ch);
                }
                else if (IS_ACT_FLAG(wf, ACT_AGGRESSIVE))
                    start_hating(wf, ch);
            }

            if (los)
                gain_exp(ch, -los);


            if ( ch->pcdata->deity )
            {
                if ( wf && wf->race == ch->pcdata->deity->npcrace )
                    adjust_favor( ch, 1, 1 );
                else
                    if ( wf && wf->race == ch->pcdata->deity->npcfoe )
                        adjust_favor( ch, 16, 1 );
                    else
                        adjust_favor( ch, 0, 1 );
            }
        }
        stop_fighting( ch, TRUE );
        return;
    }

    send_to_char("PANIC! You couldn't escape!\n\r", ch);
    return;
}



void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}



void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );
    one_argument( argument, arg2 );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Slay whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( ch == victim )
    {
        send_to_char( "Suicide is a mortal sin.\n\r", ch );
        return;
    }

    if ( !IS_NPC(victim) && get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "immolate" ) )
    {
        act( AT_FIRE, "Your fireball turns $N into a blazing inferno.",  ch, NULL, victim, TO_CHAR    );
        act( AT_FIRE, "$n releases a searing fireball in your direction.", ch, NULL, victim, TO_VICT    );
        act( AT_FIRE, "$n points at $N, who bursts into a flaming inferno.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "shatter" ) )
    {
        act( AT_LBLUE, "You freeze $N with a glance and shatter the frozen corpse into tiny shards.",  ch, NULL, victim, TO_CHAR    );
        act( AT_LBLUE, "$n freezes you with a glance and shatters your frozen body into tiny shards.", ch, NULL, victim, TO_VICT    );
        act( AT_LBLUE, "$n freezes $N with a glance and shatters the frozen body into tiny shards.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "demon" ) )
    {
        act( AT_IMMORT, "You gesture, and a slavering demon appears.  With a horrible grin, the",  ch, NULL, victim, TO_CHAR );
        act( AT_IMMORT, "foul creature turns on $N, who screams in panic before being eaten alive.",  ch, NULL, victim, TO_CHAR );
        act( AT_IMMORT, "$n gestures, and a slavering demon appears.  The foul creature turns on",  ch, NULL, victim, TO_VICT );
        act( AT_IMMORT, "you with a horrible grin.   You scream in panic before being eaten alive.",  ch, NULL, victim, TO_VICT );
        act( AT_IMMORT, "$n gestures, and a slavering demon appears.  With a horrible grin, the",  ch, NULL, victim, TO_NOTVICT );
        act( AT_IMMORT, "foul creature turns on $N, who screams in panic before being eaten alive.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "pounce" ) && get_trust(ch) >= LEVEL_ASCENDANT )
    {
        act( AT_BLOOD, "Leaping upon $N with bared fangs, you tear open $S throat and toss the corpse to the ground...",  ch, NULL, victim, TO_CHAR );
        act( AT_BLOOD, "In a heartbeat, $n rips $s fangs through your throat!  Your blood sprays and pours to the ground as your life ends...", ch, NULL, victim, TO_VICT );
        act( AT_BLOOD, "Leaping suddenly, $n sinks $s fangs into $N's throat.  As blood sprays and gushes to the ground, $n tosses $N's dying body away.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "slit" ) && get_trust(ch) >= LEVEL_ASCENDANT )
    {
        act( AT_BLOOD, "You calmly slit $N's throat.", ch, NULL, victim, TO_CHAR );
        act( AT_BLOOD, "$n reaches out with a clawed finger and calmly slits your throat.", ch, NULL, victim, TO_VICT );
        act( AT_BLOOD, "$n calmly slits $N's throat.", ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "cow" ) && get_trust(ch) >= LEVEL_ASCENDANT )
    {
        act( AT_WHITE, "You summon the cow spirits to do harm to $N.", ch, NULL, victim, TO_CHAR );
        act( AT_WHITE, "$n summons a giant cow which procedes to chew your head off.", ch, NULL, victim, TO_VICT );
        act( AT_WHITE, "$n summons a giant cow which procedes to chew $N's head off.", ch, NULL, victim, TO_NOTVICT );
    }

    else
    {
        act( AT_IMMORT, "You slay $N in cold blood!",  ch, NULL, victim, TO_CHAR    );
        act( AT_IMMORT, "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
        act( AT_IMMORT, "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );
    }

    set_cur_char(victim);
    raw_kill( ch, victim, TYPE_UNDEFINED );
    return;
}

int BarbarianToHitMagicBonus(CHAR_DATA *ch)
{
    if (GetMaxLevel(ch) <= 7)
        return 1;
    if (GetMaxLevel(ch) <= 13)
        return 2;
    if (GetMaxLevel(ch) <= 20)
        return 3;
    if (GetMaxLevel(ch) <= 28)
        return 4;
    if (GetMaxLevel(ch) <= 35)
        return 5;
    if (GetMaxLevel(ch) <= 50)
        return 5;

    return(6);
}

int berserkthaco(CHAR_DATA *ch)
{
    if (GetMaxLevel(ch) <= 10) /* -5 to hit when berserked */
        return(5);
    if (GetMaxLevel(ch) <= 25) /* -3 */
        return(3);
    if (GetMaxLevel(ch) <= 40) /* -2 */
        return(2);

    return(1);
}

int berserkdambonus(CHAR_DATA *ch, int dam)
{
    if (GetMaxLevel(ch) <= 10)     /* 1.33 dam when berserked */
        return((int)((float)dam*1.33));
    if (GetMaxLevel(ch) <= 25)     /* 1.5 */
        return((int)((float)dam*1.5));
    if (GetMaxLevel(ch) <= 40)     /* 1.7 */
        return((int)((float)dam*1.7));
    if (GetMaxLevel(ch) <= 50)     /* 1.8 */
        return((int)((float)dam*1.8));
    if (GetMaxLevel(ch) <= 60)     /* 1.8 */
        return((int)((float)dam*1.9));

    return((int)((float)dam*1.8));
}

int CalcThaco(CHAR_DATA *ch)
{
    int calc_thaco;

    /* Calculate the raw armor including magic armor */
    /* The lower AC, the better                      */

    if (!IS_NPC(ch))
        calc_thaco = thaco[BestFightingClass(ch)][GET_LEVEL(ch, BestFightingClass(ch))];
    else
        /* THAC0 for monsters is set in the HitRoll */
        calc_thaco = 20;

    /*  Drow are -4 to hit during daylight or lighted rooms. */
    if (!room_is_dark(ch->in_room) && GET_RACE(ch) == RACE_DROW && !IS_NPC(ch) &&
        !is_affected(ch,gsn_darkness) && !IS_UNDERGROUND(ch))
        calc_thaco +=4;

    if (!room_is_dark(ch->in_room) && GET_RACE(ch) == RACE_UNDEAD_VAMPIRE)
        calc_thaco +=4;

    if (IS_AFFECTED(ch,AFF_BERSERK))
        calc_thaco += berserkthaco(ch);

    if (IS_IMMUNE(ch, RIS_EVIL) || IS_RESIS(ch, RIS_EVIL))
        if (ch->fighting)
            if (IS_EVIL(who_fighting(ch))) calc_thaco -=1;

    /* you get -4 to hit a mob if your evil and he has */
    /* prot from evil */
    if (who_fighting(ch) && IS_EVIL(ch))
        if (IS_IMMUNE(who_fighting(ch), RIS_EVIL) ||
            IS_RESIS(who_fighting(ch), RIS_EVIL))
            calc_thaco+=4;

    calc_thaco -= str_app[get_curr_str(ch)].tohit;
    calc_thaco -= GET_HITROLL(ch);
    calc_thaco += IS_NPC(ch)?0:(GET_COND(ch, COND_DRUNK)/5);
    return calc_thaco;
}

int HitOrMiss(CHAR_DATA *ch, CHAR_DATA *victim, int calc_thaco)
{
    int diceroll, victim_ac;

    diceroll = number_range(1,20);

    victim_ac  = GET_AC(victim)/10;

    if (!IS_AWAKE(victim))
        victim_ac -= dex_app[get_curr_dex(victim)].defensive;

    victim_ac = UMAX(-10, victim_ac);  /* -10 is lowest */

    if (diceroll < 20 && IS_AWAKE(victim) &&
        (diceroll==1 || calc_thaco-diceroll > victim_ac))
        return FALSE;

    return TRUE;
}

CHAR_DATA *race_align_hatee(CHAR_DATA *ch)
{
    CHAR_DATA *vch;

    for (vch=ch->in_room->first_person;vch;vch=vch->next_in_room)
    {
        if (!IS_NPC(vch) && IS_IMMORTAL(vch))
            continue;
        if (char_died(vch))
            continue;
        if (can_see(ch,vch) &&
            ((IsBadSide(vch) && IsGoodSide(ch)) ||
             (IsGoodSide(vch) && IsBadSide(ch)) ||
             (IsUndead(vch) && !IsUndead(ch)) ||
             (!IsUndead(vch) && IsUndead(ch))))
            return vch;
    }

    return NULL;
}

int lorebonus(CHAR_DATA *ch, CHAR_DATA *victim, int sn)
{
    int mult=0, loresn=-1;

    if (IsAnimal(victim))
        loresn = gsn_animal_lore;
    else if (IsVeggie(victim))
        loresn = gsn_vegetable_lore;
    else if (IsDiabolic(victim))
        loresn = gsn_demonology;
    else if (IsReptile(victim))
        loresn = gsn_reptile_lore;
    else if (IsUndead(victim))
        loresn = gsn_necromancy;
    else if (IsGiantish(victim) || IsGiant(victim))
        loresn = gsn_giant_lore;
    else if (IsPerson(victim))
        loresn = gsn_people_lore;
    else if (IsOther(victim))
        loresn = gsn_other_lore;
    else
        return 0;

    if (loresn == -1)
    {
        bug("lorebonus: unknown lore");
        return 0;
    }

    if (LEARNED(ch, loresn) > 40)
        mult += 1;
    if (LEARNED(ch, loresn) > 74)
        mult += 1;

    if (mult > 0)
        send_to_char("Your lore aids your attack!\n\r", ch);

    return mult;
}

bool life_protection_object(CHAR_DATA *ch)
{
    OBJ_DATA *obj, *obj_next;
    AFFECT_DATA *paf;

    for ( obj = ch->first_carrying; obj; obj = obj_next )
    {
        obj_next = obj->next_content;
        if ( obj->wear_loc == WEAR_NONE )
            continue;

        for (paf = obj->first_affect; paf; paf = paf->next)
            if (paf->location == APPLY_AFF2 &&
                IS_SET(paf->modifier, AFF2_LIFE_PROT))
            {
                act(AT_PLAIN, "$p shatters with a blinding flash of light!", ch, obj, NULL, TO_CHAR);
                make_scraps(obj,TRUE);
                return TRUE;
            }

        /*
        if (!obj->pIndexData)
            continue;
        for (paf = obj->pIndexData->first_affect; paf; paf = paf->next)
            if (paf->location == APPLY_AFF2 &&
                IS_SET(paf->modifier, AFF2_LIFE_PROT))
            {
                act(AT_PLAIN, "$p shatters with a blinding flash of light!", ch, obj, NULL, TO_CHAR);
                make_scraps(obj,TRUE);
                return TRUE;
            }
        */
    }

    return FALSE;
}
