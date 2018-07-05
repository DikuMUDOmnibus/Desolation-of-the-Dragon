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
 *			     Player skills module			    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: skills.c,v 1.64 2004/04/06 22:00:11 dotd Exp $";*/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "mud.h"
#include "gsn.h"

DECLARE_DO_FUN(do_steal);
DECLARE_DO_FUN(do_gossip);
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_recall);
DECLARE_DO_FUN(do_shout);
DECLARE_DO_FUN(do_cast);

const char *att_kick_kill_ch[] = {
    "Your kick caves $N's chest in, killing $M instantly.",
    "Your kick destroys $N's arm and caves in one side of $S rib cage.",
    "Your kick smashes through $N's leg and into $S pelvis, killing $M.",
    "Your kick shatters $N's skull.",
    "Your kick at $N's snout shatters $S jaw, killing $M.",
    "You kick $N in the rump with such force that $E keels over dead.",
    "You kick $N in the belly, mangling several ribs and killing $M instantly.",
    "$N's scales cave in as your mighty kick kills $N.",
    "Your kick rips bark asunder and leaves fly everywhere, killing the $N.",
    "Bits of $N are sent flying as you kick him to pieces.",
    "You punt $N across the room, $E lands in a heap of broken flesh.",
    "You kick $N in the groin, $E dies screaming an octave higher.",
    "",  /* GHOST */
    "Feathers fly about as you blast $N to pieces with your kick.",
    "Your kick splits $N to pieces, rotting flesh flies everywhere.",
    "Your kick topples $N over, killing it.",
    "Your foot shatters cartilage, sending bits of $N everywhere.",
    "You launch a mighty kick at $N's gills, killing it.",
    "Your kick at $N sends $M to the grave.",
    "."
};
const char *att_kick_kill_victim[] = {
    "$n crushes you beneath $s foot, killing you.",
    "$n destroys your arm and half your ribs.  You die.",
    "$n neatly splits your groin in two, you collapse and die instantly.",
    "$n splits your head in two, killing you instantly.",
    "$n forces your jaw into the lower part of your brain.",
    "$n kicks you from behind, snapping your spine and killing you.",
    "$n kicks your stomach and you into the great land beyond!!",
    "Your scales are no defense against $n's mighty kick.",
    "$n rips you apart with a massive kick, you die in a flutter of leaves.",
    "You are torn to little pieces as $n splits you with $s kick.",
    "$n's kick sends you flying, you die before you land.",
    "Puny little $n manages to land a killing blow to your groin, OUCH!",
    "", /* GHOST */
    "Your feathers fly about as $n pulverizes you with a massive kick.",
    "$n's kick rips your rotten body into shreds, and your various pieces die.",
    "$n kicks you so hard, you fall over and die.",
    "$n shatters your exoskeleton, you die.",
    "$n kicks you in the gills!  You cannot breath..... you die!.",
    "$n sends you to the grave with a mighty kick.",
    "."
};
const char *att_kick_kill_room[] = {
    "$n strikes $N in chest, shattering the ribs beneath it.",
    "$n kicks $N in the side, destroying $S arm and ribs.",
    "$n nails $N in the groin, the pain killing $M.",
    "$n shatters $N's head, reducing $M to a twitching heap!",
    "$n blasts $N in the snout, destroying bones and causing death.",
    "$n kills $N with a massive kick to the rear.",
    "$n sends $N to the grave with a massive blow to the stomach!",
    "$n ignores $N's scales and kills $M with a mighty kick.",
    "$n sends bark and leaves flying as $e splits $N in two.",
    "$n blasts $N to pieces with a ferocious kick.",
    "$n sends $N flying, $E lands with a LOUD THUD, making no other noise.",
    "$N falls to the ground and dies clutching $S crotch due to $n's kick.",
    "", /* GHOST */
    "$N disappears into a cloud of feathers as $n kicks $M to death.",
    "$n blasts $N's rotten body into pieces with a powerful kick.",
    "$n kicks $N so hard, it falls over and dies.",
    "$n blasts $N's exoskeleton to little fragments.",
    "$n kicks $N in the gills, killing it.",
    "$n sends $N to the grave with a mighty kick.",
    "."
};
const char *att_kick_miss_ch[] = {
    "$N steps back, and your kick misses $M.",
    "$N deftly blocks your kick with $S forearm.",
    "$N dodges, and you miss your kick at $S legs.",
    "$N ducks, and your foot flies a mile high.",
    "$N steps back and grins evilly as your foot flys by $S face.",
    "$N laughs at your feeble attempt to kick $M from behind.",
    "Your kick at $N's belly makes it laugh.",
    "$N chuckles as your kick bounces off $S tough scales.",
    "You kick $N in the side, denting your foot.",
    "Your sloppy kick is easily avoided by $N.",
    "You misjudge $N's height and kick well above $S head.",
    "You stub your toe against $N's shin as you try to kick $M.",
    "Your kick passes through $N!!",  /* Ghost */
    "$N nimbly flitters away from your kick.",
    "$N sidesteps your kick and sneers at you.",
    "Your kick bounces off $N's leathery hide.",
    "Your kick bounces off $N's tough exoskeleton.",
    "$N deflects your kick with a fin.",
    "$N avoids your paltry attempt at a kick.",
    "."
};
const char *att_kick_miss_victim[] = {
    "$n misses you with $s clumsy kick at your chest.",
    "You block $n's feeble kick with your arm.",
    "You dodge $n's feeble leg sweep.",
    "You duck under $n's lame kick.",
    "You step back and grin as $n misses your face with a kick.",
    "$n attempts a feeble kick from behind, which you neatly avoid.",
    "You laugh at $n's feeble attempt to kick you in the stomach.",
    "$n kicks you, but your scales are much too tough for that wimp.",
    "You laugh as $n dents $s foot on your bark.",
    "You easily avoid a sloppy kick from $n.",
    "$n's kick parts your hair but does little else.",
    "$n's light kick to your shin bearly gets your attention.",
    "$n passes through you with $s puny kick.",
    "You nimbly flitter away from $n's kick.",
    "You sneer as you sidestep $n's kick.",
    "$n's kick bounces off your tough hide.",
    "$n tries to kick you, but your too tough.",
    "$n tried to kick you, but you deflected it with a fin.",
    "You avoid $n's feeble attempt to kick you.",
    "."
};

const char *att_kick_miss_room[] = {
    "$n misses $N with a clumsy kick.",
    "$N blocks $n's kick with $S arm.",
    "$N easily dodges $n's feeble leg sweep.",
    "$N easily ducks under $n's lame kick.",
    "$N steps back and grins evilly at $n's feeble kick to $S face misses.",
    "$n launches a kick at $N's behind, but fails miserably.",
    "$N laughs at $n's attempt to kick $M in the stomach.",
    "$n tries to kick $N, but $s foot bounces off of $N's scales.",
    "$n hurts his foot trying to kick $N.",
    "$N avoids a lame kick launched by $n.",
    "$n misses a kick at $N due to $S small size.",
    "$n misses a kick at $N's groin, stubbing $s toe in the process.",
    "$n's foot goes right through $N!!!!",
    "$N flitters away from $n's kick.",
    "$N sneers at $n while sidestepping $s kick.",
    "$N's tough hide deflects $n's kick.",
    "$n hurts $s foot on $N's tough exterior.",
    "$n tries to kick $N, but is thwarted by a fin.",
    "$N avoids $n's feeble kick.",
    "."
};

const char *att_kick_hit_ch[] = {
    "Your kick crashes into $N's chest.",
    "Your kick hits $N in the side.",
    "You hit $N in the thigh with a hefty sweep.",
    "You hit $N in the face, sending $M reeling.",
    "You plant your foot firmly in $N's snout, smashing it to one side.",
    "You nail $N from behind, sending him reeling.",
    "You kick $N in the stomach, winding $M.",
    "You find a soft spot in $N's scales and launch a solid kick there.",
    "Your kick hits $N, sending small branches and leaves everywhere.",
    "Your kick contacts with $N, dislodging little pieces of $M.",
    "Your kick hits $N right in the stomach, $N is rendered breathless.",
    "You stomp on $N's foot. After all, thats about all you can do to a giant.",
    "", /* GHOST */
    "Your kick  sends $N reeling through the air.",
    "You kick $N and feel rotten bones crunch from the blow.",
    "You smash $N with a hefty roundhouse kick.",
    "You kick $N, cracking it's exoskeleton.",
    "Your mighty kick rearranges $N's scales.",
    "You leap off the ground and crash into $N with a powerful kick.",
    "."
};

const char *att_kick_hit_victim[] = {
    "$n's kick crashes into your chest.",
    "$n's kick hits you in your side.",
    "$n's sweep catches you in the side and you almost stumble.",
    "$n hits you in the face, gee, what pretty colors...",
    "$n kicks you in the snout, smashing it up against your face.",
    "$n blasts you in the rear, ouch!",
    "Your breath rushes from you as $n kicks you in the stomach.",
    "$n finds a soft spot on your scales and kicks you, ouch!",
    "$n kicks you hard, sending leaves flying everywhere!",
    "$n kicks you in the side, dislodging small parts of you.",
    "You suddenly see $n's foot in your chest.",
    "$n lands a kick hard on your foot making you jump around in pain.",
    "", /* GHOST */
    "$n kicks you, and you go reeling through the air.",
    "$n kicks you and your bones crumble.",
    "$n hits you in the flank with a hefty roundhouse kick.",
    "$n ruins some of your scales with a well placed kick.",
    "$n leaps off of the grand and crashes into you with $s kick.",
    "."
};

const char *att_kick_hit_room[] = {
    "$n hits $N with a mighty kick to $S chest.",
    "$n whacks $N in the side with a sound kick.",
    "$n almost sweeps $N off of $S feet with a well placed leg sweep.",
    "$N's eyes roll as $n plants a foot in $S face.",
    "$N's snout is smashed as $n relocates it with $s foot.",
    "$n hits $N with an impressive kick from behind.",
    "$N gasps as $n kick $N in the stomach.",
    "$n finds a soft spot in $N's scales and launches a solid kick there.",
    "$n kicks $N.  Leaves fly everywhere!!",
    "$n hits $N with a mighty kick, $N loses parts of $Mself.",
    "$n kicks $N in the stomach, $N is rendered breathless.",
    "$n kicks $N in the foot, $N hops around in pain.",
    "", /* GHOST */
    "$n sends $N reeling through the air with a mighty kick.",
    "$n kicks $N causing parts of $N to cave in!",
    "$n kicks $N in the side with a hefty roundhouse kick.",
    "$n kicks $N, cracking exo-skelelton.",
    "$n kicks $N hard, sending scales flying!",
    "$n leaps up and nails $N with a mighty kick.",
    "."
};


char * const spell_flag[] =
{
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
    "water", "earth", "air", "astral", "area", "distant", "reverse",
    "save_half_dam", "save_negates", "accumulative", "recastable", "noscribe",
    "nobrew", "group", "object", "character", "secretskill", "pksensitive",
    "stoponfail", "verbalize_skill", "requires_forge"
};

char * const spell_save[] =
{ "none", "poison_death", "wands", "para_petri", "breath", "spell_staff" };

char * const spell_damage[] =
{ "none", "fire", "cold", "electricity", "energy", "acid", "poison", "drain" };

char * const spell_action[] =
{ "none", "create", "destroy", "resist", "suscept", "divinate", "obscure",
"change" };

char * const spell_power[] =
{ "none", "minor", "greater", "major" };

char * const spell_ch_class[] =
{ "none", "lunar", "solar", "travel", "summon", "life", "death", "illusion",
  "curse" };

char * const target_type[] =
{ "ignore", "offensive", "defensive", "self", "objinv", "objroom"};


void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch );

int ris_save( CHAR_DATA *ch, int chance, int ris );
bool check_illegal_psteal( CHAR_DATA *ch, CHAR_DATA *victim );

/* from magic.c */
void failed_casting( SKILLTYPE *skill, CHAR_DATA *ch,
                     CHAR_DATA *victim, OBJ_DATA *obj );


/*
 * Dummy function
 */
void skill_notfound( CHAR_DATA *ch, char *argument )
{
    send_to_char( "Huh?\n\r", ch );
    return;
}


int get_starget( char *name )
{
    unsigned int x;

    for ( x = 0; x < sizeof(target_type) / sizeof(target_type[0]); x++ )
        if ( !str_cmp( name, target_type[x] ) )
            return x;
    return -1;
}

int get_sflag( char *name )
{
    unsigned int x;

    for ( x = 0; x < sizeof(spell_flag) / sizeof(spell_flag[0]); x++ )
        if ( !str_cmp( name, spell_flag[x] ) )
            return x;
    return -1;
}

int get_sdamage( char *name )
{
    unsigned int x;

    for ( x = 0; x < sizeof(spell_damage) / sizeof(spell_damage[0]); x++ )
        if ( !str_cmp( name, spell_damage[x] ) )
            return x;
    return -1;
}

int get_saction( char *name )
{
    unsigned int x;

    for ( x = 0; x < sizeof(spell_action) / sizeof(spell_action[0]); x++ )
        if ( !str_cmp( name, spell_action[x] ) )
            return x;
    return -1;
}

int get_spower( char *name )
{
    unsigned int x;

    for ( x = 0; x < sizeof(spell_power) / sizeof(spell_power[0]); x++ )
        if ( !str_cmp( name, spell_power[x] ) )
            return x;
    return -1;
}

int get_ssave( char *name )
{
    unsigned int x;

    for ( x = 0; x < sizeof(spell_save) / sizeof(spell_save[0]); x++ )
        if ( !str_cmp( name, spell_save[x] ) )
            return x;
    return -1;
}

int get_sch_class( char *name )
{
    unsigned int x;

    for ( x = 0; x < sizeof(spell_ch_class) / sizeof(spell_ch_class[0]); x++ )
        if ( !str_cmp( name, spell_ch_class[x] ) )
            return x;
    return -1;
}

bool is_legal_kill(CHAR_DATA *ch, CHAR_DATA *vch)
{
    if ( IS_NPC(ch) || IS_NPC(vch) )
        return TRUE;
    if ( !IS_PKILL(ch) || !IS_PKILL(vch) )
        return FALSE;
    if ( ch->pcdata->clan && ch->pcdata->clan == vch->pcdata->clan )
        return FALSE;
    return TRUE;
}


extern char *target_name;	/* from magic.c */

/*
 * Perform a binary search on a section of the skill table
 * Each different section of the skill table is sorted alphabetically
 * Only match skills player knows				-Thoric
 */
bool check_skill( CHAR_DATA *ch, char *command, char *argument )
{
    int sn;
    int max=0;
    sh_int cl;
    int first = gsn_first_skill;
    int top   = GSN_LAST_SKILL;
    int mana, blood;
    struct timeval time_used;
    char tempstr[MAX_INPUT_LENGTH];

    /* bsearch for the skill */
    for (;;)
    {
        sn = (first + top) >> 1;

        if ( LOWER(command[0]) == LOWER(skill_table[sn]->name[0])
             &&  !str_prefix(command, skill_table[sn]->name)
             &&  (skill_table[sn]->skill_fun || skill_table[sn]->spell_fun != spell_null)
             &&  ((IS_NPC(ch) && !IS_SET(ch->act, ACT_POLYMORPHED))
                  ||  (LEARNED(ch, sn) > 0
                       &&   CanUseSkill(ch, sn))) )
            break;
        if (first >= top)
            return FALSE;
        if (strcmp( command, skill_table[sn]->name) < 1)
            top = sn - 1;
        else
            first = sn + 1;
    }

    if ( !check_pos( ch, skill_table[sn]->minimum_position ) )
        return TRUE;

    if ( IS_NPC(ch)
         &&  (IS_AFFECTED( ch, AFF_CHARM ) || IS_AFFECTED( ch, AFF_POSSESS )) )
    {
        send_to_char( "For some reason, you seem unable to perform that...\n\r", ch );
        act( AT_GREY,"$n wanders around aimlessly.", ch, NULL, NULL, TO_ROOM );
        return TRUE;
    }

    /* check if mana is required */
    if ( skill_table[sn]->min_mana )
    {
        mana = IS_NPC(ch) ? 0 : UMAX(skill_table[sn]->min_mana,
                                     100 / (UMAX(2,(2 + ch->levels[LowSkCl(ch, sn)]) - LowSkLv(ch, sn)) ) );
        blood = UMAX(1, (mana+4) / 8);      /* NPCs don't have PCDatas. -- Altrag */
        if ( IS_VAMPIRE(ch) )
        {
            if (GET_COND(ch, COND_BLOODTHIRST) < blood)
            {
                send_to_char( "You don't have enough blood power.\n\r", ch );
                return TRUE;
            }
        }
        else
            if ( !IS_NPC(ch) && GET_MANA(ch) < mana )
            {
                send_to_char( "You don't have enough mana.\n\r", ch );
                return TRUE;
            }
    }
    else
    {
        mana = 0;
        blood = 0;
    }

    /*
     * Is this a real do-fun, or a really a spell?
     */
    if ( !skill_table[sn]->skill_fun )
    {
        ch_ret retcode = rNONE;
        void *vo = NULL;
        CHAR_DATA *victim = NULL;
        OBJ_DATA *obj = NULL;

        target_name = "";

        switch ( skill_table[sn]->target )
        {
        default:
            bug( "Check_skill: bad target for sn %d.", sn );
            send_to_char( "Something went wrong...\n\r", ch );
            return TRUE;

        case TAR_IGNORE:
            vo = NULL;
            if ( argument[0] == '\0' )
            {
                if ( (victim=who_fighting(ch)) != NULL )
                    target_name = victim->name;
            }
            else
                target_name = argument;
            break;

        case TAR_CHAR_OFFENSIVE:
            if ( argument[0] == '\0'
                 &&  (victim=who_fighting(ch)) == NULL )
            {
                ch_printf( ch, "%s who?\n\r", capitalize( skill_table[sn]->name ) );
                return TRUE;
            }
            else
                if ( argument[0] != '\0'
                     &&  (victim=get_char_room(ch, argument)) == NULL )
                {
                    send_to_char( "They aren't here.\n\r", ch );
                    return TRUE;
                }
            if ( is_safe( ch, victim ) )
                return TRUE;
            vo = (void *) victim;
            break;

        case TAR_CHAR_DEFENSIVE:
            if ( argument[0] != '\0'
                 &&  (victim=get_char_room(ch, argument)) == NULL )
            {
                send_to_char( "They aren't here.\n\r", ch );
                return TRUE;
            }
            if ( !victim )
                victim = ch;
            vo = (void *) victim;
            break;

        case TAR_CHAR_SELF:
            if ( !victim )
                victim = ch;
            vo = (void *) ch;
            break;

        case TAR_OBJ_INV:
            if ( (obj=get_obj_carry(ch, argument)) == NULL )
            {
                send_to_char( "You can't find that.\n\r", ch );
                return TRUE;
            }
            vo = (void *) obj;
            break;
        }

        /* waitstate */
        spell_lag(ch, sn);
        /* check for failure */

        /* check for anti eq and give penalty must like cast does */
        for (cl = FIRST_CLASS; cl < MAX_CLASS; cl++)
        {
            if (!IS_ACTIVE(ch, cl))
                continue;

            switch (cl)
            {
            case CLASS_NONE:
            case LAST_CLASS:
                break;
            case CLASS_MAGE:
            case CLASS_SORCERER:
                if (EqWBits(ch, ITEM_ANTI_MAGE))
                    max += 5;
                break;
            case CLASS_NECROMANCER:
                if (EqWBits2(ch, ITEM2_ANTI_NECROMANCER))
                    max += 5;
                break;
            case CLASS_CLERIC:
                if (EqWBits(ch, ITEM_ANTI_CLERIC))
                    max += 5;
                break;
            case CLASS_DRUID:
                if (EqWBits(ch, ITEM_ANTI_DRUID))
                    max += 5;
                break;
            case CLASS_PALADIN:
                if (EqWBits2(ch, ITEM2_ANTI_PALADIN))
                    max += 10;
                break;
            case CLASS_ANTIPALADIN:
                if (EqWBits2(ch, ITEM2_ANTI_APALADIN))
                    max += 10;
                break;
            case CLASS_PSIONIST:
                if (EqWBits2(ch, ITEM2_ANTI_PSI))
                    max += 10;
                break;
            case CLASS_RANGER:
                if (EqWBits2(ch, ITEM2_ANTI_RANGER))
                    max += 10;
                break;
            case CLASS_VAMPIRE:
                if (EqWBits(ch, ITEM_ANTI_VAMPIRE))
                    max += 5;
                break;
            case CLASS_ARTIFICER:
                if (EqWBits2(ch, ITEM2_ANTI_ARTIFICER))
                    max += 5;
                break;
            case CLASS_MONK:
                if (EqWBits2(ch, ITEM2_ANTI_MONK))
                    max += 10;
                break;
            case CLASS_THIEF:
                if (EqWBits(ch, ITEM_ANTI_THIEF))
                    max += 10;
                break;
            case CLASS_AMAZON:
                if (EqWBits2(ch, ITEM2_ANTI_AMAZON))
                    max += 10;
                break;
            case CLASS_WARRIOR:
                if (EqWBits(ch, ITEM_ANTI_WARRIOR))
                    max += 10;
                break;
            case CLASS_BARBARIAN:
                if (EqWBits2(ch, ITEM2_ANTI_BARBARIAN))
                    max += 10;
                break;
            }
        }

        if (max > 0 && GetMaxLevel(ch)<5)
            send_to_char("Note: You are wearing anti-class eq, removing it will improve your skill ability.\n\r(This message will self-destruct when you reach level 5)\n\r", ch);

        max += skill_table[sn]->difficulty * 5;

        if (!IS_NPC(ch))
            log_printf_plus(LOG_DEBUG, LEVEL_IMMORTAL, SEV_DEBUG, "check_skill: %s skillfail: %d  sn: %d", GET_NAME(ch), max, sn);

        if ( (number_percent( ) + max) > LEARNED(ch, sn) )
        {
            failed_casting( skill_table[sn], ch, victim, obj );
            learn_from_failure( ch, sn );
            if ( mana )
            {
                if ( IS_VAMPIRE(ch) )
                    gain_condition( ch, COND_BLOODTHIRST, - blood );
                else
                    GET_MANA(ch) -= mana;
            }
            return TRUE;
        }
        if ( mana )
        {
            if ( IS_VAMPIRE(ch) )
                gain_condition( ch, COND_BLOODTHIRST, - (blood >> 1) );
            else
                GET_MANA(ch) -= (mana >> 1);
        }
        start_timer(&time_used);
        retcode = (*skill_table[sn]->spell_fun) ( sn, ch->levels[BestSkCl(ch, sn)], ch, vo );
        end_timer(&time_used);
        update_userec(&time_used, &skill_table[sn]->userec);

        if ( retcode == rCHAR_DIED || retcode == rERROR )
            return TRUE;

        if ( char_died(ch) )
            return TRUE;

        if ( retcode == rSPELL_FAILED )
        {
            learn_from_failure( ch, sn );
            retcode = rNONE;
        }
        else
            learn_from_success( ch, sn );

        if ( skill_table[sn]->target == TAR_CHAR_OFFENSIVE
             &&   victim != ch
             &&  !char_died(victim) )
        {
            CHAR_DATA *vch;
            CHAR_DATA *vch_next;

            for ( vch = ch->in_room->first_person; vch; vch = vch_next )
            {
                vch_next = vch->next_in_room;
                if ( victim == vch && !victim->fighting && victim->master != ch )
                {
                    retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
                    break;
                }
            }
        }
        return TRUE;
    }

    if ( mana )
    {
        if ( IS_VAMPIRE(ch) )
            gain_condition( ch, COND_BLOODTHIRST, - (blood >> 1) );
        else
            GET_MANA(ch) -= (mana >> 1);
    }
    ch->prev_cmd = ch->last_cmd;    /* haus, for automapping */
    ch->last_cmd = skill_table[sn]->skill_fun;
    start_timer(&time_used);
    if (skill_table[sn]->skill_fun!=do_smaug_skill)
        (*skill_table[sn]->skill_fun) ( ch, argument );
    else {
        sprintf(tempstr, "%s %s", skill_table[sn]->name, argument);
        (*skill_table[sn]->skill_fun) ( ch, tempstr );
    }
    end_timer(&time_used);
    update_userec(&time_used, &skill_table[sn]->userec);

    tail_chain( );
    return TRUE;
}

/*
 * Lookup a skills information
 * High god command
 */
void do_slookup( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;
    int iClass;
    int type;
    SKILLTYPE *skill = NULL;
    char s1[16], s2[16], s3[16], s4[16];

    buf[0] = '\0';
    arg[0] = '\0';

    strcpy(arg, argument);
    if ( arg[0] == '\0' )
    {
        send_to_char( "Slookup what?\n\r"
                      "  slookup all\n\r  slookup null\n\r  slookup smaug\n\r"
                      "  slookup herbs\n\r  slookup lore\n\r  slookup tongue\n\r"
                      "  slookup <class>\n\r  slookup slots\n\r", ch );
        return;
    }

    sprintf(s1, "%s", color_str(AT_SCORE, ch));
    sprintf(s2, "%s", color_str(AT_SCORE2, ch));
    sprintf(s3, "%s", color_str(AT_SCORE3, ch));
    sprintf(s4, "%s", color_str(AT_SCORE4, ch));

    if ( !str_cmp( arg, "all" ) )
    {
        for ( sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++ )
            pager_printf( ch, "Sn: %3d Slot: %3d Name: '%-24s' Damtype: %s\n\r",
                          sn, skill_table[sn]->slot, skill_table[sn]->name,
                          spell_damage[SPELL_DAMAGE( skill_table[sn] )] );
    }
    else if ( !str_cmp( arg, "null" ) )
    {
        int num=0;
        for ( sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++ )
            if ((skill_table[sn]->skill_fun==skill_notfound ||
                 skill_table[sn]->skill_fun==NULL) &&
                (skill_table[sn]->spell_fun==spell_notfound ||
                 skill_table[sn]->spell_fun==spell_null) &&
                skill_table[sn]->type != SKILL_LORE &&
                skill_table[sn]->type != SKILL_TONGUE)
            {
                pager_printf( ch, "Sn: %3d Slot: %3d Name: '%-24s' Damtype: %s\n\r",
                              sn, skill_table[sn]->slot, skill_table[sn]->name,
                              spell_damage[SPELL_DAMAGE( skill_table[sn] )] );
                num++;
            }
        pager_printf(ch, "%d matches found.\n\r", num);
    }
    else if ( (type = get_skill_tname(arg)) )
    {
        int num=0;
        for ( sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++ )
            if (skill_table[sn]->type == type)
            {
                pager_printf( ch, "Sn: %3d Slot: %3d Name: '%-24s'\n\r",
                              sn, skill_table[sn]->slot, skill_table[sn]->name );
                num++;
            }
        pager_printf(ch, "%d matches found.\n\r", num);
    }
    else if ( (iClass = get_classtype(arg)) != CLASS_NONE )
    {
	int num=0;

        for ( sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++ )
            if (skill_table[sn]->skill_level[iClass] < LEVEL_IMMORTAL)
            {
                pager_printf( ch, "Sn: %3d Slot: %3d Name: '%-24s' Damtype: %s\n\r",
                              sn, skill_table[sn]->slot, skill_table[sn]->name,
                              spell_damage[SPELL_DAMAGE( skill_table[sn] )] );
                num++;
            }
        pager_printf(ch, "%d matches found for %s.\n\r", num, pc_class[iClass]);
    }
    else if ( !str_cmp( arg, "smaug" ) )
    {
        int num=0;

        for ( sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++ )
            if (skill_table[sn]->skill_fun==do_smaug_skill ||
                skill_table[sn]->spell_fun==spell_smaug)

            {
                pager_printf( ch, "Sn: %3d Slot: %3d Name: '%-24s' Damtype: %s\n\r",
                              sn, skill_table[sn]->slot, skill_table[sn]->name,
                              spell_damage[SPELL_DAMAGE( skill_table[sn] )] );
                num++;
            }
        pager_printf(ch, "%d matches found.\n\r", num);
    }
    else if ( !str_cmp( arg, "herbs" ) )
    {
        for ( sn = 0; sn < top_herb && herb_table[sn] && herb_table[sn]->name; sn++ )
            pager_printf( ch, "%d) %s\n\r", sn, herb_table[sn]->name );
    }
    else if ( !str_cmp( arg, "slots" ) )
    {
        int maxslot=0, x;

        for ( sn = 0; sn < top_sn && skill_table[sn]; sn++ )
            maxslot = UMAX(maxslot, skill_table[sn]->slot);

        for ( x = 0; x <= maxslot; x++ )
        {
            if ( (sn = slot_lookup(x)) < 1 )
                continue;

            pager_printf( ch, "Slot: %3d Sn: %3d Name: '%-24s' Damtype: %s\n\r",
                          skill_table[sn]->slot, sn, skill_table[sn]->name,
                          spell_damage[SPELL_DAMAGE( skill_table[sn] )] );
        }
        return;
    }
    else
    {
        SMAUG_AFF *aff;
        int cnt = 0;

        if ( arg[0] == 'h' && is_number(arg+1) )
        {
            sn = atoi(arg+1);
            if ( !IS_VALID_HERB(sn) )
            {
                send_to_char( "Invalid herb.\n\r", ch );
                return;
            }
            skill = herb_table[sn];
        }
        else
            if ( is_number(arg) )
            {
                sn = atoi(arg);
                if ( (skill=get_skilltype(sn)) == NULL )
                {
                    send_to_char( "Invalid sn.\n\r", ch );
                    return;
                }
                sn %= 1000;
            }
            else
                if ( ( sn = skill_lookup( arg ) ) >= 0 )
                    skill = skill_table[sn];
                else
                    if ( ( sn = herb_lookup( arg ) ) >= 0 )
                        skill = herb_table[sn];
                    else
                    {
                        send_to_char( "No such skill, spell, proficiency or tongue.\n\r", ch );
                        return;
                    }
        if ( !skill )
        {
            send_to_char( "Not created yet.\n\r", ch );
            return;
        }

        set_char_color(AT_SCORE,ch);
        ch_printf( ch, "%sSn: %s%4d%s Slot: %s%4d%s %s: '%s%-20s%s'\n\r",
                   s1, s2, sn,
                   s1, s2, skill->slot,
                   s1, skill_tname[skill->type],
                   s3, skill->name, s1 );

        if ( skill->flags )
        {
            ch_printf( ch, "%sDamtype: %s%s%s  Acttype: %s%s%s  Classtype: %s%s%s Powertype: %s%s\n\r",
                       s1, s3, spell_damage[SPELL_DAMAGE( skill )],
                       s1, s3, spell_action[SPELL_ACTION( skill )],
                       s1, s3, spell_ch_class[SPELL_CLASS( skill )],
                       s1, s3, spell_power[SPELL_POWER( skill )] );

            ch_printf( ch, "%sFlags: %s%s\n\r",
                       s1, s3, flag_string(skill->flags, spell_flag));
        }
        ch_printf( ch, "%sSaves: %s%s\n\r", s1, s3, spell_save[SPELL_SAVE(skill)] );

        if ( skill->difficulty != '\0' )
            ch_printf( ch, "%sDifficulty: %s%d%s\n\r", s1, s2, (int) skill->difficulty, s1 );

        ch_printf( ch, "%sType: %s%s%s  Target: %s%s%s  Minpos: %s%s%s  Mana: %s%d%s Beats: %s%d%s\n\r",
                   s1, s3, skill_tname[skill->type],
                   s1, s3, target_type[URANGE(TAR_IGNORE, skill->target, TAR_OBJ_ROOM)],
                   s1, s3, position_types[skill->minimum_position],
                   s1, s2, skill->min_mana,
                   s1, s2, skill->beats, s1 );
        ch_printf( ch, "Flags: %s%d%s  Guild: %s%d%s  Code: %s%s%s\n\r",
                   s2, skill->flags, s1,
                   s2, skill->guild, s1,
                   s3, skill->skill_fun ? skill_name(skill->skill_fun)
                   : spell_name(skill->spell_fun), s1);

	if ( skill->noun_damage )
	    ch_printf( ch, "Dammsg: %s%s%s\n\r",
		      s3, skill->noun_damage, s1 );

        if ( skill->msg_off && skill->msg_off[0] != '\0' )
            ch_printf( ch, "Wearoff: %s%s%s\n\r",
                       s3, skill->msg_off, s1 );

        if ( skill->msg_off_room && skill->msg_off_room[0] != '\0' )
            ch_printf(ch, "Wearoffroom: %s%s%s\n\r",
                      s3, skill->msg_off_room, s1);

        if ( skill->msg_off_soon && skill->msg_off_soon[0] != '\0' )
            ch_printf( ch, "Wearoffsoon: %s%s%s\n\r",
                       s3, skill->msg_off_soon, s1);

        if ( skill->msg_off_soon_room && skill->msg_off_soon_room[0] != '\0' )
            ch_printf(ch, "Wearoffsoonroom: %s%s%s\n\r",
                      s3, skill->msg_off_soon_room, s1 );

        if ( skill->dice && skill->dice[0] != '\0' )
            ch_printf( ch, "Dice: %s%s%s\n\r", s3, skill->dice, s1 );
        if ( skill->teachers && skill->teachers[0] != '\0' )
            ch_printf( ch, "Teachers: %s%s%s\n\r", s3, skill->teachers, s1  );
        if ( skill->components && skill->components[0] != '\0' )
            ch_printf( ch, "Components: %s%s%s\n\r", s3, skill->components, s1 );
        if ( skill->participants )
            ch_printf( ch, "Participants : %s%d%s\n\r", s2, (int) skill->participants, s1 );

        if ( skill->part_start_char && skill->part_start_char[0] != '\0' )
            ch_printf( ch, "PartStartChar: %s%s%s\n\r",
                       s3, skill->part_start_char, s1 );
        if ( skill->part_start_room && skill->part_start_room[0] != '\0' )
            ch_printf( ch, "PartStartRoom: %s%s%s\n\r",
                       s3, skill->part_start_room, s1 );
        if ( skill->part_end_char && skill->part_end_char[0] != '\0' )
            ch_printf( ch, "PartEndChar  : %s%s%s\n\r",
                       s3, skill->part_end_char, s1 );
        if ( skill->part_end_vict && skill->part_end_vict[0] != '\0' )
            ch_printf( ch, "PartEndVict  : %s%s%s\n\r",
                       s3, skill->part_end_vict, s1 );
        if ( skill->part_end_room && skill->part_end_room[0] != '\0' )
            ch_printf( ch, "PartEndRoom  : %s%s%s\n\r",
                       s3, skill->part_end_room, s1 );
        if ( skill->part_end_caster && skill->part_end_caster[0] != '\0' )
            ch_printf( ch, "PartEndCaster: %s%s%s\n\r",
                       s3, skill->part_end_caster, s1 );
        if ( skill->part_miss_char && skill->part_miss_char[0] != '\0' )
            ch_printf( ch, "PartMissChar : %s%s%s\n\r",
                       s3, skill->part_miss_char, s1 );
        if ( skill->part_miss_room && skill->part_miss_room[0] != '\0' )
            ch_printf( ch, "PartMissRoom : %s%s%s\n\r",
                       s3, skill->part_miss_room, s1 );
        if ( skill->part_abort_char && skill->part_abort_char[0] != '\0' )
            ch_printf( ch, "PartAbortChar: %s%s%s\n\r",
                       s3, skill->part_abort_char, s1 );

        if ( skill->userec.num_uses )
            send_timer(&skill->userec, ch);
        for ( aff = skill->affects; aff; aff = aff->next )
        {
            if ( aff == skill->affects )
                send_to_char( "\n\r", ch );
            sprintf( buf, "Affect %s%d%s", s2, ++cnt, s1 );
            if ( aff->location )
            {
                strcat( buf, " modifies " );
                strcat( buf, s3 );
                if (aff->location > REVERSE_APPLY)
                    strcat(buf, "!");
                strcat( buf, a_types[aff->location % REVERSE_APPLY] );
                strcat( buf, s1 );
                strcat( buf, " by '" );
                strcat( buf, s3 );
                strcat( buf, aff->modifier );
                strcat( buf, s1 );
                if ( aff->bitvector )
                    strcat( buf, "' and" );
                else
                    strcat( buf, "'" );
                strcat( buf, s1 );
            }
            if ( aff->bitvector )
            {
                int x;

                strcat( buf, " applies" );
                strcat( buf, s3 );
                for ( x = 0; x < 32; x++ )
                    if ( IS_SET(aff->bitvector, 1 << x) )
                    {
                        strcat( buf, " " );
                        strcat( buf, a_flags[x] );
                    }
                strcat( buf, s1 );
            }
            if ( aff->duration[0] != '\0' && aff->duration[0] != '0' )
            {
                strcat( buf, " for '" );
                strcat( buf, s3 );
                strcat( buf, aff->duration );
                strcat( buf, s1 );
                strcat( buf, "' rounds" );
            }
            if ( aff->location >= REVERSE_APPLY )
                strcat( buf, " (affects caster only)" );
            strcat( buf, "\n\r" );
            send_to_char( buf, ch );
            if ( !aff->next )
                send_to_char( "\n\r", ch );
        }
        if ( skill->hit_char && skill->hit_char[0] != '\0' )
            ch_printf( ch, "Hitchar : %s%s%s\n\r",
                       s3, skill->hit_char, s1 );
        if ( skill->hit_vict && skill->hit_vict[0] != '\0' )
            ch_printf( ch, "Hitvict : %s%s%s\n\r",
                       s3, skill->hit_vict, s1 );
        if ( skill->hit_room && skill->hit_room[0] != '\0' )
            ch_printf( ch, "Hitroom : %s%s%s\n\r",
                       s3, skill->hit_room, s1 );
        if ( skill->miss_char && skill->miss_char[0] != '\0' )
            ch_printf( ch, "Misschar: %s%s%s\n\r",
                       s3, skill->miss_char, s1 );
        if ( skill->miss_vict && skill->miss_vict[0] != '\0' )
            ch_printf( ch, "Missvict: %s%s%s\n\r",
                       s3, skill->miss_vict, s1 );
        if ( skill->miss_room && skill->miss_room[0] != '\0' )
            ch_printf( ch, "Missroom: %s%s%s\n\r",
                       s3, skill->miss_room, s1 );
        if ( skill->die_char && skill->die_char[0] != '\0' )
            ch_printf( ch, "Diechar : %s%s%s\n\r",
                       s3, skill->die_char, s1 );
        if ( skill->die_vict && skill->die_vict[0] != '\0' )
            ch_printf( ch, "Dievict : %s%s%s\n\r",
                       s3, skill->die_vict, s1 );
        if ( skill->die_room && skill->die_room[0] != '\0' )
            ch_printf( ch, "Dieroom : %s%s%s\n\r",
                       s3, skill->die_room, s1 );
        if ( skill->imm_char && skill->imm_char[0] != '\0' )
            ch_printf( ch, "Immchar : %s%s%s\n\r",
                       s3, skill->imm_char, s1 );
        if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
            ch_printf( ch, "Immvict : %s%s%s\n\r",
                       s3, skill->imm_vict, s1 );
        if ( skill->imm_room && skill->imm_room[0] != '\0' )
            ch_printf( ch, "Immroom : %s%s%s\n\r",
                       s3, skill->imm_room, s1 );
        if ( skill->abs_char && skill->abs_char[0] != '\0' )
            ch_printf( ch, "Abschar : %s%s%s\n\r",
                       s3, skill->abs_char, s1 );
        if ( skill->abs_vict && skill->abs_vict[0] != '\0' )
            ch_printf( ch, "Absvict : %s%s%s\n\r",
                       s3, skill->abs_vict, s1 );
        if ( skill->abs_room && skill->abs_room[0] != '\0' )
            ch_printf( ch, "Absroom : %s%s%s\n\r",
                       s3, skill->abs_room, s1 );
        if ( skill->corpse_string && skill->corpse_string[0] != '\0' )
        {
            ch_printf( ch, "Corpse  : %s%s%s\n\r",
                       s3, skill->corpse_string, s1 );
            ch_printf( ch, "DecayLev: %s%d%s\n\r",
                       s3, skill->corpse_stage, s1 );
        }
        if ( skill->type != SKILL_HERB )
        {
            for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	    {
                if ( skill->skill_level[iClass] != LEVEL_IMMORTAL )
		    sprintf(buf, "%s%3.3s%s) lvl: %s%2d%s max: %s%2d%s%% mana: %s%2d%s bts: %s%2d%s",
			    s3, class_table[iClass]->who_name, s1,
			    s2, skill->skill_level[iClass], s1,
			    s4, skill->skill_adept[iClass], s1,
			    s2, skill->class_mana[iClass], s1,
			    s4, skill->class_beats[iClass], s1);
		else
		    sprintf(buf, "%3.3s) lvl: %2d max: %2d%% mana: %2d bts: %2d",
			    class_table[iClass]->who_name,
			    skill->skill_level[iClass],
			    skill->skill_adept[iClass],
			    skill->class_mana[iClass],
			    skill->class_beats[iClass]);

                if ( iClass < MAX_CLASS )
                {
                    if ( iClass % 2 == 1 )
                        strcat(buf, "\n\r" );
                    else
                        strcat(buf, "   " );
                }
                send_to_char( buf, ch );
            }
        }
        send_to_char( "\n\r", ch );
    }
    return;
}

/*
 * Set a skill's attributes or what skills a player has.
 * High god command, with support for creating skills/spells/herbs/etc
 */
void do_sset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (!str_cmp(arg1, "list"))
    {
        int x;

        if (!str_prefix(arg2, "affecttypes"))
        {
            send_to_char("Spell affect types:\n\r", ch);
            for (x=0;x<MAX_APPLY_TYPE;x++)
            {
                ch_printf(ch, "%-15s", a_types[x]);
                if (x%5==4)
                    send_to_char("\n\r", ch);
            }
            send_to_char("\n\r", ch);
            return;
        }
        if (!str_prefix(arg2, "affectedby"))
        {
            send_to_char("Affectedby bits:\n\r", ch);
            for (x=0;x<32;x++)
            {
                ch_printf(ch, "%-15s", a_flags[x]);
                if (x%5==4)
                    send_to_char("\n\r", ch);
            }
            send_to_char("\n\r", ch);
            return;
        }
        if (!str_cmp(arg2, "spellflags"))
        {
            send_to_char("Spell flags:\n\r", ch);
            for ( x = 0; x < sizeof(spell_flag) / sizeof(spell_flag[0]); x++ )
            {
                ch_printf(ch, "%-15s", spell_flag[x]);
                if (x%5==4)
                    send_to_char("\n\r", ch);
            }
            send_to_char("\n\r", ch);
            return;
        }
        do_sset(ch, "");
        return;
    }

    if ( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "Syntax: sset <victim> <skill> <value>\n\r",	ch );
        send_to_char( "or:     sset <victim> all     <value>\n\r",	ch );
        if ( get_trust(ch) > LEVEL_GOD )
        {
            send_to_char( "or:     sset save skill table\n\r",		ch );
            send_to_char( "or:     sset save herb table\n\r",		ch );
            send_to_char( "or:     sset create skill 'new skill'\n\r",	ch );
            send_to_char( "or:     sset copy skill <sn> 'new skill'\n\r",	ch );
            send_to_char( "or:     sset create herb 'new herb'\n\r",	ch );
        }
        if ( get_trust(ch) > LEVEL_GOD )
        {
            send_to_char( "or:     sset <sn>     <field> <value>\n\r",	ch );
            send_to_char( "or:     sset list affecttypes\n\r",          ch );
            send_to_char( "or:     sset list affectedby\n\r",           ch );
            send_to_char( "or:     sset list spellflags\n\r",           ch );
            send_to_char( "\n\rField being one of:\n\r",		ch );
            send_to_char( "  name code target minpos slot mana beats dammsg wearoff guild minlevel\n\r", ch );
            send_to_char( "  type damtype acttype classtype powertype flags dice value difficulty affect\n\r", ch );
            send_to_char( "  rmaffect level adept hit miss die imm (char/vict/room) corpse decay\n\r", ch );
            send_to_char( "  components teachers wearoffroom wearoffsoon wearoffsoonroom\n\r", ch );
            send_to_char( "  partstartchar partstartroom partendchar partendvict partendroom\n\r", ch);
	    send_to_char( "  partendcaster partmisschar partmissroom partabortchar\n\r", ch);
            send_to_char( "  cmana cbeats amana abeats\n\r", ch);
            send_to_char( "Affect having the fields: <location> <modfifier> [duration] [bitvector]\n\r", ch );

            send_to_char( "(See AFFECTTYPES for location, and AFFECTEDBY for bitvector)\n\r", ch );
        }
        send_to_char( "Skill being any skill or spell.\n\r",		ch );
        return;
    }

    if ( get_trust(ch) > LEVEL_GOD )
    {
        if (!str_cmp( arg1, "save" ) &&
            !str_cmp( argument, "table" ) )
        {
            if ( !str_cmp( arg2, "skill" ) )
            {
                send_to_char( "Saving skill table...\n\r", ch );
                save_skill_table();
                save_classes();
                return;
            }
            if ( !str_cmp( arg2, "herb" ) )
            {
                send_to_char( "Saving herb table...\n\r", ch );
                save_herb_table();
                return;
            }
        }
        if ( (!str_cmp( arg1, "create" ) || !str_cmp( arg1, "copy" )) &&
             (!str_cmp( arg2, "skill" ) || !str_cmp( arg2, "herb" )) )
        {
            SKILLTYPE *skill;
            sh_int type = SKILL_UNKNOWN;

            if ( !str_cmp( arg2, "herb" ) )
            {
                type = SKILL_HERB;
                if ( top_herb >= MAX_HERB )
                {
                    ch_printf( ch, "The current top herb is %d, which is the maximum.  "
                               "To add more herbs,\n\rMAX_HERB will have to be "
                               "raised in mud.h, and the mud recompiled.\n\r",
                               top_sn );
                    return;
                }
            }
            else
                if ( top_sn >= MAX_SKILL )
                {
                    ch_printf( ch, "The current top sn is %d, which is the maximum.  "
                               "To add more skills,\n\rMAX_SKILL will have to be "
                               "raised in mud.h, and the mud recompiled.\n\r",
                               top_sn );
                    return;
                }

            CREATE( skill, SKILLTYPE, 1 );
            if ( !str_cmp( arg1, "copy" ) )
            {
                char arg3 [MAX_INPUT_LENGTH];
                argument = one_argument( argument, arg3 );

                if ( arg3[0] == 'h' )
                    sn = atoi( arg3+1 );
                else
                    sn = atoi( arg3 );

                if (!IS_VALID_SN(sn))
                {
                    send_to_char("That is not a valid sn.\n\r", ch);
                    return;
                }

                /* copy it here */
            }
            else
            {
                int x;

                skill->noun_damage = str_dup( "" );
                skill->msg_off = str_dup( "" );
                skill->msg_off_room = str_dup( "" );
                skill->msg_off_soon = str_dup( "" );
                skill->msg_off_soon_room = str_dup( "" );
                skill->spell_fun = spell_smaug;
                skill->type = type;

                for (x = 0; x < MAX_CLASS; x++)
                {
                    skill->skill_level[x] = LEVEL_IMMORTAL;
                    skill->skill_adept[x] = 95;
                }
            }
            skill->name = str_dup( argument );

            if ( type == SKILL_HERB )
            {
                int max, x;

                herb_table[top_herb++] = skill;
                for ( max = x = 0; x < top_herb-1; x++ )
                    if ( herb_table[x] && herb_table[x]->slot > max )
                        max = herb_table[x]->slot;
                skill->slot = max+1;
            }
            else
                skill_table[top_sn++] = skill;

            ch_printf(ch, "Done, spell is sn %d.\n\r", top_sn-1);
            return;
        }
    }

    if ( arg1[0] == 'h' )
        sn = atoi( arg1+1 );
    else
        sn = atoi( arg1 );

    if (!IS_VALID_SN(sn))
    {
        send_to_char("That is not a valid sn.\n\r", ch);
        return;
    }

    if ( get_trust(ch) > LEVEL_GOD
         && ((arg1[0] == 'h' && is_number(arg1+1) && (sn=atoi(arg1+1))>=0)
             ||  (is_number(arg1) && (sn=atoi(arg1)) >= 0)) )
    {
        SKILLTYPE *skill;

        if ( arg1[0] == 'h' )
        {
            if ( sn >= top_herb )
            {
                send_to_char( "Herb number out of range.\n\r", ch );
                return;
            }
            skill = herb_table[sn];
        }
        else
        {
            if ( (skill=get_skilltype(sn)) == NULL )
            {
                send_to_char( "Skill number out of range.\n\r", ch );
                return;
            }
            sn %= 1000;
        }

        if ( !str_cmp( arg2, "difficulty" ) )
        {
            skill->difficulty = atoi( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "participants" ) )
        {
            skill->participants = atoi( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "damtype" ) )
        {
            int x = get_sdamage( argument );

            if ( x == -1 )
                send_to_char( "Not a spell damage type.\n\r", ch );
            else
            {
                SET_SDAM( skill, x );
                send_to_char( "Ok.\n\r", ch );
            }
            return;
        }
        if ( !str_cmp( arg2, "acttype" ) )
        {
            int x = get_saction( argument );

            if ( x == -1 )
                send_to_char( "Not a spell action type.\n\r", ch );
            else
            {
                SET_SACT( skill, x );
                send_to_char( "Ok.\n\r", ch );
            }
            return;
        }
        if ( !str_cmp( arg2, "classtype" ) )
        {
            int x = get_sch_class( argument );

            if ( x == -1 )
                send_to_char( "Not a spell class type.\n\r", ch );
            else
            {
                SET_SCLA( skill, x );
                send_to_char( "Ok.\n\r", ch );
            }
            return;
        }
        if ( !str_cmp( arg2, "powertype" ) )
        {
            int x = get_spower( argument );

            if ( x == -1 )
                send_to_char( "Not a spell power type.\n\r", ch );
            else
            {
                SET_SPOW( skill, x );
                send_to_char( "Ok.\n\r", ch );
            }
            return;
        }
        if ( !str_cmp( arg2, "savetype" ) )
        {
            int x = get_ssave( argument );

            if ( x == -1 )
                send_to_char( "Not a spell save type.\n\r", ch );
            else
            {
                SET_SSAV( skill, x );
                send_to_char( "Ok.\n\r", ch );
            }
            return;
        }
        if ( !str_cmp( arg2, "flags" ) )
        {
            int x = get_sflag( argument );

            if ( x == -1 )
                send_to_char( "Not a spell flag.\n\r", ch );
            else
            {
                TOGGLE_BIT( skill->flags, 1 << x );
                send_to_char( "Ok.\n\r", ch );
            }
            return;
        }

        if ( !str_cmp( arg2, "code" ) )
        {
            SPELL_FUN *spellfun;
            DO_FUN    *dofun;

            if ( (spellfun=spell_function(argument)) != spell_notfound )
            {
                skill->spell_fun = spellfun;
                skill->skill_fun = NULL;
            }
            else
                if ( (dofun=skill_function(argument)) != skill_notfound )
                {
                    skill->skill_fun = dofun;
                    skill->spell_fun = NULL;
                }
                else
                {
                    send_to_char( "Not a spell or skill.\n\r", ch );
                    return;
                }
            send_to_char( "Ok.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg2, "target" ) )
        {
            int x = get_starget( argument );

            if ( x == -1 )
                send_to_char( "Not a valid target type.\n\r", ch );
            else
            {
                skill->target = x;
                send_to_char( "Ok.\n\r", ch );
            }
            return;
        }
        if ( !str_cmp( arg2, "minpos" ) )
        {
            skill->minimum_position = URANGE( POS_DEAD, get_postype( argument ), POS_DRAG );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "minlevel" ) )
        {
            skill->min_level = URANGE( 1, atoi( argument ), MAX_LEVEL );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "slot" ) )
        {
            skill->slot = URANGE( 0, atoi( argument ), 30000 );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "mana" ) )
        {
            skill->min_mana = URANGE( 0, atoi( argument ), 2000 );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "amana" ) )
	{
	    sh_int cl;
	    for (cl = FIRST_CLASS; cl < MAX_CLASS; cl++)
		skill->class_mana[cl] = URANGE( 0, atoi( argument ), 2000 );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "beats" ) )
        {
            skill->beats = URANGE( 0, atoi( argument ), 288 );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "abeats" ) )
	{
	    sh_int cl;
	    for (cl = FIRST_CLASS; cl < MAX_CLASS; cl++)
		skill->class_beats[cl] = URANGE( 0, atoi( argument ), 288 );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "guild" ) )
        {
            skill->guild = atoi( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "value" ) )
        {
            skill->value = atoi( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "decay" ) )
        {
            skill->corpse_stage = atoi( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "type" ) )
        {
            skill->type = get_skill_tname( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "rmaffect" ) )
        {
            SMAUG_AFF *aff = skill->affects;
            SMAUG_AFF *aff_next;
            int num = atoi( argument );
            int cnt = 1;

            if ( !aff )
            {
                send_to_char( "This spell has no special affects to remove.\n\r", ch );
                return;
            }
            if ( num == 1 )
            {
                skill->affects = aff->next;
                DISPOSE( aff->duration );
                DISPOSE( aff->modifier );
                DISPOSE( aff );
                send_to_char( "Removed.\n\r", ch );
                return;
            }
            for ( ; aff; aff = aff->next )
            {
                if ( ++cnt == num && (aff_next=aff->next) != NULL )
                {
                    aff->next = aff_next->next;
                    DISPOSE( aff_next->duration );
                    DISPOSE( aff_next->modifier );
                    DISPOSE( aff_next );
                    send_to_char( "Removed.\n\r", ch );
                    return;
                }
            }
            send_to_char( "Not found.\n\r", ch );
            return;
        }
        /*
         * affect <location> <modifier> <duration> <bitvector>
         */
        if ( !str_cmp( arg2, "affect" ) )
        {
            char location[MAX_INPUT_LENGTH];
            char modifier[MAX_INPUT_LENGTH];
            char duration[MAX_INPUT_LENGTH];
            char bitvector[MAX_INPUT_LENGTH];
            int loc, bit, tmpbit;
            SMAUG_AFF *aff;

            argument = one_argument( argument, location );
            argument = one_argument( argument, modifier );
            argument = one_argument( argument, duration );

            if ( location[0] == '!' )
                loc = get_atype( location+1 ) + REVERSE_APPLY;
            else
                loc = get_atype( location );
            if ( (loc % REVERSE_APPLY) < 0
                 ||   (loc % REVERSE_APPLY) >= MAX_APPLY_TYPE )
            {
                send_to_char( "Unknown affect location.  See AFFECTTYPES.\n\r", ch );
                return;
            }
            bit = 0;
            while ( argument[0] != 0 )
            {
                argument = one_argument( argument, bitvector );
                if ( (tmpbit=get_aflag( bitvector )) == -1 )
                    ch_printf( ch, "Unknown bitvector: %s.  See AFFECTEDBY\n\r", bitvector );
                else
                    bit |= (1 << tmpbit);
            }
            CREATE( aff, SMAUG_AFF, 1 );
            if ( !str_cmp( duration, "0" ) )
                duration[0] = '\0';
            if ( !str_cmp( modifier, "0" ) )
                modifier[0] = '\0';
            aff->duration = str_dup( duration );
            aff->location = loc;
            aff->modifier = str_dup( modifier );
            aff->bitvector = bit;
            aff->next = skill->affects;
            skill->affects = aff;
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "level" ) )
        {
            char arg3[MAX_INPUT_LENGTH];
            int ch_class;

            argument = one_argument( argument, arg3 );
            ch_class = get_classtype( arg3 );
            if ( ch_class >= MAX_CLASS || ch_class < 0 )
                send_to_char( "Not a valid class.\n\r", ch );
            else
                skill->skill_level[ch_class] =
                    URANGE(0, atoi(argument), MAX_LEVEL);
            return;
        }
        if ( !str_cmp( arg2, "adept" ) )
        {
            char arg3[MAX_INPUT_LENGTH];
            int ch_class;

            argument = one_argument( argument, arg3 );
            ch_class = get_classtype( arg3 );
            if ( ch_class >= MAX_CLASS || ch_class < 0 )
                send_to_char( "Not a valid class.\n\r", ch );
            else
                skill->skill_adept[ch_class] =
                    URANGE(0, atoi(argument), 100);
            return;
        }
        if ( !str_cmp( arg2, "cmana" ) )
        {
            char arg3[MAX_INPUT_LENGTH];
            int ch_class;

            argument = one_argument( argument, arg3 );
            ch_class = get_classtype( arg3 );
            if ( ch_class >= MAX_CLASS || ch_class < 0 )
                send_to_char( "Not a valid class.\n\r", ch );
            else
                skill->class_mana[ch_class] =
                    URANGE(0, atoi(argument), 2000);
            return;
        }
        if ( !str_cmp( arg2, "cbeats" ) )
        {
            char arg3[MAX_INPUT_LENGTH];
            int ch_class;

            argument = one_argument( argument, arg3 );
            ch_class = get_classtype( arg3 );
            if ( ch_class >= MAX_CLASS || ch_class < 0 )
                send_to_char( "Not a valid class.\n\r", ch );
            else
                skill->class_beats[ch_class] =
                    URANGE(0, atoi(argument), 288);
            return;
        }
        if ( !str_cmp( arg2, "name" ) )
        {
            DISPOSE(skill->name);
            skill->name = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "dammsg" ) )
        {
            DISPOSE(skill->noun_damage);
            if ( !str_cmp( argument, "clear" ) )
                skill->noun_damage = str_dup( "" );
            else
                skill->noun_damage = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "wearoff" ) )
        {
            DISPOSE(skill->msg_off);
            if ( str_cmp( argument, "clear" ) )
                skill->msg_off = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "wearoffroom" ) )
        {
            DISPOSE(skill->msg_off_room);
            if ( str_cmp( argument, "clear" ) )
                skill->msg_off_room = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "wearoffsoon" ) )
        {
            DISPOSE(skill->msg_off_soon);
            if ( str_cmp( argument, "clear" ) )
                skill->msg_off_soon = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "wearoffsoonroom" ) )
        {
            DISPOSE(skill->msg_off_soon_room);
            if ( str_cmp( argument, "clear" ) )
                skill->msg_off_soon_room = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "hitchar" ) )
        {
            if ( skill->hit_char )
                DISPOSE(skill->hit_char);
            if ( str_cmp( argument, "clear" ) )
                skill->hit_char = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "hitvict" ) )
        {
            if ( skill->hit_vict )
                DISPOSE(skill->hit_vict);
            if ( str_cmp( argument, "clear" ) )
                skill->hit_vict = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "hitroom" ) )
        {
            if ( skill->hit_room )
                DISPOSE(skill->hit_room);
            if ( str_cmp( argument, "clear" ) )
                skill->hit_room = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "misschar" ) )
        {
            if ( skill->miss_char )
                DISPOSE(skill->miss_char);
            if ( str_cmp( argument, "clear" ) )
                skill->miss_char = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "missvict" ) )
        {
            if ( skill->miss_vict )
                DISPOSE(skill->miss_vict);
            if ( str_cmp( argument, "clear" ) )
                skill->miss_vict = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "missroom" ) )
        {
            if ( skill->miss_room )
                DISPOSE(skill->miss_room);
            if ( str_cmp( argument, "clear" ) )
                skill->miss_room = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "diechar" ) )
        {
            if ( skill->die_char )
                DISPOSE(skill->die_char);
            if ( str_cmp( argument, "clear" ) )
                skill->die_char = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "dievict" ) )
        {
            if ( skill->die_vict )
                DISPOSE(skill->die_vict);
            if ( str_cmp( argument, "clear" ) )
                skill->die_vict = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "dieroom" ) )
        {
            if ( skill->die_room )
                DISPOSE(skill->die_room);
            if ( str_cmp( argument, "clear" ) )
                skill->die_room = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "immchar" ) )
        {
            if ( skill->imm_char )
                DISPOSE(skill->imm_char);
            if ( str_cmp( argument, "clear" ) )
                skill->imm_char = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "immvict" ) )
        {
            if ( skill->imm_vict )
                DISPOSE(skill->imm_vict);
            if ( str_cmp( argument, "clear" ) )
                skill->imm_vict = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "immroom" ) )
        {
            if ( skill->imm_room )
                DISPOSE(skill->imm_room);
            if ( str_cmp( argument, "clear" ) )
                skill->imm_room = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "abschar" ) )
        {
            if ( skill->abs_char )
                DISPOSE(skill->abs_char);
            if ( str_cmp( argument, "clear" ) )
                skill->abs_char = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "absvict" ) )
        {
            if ( skill->abs_vict )
                DISPOSE(skill->abs_vict);
            if ( str_cmp( argument, "clear" ) )
                skill->abs_vict = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "absroom" ) )
        {
            if ( skill->abs_room )
                DISPOSE(skill->abs_room);
            if ( str_cmp( argument, "clear" ) )
                skill->abs_room = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "corpse" ) )
        {
            if ( skill->corpse_string )
                DISPOSE(skill->corpse_string);
            if ( str_cmp( argument, "clear" ) )
                skill->corpse_string = str_dup( argument );
            skill->corpse_stage = 0;
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "dice" ) )
        {
            if ( skill->dice )
                DISPOSE(skill->dice);
            if ( str_cmp( argument, "clear" ) )
                skill->dice = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "components" ) )
        {
            if ( skill->components )
                DISPOSE(skill->components);
            if ( str_cmp( argument, "clear" ) )
                skill->components = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "teachers" ) )
        {
            if ( skill->teachers )
                DISPOSE(skill->teachers);
            if ( str_cmp( argument, "clear" ) )
                skill->teachers = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "partstartchar" ) )
        {
            if ( skill->part_start_char )
                DISPOSE(skill->part_start_char);
            if ( str_cmp( argument, "clear" ) )
                skill->part_start_char = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "partstartroom" ) )
        {
            if ( skill->part_start_room )
                DISPOSE(skill->part_start_room);
            if ( str_cmp( argument, "clear" ) )
                skill->part_start_room = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "partendchar" ) )
        {
            if ( skill->part_end_char )
                DISPOSE(skill->part_end_char);
            if ( str_cmp( argument, "clear" ) )
                skill->part_end_char = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "partendvict" ) )
        {
            if ( skill->part_end_vict )
                DISPOSE(skill->part_end_vict);
            if ( str_cmp( argument, "clear" ) )
                skill->part_end_vict = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "partendroom" ) )
        {
            if ( skill->part_end_room )
                DISPOSE(skill->part_end_room);
            if ( str_cmp( argument, "clear" ) )
                skill->part_end_room = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "partendcaster" ) )
        {
            if ( skill->part_end_caster )
                DISPOSE(skill->part_end_caster);
            if ( str_cmp( argument, "clear" ) )
                skill->part_end_caster = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "partmisschar" ) )
        {
            if ( skill->part_miss_char )
                DISPOSE(skill->part_miss_char);
            if ( str_cmp( argument, "clear" ) )
                skill->part_miss_char = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "partmissroom" ) )
        {
            if ( skill->part_miss_room )
                DISPOSE(skill->part_miss_room);
            if ( str_cmp( argument, "clear" ) )
                skill->part_miss_room = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg2, "partabortchar" ) )
        {
            if ( skill->part_abort_char )
                DISPOSE(skill->part_abort_char);
            if ( str_cmp( argument, "clear" ) )
                skill->part_abort_char = str_dup( argument );
            send_to_char( "Ok.\n\r", ch );
            return;
        }

        do_sset( ch, "" );
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        if ( (sn = skill_lookup(arg1)) >= 0 )
        {
            sprintf(arg1, "%d %s %s", sn, arg2, argument);
            do_sset(ch, arg1);
        }
        else
            send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
        send_to_char( "No such skill or spell.\n\r", ch );
        return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( argument ) )
    {
        send_to_char( "Value must be numeric.\n\r", ch );
        return;
    }

    value = atoi( argument );
    if ( value < 0 || value > 100 )
    {
        send_to_char( "Value range is 0 to 100.\n\r", ch );
        return;
    }

    if ( fAll )
    {
        for ( sn = 0; sn < top_sn; sn++ )
        {
            /* Fix by Narn to prevent ssetting skills the player shouldn't have. */
            if ( skill_table[sn]->name
                 && ( CanUseSkill(victim, sn)
                      || value == 0 ) )
                victim->pcdata->learned[sn] = value;
        }
    }
    else
        victim->pcdata->learned[sn] = value;

    return;
}


void learn_from_success( CHAR_DATA *ch, int sn )
{
#if 0
    int adept, /*gain,*/ sklvl, learn, percent, chance;

    if ( IS_NPC(ch) || LEARNED(ch, sn) == 0 )
        return;
    adept = GET_ADEPT(ch,sn);
    sklvl = skill_table[sn]->skill_level[BestSkCl(ch, sn)];

    if ( sklvl == 0 )
        sklvl = GetMaxLevel(ch);
    if ( LEARNED(ch, sn) < adept )
    {
        chance = LEARNED(ch, sn) + (5 * skill_table[sn]->difficulty);
        percent = number_percent();
        if ( percent >= chance )
            learn = 2;
        else
            if ( chance - percent > 25 )
                return;
            else
                learn = 1;
        ch->pcdata->learned[sn] = UMIN( adept, LEARNED(ch, sn) + learn );
        ch_printf( ch, "You learn from your success.\n\r" );
        if ( LEARNED(ch, sn) == adept )	 /* fully learned! */
        {
            gain = 1000 * sklvl;
            if (IS_ACTIVE(ch, CLASS_MAGE)) gain = gain *5;  /* h, mage upgrade */
            set_char_color( AT_WHITE, ch );
            ch_printf( ch, "You are now an adept of %s!  You gain %d bonus experience!\n\r",
                       skill_table[sn]->name, gain );
        }
        else
        {
            gain = 20 * sklvl;
            if (IS_ACTIVE(ch, CLASS_MAGE)) gain = gain *6;  /* h, mage upgrade */
            if ( !ch->fighting && sn != gsn_hide && sn != gsn_sneak )
            {
                set_char_color( AT_WHITE, ch );
                ch_printf( ch, "You gain %d experience points from your success!\n\r", gain );
            }
        }
        gain_exp( ch, gain );
    }
#endif
}


void learn_from_failure(CHAR_DATA *ch, int sn)
{
    int adept, chance;

    if (IS_NPC(ch) || sn<0 || LEARNED(ch, sn) == 0)
        return;

    chance = LEARNED(ch, sn) + (5 * skill_table[sn]->difficulty);

    if (number_percent() < (chance/2))
        return;

    adept = GET_ADEPT(ch, sn);

    if (LEARNED(ch, sn) == adept)
        return;

    ch->pcdata->learned[sn] = UMIN( adept, LEARNED(ch, sn) + 1);

    if (LEARNED(ch, sn) == adept)
        send_to_char("You are now learned in this skill!\n\r", ch);
    else if (LEARNED(ch, sn) < adept && sn != gsn_drinking)
        send_to_char("You learn from your mistake.\n\r",ch);
}


void do_gouge( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    sh_int dam;
    int percent;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    if ( !IS_NPC(ch) && !LEARNED(ch, gsn_gouge) )
    {
        send_to_char("You do not yet know of this skill.\n\r", ch );
        return;
    }

    if ( ch->mount )
    {
        send_to_char( "You can't get close enough while mounted.\n\r", ch );
        return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
        send_to_char( "You aren't fighting anyone.\n\r", ch );
        return;
    }

    percent = number_percent( ) - (get_curr_lck(ch) - 13);

    if ( IS_NPC(ch) || percent < LEARNED(ch, gsn_gouge) )
    {
        dam = number_range( 1, ch->levels[BestSkCl(ch, gsn_gouge)] );
        global_retcode = damage( ch, victim, dam, gsn_gouge );
        if ( global_retcode == rNONE )
        {
            if ( !IS_AFFECTED( victim, AFF_BLIND ) )
            {
                af.type      = gsn_blindness;
                af.location  = APPLY_HITROLL;
                af.modifier  = -6;
                af.duration  = 3 + (ch->levels[BestSkCl(ch, gsn_gouge)] / 15);
                af.bitvector = AFF_BLIND;
                affect_to_char( victim, &af );
                act( AT_SKILL, "You can't see a thing!", victim, NULL, NULL, TO_CHAR );
            }
            WAIT_STATE( ch,     PULSE_VIOLENCE );
            WAIT_STATE( victim, PULSE_VIOLENCE );
            /* Taken out by request - put back in by Thoric
             * This is how it was designed.  You'd be a tad stunned
             * if someone gouged you in the eye.
             */
        }
        else
            if ( global_retcode == rVICT_DIED )
            {
                act( AT_BLOOD, "Your fingers plunge into your victim's brain, causing immediate death!",
                     ch, NULL, NULL, TO_CHAR );
            }
        if ( global_retcode != rCHAR_DIED && global_retcode != rBOTH_DIED )
            learn_from_success( ch, gsn_gouge );
    }
    else
    {
        spell_lag(ch, gsn_gouge);
        global_retcode = damage( ch, victim, 0, gsn_gouge );
        learn_from_failure( ch, gsn_gouge );
    }

    return;
}

void do_detrap( CHAR_DATA *ch, char *argument )
{
    char arg  [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *trap;
    int percent;
    bool found = FALSE;

    switch( ch->substate )
    {
    default:
        if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
        {
            send_to_char( "You can't concentrate enough for that.\n\r", ch );
            return;
        }
        argument = one_argument( argument, arg );
        if ( !IS_NPC(ch) && !LEARNED(ch, gsn_detrap) )
        {
            send_to_char("You do not yet know of this skill.\n\r", ch );
            return;
        }
        if ( arg[0] == '\0' )
        {
            send_to_char( "Detrap what?\n\r", ch );
            return;
        }
        if ( ms_find_obj(ch) )
            return;
        found = FALSE;
        if ( ch->mount )
        {
            send_to_char( "You can't do that while mounted.\n\r", ch );
            return;
        }
        if ( !ch->in_room->first_content )
        {
            send_to_char( "You can't find that here.\n\r", ch );
            return;
        }
        for ( obj = ch->in_room->first_content; obj; obj = obj->next_content )
        {
            if ( can_see_obj( ch, obj ) && nifty_is_name( arg, obj->name ) )
            {
                found = TRUE;
                break;
            }
        }
        if ( !found )
        {
            send_to_char( "You can't find that here.\n\r", ch );
            return;
        }
        act( AT_ACTION, "You carefully begin your attempt to remove a trap from $p...", ch, obj, NULL, TO_CHAR );
        act( AT_ACTION, "$n carefully attempts to remove a trap from $p...", ch, obj, NULL, TO_ROOM );
        ch->dest_buf = str_dup( obj->name );
        add_timer( ch, TIMER_DO_FUN,
                   UMAX(skill_table[gsn_detrap]->beats/SPELL_BEATS_PER_ROUND, 1),
                   do_detrap, 1 );
        return;
    case 1:
        if ( !ch->dest_buf )
        {
            send_to_char( "Your detrapping was interrupted!\n\r", ch );
            bug( "do_detrap: ch->dest_buf NULL!" );
            return;
        }
        strcpy( arg, (const char *)ch->dest_buf );
        DISPOSE( ch->dest_buf );
        ch->dest_buf = NULL;
        ch->substate = SUB_NONE;
        break;
    case SUB_TIMER_DO_ABORT:
        DISPOSE(ch->dest_buf);
        ch->substate = SUB_NONE;
        send_to_char( "You carefully stop what you were doing.\n\r", ch );
        return;
    }

    for ( obj = ch->in_room->first_content; obj; obj = obj->next_content )
    {
        if ( can_see_obj( ch, obj ) && nifty_is_name( arg, obj->name ) )
        {
            found = TRUE;
            break;
        }
    }
    if ( !found )
    {
        send_to_char( "You can't find that here.\n\r", ch );
        return;
    }
    if ( (trap = get_trap( obj )) == NULL )
    {
        send_to_char( "You find no trap on that.\n\r", ch );
        return;
    }

    percent  = number_percent( ) - ( ch->levels[BestSkCl(ch, gsn_detrap)] / 15 )
        - (get_curr_lck(ch) - 16);

    separate_obj(obj);
    if ( !IS_NPC(ch) || percent > LEARNED(ch, gsn_detrap) )
    {
        send_to_char( "Ooops!\n\r", ch );
        spring_trap( ch, trap );
        learn_from_failure( ch, gsn_detrap );
        return;
    }

    extract_obj( trap );

    send_to_char( "You successfully remove a trap.\n\r", ch );
    learn_from_success( ch, gsn_detrap );
    return;
}

void do_dig( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *startobj;
    bool found, shovel;
    EXIT_DATA *pexit;

    switch( ch->substate )
    {
    default:
        if ( IS_NPC(ch)  && IS_AFFECTED( ch, AFF_CHARM ) )
        {
            send_to_char( "You can't concentrate enough for that.\n\r", ch );
            return;
        }
        if ( ch->mount )
        {
            send_to_char( "You can't do that while mounted.\n\r", ch );
            return;
        }
        one_argument( argument, arg );
        if ( arg[0] != '\0' )
        {
            if ( ( pexit = find_door( ch, arg, TRUE ) ) == NULL
                 &&     get_dir(arg) == -1 )
            {
                send_to_char( "What direction is that?\n\r", ch );
                return;
            }
            if ( pexit )
            {
                if ( !IS_SET(pexit->exit_info, EX_DIG)
                     &&   !IS_SET(pexit->exit_info, EX_CLOSED) )
                {
                    send_to_char( "There is no need to dig out that exit.\n\r", ch );
                    return;
                }
            }
        }
        else
        {
            switch( ch->in_room->sector_type )
            {
            case SECT_CITY:
            case SECT_INSIDE:
                send_to_char( "The floor is too hard to dig through.\n\r", ch );
                return;
            case SECT_WATER_SWIM:
            case SECT_WATER_NOSWIM:
            case SECT_UNDERWATER:
                send_to_char( "You cannot dig here.\n\r", ch );
                return;
            case SECT_AIR:
                send_to_char( "What?  In the air?!\n\r", ch );
                return;
            }
        }
        add_timer( ch, TIMER_DO_FUN,
                   UMAX(skill_table[gsn_dig]->beats/SPELL_BEATS_PER_ROUND, 1),
                   do_dig, 1);
        ch->dest_buf = str_dup( arg );
        send_to_char( "You begin digging...\n\r", ch );
        act( AT_SKILL, "$n begins digging...", ch, NULL, NULL, TO_ROOM );
        return;

    case 1:
        if ( !ch->dest_buf )
        {
            send_to_char( "Your digging was interrupted!\n\r", ch );
            act( AT_SKILL, "$n's digging was interrupted!", ch, NULL, NULL, TO_ROOM );
            bug( "do_dig: dest_buf NULL" );
            return;
        }
        strcpy( arg, (const char *)ch->dest_buf );
        DISPOSE( ch->dest_buf );
        break;

    case SUB_TIMER_DO_ABORT:
        DISPOSE( ch->dest_buf );
        ch->substate = SUB_NONE;
        send_to_char( "You stop digging...\n\r", ch );
        act( AT_SKILL, "$n stops digging...", ch, NULL, NULL, TO_ROOM );
        return;
    }

    ch->substate = SUB_NONE;

    /* not having a shovel makes it harder to succeed */
    shovel = FALSE;
    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
        if ( obj->item_type == ITEM_SHOVEL )
        {
            shovel = TRUE;
            break;
        }

    /* dig out an EX_DIG exit... */
    if ( arg[0] != '\0' )
    {
        if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL
             &&     IS_SET( pexit->exit_info, EX_DIG )
             &&     IS_SET( pexit->exit_info, EX_CLOSED ) )
        {
            /* 4 times harder to dig open a passage without a shovel */
            if ( (number_percent() * (shovel ? 1 : 4)) <
                 (IS_NPC(ch) ? 80 : LEARNED(ch, gsn_dig)) )
            {
                REMOVE_BIT( pexit->exit_info, EX_CLOSED );
                send_to_char( "You dig open a passageway!\n\r", ch );
                act( AT_SKILL, "$n digs open a passageway!", ch, NULL, NULL, TO_ROOM );
                learn_from_success( ch, gsn_dig );
                return;
            }
        }
        learn_from_failure( ch, gsn_dig );
        send_to_char( "Your dig did not discover any exit...\n\r", ch );
        act( AT_SKILL, "$n's dig did not discover any exit...", ch, NULL, NULL, TO_ROOM );
        return;
    }

    startobj = ch->in_room->first_content;
    found = FALSE;

    for ( obj = startobj; obj; obj = obj->next_content )
    {
        /* twice as hard to find something without a shovel */
        if ( IS_OBJ_STAT( obj, ITEM_BURRIED )
             &&  (number_percent() * (shovel ? 1 : 2)) <
             (IS_NPC(ch) ? 80 : LEARNED(ch, gsn_dig)) )
        {
            found = TRUE;
            break;
        }
    }

    if ( !found )
    {
        send_to_char( "Your dig uncovered nothing.\n\r", ch );
        act( AT_SKILL, "$n's dig uncovered nothing.", ch, NULL, NULL, TO_ROOM );
        learn_from_failure( ch, gsn_dig );
        return;
    }

    separate_obj(obj);
    REMOVE_BIT( obj->extra_flags, ITEM_BURRIED );
    act( AT_SKILL, "Your dig uncovered $p!", ch, obj, NULL, TO_CHAR );
    act( AT_SKILL, "$n's dig uncovered $p!", ch, obj, NULL, TO_ROOM );
    learn_from_success( ch, gsn_dig );
    if ( obj->item_type == ITEM_CORPSE_PC
         ||   obj->item_type == ITEM_CORPSE_NPC )
        adjust_favor( ch, 14, 1 );

    return;
}


void do_search( CHAR_DATA *ch, char *argument )
{
    char arg  [MAX_INPUT_LENGTH];
    OBJ_DATA *obj = NULL;
    OBJ_DATA *container;
    OBJ_DATA *startobj;
    int percent, door;
    bool found, room;

    door = -1;
    switch( ch->substate )
    {
    default:
        if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
        {
            send_to_char( "You can't concentrate enough for that.\n\r", ch );
            return;
        }
        if ( ch->mount )
        {
            send_to_char( "You can't do that while mounted.\n\r", ch );
            return;
        }
        argument = one_argument( argument, arg );
        if ( arg[0] != '\0' && (door = get_door( arg )) == -1 )
        {
            container = get_obj_here( ch, arg );
            if ( !container )
            {
                send_to_char( "You can't find that here.\n\r", ch );
                return;
            }
            if ( container->item_type != ITEM_CONTAINER )
            {
                send_to_char( "You can't search in that!\n\r", ch );
                return;
            }
            if ( IS_SET(container->value[1], CONT_CLOSED) )
            {
                send_to_char( "It is closed.\n\r", ch );
                return;
            }
        }
        add_timer( ch, TIMER_DO_FUN,
                   UMAX(skill_table[gsn_search]->beats/SPELL_BEATS_PER_ROUND, 1),
                   do_search, 1 );
        send_to_char( "You begin your search...\n\r", ch );
        ch->dest_buf = str_dup( arg );
        return;

    case 1:
        if ( !ch->dest_buf )
        {
            send_to_char( "Your search was interrupted!\n\r", ch );
            bug( "do_search: dest_buf NULL" );
            return;
        }
        strcpy( arg, (const char *)ch->dest_buf );
        DISPOSE( ch->dest_buf );
        break;
    case SUB_TIMER_DO_ABORT:
        DISPOSE( ch->dest_buf );
        ch->substate = SUB_NONE;
        send_to_char( "You stop your search...\n\r", ch );
        return;
    }
    ch->substate = SUB_NONE;
    if ( arg[0] == '\0' )
    {
        room = TRUE;
        startobj = ch->in_room->first_content;
    }
    else
    {
        if ( (door = get_door( arg )) != -1 )
            startobj = NULL;
        else
        {
            container = get_obj_here( ch, arg );
            if ( !container )
            {
                send_to_char( "You can't find that here.\n\r", ch );
                return;
            }
            startobj = container->first_content;
        }
    }

    found = FALSE;

    if ( (!startobj && door == -1) || IS_NPC(ch) )
    {
        send_to_char( "You find nothing.\n\r", ch );
        learn_from_failure( ch, gsn_search );
        return;
    }

    percent  = number_percent( ) + number_percent( ) -
        ( ch->levels[BestSkCl(ch, gsn_search)] / 10 );

    if ( door != -1 )
    {
        EXIT_DATA *pexit;

        if ( (pexit = get_exit( ch->in_room, door )) != NULL
             &&   IS_SET( pexit->exit_info, EX_SECRET )
             &&   IS_SET( pexit->exit_info, EX_xSEARCHABLE )
             &&   percent < (IS_NPC(ch) ? 80 : LEARNED(ch, gsn_search)) )
        {
            act( AT_SKILL, "Your search reveals the $d!", ch, NULL, pexit->keyword, TO_CHAR );
            act( AT_SKILL, "$n finds the $d!", ch, NULL, pexit->keyword, TO_ROOM );
            REMOVE_BIT( pexit->exit_info, EX_SECRET );
            learn_from_success( ch, gsn_search );
            return;
        }
    }
    else
        for ( obj = startobj; obj; obj = obj->next_content )
        {
            if ( IS_OBJ_STAT( obj, ITEM_HIDDEN )
                 &&   percent < LEARNED(ch, gsn_search) )
            {
                found = TRUE;
                break;
            }
        }

    if ( !found )
    {
        send_to_char( "You find nothing.\n\r", ch );
        learn_from_failure( ch, gsn_search );
        return;
    }

    separate_obj(obj);
    REMOVE_BIT( obj->extra_flags, ITEM_HIDDEN );
    act( AT_SKILL, "Your search reveals $p!", ch, obj, NULL, TO_CHAR );
    act( AT_SKILL, "$n finds $p!", ch, obj, NULL, TO_ROOM );
    learn_from_success( ch, gsn_search );
    return;
}


void do_steal( CHAR_DATA *ch, char *argument )
{
    char buf  [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim, *mst;
    OBJ_DATA *obj;
    int percent;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ch->mount )
    {
        send_to_char( "You can't do that while mounted.\n\r", ch );
        return;
    }

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Steal what from whom?\n\r", ch );
        return;
    }

    if ( ms_find_obj(ch) )
        return;

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "That's pointless.\n\r", ch );
        return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) &&
	 !IS_IMMORTAL(ch) )
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "A magical force interrupts you.\n\r", ch );
        return;
    }

    /* Disabled stealing among players because of complaints naked avatars were
     running around stealing eq from equipped pkillers. -- Narn
     */
    /*    if ( check_illegal_psteal( ch, victim ) )
     {
     send_to_char( "You can't steal from that player.\n\r", ch );
     return;
     }
     */
    if ( !IS_IMMORTAL( ch ) && !IS_NPC( ch ) && !IS_NPC( victim ) )
    {
        set_char_color( AT_IMMORT, ch );
        send_to_char( "The gods forbid theft between players.\n\r", ch );
        return;
    }

    spell_lag(ch, gsn_steal);
    percent  = number_percent( ) + ( IS_AWAKE(victim) ? 10 : -50 )
        - (get_curr_lck(ch) - 15) + (get_curr_lck(victim) - 13);

    /* Changed the level check, made it 10 levels instead of five and made the
     victim not attack in the case of a too high level difference.  This is
     to allow mobprogs where the mob steals eq without having to put level
     checks into the progs.  Also gave the mobs a 10% chance of failure.
     */
    if( GetMaxLevel(ch) + 10 < GetMaxLevel(victim) )
    {
        send_to_char( "You really don't want to try that!\n\r", ch );
        return;
    }

    if ( victim->position == POS_FIGHTING
         ||   percent > ( IS_NPC(ch) ? 90 : LEARNED(ch, gsn_steal) ) )
    {
        /*
         * Failure.
         */
        send_to_char( "Oops...\n\r", ch );
        act( AT_ACTION, "$n tried to steal from you!\n\r", ch, NULL, victim, TO_VICT    );
        act( AT_ACTION, "$n tried to steal from $N.\n\r",  ch, NULL, victim, TO_NOTVICT );

        sprintf( buf, "%s is a bloody thief!", ch->name );
        do_gossip( victim, buf );

        learn_from_failure( ch, gsn_steal );
        if ( !IS_NPC(ch) )
        {
            if ( legal_loot( ch, victim ) )
            {
                global_retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
            }
            else
            {
                /* log_string( buf ); */
                if ( IS_NPC( ch ) )
                {
                    if ( (mst = ch->master) == NULL )
                        return;
                }
                else
                    mst = ch;
                if ( IS_NPC( mst ) )
                    return;
                if ( !IS_SET(mst->act, PLR_THIEF) )
                {
                    SET_BIT(mst->act, PLR_THIEF);
                    set_char_color( AT_WHITE, ch );
                    send_to_char( "A strange feeling grows deep inside you, and a tingle goes up your spine...\n\r", ch );
                    set_char_color( AT_IMMORT, ch );
                    send_to_char( "A deep voice booms inside your head, 'Thou shall now be known as a lowly thief!'\n\r", ch );
                    set_char_color( AT_WHITE, ch );
                    send_to_char( "You feel as if your soul has been revealed for all to see.\n\r", ch );
                    save_char_obj( mst );
                }
            }
        }

        return;
    }

    if ( !str_cmp( arg1, "coin"  ) ||
         !str_cmp( arg1, "coins" ) )
    {
        int amount, type=CURR_GOLD;

        for (amount=FIRST_CURR;amount<=LAST_CURR;amount++)
        {
            type = number_range(FIRST_CURR, LAST_CURR);
            if (GET_MONEY(victim,type))
                break;
        }
        amount = (int) (GET_MONEY(victim,type) * number_range(1, 10) / 100);
        if ( amount <= 0 )
        {
            send_to_char( "You couldn't get any money.\n\r", ch );
            learn_from_failure( ch, gsn_steal );
            return;
        }

        GET_MONEY(ch,type)     += amount;
        GET_MONEY(victim,type) -= amount;
        ch_printf( ch, "Aha!  You got %d %s coins.\n\r", amount, curr_types[type] );
        learn_from_success( ch, gsn_steal );
        return;
    }

    if ( ( obj = get_obj_carry( victim, arg1 ) ) == NULL )
    {
        send_to_char( "You can't seem to find it.\n\r", ch );
        learn_from_failure( ch, gsn_steal );
        return;
    }

    if ( !can_drop_obj( ch, obj )
         ||   IS_OBJ_STAT(obj, ITEM_INVENTORY)
         ||	 IS_OBJ_STAT(obj, ITEM_PROTOTYPE)
         ||   item_ego(obj) > char_ego(ch) )
    {
        send_to_char( "You can't manage to pry it away.\n\r", ch );
        learn_from_failure( ch, gsn_steal );
        return;
    }

    if ( carry_n(ch) + (get_obj_number(obj)/obj->count) > can_carry_n( ch ) )
    {
        send_to_char( "You have your hands full.\n\r", ch );
        learn_from_failure( ch, gsn_steal );
        return;
    }

    if ( carry_w(ch) + (get_obj_weight(obj)/obj->count) > can_carry_w( ch ) )
    {
        send_to_char( "You can't carry that much weight.\n\r", ch );
        learn_from_failure( ch, gsn_steal );
        return;
    }

    separate_obj( obj );
    obj_from_char( obj );
    obj_to_char( obj, ch );
    send_to_char( "Ok.\n\r", ch );
    learn_from_success( ch, gsn_steal );
    adjust_favor( ch, 9, 1 );
    return;
}


void do_backstab( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't do that right now.\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( ch->mount )
    {
        send_to_char( "You can't get close enough while mounted.\n\r", ch );
        return;
    }

    if ( arg[0] == '\0' )
    {
        send_to_char( "Backstab whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "How can you sneak up on yourself?\n\r", ch );
        return;
    }

    if ( is_safe( ch, victim ) )
        return;

    /* Added stabbing weapon. -Narn */
    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
         ||   ( obj->value[3] != DAM_STAB && obj->value[3] != DAM_PIERCE && obj->value[3] != DAM_STING) )
    {
        send_to_char( "You need to wield a piercing or stabbing weapon.\n\r", ch );
        return;
    }

    percent = number_percent() - (get_curr_lck(victim) - 14)
        + (get_curr_lck(ch) - 13);

    if ( !IS_NPC(victim) &&
         CanUseSkill(victim, gsn_avoid_back_attack) &&
         percent < LEARNED(victim, gsn_avoid_back_attack) )
    {
       act( AT_DANGER, "You avoid a back attack from $N!", victim, NULL,
            ch, TO_CHAR);
       act( AT_DANGER, "$n avoided your attack!", victim, NULL,
            ch, TO_VICT);

       learn_from_success( victim, gsn_avoid_back_attack );
       return;
    }

    percent = number_percent( ) - (get_curr_lck(ch) - 14)
        + (get_curr_lck(victim) - 13);

    check_attacker( ch, victim );
    check_illegal_pk( ch, victim );
    spell_lag(ch, gsn_backstab);
    if ( !IS_AWAKE(victim)
         ||   IS_NPC(ch)
         ||   percent < LEARNED(ch, gsn_backstab) )
    {
        global_retcode = multi_hit( ch, victim, gsn_backstab );
        adjust_favor( ch, 10, 1 );
    }
    else
    {
        learn_from_failure( ch, gsn_backstab );
        global_retcode = damage( ch, victim, 0, gsn_backstab );
    }
    return;
}

void do_meditate( CHAR_DATA *ch, char *argument )
{
    if (is_affected(ch, gsn_mindwipe) ||
        is_affected(ch, gsn_mindwipe))
    {
        send_to_char("Der, what is that?\n\r", ch);
        return;
    }

    if ( ch->mount )
    {
        send_to_char( "I doubt your mount would hold still enough.\n\r", ch );
        return;
    }

    if ( !IS_AWAKE(ch) )
    {
        send_to_char( "You seem too tired.\n\r", ch );
        return;
    }

    spell_lag(ch, gsn_meditate);
    if ( IS_NPC(ch) || number_percent() <
         LEARNED(ch, gsn_meditate) )
    {
        send_to_char("You sit down and begin humming.\n\r", ch);
        if (GET_LEVEL(ch, CLASS_PSIONIST)<10)
            send_to_char("(Remember: to visit the psionist guild, type psitrain when meditating)\n\r", ch);
        GET_POS(ch) = POS_MEDITATING;
    }
    else learn_from_failure( ch, gsn_meditate );
    return;
}


void do_rescue( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;
    int percent;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Rescue whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "How about fleeing instead?\n\r", ch );
        return;
    }

    if ( ch->mount )
    {
        send_to_char( "You can't do that while mounted.\n\r", ch );
        return;
    }

    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
        send_to_char( "Doesn't need your help!\n\r", ch );
        return;
    }

    if ( !ch->fighting )
    {
        send_to_char( "Too late...\n\r", ch );
        return;
    }

    if ( ( fch = who_fighting( victim) ) == NULL )
    {
        send_to_char( "They are not fighting right now.\n\r", ch );
        return;
    }

    percent = number_percent( ) - (get_curr_lck(ch) - 14)
        - (get_curr_lck(victim) - 16);

    spell_lag(ch, gsn_rescue);
    if ( !IS_NPC(ch) && percent > LEARNED(ch, gsn_rescue) )
    {
        send_to_char( "You fail the rescue.\n\r", ch );
        act( AT_SKILL, "$n tries to rescue you!", ch, NULL, victim, TO_VICT   );
        act( AT_SKILL, "$n tries to rescue $N!", ch, NULL, victim, TO_NOTVICT );
        learn_from_failure( ch, gsn_rescue );
        return;
    }

    act( AT_SKILL, "You rescue $N!",  ch, NULL, victim, TO_CHAR    );
    act( AT_SKILL, "$n rescues you!", ch, NULL, victim, TO_VICT    );
    act( AT_SKILL, "$n moves in front of $N!",  ch, NULL, victim, TO_NOTVICT );

    learn_from_success( ch, gsn_rescue );
    adjust_favor( ch, 8, 1 );
    stop_fighting( fch, FALSE );
    stop_fighting( victim, FALSE );
    if ( ch->fighting )
        stop_fighting( ch, FALSE );

    /* check_killer( ch, fch ); */
    set_fighting( ch, fch );
    set_fighting( fch, ch );
    return;
}

void kick_messages(CHAR_DATA *ch, CHAR_DATA *victim,
                   int hard_damage)
{
    int i, soft_damage;

    soft_damage = hard_damage;
    if (IS_RESIS(victim, RIS_BLUNT))
       soft_damage /= 2;
    if (IS_AFFECTED(victim, AFF_SANCTUARY))
       soft_damage /= 2;
    if (IS_IMMUNE(victim, RIS_BLUNT))
       soft_damage = 0;

    switch(GET_RACE(victim)) {
    case RACE_HUMAN:
    case RACE_ELVEN:
    case RACE_DWARF:
    case RACE_DROW:
    case RACE_ORC:
    case RACE_LYCANTH:
    case RACE_TROLL:
    case RACE_DEMON:
    case RACE_DEVIL:
    case RACE_MFLAYER:
    case RACE_ASTRAL:
    case RACE_PATRYN:
    case RACE_SARTAN:
    case RACE_DRAAGDIM:
    case RACE_GOLEM:
    case RACE_TROGMAN:
    case RACE_LIZARDMAN:
    case RACE_HALF_ELVEN:
    case RACE_HALF_OGRE:
    case RACE_HALF_ORC:
    case RACE_HALF_GIANT:
        i=number_range(0,3);
        break;
    case RACE_PREDATOR:
    case RACE_HERBIV:
    case RACE_LABRAT:
        i=number_range(4,6);
        break;
    case RACE_REPTILE:
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
    case RACE_DRAGON_BRASS	 :
        i=number_range(4,7);
        break;
    case RACE_TREE:
        i=8;
        break;
    case RACE_PARASITE:
    case RACE_SLIME:
    case RACE_VEGGIE:
    case RACE_VEGMAN:
        i=9;
        break;
    case RACE_ROO:
    case RACE_GNOME:
    case RACE_HALFLING:
    case RACE_GOBLIN:
    case RACE_SMURF:
    case RACE_ENFAN:
        i=10;
        break;
    case RACE_GIANT:
    case RACE_GIANT_HILL   :
    case RACE_GIANT_FROST  :
    case RACE_GIANT_FIRE   :
    case RACE_GIANT_CLOUD  :
    case RACE_GIANT_STORM  :
    case RACE_GIANT_STONE  :
    case RACE_TYTAN:
    case RACE_GOD:
        i=11;
        break;
    case RACE_GHOST:
        i=12;
        break;
    case RACE_BIRD:
    case RACE_SKEXIE:
        i=13;
        break;
    case RACE_UNDEAD:
    case RACE_UNDEAD_VAMPIRE :
    case RACE_UNDEAD_LICH    :
    case RACE_UNDEAD_WIGHT   :
    case RACE_UNDEAD_GHAST   :
    case RACE_UNDEAD_SPECTRE :
    case RACE_UNDEAD_ZOMBIE  :
    case RACE_UNDEAD_SKELETON :
    case RACE_UNDEAD_GHOUL    :
        i=14;
        break;
    case RACE_DINOSAUR:
        i=15;
        break;
    case RACE_INSECT:
    case RACE_ARACHNID:
        i=16;
        break;
    case RACE_FISH:
        i=17;
        break;
    default:
        i=18;
    };

    if(!soft_damage){
        act(AT_GREY, att_kick_miss_ch[i], ch, NULL, victim, TO_CHAR);
        act(AT_GREY, att_kick_miss_victim[i], ch, NULL, victim, TO_VICT);
        act(AT_GREY, att_kick_miss_room[i], ch, NULL, victim, TO_NOTVICT);
    }
    else if(soft_damage > GET_HIT(victim))
    {
        act(AT_GREY, att_kick_kill_ch[i], ch, NULL, victim, TO_CHAR);
        act(AT_GREY, att_kick_kill_victim[i], ch, NULL, victim, TO_VICT);
        act(AT_GREY, att_kick_kill_room[i], ch, NULL, victim, TO_NOTVICT);
    }
    else
    {
        act(AT_GREY, att_kick_hit_ch[i], ch, NULL, victim, TO_CHAR);
        act(AT_GREY, att_kick_hit_victim[i], ch, NULL, victim, TO_VICT);
        act(AT_GREY, att_kick_hit_room[i],ch, NULL, victim, TO_NOTVICT);
    }

}

void do_kick( CHAR_DATA *ch, char *argument )
{
    int dam;
    CHAR_DATA *victim;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    if ( !IS_NPC(ch)
         &&   !CanUseSkill(ch, gsn_kick) )
    {
        send_to_char(
                     "You better leave the martial arts to fighters.\n\r", ch );
        return;
    }

    if ( !(victim = who_fighting(ch)) )
        if ( !(victim = get_char_room(ch, argument)) )
        {
            send_to_char("They aren't here.\n\r", ch );
            return;
        }

    if ( !is_legal_kill(ch, victim) || is_safe(ch, victim) )
    {
        send_to_char("You can't do that!\n\r", ch);
        return;
    }

    dam = IS_ACTIVE(ch, CLASS_MONK)? GetMaxLevel(ch):(GetMaxLevel(ch)/2);

    spell_lag(ch, gsn_kick);
    if ( (IS_NPC(ch) || number_percent( ) < LEARNED(ch, gsn_kick) ) && GET_RACE(victim)!=RACE_GHOST)
    {
        learn_from_success( ch, gsn_kick );
        kick_messages(ch, victim, dam);
        global_retcode = damage( ch, victim, dam, gsn_kick);
    }
    else
    {
        learn_from_failure( ch, gsn_kick );
        kick_messages(ch, victim, 0);
        global_retcode = damage( ch, victim, 0, gsn_kick );
    }
    return;
}

void do_punch( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    if ( !IS_NPC(ch)
         &&   !CanUseSkill(ch, gsn_punch) )
    {
        send_to_char(
                     "You better leave the martial arts to fighters.\n\r", ch );
        return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
        if ( !(victim = get_char_room(ch, argument)) )
        {
            send_to_char("They aren't here.\n\r", ch );
            return;
        }

    if (ch == victim)
    {
        send_to_char("You try to punch yourself, but fail!\n\r", ch);
        return;
    }

    spell_lag(ch, gsn_punch);
    if ( IS_NPC(ch) || number_percent( ) < LEARNED(ch, gsn_punch) )
    {
        learn_from_success( ch, gsn_punch );
        global_retcode = damage( ch, victim, number_range( 1,
                                                           GetMaxLevel(ch)), gsn_punch );
    }
    else
    {
        learn_from_failure( ch, gsn_punch );
        global_retcode = damage( ch, victim, 0, gsn_punch );
    }
    return;
}


void do_bite( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    if ( !IS_NPC(ch)
         &&   !CanUseSkill(ch, gsn_bite) )
    {
        send_to_char(
                     "That isn't quite one of your natural skills.\n\r", ch );
        return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
        send_to_char( "You aren't fighting anyone.\n\r", ch );
        return;
    }

    spell_lag(ch, gsn_bite);
    if ( IS_NPC(ch) || number_percent( ) < LEARNED(ch, gsn_bite) )
    {
        learn_from_success( ch, gsn_bite );
        global_retcode = damage( ch, victim, number_range( 1,
                                                           GetMaxLevel(ch) ), gsn_bite );
    }
    else
    {
        learn_from_failure( ch, gsn_bite );
        global_retcode = damage( ch, victim, 0, gsn_bite );
    }
    return;
}


void do_claw( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( !IS_NPC(ch)
         &&   !CanUseSkill(ch, gsn_claw) )
    {
        send_to_char(
                     "That isn't quite one of your natural skills.\n\r", ch );
        return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
        send_to_char( "You aren't fighting anyone.\n\r", ch );
        return;
    }

    spell_lag(ch, gsn_claw);
    if ( IS_NPC(ch) || number_percent( ) < LEARNED(ch, gsn_claw) )
    {
        learn_from_success( ch, gsn_claw );
        global_retcode = damage( ch, victim, number_range( 1,
                                                           GetMaxLevel(ch) ), gsn_claw );
    }
    else
    {
        learn_from_failure( ch, gsn_claw );
        global_retcode = damage( ch, victim, 0, gsn_claw );
    }
    return;
}


void do_sting( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    if ( !IS_NPC(ch)
         &&   !CanUseSkill(ch, gsn_sting) )
    {
        send_to_char(
                     "That isn't quite one of your natural skills.\n\r", ch );
        return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
        send_to_char( "You aren't fighting anyone.\n\r", ch );
        return;
    }

    spell_lag(ch, gsn_sting);
    if ( IS_NPC(ch) || number_percent( ) < LEARNED(ch, gsn_sting) )
    {
        learn_from_success( ch, gsn_sting );
        global_retcode = damage( ch, victim, number_range( 1,
                                                           GetMaxLevel(ch) ), gsn_sting );
    }
    else
    {
        learn_from_failure( ch, gsn_sting );
        global_retcode = damage( ch, victim, 0, gsn_sting );
    }
    return;
}


void do_tail( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    if ( !IS_NPC(ch)
         &&   !CanUseSkill(ch, gsn_tail) )
    {
        send_to_char(
                     "That isn't quite one of your natural skills.\n\r", ch );
        return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
        send_to_char( "You aren't fighting anyone.\n\r", ch );
        return;
    }

    spell_lag(ch, gsn_tail);
    if ( IS_NPC(ch) || number_percent( ) < LEARNED(ch, gsn_tail) )
    {
        learn_from_success( ch, gsn_tail );
        global_retcode = damage( ch, victim, number_range( 1,
                                                           GetMaxLevel(ch) ), gsn_tail );
    }
    else
    {
        learn_from_failure( ch, gsn_tail );
        global_retcode = damage( ch, victim, 0, gsn_tail );
    }
    return;
}


void do_bash( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int chance;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    if ( !IS_NPC(ch) && !CanUseSkill(ch, gsn_bash) )
    {
        send_to_char("You better leave the martial arts to fighters.\n\r", ch );
        return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
        if ( !(victim = get_char_room(ch, argument)) )
        {
            send_to_char("They aren't here.\n\r", ch );
            return;
        }

    if (ch == victim)
    {
        send_to_char("You try to bash yourself, but fail!\n\r", ch);
        return;
    }

    if ((IS_NPC(victim) && IS_SET(victim->act, ACT_HUGE)) ||
        IS_SET(victim->affected_by2, AFF2_GROWTH) )
        if (!IsGiant(ch) && !IS_SET(ch->affected_by2, AFF2_GROWTH))
        {
            ch_printf(ch, "%s is MUCH too large to bash!\n\r", PERS(victim, ch));
            return;
        }

    chance = ( (get_curr_dex(victim) + get_curr_str(victim) + GetMaxLevel(victim))
               -   (get_curr_dex(ch) + get_curr_str(ch) + GetMaxLevel(ch)) );
    if ( victim->fighting && victim->fighting->who != ch )
        chance += 25;

    spell_lag(ch, gsn_bash);
    if ( IS_NPC(ch)
         || (number_percent( ) + chance) < LEARNED(ch, gsn_bash) )
    {
        WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
        WAIT_STATE( victim, 2 * PULSE_VIOLENCE );
        victim->position = POS_SITTING;
        global_retcode = damage( ch, victim, number_range( 1, 2 ), gsn_bash );
    }
    else
    {
        WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
        learn_from_failure( ch, gsn_bash );
        ch->position = POS_SITTING;
        global_retcode = damage( ch, victim, 0, gsn_bash );
    }
}


void do_stun( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    int chance;
    bool fail;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    if ( !IS_NPC(ch)
         &&   !CanUseSkill(ch, gsn_stun) )
    {
        send_to_char(
                     "You better leave the martial arts to fighters.\n\r", ch );
        return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
        send_to_char( "You aren't fighting anyone.\n\r", ch );
        return;
    }

    if ( GET_MOVE(ch) < 16 )
    {
        set_char_color( AT_SKILL, ch );
        send_to_char( "You are far too tired to do that.\n\r", ch );
        return;		/* missing return fixed March 11/96 */
    }

    spell_lag(ch, gsn_stun);
    fail = FALSE;
    chance = ris_save( victim, ch->levels[BestSkCl(ch, gsn_stun)], RIS_PARALYSIS );
    if ( chance == 1000 )
        fail = TRUE;
    else
        fail = saves_para_petri( chance, victim );

    chance = (((get_curr_dex(victim) + get_curr_str(victim))
               -   (get_curr_dex(ch)     + get_curr_str(ch))) * 10) + 10;
    /* harder for player to stun another player */
    if ( !IS_NPC(ch) && !IS_NPC(victim) )
        chance += sysdata.stun_plr_vs_plr;
    else
        chance += sysdata.stun_regular;
    if ( !fail
         && (  IS_NPC(ch)
               || (number_percent( ) + chance) < LEARNED(ch, gsn_stun) ) )
    {
        learn_from_success( ch, gsn_stun );
        /*    DO *NOT* CHANGE!    -Thoric    */
        GET_MOVE(ch) -= 15;
        WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
        WAIT_STATE( victim, PULSE_VIOLENCE );
        act( AT_SKILL, "$N smashes into you, leaving you stunned!", victim, NULL, ch, TO_CHAR );
        act( AT_SKILL, "You smash into $N, leaving $M stunned!", ch, NULL, victim, TO_CHAR );
        act( AT_SKILL, "$n smashes into $N, leaving $M stunned!", ch, NULL, victim, TO_NOTVICT );
        if ( !IS_AFFECTED( victim, AFF_PARALYSIS ) )
        {
            af.type      = gsn_stun;
            af.location  = APPLY_AC;
            af.modifier  = 20;
            af.duration  = 3;
            af.bitvector = AFF_PARALYSIS;
            affect_to_char( victim, &af );
            update_pos( victim );
        }
    }
    else
    {
        WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
        GET_MOVE(ch) -= 5;
        learn_from_failure( ch, gsn_stun );
        act( AT_SKILL, "$N charges at you screaming, but you dodge out of the way.", victim, NULL, ch, TO_CHAR );
        act( AT_SKILL, "You try to stun $N, but $E dodges out of the way.", ch, NULL, victim, TO_CHAR );
        act( AT_SKILL, "$n charges screaming at $N, but keeps going right on past.", ch, NULL, victim, TO_NOTVICT );
    }
    return;
}


void do_feed( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    sh_int dam;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    if ( !IS_NPC(ch)
         &&   !IS_VAMPIRE(ch) )
    {
        send_to_char( "It is not of your nature to feed on living creatures.\n\r", ch );
        return;
    }
    if ( !IS_NPC(ch)
         &&   !LEARNED(ch, gsn_feed) )
    {
        send_to_char( "You have not yet practiced your new teeth.\n\r", ch );
        return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
        send_to_char( "You aren't fighting anyone.\n\r", ch );
        return;
    }

    if ( ch->mount )
    {
        send_to_char( "You can't do that while mounted.\n\r", ch );
        return;
    }

    spell_lag(ch, gsn_feed);
    if ( IS_NPC(ch) || number_percent( ) < LEARNED(ch, gsn_feed) )
    {
        dam = number_range( 1, ch->levels[CLASS_VAMPIRE] );
        global_retcode = damage( ch, victim, dam, gsn_feed );
        if ( global_retcode == rNONE && !IS_NPC(ch) && dam
             &&  ch->fighting
             &&  GET_COND(ch, COND_BLOODTHIRST) < (GET_MAX_BLOOD(ch)) )
        {
            gain_condition( ch, COND_BLOODTHIRST,
                            UMIN( number_range(1, (ch->levels[CLASS_VAMPIRE]+
                                                   GetMaxLevel(victim) / 20) + 3 ),
                                  GET_MAX_BLOOD(ch) - GET_COND(ch, COND_BLOODTHIRST) ));
            gain_condition( ch, COND_FULL, 2);
            gain_condition( ch, COND_THIRST, 2);
            act( AT_BLOOD, "You manage to suck a little life out of $N.", ch, NULL,
                 victim, TO_CHAR );
            act( AT_BLOOD, "$n sucks some of your blood!", ch, NULL, victim, TO_VICT );
            learn_from_success( ch, gsn_feed );
        }
    }
    else
    {
        global_retcode = damage( ch, victim, 0, gsn_feed );
        if ( global_retcode == rNONE && !IS_NPC(ch)
             &&  ch->fighting
             &&  GET_COND(ch, COND_BLOODTHIRST) < (GET_MAX_BLOOD(ch)) )
        {
            act( AT_BLOOD, "The smell of $N's blood is driving you insane!",
                 ch, NULL, victim, TO_CHAR );
            act( AT_BLOOD, "$n is lusting after your blood!", ch, NULL, victim, TO_VICT );
            learn_from_failure( ch, gsn_feed );
        }
    }
    return;
}


/*
 * Disarm a creature.
 * Caller must check for successful attack.
 * Check for loyalty flag (weapon disarms to inventory) for pkillers -Blodkai
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj, *tmpobj;

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
        return;

    if ( ( tmpobj = get_eq_char( victim, WEAR_DUAL_WIELD ) ) != NULL
         &&     number_bits( 1 ) == 0 )
        obj = tmpobj;

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL && number_bits( 1 ) == 0 )
    {
        learn_from_failure( ch, gsn_disarm );
        return;
    }

    if ( IS_NPC( ch ) && !can_see_obj( ch, obj ) && number_bits( 1 ) == 0)
    {
        learn_from_failure( ch, gsn_disarm );
        return;
    }

    if ( check_grip( ch, victim ) )
    {
        learn_from_failure( ch, gsn_disarm );
        return;
    }

    act( AT_SKILL, "$n DISARMS you!", ch, NULL, victim, TO_VICT    );
    act( AT_SKILL, "You disarm $N!",  ch, NULL, victim, TO_CHAR    );
    act( AT_SKILL, "$n disarms $N!",  ch, NULL, victim, TO_NOTVICT );
    learn_from_success( ch, gsn_disarm );

    if ( obj == get_eq_char( victim, WEAR_WIELD )
         &&  (tmpobj = get_eq_char( victim, WEAR_DUAL_WIELD)) != NULL )
        tmpobj->wear_loc = WEAR_WIELD;

    obj_from_char( obj );
    if ( IS_NPC(victim)
         || ( IS_OBJ_STAT(obj, ITEM_LOYAL) && IS_PKILL(victim) ) )
        obj_to_char( obj, victim );
    else
        obj_to_room( obj, victim->in_room );

    return;
}


void do_disarm( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    if ( !IS_NPC(ch)
         &&  !CanUseSkill(ch, gsn_disarm) )
    {
        send_to_char( "You don't know how to disarm opponents.\n\r", ch );
        return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL )
    {
        send_to_char( "You must wield a weapon to disarm.\n\r", ch );
        return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
        send_to_char( "You aren't fighting anyone.\n\r", ch );
        return;
    }

    if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
    {
        send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
        return;
    }

    spell_lag(ch, gsn_disarm);
    percent = number_percent( ) + GetMaxLevel(victim)
        - ch->levels[BestSkCl(ch, gsn_disarm)]
        - (get_curr_lck(ch) - 15) + (get_curr_lck(victim) - 15);
    if ( !can_see_obj( ch, obj ) )
        percent += 10;
    if ( IS_NPC(ch) || percent < LEARNED(ch, gsn_disarm) * 2 / 3 )
        disarm( ch, victim );
    else
    {
        send_to_char( "You failed.\n\r", ch );
        learn_from_failure( ch, gsn_disarm );
    }
    return;
}


/*
 * Trip a creature.
 * Caller must check for successful attack.
 */
void trip( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if (GET_POS(victim) == POS_SITTING)
    {
       send_to_char("They are already on the ground.\n\r", ch);
       return;
    }

    if ( IS_AFFECTED( victim, AFF_FLYING )
         ||   IS_AFFECTED( victim, AFF_FLOATING ) )
    {
        act( AT_SKILL, "$n trys to trip you, buy you nimbly float away!", ch, NULL, victim, TO_VICT    );
        act( AT_SKILL, "$N nimbly floats away from your well placed trip!", ch, NULL, victim, TO_CHAR    );
        act( AT_SKILL, "$N nimbly floats away from $n's well placed trip!", ch, NULL, victim, TO_NOTVICT );
        return;
    }
    if ( victim->mount )
    {
        if ( IS_AFFECTED( victim->mount, AFF_FLYING )
             ||   IS_AFFECTED( victim->mount, AFF_FLOATING ) )
            return;
        act( AT_SKILL, "$n trips your mount and you fall off!", ch, NULL, victim, TO_VICT    );
        act( AT_SKILL, "You trip $N's mount and $N falls off!", ch, NULL, victim, TO_CHAR    );
        act( AT_SKILL, "$n trips $N's mount and $N falls off!", ch, NULL, victim, TO_NOTVICT );
        REMOVE_BIT( victim->mount->act, ACT_MOUNTED );
        victim->mount = NULL;
        WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
        WAIT_STATE( victim, 2 * PULSE_VIOLENCE );
        victim->position = POS_SITTING;
        return;
    } else {
        act( AT_SKILL, "$n trips you and you go down!", ch, NULL, victim, TO_VICT    );
        act( AT_SKILL, "You trip $N and $N goes down!", ch, NULL, victim, TO_CHAR    );
        act( AT_SKILL, "$n trips $N and $N goes down!", ch, NULL, victim, TO_NOTVICT );

        WAIT_STATE( ch,     2 * PULSE_VIOLENCE );
        WAIT_STATE( victim, 2 * PULSE_VIOLENCE );
        victim->position = POS_SITTING;
    }

    return;
}


void do_legsweep( CHAR_DATA *ch, char *argument )
{
   int gsn_legsweep = skill_lookup("legsweep");
   CHAR_DATA *victim;

   int fl = 0;


   if (!ch)
      return;

    if ( ( victim = who_fighting( ch ) ) == NULL )
        if ( !(victim = get_char_room(ch, argument)) )
        {
            send_to_char("They aren't here.\n\r", ch );
            return;
        }

    if (ch == victim)
       {
            send_to_char("You try to legsweep yourself, but fail!\n\r", ch);
            return;
       }

   if (HAS_BODYPART(victim, PART_FORELEGS))
      fl = 20;
   if (!CanUseSkill(ch, gsn_legsweep))
   {
       send_to_char("Leave the martial arts to fighters!\n\r", ch);
       return;
   }
   if ((number_percent()+fl) > (IS_NPC(ch)? 80:LEARNED(ch, gsn_legsweep)) )
   {
       send_to_char("You fail and fall down!\n\r", ch);
       ch->position = POS_SITTING;
       learn_from_failure(ch, gsn_legsweep);
       return;
   }

   if (!HAS_BODYPART(victim, PART_LEGS) &&
       !HAS_BODYPART(victim, PART_FORELEGS))
   {
       send_to_char("How can you legsweep something with no legs?\n\r", ch);
       return;
   }

   trip(ch, victim);
}

void do_pick( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    OBJ_DATA *obj;
    EXIT_DATA *pexit;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Pick what?\n\r", ch );
        return;
    }

    if ( ms_find_obj(ch) )
        return;

    if ( ch->mount )
    {
        send_to_char( "You can't do that while mounted.\n\r", ch );
        return;
    }

    spell_lag(ch, gsn_pick_lock);

    /* look for guards */
    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
        if ( IS_NPC(gch) && IS_AWAKE(gch) && GetMaxLevel(ch) + 5 < GetMaxLevel(gch) )
        {
            act( AT_SKILL, "$N is standing too close to the lock.",
                 ch, NULL, gch, TO_CHAR );
            return;
        }
    }

    if ( !IS_NPC(ch) && number_percent( ) > LEARNED(ch, gsn_pick_lock) )
    {
        send_to_char( "You failed.\n\r", ch);
        learn_from_failure( ch, gsn_pick_lock );
        return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
        /* 'pick door' */
        /*	ROOM_INDEX_DATA *to_room; */ /* Unused */
        EXIT_DATA *pexit_rev;

        if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
        { send_to_char( "It's not closed.\n\r",        ch ); return; }
        if ( pexit->key < 0 )
        { send_to_char( "It can't be picked.\n\r",     ch ); return; }
        if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
        { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
        if ( IS_SET(pexit->exit_info, EX_PICKPROOF) )
        {
            send_to_char( "You failed.\n\r", ch );
            learn_from_failure( ch, gsn_pick_lock );
            check_room_for_traps( ch, TRAP_PICK | trap_door[pexit->vdir] );
            return;
        }

        REMOVE_BIT(pexit->exit_info, EX_LOCKED);
        send_to_char( "*Click*\n\r", ch );
        act( AT_ACTION, "$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
        learn_from_success( ch, gsn_pick_lock );
        adjust_favor( ch, 9, 1 );
        /* pick the other side */
        if ( ( pexit_rev = pexit->rexit ) != NULL
             &&   pexit_rev->to_room == ch->in_room )
        {
            REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
        }
        check_room_for_traps( ch, TRAP_PICK | trap_door[pexit->vdir] );
        return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
        /* 'pick object' */
        if ( obj->item_type != ITEM_CONTAINER )
        { send_to_char( "That's not a container.\n\r", ch ); return; }
        if ( !IS_SET(obj->value[1], CONT_CLOSED) )
        { send_to_char( "It's not closed.\n\r",        ch ); return; }
        if ( obj->value[2] < 0 )
        { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
        if ( !IS_SET(obj->value[1], CONT_LOCKED) )
        { send_to_char( "It's already unlocked.\n\r",  ch ); return; }
        if ( IS_SET(obj->value[1], CONT_PICKPROOF) )
        {
            send_to_char( "You failed.\n\r", ch );
            learn_from_failure( ch, gsn_pick_lock );
            check_for_trap( ch, obj, TRAP_PICK );
            return;
        }

        separate_obj( obj );
        REMOVE_BIT(obj->value[1], CONT_LOCKED);
        send_to_char( "*Click*\n\r", ch );
        act( AT_ACTION, "$n picks $p.", ch, obj, NULL, TO_ROOM );
        learn_from_success( ch, gsn_pick_lock );
        adjust_favor( ch, 9, 1 );
        check_for_trap( ch, obj, TRAP_PICK );
        return;
    }

    ch_printf( ch, "You see no %s here.\n\r", arg );
    return;
}



void do_sneak( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA af;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    if ( ch->mount )
    {
        send_to_char( "You can't do that while mounted.\n\r", ch );
        return;
    }

    send_to_char( "You attempt to move silently.\n\r", ch );
    affect_strip( ch, gsn_sneak );

    if ( IS_NPC(ch) || number_percent( ) < LEARNED(ch, gsn_sneak) )
    {
        af.type      = gsn_sneak;
        af.duration  = (int)((float)ch->levels[BestSkCl(ch, gsn_sneak)] * DUR_CONV);
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = AFF_SNEAK;
        affect_to_char( ch, &af );
        learn_from_success( ch, gsn_sneak );
    }
    else
        learn_from_failure( ch, gsn_sneak );

    return;
}



void do_hide( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    if ( ch->mount )
    {
        send_to_char( "You can't do that while mounted.\n\r", ch );
        return;
    }

    send_to_char( "You attempt to hide.\n\r", ch );

    if ( IS_AFFECTED(ch, AFF_HIDE) )
        REMOVE_BIT(ch->affected_by, AFF_HIDE);

    if ( IS_NPC(ch) || number_percent( ) < LEARNED(ch, gsn_hide) )
    {
        SET_BIT(ch->affected_by, AFF_HIDE);
        learn_from_success( ch, gsn_hide );
    }
    else
        learn_from_failure( ch, gsn_hide );
    return;
}



/*
 * Contributed by Alander.
 */
void do_visible( CHAR_DATA *ch, char *argument )
{
    affect_strip ( ch, gsn_invis			);
    affect_strip ( ch, gsn_group_invis			);
    affect_strip ( ch, gsn_sneak			);
    REMOVE_BIT   ( ch->affected_by, AFF_HIDE		);
    REMOVE_BIT   ( ch->affected_by, AFF_INVISIBLE	);
    if (ch->race != RACE_HALFLING) /* Halfling has perm sneak SB */
        REMOVE_BIT   ( ch->affected_by, AFF_SNEAK		);
    if (!IS_NPC(ch))
        ch->pcdata->wizinvis = 0;
    if (IS_NPC(ch))
        ch->mobinvis = 0;
    send_to_char( "Ok.\n\r", ch );
    return;
}


void do_recall( CHAR_DATA *ch, char *argument )
{
    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL) )
    {
        send_to_char( "For some strange reason... nothing happens.\n\r", ch );
        return;
    }

    if ( who_fighting( ch ) )
    {
        send_to_char("You cannot recall while fighting.\n\r",ch);
        return;
    }

    act( AT_ACTION, "$n disappears in a swirl of smoke.", ch, NULL, NULL, TO_ROOM );

    recall_char( ch );

    if ( ch->mount )
    {
        char_from_room( ch->mount );
        char_to_room( ch->mount, ch->in_room );
    }
    act( AT_ACTION, "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    return;
}


void do_aid( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int percent;

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Aid whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( ch->mount )
    {
        send_to_char( "You can't do that while mounted.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "Aid yourself?\n\r", ch );
        return;
    }

    if ( victim->position > POS_STUNNED )
    {
        act( AT_SKILL, "$N doesn't need your help.", ch, NULL, victim,
             TO_CHAR);
        return;
    }

    if ( GET_HIT(victim) <= -6 )
    {
        act( AT_SKILL, "$N's condition is beyond your aiding ability.", ch,
             NULL, victim, TO_CHAR);
        return;
    }

    percent = number_percent( ) - (get_curr_lck(ch) - 13);
    spell_lag(ch, gsn_aid);
    if ( !IS_NPC(ch) && percent > LEARNED(ch, gsn_aid) )
    {
        send_to_char( "You fail.\n\r", ch );
        learn_from_failure( ch, gsn_aid );
        return;
    }

    act( AT_SKILL, "You aid $N!",  ch, NULL, victim, TO_CHAR    );
    act( AT_SKILL, "$n aids $N!",  ch, NULL, victim, TO_NOTVICT );
    learn_from_success( ch, gsn_aid );
    adjust_favor( ch, 8, 1 );
    if ( GET_HIT(victim) < 1 )
        GET_HIT(victim) = 1;

    update_pos( victim );
    act( AT_SKILL, "$n aids you!", ch, NULL, victim, TO_VICT    );
    return;
}

int mount_ego_check(CHAR_DATA *ch, CHAR_DATA *horse)
{
    int ride_ego, drag_ego, align, check;

    if (IsDragon(horse)) {
        drag_ego = GetMaxLevel(horse)*2;
        if (IS_ACT_FLAG(horse, ACT_AGGRESSIVE) ||
            IS_ACT_FLAG(horse, ACT_META_AGGR)) {
            drag_ego += GetMaxLevel(horse);
        }
        ride_ego = LEARNED(ch, gsn_mount)/10 +
            GetMaxLevel(ch)/2;
        if (is_affected(ch, gsn_dragon_ride))
        {
            ride_ego += ((get_curr_int(ch) + get_curr_wis(ch))/2);
        }
        align = GET_ALIGN(ch) - GET_ALIGN(horse);
        if (align < 0) align = -align;
        align/=100;
        align -= 5;
        drag_ego += align;
        if (GET_HIT(horse) > 0)
            drag_ego -= GET_MAX_HIT(horse)/GET_HIT(horse);
        else
            drag_ego = 0;
        if (GET_HIT(ch) > 0)
            ride_ego -= GET_MAX_HIT(ch)/GET_HIT(ch);
        else
            ride_ego = 0;

        check = drag_ego+number_range(1,10)-(ride_ego+number_range(1,10));
        return(check);
    } else {
        drag_ego = GetMaxLevel(horse);

        if (drag_ego > 15)
            drag_ego *= 2;

        ride_ego = LEARNED(ch, gsn_mount)/10 +
            GetMaxLevel(ch);

        if (is_affected(ch, gsn_dragon_ride))
        {
            ride_ego += (get_curr_int(ch) + get_curr_wis(ch));
        }
        check = drag_ego+number_range(1,5)-(ride_ego+number_range(1,10));
        return(check);
    }
}

void do_mount( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    sh_int learned;
    int check;

    if ( !IS_NPC(ch)
         &&   !CanUseSkill(ch, gsn_mount) )
    {
        send_to_char(
                     "I don't think that would be a good idea...\n\r", ch );
        return;
    }

    if ( ch->mount )
    {
        send_to_char( "You're already mounted!\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    {
        send_to_char( "You can't find that here.\n\r", ch );
        return;
    }

    if (!IS_NPC(victim) || (!IS_SET(victim->act, ACT_MOUNTABLE) &&
                            GET_RACE(victim) != RACE_HORSE &&
                            !IsDragon(victim)))
    {
        send_to_char( "You can't mount that!\n\r", ch );
        return;
    }

    if ( IS_SET(victim->act, ACT_MOUNTED ) )
    {
        send_to_char( "That mount already has a rider.\n\r", ch );
        return;
    }

    if ( victim->position < POS_STANDING )
    {
        send_to_char( "Your mount must be standing.\n\r", ch );
        return;
    }

    if ( victim->position == POS_FIGHTING || victim->fighting )
    {
        send_to_char( "Your mount is moving around too much.\n\r", ch );
        return;
    }

    spell_lag(ch, gsn_mount);

    check = mount_ego_check(ch, victim);

    if (check > 5) {
        act(AT_SKILL, "$N snarls and attacks!", ch, NULL, victim, TO_CHAR);
        act(AT_SKILL, "As $n tries to mount $N, $N attacks $n!", ch, NULL, victim, TO_NOTVICT);
        one_hit(ch, victim, gsn_mount);
        return;
    }
    else if (check > -1)
    {
        act(AT_SKILL, "$N moves out of the way and you fall on your butt.", ch, NULL, victim, TO_CHAR);
        act(AT_SKILL, "as $n tries to mount $N, $N moves out of the way", ch, NULL, victim, TO_NOTVICT);
        GET_POS(ch) = POS_SITTING;
        return;
    }

    learned = LEARNED(ch, gsn_mount);
    if (is_affected(ch, gsn_dragon_ride))
        learned += GET_LEVEL(ch, BestMagicClass(ch));
    if ( IS_NPC(ch) || number_percent( ) < learned )
    {
        SET_BIT( victim->act, ACT_MOUNTED );
        ch->mount = victim;
        act( AT_SKILL, "You mount $N.", ch, NULL, victim, TO_CHAR );
        act( AT_SKILL, "$n skillfully mounts $N.", ch, NULL, victim, TO_NOTVICT );
        act( AT_SKILL, "$n mounts you.", ch, NULL, victim, TO_VICT );
        learn_from_success( ch, gsn_mount );
        GET_POS(ch) = POS_MOUNTED;
    }
    else
    {
        act( AT_SKILL, "You unsuccessfully try to mount $N.", ch, NULL, victim, TO_CHAR );
        act( AT_SKILL, "$n unsuccessfully attempts to mount $N.", ch, NULL, victim, TO_NOTVICT );
        act( AT_SKILL, "$n tries to mount you.", ch, NULL, victim, TO_VICT );
        learn_from_failure( ch, gsn_mount );
    }
    return;
}


void do_dismount( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if ( (victim = ch->mount) == NULL )
    {
        send_to_char( "You're not mounted.\n\r", ch );
        return;
    }

    spell_lag(ch, gsn_mount);
    if ( IS_NPC(ch) || number_percent( ) < LEARNED(ch, gsn_mount) )
    {
        act( AT_SKILL, "You dismount $N.", ch, NULL, victim, TO_CHAR );
        act( AT_SKILL, "$n skillfully dismounts $N.", ch, NULL, victim, TO_NOTVICT );
        act( AT_SKILL, "$n dismounts you.  Whew!", ch, NULL, victim, TO_VICT );
        REMOVE_BIT( victim->act, ACT_MOUNTED );
        ch->mount = NULL;
        ch->position = POS_STANDING;
        learn_from_success( ch, gsn_mount );
    }
    else
    {
        act( AT_SKILL, "You fall off while dismounting $N.  Ouch!", ch, NULL, victim, TO_CHAR );
        act( AT_SKILL, "$n falls off of $N while dismounting.", ch, NULL, victim, TO_NOTVICT );
        act( AT_SKILL, "$n falls off your back.", ch, NULL, victim, TO_VICT );
        learn_from_failure( ch, gsn_mount );
        REMOVE_BIT( victim->act, ACT_MOUNTED );
        ch->mount = NULL;
        ch->position = POS_SITTING;
        global_retcode = damage( ch, ch, 1, gsn_mount );
    }
    return;
}


/**************************************************************************/


/*
 * Check for parry.
 */
int check_parry( CHAR_DATA *ch, CHAR_DATA *victim, int dam )
{
    int chances;

    if ( !IS_AWAKE(victim) )
        return dam;

    if ( IS_NPC(victim) && !IS_SET(victim->defenses, DFND_PARRY) )
        return dam;

    if (!LEARNED(ch, gsn_parry))
        return dam;

    if ( IS_NPC(victim) )
    {
        /* Tuan was here.  :) */
        /* Heath was here bigger, badder, AND more recently! */
        chances	= UMIN( 60, 2 * GetMaxLevel(victim) );
    }
    else
    {
        if ( get_eq_char( victim, WEAR_WIELD ) == NULL )
            return dam;
        chances	= (int) (LEARNED(victim, gsn_parry) / 2);
    }

    /* Put in the call to chance() to allow penalties for misaligned
     clannies.  */
    if ( !chance( victim, chances + GetMaxLevel(victim) - GetMaxLevel(ch) ) )
    {
        learn_from_failure( victim, gsn_parry );
        return dam;
    }

    if ( dam == 0 )
    {
        if ( !IS_NPC(victim) && !IS_SET( victim->pcdata->flags, PCFLAG_GAG) )
            act( AT_SKILL, "You parry $n's attack.",  ch, NULL, victim, TO_VICT );

        if ( !IS_NPC(ch)
             && !IS_SET( ch->pcdata->flags, PCFLAG_GAG) )
            act( AT_SKILL, "$N parries your attack.", ch, NULL, victim, TO_CHAR );
    }

    learn_from_success( victim, gsn_parry );
    return dam;
}



/*
 * Check for dodge.
 */
int check_dodge( CHAR_DATA *ch, CHAR_DATA *victim, int dam )
{
    int chances;

    if ( !IS_AWAKE(victim) )
        return dam;

    if ( IS_NPC(victim) && !IS_SET(victim->defenses, DFND_DODGE) )
        return dam;

    if ( !IS_NPC(victim) && !LEARNED(victim, gsn_dodge))
        return dam;

    if ( IS_NPC(victim) )
        chances  = UMIN( 60, 2 * GetMaxLevel(victim) );
    else
        chances  = (int) (LEARNED(victim, gsn_dodge) / 2);

    /* Consider luck as a factor */
    if ( !chance( victim, chances + GetMaxLevel(victim) - GetMaxLevel(ch)) )
    {
        learn_from_failure( victim, gsn_dodge );
        return dam;
    }

    learn_from_success( victim, gsn_dodge );

    dam -= number_range(1,3);
    if (!IS_NPC(victim) && IS_ACTIVE(victim,CLASS_MONK) &&
        number_range(1,20000)<LEARNED(victim, gsn_dodge)*GET_LEVEL(ch,CLASS_MONK))
        dam = 0;
    else
        dam -= (GET_LEVEL(ch,CLASS_MONK)/10);

    if ( dam == 0 )
    {
        if ( !IS_NPC(victim) && !IS_SET( victim->pcdata->flags, PCFLAG_GAG) )
            act( AT_SKILL, "You dodge $n's attack.", ch, NULL, victim, TO_VICT );

        if ( !IS_NPC(ch) && !IS_SET( ch->pcdata->flags, PCFLAG_GAG) )
            act( AT_SKILL, "$N dodges your attack.", ch, NULL, victim, TO_CHAR );
    }

    return dam;
}

void do_poison_weapon( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    OBJ_DATA *pobj;
    OBJ_DATA *wobj;
    char      arg [ MAX_INPUT_LENGTH ];
    int       percent;

    if ( !IS_NPC( ch )
         && !CanUseSkill(ch, gsn_poison_weapon) )
    {
        send_to_char( "What do you think you are, a thief?\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "What are you trying to poison?\n\r",    ch );
        return;
    }
    if ( ch->fighting )
    {
        send_to_char( "While you're fighting?  Nice try.\n\r", ch );
        return;
    }
    if ( ms_find_obj(ch) )
        return;

    if ( !( obj = get_obj_carry( ch, arg ) ) )
    {
        send_to_char( "You do not have that weapon.\n\r",      ch );
        return;
    }
    if ( obj->item_type != ITEM_WEAPON )
    {
        send_to_char( "That item is not a weapon.\n\r",        ch );
        return;
    }
    if ( IS_OBJ_STAT( obj, ITEM_POISONED ) )
    {
        send_to_char( "That weapon is already poisoned.\n\r",  ch );
        return;
    }
    /* Now we have a valid weapon...check to see if we have the powder. */
    for ( pobj = ch->first_carrying; pobj; pobj = pobj->next_content )
    {
        if ( pobj->vnum == OBJ_VNUM_BLACK_POWDER )
            break;
    }
    if ( !pobj )
    {
        send_to_char( "You do not have the black poison powder.\n\r", ch );
        return;
    }
    /* Okay, we have the powder...do we have water? */
    for ( wobj = ch->first_carrying; wobj; wobj = wobj->next_content )
    {
        if ( wobj->item_type == ITEM_DRINK_CON
             && wobj->value[1]  >  0
             && wobj->value[2]  == 0 )
            break;
    }
    if ( !wobj )
    {
        send_to_char( "You have no water to mix with the powder.\n\r", ch );
        return;
    }
    /* Great, we have the ingredients...but is the thief smart enough? */
    if ( !IS_NPC( ch ) && get_curr_wis( ch ) < 16 )
    {
        send_to_char( "You can't quite remember what to do...\n\r", ch );
        return;
    }
    /* And does the thief have steady enough hands? */
    if ( !IS_NPC( ch )
         && ( (get_curr_dex( ch ) < 17) || GET_COND(ch, COND_DRUNK) > 0 ) )
    {
        send_to_char("Your hands aren't steady enough to properly mix the poison.\n\r", ch );
        return;
    }

    spell_lag(ch, gsn_poison_weapon);

    percent = (number_percent( ) - get_curr_lck(ch) - 14);

    /* Check the skill percentage */
    separate_obj( pobj );
    separate_obj( wobj );
    if ( !IS_NPC( ch )
         && percent > LEARNED(ch, gsn_poison_weapon) )
    {
        set_char_color( AT_RED, ch );
        send_to_char( "You failed and spill some on yourself.  Ouch!\n\r", ch );
        set_char_color( AT_GREY, ch );
        damage( ch, ch, BestSkLv(ch, gsn_poison_weapon), gsn_poison_weapon);
        act(AT_RED, "$n spills the poison all over!", ch, NULL, NULL, TO_ROOM );
        extract_obj( pobj );
        extract_obj( wobj );
        learn_from_failure( ch, gsn_poison_weapon );
        return;
    }
    separate_obj( obj );
    /* Well, I'm tired of waiting.  Are you? */
    act(AT_RED, "You mix $p in $P, creating a deadly poison!", ch, pobj, wobj, TO_CHAR );
    act(AT_RED, "$n mixes $p in $P, creating a deadly poison!",ch, pobj, wobj, TO_ROOM );
    act(AT_GREEN, "You pour the poison over $p, which glistens wickedly!",ch, obj, NULL, TO_CHAR  );
    act(AT_GREEN, "$n pours the poison over $p, which glistens wickedly!",ch, obj, NULL, TO_ROOM  );
    SET_BIT( obj->extra_flags, ITEM_POISONED );
    obj->cost *= BestSkLv(ch, gsn_poison_weapon);
    /* Set an object timer.  Don't want proliferation of poisoned weapons */
    obj->timer = 10 + BestSkLv(ch, gsn_poison_weapon);

    if ( IS_OBJ_STAT( obj, ITEM_BLESS ) )
        obj->timer *= 2;

    if ( IS_OBJ_STAT( obj, ITEM_MAGIC ) )
        obj->timer *= 2;

    /* WHAT?  All of that, just for that one bit?  How lame. ;) */
    act(AT_BLUE, "The remainder of the poison eats through $p.", ch, wobj, NULL, TO_CHAR );
    act(AT_BLUE, "The remainder of the poison eats through $p.", ch, wobj, NULL, TO_ROOM );
    extract_obj( pobj );
    extract_obj( wobj );
    learn_from_success( ch, gsn_poison_weapon );
    return;
}

void do_scribe( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *scroll;
    int sn;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    int mana;

    if ( IS_NPC(ch) )
        return;

    if ( !IS_NPC(ch)
         &&   !CanUseSkill(ch, gsn_scribe) )
    {
        send_to_char( "A skill such as this requires more magical ability than that of your class.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' || !str_cmp(argument, "") )
    {
        send_to_char( "Scribe what?\n\r", ch );
        return;
    }

    if ( ms_find_obj(ch) )
        return;

    if ( (sn = find_spell( ch, argument, TRUE )) < 0 )
    {
        send_to_char( "You have not learned that spell.\n\r", ch );
        return;
    }

    if ( skill_table[sn]->spell_fun == spell_null )
    {
        send_to_char( "That's not a spell!\n\r", ch );
        return;
    }

    if ( SPELL_FLAG(skill_table[sn], SF_NOSCRIBE) )
    {
        send_to_char( "You cannot scribe that spell.\n\r", ch );
        return;
    }

    mana = IS_NPC(ch) ? 0 : UMAX(skill_table[sn]->min_mana,
                                 100 / ( 2 + BestSkLv(ch, sn) - LowSkLv(ch, sn) ) );

    mana *=5;

    if ( !IS_NPC(ch) && GET_MANA(ch) < mana )
    {
        send_to_char( "You don't have enough mana.\n\r", ch );
        return;
    }

    if ( ( scroll = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
        send_to_char( "You must be holding a blank scroll to scribe it.\n\r", ch );
        return;
    }

    if( scroll->vnum != OBJ_VNUM_SCROLL_SCRIBING )
    {
        send_to_char( "You must be holding a blank scroll to scribe it.\n\r", ch );
        return;
    }

    if ( ( scroll->value[1] != -1 )
         && ( scroll->vnum == OBJ_VNUM_SCROLL_SCRIBING ) )
    {
        send_to_char( "That scroll has already been inscribed.\n\r", ch);
        return;
    }

    if ( !process_spell_components( ch, sn ) )
    {
        learn_from_failure( ch, gsn_scribe );
        GET_MANA(ch) -= (mana / 2);
        return;
    }

    if ( !IS_NPC(ch) && number_percent( ) > LEARNED(ch, gsn_scribe) )
    {
        set_char_color ( AT_MAGIC, ch );
        send_to_char("You failed.\n\r", ch);
        learn_from_failure( ch, gsn_scribe );
        GET_MANA(ch) -= (mana / 2);
        return;
    }

    scroll->value[1] = sn;
    scroll->value[0] = BestSkLv(ch, gsn_scribe);
    sprintf(buf1, "%s scroll", skill_table[sn]->name);
    STRFREE(scroll->short_descr);
    scroll->short_descr = STRALLOC( aoran(buf1) );

    sprintf(buf2, "A glowing scroll inscribed '%s' lies in the dust.",
            skill_table[sn]->name);

    STRFREE(scroll->description);
    scroll->description = STRALLOC(buf2);

    sprintf(buf3, "scroll scribing %s", skill_table[sn]->name);
    STRFREE(scroll->name);
    scroll->name = STRALLOC(buf3);

    act( AT_MAGIC, "$n magically scribes $p.",   ch,scroll, NULL, TO_ROOM );
    act( AT_MAGIC, "You magically scribe $p.",   ch,scroll, NULL, TO_CHAR );

    learn_from_success( ch, gsn_scribe );

    GET_MANA(ch) -= mana;

}

void do_brew( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *potion;
    OBJ_DATA *fire;
    int sn;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    int mana;
    bool found;

    if ( IS_NPC(ch) )
        return;

    if ( !IS_NPC(ch)
         &&   !CanUseSkill(ch, gsn_brew) )
    {
        send_to_char( "A skill such as this requires more magical ability than that of your ch_class.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' || !str_cmp(argument, "") )
    {
        send_to_char( "Brew what?\n\r", ch );
        return;
    }

    if ( ms_find_obj(ch) )
        return;

    if ( (sn = find_spell( ch, argument, TRUE )) < 0 )
    {
        send_to_char( "You have not learned that spell.\n\r", ch );
        return;
    }

    if ( skill_table[sn]->spell_fun == spell_null )
    {
        send_to_char( "That's not a spell!\n\r", ch );
        return;
    }

    if ( SPELL_FLAG(skill_table[sn], SF_NOBREW) )
    {
        send_to_char( "You cannot brew that spell.\n\r", ch );
        return;
    }

    mana = IS_NPC(ch) ? 0 : UMAX(skill_table[sn]->min_mana,
                                 100 / ( 2 + BestSkLv(ch, gsn_brew) - LowSkLv(ch, sn)));

    mana *=4;

    if ( !IS_NPC(ch) && GET_MANA(ch) < mana )
    {
        send_to_char( "You don't have enough mana.\n\r", ch );
        return;
    }

    found = FALSE;

    for ( fire = ch->in_room->first_content; fire;
          fire = fire->next_content )
    {
        if( fire->item_type == ITEM_FIRE)
        {
            found = TRUE;
            break;
        }
    }

    if ( !found )
    {
        send_to_char(
                     "There must be a fire in the room to brew a potion.\n\r", ch );
        return;
    }

    if ( ( potion = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
        send_to_char(
                     "You must be holding an empty flask to brew a potion.\n\r", ch );
        return;
    }

    if( potion->vnum != OBJ_VNUM_FLASK_BREWING )
    {
        send_to_char( "You must be holding an empty flask to brew a potion.\n\r", ch );
        return;
    }

    if ( ( potion->value[1] != -1 )
         && ( potion->vnum == OBJ_VNUM_FLASK_BREWING ) )
    {
        send_to_char( "That's not an empty flask.\n\r", ch);
        return;
    }

    if ( !process_spell_components( ch, sn ) )
    {
        learn_from_failure( ch, gsn_brew );
        GET_MANA(ch) -= (mana / 2);
        return;
    }

    if ( !IS_NPC(ch) && number_percent( ) > LEARNED(ch, gsn_brew) )
    {
        set_char_color ( AT_MAGIC, ch );
        send_to_char("You failed.\n\r", ch);
        learn_from_failure( ch, gsn_brew );
        GET_MANA(ch) -= (mana / 2);
        return;
    }

    potion->value[1] = sn;
    potion->value[0] = BestSkLv(ch, gsn_brew);
    sprintf(buf1, "%s potion", skill_table[sn]->name);
    STRFREE(potion->short_descr);
    potion->short_descr = STRALLOC( aoran(buf1) );

    sprintf(buf2, "A strange potion labelled '%s' sizzles in a glass flask.",
            skill_table[sn]->name);

    STRFREE(potion->description);
    potion->description = STRALLOC(buf2);

    sprintf(buf3, "flask potion %s", skill_table[sn]->name);
    STRFREE(potion->name);
    potion->name = STRALLOC(buf3);

    act( AT_MAGIC, "$n brews up $p.",   ch,potion, NULL, TO_ROOM );
    act( AT_MAGIC, "You brew up $p.",   ch,potion, NULL, TO_CHAR );

    learn_from_success( ch, gsn_brew );

    GET_MANA(ch) -= mana;

}

bool check_grip( CHAR_DATA *ch, CHAR_DATA *victim )
{
    int chance;

    if ( !IS_AWAKE(victim) )
        return FALSE;

    if ( IS_NPC(victim) && !IS_SET(victim->defenses, DFND_GRIP) )
        return FALSE;

    if ( IS_NPC(victim) )
        chance  = UMIN( 60, 2 * GetMaxLevel(victim) );
    else
        chance  = (int) (LEARNED(victim, gsn_grip) / 2);

    /* Consider luck as a factor */
    chance += (2 * (get_curr_lck(victim) - 13 ) );

    if ( number_percent( ) >= chance + GetMaxLevel(victim) - GetMaxLevel(ch) )
    {
        learn_from_failure( victim, gsn_grip );
        return FALSE;
    }
    act( AT_SKILL, "You evade $n's attempt to disarm you.", ch, NULL, victim, TO_VICT    );
    act( AT_SKILL, "$N holds $S weapon strongly, and is not disarmed.",
         ch, NULL, victim, TO_CHAR    );
    learn_from_success( victim, gsn_grip );
    return TRUE;
}

void base_berserk(CHAR_DATA *ch)
{
    AFFECT_DATA af;

    af.type = gsn_berserk;
    /* Hmmm.. 10-20 combat rounds at level 50.. good enough for most mobs,
     and if not they can always go berserk again.. shrug.. maybe even
     too high. -- Altrag */
    af.duration = number_range(GetMaxLevel(ch)/5, GetMaxLevel(ch)*2/5);
    /* Hmm.. you get stronger when yer really enraged.. mind over matter
     type thing.. */
    af.location = APPLY_STR;
    af.modifier = 1;
    af.bitvector = AFF_BERSERK;
    affect_to_char(ch, &af);
    send_to_char( "You start to lose control..\n\r", ch );
}

/* Berserk and HitAll. -- Altrag */
void do_berserk( CHAR_DATA *ch, char *argument )
{
    sh_int percent;

    if ( !ch->fighting )
    {
        send_to_char( "But you aren't fighting!\n\r", ch );
        return;
    }

    if ( IS_AFFECTED(ch, AFF_BERSERK) )
    {
        send_to_char( "Your rage is already at its peak!\n\r", ch );
        return;
    }

    percent = IS_NPC(ch) ? 80 : LEARNED(ch, gsn_berserk);
    spell_lag(ch, gsn_berserk);
    if ( !chance(ch, percent) )
    {
        send_to_char( "You couldn't build up enough rage.\n\r", ch);
        learn_from_failure(ch, gsn_berserk);
        return;
    }
    base_berserk(ch);
    learn_from_success(ch, gsn_berserk);
    return;
}


bool check_illegal_psteal( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if (!IS_NPC (victim) && !IS_NPC(ch))
    {
        if ( ( !IS_SET( victim->pcdata->flags, PCFLAG_DEADLY )
               || GetMaxLevel(ch) - GetMaxLevel(victim) > 10
               || !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
             && ( ch->in_room->vnum < 29 || ch->in_room->vnum > 43 )
             && ch != victim )
        {
            /*
             sprintf( log_buf, "%s illegally stealing from %s at %d",
             (IS_NPC(ch) ? ch->short_descr : ch->name),
             victim->name,
             victim->in_room->vnum );
             log_string_plus( log_buf,LOG_MONITOR,LEVEL_IMMORTAL);
             */
            return TRUE;
        }
    }
    return FALSE;
}

static char *dir_desc[] =
{
    "to the north",
    "to the east",
    "to the south",
    "to the west",
    "upwards",
    "downwards",
    "to the northeast",
    "to the northwest",
    "to the southeast",
    "to the southwest",
    "through the portal"
};
static char *rng_desc[] =
{
    "right here",
    "immediately",
    "nearby",
    "a ways",
    "a good ways",
    "far",
    "far off",
    "very far",
    "very far off",
    "in the distance"
};

static void scanroom(CHAR_DATA *ch, ROOM_INDEX_DATA *room, int dir, int maxdist, int dist)
{
    CHAR_DATA *tch;
    EXIT_DATA *ex;

    for (tch=room->first_person;tch;tch=tch->next_in_room)
    {
        if (can_see(ch,tch))
            ch_printf(ch, "%-30s : %s %s\n\r",
                      PERS(tch,ch), rng_desc[dist], dist==0?"":dir_desc[dir]);
    }
    for (ex=room->first_exit;ex;ex=ex->next)
        if (ex->vdir==dir)
            break;

    if (!ex || ex->vdir!=dir || ex->vdir>=MAX_REXITS || maxdist-1==0 ||
        IS_SET(ex->exit_info, EX_CLOSED) || IS_SET(ex->exit_info, EX_DIG))
        return;

    if (ex->vdir == DIR_SOMEWHERE)
        return;

    scanroom(ch,ex->to_room,dir,maxdist-1,dist+1);
}

void do_scan(CHAR_DATA *ch, char *argument)
{
    int maxdist=1;
    EXIT_DATA *ex;

    if (number_percent() > LEARNED(ch, gsn_spot) )
    {
       send_to_char("You fail at trying to spot the rooms around you!\n\r", ch);
       learn_from_failure(ch, gsn_spot);
       return;
    }

    if (IS_NPC(ch))
        maxdist=GetMaxLevel(ch)/10;
    else
        maxdist=LEARNED(ch, gsn_spot)/20;

    maxdist=UMAX(maxdist,9);

    scanroom(ch,ch->in_room,-1,1,0);
    if ((ex = find_door(ch, argument, TRUE)))
        scanroom(ch,ex->to_room,ex->vdir,maxdist,1);
    else
        for (ex=ch->in_room->first_exit;ex;ex=ex->next)
        {
            if (IS_SET(ex->exit_info, EX_DIG) ||
                IS_SET(ex->exit_info, EX_CLOSED))
                continue;
	    if (ex->vdir>=MAX_REXITS)
		return;
	    if (ex->vdir == DIR_SOMEWHERE && !IS_IMMORTAL(ch))
                continue;
            scanroom(ch,ex->to_room,ex->vdir,maxdist,1);
        }
}

/*
 * Basically the same guts as do_scan() from above (please keep them in
 * sync) used to find the victim we're firing at.	-Thoric
 */
CHAR_DATA *scan_for_victim( CHAR_DATA *ch, EXIT_DATA *pexit, char *name )
{
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *was_in_room;
    sh_int dist, dir;
    sh_int max_dist = 8;

    if ( IS_AFFECTED(ch, AFF_BLIND) || !pexit )
        return NULL;

    was_in_room = ch->in_room;
    if ( IS_VAMPIRE(ch) && time_info.hour < 21 && time_info.hour > 5 )
        max_dist = 1;

    if ( GetMaxLevel(ch) < 50 ) --max_dist;
    if ( GetMaxLevel(ch) < 40 ) --max_dist;
    if ( GetMaxLevel(ch) < 30 ) --max_dist;

    for ( dist = 1; dist <= max_dist; )
    {
        if ( IS_SET(pexit->exit_info, EX_CLOSED) )
            break;

        if ( room_is_private( pexit->to_room )
             &&   GetMaxLevel(ch) < LEVEL_GREATER )
            break;

        char_from_room( ch );
        char_to_room( ch, pexit->to_room );

        if ( (victim=get_char_room(ch, name)) != NULL )
        {
            char_from_room(ch);
            char_to_room(ch, was_in_room);
            return victim;
        }

        switch( ch->in_room->sector_type )
        {
        default: dist++; break;
        case SECT_AIR:
            if ( number_percent() < 80 ) dist++; break;
        case SECT_INSIDE:
        case SECT_FIELD:
        case SECT_UNDERGROUND:
            dist++; break;
        case SECT_FOREST:
        case SECT_CITY:
        case SECT_DESERT:
        case SECT_HILLS:
            dist += 2; break;
        case SECT_WATER_SWIM:
        case SECT_WATER_NOSWIM:
            dist += 3; break;
        case SECT_MOUNTAIN:
        case SECT_UNDERWATER:
        case SECT_OCEANFLOOR:
            dist += 4; break;
        }

        if ( dist >= max_dist )
            break;

        dir = pexit->vdir;
        if ( (pexit=get_exit(ch->in_room, dir)) == NULL )
            break;
    }

    char_from_room(ch);
    char_to_room(ch, was_in_room);

    return NULL;
}

/*
 * Search inventory for an appropriate projectile to fire.
 * Also search open quivers.					-Thoric
 */
OBJ_DATA *find_projectile( CHAR_DATA *ch, int type )
{
    OBJ_DATA *obj, *obj2;

    for ( obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
        if ( can_see_obj(ch, obj) )
        {
            if ( obj->item_type == ITEM_QUIVER && !IS_SET(obj->value[1], CONT_CLOSED) )
            {
                for ( obj2 = obj->last_content; obj2; obj2 = obj2->prev_content )
                {
                    if ( obj2->item_type == ITEM_PROJECTILE
                         &&   obj2->value[2] == type )
                        return obj2;
                }
            }
            if ( obj->item_type == ITEM_PROJECTILE && obj->value[2] == type )
                return obj;
        }
    }

    return NULL;
}


ch_ret spell_attack( int, int, CHAR_DATA *, void * );

/*
 * Perform the actual attack on a victim			-Thoric
 */
ch_ret ranged_got_target( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *weapon,
                          OBJ_DATA *projectile, sh_int dist, sh_int dt, char *stxt, sh_int color )
{
    if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE) )
    {
        /* safe room, bubye projectile */
        if ( projectile )
        {
            ch_printf(ch, "Your %s is blasted from existance by a godly presense.",
                      myobj(projectile) );
            act( color, "A godly presence smites $p!", ch, projectile, NULL, TO_ROOM );
            extract_obj(projectile);
        }
        else
        {
            ch_printf(ch, "Your %s is blasted from existance by a godly presense.",
                      stxt );
            act( color, "A godly presence smites $t!", ch, aoran(stxt), NULL, TO_ROOM );
        }
        return rNONE;
    }

    if ( IS_NPC(victim) && IS_SET(victim->act, ACT_SENTINEL)
         &&   ch->in_room != victim->in_room )
    {
        /*
         * letsee, if they're high enough.. attack back with fireballs
         * long distance or maybe some minions... go herne! heh..
         *
         * For now, just always miss if not in same room  -Thoric
         */

        if ( projectile )
        {
            learn_from_failure( ch, gsn_archery );

            /* 50% chance of projectile getting lost */
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
        }
        return damage( ch, victim, 0, dt );
    }

    if ( number_percent() > 50 || (projectile && weapon
                                   &&   CanUseSkill(ch, gsn_archery)) )
    {
        if ( projectile )
            global_retcode = projectile_hit(ch, victim, weapon, projectile, dist );
        else
            global_retcode = spell_attack( dt, GetMaxLevel(ch), ch, victim );
    }
    else
    {
        learn_from_failure( ch, gsn_archery );
        global_retcode = damage( ch, victim, 0, dt );

        if ( projectile )
        {
            /* 50% chance of getting lost */
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
        }
    }
    return global_retcode;
}

/*
 * Generic use ranged attack function			-Thoric & Tricops
 */
ch_ret ranged_attack( CHAR_DATA *ch, char *argument, OBJ_DATA *weapon,
                      OBJ_DATA *projectile, sh_int dt, sh_int range )
{
    CHAR_DATA *victim, *vch;
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *was_in_room;
    char arg[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char temp[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    SKILLTYPE *skill = NULL;
    sh_int dir = -1, dist = 0, color = AT_GREY;
    char *dtxt = "somewhere";
    char *stxt = "burst of energy";
    int count;


    if ( argument && argument[0] != '\0' && argument[0] == '\''){
        one_argument( argument, temp );
        argument = temp;
    }

    argument = one_argument(argument, arg);
    argument = one_argument(argument, arg1);

    if ( arg[0] == '\0' )
    {
        send_to_char( "Where?  At who?\n\r", ch );
        return rNONE;
    }

    victim = NULL;

    /* get an exit or a victim */
    if ( (pexit = find_door(ch, arg, TRUE)) == NULL )
    {
        if ( (victim=get_char_room(ch, arg)) == NULL )
        {
            send_to_char( "Aim in what direction?\n\r", ch );
            return rNONE;
        }
        else
        {
            if ( who_fighting(ch) == victim )
            {
                send_to_char( "They are too close to release that type of attack!\n\r", ch );
                return rNONE;
            }
            if ( !IS_NPC(ch) && !IS_NPC(victim) )
            {
                send_to_char("Pkill like a real pkiller.\n\r", ch );
                return rNONE;
            }
        }
    }
    else
        dir = pexit->vdir;

    /* check for ranged attacks from private rooms, etc */
    if ( !victim )
    {
        if ( IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)
             ||   IS_SET(ch->in_room->room_flags, ROOM_SOLITARY) )
        {
            send_to_char( "You cannot perform a ranged attack from a private room.\n\r", ch );
            return rNONE;
        }
        if ( ch->in_room->tunnel > 0 )
        {
            count = 0;
            for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
                ++count;
            if ( count >= ch->in_room->tunnel )
            {
                send_to_char( "This room is too cramped to perform such an attack.\n\r", ch );
                return rNONE;
            }
        }
    }

    if ( IS_VALID_SN(dt) )
        skill = skill_table[dt];

    if ( pexit && !pexit->to_room )
    {
        send_to_char( "Are you expecting to fire through a wall!?\n\r", ch );
        return rNONE;
    }

    /* Check for obstruction */
    if ( pexit && IS_SET(pexit->exit_info, EX_CLOSED) )
    {
        if ( IS_SET(pexit->exit_info, EX_SECRET)
             ||   IS_SET(pexit->exit_info, EX_DIG) )
            send_to_char( "Are you expecting to fire through a wall!?\n\r", ch );
        else
            send_to_char( "Are you expecting to fire through a door!?\n\r", ch );
        return rNONE;
    }

    vch = NULL;
    if ( pexit && arg1[0] != '\0' )
    {
        if ( (vch=scan_for_victim(ch, pexit, arg1)) == NULL )
        {
            send_to_char( "You cannot see your target.\n\r", ch );
            return rNONE;
        }

        /* don't allow attacks on mobs stuck in another area?
         if ( IS_NPC(vch) && xIS_SET(vch->act, ACT_STAY_AREA)
         &&   ch->in_room->area != vch->in_room->area) )
         {
         }
         */
        /*don't allow attacks on mobs that are in a no-missile room --Shaddai */
        if ( IS_SET(vch->in_room->room_flags, ROOM_NOMISSILE) )
        {
            send_to_char( "You can't get a clean shot off.\n\r", ch );
            return rNONE;
        }
        if ( !IS_NPC(ch) && !IS_NPC(vch) )
        {
            send_to_char("Pkill like a real pkiller.\n\r", ch );
            return rNONE;
        }

        /* can't properly target someone heavily in battle */
        if ( vch->num_fighting > max_fight(vch) )
        {
            send_to_char( "There is too much activity there for you to get a clear shot.\n\r", ch );
            return rNONE;
        }
    }
    if ( vch ) {
        if ( !IS_NPC( vch ) && !IS_NPC( ch ) &&
             IS_SET(ch->act, PLR_NICE ) )
        {
            send_to_char( "Your too nice to do that!\n\r", ch );
            return rNONE;
        }
        if ( vch && is_safe(ch, vch) )
            return rNONE;
    }
    was_in_room = ch->in_room;

    if ( projectile )
    {
        separate_obj(projectile);
        if ( pexit )
        {
            if ( weapon )
            {
                act( AT_GREY, "You fire $p $T.", ch, projectile, dir_name(dir), TO_CHAR );
                act( AT_GREY, "$n fires $p $T.", ch, projectile, dir_name(dir), TO_ROOM );
            }
            else
            {
                act( AT_GREY, "You throw $p $T.", ch, projectile, dir_name(dir), TO_CHAR );
                act( AT_GREY, "$n throw $p $T.", ch, projectile, dir_name(dir), TO_ROOM );
            }
        }
        else
        {
            if ( weapon )
            {
                act( AT_GREY, "You fire $p at $N.", ch, projectile, victim, TO_CHAR );
                act( AT_GREY, "$n fires $p at $N.", ch, projectile, victim, TO_NOTVICT );
                act( AT_GREY, "$n fires $p at you!", ch, projectile, victim, TO_VICT );
            }
            else
            {
                act( AT_GREY, "You throw $p at $N.", ch, projectile, victim, TO_CHAR );
                act( AT_GREY, "$n throws $p at $N.", ch, projectile, victim, TO_NOTVICT );
                act( AT_GREY, "$n throws $p at you!", ch, projectile, victim, TO_VICT );
            }
        }
    }
    else
        if ( skill )
        {
            if ( skill->noun_damage && skill->noun_damage[0] != '\0' )
                stxt = skill->noun_damage;
            else
                stxt = skill->name;
            /* a plain "spell" flying around seems boring */
            if ( !str_cmp(stxt, "spell") )
                stxt = "magical burst of energy";
            if ( skill->type == SKILL_SPELL )
            {
                color = AT_MAGIC;
                if ( pexit )
                {
                    act( AT_MAGIC, "You release $t $T.", ch, aoran(stxt), dir_name(dir), TO_CHAR );
                    act( AT_MAGIC, "$n releases $s $t $T.", ch, stxt, dir_name(dir), TO_ROOM );
                }
                else
                {
                    act( AT_MAGIC, "You release $t at $N.", ch, aoran(stxt), victim, TO_CHAR );
                    act( AT_MAGIC, "$n releases $s $t at $N.", ch, stxt, victim, TO_NOTVICT );
                    act( AT_MAGIC, "$n releases $s $t at you!", ch, stxt, victim, TO_VICT );
                }
            }
        }
        else
        {
            bug( "Ranged_attack: no projectile, no skill dt %d", dt );
            return rNONE;
        }

    /* victim in same room */
    if ( victim )
    {
        check_illegal_pk( ch, victim );
        check_attacker( ch, victim );
        return ranged_got_target( ch, victim, weapon, projectile,
                                  0, dt, stxt, color );
    }

    /* assign scanned victim */
    victim = vch;

    /* reverse direction text from move_char */
    dtxt = dir_name(pexit->rdir);

    while ( dist <= range )
    {
        char_from_room(ch);
        char_to_room(ch, pexit->to_room);

        if ( IS_SET(pexit->exit_info, EX_CLOSED) )
        {
            /* whadoyahknow, the door's closed */
            if ( projectile )
                sprintf(buf,"You see your %s pierce a door in the distance to the %s.",
                        myobj(projectile), dir_name(dir) );
            else
                sprintf(buf, "You see your %s hit a door in the distance to the %s.",
                        stxt, dir_name(dir) );
            act( color, buf, ch, NULL, NULL, TO_CHAR );
            if ( projectile )
            {
                sprintf(buf,"$p flies in from %s and implants itself solidly in the %sern door.",
                        dtxt, dir_name(dir) );
                act( color, buf, ch, projectile, NULL, TO_ROOM );
            }
            else
            {
                sprintf(buf, "%s flies in from %s and implants itself solidly in the %sern door.",
                        aoran(stxt), dtxt, dir_name(dir) );
                buf[0] = UPPER(buf[0]);
                act( color, buf, ch, NULL, NULL, TO_ROOM );
            }
            break;
        }


        /* no victim? pick a random one */
        if ( !victim )
        {
            for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
            {
                if ( ((IS_NPC(ch) && !IS_NPC(vch))
                      ||   (!IS_NPC(ch) &&  IS_NPC(vch)))
                     &&    number_bits(1) == 0 )
                {
                    victim = vch;
                    break;
                }
            }
            if ( victim && is_safe(ch, victim) )
            {
                char_from_room(ch);
                char_to_room(ch, was_in_room);
                return rNONE;
            }
        }

        /* In the same room as our victim? */
        if ( victim && ch->in_room == victim->in_room )
        {
            if ( projectile )
                act( color, "$p flies in from $T.", ch, projectile, dtxt, TO_ROOM );
            else
                act( color, "$t flies in from $T.", ch, aoran(stxt), dtxt, TO_ROOM );

            /* get back before the action starts */
            char_from_room(ch);
            char_to_room(ch, was_in_room);

            check_illegal_pk( ch, victim );
            check_attacker( ch, victim );
            return ranged_got_target( ch, victim, weapon, projectile,
                                      dist, dt, stxt, color );
        }

        if ( dist == range )
        {
            if ( projectile )
            {
                act( color, "Your $t falls harmlessly to the ground to the $T.",
                     ch, myobj(projectile), dir_name(dir), TO_CHAR );
                act( color, "$p flies in from $T and falls harmlessly to the ground here.",
                     ch, projectile, dtxt, TO_ROOM );
                if ( projectile->in_obj )
                    obj_from_obj(projectile);
                if ( projectile->carried_by )
                    obj_from_char(projectile);
                obj_to_room(projectile, ch->in_room);
            }
            else
            {
                act( color, "Your $t fizzles out harmlessly to the $T.", ch, stxt, dir_name(dir), TO_CHAR );
                act( color, "$t flies in from $T and fizzles out harmlessly.",
                     ch, aoran(stxt), dtxt, TO_ROOM );
            }
            break;
        }

        if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
        {
            if ( projectile )
            {
                act( color, "Your $t hits a wall and bounces harmlessly to the ground to the $T.",
                     ch, myobj(projectile), dir_name(dir), TO_CHAR );
                act( color, "$p strikes the $Tsern wall and falls harmlessly to the ground.",
                     ch, projectile, dir_name(dir), TO_ROOM );
                if ( projectile->in_obj )
                    obj_from_obj(projectile);
                if ( projectile->carried_by )
                    obj_from_char(projectile);
                obj_to_room(projectile, ch->in_room);
            }
            else
            {
                act( color, "Your $t harmlessly hits a wall to the $T.",
                     ch, stxt, dir_name(dir), TO_CHAR );
                act( color, "$t strikes the $Tsern wall and falls harmlessly to the ground.",
                     ch, aoran(stxt), dir_name(dir), TO_ROOM );
            }
            break;
        }
        if ( projectile )
            act( color, "$p flies in from $T.", ch, projectile, dtxt, TO_ROOM );
        else
            act( color, "$t flies in from $T.", ch, aoran(stxt), dtxt, TO_ROOM );
        dist++;
    }

    char_from_room( ch );
    char_to_room( ch, was_in_room );

    return rNONE;
}

/*
 * Fire <direction> <target>
 *
 * Fire a projectile from a missile weapon (bow, crossbow, etc)
 *
 * Design by Thoric, coding by Thoric and Tricops.
 *
 * Support code (see projectile_hit(), quiver support, other changes to
 * fight.c, etc by Thoric.
 */
void do_fire( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *arrow;
    OBJ_DATA *bow;
    sh_int max_dist;

    if ( argument[0] == '\0' || !str_cmp(argument, " ") )
    {
        send_to_char( "Fire where at who?\n\r", ch );
        return;
    }

    if ( !IS_IMMORTAL(ch) && IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "A magical force prevents you from attacking.\n\r", ch );
        return;
    }

    /*
     * Find the projectile weapon
     */
    if ( (bow=get_eq_char(ch, WEAR_WIELD)) != NULL )
        if ( !(bow->item_type == ITEM_MISSILE_WEAPON) )
            bow = NULL;

    if ( !bow )
    {
        send_to_char( "You are not wielding a missile weapon.\n\r", ch );
        return;
    }

    if ( (arrow=get_eq_char(ch, WEAR_MISSILE_WIELD)) != NULL )
        if ( !(arrow->item_type == ITEM_PROJECTILE) )
            arrow = NULL;

    if ( !arrow )
    {
        send_to_char( "You do not have the proper ammunition loaded.\n\r", ch );
        return;
    }

    /* modify maximum distance based on bow-type and ch's class/str/etc */
    max_dist = URANGE( 1, bow->value[2], 10 );

/*    if ( (arrow=find_projectile(ch, bow->value[3])) == NULL )
    {
        char *msg = "You have nothing to fire...\n\r";

        switch( bow->value[3] )
        {
        case DAM_BOLT:	msg = "You have no bolts...\n\r";	break;
        case DAM_ARROW:	msg = "You have no arrows...\n\r";	break;
        case DAM_DART:	msg = "You have no darts...\n\r";	break;
        case DAM_STONE:	msg = "You have no slingstones...\n\r";	break;
        case DAM_PEA:	msg = "You have no peas...\n\r";	break;
        }
        send_to_char( msg, ch );
        return;
    }*/

    /* handle the ranged attack */
    ranged_attack( ch, argument, bow, arrow, TYPE_HIT + arrow->value[2], max_dist );

    return;
}

/*
 * Attempt to fire at a victim.
 * Returns FALSE if no attempt was made
 */
bool mob_fire( CHAR_DATA *ch, char *name )
{
    OBJ_DATA *arrow;
    OBJ_DATA *bow;
    sh_int max_dist;

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
        return FALSE;

    if ( (bow=get_eq_char(ch, WEAR_MISSILE_WIELD)) != NULL )
        if ( !(bow->item_type == ITEM_MISSILE_WEAPON) )
            bow = NULL;

    if ( !bow )
        return FALSE;

    /* modify maximum distance based on bow-type and ch's class/str/etc */
    max_dist = URANGE( 1, bow->value[2], 10 );

    if ( (arrow=find_projectile(ch, bow->value[3])) == NULL )
        return FALSE;

    ranged_attack( ch, name, bow, arrow, TYPE_HIT + arrow->value[2], max_dist );

    return TRUE;
}

/* -- working on --
 * Syntaxes: throw object  (assumed already fighting)
 *	     throw object direction target  (all needed args for distance
 *	          throwing)
 *	     throw object  (assumed same room throw)

 void do_throw( CHAR_DATA *ch, char *argument )
 {
 ROOM_INDEX_DATA *was_in_room;
 CHAR_DATA *victim;
 OBJ_DATA *throw_obj;
 EXIT_DATA *pexit;
 sh_int dir;
 sh_int dist;
 sh_int max_dist = 3;
 char arg[MAX_INPUT_LENGTH];
 char arg1[MAX_INPUT_LENGTH];
 char arg2[MAX_INPUT_LENGTH];

 argument = one_argument( argument, arg );
 argument = one_argument( argument, arg1 );
 argument = one_argument( argument, arg2 );

 for ( throw_obj = ch->last_carrying; throw_obj;
 throw_obj = throw_obj=>prev_content )
 {
 ---    if ( can_see_obj( ch, throw_obj )
 && ( throw_obj->wear_loc == WEAR_HELD || throw_obj->wear_loc ==
 WEAR_WIELDED || throw_obj->wear_loc == WEAR_DUAL_WIELDED )
 && nifty_is_name( arg, throw_obj->name ) )
 break;
 ----
 if ( can_see_obj( ch, throw_obj ) && nifty_is_name( arg, throw_obj->name )
 break;
 }

 if ( !throw_obj )
 {
 send_to_char( "You aren't holding or wielding anything like that.\n\r", ch );
 return;
 }

 ----
 if ( ( throw_obj->item_type != ITEM_WEAPON)
 {
 send_to_char("You can only throw weapons.\n\r", ch );
 return;
 }
 ----

 if (get_obj_weight( throw_obj ) - ( 3 * (get_curr_str(ch) - 15) ) > 0)
 {
 send_to_char("That is too heavy for you to throw.\n\r", ch);
 if (!number_range(0,10))
 learn_from_failure( ch, gsn_throw );
 return;
 }

 if ( ch->fighting )
 victim = ch->fighting;
 else
 {
 if ( ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
 && ( arg2[0] == '\0' ) )
 {
 act( AT_GREY, "Throw $t at whom?", ch, obj->short_descr, NULL,
 TO_CHAR );
 return;
 }
 }
 }*/

/*
 *
 *
 void do_release( CHAR_DATA *ch, char *argument )
 {
 CHAR_DATA *victim;
 OBJ_INDEX_DATA *arrow;
 OBJ_DATA *quiver;
 OBJ_DATA *bow;
 EXIT_DATA *pexit;
 ROOM_INDEX_DATA *was_in_room;
 char arg[MAX_INPUT_LENGTH];
 char arg1[MAX_INPUT_LENGTH];
 sh_int dir;
 sh_int dist;
 sh_int max_dist = 3;

 for ( bow = ch->last_carrying; bow; bow = bow->prev_content )
 {
 if ( can_see_obj( ch, bow )
 && ( bow->wear_loc == WEAR_WIELDED
 || bow->wear_loc == WEAR_DUAL_WIELDED )
 && ( ( bow->item_type == ITEM_SHORT_BOW ) || ( bow->item_type ==
 ITEM_LONG_BOW ) || ( bow->item_type == ITEM_CROSS_BOW ) ) )
 break;
 }

 if ( !bow )
 {
 send_to_char( "You are not wielding a bow.\n\r", ch );
 return;
 }

 switch ( bow->item_type )
 {
 case ITEM_SHORT_BOW: max_dist = 4; break;
 case ITEM_LONG_BOW:  max_dist = 5; break;
 case ITEM_CROSS_BOW: max_dist = 6; break;
 }

 for ( quiver = ch->last_carrying; quiver; quiver = quiver->prev )
 {
 if ( can_see_obj( ch, quiver )
 && ( quiver->item_type == ITEM_QUIVER ) )
 break;
 }

 for ( arrow = ch->last_carrying; arrow; arrow = arrow->prev )
 {
 if ( can_see_obj( ch, arrow )
 && ( arrow->item_type == ITEM_PROJECTILE ) )
 break;
 }

 if ( ( dir = get_door( arg ) ) == -1 )
 {
 send_to_char( "Aim in what direction?\n\r", ch );
 return;
 }

 if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
 {
 send_to_char( "Are you expecting to fire through a wall!?\n\r", ch );
 return;
 }

 if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
 {
 send_to_char( "Are you expecting to fire through a door!?\n\r", ch );
 return;
 }

 was_in_room = ch->in_room;

 act( AT_GREY, "You release an arrow $t.", ch, dir_name(dir), NULL,
 TO_CHAR );
 act( AT_GREY, "$n releases an arrow $t.", ch, dir_name(dir), NULL,
 TO_ROOM );

 for ( dist = 0; dist <= max_dist; )
 {
 if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
 {
 act( AT_GREY, "You see your arrow pierce a door in the distance.",
 ch, NULL, NULL, TO_CHAR );
 break;
 }

 char_from_room( ch );
 char_to_room( ch, pexit->to_room );

 if ( ( victim = get_char_room( ch, arg1 ) ) != NULL )
 {
 }

 if ( dist == max_dist ) break;

 if ( ( pexit = get_exit( ch->in_room, dir ) ) == NULL )
 {
 }

 }

 char_from_room( ch );
 char_to_room( ch, was_in_room );

 return;
 }
 */

/* -- working on --
 * Syntaxes: throw object  (assumed already fighting)
 *	     throw object direction target  (all needed args for distance
 *	          throwing)
 *	     throw object  (assumed same room throw)

 void do_throw( CHAR_DATA *ch, char *argument )
 {
 ROOM_INDEX_DATA *was_in_room;
 CHAR_DATA *victim;
 OBJ_DATA *throw_obj;
 EXIT_DATA *pexit;
 sh_int dir;
 sh_int dist;
 sh_int max_dist = 3;
 char arg[MAX_INPUT_LENGTH];
 char arg1[MAX_INPUT_LENGTH];
 char arg2[MAX_INPUT_LENGTH];

 argument = one_argument( argument, arg );
 argument = one_argument( argument, arg1 );
 argument = one_argument( argument, arg2 );

 for ( throw_obj = ch->last_carrying; throw_obj;
 throw_obj = throw_obj=>prev_content )
 {
 if ( can_see_obj( ch, throw_obj )
 && ( throw_obj->wear_loc == WEAR_HELD || throw_obj->wear_loc ==
 WEAR_WIELDED || throw_obj->wear_loc == WEAR_DUAL_WIELDED )
 && nifty_is_name( arg, throw_obj->name ) )
 break;
 }

 if ( !throw_obj )
 {
 send_to_char( "You aren't holding or wielding anything like that.\n\r",
 ch );
 return;
 }

 if ( ( throw_obj->item_type != ITEM_WEAPON)
 {
 send_to_char("You can only throw weapons.\n\r", ch );
 return;
 }

 if (get_obj_weight( throw_obj ) - ( 3 * (get_curr_str(ch) - 15) ) > 0)
 {
 send_to_char("That is too heavy for you to throw.\n\r", ch);
 learn_from_failure( ch, gsn_throw );
 return;
 }

 if ( ch->fighting )
 victim = ch->fighting;
 else
 {
 if ( ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
 && ( arg2[0] == '\0' ) )
 {
 act( AT_GREY, "Throw $t at whom?", ch, obj->short_descr, NULL,
 TO_CHAR );
 return;
 }
 }
 }*/

void do_slice( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *slice;
    bool found;
    MOB_INDEX_DATA *pMobIndex;
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    found = FALSE;


    if ( !IS_NPC(ch) && !IS_IMMORTAL(ch)
         &&  !CanUseSkill(ch, gsn_slice) )
    {
        send_to_char("You are not learned in this skill.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char("From what do you wish to slice meat?\n\r", ch);
        return;
    }


    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL
         ||   ( obj->value[3] != 1 && obj->value[3] != 2 && obj->value[3] != 3
                && obj->value[3] != 11) )
    {
        send_to_char( "You need to wield a sharp weapon.\n\r", ch);
        return;
    }

    if ( (corpse = get_obj_here( ch, argument )) == NULL)
    {
        send_to_char("You can't find that here.\n\r", ch);
        return;
    }

    if (corpse->item_type != ITEM_CORPSE_NPC || corpse->value[2] < 6)
    {
        send_to_char("That is not a suitable source of meat.\n\r", ch);
        return;
    }

    if ( (pMobIndex = get_mob_index((sh_int) (0 - corpse->cost) )) == NULL )
    {
        bug("Can not find mob vnum of corpse, do_slice");
        return;
    }

    if ( !obj_exists_index(OBJ_VNUM_SLICE) )
    {
        bug("Vnum %d not found for do_slice!", OBJ_VNUM_SLICE);
        return;
    }

    if ( !IS_NPC(ch) && !IS_IMMORTAL(ch) && number_percent() > LEARNED(ch, gsn_slice) )
    {
        send_to_char("You fail to slice the meat properly.\n\r", ch);
        learn_from_failure(ch, gsn_slice); /* Just in case they die :> */
        if ( number_percent() + (get_curr_dex(ch) - 13) < 10)
        {
            act(AT_BLOOD, "You cut yourself!", ch, NULL, NULL, TO_CHAR);
            damage(ch, ch, BestSkLv(ch, gsn_slice), gsn_slice);
        }
        return;
    }

    slice = create_object( OBJ_VNUM_SLICE );

    sprintf(buf, "meat fresh slice %s", pMobIndex->player_name);
    STRFREE(slice->name);
    slice->name = STRALLOC(buf);

    sprintf(buf, "a slice of raw meat from %s", pMobIndex->short_descr);
    STRFREE(slice->short_descr);
    slice->short_descr = STRALLOC(buf);

    sprintf(buf1, "A slice of raw meat from %s lies on the ground.", pMobIndex->short_descr);
    STRFREE(slice->description);
    slice->description = STRALLOC(buf1);

    act( AT_BLOOD, "$n cuts a slice of meat from $p.", ch, corpse, NULL, TO_ROOM);
    act( AT_BLOOD, "You cut a slice of meat from $p.", ch, corpse, NULL, TO_CHAR);

    obj_to_char(slice, ch);
    corpse->value[3] -= 25;
    learn_from_success(ch, gsn_slice);
    return;
}

extern int pAbort;

void *locate_targets( CHAR_DATA *ch, char *arg, int sn, CHAR_DATA **victim, OBJ_DATA **obj );

void do_smaug_skill( CHAR_DATA *ch, char *argument )
{
#if 1
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    static char staticbuf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    void *vo = NULL;
    int mana;
    int blood;
    int sn;
    ch_ret retcode;
    bool dont_wait = FALSE;
    SKILLTYPE *skill = NULL;
    struct timeval time_used;

    retcode = rNONE;

    switch( ch->substate )
    {
    default:
        /* no ordering charmed mobs to cast spells */
        if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
        {
            send_to_char( "You can't seem to do that right now...\n\r", ch );
            return;
        }

        target_name = one_argument( argument, arg1 );
        one_argument( target_name, arg2 );

        if ( get_trust(ch) < LEVEL_GOD )
        {
            if ( ( sn = find_skill( ch, arg1, TRUE ) ) < 0
                 || ( !IS_NPC(ch) && !CanUseSkill(ch, sn) ) )
            {
                send_to_char( "You can't do that.\n\r", ch );
                bug("You can't do that in do_smaug_skill");
                return;
            }
            if ( (skill=get_skilltype(sn)) == NULL )
            {
                send_to_char( "You can't do that right now...\n\r", ch );
                return;
            }
        }
        else
        {
            if ( (sn=skill_lookup(arg1)) < 0 )
            {
                send_to_char( "We didn't create that yet...\n\r", ch );
                return;
            }
            if ( sn >= MAX_SKILL )
            {
                send_to_char( "Hmm... that might hurt.\n\r", ch );
                return;
            }
            if ( (skill=get_skilltype(sn)) == NULL )
            {
                send_to_char( "Somethis is severely wrong with that one...\n\r", ch );
                return;
            }
            if ( skill->type != SKILL_SKILL )
            {
                send_to_char( "That isn't a skill.\n\r", ch );
                return;
            }
            if ( !skill->skill_fun )
            {
                send_to_char( "We didn't finish that one yet...\n\r", ch );
                return;
            }
        }

        /*
         * Something else removed by Merc			-Thoric
         */
        if ( ch->position < skill->minimum_position )
        {
            switch( ch->position )
            {
            default:
                send_to_char( "You can't concentrate enough.\n\r", ch );
                break;
            case POS_SITTING:
            case POS_MEDITATING:
                send_to_char( "You can't seem to do this sitting down.\n\r", ch );
                break;
            case POS_RESTING:
                send_to_char( "You're too relaxed to do that.\n\r", ch );
                break;
            case POS_FIGHTING:
                send_to_char( "You can't concentrate enough while fighting!\n\r", ch );
                break;
            case POS_SLEEPING:
                send_to_char( "In your dreams?\n\r", ch );
                break;
            }
            return;
        }

        if ( !skill->skill_fun )
        {
            send_to_char( "You cannot cast that... yet.\n\r", ch );
            return;
        }

        if ( !IS_NPC(ch)			/* fixed by Thoric */
             &&   !IS_IMMORTAL(ch)
             &&    skill->guild != CLASS_NONE
             &&  (!ch->pcdata->clan
                  || skill->guild != ch->pcdata->clan->cl) )
        {
            send_to_char( "That is only available to members of a certain guild.\n\r", ch);
            return;
        }

        mana = IS_NPC(ch) ? 0 : skill->min_mana;
        /*
         * Locate targets.
         */
        vo = locate_targets( ch, arg2, sn, &victim, &obj );
        if ( vo == &pAbort )
            return;

        if ( !IS_NPC( ch ) && victim && !IS_NPC( victim )
             &&    CAN_PKILL( victim ) && !CAN_PKILL( ch )
             &&   !in_arena( ch ) && !in_arena( victim ) )
        {
            set_char_color( AT_GREY, ch );
            send_to_char( "The gods will not permit you to do that.\n\r", ch );
            return;
        }

        /*
         * Vampire spell casting				-Thoric
         */
        blood = UMAX(1, (mana+4) / 8);      /* NPCs don't have PCDatas. -- Altrag */
        if ( IS_VAMPIRE(ch) )
        {
            if (GET_COND(ch, COND_BLOODTHIRST) < blood)
            {
                send_to_char( "You don't have enough blood power.\n\r", ch );
                return;
            }
        }
        else
            if ( !IS_NPC(ch) && GET_MANA(ch) < mana )
            {
                send_to_char( "You don't have enough mana.\n\r", ch );
                return;
            }
        if ( skill->participants <= 1 )
            break;
        /* multi-participant spells			-Thoric */
        add_timer( ch, TIMER_DO_FUN,
                   UMAX(skill->beats/SPELL_BEATS_PER_ROUND, 1),
                   do_cast, 1 );

        if (skill->part_start_char && *skill->part_start_char)
            act( AT_MAGIC, skill->part_start_char, ch, NULL, NULL, TO_CHAR );
        else
            act( AT_MAGIC, "You begin to chant...", ch, NULL, NULL, TO_CHAR );
        if (skill->part_start_room && *skill->part_start_room)
            act( AT_MAGIC, skill->part_start_room, ch, NULL, NULL, TO_ROOM );
        else
            act( AT_MAGIC, "$n begins to chant...", ch, NULL, NULL, TO_ROOM );

        sprintf( staticbuf, "'%s' %s", arg2, target_name );
        ch->dest_buf = str_dup( staticbuf );
        ch->tempnum = sn;
        return;
    case SUB_TIMER_DO_ABORT:
        DISPOSE( ch->dest_buf );
        if ( IS_VALID_SN((sn = ch->tempnum)) )
        {
            if ( (skill=get_skilltype(sn)) == NULL )
            {
                send_to_char( "Something went wrong...\n\r", ch );
                bug( "do_cast: SUB_TIMER_DO_ABORT: bad sn %d", sn );
                return;
            }
            mana = IS_NPC(ch) ? 0 : skill->min_mana;
            blood = UMAX(1, (mana+4) / 8);
            if ( IS_VAMPIRE(ch) )
                gain_condition( ch, COND_BLOODTHIRST, - UMAX(1, blood / 3) );
            else
                if (GetMaxLevel(ch) < LEVEL_IMMORTAL)    /* so imms dont lose mana */
                    GET_MANA(ch) -= mana / 3;
        }
        if (skill->part_abort_char && *skill->part_abort_char)
            act( AT_GREY, skill->part_abort_char, ch, NULL, NULL, TO_CHAR );
        else
            act( AT_GREY, "You stop chanting...", ch, NULL, NULL, TO_CHAR );
        /* should add chance of backfire here */
        return;
    case 1:
        sn = ch->tempnum;
        if ( (skill=get_skilltype(sn)) == NULL )
        {
            send_to_char( "Something went wrong...\n\r", ch );
            bug( "do_smaug_skill: substate 1: bad sn %d", sn );
            return;
        }
        if ( !ch->dest_buf || !IS_VALID_SN(sn) || skill->type != SKILL_SPELL )
        {
            send_to_char( "Something cancels out the spell!\n\r", ch );
            bug( "do_smaug_skill: ch->dest_buf NULL or bad sn (%d)", sn );
            return;
        }
        mana = IS_NPC(ch) ? 0 : skill->min_mana;
        blood = UMAX(1, (mana+4) / 8);
        strcpy( staticbuf, (const char *)ch->dest_buf );
        target_name = one_argument(staticbuf, arg2);
        DISPOSE( ch->dest_buf );
        ch->substate = SUB_NONE;
        if ( skill->participants > 1 )
        {
            int cnt = 1;
            CHAR_DATA *tmp;
            TIMER *t;

            for ( tmp = ch->in_room->first_person; tmp; tmp = tmp->next_in_room )
                if (  tmp != ch
                      &&   (t = get_timerptr( tmp, TIMER_DO_FUN )) != NULL
                      &&    t->count >= 1 && t->do_fun == do_cast
                      &&    tmp->tempnum == sn && tmp->dest_buf
                      &&   !str_cmp( (const char *)tmp->dest_buf, staticbuf ) )
                    ++cnt;
            if ( cnt >= skill->participants )
            {
                for ( tmp = ch->in_room->first_person; tmp; tmp = tmp->next_in_room )
                    if (  tmp != ch
                          &&   (t = get_timerptr( tmp, TIMER_DO_FUN )) != NULL
                          &&    t->count >= 1 && t->do_fun == do_cast
                          &&    tmp->tempnum == sn && tmp->dest_buf
                          &&   !str_cmp( (const char *)tmp->dest_buf, staticbuf ) )
                    {
                        extract_timer( tmp, t );

                        if (skill->part_end_vict && *skill->part_end_vict)
                            act( AT_GREY, skill->part_end_vict, ch, NULL, tmp, TO_VICT );
                        else
                            act( AT_GREY, "You channel your energy into $n!", ch, NULL, tmp, TO_VICT );
                        if (skill->part_end_char && *skill->part_end_char)
                            act( AT_GREY, skill->part_end_char, ch, NULL, tmp, TO_CHAR );
                        else
                            act( AT_GREY, "$N channels $S energy into you!", ch, NULL, tmp, TO_CHAR );
                        if (skill->part_end_room && *skill->part_end_room)
                            act( AT_GREY, skill->part_end_room, ch, NULL, tmp, TO_NOTVICT );
                        else
                            act( AT_GREY, "$N channels $S energy into $n!", ch, NULL, tmp, TO_NOTVICT );

                        learn_from_success( tmp, sn );
                        if ( IS_VAMPIRE(ch) )
                            gain_condition( tmp, COND_BLOODTHIRST, - blood );
                        else if (!IS_IMMORTAL(tmp))
                            GET_MANA(tmp) -= mana;
                        tmp->substate = SUB_NONE;
                        tmp->tempnum = -1;
                        DISPOSE( tmp->dest_buf );
                    }
                dont_wait = TRUE;
                if (skill->part_end_caster && *skill->part_end_caster)
                    act( AT_GREY, skill->part_end_caster, ch, NULL, NULL, TO_CHAR );
                else
                    act( AT_GREY, "You concentrate all the energy into a single burst!", ch, NULL, NULL, TO_CHAR );
                vo = locate_targets( ch, arg2, sn, &victim, &obj );
                if ( vo == &pAbort )
                    return;
            }
            else
            {
                if (skill->part_miss_char && *skill->part_miss_char)
                    act( AT_GREY, skill->part_miss_char, ch, NULL, NULL, TO_CHAR );
                else
                    act( AT_GREY, "There was not enough power for success...", ch, NULL, NULL, TO_CHAR );
                if (skill->part_miss_room && *skill->part_miss_room)
                    act( AT_GREY, skill->part_miss_room, ch, NULL, NULL, TO_ROOM );

                if ( IS_VAMPIRE(ch) )
                    gain_condition( ch, COND_BLOODTHIRST, - UMAX(1, blood / 2) );
                else
                    if (GetMaxLevel(ch) < LEVEL_IMMORTAL)    /* so imms dont lose mana */
                        GET_MANA(ch) -= mana / 2;
                learn_from_failure( ch, sn );
                return;
            }
        }
    }

    if ( !dont_wait )
        /*if (!IS_NPC(ch))*/
            spell_lag(ch, sn);

    /*
     * Getting ready to cast... check for spell components	-Thoric
     */
    if ( !process_spell_components( ch, sn ) )
    {
        if ( IS_VAMPIRE(ch) )
            gain_condition( ch, COND_BLOODTHIRST, - UMAX(1, blood / 2) );
        else
            if (GetMaxLevel(ch) < LEVEL_IMMORTAL)    /* so imms dont lose mana */
                GET_MANA(ch) -= mana / 2;
        learn_from_failure( ch, sn );
        return;
    }

    if ( !IS_NPC(ch)
         &&   (number_percent( ) + skill->difficulty * 5) > LEARNED(ch, sn) )
    {
        /* Some more interesting loss of concentration messages  -Thoric */
        switch( number_bits(2) )
        {
        case 0:	/* too busy */
            if ( ch->fighting )
                send_to_char( "This round of battle is too hectic to concentrate properly.\n\r", ch );
            else
                send_to_char( "You lost your concentration.\n\r", ch );
            break;
        case 1:	/* irritation */
            if ( number_bits(2) == 0 )
            {
                switch( number_bits(2) )
                {
                case 0: send_to_char( "A tickle in your nose prevents you from keeping your concentration.\n\r", ch ); break;
                case 1: send_to_char( "An itch on your leg keeps you from properly finishing.\n\r", ch ); break;
                case 2: send_to_char( "Something prevents you from doing that task.\n\r", ch ); break;
                case 3: send_to_char( "A twitch in your eye disrupts your concentration for a moment.\n\r", ch ); break;
                }
            }
            else
                send_to_char( "Something distracts you, and you lose your concentration.\n\r", ch );
            break;
        case 2:	/* not enough time */
            if ( ch->fighting )
                send_to_char( "There wasn't enough time this round to complete what you were doing.\n\r", ch );
            else
                send_to_char( "You lost your concentration.\n\r", ch );
            break;
        case 3:
            send_to_char( "You get a mental block mid-way through what you were doing.\n\r", ch );
            break;
        }
        if ( IS_VAMPIRE(ch) )
            gain_condition( ch, COND_BLOODTHIRST, - UMAX(1, blood / 2) );
        else
            if (GetMaxLevel(ch) < LEVEL_IMMORTAL)    /* so imms dont lose mana */
                GET_MANA(ch) -= mana / 2;
        learn_from_failure( ch, sn );
        return;
    }
    else
    {
        if ( IS_VAMPIRE(ch) )
            gain_condition( ch, COND_BLOODTHIRST, - blood );
        else
            GET_MANA(ch) -= mana;

        start_timer(&time_used);
        if ( SPELL_FLAG(skill, SF_VERBALIZE_SKILL) ) {
            sprintf(buf, "%s!", skill->name);
            buf[0] = toupper(buf[0]);
            do_shout( ch, buf);
        }
        retcode = spell_smaug ( sn, BestSkLv(ch, sn), ch, vo );
        end_timer(&time_used);
        update_userec(&time_used, &skill->userec);
    }

    if ( retcode == rCHAR_DIED || retcode == rERROR || char_died(ch) )
        return;
    if ( retcode != rSPELL_FAILED )
        learn_from_success( ch, sn );
    else
        learn_from_failure( ch, sn );

    /*
     * Fixed up a weird mess here, and added double safeguards	-Thoric
     */
    if ( skill->target == TAR_CHAR_OFFENSIVE
         &&   victim
         &&  !char_died(victim)
         &&	 victim != ch )
    {
        CHAR_DATA *vch, *vch_next;

        for ( vch = ch->in_room->first_person; vch; vch = vch_next )
        {
            vch_next = vch->next_in_room;

            if ( vch == victim )
            {
                if ( victim->master != ch
                     &&  !victim->fighting )
                    retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
                break;
            }
        }
    }

    return;
#endif
}


/*
 * Cook was coded by Blackmane and heavily modified by Shaddai
 */
void do_cook ( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *food, *fire;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    one_argument( argument, arg );
    if ( IS_NPC(ch) || !CanUseSkill(ch, gsn_cook))
    {
        send_to_char("That skill is beyond your understanding.\n\r", ch );
        return;
    }
    if ( arg[0] == '\0' )
    {
        send_to_char("Cook what?\n\r", ch );
        return;
    }

    if ( ms_find_obj(ch) )
        return;

    if ( ( food = get_obj_carry( ch, arg ) ) == NULL )
    {
        send_to_char("You do not have that item.\n\r", ch );
        return;
    }
    if ( food->item_type != ITEM_COOK )
    {
        send_to_char("How can you cook that?\n\r", ch );
        return;
    }
    if ( food->value[2] > 2 )
    {
        send_to_char("That is already burnt to a crisp.\n\r", ch );
        return;
    }
    for ( fire = ch->in_room->first_content; fire; fire = fire->next_content )
    {
        if ( fire->item_type == ITEM_FIRE )
            break;
    }
    if ( !fire )
    {
        send_to_char("There is no fire here!\n\r", ch );
        return;
    }
    if ( number_percent() > LEARNED(ch, gsn_cook)  )
    {
        food->timer = food->timer/2;
        food->value[0] = 0;
        food->value[2] = 3;
        act( AT_MAGIC, "$p catches on fire burning it to a crisp!\n\r",
             ch, food, NULL, TO_CHAR );
        act( AT_MAGIC, "$n catches $p on fire burning it to a crisp.",
             ch, food, NULL, TO_ROOM);
        sprintf( buf, "a burnt %s", food->pIndexData->name );
        STRFREE(food->short_descr);
        food->short_descr = STRALLOC(buf);
        sprintf( buf, "A burnt %s.", food->pIndexData->name);
        STRFREE(food->description);
        food->description = STRALLOC(buf);
        return;
    }

    if ( number_percent() > 85 )
    {
        food->timer = food->timer*3;
        food->value[2]+=2;
        act( AT_MAGIC, "$n overcooks a $p.",ch, food, NULL, TO_ROOM);
        act( AT_MAGIC, "You overcook a $p.",ch, food, NULL, TO_CHAR);
        sprintf( buf, "an overcooked %s", food->pIndexData->name );
        STRFREE(food->short_descr);
        food->short_descr = STRALLOC(buf);
        sprintf( buf, "An overcooked %s.", food->pIndexData->name);
        STRFREE(food->description);
        food->description = STRALLOC(buf);
    }
    else
    {
        food->timer = food->timer*4;
        food->value[0] *= 2;
        act( AT_MAGIC, "$n roasts a $p.",ch, food, NULL, TO_ROOM);
        act( AT_MAGIC, "You roast a $p.",ch, food, NULL, TO_CHAR);
        sprintf( buf, "a roasted %s", food->pIndexData->name );
        STRFREE(food->short_descr);
        food->short_descr = STRALLOC(buf);
        sprintf( buf, "A roasted %s.", food->pIndexData->name);
        STRFREE(food->description);
        food->description = STRALLOC(buf);
        food->value[2]++;
    }
    learn_from_success(ch, gsn_cook);
}

void do_forge(CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *forge = NULL;
    OBJ_DATA *material = NULL;
    OBJ_DATA *newitem = NULL;
    AFFECT_DATA *paf = NULL, *paf2 = NULL, *paf3 = NULL;
    char itemname[MAX_INPUT_LENGTH], itemtype[MAX_INPUT_LENGTH], sdesc[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int percent=0, quality = 0;
    int gsn_forge, hplus = 0, dplus = 0;

    if ( (gsn_forge = skill_lookup("forge")) == -1 )
        return;

    if (IS_NPC(ch) && !IS_ACT_FLAG(ch, ACT_POLYMORPHED))
        return;

    if (ch->mount) {
        send_to_char("Not from this mount you cannot!\n\r",ch);
        return;
    }

    if (!CanUseSkill(ch, gsn_forge)) {
        send_to_char("What do you think you are, an artificer?\n\r",ch);
        return;
    }

    argument = one_argument(argument, itemname);
    argument = one_argument(argument, itemtype);

    if (!*itemname) {
        send_to_char("Forge what?\n\r",ch);
        return;
    }

    if (!*itemtype) {
        send_to_char("I see that, but what do you wanna make?\n\r",ch);
        return;
    }

    if (!(forge = get_obj_here(ch, "forge")))
    {
        send_to_char("You need a forge to complete the process.\n\r",ch);
        return;
    } else if (forge->item_type != ITEM_FORGE)
           {
            send_to_char("This forge is inoperable!\n\r",ch);
            return;
           }

    if (!(material = get_obj_carry(ch, itemname)))
    {
        send_to_char("Where did those materials go?\n\r",ch);
        return;
    }

    if (material->item_type != ITEM_MATERIAL)
    {
        send_to_char("These materials are improper for the task at hand.\n\r", ch);
        return;
    }

    if (MTYPE(material) == MAT_WORTHLESS || MGRADE(material) == QUAL_WORTHLESS)
    {
        send_to_char("These materials are just scraps, not enough for an attmpt.\n\r", ch);
        return;
    }

    percent = number_percent();

    if (percent > LEARNED(ch, gsn_forge))
    {
        if (material->value[1] > 0)
           material->value[1]--;
        sprintf(buf,"You pound at %s but accomplish nothing.\n\r",
                material->short_descr);
        send_to_char(buf,ch);

        sprintf(buf,"$n tries to forge %s into a %s, but fails.",
                material->short_descr, itemtype);
        act(AT_SKILL, buf, ch, NULL, NULL, TO_ROOM);
        learn_from_failure(ch, gsn_tan);
        WAIT_STATE(ch, PULSE_VIOLENCE*5);
        return;
    }

    quality = (int)((GetClassLevel(ch, CLASS_ARTIFICER) / 10) + MGRADE(material) - 2);
    quality = URANGE(0, quality, 10);


    switch(quality)
    {
    case 0:
        sprintf(sdesc," worthless");
        dplus = -1;
        break;
    case 1:
    case 2:
        sprintf(sdesc," poor");
        break;
    case 3:
    case 4:
    case 5:
        sprintf(sdesc,"n average");
        break;
    case 6:
    case 7:
        sprintf(sdesc," well crafted");
        break;
    case 8:
    case 9:
        sprintf(sdesc,"n excelently crafted");
        hplus = 1;
        break;
    case 10:
        sprintf(sdesc," masterfully crafted");
        hplus = 1;
        dplus = 1;
        break;

    }

    if (!str_cmp(itemtype,"dagger") && (ARTLEV(ch) >= 1))
    {
        newitem = create_object(45002);
        strcat(sdesc," dagger");
    }
    else if (!str_cmp(itemtype,"sword") && (ARTLEV(ch) >= 2))
    {
        newitem = create_object(45003);
        strcat(sdesc," sword");
    }
    else if (!str_cmp(itemtype,"longsword") && (ARTLEV(ch) >= 5))
    {
        newitem = create_object(45004);
        strcat(sdesc," long sword");
    }
    else if (!str_cmp(itemtype,"twohander") && (ARTLEV(ch) >= 10))
    {
        newitem = create_object(45005);
        strcat(sdesc," two handed sword");
    }
    else if (!str_cmp(itemtype,"broadsword") && (ARTLEV(ch) >= 15))
    {
        newitem = create_object(45006);
        strcat(sdesc," broad sword");
    }
    else if (!str_cmp(itemtype,"halberd") && (ARTLEV(ch) >= 20))
    {
        newitem = create_object(45007);
        strcat(sdesc," halberd");
    }
    else  if (!str_cmp(itemtype,"spear") && (ARTLEV(ch) >= 20))
    {
        newitem = create_object(45008);
        strcat(sdesc," spear");
    }
    else  if (!str_cmp(itemtype,"axe") && (ARTLEV(ch) >= 7))
    {
        newitem = create_object(45009);
        strcat(sdesc," axe");
    }
    else  if (!str_cmp(itemtype,"battleaxe") && (ARTLEV(ch) >= 18))
    {
        newitem = create_object(45010);
        strcat(sdesc," battle axe");
    }
    else  if (!str_cmp(itemtype,"warhammer") && (ARTLEV(ch) >= 30))
    {
        newitem = create_object(45011);
        strcat(sdesc," warhammer");
    }
    else  if (!str_cmp(itemtype,"hammer") && (ARTLEV(ch) >= 1))
    {
        newitem = create_object(45012);
        strcat(sdesc," smith's hammer");
    }
    else  {
        send_to_char("You have not learned how to make that yet!\n\r",ch);
        return;
    }
    if (!newitem)
    {
        bug("forge objects missing.");
        send_to_char("You messed up the hide and it's useless.\n\r", ch);
        return;
    }
    obj_to_char(newitem, ch);

    sprintf( buf, "a%s", sdesc );
    STRFREE(newitem->short_descr);
    newitem->short_descr = STRALLOC(buf);

    sprintf( buf, "A%s lies here.", sdesc );
    STRFREE(newitem->description);
    newitem->description = STRALLOC(buf);

    newitem->cost = (int)((material->cost / 100 * mat_costmul[MTYPE(material)]) + (quality * quality * 10));

    hplus += mat_hitplus[MTYPE(material)];
    dplus += mat_damplus[MTYPE(material)];

    if (hplus != 0)
    {
      CREATE(paf, AFFECT_DATA, 1);
      paf->type      = -1;
      paf->duration  = -1;
      paf->location  = APPLY_HITROLL;
      paf->modifier  = hplus;
      paf->bitvector = 0;
      paf->next      = NULL;
      LINK(paf, newitem->first_affect, newitem->last_affect, next, prev);
    }

    if (dplus != 0)
    {
      CREATE(paf2, AFFECT_DATA, 1);
      paf2->type      = -1;
      paf2->duration  = -1;
      paf2->location  = APPLY_DAMROLL;
      paf2->modifier  = dplus;
      paf2->bitvector = 0;
      paf2->next      = NULL;
      LINK(paf2, newitem->first_affect, newitem->last_affect, next, prev);
    }

    if (MTYPE(material) == MAT_SILVER)
    {
      CREATE(paf3, AFFECT_DATA, 1);
      paf3->type      = -1;
      paf3->duration  = -1;
      paf3->location  = APPLY_RACE_SLAYER;
      paf3->modifier  = RACE_LYCANTH;
      paf3->bitvector = 0;
      paf3->next      = NULL;
      LINK(paf2, newitem->first_affect, newitem->last_affect, next, prev);
    } else if (MTYPE(material) == MAT_LEAD)
    {
      SET_BIT(newitem->extra_flags, ITEM_POISONED);
    }

    sprintf(buf,"You pound at the %s and finally forge %s.\n\r",
            material->short_descr, newitem->short_descr);
    send_to_char(buf,ch);

    sprintf(buf,"$n manages to forge %s.",
            newitem->short_descr);
    act(AT_SKILL, buf, ch, NULL, NULL, TO_ROOM);

    MGRADE(material) = 0;
    sprintf(buf, "All that remains of %s is scraps.\n\r",
            material->short_descr);
    send_to_char(buf, ch);

    WAIT_STATE(ch, PULSE_VIOLENCE*5);
    return;
}

void do_probability_travel(CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *gch, *gch_next;
    ROOM_INDEX_DATA *astral;

    astral = get_room_index( ROOM_VNUM_ASTRAL_ENTRANCE );

    if ( IS_SET (ch->in_room->room_flags, ROOM_NO_ASTRAL) ||
         IS_SET (ch->in_room->area->flags, AFLAG_ARENA)   ||
         IS_SET (ch->in_room->area->flags, AFLAG_NOPKILL) ||
         IS_SET (ch->in_room->room_flags, ROOM_SAFE) )
    {
        send_to_char("You can't access the astral plane from here.\n", ch);
        return;
    }

    for ( gch = ch->in_room->first_person; gch; gch = gch = gch_next )
    {
        gch_next = gch->next_in_room;

        if ( IS_NPC(gch) && IS_SET (gch->act, ACT_PROTOTYPE) )
            continue;

        if ( is_same_group(ch, gch) && gch->position != POS_FIGHTING )
        {
            act( AT_MAGIC, "$n wavers, fades and dissappears.", gch, NULL, NULL, TO_ROOM );
            char_from_room(gch);
            char_to_room(gch, astral);
            act( AT_MAGIC, "$n wavers into existence.", gch, NULL, NULL, TO_ROOM );
            do_look( gch, "auto" );
        }
    }
}
