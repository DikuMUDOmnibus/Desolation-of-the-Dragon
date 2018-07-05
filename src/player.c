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
 * 		Commands for personal player settings/statictics	    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: player.c,v 1.38 2004/04/06 22:00:10 dotd Exp $";*/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "mxp.h"
#include "rareobj.h"

/*
 *  Locals
 */
void do_gold(CHAR_DATA * ch, char *argument)
{
    int i;
    bool found = FALSE;
    
    set_char_color( AT_GOLD, ch );
    for (i=0;i<MAX_CURR_TYPE;i++)
        if (GET_MONEY(ch, i))
        {
            ch_printf( ch,  "You have %d %s pieces.\n\r", GET_MONEY(ch,i), curr_types[i] );
            found = TRUE;
        }

    if (!found)
        send_to_char("You have no money!\n\r", ch);
}

/*
 * Dale like score command by Heath
 */
void do_score(CHAR_DATA * ch, char *argument)
{
    switch(GET_INTF(ch)) {
    case INT_DALE:  dale_score(ch, argument);
    break;
    case INT_SMAUG: smaug_score(ch, argument);
    break;
    case INT_MERC:  envy_score(ch, argument);
    break;
    case INT_ENVY:  envy_score(ch, argument);
    break;
    default: dale_score(ch, argument);
    break;
    }

    if (GetMaxLevel(ch)<5)
        send_to_char("Note: Also try the 'attribute' command.\n\r(This message will self-destruct when you reach level 5)\n\r", ch);
}

void do_attrib(CHAR_DATA * ch, char *argument)
{
    switch(GET_INTF(ch)) {
    case INT_SMAUG: smaug_score(ch, argument);
    break;
    case INT_DALE:  dale_attrib(ch, argument);
    break;
    default: dale_attrib(ch, argument);
    break;
    }

    if (GetMaxLevel(ch)<5)
        send_to_char("Note: This command will show your stats when you reach level 15.\n\r(This message will self-destruct when you reach level 5)\n\r", ch);
}

char * get_class_name(CHAR_DATA *ch)
{
    return (str_dup(GetClassString(ch)));
}


char * get_race_name( CHAR_DATA *ch)
{
    if ( GET_RACE(ch) < MAX_RACE && GET_RACE(ch) >= 0)
        return ( race_table[GET_RACE(ch)].race_name );
    return ("Unknown");
}


/*								-Thoric
 * Display your current exp, level, and surrounding level exp requirements
 */
void do_level( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_STRING_LENGTH];
    int x, lowlvl, hilvl, lev;
    sh_int i;

    lev = atoi(argument);

    for (i = FIRST_CLASS; i < MAX_CLASS; ++i) {
        if (IS_ACTIVE(ch, i)) {
            if ( ch->levels[i] == 1 )
                lowlvl = 1;
            else
                lowlvl = UMAX( 2, (lev?lev:ch->levels[i]) - 5 );
            hilvl = URANGE( lev?lev:ch->levels[i], (lev?lev:ch->levels[i]) + 5, RacialMax[GET_RACE(ch)][i] );
            set_char_color( AT_SCORE, ch );
            ch_printf( ch, "Experience required %s levels %d to %d:\n\r", 
                       pc_class[i], lowlvl, hilvl );
            sprintf( buf, " exp (You have %d)", ch->exp );
            for ( x = lowlvl; x <= hilvl; x++ )
                ch_printf( ch, " %3d) %11ld%s\n\r", x, exp_level( ch, x, i ),
                           (x == ch->levels[i]) ? buf : " exp" );
        }
    }
}


void do_affected ( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    SKILLTYPE *skill;
    
    if ( IS_NPC(ch) )
        return;
    
    argument = one_argument( argument, arg );
    
    if ( !str_cmp( arg, "by" ) )
    {
        set_char_color( AT_BLUE, ch );
        send_to_char( "\n\rImbued with:\n\r", ch );
        set_char_color( AT_SCORE, ch );
        ch_printf( ch, "%s %s\n\r",
                   flag_string( ch->affected_by, a_flags ),
                   flag_string( ch->affected_by2, a2_flags ));
        if ( GetMaxLevel(ch) >= 20 )
        {
            send_to_char( "\n\r", ch );
            if ( ch->resistant > 0 )
            {
                set_char_color ( AT_BLUE, ch );
                send_to_char( "Resistances:  ", ch );
                set_char_color( AT_SCORE, ch );
                ch_printf( ch, "%s\n\r", flag_string(ch->resistant, ris_flags) );
            }
            if ( ch->immune > 0 )
            {
                set_char_color( AT_BLUE, ch );
                send_to_char( "Immunities:   ", ch);
                set_char_color( AT_SCORE, ch );
                ch_printf( ch, "%s\n\r", flag_string(ch->immune, ris_flags) );
            }
            if ( ch->susceptible > 0 )
            {
                set_char_color( AT_BLUE, ch );
                send_to_char( "Suscepts:     ", ch );
                set_char_color( AT_SCORE, ch );
                ch_printf( ch, "%s\n\r", flag_string(ch->susceptible, ris_flags) );
            }
            if ( ch->absorb > 0 )
            {
                set_char_color( AT_BLUE, ch );
                send_to_char( "Absorbs:     ", ch );
                set_char_color( AT_SCORE, ch );
                ch_printf( ch, "%s\n\r", flag_string(ch->absorb, ris_flags) );
            }
        }
        return;      
    }
    
    if ( !ch->first_affect )
    {
        set_char_color( AT_SCORE, ch );
        send_to_char( "\n\rNo cantrip or skill affects you.\n\r", ch );
    }
    else
    {
        send_to_char( "\n\r", ch );
        for (paf = ch->first_affect; paf; paf = paf->next)
            if ( (skill=get_skilltype(paf->type)) != NULL )
            {
                set_char_color( AT_BLUE, ch );
                send_to_char( "Affected:  ", ch );
                set_char_color( AT_SCORE, ch );
                if ( GetMaxLevel(ch) >= 20   ||   IS_PKILL( ch ) )
                {
                    if (paf->duration < 25 ) set_char_color( AT_WHITE, ch );
                    if (paf->duration < 6  ) set_char_color( AT_WHITE + AT_BLINK, ch );
                    ch_printf( ch, "(%d hours)   ", paf->duration/(int)DUR_CONV );
                }
                ch_printf( ch, "%-18s\n\r", skill->name );
            }
    }

    if (GetMaxLevel(ch)<5)
        send_to_char("Note: Also try the 'attribute' command.\n\r(This message will self-destruct when you reach level 5)\n\r", ch);
    return;
}

void do_inventory( CHAR_DATA *ch, char *argument )
{
    set_char_color( AT_PLAIN, ch );
    send_to_char( "You are carrying:\n\r", ch );
    strcpy(mxpprecommand, "wear");
    mxpposcommand[0]='\0';
    show_list_to_char( ch->first_carrying, ch, TRUE, TRUE );
    return;
}


void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int iWear;
    bool found;
    
    set_char_color( AT_PLAIN, ch );
    send_to_char( "You are using:\n\r", ch );
    found = FALSE;
    set_char_color( AT_OBJECT, ch );
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
        for ( obj = ch->first_carrying; obj; obj = obj->next_content )
            if ( obj->wear_loc == iWear )
            {
                send_to_char( where_name[iWear], ch );
                if ( can_see_obj( ch, obj ) )
                {
                    strcpy(mxpprecommand, "remove");
                    mxpposcommand[0]='\0';
                    send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
                    send_to_char( "\n\r", ch );
                }
                else
                    send_to_char( "something.\n\r", ch );
                found = TRUE;
            }
    }
    
    if ( !found )
        send_to_char( "Nothing.\n\r", ch );
    
    return;
}



void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];
    
    if ( IS_NPC(ch) )
    {
        bug( "Set_title: NPC." );
        return;
    }
    /*
     if ( isalpha(title[0]) || 
     isdigit(title[0]) ||
     (title[0] == '$') )
     {*/
    buf[0] = ' ';
    strcpy( buf+1, title );
    /*    }
     else
     strcpy( buf, title );
     */
    STRFREE( ch->pcdata->title );
    ch->pcdata->title = STRALLOC( buf );
    return;
}



void do_title( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    
    if ( IS_NPC(ch) )
        return;
    
    if ( IS_SET( ch->pcdata->flags, PCFLAG_NOTITLE ))
    {
        send_to_char( "The Gods prohibit you from changing your title.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        sprintf(buf, "the %s %s", get_race_name(ch), GetTitleString(ch));
        set_title(ch, buf);
        ch_printf(ch, "Your title is now: %s\n\r", buf);
        return;
    }
    
    if ( strlen(argument) > 50 )
        argument[50] = '\0';
    
    smash_tilde( argument );
    set_title( ch, argument );
    send_to_char( "Ok.\n\r", ch );
}


void do_homepage( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    
    if ( IS_NPC(ch) )
        return;
    
    if ( argument[0] == '\0' )
    {
        if ( !ch->pcdata->homepage )
            ch->pcdata->homepage = str_dup( "" );
        ch_printf( ch, "Your homepage is: %s\n\r",
                   show_tilde( ch->pcdata->homepage ) );
        return;
    }
    
    if ( !str_cmp( argument, "clear" ) )
    {
        if ( ch->pcdata->homepage )
            DISPOSE(ch->pcdata->homepage);
        ch->pcdata->homepage = str_dup("");
        send_to_char( "Homepage cleared.\n\r", ch );
        return;
    }
    
    if ( strstr( argument, "://" ) )
        strcpy( buf, argument );
    else
        sprintf( buf, "http://%s", argument );
    if ( strlen(buf) > 70 )
        buf[70] = '\0';
    
    hide_tilde( buf );
    if ( ch->pcdata->homepage )
        DISPOSE(ch->pcdata->homepage);
    ch->pcdata->homepage = str_dup(buf);
    send_to_char( "Homepage set.\n\r", ch );
}



/*
 * Set your personal description				-Thoric
 */
void do_description( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
    {
        send_to_char( "Monsters are too dumb to do that!\n\r", ch );
        return;	  
    }
    
    if ( !ch->desc )
    {
        bug( "do_description: no descriptor" );
        return;
    }
    
    switch( ch->substate )
    {
    default:
        bug( "do_description: illegal substate" );
        return;
        
    case SUB_RESTRICTED:
        send_to_char( "You cannot use this command from within another command.\n\r", ch );
        return;
        
    case SUB_NONE:
        ch->substate = SUB_PERSONAL_DESC;
        ch->dest_buf = ch;
        start_editing( ch, ch->description );
        return;
        
    case SUB_PERSONAL_DESC:
        STRFREE( ch->description );
        ch->description = copy_buffer( ch );
        stop_editing( ch );
        return;	
    }
}

/* Ripped off do_description for whois bio's -- Scryn*/
void do_bio( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mobs can't set bio's!\n\r", ch );
        return;	  
    }
    
    if ( !ch->desc )
    {
        bug( "do_bio: no descriptor" );
        return;
    }
    
    switch( ch->substate )
    {
    default:
        bug( "do_bio: illegal substate" );
        return;
        
    case SUB_RESTRICTED:
        send_to_char( "You cannot use this command from within another command.\n\r", ch );
        return;
        
    case SUB_NONE:
        ch->substate = SUB_PERSONAL_BIO;
        ch->dest_buf = ch;
        start_editing( ch, ch->pcdata->bio );
        return;
        
    case SUB_PERSONAL_BIO:
        STRFREE( ch->pcdata->bio );
        ch->pcdata->bio = copy_buffer( ch );
        stop_editing( ch );
        return;	
    }
}



void do_report( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    
    if ( IS_AFFECTED(ch, AFF_POSSESS) )
    {   
        send_to_char("You can't do that in your current state of mind!\n\r", ch);
        return;
    }
    
    if ( !IS_VAMPIRE(ch) )
    {
        sprintf(buf, "You report 'HP:%d%% MANA:%d%% MV:%d%%'",
		GET_HIT(ch)  * 100 / GET_MAX_HIT(ch),
		GET_MANA(ch) * 100 / GET_MAX_MANA(ch),
		GET_MOVE(ch) * 100 / GET_MAX_MOVE(ch));
        act(AT_REPORT, buf, ch, NULL, NULL, TO_CHAR);
	sprintf(buf, "[$n] reports 'HP:%d%% MANA:%d%% MV:%d%%'",
		GET_HIT(ch)  * 100 / GET_MAX_HIT(ch),
                GET_MANA(ch) * 100 / GET_MAX_MANA(ch),
		GET_MOVE(ch) * 100 / GET_MAX_MOVE(ch));
        act(AT_REPORT, buf, ch, NULL, NULL, TO_ROOM);
    }
    else
    {
	sprintf(buf, "You report 'HP:%d%% BLOOD:%d%% MV:%d%%'",
		GET_HIT(ch)  * 100 / GET_MAX_HIT(ch),
		GET_COND(ch, COND_BLOODTHIRST) * 100 / GET_MAX_BLOOD(ch),
                GET_MOVE(ch) * 100 / GET_MAX_MOVE(ch));
        act(AT_REPORT, buf, ch, NULL, NULL, TO_CHAR);
        sprintf(buf, "[$n] reports 'HP:%d%% BLOOD:%d%% MV:%d%%'",
                GET_HIT(ch)  * 100 / (int) GET_MAX_HIT(ch),
		GET_COND(ch, COND_BLOODTHIRST) * 100 / GET_MAX_BLOOD(ch),
                GET_MOVE(ch) * 100 / (int) GET_MAX_MOVE(ch));
        act(AT_REPORT, buf, ch, NULL, NULL, TO_ROOM);	
    }
    
}

void do_prompt( CHAR_DATA *ch, char *argument )
{
    static struct def_prompt {
        int n;
        char *pr;
    } prompts[] = {
        {1,  "DOTD> "},
        {2,  "H:%h V:%v> "},
        {3,  "H:%h M:%m V:%v> "},
        {4,  "H:%h V:%v C:%C> "},
        {5,  "H:%h/%H V:%v/%V> "},
        {6,  "H:%h V:%v C:%C %S> "},
        {7,  "H:%h M:%m V:%v C:%C> "},
        {8,  "H:%h/%H M:%m/%M V:%v/%V> "},
        {9,  "H:%h/%H V:%v/%V C:%C %S> "},
        {10, "H:%h M:%m V:%v C:%C %S> "},
        {11, "%h/%m/%v (%H/%M/%V) %S %C> "},
        {12, "H:%h/%H M:%m/%M V:%v/%V C:%C %S> "},
        {40, "R:%r> "},
        {41, "R:%r i%I+> "},
        {0,NULL}
    };
    char buf[512];
    int i,n;
    
    if (IS_NPC(ch) || !ch->desc)
        return;
    
    smash_tilde( argument );
    
    if (*argument) {
        if (is_number(argument)) {
            n = atoi(argument);
            if (n>39 && !IS_IMMORTAL(ch)) {
                send_to_char("No picking immortal prompts.\r\n",ch);
                return;
            }
            for(i=0;prompts[i].pr;i++)
                if (prompts[i].n==n) {
                    if (ch->pcdata->prompt) 
                        STRFREE(ch->pcdata->prompt);
                    ch->pcdata->prompt = STRALLOC(prompts[i].pr);
                    return;
                }
            send_to_char("Invalid prompt number.\n\r",ch);
        } else {
            if (ch->pcdata->prompt) 
                STRFREE(ch->pcdata->prompt);
            ch->pcdata->prompt = STRALLOC(argument);
        }
    } else {
        for(i=0;prompts[i].pr;i++) {
            sprintf(buf,"%s%d: %s%s\n\r",
                    color_str(AT_DGREY,ch),prompts[i].n,color_str(AT_BLUE,ch),prompts[i].pr);
            send_to_char(buf,ch);
        }
        set_char_color(AT_PLAIN, ch);
        sprintf(buf,"Your current prompt is : %s\n\r",ch->pcdata->prompt);
        send_to_char(buf,ch);
    }
    
}


void do_delete(CHAR_DATA *ch, char *argument)
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char name[MAX_INPUT_LENGTH];
    int x, y, vlev;

    if (IS_NPC(ch))
        return;

    if (IS_AFFECTED(ch, AFF_CHARM))
        return;

    if ( str_cmp(argument, "me") )
    {
        send_to_char( "Type 'delete me' if you're sure.\n\r", ch );
        return;
    }
    /* Make sure they aren't halfway logged in. */
    for ( d = first_descriptor; d; d = d->next )
        if ( ch == d->character )
            break;
    if ( d && ch == d->character )
        close_socket( d, TRUE );

    quitting_char = ch;
    save_char_obj( ch );
    delete_char_rare_obj( ch );
    saving_char = NULL;

    strcpy(name, GET_NAME(ch));
    vlev = GetMaxLevel(ch);

    extract_char( ch, TRUE );

    for ( x = 0; x < MAX_WEAR; x++ )
        for ( y = 0; y < MAX_LAYERS; y++ )
            save_equipment[x][y] = NULL;

    if (vlev <= 1)
        return;

    sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower(name[0]),
             capitalize( name ) );
    sprintf( buf2, "%s%c/%s", BACKUP_DIR, tolower(name[0]),
             capitalize( name ) );

    if ( !rename( buf, buf2 ) )
    {
        AREA_DATA *pArea;

        sprintf( buf, "%s%s", GOD_DIR, capitalize(name) );
        if (remove( buf ))
        {
	    bug( "do_delete: error destroying %s", buf );
            perror( buf );
        }

        sprintf( buf2, "%s.are", capitalize(name) );
        for ( pArea = first_build; pArea; pArea = pArea->next )
            if ( !strcmp( pArea->name, buf2 ) )
            {
                sprintf( buf, "%s%s", BUILD_DIR, buf2 );
                if ( IS_SET( pArea->status, AREA_LOADED ) )
                    fold_area( pArea, buf, FALSE );
                close_area( pArea, TRUE );
                sprintf( buf2, "%s.bak", buf );
                if ( rename( buf, buf2 ) )
                {
		    bug( "do_delete: error destroying %s", buf );
                    perror( buf );
                }
            }
    }
    else
    {
	bug( "do_delete: error deleting %s", name );
        perror( buf );
    }
}


bool IsHumanoid(CHAR_DATA *ch)
{
    if (!ch)
        return (FALSE);
    
    switch (GET_RACE(ch))
    {
    case RACE_HUMAN:
    case RACE_GNOME:
    case RACE_DEEP_GNOME:
    case RACE_ELVEN:
    case RACE_GOLD_ELF:
    case RACE_WILD_ELF:
    case RACE_SEA_ELF:
    case RACE_DWARF:
    case RACE_DARK_DWARF:
    case RACE_HALFLING:
    case RACE_ORC:
    case RACE_LYCANTH:
    case RACE_UNDEAD:
    case RACE_UNDEAD_VAMPIRE:
    case RACE_UNDEAD_LICH:
    case RACE_UNDEAD_WIGHT:
    case RACE_UNDEAD_GHAST:
    case RACE_UNDEAD_SPECTRE:
    case RACE_UNDEAD_ZOMBIE:
    case RACE_UNDEAD_SKELETON:
    case RACE_UNDEAD_GHOUL:
    case RACE_GIANT:
    case RACE_GIANT_HILL:
    case RACE_GIANT_FROST:
    case RACE_GIANT_FIRE:
    case RACE_GIANT_CLOUD:
    case RACE_GIANT_STORM:
    case RACE_GIANT_STONE:
    case RACE_GOBLIN:
    case RACE_DEVIL:
    case RACE_TROLL:
    case RACE_VEGMAN:
    case RACE_MFLAYER:
    case RACE_ENFAN:
    case RACE_PATRYN:
    case RACE_SARTAN:
    case RACE_ROO:
    case RACE_SMURF:
    case RACE_TROGMAN:
    case RACE_LIZARDMAN:
    case RACE_SKEXIE:
    case RACE_TYTAN:
    case RACE_DROW:
    case RACE_GOLEM:
    case RACE_DEMON:
    case RACE_DRAAGDIM:
    case RACE_ASTRAL:
    case RACE_GOD:
    case RACE_HALF_ELVEN:
    case RACE_HALF_ORC:
    case RACE_HALF_OGRE:
    case RACE_HALF_GIANT:
    case RACE_GNOLL:
    case RACE_TIEFLING:
    case RACE_AASIMAR:
    case RACE_VAGABOND:
    case RACE_GITHZERAI:
    case RACE_GITHYANKI:
    case RACE_BARIAUR:
        return (TRUE);
        break;
        
    default:
        return (FALSE);
        break;
    }
    
}

bool IsRideable(CHAR_DATA *ch)
{
    if (!ch || !IS_NPC(ch))
        return (FALSE);

    if (IsDragon(ch))
        return TRUE;

    switch (GET_RACE(ch))
    {
    case RACE_BARIAUR:
    case RACE_HORSE:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }

    return (FALSE);
}

bool IsAnimal(CHAR_DATA *ch)
{
    if (!ch)
        return (FALSE);
    
    switch (GET_RACE(ch))
    {
    case RACE_PREDATOR:
    case RACE_FISH:
    case RACE_BIRD:
    case RACE_HERBIV:
    case RACE_REPTILE:
    case RACE_LABRAT:
    case RACE_ROO:
    case RACE_INSECT:
    case RACE_ARACHNID:
    case RACE_SNAKE:
    case RACE_DINOSAUR:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
    
}

bool IsVeggie(CHAR_DATA *ch)
{
    if (!ch)
        return (FALSE);
    
    switch (GET_RACE(ch))
    {
    case RACE_PARASITE:
    case RACE_SLIME:
    case RACE_TREE:
    case RACE_VEGGIE:
    case RACE_VEGMAN:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
    
}

bool IsUndead(CHAR_DATA *ch)
{
    if (!ch)
        return (FALSE);
    
    switch (GET_RACE(ch))
    {
    case RACE_UNDEAD:
    case RACE_GHOST:
    case RACE_UNDEAD_VAMPIRE:
    case RACE_UNDEAD_LICH:
    case RACE_UNDEAD_WIGHT:
    case RACE_UNDEAD_GHAST:
    case RACE_UNDEAD_SPECTRE:
    case RACE_UNDEAD_ZOMBIE:
    case RACE_UNDEAD_SKELETON:
    case RACE_UNDEAD_GHOUL:
    case RACE_UNDEAD_SHADOW:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
    
}

bool IsLycanthrope(CHAR_DATA *ch)
{
    if (!ch)
        return (FALSE);

    switch (GET_RACE(ch))
    {
    case RACE_LYCANTH:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
    
}

bool IsDiabolic(CHAR_DATA *ch)
{
    if (!ch)
        return (FALSE);

    switch (GET_RACE(ch))
    {
    case RACE_DEMON:
    case RACE_DEVIL:
    case RACE_DAEMON:
    case RACE_DEMODAND:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
    
}

bool IsReptile(CHAR_DATA *ch)
{
    if (!ch)
        return (FALSE);

    if (IsDragon(ch))
        return (TRUE);

    switch (GET_RACE(ch))
    {
    case RACE_REPTILE:
    case RACE_DINOSAUR:
    case RACE_SNAKE:
    case RACE_TROGMAN:
    case RACE_LIZARDMAN:
    case RACE_SKEXIE:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}

bool HasHands(CHAR_DATA *ch)
{
    if (!ch)
        return (FALSE);
    
    if (IsHumanoid(ch))
        return (TRUE);
    if (IsUndead(ch))
        return (TRUE);
    if (IsLycanthrope(ch))
        return (TRUE);
    if (IsDiabolic(ch))
        return (TRUE);
    if (GET_RACE(ch) == RACE_GOLEM || GET_RACE(ch) == RACE_SPECIAL)
        return (TRUE);
    if (IS_IMMORTAL(ch))
        return (TRUE);
    return (FALSE);
}

bool IsPerson(CHAR_DATA *ch)
{
    if (!ch)
        return (FALSE);

    switch (GET_RACE(ch))
    {
    case RACE_HUMAN:
    case RACE_ELVEN:
    case RACE_DROW:
    case RACE_DWARF:
    case RACE_DARK_DWARF:
    case RACE_HALFLING:
    case RACE_GNOME:
    case RACE_DEEP_GNOME:
    case RACE_GOLD_ELF:
    case RACE_WILD_ELF:
    case RACE_SEA_ELF:
    case RACE_HALF_ELVEN:
    case RACE_GOBLIN:
    case RACE_ORC:
    case RACE_TROLL:
    case RACE_SKEXIE:
    case RACE_MFLAYER:
    case RACE_HALF_ORC:
    case RACE_HALF_OGRE:
    case RACE_HALF_GIANT:
    case RACE_TIEFLING:
    case RACE_GITHZERAI:
    case RACE_BARIAUR:
        return (TRUE);
        break;
        
    default:
        return (FALSE);
        break;
        
    }
}

bool IsGiantish(CHAR_DATA *ch)
{
    if (!ch)
        return (FALSE);
    
    switch (GET_RACE(ch))
    {
    case RACE_ENFAN:
    case RACE_GOBLIN:
    case RACE_ORC:
    case RACE_GIANT:
    case RACE_GIANT_HILL:
    case RACE_GIANT_FROST:
    case RACE_GIANT_FIRE:
    case RACE_GIANT_CLOUD:
    case RACE_GIANT_STORM:
    case RACE_GIANT_STONE:
    case RACE_TYTAN:
    case RACE_TROLL:
    case RACE_DRAAGDIM:
        
    case RACE_HALF_ORC:
    case RACE_HALF_OGRE:
    case RACE_HALF_GIANT:
        return (TRUE);
    default:
        return (FALSE);
        break;
    }
}

bool IsSmall(CHAR_DATA *ch)
{
    if (!ch)
        return (FALSE);
    
    switch (GET_RACE(ch))
    {
    case RACE_SMURF:
    case RACE_GNOME:
    case RACE_HALFLING:
    case RACE_GOBLIN:
    case RACE_ENFAN:
    case RACE_DEEP_GNOME:
        return (TRUE);
    default:
        return (FALSE);
    }
}

bool IsGiant(CHAR_DATA *ch)
{
    if (!ch)
        return (FALSE);
    
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
    case RACE_TYTAN:
    case RACE_GOD:
    case RACE_GIANT_SKELETON:
    case RACE_TROLL:
        return (TRUE);
    default:
        return (FALSE);
    }
}
bool IsExtraPlanar(CHAR_DATA *ch)
{
    if (!ch)
        return (FALSE);

    if (IS_NPC(ch) && IS_ACT2_FLAG(ch, ACT2_PLANAR))
        return (TRUE);
    if (!IS_NPC(ch) && IS_PLR2_FLAG(ch, PLR2_PLANAR))
        return (TRUE);

    switch (GET_RACE(ch))
    {
    case RACE_DEMON:
    case RACE_DEVIL:
    case RACE_PLANAR:
    case RACE_ELEMENT:
    case RACE_ASTRAL:
    case RACE_GOD:
    case RACE_TIEFLING:
    case RACE_AASIMAR:
    case RACE_SOLAR:
    case RACE_PLANITAR:
    case RACE_DEVAS:
    case RACE_TARASQUE:
    case RACE_DIETY:
    case RACE_VAGABOND:
    case RACE_GITHZERAI:
    case RACE_GITHYANKI:
    case RACE_BARIAUR:
    case RACE_MODRON:
    case RACE_DABUS:
    case RACE_CRANIUM_RAT:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}
bool IsOther(CHAR_DATA *ch)
{
    if (!ch)
        return (FALSE);
    
    switch (GET_RACE(ch))
    {
    case RACE_MFLAYER:
    case RACE_SPECIAL:
    case RACE_GOLEM:
    case RACE_ELEMENT:
    case RACE_PLANAR:
    case RACE_LYCANTH:
        return (TRUE);
    default:
        return (FALSE);
        break;
    }
}

bool IsGodly(CHAR_DATA *ch)
{
    
    if (!ch)
        return (FALSE);
    
    if (GetMaxLevel(ch) >= LEVEL_DEMI)
        return (TRUE);

    switch (GET_RACE(ch))
    {
    case RACE_GOD:
    case RACE_DEMON:
    case RACE_DEVIL:
    case RACE_AASIMAR:
    case RACE_SOLAR:
    case RACE_PLANITAR:
    case RACE_DEVAS:
    case RACE_TARASQUE:
    case RACE_DIETY:
        return (TRUE);
    default:
        return (FALSE);
    }
}

bool IsDragon(CHAR_DATA *ch)
{

    if (!ch)
        return (FALSE);
    
    switch (GET_RACE(ch))
    {
    case RACE_DRAGON:
    case RACE_DRAGON_RED:
    case RACE_DRAGON_BLACK:
    case RACE_DRAGON_GREEN:
    case RACE_DRAGON_WHITE:
    case RACE_DRAGON_BLUE:
    case RACE_DRAGON_SILVER:
    case RACE_DRAGON_GOLD:
    case RACE_DRAGON_BRONZE:
    case RACE_DRAGON_COPPER:
    case RACE_DRAGON_BRASS:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}

bool IsGoodSide(CHAR_DATA *ch)
{
    if (!IS_NPC(ch) && IS_IMMORTAL(ch))
        return (FALSE);

    if (HAS_CLASS(ch, CLASS_VAMPIRE))
        return (FALSE);

    switch (GET_RACE(ch))
    {
    case RACE_ELVEN:
    case RACE_GOLD_ELF:
    case RACE_WILD_ELF:
    case RACE_SEA_ELF:
    case RACE_DWARF:
    case RACE_HALFLING:
    case RACE_GNOME:
    case RACE_HALF_OGRE:
    case RACE_HALF_ORC:
    case RACE_HALF_GIANT:
    case RACE_AASIMAR:
        return (TRUE);
    case RACE_HALF_ELVEN:
        if (IS_NPC(ch) && IS_ACT2_FLAG(ch, ACT2_PLANAR))
            return (FALSE);
        if (!IS_NPC(ch) && IS_PLR2_FLAG(ch, PLR2_PLANAR))
            return (FALSE);
        return (TRUE);
    }
    
    return (FALSE);
}

bool IsBadSide(CHAR_DATA *ch)
{
    if (!IS_NPC(ch) && IS_IMMORTAL(ch))
        return (FALSE);

    if (IsDragon(ch) || IsUndead(ch) || IsDiabolic(ch))
        return (TRUE);

    if (HAS_CLASS(ch, CLASS_VAMPIRE))
        return (TRUE);

    switch (GET_RACE(ch))
    {
    case RACE_GNOLL:
    case RACE_LIZARDMAN:
    case RACE_SKEXIE:
    case RACE_DEEP_GNOME:
    case RACE_GOBLIN:
    case RACE_DARK_DWARF:
    case RACE_ORC:
    case RACE_TROLL:
    case RACE_DROW:
    case RACE_UNDEAD_VAMPIRE:
    case RACE_DEVIL:
    case RACE_GIANT:
    case RACE_GIANT_HILL:
    case RACE_GIANT_FROST:
    case RACE_GIANT_FIRE:
    case RACE_GIANT_CLOUD:
    case RACE_GIANT_STORM:
    case RACE_GIANT_STONE:
        return (TRUE);
    case RACE_MFLAYER:
        if (IS_NPC(ch) && IS_ACT2_FLAG(ch, ACT2_PLANAR))
            return (FALSE);
        if (!IS_NPC(ch) && IS_PLR2_FLAG(ch, PLR2_PLANAR))
            return (FALSE);
        return (TRUE);
        break;
    }
    
    return (FALSE);
}

bool IsNeutralSide(CHAR_DATA *ch)
{
    if (!IS_NPC(ch) && IS_IMMORTAL(ch))
        return (TRUE);
    if (!IsBadSide(ch) && !IsGoodSide(ch))
        return (TRUE);
    return (FALSE);
}

int race_bodyparts(CHAR_DATA *ch)
{
    int xflags = 0;
    
    if (race_table[GET_RACE(ch)].body_parts)
        return(race_table[GET_RACE(ch)].body_parts);

    SET_BIT(xflags,PART_GUTS);
    
    if (!ch)
        return(xflags);
    
    if (IsHumanoid(ch) || IsPerson(ch))
        SET_BIT(xflags,
                PART_HEAD|PART_ARMS|PART_LEGS|PART_FEET| \
                PART_FINGERS|PART_EAR|PART_EYE| \
                PART_BRAINS|PART_GUTS|PART_HEART| \
                PART_CHEST|PART_STOMACH
               );

    if (IsRideable(ch) || IsAnimal(ch))
        SET_BIT(xflags,
                PART_HEAD|PART_ARMS|PART_LEGS|PART_FEET| \
                PART_EAR|PART_EYE|PART_BRAINS|PART_GUTS|PART_HEART| \
                PART_CHEST|PART_STOMACH
               );
    
    if (IsUndead(ch))
        REMOVE_BIT(xflags,
                   PART_BRAINS|PART_HEART|PART_EYE
                  );
    
    if (IsDragon(ch))
        SET_BIT(xflags,
                PART_WINGS|PART_TAIL|PART_SCALES| \
                PART_CLAWS|PART_TAILATTACK
               );
    
    if (IsReptile(ch))
        SET_BIT(xflags,
                PART_TAIL|PART_SCALES|PART_CLAWS|PART_FORELEGS
               );
    
    if (GET_RACE(ch) == RACE_SPECIAL) /* beholders */
	SET_BIT(xflags,
		PART_HEAD|PART_EYESTALKS|PART_EYE
	       );
    
    if (HasHands(ch))
        SET_BIT(xflags,PART_HANDS);
    else
        REMOVE_BIT(xflags,PART_HANDS);
    
    return(xflags);
}

int EqWBits(CHAR_DATA *ch, int bit)
{
    OBJ_DATA *obj;
    
    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
        if (obj->wear_loc!=WEAR_NONE && IS_OBJ_STAT(obj,bit))
            return 1;
    
    return 0;
}

int EqWBits2(CHAR_DATA *ch, int bit)
{
    OBJ_DATA *obj;
    
    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
        if (obj->wear_loc!=WEAR_NONE && IS_OBJ_STAT2(obj,bit))
            return 1;
    
    return 0;
}

void update_speaks(CHAR_DATA *ch)
{
    int langs, sn;

    ch->speaks = race_table[GET_RACE(ch)].language;
    for ( langs = 0; lang_array[langs] != LANG_UNKNOWN; langs++ )
    {
        if ((sn=find_tongue(ch, lang_names[langs], TRUE))>-1)
        {
            /*log_printf_plus(LOG_DEBUG, GetMaxLevel(ch), SEV_SPAM, "knows: %s, sn: %d, learned: %d", lang_names[langs], sn, LEARNED(ch, sn));*/
            SET_BIT(ch->speaks, lang_array[langs]);
        }
    }
}
