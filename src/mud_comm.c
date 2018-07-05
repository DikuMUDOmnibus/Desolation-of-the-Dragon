/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider             |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops  |~'~.VxvxV.~'~*
 ****************************************************************************
 *  The MUDprograms are heavily based on the original MOBprogram code that  *
 *  was written by N'Atas-ha.						    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: mud_comm.c,v 1.29 2004/04/06 22:00:10 dotd Exp $";*/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include "mud.h"
#include "gsn.h"

DECLARE_DO_FUN(do_help);
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_recall);
DECLARE_DO_FUN(do_flee);
DECLARE_DO_FUN(do_sit);
DECLARE_DO_FUN(do_variables);
DECLARE_DO_FUN(do_order);

bool MOBtrigger;

int get_exflag( char *flag );
ch_ret	simple_damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt );

char *mprog_type_to_name( sh_int type )
{
    switch ( type )
    {
    case IN_FILE_PROG:      return "in_file_prog";
    case ACT_PROG:          return "act_prog";
    case SPEECH_PROG:       return "speech_prog";
    case RAND_PROG:         return "rand_prog";
    case FIGHT_PROG:        return "fight_prog";
    case HITPRCNT_PROG:     return "hitprcnt_prog";
    case DEATH_PROG:        return "death_prog";
    case BIRTH_PROG:        return "birth_prog";
    case ENTRY_PROG:        return "entry_prog";
    case GREET_PROG:        return "greet_prog";
    case ALL_GREET_PROG:    return "all_greet_prog";
    case GIVE_PROG:         return "give_prog";
    case BRIBE_PROG:        return "bribe_prog";
    case HOUR_PROG:         return "hour_prog";
    case TIME_PROG:	    return "time_prog";
    case WEAR_PROG:         return "wear_prog";
    case REMOVE_PROG:       return "remove_prog";
    case SAC_PROG :         return "sac_prog";
    case LOOK_PROG:         return "look_prog";
    case EXA_PROG:          return "exa_prog";
    case ZAP_PROG:          return "zap_prog";
    case GET_PROG:          return "get_prog";
    case DROP_PROG:         return "drop_prog";
    case REPAIR_PROG:       return "repair_prog";
    case DAMAGE_PROG:       return "damage_prog";
    case PULL_PROG:         return "pull_prog";
    case PUSH_PROG:         return "push_prog";
    case SCRIPT_PROG:       return "script_prog";
    case SLEEP_PROG:        return "sleep_prog";
    case REST_PROG:         return "rest_prog";
    case LEAVE_PROG:        return "leave_prog";
    case USE_PROG:          return "use_prog";
    case QUEST_PROG:        return "quest_prog";
    case COMMAND_PROG:      return "command_prog";
    case AREA_RESET_PROG:   return "area_reset_prog";
    case AREA_INIT_PROG:    return "area_init_prog";
    default:                return "ERROR_PROG";
    }
}

bool charm_order_check(CHAR_DATA *ch)
{
    /* don't need to check AFF_CHARM, master is enough */
    if (ch->master)
    {
        if (ch->master->last_cmd == do_order)
        {
            sprintf(log_buf, "%s tried to order an mpcommand", GET_NAME(ch->master));
            log_string_plus(log_buf, LOG_MONITOR, LEVEL_LOG_CSET, SEV_ALERT);
            return TRUE;
        }
    }

    return FALSE;
}

/* A trivial rehack of do_mstat.  This doesnt show all the data, but just
 * enough to identify the mob and give its basic condition.  It does however,
 * show the MUDprograms which are set.
 */
void do_mpstat( CHAR_DATA *ch, char *argument )
{
    char        arg[MAX_INPUT_LENGTH];
    MPROG_DATA *mprg;
    CHAR_DATA  *victim;
    
    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
        send_to_char( "MProg stat whom?\n\r", ch );
        return;
    }
    
    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
    
    if ( !IS_NPC( victim ) )
    {
        send_to_char( "Only Mobiles can have MobPrograms!\n\r", ch);
        return;
    }
    
    if ( xIS_EMPTY(victim->pIndexData->progtypes) )
    {
        send_to_char( "That Mobile has no Programs set.\n\r", ch);
        return;
    }
    
    ch_printf( ch, "Name: %s.  Vnum: %d.\n\r",
               victim->name, victim->vnum );
    
    ch_printf( ch, "Short description: %s.\n\rLong  description: %s",
               victim->short_descr,
               victim->long_descr[0] != '\0' ?
               victim->long_descr : "(none).\n\r" );
    
    ch_printf( ch, "Hp: %d/%d.  Mana: %d/%d.  Move: %d/%d. \n\r",
               GET_HIT(victim),	GET_MAX_HIT(victim),
               GET_MANA(victim),	GET_MAX_MANA(victim),
               GET_MOVE(victim),	GET_MAX_MOVE(victim) );
    
    ch_printf( ch,
               "Lv: %s.  Class: %s.  Align: %d.  AC: %d.  Exp: %d.\n\r",
               GetLevelString(victim), GetClassString(victim), victim->alignment,
               GET_AC( victim ), GET_EXP(victim) );
    
    for ( mprg = victim->pIndexData->mudprogs; mprg; mprg = mprg->next )
        ch_printf( ch, ">%s %s\n\r%s\n\r",
                   mprog_type_to_name( mprg->progtype ),
                   mprg->arglist,
                   mprg->comlist );
    return;
}

/* Opstat - Scryn 8/12*/
void do_opstat( CHAR_DATA *ch, char *argument )
{
    char        arg[MAX_INPUT_LENGTH];
    MPROG_DATA *mprg;
    OBJ_DATA   *obj;
    
    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
        send_to_char( "OProg stat what?\n\r", ch );
        return;
    }
    
    if ( ( obj = get_obj_world( ch, arg ) ) == NULL )
    {
        send_to_char( "You cannot find that.\n\r", ch );
        return;
    }
    
    if ( xIS_EMPTY(obj->pIndexData->progtypes) )
    {
        send_to_char( "That object has no programs set.\n\r", ch);
        return;
    }
    
    ch_printf( ch, "Name: %s.  Vnum: %d.\n\r",
               obj->name, obj->vnum );
    
    ch_printf( ch, "Short description: %s.\n\r",
               obj->short_descr );
    
    for ( mprg = obj->pIndexData->mudprogs; mprg; mprg = mprg->next )
        ch_printf( ch, ">%s %s\n\r%s\n\r",
                   mprog_type_to_name( mprg->progtype ),
                   mprg->arglist,
                   mprg->comlist );
    
    return;
    
}

/* Rpstat - Scryn 8/12 */
void do_rpstat( CHAR_DATA *ch, char *argument )
{
    MPROG_DATA *mprg;
    
    if ( xIS_EMPTY(ch->in_room->progtypes) )
    {
        send_to_char( "This room has no programs set.\n\r", ch);
        return;
    }
    
    ch_printf( ch, "Name: %s.  Vnum: %d.\n\r",
               ch->in_room->name, ch->in_room->vnum );
    
    for ( mprg = ch->in_room->mudprogs; mprg; mprg = mprg->next )
        ch_printf( ch, ">%s %s\n\r%s\n\r",
                   mprog_type_to_name( mprg->progtype ),
                   mprg->arglist,
                   mprg->comlist );
    return;
}

/* Drops absolutely w/o messages. */
void do_mpdrop( CHAR_DATA *ch, char *argument )
{
OBJ_DATA *obj;

    if (!ch )
    {
        bug("Nonexistent ch in do_mpdrop!");
        return;
    }

    if ( charm_order_check(ch) )
        return;
    
    if ( !IS_NPC( ch ) || (ch->desc))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    if ( argument[0] == '\0' )
    {
        progbug( "Mpdrop - No argument", ch );
        return;
    }
    

if ((obj = get_obj_carry(ch, argument)) == NULL)
   return;

separate_obj(obj);
obj_from_char(obj);
obj = obj_to_room(obj, ch->in_room);
return;
}

/* Prints the argument to all the rooms around the mobile */
void do_mpasound( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in_room;
    EXIT_DATA       *pexit;
    int		     actflags;
    
    if (!ch )
    {
        bug("Nonexistent ch in do_mpasound!");
        return;
    }
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || (ch->desc))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    if ( argument[0] == '\0' )
    {
        progbug( "Mpasound - No argument", ch );
        return;
    }
    
    actflags = ch->act;
    REMOVE_BIT(ch->act, ACT_SECRETIVE);
    was_in_room = ch->in_room;
    for ( pexit = was_in_room->first_exit; pexit; pexit = pexit->next )
    {
        if ( pexit->to_room
             &&   pexit->to_room != was_in_room )
        {
            ch->in_room = pexit->to_room;
            MOBtrigger  = FALSE;
            act( AT_SAY, argument, ch, NULL, NULL, TO_ROOM );
        }
    }
    ch->act = actflags;
    ch->in_room = was_in_room;
    return;
}

/* lets the mobile kill any player or mobile without murder*/

void do_mpkill( CHAR_DATA *ch, char *argument )
{
    char      arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    
    if (!ch )
    {
        bug( "Nonexistent ch in do_mpkill!" );
        return;
    }
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || (ch->desc))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
        progbug( "MpKill - no argument", ch );
        return;
    }
    
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        progbug( "MpKill - Victim not in room", ch );
        return;
    }
    
    if ( victim == ch )
    {
        progbug( "MpKill - Bad victim to attack", ch );
        return;
    }
    
    if ( charm_order_check( ch ) && ch->master == victim )
    {
        progbug( "MpKill - Charmed mob attacking master", ch );
        return;
    }
    
    if ( ch->position == POS_FIGHTING )
    {
        progbug( "MpKill - Already fighting", ch );
        return;
    }
    
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}


/* lets the mobile destroy an object in its inventory
 it can also destroy a worn object and it can destroy
 items using all.xxxxx or just plain all of them */

void do_mpjunk( CHAR_DATA *ch, char *argument )
{
    char      arg[ MAX_INPUT_LENGTH ];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || (ch->desc))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    one_argument( argument, arg );
    
    if ( arg[0] == '\0')
    {
        progbug( "Mpjunk - No argument", ch );
        return;
    }
    
    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
        if ( ( obj = get_obj_wear( ch, arg ) ) != NULL )
        {
            unequip_char( ch, obj );
            extract_obj( obj );
            return;
        }
        if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
            return;
        extract_obj( obj );
    }
    else
        for ( obj = ch->first_carrying; obj; obj = obj_next )
        {
            obj_next = obj->next_content;
            if ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
            {
                if ( obj->wear_loc != WEAR_NONE)
                    unequip_char( ch, obj );
                extract_obj( obj );
            }
        }
    
    return;
    
}

/*
 * This function examines a text string to see if the first "word" is a
 * color indicator (e.g. _red, _whi_, _blu).  -  Gorog
 */
int get_color(char *argument)    /* get color code from command string */ 
{ 
    char color[MAX_INPUT_LENGTH]; 
    char *cptr; 
    static char const * color_list= 
        "_bla_red_dgr_bro_dbl_pur_cya_cha_dch_ora_gre_yel_blu_pin_lbl_whi"; 
    static char const * blink_list= 
        "*bla*red*dgr*bro*dbl*pur*cya*cha*dch*ora*gre*yel*blu*pin*lbl*whi"; 
    
    one_argument (argument, color); 
    if (color[0]!='_' && color[0]!='*') return 0;
    if ( (cptr = strstr(color_list, color)) ) 
        return (cptr - color_list) / 4; 
    if ( (cptr = strstr(blink_list, color)) ) 
        return (cptr - blink_list) / 4 + AT_BLINK; 
    return 0; 
} 

/* prints the message to everyone */
void do_mpechoall( CHAR_DATA *ch, char *argument )
{
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || (ch->desc))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    echo_to_all(AT_PLAIN, argument, ECHOTAR_ALL);
}

/* prints the message to everyone in the room other than the mob and victim */
void do_mpechoaround( CHAR_DATA *ch, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    int        actflags;
    sh_int     color;
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || (ch->desc))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    argument = one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
        progbug( "Mpechoaround - No argument", ch );
        return;
    }
    
    if ( !( victim=get_char_room( ch, arg ) ) )
    {
        progbug( "Mpechoaround - victim does not exist", ch );
        return;
    }
    
    actflags = ch->act;
    REMOVE_BIT(ch->act, ACT_SECRETIVE);
    
    if ( (color = get_color(argument)) )
    {
        argument = one_argument( argument, arg );
        act( color, argument, ch, NULL, victim, TO_NOTVICT );
    }
    else
        act( AT_ACTION, argument, ch, NULL, victim, TO_NOTVICT );
    
    ch->act = actflags;
}


/* prints message only to victim */

void do_mpechoat( CHAR_DATA *ch, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    int        actflags;
    sh_int     color;
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || (ch->desc))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    argument = one_argument( argument, arg );
    
    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        progbug( "Mpechoat - No argument", ch );
        return;
    }
    
    if ( !( victim = get_char_room( ch, arg ) ) )
    {
        progbug( "Mpechoat - victim does not exist", ch );
        return;
    }
    
    actflags = ch->act;
    REMOVE_BIT(ch->act, ACT_SECRETIVE);
    
    if ( (color = get_color(argument)) )
    {
        argument = one_argument( argument, arg );
        act( color, argument, ch, NULL, victim, TO_VICT );
    }
    else
        act( AT_ACTION, argument, ch, NULL, victim, TO_VICT );
    
    ch->act = actflags;
}


/* prints message to room at large. */

void do_mpecho( CHAR_DATA *ch, char *argument )
{
    char       arg1 [MAX_INPUT_LENGTH];
    sh_int     color;
    int        actflags;
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC(ch) || (ch->desc))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    if ( argument[0] == '\0' )
    {
        progbug( "Mpecho - called w/o argument", ch );
        return;
    }
    
    actflags = ch->act;
    REMOVE_BIT(ch->act, ACT_SECRETIVE);
    
    if ( (color = get_color(argument)) )
    {
        argument = one_argument ( argument, arg1 );
        act( color, argument, ch, NULL, NULL, TO_ROOM );
    }
    else
        act( AT_ACTION, argument, ch, NULL, NULL, TO_ROOM );
    
    ch->act = actflags;
}


/* lets the mobile load an item or mobile.  All items
 are loaded into inventory.  you can specify a level with
 the load object portion as well. */

void do_mpmload( CHAR_DATA *ch, char *argument )
{
    char            arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA      *victim;
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || (ch->desc))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    one_argument( argument, arg );
    
    if ( arg[0] == '\0' || !is_number(arg) )
    {
        progbug( "Mpmload - Bad vnum as arg", ch );
        return;
    }
    
    if ( ( victim = create_mobile( atoi( arg ) ) ) == NULL )
    {
        progbug( "Mpmload - Bad mob vnum", ch );
        return;
    }
    char_to_room( victim, ch->in_room );
    mprog_birth_trigger(ch, victim);
    return;
}

void do_mpoload( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    OBJ_DATA       *obj;
    int		    timer = 0;
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || (ch->desc))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    argument = one_argument( argument, arg1 );
    
    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
        progbug( "Mpoload - Bad syntax", ch );
        return;
    }
    
    /*
     * New feature from Thoric.
     */
    timer = atoi( argument );
    if ( timer < 0 )
    {
        progbug( "Mpoload - Bad timer", ch );
        return;
    }
    
    if ( ( obj = create_object( atoi( arg1 ) ) ) == NULL )
    {
        progbug( "Mpoload - Bad vnum arg", ch );
        return;
    }
    obj->timer = timer;
    if ( CAN_WEAR(obj, ITEM_TAKE) )
        obj_to_char( obj, ch );
    else
        obj_to_room( obj, ch->in_room );
    
    return;
}

/* lets the mobile purge all objects and other npcs in the room,
 or purge a specified object or mob in the room.  It can purge
 itself, but this had best be the last command in the MUDprogram
 otherwise ugly stuff will happen */

void do_mppurge( CHAR_DATA *ch, char *argument )
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || (ch->desc))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
        /* 'purge' */
        CHAR_DATA *vnext;
        
        for ( victim = ch->in_room->first_person; victim; victim = vnext )
        {
            vnext = victim->next_in_room;
            if ( IS_NPC( victim ) && victim != ch )
                extract_char( victim, TRUE );
        }
        while ( ch->in_room->first_content )
            extract_obj( ch->in_room->first_content );
        
        return;
    }
    
    if ( (victim = get_char_room( ch, arg )) == NULL )
    {
        if ( (obj = get_obj_here( ch, arg )) != NULL )
            extract_obj( obj );
        else
            progbug( "Mppurge - Bad argument", ch );
        return;
    }
    
    if ( !IS_NPC( victim ) )
    {
        progbug( "Mppurge - Trying to purge a PC", ch );
        return;
    }
    
    if ( victim == ch )
    {
        progbug( "Mppurge - Trying to purge oneself", ch );
        return;
    }
    
    if ( IS_NPC( victim ) && victim->vnum == 3 )
    {
        progbug( "Mppurge: trying to purge supermob", ch );
        return;
    }
    
    extract_char( victim, TRUE );
    return;
}


/* Allow mobiles to go wizinvis with programs -- SB */

void do_mpinvis( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int level;
    
    if ( !IS_NPC(ch) || (ch->desc))
    {
        send_to_char( "Huh?\n\r", ch);
        return;
    }
    
    argument = one_argument( argument, arg );
    if ( arg && arg[0] != '\0' )
    {
        if ( !is_number( arg ) )
        {
            progbug( "Mpinvis - Non numeric argument ", ch );
            return;
        }       
        level = atoi( arg );
        if ( level < 2 || level > 51 )
        {
            progbug( "MPinvis - Invalid level ", ch );
            return;
        }
        
        ch->mobinvis = level;
        ch_printf( ch, "Mobinvis level set to %d.\n\r", level );
        return;
    }
    
    if ( ch->mobinvis < 2 )
        ch->mobinvis = GetMaxLevel(ch);
    
    if ( IS_SET(ch->act, ACT_MOBINVIS) )
    {
        REMOVE_BIT(ch->act, ACT_MOBINVIS);
        act(AT_IMMORT, "$n slowly fades into existence.", ch, NULL, NULL,TO_ROOM );
        send_to_char( "You slowly fade back into existence.\n\r", ch );       
    }
    else
    {
        SET_BIT(ch->act, ACT_MOBINVIS);
        act( AT_IMMORT, "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
        send_to_char( "You slowly vanish into thin air.\n\r", ch );
    }
    return;
}

/* lets the mobile goto any location it wishes that is not private */

void do_mpgoto( CHAR_DATA *ch, char *argument )
{
    char             arg[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *location;
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || (ch->desc))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        progbug( "Mpgoto - No argument", ch );
        return;
    }
    
    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
        progbug( "Mpgoto - No such location", ch );
        return;
    }
    
    if ( ch->fighting )
        stop_fighting( ch, TRUE );
    
    char_from_room( ch );
    char_to_room( ch, location );
    
    return;
}

/* lets the mobile do a command at another location. Very useful */

void do_mpat( CHAR_DATA *ch, char *argument )
{
    char             arg[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA       *wch;
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || (ch->desc))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    argument = one_argument( argument, arg );
    
    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        progbug( "Mpat - Bad argument", ch );
        return;
    }
    
    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
        progbug( "Mpat - No such location", ch );
        return;
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
        if ( wch == ch )
        {
            char_from_room( ch );
            char_to_room( ch, original );
            break;
        }
    
    return;
}

/* allow a mobile to advance a player's level... very dangerous */
void do_mpadvance( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int level;
    int iLevel, i;
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || ch->desc )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    argument = one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
        progbug( "Mpadvance - Bad syntax", ch );
        return;
    }
    
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        progbug( "Mpadvance - Victim not there", ch );
        return;
    }
    
    if ( IS_NPC(victim) )
    {
        progbug( "Mpadvance - Victim is NPC", ch );
        return;
    }
    
    if ( GetMaxLevel(victim) >= LEVEL_AVATAR )
        return;
    
    level = GetMaxLevel(victim) + 1;
    
    if ( GetMaxLevel(victim) > GetMaxLevel(ch) )
    {
        act( AT_TELL, "$n tells you, 'Sorry... you must seek someone more powerful than I.'",
             ch, NULL, victim, TO_VICT );
        return;
    }
    
    if (GetMaxLevel(victim) >= LEVEL_AVATAR)
    {
        set_char_color( AT_IMMORT, victim );
        act( AT_IMMORT, "$n makes some arcane gestures with $s hands, then points $s fingers at you!",
             ch, NULL, victim, TO_VICT );
        act( AT_IMMORT, "$n makes some arcane gestures with $s hands, then points $s fingers at $N!",
             ch, NULL, victim, TO_NOTVICT );
        set_char_color( AT_WHITE, victim );
        send_to_char( "You suddenly feel very strange...\n\r\n\r", victim );
        set_char_color( AT_LBLUE, victim );
    }
    
    switch(level)
    {
    default:
        send_to_char( "You feel more powerful!\n\r", victim );
        break;
    case LEVEL_IMMORTAL:
        do_help(victim, "M_GODLVL1_" );
        send_to_char( "You awake... all your possessions are gone.\n\r", victim );
        
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
    
    for (i = 0; i < MAX_CLASS; ++i) {
        if (IS_ACTIVE(victim, i) && !HAD_CLASS(ch, i)) {
            for ( iLevel = GET_LEVEL(victim, i) ; iLevel < level; iLevel++ )
            {
                if (level < LEVEL_IMMORTAL)
                    send_to_char( "You raise a level!!  ", victim );
                advance_level( victim, i );
            }
        }
    }
    victim->exp   = 1000 * UMAX( 1, GetMaxLevel(victim) );
    victim->trust = 0;
    return;
}



/* lets the mobile transfer people.  the all argument transfers
 everyone in the current room to the specified location */

void do_mptransfer( CHAR_DATA *ch, char *argument )
{
    char             arg1[ MAX_INPUT_LENGTH ];
    char             arg2[ MAX_INPUT_LENGTH ];
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *location;
    CHAR_DATA       *victim;
    CHAR_DATA       *nextinroom;
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || (ch->desc))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    
    if ( arg1[0] == '\0' )
    {
        progbug( "Mptransfer - Bad syntax", ch );
        return;
    }
    
    /* Put in the variable nextinroom to make this work right. -Narn */
    if ( !str_cmp( arg1, "all" ) )
    {
        for ( victim = ch->in_room->first_person; victim; victim = nextinroom )
        {
            nextinroom = victim->next_in_room;
            if ( victim != ch
                 &&   !NOT_AUTHED(victim)
                 &&   can_see( ch, victim ) )
            {
                sprintf( buf, "\"%s\" %s", victim->name, arg2 );
                do_mptransfer( ch, buf );
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
            progbug( "Mptransfer - No such location", ch );
            return;
        }
        
        if ( room_is_private( location ) )
        {
            progbug( "Mptransfer - Private room", ch );
            return;
        }
    }
    
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        progbug( "Mptransfer - No such person", ch );
        return;
    }
    
    if ( !victim->in_room )
    {
        progbug( "Mptransfer - Victim in Limbo", ch );
        return;
    }
    
    if (NOT_AUTHED(victim) && location->area != victim->in_room->area)
    {
        progbug( "Mptransfer - transferring unauthorized player", ch);
        return;
    }
    
    
    /* If victim not in area's level range, do not transfer */
    if ( !in_hard_range( victim, location->area ) 
         &&   !IS_SET( location->room_flags, ROOM_PROTOTYPE ) )
        return;
    
    if ( victim->fighting )
        stop_fighting( victim, TRUE );
    
    char_from_room( victim );
    char_to_room( victim, location );
    
    return;
}

/* lets the mobile force someone to do something.  must be mortal level
 and the all argument only affects those in the room with the mobile */

void do_mpforce( CHAR_DATA *ch, char *argument )
{
    char arg[ MAX_INPUT_LENGTH ];
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || ch->desc )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    argument = one_argument( argument, arg );
    
    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        progbug( "Mpforce - Bad syntax", ch );
        return;
    }
    
    if ( !str_cmp( arg, "all" ) )
    {
        CHAR_DATA *vch;
        
        for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
            if ( get_trust( vch ) < get_trust( ch ) && can_see( ch, vch ) )
                interpret( vch, argument );
    }
    else
    {
        CHAR_DATA *victim;
        
        if ( ( victim = get_char_room( ch, arg ) ) == NULL )
        {
            progbug( "Mpforce - No such victim", ch );
            return;
        }
        
        if ( victim == ch )
        {
            progbug( "Mpforce - Forcing oneself", ch );
            return;
        }
        
        if ( !IS_NPC( victim )
             && ( !victim->desc )
             && IS_IMMORTAL( victim ) )
        {
            progbug( "Mpforce - Attempting to force link dead immortal", ch );
            return;
        }
        
        
        interpret( victim, argument );
    }
    
    return;
}



/*
 *  Haus' toys follow:
 */


/*
 * syntax:  mppractice victim spell_name max%
 *
 */
void do_mp_practice( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char arg3[ MAX_INPUT_LENGTH ];
    char buf[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    int sn, max, tmp, adept;
    char *skill_name;
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || (ch->desc))   /* security breach, i guess */
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    argument = one_argument( argument, arg1 );  
    argument = one_argument( argument, arg2 );  
    argument = one_argument( argument, arg3 );  
    
    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        send_to_char( "Mppractice: bad syntax", ch );
        progbug( "Mppractice - Bad syntax", ch );
        return;
    }
    
    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char("Mppractice: Student not in room? Invis?", ch);
        progbug( "Mppractice: Invalid student not in room", ch );
        return;
    }
    
    if ( ( sn = skill_lookup( arg2 ) ) < 0 )
    {
        send_to_char("Mppractice: Invalid spell/skill name", ch);
        progbug( "Mppractice: Invalid spell/skill name", ch );
        return;
    }
    
    
    if(IS_NPC(victim))
    {
        send_to_char("Mppractice: Can't train a mob", ch);
        progbug("Mppractice: Can't train a mob", ch );
        return;
    }
    
    skill_name = skill_table[sn]->name;
    
    max = atoi( arg3 );
    if( (max<0) || (max>100) )
    {
        sprintf( bug_buf, "mp_practice: Invalid maxpercent: %d", max );
        send_to_char( bug_buf, ch);
        progbug( bug_buf, ch );
        return;
    }
    
    if(GetMaxLevel(victim) < skill_table[sn]->skill_level[LowSkCl(ch, sn)])
    {
        sprintf(buf,"$n attempts to tutor you in %s, but it's beyond your comprehension.",skill_name);
        act( AT_TELL, buf, ch, NULL, victim, TO_VICT );
        return;
    }
    
    /* adept is how high the player can learn it */
    /* adept = class_table[ch->class]->skill_adept; */
    adept = GET_ADEPT(victim,sn);
    
    if ( (LEARNED(victim, sn) >= adept )
         ||   (LEARNED(victim, sn) >= max   ) )
    {
        sprintf(buf,"$n shows some knowledge of %s, but yours is clearly superior.",skill_name);
        act( AT_TELL, buf, ch, NULL, victim, TO_VICT );
        return;
    }
    
    
    /* past here, victim learns something */
    tmp = UMIN(LEARNED(victim, sn) + int_app[get_curr_int(victim)].learn, max);
    act( AT_ACTION, "$N demonstrates $t to you.  You feel more learned in this subject.", victim, skill_table[sn]->name, ch,TO_CHAR );
    
    victim->pcdata->learned[sn] = max; 
    
    
    if ( LEARNED(victim, sn) >= adept )
    {
        victim->pcdata->learned[sn] = adept;
        act( AT_TELL, "$n tells you, 'You have learned all I know on this subject...'",
             ch, NULL, victim, TO_VICT );
    }
    return;
    
}

/*
 * syntax: mpslay (character)
 */
void do_mp_slay( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || ch->desc )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    argument = one_argument( argument, arg1 );  
    if ( arg1[0] == '\0' )
    {
        send_to_char( "mpslay whom?\n\r", ch );
        progbug( "Mpslay: invalid (nonexistent?) argument", ch );
        return;
    }
    
    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mpslay: victim not in room", ch );
        return;
    }
    
    if ( victim == ch )
    {
        send_to_char( "You try to slay yourself.  You fail.\n\r", ch );
        progbug( "Mpslay: trying to slay self", ch );
        return;
    }
    
    if ( IS_NPC( victim ) && victim->vnum == 3 )
    {
        send_to_char( "You cannot slay supermob!\n\r", ch );
        progbug( "Mpslay: trying to slay supermob", ch );
        return;
    }
    
    if( GetMaxLevel(victim) < LEVEL_IMMORTAL)
    {
        /*
         act( AT_IMMORT, "You slay $M in cold blood!",  ch, NULL, victim, TO_CHAR);
         act( AT_IMMORT, "$n slays you in cold blood!", ch, NULL, victim, TO_VICT);
         act( AT_IMMORT, "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT);
         */ 
        set_cur_char(victim);
        raw_kill( ch, victim, TYPE_UNDEFINED );
        stop_fighting( ch, FALSE );
        stop_hating( ch );
        stop_fearing( ch );
        stop_hunting( ch );
    } 
    else 
    {
        act( AT_IMMORT, "You attempt to slay $M and fail!",  ch, NULL, victim, TO_CHAR);
        act( AT_IMMORT, "$n attempts to slay you.  What a kneebiter!", ch, NULL, victim, TO_VICT);
        act( AT_IMMORT, "$n attempts to slay $N.  Needless to say $e fails.",  ch, NULL, victim, TO_NOTVICT);
    }
    return;
}

/*
 * syntax: mpdamage (character) (#hps)
 */
void do_mp_damage( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    int dam;
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || ( ch->desc && get_trust( ch ) < LEVEL_IMMORTAL )  )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    argument = one_argument( argument, arg1 );  
    argument = one_argument( argument, arg2 );  
    
    if ( arg1[0] == '\0' )
    {
        send_to_char( "mpdamage whom?\n\r", ch );
        progbug( "Mpdamage: invalid argument1", ch );
        return;
    }
    
    if ( arg2[0] == '\0' )
    {
        send_to_char( "mpdamage inflict how many hps?\n\r", ch );
        progbug( "Mpdamage: invalid argument2", ch );
        return;
    }
    
    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mpdamage: victim not in room", ch );
        return;
    }
    
    if ( victim == ch )
    {
        send_to_char( "You can't mpdamage yourself.\n\r", ch );
        progbug( "Mpdamage: trying to damage self", ch );
        return;
    }
    
    dam = atoi(arg2);
    
    if( (dam<0) || (dam>32000) )
    {
        send_to_char( "Mpdamage how much?\n\r", ch );
        progbug( "Mpdamage: invalid (nonexistent?) argument", ch );
        return;
    }
    
    /* this is kinda begging for trouble        */
    /*
     * Note from Thoric to whoever put this in...
     * Wouldn't it be better to call damage(ch, ch, dam, dt)?
     * I hate redundant code
     */
    if ( simple_damage(ch, victim, dam, TYPE_UNDEFINED ) == rVICT_DIED ) 
    {
        stop_fighting( ch, FALSE );
        stop_hating( ch );
        stop_fearing( ch );
        stop_hunting( ch );
    }
    
    return;
}


/*
 * syntax: mprestore (character) (#hps)                Gorog
 */
void do_mp_restore( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    int hp;
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || ( ch->desc && get_trust( ch ) < LEVEL_IMMORTAL )  )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    argument = one_argument( argument, arg1 );  
    argument = one_argument( argument, arg2 );  
    
    if ( arg1[0] == '\0' )
    {
        send_to_char( "mprestore whom?\n\r", ch );
        progbug( "Mprestore: invalid argument1", ch );
        return;
    }
    
    if ( arg2[0] == '\0' )
    {
        send_to_char( "mprestore how many hps?\n\r", ch );
        progbug( "Mprestore: invalid argument2", ch );
        return;
    }
    
    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mprestore: victim not in room", ch );
        return;
    }
    
    hp = atoi(arg2);
    
    if( (hp<0) || (hp>32000) )
    {
        send_to_char( "Mprestore how much?\n\r", ch );
        progbug( "Mprestore: invalid (nonexistent?) argument", ch );
        return;
    }
    hp += GET_HIT(victim);
    GET_HIT(victim) = (hp > 32000 || hp < 0 || hp > GET_MAX_HIT(victim)) ?
        GET_MAX_HIT(victim) : hp;
}

/*
 * Syntax mpfavor target number
 * Raise a player's favor in progs.
 */
void  do_mpfavor( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    int favor;
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || ( ch->desc && get_trust( ch ) < LEVEL_IMMORTAL )  )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    
    if ( arg1[0] == '\0' )
    {
        send_to_char( "mpfavor whom?\n\r", ch );
        progbug( "Mpfavor: invalid argument1", ch );
        return;
    }
    
    if ( arg2[0] == '\0' )
    {
        send_to_char( "mpfavor how much favor?\n\r", ch );
        progbug( "Mpfavor: invalid argument2", ch );
        return;
    }
    
    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Victim must be in room.\n\r", ch );
        progbug( "Mpfavor: victim not in room", ch );
        return;
    }
    
    favor = atoi(arg2);
    victim->pcdata->favor = URANGE( -1000, victim->pcdata->favor + favor, 1000 );
} 

/*
 * Syntax mp_open_passage x y z
 *
 * opens a 1-way passage from room x to room y in direction z
 *
 *  won't mess with existing exits
 */
void do_mp_open_passage( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char arg3[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *targetRoom, *fromRoom;
    int targetRoomVnum, fromRoomVnum, exit_num, flags = EX_PASSAGE;
    EXIT_DATA *pexit;
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || ( ch->desc && get_trust( ch ) < LEVEL_IMMORTAL )  )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    argument = one_argument( argument, arg1 );  
    argument = one_argument( argument, arg2 );  
    argument = one_argument( argument, arg3 );  
    
    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        progbug( "MpOpenPassage - Bad syntax", ch );
        return;
    }
    
    if( !is_number(arg1) )
    {
        progbug( "MpOpenPassage - Bad syntax", ch );
        return;
    }
    
    fromRoomVnum = atoi(arg1);
    if(  (fromRoom = get_room_index( fromRoomVnum ) )  ==NULL)
    {
        progbug( "MpOpenPassage - Bad syntax", ch );
        return;
    }
    
    if( !is_number(arg2) )
    {
        progbug( "MpOpenPassage - Bad syntax", ch );
        return;
    }
    
    targetRoomVnum = atoi(arg2);
    if(  (targetRoom = get_room_index( targetRoomVnum ) )  ==NULL)
    {
        progbug( "MpOpenPassage - Bad syntax", ch );
        return;
    }
    
    if( is_number(arg3) )
        exit_num = atoi(arg3);
    else
        exit_num = get_dir(arg3);

    if( (exit_num < 0) || (exit_num >= MAX_REXITS) )
    {
        progbug( "MpOpenPassage - Bad syntax", ch );
        return;
    }
    
    if( (pexit = get_exit( fromRoom, exit_num )) != NULL )
    {
        if( !IS_SET( pexit->exit_info, EX_PASSAGE) )
            return;
        progbug( "MpOpenPassage - Exit exists", ch );
        return;
    }

    if (argument)
    {
        int value;
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg1 );
            value = get_exflag( arg1 );
            if ( value < 0 || value > MAX_EXFLAG )
                progbug( "MpOpenPassage - Unknown exit flag", ch );
            else
                TOGGLE_BIT( flags, 1 << value );
        }

    }

    pexit = make_exit( fromRoom, targetRoom, exit_num );
    pexit->keyword 		= STRALLOC( "" );
    pexit->description		= STRALLOC( "" );
    pexit->key     		= 0;
    pexit->exit_info		= flags;
    
    /* act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_CHAR ); */
    /* act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_ROOM ); */
    
    return;
}

/*
 * Syntax mp_close_passage x y 
 *
 * closes a passage in room x leading in direction y
 *
 * the exit must have EX_PASSAGE set
 */
void do_mp_close_passage( CHAR_DATA *ch, char *argument )
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    char arg3[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *fromRoom;
    int fromRoomVnum, exit_num;
    EXIT_DATA *pexit;
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || ( ch->desc && get_trust( ch ) < LEVEL_IMMORTAL )  )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    argument = one_argument( argument, arg1 );  
    argument = one_argument( argument, arg2 );  
    argument = one_argument( argument, arg3 );  
    
    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg2[0] == '\0' )
    {
        progbug( "MpClosePassage - Bad syntax", ch );
        return;
    }
    
    if( !is_number(arg1) )
    {
        progbug( "MpClosePassage - Bad syntax", ch );
        return;
    }
    
    fromRoomVnum = atoi(arg1);
    if(  (fromRoom = get_room_index( fromRoomVnum ) )  ==NULL)
    {
        progbug( "MpClosePassage - Bad syntax", ch );
        return;
    }
    
    if( is_number(arg2) )
        exit_num = atoi(arg2);
    else
        exit_num = get_dir(arg2);
    
    if( (exit_num < 0) || (exit_num >= MAX_REXITS) )
    {
        progbug( "MpClosePassage - Bad syntax", ch );
        return;
    }
    
    if( ( pexit = get_exit(fromRoom, exit_num) ) == NULL )
    {
        return;    /* already closed, ignore...  so rand_progs */
        /*                            can close without spam */
    }
    
    if( !IS_SET( pexit->exit_info, EX_PASSAGE) )
    {
        progbug( "MpClosePassage - Exit not a passage", ch );
        return;
    }
    
    extract_exit( fromRoom, pexit );
    
    /* act( AT_PLAIN, "A passage closes!", ch, NULL, NULL, TO_CHAR ); */
    /* act( AT_PLAIN, "A passage closes!", ch, NULL, NULL, TO_ROOM ); */
    
    return;
}



/*
 * Does nothing.  Used for scripts.
 */
void do_mpnothing( CHAR_DATA *ch, char *argument )
{
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || ( ch->desc && get_trust( ch ) < LEVEL_IMMORTAL )  )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    return;
}


/*
 *   Sends a message to sleeping character.  Should be fun
 *    with room sleep_progs
 *
 */
void do_mpdream( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_STRING_LENGTH];
    CHAR_DATA *vict;
    
    if ( charm_order_check( ch ) )
        return;
    
    if ( !IS_NPC( ch ) || ( ch->desc && get_trust( ch ) < LEVEL_IMMORTAL )  )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    argument = one_argument( argument, arg1 );  
    
    if (  (vict =get_char_world(ch, arg1)) == NULL )
    {
        progbug( "Mpdream: No such character", ch );
        return;
    }
    
    if( vict->position <= POS_SLEEPING)
    {
        send_to_char(argument, vict);
        send_to_char("\n\r",   vict);
    } 
    return;
}

void do_mpapply( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    
    if ( !IS_NPC( ch ) || (ch->desc))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    if ( charm_order_check( ch ) )
        return;
    
    if (argument[0] == '\0')
    {
        progbug("Mpapply - bad syntax", ch );
        return;
    }
    
    if ( (victim = get_char_room( ch, argument ) ) == NULL )
    {
        progbug("Mpapply - no such player in room.", ch );
        return;
    }
    
    if ( !victim->desc )
    {
        send_to_char( "Not on linkdeads.\n\r", ch );
        return;
    }
    
    if( !NOT_AUTHED(victim) )
        return;
    
    if( victim->pcdata->auth_state >= 1 )
        return;
    
    sprintf( log_buf, "%s@%s new %s %s applying for authorization...", 
             victim->name, victim->desc->host, 
             race_table[victim->race].race_name, 
             GetClassString(victim) );
    log_string_plus( log_buf,LOG_MONITOR,LEVEL_IMMORTAL,SEV_ALERT);
    victim->pcdata->auth_state = 1;
    return;
}

void do_mpapplyb( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    
    if ( !IS_NPC( ch ) || (ch->desc))
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    
    if ( charm_order_check( ch ) )
        return;
    
    if (argument[0] == '\0')
    {
        progbug("Mpapplyb - bad syntax", ch );
        return;
    }
    
    if ( (victim = get_char_room( ch, argument ) ) == NULL )
    {
        progbug("Mpapplyb - no such player in room.", ch );
        return;
    }
    
    if ( !victim->desc )
    {
        send_to_char( "Not on linkdeads.\n\r", ch );
        return;
    }
    
    if( !NOT_AUTHED(victim) )
        return;
    
    if ( get_timer(victim, TIMER_APPLIED) >= 1)
        return;
    
    switch( victim->pcdata->auth_state )
    {
    case 0:
    case 1:   
    default:
        send_to_char( "You attempt to regain the gods' attention.\n\r", victim);
        sprintf( log_buf, "%s@%s new %s %s applying for authorization...",                      
                 victim->name, victim->desc->host,
                 race_table[victim->race].race_name, 
                 GetClassString(victim) );
        log_string_plus( log_buf,LOG_MONITOR,LEVEL_IMMORTAL,SEV_ALERT);
        add_timer(victim, TIMER_APPLIED, 10, NULL, 0);
        victim->pcdata->auth_state = 1;
        break; 
        
    case 2:
        send_to_char("Your name has been deemed unsuitable by the gods.  Please choose a more medieval name with the 'name' command.\n\r", victim);
        add_timer(victim, TIMER_APPLIED, 10, NULL, 0);
        break;
        
    case 3: {
        char buf[MAX_INPUT_LENGTH];
        sprintf(buf, "The gods permit you to enter the %s.\n\r", MUD_NAME);
        send_to_char(buf,victim);
        REMOVE_BIT(victim->pcdata->flags, PCFLAG_UNAUTHED);
        if ( victim->fighting )
            stop_fighting( victim, TRUE );
        char_from_room(victim);
        char_to_room(victim, get_room_index(ROOM_VNUM_SCHOOL));
        act( AT_WHITE, "$n enters this world from within a column of blinding light!",
             victim, NULL, NULL, TO_ROOM );
        do_look(victim, "auto");
    }
    break;
    }
    
    return;
}

/*
 * Deposit some gold into the current area's economy		-Thoric
 */
void do_mp_deposit( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_STRING_LENGTH];
    int money;
    
    if ( !IS_NPC(ch) || (ch->desc))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    
    if ( charm_order_check( ch ) )
        return;
    
    one_argument(argument, arg);
    
    if ( arg[0] == '\0' )
    {
        progbug("Mpdeposit - bad syntax", ch );
        return;
    }
    money = atoi( arg );
    if ( money <= GET_MONEY(ch,DEFAULT_CURR) && ch->in_room )
    {
        GET_MONEY(ch,DEFAULT_CURR) -= money;
        boost_economy( ch->in_room->area, money, DEFAULT_CURR );
    }
}


/*
 * Withdraw some money from the current area's economy		-Thoric
 */
void do_mp_withdraw( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_STRING_LENGTH];
    int money;
    
    if ( !IS_NPC(ch) || (ch->desc))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    
    if ( charm_order_check( ch ) )
        return;
    
    one_argument(argument, arg);
    
    if ( arg[0] == '\0' )
    {
        progbug("Mpwithdraw - bad syntax", ch );
        return;
    }
    money = atoi( arg );
    if ( GET_MONEY(ch,DEFAULT_CURR) < 1000000000 &&
         money < 1000000000 && ch->in_room &&
         economy_has( ch->in_room->area, money, DEFAULT_CURR ) )
    {
        GET_MONEY(ch,DEFAULT_CURR) += money;
        lower_economy( ch->in_room->area, money, DEFAULT_CURR );
    }
}


void do_mppkset( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_STRING_LENGTH];
    
    if (!IS_NPC(ch) || (ch->desc))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    
    if ( charm_order_check( ch ) )
        return;
    
    argument = one_argument(argument, arg);
    
    if (argument[0] == '\0' || arg[0] == '\0')
    {
        progbug("Mppkset - bad syntax", ch );
        return;
    }
    
    if ( (victim = get_char_room( ch, arg ) ) == NULL )
    {
        progbug("Mppkset - no such player in room.", ch );
        return;
    }
    
    if (!str_cmp(argument, "yes") || !str_cmp(argument, "y"))
    {
        if(!IS_SET(victim->pcdata->flags, PCFLAG_DEADLY))
            SET_BIT(victim->pcdata->flags, PCFLAG_DEADLY);
    }
    else if (!str_cmp(argument, "no") || !str_cmp(argument, "n"))
    {
        if(IS_SET(victim->pcdata->flags, PCFLAG_DEADLY))
            REMOVE_BIT(victim->pcdata->flags, PCFLAG_DEADLY);
    }
    else
    {
        progbug("Mppkset - bad syntax", ch);
        return;
    }
}



/*
 * Inflict damage from a mudprogram
 *
 *  note: should be careful about using victim afterwards
 */
ch_ret simple_damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
    sh_int dameq;
    bool npcvict;
    OBJ_DATA *damobj;
    ch_ret retcode;
    
    
    retcode = rNONE;
    
    if ( !ch )
    {
        bug( "Damage: null ch!" );
        return rERROR;
    }
    if ( !victim )
    {
        progbug( "Damage: null victim!", ch );
        return rVICT_DIED;
    }
    
    if ( victim->position == POS_DEAD )
    {
        return rVICT_DIED;
    }
    
    npcvict = IS_NPC(victim);
    
    if ( dam )
    {
        if ( IS_FIRE(dt) )
            dam = ris_damage(victim, dam, RIS_FIRE);
        else
            if ( IS_COLD(dt) )
                dam = ris_damage(victim, dam, RIS_COLD);
            else
                if ( IS_ACID(dt) )
                    dam = ris_damage(victim, dam, RIS_ACID);
                else
                    if ( IS_ELECTRICITY(dt) )
                        dam = ris_damage(victim, dam, RIS_ELECTRICITY);
                    else
                        if ( IS_ENERGY(dt) )
                            dam = ris_damage(victim, dam, RIS_ENERGY);
                        else
                            if ( dt == gsn_poison )
                                dam = ris_damage(victim, dam, RIS_POISON);
                            else
                                if ( dt == (TYPE_HIT + 7) || dt == (TYPE_HIT + 8) )
                                    dam = ris_damage(victim, dam, RIS_BLUNT);
                                else
                                    if ( dt == (TYPE_HIT + 2) || dt == (TYPE_HIT + 11) )
                                        dam = ris_damage(victim, dam, RIS_PIERCE);
                                    else
                                        if ( dt == (TYPE_HIT + 1) || dt == (TYPE_HIT + 3) )
                                            dam = ris_damage(victim, dam, RIS_SLASH);
        if ( dam < 0 )
            dam = 0;
    }
    
    if ( victim != ch )
    {
        /*
         * Damage modifiers.
         */
        if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
            dam /= 2;
        
        if ( IS_AFFECTED(victim, AFF_PROTECT) && IS_EVIL(ch) )
            dam -= (int) (dam / 4);
        
        if ( dam < 0 )
            dam = 0;
        
        /* dam_message( ch, victim, dam, dt ); */
    }
    
    /*
     * Check for EQ damage.... ;)
     */
    
    if (dam > 10)
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
        }
    }
    
    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    GET_HIT(victim) -= dam;
    if ( !IS_NPC(victim)
         &&   GetMaxLevel(victim) >= LEVEL_IMMORTAL
         &&   GET_HIT(victim) < 1 )
        GET_HIT(victim) = GET_MAX_HIT(victim);
    
    if ( !npcvict
         &&   get_trust(victim) >= LEVEL_IMMORTAL
         &&	 get_trust(ch)	   >= LEVEL_IMMORTAL
         &&   GET_HIT(victim) < 1 )
        GET_HIT(victim) = GET_MAX_HIT(victim);
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
        act( AT_DEAD, "$n is DEAD!!", victim, 0, 0, TO_ROOM );
        act( AT_DEAD, "You have been KILLED!!\n\r", victim, 0, 0, TO_CHAR );
        break;
        
    default:
        if ( dam > GET_MAX_HIT(victim) / 4 )
            act( AT_HURT, "That really did HURT!", victim, 0, 0, TO_CHAR );
        if ( GET_HIT(victim) < GET_MAX_HIT(victim) / 4 )
            act( AT_DANGER, "You wish that your wounds would stop BLEEDING so much!",
                 victim, 0, 0, TO_CHAR );
        break;
    }
    
    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
        if ( !npcvict )
        {
            sprintf( log_buf, "%s killed by %s at %d",
                     victim->name,
                     (IS_NPC(ch) ? ch->short_descr : ch->name),
                     victim->in_room->vnum );
            log_string_plus( log_buf,LOG_MONITOR,LEVEL_IMMORTAL,SEV_CRIT);
            
            /*
             * Dying penalty:
             * 1/2 way back to previous level.
             */
            if ( victim->exp > 	exp_level(victim, GetMaxLevel(victim),
                                          GetMaxClass(victim)) )
                gain_exp( victim, (exp_level(victim, GetMaxLevel(victim),
                                             GetMaxClass(victim)) - victim->exp)/2 );
            
        }
        set_cur_char(victim);
        raw_kill( ch, victim, dt );
        victim = NULL;
        
        return rVICT_DIED;
    }
    
    if ( victim == ch )
        return rNONE;
    
    /*
     * Take care of link dead people.
     */
    if ( !npcvict && !victim->desc )
    {
        if ( number_range( 0, victim->wait ) == 0 )
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
        if ( ( IS_SET(victim->act, ACT_WIMPY) && number_bits( 1 ) == 0
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


void do_mppeace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    
    if (!IS_NPC(ch) || (ch->desc))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if ( charm_order_check( ch ) )
        return;

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

    return;
}

void do_mpdispel( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    AFFECT_DATA *paf;

    if (!IS_NPC(ch) || (ch->desc))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if ( charm_order_check( ch ) )
        return;

    if ( !*argument )
    {
        progbug( "Mpdispel whom?", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
        progbug( "Mpdispel - No such person", ch );
        return;
    }

    while ((paf = victim->first_affect))
        affect_remove(victim, paf);
}

void do_mpexp( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int gain;

    if (!IS_NPC(ch) || (ch->desc))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if ( charm_order_check( ch ) )
        return;
    
    argument = one_argument( argument, arg );
    
    if ( arg[0] == '\0' )
    {
        progbug( "Mpexp whom?", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        progbug( "Mpexp - No such person", ch );
        return;
    }

    gain = atoi(argument);

    if (gain == 0)
    {
        progbug( "Mpexp - gain is zero", ch );
        return;
    }

    gain_exp(victim, gain);
}

void do_mpsetvar( CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];

    if (!IS_NPC(ch) || (ch->desc))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if ( charm_order_check( ch ) )
        return;

    argument = one_argument(argument, arg);

    if ( arg[0] == '\0' )
    {
        progbug( "Mpsetvar whom?", ch );
        return;
    }

    if ( !argument || !*argument )
    {
        progbug( "Mpsetvar what variable?", ch );
        return;
    }

    sprintf(buf, "\"%s\" set %s", arg, argument);

    do_variables(ch, buf);
}

void do_mpdelvar( CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];

    if (!IS_NPC(ch) || (ch->desc))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if ( charm_order_check( ch ) )
        return;
    
    argument = one_argument(argument, arg);

    if ( arg[0] == '\0' )
    {
        progbug( "Mpdelvar whom?", ch );
        return;
    }

    if ( !argument || !*argument )
    {
        progbug( "Mpdelvar what variable?", ch );
        return;
    }

    sprintf(buf, "\"%s\" del %s", arg, argument);

    do_variables(ch, buf);
}

void do_mpmodvar( CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];

    if (!IS_NPC(ch) || (ch->desc))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if ( charm_order_check( ch ) )
        return;
    
    argument = one_argument(argument, arg);

    if ( arg[0] == '\0' )
    {
        progbug( "Mpmodvar whom?", ch );
        return;
    }

    if ( !argument || !*argument )
    {
        progbug( "Mpdelvar what variable and how?", ch );
        return;
    }

    sprintf(buf, "\"%s\" mod %s", arg, argument);

    do_variables(ch, buf);
}

void do_mpmtransform( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim, *newmob;
    OBJ_DATA *obj;
    sh_int loc;
    char arg[MAX_INPUT_LENGTH];

    if (!IS_NPC(ch) || (ch->desc))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if ( charm_order_check( ch ) )
        return;
    
    argument = one_argument(argument, arg);

    if ( arg[0] == '\0' )
    {
        progbug( "Mpmtransform whom?", ch );
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
        progbug( "Mpmtransform - No such person", ch );
        return;
    }

    if (!IS_NPC(victim) || IS_ACT_FLAG(victim, ACT_POLYMORPHED))
    {
	progbug( "Mpmtrnasform - Can't do this to a PC", ch );
	return;
    }

    if (!(newmob = create_mobile(atoi(argument))))
    {
	progbug( "Mpmtransform - Bad mob vnum", ch );
	return;
    }

    while ((obj = victim->first_carrying))
    {
	loc = obj->wear_loc;
	obj_from_char(obj);
	obj_to_char(obj, newmob);
	if (loc && obj->carried_by == newmob)
	    equip_char(newmob, obj, loc);
	if (obj->carried_by != newmob)
	    progbug( "Mpmtransform - obj not carried by new mob", ch );
    }

    newmob->leader = victim->leader;
    newmob->master = victim->master;

    if (IS_ACT_FLAG(victim, ACT_PET))
	SET_ACT_FLAG(victim, ACT_PET);

    if (IS_ACT_FLAG(victim, ACT_MOUNTED))
    {
	CHAR_DATA *mounter;
	for (mounter = first_char; mounter; mounter = mounter->next)
	    if (mounter->mount == victim)
	    {
		mounter->mount = newmob;
		SET_ACT_FLAG(newmob, ACT_MOUNTED);
		break;
	    }
	if (!IS_ACT_FLAG(newmob, ACT_MOUNTED))
	    progbug( "Mpmtransform - could not find mounter for new mob", ch );
    }

    victim->leader = NULL;
    victim->master = NULL;

    char_to_room(newmob, victim->in_room);

    extract_char(victim, TRUE);
}

void do_mpotransform( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char arg[MAX_INPUT_LENGTH];

    if (!IS_NPC(ch) || (ch->desc))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if ( charm_order_check( ch ) )
        return;
    
    argument = one_argument(argument, arg);

    if ( arg[0] == '\0' )
    {
        progbug( "Mpotransform what?", ch );
        return;
    }

    if ( ( obj = get_obj_world( ch, arg ) ) == NULL )
    {
        progbug( "Mpotransform - No such object", ch );
        return;
    }
    

}

void do_mphate( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if (!IS_NPC(ch) || (ch->desc))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if ( charm_order_check( ch ) )
        return;
    
    if ( !argument || argument[0] == '\0' )
    {
        progbug( "Mphate whom?", ch );
        return;
    }

    if ( !str_cmp(argument, "nobody") )
    {
        if (ch->hating)
            stop_hating(ch);
        return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
        progbug( "Mphate - No such person", ch );
        return;
    }

    if (ch->hating)
        stop_hating(ch);
    start_hating(ch, victim);
}

void do_mpfear( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if (!IS_NPC(ch) || (ch->desc))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if ( charm_order_check( ch ) )
        return;
    
    if ( !argument || argument[0] == '\0' )
    {
        progbug( "Mpfear whom?", ch );
        return;
    }

    if ( !str_cmp(argument, "nobody") )
    {
        if (ch->fearing)
            stop_fearing(ch);
        return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
        progbug( "Mpfear - No such person", ch );
        return;
    }

    if (ch->fearing)
        stop_fearing(ch);
    start_fearing(ch, victim);
}

void do_mphunt( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    if (!IS_NPC(ch) || (ch->desc))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if ( charm_order_check( ch ) )
        return;
    
    if ( !argument || argument[0] == '\0' )
    {
        progbug( "Mphunt whom?", ch );
        return;
    }

    if ( !str_cmp(argument, "nobody") )
    {
        if (ch->hunting)
            stop_hunting(ch);
        return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
        progbug( "Mphunt - No such person", ch );
        return;
    }

    if (ch->hunting)
        stop_hunting(ch);
    start_hunting(ch, victim);
}

