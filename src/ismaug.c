/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer and Heath Leach
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*static char rcsid[] = "$Id: ismaug.c,v 1.16 2004/04/06 22:00:10 dotd Exp $";*/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "gsn.h"

void smaug_who( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char clan_name[MAX_INPUT_LENGTH];
    char council_name[MAX_INPUT_LENGTH];
    char invis_str[MAX_INPUT_LENGTH];
    char char_name[MAX_INPUT_LENGTH];
    char *extra_title;
    DESCRIPTOR_DATA *d;
    int iClass = 0, iRace;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    bool rgfRace[MAX_RACE];
    bool fClassRestrict;
    bool fRaceRestrict;
    bool fImmortalOnly;
    bool fPkill;
    bool fShowHomepage;
    bool fClanMatch; /* SB who clan (order),who guild, and who council */
    bool fCouncilMatch;
    bool fDeityMatch;
    CLAN_DATA *pClan=NULL;
    COUNCIL_DATA *pCouncil=NULL;
    DEITY_DATA *pDeity=NULL;
    FILE *whoout=NULL;
    
    /*
     #define WT_IMM    0;
     #define WT_MORTAL 1;
     #define WT_DEADLY 2;
     */
    
    WHO_DATA *cur_who = NULL;
    WHO_DATA *next_who = NULL;
    WHO_DATA *first_mortal = NULL;
    WHO_DATA *first_imm = NULL;
    WHO_DATA *first_deadly  = NULL;
    
    
    /*
     * Set default arguments.
     */
    iLevelLower    = 0;
    iLevelUpper    = MAX_LEVEL;
    fClassRestrict = FALSE;
    fRaceRestrict  = FALSE;
    fImmortalOnly  = FALSE;
    fPkill         = FALSE;
    fShowHomepage  = FALSE;
    fClanMatch	   = FALSE; /* SB who clan (order), who guild, who council */
    fCouncilMatch  = FALSE;
    fDeityMatch    = FALSE;
    for ( iRace = 0; iRace < MAX_RACE; iRace++ )
        rgfRace[iRace] = FALSE;
    
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
            if ( strlen(arg) < 3 )
            {
                send_to_char( "Classes must be longer than that.\n\r", ch );
                return;
            }
            
            /*
             * Look for ch_clases to turn on.
             */
            if ( !str_cmp( arg, "deadly" ) || !str_cmp( arg, "pkill" ) )
                fPkill = TRUE;
            else
                if ( !str_cmp( arg, "imm" ) || !str_cmp( arg, "gods" ) )
                    fImmortalOnly = TRUE;
                else
                    if ( !str_cmp( arg, "www" ) )
                        fShowHomepage = TRUE;
                    else		 /* SB who clan (order), guild, council */
                        if  ( ( pClan = get_clan (arg) ) )
                            fClanMatch = TRUE;
                        else
                            if ( ( pCouncil = get_council (arg) ) )
                                fCouncilMatch = TRUE;
                            else
                                if ( ( pDeity = get_deity (arg) ) )
                                    fDeityMatch = TRUE;
                                else
                                {
                                    if ( iClass != MAX_CLASS )
                                        fClassRestrict = TRUE;
                                    
                                    for ( iRace = 0; iRace < MAX_RACE; iRace++ )
                                    {
                                        if ( !str_cmp( arg, race_table[iRace].race_name ) )
                                        {
                                            rgfRace[iRace] = TRUE;
                                            break;
                                        }
                                    }
                                    if ( iRace != MAX_RACE )
                                        fRaceRestrict = TRUE;
                                    
                                    if ( iClass == MAX_CLASS && iRace == MAX_RACE 
                                         && fClanMatch == FALSE 
                                         && fCouncilMatch == FALSE
                                         && fDeityMatch == FALSE )
                                    {
                                        send_to_char( "That's not a ch_clas, race, order, guild,"
                                                      " council or deity.\n\r", ch );
                                        return;
                                    }
                                }
        }
    }
    
    /*
     * Now find matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    if ( ch )
        set_pager_color( AT_GREEN, ch );
    else
    {
        if ( fShowHomepage )
            whoout = fopen( WEBWHO_FILE, "w" );
        else
            whoout = fopen( WHO_FILE, "w" );
    }
    
    /* start from last to first to get it in the proper order */
    for ( d = last_descriptor; d; d = d->prev )
    {
        CHAR_DATA *wch;
        char const *ch_clas;
        
        if ( (d->connected != CON_PLAYING && d->connected != CON_EDITING)
             ||   !can_see( ch, d->character ) || d->original)
            continue;
        wch   = d->original ? d->original : d->character;
        if ( GetMaxLevel(wch) < iLevelLower
             ||   GetMaxLevel(wch) > iLevelUpper
             || ( fPkill && !CAN_PKILL( wch ) ) 
             || ( fImmortalOnly  && GetMaxLevel(wch) < LEVEL_IMMORTAL )
             || ( fRaceRestrict && !rgfRace[wch->race] )
             || ( fClanMatch && ( pClan != wch->pcdata->clan ))  /* SB */
             || ( fCouncilMatch && ( pCouncil != wch->pcdata->council )) /* SB */ 
             || ( fDeityMatch && ( pDeity != wch->pcdata->deity )) )
            continue;
        
        nMatch++;
        
        if ( fShowHomepage
             &&   wch->pcdata->homepage
             &&   wch->pcdata->homepage[0] != '\0' )
            sprintf( char_name, "<A HREF=\"%s\">%s</A>",
                     show_tilde( wch->pcdata->homepage ),
                     PERSLONG(wch, ch) );
        else
            strcpy( char_name, PERSLONG(wch, ch) );
        
        switch ( GetMaxLevel(wch) )
        {
        default: ch_clas = GetClassString(wch); break;
        case MAX_LEVEL -  0: ch_clas = "Supreme Entity";	break;
        case MAX_LEVEL -  1: ch_clas = "Infinite";	break;
        case MAX_LEVEL -  2: ch_clas = "Eternal";		break;
        case MAX_LEVEL -  3: ch_clas = "Ancient";		break;
        case MAX_LEVEL -  4: ch_clas = "Exalted God";	break;
        case MAX_LEVEL -  5: ch_clas = "Ascendant God";	break;
        case MAX_LEVEL -  6: ch_clas = "Greater God";	break;
        case MAX_LEVEL -  7: ch_clas = "God";		break;
        case MAX_LEVEL -  8: ch_clas = "Lesser God";	break;
        case MAX_LEVEL -  9: ch_clas = "Immortal";	break;
        case MAX_LEVEL - 10: ch_clas = "Demi God";	break;
        case MAX_LEVEL - 11: ch_clas = "Savior";		break;
        case MAX_LEVEL - 12: ch_clas = "Creator";		break;
        case MAX_LEVEL - 13: ch_clas = "Acolyte";		break;
        case MAX_LEVEL - 14: ch_clas = "Neophyte";	break;
        case MAX_LEVEL - 15: ch_clas = "Avatar";		break;
        }
        
        if ( !str_cmp( wch->name, sysdata.guild_overseer ) )
            extra_title = " [Overseer of Guilds]";
        else if ( !str_cmp( wch->name, sysdata.guild_advisor ) )
            extra_title = " [Advisor to Guilds]";
        else
            extra_title = "";
        
        if ( IS_RETIRED( wch ) )
            ch_clas = "Retired"; 
        else if ( IS_GUEST( wch ) )
            ch_clas = "Guest"; 
        else if ( wch->pcdata->rank && wch->pcdata->rank[0] != '\0' )
            ch_clas = wch->pcdata->rank;
        
        if ( wch->pcdata->clan )
        {
            CLAN_DATA *pclan = wch->pcdata->clan;
            if ( pclan->clan_type == CLAN_GUILD )
                strcpy( clan_name, " <" );
            else
                strcpy( clan_name, " (" );
            
            if ( pclan->clan_type == CLAN_ORDER )
            {
                if ( !str_cmp( wch->name, pclan->deity ) )
                    strcat( clan_name, "Deity, Order of " );
                else
                    if ( !str_cmp( wch->name, pclan->leader ) )
                        strcat( clan_name, "Leader, Order of " );
                    else
                        if ( !str_cmp( wch->name, pclan->number1 ) )
                            strcat( clan_name, "Number One, Order of " );
                        else
                            if ( !str_cmp( wch->name, pclan->number2 ) )
                                strcat( clan_name, "Number Two, Order of " );
                            else
                                strcat( clan_name, "Order of " );
            }
            else
                if ( pclan->clan_type == CLAN_GUILD )
                {
                    if ( !str_cmp( wch->name, pclan->leader ) )
                        strcat( clan_name, "Leader, " );
                    if ( !str_cmp( wch->name, pclan->number1 ) )
                        strcat( clan_name, "First, " );
                    if ( !str_cmp( wch->name, pclan->number2 ) )
                        strcat( clan_name, "Second, " );
                }
                else
                {
                    if ( !str_cmp( wch->name, pclan->deity ) )
                        strcat( clan_name, "Deity of " );
                    else
                        if ( !str_cmp( wch->name, pclan->leader ) )
                            strcat( clan_name, "Leader of " );
                        else
                            if ( !str_cmp( wch->name, pclan->number1 ) )
                                strcat( clan_name, "Number One " );
                            else
                                if ( !str_cmp( wch->name, pclan->number2 ) )
                                    strcat( clan_name, "Number Two " );
                } 
            strcat( clan_name, pclan->name );
            if ( pclan->clan_type == CLAN_GUILD )
                strcat( clan_name, ">" );
            else
                strcat( clan_name, ")" );
        }
        else
            clan_name[0] = '\0';
        
        if ( wch->pcdata->council )
        {
            strcpy( council_name, " [" );
            if ( !str_cmp( wch->name, wch->pcdata->council->head ) )
                strcat( council_name, "Head of " );
            strcat( council_name, wch->pcdata->council_name );
            strcat( council_name, "]" );
        }
        else
            council_name[0] = '\0';
        
        if ( wch->pcdata->wizinvis )
            sprintf( invis_str, "(%d) ", wch->pcdata->wizinvis );
        else
            invis_str[0] = '\0';
        sprintf( buf, "%-15s %s%s%s%s%s%s\n\r",
                 ch_clas,
                 invis_str,
                 IS_SET(wch->act, PLR_AFK) ? "[AFK] " : "",
                 IS_SET(wch->act, PLR_ATTACKER) ? "(ATTACKER) " : "",
                 IS_SET(wch->act, PLR_KILLER) ? "(KILLER) " : "",
                 IS_SET(wch->act, PLR_THIEF)  ? "(THIEF) "  : "",
                 PERSLONG(wch, ch));
        
        /*  
         * This is where the old code would display the found player to the ch.
         * What we do instead is put the found data into a linked list
         */ 
        
        /* First make the structure. */
        CREATE( cur_who, WHO_DATA, 1 );
        cur_who->text = str_dup( buf );
        if ( IS_IMMORTAL( wch ) )
            cur_who->type = WT_IMM;
        else if ( CAN_PKILL( wch ) ) 
            cur_who->type = WT_DEADLY;
        else
            cur_who->type = WT_MORTAL;
        
        /* Then put it into the appropriate list. */
        switch ( cur_who->type )
        {
        case WT_MORTAL:
            cur_who->next = first_mortal;
            first_mortal = cur_who;
            break;
        case WT_DEADLY:
            cur_who->next = first_deadly;
            first_deadly = cur_who;
            break;
        case WT_IMM:
            cur_who->next = first_imm;
            first_imm = cur_who;
            break;
        }
        
    }
    
    
    /* Ok, now we have three separate linked lists and what remains is to 
     * display the information and clean up.
     */
    
    for ( cur_who = first_mortal; cur_who; cur_who = next_who )
    {
        if ( !ch )
            fprintf( whoout, "%s", cur_who->text );
        else
            send_to_pager( cur_who->text, ch );
        next_who = cur_who->next;
        DISPOSE( cur_who->text );
        DISPOSE( cur_who ); 
    } 
    
    if ( first_deadly )
    {
        if ( !ch )
            fprintf( whoout, "\n\r-------------------------------[ DEADLY CHARACTERS ]-------------------------\n\r\n\r" );
        else
            send_to_pager( "\n\r-------------------------------[ DEADLY CHARACTERS ]--------------------------\n\r\n\r", ch );
    }
    
    for ( cur_who = first_deadly; cur_who; cur_who = next_who )
    {
        if ( !ch )
            fprintf( whoout, "%s", cur_who->text );
        else
            send_to_pager( cur_who->text, ch );
        next_who = cur_who->next;
        DISPOSE( cur_who->text );
        DISPOSE( cur_who ); 
    } 
    
    if ( first_imm )
    {
        if ( !ch )
            fprintf( whoout, "\n\r-----------------------------------[ IMMORTALS ]-----------------------------\n\r\n\r" );
        else
            send_to_pager( "\n\r-----------------------------------[ IMMORTALS ]------------------------------\n\r\n\r", ch );
    }
    
    for ( cur_who = first_imm; cur_who; cur_who = next_who )
    {
        if ( !ch )
            fprintf( whoout, "%s", cur_who->text );
        else
            send_to_pager( cur_who->text, ch );
        next_who = cur_who->next;
        DISPOSE( cur_who->text );
        DISPOSE( cur_who ); 
    } 
    
    if ( !ch )
    {
        fprintf( whoout, "%d player%s.\n\r", nMatch, nMatch == 1 ? "" : "s" );
        fclose( whoout );
        return;
    }
    
    set_char_color( AT_YELLOW, ch );
    ch_printf( ch, "%d player%s.\n\r", nMatch, nMatch == 1 ? "" : "s" );
    return;
}

char * get_ch_clas_smaug(CHAR_DATA *ch)
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

void smaug_score(CHAR_DATA * ch, char *argument)
{
    char            buf[MAX_STRING_LENGTH];
    AFFECT_DATA    *paf;
    int iLang;
    
    set_char_color(AT_SCORE4, ch);
    
    ch_printf(ch, "\n\rScore for %s%s.\n\r", ch->name, ch->pcdata->title);
    if ( get_trust( ch ) != GetMaxLevel(ch) )
        ch_printf( ch, "You are trusted at level %d.\n\r", get_trust( ch ) );
    
    send_to_char("----------------------------------------------------------------------------\n\r", ch);
    
    ch_printf(ch, "LEVEL: %-11.11s Race : %-10.10s        Played: %ld hours\n\r",
              GetLevelString(ch), capitalize(get_race_name(ch)), GET_TIME_PLAYED(ch));
    
    ch_printf(ch, "YEARS: %-6d      Class: %-11.11s       Log In: %s\r",
              get_age(ch), get_ch_clas_smaug(ch), ctime(&(ch->logon)) );
    
    if (GetMaxLevel(ch) >= 15){
        ch_printf(ch, "STR  : %2.2d(%2.2d)    HitRoll: %-15.15s   Saved: %s\r",
                  get_curr_str(ch), ch->perm_str, 
                  RollDesc(GET_HITROLL(ch)), ch->save_time ? ctime(&(ch->save_time)) : "no save this session\n" );
    } else {
        ch_printf(ch, "STR  : %2.2s(%2.2s)    HitRoll: %-15.15s   Saved: %s\r",
                  "XX", "XX", 
                  RollDesc(GET_HITROLL(ch)), ch->save_time ? ctime(&(ch->save_time)) : "no save this session\n" );
    }
    if (GetMaxLevel(ch) >= 15) {
        ch_printf(ch, "INT  : %2.2d(%2.2d)    DamRoll: %-15.15s   Time:  %s\r",
                  get_curr_int(ch), ch->perm_int, 
                  RollDesc(GET_DAMROLL(ch)), ctime(&current_time) );
    } else {
        ch_printf(ch, "INT  : %2.2s(%2.2s)    DamRoll: %-15.15s   Time:  %s\r",
                  "XX", "XX", 
                  RollDesc(GET_DAMROLL(ch)), ctime(&current_time) );
    }
    if (GET_AC(ch) >= 101)
        sprintf(buf, "the rags of a beggar");
    else if (GET_AC(ch) >= 80)
        sprintf(buf, "improper for adventure");
    else if (GET_AC(ch) >= 55)
        sprintf(buf, "shabby and threadbare");
    else if (GET_AC(ch) >= 40)
        sprintf(buf, "of poor quality");
    else if (GET_AC(ch) >= 20)
        sprintf(buf, "scant protection");
    else if (GET_AC(ch) >= 10)
        sprintf(buf, "that of a knave");
    else if (GET_AC(ch) >= 0)
        sprintf(buf, "moderately crafted");
    else if (GET_AC(ch) >= -10)
        sprintf(buf, "well crafted");
    else if (GET_AC(ch) >= -20)
        sprintf(buf, "the envy of squires");
    else if (GET_AC(ch) >= -40)
        sprintf(buf, "excellently crafted");
    else if (GET_AC(ch) >= -60)
        sprintf(buf, "the envy of knights");
    else if (GET_AC(ch) >= -80)
        sprintf(buf, "the envy of barons");
    else if (GET_AC(ch) >= -100)
        sprintf(buf, "the envy of dukes");
    else if (GET_AC(ch) >= -200)
        sprintf(buf, "the envy of emperors");
    else
        sprintf(buf, "that of an avatar");
    
    if (GetMaxLevel(ch) >= 15) {
        ch_printf(ch, "WIS  : %2.2d(%2.2d)      Armor: %s \n\r",
                  get_curr_wis(ch), ch->perm_wis, buf);
    } else {
        ch_printf(ch, "WIS  : %2.2s(%2.2s)      Armor: %s \n\r",
                  "XX", "XX", buf);
    }
    if (ch->alignment > 900)
        sprintf(buf, "devout");
    else if (ch->alignment > 700)
        sprintf(buf, "noble");
    else if (ch->alignment > 350)
        sprintf(buf, "honorable");
    else if (ch->alignment > 100)
        sprintf(buf, "worthy");
    else if (ch->alignment > -100)
        sprintf(buf, "neutral");
    else if (ch->alignment > -350)
        sprintf(buf, "base");
    else if (ch->alignment > -700)
        sprintf(buf, "evil");
    else if (ch->alignment > -900)
        sprintf(buf, "ignoble");
    else
        sprintf(buf, "fiendish");
    
    if (GetMaxLevel(ch) >= 15) {
        ch_printf(ch, "DEX  : %2.2d(%2.2d)      Align: %-20.20s    Items: %5.5d (max %5.5d)\n\r",
                  get_curr_dex(ch), ch->perm_dex, 
                  buf, carry_n(ch), can_carry_n(ch));
    } else {
        ch_printf(ch, "DEX  : %2.2s(%2.2s)      Align: %-20.20s    Items: %5.5d (max %5.5d)\n\r",
                  "XX", "XX", 
                  buf, carry_n(ch), can_carry_n(ch));
    }
    switch (ch->position)
    {
    case POS_DEAD:
        sprintf(buf, "slowly decomposing");
        break;
    case POS_MORTAL:
        sprintf(buf, "mortally wounded");
        break;
    case POS_INCAP:
        sprintf(buf, "incapacitated");
        break;
    case POS_STUNNED:
        sprintf(buf, "stunned");
        break;
    case POS_SLEEPING:
        sprintf(buf, "sleeping");
        break;
    case POS_RESTING:
        sprintf(buf, "resting");
        break;
    case POS_STANDING:
        sprintf(buf, "standing");
        break;
    case POS_FIGHTING:
        sprintf(buf, "fighting");
        break;
    case POS_MOUNTED:
        sprintf(buf, "mounted");
        break;
    case POS_SITTING:
        sprintf(buf, "sitting");
        break;
    case POS_MEDITATING:
        sprintf(buf, "meditating");
        break;
    }
    if (GetMaxLevel(ch) >= 15) {
        ch_printf(ch, "CON  : %2.2d(%2.2d)      Pos'n: %-21.21s  Weight: %5.5d (max %7.7d)\n\r",
                  get_curr_con(ch), ch->perm_con, 
                  buf, carry_w(ch), can_carry_w(ch));
        
        ch_printf(ch, "CHA  : %2.2d(%2.2d)      Wimpy: %d \n\r",
                  get_curr_cha(ch), ch->perm_cha, 
                  ch->wimpy);
        
        ch_printf(ch, "LCK  : %2.2d(%2.2d) \n\r",
                  get_curr_lck(ch), ch->perm_lck);
    } else {
        ch_printf(ch, "CON  : %2.2s(%2.2s)      Pos'n: %-21.21s  Weight: %5.5d (max %7.7d)\n\r",
                  "XX", "XX", 
                  buf, carry_w(ch), can_carry_w(ch));
        
        ch_printf(ch, "CHA  : %2.2s(%2.2s)      Wimpy: %d \n\r",
                  "XX", "XX", ch->wimpy);
        ch_printf(ch, "LCK  : %2.2s(%2.2s) \n\r",
                  "XX", "XX");
    }
    ch_printf(ch, "Glory: %4.4d(%4.4d) \n\r",
              ch->pcdata->quest_curr, ch->pcdata->quest_accum );
    
    ch_printf(ch, "PRACT: %3.3d         Hitpoints: %-5d of %5d   Pager: (%c) %3d    AutoExit(%c)\n\r",
              ch->practice, GET_HIT(ch), GET_MAX_HIT(ch),
              IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) ? 'X' : ' ',
              ch->pcdata->pagerlen, IS_SET(ch->act, PLR_AUTOEXIT) ? 'X' : ' ');
    
    if (IS_VAMPIRE(ch))
        ch_printf(ch, "Blood: %d of %d\n\r", GET_BLOOD(ch), GET_MAX_BLOOD(ch));
    
    ch_printf(ch, "XP   : %-9d        Mana: %-5d of %5d   MKills:  %-5.5d AutoLoot(%c)\n\r",
              GET_EXP(ch), GET_MANA(ch), GET_MAX_MANA(ch), ch->pcdata->mkills, IS_SET(ch->act, PLR_AUTOLOOT) ? 'X' : ' ');
    
    ch_printf(ch, "Gold : %-10d       Move: %-5d of %5d   Mdeaths: %-5.5d    AutoSac (%c)\n\r",
              GET_MONEY(ch,DEFAULT_CURR), GET_MOVE(ch), GET_MAX_MOVE(ch), ch->pcdata->mdeaths, IS_SET(ch->act, PLR_AUTOSAC) ? 'X' : ' ');
    
    if (!IS_NPC(ch) && GET_COND(ch, COND_DRUNK))
        send_to_char("You are drunk.\n\r", ch);
    if (!IS_NPC(ch) && GET_COND(ch, COND_THIRST))
        send_to_char("You are in danger of dehydrating.\n\r", ch);
    if (!IS_NPC(ch) && GET_COND(ch, COND_FULL))
        send_to_char("You are starving to death.\n\r", ch);
    if ( ch->position != POS_SLEEPING )
        switch( ch->mental_state / 10 )
        {
        default:   send_to_char( "You're completely messed up!\n\r", ch );	break;
        case -10:  send_to_char( "You're barely conscious.\n\r", ch );	break;
        case  -9:  send_to_char( "You can barely keep your eyes open.\n\r", ch );	break;
        case  -8:  send_to_char( "You're extremely drowsy.\n\r", ch );	break;
        case  -7:  send_to_char( "You feel very unmotivated.\n\r", ch );	break;
        case  -6:  send_to_char( "You feel sedated.\n\r", ch );		break;
        case  -5:  send_to_char( "You feel sleepy.\n\r", ch );		break;
        case  -4:  send_to_char( "You feel tired.\n\r", ch );		break;
        case  -3:  send_to_char( "You could use a rest.\n\r", ch );		break;
        case  -2:  send_to_char( "You feel a little under the weather.\n\r", ch );	break;
        case  -1:  send_to_char( "You feel fine.\n\r", ch );		break;
        case   0:  send_to_char( "You feel great.\n\r", ch );		break;
        case   1:  send_to_char( "You feel energetic.\n\r", ch );	break;
        case   2:  send_to_char( "Your mind is racing.\n\r", ch );	break;
        case   3:  send_to_char( "You can't think straight.\n\r", ch );	break;
        case   4:  send_to_char( "Your mind is going 100 miles an hour.\n\r", ch );	break;
        case   5:  send_to_char( "You're high as a kite.\n\r", ch );	break;
        case   6:  send_to_char( "Your mind and body are slipping apart.\n\r", ch );	break;
        case   7:  send_to_char( "Reality is slipping away.\n\r", ch );	break;
        case   8:  send_to_char( "You have no idea what is real, and what is not.\n\r", ch );	break;
        case   9:  send_to_char( "You feel immortal.\n\r", ch );	break;
        case  10:  send_to_char( "You are a Supreme Entity.\n\r", ch );	break;
        }
    else
        if ( ch->mental_state >45 )
            send_to_char( "Your sleep is filled with strange and vivid dreams.\n\r", ch );
        else
            if ( ch->mental_state >25 )
                send_to_char( "Your sleep is uneasy.\n\r", ch );
            else
                if ( ch->mental_state <-35 )
                    send_to_char( "You are deep in a much needed sleep.\n\r", ch );
                else
                    if ( ch->mental_state <-25 )
                        send_to_char( "You are in deep slumber.\n\r", ch );
    send_to_char("Languages: ", ch );
    for ( iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
        if ( knows_language( ch, lang_array[iLang], ch )
             ||  (IS_NPC(ch) && ch->speaks == 0) )
        {
            if ( lang_array[iLang] & ch->speaking
                 ||  (IS_NPC(ch) && !ch->speaking) )
                set_char_color( AT_RED, ch );
            send_to_char( lang_names[iLang], ch );
            send_to_char( " ", ch );
            set_char_color( AT_SCORE4, ch );
        }
    send_to_char( "\n\r", ch );
    
    if ( ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0' )
        ch_printf( ch, "You are bestowed with the command(s): %s.\n\r", 
                   ch->pcdata->bestowments );
    
    if ( CAN_PKILL( ch ) )
    {
        send_to_char( "----------------------------------------------------------------------------\n\r", ch);
        ch_printf(ch, "PKILL DATA:  Pkills (%3.3d)     Illegal Pkills (%3.3d)     Pdeaths (%3.3d)\n\r",
                  ch->pcdata->pkills, ch->pcdata->illegal_pk, ch->pcdata->pdeaths );
    }
    if (ch->pcdata->clan && ch->pcdata->clan->clan_type != CLAN_ORDER  && ch->pcdata->clan->clan_type != CLAN_GUILD )
    {
        send_to_char( "----------------------------------------------------------------------------\n\r", ch);
        ch_printf(ch, "CLAN STATS:  %-15.15s  Clan Pkills:  %-6d     Clan Pdeaths:  %-6d\n\r",
                  ch->pcdata->clan->name, ch->pcdata->clan->pkills, ch->pcdata->clan->pdeaths) ;
    }
    if (ch->pcdata->deity)
    {
        send_to_char( "----------------------------------------------------------------------------\n\r", ch);
        ch_printf(ch, "Deity:  %-20s  Favor: %d\n\r", ch->pcdata->deity->name, ch->pcdata->favor );
    }
    if (ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_ORDER )
    {
        send_to_char( "----------------------------------------------------------------------------\n\r", ch);
        ch_printf(ch, "Order:  %-20s  Order Mkills:  %-6d   Order MDeaths:  %-6d\n\r",
                  ch->pcdata->clan->name, ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths);
    }
    if (ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_GUILD )
    {
        send_to_char( "----------------------------------------------------------------------------\n\r", ch);
        ch_printf(ch, "Guild:  %-20s  Guild Mkills:  %-6d   Guild MDeaths:  %-6d\n\r",
                  ch->pcdata->clan->name, ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths);
    }
    if (IS_IMMORTAL(ch))
    {
        send_to_char( "----------------------------------------------------------------------------\n\r", ch);
        
        ch_printf(ch, "IMMORTAL DATA:  Wizinvis [%s]  Wizlevel (%d)\n\r",
                  ch->pcdata->wizinvis ? "X" : " ", ch->pcdata->wizinvis );
        
        ch_printf(ch, "Bamfin:  %s %s\n\r", ch->name, (ch->pcdata->bamfin[0] != '\0')
                  ? ch->pcdata->bamfin : "appears in a swirling mist.");
        ch_printf(ch, "Bamfout: %s %s\n\r", ch->name, (ch->pcdata->bamfout[0] != '\0')
                  ? ch->pcdata->bamfout : "leaves in a swirling mist.");
        
        
        /* Area Loaded info - Scryn 8/11*/
        if (ch->pcdata->area)
        {
            ch_printf(ch, "Vnums:   Room (%-5.5d - %-5.5d)   Object (%-5.5d - %-5.5d)   Mob (%-5.5d - %-5.5d)\n\r",
                      ch->pcdata->area->low_r_vnum, ch->pcdata->area->hi_r_vnum,
                      ch->pcdata->area->low_o_vnum, ch->pcdata->area->hi_o_vnum,
                      ch->pcdata->area->low_m_vnum, ch->pcdata->area->hi_m_vnum);
            ch_printf(ch, "Area Loaded [%s]\n\r", (IS_SET (ch->pcdata->area->status, AREA_LOADED)) ? "yes" : "no");
        }
    }
    if (ch->first_affect)
    {
        int i;
        SKILLTYPE *sktmp;
        
        i = 0;
        send_to_char( "----------------------------------------------------------------------------\n\r", ch);
        send_to_char("AFFECT DATA:                            ", ch);
        for (paf = ch->first_affect; paf; paf = paf->next)
        {
            if ( (sktmp=get_skilltype(paf->type)) == NULL )
                continue;
            if (GetMaxLevel(ch) < 20)
            {
                ch_printf(ch, "[%-34.34s]    ", sktmp->name);
                if (i == 0)
                    i = 2;
                if ((++i % 3) == 0)
                    send_to_char("\n\r", ch);
            }
            if (GetMaxLevel(ch) >= 20)
            {
                if (paf->modifier == 0)
                    ch_printf(ch, "[%-24.24s;%5d rds]    ",
                              sktmp->name,
                              paf->duration);
                else
                    if (paf->modifier > 999)
                        ch_printf(ch, "[%-15.15s; %5d rds]    ",
                                  sktmp->name,
                                  paf->duration);
                    else
                        ch_printf(ch, "[%-11.11s; %+-3.3d; %5d rds]    ",
                                  sktmp->name,
                                  paf->modifier,
                                  paf->duration);
                if (i == 0)
                    i = 1;
                if ((++i % 2) == 0)
                    send_to_char("\n\r", ch);
            }
        }
    }
    send_to_char("\n\r", ch);
    return;
}

void smaug_group( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *gch;
    CHAR_DATA *leader;
    
    leader = ch->leader ? ch->leader : ch;
    ch_printf( ch, "%s's group:\n\r", PERS(leader, ch) );
    
    for ( gch = first_char; gch; gch = gch->next )
    {
        if ( is_same_group( gch, ch ) )
        {
            ch_printf( ch,
                       "%-16s %4d/%4d hp %4d/%4d %s %4d/%4d mv %5d xp\n\r",
                       capitalize( PERS(gch, ch) ),
                       gch->hit,   
                       gch->max_hit,
                       IS_VAMPIRE(gch) ? GET_COND(gch, COND_BLOODTHIRST)
                       : gch->mana,
                       IS_VAMPIRE(gch) ? GET_BLOOD(ch) : gch->max_mana,
                       IS_VAMPIRE(gch) ? "bp" : "mana",
                       gch->move,  
                       gch->max_move,  
                       gch->exp    );
        }
    }
}

void smaug_prac_output( CHAR_DATA *ch, CHAR_DATA *is_at_gm )
{
    int	col, sn, i, is_ok;
    sh_int	lasttype, cnt;
    
    col = cnt = 0;	lasttype = SKILL_SPELL;
    set_pager_color( AT_MAGIC, ch );
    for ( sn = 0; sn < top_sn; sn++ )
    {
        if ( !skill_table[sn]->name )
            break;

        if (sn == gsn_drinking)
            continue;

        if ( strcmp(skill_table[sn]->name, "reserved") == 0
             && ( IS_IMMORTAL(ch) || CAN_CAST(ch) ) )
        {
            if ( col % 3 != 0 )
                send_to_pager( "\n\r", ch );
            send_to_pager(
                          "--------------------------------[Spells]---------------------------------\n\r", ch);
            col = 0;
        }
        if ( skill_table[sn]->type != lasttype )
        {
            if ( !cnt )
                send_to_pager( "                                (none)\n\r", ch );
            else
                if ( col % 3 != 0 )
                    send_to_pager( "\n\r", ch );
            pager_printf( ch,
                          "--------------------------------%ss---------------------------------\n\r",
                          skill_tname[skill_table[sn]->type]);
            col = cnt = 0;
        }
        lasttype = skill_table[sn]->type;
        
        if (!IS_IMMORTAL(ch) 
            && ( skill_table[sn]->guild != CLASS_NONE 
                 && ( !IS_GUILDED(ch)
                      || (ch->pcdata->clan->cl != skill_table[sn]->guild) ) ) )
            continue;
        
        is_ok = FALSE;
        if (is_at_gm) {
            if (GET_LEVEL(ch, FirstActive(is_at_gm)) >= skill_table[sn]->skill_level[FirstActive(is_at_gm)])
                is_ok = TRUE;
        } else
            for (i = 0; i < MAX_CLASS; ++i) {
                if (IS_ACTIVE(ch, i) && 
                    (GET_LEVEL(ch, i) >= skill_table[sn]->skill_level[i]))
                    is_ok = TRUE;                                      
            }                        
        
        if (!is_ok)
            continue;
        
        if ( LEARNED(ch, sn) == 0
             &&   SPELL_FLAG(skill_table[sn], SF_SECRETSKILL) )
            continue;
        
        if (is_at_gm)
            if (!skill_table[sn]->skill_level[FirstActive(is_at_gm)])
                continue;
        
        if (LEARNED(ch, sn) || is_at_gm) {
            ++cnt;
            pager_printf( ch, "%18s %3d%%  ",
                          skill_table[sn]->name, LEARNED(ch, sn) );
            if ( ++col % 3 == 0 )
                send_to_pager( "\n\r", ch ); 
        }
    }
    
    if ( col % 3 != 0 )
        send_to_pager( "\n\r", ch );
    
    pager_printf( ch, "You have %d practice sessions left.\n\r",
                  ch->practice );
}

