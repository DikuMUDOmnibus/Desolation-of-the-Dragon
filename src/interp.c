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
 *			 Command interpretation module			    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: interp.c,v 1.24 2004/04/06 22:00:10 dotd Exp $";*/

#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/*
 * Externals
 */
void refresh_page( CHAR_DATA *ch );
void subtract_times( struct timeval *etime, struct timeval *start_time );

ALIAS_DATA	*find_alias	args( ( CHAR_DATA *ch, char *argument ) );


bool	check_social	args( ( CHAR_DATA *ch, char *command,
				char *argument ) );
bool	check_alias	args( ( CHAR_DATA *ch, char *command,
				char *argument ) );
bool	check_special	args( ( CHAR_DATA *ch, CMDTYPE *cmd,
				char *command, char *argument ) );
bool	check_progs	args( ( CHAR_DATA *ch, CMDTYPE *cmd,
				char *command, char *argument ) );


/*
 * Log-all switch.
 */
bool				fLogAll		= FALSE;


CMDTYPE	   *command_hash[126];	/* hash table for cmd_table */
SOCIALTYPE *social_index[27];   /* hash table for socials   */

/*
 * Character not in position for command?
 */
bool check_pos( CHAR_DATA *ch, sh_int position )
{
    if ( ch->position < position )
    {
	switch( ch->position )
	{
	case POS_DEAD:
	    send_to_char( "A little difficult to do when you are DEAD...\n\r", ch );
	    break;

	case POS_MORTAL:
	case POS_INCAP:
	    send_to_char( "You are hurt far too bad for that.\n\r", ch );
	    break;

	case POS_STUNNED:
	    send_to_char( "You are too stunned to do that.\n\r", ch );
	    break;

	case POS_SLEEPING:
	    send_to_char( "In your dreams, or what?\n\r", ch );
	    break;

	case POS_RESTING:
	    send_to_char( "Nah... You feel too relaxed...\n\r", ch);
	    break;

	case POS_SITTING:
	case POS_MEDITATING:
	    send_to_char( "You can't do that sitting down.\n\r", ch);
	    break;

	case POS_FIGHTING:
	    send_to_char( "No way!  You are still fighting!\n\r", ch);
	    break;

	}
	return FALSE;
    }
    return TRUE;
}

extern char lastplayercmd[MAX_INPUT_LENGTH*2];

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA *ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
    char logname[MAX_INPUT_LENGTH];
#ifdef IBUILD
    char newcommand[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
#endif
    TIMER *timer = NULL;
    CMDTYPE *cmd = NULL;
    int trust;
    int loglvl;
    bool found;
    struct timeval time_used;
    long tmptime;


    if ( !ch )
    {
	bug( "interpret: null ch!" );
	return;
    }

    found = FALSE;
    if ( ch->substate == SUB_REPEATCMD )
    {
	DO_FUN *fun;

	if ( (fun=ch->last_cmd) == NULL )
	{
	    ch->substate = SUB_NONE;
	    bug( "interpret: SUB_REPEATCMD with NULL last_cmd" );
	    return;
	}
	else
	{
	    int x;

	    /*
	     * yes... we lose out on the hashing speediness here...
	     * but the only REPEATCMDS are wizcommands (currently)
	     */
	    for ( x = 0; x < 126; x++ )
	    {
		for ( cmd = command_hash[x]; cmd; cmd = cmd->next )
		   if ( cmd->do_fun == fun )
		   {
			found = TRUE;
			break;
		   }
		if ( found )
		   break;
	    }
	    if ( !found )
	    {
		cmd = NULL;
		bug( "interpret: SUB_REPEATCMD: last_cmd invalid" );
		return;
	    }
	    sprintf( logline, "(%s) %s", cmd->name, argument );
	}
    }

    if ( !cmd )
    {
	/* Changed the order of these ifchecks to prevent crashing. */
	if ( !argument || !strcmp(argument,"") )
	{
	    bug( "interpret: null argument!" );
	    return;
	}

	/*
	 * Strip leading spaces.
	 */
	while ( isspace(*argument) )
	    argument++;
	if ( argument[0] == '\0' )
	    return;

	timer = get_timerptr( ch, TIMER_DO_FUN );

	/* REMOVE_BIT( ch->affected_by, AFF_HIDE ); */

	/*
	 * Implement freeze command.
	 */
	if ( !IS_NPC(ch) && IS_PLR_FLAG(ch, PLR_FREEZE) )
	{
	    send_to_char( "You're totally frozen!\n\r", ch );
	    return;
	}

	/*
	 * Grab the command word.
	 * Special parsing so ' can be a command,
	 *   also no spaces needed after punctuation.
	 */
	strcpy( logline, argument );
	if ( !isalpha(argument[0]) && !isdigit(argument[0]) )
	{
	    command[0] = argument[0];
	    command[1] = '\0';
	    argument++;
	    while ( isspace(*argument) )
		argument++;
	}
	else
	    argument = one_argument( argument, command );

	/*
	 * Look for command in command table.
	 * Check for council powers and/or bestowments
	 */
	trust = get_trust( ch );
	for ( cmd = command_hash[LOWER(command[0])%126]; cmd; cmd = cmd->next )
	    if ( !str_prefix( command, cmd->name )
	    &&   (cmd->level <= trust
	    ||  (!IS_NPC(ch) && ch->pcdata->council
	    &&    is_name( cmd->name, ch->pcdata->council->powers )
	    &&    cmd->level <= (trust+MAX_CPD))
	    ||  (!IS_NPC(ch) && ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0'
	    &&    is_name( cmd->name, ch->pcdata->bestowments )
	    &&    cmd->level <= (trust+10)) ) )
	    {
		found = TRUE;
		break;
	    }

        if (!found)
            cmd = NULL;

        /*
	 * Turn off afk bit when any command performed.
	 */
	if ( !IS_NPC(ch) && IS_PLR_FLAG(ch, PLR_AFK) && (str_cmp(command, "AFK")))
	{
	    REMOVE_BIT( ch->act, PLR_AFK );
     	    act( AT_GREY, "$n is no longer afk.", ch, NULL, NULL, TO_ROOM );
     	    act( AT_GREY, "You return to keyboard.", ch, NULL, NULL, TO_CHAR );
	}
    }

    /*
     * Log and snoop.
     */
    sprintf( lastplayercmd, "** %s: %s", ch->name, logline );

    if ( found && cmd->log == LOG_NEVER )
	strcpy( logline, "XXXXXXXX XXXXXXXX XXXXXXXX" );

    loglvl = found ? cmd->log : LOG_NORMAL;

    if ( ( !IS_NPC(ch) && (GetMaxLevel(ch) == 1 || IS_PLR_FLAG(ch, PLR_LOG)) )
    ||   fLogAll
    ||	 loglvl == LOG_BUILD
    ||   loglvl == LOG_HIGH
    ||   loglvl == LOG_PC
    ||   loglvl == LOG_ALWAYS )
    {
        /* Added by Narn to show who is switched into a mob that executes
           a logged command.  Check for descriptor in case force is used. */
        if ( ch->desc && ch->desc->original )
          sprintf( log_buf, "Log %s (%s): %s", ch->name,
                   ch->desc->original->name, logline );
        else
          sprintf( log_buf, "Log %s: %s", ch->name, logline );

	/*
	 * Make it so a 'log all' will send most output to the log
	 * file only, and not spam the log channel to death	-Thoric
	 */
	if ( fLogAll && loglvl == LOG_NORMAL
	&&  (IS_NPC(ch) || !IS_SET(ch->act, PLR_LOG)) )
	    loglvl = LOG_ALL;
	else if (loglvl == LOG_NORMAL && !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG))
	    loglvl = LOG_PC;

	/* This is handled in get_trust already */
/*	if ( ch->desc && ch->desc->original )
	  log_string_plus( log_buf, loglvl,
          ch->desc->original->level );
          else*/

        if (loglvl != LOG_NEVER)
            log_string_plus( log_buf, loglvl, UMIN(get_trust(ch)+1, MAX_LEVEL), IS_IMMORTAL(ch)?SEV_INFO+1:SEV_INFO );
    }

    if ( ch->desc && ch->desc->snoop_by )
    {
  	sprintf( logname, "%s", ch->name);
	write_to_buffer( ch->desc->snoop_by, logname, 0 );
	write_to_buffer( ch->desc->snoop_by, "% ",    2 );
	write_to_buffer( ch->desc->snoop_by, logline, 0 );
	write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
    }

    /* BUILD INTERFACE (start)*/
#ifdef IBUILD
    if( ch->inter_type == OBJ_TYPE )
    {
	MENU_DATA *m_data = NULL;
	int i;
        switch (ch->inter_page)
	{
          case OBJ_PAGE_A:  m_data=obj_page_a_data;
			    break;
          case OBJ_PAGE_B:  m_data=obj_page_b_data;
			    break;
          case OBJ_PAGE_C:  m_data=obj_page_c_data;
			    break;
          case OBJ_PAGE_D:  m_data=obj_page_d_data;
			    break;
          case OBJ_PAGE_E:  m_data=obj_page_e_data;
			    break;
          case OBJ_HELP_PAGE:  m_data=obj_help_page_data;
			    break;
        }
	if( m_data )
	{
	  for(i=0;m_data[i].ptrType!= (int) NULL;i++)
	  {
             /*IF 1st char matches && 2nd CHAR MATCHES...*/
	     if(! strcmp(m_data[i].sectionNum,command))
	     {
	       if(! strncmp(m_data[i].charChoice,argument,1) )   /* just check the 1st char */
	       {
	          /* ...then MAKE NEW_COMMAND, and */
	          switch (m_data[i].cmdArgs)
	          {
                    case 1:
		       sprintf(newcommand,m_data[i].cmdString,ch->inter_editing);
		       break;
                    case 2:
		       argument = one_argument( argument, arg2 );
		       sprintf(newcommand,m_data[i].cmdString,ch->inter_editing, argument);
		       break;
                    case 0:
	            default:
		       sprintf(newcommand,m_data[i].cmdString);
	          }
		  send_to_char( newcommand, ch );
		  send_to_char( "\n\r", ch );
	          /* ... interpret NEW_COMMAND */
	          interpret( ch, newcommand );
		  refresh_page( ch );
	          return;
	       }
             }
          }
          if(!strcmp("+",command) && get_obj_world(ch, argument))
          {
             sprintf(newcommand,"omenu %s %c",argument,ch->inter_page);
		  send_to_char( newcommand, ch );
		  send_to_char( "\n\r", ch );
	     interpret( ch, newcommand );
	     refresh_page( ch );
	     return;
          }
        }
    }
    if( ch->inter_type == MOB_TYPE )
    {
	MENU_DATA *m_data = NULL;
	int i;
        switch (ch->inter_page)
	{
          case MOB_PAGE_A:  m_data=mob_page_a_data;
			    break;
          case MOB_PAGE_B:  m_data=mob_page_b_data;
			    break;
          case MOB_PAGE_C:  m_data=mob_page_c_data;
			    break;
          case MOB_PAGE_D:  m_data=mob_page_d_data;
			    break;
          case MOB_PAGE_E:  m_data=mob_page_e_data;
			    break;
          case MOB_PAGE_F:  m_data=mob_page_f_data;   /* 7/19 */
			    break;
          case MOB_HELP_PAGE:  m_data=mob_help_page_data;
			    break;
        }
	if( m_data )
	{
	  for(i=0;m_data[i].ptrType!=(int)NULL;i++)
	  {
             /*IF 1st char matches && 2nd CHAR MATCHES...*/
	     if(! strcmp(m_data[i].sectionNum,command))
	     {
	       if(! strncmp(m_data[i].charChoice,argument,1) )   /* just check the 1st char */
	       {
	          /* ...then MAKE NEW_COMMAND, and */
	          switch (m_data[i].cmdArgs)
	          {
                    case 1:
		       sprintf(newcommand,m_data[i].cmdString,ch->inter_editing);
		       break;
                    case 2:
		       argument = one_argument( argument, arg2 );
		       sprintf(newcommand,m_data[i].cmdString,ch->inter_editing, argument);
		       break;
                    case 0:
	            default:
		       sprintf(newcommand,m_data[i].cmdString);
	          }
		  send_to_char( newcommand, ch );
		  send_to_char( "\n\r", ch );
	          /* ... interpret NEW_COMMAND */
	          interpret( ch, newcommand );
		  refresh_page( ch );
	          return;
	       }
             }
          }
          if(!strcmp("+",command) && get_char_world(ch, argument))
          {
             sprintf(newcommand,"mmenu %s %c",argument,ch->inter_page);
		  send_to_char( newcommand, ch );
		  send_to_char( "\n\r", ch );
	     interpret( ch, newcommand );
	     refresh_page( ch );
	     return;
          }
        }
    }
    if( ch->inter_type == ROOM_TYPE )
    {
	MENU_DATA *m_data = NULL;
	int i;
        switch (ch->inter_page)
	{
          case ROOM_PAGE_A:  m_data=room_page_a_data;
			    break;
          case ROOM_PAGE_B:  m_data=room_page_b_data;
			    break;
          case ROOM_PAGE_C:  m_data=room_page_c_data;
			    break;
          case ROOM_HELP_PAGE:  m_data=room_help_page_data;
			    break;
        }
	if( m_data )
	{
	  for(i=0;m_data[i].ptrType!=(int)NULL;i++)
	  {
             /*IF 1st char matches && 2nd CHAR MATCHES...*/
	     if(! strcmp(m_data[i].sectionNum,command))
	     {
	       if(! strncmp(m_data[i].charChoice,argument,1) )   /* just check the 1st char */
	       {
	          /* ...then MAKE NEW_COMMAND, and */
	          switch (m_data[i].cmdArgs)
	          {
                    case 1:
		       argument = one_argument( argument, arg2 ); /* new */
		       argument = one_argument( argument, arg2 ); /* new */
		       sprintf(newcommand,m_data[i].cmdString,
					  argument);  /* different than mobs */
		       break;                         /* on purpose */
                    case 2:
		       argument = one_argument( argument, arg2 );
		       sprintf(newcommand,m_data[i].cmdString,
					  ch->inter_editing,
					  argument);
		       break;
                    case 0:
	            default:
		       sprintf(newcommand,m_data[i].cmdString);
	          }
		  send_to_char( newcommand, ch );
		  send_to_char( "\n\r", ch );
	          /* ... interpret NEW_COMMAND */
	          interpret( ch, newcommand );
		  refresh_page( ch );
	          return;
	       }
             }
          }
        }
    }
    /* BUILD INTERFACE ( end )*/
#endif

    if ( timer )
    {
	int tempsub;

	tempsub = ch->substate;
	ch->substate = SUB_TIMER_DO_ABORT;
	(timer->do_fun)(ch,"");
	if ( char_died(ch) )
	  return;
	if ( ch->substate != SUB_TIMER_CANT_ABORT )
	{
	  ch->substate = tempsub;
	  extract_timer( ch, timer );
	}
	else
	{
	  ch->substate = tempsub;
	  return;
	}
    }

    if ( check_special( ch, cmd, command, argument ) )
        return;

    if ( check_progs( ch, cmd, command, argument ) )
        return;
    /*
     * Look for command in skill and socials table.
     */
    if ( !found )
    {
	if ( !check_skill( ch, command, argument )
	&&   !check_alias( ch, command, argument )
	&&   !check_social( ch, command, argument )
#ifdef I3
        &&   !I3_command_hook( ch, command, argument )
#endif
#ifdef IMC
        &&   !imc_command_hook( ch, command, argument )
#endif
           )
	{
	    EXIT_DATA *pexit;

	    /* check for an auto-matic exit command */
	    if ( (pexit = find_door( ch, command, TRUE )) != NULL
	    &&   IS_SET( pexit->exit_info, EX_xAUTO ))
	    {
		if ( IS_SET(pexit->exit_info, EX_CLOSED)
		&& (!IS_AFFECTED(ch, AFF_PASS_DOOR)
		||   IS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
		{
		  if ( !IS_SET( pexit->exit_info, EX_SECRET ) )
		    act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
		  else
		    send_to_char( "You cannot do that here.\n\r", ch );
		  return;
		}
		move_char( ch, pexit, 0 );
		return;
	    }
	    send_to_char( "Pardon?\n\r", ch );
	}
	return;
    }

    /*
     * Character not in position for command?
     */
    if ( !check_pos( ch, cmd->position ) )
	return;

    /* Berserk check for flee.. maybe add drunk to this?.. but too much
       hardcoding is annoying.. -- Altrag */
    if ( !str_cmp(cmd->name, "flee") &&
          IS_AFFECTED(ch, AFF_BERSERK) )
    {
	send_to_char( "You aren't thinking very clearly..\n\r", ch);
	return;
    }

    /*
     * Dispatch the command.
     */
    ch->prev_cmd = ch->last_cmd;    /* haus, for automapping */
    ch->last_cmd = cmd->do_fun;
    set_char_color(AT_PLAIN,ch);
    set_pager_color(AT_PLAIN,ch);
    start_timer(&time_used);
    (*cmd->do_fun) ( ch, argument );
    end_timer(&time_used);
    /*
     * Update the record of how many times this command has been used (haus)
     */
    update_userec(&time_used, &cmd->userec);
    tmptime = UMIN(time_used.tv_sec,19) * 1000000 + time_used.tv_usec;

    /* laggy command notice: command took longer than 1.5 seconds */
    if ( tmptime > 1500000 )
    {
        sprintf(log_buf, "[*****] LAG: %s: %s %s (R:%d S:%ld.%06ld)", ch->name,
                cmd->name, (cmd->log == LOG_NEVER ? "XXX" : argument),
		ch->in_room ? ch->in_room->vnum : 0,
		(long int)time_used.tv_sec, (long int)time_used.tv_usec );
	log_string_plus(log_buf, LOG_NORMAL, get_trust(ch), SEV_CRIT);
    }

    tail_chain( );
}

CMDTYPE *find_command( char *command )
{
    CMDTYPE *cmd;
    int hash;

    hash = LOWER(command[0]) % 126;

    for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
	if ( !str_prefix( command, cmd->name ) )
	    return cmd;

    return NULL;
}

CMDTYPE *find_command_exact( char *command )
{
    CMDTYPE *cmd;
    int hash;

    hash = LOWER(command[0]) % 126;

    for ( cmd = command_hash[hash]; cmd; cmd = cmd->next )
	if ( !str_cmp( command, cmd->name ) )
	    return cmd;

    return NULL;
}

SOCIALTYPE *find_social( char *command )
{
    SOCIALTYPE *social;
    int hash;

    if ( command[0] < 'a' || command[0] > 'z' )
	hash = 0;
    else
	hash = (command[0] - 'a') + 1;

    for ( social = social_index[hash]; social; social = social->next )
	if ( !str_prefix( command, social->name ) )
	    return social;

    return NULL;
}

SOCIALTYPE *find_social_exact( char *command )
{
    SOCIALTYPE *social;
    int hash;

    if ( command[0] < 'a' || command[0] > 'z' )
	hash = 0;
    else
	hash = (command[0] - 'a') + 1;

    for ( social = social_index[hash]; social; social = social->next )
	if ( !str_cmp( command, social->name ) )
	    return social;

    return NULL;
}

bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    SOCIALTYPE *social;

    if ( (social=find_social(command)) == NULL )
	return FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_NO_EMOTE) )
    {
	send_to_char( "You are anti-social!\n\r", ch );
	return TRUE;
    }

    if ( !check_pos( ch, social->position ) )
        return TRUE;

    one_argument( argument, arg );
    victim = NULL;
    if ( arg[0] == '\0' )
    {
	act( AT_SOCIAL, social->others_no_arg, ch, NULL, victim, TO_ROOM    );
        act( AT_SOCIAL, social->char_no_arg,   ch, NULL, victim, TO_CHAR    );
        return TRUE;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        act( AT_SOCIAL, social->not_found,     ch, NULL, NULL,   TO_CHAR    );
        return TRUE;
    }

    if ( victim == ch )
    {
	act( AT_SOCIAL, social->others_auto,   ch, NULL, victim, TO_ROOM    );
        act( AT_SOCIAL, social->char_auto,     ch, NULL, victim, TO_CHAR    );
        return TRUE;
    }

    act( AT_SOCIAL, social->others_found,  ch, NULL, victim, TO_NOTVICT );
    act( AT_SOCIAL, social->char_found,    ch, NULL, victim, TO_CHAR    );
    act( AT_SOCIAL, social->vict_found,    ch, NULL, victim, TO_VICT    );

    /* handle social->mob_response  -Garil 02/03/2002 */
    if ( social->mob_response &&
         IS_NPC(victim) &&
         IS_AWAKE(victim) &&
         !IS_AFFECTED(victim, AFF_CHARM) &&
         !HAS_PROG( victim->pIndexData, ACT_PROG ) &&
         can_see(victim, ch) )
    {
        sprintf(buf, "%s %s", social->mob_response, spacetodash(ch->name));
        interpret(victim, buf);
    }

    /*
    if ( !IS_NPC(ch) && IS_NPC(victim)
         &&   !IS_AFFECTED(victim, AFF_CHARM)
         &&   IS_AWAKE(victim)
         &&   !IS_SET( victim->pIndexData->progtypes, ACT_PROG ) )
    {
        switch ( number_bits( 4 ) )
        {
        case 0:
            if ( !IS_SET(ch->in_room->room_flags, ROOM_SAFE )
                 &&    IS_EVIL(ch) )
                multi_hit( victim, ch, TYPE_UNDEFINED );
            else
                if ( IS_NEUTRAL(ch) )
                {
                    act( AT_ACTION, "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
                    act( AT_ACTION, "You slap $N.",  victim, NULL, ch, TO_CHAR    );
                    act( AT_ACTION, "$n slaps you.", victim, NULL, ch, TO_VICT    );
                }
                else
                {
                    act( AT_ACTION, "$n acts like $N doesn't even exist.",  victim, NULL, ch, TO_NOTVICT );
                    act( AT_ACTION, "You just ignore $N.",  victim, NULL, ch, TO_CHAR    );
                    act( AT_ACTION, "$n appears to be ignoring you.", victim, NULL, ch, TO_VICT    );
                }
            break;

        case 1: case 2: case 3: case 4:
        case 5: case 6: case 7: case 8:
            act( AT_SOCIAL, social->others_found,
                 victim, NULL, ch, TO_NOTVICT );
            act( AT_SOCIAL, social->char_found,
                 victim, NULL, ch, TO_CHAR    );
            act( AT_SOCIAL, social->vict_found,
                 victim, NULL, ch, TO_VICT    );
            break;

        case 9: case 10: case 11: case 12:
            act( AT_ACTION, "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
            act( AT_ACTION, "You slap $N.",  victim, NULL, ch, TO_CHAR    );
            act( AT_ACTION, "$n slaps you.", victim, NULL, ch, TO_VICT    );
            break;
        }
    }
    */

    return TRUE;
}

bool check_alias( CHAR_DATA *ch, char *command, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ALIAS_DATA *alias;

    if ( (alias=find_alias(ch,command)) == NULL )
	return FALSE;

    if (!alias->cmd || !*alias->cmd)
	return FALSE;

    sprintf(arg, "%s", alias->cmd);

    if (ch->cmd_recurse==-1 || ++ch->cmd_recurse>50)
    {
	if (ch->cmd_recurse!=-1)
	{
	    send_to_char("Unable to further process command, recurses too much.\n\r", ch);
	    ch->cmd_recurse=-1;
	}
	return FALSE;
    }

/*
   {
    char buf[MAX_INPUT_LENGTH];

    sprintf(buf, "%s", alias->name);

    if ( (alias=find_alias(ch,arg)) != NULL )
    {
	sprintf(arg, "Your alias '%s' calls another alias and cannot be executed.\n\r", buf);
	send_to_char(arg, ch);
	return TRUE;
    }
   }
*/
    if (argument && *argument!='\0')
    {
	strcat(arg, " ");
	strcat(arg, argument);
    }

    interpret(ch, arg);
    return TRUE;
}

bool check_special ( CHAR_DATA *ch, CMDTYPE *cmd, char *command, char *argument)
{
    CHAR_DATA *mob;
    OBJ_DATA *obj;
    char buf[MAX_INPUT_LENGTH];

    if (sysdata.specials_enabled==FALSE)
	return FALSE;

    sprintf(buf, "%s %s", command, argument);

    for (obj=ch->first_carrying;obj;obj=obj->next_content)
	if (!obj_extracted(obj) && obj->spec_fun)
	    if ((*obj->spec_fun)(obj,cmd,
		cmd?argument:buf,ch,cmd?SFT_COMMAND:SFT_ARGUMENT))
		return TRUE;

    if (!ch || !ch->in_room || char_died(ch))
	return FALSE;

    for (obj=ch->in_room->first_content;obj;obj=obj->next_content)
	if (!obj_extracted(obj) && obj->spec_fun)
	    if ((*obj->spec_fun)(obj,cmd,
		cmd?argument:buf,ch,cmd?SFT_COMMAND:SFT_ARGUMENT))
		return TRUE;

    if (!ch || !ch->in_room || char_died(ch))
	return FALSE;

    for (mob=ch->in_room->first_person;mob;mob=mob->next_in_room)
	if (!char_died(mob) && mob->spec_fun)
	    if ((*mob->spec_fun)(mob,cmd,
		cmd?argument:buf,ch,cmd?SFT_COMMAND:SFT_ARGUMENT))
		return TRUE;

    if (!ch || !ch->in_room || char_died(ch))
	return FALSE;

    if (ch->in_room->spec_fun)
	if ((*ch->in_room->spec_fun)(ch->in_room,cmd,
	    cmd?argument:buf,ch,cmd?SFT_COMMAND:SFT_ARGUMENT))
	    return TRUE;

    return FALSE;
}

bool check_progs ( CHAR_DATA *ch, CMDTYPE *cmd, char *command, char *argument)
{
    CHAR_DATA *mob;
    char buf[MAX_INPUT_LENGTH];

    sprintf(buf, "%s %s", command, argument);

    for (mob=ch->in_room->first_person;mob;mob=mob->next_in_room)
        if (!char_died(mob))
            if (mprog_command_trigger(buf, mob, ch))
		return TRUE;

    if (!ch || !ch->in_room || char_died(ch))
        return FALSE;

    if (rprog_command_trigger(buf, ch))
        return TRUE;

    return FALSE;
}

/*
 * Return true if an argument is completely numeric.
 */
bool is_number( char *arg )
{
    if ( *arg == '\0' )
	return FALSE;

    for ( ; *arg != '\0'; arg++ )
    {
	if ( !isdigit(*arg) && *arg != '.')
	    return FALSE;
    }

    return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int number;

    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	if ( *pdot == '.' )
	{
	    *pdot = '\0';
	    number = atoi( argument );
	    *pdot = '.';
	    strcpy( arg, pdot+1 );
	    return number;
	}
    }

    strcpy( arg, argument );
    return 1;
}



/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;
    sh_int count;

    count = 0;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' || ++count >= 255 )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.  Delimiters = { ' ', '-' }
 */
char *one_argument2( char *argument, char *arg_first )
{
    char cEnd;
    sh_int count;

    count = 0;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' || ++count >= 255 )
    {
	if ( *argument == cEnd || *argument == '-' )
	{
	    argument++;
	    break;
	}
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

/*
 * Pick off one argument from a string and return the rest.
 * Delimiters = { x }
 */
char *one_argumentx( char *argument, char *arg_first, char cEnd )
{
    sh_int count;

    count = 0;

    while ( isspace(*argument) )
	argument++;

    while ( *argument != '\0' || ++count >= 255 )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = LOWER(*argument);
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

char *spacetodash( char *argument )
{
    static char retbuf[64];
    unsigned int x;

    strncpy(retbuf, argument, 63);

    for (x=0;x<strlen(retbuf);x++)
        if (retbuf[x]==' ')
            retbuf[x]='-';

    return retbuf;
}

void do_timecmd( CHAR_DATA *ch, char *argument )
{
    struct timeval start_time;
    struct timeval etime;
    static bool timing;
    extern CHAR_DATA *timechar;
    char arg[MAX_INPUT_LENGTH];

    send_to_char("Timing\n\r",ch);
    if ( timing )
        return;
    one_argument(argument, arg);
    if ( !*arg )
    {
        send_to_char( "No command to time.\n\r", ch );
        return;
    }
    if ( !str_cmp(arg, "update") )
    {
        if ( timechar )
            send_to_char( "Another person is already timing updates.\n\r", ch );
        else
        {
            timechar = ch;
            send_to_char( "Setting up to record next update loop.\n\r", ch );
        }
        return;
    }
    set_char_color(AT_PLAIN, ch);
    send_to_char( "Starting timer.\n\r", ch );
    timing = TRUE;
    gettimeofday(&start_time, NULL);
    interpret(ch, argument);
    gettimeofday(&etime, NULL);
    timing = FALSE;
    set_char_color(AT_PLAIN, ch);
    send_to_char( "Timing complete.\n\r", ch );
    subtract_times(&etime, &start_time);
    ch_printf( ch, "Timing took %ld.%06ld seconds.\n\r",
               etime.tv_sec, etime.tv_usec );
    return;
}

void start_timer(struct timeval *start_time)
{
    if ( !start_time )
    {
        bug( "Start_timer: NULL start_time." );
        return;
    }
    gettimeofday(start_time, NULL);
    return;
}

time_t end_timer(struct timeval *start_time)
{
    struct timeval etime;

    /* Mark etime before checking start_time, so that we get a better reading.. */
    gettimeofday(&etime, NULL);
    if ( !start_time || (!start_time->tv_sec && !start_time->tv_usec) )
    {
        bug( "End_timer: bad start_time." );
        return 0;
    }
    subtract_times(&etime, start_time);
    /* start_time becomes time used */
    *start_time = etime;
    return (etime.tv_sec*1000000)+etime.tv_usec;
}

void send_timer(struct timerset *vtime, CHAR_DATA *ch)
{
    struct timeval ntime;
    int carry;

    if ( vtime->num_uses == 0 )
        return;
    ntime.tv_sec  = vtime->total_time.tv_sec / vtime->num_uses;
    carry = (vtime->total_time.tv_sec % vtime->num_uses) * 1000000;
    ntime.tv_usec = (vtime->total_time.tv_usec + carry) / vtime->num_uses;
    ch_printf(ch, "Has been used %d times this boot.\n\r", vtime->num_uses);
    ch_printf(ch, "Time (in secs): min %ld.%06ld; avg: %ld.%06ld; max %ld.%06ld"
              "\n\r", vtime->min_time.tv_sec, vtime->min_time.tv_usec, ntime.tv_sec,
              ntime.tv_usec, vtime->max_time.tv_sec, vtime->max_time.tv_usec);
    return;
}

void update_userec(struct timeval *time_used, struct timerset *userec)
{
    userec->num_uses++;

    if ( !timerisset(&userec->min_time)
         ||    timercmp(time_used, &userec->min_time, <) )
    {
        userec->min_time.tv_sec  = time_used->tv_sec;
        userec->min_time.tv_usec = time_used->tv_usec;
    }
    if ( !timerisset(&userec->max_time)
         ||    timercmp(time_used, &userec->max_time, >) )
    {
        userec->max_time.tv_sec  = time_used->tv_sec;
        userec->max_time.tv_usec = time_used->tv_usec;
    }
    userec->total_time.tv_sec  += time_used->tv_sec;
    userec->total_time.tv_usec += time_used->tv_usec;
    while ( userec->total_time.tv_usec >= 1000000 )
    {
        userec->total_time.tv_sec++;
        userec->total_time.tv_usec -= 1000000;
    }
    return;
}
