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
 *			   Wizard/god command module			    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: act_wiz.c,v 1.166 2004/04/06 22:00:08 dotd Exp $";*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mud.h"
#include "gsn.h"
#include "mxp.h"
#include "currency.h"
#include "christen.h"
#include "rareobj.h"
#ifdef IRC
#include "irc.h"
#endif

#define RESTORE_INTERVAL 21600

char * const save_flag[] =
{ "death", "kill", "passwd", "drop", "put", "give", "auto", "zap",
"auction", "get", "receive", "idle", "backup", "r13", "r14", "r15", "r16",
"r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27",
"r28", "r29", "r30", "r31" };


/*
 * Local functions.
 */
void              save_banlist  args( ( void ) );

int               get_color (char *argument); /* function proto */

/*
 * Global variables.
 */

char reboot_time[50];
time_t new_boot_time_t;
extern struct tm new_boot_struct;
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
extern bool mud_down;
extern bool wizlock;

extern int top_room_vnum;
extern int top_obj_vnum;

ALIAS_DATA	*find_alias	args( ( CHAR_DATA *ch, char *argument ) );

int	get_npc_race	args( ( char *type ) );

DECLARE_DO_FUN(do_help);
DECLARE_DO_FUN(do_cedit);
DECLARE_DO_FUN(do_quit);
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_goto);
DECLARE_DO_FUN(do_auction);
DECLARE_DO_FUN(do_restore);
DECLARE_DO_FUN(do_save);
DECLARE_DO_FUN(do_makewizlist);
DECLARE_DO_FUN(do_sit);
DECLARE_DO_FUN(do_say);
DECLARE_DO_FUN(do_recall);
DECLARE_DO_FUN(do_restoretime);
DECLARE_DO_FUN(do_rent);

int get_saveflag( char *name )
{
    unsigned int x;

    for ( x = 0; x < sizeof(save_flag) / sizeof(save_flag[0]); x++ )
        if ( !str_cmp( name, save_flag[x] ) )
            return x;
    return -1;
}

void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    CMDTYPE * cmd;
    int col, c, loopcolor, hash, lev=0, color;
    bool only = FALSE;

    if (!(lev = atoi(argument)))
    {
        if (*argument=='=' && (lev = atoi(argument+1)))
            only = TRUE;
        else
            lev = get_trust(ch);
    }
    else
        lev = UMIN(get_trust(ch), lev);

    col = 0;
    set_pager_color( AT_PLAIN, ch );

    for ( c = 0; c < 12; c++ )
    {
        switch (c)
        {
        case 0: loopcolor = AT_DANGER; break;
        case 1: loopcolor = AT_WHITE; break;
        case 2: loopcolor = AT_YELLOW; break;
        case 3: loopcolor = AT_DGREEN; break;
        case 4: loopcolor = AT_PURPLE; break;
        case 5: loopcolor = AT_PINK; break;
        case 6: loopcolor = AT_CYAN; break;
        case 7: loopcolor = AT_GREEN; break;
        case 8: loopcolor = AT_LBLUE; break;
        case 9: loopcolor = AT_BLUE; break;
        case 10: loopcolor = AT_ORANGE; break;
        default: loopcolor = AT_PLAIN; break;
        }
        for ( hash = 0; hash < 126; hash++ )
            for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
            {
                if ( !only && ( cmd->level < LEVEL_HERO ||
                                cmd->level > lev ) )
                    continue;
                if ( only && cmd->level != lev )
                    continue;

                if (IS_SET(cmd->flags, CMD_EXPERIMENTAL))
                    color = AT_DANGER;
                else if (IS_SET(cmd->flags, CMD_SYSTEM))
                    color = AT_WHITE;
                else if (IS_SET(cmd->flags, CMD_COMMUNICATION))
                    color = AT_YELLOW;
                else if (IS_SET(cmd->flags, CMD_UTILITY))
                    color = AT_DGREEN;
                else if (IS_SET(cmd->flags, CMD_CREATION))
                    color = AT_PURPLE;
                else if (IS_SET(cmd->flags, CMD_DELETION))
                    color = AT_PINK;
                else if (IS_SET(cmd->flags, CMD_MODIFICATION))
                    color = AT_CYAN;
                else if (IS_SET(cmd->flags, CMD_MAINTENANCE))
                    color = AT_GREEN;
                else if (IS_SET(cmd->flags, CMD_BUILDING))
                    color = AT_LBLUE;
                else if (IS_SET(cmd->flags, CMD_INFORMATIONAL))
                    color = AT_BLUE;
                else if (IS_SET(cmd->flags, CMD_IMC))
                    color = AT_ORANGE;
                else
                    color = AT_PLAIN;

                if (color != loopcolor)
                    continue;

                pager_printf( ch, "%s%-13s%s",
                              color_str(color, ch),
                              cmd->name,
                              color_str(AT_PLAIN, ch) );
                if ( ++col % 6 == 0 )
                    send_to_pager( "\n\r", ch );
            }
    }
    if ( col % 6 != 0 )
        send_to_pager( "\n\r", ch );
    return;
}

void do_restrict( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    sh_int level, hash;
    CMDTYPE *cmd;
    bool found;

    found = FALSE;

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Restrict which command?\n\r", ch );
        return;
    }

    argument = one_argument ( argument, arg2 );
    if ( arg2[0] == '\0' )
        level = get_trust( ch );
    else
        level = atoi( arg2 );

    level = UMAX( UMIN( get_trust( ch ), level ), 0 );

    hash = arg[0] % 126;
    for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
    {
        if ( !str_prefix( arg, cmd->name )
             &&    cmd->level <= get_trust( ch ) )
        {
            found = TRUE;
            break;
        }
    }

    if ( found )
    {
        if ( !str_prefix( arg2, "show" ) )
        {
            sprintf(buf, "%s show", cmd->name);
            do_cedit(ch, buf);
            /*    		ch_printf( ch, "%s is at level %d.\n\r", cmd->name, cmd->level );*/
            return;
        }
        cmd->level = level;
        ch_printf( ch, "You restrict %s to level %d\n\r",
                   cmd->name, level );
        sprintf( log_buf, "%s restricting %s to level %d",
                 ch->name, cmd->name, level );
        log_string_plus( log_buf, LOG_MONITOR, LEVEL_LOG_CSET, SEV_NOTICE );
    }
    else
        send_to_char( "You may not restrict that command.\n\r", ch );

    return;
}

/*
 * Check if the name prefix uniquely identifies a char descriptor
 */
CHAR_DATA *get_waiting_desc( CHAR_DATA *ch, char *name )
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA       *ret_char=NULL;
    static unsigned int number_of_hits;

    number_of_hits = 0;
    for ( d = first_descriptor; d; d = d->next )
    {
        if ( d->character && (!str_prefix( name, d->character->name )) &&
             IS_WAITING_FOR_AUTH(d->character) )
        {
            if ( ++number_of_hits > 1 )
            {
                ch_printf( ch, "%s does not uniquely identify a char.\n\r", name );
                return NULL;
            }
            ret_char = d->character;       /* return current char on exit */
        }
    }
    if ( number_of_hits==1 )
        return ret_char;
    else
    {
        send_to_char( "No one like that waiting for authorization.\n\r", ch );
        return NULL;
    }
}

void do_authorize( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Usage:  authorize <player> <yes|name|no/deny>\n\r", ch );
        send_to_char( "Pending authorizations:\n\r", ch );
        send_to_char( " Chosen Character Name\n\r", ch );
        send_to_char( "---------------------------------------------\n\r", ch );
        for ( d = first_descriptor; d; d = d->next )
            if ( (victim = d->character) != NULL && IS_WAITING_FOR_AUTH(victim) )
                ch_printf( ch, " %s@%s new %s %s...\n\r",
                           victim->name,
                           victim->desc->host,
                           race_table[victim->race].race_name,
                           GetClassString(victim) );
        return;
    }

    victim = get_waiting_desc( ch, arg1 );
    if ( victim == NULL )
        return;

    if ( arg2[0]=='\0' || !str_cmp( arg2,"accept" ) || !str_cmp( arg2,"yes" ))
    {
        victim->pcdata->auth_state = 3;
        if ( victim->pcdata->authed_by )
            STRFREE( victim->pcdata->authed_by );
        victim->pcdata->authed_by = QUICKLINK( ch->name );
        sprintf( log_buf, "%s authorized %s", ch->name,
                 victim->name );
        log_string_plus(log_buf,LOG_MONITOR,LEVEL_IMMORTAL,SEV_NOTICE);
        ch_printf( ch, "You have authorized %s.\n\r", victim->name);

        /* Below sends a message to player when name is accepted - Brittany */

        ch_printf( victim,                                            /* B */
                   "The MUD Administrators have accepted the name %s.\n\r"       /* B */
                   "You will be authorized to enter the realm at the end of "    /* B */
                   "this area.\n\r",victim->name);                               /* B */
        return;
    }
    else if ( !str_cmp( arg2, "no" ) || !str_cmp( arg2, "deny" ) )
    {
        send_to_char( "You have been denied access.\n\r", victim);
        sprintf( log_buf, "%s denied authorization to %s", ch->name,
                 victim->name );
        log_string_plus(log_buf,LOG_MONITOR,LEVEL_IMMORTAL,SEV_NOTICE);
        ch_printf( ch, "You have denied %s.\n\r", victim->name);
        do_quit(victim, "yes");
    }

    else if ( !str_cmp( arg2, "name" ) || !str_cmp(arg2, "n" ) )
    {
        victim->pcdata->auth_state = 2;
        sprintf( log_buf, "%s has denied %s's name", ch->name,
                 victim->name );
        log_string_plus(log_buf,LOG_MONITOR,LEVEL_IMMORTAL,SEV_NOTICE);
        ch_printf (victim,
                   "The MUD Administrators have found the name %s "
                   "to be unacceptable.\n\r"
                   "You may choose a new name when you reach "               /* B */
                   "the end of this area.\n\r"                               /* B */
                   "The name you choose must be medieval and original.\n\r"
                   "No titles, descriptive words, or names close to any existing "
                   "Immortal's name.\n\r", victim->name);
        ch_printf( ch, "You requested %s change names.\n\r", victim->name);
        return;
    }

    else
    {
        send_to_char("Invalid argument.\n\r", ch);
        return;
    }
}

void do_bamfin( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );
        STRFREE( ch->pcdata->bamfin );
        ch->pcdata->bamfin = STRALLOC( argument );
        send_to_char( "Ok.\n\r", ch );
    }
    return;
}



void do_bamfout( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );
        STRFREE( ch->pcdata->bamfout );
        ch->pcdata->bamfout = STRALLOC( argument );
        send_to_char( "Ok.\n\r", ch );
    }
    return;
}

void do_rank( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
        return;

    if ( !argument || argument[0] == '\0' )
    {
        send_to_char( "Usage: rank <string>.\n\r", ch );
        send_to_char( "   or: rank none.\n\r", ch );
        return;
    }

    smash_tilde( argument );
    DISPOSE( ch->pcdata->rank );
    if ( !str_cmp( argument, "none" ) )
        ch->pcdata->rank = str_dup( "" );
    else
        ch->pcdata->rank = str_dup( argument );
    send_to_char( "Ok.\n\r", ch );

    return;
}


void do_retire( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Retire whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( GetMaxLevel(victim) < LEVEL_SAVIOR )
    {
        send_to_char( "The minimum level for retirement is savior.\n\r", ch );
        return;
    }

    if ( IS_RETIRED( victim ) )
    {
        REMOVE_BIT( victim->pcdata->flags, PCFLAG_RETIRED );
        ch_printf( ch, "%s returns from retirement.\n\r", victim->name );
        ch_printf( victim, "%s brings you back from retirement.\n\r", ch->name );
    }
    else
    {
        SET_BIT( victim->pcdata->flags, PCFLAG_RETIRED );
        ch_printf( ch, "%s is now a retired immortal.\n\r", victim->name );
        ch_printf( victim, "Courtesy of %s, you are now a retired immortal.\n\r", ch->name );
    }
    return;
}

void do_deny( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Deny whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    SET_BIT(victim->act, PLR_DENY);
    send_to_char( "You are denied access!\n\r", victim );
    send_to_char( "OK.\n\r", ch );
    do_quit( victim, "yes" );

    return;
}



void do_disconnect( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Disconnect whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim->desc == NULL )
    {
        act( AT_PLAIN, "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( get_trust(ch) <= get_trust( victim ) )
    {
        send_to_char( "They might not like that...\n\r", ch );
        return;
    }

    close_socket( victim->desc, FALSE );
    send_to_char( "Ok.\n\r", ch );
    return;
}

/*
 * Force a level one player to quit.             Gorog
 */
void do_fquit( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg1[MAX_INPUT_LENGTH];
    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Force whom to quit?\n\r", ch );
        return;
    }

    if ( !( victim = get_char_world( ch, arg1 ) ) )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( GetMaxLevel(victim) != 1 )
    {
        send_to_char( "They are not level one!\n\r", ch );
        return;
    }

    send_to_char( "The MUD administrators force you to quit\n\r", victim );
    do_quit (victim, "yes");
    send_to_char( "Ok.\n\r", ch );
    return;
}


void do_forceclose( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
#ifndef MUD_LISTENER
    int descriptor;
#else
    int uid;
#endif

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
#ifndef MUD_LISTENER
        send_to_char( "Usage: forceclose <descriptor#>\n\r", ch );
#else
        send_to_char( "Usage: forceclose <uid>\n\r", ch );
#endif
        return;
    }
#ifndef MUD_LISTENER
    descriptor = atoi( arg );
#else
    uid = atoi( arg );
#endif

    for ( d = first_descriptor; d; d = d->next )
    {
#ifndef MUD_LISTENER
        if ( d->descriptor == descriptor )
#else
        if ( d->uid == uid )
#endif
        {
            if ( d->character && get_trust(d->character) >= get_trust(ch) )
            {
                send_to_char( "They might not like that...\n\r", ch );
                return;
            }
            close_socket( d, FALSE );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
    }

    send_to_char( "Not found!\n\r", ch );
    return;
}



void do_pardon( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Syntax: pardon <character> <killer|thief|attacker>.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "attacker" ) )
    {
        if ( IS_SET(victim->act, PLR_ATTACKER) )
        {
            REMOVE_BIT( victim->act, PLR_ATTACKER );
            send_to_char( "Attacker flag removed.\n\r", ch );
            send_to_char( "You are no longer an ATTACKER.\n\r", victim );
        }
        return;
    }

    if ( !str_cmp( arg2, "killer" ) )
    {
        if ( IS_SET(victim->act, PLR_KILLER) )
        {
            REMOVE_BIT( victim->act, PLR_KILLER );
            send_to_char( "Killer flag removed.\n\r", ch );
            send_to_char( "You are no longer a KILLER.\n\r", victim );
        }
        return;
    }

    if ( !str_cmp( arg2, "thief" ) )
    {
        if ( IS_SET(victim->act, PLR_THIEF) )
        {
            REMOVE_BIT( victim->act, PLR_THIEF );
            send_to_char( "Thief flag removed.\n\r", ch );
            send_to_char( "You are no longer a THIEF.\n\r", victim );
        }
        return;
    }

    send_to_char( "Syntax: pardon <character> <killer|thief>.\n\r", ch );
    return;
}


void echo_to_all( sh_int AT_COLOR, char *argument, sh_int tar )
{
    CHAR_DATA *vch;

    if ( !argument || argument[0] == '\0' )
        return;

    for (vch = first_char; vch; vch = vch->next)
    {
        if (GET_CON_STATE(vch) != CON_PLAYING &&
            GET_CON_STATE(vch) != CON_EDITING)
            continue;

        /* This one is kinda useless except for switched.. */
        if ( tar == ECHOTAR_PC && IS_NPC(vch) )
            continue;
        if ( tar == ECHOTAR_IMM && !IS_IMMORTAL(vch) )
            continue;
        if ( tar == ECHOTAR_OUTSIDE && !IS_OUTSIDE(vch) )
            continue;
        if (MXP_ON(vch))
            send_to_char(MXP_TAG_SECURE, vch);
        set_char_color( AT_COLOR, vch );
        ch_printf(vch, "%s\n\r", argument);
    }
}

void do_echo( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int color;
    int target;
    char *parg;

    if ( IS_SET(ch->act, PLR_NO_EMOTE) )
    {
        send_to_char( "You are noemoted and can not echo.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Echo what?\n\r", ch );
        return;
    }

    if ( (color = get_color(argument)) )
        argument = one_argument(argument, arg);
    parg = argument;
    argument = one_argument(argument, arg);
    if ( !str_cmp( arg, "PC" )
         ||   !str_cmp( arg, "player" ) )
        target = ECHOTAR_PC;
    else if ( !str_cmp( arg, "imm" ) )
        target = ECHOTAR_IMM;
    else
    {
        target = ECHOTAR_ALL;
        argument = parg;
    }
    if ( !color && (color = get_color(argument)) )
        argument = one_argument(argument, arg);
    if ( !color )
        color = AT_IMMORT;
    one_argument(argument, arg);
    if ( !str_cmp( arg, "Thoric" )
         ||   !str_cmp( arg, "Dominus" )
         ||   !str_cmp( arg, "Circe" )
         ||   !str_cmp( arg, "Haus" )
         ||   !str_cmp( arg, "Narn" )
         ||   !str_cmp( arg, "Scryn" )
         ||   !str_cmp( arg, "Damian" )
         ||   !str_cmp( arg, "Blodkai" ))
    {
        ch_printf( ch, "I don't think %s would like that!\n\r", arg );
        return;
    }
    echo_to_all ( color, argument, target );
}

void echo_to_room( sh_int AT_COLOR, ROOM_INDEX_DATA *room, char *argument )
{
    CHAR_DATA *vic;

    for ( vic = room->first_person; vic; vic = vic->next_in_room )
    {
        if (MXP_ON(vic))
            send_to_char(MXP_TAG_SECURE, vic);
        set_char_color( AT_COLOR, vic );
        send_to_char( argument, vic );
        send_to_char( "\n\r",   vic );
    }
}

void do_recho( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int color;

    if ( IS_SET(ch->act, PLR_NO_EMOTE) )
    {
        send_to_char( "You are noemoted and can not recho.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Recho what?\n\r", ch );
        return;
    }

    one_argument( argument, arg );
    if ( !str_cmp( arg, "Thoric" )
         ||   !str_cmp( arg, "Dominus" )
         ||   !str_cmp( arg, "Circe" )
         ||   !str_cmp( arg, "Haus" )
         ||   !str_cmp( arg, "Narn" )
         ||   !str_cmp( arg, "Scryn" )
         ||   !str_cmp( arg, "Blodkai" )
         ||   !str_cmp( arg, "Damian" ) )
    {
        ch_printf( ch, "I don't think %s would like that!\n\r", arg );
        return;
    }
    if ( (color = get_color ( argument )) )
    {
        argument = one_argument ( argument, arg );
        echo_to_room ( color, ch->in_room, argument );
    }
    else
        echo_to_room ( AT_IMMORT, ch->in_room, argument );
}


ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
        return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) != NULL )
        return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
        return obj->in_room;

    if (!str_cmp(arg, "home") && !IS_NPC(ch))
        return get_room_index(ch->pcdata->home);

    return NULL;
}



void do_transfer( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Transfer whom (and where)?\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
        for ( d = first_descriptor; d; d = d->next )
        {
            if ( d->connected == CON_PLAYING
                 &&   d->character != ch
                 &&   d->character->in_room
                 &&   d->newstate != 2
                 &&   can_see( ch, d->character ) )
            {
                char buf[MAX_STRING_LENGTH];
                sprintf( buf, "%s %s", d->character->name, arg2 );
                do_transfer( ch, buf );
            }
        }
        return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
        location = ch->in_room;
    }
    else
    {
        if ( ( location = find_location( ch, arg2 ) ) == NULL )
        {
            send_to_char( "No such location.\n\r", ch );
            return;
        }

        if ( room_is_private( location ) )
        {
            send_to_char( "That room is private right now.\n\r", ch );
            return;
        }
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( GetMaxLevel(victim) > GetMaxLevel(ch) )
    {
        send_to_char("That's not a good idea.\n\r", ch);
        return;
    }

    if (NOT_AUTHED(victim))
    {
        send_to_char( "They are not authorized yet!\n\r", ch);
        return;
    }

    if ( !victim->in_room )
    {
        send_to_char( "They are in limbo.\n\r", ch );
        return;
    }

    if ( GetMaxLevel(ch) < MAX_LEVEL-3 &&
         IS_ROOM_FLAG(ch->in_room, ROOM_DEATH ) )
    {
        log_printf_plus(LOG_MONITOR, UMAX(GetMaxLevel(ch),LEVEL_IMMORTAL), SEV_CRIT,
                        "do_transfer: %s tried to transfer %s to death room",
                        GET_NAME(ch), GET_NAME(victim));
        send_to_char( "You can't transfer people to death rooms.\n\r", ch);
        return;
    }

    if ( victim->fighting )
        stop_fighting( victim, TRUE );
    act( AT_MAGIC, "$n disappears in a cloud of swirling colors.", victim, NULL, NULL, TO_ROOM );
    victim->retran = victim->in_room->vnum;
    char_from_room( victim );
    char_to_room( victim, location );
    act( AT_MAGIC, "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
    if ( ch != victim )
        act( AT_IMMORT, "$n has transferred you.", ch, NULL, victim, TO_VICT );
    do_look( victim, "auto" );
    send_to_char( "Ok.\n\r", ch );
    if (!IS_IMMORTAL(victim) && !IS_NPC(victim)
        &&  !in_hard_range( victim, location->area ) )
        send_to_char("Warning: the player's level is not within the area's level range.\n\r", ch);
}

void do_retran( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char("Retransfer whom?\n\r", ch );
        return;
    }
    if ( !(victim = get_char_world(ch, arg)) )
    {
        send_to_char("They aren't here.\n\r", ch );
        return;
    }
    sprintf(buf, "'%s' %d", victim->name, victim->retran);
    do_transfer(ch, buf);
    return;
}

void do_regoto( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    sprintf(buf, "%d", ch->regoto);
    do_goto(ch, buf);
    return;
}

void do_at( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA *wch;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "At where what?\n\r", ch );
        return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    if ( room_is_private( location ) )
    {
        if ( get_trust( ch ) < LEVEL_GREATER )
        {
            send_to_char( "That room is private right now.\n\r", ch );
            return;
        }
        else
        {
            send_to_char( "Overriding private flag!\n\r", ch );
        }

    }

    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = first_char; wch; wch = wch->next )
    {
        if ( wch == ch )
        {
            char_from_room( ch );
            char_to_room( ch, original );
            break;
        }
    }

    return;
}

void do_rat( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    int Start, End, vnum;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "Syntax: rat <start> <end> <command>\n\r", ch );
        return;
    }

    Start = atoi( arg1 );	End = atoi( arg2 );

    if ( Start < 1 || End < Start || Start > End || Start == End || End > top_room_vnum )
    {
        send_to_char( "Invalid range.\n\r", ch );
        return;
    }

    if ( !str_cmp( argument, "quit" ) )
    {
        send_to_char( "I don't think so!\n\r", ch );
        return;
    }

    original = ch->in_room;
    for ( vnum = Start; vnum <= End; vnum++ )
    {
        if ( (location = get_room_index(vnum)) == NULL )
            continue;
        char_from_room( ch );
        char_to_room( ch, location );
        interpret( ch, argument );
    }

    char_from_room( ch );
    char_to_room( ch, original );
    send_to_char( "Done.\n\r", ch );
    return;
}


void do_rstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    EXIT_DATA *pexit;
    int x;
    char s1[16], s2[16], s3[16], s4[16];

    static char *dir_text[LAST_NORMAL_DIR+1] = { "n", "e", "s", "w", "u", "d", "ne", "nw", "se", "sw", "?" };

    sprintf(s1,"%s",color_str(AT_SCORE,ch));
    sprintf(s2,"%s",color_str(AT_SCORE2,ch));
    sprintf(s3,"%s",color_str(AT_SCORE3,ch));
    sprintf(s4,"%s",color_str(AT_SCORE4,ch));

    one_argument( argument, arg );
    if ( !str_cmp( arg, "exits" ) )
    {
        location = ch->in_room;

        ch_printf( ch, "%sExits for room '%s%s%s' vnum %s%d\n\r",
                   s1, s3, location->name,
                   s1, s2, location->vnum );

        for ( pexit = location->first_exit; pexit; pexit = pexit->next )
        {
            buf[0]='\0';
            for (x=0;x<MAX_EXFLAG;x++)
                if (IS_SET(pexit->exit_info,1<<x))
                {
                    strcat(buf,ex_flags[x]);
                    strcat(buf," ");
                }
            ch_printf( ch,
                       "%s%2d%s) %s%2s %sto %s%-5d  %sKey: %s%2d  \n\r%sFlags: %s%s\n\r%sKeywords: '%s%s%s'\n\r%sDescription: %s%s%sExit links back to vnum: %s%d  %sExit's RoomVnum: %s%d  %sDistance: %s%d\n\r",
                       s2, pexit->vdir,
		       s1, s3, pexit->vdir<=LAST_NORMAL_DIR?dir_text[pexit->vdir]:exit_name(pexit),
                       s1, s2, pexit->to_room ? pexit->to_room->vnum : 0,
                       s1, s2, pexit->key,
                       s1, s2, buf,
                       s1, s3, pexit->keyword,
                       s1, s1, s3, pexit->description[0] != '\0'
                       ? pexit->description : "(none)\n\r",
                       s1, s2, pexit->rexit ? pexit->rexit->vnum : 0,
                       s1, s2, pexit->rvnum,
                       s1, s2, pexit->distance );
        }
        return;
    }
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( !location )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    if ( ch->in_room != location && room_is_private( location ) )
    {
        if ( get_trust( ch ) < LEVEL_GREATER )
        {
            send_to_char( "That room is private right now.\n\r", ch );
            return;
        }
        else
        {
            send_to_char( "Overriding private flag!\n\r", ch );
        }

    }

    ch_printf( ch, "%sName: %s%s  %sSpecial: %s%s\n\r%sArea: %s%s  ",
               s1, s3, location->name,
               s1, s3, location->spec_fun ? r_lookup_spec(location->spec_fun) : "none",
               s1, s3, location->area ? location->area->name : "None????" );

#ifndef USE_DB
    ch_printf( ch, "%sFilename: %s%s\n\r",
               s1, s3, location->area ? location->area->filename : "None????" );
#endif

    ch_printf( ch, "%sVnum: %s%d  %sCurrency vnum: %s%d\n\r%sSector: %s%s  %sLight: %s%d  %sTeleDelay: %s%d  %sTeleVnum: %s%d  %sTunnel: %s%d\n\r",
               s1, s2, location->vnum,
               s1, s2, location->currindex ? location->currindex->vnum : -1,
               s1, s2, sect_types[location->sector_type],
               s1, room_is_dark(location)?color_str(AT_DGREY,ch):s2, location->light,
               s1, s2, location->tele_delay,
               s1, s2, location->tele_vnum,
               s1, s2, location->tunnel );

    ch_printf( ch, "%sElevation: %s%d  %sDepth: %s%d  %sSpeed: %s%d\n\r",
               s1, s2, location->elevation,
               s1, s2, location->river ? location->river->depth : 0,
               s1, s2, location->river ? location->river->speed : 0);

    ch_printf( ch, "%sRoom flags: %s%s\n\r",
               s1, s3, flag_string(location->room_flags, r_flags) );
    if (location->description)
    {
      ch_printf( ch, "%sDescription:\n\r%s%s",
               s1, s3, location->description );
    }
    else ch_printf( ch, "%sDescription:%s (NULL)\n\r", s1, s3);

    if ( location->first_extradesc )
    {
        EXTRA_DESCR_DATA *ed;

        ch_printf( ch, "%sExtra description keywords: '%s", s1, s3 );
        for ( ed = location->first_extradesc; ed; ed = ed->next )
        {
            send_to_char( ed->keyword, ch );
            if ( ed->next )
                send_to_char( " ", ch );
        }
        ch_printf( ch, "%s'\n\r", s1 );
    }

    ch_printf( ch, "%sCharacters:%s", s1, s3 );
    for ( rch = location->first_person; rch; rch = rch->next_in_room )
    {
        if ( can_see( ch, rch ) )
        {
	    char buf2[128];
            one_argument( rch->name, buf );
	    if (IS_NPC(rch) && rch->vnum)
		sprintf(buf2, "(%d)", rch->vnum);
	    else if (IS_NPC(rch))
		sprintf(buf2, "(%s)", "NP");
	    else
		sprintf(buf2, "(%s)", "PC");
	    ch_printf(ch, " %s%s", buf, buf2);
        }
    }

    ch_printf( ch, "\n\r%sObjects:   %s", s1, s3 );
    for ( obj = location->first_content; obj; obj = obj->next_content )
    {
	char buf2[128];
        one_argument( obj->name, buf );
	if (obj->vnum)
	    sprintf(buf2, "(%d)", obj->vnum);
	else
	    sprintf(buf2, "(%s)", "NP");
	ch_printf(ch, " %s%s", buf, buf2);
    }
    send_to_char( "\n\r", ch );

    if ( location->first_exit )
    {
        ch_printf( ch, "%s------------------- EXITS -------------------\n\r", s1 );
        for ( pexit = location->first_exit; pexit; pexit = pexit->next )
            ch_printf( ch,
                       "%s%2d%s) %s%-2s %sto %s%-5d  %sKey: %s%2d  %sFlags: %s%2d  %sKeywords: %s%s\n\r",
                       s2, pexit->vdir,
		       s1, s3, pexit->vdir<=LAST_NORMAL_DIR?dir_text[pexit->vdir]:exit_name(pexit),
                       s1, s2, pexit->to_room ? pexit->to_room->vnum : 0,
                       s1, s2, pexit->key,
                       s1, s2, pexit->exit_info,
                       s1, s3, pexit->keyword[0] != '\0' ? pexit->keyword : "(none)" );
    }
    return;
}



void do_ostat( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;
    char s1[16], s2[16], s3[16], s4[16];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Ostat what?\n\r", ch );
        return;
    }
    if ( arg[0] != '\'' && arg[0] != '"' && strlen(argument) > strlen(arg) )
        strcpy( arg, argument );

    if ( ( obj = get_obj_world( ch, arg ) ) == NULL )
    {
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
        return;
    }

    sprintf(s1,"%s",color_str(AT_SCORE,ch));
    sprintf(s2,"%s",color_str(AT_SCORE2,ch));
    sprintf(s3,"%s",color_str(AT_SCORE3,ch));
    sprintf(s4,"%s",color_str(AT_SCORE4,ch));

    ch_printf( ch, "%sName: %s%s  %sArea: %s%s  %sSpecial: %s%s\n\r",
               s1, s3, obj->name,
               s1, s3, obj->pIndexData->area?obj->pIndexData->area->name:"(none)",
               s1, s3, obj->spec_fun ? o_lookup_spec(obj->spec_fun) : "(none)" );

    ch_printf( ch, "%sVnum: %s%d  %sUnum: %s%d  %sType: %s%s  %sCount: %s%d  %sGcount: %s%d  %sSerial#: %s%d\n\r",
               s1, s2, obj->vnum,
               s1, s2, obj->unum,
               s1, s3, item_type_name( obj ),
               s1, s2, obj->pIndexData->count,
               s1, s4, obj->count,
               s1, s2, obj->serial );

    ch_printf( ch, "%sShort description: %s%s\n\r%sLong description: %s%s\n\r",
               s1, s3, obj->short_descr,
               s1, s3, obj->description );

    if ( obj->action_desc[0] != '\0' )
        ch_printf( ch, "%sAction description: %s%s\n\r",
                   s1, s3, obj->action_desc );

    if ( obj->last_carried_by && obj->last_carried_by[0] != '\0' )
        ch_printf( ch, "%sLast carried by: %s%s\n\r",
                   s1, s3, obj->last_carried_by );

    if ( obj->christened )
    {
        ch_printf( ch, "%sChristened: %s%s  %sBy: %s%s  %sCvnum/Ovnum: %s%d%s/%s%d\n\r  %sTime christened: %s%24.24s\n\r",
                   s1, s3, get_christen_name(obj),
                   s1, s3, obj->christened->owner,
                   s1, s2, obj->christened->cvnum,
                   s1, s4, obj->christened->ovnum,
                   s1, s3, ctime(&obj->christened->when));
    }

    if (obj->wear_flags)
        ch_printf( ch, "%sWear flags : %s%s\n\r", s1, s3, flag_string(obj->wear_flags, w_flags) );
    if (obj->extra_flags)
        ch_printf( ch, "%sExtra flags: %s%s\n\r", s1, s3, flag_string(obj->extra_flags, o_flags) );
    if (obj->extra_flags2)
        ch_printf( ch, "%sExtra flag2: %s%s\n\r", s1, s3, flag_string(obj->extra_flags2, o2_flags) );
    if (obj->magic_flags)
        ch_printf( ch, "%sMagic flags: %s%s\n\r", s1, s3, flag_string(obj->magic_flags, mag_flags) );

    ch_printf( ch, "%sNumber: %s%d%s/%s%d  %sWeight: %s%d%s/%s%d  %sLayers: %s%d\n\r",
               s1, s2, 1,
               s1, s4, get_obj_number( obj ),
               s1, s2, obj->weight,
               s1, s4, get_obj_weight( obj ),
               s1, s2, obj->pIndexData->layers );

    {
        int itemsave=0, x;

        for (x=0;x<100;x++)
            itemsave += ItemSave(obj,TYPE_UNDEFINED);
        ch_printf( ch, "%sCost: %s%d%s/%s%s  %sRent: %s%d  %sTimer: %s%d  %sEgo: %s%d  %sItemSave(TYPE_UNDEFINED)%%: %s%d\n\r",
                   s1, s2, obj->cost,
                   s1, s3, curr_types[obj->currtype],
                   s1, s2, obj->pIndexData->rent,
                   s1, s2, obj->timer,
                   s1, s2, item_ego(obj),
                   s1, s2, itemsave );
    }

    if ( obj->in_room )
        ch_printf( ch,
                   "%sIn Room: %s%d %s(%s%s%s)\n\r",
                   s1, s2, obj->in_room->vnum,
                   s1, s3, obj->in_room->name,
                   s1);
    else if ( obj->in_obj )
        ch_printf( ch,
                   "%sIn Object: %s%s\n\r",
                   s1, s3, obj->in_obj->short_descr );
    else if ( obj->carried_by )
        ch_printf( ch,
                   "%sCarried by: %s%s\n\r",
                   s1, s3, obj->carried_by->name );

    if (GetMaxLevel(ch) == MAX_LEVEL)
        ch_printf( ch,
                   "%sRoom: %s%p  %sObject: %s%p  %sCarried: %s%p\n\r",
                   s1, s2, obj->in_room,
                   s1, s2, obj->in_obj,
                   s1, s2, obj->carried_by);

    if ( obj->wear_loc != WEAR_NONE )
        ch_printf( ch,
                   "%sWear_loc: %s%s\n\r",
                   s1, s3, wear_locs[obj->wear_loc] );

    ch_printf( ch, "%sIndex Values : %s%-3d %s%-3d %s%-3d %s%-3d %s%-3d %s%-3d\n\r",
               s1, s2, obj->pIndexData->value[0],
               s4, obj->pIndexData->value[1],
               s2, obj->pIndexData->value[2],
               s4, obj->pIndexData->value[3],
               s2, obj->pIndexData->value[4],
               s4, obj->pIndexData->value[5] );
    ch_printf( ch, "%sObject Values: %s%-3d %s%-3d %s%-3d %s%-3d %s%-3d %s%-3d%s\n\r",
               s1, s2, obj->value[0],
               s4, obj->value[1],
               s2, obj->value[2],
               s4, obj->value[3],
               s2, obj->value[4],
               s4, obj->value[5],
               s1 );

    if ( obj->pIndexData->first_extradesc )
    {
        EXTRA_DESCR_DATA *ed;

        send_to_char( "Primary description keywords:  ", ch );
        for ( ed = obj->pIndexData->first_extradesc; ed; ed = ed->next )
            ch_printf(ch, " '%s%s%s'", s3, ed->keyword, s1);
        send_to_char( "\n\r", ch);
    }
    if ( obj->first_extradesc )
    {
        EXTRA_DESCR_DATA *ed;

        send_to_char( "Secondary description keywords:", ch );
        for ( ed = obj->first_extradesc; ed; ed = ed->next )
            ch_printf(ch, " '%s%s%s'", s3, ed->keyword, s1);
        send_to_char( "\n\r", ch);
    }

    for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
    {
        send_to_char("-", ch);
        showaffect(ch, paf);
    }

    for ( paf = obj->first_affect; paf; paf = paf->next )
    {
        send_to_char("*", ch);
        showaffect(ch, paf);
    }
}



void do_mstat( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;
    SKILLTYPE *skill;
    int x, i;
    char s1[16], s2[16], s3[16], s4[16];

    set_char_color( AT_PLAIN, ch );

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Mstat whom?\n\r", ch );
        return;
    }
    if ( arg[0] != '\'' && arg[0] != '"' && strlen(argument) > strlen(arg) )
        strcpy( arg, argument );

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
    if ( get_trust( ch ) < get_trust( victim ) && !IS_NPC(victim) )
    {
        set_char_color( AT_IMMORT, ch );
        send_to_char( "Their godly glow prevents you from getting a good look.\n\r", ch );
        return;
    }

    sprintf(s1,"%s",color_str(AT_SCORE,ch));
    sprintf(s2,"%s",color_str(AT_SCORE2,ch));
    sprintf(s3,"%s",color_str(AT_SCORE3,ch));
    sprintf(s4,"%s",color_str(AT_SCORE4,ch));

    ch_printf( ch, "%sName: %s%s  %sWhere: %s%s%s/%s%s\n\r",
               s1, s3, victim->name,
               s1, s3, victim->in_room->name,
               s1, s3, victim->in_room->area->name);

    ch_printf( ch, "%sVnum: %s%d  %sUnum: %s%d  %sSex: %s%s  %sRoom: %s%d  %sCount: %s%d  %sKilled: %s%d\n\r",
               s1, s2, victim->vnum,
               s1, s2, victim->unum,
               s1, s3, victim->sex == SEX_MALE    ? "male"   :
               victim->sex == SEX_FEMALE  ? "female" : "neutral",
               s1, s2, victim->in_room == NULL    ?        0 : victim->in_room->vnum,
               s1, s2, IS_NPC(victim) ? victim->pIndexData->count : 1,
               s1, s2, IS_NPC(victim) ? victim->pIndexData->killed
               : victim->pcdata->mdeaths + victim->pcdata->pdeaths
             );
    ch_printf( ch, "%sStr: %s%d   %sInt: %s%d   %sWis: %s%d   %sDex: %s%d   %sCon: %s%d   %sCha: %s%d   %sLck: %s%d\n\r",
               s1, s2, get_curr_str(victim),
               s1, s2, get_curr_int(victim),
               s1, s2, get_curr_wis(victim),
               s1, s2, get_curr_dex(victim),
               s1, s2, get_curr_con(victim),
               s1, s2, get_curr_cha(victim),
               s1, s2, get_curr_lck(victim) );
    if (IS_VAMPIRE(victim) && !IS_NPC(victim))
        ch_printf( ch, "%sHps: %s%d%s/%s%d%s+%s%d   %sBlood: %s%d%s/%s%d%s+%s%d   %sMove: %s%d%s/%s%d%s+%s%d   %sPractices: %s%d\n\r",
                   s1, s2, GET_HIT(victim),
                   s1, s4, GET_MAX_HIT(victim),
                   s1, s2, victim->hit_regen,
                   s1, s2, GET_COND(victim, COND_BLOODTHIRST),
                   s1, s4, GET_MAX_BLOOD(victim),
                   s1, s2, victim->mana_regen,
                   s1, s2, GET_MOVE(victim),
                   s1, s4, GET_MAX_MOVE(victim),
                   s1, s2, victim->move_regen,
                   s1, s2, victim->practice );
    else
        ch_printf( ch, "%sHps: %s%d%s/%s%d%s+%s%d   %sMana: %s%d%s/%s%d%s+%s%d   %sMove: %s%d%s/%s%d%s+%s%d   %sPractices: %s%d\n\r",
                   s1, s2, GET_HIT(victim),
                   s1, s4, GET_MAX_HIT(victim),
                   s1, s2, victim->hit_regen,
                   s1, s2, GET_MANA(victim),
                   s1, s4, GET_MAX_MANA(victim),
                   s1, s2, victim->mana_regen,
                   s1, s2, GET_MOVE(victim),
                   s1, s4, GET_MAX_MOVE(victim),
                   s1, s2, victim->move_regen,
                   s1, s2, victim->practice );
    ch_printf( ch,
               "%sRace: %s%s  %sAlign: %s%d  %sAC: %s%d  %sXP: %s%d  %sAntiMag: %s%d%%  %sSF: %s%d\n\r",
               s1, s3, race_table[victim->race].race_name,
               s1, s2, victim->alignment,
               s1, s2, GET_AC(victim),
               s1, s2, victim->exp,
               s1, s2, GET_AMAGICP(victim),
               s1, s2, victim->spellfail );
    for (x=0;x<MAX_CURR_TYPE;x++)
        ch_printf(ch, "%s%s: %s%d%s+%s%d ",
                  s1, cap_curr_types[x], s2, GET_MONEY(victim,x),
                  s1, s4, GET_BALANCE(victim,x) );
    ch_printf( ch, "\n\r%sRent: %s%d", s1, s2, calc_rent(victim));
    ch_printf( ch, "  %sEgo: %s%d", s1, s2, char_ego(victim));
    ch_printf( ch, "  %sGML: %s%d", s1, s2, GetMaxLevel(victim));
    ch_printf( ch, "  %sHMC: %s%d", s1, s2, HowManyClasses(victim));
    ch_printf( ch, "  %sCalcThaco: %s%d\n\r", s1, s2, CalcThaco(victim));

    ch_printf( ch, "%sClass : %s%s  %sDeity: %s%s\n\r",
               s1, s3, GetClassString(victim),
               s1, s3, ( IS_NPC( victim ) || !victim->pcdata->deity ) ? "(none)"
               : victim->pcdata->deity->name );

    ch_printf( ch, "%sLevels:", s1);
    for (i = 0; i < MAX_CLASS; ++i)
        if ( GET_LEVEL(victim, i) > 0 )
            ch_printf(ch, " %s%2d", s2, GET_LEVEL(victim, i) );
    ch_printf(ch, "\n\r");

    ch_printf( ch, "%sFighting: %s%s  %sMaster: %s%s  %sLeader: %s%s\n\r",
               s1, s3, victim->fighting ? victim->fighting->who->name : "(none)",
               s1, s3, victim->master      ? victim->master->name   : "(none)",
               s1, s3, victim->leader      ? victim->leader->name   : "(none)" );
    if (victim->hunting)
        ch_printf( ch, "%sHunting: %s%s  %sSince: %s%24.24s\n\r",
                   s1, s3, GET_NAME(victim->hunting->who),
                   s1, s2, ctime(&victim->hunting->start_time) );
    if (victim->hating)
        ch_printf( ch, "%sHating : %s%s  %sSince: %s%24.24s\n\r",
                   s1, s3, GET_NAME(victim->hating->who),
                   s1, s2, ctime(&victim->hating->start_time) );
    if (victim->fearing)
        ch_printf( ch, "%sFearing: %s%s  %sSince: %s%24.24s\n\r",
                   s1, s3, GET_NAME(victim->fearing->who),
                   s1, s2, ctime(&victim->fearing->start_time) );
    ch_printf( ch, "%sHitroll: %s%d%s/%s%d   %sDamroll: %s%d%s/%s%d   %sPosition: %s%d%s/%s%d   %sWimpy: %s%d   %sBare: %s%d%sd%s%d\n\r",
               s1, s2, GET_HITROLL(victim),
	       s1, s4, victim->hitroll,
               s1, s2, GET_DAMROLL(victim),
               s1, s4, victim->damroll,
               s1, s2, victim->position,
               s1, s4, victim->position,
               s1, s2, victim->wimpy,
               s1, s2, victim->barenumdie,
               s1, s4, victim->baresizedie );
    if ( !IS_NPC(victim) )
        ch_printf( ch, "%sThirst: %s%d   %sFull: %s%d   %sDrunk: %s%d\n\r",
                   s1, s2, GET_COND(victim, COND_THIRST),
                   s1, s2, GET_COND(victim, COND_FULL),
                   s1, s2, GET_COND(victim, COND_DRUNK));
    else
        ch_printf( ch, "%sHit dice: %s%d%sd%s%d%s+%s%d  %sDamage dice: %s%d%sd%s%d%s+%s%d\n\r",
                   s1, s2, victim->pIndexData->hitnodice,
                   s1, s4, victim->pIndexData->hitsizedice,
                   s1, s2, victim->pIndexData->hitplus,
                   s1, s2, victim->pIndexData->damnodice,
                   s1, s4, victim->pIndexData->damsizedice,
                   s1, s2, victim->pIndexData->damplus );
    ch_printf( ch, "%sMentalState: %s%d  %sEmotionalState: %s%d  ",
               s1, s2, victim->mental_state,
               s1, s2, victim->emotional_state );
    ch_printf( ch, "%sSaving throws: %s%d %s%d %s%d %s%d %s%d\n\r",
               s1, s2, victim->saving_poison_death,
               s4, victim->saving_wand,
               s2, victim->saving_para_petri,
               s4, victim->saving_breath,
               s2, victim->saving_spell_staff  );
    ch_printf( ch, "%sCarry figures: items %s%d%s(%s%d%s)/%s%d  %sweight %s%d%s(%s%d%s)/%s%d  %sNumattacks: %s%1.3f%s/%s%1.3f\n\r",
               s1, s2, victim->carry_number,
               s1, s2, carry_n(victim),
               s1, s4, can_carry_n(victim),
               s1, s2, victim->carry_weight,
               s1, s2, carry_w(victim),
               s1, s4, can_carry_w(victim),
               s1, s2, (float)victim->numattacks/1000,
               s1, s4, (float)get_numattacks(victim)/1000 );
    ch_printf( ch, "%sYears: %s%d   %sSeconds Played: %s%d   %sTimer: %s%d   %sAct: %s%d   %sAct2: %s%d\n\r",
               s1, s2, get_age(victim),
               s1, s2, (int)victim->played,
               s1, s2, victim->timer,
               s1, s2, victim->act,
               s1, s2, victim->act2 );
    if (IS_NPC(victim)) {
        if (victim->act)
            ch_printf(ch, "%sAct flags: %s%s\n\r",
                      s1, s3, flag_string(victim->act, act_flags));
        if (victim->act2)
            ch_printf(ch, "%sAct2 flags: %s%s\n\r",
                      s1, s3, flag_string(victim->act2, act2_flags));
    }
    else
    {
        if (victim->act)
            ch_printf(ch, "%sPlayer flags: %s%s\n\r",
                      s1, s3, flag_string(victim->act, plr_flags));
        if (victim->act2)
            ch_printf(ch, "%sPlayer2 flags: %s%s\n\r",
                      s1, s3, flag_string(victim->act2, plr2_flags));
    }
    if (victim->affected_by)
        ch_printf(ch, "%sAffected by: %s%s\n\r",
                  s1, s3, flag_string(victim->affected_by, a_flags));
    if (victim->affected_by2 )
        ch_printf(ch, "%sAffected by2: %s%s\n\r",
                  s1, s3, flag_string(victim->affected_by2, a2_flags));
    ch_printf(ch, "%sSpeaks: %s%d   %sSpeaking: %s%d\n\r",
              s1, s2, victim->speaks,
              s1, s4, victim->speaking);
    ch_printf(ch, "%sLanguages: ", s1);
    for ( x = 0; lang_array[x] != LANG_UNKNOWN; x++)
        if (knows_language( victim, lang_array[x], victim))
        {
            if (IS_SET(lang_array[x], victim->speaking) ||
                (IS_NPC(victim) && !victim->speaking))
                set_char_color(AT_RED, ch);
            send_to_char(lang_names[x], ch);
            send_to_char(" ", ch);
            set_char_color(AT_PLAIN, ch);
        }
        else
            if (IS_SET(lang_array[x], victim->speaking) ||
                (IS_NPC(victim) && !victim->speaking))
            {
                set_char_color(AT_PINK, ch);
                send_to_char(lang_names[x], ch);
                send_to_char(" ", ch);
                set_char_color(AT_PLAIN, ch);
            }
    send_to_char("\n\r", ch);
    if ( victim->short_descr && *victim->short_descr )
        ch_printf( ch, "%sShort description: %s%s\n\r",
                   s1, s3, victim->short_descr);
    if ( victim->long_descr && *victim->long_descr )
        ch_printf( ch, "%sLong description : %s%s",
                   s1, s3, victim->long_descr);
    if ( victim->intro_descr && *victim->intro_descr )
        ch_printf( ch, "%sIntro Description: %s%s\n\r",
                   s1, s3, victim->intro_descr );
    if ( IS_NPC(victim) && victim->spec_fun )
        ch_printf( ch, "%sMobile has spec  : %s%s\n\r",
                   s1, s3, m_lookup_spec( victim->spec_fun ) );
    if ( victim->xflags )
        ch_printf( ch, "%sBody Parts : %s%s\n\r",
                   s1, s3, flag_string(victim->xflags, part_flags) );
    if ( victim->resistant )
        ch_printf( ch, "%sResistant  : %s%s\n\r",
                   s1, s3, flag_string(victim->resistant, ris_flags) );
    if ( victim->immune )
        ch_printf( ch, "%sImmune     : %s%s\n\r",
                   s1, s3, flag_string(victim->immune, ris_flags) );
    if ( victim->susceptible )
        ch_printf( ch, "%sSusceptible: %s%s\n\r",
                   s1, s3, flag_string(victim->susceptible, ris_flags) );
    if ( victim->absorb )
        ch_printf( ch, "%sAbsorb     : %s%s\n\r",
                   s1, s3, flag_string(victim->absorb, ris_flags) );
    if ( victim->attacks )
        ch_printf( ch, "%sAttacks    : %s%s\n\r",
                   s1, s3, flag_string(victim->attacks, attack_flags) );
    if ( victim->defenses )
        ch_printf( ch, "%sDefenses   : %s%s\n\r",
                   s1, s3, flag_string(victim->defenses, defense_flags) );
    for ( paf = victim->first_affect; paf; paf = paf->next )
        if ( (skill=get_skilltype(paf->type)) != NULL )
            ch_printf( ch,
                       "%s%s: '%s%s%s' modifies %s%s %sby %s%d %sfor %s%d %srounds with bits %s%s\n\r",
                       s1, skill_tname[skill->type],
                       s3, skill->name, s1,
                       s3, affect_loc_name( paf->location ),
                       s1, s2, paf->modifier,
                       s1, s4, paf->duration,
                       s1, s3, flag_string( paf->bitvector, a_flags )
                     );
    return;
}

void do_pstat(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char s1[16], s2[16], s3[16], s4[16];
    int i;

    set_char_color( AT_PLAIN, ch );

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Pstat whom?\n\r", ch );
        return;
    }
    if ( arg[0] != '\'' && arg[0] != '"' && strlen(argument) > strlen(arg) )
        strcpy( arg, argument );

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if (IS_NPC(victim))
    {
        send_to_char("You can only use pstat on PC's\n\r", ch);
        return;
    }

    if (get_trust( ch ) < get_trust( victim ))
    {
        set_char_color( AT_IMMORT, ch );
        send_to_char( "Their godly glow prevents you from getting a good look.\n\r", ch );
        return;
    }

    sprintf(s1,"%s",color_str(AT_SCORE,ch));
    sprintf(s2,"%s",color_str(AT_SCORE2,ch));
    sprintf(s3,"%s",color_str(AT_SCORE3,ch));
    sprintf(s4,"%s",color_str(AT_SCORE4,ch));


    ch_printf( ch, "%sName: %s%s  %sTrust: %s%d  %sMax/MinLevel: %s%d%s/%s%d  %sArea: %s%s\n\r",
               s1, s3, victim->name,
               s1, s2, get_trust(victim),
               s1, s2, GetMaxLevel(victim),
               s1, s4, GetMinLevel(victim),
               s1, s3, !victim->pcdata->area ? "(none)" :
               victim->pcdata->area->name);

    if ( get_trust(ch) >= LEVEL_GOD && victim->desc )
#ifndef MUD_LISTENER
        ch_printf( ch, "%sUser: %s%s@%s\n\r%sDescriptor: %s%d  %sConnId: %s%d  %sAuthedBy: %s%s\n\r",
#else
        ch_printf( ch, "%sUser: %s%s@%s\n\r%sUID: %s%d  %sConnId: %s%d  %sAuthedBy: %s%s\n\r",
#endif
                   s1, s3, victim->desc->user, victim->desc->host,
#ifndef MUD_LISTENER
                   s1, s2, victim->desc->descriptor,
#else
                   s1, s2, victim->desc->uid,
#endif
                   s1, s2, victim->desc->conn_id,
                   s1, s3, victim->pcdata->authed_by ?
                   victim->pcdata->authed_by : "(unknown)" );

    if ( victim->desc )
        ch_printf( ch, "%sMXP: %s%d  %sMSP: %s%d\n\r",
                   s1, s2,
#ifdef MXP
                   victim->desc->mxp_detected,
#else
                   0,
#endif
                   s1, s2,
#ifdef MSP
                   victim->desc->msp_detected
#else
                   0
#endif
                 );

    ch_printf( ch, "%sClass: %s%s  %sRace: %s%s\n\r%sDeity: %s%s  %sCouncil: %s%s  %sClan: %s%s\n\r",
               s1, s3, GetClassString(victim),
	       s1, s3, race_table[GET_RACE(victim)].race_name,
               s1, s3, victim->pcdata->deity ? victim->pcdata->deity->name : "(none)",
               s1, s3, victim->pcdata->council ? victim->pcdata->council->name : "(none)",
               s1, s3, victim->pcdata->clan ? victim->pcdata->clan->name : "(none)");


    if (!IS_IMMORTAL(victim))
    {
        ch_printf( ch, "%sLevels:\n\r", s1);
        for (i = 0; i < MAX_CLASS; ++i)
            if ( GET_LEVEL(victim, i) > 0 )
                ch_printf(ch, " %s%s: %s%-2d  %s%-10.10s%-10.10s%-10.10s\n\r",
                          s1, short_pc_class[i],
                          s2, GET_LEVEL(victim, i),
                          s1,
                          IS_ACTIVE(victim, i)?"ACTIVE":"",
                          IS_FALLEN(victim, i)?"FALLEN":"",
                          HAD_CLASS(victim, i)?"HADCLASS":"");
    }

    ch_printf( ch, "%sChannels:%s", s1, s3);
    for (i = 0; i < 32; i++)
        if (!IS_SET( victim->deaf, 1<<i))
            ch_printf( ch, " %s", channel_names[i] );
    send_to_char("\n\r", ch);

#ifdef I3
    ch_printf( ch, "%sI3 Channels: %s%s\n\r",
               s1, s3, I3LISTEN(victim)?I3LISTEN(victim):"" );
#endif


    if (victim->pcdata->bamfin || victim->pcdata->bamfout)
        ch_printf( ch, "%sBamfin: %s%s\n\r%sBamfout: %s%s\n\r",
                   s1, s3, victim->pcdata->bamfin,
                   s1, s3, victim->pcdata->bamfout);

    if (victim->pcdata->bestowments)
        ch_printf( ch, "%sBestowments: %s%s\n\r",
                   s1, s3, victim->pcdata->bestowments);

    ch_printf( ch, "%sRank: %s%s\n\r%sTitle: %s%s\n\r%sPrompt: %s%s\n\r",
               s1, s3, victim->pcdata->rank,
               s1, s3, victim->pcdata->title,
               s1, s3, victim->pcdata->prompt);

    ch_printf(ch, "%sTimes played (>5 minutes): %s%d\n\r",
              s1, s2, victim->pcdata->times_played);

    ch_printf(ch, "%sLogged on                  : %s%24.24s\n\r",
              s1, s3, ctime(&victim->logon));

    if (victim->pcdata->release_date)
        ch_printf(ch, "%sHelled until               : %s%24.24s %sby %s%s.\n\r",
                  s1, s3, ctime(&victim->pcdata->release_date),
                  s1, s3, victim->pcdata->helled_by);

    if (victim->pcdata->restore_time)
        ch_printf(ch, "%sLast time restored somebody: %s%24.24s\n\r",
                  s1, s3, ctime(&victim->pcdata->restore_time));

    if (victim->pcdata->time_created)
        ch_printf(ch, "%sTime of Creation           : %s%24.24s\n\r",
                  s1, s3, ctime(&victim->pcdata->time_created));

    if (victim->pcdata->time_immortal)
        ch_printf(ch, "%sBecame Immortal            : %s%24.24s\n\r",
                  s1, s3, ctime(&victim->pcdata->time_immortal));

    if (victim->pcdata->time_to_die)
        ch_printf(ch, "%sTime to Die                : %s%24.24s\n\r",
                  s1, s3, ctime(&victim->pcdata->time_to_die));

    ch_printf( ch, "%sPKills: %s%d  %sIllegal PK: %s%d  %sPDeaths: %s%d  %sMKills: %s%d  %sMDeaths: %s%d\n\r",
               s1, s2, victim->pcdata->pkills,
               s1, s4, victim->pcdata->illegal_pk,
               s1, s2, victim->pcdata->pdeaths,
               s1, s4, victim->pcdata->mkills,
               s1, s2, victim->pcdata->mdeaths);

    ch_printf( ch, "%sHome: %s%d  %sFavor: %s%d  %sGlory: %s%d%s/%s%d  %sPercent of money online: %s%d%%\n\r%sInterface: %s%d  %sPagerLen: %s%d\n\r",
               s1, s2, victim->pcdata->home,
               s1, s4, victim->pcdata->favor,
               s1, s2, victim->pcdata->quest_curr,
               s1, s4, victim->pcdata->quest_accum,
               s1, s2, player_worth_percentage(victim),
               s1, s2, victim->pcdata->interface,
               s1, s4, victim->pcdata->pagerlen);

    if (victim->act)
        ch_printf(ch, "%sPlayer flags: %s%s\n\r",
                  s1, s3, flag_string(victim->act, plr_flags));
    if (victim->act2)
        ch_printf(ch, "%sPlayer2 flags: %s%s\n\r",
                  s1, s3, flag_string(victim->act2, plr2_flags));
    if (victim->pcdata->flags)
        ch_printf(ch, "%sPcflags: %s%s\n\r",
                  s1, s3, flag_string(victim->pcdata->flags, pc_flags));

    ch_printf( ch, "%sRoom range:   %s%5d%s-%s%-5d\n\r%sObject range: %s%5d%s-%s%-5d\n\r%sMob range:    %s%5d%s-%s%-5d\n\r",
               s1, s2, victim->pcdata->r_range_lo,
               s1, s4, victim->pcdata->r_range_hi,
               s1, s2, victim->pcdata->o_range_lo,
               s1, s4, victim->pcdata->o_range_hi,
               s1, s2, victim->pcdata->m_range_lo,
               s1, s4, victim->pcdata->m_range_hi);

#ifdef VTRACK
    ch_printf( ch, "%sExplored rooms  : %s%-6d %s(%s%.2f%%%s)\n\r",
               s1, s2, vtrack_count_room(victim),
               s1, s4, vtrack_percent_room(victim),
               s1 );
    ch_printf( ch, "%sExplored mobiles: %s%-6d %s(%s%.2f%%%s)\n\r",
               s1, s2, vtrack_count_mob(victim),
               s1, s4, vtrack_percent_mob(victim),
               s1 );
    ch_printf( ch, "%sKilled mobiles  : %s%-6d %s(%s%.2f%%%s)\n\r",
               s1, s2, vtrack_count_mobkill(victim),
               s1, s4, vtrack_percent_mobkill(victim),
               s1 );
    ch_printf( ch, "%sExplored objects: %s%-6d %s(%s%.2f%%%s)\n\r",
               s1, s2, vtrack_count_obj(victim),
               s1, s4, vtrack_percent_obj(victim),
               s1 );
#endif

}

void do_mfind( CHAR_DATA *ch, char *argument )
{
    /*  extern int top_mob_index; */
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    /*  int vnum; */
    int hash;
    int nMatch = 0;
    int act = 0, act2 = 0, aff = 0, race = -1;

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Mfind whom?\n\r", ch );
        return;
    }
    set_pager_color( AT_PLAIN, ch );

    if ( !str_cmp(arg, "act") )
    {
        one_argument( argument, arg );
        if ( arg[0] == '\0' )
        {
            send_to_char( "Mfind what act flag?\n\r", ch );
            return;
        }

        act = get_actflag(arg);
        if ( act < 0 )
        {
            ch_printf( ch, "Invalid flag: %s\n\r", arg );
            return;
        }
        arg[0] = '\0';
    }
    else if ( !str_cmp(arg, "act2") )
    {
        one_argument( argument, arg );
        if ( arg[0] == '\0' )
        {
            send_to_char( "Mfind what act flag?\n\r", ch );
            return;
        }

        act2 = get_act2flag(arg);
        if ( act2 < 0 )
        {
            ch_printf( ch, "Invalid flag: %s\n\r", arg );
            return;
        }
        arg[0] = '\0';
    }
    else if ( !str_cmp(arg, "aff") )
    {
        one_argument( argument, arg );
        if ( arg[0] == '\0' )
        {
            send_to_char( "Mfind what act flag?\n\r", ch );
            return;
        }

        aff = get_aflag(arg);
        if ( aff < 0 )
        {
            ch_printf( ch, "Invalid flag: %s\n\r", arg );
            return;
        }
        arg[0] = '\0';
    }
    else if ( !str_cmp(arg, "race") )
    {
        one_argument( argument, arg );
        if ( arg[0] == '\0' )
        {
            send_to_char( "Mfind what race?\n\r", ch );
            return;
        }

	race = get_npc_race(arg);
        if ( race < 0 )
        {
            ch_printf( ch, "Invalid race: %s\n\r", arg );
            return;
        }
        arg[0] = '\0';
    }

    /*
     * This goes through all the hash entry points (1024), and is therefore
     * much faster, though you won't get your vnums in order... oh well. :)
     *
     * Tests show that Furey's method will usually loop 32,000 times, calling
     * get_mob_index()... which loops itself, an average of 1-2 times...
     * So theoretically, the above routine may loop well over 40,000 times,
     * and my routine bellow will loop for as many index_mobiles are on
     * your mud... likely under 3000 times.
     * -Thoric
     */
    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pMobIndex = mob_index_hash[hash];
              pMobIndex;
              pMobIndex = pMobIndex->next )
            if ( (arg[0] != '\0' && nifty_is_name( arg, pMobIndex->player_name )) ||
                 (act != 0 && IS_ACT_FLAG(pMobIndex, act)) ||
                 (act2 != 0 && IS_ACT2_FLAG(pMobIndex, act2)) ||
                 (aff != 0 && IS_AFFECTED(pMobIndex, aff)) || 
		 (race != -1 && pMobIndex->race == race) )
            {
                nMatch++;
                pager_printf( ch, "[%5d] %s\n\r",
                              pMobIndex->ivnum, capitalize( pMobIndex->short_descr ) );
            }

    if ( nMatch )
        pager_printf( ch, "Number of matches: %d\n", nMatch );
    else
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

    return;
}


void do_rfind( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *pRoom;
    int hash;
    int nMatch;
    bool name = FALSE, desc = FALSE;

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Rfind what?\n\r", ch );
        return;
    }

    if ( !str_cmp(arg, "desc") )
    {
        if ( !argument || !*argument )
        {
            send_to_char( "Rfind desc what?\n\r", ch );
            return;
        }
        desc = TRUE;
    }
    else
        name = TRUE;

    set_pager_color( AT_PLAIN, ch );
    nMatch	= 0;

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pRoom = room_index_hash[hash];
              pRoom;
              pRoom = pRoom->next )
            if ( (name && nifty_is_name_prefix( arg, pRoom->name )) ||
                 (desc && nifty_is_name_prefix( arg, pRoom->description )) )
            {
                nMatch++;
                pager_printf( ch, "[%5d] %s\n\r",
                              pRoom->vnum, pRoom->name );
            }

    if ( nMatch )
        pager_printf( ch, "Number of matches: %d\n", nMatch );
    else
        send_to_char( "Nowhere like that in hell, earth, or heaven.\n\r", ch );

    return;
}


void do_ofind( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int hash;
    int nMatch;
    int otype=-1;

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Ofind what?\n\r", ch );
        return;
    }

    if ( !str_cmp(arg, "type") )
    {
        one_argument( argument, arg );
        if ( arg[0] == '\0' )
        {
            send_to_char( "Ofind what item type?\n\r", ch );
            return;
        }

        otype = get_otype(arg);
        if ( otype < 0 )
        {
            ch_printf( ch, "Invalid type: %s\n\r", arg );
            return;
        }
    }

    set_pager_color( AT_PLAIN, ch );
    nMatch	= 0;

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for ( pObjIndex = obj_index_hash[hash];
              pObjIndex;
              pObjIndex = pObjIndex->next )
            if ( pObjIndex->item_type == otype ||
                 (otype < 0 && nifty_is_name( arg, pObjIndex->name )) )
            {
                nMatch++;
                pager_printf( ch, "[%5d] %s\n\r",
                              pObjIndex->ivnum, capitalize( pObjIndex->short_descr ) );
            }

    if ( nMatch )
        pager_printf( ch, "Number of matches: %d\n", nMatch );
    else
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

    return;
}


void do_mwhere( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int icnt = 0;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Mwhere whom?\n\r", ch );
        return;
    }

    set_pager_color( AT_PLAIN, ch );
    found = FALSE;
    for ( victim = first_char; victim; victim = victim->next )
    {
        if ( victim->in_room && can_see(ch, victim) &&
             ( nifty_is_name( arg, victim->name ) ||
               victim->unum == unum_arg(arg) ||
               victim->vnum == vnum_arg(arg) ) )
        {
            found = TRUE;
            pager_printf( ch, "%3d) u%-6d [%5d] %-24s [%5d] %s\n\r",
                          ++icnt,
                          victim->unum,
                          victim->vnum,
                          PERS(victim, ch),
                          victim->in_room->vnum,
                          victim->in_room->name );
        }
    }

    if ( !found )
        act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    else
        pager_printf(ch, "%d matches.\n\r", icnt);

    return;
}

void do_gfighting( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim = NULL;
    DESCRIPTOR_DATA *d;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    bool found = FALSE, pmobs = FALSE, phating = FALSE, phunting = FALSE;
    int low = 1, high = 65, count = 0;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] != '\0' )
    {
        if ( arg1[0] == '\0' || arg2[0] == '\0' )
        {
            send_to_pager_color( "\n\rSyntax:  gfighting | gfighting <low> <high> | gfighting <low> <high> mobs\n\r", ch );
            return;
        }
        low = atoi( arg1 );
        high = atoi( arg2 );
    }
    if ( low < 1 || high < low || low > high || high > 65 )
    {
        send_to_pager_color( "Invalid level range.\n\r", ch );
        return;
    }
    argument = one_argument( argument, arg3 );
    if ( !str_cmp( arg3, "mobs" ) )
        pmobs = TRUE;
    else if ( !str_cmp( arg3, "hating" ) )
        phating = TRUE;
    else if ( !str_cmp( arg3, "hunting" ) )
        phunting = TRUE;

    pager_printf( ch, "\n\rGlobal %s conflict:\n\r", pmobs ? "mob" : "character" );
    if ( !pmobs && !phating && !phunting)
    {
        for ( d = first_descriptor; d; d = d->next )
            if ( (d->connected == CON_PLAYING || d->connected == CON_EDITING )
                 && ( victim = d->character ) != NULL && !IS_NPC( victim ) && victim->in_room
                 &&   can_see( ch, victim )
                 &&   victim->fighting && GetMaxLevel(victim) >= low && GetMaxLevel(victim) <= high )
            {
                found = TRUE;
                pager_printf( ch, "%-12.12s |%2d vs %2d| %-16.16s [%5d]  %-20.20s [%5d]\n\r",
                              victim->name, GetMaxLevel(victim), GetMaxLevel(victim->fighting->who),
                              IS_NPC( victim->fighting->who ) ? victim->fighting->who->short_descr : victim->fighting->who->name,
                              victim->fighting->who->vnum,
                              victim->in_room->area->name, victim->in_room == NULL ? 0 : victim->in_room->vnum );
                count++;
            }
    }
    else if ( !phating && !phunting)
    {
        for ( victim = first_char; victim; victim = victim->next )
            if ( IS_NPC( victim )
                 &&   victim->in_room && can_see( ch, victim )
                 &&   victim->fighting && GetMaxLevel(victim) >= low && GetMaxLevel(victim) <= high )
            {
                found = TRUE;
                pager_printf( ch, "%-12.12s |%2d vs %2d| %-16.16s [%5d]  %-20.20s [%5d]\n\r",
                              victim->name, GetMaxLevel(victim), GetMaxLevel(victim->fighting->who),
                              IS_NPC( victim->fighting->who ) ? victim->fighting->who->short_descr : victim->fighting->who->name,
                              victim->fighting->who->vnum,
                              victim->in_room->area->name, victim->in_room == NULL ? 0 : victim->in_room->vnum );
                count++;
            }
    }
    else if ( !phunting && phating)
    {
        for ( victim = first_char; victim; victim = victim->next )
            if ( IS_NPC( victim )
                 &&   victim->in_room && can_see( ch, victim )
                 &&   victim->hating && GetMaxLevel(victim) >= low && GetMaxLevel(victim) <= high )
            {
                found = TRUE;
                pager_printf( ch, "%-12.12s |%2d vs %2d| %-16.16s [%5d]  %-20.20s [%5d]\n\r",
                              victim->name, GetMaxLevel(victim), GetMaxLevel(victim->hating->who), IS_NPC( victim->hating->who ) ?
                              victim->hating->who->short_descr : victim->hating->who->name,
                              victim->hating->who->vnum,
                              victim->in_room->area->name, victim->in_room == NULL ? 0 :
                              victim->in_room->vnum );
                count++;
            }
    }
    else if ( phunting )
    {
        for ( victim = first_char; victim; victim = victim->next )
            if ( IS_NPC( victim )
                 &&   victim->in_room && can_see( ch, victim )
                 &&   victim->hunting && GetMaxLevel(victim) >= low && GetMaxLevel(victim) <= high )
            {
                found = TRUE;
                pager_printf( ch, "%-12.12s |%2d vs %2d| %-16.16s [%5d]  %-20.20s [%5d]\n\r",
                              victim->name, GetMaxLevel(victim), GetMaxLevel(victim->hunting->who), IS_NPC( victim->hunting->who ) ?
                              victim->hunting->who->short_descr : victim->hunting->who->name,
                              victim->hunting->who->vnum,
                              victim->in_room->area->name, victim->in_room == NULL ? 0 : victim->in_room->vnum );
                count++;
            }
    }
    pager_printf( ch, "%d %s conflicts located.\n\r", count, pmobs ? "mob" : "character" );
    return;
}

/* Added 'show' argument for lowbie imms without ostat -- Blodkai */
/* Made show the default action :) Shaddai */
/* Trimmed size, added vict info, put lipstick on the pig -- Blod */
void do_bodybag( CHAR_DATA *ch, char *argument )
{
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *owner;
    OBJ_DATA *obj;
    bool found = FALSE, bag = FALSE;

    argument = one_argument( argument, arg1 );
    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax:  bodybag <character> | bodybag <character> yes/bag/now\n\r", ch );
        return;
    }

    sprintf( buf3, " " );
    sprintf( buf2, "the corpse of %s", arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg2[0] !='\0' && ( str_cmp( arg2, "yes" )
                             && str_cmp( arg2, "bag" ) && str_cmp( arg2, "now" ) ) )
    {
        send_to_char( "\n\rSyntax:  bodybag <character> | bodybag <character> yes/bag/now\n\r", ch );
        return;
    }
    if ( !str_cmp( arg2, "yes" ) || !str_cmp( arg2, "bag" )
         ||   !str_cmp( arg2, "now" ) )
        bag = TRUE;

    pager_printf( ch, "\n\r%s remains of %s ... ",
                  bag ? "Retrieving" : "Searching for",
                  capitalize( arg1 ) );
    for ( obj = first_object; obj; obj = obj->next )
    {
        if ( obj->in_room
             && !str_cmp( buf2, obj->short_descr )
             && (obj->vnum == OBJ_VNUM_CORPSE_PC ) )
        {
            send_to_pager( "\n\r", ch );
            found = TRUE;
            pager_printf( ch, "%s:  %-12.12s   In:  %-22.22s  [%5d]   Timer:  %2d",
                          bag ? "Bagging" : "Corpse",
                          capitalize( arg1 ),
                          obj->in_room->area->name,
                          obj->in_room->vnum,
                          obj->timer );
            if ( bag )
            {
                obj_from_room( obj );
                obj = obj_to_char( obj, ch );
                obj->timer = -1;
                save_char_obj( ch );
            }
        }
    }
    if ( !found )
    {
        send_to_pager( "No corpse was found.\n\r", ch );
        return;
    }
    send_to_pager( "\n\r", ch );
    for ( owner = first_char; owner; owner = owner->next )
    {
        if ( IS_NPC( owner ) )
            continue;
        if ( can_see( ch, owner) && !str_cmp( arg1, owner->name ) )
            break;
    }
    if ( owner == NULL )
    {
        pager_printf( ch, "%s is not currently online.\n\r",
                      capitalize( arg1 ) );
        return;
    }
    if ( owner->pcdata->deity )
        pager_printf( ch, "%s (%d) has %d favor with %s (needed to supplicate: %d)\n\r",
                      owner->name,
                      GetMaxLevel(owner),
                      owner->pcdata->favor,
                      owner->pcdata->deity->name,
                      owner->pcdata->deity->scorpse );
    else
        pager_printf( ch, "%s (%d) has no deity.\n\r",
                      owner->name, GetMaxLevel(owner) );
    return;
}


/* New owhere by Altrag, 03/14/96 */
void do_owhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    bool found;
    int icnt = 0;

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Owhere what?\n\r", ch );
        return;
    }
    argument = one_argument(argument, arg1);

    set_pager_color( AT_PLAIN, ch );
    if ( arg1[0] != '\0' && !str_prefix(arg1, "nesthunt") )
    {
        if ( !(obj = get_obj_world(ch, arg)) )
        {
            send_to_char( "Nesthunt for what object?\n\r", ch );
            return;
        }
        for ( ; obj->in_obj; obj = obj->in_obj )
        {
            pager_printf(ch, "[%5d] %-28s in object [%5d] %s\n\r",
                         obj->vnum, obj_short(obj),
                         obj->in_obj->vnum, obj->in_obj->short_descr);
            ++icnt;
        }
        sprintf(buf, "[%5d] %-28s in ", obj->vnum,
                obj_short(obj));
        if ( obj->carried_by )
            sprintf(buf+strlen(buf), "invent [%5d] %s\n\r",
                    obj->carried_by->vnum, PERS(obj->carried_by, ch));
        else if ( obj->in_room )
            sprintf(buf+strlen(buf), "room   [%5d] %s\n\r",
                    obj->in_room->vnum, obj->in_room->name);
        else if ( obj->in_obj )
        {
            bug("do_owhere: obj->in_obj after NULL!");
            strcat(buf, "object??\n\r");
        }
        else if ( obj_extracted(obj) )
        {
            bug("do_owhere: object extracted!");
            strcat(buf, "extracted??\n\r");
        }
        else
        {
            bug("do_owhere: object doesnt have location!");
            strcat(buf, "nowhere??\n\r");
        }
        send_to_pager(buf, ch);
        ++icnt;
        pager_printf(ch, "Nested %d levels deep.\n\r", icnt);
        return;
    }

    found = FALSE;
    for ( obj = first_object; obj; obj = obj->next )
    {
        if (obj_extracted(obj))
            continue;

        if ( !nifty_is_name( arg, obj->name ) &&
             obj->unum != unum_arg(arg) &&
             obj->vnum != vnum_arg(arg) )
            continue;
        found = TRUE;

        sprintf(buf, "%3d) u%-6d [%5d] %-24s in ",
                ++icnt, obj->unum, obj->vnum, obj_short(obj));
        if ( obj->carried_by )
            sprintf(buf+strlen(buf), "invent [%5d] %s\n\r",
                    obj->carried_by->vnum, PERS(obj->carried_by, ch));
        else if ( obj->in_room )
            sprintf(buf+strlen(buf), "room   [%5d] %s\n\r",
                    obj->in_room->vnum, obj->in_room->name);
        else if ( obj->in_obj )
            sprintf(buf+strlen(buf), "object [%5d] %s\n\r",
                    obj->in_obj->vnum, obj_short(obj->in_obj));
        else
        {
            bug("do_owhere: object doesnt have location!");
            strcat(buf, "nowhere??\n\r");
        }
        send_to_pager(buf, ch);
    }

    if ( !found )
        act( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    else
        pager_printf(ch, "%d matches.\n\r", icnt);

    return;
}


void do_reboo( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}

#ifdef USE_CRBS
void do_clogoff(CHAR_DATA *ch, char *argument);
#endif

void do_reboot( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *vch;

    if ( str_cmp( argument, "mud now" )
         &&   str_cmp( argument, "nosave" )
         &&   str_cmp( argument, "and sort skill table" )
         &&   str_cmp( argument, "and sort skill table by sn" ) )
    {
        send_to_char( "Syntax: 'reboot mud now' or 'reboot nosave'\n\r", ch );
        return;
    }

    if ( auction->item )
        do_auction( ch, "stop");

    sprintf( buf, "Reboot by %s.", ch->name );
    do_echo( ch, buf );

    if ( !str_cmp(argument, "and sort skill table") )
    {
        sort_skill_table();
        save_skill_table();
    }
    else if ( !str_cmp(argument, "and sort skill table by sn") )
    {
        sort_skill_table_sn();
        save_skill_table();
    }

    /* Save all characters before booting. */
    if ( str_cmp(argument, "nosave") )
        for ( vch = first_char; vch; vch = vch->next )
            if ( !IS_NPC( vch ) )
            {
                save_char_obj( vch );
#ifdef IRC
                irc_logoff(vch);
#endif

#ifdef USE_CRBS
                do_clogoff(vch, NULL); /* CRBS logoff */
#endif
            }

    mud_down = TRUE;
    return;
}



void do_shutdow( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}



void do_shutdown( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *vch;

    if ( auction->item )
        do_auction( ch, "stop");

    sprintf( buf, "Shutdown by %s.", ch->name );
    append_file( ch, SHUTDOWN_FILE, buf );
    strcat( buf, "\n\r" );
    do_echo( ch, buf );

    /* Save all characters before booting. */
    if ( str_cmp(argument, "nosave") )
        for ( vch = first_char; vch; vch = vch->next )
            if ( !IS_NPC( vch ) )
            {
                save_char_obj( vch );
#ifdef IRC
                irc_logoff(vch);
#endif

#ifdef USE_CRBS
                do_clogoff(vch, NULL); /* CRBS logoff */
#endif
            }
    mud_down = TRUE;
    return;
}


void do_snoop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Snoop whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( !victim->desc )
    {
        send_to_char( "No descriptor to snoop.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "Cancelling all snoops.\n\r", ch );
        for ( d = first_descriptor; d; d = d->next )
            if ( d->snoop_by == ch->desc )
                d->snoop_by = NULL;
        return;
    }

    if ( victim->desc->snoop_by )
    {
        send_to_char( "Busy already.\n\r", ch );
        return;
    }

    /*
     * Minimum snoop level... a secret mset value
     * makes the snooper think that the victim is already being snooped
     */
    if ( get_trust( victim ) >= get_trust( ch )
         ||  (victim->pcdata && victim->pcdata->min_snoop > get_trust( ch )) )
    {
        send_to_char( "Busy already.\n\r", ch );
        return;
    }

    if ( ch->desc )
    {
        for ( d = ch->desc->snoop_by; d; d = d->snoop_by )
            if ( d->character == victim || d->original == victim )
            {
                send_to_char( "No snoop loops.\n\r", ch );
                return;
            }
    }

    victim->desc->snoop_by = ch->desc;
    send_to_char( "Ok.\n\r", ch );
    return;
}



void do_switch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Switch into whom?\n\r", ch );
        return;
    }

    if ( !ch->desc )
        return;

    if ( ch->desc->original )
    {
        send_to_char( "You are already switched.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( victim->desc )
    {
        send_to_char( "Character in use.\n\r", ch );
        return;
    }

    if ( !IS_NPC(victim) && GetMaxLevel(ch) < LEVEL_IMPLEMENTOR )
    {
        send_to_char( "You cannot switch into a player!\n\r", ch );
        return;
    }

    /* let people switch into their bestowed mob */
    if ( IS_NPC(victim) && GetMaxLevel(ch) < LEVEL_GREATER )
    {
        char buf[MAX_INPUT_LENGTH];

        sprintf(buf, "mob:%d", victim->vnum);
        if (!is_name(buf, ch->pcdata->bestowments))
            return;
    }

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    ch->switched	= victim;
    send_to_char( "Ok.\n\r", victim );
    return;
}



void do_return( CHAR_DATA *ch, char *argument )
{
    if ( !ch->desc )
        return;

    if (IS_SET(ch->act, ACT_POLYMORPHED))
    {
        send_to_char("Use revert to return from a polymorphed mob.\n\r", ch);
        return;
    }

    if ( !ch->desc->original )
    {
        send_to_char( "You aren't switched.\n\r", ch );
        return;
    }

    send_to_char( "You return to your original body.\n\r", ch );
    if ( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_POSSESS ) )
    {
        affect_strip( ch, gsn_possess );
        REMOVE_BIT( ch->affected_by, AFF_POSSESS );
    }
    /*    if ( IS_NPC( ch->desc->character ) )
     REMOVE_BIT( ch->desc->character->affected_by, AFF_POSSESS );*/
    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc;
    ch->desc->character->switched = NULL;
    ch->desc                  = NULL;
    return;
}


void do_minvoke( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    int vnum;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: load m <vnum>.\n\r", ch );
        return;
    }

    if ( !is_number( arg ) )
    {
        char arg2[MAX_INPUT_LENGTH];
        int  hash, cnt;
        int  count = number_argument( arg, arg2 );

        vnum = -1;
        for ( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
            for ( pMobIndex = mob_index_hash[hash];
                  pMobIndex;
                  pMobIndex = pMobIndex->next )
                if ( nifty_is_name( arg2, pMobIndex->player_name )
                     &&   ++cnt == count )
                {
                    vnum = pMobIndex->ivnum;
                    break;
                }
    }
    else
        vnum = atoi( arg );

    if ( !mob_exists_index( vnum ) )
    {
        send_to_char("No such mobile exists.\n\r", ch);
        return;
    }

    if ( (vnum<VNUM_START_SCRATCH || vnum>VNUM_END_SCRATCH) &&
         get_trust(ch) < LEVEL_DEMI )
    {
        AREA_DATA *pArea;

        if ( IS_NPC(ch) )
        {
            send_to_char( "Huh?\n\r", ch );
            return;
        }

        if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
        {
            send_to_char( "You must have an assigned area to invoke this mobile.\n\r", ch );
            return;
        }
        if ( vnum < pArea->low_m_vnum
             &&   vnum > pArea->hi_m_vnum )
        {
            send_to_char( "That number is not in your allocated range.\n\r", ch );
            return;
        }
    }

    if ( ( victim = create_mobile( vnum ) ) == NULL )
    {
        send_to_char( "No mobile has that vnum.\n\r", ch );
        return;
    }
    char_to_room( victim, ch->in_room );
    ch_printf(ch, "Mobile has unum %d\n\r", victim->unum);

    mprog_birth_trigger(ch, victim);
    if (ch->pcdata && !ch->pcdata->wizinvis)
        act( AT_IMMORT, "$n has created $N!", ch, NULL, victim, TO_ROOM );
    act( AT_IMMORT, "You have created $N!", ch, NULL, victim, TO_CHAR );
    return;
}


void do_oinvoke( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int vnum;

    argument = one_argument( argument, arg1 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: load o <vnum>.\n\r", ch );
        return;
    }

    if ( !is_number( arg1 ) )
    {
        char arg[MAX_INPUT_LENGTH];
        int  hash, cnt;
        int  count = number_argument( arg1, arg );

        vnum = -1;
        for ( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
            for ( pObjIndex = obj_index_hash[hash];
                  pObjIndex;
                  pObjIndex = pObjIndex->next )
                if ( nifty_is_name( arg, pObjIndex->name )
                     &&   ++cnt == count )
                {
                    vnum = pObjIndex->ivnum;
                    break;
                }
    }
    else
        vnum = atoi( arg1 );

    if ( !obj_exists_index( vnum ) )
    {
        send_to_char("No such object exists.\n\r", ch);
        return;
    }

    if ( (vnum<VNUM_START_SCRATCH || vnum>VNUM_END_SCRATCH) &&
         get_trust(ch) < LEVEL_DEMI )
    {
        AREA_DATA *pArea;

        if ( IS_NPC(ch) )
        {
            send_to_char( "Huh?\n\r", ch );
            return;
        }

        if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
        {
            send_to_char( "You must have an assigned area to invoke this object.\n\r", ch );
            return;
        }
        if ( vnum < pArea->low_o_vnum
             &&   vnum > pArea->hi_o_vnum )
        {
            send_to_char( "That number is not in your allocated range.\n\r", ch );
            return;
        }
    }

    if ( ( obj = create_object( vnum ) ) == NULL )
    {
        send_to_char( "No object has that vnum.\n\r", ch );
        return;
    }
    if ( CAN_WEAR(obj, ITEM_TAKE) )
        obj = obj_to_char( obj, ch );
    else
        obj = obj_to_room( obj, ch->in_room );
    ch_printf(ch, "Object has unum %d\n\r", obj->unum);
    if (ch->pcdata && !ch->pcdata->wizinvis)
        act( AT_IMMORT, "$n has created $p!", ch, obj, NULL, TO_ROOM );
    act( AT_IMMORT, "You have created $p!", ch, obj, NULL, TO_CHAR );
    return;
}

void do_load(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        do_minvoke(ch, "");
        do_oinvoke(ch, "");
        return;
    }

    if (!str_prefix(arg, "mobile"))
    {
        do_minvoke(ch, argument);
    }
    else if (!str_prefix(arg, "object"))
    {
        do_oinvoke(ch, argument);
    }
    else {
        send_to_char("Sorry we don't have those here.\n\r", ch);
    }
}

void do_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim, *tch;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        /* 'purge' */
        CHAR_DATA *vnext;
        OBJ_DATA  *obj_next;

        for ( victim = ch->in_room->first_person; victim; victim = vnext )
        {
            vnext = victim->next_in_room;

            for (tch=ch->in_room->first_person; tch; tch = tch->next_in_room)
                if (!IS_NPC(tch) && tch->dest_buf == victim)
                    break;
            if (tch && !IS_NPC(tch) && tch->dest_buf == victim)
                continue;

            if ( IS_NPC(victim) && victim != ch && !IS_SET(victim->act, ACT_POLYMORPHED))
                extract_char( victim, TRUE );
        }

        for ( obj = ch->in_room->first_content; obj; obj = obj_next )
        {
            obj_next = obj->next_content;

            for (tch=ch->in_room->first_person; tch; tch = tch->next_in_room)
                if (!IS_NPC(tch) && tch->dest_buf == obj)
                    break;
            if (tch && !IS_NPC(tch) && tch->dest_buf == obj)
                continue;

            extract_obj( obj );
        }

        act( AT_IMMORT, "$n purges the room!", ch, NULL, NULL, TO_ROOM);
        send_to_char( "Ok.\n\r", ch );
        return;
    }
    victim = NULL; obj = NULL;

    /* fixed to get things in room first -- i.e., purge portal (obj),
     * no more purging mobs with that keyword in another room first
     * -- Tri */
    if ( ( victim = get_char_room( ch, arg ) ) == NULL
         && ( obj = get_obj_here( ch, arg ) ) == NULL )
    {
        if ( ( victim = get_char_world( ch, arg ) ) == NULL
             &&   ( obj = get_obj_world( ch, arg ) ) == NULL )  /* no get_obj_room */
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }
    }

    /* Single object purge in room for high level purge - Scryn 8/12*/
    if ( obj )
    {
        for (tch=ch->in_room->first_person; tch; tch = tch->next_in_room)
            if (!IS_NPC(tch) && tch->dest_buf == obj)
            {
                send_to_char("You cannot purge something being edited.\n\r", ch);
                return;
            }

        separate_obj( obj );
        act( AT_IMMORT, "$n purges $p.", ch, obj, NULL, TO_ROOM);
        act( AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj, NULL, TO_CHAR);
        extract_obj( obj );
        return;
    }


    if ( !IS_NPC(victim) )
    {
        send_to_char( "Not on PC's.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "You cannot purge yourself!\n\r", ch );
        return;
    }

    for (tch=ch->in_room->first_person; tch; tch = tch->next_in_room)
        if (!IS_NPC(tch) && tch->dest_buf == victim)
        {
            send_to_char("You cannot purge something being edited.\n\r", ch);
            return;
        }

    if (IS_SET(victim->act, ACT_POLYMORPHED))
    {
        send_to_char("You cannot purge a polymorphed player.\n\r", ch);
        return;
    }
    act( AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    act( AT_IMMORT, "You purge $N.", ch, NULL, victim, TO_CHAR );
    send_to_char("Ok.\n\r", ch);
    extract_char( victim, TRUE );
    return;
}


void do_low_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Purge what?\n\r", ch );
        return;
    }

    victim = NULL; obj = NULL;
    if ( ( victim = get_char_room( ch, arg ) ) == NULL
         &&	 ( obj    = get_obj_here ( ch, arg ) ) == NULL )
    {
        send_to_char( "You can't find that here.\n\r", ch );
        return;
    }

    if ( obj )
    {
        separate_obj( obj );
        act( AT_IMMORT, "$n purges $p!", ch, obj, NULL, TO_ROOM );
        act( AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj, NULL, TO_CHAR );
        extract_obj( obj );
        return;
    }

    if ( !IS_NPC(victim) )
    {
        send_to_char( "Not on PC's.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "You cannot purge yourself!\n\r", ch );
        return;
    }

    act( AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    act( AT_IMMORT, "You make $N disappear in a puff of smoke!", ch, NULL, victim, TO_CHAR );
    extract_char( victim, TRUE );
    return;
}


void do_balzhur( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    AREA_DATA *pArea, *pArea_next;
    int sn;
    int i;
    int level;

    if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg );

    argument = one_argument( argument, arg2 );

    if (is_number(arg2))
    {
        level = atoi(arg2);
    }
    else
    {
        send_to_char( "Usage: demote <char> <level>\n\r", ch);
        return;
    }

    if ( arg[0] == '\0' )
    {
        send_to_char( "Who is deserving of such a fate?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't playing.\n\r", ch);
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( GetMaxLevel(victim) >= get_trust( ch ) )
    {
        send_to_char( "I wouldn't even think of that if I were you...\n\r", ch );
        return;
    }

    send_to_char( "You summon the demon Balzhur to wreak your wrath!\n\r", ch );
    send_to_char( "Balzhur sneers at you evilly, then vanishes in a puff of smoke.\n\r", ch );
    send_to_char( "You hear an ungodly sound in the distance that makes your blood run cold!\n\r", victim );
    sprintf( buf, "Balzhur screams, 'You are MINE %s!!!'", victim->name );
    echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );

    for (i = 0; i < MAX_CLASS; ++i) {
        if (HAD_CLASS(victim, i)) {
            victim->classes[i] = 0;
            GET_LEVEL(victim, i) = 0;
        } else if (IS_ACTIVE(victim, i)) {
            GET_LEVEL(victim, i) = level;
        } else {
            GET_LEVEL(victim, i) = 0;
        }
    }
    victim->trust	 = level;
    victim->exp      = 2000;
    victim->max_hit  = 10;
    victim->max_mana = 0;
    victim->max_move = 0;
    for ( sn = 0; sn < top_sn; sn++ )
        victim->pcdata->learned[sn] = 0;
    victim->practice = 0;
    GET_HIT(victim)  = GET_MAX_HIT(victim);
    GET_MANA(victim) = GET_MAX_MANA(victim);
    GET_MOVE(victim) = GET_MAX_MOVE(victim);


    sprintf( buf, "%s%s", GOD_DIR, capitalize(victim->name) );

    if ( !remove( buf ) )
        send_to_char( "Player's immortal data destroyed.\n\r", ch );
    else if ( errno != ENOENT )
    {
        ch_printf( ch, "Unknown error #%d - %s (immortal data).  Report to Thoric\n\r",
                   errno, strerror( errno ) );
        sprintf( buf2, "%s balzhuring %s", ch->name, buf );
        perror( buf2 );
    }
    sprintf( buf2, "%s.are", capitalize(arg) );
    for ( pArea = first_build; pArea; pArea = pArea_next )
    {
        pArea_next = pArea->next;
        if ( !pArea->filename )
            continue;
        if ( !strcmp( pArea->filename, buf2 ) )
        {
            sprintf( buf, "%s%s", BUILD_DIR, buf2 );
            if ( IS_SET( pArea->status, AREA_LOADED ) )
                fold_area( pArea, buf, FALSE );
            close_area( pArea, TRUE );
            sprintf( buf2, "%s.bak", buf );
            set_char_color( AT_RED, ch ); /* Log message changes colors */
            if ( !rename( buf, buf2 ) )
                send_to_char( "Player's area data destroyed.  Area saved as backup.\n\r", ch);
            else if ( errno != ENOENT )
            {
                ch_printf( ch, "Unknown error #%d - %s (area data).  Report to Thoric.\n\r",
                           errno, strerror( errno ) );
                sprintf( buf2, "%s destroying %s", ch->name, buf );
                perror( buf2 );
            }
        }
    }

    make_wizlist();
    do_help(victim, "M_BALZHUR_" );
    send_to_char( "You awake after a long period of time...\n\r", victim );
    while ( victim->first_carrying )
        extract_obj( victim->first_carrying );
    return;
}

void do_advance( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    int iLevel;
    sh_int ch_class;

    if ( IS_NPC(ch) )
        return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' ||
         arg3[0] == '\0' || !is_number(arg3) )
    {
        send_to_char( "Syntax: advance <char> <class name> <level>\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch);
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    /* You can demote yourself but not someone else at your own trust.
     *  -- Narn
     */
    if ( get_trust( ch ) <= get_trust( victim ) && ch != victim )
    {
        send_to_char( "You can't do that.\n\r", ch );
        return;
    }

    if ( ( level = atoi( arg3 ) ) < 1 || level > MAX_LEVEL )
    {
        send_to_char( "Level must be 1 to 75.\n\r", ch );
        return;
    }

    if ( ( ch_class = get_classtype( arg2 ) ) < 0 )
    {
        send_to_char( "Invalid Class.\n\r", ch);
        return;
    }

    if ( level > get_trust( ch ) )
    {
        send_to_char( "Limited to your trust level.\n\r", ch );
        return;
    }

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
    if ( level <= GET_LEVEL(victim, ch_class) )
    {
        send_to_char("You can't lower a level with advance.\n\r", ch);
    }
    else
    {
        send_to_char( "Raising a player's level!\n\r", ch );
        if (GET_LEVEL(victim, ch_class) >= LEVEL_AVATAR)
        {
            act( AT_IMMORT, "$n makes some arcane gestures with $s hands, then points $s finger at you!",
                 ch, NULL, victim, TO_VICT );
            act( AT_IMMORT, "$n makes some arcane gestures with $s hands, then points $s finger at $N!",
                 ch, NULL, victim, TO_NOTVICT );
            send_to_char( "You suddenly feel very strange...\n\r\n\r", victim );
        }

        switch(level)
        {
        default:
            send_to_char( "The gods feel fit to raise your level!\n\r", victim );
            break;
        case LEVEL_IMMORTAL:
            do_help(victim, "M_GODLVL1_" );
            send_to_char( "You awaken... all your possessions are gone.\n\r", victim );
            victim->pcdata->time_immortal = current_time;
            victim->pcdata->time_to_die = current_time + (60*60*24*30);
            while ( victim->first_carrying )
                extract_obj( victim->first_carrying );
            break;
        case LEVEL_ACOLYTE:
            do_help(victim, "M_GODLVL2_" );
            break;
        case LEVEL_CREATOR:
            do_help(victim, "M_GODLVL3_" );
            break;
        case LEVEL_SAVIOR:
            do_help(victim, "M_GODLVL4_" );
            break;
        case LEVEL_DEMI:
            do_help(victim, "M_GODLVL5_" );
            break;
        case LEVEL_TRUEIMM:
            do_help(victim, "M_GODLVL6_" );
            break;
        case LEVEL_LESSER:
            do_help(victim, "M_GODLVL7_" );
            break;
        case LEVEL_GOD:
            do_help(victim, "M_GODLVL8_" );
            break;
        case LEVEL_GREATER:
            do_help(victim, "M_GODLVL9_" );
            break;
        case LEVEL_ASCENDANT:
            do_help(victim, "M_GODLVL10_" );
            break;
        case LEVEL_SUB_IMPLEM:
            do_help(victim, "M_GODLVL11_" );
            break;
        case LEVEL_IMPLEMENTOR:
            do_help(victim, "M_GODLVL12_" );
            break;
        case LEVEL_ETERNAL:
            do_help(victim, "M_GODLVL13_" );
            break;
        case LEVEL_INFINITE:
            do_help(victim, "M_GODLVL14_" );
            break;
        case LEVEL_SUPREME:
            do_help(victim, "M_GODLVL15_" );
        }
    }

    for ( iLevel = GET_LEVEL(victim, ch_class) ; iLevel < level; iLevel++ )
    {
        if (level < LEVEL_IMMORTAL)
            send_to_char( "You raise a level!!\n\r", victim );
        if (iLevel > RacialMax[GET_RACE(victim)][ch_class])
        {
            send_to_char( "By the power of the Gods, you exceed the normal skill of your race.\n\r", victim );
            send_to_char( "You advanced them past their level limit in that class.\n\r", ch);
        }
        advance_level( victim, ch_class );
    }
    victim->exp   = exp_level( victim, GET_LEVEL(victim, ch_class), ch_class);
    victim->trust = GetMaxLevel(victim);

    do_restore(ch, GET_NAME(victim));
    do_save(victim, NULL);
    if (level>=LEVEL_IMMORTAL)
        do_makewizlist(ch, NULL);

    sprintf( log_buf, "%s advanced %s to level %d",
             GET_NAME(ch), GET_NAME(victim), level );
    log_string_plus(log_buf, LOG_MONITOR, GetMaxLevel(ch), SEV_CRIT);

    return;
}

void do_immortalize( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int i;

    if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: immortalize <char>\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch);
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( GetAveLevel(victim) != LEVEL_AVATAR )
    {
        send_to_char( "This player is not worthy of immortality yet.\n\r", ch );
        return;
    }

    send_to_char( "Immortalizing a player...\n\r", ch );
    act( AT_IMMORT, "$n begins to chant softly... then raises $s arms to the sky...",
         ch, NULL, NULL, TO_ROOM );
    send_to_char( "You suddenly feel very strange...\n\r\n\r", victim );

    do_help(victim, "M_GODLVL1_" );
    send_to_char( "You awake... all your possessions are gone.\n\r", victim );
    while ( victim->first_carrying )
        extract_obj( victim->first_carrying );

    for (i = 0; i < MAX_CLASS; ++i) {
        GET_LEVEL(victim, i) = LEVEL_IMMORTAL;
        victim->classes[i] = 1;
    }
    victim->exp   = exp_level( victim, LEVEL_IMMORTAL, i );
    victim->trust = LEVEL_IMMORTAL;
    interpret(ch,"ichan rimm");
    return;
}



void do_trust( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;

    if ( IS_NPC(ch) )
	return;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
        send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch);
        return;
    }

    if ( ( level = atoi( arg2 ) ) < 0 || level > MAX_LEVEL )
    {
        send_to_char( "Level must be 0 (reset) or 1 to 60.\n\r", ch );
        return;
    }

    if ( level > get_trust( ch ) )
    {
        send_to_char( "Limited to your own trust.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You can't do that.\n\r", ch );
        return;
    }

    victim->trust = level;
    send_to_char( "Ok.\n\r", ch );
    return;
}

/* Summer 1997 --Blod */
void do_scatter( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *pRoomIndex;

    set_char_color( AT_IMMORT, ch );

    one_argument( argument, arg );
    if ( arg[0] == '\0' ) {
        send_to_char( "Scatter whom?\n\r", ch );
        return; }
    if ( ( victim = get_char_room( ch, arg ) ) == NULL ) {
        send_to_char( "They aren't here.\n\r", ch );
        return; }
    if ( victim == ch ) {
        send_to_char( "It's called teleport.  Try it.\n\r", ch );
        return;
    }
    if ( !IS_NPC(victim) && get_trust( victim ) >= get_trust( ch ) ) {
        send_to_char( "You haven't the power to succeed against them.\n\r", ch );
        return; }
    for ( ; ; ) {
        pRoomIndex = get_room_index( number_range( 0, 60000 ) );
        if ( pRoomIndex )
            if ( !IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)
                 &&   !IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY)
                 &&   !IS_SET(pRoomIndex->room_flags, ROOM_NO_ASTRAL)
                 &&   !IS_SET(pRoomIndex->room_flags, ROOM_PROTOTYPE) )
                break; }
    if ( victim->fighting ) stop_fighting( victim, TRUE );
    act( AT_MAGIC, "With the sweep of an arm, $n flings $N to the winds.", ch, NULL, victim, TO_NOTVICT );
    act( AT_MAGIC, "With the sweep of an arm, $n flings you to the astral winds.", ch, NULL, victim, TO_VICT );
    act( AT_MAGIC, "With the sweep of an arm, you fling $N to the astral winds.", ch, NULL, victim, TO_CHAR );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    victim->position = POS_RESTING;
    act( AT_MAGIC, "$n staggers forth from a sudden gust of wind, and collapses.", victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );
    return;
}

void do_strew( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj_next;
    OBJ_DATA *obj_lose;
    ROOM_INDEX_DATA *pRoomIndex;

    set_char_color( AT_IMMORT, ch );

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' || arg2[0] == '\0' ) {
        send_to_char( "Strew who, what?\n\r", ch );
        return;
    }
    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL ) {
        send_to_char( "It would work better if they were here.\n\r", ch );
        return;
    }
    if ( victim == ch ) {
        send_to_char( "Try taking it out on someone else first.\n\r", ch );
        return;
    }
    if ( !IS_NPC(victim) && get_trust( victim ) >= get_trust( ch ) ) {
        send_to_char( "You haven't the power to succeed against them.\n\r", ch );
        return;
    }
    if ( !str_cmp( arg2, "coins"  ) )
    {
        int x;
        for (x=0;x<MAX_CURR_TYPE;x++)
            GET_MONEY(victim,x) = 0;
        act( AT_MAGIC, "$n gestures and an unearthly gale sends $N's coins flying!", ch, NULL, victim, TO_NOTVICT );
        act( AT_MAGIC, "You gesture and an unearthly gale sends $N's coins flying!", ch, NULL, victim, TO_CHAR );
        act( AT_MAGIC, "As $n gestures, an unearthly gale sends your money flying!", ch, NULL, victim, TO_VICT );
        return;
    }
    for ( ; ; ) {
        pRoomIndex = get_room_index( number_range( 0, 60000 ) );
        if ( pRoomIndex )
            if ( !IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)
                 &&   !IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY)
                 &&   !IS_SET(pRoomIndex->room_flags, ROOM_NO_ASTRAL)
                 &&   !IS_SET(pRoomIndex->room_flags, ROOM_PROTOTYPE) )
                break;
    }
    if ( !str_cmp( arg2, "inventory" ) ) {
        act( AT_MAGIC, "$n speaks a single word, sending $N's possessions flying!", ch, NULL, victim, TO_NOTVICT );
        act( AT_MAGIC, "You speak a single word, sending $N's possessions flying!", ch, NULL, victim, TO_CHAR );
        act( AT_MAGIC, "$n speaks a single word, sending your possessions flying!", ch, NULL, victim, TO_VICT );
        for ( obj_lose=victim->first_carrying; obj_lose; obj_lose=obj_next ) {
            obj_next = obj_lose->next_content;
            obj_from_char( obj_lose );
            obj_to_room( obj_lose, pRoomIndex );
            pager_printf( ch, "\t%s sent to %d\n\r", capitalize(obj_lose->short_descr), pRoomIndex->vnum );
        }
        return;
    }
    send_to_char( "Strew their coins or inventory?\n\r", ch );
    return;
}

void do_strip( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj_next;
    OBJ_DATA *obj_lose;
    int count = 0;

    set_char_color( AT_OBJECT, ch );
    if ( !argument ) {
        send_to_char( "Strip who?\n\r", ch );
        return;
    }
    if ( ( victim = get_char_room( ch, argument ) ) == NULL ) {
        send_to_char( "They're not here.\n\r", ch );
        return;
    }
    if ( victim == ch ) {
        send_to_char( "Kinky.\n\r", ch );
        return;
    }
    if ( !IS_NPC(victim) && get_trust( victim ) >= get_trust( ch ) ) {
        send_to_char( "You haven't the power to succeed against them.\n\r", ch );
        return;
    }
    act( AT_OBJECT, "Searching $N ...", ch, NULL, victim, TO_CHAR );
    for ( obj_lose=victim->first_carrying; obj_lose; obj_lose=obj_next ) {
        obj_next = obj_lose->next_content;
        obj_from_char( obj_lose );
        obj_lose=obj_to_char( obj_lose, ch );
        pager_printf( ch, "  ... %s (%s) taken.\n\r",
                      capitalize(obj_lose->short_descr), obj_lose->name );
        count++;
    }
    if ( !count )
        pager_printf( ch, "Nothing found to take.\n\r" );
    return;
}

void do_restore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Restore whom?\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

        if ( !ch->pcdata )
            return;

        if ( get_trust( ch ) < LEVEL_SUB_IMPLEM )
        {
            if ( IS_NPC( ch ) )
            {
                send_to_char( "You can't do that.\n\r", ch );
                return;
            }
            else
            {
                /* Check if the player did a restore all within the last 18 hours. */
                if ( current_time - last_restore_all_time < RESTORE_INTERVAL )
                {
                    send_to_char( "Sorry, you can't do a restore all yet.\n\r", ch );
                    do_restoretime( ch, "" );
                    return;
                }
            }
        }
        last_restore_all_time    = current_time;
        ch->pcdata->restore_time = current_time;
        save_char_obj( ch );
        send_to_char( "Ok.\n\r", ch);
        for ( vch = first_char; vch; vch = vch_next )
        {
            vch_next = vch->next;

            if ( !IS_NPC( vch ) && !IS_IMMORTAL( vch )
                 && !CAN_PKILL( vch ) && !in_arena( vch ) )
            {
                GET_HIT(vch)  = GET_MAX_HIT(vch);
                GET_MANA(vch) = GET_MAX_MANA(vch);
                GET_MOVE(vch) = GET_MAX_MOVE(vch);
                vch->pcdata->condition[COND_BLOODTHIRST] = GET_MAX_BLOOD(vch);
                vch->pcdata->condition[COND_THIRST] = MAX_COND_VAL;
                vch->pcdata->condition[COND_FULL] = MAX_COND_VAL;
                vch->pcdata->condition[COND_DRUNK] = 0;
                update_pos (vch);
                act( AT_IMMORT, "$n has restored you.", ch, NULL, vch, TO_VICT);
            }
        }
    }
    else
    {

        CHAR_DATA *victim;

        if ( ( victim = get_char_world( ch, arg ) ) == NULL )
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }

        if ( get_trust( ch ) < LEVEL_LESSER
             &&  victim != ch
             && !( IS_NPC( victim ) && IS_SET( victim->act, ACT_PROTOTYPE ) ) )
        {
            send_to_char( "You can't do that.\n\r", ch );
            return;
        }

        GET_HIT(victim)    	= GET_MAX_HIT(victim);
        GET_MANA(victim)	= GET_MAX_MANA(victim);
        GET_MOVE(victim)	= GET_MAX_MOVE(victim);
        victim->mental_state = 0;
        victim->emotional_state = 0;
        if ( victim->pcdata )
	{
            victim->pcdata->condition[COND_THIRST] = MAX_COND_VAL;
            victim->pcdata->condition[COND_FULL] = MAX_COND_VAL;
            victim->pcdata->condition[COND_BLOODTHIRST] = MAX_COND_VAL;
            victim->pcdata->condition[COND_DRUNK] = 0;
            victim->pcdata->condition[COND_BLOODTHIRST] = GET_MAX_BLOOD(victim);
	}
        update_pos( victim );
        if (!IS_NPC(victim) && IS_IMMORTAL(victim))
        {
            int sn;
            for (sn = 1; sn < top_sn; ++sn)
                victim->pcdata->learned[sn] = GET_ADEPT(victim, sn);
            if (victim->trust > GetMaxLevel(victim))
            {
                GET_LEVEL(victim, 0) = victim->trust;
                victim->classes[sn] = STAT_ACTCLASS;
            }
            victim->trust = GetMaxLevel(victim);
            for (sn = 0; sn < MAX_CLASS; sn++)
            {
                GET_LEVEL(victim, sn) = GetMaxLevel(victim);
                victim->classes[sn] = STAT_ACTCLASS;
            }
            victim->exp = exp_level( victim, GetMaxLevel(victim), CLASS_MAGE );
        }
        if ( ch != victim )
            act( AT_IMMORT, "$n has restored you.", ch, NULL, victim, TO_VICT );
        send_to_char( "Ok.\n\r", ch );
        return;
    }
}

void do_restoretime( CHAR_DATA *ch, char *argument )
{
    long int time_passed;
    int hour, minute;

    if ( !last_restore_all_time )
        ch_printf( ch, "There has been no restore all since reboot\n\r");
    else
    {
        time_passed = current_time - last_restore_all_time;
        hour = (int) ( time_passed / 3600 );
        minute = (int) ( ( time_passed - ( hour * 3600 ) ) / 60 );
        ch_printf( ch, "The  last restore all was %d hours and %d minutes ago.\n\r",
                   hour, minute );
    }

    if ( !ch->pcdata )
        return;

    if ( !ch->pcdata->restore_time )
    {
        send_to_char( "You have never done a restore all.\n\r", ch );
        return;
    }

    time_passed = current_time - ch->pcdata->restore_time;
    hour = (int) ( time_passed / 3600 );
    minute = (int) ( ( time_passed - ( hour * 3600 ) ) / 60 );
    ch_printf( ch, "Your last restore all was %d hours and %d minutes ago.\n\r",
               hour, minute );
    return;
}

void do_freeze( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Freeze whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
        REMOVE_BIT(victim->act, PLR_FREEZE);
        send_to_char( "You can play again.\n\r", victim );
        send_to_char( "FREEZE removed.\n\r", ch );
    }
    else
    {
        SET_BIT(victim->act, PLR_FREEZE);
        send_to_char( "You can't do ANYthing!\n\r", victim );
        send_to_char( "FREEZE set.\n\r", ch );
    }

    save_char_obj( victim );

    return;
}



void do_log( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Log whom?\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        if ( fLogAll )
        {
            fLogAll = FALSE;
            send_to_char( "Log ALL off.\n\r", ch );
        }
        else
        {
            fLogAll = TRUE;
            send_to_char( "Log ALL on.\n\r", ch );
        }
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->act, PLR_LOG) )
    {
        REMOVE_BIT(victim->act, PLR_LOG);
        send_to_char( "LOG removed.\n\r", ch );
    }
    else
    {
        SET_BIT(victim->act, PLR_LOG);
        send_to_char( "LOG set.\n\r", ch );
    }

    return;
}


void do_litterbug( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Set litterbug flag on whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( IS_SET(victim->act, PLR_LITTERBUG) )
    {
        REMOVE_BIT(victim->act, PLR_LITTERBUG);
        send_to_char( "You can drop items again.\n\r", victim );
        send_to_char( "LITTERBUG removed.\n\r", ch );
    }
    else
    {
        SET_BIT(victim->act, PLR_LITTERBUG);
        send_to_char( "You a strange force prevents you from dropping any more items!\n\r", victim );
        send_to_char( "LITTERBUG set.\n\r", ch );
    }

    return;
}


void do_noemote( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Noemote whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( IS_SET(victim->act, PLR_NO_EMOTE) )
    {
        REMOVE_BIT(victim->act, PLR_NO_EMOTE);
        send_to_char( "You can emote again.\n\r", victim );
        send_to_char( "NO_EMOTE removed.\n\r", ch );
    }
    else
    {
        SET_BIT(victim->act, PLR_NO_EMOTE);
        send_to_char( "You can't emote!\n\r", victim );
        send_to_char( "NO_EMOTE set.\n\r", ch );
    }

    return;
}



void do_notell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Notell whom?", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( IS_SET(victim->act, PLR_NO_TELL) )
    {
        REMOVE_BIT(victim->act, PLR_NO_TELL);
        send_to_char( "You can use tell again.\n\r", victim );
        send_to_char( "NO_TELL removed.\n\r", ch );
    }
    else
    {
        SET_BIT(victim->act, PLR_NO_TELL);
        send_to_char( "You can no longer use tell.\n\r", victim );
        send_to_char( "NO_TELL set.\n\r", ch );
    }

    return;
}

void do_noooc( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "NoOOC whom?", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( IS_SET(victim->act, PLR_NO_OOC) )
    {
        REMOVE_BIT(victim->act, PLR_NO_OOC);
        send_to_char( "You can OOC again.\n\r", victim );
        send_to_char( "NO_OOC removed.\n\r", ch );
    }
    else
    {
        SET_BIT(victim->act, PLR_NO_OOC);
        send_to_char( "You can no longer use the OOC channel.\n\r", victim );
        send_to_char( "NO_OOC set.\n\r", ch );
    }

    return;
}


void do_notitle( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Notitle whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( IS_SET(victim->pcdata->flags, PCFLAG_NOTITLE) )
    {
        REMOVE_BIT(victim->pcdata->flags, PCFLAG_NOTITLE);
        send_to_char( "You can set your own title again.\n\r", victim );
        send_to_char( "NOTITLE removed.\n\r", ch );
    }
    else
    {
        SET_BIT(victim->pcdata->flags, PCFLAG_NOTITLE);
        sprintf( buf, "the %s %s", get_race_name(victim),
                 GetTitleString(victim));
        set_title( victim, buf );
        send_to_char( "You can't set your own title!\n\r", victim );
        send_to_char( "NOTITLE set.\n\r", ch );
    }

    return;
}

void do_silence( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Silence whom?", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( IS_PLR_FLAG(victim, PLR_SILENCE) )
    {
        send_to_char( "Player already silenced, use unsilence to remove.\n\r", ch );
    }
    else
    {
        SET_PLR_FLAG(victim, PLR_SILENCE);
        send_to_char( "You can't use channels!\n\r", victim );
        send_to_char( "SILENCE set.\n\r", ch );
    }

    return;
}

void do_unsilence( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Unsilence whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( IS_PLR_FLAG(victim, PLR_SILENCE) )
    {
        REMOVE_PLR_FLAG(victim, PLR_SILENCE);
        send_to_char( "You can use channels again.\n\r", victim );
        send_to_char( "SILENCE removed.\n\r", ch );
    }
    else
    {
        send_to_char( "That player is not silenced.\n\r", ch );
    }

    return;
}




void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;

    act( AT_IMMORT, "$n booms, 'PEACE!'", ch, NULL, NULL, TO_ROOM );
    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
        if ( rch->fighting )
        {
            stop_fighting( rch, TRUE );
            do_sit( rch, "" );
        }

        /* Added by Narn, Nov 28/95 */
        stop_hating( rch );
        stop_hunting( rch );
        stop_fearing( rch );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}



BAN_DATA *		first_ban;
BAN_DATA *		last_ban;

void save_banlist( void )
{
    BAN_DATA *pban;
    FILE *fp;

    if ( !(fp = fopen( SYSTEM_DIR BAN_LIST, "w" )) )
    {
        bug( "Save_banlist: Cannot open " BAN_LIST );
        perror(BAN_LIST);
        return;
    }
    for ( pban = first_ban; pban; pban = pban->next )
        fprintf( fp, "%d %s~~%s~\n", pban->level, pban->name, pban->ban_time );
    fprintf( fp, "-1\n" );
    fclose( fp );
    return;
}


void do_ban( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA *pban;
    int bnum;

    if ( IS_NPC(ch) )
        return;

    argument = one_argument( argument, arg );

    set_pager_color( AT_PLAIN, ch );
    if ( arg[0] == '\0' )
    {
        send_to_pager( "Banned sites:\n\r", ch );
        send_to_pager( "[ #] (Lv) Time                     Site\n\r", ch );
        send_to_pager( "---- ---- ------------------------ ---------------\n\r", ch );
        for ( pban = first_ban, bnum = 1; pban; pban = pban->next, bnum++ )
            pager_printf(ch, "[%2d] (%2d) %-24s %s\n\r", bnum,
                         pban->level, pban->ban_time, pban->name);
        return;
    }

    /* People are gonna need .# instead of just # to ban by just last
     number in the site ip.                               -- Altrag */
    if ( is_number(arg) )
    {
        for ( pban = first_ban, bnum = 1; pban; pban = pban->next, bnum++ )
            if ( bnum == atoi(arg) )
                break;
        if ( !pban )
        {
            do_ban(ch, "");
            return;
        }
        argument = one_argument(argument, arg);
        if ( arg[0] == '\0' )
        {
            do_ban( ch, "help" );
            return;
        }
        if ( !str_cmp(arg, "level") )
        {
            argument = one_argument(argument, arg);
            if ( arg[0] == '\0' || !is_number(arg) )
            {
                do_ban( ch, "help" );
                return;
            }
            if ( atoi(arg) < -1 || atoi(arg) > LEVEL_SUPREME )
            {
                ch_printf(ch, "Level range: -1 - %d.\n\r", LEVEL_SUPREME);
                return;
            }
            pban->level = atoi(arg);
            send_to_char( "Ban level set.\n\r", ch );
        }
        else if ( !str_cmp(arg, "newban") )
        {
            pban->level = 1;
            send_to_char( "New characters banned.\n\r", ch );
        }
        else if ( !str_cmp(arg, "mortal") )
        {
            pban->level = LEVEL_AVATAR;
            send_to_char( "All mortals banned.\n\r", ch );
        }
        else if ( !str_cmp(arg, "total") )
        {
            pban->level = LEVEL_SUPREME;
            send_to_char( "Everyone banned.\n\r", ch );
        }
        else
        {
            do_ban(ch, "help");
            return;
        }
        save_banlist( );
        return;
    }

    if ( !str_cmp(arg, "help") )
    {
        send_to_char( "Syntax: ban <site address>\n\r", ch );
        send_to_char( "Syntax: ban <ban number> <level <lev>|newban|mortal|"
                      "total>\n\r", ch );
        return;
    }

    for ( pban = first_ban; pban; pban = pban->next )
    {
        if ( !str_cmp( arg, pban->name ) )
        {
            send_to_char( "That site is already banned!\n\r", ch );
            return;
        }
    }

    CREATE( pban, BAN_DATA, 1 );
    LINK( pban, first_ban, last_ban, next, prev );
    pban->name	= str_dup( arg );
    pban->level = LEVEL_AVATAR;
    sprintf(buf, "%24.24s", ctime(&current_time));
    pban->ban_time = str_dup( buf );
    save_banlist( );
    send_to_char( "Ban created.  Mortals banned from site.\n\r", ch );
    return;
}


void do_allow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA *pban;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Remove which site from the ban list?\n\r", ch );
        return;
    }

    for ( pban = first_ban; pban; pban = pban->next )
    {
        if ( !str_cmp( arg, pban->name ) )
        {
            UNLINK( pban, first_ban, last_ban, next, prev );
            if ( pban->ban_time )
                DISPOSE(pban->ban_time);
            DISPOSE( pban->name );
            DISPOSE( pban );
            save_banlist( );
            send_to_char( "Site no longer banned.\n\r", ch );
            return;
        }
    }

    send_to_char( "Site is not banned.\n\r", ch );
    return;
}



void do_wizlock( CHAR_DATA *ch, char *argument )
{
    wizlock = !wizlock;

    if ( wizlock )
        send_to_char( "Game wizlocked.\n\r", ch );
    else
        send_to_char( "Game un-wizlocked.\n\r", ch );

    return;
}


void do_noresolve( CHAR_DATA *ch, char *argument )
{
    sysdata.NO_NAME_RESOLVING = !sysdata.NO_NAME_RESOLVING;

    if ( sysdata.NO_NAME_RESOLVING )
        send_to_char( "Name resolving disabled.\n\r", ch );
    else
        send_to_char( "Name resolving enabled.\n\r", ch );

    return;
}


void do_users( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int count;
    char arg[MAX_INPUT_LENGTH];

    one_argument (argument, arg);
    count	= 0;
    buf[0]	= '\0';

    set_pager_color( AT_PLAIN, ch );
#ifndef MUD_LISTENER
    sprintf(buf, "Desc|Con|Idle |Port |Player@HostIP                                   ");
#else
    sprintf(buf, "UID |Con|Idle |Port |Player@HostIP                                   ");
#endif
    if ( get_trust(ch) >= LEVEL_GOD)
        strcat(buf, "| Username");
    strcat(buf, "\n\r");
    strcat(buf,"----+---+-----+-----+------------------------------------------------");
    if ( get_trust(ch) >= LEVEL_GOD)
        strcat(buf, "+---------");
    strcat(buf, "\n\r");
    send_to_pager(buf, ch);

    for ( d = first_descriptor; d; d = d->next )
    {
        if (arg[0] == '\0')
        {
            if (  get_trust(ch) >= LEVEL_SUPREME
                  ||   (d->character && can_see( ch, d->character )) )
            {
                count++;
                sprintf( buf,
                         " %3d|%3d|%5d|%5d|%-10s%c%-37.37s",
#ifndef MUD_LISTENER
                         d->descriptor,
#else
                         d->uid,
#endif
                         d->connected,
                         d->idle / PULSE_PER_SECOND,
                         d->port,
                         d->original  ? d->original->name  :
                         d->character ? d->character->name : "(none)",
#ifdef COMPRESS
                         d->compressing ? '#' : '@',
#else
                         '@',
#endif
                         d->host);
                if (get_trust(ch) >= LEVEL_GOD)
                    sprintf( buf + strlen( buf ), "| %s", d->user );
                strcat(buf, "\n\r");
                send_to_pager( buf, ch );
            }
        }
        else
        {
            if ( (get_trust(ch) >= LEVEL_SUPREME
                  ||   (d->character && can_see( ch, d->character )) )
                 &&   ( !str_prefix( arg, d->host )
                        ||   ( d->character && !str_prefix( arg, d->character->name ) ) ) )
            {
                count++;
                pager_printf( ch,
                              " %3d|%3d|%5d|%5d|%-10s%c%-37s",
#ifndef MUD_LISTENER
                              d->descriptor,
#else
                              d->uid,
#endif
                              d->connected,
                              d->idle / PULSE_PER_SECOND,
                              d->port,
                              d->original  ? d->original->name  :
                              d->character ? d->character->name : "(none)",
#ifdef COMPRESS
                              d->compressing ? '#' : '@',
#else
                              '@',
#endif
                              d->host
                            );
                buf[0] = '\0';
                if (get_trust(ch) >= LEVEL_GOD)
                    sprintf(buf, "| %s", d->user);
                strcat(buf, "\n\r");
                send_to_pager( buf, ch );
            }
        }
    }
    pager_printf( ch, "%d user%s.\n\r", count, count == 1 ? "" : "s" );
    return;
}



/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    char arg[MAX_INPUT_LENGTH];
    bool mobsonly;
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "Force whom to do what?\n\r", ch );
        return;
    }

    mobsonly = get_trust( ch ) < sysdata.level_forcepc;

    if ( !str_cmp( arg, "all" ) )
    {
        CHAR_DATA *vch_next;

        if ( mobsonly )
        {
            send_to_char( "Force whom to do what?\n\r", ch );
            return;
        }

        for ( vch = first_char; vch; vch = vch_next )
        {
            vch_next = vch->next;

            if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) )
            {
                act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, vch, TO_VICT );
                interpret( vch, argument );
            }
        }
        return;
    }

    if ( !str_cmp( arg, "newbie" ) )
    {
        CHAR_DATA *vch_next;

        if ( mobsonly )
        {
            send_to_char( "Force whom to do what?\n\r", ch );
            return;
        }

        for ( vch = first_char; vch; vch = vch_next )
        {
            vch_next = vch->next;

            if ( !IS_NPC(vch) && get_trust( vch ) <= 1 )
            {
                act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, vch, TO_VICT );
                interpret( vch, argument );
            }
        }
        return;
    }

    if ( ( vch = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( vch == ch )
    {
        send_to_char( "Aye aye, right away!\n\r", ch );
        return;
    }

    if ( ( get_trust( vch ) >= get_trust( ch ) )
         || ( mobsonly && !IS_NPC( vch ) ) )
    {
        send_to_char( "Do it yourself!\n\r", ch );
        return;
    }

    act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, vch, TO_VICT );
    interpret( vch, argument );

    send_to_char( "Ok.\n\r", ch );
    return;
}


void do_invis( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int level;

    /*
     if ( IS_NPC(ch))
     return;
     */

    argument = one_argument( argument, arg );
    if ( arg && arg[0] != '\0' )
    {
        if ( !is_number( arg ) )
        {
            send_to_char( "Usage: invis | invis <level>\n\r", ch );
            return;
        }
        level = atoi( arg );
        if ( level > get_trust( ch ) )
        {
            send_to_char( "Invalid level.\n\r", ch );
            return;
        }

        if (!IS_NPC(ch))
        {
            ch->pcdata->wizinvis = level;
            ch_printf( ch, "Wizinvis level set to %d.\n\r", level );
        }

        if (IS_NPC(ch))
        {
            ch->mobinvis = level;
            ch_printf( ch, "Mobinvis level set to %d.\n\r", level );
        }
        return;
    }

    if (IS_NPC(ch))
    {
        if ( ch->mobinvis < 2 )
            ch->mobinvis = GetMaxLevel(ch);
    }

    if ( ch->pcdata->wizinvis )
    {
        ch->pcdata->wizinvis = 0;
        act( AT_IMMORT, "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
        send_to_char( "You slowly fade back into existence.\n\r", ch );
    }
    else
    {
        ch->pcdata->wizinvis = LEVEL_IMMORTAL;
        act( AT_IMMORT, "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
        send_to_char( "You slowly vanish into thin air.\n\r", ch );
    }

    return;
}


void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
        return;

    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
        REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
        send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
        SET_BIT(ch->act, PLR_HOLYLIGHT);
        send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}

void do_rassign( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int  r_lo, r_hi;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    r_lo = atoi( arg2 );  r_hi = atoi( arg3 );

    if ( arg1[0] == '\0' || r_lo < 0 || r_hi < 0 )
    {
        send_to_char( "Syntax: rassign <who> <low> <high>\n\r", ch );
        return;
    }
    if ( (victim = get_char_world( ch, arg1 )) == NULL )
    {
        send_to_char( "They don't seem to be around.\n\r", ch );
        return;
    }
    if ( IS_NPC( victim ) || get_trust( victim ) < LEVEL_CREATOR )
    {
        send_to_char( "They wouldn't know what to do with a room range.\n\r", ch );
        return;
    }
    if ( r_lo > r_hi )
    {
        send_to_char( "Unacceptable room range.\n\r", ch );
        return;
    }
    if ( r_lo == 0 )
        r_hi = 0;
    victim->pcdata->r_range_lo = r_lo;
    victim->pcdata->r_range_hi = r_hi;
    assign_area( victim );
    send_to_char( "Done.\n\r", ch );
    ch_printf( victim, "%s has assigned you the room vnum range %d - %d.\n\r",
               ch->name, r_lo, r_hi );
    assign_area( victim );	/* Put back by Thoric on 02/07/96 */
    if ( !victim->pcdata->area )
    {
        bug( "rassign: assign_area failed" );
        return;
    }

    if (r_lo == 0)				/* Scryn 8/12/95 */
    {
        REMOVE_BIT ( victim->pcdata->area->status, AREA_LOADED );
        SET_BIT( victim->pcdata->area->status, AREA_DELETED );
    }
    else
    {
        SET_BIT( victim->pcdata->area->status, AREA_LOADED );
        REMOVE_BIT( victim->pcdata->area->status, AREA_DELETED );
    }
    return;
}

void do_oassign( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int  o_lo, o_hi;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    o_lo = atoi( arg2 );  o_hi = atoi( arg3 );

    if ( arg1[0] == '\0' || o_lo < 0 || o_hi < 0 )
    {
        send_to_char( "Syntax: oassign <who> <low> <high>\n\r", ch );
        return;
    }
    if ( (victim = get_char_world( ch, arg1 )) == NULL )
    {
        send_to_char( "They don't seem to be around.\n\r", ch );
        return;
    }
    if ( IS_NPC( victim ) || get_trust( victim ) < LEVEL_SAVIOR )
    {
        send_to_char( "They wouldn't know what to do with an object range.\n\r", ch );
        return;
    }
    if ( o_lo > o_hi )
    {
        send_to_char( "Unacceptable object range.\n\r", ch );
        return;
    }
    victim->pcdata->o_range_lo = o_lo;
    victim->pcdata->o_range_hi = o_hi;
    assign_area( victim );
    send_to_char( "Done.\n\r", ch );
    ch_printf( victim, "%s has assigned you the object vnum range %d - %d.\n\r",
               ch->name, o_lo, o_hi );
    return;
}

void do_massign( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int  m_lo, m_hi;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    m_lo = atoi( arg2 );  m_hi = atoi( arg3 );

    if ( arg1[0] == '\0' || m_lo < 0 || m_hi < 0 )
    {
        send_to_char( "Syntax: massign <who> <low> <high>\n\r", ch );
        return;
    }
    if ( (victim = get_char_world( ch, arg1 )) == NULL )
    {
        send_to_char( "They don't seem to be around.\n\r", ch );
        return;
    }
    if ( IS_NPC( victim ) || get_trust( victim ) < LEVEL_SAVIOR )
    {
        send_to_char( "They wouldn't know what to do with a monster range.\n\r", ch );
        return;
    }
    if ( m_lo > m_hi )
    {
        send_to_char( "Unacceptable monster range.\n\r", ch );
        return;
    }
    victim->pcdata->m_range_lo = m_lo;
    victim->pcdata->m_range_hi = m_hi;
    assign_area( victim );
    send_to_char( "Done.\n\r", ch );
    ch_printf( victim, "%s has assigned you the monster vnum range %d - %d.\n\r",
               ch->name, m_lo, m_hi );
    return;
}

void do_cmdtable( CHAR_DATA *ch, char *argument )
{
    int hash, cnt;
    CMDTYPE *cmd;

    set_pager_color( AT_PLAIN, ch );
    send_to_pager("Commands and Number of Uses This Run\n\r", ch);

    for ( cnt = hash = 0; hash < 126; hash++ )
        for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
        {
            if ((++cnt)%4)
                pager_printf(ch,"%-10.10s %4d   ",cmd->name,cmd->userec.num_uses);
            else
                pager_printf(ch,"%-10.10s %4d\n\r", cmd->name,cmd->userec.num_uses );
        }
    return;
}

/*
 * Load up a player file
 */
void do_loadup( CHAR_DATA *ch, char *argument )
{
    char fname[1024];
    char name[256];
    struct stat fst;
    bool loaded;
    DESCRIPTOR_DATA *d;
    int old_room_vnum;
    char buf[MAX_STRING_LENGTH];

    one_argument( argument, name );
    if ( name[0] == '\0' )
    {
        send_to_char( "Usage: loadup <playername>\n\r", ch );
        return;
    }

    name[0] = UPPER(name[0]);

    sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower(name[0]),
             capitalize( name ) );
    if ( stat( fname, &fst ) != -1 )
    {
        CREATE( d, DESCRIPTOR_DATA, 1 );
        d->next = NULL;
        d->prev = NULL;
        d->connected = CON_GET_NAME;
        d->outsize = 2000;
        CREATE( d->outbuf, char, d->outsize );

        loaded = load_char_obj( d, name, FALSE, ch->in_room, TRUE );
        add_char( d->character );
        old_room_vnum = d->character->in_room?d->character->in_room->vnum:ROOM_VNUM_LIMBO;
        char_to_room( d->character, ch->in_room );
        if ( get_trust(d->character) >= get_trust( ch ) )
        {
            do_say( d->character, "Do *NOT* disturb me again!" );
            send_to_char( "I think you'd better leave that player alone!\n\r", ch );
            d->character->desc	= NULL;
            do_quit( d->character, "yes" );
            return;
        }
        d->character->desc	= NULL;
        d->character->retran    = old_room_vnum;
        d->character		= NULL;
        DISPOSE( d->outbuf );
        DISPOSE( d );
        ch_printf(ch, "Player %s loaded from room %d.\n\r", capitalize( name ),old_room_vnum );
        sprintf(buf, "%s appears from nowhere, eyes glazed over.", capitalize( name ) );
        act( AT_IMMORT, buf, ch, NULL, NULL, TO_ROOM );
        send_to_char( "Done.\n\r", ch );
        return;
    }
    /* else no player file */
    send_to_char( "No such player.\n\r", ch );
    return;
}

void do_fixchar( CHAR_DATA *ch, char *argument )
{
    char name[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, name );
    if ( name[0] == '\0' )
    {
        send_to_char( "Usage: fixchar <playername>\n\r", ch );
        return;
    }
    victim = get_char_room( ch, name );
    if ( !victim )
    {
        send_to_char( "They're not here.\n\r", ch );
        return;
    }
    fix_char( victim );
    /*  victim->armor	= 100;
     victim->mod_str	= 0;
     victim->mod_dex	= 0;
     victim->mod_wis	= 0;
     victim->mod_int	= 0;
     victim->mod_con	= 0;
     victim->mod_cha	= 0;
     victim->mod_lck	= 0;
     victim->damroll	= 0;
     victim->hitroll	= 0;
     victim->alignment	= URANGE( -1000, victim->alignment, 1000 );
     victim->saving_spell_staff = 0; */
    send_to_char( "Done.\n\r", ch );
}

void do_newbieset( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument (argument, arg2);

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: newbieset <char>.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch);
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( ( GetAveLevel(victim) < 1 ) || ( GetAveLevel(victim) > 5 ) )
    {
        send_to_char( "Level of victim must be 1 to 5.\n\r", ch );
        return;
    }
    obj = create_object( OBJ_VNUM_SCHOOL_VEST );
    if (obj)
        obj_to_char(obj, victim);

    obj = create_object( OBJ_VNUM_SCHOOL_SHIELD );
    if (obj)
        obj_to_char(obj, victim);

    obj = create_object( OBJ_VNUM_SCHOOL_BANNER );
    if (obj)
        obj_to_char(obj, victim);

    if ( IS_ACTIVE(victim, CLASS_MAGE) || IS_ACTIVE(victim, CLASS_THIEF)
         || IS_ACTIVE(victim, CLASS_VAMPIRE) )
    {
        obj = create_object( OBJ_VNUM_SCHOOL_DAGGER );
        if (obj)
            obj_to_char(obj, victim);
    }
    else if ( IS_ACTIVE(victim, CLASS_CLERIC) || IS_ACTIVE(victim, CLASS_DRUID) )
    {
        obj = create_object( OBJ_VNUM_SCHOOL_MACE );
        if (obj)
            obj_to_char(obj, victim);
    }
    else if ( IS_ACTIVE(victim, CLASS_WARRIOR) || IS_ACTIVE(victim, CLASS_RANGER) ||
              IS_ACTIVE(victim, CLASS_AMAZON) )
    {
        obj = create_object( OBJ_VNUM_SCHOOL_SWORD );
        if (obj)
            obj_to_char(obj, victim);
    }

    /* Added by Brittany, on Nov. 24, 1996. The object is the adventurer's
     guide to the realms of despair, part of academy.are. */
    /* Isn't she special? *rofl* -Garil Jun. 17, 1997. */
    if ( (obj = create_object( 10333 )) )
        obj_to_char( obj, victim );

    /* Added the burlap sack to the newbieset.  The sack is part of sgate.are
     called Spectral Gate.  Brittany */
    if ( (obj = create_object( 123 )) )
        obj_to_char( obj, victim );

    act( AT_IMMORT, "$n has equipped you with a newbieset.", ch, NULL, victim, TO_VICT);
    ch_printf( ch, "You have re-equipped %s.\n\r", victim->name );
    return;
}

/*
 * Extract area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "joe.are susan.are"
 * - Gorog
 */
void extract_area_names (char *inp, char *out)
{
    char buf[MAX_INPUT_LENGTH], *pbuf=buf;
    int  len;

    *out='\0';
    while (inp && *inp)
    {
        inp = one_argument(inp, buf);
        if ( (len=strlen(buf)) >= 5 && !strcmp(".are", pbuf+len-4) )
        {
            if (*out) strcat (out, " ");
            strcat (out, buf);
        }
    }
}

/*
 * Remove area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "aset sedit cset"
 * - Gorog
 */
void remove_area_names (char *inp, char *out)
{
    char buf[MAX_INPUT_LENGTH], *pbuf=buf;
    int  len;

    *out='\0';
    while (inp && *inp)
    {
        inp = one_argument(inp, buf);
        if ( (len=strlen(buf)) < 5 || strcmp(".are", pbuf+len-4) )
        {
            if (*out) strcat (out, " ");
            strcat (out, buf);
        }
    }
}

/*
 * Allows members of the Area Council to add Area names to the bestow field.
 * Area must exist not not be proto
 */
void do_bestowarea( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    AREA_DATA *pArea;

    argument = one_argument( argument, arg );

    if ( str_cmp( ch->pcdata->council_name, "Area Council" )
         &&   get_trust (ch) < LEVEL_SUB_IMPLEM )
    {
        send_to_char( "Sorry. You are not on the Area Council.\n\r", ch );
        return;
    }

    if ( !*arg )
    {
        send_to_char(
                     "Syntax:\n\r"
                     "bestowarea <victim> <filename>.are\n\r"
                     "bestowarea <victim> none             removes bestowed areas\n\r"
                     "bestowarea <victim> list             lists bestowed areas\n\r"
                     "bestowarea <victim>                  lists bestowed areas\n\r", ch);
        return;
    }

    if ( !(victim = get_char_world( ch, arg )) )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "You can't give special abilities to a mob!\n\r", ch );
        return;
    }

    if ( get_trust(victim) < LEVEL_IMMORTAL )
    {
        send_to_char( "They aren't an immortal.\n\r", ch );
        return;
    }

    if (!victim->pcdata->bestowments)
        victim->pcdata->bestowments = str_dup("");

    if ( !*argument || !str_cmp (argument, "list") )
    {
        extract_area_names (victim->pcdata->bestowments, buf);
        ch_printf( ch, "Bestowed areas: %s\n\r", buf);
        return;
    }

    if ( !str_cmp (argument, "none") )
    {
        remove_area_names (victim->pcdata->bestowments, buf);
        DISPOSE( victim->pcdata->bestowments );
        victim->pcdata->bestowments = str_dup( buf );
        send_to_char( "Done.\n\r", ch);
        return;
    }

    for (pArea = first_area; pArea; pArea = pArea->next)
    {
        if (!str_cmp(argument, pArea->filename))
            break;
    }

    if (!pArea || str_cmp(argument, pArea->filename))
    {
        send_to_char("That isn't a valid area to bestow.\n\r", ch);
        return;
    }

    sprintf( buf, "%s %s", victim->pcdata->bestowments, pArea->filename );
    DISPOSE( victim->pcdata->bestowments );
    victim->pcdata->bestowments = str_dup( buf );
    ch_printf( victim, "%s has bestowed on you the area: %s\n\r",
               ch->name, pArea->filename );
    send_to_char( "Done.\n\r", ch );
}

void do_bestow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Bestow whom with what?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "You can't give special abilities to a mob!\n\r", ch );
        return;
    }

    if ( get_trust( victim ) > get_trust( ch ) )
    {
        send_to_char( "You aren't powerful enough...\n\r", ch );
        return;
    }

    if (!victim->pcdata->bestowments)
        victim->pcdata->bestowments = str_dup("");

    if ( argument[0] == '\0' || !str_cmp( argument, "list" ) )
    {
        ch_printf( ch, "Current bestowed commands on %s: %s.\n\r",
                   victim->name, victim->pcdata->bestowments );
        return;
    }

    if ( !str_cmp( argument, "none" ) )
    {
        DISPOSE( victim->pcdata->bestowments );
        victim->pcdata->bestowments = str_dup("");
        ch_printf( ch, "Bestowments removed from %s.\n\r", victim->name );
        ch_printf( victim, "%s has removed your bestowed commands.\n\r", ch->name );
        return;
    }

    sprintf( buf, "%s %s", victim->pcdata->bestowments, argument );
    DISPOSE( victim->pcdata->bestowments );
    victim->pcdata->bestowments = str_dup( buf );
    ch_printf( victim, "%s has bestowed on you the command(s): %s\n\r",
               ch->name, argument );
    send_to_char( "Done.\n\r", ch );
}

struct tm *update_time ( struct tm *old_time )
{
    time_t new_time;

    new_time = mktime(old_time);
    return localtime(&new_time);
}

void do_set_boot_time( CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    bool check;

    check = FALSE;

    argument = one_argument(argument, arg);

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: setboot time {hour minute <day> <month> <year>}\n\r", ch);
        send_to_char( "        setboot manual {0/1}\n\r", ch);
        send_to_char( "        setboot default\n\r", ch);
        ch_printf( ch, "Boot time is currently set to %s, manual bit is set to %d\n\r"
                   ,reboot_time, set_boot_time->manual );
        return;
    }

    if ( !str_cmp(arg, "time") )
    {
        struct tm *now_time;

        argument = one_argument(argument, arg);
        argument = one_argument(argument, arg1);
        if ( !*arg || !*arg1 || !is_number(arg) || !is_number(arg1) )
        {
            send_to_char("You must input a value for hour and minute.\n\r", ch);
            return;
        }
        now_time = localtime(&current_time);

        if ( (now_time->tm_hour = atoi(arg)) < 0 || now_time->tm_hour > 23 )
        {
            send_to_char("Valid range for hour is 0 to 23.\n\r", ch);
            return;
        }

        if ( (now_time->tm_min = atoi(arg1)) < 0 || now_time->tm_min > 59 )
        {
            send_to_char("Valid range for minute is 0 to 59.\n\r", ch);
            return;
        }

        argument = one_argument(argument, arg);
        if ( *arg != '\0' && is_number(arg) )
        {
            if ( (now_time->tm_mday = atoi(arg)) < 1 || now_time->tm_mday > 31 )
            {
                send_to_char("Valid range for day is 1 to 31.\n\r", ch);
                return;
            }
            argument = one_argument(argument, arg);
            if ( *arg != '\0' && is_number(arg) )
            {
                if ( (now_time->tm_mon = atoi(arg)) < 1 || now_time->tm_mon > 12 )
                {
                    send_to_char( "Valid range for month is 1 to 12.\n\r", ch );
                    return;
                }
                now_time->tm_mon--;
                argument = one_argument(argument, arg);
                if ( (now_time->tm_year = atoi(arg)-1900) < 0 ||
                     now_time->tm_year > 199 )
                {
                    send_to_char( "Valid range for year is 1900 to 2099.\n\r", ch );
                    return;
                }
            }
        }
        now_time->tm_sec = 0;
        if ( mktime(now_time) < current_time )
        {
            send_to_char( "You can't set a time previous to today!\n\r", ch );
            return;
        }
        if (set_boot_time->manual == 0)
            set_boot_time->manual = 1;
        new_boot_time = update_time(now_time);
        new_boot_struct = *new_boot_time;
        new_boot_time = &new_boot_struct;
        reboot_check(mktime(new_boot_time));
        get_reboot_string();

        ch_printf(ch, "Boot time set to %s\n\r", reboot_time);
        check = TRUE;
    }
    else if ( !str_cmp(arg, "manual") )
    {
        argument = one_argument(argument, arg1);
        if (arg1[0] == '\0')
        {
            send_to_char("Please enter a value for manual boot on/off\n\r", ch);
            return;
        }

        if ( !is_number(arg1))
        {
            send_to_char("Value for manual must be 0 (off) or 1 (on)\n\r", ch);
            return;
        }

        if (atoi(arg1) < 0 || atoi(arg1) > 1)
        {
            send_to_char("Value for manual must be 0 (off) or 1 (on)\n\r", ch);
            return;
        }

        set_boot_time->manual = atoi(arg1);
        ch_printf(ch, "Manual bit set to %s\n\r", arg1);
        check = TRUE;
        get_reboot_string();
        return;
    }

    else if (!str_cmp( arg, "default" ))
    {
        set_boot_time->manual = 0;
        /* Reinitialize new_boot_time */
        new_boot_time = localtime(&current_time);
        new_boot_time->tm_mday += 1;
        if (new_boot_time->tm_hour > 12)
            new_boot_time->tm_mday += 1;
        new_boot_time->tm_hour = 6;
        new_boot_time->tm_min = 0;
        new_boot_time->tm_sec = 0;
        new_boot_time = update_time(new_boot_time);

        sysdata.DENY_NEW_PLAYERS = FALSE;

        send_to_char("Reboot time set back to normal.\n\r", ch);
        check = TRUE;
    }

    if (!check)
    {
        send_to_char("Invalid argument for setboot.\n\r", ch);
        return;
    }

    else
    {
        get_reboot_string();
        new_boot_time_t = mktime(new_boot_time);
    }
}

/* Online high level immortal command for displaying what the encryption
 * of a name/password would be, taking in 2 arguments - the name and the
 * password - can still only change the password if you have access to
 * pfiles and the correct password
 */
void do_form_password( CHAR_DATA *ch, char *argument)
{
    char arg[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg);

    ch_printf(ch, "L:%s P:%s encrypted would result in: %s\n\r",
              argument, arg,
              crypt(arg, argument));
    return;
}

/*
 * Purge a player file.  No more player.  -- Altrag
 */
void do_destro( CHAR_DATA *ch, char *argument )
{
    set_char_color( AT_PLAIN, ch );
    send_to_char("If you want to destroy a character, spell it out!\n\r",ch);
    return;
}

/*
 * This could have other applications too.. move if needed. -- Altrag
 */
void close_area( AREA_DATA *pArea, bool build )
{
    CHAR_DATA *ech;
    CHAR_DATA *ech_next;
    OBJ_DATA *eobj;
    OBJ_DATA *eobj_next;
    int icnt;
    ROOM_INDEX_DATA *rid;
    ROOM_INDEX_DATA *rid_next;
    OBJ_INDEX_DATA *oid;
    OBJ_INDEX_DATA *oid_next;
    MOB_INDEX_DATA *mid;
    MOB_INDEX_DATA *mid_next;
    RESET_DATA *ereset;
    RESET_DATA *ereset_next;
    EXTRA_DESCR_DATA *eed;
    EXTRA_DESCR_DATA *eed_next;
    EXIT_DATA *pexit;
    EXIT_DATA *exit_next;
    MPROG_ACT_LIST *mpact;
    MPROG_ACT_LIST *mpact_next;
    MPROG_DATA *mprog;
    MPROG_DATA *mprog_next;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    NEIGHBOR_DATA *neigh;
    NEIGHBOR_DATA *neigh_next;


    fprintf(stderr, " Area: %s\n", pArea->filename);

    for ( ech = first_char; ech; ech = ech_next )
    {
        ech_next = ech->next;

	if ( char_died(ech) )
	    continue;

	if ( ech->in_room->area == pArea )
	{
	    if ( !IS_NPC( ech ) )
		save_char_obj( ech );
	    extract_char( ech, TRUE );
            continue;
	}
        if ( IS_NPC( ech ) )
        {
	    /* if mob is in area, or part of area. */
	    if ( URANGE(pArea->low_m_vnum, ech->vnum,
			pArea->hi_m_vnum) == ech->vnum )
                extract_char( ech, TRUE );
            continue;
	}
    }

    for ( eobj = first_object; eobj; eobj = eobj_next )
    {
	eobj_next = eobj->next;

        /* if obj is in area, or part of area. */
        if ( URANGE(pArea->low_o_vnum, eobj->vnum,
                    pArea->hi_o_vnum) == eobj->vnum ||
             (eobj->in_room && eobj->in_room->area == pArea) )
            extract_obj( eobj );
    }

    for ( icnt = 0; icnt < MAX_KEY_HASH; icnt++ )
    {
        for ( rid = room_index_hash[icnt]; rid; rid = rid_next )
        {
            rid_next = rid->next;
            for ( pexit = rid->first_exit; pexit; pexit = exit_next )
            {
		exit_next = pexit->next;
		if ( pexit->to_room && pexit->to_room->area == pArea )
                {
                    STRFREE( pexit->keyword );
                    STRFREE( pexit->description );
                    UNLINK( pexit, rid->first_exit, rid->last_exit, next, prev );
                    DISPOSE( pexit );
                }
            }
	}
    }

    for ( icnt = 0; icnt < MAX_KEY_HASH; icnt++ )
    {
        for ( rid = room_index_hash[icnt]; rid; rid = rid_next )
        {
            rid_next = rid->next;

            if ( rid->area != pArea )
                continue;

            for ( pexit = rid->first_exit; pexit; pexit = exit_next )
            {
		exit_next = pexit->next;
                STRFREE( pexit->keyword );
                STRFREE( pexit->description );
                UNLINK( pexit, rid->first_exit, rid->last_exit, next, prev );
                DISPOSE( pexit );
            }

            STRFREE(rid->name);
            STRFREE(rid->description);
            if ( rid->first_person )
            {
                bug( "close_area: room with people #%d", rid->vnum );
                for ( ech = rid->first_person; ech; ech = ech_next )
                {
                    ech_next = ech->next_in_room;
                    if ( ech->fighting )
			stop_fighting( ech, TRUE );
		    if ( !IS_NPC( ech ) )
			save_char_obj( ech );
		    extract_char( ech, TRUE );
                }
            }
            if ( rid->first_content )
            {
                bug( "close_area: room with contents #%d", rid->vnum );
                eobj_next = rid->first_content;
                while ((eobj=eobj_next))
                {
                    eobj_next = eobj->next_content;
                    extract_obj( eobj );
                }
            }
            for ( eed = rid->first_extradesc; eed; eed = eed_next )
            {
                eed_next = eed->next;
                STRFREE( eed->keyword );
                STRFREE( eed->description );
                DISPOSE( eed );
            }
            for ( mpact = rid->mpact; mpact; mpact = mpact_next )
            {
                mpact_next = mpact->next;
                DISPOSE( mpact->buf );
                DISPOSE( mpact );
            }
            for ( mprog = rid->mudprogs; mprog; mprog = mprog_next )
            {
                mprog_next = mprog->next;
                STRFREE( mprog->arglist );
                STRFREE( mprog->comlist );
                DISPOSE( mprog );
            }
            if ( rid == room_index_hash[icnt] )
                room_index_hash[icnt] = rid->next;
            else
            {
                ROOM_INDEX_DATA *trid;

                for ( trid = room_index_hash[icnt]; trid; trid = trid->next )
                    if ( trid->next == rid )
                        break;
                if ( !trid )
                {
                    bug( "Close_area: rid not in hash list %d", rid->vnum );
                }
                else
                    trid->next = rid->next;
            }
            DISPOSE(rid);
        }

        for ( mid = mob_index_hash[icnt]; mid; mid = mid_next )
        {
            mid_next = mid->next;

            if ( mid->ivnum < pArea->low_m_vnum || mid->ivnum > pArea->hi_m_vnum )
                continue;

            STRFREE( mid->player_name );
            STRFREE( mid->short_descr );
            STRFREE( mid->long_descr  );
            STRFREE( mid->description );
            if ( mid->pShop )
            {
                UNLINK( mid->pShop, first_shop, last_shop, next, prev );
                DISPOSE( mid->pShop );
            }
            if ( mid->rShop )
            {
                UNLINK( mid->rShop, first_repair, last_repair, next, prev );
                DISPOSE( mid->rShop );
            }
            for ( mprog = mid->mudprogs; mprog; mprog = mprog_next )
            {
                mprog_next = mprog->next;
                STRFREE(mprog->arglist);
                STRFREE(mprog->comlist);
                DISPOSE(mprog);
            }
            if ( mid == mob_index_hash[icnt] )
                mob_index_hash[icnt] = mid->next;
            else
            {
                MOB_INDEX_DATA *tmid;

                for ( tmid = mob_index_hash[icnt]; tmid; tmid = tmid->next )
                    if ( tmid->next == mid )
                        break;
                if ( !tmid )
                {
                    bug( "Close_area: mid not in hash list %d", mid->ivnum );
                }
                else
                    tmid->next = mid->next;
            }
            DISPOSE(mid);
        }

        for ( oid = obj_index_hash[icnt]; oid; oid = oid_next )
        {
            oid_next = oid->next;

            if ( oid->ivnum < pArea->low_o_vnum || oid->ivnum > pArea->hi_o_vnum )
                continue;

            STRFREE(oid->name);
            STRFREE(oid->short_descr);
            STRFREE(oid->description);
            STRFREE(oid->action_desc);

            for ( eed = oid->first_extradesc; eed; eed = eed_next )
            {
                eed_next = eed->next;
                STRFREE(eed->keyword);
                STRFREE(eed->description);
                DISPOSE(eed);
            }
            for ( paf = oid->first_affect; paf; paf = paf_next )
            {
                paf_next = paf->next;
                DISPOSE(paf);
            }
            for ( mprog = oid->mudprogs; mprog; mprog = mprog_next )
            {
                mprog_next = mprog->next;
                STRFREE(mprog->arglist);
                STRFREE(mprog->comlist);
                DISPOSE(mprog);
            }
            if ( oid == obj_index_hash[icnt] )
                obj_index_hash[icnt] = oid->next;
            else
            {
                OBJ_INDEX_DATA *toid;

                for ( toid = obj_index_hash[icnt]; toid; toid = toid->next )
                    if ( toid->next == oid )
                        break;
                if ( !toid )
                {
                    bug( "Close_area: oid not in hash list %d", oid->ivnum );
                }
                else
                    toid->next = oid->next;
            }
            DISPOSE(oid);
        }
    }
    for ( ereset = pArea->first_reset; ereset; ereset = ereset_next )
    {
        ereset_next = ereset->next;
        DISPOSE(ereset);
    }
    if (pArea->name)
        DISPOSE(pArea->name);
#ifndef USE_DB
    DISPOSE(pArea->filename);
#endif
    if (pArea->author)
        STRFREE(pArea->author);
    if (pArea->weather)
    {
	for (neigh=pArea->weather->first_neighbor;neigh;neigh=neigh_next)
	{
	    neigh_next = neigh->next;
	    STRFREE(neigh->name);
	    UNLINK(neigh,pArea->weather->first_neighbor,pArea->weather->last_neighbor,next,prev);
	    DISPOSE(neigh);
	}
	DISPOSE(pArea->weather);
    }
    if (pArea->resetmsg)
        DISPOSE(pArea->resetmsg);
    if (pArea->comment)
	DISPOSE(pArea->comment);
    if (build)
	UNLINK( pArea, first_build, last_build, next, prev );
    else
	UNLINK( pArea, first_area, last_area, next, prev );
    UNLINK( pArea, first_asort, last_asort, next_sort, prev_sort );
    DISPOSE( pArea );
}

void do_destroy( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Destroy what player file?\n\r", ch );
        return;
    }

    for ( victim = first_char; victim; victim = victim->next )
        if ( !IS_NPC(victim) && !str_cmp(victim->name, arg) )
            break;
    if ( !victim )
    {
        DESCRIPTOR_DATA *d;

        /* Make sure they aren't halfway logged in. */
        for ( d = first_descriptor; d; d = d->next )
            if ( (victim = d->character) && !IS_NPC(victim) &&
                 !str_cmp(victim->name, arg) )
                break;
        if ( d )
            close_socket( d, TRUE );
    }
    else
    {
        int x, y, vlev;

        quitting_char = victim;
        save_char_obj( victim );
        delete_char_rare_obj( victim );
        saving_char = NULL;
        vlev = GetMaxLevel(victim);
        extract_char( victim, TRUE );
        for ( x = 0; x < MAX_WEAR; x++ )
            for ( y = 0; y < MAX_LAYERS; y++ )
                save_equipment[x][y] = NULL;

        if (vlev <= 1)
        {
            send_to_char("Player destroyed.\n\r", ch);
            return;
        }
    }

    sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower(arg[0]),
             capitalize( arg ) );
    sprintf( buf2, "%s%c/%s", BACKUP_DIR, tolower(arg[0]),
             capitalize( arg ) );
    if ( !rename( buf, buf2 ) )
    {
        AREA_DATA *pArea;

        set_char_color( AT_RED, ch );
        send_to_char( "Player destroyed.  Pfile saved in backup directory.\n\r", ch );
        sprintf( buf, "%s%s", GOD_DIR, capitalize(arg) );
        if ( !remove( buf ) )
            send_to_char( "Player's immortal data destroyed.\n\r", ch );
        else if ( errno != ENOENT )
        {
            ch_printf( ch, "Unknown error #%d - %s (immortal data).  Report to Thoric.\n\r",
                       errno, strerror( errno ) );
            sprintf( buf2, "%s destroying %s", ch->name, buf );
            perror( buf2 );
        }

        sprintf( buf2, "%s.are", capitalize(arg) );
        for ( pArea = first_build; pArea; pArea = pArea->next )
            if ( !strcmp( pArea->name, buf2 ) )
            {
                sprintf( buf, "%s%s", BUILD_DIR, buf2 );
                if ( IS_SET( pArea->status, AREA_LOADED ) )
                    fold_area( pArea, buf, FALSE );
                close_area( pArea, TRUE );
                sprintf( buf2, "%s.bak", buf );
                set_char_color( AT_RED, ch ); /* Log message changes colors */
                if ( !rename( buf, buf2 ) )
                    send_to_char( "Player's area data destroyed.  Area saved as backup.\n\r", ch );
                else if ( errno != ENOENT )
                {
                    ch_printf( ch, "Unknown error #%d - %s (area data).  Report to Thoric.\n\r",
                               errno, strerror( errno ) );
                    sprintf( buf2, "%s destroying %s", ch->name, buf );
                    perror( buf2 );
                }
            }
    }
    else if ( errno == ENOENT )
    {
        set_char_color( AT_PLAIN, ch );
        send_to_char( "Player does not exist.\n\r", ch );
    }
    else
    {
        set_char_color( AT_WHITE, ch );
        ch_printf( ch, "Unknown error #%d - %s.  Report to Thoric.\n\r",
                   errno, strerror( errno ) );
        sprintf( buf, "%s destroying %s", ch->name, arg );
        perror( buf );
    }
    return;
}


/* Super-AT command:

 FOR ALL <action>
 FOR MORTALS <action>
 FOR GODS <action>
 FOR MOBS <action>
 FOR EVERYWHERE <action>


 Executes action several times, either on ALL players (not including yourself),
 MORTALS (including trusted characters), GODS (characters with level higher than
 L_HERO), MOBS (Not recommended) or every room (not recommended either!)

 If you insert a # in the action, it will be replaced by the name of the target.

 If # is a part of the action, the action will be executed for every target
 in game. If there is no #, the action will be executed for every room containg
 at least one target, but only once per room. # cannot be used with FOR EVERY-
 WHERE. # can be anywhere in the action.

 Example:

 FOR ALL SMILE -> you will only smile once in a room with 2 players.
 FOR ALL TWIDDLE # -> In a room with A and B, you will twiddle A then B.

 Destroying the characters this command acts upon MAY cause it to fail. Try to
 avoid something like FOR MOBS PURGE (although it actually works at my MUD).

 FOR MOBS TRANS 3054 (transfer ALL the mobs to Midgaard temple) does NOT work
 though :)

 The command works by transporting the character to each of the rooms with
 target in them. Private rooms are not violated.

 */

/* Expand the name of a character into a string that identifies THAT
 character within a room. E.g. the second 'guard' -> 2. guard
 */
const char * name_expand (CHAR_DATA *ch)
{
    int count = 1;
    CHAR_DATA *rch;
    char name[MAX_INPUT_LENGTH]; /*  HOPEFULLY no mob has a name longer than THAT */

    static char outbuf[MAX_INPUT_LENGTH];

    if (!IS_NPC(ch))
        return ch->name;

    one_argument (ch->name, name); /* copy the first word into name */

    if (!name[0]) /* weird mob .. no keywords */
    {
        strcpy (outbuf, ""); /* Do not return NULL, just an empty buffer */
        return outbuf;
    }

    /* ->people changed to ->first_person -- TRI */
    for (rch = ch->in_room->first_person; rch && (rch != ch);rch =
         rch->next_in_room)
        if (is_name (name, rch->name))
            count++;


    sprintf (outbuf, "%d.%s", count, name);
    return outbuf;
}


void do_for (CHAR_DATA *ch, char *argument)
{
    char range[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    bool fGods = FALSE, fMortals = FALSE, fMobs = FALSE, fEverywhere = FALSE, found;
    ROOM_INDEX_DATA *room, *old_room;
    CHAR_DATA *p, *p_prev;  /* p_next to p_prev -- TRI */
    int i;

    argument = one_argument (argument, range);

    if (!range[0] || !argument[0]) /* invalid usage? */
    {
        do_help (ch, "for");
        return;
    }

    if (!str_prefix("quit", argument))
    {
        send_to_char ("Are you trying to crash the MUD or something?\n\r",ch);
        return;
    }


    if (!str_cmp (range, "all"))
    {
        fMortals = TRUE;
        fGods = TRUE;
    }
    else if (!str_cmp (range, "gods"))
        fGods = TRUE;
    else if (!str_cmp (range, "mortals"))
        fMortals = TRUE;
    else if (!str_cmp (range, "mobs"))
        fMobs = TRUE;
    else if (!str_cmp (range, "everywhere"))
        fEverywhere = TRUE;
    else
        do_help (ch, "for"); /* show syntax */

    /* do not allow # to make it easier */
    if (fEverywhere && strchr (argument, '#'))
    {
        send_to_char ("Cannot use FOR EVERYWHERE with the # thingie.\n\r",ch);
        return;
    }

    if (strchr (argument, '#')) /* replace # ? */
    {
        /* char_list - last_char, p_next - gch_prev -- TRI */
        for (p = last_char; p ; p = p_prev )
        {
            p_prev = p->prev;  /* TRI */
            /*	p_next = p->next; */ /* In case someone DOES try to AT MOBS SLAY # */
            found = FALSE;

            if (!(p->in_room) || room_is_private(p->in_room) || (p == ch))
                continue;

            if (IS_NPC(p) && fMobs)
                found = TRUE;
            else if (!IS_NPC(p) && GetMaxLevel(p) >= LEVEL_IMMORTAL && fGods)
                found = TRUE;
            else if (!IS_NPC(p) && GetMaxLevel(p) < LEVEL_IMMORTAL && fMortals)
                found = TRUE;

            /* It looks ugly to me.. but it works :) */
            if (found) /* p is 'appropriate' */
            {
                char *pSource = argument; /* head of buffer to be parsed */
                char *pDest = buf; /* parse into this */

                while (*pSource)
                {
                    if (*pSource == '#') /* Replace # with name of target */
                    {
                        const char *namebuf = name_expand (p);

                        if (namebuf) /* in case there is no mob name ?? */
                            while (*namebuf) /* copy name over */
                                *(pDest++) = *(namebuf++);

                        pSource++;
                    }
                    else
                        *(pDest++) = *(pSource++);
                } /* while */
                *pDest = '\0'; /* Terminate */

                /* Execute */
                old_room = ch->in_room;
                char_from_room (ch);
                char_to_room (ch,p->in_room);
                interpret (ch, buf);
                char_from_room (ch);
                char_to_room (ch,old_room);

            } /* if found */
        } /* for every char */
    }
    else /* just for every room with the appropriate people in it */
    {
        for (i = 0; i < MAX_KEY_HASH; i++) /* run through all the buckets */
            for (room = room_index_hash[i] ; room ; room = room->next)
            {
                found = FALSE;

                /* Anyone in here at all? */
                if (fEverywhere) /* Everywhere executes always */
                    found = TRUE;
                else if (!room->first_person) /* Skip it if room is empty */
                    continue;
                /* ->people changed to first_person -- TRI */

                /* Check if there is anyone here of the requried type */
                /* Stop as soon as a match is found or there are no more ppl in room */
                /* ->people to ->first_person -- TRI */
                for (p = room->first_person; p && !found; p = p->next_in_room)
                {

                    if (p == ch) /* do not execute on oneself */
                        continue;

                    if (IS_NPC(p) && fMobs)
                        found = TRUE;
                    else if (!IS_NPC(p) && (GetMaxLevel(p) >= LEVEL_IMMORTAL) && fGods)
                        found = TRUE;
                    else if (!IS_NPC(p) && (GetMaxLevel(p) <= LEVEL_IMMORTAL) && fMortals)
                        found = TRUE;
                } /* for everyone inside the room */

                if (found && !room_is_private(room)) /* Any of the required type here AND room not private? */
                {
                    /* This may be ineffective. Consider moving character out of old_room
                     once at beginning of command then moving back at the end.
                     This however, is more safe?
                     */

                    old_room = ch->in_room;
                    char_from_room (ch);
                    char_to_room (ch, room);
                    interpret (ch, argument);
                    char_from_room (ch);
                    char_to_room (ch, old_room);
                } /* if found */
            } /* for every room in a bucket */
    } /* if strchr */
} /* do_for */

void save_sysdata  args( ( SYSTEM_DATA sys ) );

void do_cset( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_STRING_LENGTH];
    sh_int level;
    int chan;

    set_char_color( AT_IMMORT, ch );

    if (argument[0] == '\0')
    {
        ch_printf(ch, "Mail:\n\r  Read all mail: %d. Read mail for free: %d. Write mail for free: %d.\n\r",
                  sysdata.read_all_mail, sysdata.read_mail_free, sysdata.write_mail_free );
        ch_printf(ch, "  Take all mail: %d.",
                  sysdata.take_others_mail);
        ch_printf(ch, "Channels:\n\r  Muse: %d. Think: %d. Log: %d. Build: %d.\n\r",
                  sysdata.muse_level, sysdata.think_level, sysdata.log_level,
                  sysdata.build_level);
        ch_printf(ch, "Building:\n\r  Prototype modification: %d.  Player msetting: %d.\n\r",
                  sysdata.level_modify_proto, sysdata.level_mset_player );
        ch_printf(ch, "Guilds:\n\r  Overseer: %s.  Advisor: %s.\n\r",
                  sysdata.guild_overseer, sysdata.guild_advisor );
        ch_printf(ch, "Other:\n\r  Force on players: %d.  ", sysdata.level_forcepc);
        ch_printf(ch, "Private room override: %d.\n\r", sysdata.level_override_private);
        ch_printf(ch, "  Penalty to regular stun chance: %d.  ", sysdata.stun_regular );
        ch_printf(ch, "Penalty to stun plr vs. plr: %d.\n\r", sysdata.stun_plr_vs_plr );
        ch_printf(ch, "  Percent damage plr vs. plr: %3d.  ", sysdata.dam_plr_vs_plr );
        ch_printf(ch, "Percent damage plr vs. mob: %d.\n\r", sysdata.dam_plr_vs_mob );
        ch_printf(ch, "  Percent damage mob vs. plr: %3d.  ", sysdata.dam_mob_vs_plr );
        ch_printf(ch, "Percent damage mob vs. mob: %d.\n\r", sysdata.dam_mob_vs_mob );
        ch_printf(ch, "  Mobile Aggression: %d%%.\n\r", sysdata.percent_aggr );
        ch_printf(ch, "  Get object without take flag: %d.  ", sysdata.level_getobjnotake);
        ch_printf(ch, "Autosave frequency (minutes): %d.\n\r", sysdata.save_frequency );
        ch_printf(ch, "  Save flags: %s\n\r", flag_string( sysdata.save_flags, save_flag ) );
        ch_printf(ch, "LogDefs:");
        for (chan=0;chan<LOG_LAST;chan++)
        {
            if (chan%4==0)
                ch_printf(ch, "\n\r");
            ch_printf(ch, "  %s: %d.",
                      sysdata.logdefs[chan].name,
                      sysdata.logdefs[chan].level);
        }
        return;
    }

    argument = one_argument( argument, arg );

    if (!str_cmp(arg, "help"))
    {
        do_help(ch, "controls");
        return;
    }

    if (!str_cmp(arg, "save"))
    {
        save_sysdata(sysdata);
        send_to_char("Ok.\n\r", ch);
        return;
    }

    for (chan=0;chan<LOG_LAST;chan++)
        if (!str_cmp(sysdata.logdefs[chan].name, arg))
            break;
    if (!str_cmp(sysdata.logdefs[chan].name, arg))
    {
        argument = one_argument( argument, arg );
        if (!str_cmp(arg, "name"))
        {
            if (sysdata.logdefs[chan].name)
                STRFREE(sysdata.logdefs[chan].name);
            sysdata.logdefs[chan].name = STRALLOC( argument );
        }
        else if (!str_cmp(arg, "level"))
            sysdata.logdefs[chan].level = atoi( argument );
        else
            send_to_char("Unknown field for cset logdef.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "saveflag"))
    {
        int x = get_saveflag( argument );

        if ( x == -1 )
            send_to_char( "Not a save flag.\n\r", ch );
        else
        {
            TOGGLE_BIT( sysdata.save_flags, 1 << x );
            send_to_char( "Ok.\n\r", ch );
        }
        return;
    }

    if (!str_prefix( arg, "denynewplayers" ) )
    {
        sysdata.DENY_NEW_PLAYERS = !sysdata.DENY_NEW_PLAYERS;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_prefix( arg, "specialsenabled" ) )
    {
        sysdata.specials_enabled = !sysdata.specials_enabled;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_prefix( arg, "guild_overseer" ) )
    {
        STRFREE( sysdata.guild_overseer );
        sysdata.guild_overseer = STRALLOC( argument );
        send_to_char("Ok.\n\r", ch);
        return;
    }
    if (!str_prefix( arg, "guild_advisor" ) )
    {
        STRFREE( sysdata.guild_advisor );
        sysdata.guild_advisor = STRALLOC( argument );
        send_to_char("Ok.\n\r", ch);
        return;
    }

    level = (sh_int) atoi(argument);

    if (!str_prefix( arg, "savefrequency" ) )
    {
        sysdata.save_frequency = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "stun"))
    {
        sysdata.stun_regular = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "stun_pvp"))
    {
        sysdata.stun_plr_vs_plr = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "dam_pvp"))
    {
        sysdata.dam_plr_vs_plr = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "get_notake"))
    {
        sysdata.level_getobjnotake = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "dam_pvm"))
    {
        sysdata.dam_plr_vs_mob = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "dam_mvp"))
    {
        sysdata.dam_mob_vs_plr = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "dam_mvm"))
    {
        sysdata.dam_mob_vs_mob = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }
    if (!str_cmp(arg, "mobaggr"))
    {
        sysdata.percent_aggr = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "intro"))
    {
        sysdata.intro_disabled = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (level < 0 || level > MAX_LEVEL)
    {
        send_to_char("Invalid value for new control.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "read_all"))
    {
        sysdata.read_all_mail = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }
    if (!str_cmp(arg, "read_free"))
    {
        sysdata.read_mail_free = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }
    if (!str_cmp(arg, "write_free"))
    {
        sysdata.write_mail_free = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }
    if (!str_cmp(arg, "take_all"))
    {
        sysdata.take_others_mail = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }
    if (!str_cmp(arg, "muse"))
    {
        sysdata.muse_level = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }
    if (!str_cmp(arg, "think"))
    {
        sysdata.think_level = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }
    if (!str_cmp(arg, "log"))
    {
        sysdata.log_level = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }
    if (!str_cmp(arg, "build"))
    {
        sysdata.build_level = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }
    if (!str_cmp(arg, "proto_modify"))
    {
        sysdata.level_modify_proto = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }
    if (!str_cmp(arg, "override_private"))
    {
        sysdata.level_override_private = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }
    if (!str_cmp(arg, "forcepc"))
    {
        sysdata.level_forcepc = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }
    if (!str_cmp(arg, "mset_player"))
    {
        sysdata.level_mset_player = level;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    send_to_char("Invalid argument.\n\r", ch);
}

void get_reboot_string(void)
{
    sprintf(reboot_time, "%s", asctime(new_boot_time));
}


void do_orange( CHAR_DATA *ch, char *argument )
{
    send_to_char( "Function under construction.\n\r", ch );
    return;
}

void do_mrange( CHAR_DATA *ch, char *argument )
{
    send_to_char( "Function under construction.\n\r", ch );
    return;
}

void do_hell( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    sh_int htime;
    bool h_d = FALSE;
    struct tm *tms;

    argument = one_argument(argument, arg);
    if ( !*arg )
    {
        send_to_char( "Hell who, and for how long?\n\r", ch );
        return;
    }
    if ( !(victim = get_char_world(ch, arg)) || IS_NPC(victim) )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
    if ( IS_IMMORTAL(victim) )
    {
        send_to_char( "There is no point in helling an immortal.\n\r", ch );
        return;
    }
    if ( victim->pcdata->release_date != 0 )
    {
        ch_printf(ch, "They are already in hell until %24.24s, by %s.\n\r",
                  ctime(&victim->pcdata->release_date), victim->pcdata->helled_by);
        return;
    }
    argument = one_argument(argument, arg);
    if ( !*arg || !is_number(arg) )
    {
        send_to_char( "Hell them for how long?\n\r", ch );
        return;
    }
    htime = atoi(arg);
    if ( htime <= 0 )
    {
        send_to_char( "You cannot hell for zero or negative time.\n\r", ch );
        return;
    }
    argument = one_argument(argument, arg);
    if ( !*arg || !str_prefix(arg, "hours") )
        h_d = TRUE;
    else if ( str_prefix(arg, "days") )
    {
        send_to_char( "Is that value in hours or days?\n\r", ch );
        return;
    }
    else if ( htime > 30 )
    {
        send_to_char( "You may not hell a person for more than 30 days at a time.\n\r", ch );
        return;
    }
    tms = localtime(&current_time);
    if ( h_d )
        tms->tm_hour += htime;
    else
        tms->tm_mday += htime;
    victim->pcdata->release_date = mktime(tms);
    victim->pcdata->helled_by = STRALLOC(ch->name);
    ch_printf(ch, "%s will be released from hell at %24.24s.\n\r", victim->name,
              ctime(&victim->pcdata->release_date));
    act(AT_MAGIC, "$n disappears in a cloud of hellish light.", victim, NULL, ch, TO_NOTVICT);
    char_from_room(victim);
    char_to_room(victim, get_room_index(ROOM_VNUM_HELL));
    act(AT_MAGIC, "$n appears in a could of hellish light.", victim, NULL, ch, TO_NOTVICT);
    do_look(victim, "auto");
    ch_printf(victim, "The immortals are not pleased with your actions.\n\r"
              "You shall remain in hell for %d %s%s.\n\r", htime,
              (h_d ? "hour" : "day"), (htime == 1 ? "" : "s"));
    save_char_obj(victim);	/* used to save ch, fixed by Thoric 09/17/96 */
    return;
}

void do_unhell( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;

    argument = one_argument(argument, arg);
    if ( !*arg )
    {
        send_to_char( "Unhell whom..?\n\r", ch );
        return;
    }
    location = ch->in_room;
    ch->in_room = get_room_index(ROOM_VNUM_HELL);
    victim = get_char_room(ch, arg);
    ch->in_room = location;            /* The case of unhell self, etc. */
    if ( !victim || IS_NPC(victim) || victim->in_room->vnum != ROOM_VNUM_HELL )
    {
        send_to_char( "No one like that is in hell.\n\r", ch );
        return;
    }
    if ( victim->pcdata->clan )
        location = get_room_index(victim->pcdata->clan->recall);
    if ( !location && !IS_NPC(victim) )
        location = get_room_index(victim->pcdata->home);
    if ( !location )
    {
        if (IsBadSide(ch))
            location = get_room_index(ROOM_START_EVIL);
        else
            location = get_room_index(ROOM_START_GOOD);
    }
    if ( !location )
        location = get_room_index(ROOM_VNUM_TEMPLE);
    if ( !location )
        location = ch->in_room;
    MOBtrigger = FALSE;
    act( AT_MAGIC, "$n disappears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT );
    char_from_room(victim);
    char_to_room(victim, location);
    send_to_char( "The gods have smiled on you and released you from hell early!\n\r", victim );
    do_look(victim, "auto");
    send_to_char( "They have been released.\n\r", ch );

    if ( victim->pcdata->helled_by )
    {
        if( str_cmp(ch->name, victim->pcdata->helled_by) )
            ch_printf(ch, "(You should probably write a note to %s, explaining the early release.)\n\r",
                      victim->pcdata->helled_by);
        STRFREE(victim->pcdata->helled_by);
        victim->pcdata->helled_by = NULL;
    }

    MOBtrigger = FALSE;
    act( AT_MAGIC, "$n appears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT );
    victim->pcdata->release_date = 0;
    save_char_obj(victim);
    return;
}

/* Vnum search command by Swordbearer */
void do_vsearch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    bool found = FALSE;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    int obj_counter = 1;
    int argi;

    one_argument( argument, arg );

    if( arg[0] == '\0' )
    {
        send_to_char( "Syntax:  vsearch <vnum>.\n\r", ch );
        return;
    }

    set_pager_color( AT_PLAIN, ch );
    argi=atoi(arg);
    if (argi<0 || argi>top_obj_vnum)
    {
        send_to_char( "Vnum out of range.\n\r", ch);
        return;
    }
    for ( obj = first_object; obj != NULL; obj = obj->next )
    {
        if ( !can_see_obj( ch, obj ) || !( argi == obj->vnum ))
            continue;

        found = TRUE;
        for ( in_obj = obj; in_obj->in_obj != NULL;
              in_obj = in_obj->in_obj );

        if ( in_obj->carried_by != NULL )
            pager_printf( ch, "[%2d] %s carried by %s.\n\r",
                          obj_counter,
                          obj_short(obj),
                          PERS( in_obj->carried_by, ch ) );
        else
            pager_printf( ch, "[%2d] [%-5d] %s in %s.\n\r", obj_counter,
                          ( ( in_obj->in_room ) ? in_obj->in_room->vnum : 0 ),
                          obj_short(obj), ( in_obj->in_room == NULL ) ?
                          "somewhere" : in_obj->in_room->name );

        obj_counter++;
    }

    if ( !found )
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r" , ch );

    return;
}

/*
 * Simple function to let any imm make any player instantly sober.
 * Saw no need for level restrictions on this.
 * Written by Narn, Apr/96
 */
void do_sober( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg1 [MAX_INPUT_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on mobs.\n\r", ch );
        return;
    }

    if ( victim->pcdata )
        victim->pcdata->condition[COND_DRUNK] = 0;
    send_to_char( "Ok.\n\r", ch );
    send_to_char( "You feel sober again.\n\r", victim );
    return;
}


/*
 * Free a social structure					-Thoric
 */
void free_social( SOCIALTYPE *social )
{
    if ( social->name )
        DISPOSE( social->name );
    if ( social->mob_response )
        DISPOSE( social->mob_response );
    if ( social->char_no_arg )
        DISPOSE( social->char_no_arg );
    if ( social->others_no_arg )
        DISPOSE( social->others_no_arg );
    if ( social->char_found )
        DISPOSE( social->char_found );
    if ( social->others_found )
        DISPOSE( social->others_found );
    if ( social->vict_found )
        DISPOSE( social->vict_found );
    if ( social->char_auto )
        DISPOSE( social->char_auto );
    if ( social->others_auto )
        DISPOSE( social->others_auto );
    if ( social->not_found )
        DISPOSE( social->not_found );
    DISPOSE( social );
}

/*
 * Remove a social from it's hash index				-Thoric
 */
void unlink_social( SOCIALTYPE *social )
{
    SOCIALTYPE *tmp, *tmp_next;
    int hash;

    if ( !social )
    {
        bug( "Unlink_social: NULL social" );
        return;
    }

    if ( social->name[0] < 'a' || social->name[0] > 'z' )
        hash = 0;
    else
        hash = (social->name[0] - 'a') + 1;

    if ( social == (tmp=social_index[hash]) )
    {
        social_index[hash] = tmp->next;
        return;
    }
    for ( ; tmp; tmp = tmp_next )
    {
        tmp_next = tmp->next;
        if ( social == tmp_next )
        {
            tmp->next = tmp_next->next;
            return;
        }
    }
}

/*
 * Add a social to the social index table			-Thoric
 * Hashed and insert sorted
 */
void add_social( SOCIALTYPE *social )
{
    int hash, x;
    SOCIALTYPE *tmp, *prev;

    if ( !social )
    {
        bug( "Add_social: NULL social" );
        return;
    }

    if ( !social->name )
    {
        bug( "Add_social: NULL social->name" );
        return;
    }

    if ( !social->char_no_arg )
    {
        bug( "Add_social: NULL social->char_no_arg" );
        return;
    }

    /* make sure the name is all lowercase */
    for ( x = 0; social->name[x] != '\0'; x++ )
        social->name[x] = LOWER(social->name[x]);

    if ( social->name[0] < 'a' || social->name[0] > 'z' )
        hash = 0;
    else
        hash = (social->name[0] - 'a') + 1;

    if ( (prev = tmp = social_index[hash]) == NULL )
    {
        social->next = social_index[hash];
        social_index[hash] = social;
        return;
    }

    for ( ; tmp; tmp = tmp->next )
    {
        if ( (x=strcmp(social->name, tmp->name)) == 0 )
        {
            bug( "Add_social: trying to add duplicate name to bucket %d", hash );
            free_social( social );
            return;
        }
        else
            if ( x < 0 )
            {
                if ( tmp == social_index[hash] )
                {
                    social->next = social_index[hash];
                    social_index[hash] = social;
                    return;
                }
                prev->next = social;
                social->next = tmp;
                return;
            }
        prev = tmp;
    }

    /* add to end */
    prev->next = social;
    social->next = NULL;
    return;
}

/*
 * Social editor/displayer/save/delete				-Thoric
 */
void do_sedit( CHAR_DATA *ch, char *argument )
{
    SOCIALTYPE *social;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    sh_int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    set_char_color( AT_SOCIAL, ch );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: sedit <social> [field]\n\r", ch );
        send_to_char( "Syntax: sedit <social> create\n\r", ch );
        if ( get_trust(ch) > LEVEL_GOD )
            send_to_char( "Syntax: sedit <social> delete\n\r", ch );
        if ( get_trust(ch) > LEVEL_LESSER )
            send_to_char( "Syntax: sedit <save>\n\r", ch );
        send_to_char( "\n\rField being one of:\n\r", ch );
        send_to_char( "  cnoarg onoarg cfound ofound vfound cauto oauto nfound pos mresponse\n\r", ch );
        return;
    }

    if ( get_trust(ch) > LEVEL_LESSER && !str_cmp( arg1, "save" ) )
    {
        save_socials();
        send_to_char( "Saved.\n\r", ch );
        return;
    }

    social = find_social( arg1 );

    if ( !str_cmp( arg2, "create" ) )
    {
        if ( social )
        {
            send_to_char( "That social already exists!\n\r", ch );
            return;
        }
        CREATE( social, SOCIALTYPE, 1 );
        social->name = str_dup( arg1 );
        sprintf( arg2, "You %s.", arg1 );
        social->char_no_arg = str_dup( arg2 );
        add_social( social );
        send_to_char( "Social added.\n\r", ch );
        return;
    }

    if ( !social )
    {
        send_to_char( "Social not found.\n\r", ch );
        return;
    }

    if ( arg2[0] == '\0' || !str_cmp( arg2, "show" ) )
    {
        ch_printf( ch, "Social: %s\n\r\n\rPosition: %s\n\rMob Response: %s\n\rCNoArg: %s\n\r",
                   social->name,
                   position_types[social->position],
                   social->mob_response ? social->mob_response  : "(not set)",
                   social->char_no_arg  ? social->char_no_arg   : "(not set)" );
        ch_printf( ch, "ONoArg: %s\n\rCFound: %s\n\rOFound: %s\n\r",
                   social->others_no_arg? social->others_no_arg	: "(not set)",
                   social->char_found	? social->char_found	: "(not set)",
                   social->others_found	? social->others_found	: "(not set)" );
        ch_printf( ch, "VFound: %s\n\rCAuto : %s\n\rOAuto : %s\n\r",
                   social->vict_found	? social->vict_found	: "(not set)",
                   social->char_auto	? social->char_auto	: "(not set)",
                   social->others_auto	? social->others_auto	: "(not set)" );
        ch_printf( ch, "NotFound: %s\n\r",
                   social->not_found	? social->not_found	: "(not set)" );
        return;
    }

    if ( get_trust(ch) > LEVEL_GOD && !str_cmp( arg2, "delete" ) )
    {
        unlink_social( social );
        free_social( social );
        send_to_char( "Deleted.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "pos" ) )
    {
        value = get_postype( argument );
        if ( value < 0 || value >= MAX_POSITION )
        {
            ch_printf( ch, "Invalid position: %s\n\r", argument );
            return;
        }
        social->position = value;
        return;
    }

    if ( !str_cmp( arg2, "mresponse" ) )
    {
	if ( get_trust(ch) < LEVEL_IMPLEMENTOR )
	{
	    send_to_char("Only implementors can do this.\n\r", ch);
	    return;
	}

        if ( social->mob_response )
            DISPOSE( social->mob_response );
        if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
            social->mob_response = str_dup( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "cnoarg" ) )
    {
        if ( argument[0] == '\0' || !str_cmp( argument, "clear" ) )
        {
            send_to_char( "You cannot clear this field.  It must have a message.\n\r", ch );
            return;
        }
        if ( social->char_no_arg )
            DISPOSE( social->char_no_arg );
        social->char_no_arg = str_dup( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "onoarg" ) )
    {
        if ( social->others_no_arg )
            DISPOSE( social->others_no_arg );
        if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
            social->others_no_arg = str_dup( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "cfound" ) )
    {
        if ( social->char_found )
            DISPOSE( social->char_found );
        if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
            social->char_found = str_dup( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "ofound" ) )
    {
        if ( social->others_found )
            DISPOSE( social->others_found );
        if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
            social->others_found = str_dup( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "vfound" ) )
    {
        if ( social->vict_found )
            DISPOSE( social->vict_found );
        if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
            social->vict_found = str_dup( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "cauto" ) )
    {
        if ( social->char_auto )
            DISPOSE( social->char_auto );
        if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
            social->char_auto = str_dup( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "oauto" ) )
    {
        if ( social->others_auto )
            DISPOSE( social->others_auto );
        if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
            social->others_auto = str_dup( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "nfound" ) )
    {
        if ( social->not_found )
            DISPOSE( social->not_found );
        if ( argument[0] != '\0' && str_cmp( argument, "clear" ) )
            social->not_found = str_dup( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( get_trust(ch) > LEVEL_GREATER && !str_cmp( arg2, "name" ) )
    {
        bool relocate;

        one_argument( argument, arg1 );
        if ( arg1[0] == '\0' )
        {
            send_to_char( "Cannot clear name field!\n\r", ch );
            return;
        }
        if ( arg1[0] != social->name[0] )
        {
            unlink_social( social );
            relocate = TRUE;
        }
        else
            relocate = FALSE;
        if ( social->name )
            DISPOSE( social->name );
        social->name = str_dup( arg1 );
        if ( relocate )
            add_social( social );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    /* display usage message */
    do_sedit( ch, "" );
}

/*
 * Free a command structure					-Thoric
 */
void free_command( CMDTYPE *command )
{
    if ( command->name )
        DISPOSE( command->name );
    DISPOSE( command );
}

/*
 * Remove a command from it's hash index			-Thoric
 */
void unlink_command( CMDTYPE *command )
{
    CMDTYPE *tmp, *tmp_next;
    int hash;

    if ( !command )
    {
        bug( "Unlink_command NULL command" );
        return;
    }

    hash = command->name[0]%126;

    if ( command == (tmp=command_hash[hash]) )
    {
        command_hash[hash] = tmp->next;
        return;
    }
    for ( ; tmp; tmp = tmp_next )
    {
        tmp_next = tmp->next;
        if ( command == tmp_next )
        {
            tmp->next = tmp_next->next;
            return;
        }
    }
}

/*
 * Add a command to the command hash table			-Thoric
 */
void add_command( CMDTYPE *command )
{
    int hash, x;
    CMDTYPE *tmp, *prev;

    if ( !command )
    {
        bug( "Add_command: NULL command" );
        return;
    }

    if ( !command->name )
    {
        bug( "Add_command: NULL command->name" );
        return;
    }

    if ( !command->do_fun )
    {
        bug( "Add_command: NULL command->do_fun" );
        return;
    }

    /* make sure the name is all lowercase */
    for ( x = 0; command->name[x] != '\0'; x++ )
        command->name[x] = LOWER(command->name[x]);

    hash = command->name[0] % 126;

    if ( (prev = tmp = command_hash[hash]) == NULL )
    {
        command->next = command_hash[hash];
        command_hash[hash] = command;
        return;
    }

    /* add to the END of the list */
    for ( ; tmp; tmp = tmp->next )
        if ( !tmp->next )
        {
            tmp->next = command;
            command->next = NULL;
        }
    return;
}


char *	const	command_flags	[] =
{
    "normal", "experimental", "building",
    "informational", "imc", "utility", "maintenance",
    "creation", "deletion", "modification", "communication",
    "system", "r13", "r14", "r15",
    "r16", "r17", "r18", "r19", "r20", "r21",
    "r22", "r23", "r24", "r25", "r26", "r27",
    "r28", "r29", "r30", "r31", "r32"
};

int get_command_flag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_prefix( flag, command_flags[x] ) )
            return x;
    return -1;
}


/*
 * Command editor/displayer/save/delete				-Thoric
 */
void do_cedit( CHAR_DATA *ch, char *argument )
{
    CMDTYPE *command;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    set_char_color( AT_IMMORT, ch );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: cedit save\n\r", ch );
        if ( get_trust(ch) > LEVEL_SUB_IMPLEM )
        {
            send_to_char( "Syntax: cedit <command> create [code]\n\r", ch );
            send_to_char( "Syntax: cedit <command> delete\n\r", ch );
            send_to_char( "Syntax: cedit <command> show\n\r", ch );
            send_to_char( "Syntax: cedit <command> [field]\n\r", ch );
            send_to_char( "\n\rField being one of:\n\r", ch );
            send_to_char( "  level position log code flags\n\r", ch );
        }
        return;
    }

    if ( get_trust(ch) > LEVEL_GREATER && !str_cmp( arg1, "save" ) && !*argument)
    {
        save_commands();
        send_to_char( "Saved.\n\r", ch );
        return;
    }

    command = find_command_exact( arg1 );

    if ( get_trust(ch) > LEVEL_SUB_IMPLEM && !str_cmp( arg2, "create" ) )
    {
        if ( command )
        {
            send_to_char( "That command already exists!\n\r", ch );
            return;
        }
        CREATE( command, CMDTYPE, 1 );
        command->name = str_dup( arg1 );
        command->level = get_trust(ch);
        if ( *argument )
            one_argument(argument, arg2);
        else
            sprintf( arg2, "do_%s", arg1 );
        command->do_fun = skill_function( arg2 );
        command->flags = CMD_EXPERIMENTAL;
        add_command( command );
        send_to_char( "Command added.\n\r", ch );
        if ( command->do_fun == skill_notfound )
            ch_printf( ch, "Code %s not found.  Set to no code.\n\r", arg2 );
        return;
    }

    if ( !command )
        command = find_command( arg1 );

    if ( !command )
    {
        send_to_char( "Command not found.\n\r", ch );
        return;
    }

    if ( command->level > get_trust(ch) )
    {
        send_to_char( "You cannot touch this command.\n\r", ch );
        return;
    }

    if ( arg2[0] == '\0' || !str_cmp( arg2, "show" ) )
    {
        ch_printf( ch, "Command:  %s\n\rLevel:    %d\n\rPosition: %s\n\rLog:      %d\n\rCode:     %s\n\rFlags:    %s\n\r",
                   command->name, command->level, position_types[command->position], command->log,
                   skill_name(command->do_fun),
                   flag_string(command->flags, command_flags));
        if ( command->userec.num_uses )
            send_timer(&command->userec, ch);
        return;
    }

    if ( get_trust(ch) <= LEVEL_SUB_IMPLEM )
    {
        do_cedit( ch, "" );
        return;
    }

    if ( !str_cmp( arg2, "delete" ) )
    {
        unlink_command( command );
        free_command( command );
        send_to_char( "Deleted.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "code" ) )
    {
        DO_FUN *fun = skill_function( argument );

        if ( fun == skill_notfound )
        {
            send_to_char( "Code not found.\n\r", ch );
            return;
        }
        command->do_fun = fun;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "level" ) )
    {
        int level = atoi( argument );

        if ( level < 0 || level > get_trust(ch) )
        {
            send_to_char( "Level out of range.\n\r", ch );
            return;
        }
        command->level = level;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "log" ) )
    {
        int log = atoi( argument );

        if ( log < 0 || log > LOG_COMM )
        {
            send_to_char( "Log out of range.\n\r", ch );
            return;
        }
        command->log = log;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "position" ) )
    {
        int position = get_postype( argument );

        if ( position < 0 || position >= MAX_POSITION )
        {
            send_to_char( "Position out of range.\n\r", ch );
            return;
        }
        command->position = position;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
        bool relocate;

        one_argument( argument, arg1 );
        if ( arg1[0] == '\0' )
        {
            send_to_char( "Cannot clear name field!\n\r", ch );
            return;
        }
        if ( arg1[0] != command->name[0] )
        {
            unlink_command( command );
            relocate = TRUE;
        }
        else
            relocate = FALSE;
        if ( command->name )
            DISPOSE( command->name );
        command->name = str_dup( arg1 );
        if ( relocate )
            add_command( command );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "flags" ) )
    {
        int value;

        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: cedit <command> flag <flag> [flag]...\n\r", ch );
            for ( value = 0; value < 32; value++ )
                ch_printf(ch, "%s ", command_flags[value]);
            send_to_char("\n\r", ch);
            return;
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg1 );
            value = get_command_flag( arg1 );
            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg1 );
            else
                TOGGLE_BIT(command->flags, 1 << value);
        }
        return;
    }

    /* display usage message */
    do_cedit( ch, "" );
}

/*
 * Display class information					-Thoric
 */
void do_showch_class( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    struct class_type *ch_class;
    int cl=0, low, hi;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: showclass <class> [level range]\n\r", ch );
        return;
    }
    if ( is_number(arg1) && (cl=atoi(arg1)) >= 0 && cl < MAX_CLASS )
        ch_class = class_table[cl];
    else
    {
        ch_class = NULL;
        for ( cl = 0; cl < MAX_CLASS && class_table[cl]; cl++ )
            if ( !str_cmp(class_table[cl]->who_name, arg1) )
            {
                ch_class = class_table[cl];
                break;
            }
    }
    if ( !ch_class )
    {
        send_to_char( "No such class.\n\r", ch );
        return;
    }
    pager_printf( ch, "Class: %s\n\rPrime Attribute: %-14s Weapon: %-5d  Guild:   %d\n\r"
		 "Attribute string for chargen: %s\n\r"
		 "Max Skill Adept: %-3d     Thac0:  %-5d  Thac32:  %d\n\r"
		 "Hp Min / Hp Max: %d/%-2d    Mana:   %s    Exp Formula: %ld*x^^%ld\n\r"
		 "Hp Lev / Hp Add: %-2d/%d\n\r",
		 ch_class->who_name, affect_loc_name(ch_class->attr_prime), ch_class->weapon, ch_class->guild,
                 ch_class->attr_string?ch_class->attr_string:"(none)",
		 ch_class->skill_adept, ch_class->thac0_00, ch_class->thac0_32,
		 ch_class->hp_min, ch_class->hp_max, ch_class->fMana ? "yes" : "no ",
		 ch_class->exp_base, ch_class->exp_power,
		 ch_class->hp_const_lev, ch_class->hp_const_add );
    if ( arg2[0] != '\0' )
    {
        int x, y, cnt;

        low = UMIN( 0, atoi(arg2) );
        hi  = URANGE( low, atoi(argument), MAX_LEVEL );
        for ( x = low; x <= hi; x++ )
        {
            set_pager_color( AT_LBLUE, ch );
            pager_printf( ch, "Male: %-30s Female: %s\n\r",
                          title_table[cl][x][0], title_table[cl][x][1] );
            cnt = 0;
            set_pager_color( AT_BLUE, ch );
            for ( y = gsn_first_spell; y < gsn_top_sn; y++ )
                if ( skill_table[y]->skill_level[cl] == x )
                {
                    pager_printf( ch, "  %-7s %-19s%3d     ",
                                  skill_tname[skill_table[y]->type],
                                  skill_table[y]->name, skill_table[y]->skill_adept[cl] );
                    if ( ++cnt % 2 == 0 )
                        send_to_pager( "\n\r", ch );
                }
            if ( cnt % 2 != 0 )
                send_to_pager( "\n\r", ch );
            send_to_pager( "\n\r", ch );
        }
    }
}

/*
 * Edit class information					-Thoric
 */
void do_setch_class( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    struct class_type *ch_class;
    int cl=0;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    set_char_color( AT_IMMORT, ch );
    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: setclass <class> <field> <value>\n\r",	ch );
        send_to_char( "\n\rField being one of:\n\r",			ch );
        send_to_char( "  name prime weapon guild thac0 thac32\n\r",	ch );
        send_to_char( "  hpmin hpmax mana expbase mtitle ftitle\n\r",	ch );
        send_to_char( "  hplev hpadd exppower attrstring save\n\r",	ch );
        return;
    }
    if ( is_number(arg1) && (cl=atoi(arg1)) >= 0 && cl < MAX_CLASS )
        ch_class = class_table[cl];
    else
    {
        ch_class = NULL;
        for ( cl = 0; cl < MAX_CLASS && class_table[cl]; cl++ )
            if ( !str_cmp(class_table[cl]->who_name, arg1) )
            {
                ch_class = class_table[cl];
                break;
            }
    }
    if ( !ch_class )
    {
        send_to_char( "No such class.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "save" ) )
    {
        write_class_file( cl );
        send_to_char( "Saved.\n\r", ch );
        return;
    }
    if ( !str_cmp( arg2, "name" ) )
    {
        STRFREE(ch_class->who_name );
        ch_class->who_name = STRALLOC( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }
    if ( !str_cmp( arg2, "prime" ) )
    {
        int x = get_atype( argument );

        if ( x < APPLY_STR || (x > APPLY_CON && x != APPLY_LCK) )
            send_to_char( "Invalid prime attribute!\n\r", ch );
        else
        {
            ch_class->attr_prime = x;
            send_to_char( "Done.\n\r", ch );
        }
        return;
    }
    if ( !str_cmp( arg2, "weapon" ) )
    {
        ch_class->weapon = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }
    if ( !str_cmp( arg2, "guild" ) )
    {
        ch_class->guild = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }
    if ( !str_cmp( arg2, "thac0" ) )
    {
        ch_class->thac0_00 = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }
    if ( !str_cmp( arg2, "thac32" ) )
    {
        ch_class->thac0_32 = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }
    if ( !str_cmp( arg2, "hpmin" ) )
    {
        ch_class->hp_min = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }
    if ( !str_cmp( arg2, "hpmax" ) )
    {
        ch_class->hp_max = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }
    if ( !str_cmp( arg2, "hplev" ) )
    {
        ch_class->hp_const_lev = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }
    if ( !str_cmp( arg2, "hpadd" ) )
    {
        ch_class->hp_const_add = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }
    if ( !str_cmp( arg2, "mana" ) )
    {
        if ( UPPER(argument[0]) == 'Y' )
            ch_class->fMana = TRUE;
        else
            ch_class->fMana = FALSE;
        send_to_char( "Done.\n\r", ch );
        return;
    }
    if ( !str_cmp( arg2, "expbase" ) )
    {
        ch_class->exp_base = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }
    if ( !str_cmp( arg2, "exppower" ) )
    {
        ch_class->exp_power = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }
    if ( !str_cmp( arg2, "mtitle" ) )
    {
        char arg3[MAX_INPUT_LENGTH];
        int x;

        argument = one_argument( argument, arg3 );
        if ( arg3[0] == '\0' || argument[0] == '\0' )
        {
            send_to_char( "Syntax: setclass <class> mtitle <level> <title>\n\r", ch );
            return;
        }
        if ( (x=atoi(arg3)) < 0 || x > MAX_LEVEL )
        {
            send_to_char( "Invalid level.\n\r", ch );
            return;
        }
        DISPOSE( title_table[cl][x-1][0] );
        title_table[cl][x-1][0] = str_dup( argument );
        send_to_char("Done.\n\r", ch);
        return;
    }
    if ( !str_cmp( arg2, "ftitle" ) )
    {
        char arg3[MAX_INPUT_LENGTH];
        int x;

        argument = one_argument( argument, arg3 );
        if ( arg3[0] == '\0' || argument[0] == '\0' )
        {
            send_to_char( "Syntax: setclass <class> ftitle <level> <title>\n\r", ch );
            return;
        }
        if ( (x=atoi(arg3)) < 0 || x > MAX_LEVEL )
        {
            send_to_char( "Invalid level.\n\r", ch );
            return;
        }
        DISPOSE( title_table[cl][x-1][1] );
        title_table[cl][x-1][1] = str_dup( argument );
        send_to_char("Done.\n\r", ch);
        return;
    }
    if ( !str_cmp( arg2, "attrstring" ) )
    {
        STRFREE(ch_class->attr_string );
        ch_class->attr_string = STRALLOC( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }
    do_setch_class( ch, "" );
}

/*
 * quest point set - TRI
 * syntax is: qpset char give/take amount
 */

void do_qpset( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int amount;
    bool give = TRUE;

    if ( IS_NPC(ch) )
    {
        send_to_char( "Cannot qpset as an NPC.\n\r", ch );
        return;
    }

    if ( get_trust( ch ) < LEVEL_IMMORTAL )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    else if ( str_cmp( ch->pcdata->council_name, "Quest Council" )
              && ( get_trust( ch ) < LEVEL_DEMI ) )
    {
        send_to_char( "You must be a member of Quest Council to give or remove qp from players.\n\r", ch );
        return;
    }

    set_char_color( AT_LOG, ch );
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    amount = atoi( arg3 );

    if ( arg[0] == '\0' || arg2[0] == '\0' || amount <= 0 )
    {
        send_to_char( "Syntax: qpset <character> <give/take> <amount>\n\r", ch );
        send_to_char( "Amount must be a positive number greater than 0.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "There is no such player currently playing.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Glory cannot be given to or taken from a "
                      "mob.\n\r", ch );
        return;
    }

    if ( nifty_is_name_prefix( arg2, "give" ) )
        give = TRUE;
    else if ( nifty_is_name_prefix( arg2, "take" ) )
        give = FALSE;
    else
    {
        do_qpset( ch, "" );
        return;
    }

    if ( give )
    {
        victim->pcdata->quest_curr += amount;
        victim->pcdata->quest_accum += amount;
        ch_printf( victim, "Your glory has been increased by %d.\n\r", amount );
    }
    else
    {
        victim->pcdata->quest_curr -= amount;
        ch_printf( victim, "Your glory has been decreased by %d.\n\r", amount );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_resolve( CHAR_DATA *ch, char *argument )
{
    struct hostent *host;
    struct sockaddr_in sock;

    if (!inet_aton(argument, &sock.sin_addr))
        send_to_char("Unable to convert.", ch);
    else if (( host = gethostbyaddr((char *) &sock.sin_addr, sizeof(sock.sin_addr), AF_INET) ))
        send_to_char(host->h_name, ch);
    else
        send_to_char("Unable to resolve.", ch);
    send_to_char("\n\r", ch);
}

/*
 * Command to display the weather status of all the areas
 * Last Modified: July 21, 1997
 * Fireblade
 */
void do_showweather(CHAR_DATA *ch, char *argument)
{
    AREA_DATA *pArea;
    char arg[MAX_INPUT_LENGTH];
    char s1[16], s2[16], s3[16];

    if(!ch)
    {
        bug("do_showweather: NULL char data");
        return;
    }

    sprintf(s1,"%s",color_str(AT_BLUE,ch));
    sprintf(s2,"%s",color_str(AT_LBLUE,ch));
    sprintf(s3,"%s",color_str(AT_WHITE,ch));

    argument = one_argument(argument, arg);

    pager_printf(ch, "%-40s%-8s %-8s %-8s\n\r",
                 "Area Name:", "Temp:", "Precip:", "Wind:");

    for(pArea = first_area; pArea; pArea = pArea->next)
    {
        if(arg[0] == '\0' ||
           nifty_is_name_prefix(arg, pArea->name))
        {
            pager_printf(ch, "%s%-40s%s%3d%s(%s%3d%s) %s%3d%s(%s%3d%s) %s%3d%s(%s%3d%s)\n\r",
                         s1, pArea->name,
                         s3, pArea->weather->temp,
                         s1, s2, pArea->weather->temp_vector,
                         s1, s3, pArea->weather->precip,
                         s1, s2, pArea->weather->precip_vector,
                         s1, s3, pArea->weather->wind,
                         s1, s2, pArea->weather->wind_vector,
                         s1);
        }
    }

    return;
}

/*
 * Command to control global weather variables and to reset weather
 * Last Modified: July 23, 1997
 * Fireblade
 */
void do_setweather(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];

    set_char_color(AT_PLAIN, ch);

    argument = one_argument(argument, arg);

    if(arg[0] == '\0')
    {
        ch_printf(ch, "%-15s%-6s\n\r",
                  "Parameters:", "Value:");
        ch_printf(ch, "%-15s%-6d\n\r",
                  "random", rand_factor);
        ch_printf(ch, "%-15s%-6d\n\r",
                  "climate", climate_factor);
        ch_printf(ch, "%-15s%-6d\n\r",
                  "neighbor", neigh_factor);
        ch_printf(ch, "%-15s%-6d\n\r",
                  "unit", weath_unit);
        ch_printf(ch, "%-15s%-6d\n\r",
                  "maxvector", max_vector);

        ch_printf(ch, "\n\rResulting values:\n\r");
        ch_printf(ch, "Weather variables range from "
                  "%d to %d.\n\r", -3*weath_unit,
                  3*weath_unit);
        ch_printf(ch, "Weather vectors range from "
                  "%d to %d.\n\r", -1*max_vector,
                  max_vector);
        ch_printf(ch, "The maximum a vector can "
                  "change in one update is %d.\n\r",
                  rand_factor + 2*climate_factor +
                  (6*weath_unit/neigh_factor));

    }
    else if(!str_cmp(arg, "random"))
    {
        if(!is_number(argument))
        {
            ch_printf(ch, "Set maximum random "
                      "change in vectors to what?\n\r");
        }
        else
        {
            rand_factor = atoi(argument);
            ch_printf(ch, "Maximum random "
                      "change in vectors now "
                      "equals %d.\n\r", rand_factor);
            save_weatherdata();
        }
    }
    else if(!str_cmp(arg, "climate"))
    {
        if(!is_number(argument))
        {
            ch_printf(ch, "Set climate effect "
                      "coefficient to what?\n\r");
        }
        else
        {
            climate_factor = atoi(argument);
            ch_printf(ch, "Climate effect "
                      "coefficient now equals "
                      "%d.\n\r", climate_factor);
            save_weatherdata();
        }
    }
    else if(!str_cmp(arg, "neighbor"))
    {
        if(!is_number(argument))
        {
            ch_printf(ch, "Set neighbor effect "
                      "divisor to what?\n\r");
        }
        else
        {
            neigh_factor = atoi(argument);

            if(neigh_factor <= 0)
                neigh_factor = 1;

            ch_printf(ch, "Neighbor effect "
                      "coefficient now equals "
                      "1/%d.\n\r", neigh_factor);
            save_weatherdata();
        }
    }
    else if(!str_cmp(arg, "unit"))
    {
        if(!is_number(argument))
        {
            ch_printf(ch, "Set weather unit "
                      "size to what?\n\r");
        }
        else
        {
            weath_unit = atoi(argument);
            ch_printf(ch, "Weather unit size "
                      "now equals %d.\n\r",
                      weath_unit);
            save_weatherdata();
        }
    }
    else if(!str_cmp(arg, "maxvector"))
    {
        if(!is_number(argument))
        {
            ch_printf(ch, "Set maximum vector "
                      "size to what?\n\r");
        }
        else
        {
            max_vector = atoi(argument);
            ch_printf(ch, "Maximum vector size "
                      "now equals %d.\n\r",
                      max_vector);
            save_weatherdata();
        }
    }
    else if(!str_cmp(arg, "reset"))
    {
        init_area_weather();
        ch_printf(ch, "Weather system reinitialized.\n\r");
    }
    else if(!str_cmp(arg, "update"))
    {
        int i, number;

        number = atoi(argument);

        if(number < 1)
            number = 1;

        for(i = 0; i < number; i++)
            weather_update();

        ch_printf(ch, "Weather system updated.\n\r");
    }
    else
    {
        ch_printf(ch, "You may only use one of the "
                  "following fields:\n\r");
        ch_printf(ch, "\trandom\n\r\tclimate\n\r"
                  "\tneighbor\n\r\tunit\n\r\tmaxvector\n\r");
        ch_printf(ch, "You may also reset or update "
                  "the system using the fields 'reset' "
                  "and 'update' respectively.\n\r");
    }

    return;
}

/*
 * Command to check for multiple ip addresses in the mud.
 * --Shaddai
 */

/*
 * Added this new struct to do matching
 * If ya think of a better way do it, easiest way I could think of at
 * 2 in the morning :) --Shaddai
 */

typedef struct ipcompare_data IPCOMPARE_DATA;
struct ipcompare_data {
    struct ipcompare_data *prev;
    struct ipcompare_data *next;
    char   *host;
    char   *name;
    char   *user;
    int    connected;
    int    count;
#ifndef MUD_LISTENER
    int    descriptor;
#else
    unsigned int    uid;
#endif
    int    idle;
    int    port;
    bool   printed;
};

void do_ipcompare ( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    char arg[MAX_INPUT_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char *addie = NULL;
    bool prefix = FALSE, suffix = FALSE, inarea = FALSE, inroom = FALSE, inworld =
    FALSE;
    int count = 0, times = -1;
    bool fMatch;
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg1);
    argument = one_argument( argument, arg2);

    set_pager_color (AT_PLAIN, ch);

    if ( IS_NPC(ch) )
    {
        send_to_char("Huh?\n\r", ch );
        return;
    }

    if ( arg[0] == '\0' )
    {
        send_to_char( "ipcompare pkill\n\r", ch );
        send_to_char( "ipcompare total\n\r", ch );
        send_to_char( "ipcompare <person> [room|area|world] [#]\n\r", ch );
        send_to_char( "ipcompare <site>   [room|area|world] [#]\n\r", ch );
        return;
    }
    if ( !str_cmp ( arg, "total" ) )
    {
        IPCOMPARE_DATA *first_ip=NULL,*last_ip=NULL,*hmm,*hmm_next;
        for ( d = first_descriptor; d; d = d->next)
        {
            fMatch = FALSE;
            for ( hmm = first_ip; hmm; hmm = hmm->next )
                if ( !str_cmp( hmm->host, d->host ) )
                    fMatch = TRUE;
            if ( !fMatch )
            {
                IPCOMPARE_DATA *temp;
                CREATE( temp, IPCOMPARE_DATA, 1);
                temp->host = str_dup ( d->host );
                LINK( temp, first_ip, last_ip, next, prev );
                count++;
            }
        }
        for ( hmm = first_ip; hmm; hmm = hmm_next )
        {
            hmm_next = hmm->next;
            UNLINK( hmm, first_ip, last_ip, next, prev );
            if ( hmm->host )
                DISPOSE( hmm->host );
            DISPOSE( hmm );
        }
        ch_printf(ch, "There were %d unique ip addresses found.\n\r",
                  count );
        return;
    }
    else if ( !str_cmp ( arg, "pkill" ) )
    {
        IPCOMPARE_DATA *first_ip=NULL,*last_ip=NULL,*hmm, *hmm_next;
#ifndef MUD_LISTENER
        sprintf (buf, "\n\rDesc|Con|Idle| Port | Player      ");
#else
        sprintf (buf, "\n\rUID |Con|Idle| Port | Player      ");
#endif
        if (get_trust (ch) >= LEVEL_SAVIOR )
            strcat (buf, "@HostIP           ");
        if (get_trust (ch) >= LEVEL_GOD)
            strcat (buf, "| Username");
        strcat (buf, "\n\r");
        strcat (buf, "----+---+----+------+-------------");
        if ( get_trust ( ch ) >= LEVEL_SAVIOR )
            strcat (buf, "------------------" );
        if (get_trust (ch) >= LEVEL_GOD)
            strcat (buf, "+---------");
        strcat (buf, "\n\r");
        send_to_pager (buf, ch);

        for ( d = first_descriptor; d; d = d->next)
        {
            IPCOMPARE_DATA *temp;

            if ( (d->connected != CON_PLAYING && d->connected != CON_EDITING)
                 || d->character == NULL || !CAN_PKILL(d->character)
                 || !can_see (ch, d->character) )
                continue;
            CREATE( temp, IPCOMPARE_DATA, 1);
            temp->host = str_dup ( d->host );
#ifndef MUD_LISTENER
            temp->descriptor = d->descriptor;
#else
            temp->uid = d->uid;
#endif
            temp->connected = d->connected;
            temp->idle = d->idle;
            temp->port = d->port;
            temp->name = (d->original ? str_dup( d->original->name ) :
                          d->character? str_dup( d->character->name ) :
                          str_dup ( "(none)" ));
            temp->user = str_dup(d->user);
            temp->count = 0;
            temp->printed = FALSE;
            LINK( temp, first_ip, last_ip, next, prev );
        }

        for ( d = first_descriptor; d; d = d->next)
        {
            fMatch = FALSE;
            if ( (d->connected != CON_PLAYING && d->connected != CON_EDITING)
                 || d->character == NULL || !can_see (ch, d->character))
                continue;
            for ( hmm = first_ip; hmm; hmm = hmm->next )
            {
                if ( !str_cmp( hmm->host, d->host) &&
                     str_cmp(hmm->name, (d->original ? d->original->name :
                                         d->character? d->character->name : "(none)" )))
                {
                    fMatch = TRUE;
                    break;
                }
            }
            if ( fMatch && hmm )
            {
                hmm->count++;
                if ( !hmm->printed && hmm->count > 0)
                {
                    sprintf (buf,
                             " %3d| %2d|%4d|%6d| %-12s",
#ifndef MUD_LISTENER
                             hmm->descriptor,
#else
                             hmm->uid,
#endif
                             hmm->connected,
                             hmm->idle / 4,
                             hmm->port,
                             hmm->name );
                    if (get_trust (ch) >= LEVEL_SAVIOR )
                        sprintf(buf + strlen (buf), "@%-16s ", hmm->host );
                    if (get_trust (ch) >= LEVEL_GOD)
                        sprintf (buf + strlen (buf), "| %s", hmm->user);
                    strcat (buf, "\n\r");
                    send_to_pager (buf, ch);
                    hmm->printed = TRUE;
                }
                sprintf (buf,
                         " %3d| %2d|%4d|%6d| %-12s",
#ifndef MUD_LISTENER
                         d->descriptor,
#else
                         d->uid,
#endif
                         d->connected,
                         d->idle / 4,
                         d->port,
                         d->original ? d->original->name :
                         d->character ? d->character->name : "(none)");
                if (get_trust (ch) >= LEVEL_SAVIOR )
                    sprintf(buf + strlen (buf), "@%-16s ", d->host );
                if (get_trust (ch) >= LEVEL_GOD)
                    sprintf (buf + strlen (buf), "| %s", d->user);
                strcat (buf, "\n\r");
                send_to_pager (buf, ch);
            }
        }
        for ( hmm = first_ip; hmm; hmm = hmm_next )
        {
            hmm_next = hmm->next;
            UNLINK( hmm, first_ip, last_ip, next, prev );
            if ( hmm->name )
                DISPOSE( hmm->name );
            if ( hmm->host )
                DISPOSE( hmm->host );
            if ( hmm->user )
                DISPOSE( hmm->user );
            DISPOSE( hmm );
        }
        return;
    }
    if ( arg1[0] != '\0' )
    {
        if ( is_number( arg1 ) )
            times = atoi( arg1 );
        else
        {
            if ( !str_cmp ( arg1, "room" ) )
                inroom = TRUE;
            else if ( !str_cmp ( arg1, "area" ) )
                inarea = TRUE;
            else
                inworld = TRUE;
        }
        if ( arg2[0] != '\0' )
        {
            if ( is_number( arg2 ) )
                times = atoi( arg2 );
            else
            {
                send_to_char("Please see help ipcompare for more info.\n\r",ch);
                return;
            }
        }
    }
    if ( ( victim = get_char_world( ch, arg ) ) != NULL  && victim->desc)
    {
        if ( IS_NPC(victim) )
        {
            send_to_char ( "Not on NPC's.\n\r", ch );
            return;
        }
        addie = victim->desc->host;
    }
    else
    {
        addie = arg;
        if ( arg[0] == '*' )
        {
            prefix = TRUE;
            addie++;
        }
        if ( addie[strlen(addie) -1] == '*' )
        {
            suffix = TRUE;
            addie[strlen(addie)-1] = '\0';
        }
    }
    sprintf (buf, "\n\rDesc|Con|Idle| Port | Player      ");
    if (get_trust (ch) >= LEVEL_SAVIOR )
        strcat (buf, "@HostIP           ");
    if (get_trust (ch) >= LEVEL_GOD)
        strcat (buf, "| Username");
    strcat (buf, "\n\r");
    strcat (buf, "----+---+----+------+-------------");
    if ( get_trust ( ch ) >= LEVEL_SAVIOR )
        strcat (buf, "------------------" );
    if (get_trust (ch) >= LEVEL_GOD)
        strcat (buf, "+---------");
    strcat (buf, "\n\r");
    send_to_pager (buf, ch);
    for ( d = first_descriptor; d; d = d->next)
    {
        if ( !d->character || (d->connected != CON_PLAYING &&
                               d->connected != CON_EDITING) || !can_see (ch, d->character) )
            continue;
        if ( inroom && ch->in_room != d->character->in_room )
            continue;
        if ( inarea && ch->in_room->area != d->character->in_room->area )
            continue;
        if ( times > 0 && count == (times - 1 ) )
            break;
        if ( prefix && suffix && strstr( addie, d->host ) )
            fMatch = TRUE;
        else if ( prefix && !str_suffix( addie , d->host) )
            fMatch = TRUE;
        else if ( suffix && !str_prefix( addie , d->host) )
            fMatch = TRUE;
        else if ( !str_cmp( d->host, addie ) )
            fMatch = TRUE;
        else
            fMatch = FALSE;
        if ( fMatch )
        {
            count++;
            sprintf (buf,
                     " %3d| %2d|%4d|%6d| %-12s",
#ifndef MUD_LISTENER
                     d->descriptor,
#else
                     d->uid,
#endif
                     d->connected,
                     d->idle / 4,
                     d->port,
                     d->original ? d->original->name :
                     d->character ? d->character->name : "(none)");
            if (get_trust (ch) >= LEVEL_SAVIOR )
                sprintf(buf + strlen (buf), "@%-16s ", d->host );
            if (get_trust (ch) >= LEVEL_GOD)
                sprintf (buf + strlen (buf), "| %s", d->user);
            strcat (buf, "\n\r");
            send_to_pager (buf, ch);
        }
    }
    pager_printf (ch, "%d user%s.\n\r", count, count == 1 ? "" : "s");
    return;
}

void do_delay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    int delay;

    set_char_color( AT_IMMORT, ch );

    argument = one_argument( argument, arg );
    if ( !*arg ) {
        send_to_char( "Syntax:  delay <victim> <# of rounds>\n\r", ch );
        return;
    }
    if ( !( victim = get_char_world( ch, arg ) ) ) {
        send_to_char( "No such character online.\n\r", ch );
        return;
    }
    if ( IS_NPC( victim ) ) {
        send_to_char( "Mobiles are unaffected by lag.\n\r", ch );
        return;
    }
    if ( !IS_NPC(victim) && get_trust( victim ) >= get_trust( ch ) ) {
        send_to_char( "You haven't the power to succeed against them.\n\r", ch );
        return;
    }
    argument = one_argument(argument, arg);
    if ( !*arg ) {
        send_to_char( "For how long do you wish to delay them?\n\r", ch );
        return;
    }
    if ( !str_cmp( arg, "none" ) ) {
        send_to_char( "All character delay removed.\n\r", ch );
        victim->wait = 0;
        return;
    }
    delay = atoi( arg );
    if ( delay < 1 ) {
        send_to_char( "Pointless.  Try a positive number.\n\r", ch );
        return;
    }
    if ( delay > 999 ) {
        send_to_char( "You cruel bastard.  Just kill them.\n\r", ch );
        return;
    }
    WAIT_STATE( victim, delay * PULSE_VIOLENCE );
    ch_printf( ch, "You've delayed %s for %d rounds.\n\r", victim->name, delay );
    return;
}

void do_setseverity( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int x;

    if (IS_NPC(ch))
        return;

    if ( !argument || !*argument )
    {
        send_to_char( "Syntax: setseverity [afk]<channel> <level>\n\r\n\r", ch );
        send_to_char( "Current log severities:\n\r", ch);

        ch_printf(ch, "%-12.12s %-4s %-4s\n\r",
                  "Channel", "Norm", "AFK");
        for (x=LOG_NORMAL; x<LOG_LAST; x++)
        {
            ch_printf(ch, "%-12.12s %-4d %-4d\n\r",
                      sysdata.logdefs[x].name,
                      ch->pcdata->log_severity[x],
                      ch->pcdata->afk_log_severity[x]);
        }
        return;
    }

    argument = one_argument(argument, arg);

    for (x=LOG_NORMAL; x<LOG_LAST; x++)
    {
        if (!str_prefix(sysdata.logdefs[x].name, arg) ||
            !str_cmp("all", arg))
        {
            ch->pcdata->log_severity[x] = UMAX(atoi(argument), SEV_ERR);
            ch_printf(ch, "Log severity '%s' level set to: %d\n\r",
                      sysdata.logdefs[x].name,
                      ch->pcdata->log_severity[x]);
        }
        if ((!str_prefix("afk", arg) &&
             !str_prefix(sysdata.logdefs[x].name, &arg[3])) ||
            !str_cmp("all", arg))
        {
            ch->pcdata->afk_log_severity[x] = UMAX(atoi(argument), SEV_CRIT);
            ch_printf(ch, "AFK Log severity '%s' level set to: %d\n\r",
                      sysdata.logdefs[x].name,
                      ch->pcdata->afk_log_severity[x]);
        }
    }

    send_to_char("Ok.\n\r", ch);
}

void do_drain( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    int x, drop;

    argument = one_argument(argument, arg);

    if (!(victim = get_char_world(ch, arg)))
    {
        send_to_char("No such character online.\n\r", ch);
        return;
    }

    if (ch == victim)
    {
        send_to_char("Are you mad?\n\r", ch);
        return;
    }

    if (!is_number(argument) || (drop=atoi(argument))<=0)
    {
        send_to_char("That is an invalid number of levels.\n\r", ch);
        return;
    }

    if (GetMaxLevel(ch) <= GetMaxLevel(victim))
    {
        send_to_char("They are too powerful to do this to.\n\r", ch);
        return;
    }

    for (x=0;x<MAX_CLASS;x++)
        victim->levels[x] = UMAX(victim->levels[x]-drop, 1);

    sprintf( log_buf, "%s drained %s %d levels",
             GET_NAME(ch), GET_NAME(victim), drop );
    log_string_plus(log_buf, LOG_MONITOR, GetMaxLevel(ch), SEV_CRIT);
    send_to_char("You have lost levels!\n\r", victim);
    send_to_char("Ok.\n\r", ch);
}

void do_sendcl( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;

    if ( !argument || argument[0] == '\0' )
    {
        send_to_char("Sendcl what?\n\r", ch);
        return;
    }

    for (vch = first_char; vch; vch = vch->next)
    {
        if (!vch->desc)
            continue;

        if ( GET_CON_STATE(vch) != CON_PLAYING &&
             GET_CON_STATE(vch) != CON_EDITING )
            continue;
        ch_printf(vch, "*** %s\n\r", argument);
    }
}


void do_reassigncmd(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    ALIAS_DATA *pal;
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg);

    if ( arg[0] == '\0' )
    {
        send_to_char( "Reassign who's command?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You can't do that to them.\n\r", ch );
        return;
    }

    if ( !*argument )
    {
        if (!victim->pcdata->first_alias)
        {
            send_to_char("They have no aliases defined!\n\r", ch);
            return;
        }
        pager_printf( ch, "%-20s   What it does\n\r", "Alias" );
        for (pal=victim->pcdata->first_alias;pal;pal=pal->next)
                pager_printf( ch, "%-20s %c %s\n\r",
                              pal->name,
                              IS_SET(pal->flags, ALIAS_REASSIGN)?'R':'A',
                              pal->cmd );
        return;
    }

    argument = one_argument(argument, arg);

    if ( !*argument )
    {
        pal = find_alias(victim, arg);
        if ( pal != NULL )
        {
            DISPOSE(pal->name);
            DISPOSE(pal->cmd);
            UNLINK(pal, victim->pcdata->first_alias, victim->pcdata->last_alias, next, prev);
            DISPOSE(pal);
            send_to_char("Deleted Alias/Reassign.\n\r", ch);
        } else
            send_to_char("That alias/reassign does not exist.\n\r", ch);
        return;
    }

    if ( (pal=find_alias(victim, arg)) == NULL )
    {
        CREATE(pal, ALIAS_DATA, 1);
        pal->name = str_dup(arg);
        smash_tilde(pal->name);
        pal->cmd  = str_dup(argument);
        smash_tilde(pal->cmd);
        pal->flags= ALIAS_REASSIGN;
        LINK(pal, victim->pcdata->first_alias, victim->pcdata->last_alias, next, prev);
        send_to_char("Created Reassign.\n\r", ch);
        return;
    }
    else
    {
        if (pal->cmd)
            DISPOSE(pal->cmd);
        pal->cmd  = str_dup(argument);
        smash_tilde(pal->cmd);
        send_to_char("Modified Alias/Reassign.\n\r", ch);
        return;
    }

    send_to_char("Ok.\n\r", ch);
}

char *	const	system_flags [] =
{
    "eclipse", "noportal", "noastral", "nosummon", "nokill", "nomagic",
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", ""
};

int SystemFlags=0;

void do_intervene(CHAR_DATA *ch, char *argument)
{
    if (!str_cmp("eclipse",argument))
    {
        if (IS_SYSTEMFLAG(SYS_ECLIPSE))
        {
            REMOVE_BIT(SystemFlags,SYS_ECLIPSE);
            send_to_char("You part the planets and the sun shines through!\n\r",ch);
            echo_to_all(AT_WHITE, "The planets return to thier normal orbit, slowly the light will return.", ECHOTAR_OUTSIDE);
            log_string_plus("The world is enlightend", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO);
        } else {
            SET_BIT(SystemFlags,SYS_ECLIPSE);
            send_to_char("You summon the planets and force an eclipse!\n\r",ch);
            echo_to_all(AT_DGREY, "The planets eclipse and hide the sun spreading darkness through out the land!", ECHOTAR_OUTSIDE);
            log_string_plus("World has been darkened", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO);
        }
        return;
    }
    if (!str_cmp("portal",argument))
    {
        if (IS_SYSTEMFLAG(SYS_NOPORTAL))
        {
            REMOVE_BIT(SystemFlags,SYS_NOPORTAL);
            send_to_char("You sort out the planes and allow portaling.\n\r",ch);
            log_string_plus("Portaling enabled", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO);
        } else {
            SET_BIT(SystemFlags,SYS_NOPORTAL);
            send_to_char("You scramble the planes to make portaling impossible.\n\r",ch);
            log_string_plus("Portaling disabled", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO);
        }
        return;
    }
    if (!str_cmp("astral",argument))
    {
        if (IS_SYSTEMFLAG(SYS_NOASTRAL))
        {
            REMOVE_BIT(SystemFlags,SYS_NOASTRAL);
            send_to_char("You shift the planes and allow astral travel.\n\r",ch);
            log_string_plus("Astral enabled", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO);
        } else {
            SET_BIT(SystemFlags,SYS_NOASTRAL);
            send_to_char("You shift the astral planes and make astral travel impossible.\n\r",ch);
            log_string_plus("Astral disabled", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO);
        }
        return;
    }
    if (!str_cmp("summon",argument))
    {
        if (IS_SYSTEMFLAG(SYS_NOSUMMON))
        {
            REMOVE_BIT(SystemFlags,SYS_NOSUMMON);
            send_to_char("You clear the fog to enable summons.\n\r",ch);
            log_string_plus("Summons enabled", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO);
        } else {
            SET_BIT(SystemFlags,SYS_NOSUMMON);
            send_to_char("A magical fog spreads throughout the land making summons impossible.\n\r",ch);
            log_string_plus("Summons disabled", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO);
        }
        return;
    }
    if (!str_cmp("kill",argument))
    {
        if (IS_SYSTEMFLAG(SYS_NOKILL))
        {
            REMOVE_BIT(SystemFlags,SYS_NOKILL);
            send_to_char("You let the anger lose inside you and the people of the land fight.\n\r",ch);
            log_string_plus("Killing enabled", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO);
        } else {
            SET_BIT(SystemFlags,SYS_NOKILL);
            send_to_char("You spread thoughts of peace throught the people of the land.\n\r",ch);
            log_string_plus("Killing disabled", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO);
        }
        return;
    }
    if (!str_cmp("magic",argument))
    {
        if (IS_SYSTEMFLAG(SYS_NOMAGIC))
        {
            REMOVE_BIT(SystemFlags,SYS_NOMAGIC);
            send_to_char("You realign the ebb and flow of magic.\n\r",ch);
            log_string_plus("Magic enabled", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO);
        } else {
            SET_BIT(SystemFlags,SYS_NOMAGIC);
            send_to_char("You disrupt the ebb and flow of magic.\n\r",ch);
            log_string_plus("Magic disabled", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO);
        }
        return;
    }
    if (!str_cmp("freexp",argument))
    {
        if (IS_SYSTEMFLAG(SYS_FREEXP))
        {
	    REMOVE_BIT(SystemFlags,SYS_FREEXP);
            send_to_char("You put the cap back on the experience bottle.\n\r",ch);
            log_string_plus("FreeXP disabled", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO);
        } else {
            SET_BIT(SystemFlags,SYS_FREEXP);
            send_to_char("You uncap the experience bottle and it flows forth.\n\r",ch);
            log_string_plus("FreeXP enabled", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO);
        }
        return;
    }
    send_to_char("Godly powers you have, but how do you wanna use them?\n\r",ch);

    ch_printf(ch, "Current affects: %s\n\r", flag_string(SystemFlags, system_flags));
    send_to_char("Availlable affects: eclipse portal astral summon kill magic\n\r", ch );
}

void do_dispel(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    AFFECT_DATA *paf;

    if ( !*argument )
    {
        send_to_char( "Syntax: dispel <character>\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if (victim != ch && GetMaxLevel(ch) < LEVEL_GREATER)
        victim = ch;

    while ((paf = victim->first_affect))
        affect_remove(victim, paf);

    send_to_char("Ok.\n\r", ch);
}

void do_planes(CHAR_DATA *ch, char *argument)
{
    sh_int p;

    for (p = FIRST_PLANE; p < LAST_PLANE; p++)
        ch_printf(ch, "%s\n\r", plane_names[p]);
}
