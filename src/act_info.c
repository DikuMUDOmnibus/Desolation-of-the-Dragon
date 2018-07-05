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
 *			     Informational module			    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: act_info.c,v 1.113 2004/04/06 22:00:08 dotd Exp $";*/


#include <sys/types.h>
#include <sys/utsname.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mud.h"
#include "gsn.h"
#ifdef MXP
#include "mxp.h"
#endif
#include "christen.h"
#include "quest.h"

DECLARE_DO_FUN(do_lookmap);
DECLARE_DO_FUN(do_exits);
DECLARE_DO_FUN(do_track);
DECLARE_DO_FUN(do_noteroom);
DECLARE_DO_FUN(do_examine);
DECLARE_DO_FUN(do_whozone);
DECLARE_DO_FUN(do_mwhere);
DECLARE_DO_FUN(do_owhere);
DECLARE_DO_FUN(do_remove);
DECLARE_DO_FUN(do_foldarea);
DECLARE_DO_FUN(do_comment);


extern int	top_room;
extern int	top_mob_index;
extern int	top_obj_index;
extern char     str_boot_time[];
extern char     reboot_time[];

extern char *	help_greeting;

void save_sysdata     args( ( SYSTEM_DATA sys ) );

char *river_room_desc args( (ROOM_INDEX_DATA *rp) );

char *	const	where_name	[] =
{
    "<used as light>     ",
    "<worn on finger>    ",
    "<worn on finger>    ",
    "<worn around neck>  ",
    "<worn around neck>  ",
    "<worn on body>      ",
    "<worn on head>      ",
    "<worn on legs>      ",
    "<worn on feet>      ",
    "<worn on hands>     ",
    "<worn on arms>      ",
    "<worn as shield>    ",
    "<worn about body>   ",
    "<worn about waist>  ",
    "<worn around wrist> ",
    "<worn around wrist> ",
    "<wielded>           ",
    "<held>              ",
    "<dual wielded>      ",
    "<worn on ear>       ",
    "<worn on ear>       ",
    "<worn on eyes>      ",
    "<missile wielded>   ",
    "<worn on back>      ",
    "<worn in nose>      ",
    "<worn on ankle>     ",
    "<worn on ankle>     "
};

sh_int  const   dir_color      [LAST_NORMAL_DIR+1]              =
{
    AT_DIR_NORTH, AT_DIR_EAST, AT_DIR_SOUTH, AT_DIR_WEST, AT_DIR_UP, AT_DIR_DOWN,
    AT_DIR_NORTHEAST, AT_DIR_NORTHWEST, AT_DIR_SOUTHEAST, AT_DIR_SOUTHWEST,
    AT_DGREY
};

/*
 * Local functions.
 */
void	show_char_to_char_0	args( ( CHAR_DATA *victim, CHAR_DATA *ch, int num ) );
void	show_char_to_char_1	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char	args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool	check_blind		args( ( CHAR_DATA *ch ) );
void    show_condition          args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );


char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort )
{
    static char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';
    if ( IS_OBJ_STAT(obj, ITEM_INVIS)     )   strcat( buf, "(Invis) "     );
    if ( IS_AFFECTED(ch, AFF_DETECT_EVIL)
         && IS_OBJ_STAT(obj, ITEM_EVIL)   )   strcat( buf, "(Red Aura) "  );
    if ( IS_AFFECTED(ch, AFF_DETECT_MAGIC)
         && IS_OBJ_STAT(obj, ITEM_MAGIC)  )   strcat( buf, "(Magical) "   );
    if ( IS_OBJ_STAT(obj, ITEM_GLOW)      )   strcat( buf, "(Glowing) "   );
    if ( IS_OBJ_STAT(obj, ITEM_HUM)       )   strcat( buf, "(Humming) "   );
    if ( IS_OBJ_STAT(obj, ITEM_HIDDEN)	  )   strcat( buf, "(Hidden) "	  );
    if ( IS_OBJ_STAT(obj, ITEM_BURRIED)	  )   strcat( buf, "(Burried) "	  );
    if ( IS_IMMORTAL(ch)
         && IS_OBJ_STAT(obj, ITEM_PROTOTYPE) ) strcat( buf, "(PROTO) "	  );
    if ( IS_AFFECTED(ch, AFF_DETECTTRAPS)
         && is_trapped(obj)   )   strcat( buf, "(Trap) "  );

#ifdef MXP
    strcat(buf, mxp_obj_str(ch, obj));
#endif

    if ( fShort )
    {
        if ( obj->christened )
            strcat( buf, get_christen_name(obj) );
        else if ( obj->short_descr )
            strcat( buf, obj->short_descr );
    }
    else
    {
        if ( obj->christened )\
        {
            strcat( buf, "(");
            strcat( buf, get_christen_name(obj) );
            strcat( buf, ") ");
        }
        if ( obj->description )
            strcat( buf, obj->description );
    }

#ifdef MXP
    strcat(buf, mxp_obj_str_close(ch));
#endif

    return buf;
}


/*
 * Some increasingly freaky halucinated objects		-Thoric
 */
char *halucinated_object( int ms, bool fShort )
{
    int sms = URANGE( 1, (ms+10)/5, 20 );

    if ( fShort )
        switch( number_range( 6-URANGE(1,sms/2,5), sms ) )
        {
        case  1: return "a sword";
        case  2: return "a stick";
        case  3: return "something shiny";
        case  4: return "something";
        case  5: return "something interesting";
        case  6: return "something colorful";
        case  7: return "something that looks cool";
        case  8: return "a nifty thing";
        case  9: return "a cloak of flowing colors";
        case 10: return "a mystical flaming sword";
        case 11: return "a swarm of insects";
        case 12: return "a deathbane";
        case 13: return "a figment of your imagination";
        case 14: return "your gravestone";
        case 15: return "the long lost boots of Ranger Thoric";
        case 16: return "a glowing tome of arcane knowledge";
        case 17: return "a long sought secret";
        case 18: return "the meaning of it all";
        case 19: return "the answer";
        case 20: return "the key to life, the universe and everything";
        }
    switch( number_range( 6-URANGE(1,sms/2,5), sms ) )
    {
    case  1: return "A nice looking sword catches your eye.";
    case  2: return "The ground is covered in small sticks.";
    case  3: return "Something shiny catches your eye.";
    case  4: return "Something catches your attention.";
    case  5: return "Something interesting catches your eye.";
    case  6: return "Something colorful flows by.";
    case  7: return "Something that looks cool calls out to you.";
    case  8: return "A nifty thing of great importance stands here.";
    case  9: return "A cloak of flowing colors asks you to wear it.";
    case 10: return "A mystical flaming sword awaits your grasp.";
    case 11: return "A swarm of insects buzzes in your face!";
    case 12: return "The extremely rare Deathbane lies at your feet.";
    case 13: return "A figment of your imagination is at your command.";
    case 14: return "You notice a gravestone here... upon closer examination, it reads your name.";
    case 15: return "The long lost boots of Ranger Thoric lie off to the side.";
    case 16: return "A glowing tome of arcane knowledge hovers in the air before you.";
    case 17: return "A long sought secret of all mankind is now clear to you.";
    case 18: return "The meaning of it all, so simple, so clear... of course!";
    case 19: return "The answer.  One.  It's always been One.";
    case 20: return "The key to life, the universe and everything awaits your hand.";
    }
    return "Whoa!!!";
}


/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing )
{
    char **prgpstrShow;
    int *prgnShow;
    int *pitShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count, offcount, tmp, ms, cnt;
    bool fCombine;

    if ( !ch->desc )
        return;

    /*
     * if there's no list... then don't do all this crap!  -Thoric
     */
    if ( !list )
    {
        if ( fShowNothing )
        {
            if ( IS_NPC(ch) || IS_PLR_FLAG(ch, PLR_COMBINE) )
                send_to_char( "     ", ch );
            send_to_char( "Nothing.\n\r", ch );
        }
        return;
    }
    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( obj = list; obj; obj = obj->next_content )
        count++;

    ms  = (ch->mental_state ? ch->mental_state : 1)
        * (IS_NPC(ch) ? 1 : (GET_COND(ch,COND_DRUNK) ? (GET_COND(ch,COND_DRUNK)/12) : 1));

    /*
     * If not mentally stable...
     */
    if ( abs(ms) > 40 )
    {
        offcount = URANGE( -(count), (count * ms) / 100, count*2 );
        if ( offcount < 0 )
            offcount += number_range(0, abs(offcount));
        else
            if ( offcount > 0 )
                offcount -= number_range(0, offcount);
    }
    else
        offcount = 0;

    if ( count + offcount <= 0 )
    {
        if ( fShowNothing )
        {
            if ( IS_NPC(ch) || IS_PLR_FLAG(ch, PLR_COMBINE) )
                send_to_char( "     ", ch );
            send_to_char( "Nothing.\n\r", ch );
        }
        return;
    }

    CREATE( prgpstrShow,	char*,	count + ((offcount > 0) ? offcount : 0) );
    CREATE( prgnShow,		int,	count + ((offcount > 0) ? offcount : 0) );
    CREATE( pitShow,		int,	count + ((offcount > 0) ? offcount : 0) );
    nShow	= 0;
    tmp		= (offcount > 0) ? offcount : 0;
    cnt		= 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj; obj = obj->next_content )
    {
        quest_trigger_objfind(ch, obj);

        if ( offcount < 0 && ++cnt > (count + offcount) )
            break;
        if ( tmp > 0 && number_bits(1) == 0 )
        {
            prgpstrShow [nShow] = str_dup( halucinated_object(ms, fShort) );
            prgnShow	[nShow] = 1;
            pitShow	[nShow] = number_range( ITEM_LIGHT, ITEM_BOOK );
            nShow++;
            --tmp;
        }
        if ( obj->wear_loc == WEAR_NONE
             && can_see_obj( ch, obj )
             && (obj->item_type != ITEM_TRAP || IS_AFFECTED(ch, AFF_DETECTTRAPS) ) )
        {
            pstrShow = format_obj_to_char( obj, ch, fShort );
            fCombine = FALSE;

            if ( IS_NPC(ch) || IS_PLR_FLAG(ch, PLR_COMBINE) )
            {
                /*
                 * Look for duplicates, case sensitive.
                 * Matches tend to be near end so run loop backwords.
                 */
                for ( iShow = nShow - 1; iShow >= 0; iShow-- )
                {
                    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
                    {
                        prgnShow[iShow] += obj->count;
                        fCombine = TRUE;
                        break;
                    }
                }
            }

            pitShow[nShow] = obj->item_type;
            /*
             * Couldn't combine, or didn't want to.
             */
            if ( !fCombine )
            {
                prgpstrShow [nShow] = str_dup( pstrShow );
                prgnShow    [nShow] = obj->count;
                nShow++;
            }
        }
    }
    if ( tmp > 0 )
    {
        int x;
        for ( x = 0; x < tmp; x++ )
        {
            prgpstrShow [nShow] = str_dup( halucinated_object(ms, fShort) );
            prgnShow	[nShow] = 1;
            pitShow	[nShow] = number_range( ITEM_LIGHT, ITEM_BOOK );
            nShow++;
        }
    }

    /*
     * Output the formatted list.		-Color support by Thoric
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
        switch(pitShow[iShow]) {
        default:
            set_char_color( AT_OBJECT, ch );
            break;
        case ITEM_BLOOD:
            set_char_color( AT_BLOOD, ch );
            break;
        case ITEM_MONEY:
        case ITEM_TREASURE:
            set_char_color( AT_GOLD, ch );
            break;
        case ITEM_FOOD:
            set_char_color( AT_HUNGRY, ch );
            break;
        case ITEM_DRINK_CON:
        case ITEM_FOUNTAIN:
            set_char_color( AT_THIRSTY, ch );
            break;
        case ITEM_FIRE:
            set_char_color( AT_FIRE, ch );
            break;
        case ITEM_SCROLL:
        case ITEM_WAND:
        case ITEM_STAFF:
            set_char_color( AT_MAGIC, ch );
            break;
        }
        if ( fShowNothing )
            send_to_char( "     ", ch );
        send_to_char( prgpstrShow[iShow], ch );
        /*	if ( IS_NPC(ch) || IS_PLR_FLAG(ch, PLR_COMBINE) ) */
        {
            if ( prgnShow[iShow] != 1 )
                ch_printf( ch, " (%d)", prgnShow[iShow] );
        }

        send_to_char( "\n\r", ch );
        DISPOSE( prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
        if ( IS_NPC(ch) || IS_PLR_FLAG(ch, PLR_COMBINE) )
            send_to_char( "     ", ch );
        send_to_char( "Nothing.\n\r", ch );
    }

    /*
     * Clean up.
     */
    DISPOSE( prgpstrShow );
    DISPOSE( prgnShow	 );
    DISPOSE( pitShow	 );
    return;
}


/*
 * Show fancy descriptions for certain spell affects		-Thoric
 */
void show_visible_affects_to_char( CHAR_DATA *victim, CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
    {
        if ( IS_GOOD(victim) )
        {
            set_char_color( AT_WHITE, ch );
            ch_printf( ch, "%s glows with an aura of divine radiance.\n\r",
                       IS_NPC( victim ) ? capitalize(victim->short_descr) :
                       PERS(victim, ch) );
        }
        else if ( IS_EVIL(victim) )
        {
            set_char_color( AT_WHITE, ch );
            ch_printf( ch, "%s shimmers beneath an aura of dark energy.\n\r",
                       IS_NPC( victim ) ? capitalize(victim->short_descr) :
                       PERS(victim, ch) );
        }
        else
        {
            set_char_color( AT_WHITE, ch );
            ch_printf( ch, "%s is shrouded in flowing shadow and light.\n\r",
                       IS_NPC( victim ) ? capitalize(victim->short_descr) :
                       PERS(victim, ch) );
        }
    }
    if ( IS_AFFECTED(victim, AFF_FIRESHIELD) )
    {
        set_char_color( AT_FIRE, ch );
        ch_printf( ch, "%s is engulfed within a blaze of mystical flame.\n\r",
                   IS_NPC( victim ) ? capitalize(victim->short_descr) :
                   PERS(victim, ch) );
    }
    if ( IS_AFFECTED(victim, AFF_SHOCKSHIELD) )
    {
        set_char_color( AT_BLUE, ch );
        ch_printf( ch, "%s is surrounded by cascading torrents of energy.\n\r",
                   IS_NPC( victim ) ? capitalize(victim->short_descr) :
                   PERS(victim, ch) );
    }
    /*Scryn 8/13*/
    if ( IS_AFFECTED(victim, AFF_ICESHIELD) )
    {
        set_char_color( AT_LBLUE, ch );
        ch_printf( ch, "%s is ensphered by shards of glistening ice.\n\r",
                   IS_NPC( victim ) ? capitalize(victim->short_descr) :
                   PERS(victim, ch) );
    }
    if ( IS_AFFECTED(victim, AFF_CHARM)       )
    {
        set_char_color( AT_MAGIC, ch );
        ch_printf( ch, "%s wanders in a dazed, zombie-like state.\n\r",
                   IS_NPC( victim ) ? capitalize(victim->short_descr) :
                   PERS(victim, ch) );
    }
    if ( !IS_NPC(victim) && !victim->desc
         &&    victim->switched && IS_AFFECTED(victim->switched, AFF_POSSESS) )
    {
        set_char_color( AT_MAGIC, ch );
        strcpy( buf, PERS( victim, ch ) );
        strcat( buf, " appears to be in a deep trance...\n\r" );
        send_to_char(buf, ch);
    }
}

void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch, int num )
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if (!IS_NPC(victim))
    {
        if ( !victim->desc )
        {
            if ( !victim->switched )
                strcat( buf, "(Link Dead) "  );
            else if ( !IS_AFFECTED(victim->switched, AFF_POSSESS) )
                strcat( buf, "(Switched) " );
        }
        if ( IS_PLR_FLAG(victim, PLR_AFK) )
            strcat( buf, "[AFK] ");
        if ( victim->pcdata->wizinvis )
        {
            sprintf( buf1,"(Invis %d) ", victim->pcdata->wizinvis );
            strcat(buf, buf1);
        }
    } else {
        if ( IS_ACT_FLAG(victim, ACT_MOBINVIS) )
        {
            sprintf( buf1,"(Mobinvis %d) ", victim->mobinvis);
            strcat(buf, buf1);
        }
    }

    if ( IS_AFFECTED(victim, AFF_INVISIBLE)   ) strcat( buf, "(Invis) "      );
    if ( IS_AFFECTED(victim, AFF_HIDE)        ) strcat( buf, "(Hide) "       );
    if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   ) strcat( buf, "(Translucent) ");
    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) strcat( buf, "(Pink Aura) "  );
    if ( IS_EVIL(victim)
         &&   IS_AFFECTED(ch, AFF_DETECT_EVIL)     ) strcat( buf, "(Red Aura) "   );
    if (!IS_NPC(victim))
    {
        if ( IS_PLR_FLAG(victim, PLR_ATTACKER ) )
            strcat( buf, "(ATTACKER) "     );
        if ( IS_PLR_FLAG(victim, PLR_KILLER ) )
            strcat( buf, "(KILLER) "     );
        if ( IS_PLR_FLAG(victim, PLR_THIEF  ) )
            strcat( buf, "(THIEF) "      );
        if ( IS_PLR_FLAG(victim, PLR_LITTERBUG  ) )
            strcat( buf, "(LITTERBUG) "  );
    } else {
        if ( IS_IMMORTAL(ch) && IS_ACT_FLAG(victim, ACT_PROTOTYPE) )
            strcat( buf, "(PROTO) " );
    }
    if ( victim->desc && victim->desc->connected == CON_EDITING )
        strcat( buf, "(Writing) " );

    set_char_color( AT_PERSON, ch );
    if ( victim->position == victim->defposition && victim->long_descr[0] != '\0' )
    {
        strcat( buf, strip_crlf( victim->long_descr ) );
        if ( num > 1 && IS_NPC( victim ) )
        {
            sprintf( buf1, " (%d)", num );
            strcat( buf, buf1 );
        }
        strcat( buf, "\n\r" );
        send_to_char( buf, ch );
        show_visible_affects_to_char( victim, ch );
        return;
    }

    strcat( buf, PERS( victim, ch ) );
    /*
     if ( !IS_NPC(victim) && !IS_PLR_FLAG(ch, PLR_BRIEF) )
     strcat( buf, victim->pcdata->title );
     */

    switch ( victim->position )
    {
    case POS_DEAD:
        strcat( buf, color_str(AT_DIEMSG, ch) );
        strcat( buf, " is DEAD!!" );
        break;
    case POS_MORTAL:   strcat( buf, " is mortally wounded." );		break;
    case POS_INCAP:    strcat( buf, " is incapacitated." );		break;
    case POS_STUNNED:  strcat( buf, " is lying here stunned." );	break;
    case POS_MEDITATING: IS_AFFECTED(victim, AFF_FLOATING) ?
                         strcat( buf, " is floating here meditating.") :
                         strcat( buf, " is meditating here.");          break;
    case POS_SLEEPING:
        if (ch->position == POS_SITTING
            ||  ch->position == POS_RESTING )
            strcat( buf, " is sleeping nearby." );
        else
            strcat( buf, " is deep in slumber here." );
        break;
    case POS_RESTING:
        if (ch->position == POS_RESTING)
            strcat ( buf, " is sprawled out alongside you." );
        else
            if (ch->position == POS_MOUNTED)
                strcat ( buf, " is sprawled out at the foot of your mount." );
            else
                strcat (buf, " is sprawled out here." );
        break;
    case POS_SITTING:
        if (ch->position == POS_SITTING)
            strcat( buf, " sits here with you." );
        else
            if (ch->position == POS_RESTING)
                strcat( buf, " sits nearby as you lie around." );
            else
            if (victim->fighting)
                strcat( buf, " is here scrambling to get up." );
                else
                  strcat( buf, " sits upright here." );
        break;
    case POS_STANDING:
        if ( IS_IMMORTAL(victim) )
            strcat( buf, " is here before you." );
        else
            if ( ( victim->in_room->sector_type == SECT_UNDERWATER )
                 && !IS_AFFECTED(victim, AFF_AQUA_BREATH) && !IS_NPC(victim) )
                strcat( buf, " is drowning here." );
            else
                if ( victim->in_room->sector_type == SECT_UNDERWATER )
                    strcat( buf, " is here in the water." );
                else
                    if ( ( victim->in_room->sector_type == SECT_OCEANFLOOR )
                         && !IS_AFFECTED(victim, AFF_AQUA_BREATH) && !IS_NPC(victim) )
                        strcat( buf, " is drowning here." );
                    else
                        if ( victim->in_room->sector_type == SECT_OCEANFLOOR )
                            strcat( buf, " is standing here in the water." );
                        else
                            if ( IS_AFFECTED(victim, AFF_FLOATING)
                                 || IS_AFFECTED(victim, AFF_FLYING) )
                                strcat( buf, " is hovering here." );
                            else
                                strcat( buf, " is standing here." );
        break;
    case POS_SHOVE:    strcat( buf, " is being shoved around." );	break;
    case POS_DRAG:     strcat( buf, " is being dragged around." );	break;
    case POS_MOUNTED:
        strcat( buf, " is here, upon " );
        if ( !victim->mount )
        {
            strcat( buf, "thin air???" );
            victim->position = POS_SITTING;
        }
        else
            if ( victim->mount == ch )
                strcat( buf, "your back." );
            else
                if ( victim->in_room == victim->mount->in_room )
                {
                    strcat( buf, PERS( victim->mount, ch ) );
                    strcat( buf, "." );
                }
                else
                    strcat( buf, "someone who left??" );
        break;
    case POS_FIGHTING:
        strcat( buf, " is here, fighting " );
        if ( !victim->fighting )
        {
            strcat( buf, "thin air???" );
            victim->position = POS_SITTING;
        }
        else if ( who_fighting( victim ) == ch )
            strcat( buf, "YOU!" );
        else if ( victim->in_room == victim->fighting->who->in_room )
        {
            strcat( buf, PERS( victim->fighting->who, ch ) );
            strcat( buf, "." );
        }
        else
            strcat( buf, "someone who left??" );
        break;
    }

    if ( num > 1 && IS_NPC( victim ) )
    {
        sprintf( buf1, " (%d)", num );
        strcat( buf, buf1 );
    }

    strcat( buf, "\n\r" );
    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );
    show_visible_affects_to_char( victim, ch );

    return;
}



void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    int iWear;
    bool found;

    if ( can_see( victim, ch ) )
    {
        act( AT_ACTION, "$n looks at you.", ch, NULL, victim, TO_VICT    );
        act( AT_ACTION, "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
    }

    if ( victim->description[0] != '\0' )
    {
        send_to_char( victim->description, ch );
    }
    else
    {
        act( AT_PLAIN, "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
    }

    show_condition( ch, victim );

    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
        if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
             &&   can_see_obj( ch, obj ) )
        {
            if ( !found )
            {
                send_to_char( "\n\r", ch );
                act( AT_PLAIN, "$N is using:", ch, NULL, victim, TO_CHAR );
                found = TRUE;
            }
            send_to_char( where_name[iWear], ch );
#ifdef MXP
            mxpprecommand[0]=mxpposcommand[0]='\0';
#endif
            send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
            send_to_char( "\n\r", ch );
        }
    }

    if (IS_NPC(victim))
        quest_trigger_mobfind(ch, victim);

    /*
     * Crash fix here by Thoric
     */
    if ( IS_NPC(ch) || victim == ch )
        return;

    if ( IS_ACTIVE(ch, CLASS_THIEF) && number_range(0, LEVEL_IMMORTAL-1) < GetMaxLevel(ch) )
    {
        send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
#ifdef MXP
        strcpy(mxpprecommand, "steal");
        sprintf(mxpposcommand, "&#39;%s&#39;", victim->name);
#endif
        show_list_to_char( victim->first_carrying, ch, TRUE, TRUE );
    }

    return;
}

bool is_same_mob( CHAR_DATA *i, CHAR_DATA *j )
{
    if (!IS_NPC(i) || !IS_NPC(j) ||
        i->fighting || j->fighting ||
        i->pIndexData   != j->pIndexData ||
        GET_POS(i)      != GET_POS(j) ||
        i->affected_by  != j->affected_by ||
        i->affected_by2 != j->affected_by2 ||
        i->act          != j->act ||
        i->act2         != j->act2 ||
        i->carry_weight != j->carry_weight ||
        i->carry_number != j->carry_number ||
        i->resistant    != j->resistant ||
        i->susceptible  != j->susceptible ||
        i->absorb       != j->absorb ||
        i->immune       != j->immune ||
        i->spec_fun     != j->spec_fun ||
        str_cmp(GET_NAME(i), GET_NAME(j)) ||
        str_cmp(i->short_descr, j->short_descr) ||
        str_cmp(i->long_descr, j->long_descr) ||
        str_cmp(i->description, j->description))
        return FALSE;

    return TRUE;
}

void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch )
{
    CHAR_DATA *rch, *j;
    sh_int num=0;

    if ( !list )
        return;

    for ( rch = list; rch; rch = rch->next_in_room )
    {
        if ( rch == ch || char_died(rch) || rch == supermob )
            continue;

        num = 0;
        for (j = list; j != rch; j = j->next_in_room)
            if ( is_same_mob(j,rch) )
                break;
        if ( j != rch )
            continue;
        for (j = rch; j; j = j->next_in_room)
            if ( is_same_mob(j,rch) )
                num++;

        if ( can_see( ch, rch ) )
        {
            show_char_to_char_0( rch, ch, num );
            if (IS_NPC(rch))
                quest_trigger_mobfind(ch, rch);
        }
        else if ( room_is_dark( ch->in_room )
                  &&        IS_AFFECTED(rch, AFF_INFRARED ) )
        {
            set_char_color( AT_BLOOD, ch );
            send_to_char( "The red form of a living creature is here.\n\r", ch );
        }
    }

    return;
}



bool check_blind( CHAR_DATA *ch )
{
    if ( !IS_NPC(ch) && IS_PLR_FLAG(ch, PLR_HOLYLIGHT) )
        return TRUE;

    if ( IS_AFFECTED(ch, AFF_TRUESIGHT) )
        return TRUE;

    if ( IS_AFFECTED(ch, AFF_BLIND) )
    {
        send_to_char( "You can't see a thing!\n\r", ch );
        return FALSE;
    }

    return TRUE;
}

/*
 * Returns classical DIKU door direction based on text in arg	-Thoric
 */
int get_door( char *arg )
{
    int door;

    if ( !str_cmp( arg, "n"  ) || !str_cmp( arg, "north"	  ) ) door = 0;
    else if ( !str_cmp( arg, "e"  ) || !str_cmp( arg, "east"	  ) ) door = 1;
    else if ( !str_cmp( arg, "s"  ) || !str_cmp( arg, "south"	  ) ) door = 2;
    else if ( !str_cmp( arg, "w"  ) || !str_cmp( arg, "west"	  ) ) door = 3;
    else if ( !str_cmp( arg, "u"  ) || !str_cmp( arg, "up"	  ) ) door = 4;
    else if ( !str_cmp( arg, "d"  ) || !str_cmp( arg, "down"	  ) ) door = 5;
    else if ( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) ) door = 6;
    else if ( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest" ) ) door = 7;
    else if ( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast" ) ) door = 8;
    else if ( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) ) door = 9;
    else door = -1;
    return door;
}



void do_look( CHAR_DATA *ch, char *argument )
{
    char arg  [MAX_INPUT_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *original;
    char *pdesc;
    bool doexaprog;
    sh_int door;
    int number, cnt;

    if ( !ch->desc && !IS_WEB(ch) )
        return;

    if ( ch->position < POS_SLEEPING )
    {
        send_to_char( "You can't see anything but stars!\n\r", ch );
        return;
    }

    if ( ch->position == POS_SLEEPING )
    {
        send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
        return;
    }

    if ( !check_blind( ch ) )
        return;

    if ( !IS_NPC(ch)
         &&   !IS_PLR_FLAG(ch, PLR_HOLYLIGHT)
         &&   !IS_AFFECTED(ch, AFF_TRUESIGHT)
         &&   room_is_dark( ch->in_room ) )
    {
        set_char_color( AT_DGREY, ch );
        send_to_char( "It is pitch black ... \n\r", ch );
        show_char_to_char( ch->in_room->first_person, ch );
        return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    doexaprog = str_cmp( "noprog", arg2 ) && str_cmp( "noprog", arg3 );

    if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) )
    {
#ifdef IBUILD
        switch ( ch->inter_page )    /* rmenu */
        {
        case ROOM_PAGE_A : do_rmenu(ch,"a");
        break;
        case ROOM_PAGE_B : do_rmenu(ch,"b");
        break;
        case ROOM_PAGE_C : do_rmenu(ch,"c");
        break;
        }
#endif
        /* 'look' or 'look auto' */

#ifdef MXP
        if (MXP_ON(ch))
            send_to_char(MXP_TAG_ROOMNAME, ch);
#endif
        set_char_color(AT_RMNAME, ch);
        send_to_char( ch->in_room->name, ch );
#ifdef MXP
        if (MXP_ON(ch))
            send_to_char(MXP_TAG_ROOMNAME_CLOSE, ch);
#endif

        if (IS_IMMORTAL(ch))
        {
            if (IS_ROOM_FLAG(ch->in_room, ROOM_ORPHANED))
            {
                set_char_color(AT_YELLOW, ch);
                send_to_char(" (Orphaned)", ch);
            }
            if (IS_ROOM_FLAG(ch->in_room, ROOM_PROTOTYPE))
            {
                set_char_color(AT_GREEN, ch);
                send_to_char(" (Prototype)", ch);
            }
        }

        send_to_char( "\n\r", ch );
        set_char_color(AT_RMDESC, ch);

        if ( arg1[0] == '\0'
             || ( !IS_NPC(ch) && !IS_PLR_FLAG(ch, PLR_BRIEF) ) )
        {
#ifdef MXP
            if (MXP_ON(ch))
                send_to_char(MXP_TAG_ROOMDESC, ch);
#endif
            miml_to_char( ch->in_room->description, ch );
#ifdef MXP
            if (MXP_ON(ch))
                send_to_char(MXP_TAG_ROOMDESC_CLOSE, ch);
#endif
        }

        set_char_color(AT_LBLUE, ch);
        send_to_char( river_room_desc(ch->in_room), ch);

        if ( !IS_NPC(ch) && IS_PLR_FLAG(ch, PLR_AUTOMAP) )   /* maps */
        {
            if(ch->in_room->map != NULL)
            {
                do_lookmap(ch, NULL);
            }
        }

        if (IS_IMMORTAL(ch) && ch->in_room->first_extradesc)
        {
            EXTRA_DESCR_DATA *ed;
            set_char_color(AT_LBLUE, ch);
            send_to_char("Extra descs: ", ch);
            for ( ed = ch->in_room->first_extradesc; ed; ed = ed->next )
                ch_printf(ch, "[%s] ", ed->keyword);
            send_to_char("\n\r", ch);

        }

        rprog_look_trigger( "", ch );

        if ( !IS_NPC(ch) &&
             IS_PLR_FLAG(ch, PLR_AUTOEXIT) &&
             !str_cmp(arg1, "auto") )
        {
            do_exits( ch, "auto" );
            do_exits( ch, "" );
        }
        else
            do_exits( ch, "auto" );

        if ( !IS_NPC(ch) && ch->hunting )
        {
            do_track(ch, "");
        }

#ifdef MXP
        strcpy(mxpprecommand, "get|examine|look in");
        mxpposcommand[0]='\0';
#endif
        if ( IS_DONATION(ch->in_room) )
            show_list_to_char( DONATION_ROOM()->first_content, ch, FALSE, FALSE );
        else
            show_list_to_char( ch->in_room->first_content, ch, FALSE, FALSE );
        show_char_to_char( ch->in_room->first_person,  ch );
        return;
    }

    if ( !str_cmp( arg1, "board") )
        do_noteroom(ch, "list");

    if ( !str_cmp( arg1, "under" ) )
    {
        int count;

        /* 'look under' */
        if ( arg2[0] == '\0' )
        {
            send_to_char( "Look beneath what?\n\r", ch );
            return;
        }

        if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
        {
            send_to_char( "You do not see that here.\n\r", ch );
            return;
        }

        if ( carry_w(ch) + obj->weight > can_carry_w( ch ) )
        {
            send_to_char( "It's too heavy for you to look under.\n\r", ch );
            return;
        }

#ifdef VTRACK
        vtrack_add_obj(ch, obj->vnum);
#endif

        count = obj->count;
        obj->count = 1;
        act( AT_PLAIN, "You lift $p and look beneath it:", ch, obj, NULL, TO_CHAR );
        act( AT_PLAIN, "$n lifts $p and looks beneath it:", ch, obj, NULL, TO_ROOM );
        obj->count = count;
#ifdef MXP
        strcpy(mxpprecommand, "get");
        sprintf(mxpposcommand, "%s", spacetodash(obj->name));
#endif
        if ( IS_OBJ_STAT( obj, ITEM_COVERING ) )
            show_list_to_char( obj->first_content, ch, TRUE, TRUE );
        else
            send_to_char( "Nothing.\n\r", ch );
        if ( doexaprog ) oprog_examine_trigger( ch, obj );
        return;
    }

    if ( !str_cmp( arg1, "a" ) || !str_cmp( arg1, "at" ) )
    {
        do_examine(ch, arg2);
        return;
    }

    if ( !str_cmp( arg1, "i" ) || !str_cmp( arg1, "in" ) )
    {
        int count;

        /* 'look in' */
        if ( arg2[0] == '\0' )
        {
            send_to_char( "Look in what?\n\r", ch );
            return;
        }

        if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
        {
            if ( !IS_IMMORTAL(ch) )
            {
                send_to_char( "You do not see that here.\n\r", ch );
                return;
            }
            if ( ( obj = get_obj_world( ch, arg2 ) ) == NULL )
            {
                send_to_char( "You can't find that anywhere.\n\r", ch );
                return;
            }
            send_to_char( "You can't see it here, but you look into it anyway.\n\r", ch );
        }

#ifdef VTRACK
        vtrack_add_obj(ch, obj->vnum);
#endif

        switch ( obj->item_type )
        {
        default:
            send_to_char( "That is not a container.\n\r", ch );
            break;

        case ITEM_DRINK_CON:
            if ( obj->value[1] <= 0 )
            {
                send_to_char( "It is empty.\n\r", ch );
                if ( doexaprog ) oprog_examine_trigger( ch, obj );
                break;
            }

            ch_printf( ch, "It's %s full of a %s liquid.\n\r",
                       obj->value[1] <     obj->value[0] / 4
                       ? "less than a quarter" :
                       obj->value[1] <     obj->value[0] / 2
                       ? "less than half" :
                       obj->value[1] ==    obj->value[0] / 2
                       ? "half" :
                       obj->value[1] < 3 * obj->value[0] / 4
                       ? "almost"     : "completely",
                       liq_table[obj->value[2]].liq_color
                     );

            if ( doexaprog ) oprog_examine_trigger( ch, obj );
            break;

        case ITEM_PORTAL:
            for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
            {
                if ( pexit->vdir == DIR_PORTAL
                     &&   IS_EXIT_FLAG(pexit, EX_PORTAL) )
                {
                    if ( room_is_private( pexit->to_room )
                         &&   get_trust(ch) < sysdata.level_override_private )
                    {
                        set_char_color( AT_WHITE, ch );
                        send_to_char( "That room is private buster!\n\r", ch );
                        return;
                    }
                    original = ch->in_room;
                    char_from_room( ch );
                    char_to_room( ch, pexit->to_room );
                    do_look( ch, "auto" );
                    char_from_room( ch );
                    char_to_room( ch, original );
                    return;
                }
            }
            send_to_char( "You see a swirling chaos...\n\r", ch );
            break;
        case ITEM_CONTAINER:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
            if ( IS_SET(obj->value[1], CONT_CLOSED) )
            {
                send_to_char( "It is closed.\n\r", ch );
                break;
            }

            count = obj->count;
            obj->count = 1;
            act( AT_PLAIN, "$p contains:", ch, obj, NULL, TO_CHAR );
            obj->count = count;
#ifdef MXP
            strcpy(mxpprecommand, "get");
            sprintf(mxpposcommand, "%s", spacetodash(obj->name));
#endif
            show_list_to_char( obj->first_content, ch, TRUE, TRUE );
            if ( doexaprog ) oprog_examine_trigger( ch, obj );
            break;
        }

        return;
    }

    rprog_look_trigger( arg1, ch );

    if ( (pdesc=get_extra_descr(arg1, ch->in_room->first_extradesc)) != NULL )
    {
        send_to_char( pdesc, ch );
        return;
    }

    door = get_door( arg1 );
    if ( ( pexit = find_door( ch, arg1, TRUE ) ) != NULL )
    {
        if ( pexit->keyword
             &&   pexit->keyword[0] != '\0'
             &&   pexit->keyword[0] != ' ' )
        {
            if ( IS_EXIT_FLAG(pexit, EX_CLOSED)
                 &&  !IS_EXIT_FLAG(pexit, EX_WINDOW) )
            {
                if ( IS_EXIT_FLAG(pexit, EX_SECRET)
                     &&   door != -1 )
                    send_to_char( "You see nothing special.\n\r", ch );
                else
                    act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
                return;
            }
            if ( IS_EXIT_FLAG( pexit, EX_BASHED ) )
                act(AT_RED, "The $d has been bashed from its hinges!",ch, NULL, pexit->keyword, TO_CHAR);
        }

        if ( pexit->description && pexit->description[0] != '\0' )
        {
            send_to_char( pexit->description, ch );
            send_to_char( "\n\r", ch);
        }
        else
            send_to_char( "You see nothing special.\n\r", ch );

        /*
         * Ability to look into the next room			-Thoric
         */
        if ( pexit->to_room
             && ( is_affected(ch, gsn_spy) ||
                  is_affected(ch, gsn_wizardeye) ||
                  IS_AFFECTED(ch, AFF_SCRYING) ||
                  IS_EXIT_FLAG( pexit, EX_xLOOK ) ||
                  IS_IMMORTAL(ch) ) )
        {
            if ( !IS_EXIT_FLAG( pexit, EX_xLOOK )
                 &&    get_trust( ch ) < LEVEL_IMMORTAL )
            {
                set_char_color( AT_GREY, ch );
                send_to_char( "You see nothing...\n\r", ch );
                /* Change by Narn, Sept 96 to allow characters who don't have the
                 scry spell to benefit from objects that are affected by scry.

                 Removed by Heath, Dec 97 to accomodate dale like scrying

                 Dale like scrying isn't like this at all ;) this
                 is spy, scry is done upon a victim to see them anywhere
                 in the world - Jesse
                 */
#if 0
                if (!IS_NPC(ch) )
                {
                    int percent = LEARNED(ch, gsn_spy);
                    if ( !percent )
                        percent = 55;		/* 95 was too good -Thoric */

                    if(  number_percent( ) > percent )
                    {
                        send_to_char( "You fail.\n\r", ch );
                        return;
                    }
                }
#endif
            }
            if ( room_is_private( pexit->to_room )
                 &&   get_trust(ch) < sysdata.level_override_private )
            {
                set_char_color( AT_WHITE, ch );
                send_to_char( "That room is private buster!\n\r", ch );
                return;
            }
            original = ch->in_room;
            char_from_room( ch );
            char_to_room( ch, pexit->to_room );
            do_look( ch, "auto" );
            char_from_room( ch );
            char_to_room( ch, original );
        }
        return;
    }
    else
        if ( door != -1 )
        {
            send_to_char( "You see nothing special.\n\r", ch );
            return;
        }

    if ( ( victim = get_char_room( ch, arg1 ) ) != NULL )
    {
        show_char_to_char_1( victim, ch );
#ifdef VTRACK
        vtrack_add_mob(ch, victim->vnum);
#endif
        return;
    }


    /* finally fixed the annoying look 2.obj desc bug	-Thoric */
    number = number_argument( arg1, arg );
    for ( cnt = 0, obj = ch->last_carrying; obj; obj = obj->prev_content )
    {
        if ( can_see_obj( ch, obj ) )
        {
            if ( (pdesc=get_extra_descr(arg, obj->first_extradesc)) != NULL )
            {
                if ( (cnt += obj->count) < number )
                    continue;
                send_to_char( pdesc, ch );
                if ( doexaprog ) oprog_examine_trigger( ch, obj );
                return;
            }

            if ( (pdesc=get_extra_descr(arg, obj->pIndexData->first_extradesc)) != NULL )
            {
                if ( (cnt += obj->count) < number )
                    continue;
                send_to_char( pdesc, ch );
                if ( doexaprog ) oprog_examine_trigger( ch, obj );
                return;
            }
            if ( nifty_is_name_prefix( arg, obj->name ) )
            {
                if ( (cnt += obj->count) < number )
                    continue;
                pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );
                if ( !pdesc )
                    pdesc = get_extra_descr( obj->name, obj->first_extradesc );
                if ( !pdesc )
                    send_to_char( "You see nothing special.\r\n", ch );
                else
                    send_to_char( pdesc, ch );
                if ( doexaprog ) oprog_examine_trigger( ch, obj );
                return;
            }
#ifdef VTRACK
            vtrack_add_obj(ch, obj->vnum);
#endif
        }
    }

    for ( obj = ch->in_room->last_content; obj; obj = obj->prev_content )
    {
        if ( can_see_obj( ch, obj ) )
        {
            if ( (pdesc=get_extra_descr(arg, obj->first_extradesc)) != NULL )
            {
                if ( (cnt += obj->count) < number )
                    continue;
                send_to_char( pdesc, ch );
                if ( doexaprog ) oprog_examine_trigger( ch, obj );
                return;
            }

            if ( (pdesc=get_extra_descr(arg, obj->pIndexData->first_extradesc)) != NULL )
            {
                if ( (cnt += obj->count) < number )
                    continue;
                send_to_char( pdesc, ch );
                if ( doexaprog ) oprog_examine_trigger( ch, obj );
                return;
            }
            if ( nifty_is_name_prefix( arg, obj->name ) )
            {
                if ( (cnt += obj->count) < number )
                    continue;
                pdesc = get_extra_descr( obj->name, obj->pIndexData->first_extradesc );
                if ( !pdesc )
                    pdesc = get_extra_descr( obj->name, obj->first_extradesc );
                if ( !pdesc )
                    send_to_char( "You see nothing special.\r\n", ch );
                else
                    send_to_char( pdesc, ch );
                if ( doexaprog ) oprog_examine_trigger( ch, obj );
                return;
            }
#ifdef VTRACK
            vtrack_add_obj(ch, obj->vnum);
#endif
        }
    }

    send_to_char( "You do not see that here.\n\r", ch );

    return;
}

void show_condition( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    int percent;

    if ( GET_MAX_HIT(victim) > 0 )
        percent = ( 100 * GET_HIT(victim) ) / GET_MAX_HIT(victim);
    else
        percent = -1;

    strcpy( buf, PERS(victim, ch) );

    if ( percent >= 100 ) strcat( buf, " is in perfect health.\n\r"  );
    else if ( percent >=  90 ) strcat( buf, " is slightly scratched.\n\r" );
    else if ( percent >=  80 ) strcat( buf, " has a few bruises.\n\r"     );
    else if ( percent >=  70 ) strcat( buf, " has some cuts.\n\r"         );
    else if ( percent >=  60 ) strcat( buf, " has several wounds.\n\r"    );
    else if ( percent >=  50 ) strcat( buf, " has many nasty wounds.\n\r" );
    else if ( percent >=  40 ) strcat( buf, " is bleeding freely.\n\r"    );
    else if ( percent >=  30 ) strcat( buf, " is covered in blood.\n\r"   );
    else if ( percent >=  20 ) strcat( buf, " is leaking guts.\n\r"       );
    else if ( percent >=  10 ) strcat( buf, " is almost dead.\n\r"        );
    else                       strcat( buf, " is DYING.\n\r"              );

    buf[0] = UPPER(buf[0]);
    send_to_char( buf, ch );
    return;
}

/* A much simpler version of look, this function will show you only
 the condition of a mob or pc, or if used without an argument, the
 same you would see if you enter the room and have config +brief.
 -- Narn, winter '96
 */
void do_glance( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int save_act;

    if ( !ch->desc )
        return;

    if ( ch->position < POS_SLEEPING )
    {
        send_to_char( "You can't see anything but stars!\n\r", ch );
        return;
    }

    if ( ch->position == POS_SLEEPING )
    {
        send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
        return;
    }

    if ( !check_blind( ch ) )
        return;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
        save_act = ch->act;
        SET_PLR_FLAG( ch, PLR_BRIEF );
        do_look( ch, "auto" );
        ch->act = save_act;
        return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They're not here.", ch );
        return;
    }
    else
    {
        if ( can_see( victim, ch ) )
        {
            act( AT_ACTION, "$n glances at you.", ch, NULL, victim, TO_VICT    );
            act( AT_ACTION, "$n glances at $N.",  ch, NULL, victim, TO_NOTVICT );
        }

        show_condition( ch, victim );
        return;
    }

    return;
}


void do_examine( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    sh_int dam;

    if ( !argument )
    {
        bug( "do_examine: null argument.");
        return;
    }

    if ( !ch )
    {
        bug( "do_examine: null ch.");
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Examine what?\n\r", ch );
        return;
    }

    sprintf( buf, "%s noprog", arg );
    do_look( ch, buf );

    /*
     * Support for looking at boards, checking equipment conditions,
     * and support for trigger positions by Thoric
     */
    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
#ifdef VTRACK
        vtrack_add_obj(ch, obj->vnum);
#endif

        switch ( obj->item_type )
        {
        default:
            break;

        case ITEM_ARMOR:
            if ( obj->value[1] == 0 )
                obj->value[1] = obj->value[0];
            if ( obj->value[1] == 0 )
                obj->value[1] = 1;
            dam = (sh_int) ((obj->value[0] * 10) / obj->value[1]);
            strcpy( buf, "As you look more closely, you notice that it is ");
            if (dam >= 10) strcat( buf, "in superb condition.");
            else if (dam ==  9) strcat( buf, "in very good condition.");
            else if (dam ==  8) strcat( buf, "in good shape.");
            else if (dam ==  7) strcat( buf, "showing a bit of wear.");
            else if (dam ==  6) strcat( buf, "a little run down.");
            else if (dam ==  5) strcat( buf, "in need of repair.");
            else if (dam ==  4) strcat( buf, "in great need of repair.");
            else if (dam ==  3) strcat( buf, "in dire need of repair.");
            else if (dam ==  2) strcat( buf, "very badly worn.");
            else if (dam ==  1) strcat( buf, "practically worthless.");
            else if (dam <=  0) strcat( buf, "broken.");
            strcat( buf, "\n\r" );
            send_to_char( buf, ch );
            break;

        case ITEM_WEAPON:
            dam = INIT_WEAPON_CONDITION - obj->value[0];
            strcpy( buf, "As you look more closely, you notice that it is ");
            if (dam ==  0) strcat( buf, "in superb condition.");
            else if (dam ==  1) strcat( buf, "in excellent condition.");
            else if (dam ==  2) strcat( buf, "in very good condition.");
            else if (dam ==  3) strcat( buf, "in good shape.");
            else if (dam ==  4) strcat( buf, "showing a bit of wear.");
            else if (dam ==  5) strcat( buf, "a little run down.");
            else if (dam ==  6) strcat( buf, "in need of repair.");
            else if (dam ==  7) strcat( buf, "in great need of repair.");
            else if (dam ==  8) strcat( buf, "in dire need of repair.");
            else if (dam ==  9) strcat( buf, "very badly worn.");
            else if (dam == 10) strcat( buf, "practically worthless.");
            else if (dam == 11) strcat( buf, "almost broken.");
            else if (dam == 12) strcat( buf, "broken.");
            strcat( buf, "\n\r" );
            send_to_char( buf, ch );
            break;

        case ITEM_FOOD:
            if ( obj->timer > 0 && obj->value[1] > 0 )
                dam = (obj->timer * 10) / obj->value[1];
            else
                dam = 10;
            strcpy( buf, "As you examine it carefully you notice that it " );
            if (dam >= 10) strcat( buf, "is fresh.");
            else if (dam ==  9) strcat( buf, "is nearly fresh.");
            else if (dam ==  8) strcat( buf, "is perfectly fine.");
            else if (dam ==  7) strcat( buf, "looks good.");
            else if (dam ==  6) strcat( buf, "looks ok.");
            else if (dam ==  5) strcat( buf, "is a little stale.");
            else if (dam ==  4) strcat( buf, "is a bit stale.");
            else if (dam ==  3) strcat( buf, "smells slightly off.");
            else if (dam ==  2) strcat( buf, "smells quite rank.");
            else if (dam ==  1) strcat( buf, "smells revolting.");
            else if (dam <=  0) strcat( buf, "is crawling with maggots.");
            strcat( buf, "\n\r" );
            send_to_char( buf, ch );
            break;

        case ITEM_SWITCH:
        case ITEM_LEVER:
        case ITEM_PULLCHAIN:
            if ( IS_SET( obj->value[0], TRIG_UP ) )
                send_to_char( "You notice that it is in the up position.\n\r", ch );
            else
                send_to_char( "You notice that it is in the down position.\n\r", ch );
            break;
        case ITEM_BUTTON:
            if ( IS_SET( obj->value[0], TRIG_UP ) )
                send_to_char( "You notice that it is depressed.\n\r", ch );
            else
                send_to_char( "You notice that it is not depressed.\n\r", ch );
            break;

            /* Not needed due to check in do_look already
             case ITEM_PORTAL:
             sprintf( buf, "in %s noprog", arg );
             do_look( ch, buf );
             break;
             */

        case ITEM_CORPSE_PC:
        case ITEM_CORPSE_NPC:
            {
                sh_int timerfrac = obj->timer;
                if ( obj->item_type == ITEM_CORPSE_PC )
                    timerfrac = (int)obj->timer / 8 + 1;

                switch (timerfrac)
                {
                default:
                    send_to_char( "This corpse has recently been slain.\n\r", ch );
                    break;
                case 4:
                    send_to_char( "This corpse was slain a little while ago.\n\r", ch );
                    break;
                case 3:
                    send_to_char( "A foul smell rises from the corpse, and it is covered in flies.\n\r", ch );
                    break;
                case 2:
                    send_to_char( "A writhing mass of maggots and decay, you can barely go near this corpse.\n\r", ch );
                    break;
                case 1:
                case 0:
                    send_to_char( "Little more than bones, there isn't much left of this corpse.\n\r", ch );
                    break;
                }
            }
        case ITEM_CONTAINER:
            if ( IS_OBJ_STAT( obj, ITEM_COVERING ) )
                break;

        case ITEM_DRINK_CON:
            send_to_char( "When you look inside, you see:\n\r", ch );
            sprintf( buf, "in %s noprog", arg );
            do_look( ch, buf );
        }

        if ( IS_OBJ_STAT( obj, ITEM_COVERING ) )
        {
            sprintf( buf, "under %s noprog", arg );
            do_look( ch, buf );
        }
        oprog_examine_trigger( ch, obj );
        if( char_died(ch) || obj_extracted(obj) )
            return;

        check_for_trap( ch, obj, TRAP_EXAMINE );
    }
    return;
}

void do_exits( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    bool fAuto;

    buf[0] = '\0';
    fAuto  = !str_cmp( argument, "auto" );

#ifdef MXP
    if (fAuto && MXP_ON(ch))
        send_to_char(MXP_TAG_ROOMEXIT, ch);
#endif

    switch(GET_INTF(ch))
    {
    case INT_SMAUG:
        break;
    default:
        set_char_color(AT_GREY, ch);
        break;
    }

    if ( !check_blind( ch ) )
        return;

    if (fAuto)
	strcpy(buf, "Exits:");
    else
	strcpy(buf, "Obvious exits:\n\r");

    if (fAuto)
    {
        switch(GET_INTF(ch))
        {
        default:
        case INT_IMP:
        case INT_DALE:
            paint(AT_DGREY, ch, "%s", buf);
            break;
        case INT_SMAUG:
            paint(AT_WHITE, ch, "%s", buf);
            break;
        }
    }
    else
	send_to_char(buf, ch);

    found = FALSE;
    for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
    {
        if ( pexit->to_room
             &&  (!IS_EXIT_FLAG(pexit, EX_CLOSED) || IS_IMMORTAL(ch))
             && (!IS_EXIT_FLAG(pexit, EX_WINDOW) || IS_IMMORTAL(ch)
                 ||   IS_EXIT_FLAG(pexit, EX_ISDOOR))
             &&  (!IS_EXIT_FLAG(pexit, EX_HIDDEN) || IS_IMMORTAL(ch)) )
        {
            found = TRUE;
            if ( fAuto )
            {
                switch(GET_INTF(ch)) {
                default:
                case INT_IMP:
		case INT_DALE:
		    if (pexit->vdir > LAST_NORMAL_DIR)
			set_char_color(AT_WHITE,ch);
		    else
			set_char_color(dir_color[pexit->vdir],ch);
                case INT_SMAUG:
#define ISP(d) (IS_EXIT_FLAG(pexit, (d)))
                    ch_printf(ch, " %s%s%s%s%s",
                              (ISP(EX_CLOSED) ? "(" : \
                               (ISP(EX_HIDDEN) ? "[" : \
                                (ISP(EX_WINDOW) ? "{" : "" ))),
#ifdef MXP
                              MXP_ON(ch) ? "<Ex>" : "",
#else
                              "",
#endif
                              exit_name(pexit),
#ifdef MXP
                              MXP_ON(ch) ? "</Ex>" : "",
#else
                              "",
#endif
                              (ISP(EX_CLOSED) ? ")" : \
                               (ISP(EX_HIDDEN) ? "]" : \
                                (ISP(EX_WINDOW) ? "}" : "" )))
                             );
#undef ISP
                    break;
                }
            }
            else
            {
                if (IS_IMMORTAL(ch))
                    ch_printf( ch, "%-9s - [%-5d] %s\n\r",
                             capitalize( exit_name(pexit) ),
                             pexit->to_room->vnum,
                             pexit->to_room->name
                           );
                else
                    ch_printf( ch, "%-9s - %s\n\r",
                             capitalize( exit_name(pexit) ),
                             room_is_dark( pexit->to_room )
                             ?  "Too dark to tell"
                             : pexit->to_room->name
                           );
            }
        }
    }

    if ( !found )
    {
        if (fAuto)
        {
            paint(AT_CYAN, ch, " None!");
#ifdef MXP
            if (MXP_ON(ch))
                send_to_char(MXP_TAG_ROOMEXIT_CLOSE, ch);
#endif
        }
	else
	    send_to_char("None.", ch);
        send_to_char("\n\r", ch);
    }
    else if ( fAuto )
    {
#ifdef MXP
        if (MXP_ON(ch))
            send_to_char(MXP_TAG_ROOMEXIT_CLOSE, ch);
#endif
        send_to_char("\n\r", ch);
    }

    set_char_color( AT_PLAIN, ch );
    return;
}

char *	const	day_name	[] =
{
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

char *	const	month_name	[] =
{
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time( CHAR_DATA *ch, char *argument )
{
    char *suf;
    char s1[16], s2[16];
    int day;

    sprintf(s1, "%s", color_str(AT_SCORE, ch));
    sprintf(s2, "%s", color_str(AT_SCORE2, ch));

    day     = time_info.day + 1;

    if ( day > 4 && day <  20 ) suf = "th";
    else if ( day % 10 ==  1       ) suf = "st";
    else if ( day % 10 ==  2       ) suf = "nd";
    else if ( day % 10 ==  3       ) suf = "rd";
    else                             suf = "th";

    if ( sysdata.longest_uptime < (current_time - boot_time) )
    {
        sysdata.longest_uptime = current_time - boot_time;
        save_sysdata(sysdata);
    }

    set_char_color( AT_SCORE, ch );
    ch_printf( ch,
               "It is %d%s, day of %s, %d%s of the month of %s.\n\r\n\r"
               "%sThe mud started up at  :  %s%s\r"
               "%sThe system time (MST)  :  %s%s\r",

               (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
               time_info.hour >= 12 ? "pm" : "am",
               day_name[day % 7],
               day, suf,
               month_name[time_info.month],
               s1, s2, str_boot_time,
               s1, s2, (char *) ctime( &current_time )
             );

    if (IS_IMMORTAL(ch))
        ch_printf( ch, "%sNext Reboot is set for :  %s%s\r",
                   s1, s2, reboot_time );

    ch_printf( ch, "%sThe mud has been up for:  %s%s\n\r",
               s1, s2, sec_to_hms(current_time - boot_time) );
    ch_printf( ch, "%sLongest uptime recorded:  %s%s\n\r",
               s1, s2, sec_to_hms(sysdata.longest_uptime) );
    return;
}


/*
 void do_weather( CHAR_DATA *ch, char *argument )
 {
 static char * const sky_look[4] =
 {
 "cloudless",
 "cloudy",
 "rainy",
 "lit by flashes of lightning"
 };

 if ( !IS_OUTSIDE(ch) )
 {
 send_to_char( "You can't see the sky from here.\n\r", ch );
 return;
 }

 set_char_color( AT_BLUE, ch );
 ch_printf( ch, "The sky is %s and %s.\n\r",
 sky_look[weather_info.sky],
 weather_info.change >= 0
 ? "a warm southerly breeze blows"
 : "a cold northern gust blows"
 );
 return;
 }
 */

char *get_weather_icons(char *areaname)
{
    static char icons[32];
    AREA_DATA *area;
    int temp, precip, wind;

    if (!(area = get_area(areaname)))
    {
        sprintf(icons, "   ");
        return icons;
    }

    temp = (area->weather->temp + 3*weath_unit - 1)/
        weath_unit;
    precip = (area->weather->precip + 3*weath_unit - 1)/
        weath_unit;
    wind = (area->weather->wind + 3*weath_unit - 1)/
        weath_unit;

    icons[0] = '\0';

    switch (temp)
    {
    case 0: strcat(icons, "&CC"); break;
    case 1: strcat(icons, "&BC"); break;
    case 2: strcat(icons, "&WN"); break;
    case 3: strcat(icons, "&OH"); break;
    case 4: strcat(icons, "&YH"); break;
    default: strcat(icons, "&px"); break;
    }

    switch (precip)
    {
    case 0: strcat(icons, "&OD"); break;
    case 1: strcat(icons, "&OD"); break;
    case 2: strcat(icons, "&WN"); break;
    case 3: strcat(icons, "&BR"); break;
    case 4: strcat(icons, "&BR"); break;
    default: strcat(icons, "&px"); break;
    }

    switch (wind)
    {
    case 0: strcat(icons, "&GS"); break;
    case 1: strcat(icons, "&GS"); break;
    case 2: strcat(icons, "&WN"); break;
    case 3: strcat(icons, "&RW"); break;
    case 4: strcat(icons, "&RW"); break;
    default: strcat(icons, "&px"); break;
    }

    return icons;
}


/*
 * Produce a description of the weather based on area weather using
 * the following sentence format:
 *		<combo-phrase> and <single-phrase>.
 * Where the combo-phrase describes either the precipitation and
 * temperature or the wind and temperature. The single-phrase
 * describes either the wind or precipitation depending upon the
 * combo-phrase.
 * Last Modified: July 31, 1997
 * Fireblade - Under Construction
 */
void do_weather(CHAR_DATA *ch, char *argument)
{
    char *combo, *single;
    int temp, precip, wind;

    if (IS_IMMORTAL(ch) && !str_cmp(argument, "map"))
    {
        ch_printf(ch, "%s  ", get_weather_icons("The Shire"));
        ch_printf(ch, "%s  ", get_weather_icons("Arthrukin Manor"));
        ch_printf(ch, "%s  ", get_weather_icons("The Great Silverstone Library"));
        ch_printf(ch, "%s  ", get_weather_icons("Temple of Lanthander"));
        ch_printf(ch, "%s\n\r", get_weather_icons("The Desolation"));
        ch_printf(ch, "%s  ", get_weather_icons("Haon-Dor-Light"));
        ch_printf(ch, "%s  ", "   ");
        ch_printf(ch, "%s  ", "   ");
        ch_printf(ch, "%s  ", "   ");
        ch_printf(ch, "%s\n\r", get_weather_icons("Geldor and Midir"));
        ch_printf(ch, "%s  ", get_weather_icons("Trade Route"));
        ch_printf(ch, "%s  ", "   ");
        ch_printf(ch, "%s  ", get_weather_icons("Silverstone"));
        ch_printf(ch, "%s  ", "   ");
        ch_printf(ch, "%s\n\r", get_weather_icons("The Chessboard"));
        ch_printf(ch, "%s  ", get_weather_icons("Silverstone Concourse"));
        ch_printf(ch, "%s  ", "   ");
        ch_printf(ch, "%s  ", "   ");
        ch_printf(ch, "%s  ", "   ");
        ch_printf(ch, "%s\n\r", get_weather_icons("Housing Complex"));
        ch_printf(ch, "%s  ", get_weather_icons("Ator's Church"));
        ch_printf(ch, "%s  ", "   ");
        ch_printf(ch, "%s  ", get_weather_icons("The Land of Tyrs"));
        ch_printf(ch, "%s  ", get_weather_icons("The Torture Keep"));
        ch_printf(ch, "%s\n\r", "   ");
        return;
    }

    if ( !IS_IMMORTAL(ch) && (!IS_OUTSIDE(ch) || !IS_AWAKE(ch)) )
    {
        ch_printf(ch, "You can't see the sky from here.\n\r");
        return;
    }

    temp = (ch->in_room->area->weather->temp + 3*weath_unit - 1)/
        weath_unit;
    precip = (ch->in_room->area->weather->precip + 3*weath_unit - 1)/
        weath_unit;
    wind = (ch->in_room->area->weather->wind + 3*weath_unit - 1)/
        weath_unit;

    if ( precip >= 3 )
    {
        combo = preciptemp_msg[precip][temp];
        single = wind_msg[wind];
    }
    else
    {
        combo = windtemp_msg[wind][temp];
        single = precip_msg[precip];
    }

    set_char_color(AT_BLUE, ch);
    ch_printf(ch, "%s and %s.\n\r", combo, single);
    if (IS_IMMORTAL(ch))
    {
        ch_printf(ch, "precip: %d  temp: %d  wind: %d\n\r",
                  precip, temp, wind);
    }
}


/*
 * Moved into a separate function so it can be used for other things
 * ie: online help editing				-Thoric
 */
HELP_DATA *get_help( CHAR_DATA *ch, char *argument )
{
    char argall[MAX_INPUT_LENGTH];
    char argone[MAX_INPUT_LENGTH];
    char argnew[MAX_INPUT_LENGTH];
    HELP_DATA *pHelp;
    int lev;

    if ( argument[0] == '\0' )
        argument = "summary";

    if ( isdigit(argument[0]) )
    {
        lev = number_argument( argument, argnew );
        argument = argnew;
    }
    else
        lev = -2;
    /*
     * Tricky argument handling so 'help a b' doesn't match a.
     */
    argall[0] = '\0';
    while ( argument[0] != '\0' )
    {
        argument = one_argument( argument, argone );
        if ( argall[0] != '\0' )
            strcat( argall, " " );
        strcat( argall, argone );
    }

    for ( pHelp = first_help; pHelp; pHelp = pHelp->next )
    {
        if ( ch )
        {
            if ( pHelp->level > get_trust( ch ) )
                continue;
        }

        if ( lev != -2 && pHelp->level != lev )
            continue;

        if ( is_name( argall, pHelp->keyword ) )
            return pHelp;
    }

    for ( pHelp = first_help; pHelp; pHelp = pHelp->next )
    {
        if ( ch )
        {
            if ( pHelp->level > get_trust( ch ) )
                continue;
        }

        if ( lev != -2 && pHelp->level != lev )
            continue;

        if ( is_name_prefix( argall, pHelp->keyword ) )
            return pHelp;
    }

    return NULL;
}


/*
 * Now this is cleaner
 */
void do_help( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;

    if ( (pHelp = get_help( ch, argument )) == NULL )
    {
        send_to_char( "No help on that word.\n\r", ch );
        log_printf_plus(LOG_MONITOR, UMAX(GetMaxLevel(ch),LEVEL_IMMORTAL), SEV_CRIT,
                        "do_help: %s tried to get help on %s", GET_NAME(ch), argument);
        return;
    }

    if ( pHelp->level >= 0 && str_cmp( argument, "imotd" ) )
    {
        send_to_pager( pHelp->keyword, ch );
        send_to_pager( "\n\r", ch );
    }

    /*
     * Strip leading '.' to allow initial blanks.
     */
    if ( pHelp->text[0] == '.' )
        send_to_pager_color( pHelp->text+1, ch );
    else
        send_to_pager_color( pHelp->text  , ch );
    return;
}

/*
 * Help editor							-Thoric
 */
void do_hedit( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;

    if ( !ch->desc )
    {
        send_to_char( "You have no descriptor.\n\r", ch );
        return;
    }

    switch( ch->substate )
    {
    default:
        break;
    case SUB_HELP_EDIT:
        if ( (pHelp = (HELP_DATA *)ch->dest_buf) == NULL )
        {
            bug( "hedit: sub_help_edit: NULL ch->dest_buf" );
            stop_editing( ch );
            return;
        }
        STRFREE( pHelp->text );
        pHelp->text = copy_buffer( ch );
        stop_editing( ch );
        if ( !str_cmp( pHelp->keyword, "greeting" ) )
            help_greeting = pHelp->text;
        return;
    }
    if ( (pHelp = get_help( ch, argument )) == NULL )	/* new help */
    {
        char argnew[MAX_INPUT_LENGTH];
        int lev;

        if ( isdigit(argument[0]) )
        {
            lev = number_argument( argument, argnew );
            argument = argnew;
        }
        else
            lev = get_trust(ch);
        CREATE( pHelp, HELP_DATA, 1 );
        pHelp->keyword = STRALLOC( strupper(argument) );
        pHelp->text    = STRALLOC( "" );
        pHelp->level   = lev;
        add_help( pHelp );
    }
    ch->substate = SUB_HELP_EDIT;
    ch->dest_buf = pHelp;
    start_editing( ch, pHelp->text );
}

/*
 * Stupid leading space muncher fix				-Thoric
 */
char *help_fix( char *text )
{
    char *fixed;

    if ( !text )
        return "";
    fixed = strip_cr(text);
    if ( fixed[0] == ' ' )
        fixed[0] = '.';
    return fixed;
}

void do_hset( CHAR_DATA *ch, char *argument )
{
    HELP_DATA *pHelp;
    char arg1[MAX_INPUT_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: hset [help page] <field> [value]\n\r", ch );
        send_to_char( "\n\r",						ch );
        send_to_char( "Field being one of:\n\r",			ch );
        send_to_char( "  level keyword remove save\n\r",		ch );
        return;
    }

    if ( !str_cmp( arg1, "save" ) )
    {
#if defined(START_DB) || defined(USE_DB)
        log_string_plus("Saving helps...", LOG_NORMAL, LEVEL_GREATER);
        db_insert_helps();
#else
        FILE *fpout;

        log_string_plus( "Saving help.are...", LOG_NORMAL, LEVEL_GREATER, SEV_INFO );

        rename( "help.are", "help.are.bak" );
        if ( ( fpout = fopen( "help.are", "w" ) ) == NULL )
        {
            bug( "hset save: fopen" );
            perror( "help.are" );
            return;
        }

        fprintf( fpout, "#HELPS\n\n" );
        for ( pHelp = first_help; pHelp; pHelp = pHelp->next )
            fprintf( fpout, "%d %s~\n%s~\n\n",
                     pHelp->level, pHelp->keyword, help_fix(pHelp->text) );

        fprintf( fpout, "0 $~\n\n\n#$\n" );
        fclose( fpout );
#endif
        send_to_char( "Saved.\n\r", ch );
        return;
    }

    if ( (pHelp = get_help( ch, arg1 )) == NULL )
    {
        send_to_char( "Cannot find help on that subject.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );

    if ( !str_cmp( arg1, "remove" ) )
    {
        UNLINK( pHelp, first_help, last_help, next, prev );
        STRFREE( pHelp->text );
        STRFREE( pHelp->keyword );
        DISPOSE( pHelp );
        send_to_char( "Removed.\n\r", ch );
        return;
    }
    if ( !str_cmp( arg1, "level" ) )
    {
        pHelp->level = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }
    if ( !str_cmp( arg1, "keyword" ) )
    {
        STRFREE( pHelp->keyword );
        pHelp->keyword = STRALLOC( strupper(argument) );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    do_hset( ch, "" );
}

/*
 * Show help topics in a level range				-Thoric
 * Idea suggested by Gorog
 */
void do_hlist( CHAR_DATA *ch, char *argument )
{
    int min, max, minlimit, maxlimit, cnt;
    char arg[MAX_INPUT_LENGTH];
    HELP_DATA *help;

    maxlimit = get_trust(ch);
    minlimit = maxlimit >= LEVEL_GREATER ? -1 : 0;
    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' )
    {
        min = URANGE( minlimit, atoi(arg), maxlimit );
        if ( argument[0] != '\0' )
            max = URANGE( min, atoi(argument), maxlimit );
        else
            max = maxlimit;
    }
    else
    {
        min = minlimit;
        max = maxlimit;
    }
    set_pager_color( AT_GREEN, ch );
    pager_printf( ch, "Help Topics in level range %d to %d:\n\r\n\r", min, max );
    for ( cnt = 0, help = first_help; help; help = help->next )
        if ( help->level >= min && help->level <= max )
        {
            pager_printf( ch, "  %3d %s\n\r", help->level, help->keyword );
            ++cnt;
        }
    if ( cnt )
        pager_printf( ch, "\n\r%d pages found.\n\r", cnt );
    else
        send_to_char( "None found.\n\r", ch );
}

/* do_who by Heath */

void do_who( CHAR_DATA *ch, char *argument )
{
    if (!ch)
        smaug_who(ch,argument);


    switch (GET_INTF(ch))
    {
    case INT_DALE:
        dale_who(ch, argument);
        break;
    case INT_SMAUG:
        smaug_who(ch, argument);
        break;
    case INT_MERC:
    case INT_ENVY:
        envy_who(ch, argument);
        break;
    case INT_IMP:
        imp_who(ch, argument);
        break;
    }
}

void do_compare( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
        send_to_char( "Compare what to what?\n\r", ch );
        return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1 ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        for ( obj2 = ch->first_carrying; obj2; obj2 = obj2->next_content )
        {
            if ( obj2->wear_loc != WEAR_NONE
                 &&   can_see_obj( ch, obj2 )
                 &&   obj1->item_type == obj2->item_type
                 && ( obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
                break;
        }

        if ( !obj2 )
        {
            send_to_char( "You aren't wearing anything comparable.\n\r", ch );
            return;
        }
    }
    else
    {
        if ( ( obj2 = get_obj_carry( ch, arg2 ) ) == NULL )
        {
            send_to_char( "You do not have that item.\n\r", ch );
            return;
        }
    }

    msg		= NULL;
    value1	= 0;
    value2	= 0;

    if ( obj1 == obj2 )
    {
        msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
        msg = "You can't compare $p and $P.";
    }
    else
    {
        switch ( obj1->item_type )
        {
        default:
            msg = "You can't compare $p and $P.";
            break;

        case ITEM_ARMOR:
            value1 = obj1->value[0];
            value2 = obj2->value[0];
            break;

        case ITEM_WEAPON:
            value1 = obj1->value[1] + obj1->value[2];
            value2 = obj2->value[1] + obj2->value[2];
            break;
        }
    }

    if ( !msg )
    {
        if ( value1 == value2 ) msg = "$p and $P look about the same.";
        else if ( value1  > value2 ) msg = "$p looks better than $P.";
        else                         msg = "$p looks worse than $P.";
    }

    act( AT_PLAIN, msg, ch, obj1, obj2, TO_CHAR );
    return;
}


void do_where( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    bool found;

    one_argument( argument, arg );

    set_pager_color( AT_PERSON, ch );
    if ( arg[0] == '\0' )
    {
        if (IS_IMMORTAL(ch))
        {
            found = FALSE;
            for ( victim = first_char; victim; victim = victim->next )
                if ( victim->in_room && !IS_NPC(victim)
                     &&   can_see( ch, victim ) )
                {
                    found = TRUE;
                    pager_printf( ch, "%-16s [%-5d] %-20s %s\n\r",
                                  victim->name,
                                  victim->in_room->vnum,
                                  victim->in_room->name,
                                  victim->in_room->area->name);
                }
            if ( !found )
                send_to_char( "None.\n\r", ch );
        }
        else
        {
            do_whozone( ch, argument );
        }
        return;
    }
    else if ( !str_cmp(arg, "_clan_") )
    {
        found = FALSE;
        for ( victim = first_char; victim; victim = victim->next )
            if ( victim->in_room &&
                 !IS_NPC(victim) &&
                 can_see( ch, victim ) &&
                 ch->pcdata->clan &&
                 ch->pcdata->clan == victim->pcdata->clan )
            {
                found = TRUE;
                pager_printf( ch, "%-28s %s\n\r",
                              PERS(victim, ch),
                              victim->in_room->name );
            }
        if ( !found )
            act( AT_PLAIN, "You didn't find any clan members anywehre.", ch, NULL, NULL, TO_CHAR );
        return;
    }
    else
    {
        if (IS_IMMORTAL(ch))
        {
            do_mwhere( ch, argument );
            do_owhere( ch, argument );
        }
        else
        {
            found = FALSE;
            for ( victim = first_char; victim; victim = victim->next )
                if ( victim->in_room
                     &&   victim->in_room->area==ch->in_room->area
                     &&   can_see( ch, victim )
                     &&   is_name( arg, victim->name ) )
                {
                    found = TRUE;
                    pager_printf( ch, "%-28s %s\n\r",
                                  PERS(victim, ch),
                                  victim->in_room->name );
                }
            if ( !found )
                act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
        }
    }

    return;
}

void do_whozone( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim = NULL;
    bool found;

    one_argument( argument, arg );

    set_pager_color( AT_PERSON, ch );
    if ( arg[0] == '\0' )
    {
        pager_printf( ch, "Players near you in '%s':\n\r----------------------------------------\n\r",
                      ch->in_room->area->name );
        found = FALSE;
        for ( victim = first_char; victim; victim = victim->next )
            if ( (GET_CON_STATE(victim) == CON_PLAYING || GET_CON_STATE(victim) == CON_EDITING )
                 &&   !IS_NPC(victim)
                 &&   victim->in_room
                 &&   victim->in_room->area == ch->in_room->area
                 &&   can_see( ch, victim ) )
            {
                found = TRUE;
                if (IS_IMMORTAL(ch))
                    pager_printf( ch, "%-40s %s [%d]\n\r",
                                  PERS(victim, ch),
                                  victim->in_room->name,
                                  victim->in_room->vnum );
                else
                    pager_printf( ch, "%-40s %s\n\r",
                                  PERS(victim, ch),
                                  victim->in_room->name );
            }
        if ( !found )
            send_to_char( "None\n\r", ch );
    }
    else
    {
        found = FALSE;
        for ( victim = first_char; victim; victim = victim->next )
            if ( victim->in_room
                 &&   victim->in_room->area == ch->in_room->area
                 &&   !IS_AFFECTED(victim, AFF_HIDE)
                 &&   !IS_AFFECTED(victim, AFF_SNEAK)
                 &&   can_see( ch, victim )
                 &&   is_name( arg, victim->name ) )
            {
                found = TRUE;
                if (IS_IMMORTAL(ch))
                    pager_printf( ch, "%-40s [%-5d] %s\n\r",
                                  PERS(victim, ch),
                                  ch->in_room->vnum,
                                  victim->in_room->name );
                else
                    pager_printf( ch, "%-40s %s\n\r",
                                  PERS(victim, ch),
                                  victim->in_room->name );
                break;
            }
        if ( !found )
            act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    }

    return;
}

#if 0
void do_consider( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char *msg;
    int diff;
    float me, him, tmp;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Consider killing whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They're not here.\n\r", ch );
        return;
    }

    diff = GetMaxLevel(victim) - GetMaxLevel(ch);

    if ( diff <= -10 ) msg = "You are far more experienced than $N.";
    else if ( diff <=  -5 ) msg = "$N is not nearly as experienced as you.";
    else if ( diff <=  -2 ) msg = "You are more experienced than $N.";
    else if ( diff <=   1 ) msg = "You are just about as experienced as $N.";
    else if ( diff <=   4 ) msg = "You are not nearly as experienced as $N.";
    else if ( diff <=   9 ) msg = "$N is far more experienced than you!";
    else                    msg = "$N would make a great teacher for you!";
    act( AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR );

    diff = (int) (GET_MAX_HIT(victim) - GET_HIT(ch)) / 6;

    if ( diff <= -200) msg = "$N looks like a feather!";
    else if ( diff <= -150) msg = "You could kill $N with your hands tied!";
    else if ( diff <= -100) msg = "Hey! Where'd $N go?";
    else if ( diff <=  -50) msg = "$N is a wimp.";
    else if ( diff <=    0) msg = "$N looks weaker than you.";
    else if ( diff <=   50) msg = "$N looks about as strong as you.";
    else if ( diff <=  100) msg = "It would take a bit of luck...";
    else if ( diff <=  150) msg = "It would take a lot of luck, and equipment!";
    else if ( diff <=  200) msg = "Why don't you dig a grave for yourself first?";
    else                    msg = "$N is built like a TANK!";
    act( AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR );

    him = ((float)GetMaxLevel(victim)/(float)MAX_LEVEL);
    tmp = ((float)get_curr_int(victim)/25.0) +
        ((float)get_curr_str(victim)/25.0) +
        ((float)get_curr_wis(victim)/25.0) +
        ((float)get_curr_con(victim)/25.0) +
        ((float)get_curr_cha(victim)/25.0) +
        ((float)get_curr_lck(victim)/25.0) +
        ((float)get_curr_dex(victim)/25.0);
    tmp /= 7.0;
    him += tmp;
    tmp = ((float)GET_HIT(victim)/(float)GET_MAX_HIT(victim));
    him += tmp;
    if (ch->armor<0)
        tmp = (float)ch->armor/-500.0;
    else
        tmp = 0.0;
    him += tmp;
    him /= 4.0;

    me  = ((float)GetMaxLevel(ch)/(float)MAX_LEVEL);
    tmp = ((float)get_curr_int(ch)/25.0) +
        ((float)get_curr_str(ch)/25.0) +
        ((float)get_curr_wis(ch)/25.0) +
        ((float)get_curr_con(ch)/25.0) +
        ((float)get_curr_cha(ch)/25.0) +
        ((float)get_curr_lck(ch)/25.0) +
        ((float)get_curr_dex(ch)/25.0);
    tmp /= 7.0;
    me  += tmp;
    tmp = ((float)GET_HIT(ch)/(float)GET_MAX_HIT(ch));
    me  += tmp;
    if (ch->armor<0)
        tmp = (float)ch->armor/-500.0;
    else
        tmp = 0.0;
    me  += tmp;
    me  /= 4.0;

    if (IS_IMMORTAL(ch) && !IS_NPC(ch))
        me = 1.1;
    if (IS_IMMORTAL(victim) && !IS_NPC(victim))
        him = 1.1;

    tmp = him-me;

    if ( tmp <=  -1) msg = "$N looks like a feather!";
    else if ( tmp <=-0.75) msg = "You could kill $N with your hands tied!";
    else if ( tmp <= -0.5) msg = "Hey! Where'd $N go?";
    else if ( tmp <=-0.25) msg = "$N is a wimp.";
    else if ( tmp <=    0) msg = "$N looks weaker than you.";
    else if ( tmp <= 0.25) msg = "$N looks about as strong as you.";
    else if ( tmp <=  0.5) msg = "It would take a bit of luck...";
    else if ( tmp <= 0.75) msg = "It would take a lot of luck, and equipment!";
    else if ( tmp <=    1) msg = "Why don't you dig a grave for yourself first?";
    else                   msg = "$N is built like a TANK!";

    if (IS_IMMORTAL(ch))
    {
        act( AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR );
        ch_printf(ch, "Him: %f, Me: %f\n\r", him, me);
    }
    return;
}
#else
int MobLevBonus(CHAR_DATA *ch)
{
    int t = 0;

/*    if (mob_index[ch->nr].func == magic_user)
        t += 5;
    if (mob_index[ch->nr].func == BreathWeapon)
        t += 7;
    if (mob_index[ch->nr].func == fighter)
        t += 3;
    if (mob_index[ch->nr].func == snake)
        t += 3;*/

    t += (ch->numattacks - 1000) * 3;

    if (t > 1000)
        t /= 1000;

    if (GET_HIT(ch) > GetMaxLevel(ch) * 8)
        t += 1;
    if (GET_HIT(ch) > GetMaxLevel(ch) * 12)
        t += 2;
    if (GET_HIT(ch) > GetMaxLevel(ch) * 16)
        t += 3;
    if (GET_HIT(ch) > GetMaxLevel(ch) * 20)
        t += 4;

    return (t);
}

char *DescRatio(float f)
{
    if (f > 6.0)
        return ("So much more than yours you doubt your knowledge");
    else if (f > 5.0)
        return ("So much more than yours you wonder why you aren't already dead");
    else if (f > 4.0)
        return ("So much more than yours you can't even imagine it");
    else if (f > 3.0)
        return ("A *LOT* more than yours");
    else if (f > 2.0)
        return ("A lot more than yours");
    else if (f > 1.0)
        return ("More than twice yours");
    else if (f > .75)
        return ("More than half again greater than yours");
    else if (f > .6)
        return ("At least a third greater than yours");
    else if (f > .4)
        return ("About the same as yours");
    else if (f > .3)
        return ("A little worse than yours");
    else if (f > .1)
        return ("Much worse than yours");
    else if (f > .05)
        return ("Extremely inferior to yours");
    else if (f > .01)
        return ("Miniscule compared to yours");

    return ("Infitesimel compared to yours");
}

char *DescAttacks(float a)
{
    if (a < 1.0)
        return ("Not many");
    else if (a < 2.0)
        return ("About average");
    else if (a < 3.0)
        return ("A few");
    else if (a < 5.0)
        return ("A lot");
    else if (a < 9.0)
        return ("Many");

    return ("A whole bunch");
}

char *DescDamage(float dam)
{
    if (dam < 1.0)
        return ("Minimal Damage");
    else if (dam <= 2.0)
        return ("Slight damage");
    else if (dam <= 4.0)
        return ("A bit of damage");
    else if (dam <= 10.0)
        return ("A decent amount of damage");
    else if (dam <= 15.0)
        return ("A lot of damage");
    else if (dam <= 25.0)
        return ("A whole lot of damage");
    else if (dam <= 35.0)
        return ("A very large amount");

    return ("A TON of damage");
}

void do_consider(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char name[MAX_INPUT_LENGTH];
    int diff, sn = 0, learn = 0, num1, num2;
    /*    double conratio = 0.0;*/
    float fnum;

    one_argument(argument, name);

    if (!(victim = get_char_room(ch, name)))
    {
        send_to_char("Consider killing who?\n\r", ch);
        return;
    }

    if (victim == ch)
    {
        send_to_char("Easy! Very easy indeed!\n\r", ch);
        return;
    }

    set_char_color(AT_CONSIDER, ch);

    act(AT_CONSIDER, "$n carefully looks at $N.", ch, NULL, victim, TO_NOTVICT);
    act(AT_CONSIDER, "$n carefully looks at you.", ch, NULL, victim, TO_VICT);

    /*
    if (IS_IMMORTAL(ch)) {
        sprintf(buf, "You %d, them %d\n\r", odds(ch), odds(victim));
        send_to_char(buf, ch);
    }
    conratio = (double) ((double) odds(ch) / (double) odds(victim));
    if (conratio <= 0.001)
        send_to_char("Not even the Gods are that lucky.\n\r", ch);
    else if (conratio <= 0.01)
        send_to_char("You really do have a death wish don't you...\n\r", ch);
    else if (conratio <= 0.1)
        send_to_char("You and what army?\n\r", ch);
    else if (conratio <= 0.5)
        send_to_char("You stand a chance.\n\r", ch);
    else if (conratio < 1.0)
        send_to_char("You might be able to take him.\n\r", ch);
    else if (conratio == 1.0)
        send_to_char("A perfect match.\n\r", ch);
    else if (conratio <= 1.5)
        send_to_char("You are a bit tougher than him.\n\r", ch);
    else if (conratio <= 2.0)
        send_to_char("You stand a very good chance.\n\r", ch);
    else if (conratio <= 10.0)
        send_to_char("You like picking on puny things?\n\r", ch);
    else if (conratio <= 50.0)
        send_to_char("Squash him like a gnat.\n\r", ch);
    else if (conratio <= 100.0)
        send_to_char("This guy should pose no problems to you.\n\r", ch);
    else
        send_to_char("Loose this fight, and you plain suck.\n\r", ch);
    return;
    */

    if (IS_IMMORTAL(ch))
    {
        send_to_char("Consider this, what the heck do you need con for?\n\r", ch);
        return;
    }

    diff = GetAveLevel(victim) - GetAveLevel(ch);
    diff += MobLevBonus(victim);
    diff += (int) ((float) (GET_DAMROLL(victim) - GET_DAMROLL(ch)) / 5.0);

    if (diff <= -10)
        send_to_char("Deceptively easy.\n\r", ch);
    else if (diff <= -5)
        send_to_char("Too easy to be believed.\n\r", ch);
    else if (diff <= -3)
        send_to_char("Not a problem.\n\r", ch);
    else if (diff <= -2)
        send_to_char("Rather easy.\n\r", ch);
    else if (diff <= -1)
        send_to_char("Easy.\n\r", ch);
    else if (diff <= 0)
        send_to_char("Fairly easy.\n\r", ch);
    else if (diff == 1)
        send_to_char("The perfect match!\n\r", ch);
    else if (diff <= 2)
        send_to_char("You would need some luck!\n\r", ch);
    else if (diff <= 3)
        send_to_char("You would need a lot of luck!\n\r", ch);
    else if (diff <= 5)
        send_to_char("You would need a lot of luck and great equipment!\n\r", ch);
    else if (diff <= 10)
        send_to_char("Do you feel lucky, punk?\n\r", ch);
    else if (diff <= 30)
        send_to_char("Are you crazy?  Is that your problem?\n\r", ch);
    else if (diff <= 40)
        send_to_char("You ARE mad!\n\r", ch);
    else if (diff <= 50)
        send_to_char("You have a death wish bub?\n\r", ch);
    else
        send_to_char("Why don't I just kill you right now and save you the trouble?\n\r", ch);

    if (IS_NPC(ch))
        return;

    if (IsAnimal(victim) && LEARNED(ch, gsn_animal_lore))
    {
        sn = gsn_animal_lore;
        learn = UMAX(learn, LEARNED(ch, sn));
        act(AT_CONSIDER, "$N seems to be an animal", ch, NULL, victim, TO_CHAR);
    }
    if (IsVeggie(victim) && LEARNED(ch, gsn_vegetable_lore))
    {
        sn = gsn_vegetable_lore;
        learn = UMAX(learn, LEARNED(ch, sn));
        act(AT_CONSIDER, "$N seems to be an ambulatory vegetable",
            ch, NULL, victim, TO_CHAR);
    }
    if (IsDiabolic(victim) && LEARNED(ch, gsn_demonology))
    {
        sn = gsn_demonology;
        learn = UMAX(learn, LEARNED(ch, sn));
        act(AT_CONSIDER, "$N seems to be a demon!", ch, NULL, victim, TO_CHAR);
    }
    if (IsReptile(victim) && LEARNED(ch, gsn_reptile_lore))
    {
        sn = gsn_reptile_lore;
        learn = UMAX(learn, LEARNED(ch, sn));
        act(AT_CONSIDER, "$N seems to be a reptilian creature",
            ch, NULL, victim, TO_CHAR);
    }
    if (IsUndead(victim) && LEARNED(ch, gsn_necromancy))
    {
        sn = gsn_necromancy;
        learn = UMAX(learn, LEARNED(ch, sn));
        act(AT_CONSIDER, "$N seems to be undead", ch, NULL, victim, TO_CHAR);
    }

    if (IsGiantish(victim) && LEARNED(ch, gsn_giant_lore))
    {
        sn = gsn_giant_lore;
        learn = UMAX(learn, LEARNED(ch, sn));
        act(AT_CONSIDER, "$N seems to be a giantish creature", ch, NULL, victim, TO_CHAR);
    }
    if (IsPerson(victim) && LEARNED(ch, gsn_people_lore))
    {
        sn = gsn_people_lore;
        learn = UMAX(learn, LEARNED(ch, sn));
        act(AT_CONSIDER, "$N seems to be a human or demi-human",
            ch, NULL, victim, TO_CHAR);
    }
    if (IsOther(victim) && LEARNED(ch, gsn_other_lore))
    {
        sn = gsn_other_lore;
        learn = UMAX(learn, LEARNED(ch, sn)/2);
        act(AT_CONSIDER, "$N seems to be a monster you know about",
            ch, NULL, victim, TO_CHAR);
    }

    if (learn < 10 || sn == 0)
        return;

    WAIT_STATE(ch, PULSE_VIOLENCE * 2);

    num1 = GET_MAX_HIT(victim) * GET_ADEPT(ch, sn) / (learn + number_fuzzy(1));
    num2 = GET_MAX_HIT(ch);
    fnum = num1 / num2;
    ch_printf(ch, "Estimated max hitpoints are: %s\n\r", DescRatio(fnum));

    if (learn > 40)
    {
        num1 = GET_AC(victim) * GET_ADEPT(ch, sn) / (learn + number_fuzzy(1));
        num2 = GET_AC(ch);
        if (num2 == 0)
            num2 = 1;
        diff = num1 - num2;
        if (diff < 0)
            diff *= -1;
        if (num1+diff == 0)
            fnum = 1.0;
        else
            fnum = (num2+diff) / (num1+diff);
        ch_printf(ch, "Estimated armor class is : %s\n\r", DescRatio(fnum));
    }

    if (learn > 50)
    {
        fnum = victim->numattacks;
        if (fnum > 20)
            fnum /= 1000;
        ch_printf(ch, "Estimated number of attacks: %s\n\r", DescAttacks(fnum));
    }

    if (learn > 60)
    {
        fnum = (victim->barenumdie*victim->baresizedie)+victim->damroll;
        ch_printf(ch, "Estimated damage of attacks is: %s\n\r",
                  DescDamage(fnum));
    }

    if (learn > 70)
    {
        num1 = GET_HITROLL(victim);
        num2 = ((int) 21 - CalcThaco(ch));
        if (num2 > 0)
            fnum = num1 / num2;
        else
            fnum = 2.0;
        ch_printf(ch, "Estimated Thaco: %s\n\r", DescRatio(fnum));
    }

    if (learn > 80)
    {
        num1 = GET_DAMROLL(victim);
        num2 = UMAX(1, GET_DAMROLL(ch));
        fnum = num1 / num2;

        ch_printf(ch, "Estimated damage bonus is: %s\n\r", DescRatio(fnum));
    }
}

#endif

/*
 * Place any skill types you don't want them to be able to practice
 * normally in this list.  Separate each with a space.
 * (Uses an is_name check). -- Altrag
 */
#define CANT_PRAC "Tongue"

void do_practice( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int sn = 0;
    CHAR_DATA *mob;
    int adept;
    sh_int ch_class = CLASS_MAGE;

    if ( IS_NPC(ch) )
        return;

    for ( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
        if ( IS_NPC(mob) && (IS_ACT_FLAG(mob, ACT_PRACTICE) ||
                             IS_ACT_FLAG(mob, ACT_TEACHER)) )
            break;

    if ( argument[0] == '\0' )
    {
        switch(GET_INTF(ch))
        {
        case INT_SMAUG: smaug_prac_output(ch, mob); break;
        default:        dale_prac_output(ch, mob);  break;
        }
        return;
    }

    if ( !mob ||
         (!IS_ACT_FLAG(mob, ACT_PRACTICE) && !IS_ACT_FLAG(mob, ACT_TEACHER)))
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    if ( ch->practice <= 0 )
    {
        act( AT_TELL, "$n tells you 'You must earn some more practice sessions.'",
             mob, NULL, ch, TO_VICT );
        return;
    }

    if ( (sn = skill_lookup( argument )) == -1 || sn == gsn_drinking)
    {
        act( AT_TELL, "$n tells you 'I can't teach you that...'",
             mob, NULL, ch, TO_VICT );
        return;
    }

    if ((ch_class = FirstActive(mob)) == CLASS_NONE)
    {
        act( AT_TELL, "$n tells you 'Hey tell an Implementor that I'm acting weird.'",
             mob, NULL, ch, TO_VICT );
        return;
    }

    if (!IS_ACT_FLAG(mob, ACT_TEACHER))
    {
	/* let mage gm's teach sorc pc's */
	if (ch_class == CLASS_MAGE && !IS_ACTIVE(ch, ch_class))
	    ch_class = CLASS_SORCERER;

	if (!IS_ACTIVE(ch, ch_class))
        {
	    act( AT_TELL, "$n tells you 'Your kind is not welcome around here!'",
		mob, NULL, ch, TO_VICT );
	    return;
        }
    }
    else
    {
        for (ch_class = FirstActive(ch); ch_class < MAX_CLASS; ch_class++)
            if (IS_ACTIVE(ch, ch_class) &&
                GET_LEVEL(ch, ch_class) >= skill_table[sn]->skill_level[ch_class])
                break;
    }

    if (!IS_ACTIVE(ch, ch_class) ||
        GET_LEVEL(ch,ch_class) < skill_table[sn]->skill_level[ch_class])
    {
        act( AT_TELL, "$n tells you 'You're not ready to learn that yet...'",
             mob, NULL, ch, TO_VICT );
        return;
    }

    /*
     * Skill requires a special teacher
     */
    if ( skill_table[sn]->teachers && skill_table[sn]->teachers[0] != '\0' )
    {
        sprintf( buf, "%d", mob->vnum );
        if ( !is_name( buf, skill_table[sn]->teachers ) )
        {
            act( AT_TELL, "$n tells you, 'You must find a specialist to learn that!'",
                 mob, NULL, ch, TO_VICT );
            return;
        }
    }
    else
    {
        if ( GetMaxLevel(mob) < (skill_table[sn]->skill_level[ch_class]+10) &&
             skill_table[sn]->skill_level[ch_class] > 0)
        {
            act( AT_TELL, "$n tells you 'You cannot learn that from me, you must find another...'",
                 mob, NULL, ch, TO_VICT );
            return;
        }

        if ( is_name( skill_tname[skill_table[sn]->type], CANT_PRAC ) )
        {
            act( AT_TELL, "$n tells you 'I do not know how to teach that.'",
                 mob, NULL, ch, TO_VICT );
            return;
        }
    }


    /*
     * Guild checks - right now, cant practice guild skills - done on
     * induct/outcast
     */
    /*
     if ( !IS_NPC(ch) && !IS_GUILDED(ch)
     &&    skill_table[sn]->guild != CLASS_NONE)
     {
     act( AT_TELL, "$n tells you 'Only guild members can use that..'"
     mob, NULL, ch, TO_VICT );
     return;
     }

     if ( !IS_NPC(ch) && skill_table[sn]->guild != CLASS_NONE
     && ch->pcdata->clan->ch_class != skill_table[sn]->guild )
     {
     act( AT_TELL, "$n tells you 'That I can not teach to your guild.'"
     mob, NULL, ch, TO_VICT );
     return;
     }

     if ( !IS_NPC(ch) && skill_table[sn]->guild != CLASS_NONE)
     {
     act( AT_TELL, "$n tells you 'That is only for members of guilds...'",
     mob, NULL, ch, TO_VICT );
     return;
     }
     */

    if ( skill_table[sn]->type == SKILL_TONGUE ||
	 skill_table[sn]->type == SKILL_LORE)
        adept = GET_ADEPT(ch, sn);
    else
        adept = (int)(GET_ADEPT(ch, sn) * 0.2 + (get_curr_int(ch)*2));
    /*adept = (int)(class_table[BestSkCl(ch, sn)]->skill_adept * 0.2 + (get_curr_int(ch)*2));*/

    if ( LEARNED(ch, sn) >= adept )
    {
        sprintf( buf, "$n tells you, 'I've taught you everything I can about %s.'",
                 skill_table[sn]->name );
        act( AT_TELL, buf, mob, NULL, ch, TO_VICT );
        act( AT_TELL, "$n tells you, 'You'll have to practice it on your own now...'",
             mob, NULL, ch, TO_VICT );
    }
    else
    {
        ch->practice--;
        ch->pcdata->learned[sn] += int_app[get_curr_int(ch)].learn;
        act( AT_ACTION, "You practice $T.",
             ch, NULL, skill_table[sn]->name, TO_CHAR );
        act( AT_ACTION, "$n practices $T.",
             ch, NULL, skill_table[sn]->name, TO_ROOM );
        if ( LEARNED(ch, sn) >= adept )
        {
            ch->pcdata->learned[sn] = adept;
            act( AT_TELL,
                 "$n tells you. 'You'll have to practice it on your own now...'",
                 mob, NULL, ch, TO_VICT );
        }
    }

    if (sn>=gsn_first_tongue && sn<=GSN_LAST_TONGUE)
        update_speaks(ch);

}


#define MAX_NEW_CLASSES 3

void do_new_ch_class( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *mob;
    sh_int ch_class = CLASS_NONE;
    int i;

    if (IS_NPC(ch) || IS_IMMORTAL(ch))
    {
        send_to_char("Not likely in your lifetime pal!\n\r", ch);
        return;
    }

    if (GET_RACE(ch) != RACE_HUMAN || IS_ACTIVE(ch, CLASS_AMAZON))
    {
        send_to_char("This path is not open to you.\n\r", ch);
        return;
    }

    for ( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
        if ( IS_NPC(mob) && IS_ACT_FLAG(mob, ACT_PRACTICE) )
            break;

    if (!mob || !IS_ACT_FLAG(mob, ACT_PRACTICE))
    {
        send_to_char("No one here can teach you a new trade.\n\r", ch);
        return;
    }

    if ((ch_class = FirstActive(mob)) == CLASS_NONE)
    {
        act( AT_TELL, "$n tells you, 'Hey tell an Implementor that I'm acting weird.'",
             mob, NULL, ch, TO_VICT );
        return;
    }

    if (HAS_CLASS(ch, ch_class))
    {
        act( AT_TELL, "$n tells you, 'You already hold knowledge of this guild.'",
             mob, NULL, ch, TO_VICT );
        return;
    }

    if (HowManyClassesPlus(ch) < 1)
    {
        act( AT_TELL, "$n tells you, 'You look a little ill, I think you should talk to an Implementorr.'",
             mob, NULL, ch, TO_VICT );
	bug("%s has no class in do_new_ch_class!", GET_NAME(ch));
        return;
    }

    if ((HowManyClassesPlus(ch) > MAX_NEW_CLASSES) || (GetMaxLevel(ch) > 50))
    {
        act( AT_TELL, "$n mumbles to the wall, '$E is too old, yes... too old to begin the training!'",
             mob, NULL, ch, TO_VICT );
        return;
    }

    if ((HowManyClasses(ch) > 1) || (RacialMax[GET_RACE(ch)][ch_class] < 1))
    {
        act( AT_TELL, "$n tells you, 'I don't teach your kind this.'",
             mob, NULL, ch, TO_VICT );
        return;
    }


    for (i = 0; i < MAX_CLASS; i++)
    {
        if (IS_ACTIVE(ch, i))
        {
            REMOVE_BIT(ch->classes[i], STAT_ACTCLASS);
            SET_BIT(ch->classes[i], STAT_OLDCLASS);
        }
    }

    sprintf(log_buf, "new_class: %s is now a %s", GET_NAME(ch), pc_class[ch_class]);
    log_string_plus( log_buf, LOG_MONITOR, LEVEL_LOG_CSET, SEV_NOTICE);

    do_remove(ch, "all");
    ch->classes[ch_class] = STAT_ACTCLASS;
    ch->levels[ch_class] = 1;
    ch->exp = 0;
    GET_PRACS(ch) = wis_app[get_curr_wis(ch)].practice;
    sprintf(buf, "the %s %s", get_race_name(ch), GetTitleString(ch));
    set_title(ch, buf);
    sprintf(buf, "%s %s", race_table[GET_RACE(ch)].race_name, GetClassString(ch));
    ch->pcdata->rank = str_dup(buf);
    ch->armor = 100;
    ch->barenumdie = 1;
    ch->baresizedie = 3;
    ch->numattacks = 1000;
    fix_char(ch);

}

void do_gain( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *mob;
    sh_int ch_class = CLASS_NONE;

    if ( IS_NPC(ch) || IS_IMMORTAL(ch))
    {
        send_to_char("Not likely in your lifetime pal!\n\r", ch);
        return;
    }

    for ( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
        if ( IS_NPC(mob) && IS_ACT_FLAG(mob, ACT_PRACTICE) )
            break;

    if (!mob || !IS_ACT_FLAG(mob, ACT_PRACTICE))
    {
        send_to_char("No one here can teach you.\n\r", ch);
        return;
    }

    if ((ch_class = FirstActive(mob)) == CLASS_NONE)
    {
        act( AT_TELL, "$n tells you, 'Hey tell a God that I'm acting weird.'",
             mob, NULL, ch, TO_VICT );
        return;
    }

    if (!IS_ACTIVE(ch, ch_class))
    {
        act( AT_TELL, "$n tells you, 'You hold no knowledge of this guild.'",
             mob, NULL, ch, TO_VICT );
        return;
    }

    if (HAD_CLASS(ch, ch_class) ||
        (GET_LEVEL(ch, ch_class) >GetMaxLevel(mob) - 10) )
    {
        act( AT_TELL, "$n tells you, 'You can learn no more from me.'",
             mob, NULL, ch, TO_VICT );
        return;
    }

    /* see the challenge rooms, rspecial.c */
    if (GET_LEVEL(ch, ch_class) >= 10 &&
        (ch_class == CLASS_MONK || ch_class == CLASS_DRUID))
    {
        act( AT_TELL, "$n tells you, 'You must fight another to increase your standing.'",
             mob, NULL, ch, TO_VICT );
        if (GET_LEVEL(ch, ch_class) == 10)
            send_to_char("Visit the challenge room, or ask a God for help.\n\r", ch);
        return;
    }

    if ( ch->exp < exp_level(ch, GET_LEVEL(ch, ch_class)+1, ch_class) )
    {
        act( AT_TELL, "$n tells you, 'You are not yet ready.'",
             mob, NULL, ch, TO_VICT );
        return;
    }

    if (!can_gain_level(ch, ch_class))
    {
        act( AT_TELL, "$n tells you, 'You are at the limits of your ability in this class.'",
             mob, NULL, ch, TO_VICT );
        return;
    }

    ch_printf( ch, "You have obtained a greater standing in the %s class!\n\r", pc_class[ch_class]);
    advance_level( ch, ch_class );
}

void do_wimpy( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
        wimpy = (int) GET_MAX_HIT(ch) / 5;
    else
        wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
        send_to_char( "Your courage exceeds your wisdom.\n\r", ch );
        return;
    }

    if ( wimpy+5 > GET_MAX_HIT(ch) )
    {
        send_to_char( "Such cowardice ill becomes you.\n\r", ch );
        return;
    }

    ch->wimpy	= wimpy;
    ch_printf( ch, "Wimpy set to %d hit points.\n\r", wimpy );
    return;
}



void do_password( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( IS_NPC(ch) )
        return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while ( isspace(*argument) )
        argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;

    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        *pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace(*argument) )
        argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;

    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        *pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Syntax: password <old> <new>.\n\r", ch );
        return;
    }

    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
        WAIT_STATE( ch, 10*PULSE_PER_SECOND );
        send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
        return;
    }

    if ( strlen(arg2) < 5 )
    {
        send_to_char(
                     "New password must be at least five characters long.\n\r", ch );
        return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
        if ( *p == '~' )
        {
            send_to_char(
                         "New password not acceptable, try again.\n\r", ch );
            return;
        }
    }

    DISPOSE( ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    if ( IS_SAVE_FLAG(sysdata, SV_PASSCHG) )
        save_char_obj( ch );
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_socials( CHAR_DATA *ch, char *argument )
{
    int iHash;
    int col = 0;
    SOCIALTYPE *social;

    set_pager_color( AT_PLAIN, ch );
    for ( iHash = 0; iHash < 27; iHash++ )
        for ( social = social_index[iHash]; social; social = social->next )
        {
            pager_printf( ch, "%-12s", social->name );
            if ( ++col % 6 == 0 )
                send_to_pager( "\n\r", ch );
        }

    if ( col % 6 != 0 )
        send_to_pager( "\n\r", ch );
    return;
}

void do_color( CHAR_DATA *ch, char *argument )
{
    char silent[MAX_INPUT_LENGTH];
    char color[MAX_INPUT_LENGTH], value[MAX_INPUT_LENGTH];
    int i, col=0;

    if ( argument[0] == '\0' )
    {
        for (i=0;i<MAX_COLOR_TYPE;i++)
        {
            set_pager_color( i, ch );
            pager_printf( ch, "%-12s", color_table[i] );
            if ( ++col % 6 == 0 )
                send_to_pager( "\n\r", ch );
        }
        if ( col % 6 != 0 )
            send_to_pager( "\n\r", ch );
        set_char_color(AT_PLAIN,ch);
        send_to_pager("\n\rTo configure color: color <name from list> <value>\n\r", ch );
        send_to_pager("To enable color: config +ansi\n\r", ch );
        return;
    }

    if ( !str_cmp( argument, "def" ) )
    {
        send_to_pager("Setting all colors to default.\n\r", ch);
        SetDefaultColor( ch );
        return;
    }

    argument=one_argument(argument,color);
    argument=one_argument(argument,value);
    argument=one_argument(argument,silent);


    if ( value[0] == '\0' )
    {
        send_to_pager("Choosing default color.\n\r", ch);
        col = DAT_GREY;
    }
    else
    {
        for (col=0;col<MAX_COLOR_TYPE;col++)
            if ( !str_cmp( value, color_table[col] ) )
                break;
        if ( str_cmp( value, color_table[col] ) )
        {
            send_to_char("Invalid value, type color for a list.\n\r", ch);
            return;
        }
    }

    for (i=0;i<MAX_COLOR_TYPE;i++)
        if ( !str_cmp( color, color_table[i] ) )
        {
            if ( col == ch->colors[i] )
            {
                if (!*silent)
                    send_to_pager( "No change.\n\r", ch );
                return;
            }
            if (!*silent) {
                set_pager_color( AT_PLAIN, ch );
                send_to_pager( "Setting '", ch );
                set_pager_color( i, ch );
                send_to_pager( color_table[i], ch );
                set_pager_color( AT_PLAIN, ch );
                send_to_pager( "' to '", ch );
                ch->colors[i] = col;
                set_pager_color( i, ch );
                send_to_pager( color_table[i], ch );
                set_pager_color( AT_PLAIN, ch );
                send_to_pager( "'.\n\r", ch );
            } else {
                ch->colors[i] = col;
            }
            return;
        }
    send_to_pager( "That is not a valid color, type color for a list.\n\r", ch );
}

void do_commands( CHAR_DATA *ch, char *argument )
{
    int col;
    bool found;
    int hash;
    CMDTYPE *command;

    col = 0;
    set_pager_color( AT_PLAIN, ch );
    if ( argument[0] == '\0' )
    {
        for ( hash = 0; hash < 126; hash++ )
            for ( command = command_hash[hash]; command; command = command->next )
                if ( command->level <  LEVEL_HERO
                     &&   command->level <= get_trust( ch )
                     &&  (command->name[0] != 'm'
                          &&   command->name[1] != 'p') )
                {
                    pager_printf( ch, "%s%-12s",
                                  get_help(ch, command->name)?"*":" ",
                                  command->name );
                    if ( ++col % 6 == 0 )
                        send_to_pager( "\n\r", ch );
                }
        if ( col % 6 != 0 )
            send_to_pager( "\n\r", ch );
    }
    else
    {
        found = FALSE;
        for ( hash = 0; hash < 126; hash++ )
            for ( command = command_hash[hash]; command; command = command->next )
                if ( command->level <  LEVEL_HERO
                     &&   command->level <= get_trust( ch )
                     &&  !str_prefix(argument, command->name)
                     &&  (command->name[0] != 'm'
                          &&   command->name[1] != 'p') )
                {
                    pager_printf( ch, "%s%-12s",
                                  get_help(ch, command->name)?"*":" ",
                                  command->name );
                    found = TRUE;
                    if ( ++col % 6 == 0 )
                        send_to_pager( "\n\r", ch );
                }

        if ( col % 6 != 0 )
            send_to_pager( "\n\r", ch );
        if ( !found )
            ch_printf( ch, "No command found under %s.\n\r", argument);
    }
    return;
}


void do_channels( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        if (IS_SILENCED(ch))
        {
            send_to_char( "You are silenced.\n\r", ch );
            return;
        }

        send_to_char( "Channels:\t", ch );

        if ( get_trust( ch ) > 2 && !NOT_AUTHED( ch ) )
        {
            send_to_char( !IS_SET(ch->deaf, CHANNEL_AUCTION)
                          ? "+AUCTION\t"
                          : "-auction\t",
                          ch );
        }

        send_to_char( !IS_SET( ch->deaf, CHANNEL_WARTALK )
                      ? "+WARTALK\t"
                      : "-wartalk\t",
                      ch );

        if ( IS_IMMORTAL(ch) )
        {
            send_to_char( !IS_SET(ch->deaf, CHANNEL_MONITOR)
                          ? "+MONITOR\t"
                          : "-monitor\t",
                          ch );

            send_to_char( !IS_SET(ch->deaf, CHANNEL_PRAY)
                          ? "+PRAY\t"
                          : "-pray\t",
                          ch );

            send_to_char( !IS_SET(ch->deaf, CHANNEL_NEWBIE)
                          ? "+NEWBIE\t"
                          : "-newbie\t",
                          ch );
        }

        send_to_char( !IS_SET(ch->deaf, CHANNEL_CHAT)
                      ? "+CHAT\t"
                      : "-chat\t",
                      ch );

        if ( !IS_NPC( ch ) && ch->pcdata->clan )
        {
            if ( ch->pcdata->clan->clan_type == CLAN_ORDER )
            {
                send_to_char( !IS_SET(ch->deaf, CHANNEL_ORDER )
                              ? "+ORDER\t"
                              : "-order\t",
                              ch );
            }
            else
                if ( ch->pcdata->clan->clan_type == CLAN_GUILD )
                {
                    send_to_char( !IS_SET(ch->deaf, CHANNEL_GUILD)
                                  ? "+GUILD\t"
                                  : "-guild\t",
                                  ch );
                }
                else
                {
                    send_to_char( !IS_SET(ch->deaf, CHANNEL_CLAN)
                                  ? "+CLAN\t"
                                  : "-clan\t",
                                  ch );
                }
        }

        if ( !IS_NPC( ch ) && ch->pcdata->council )
        {
            send_to_char( !IS_SET(ch->deaf, CHANNEL_COUNCIL)
                          ? "+COUNCIL\t"
                          : "-council\t",
                          ch );
        }

        send_to_char( !IS_SET(ch->deaf, CHANNEL_QUEST)
                      ? "+QUEST\t"
                      : "-quest\t",
                      ch );

        send_to_char( !IS_SET( ch->deaf, CHANNEL_TELLS )
                      ? "+TELLS\t"
                      : "-tells\t",
                      ch );

        if ( IS_HERO(ch) )
        {
            send_to_char( !IS_SET(ch->deaf, CHANNEL_AVTALK)
                          ? "+AVATAR\t"
                          : "-avatar\t",
                          ch );
        }

        if ( get_trust(ch) >= sysdata.muse_level )
        {
            send_to_char( !IS_SET(ch->deaf, CHANNEL_HIGHGOD)
                          ? "+MUSE\t"
                          : "-muse\t",
                          ch );
        }

        send_to_char( !IS_SET(ch->deaf, CHANNEL_MUSIC)
                      ? "+MUSIC\t"
                      : "-music\t",
                      ch );

        send_to_char( !IS_SET(ch->deaf, CHANNEL_ASK)
                      ? "+ASK\t"
                      : "-ask\t",
                      ch );

        send_to_char( !IS_SET(ch->deaf, CHANNEL_SHOUT)
                      ? "+SHOUT\t"
                      : "-shout\t",
                      ch );

        send_to_char( !IS_SET(ch->deaf, CHANNEL_OOC)
                      ? "+OOC\t"
                      : "-ooc\t",
                      ch );

        send_to_char( !IS_SET(ch->deaf, CHANNEL_GOSSIP)
                      ? "+GOSSIP\t"
                      : "-gossip\t",
                      ch );

        if ( get_trust(ch) >= sysdata.think_level )
        {
            send_to_char( !IS_SET(ch->deaf, CHANNEL_IMMTALK)
                          ? "+IMMTALK\t"
                          : "-immtalk\t",
                          ch );
        }

        if ( get_trust(ch) >= sysdata.log_level )
        {

            send_to_char( !IS_SET(ch->deaf, CHANNEL_LOG)
                          ? "+LOG\t"
                          : "-log\t",
                          ch );

            send_to_char( !IS_SET(ch->deaf, CHANNEL_BUILD)
                          ? "+BUILD\t"
                          : "-build\t",
                          ch );

            send_to_char( !IS_SET(ch->deaf, CHANNEL_COMM)
                          ? "+COMM\t"
                          : "-comm\t",
                          ch );

            send_to_char( !IS_SET(ch->deaf, CHANNEL_LOGPC)
                          ? "+LOGPC\t"
                          : "-logpc\t",
                          ch );

            send_to_char( !IS_SET(ch->deaf, CHANNEL_HTTPD)
                          ? "+HTTPD\t"
                          : "-httpd\t",
                          ch );

            send_to_char( !IS_SET(ch->deaf, CHANNEL_IMC)
                          ? "+IMC\t"
                          : "-imc\t",
                          ch );

            send_to_char( !IS_SET(ch->deaf, CHANNEL_IMCDEBUG)
                          ? "+IMCDEBUG\t"
                          : "-imcdebug\t",
                          ch );

            send_to_char( !IS_SET(ch->deaf, CHANNEL_IRC)
                          ? "+IRC\t"
                          : "-irc\t",
                          ch );

            send_to_char( !IS_SET(ch->deaf, CHANNEL_BUG)
                          ? "+BUG\t"
                          : "-bug\t",
                          ch );

            send_to_char( !IS_SET(ch->deaf, CHANNEL_DEBUG)
                          ? "+DEBUG\t"
                          : "-debug\t",
                          ch );

            send_to_char( !IS_SET(ch->deaf, CHANNEL_MAGIC)
                          ? "+MAGIC\t"
                          : "-magic\t",
                          ch );

        }
        send_to_char( ".\n\r", ch );
    }
    else
    {
        bool fClear;
        bool ClearAll;
        int bit;

        bit=0;
        ClearAll = FALSE;

        if ( arg[0] == '+' ) fClear = TRUE;
        else if ( arg[0] == '-' ) fClear = FALSE;
        else
        {
            send_to_char( "Channels -channel or +channel?\n\r", ch );
            return;
        }

        if ( !str_cmp( arg+1, "auction"  ) ) bit = CHANNEL_AUCTION;
        else if ( !str_cmp( arg+1, "chat"     ) ) bit = CHANNEL_CHAT;
        else if ( !str_cmp( arg+1, "clan"     ) ) bit = CHANNEL_CLAN;
        else if ( !str_cmp( arg+1, "council"  ) ) bit = CHANNEL_COUNCIL;
        else if ( !str_cmp( arg+1, "guild"    ) ) bit = CHANNEL_GUILD;
        else if ( !str_cmp( arg+1, "quest"    ) ) bit = CHANNEL_QUEST;
        else if ( !str_cmp( arg+1, "tells"    ) ) bit = CHANNEL_TELLS;
        else if ( !str_cmp( arg+1, "immtalk"  ) ) bit = CHANNEL_IMMTALK;
        else if ( !str_cmp( arg+1, "log"      ) ) bit = CHANNEL_LOG;
        else if ( !str_cmp( arg+1, "build"    ) ) bit = CHANNEL_BUILD;
        else if ( !str_cmp( arg+1, "pray"     ) ) bit = CHANNEL_PRAY;
        else if ( !str_cmp( arg+1, "avatar"   ) ) bit = CHANNEL_AVTALK;
        else if ( !str_cmp( arg+1, "monitor"  ) ) bit = CHANNEL_MONITOR;
        else if ( !str_cmp( arg+1, "bug"      ) ) bit = CHANNEL_BUG;
        else if ( !str_cmp( arg+1, "debug"    ) ) bit = CHANNEL_DEBUG;
        else if ( !str_cmp( arg+1, "magic"    ) ) bit = CHANNEL_MAGIC;
        else if ( !str_cmp( arg+1, "newbie"   ) ) bit = CHANNEL_NEWBIE;
        else if ( !str_cmp( arg+1, "music"    ) ) bit = CHANNEL_MUSIC;
        else if ( !str_cmp( arg+1, "muse"     ) ) bit = CHANNEL_HIGHGOD;
        else if ( !str_cmp( arg+1, "ask"      ) ) bit = CHANNEL_ASK;
        else if ( !str_cmp( arg+1, "shout"    ) ) bit = CHANNEL_SHOUT;
        else if ( !str_cmp( arg+1, "ooc"      ) ) bit = CHANNEL_OOC;
        else if ( !str_cmp( arg+1, "gossip"   ) ) bit = CHANNEL_GOSSIP;
        else if ( !str_cmp( arg+1, "comm"     ) ) bit = CHANNEL_COMM;
        else if ( !str_cmp( arg+1, "order"    ) ) bit = CHANNEL_ORDER;
        else if ( !str_cmp( arg+1, "wartalk"  ) ) bit = CHANNEL_WARTALK;
        else if ( !str_cmp( arg+1, "logpc"    ) ) bit = CHANNEL_LOGPC;
        else if ( !str_cmp( arg+1, "httpd"    ) ) bit = CHANNEL_HTTPD;
        else if ( !str_cmp( arg+1, "imc"      ) ) bit = CHANNEL_IMC;
        else if ( !str_cmp( arg+1, "imcdebug" ) ) bit = CHANNEL_IMCDEBUG;
        else if ( !str_cmp( arg+1, "irc"      ) ) bit = CHANNEL_IRC;
        else if ( !str_cmp( arg+1, "all"      ) ) ClearAll = TRUE;
        else
        {
            send_to_char( "Set or clear which channel?\n\r", ch );
            return;
        }

        if (( fClear ) && ( ClearAll ))
        {
            REMOVE_BIT (ch->deaf, CHANNEL_AUCTION);
            REMOVE_BIT (ch->deaf, CHANNEL_CHAT);
            REMOVE_BIT (ch->deaf, CHANNEL_QUEST);
            /*     REMOVE_BIT (ch->deaf, CHANNEL_IMMTALK); */
            REMOVE_BIT (ch->deaf, CHANNEL_PRAY);
            REMOVE_BIT (ch->deaf, CHANNEL_MUSIC);
            REMOVE_BIT (ch->deaf, CHANNEL_ASK);
            REMOVE_BIT (ch->deaf, CHANNEL_SHOUT);
            REMOVE_BIT (ch->deaf, CHANNEL_OOC);
            REMOVE_BIT (ch->deaf, CHANNEL_GOSSIP);
            REMOVE_BIT (ch->deaf, CHANNEL_BUG);
            REMOVE_BIT (ch->deaf, CHANNEL_DEBUG);
            REMOVE_BIT (ch->deaf, CHANNEL_MAGIC);

            /*     if (ch->pcdata->clan)
             REMOVE_BIT (ch->deaf, CHANNEL_CLAN);

             if (ch->pcdata->council)
             REMOVE_BIT (ch->deaf, CHANNEL_COUNCIL);

             if (ch->pcdata->guild)
             REMOVE_BIT (ch->deaf, CHANNEL_GUILD);
             */
            if (GetMaxLevel(ch) >= LEVEL_IMMORTAL)
                REMOVE_BIT (ch->deaf, CHANNEL_AVTALK);

            if (GetMaxLevel(ch) >= sysdata.log_level )
                REMOVE_BIT (ch->deaf, CHANNEL_COMM);

        } else if ((!fClear) && (ClearAll))
        {
            SET_BIT (ch->deaf, CHANNEL_AUCTION);
            SET_BIT (ch->deaf, CHANNEL_CHAT);
            SET_BIT (ch->deaf, CHANNEL_QUEST);
            /*     SET_BIT (ch->deaf, CHANNEL_IMMTALK); */
            SET_BIT (ch->deaf, CHANNEL_PRAY);
            SET_BIT (ch->deaf, CHANNEL_MUSIC);
            SET_BIT (ch->deaf, CHANNEL_ASK);
            SET_BIT (ch->deaf, CHANNEL_SHOUT);
            SET_BIT (ch->deaf, CHANNEL_OOC);
            SET_BIT (ch->deaf, CHANNEL_GOSSIP);
            SET_BIT (ch->deaf, CHANNEL_BUG);
            SET_BIT (ch->deaf, CHANNEL_DEBUG);
            SET_BIT (ch->deaf, CHANNEL_MAGIC);

            /*     if (ch->pcdata->clan)
             SET_BIT (ch->deaf, CHANNEL_CLAN);

             if (ch->pcdata->council)
             SET_BIT (ch->deaf, CHANNEL_COUNCIL);

             if ( IS_GUILDED(ch) )
             SET_BIT (ch->deaf, CHANNEL_GUILD);
             */
            if (GetMaxLevel(ch) >= LEVEL_IMMORTAL)
            {
                SET_BIT (ch->deaf, CHANNEL_AVTALK);
                SET_BIT (ch->deaf, CHANNEL_LOGPC);
                SET_BIT (ch->deaf, CHANNEL_HTTPD);
                SET_BIT (ch->deaf, CHANNEL_IMC);
                SET_BIT (ch->deaf, CHANNEL_IMCDEBUG);
                SET_BIT (ch->deaf, CHANNEL_IRC);
            }

            if (GetMaxLevel(ch) >= sysdata.log_level)
                SET_BIT (ch->deaf, CHANNEL_COMM);

        } else if (fClear)
        {
            REMOVE_BIT (ch->deaf, bit);
        } else
        {
            SET_BIT    (ch->deaf, bit);
        }

        send_to_char( "Ok.\n\r", ch );
    }

    return;
}


/*
 * display WIZLIST file						-Thoric
 */
void do_wizlist( CHAR_DATA *ch, char *argument )
{
    set_pager_color( AT_IMMORT, ch );
    show_file( ch, WIZLIST_FILE );
}

/*
 * Contributed by Grodyn.
 */
void do_config( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    if ( IS_NPC(ch) )
        return;

    one_argument( argument, arg );

    set_char_color( AT_PLAIN, ch );
    if ( arg[0] == '\0' )
    {
        send_to_char( "[ Keyword  ] Option\n\r", ch );

        send_to_char(  IS_PC_FLAG(ch, PCFLAG_DEADLY)
                       ? "[          ] You are a player killer.\n\r"
                       : "[-deadly   ] You are not a player killer.\n\r"
                       , ch );

        if ( !IS_PC_FLAG( ch, PCFLAG_DEADLY ) )
        {
            send_to_char(  IS_PLR_FLAG(ch, PLR_NICE)
                           ? "[+NICE     ] You are nice to other players.\n\r"
                           : "[-nice     ] You are not nice to other players.\n\r"
                           , ch );

            send_to_char(  IS_PLR_FLAG(ch, PLR_FLEE)
                           ? "[+FLEE     ] You flee if you get attacked.\n\r"
                           : "[-flee     ] You fight back if you get attacked.\n\r"
                           , ch );
        }

        send_to_char(  IS_PC_FLAG(ch, PCFLAG_NORECALL)
                       ? "[+NORECALL ] You fight to the death, link-dead or not.\n\r"
                       : "[-norecall ] You try to recall if fighting link-dead.\n\r"
                       , ch );

        send_to_char(  IS_PLR_FLAG(ch, PLR_AUTOEXIT)
                       ? "[+AUTOEXIT ] You automatically see exits.\n\r"
                       : "[-autoexit ] You don't automatically see exits.\n\r"
                       , ch );

        send_to_char(  IS_PLR_FLAG(ch, PLR_AUTOLOOT)
                       ? "[+AUTOLOOT ] You automatically loot corpses.\n\r"
                       : "[-autoloot ] You don't automatically loot corpses.\n\r"
                       , ch );

        send_to_char(  IS_PLR_FLAG(ch, PLR_AUTOSAC)
                       ? "[+AUTOSAC  ] You automatically sacrifice corpses.\n\r"
                       : "[-autosac  ] You don't automatically sacrifice corpses.\n\r"
                       , ch );

        send_to_char(  IS_PLR_FLAG(ch, PLR_AUTOGOLD)
                       ? "[+AUTOGOLD ] You automatically split gold from kills in groups.\n\r"
                       : "[-autogold ] You don't automatically split gold from kills in groups.\n\r"
                       , ch );

#ifdef PLR2_AUTOGAIN
        send_to_char(  IS_PLR2_FLAG(ch, PLR2_AUTOGAIN)
                       ? "[+AUTOGAIN ] You automatically gain upon reaching enough exp for level.\n\r"
                       : "[-autogain ] You don't automatically gain levels.\n\r"
                       , ch );
#endif

        send_to_char(  IS_PC_FLAG(ch, PCFLAG_GAG)
                       ? "[+GAG      ] You see only necessary battle text.\n\r"
                       : "[-gag      ] You see full battle text.\n\r"
                       , ch );

        send_to_char(  IS_PC_FLAG(ch, PCFLAG_PAGERON)
                       ? "[+PAGER    ] Long output is page-paused.\n\r"
                       : "[-pager    ] Long output scrolls to the end.\n\r"
                       , ch );

#ifdef MXP
        send_to_char(  IS_PLR2_FLAG(ch, PLR2_MXP)
                       ? "[+MXP      ] MXP enabled\n\r"
                       : "[-mxp      ] MXP disabled.\n\r"
                       , ch );
#endif

#ifdef MSP
        send_to_char(  IS_PLR2_FLAG(ch, PLR2_MSP)
                       ? "[+MSP      ] MSP enabled\n\r"
                       : "[-msp      ] MSP disabled.\n\r"
                       , ch );
#endif

        send_to_char(  IS_PLR_FLAG(ch, PLR_BLANK)
                       ? "[+BLANK    ] You have a blank line before your prompt.\n\r"
                       : "[-blank    ] You have no blank line before your prompt.\n\r"
                       , ch );

        send_to_char(  IS_PLR_FLAG(ch, PLR_BRIEF)
                       ? "[+BRIEF    ] You see brief descriptions.\n\r"
                       : "[-brief    ] You see long descriptions.\n\r"
                       , ch );

        send_to_char(  IS_PLR_FLAG(ch, PLR_COMBINE)
                       ? "[+COMBINE  ] You see object lists in combined format.\n\r"
                       : "[-combine  ] You see object lists in single format.\n\r"
                       , ch );

        send_to_char(  IS_PC_FLAG(ch, PCFLAG_NOINTRO)
                       ? "[+NOINTRO  ] You don't see the ascii intro screen on login.\n\r"
                       : "[-nointro  ] You see the ascii intro screen on login.\n\r"
                       , ch );

        send_to_char(  IS_PLR_FLAG(ch, PLR_PROMPT)
                       ? "[+PROMPT   ] You have a prompt.\n\r"
                       : "[-prompt   ] You don't have a prompt.\n\r"
                       , ch );

        send_to_char(  IS_PLR_FLAG(ch, PLR_TELNET_GA)
                       ? "[+TELNETGA ] You receive a telnet GA sequence.\n\r"
                       : "[-telnetga ] You don't receive a telnet GA sequence.\n\r"
                       , ch );

        send_to_char(  IS_PLR_FLAG(ch, PLR_ANSI)
                       ? "[+ANSI     ] You receive ANSI color sequences.\n\r"
                       : "[-ansi     ] You don't receive receive ANSI colors.\n\r"
                       , ch );

        send_to_char(  IS_PLR_FLAG(ch, PLR_RIP)
                       ? "[+RIP      ] You receive RIP graphic sequences.\n\r"
                       : "[-rip      ] You don't receive RIP graphics.\n\r"
                       , ch );

        if ( !IS_PC_FLAG( ch, PCFLAG_DEADLY ) )
        {
            send_to_char(  IS_PLR_FLAG(ch, PLR_SHOVEDRAG)
                           ? "[+SHOVEDRAG] You allow yourself to be shoved and dragged around.\n\r"
                           : "[-shovedrag] You'd rather not be shoved or dragged around.\n\r"
                           , ch );

            send_to_char(  IS_PC_FLAG( ch, PCFLAG_NOSUMMON )
                           ? "[+NOSUMMON ] You do not allow other players to summon you.\n\r"
                           : "[-nosummon ] You allow other players to summon you.\n\r"
                           , ch );
        }

        if ( IS_IMMORTAL( ch ) )
            send_to_char(  IS_PLR_FLAG(ch, PLR_ROOMVNUM)
                           ? "[+VNUM     ] You can see the VNUM of a room.\n\r"
                           : "[-vnum     ] You do not see the VNUM of a room.\n\r"
                           , ch );

        if ( IS_IMMORTAL( ch ) )
            send_to_char(  IS_PLR_FLAG(ch, PLR_AUTOMAP)    /* maps */
                           ? "[+MAP      ] You can see the MAP of a room.\n\r"
                           : "[-map      ] You do not see the MAP of a room.\n\r"
                           , ch );

        send_to_char(  IS_PLR2_FLAG(ch, PLR2_AUTOASSIST)
                       ? "[+AUTOASSIS] You autoassist your group.\n\r"
                       : "[-autoassis] You don't autoassist your group.\n\r"
                       , ch );

        send_to_char(  IS_PLR2_FLAG(ch, PLR2_AFK_BUFFER)
                       ? "[+AFKBUFFER] AFK holds data in a buffer.\n\r"
                       : "[-afkbuffer] AFK does NOT hold data in a buffer.\n\r"
                       , ch );

        if (IS_IMMORTAL(ch))
            send_to_char(  IS_PLR2_FLAG(ch, PLR2_MONI_AFK)
                           ? "[+MONIAFK  ] You see others go AFK.\n\r"
                           : "[-moniafk  ] You do not see others go AFK.\n\r"
                           , ch );

        send_to_char(  IS_PLR_FLAG(ch, PLR_SILENCE)
                       ? "[+SILENCE  ] You are silenced.\n\r"
                       : ""
                       , ch );

        send_to_char( !IS_PLR_FLAG(ch, PLR_NO_EMOTE)
                      ? ""
                      : "[-emote    ] You can't emote.\n\r"
                      , ch );

        send_to_char( !IS_PLR_FLAG(ch, PLR_NO_TELL)
                      ? ""
                      : "[-tell     ] You can't use 'tell'.\n\r"
                      , ch );

        send_to_char( !IS_PLR_FLAG(ch, PLR_LITTERBUG)
                      ? ""
                      : "[-litter  ] A convicted litterbug. You cannot drop anything.\n\r"
                      , ch );
    }
    else
    {
        bool fSet;
        int bit = 0;

        if ( arg[0] == '+' ) fSet = TRUE;
        else if ( arg[0] == '-' ) fSet = FALSE;
        else
        {
            send_to_char( "Config -option or +option?\n\r", ch );
            return;
        }

        if ( !str_prefix( arg+1, "autoexit" ) ) bit = PLR_AUTOEXIT;
        else if ( !str_prefix( arg+1, "autoloot" ) ) bit = PLR_AUTOLOOT;
        else if ( !str_prefix( arg+1, "autosac"  ) ) bit = PLR_AUTOSAC;
        else if ( !str_prefix( arg+1, "autogold" ) ) bit = PLR_AUTOGOLD;
        else if ( !str_prefix( arg+1, "blank"    ) ) bit = PLR_BLANK;
        else if ( !str_prefix( arg+1, "brief"    ) ) bit = PLR_BRIEF;
        else if ( !str_prefix( arg+1, "combine"  ) ) bit = PLR_COMBINE;
        else if ( !str_prefix( arg+1, "prompt"   ) ) bit = PLR_PROMPT;
        else if ( !str_prefix( arg+1, "telnetga" ) ) bit = PLR_TELNET_GA;
        else if ( !str_prefix( arg+1, "ansi"     ) ) bit = PLR_ANSI;
        else if ( !str_prefix( arg+1, "rip"      ) ) bit = PLR_RIP;
        else if ( !str_prefix( arg+1, "flee"     ) ) bit = PLR_FLEE;
        else if ( !str_prefix( arg+1, "nice"     ) ) bit = PLR_NICE;
        else if ( !str_prefix( arg+1, "shovedrag") ) bit = PLR_SHOVEDRAG;
        else if ( IS_IMMORTAL( ch )
                  &&   !str_prefix( arg+1, "vnum"     ) ) bit = PLR_ROOMVNUM;
        else if ( IS_IMMORTAL( ch )
                  &&   !str_prefix( arg+1, "map"      ) ) bit = PLR_AUTOMAP;     /* maps */

        if (bit)
        {
            if ( (bit == PLR_FLEE || bit == PLR_NICE || bit == PLR_SHOVEDRAG)
                 &&  IS_PC_FLAG( ch, PCFLAG_DEADLY ) )
            {
                send_to_char( "Pkill characters can not config that option.\n\r", ch );
                return;
            }

            if ( fSet )
                SET_PLR_FLAG (ch, bit);
            else
                REMOVE_PLR_FLAG (ch, bit);
            send_to_char( "Ok.\n\r", ch );
            return;
        }
        else
        {
            if ( !str_prefix( arg+1, "norecall" ) ) bit = PCFLAG_NORECALL;
            else if ( !str_prefix( arg+1, "nointro"  ) ) bit = PCFLAG_NOINTRO;
            else if ( !str_prefix( arg+1, "nosummon" ) ) bit = PCFLAG_NOSUMMON;
            else if ( !str_prefix( arg+1, "gag"      ) ) bit = PCFLAG_GAG;
            else if ( !str_prefix( arg+1, "pager"    ) ) bit = PCFLAG_PAGERON;
            else if ( !str_prefix( arg+1, "deadly"   ) ) bit = PCFLAG_DEADLY;
            else
            {
                if ( !str_prefix( arg+1, "autoassis" ) ) bit = PLR2_AUTOASSIST;
                else if ( !str_prefix( arg+1, "afkbuffer" ) ) bit = PLR2_AFK_BUFFER;
#ifdef PLR2_AUTOGAIN
                else if ( !str_prefix( arg+1, "autogain" ) ) bit = PLR2_AUTOGAIN;
#endif
                else if ( !str_prefix( arg+1, "mxp"  ) ) bit = PLR2_MXP;
                else if ( !str_prefix( arg+1, "msp"  ) ) bit = PLR2_MSP;
                else if ( IS_IMMORTAL( ch )
                          &&   !str_prefix( arg+1, "moniafk"    ) ) bit = PLR2_MONI_AFK;

                if ( fSet )
                    SET_PLR2_FLAG (ch, bit);
                else
                    REMOVE_PLR2_FLAG (ch, bit);

                send_to_char( "Ok.\n\r", ch );
                return;
            }

            if ( bit == PCFLAG_NOSUMMON && IS_PC_FLAG( ch, PCFLAG_DEADLY ) )
            {
                send_to_char( "Pkill characters can not config that option.\n\r", ch );
                return;
            }

            if ( fSet )
                SET_PC_FLAG (ch, bit);
            else if ( bit != PCFLAG_DEADLY )
                REMOVE_PC_FLAG (ch, bit);

            send_to_char( "Ok.\n\r", ch );
            return;
        }
    }

    return;
}


void do_credits( CHAR_DATA *ch, char *argument )
{
    do_help( ch, "credits" );
}


extern int top_area;

/*
 void do_areas( CHAR_DATA *ch, char *argument )
 {
 AREA_DATA *pArea1;
 AREA_DATA *pArea2;
 int iArea;
 int iAreaHalf;

 iAreaHalf = (top_area + 1) / 2;
 pArea1    = first_area;
 pArea2    = first_area;
 for ( iArea = 0; iArea < iAreaHalf; iArea++ )
 pArea2 = pArea2->next;

 for ( iArea = 0; iArea < iAreaHalf; iArea++ )
 {
 ch_printf( ch, "%-39s%-39s\n\r",
 pArea1->name, pArea2 ? pArea2->name : "" );
 pArea1 = pArea1->next;
 if ( pArea2 )
 pArea2 = pArea2->next;
 }

 return;
 }
 */

/*
 * New do_areas with soft/hard level ranges
 */

void do_areas( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;

    set_pager_color( AT_PLAIN, ch );
    send_to_pager("\n\r   Author    |             Area                     | Recommended |  Enforced\n\r", ch);
    send_to_pager("-------------+--------------------------------------+-------------+-----------\n\r", ch);

    for ( pArea = first_area; pArea; pArea = pArea->next )
    {
        if (argument && *argument=='_' && GetMaxLevel(ch)==LEVEL_SUPREME)
        {
            do_foldarea(ch,pArea->name);
        }
        else
            pager_printf(ch, "%-12s | %-36s | %4d - %-4d | %3d - %-3d \n\r",
                         pArea->author, pArea->name?pArea->name:"(null)",
                         pArea->low_soft_range, pArea->hi_soft_range,
                         pArea->low_hard_range, pArea->hi_hard_range);
    }
    return;
}

#ifdef USE_CRBS
void do_cafk( CHAR_DATA *ch, char *argument );
#endif

void do_afk( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
        return;

    if (IS_PLR_FLAG(ch, PLR_AFK))
    {
        REMOVE_PLR_FLAG(ch, PLR_AFK);
        send_to_char( "You are no longer afk.\n\r", ch );
        act(AT_GREY,"$n is no longer afk.", ch, NULL, NULL, TO_ROOM);
#ifdef USE_CRBS
        do_cafk(ch, "off"); /* CRBS afk */
#endif
        return;
    }

    SET_PLR_FLAG(ch, PLR_AFK);
    send_to_char( "You are now afk.\n\r", ch );
    act(AT_GREY,"$n is now afk.", ch, NULL, NULL, TO_ROOM);
#ifdef USE_CRBS
    do_cafk(ch, "on"); /* CRBS afk */
#endif
}

void do_busy( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
        return;

    if IS_PLR2_FLAG(ch, PLR2_BUSY)
    {
        REMOVE_PLR2_FLAG(ch, PLR2_BUSY);
        send_to_char( "You are no longer busy.\n\r", ch );
        act(AT_GREY,"$n is no longer busy.", ch, NULL, NULL, TO_ROOM);
    }
    else
    {
        SET_PLR_FLAG(ch, PLR2_BUSY);
        send_to_char( "You are now busy.\n\r", ch );
        act(AT_GREY,"$n is now busy.", ch, NULL, NULL, TO_ROOM);
        return;
    }

}

void do_slist( CHAR_DATA *ch, char *argument )
{
    int sn, i, lFound;
    char skn[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int lowlev, hilev;
    sh_int ch_class;
    sh_int lasttype = SKILL_SPELL;

    if ( IS_NPC(ch) )
        return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    ch_class = CLASS_NONE;
    lowlev=1;
    hilev=50;

    if (arg1[0]!='\0')
        ch_class=get_classtype(arg1);

    if (arg2[0]!='\0')
        lowlev=atoi(arg2);

    if (arg3[0]!='\0')
        hilev=atoi(arg3);

    if (ch_class == CLASS_NONE)
        ch_class = FirstActive(ch);

    if ((lowlev<1) || (lowlev>LEVEL_IMMORTAL))
        lowlev=1;

    if ((hilev<0) || (hilev>=LEVEL_IMMORTAL))
        hilev=LEVEL_HERO;

    if(lowlev>hilev)
        lowlev=hilev;

    set_pager_color( AT_MAGIC, ch );
    send_to_pager("SPELL & SKILL LIST\n\r",ch);
    send_to_pager("------------------\n\r",ch);

    for (i=lowlev; i <= hilev; i++)
    {
        lFound= 0;
        sprintf(skn,"Spell");
        for ( sn = 0; sn < top_sn; sn++ )
        {
            if ( !skill_table[sn]->name || skill_table[sn]->type == SKILL_TONGUE ||
                 (!skill_table[sn]->skill_fun && !skill_table[sn]->spell_fun))
                continue;

            if (sn == gsn_drinking)
                continue;

            if ( skill_table[sn]->type != lasttype )
            {
                lasttype = skill_table[sn]->type;
                strcpy( skn, skill_tname[lasttype] );
            }

            if ( LEARNED(ch, sn) == 0
                 &&   SPELL_FLAG(skill_table[sn], SF_SECRETSKILL) )
                continue;

            if(i==skill_table[sn]->skill_level[ch_class]  )
            {
                if( !lFound )
                {
                    lFound=1;
                    pager_printf( ch, "Level %d\n\r", i );
                }
                pager_printf(ch, "%7s: %30.30s  Current: %-10.10s",
                             skn, skill_table[sn]->name,
                             how_good(LEARNED(ch, sn)));
                pager_printf(ch, "  Max: %-10.10s\n\r",
                             how_good(GET_ADEPT(ch, sn)) );
            }
        }
    }
    return;
}


void do_whois( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    buf[0] = '\0';

    if(IS_NPC(ch))
        return;

    if(argument[0] == '\0')
    {
        send_to_char("You must input the name of a player online.\n\r", ch);
        return;
    }

    strcat(buf, "0.");
    strcat(buf, argument);
    if( ( ( victim = get_char_world(ch, buf) ) == NULL ))
    {
        send_to_char("No such player online.\n\r", ch);
        return;
    }

    if(IS_NPC(victim))
    {
        send_to_char("That's not a player!\n\r", ch);
        return;
    }

    ch_printf(ch, "%s is a %s %s.\n\rLevel: %s\n\rClass: %s\n\r",
              victim->name,
              race_table[GET_RACE(victim)].race_name,
              victim->sex == SEX_MALE ? "Male" :
              victim->sex == SEX_FEMALE ? "Female" : "Neutral",
              GetLevelString(victim),
              GetClassString(victim));

    ch_printf(ch, "%s is a %sdeadly player",
              victim->sex == SEX_MALE ? "He" :
              victim->sex == SEX_FEMALE ? "She" : "It",
              IS_PC_FLAG(victim, PCFLAG_DEADLY) ? "" : "non-");

    if ( victim->pcdata->clan )
    {
        if ( victim->pcdata->clan->clan_type == CLAN_ORDER )
            send_to_char( ", and belongs to the Order ", ch );
        else
            if ( victim->pcdata->clan->clan_type == CLAN_GUILD )
                send_to_char( ", and belongs to the ", ch );
            else
                send_to_char( ", and belongs to Clan ", ch );
        send_to_char( victim->pcdata->clan->name, ch );
    }
    send_to_char( ".\n\r", ch );

    if(victim->pcdata->council)
        ch_printf(ch, "%s belongs to the %s.\n\r",
                  victim->name,
                  victim->pcdata->council->name);

    if(victim->pcdata->deity)
        ch_printf(ch, "%s has found succor in the deity %s.\n\r",
                  victim->name,
                  victim->pcdata->deity->name);

    if(victim->pcdata->homepage && victim->pcdata->homepage[0] != '\0')
        ch_printf(ch, "%s's homepage can be found at %s.\n\r",
                  victim->name,
                  victim->pcdata->homepage);

    if(victim->pcdata->bio && victim->pcdata->bio[0] != '\0')
        ch_printf(ch, "%s's personal bio:\n\r%s",
                  victim->name,
                  victim->pcdata->bio);

    if(IS_IMMORTAL(ch))
    {
        send_to_char("----------------------------------------------------\n\r", ch);

        send_to_char("Info for immortals:\n\r", ch);

        ch_printf(ch, "%s is in room %d.\n\r",
                  victim->name, victim->in_room->vnum);

        if ( victim->pcdata->authed_by && victim->pcdata->authed_by[0] != '\0' )
            ch_printf(ch, "%s was authorized by %s.\n\r",
                      victim->name, victim->pcdata->authed_by);

        ch_printf(ch, "%s has killed %d mobiles, and been killed by a mobile %d times.\n\r",
                  victim->name, victim->pcdata->mkills, victim->pcdata->mdeaths );
        if ( victim->pcdata->pkills || victim->pcdata->pdeaths )
            ch_printf(ch, "%s has killed %d players, and been killed by a player %d times.\n\r",
                      victim->name, victim->pcdata->pkills, victim->pcdata->pdeaths );
        if ( victim->pcdata->illegal_pk )
            ch_printf(ch, "%s has committed %d illegal player kills.\n\r",
                      victim->name, victim->pcdata->illegal_pk );

        ch_printf(ch, "%s is %shelled at the moment.\n\r",
                  victim->name,
                  (victim->pcdata->release_date == 0) ? "not " : "");

        if(victim->pcdata->release_date != 0)
            ch_printf(ch, "%s was helled by %s, and will be released on %24.24s.\n\r",
                      victim->sex == SEX_MALE ? "He" :
                      victim->sex == SEX_FEMALE ? "She" : "It",
                      victim->pcdata->helled_by,
                      ctime(&victim->pcdata->release_date));

        if(get_trust(victim) < get_trust(ch))
        {
            sprintf(buf2, "list %s", buf);
            do_comment(ch, buf2);
        }

        if(IS_PLR_FLAG(victim, PLR_SILENCE) || IS_PLR_FLAG(victim, PLR_NO_EMOTE)
           || IS_PLR_FLAG(victim, PLR_NO_TELL) || IS_PLR_FLAG(victim, PLR_THIEF)
           || IS_PLR_FLAG(victim, PLR_KILLER) )
        {
            sprintf(buf2, "This player has the following flags set:");
            if(IS_PLR_FLAG(victim, PLR_SILENCE))
                strcat(buf2, " silence");
            if(IS_PLR_FLAG(victim, PLR_NO_EMOTE))
                strcat(buf2, " noemote");
            if(IS_PLR_FLAG(victim, PLR_NO_TELL) )
                strcat(buf2, " notell");
            if(IS_PLR_FLAG(victim, PLR_THIEF) )
                strcat(buf2, " thief");
            if(IS_PLR_FLAG(victim, PLR_KILLER) )
                strcat(buf2, " killer");
            strcat(buf2, ".\n\r");
            send_to_char(buf2, ch);
        }
        if ( victim->desc && victim->desc->host[0]!='\0' )   /* added by Gorog */
        {
            sprintf (buf2, "%s's IP info: %s ", victim->name, victim->desc->host);
            if (get_trust(ch) >= LEVEL_GOD)
                strcat (buf2, victim->desc->user);
            strcat (buf2, "\n\r");
            send_to_char(buf2, ch);
        }
    }
}

void do_pager( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    if ( IS_NPC(ch) )
        return;
    argument = one_argument(argument, arg);
    if ( !*arg )
    {
        if ( IS_PC_FLAG(ch, PCFLAG_PAGERON) )
            do_config(ch, "-pager");
        else
            do_config(ch, "+pager");
        return;
    }
    if ( !is_number(arg) )
    {
        send_to_char( "Set page pausing to how many lines?\n\r", ch );
        return;
    }
    ch->pcdata->pagerlen = atoi(arg);
    if ( ch->pcdata->pagerlen < 5 )
        ch->pcdata->pagerlen = 5;
    ch_printf( ch, "Page pausing set to %d lines.\n\r", ch->pcdata->pagerlen );
    return;
}

char *ArmorDesc(int a)
{
    if (a >= 100) {
        return("Buck naked");
    } else if (a >= 90) {
        return("Not armored");
    } else if (a >= 70) {
        return("Barely armored");
    } else if (a >= 50) {
        return("Lightly armored");
    } else if (a >= 30) {
        return("Medium-armored");
    } else if (a >= 10) {
        return("Fairly well armored");
    } else if (a >= -10) {
        return("Well armored");
    } else if (a >= -30) {
        return("Quite well armored");
    } else if (a >= -50) {
        return("Armored like a Monk");
    } else if (a >= -70) {
        return("Very well armored");
    } else if (a >= -90) {
        return("Extremely well armored");
    } else if (a >= -150) {
        return("Armored like a Dragon");
    } else if (a >= -200) {
        return("Armored like a Tank");
    } else if (a >= -300) {
        return("Armored like a God");
    } else if (a >= -400) {
        return("Armored like an Immortal");
    }
    return("Impossible!");
}

char *RollDesc(int a)
{
    if (a < -5) {
        return("Quite bad");
    } else if (a < -1) {
        return("Pretty lousy");
    } else if (a <= 1) {
        return("Not Much");
    } else if (a < 3) {
        return("Not bad");
    } else if (a < 8) {
        return("Pretty good");
    } else if (a < 10) {
        return("Damn good");
    } else if (a < 15) {
        return("Very good");
    } else {
        return("Awesome");
    }
}

int build_number(void);

char *world_output(CHAR_DATA *ch, char *argument)
{
#ifdef HAVE_UNAME
    static char     retbuf[MAX_STRING_LENGTH];
#endif
    char            buf[MAX_INPUT_LENGTH];
    struct utsname  utsbuf;
    time_t          ct;
    char           *tmstr;
    char	    s1[16], s2[16];

    sprintf(s1, "%s", color_str(AT_SCORE, ch));
    sprintf(s2, "%s", color_str(AT_SCORE2, ch));

    sprintf(buf, "%sBase Source: %s%s, build %d\n\r",
            s1, s2, PACKAGE_STRING, build_number());
    strcpy(retbuf,buf);

#ifdef HAVE_UNAME
    uname(&utsbuf);
    sprintf(buf, "%sRunning on: %s%s %s\n\r",
            s1, s2, utsbuf.sysname,
            IS_IMMORTAL(ch)?utsbuf.release:"");
    strcat(retbuf,buf);
#endif

    ct = time(0);
    tmstr = asctime(localtime(&ct));
    *(tmstr + strlen(tmstr) - 1) = '\0';

    sprintf(buf, "%sCurrent time is (MST): %s%s\n\r",
            s1, s2, tmstr);
    strcat(retbuf,buf);

    sprintf(buf, "%sStart time was (MST): %s%s\n\r",
            s1, s2, str_boot_time);
    strcat(retbuf,buf);

    sprintf(buf, "%sTotal number of rooms in world: %s%d\n\r",
            s1, s2, top_room);
    strcat(retbuf,buf);

    sprintf(buf, "%sTotal number of zones in world: %s%d\n\r\n\r",
            s1, s2, top_area);
    strcat(retbuf,buf);

    sprintf(buf, "%sTotal number of distinct mobiles in world: %s%d\n\r",
            s1, s2, top_mob_index);
    strcat(retbuf,buf);

    sprintf(buf, "%sTotal number of distinct objects in world: %s%d\n\r\n\r",
            s1, s2, top_obj_index);
    strcat(retbuf,buf);

    sprintf(buf, "%sMax players ever connected: %s%d\n\r",
            s1, s2, sysdata.alltimemax);
    strcat(retbuf,buf);

    sprintf(buf, "%sTotal number of monsters in game: %s%d\n\r",
            s1, s2, nummobsloaded);
    strcat(retbuf,buf);

    sprintf(buf, "%sTotal number of objects in game: %s%d\n\r",
            s1, s2, numobjsloaded);
    strcat(retbuf,buf);
    return(retbuf);
}

void do_world( CHAR_DATA *ch, char *argument )
{
    char *buf;

    buf = world_output(ch,argument);

    send_to_char_color(buf,ch);
}

ALIAS_DATA *find_alias( CHAR_DATA *ch, char *argument )
{
    ALIAS_DATA *pal;
    char buf[MAX_INPUT_LENGTH];;

    if (!ch || !ch->pcdata)
        return(NULL);

    one_argument(argument, buf);

    for (pal=ch->pcdata->first_alias;pal;pal=pal->next)
        if ( !str_prefix(buf, pal->name) )
            return(pal);

    return(NULL);
}

void do_alias( CHAR_DATA *ch, char *argument )
{
    ALIAS_DATA *pal = NULL;
    char arg[MAX_INPUT_LENGTH];

    if (!ch || !ch->pcdata)
        return;

    smash_tilde(argument);
    argument = one_argument(argument, arg);

    if ( !*arg )
    {
        int x=0;
        pager_printf( ch, "%-20s What it does\n\r", "Alias" );
        for (pal=ch->pcdata->first_alias;pal;pal=pal->next)
            if (!IS_SET(pal->flags, ALIAS_REASSIGN))
            {
                pager_printf( ch, "%-20s %s\n\r",
                              pal->name, pal->cmd );
                x++;
            }
        pager_printf(ch, "%d aliases found.\n\r", x);
        return;
    }

    if ( !*argument)
    {
        pal = find_alias(ch, arg);
        if ( pal != NULL && !IS_SET(pal->flags, ALIAS_REASSIGN) )
        {
            DISPOSE(pal->name);
            DISPOSE(pal->cmd);
            UNLINK(pal, ch->pcdata->first_alias, ch->pcdata->last_alias, next, prev);
            DISPOSE(pal);
            send_to_char("Deleted Alias.\n\r", ch);
        } else
            send_to_char("That alias does not exist.\n\r", ch);
        return;
    }

    if ( (pal=find_alias(ch, arg)) == NULL )
    {
        CREATE(pal, ALIAS_DATA, 1);
        pal->name = str_dup(arg);
        pal->cmd  = str_dup(argument);
        LINK(pal, ch->pcdata->first_alias, ch->pcdata->last_alias, next, prev);
        send_to_char("Created Alias.\n\r", ch);
        return;
    }
    else if (!IS_SET(pal->flags, ALIAS_REASSIGN))
    {
        if (pal->cmd)
            DISPOSE(pal->cmd);
        pal->cmd  = str_dup(argument);
        send_to_char("Modified Alias.\n\r", ch);
        return;
    }
    else
    {
        send_to_char("You cannot alias that.\n\r", ch);
        log_printf_plus(LOG_MONITOR,LEVEL_IMMORTAL,SEV_INFO,"%s tried to alias a reassign (%s)",
                        GET_NAME(ch), pal->name);
        return;
    }
}
