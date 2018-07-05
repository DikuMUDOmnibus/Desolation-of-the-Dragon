/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer and Heath Leach
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*static char rcsid[] = "$Id: ienvy.c,v 1.10 2004/04/06 22:00:10 dotd Exp $";*/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

char * get_ch_class_envy(CHAR_DATA *ch)
{
    static char buf[MAX_STRING_LENGTH];
    static char buf2[MAX_STRING_LENGTH];
    int i;
    
    buf[0] = '\0';
    buf2[0] = '\0';
    
    for (i = 0; i < MAX_CLASS; ++i) {
        if (IS_ACTIVE(ch, i)) {
            if (HAD_CLASS(ch, i)) {
                sprintf(buf2, "[%s]/", short_pc_class[i]);
            } else {
                sprintf(buf2, "%s/", short_pc_class[i]);
            }
            strcat(buf, buf2);
        }
    }
    
    if (strlen(buf))
        buf[strlen(buf)-1] = '\0';
    return(buf);
}

int is_in_ch_class_set(bool ch_class_set[REAL_MAX_CLASS], CHAR_DATA *ch)
{
    int i;
    
    for (i = 0; i < MAX_CLASS; i++) {
        if (ch_class_set[i]) {
            if HAS_CLASS(ch, i) return(TRUE);
        } 
    }
    
    return (FALSE);
}

/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */
void envy_who( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char             buf      [ MAX_STRING_LENGTH*3 ];
    char             buf2     [ MAX_STRING_LENGTH   ];
    int              iClass;
    int              iLevelLower;
    int              iLevelUpper;
    int              nNumber;
    int              nMatch;
    bool             rgfClass [ MAX_CLASS ];
    bool             fClassRestrict;
    bool             fImmortalOnly;
    
    /*
     * Set default arguments.
     */
    iLevelLower    = 0;
    iLevelUpper    = LEVEL_SUPREME; /*Used to be Max_level */
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
        char arg [ MAX_STRING_LENGTH ];
        
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
            
            if ( strlen( arg ) < 3 )
            {
                send_to_char( "Classes must be longer than that.\n\r", ch );
                return;
            }
            
            /*
             * Look for classes to turn on.
             */
            if ( !strncmp( arg, "imm", sizeof(arg) ) )
            {
                fImmortalOnly = TRUE;
            }
            else
            {
                fClassRestrict = TRUE;
                for ( iClass = FIRST_CLASS; iClass < MAX_CLASS; iClass++ )
                {
                    if ( !strncmp( arg, pc_class[iClass], 3 ) )
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
    for ( d = first_descriptor; d; d = d->next )
    {
        CHAR_DATA       *wch;
        char      const *ch_class;
        
        wch   = ( d->original ) ? d->original : d->character;
        
        /*
         * Check for match against restrictions.
         * Don't use trust as that exposes trusted mortals.
         */
        if ( d->connected != CON_PLAYING || !can_see( ch, wch ) )
            continue;
        
        if (   GetMaxLevel(wch) < iLevelLower
               || GetMaxLevel(wch) > iLevelUpper
               || ( fImmortalOnly  && GetMaxLevel(wch) < LEVEL_HERO )
               || ( fClassRestrict && !is_in_ch_class_set(rgfClass, wch) ) )
            continue;
        
        nMatch++;
        
        /*
         * Figure out what to print for ch_class.
         */
        ch_class = GetClassString(wch);
        if ( GetMaxLevel(wch) >= LEVEL_IMMORTAL )
            switch ( GetMaxLevel(wch) )
            {
            default: break;
            case MAX_LEVEL - 0: ch_class = "DIRECT"; break;
            case MAX_LEVEL - 1: ch_class = "SENIOR"; break;
            case MAX_LEVEL - 2: ch_class = "JUNIOR"; break;
            case MAX_LEVEL - 3: ch_class = "APPREN"; break;
            }
        
        /*
         * Format it up.
         */
        if ( GetMaxLevel(wch) < LEVEL_IMMORTAL )
            sprintf( buf + strlen( buf ), "%s(%s%-15.15s%s) %s%s%s%s%s%s\n\r",
                     color_str(AT_WHO2, ch),
                     color_str(AT_WHO3, ch),
                     GET_RANK(wch),
                     color_str(AT_WHO2, ch),
                     color_str(AT_WHO, ch),
                     IS_SET( wch->act, PLR_KILLER   ) ? "(KILLER) " : "",
                     IS_SET( wch->act, PLR_THIEF    ) ? "(THIEF) "  : "",
                     IS_SET( wch->pcdata->flags, PCFLAG_DEADLY ) ? "(PK) ":"",
                     IS_SET( wch->act, PLR_AFK      ) ? "(AFK) "    : "",
                     PERSLONG(wch, ch) );
        else
            sprintf( buf + strlen( buf ), "%s(%s%-15s%s) %s%s%s%s%s%s%s\n\r",
                     color_str(AT_WHO2, ch),
                     color_str(AT_WHO3, ch),
                     GET_RANK(wch),
                     color_str(AT_WHO2, ch),
                     color_str(AT_WHO, ch),
                     wch->pcdata->wizinvis ? "(WIZINVIS) " : "",
                     IS_SET( wch->act, PLR_KILLER   ) ? "(KILLER) " : "",
                     IS_SET( wch->act, PLR_THIEF    ) ? "(THIEF) "  : "",
                     IS_SET( wch->pcdata->flags, PCFLAG_DEADLY ) ? "(PK) ":"",
                     IS_SET( wch->act, PLR_AFK      ) ? "(AFK) "    : "",
                     PERSLONG(wch, ch) );
    }
    
    sprintf( buf2, "%sYou see %s%d%s player%s in the game.\n\r",
             color_str(AT_WHO4, ch),
             color_str(AT_WHO, ch),
             nMatch, 
             color_str(AT_WHO4, ch),
             nMatch == 1 ? "" : "s");
    strcat( buf, buf2 );
    send_to_char( buf, ch );
    return;
}


void envy_score( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA *paf;
    char         buf1 [ MAX_STRING_LENGTH ];
    
    buf1[0] = '\0';
    paint( AT_SCORE, ch, "You are ");
    paint( AT_SCORE3, ch, "%s%s", GET_NAME(ch), IS_NPC(ch) ? "" : GET_TITLE(ch));
    paint( AT_SCORE, ch, ", ");
    paint( AT_SCORE2, ch, "%d ", get_age(ch));
    paint( AT_SCORE, ch, "years old (");
    paint( AT_SCORE2, ch, "%ld ", GET_TIME_PLAYED(ch));
    paint( AT_SCORE, ch, "hours).\n\r");
    
    paint( AT_SCORE, ch, "You are of the classes: ");
    paint( AT_SCORE2, ch, "%s.\n\r", get_ch_class_envy(ch));
    paint( AT_SCORE, ch, "You are of the levels:  ");
    paint( AT_SCORE2, ch, "%s.\n\r", GetLevelString(ch));
    
    paint( AT_SCORE, ch, "You are ");
    paint( AT_SCORE3, ch, "%s", race_table[GET_RACE(ch)].race_name);
    paint( AT_SCORE, ch, ".\n\r");
    
    if ( get_trust( ch ) != GetMaxLevel(ch) )
    {
        paint(AT_SCORE, ch, "You are trusted at level ");
        paint(AT_SCORE2, ch, "%d", get_trust(ch));
        paint(AT_SCORE, ch, ".\n\r");
    }
    
    paint(AT_SCORE, ch, "You have ");
    paint(AT_SCORE2, ch, "%d/%d ", GET_HIT(ch), GET_MAX_HIT(ch));
    paint(AT_SCORE, ch, "hps, ");
    paint(AT_SCORE2, ch, "%d/%d ", GET_MANA(ch), GET_MAX_MANA(ch));
    paint(AT_SCORE, ch, "mana, ");
    paint(AT_SCORE2, ch, "%d/%d ", GET_MOVE(ch), GET_MAX_MOVE(ch));
    paint(AT_SCORE, ch, "movement, and ");
    paint(AT_SCORE2, ch, "%d ", ch->practice);
    paint(AT_SCORE, ch, "pracs.\n\r");
    
    if (IS_VAMPIRE(ch)) {
        paint(AT_SCORE, ch, "You have ");        
        paint(AT_SCORE2, ch, "%d", GET_BLOOD(ch));
        paint(AT_SCORE, ch, " of ");
        paint(AT_SCORE2, ch, "%d", GET_MAX_BLOOD(ch));
        paint(AT_SCORE, ch, " blood points left.\n\r");
    }
    paint(AT_SCORE, ch, "You are carrying ");
    paint(AT_SCORE2, ch, "%d", carry_w(ch));
    paint(AT_SCORE, ch, " pounds of equipment.\n\r");
    
    if (GetMaxLevel(ch) >= 15) {
        paint(AT_SCORE, ch, "Str: ");
        paint(AT_SCORE2, ch, "%d  ", get_curr_str(ch));
        paint(AT_SCORE, ch, "Int: ");
        paint(AT_SCORE2, ch, "%d  ", get_curr_int(ch));
        paint(AT_SCORE, ch, "Wis: ");
        paint(AT_SCORE2, ch, "%d  ", get_curr_wis(ch));
        paint(AT_SCORE, ch, "Dex: ");
        paint(AT_SCORE2, ch, "%d  ", get_curr_dex(ch));
        paint(AT_SCORE, ch, "Con: ");
        paint(AT_SCORE2, ch, "%d  ", get_curr_con(ch));
        paint(AT_SCORE, ch, "Cha: ");
        paint(AT_SCORE2, ch, "%d  ", get_curr_cha(ch));
        paint(AT_SCORE, ch, "Lck: ");
        paint(AT_SCORE2, ch, "%d", get_curr_lck(ch));
        paint(AT_SCORE, ch, ".\n\r");
    }
    paint(AT_SCORE, ch, "You have scored ");
    paint(AT_SCORE2, ch, "%d", GET_EXP(ch));
    paint(AT_SCORE, ch, " exp, and have ");
    paint(AT_SCORE2, ch, "%d", GET_MONEY(ch,DEFAULT_CURR));
    paint(AT_SCORE, ch, " gold coins.\n\r");
    
    paint(AT_SCORE, ch, "Autoexit: ");
    paint(AT_SCORE3, ch, "%s.  ", 
          ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOEXIT ) ) ? "yes" : "no");
    paint(AT_SCORE, ch, "Autoloot: ");
    paint(AT_SCORE3, ch, "%s.  ", 
          ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOLOOT ) ) ? "yes" : "no");
    paint(AT_SCORE, ch, "Autogold: ");
    paint(AT_SCORE3, ch, "%s.  ", 
          ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOGOLD ) ) ? "yes" : "no");
    paint(AT_SCORE, ch, "Autosac: ");
    paint(AT_SCORE3, ch, "%s.\n\r", 
          ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOSAC ) ) ? "yes" : "no");
    
    paint(AT_SCORE, ch, "Wimpy set to ");
    paint(AT_SCORE2, ch, "%d", ch->wimpy);
    paint(AT_SCORE, ch, " hit points.\n\r");
    
    if ( !IS_NPC( ch ) )
    {
        paint(AT_SCORE, ch, "Page pausing set to %d lines of text.\n\r",
              ch->pagelen );
    }
    
    if ( !IS_NPC( ch ) && GET_COND(ch, COND_DRUNK) > 10 )
        paint(AT_SCORE, ch, "You are drunk.\n\r"   );
    if ( !IS_NPC( ch ) && GET_COND(ch, COND_THIRST) ==  0
         && GetMaxLevel(ch) < LEVEL_IMMORTAL )
        paint(AT_SCORE, ch, "You are thirsty.\n\r" );
    if ( !IS_NPC( ch ) && GET_COND(ch, COND_FULL)   ==  0
         && GetMaxLevel(ch) < LEVEL_IMMORTAL )
        paint(AT_SCORE3, ch, "You are hungry.\n\r"  );
    
    switch ( ch->position )
    {
    case POS_DEAD:     
        paint(AT_BLOOD, ch, "You are DEAD!!\n\r"            ); break;
    case POS_MORTAL:
        paint(AT_SCORE3, ch, "You are mortally wounded.\n\r" ); break;
    case POS_INCAP:
        paint(AT_SCORE3, ch, "You are incapacitated.\n\r"    ); break;
    case POS_STUNNED:
        paint(AT_SCORE3, ch, "You are stunned.\n\r"          ); break;
    case POS_SLEEPING:
        paint(AT_SCORE3, ch, "You are sleeping.\n\r"         ); break;
    case POS_RESTING:
        paint(AT_SCORE3, ch, "You are resting.\n\r"          ); break;
    case POS_STANDING:
        paint(AT_SCORE3, ch, "You are standing.\n\r"         ); break;
    case POS_FIGHTING:
        paint(AT_SCORE3, ch, "You are fighting.\n\r"         ); break;
    }
    
    sprintf(buf1, "You are ");
    if ( GET_AC( ch ) >=  101 ) strcat( buf1, "WORSE than naked!\n\r" );
    else if ( GET_AC( ch ) >=   80 ) strcat( buf1, "naked.\n\r"            );
    else if ( GET_AC( ch ) >=   60 ) strcat( buf1, "wearing clothes.\n\r"  );
    else if ( GET_AC( ch ) >=   40 ) strcat( buf1, "slightly armored.\n\r" );
    else if ( GET_AC( ch ) >=   20 ) strcat( buf1, "somewhat armored.\n\r" );
    else if ( GET_AC( ch ) >=    0 ) strcat( buf1, "armored.\n\r"          );
    else if ( GET_AC( ch ) >= - 20 ) strcat( buf1, "well armored.\n\r"     );
    else if ( GET_AC( ch ) >= - 40 ) strcat( buf1, "strongly armored.\n\r" );
    else if ( GET_AC( ch ) >= - 60 ) strcat( buf1, "heavily armored.\n\r"  );
    else if ( GET_AC( ch ) >= - 80 ) strcat( buf1, "superbly armored.\n\r" );
    else if ( GET_AC( ch ) >= -100 ) strcat( buf1, "divinely armored.\n\r" );
    else                           strcat( buf1, "invincible!\n\r"       );
    
    paint(AT_SCORE3, ch, "%s", buf1);
    
    sprintf(buf1, "You are ");
    if ( ch->alignment >  900 ) strcat( buf1, "angelic.\n\r" );
    else if ( ch->alignment >  700 ) strcat( buf1, "saintly.\n\r" );
    else if ( ch->alignment >  350 ) strcat( buf1, "good.\n\r"    );
    else if ( ch->alignment >  100 ) strcat( buf1, "kind.\n\r"    );
    else if ( ch->alignment > -100 ) strcat( buf1, "neutral.\n\r" );
    else if ( ch->alignment > -350 ) strcat( buf1, "mean.\n\r"    );
    else if ( ch->alignment > -700 ) strcat( buf1, "evil.\n\r"    );
    else if ( ch->alignment > -900 ) strcat( buf1, "demonic.\n\r" );
    else                             strcat( buf1, "satanic.\n\r" );
    
    paint(AT_SCORE3, ch, "%s", buf1);
    
    if ( ch->first_affect )
    {
        bool printed = FALSE;
        
        for ( paf = ch->first_affect; paf; paf = paf->next )
        {
            if ( !printed )
            {
                paint(AT_SCORE, ch, "You are affected by:\n\r" );
                printed = TRUE;
            }
            
            paint(AT_SCORE, ch, "Spell: '"); 
            paint(AT_SCORE2, ch, "%s", skill_table[paf->type]->name );
            paint(AT_SCORE, ch, "'");
            
            if ( GetMaxLevel(ch) >= LEVEL_IMMORTAL )
            {
                paint(AT_SCORE, ch, 
                      " modifies %s by %d for %d rounds",
                      affect_loc_name( paf->location ),
                      paf->modifier,
                      paf->duration );
            }
            
            paint(AT_SCORE, ch, ".\n\r" );
        }
    }
    
    return;
}
