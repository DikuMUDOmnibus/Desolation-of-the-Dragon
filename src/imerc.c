/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer and Heath Leach
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*
 * $Log: imerc.c,v $
 * Revision 1.5  2002/06/15 03:48:01  dotd
 * carry_n and carry_w funtions to replace ch->carry_number and
 * ch->carry_weight so that special weights like gold can be handled
 *
 * Revision 1.4  2002/01/03 05:35:13  dotd
 * copyright info add/update for 2002 and new files
 *
 * Revision 1.3  2001/05/28 02:46:40  dotd
 * updated copyright info
 *
 * Revision 1.2  2001/05/09 01:25:13  dotd
 * revision touch
 *
 * Revision 1.1.1.1  2001/02/03 04:23:42  dotd
 * DOTDII Sources
 *
 * Revision 1.8  1999/05/30 16:31:21  garil
 * removed hunger and thirst
 *
 * Revision 1.7  1999/04/27 02:27:07  garil
 * currency updates
 *
 * Revision 1.6  1999/02/23 01:22:48  garil
 * major currency updates
 *
 * Revision 1.5  1999/01/11 06:51:13  dotd
 * DOTD License Header added prior to release of 2.2.2
 *
 * Revision 1.4  1998/12/16 19:10:24  dotd
 * added rcs id and log keywords
 *
 */

//static char rcsid[] = "$Id: imerc.c,v 1.5 2002/06/15 03:48:01 dotd Exp $";

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

void merc_who( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int iClass;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    bool rgfClass[REAL_MAX_CLASS];
    bool fClassRestrict;
    bool fImmortalOnly;
    
    /*
     * Set default arguments.
     */
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
    fClassRestrict = FALSE;
    fImmortalOnly  = FALSE;
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
        rgfClass[iClass] = FALSE;
    
    /*
     * Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {
        char arg[MAX_STRING_LENGTH];
        
        argument = one_argument( argument, arg );
        if ( arg[0] == '\0' )
            break;
        
        if ( is_number( arg ) )
        {
            switch ( ++nNumber )
            {
            case 1: iLevelLower = atoi( arg ); break;
            case 2: iLevelUpper = atoi( arg ); break;
            default:
                send_to_char( "Only two level numbers allowed.\n\r", ch );
                return;
            }
        }
        else
        {
            int iClass;
            
            if ( strlen(arg) < 3 )
            {
                send_to_char( "Classes must be longer than that.\n\r", ch );
                return;
            }
            
            /*
             * Look for classes to turn on.
             */
            arg[3]    = '\0';
            if ( !str_cmp( arg, "imm" ) )
            {
                fImmortalOnly = TRUE;
            }
            else
            {
                fClassRestrict = TRUE;
                for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
                {
                    if ( !str_cmp( arg, class_table[iClass]->who_name ) )
                    {
                        rgfClass[iClass] = TRUE;
                        break;
                    }
                }
                
                if ( iClass == MAX_CLASS )
                {
                    send_to_char( "That's not a class.\n\r", ch );
                    return;
                }
            }
        }
    }
    
    /*
     * Now show matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    for ( d = first_descriptor; d != NULL; d = d->next )
    {
        CHAR_DATA *wch;
        char const *class;
        
        /*
         * Check for match against restrictions.
         * Don't use trust as that exposes trusted mortals.
         */
        if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
            continue;
        
        wch   = ( d->original != NULL ) ? d->original : d->character;
        if ( wch->level < iLevelLower
             ||   wch->level > iLevelUpper
             || ( fImmortalOnly  && wch->level < LEVEL_HERO )
             || ( fClassRestrict && !rgfClass[wch->class] ) )
            continue;
        
        nMatch++;
        
        /*
         * Figure out what to print for class.
         */
        class = class_table[wch->class]->who_name;
        switch ( wch->level )
        {
        default: break;
        case MAX_LEVEL - 0: class = "GOD"; break;
        case MAX_LEVEL - 1: class = "SUP"; break;
        case MAX_LEVEL - 2: class = "DEI"; break;
        case MAX_LEVEL - 3: class = "ANG"; break;
        }
        
        /*
         * Format it up.
         */
        sprintf( buf + strlen(buf), "[%2d %s] %s%s%s%s\n\r",
                 wch->level,
                 class,
                 IS_SET(wch->act, PLR_KILLER) ? "(KILLER) " : "",
                 IS_SET(wch->act, PLR_THIEF)  ? "(THIEF) "  : "",
                 wch->name,
                 wch->pcdata->title );
    }
    
    sprintf( buf2, "You see %d player%s in the game.\n\r",
             nMatch, nMatch == 1 ? "" : "s" );
    strcat( buf, buf2 );
    send_to_char( buf, ch );
    return;
}


void merc_score( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;
    
    sprintf( buf,
             "You are %s%s, level %d, %d years old (%d hours).\n\r",
             ch->name,
             IS_NPC(ch) ? "" : ch->pcdata->title,
             ch->level,
             get_age(ch),
             (get_age(ch) - 17) * 2 );
    send_to_char( buf, ch );
    
    if ( get_trust( ch ) != ch->level )
    {
        sprintf( buf, "You are trusted at level %d.\n\r",
                 get_trust( ch ) );
        send_to_char( buf, ch );
    }
    
    sprintf( buf,
             "You have %d/%d hit, %d/%d mana, %d/%d movement, %d practices.\n\r",
             GET_HIT(ch),	GET_MAX_HIT(ch),
             GET_MANA(ch),	GET_MAX_MANA(ch),
             GET_MOVE(ch),	GET_MAX_MOVE(ch),
             ch->practice );
    send_to_char( buf, ch );
    
    sprintf( buf,
             "You are carrying %d/%d items with weight %d/%d kg.\n\r",
             carry_n(ch), can_carry_n(ch),
             carry_w(ch), can_carry_w(ch) );
    send_to_char( buf, ch );
    
    sprintf( buf,
             "Str: %d  Int: %d  Wis: %d  Dex: %d  Con: %d.\n\r",
             get_curr_str(ch),
             get_curr_int(ch),
             get_curr_wis(ch),
             get_curr_dex(ch),
             get_curr_con(ch) );
    send_to_char( buf, ch );
    
    sprintf( buf,
             "You have scored %d exp, and have %d gold coins.\n\r",
             ch->exp,  GET_MONEY(ch,DEFAULT_CURR) );
    send_to_char( buf, ch );
    
    sprintf( buf,
             "Autoexit: %s.  Autoloot: %s.  Autosac: %s.\n\r",
             (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT)) ? "yes" : "no",
             (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOLOOT)) ? "yes" : "no",
             (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOSAC) ) ? "yes" : "no" );
    send_to_char( buf, ch );
    
    sprintf( buf, "Wimpy set to %d hit points.\n\r", ch->wimpy );
    send_to_char( buf, ch );
    
    if ( !IS_NPC( ch ) )
    {
        sprintf( buf, "Page pausing set to %d lines of text.\n\r",
                 ch->pagelen );
        send_to_char( buf, ch );
    }
    
    if ( !IS_NPC(ch) && GET_COND(ch, COND_DRUNK) > 10 )
        send_to_char( "You are drunk.\n\r",   ch );
    if ( !IS_NPC(ch) && GET_COND(ch, COND_THIRST) ==  0 )
        send_to_char( "You are thirsty.\n\r", ch );
    if ( !IS_NPC(ch) && GET_COND(ch, COND_FULL) ==  0 )
        send_to_char( "You are hungry.\n\r",  ch );
    
    switch ( ch->position )
    {
    case POS_DEAD:     
        send_to_char( "You are DEAD!!\n\r",		ch );
        break;
    case POS_MORTAL:
        send_to_char( "You are mortally wounded.\n\r",	ch );
        break;
    case POS_INCAP:
        send_to_char( "You are incapacitated.\n\r",	ch );
        break;
    case POS_STUNNED:
        send_to_char( "You are stunned.\n\r",		ch );
        break;
    case POS_SLEEPING:
        send_to_char( "You are sleeping.\n\r",		ch );
        break;
    case POS_RESTING:
        send_to_char( "You are resting.\n\r",		ch );
        break;
    case POS_STANDING:
        send_to_char( "You are standing.\n\r",		ch );
        break;
    case POS_FIGHTING:
        send_to_char( "You are fighting.\n\r",		ch );
        break;
    }
    
    if ( ch->level >= 25 )
    {
        sprintf( buf, "AC: %d.  ", GET_AC(ch) );
        send_to_char( buf, ch );
    }
    
    send_to_char( "You are ", ch );
    if ( GET_AC(ch) >=  101 ) send_to_char( "WORSE than naked!\n\r", ch );
    else if ( GET_AC(ch) >=   80 ) send_to_char( "naked.\n\r",            ch );
    else if ( GET_AC(ch) >=   60 ) send_to_char( "wearing clothes.\n\r",  ch );
    else if ( GET_AC(ch) >=   40 ) send_to_char( "slightly armored.\n\r", ch );
    else if ( GET_AC(ch) >=   20 ) send_to_char( "somewhat armored.\n\r", ch );
    else if ( GET_AC(ch) >=    0 ) send_to_char( "armored.\n\r",          ch );
    else if ( GET_AC(ch) >= - 20 ) send_to_char( "well armored.\n\r",     ch );
    else if ( GET_AC(ch) >= - 40 ) send_to_char( "strongly armored.\n\r", ch );
    else if ( GET_AC(ch) >= - 60 ) send_to_char( "heavily armored.\n\r",  ch );
    else if ( GET_AC(ch) >= - 80 ) send_to_char( "superbly armored.\n\r", ch );
    else if ( GET_AC(ch) >= -100 ) send_to_char( "divinely armored.\n\r", ch );
    else                           send_to_char( "invincible!\n\r",       ch );
    
    if ( ch->level >= 15 )
    {
        sprintf( buf, "Hitroll: %d  Damroll: %d.\n\r",
                 GET_HITROLL(ch), GET_DAMROLL(ch) );
        send_to_char( buf, ch );
    }
    
    if ( ch->level >= 10 )
    {
        sprintf( buf, "Alignment: %d.  ", ch->alignment );
        send_to_char( buf, ch );
    }
    
    send_to_char( "You are ", ch );
    if ( ch->alignment >  900 ) send_to_char( "angelic.\n\r", ch );
    else if ( ch->alignment >  700 ) send_to_char( "saintly.\n\r", ch );
    else if ( ch->alignment >  350 ) send_to_char( "good.\n\r",    ch );
    else if ( ch->alignment >  100 ) send_to_char( "kind.\n\r",    ch );
    else if ( ch->alignment > -100 ) send_to_char( "neutral.\n\r", ch );
    else if ( ch->alignment > -350 ) send_to_char( "mean.\n\r",    ch );
    else if ( ch->alignment > -700 ) send_to_char( "evil.\n\r",    ch );
    else if ( ch->alignment > -900 ) send_to_char( "demonic.\n\r", ch );
    else                             send_to_char( "satanic.\n\r", ch );
    
    if ( ch->first_affect != NULL )
    {
        send_to_char( "You are affected by:\n\r", ch );
        for ( paf = ch->first_affect; paf != NULL; paf = paf->next )
        {
            sprintf( buf, "Spell: '%s'", skill_table[paf->type]->name );
            send_to_char( buf, ch );
            
            if ( ch->level >= 20 )
            {
                sprintf( buf,
                         " modifies %s by %d for %d hours",
                         affect_loc_name( paf->location ),
                         paf->modifier,
                         paf->duration );
                send_to_char( buf, ch );
            }
            
            send_to_char( ".\n\r", ch );
        }
    }
    
    return;
}
