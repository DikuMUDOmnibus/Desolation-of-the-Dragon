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
 *                           Deity handling module                          *
 ****************************************************************************/

/*Put together by Rennard for Realms of Despair.  Brap on...*/

/*static char rcsid[] = "$Id: deity.c,v 1.18 2004/04/06 22:00:09 dotd Exp $";*/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "gsn.h"

DECLARE_DO_FUN(do_look);

DEITY_DATA * first_deity;
DEITY_DATA * last_deity;

/* local routines */

void	fread_deity	 args( ( DEITY_DATA *deity, FILE *fp ) );
bool	load_deity_file	 args( ( const char *deityfile ) );
void	write_deity_list args( ( void ) );
int	get_risflag	 args( ( char *flag ) );
int	get_npc_race	 args( ( char *type ) );

/* Get pointer to deity structure from deity name */

DEITY_DATA *get_deity( char *name )
{
     DEITY_DATA *deity;
     for ( deity = first_deity; deity; deity = deity->next )
	if ( !str_cmp( name, deity->name ) )
	  return deity;
	return NULL;
}

void write_deity_list( )
{
     DEITY_DATA *tdeity;
     FILE *fpout;
     char filename[256];

     sprintf( filename, "%s%s", DEITY_DIR, DEITY_LIST );
     fpout = fopen( filename, "w" );
     if ( !fpout )
	bug( "FATAL: cannot open deity.lst for writing!\n\r" );
     else
     {
       for ( tdeity = first_deity; tdeity; tdeity = tdeity->next )
	  fprintf( fpout, "%s\n", tdeity->filename );
       fprintf( fpout, "$\n" );
       fclose( fpout );
     }
}

/* Save a deity's data to its data file */

void save_deity( DEITY_DATA *deity )
{
     FILE *fp;
     char filename[256];
     char buf[MAX_STRING_LENGTH];

     if ( !deity )
     {
	bug( "save_deity: null deity pointer!" );
	return;
     }

     if ( !deity->filename || deity->filename[0] == '\0' )
     {
	sprintf( buf, "save_deity: %s has no filename", deity->name );
	bug( buf, 0 );
	return;
     }

     sprintf( filename, "%s%s", DEITY_DIR, deity->filename );

     if ( ( fp = fopen( filename, "w" ) ) == NULL )
     {
	bug( "save_deity: fopen" );
	perror( filename );
     }
     else
     {
	fprintf( fp, "#DEITY\n" );
	fprintf( fp, "Filename		%s~\n",	deity->filename );
	fprintf( fp, "Name		%s~\n",	deity->name	);
	fprintf( fp, "Description	%s~\n",	deity->description );
	fprintf( fp, "Alignment		%d\n",	deity->alignment );
	fprintf( fp, "Worshippers	%d\n",	deity->worshippers );
	fprintf( fp, "Flee		%d\n",	deity->flee );
	fprintf( fp, "Flee_npcrace	%d\n",	deity->flee_npcrace );
	fprintf( fp, "Flee_npcfoe	%d\n",	deity->flee_npcfoe );
	fprintf( fp, "Kill		%d\n",	deity->kill );
	fprintf( fp, "Kill_npcrace	%d\n",	deity->kill_npcrace );
	fprintf( fp, "Kill_npcfoe	%d\n",	deity->kill_npcfoe );
	fprintf( fp, "Kill_magic	%d\n",	deity->kill_magic );
	fprintf( fp, "Sac		%d\n",	deity->sac );
	fprintf( fp, "Bury_corpse	%d\n",	deity->bury_corpse );
	fprintf( fp, "Aid_spell		%d\n",	deity->aid_spell );
	fprintf( fp, "Aid		%d\n",	deity->aid );
	fprintf( fp, "Steal		%d\n",	deity->steal );
	fprintf( fp, "Backstab		%d\n",	deity->backstab );
	fprintf( fp, "Die		%d\n",	deity->die );
	fprintf( fp, "Die_npcrace	%d\n",	deity->die_npcrace );
	fprintf( fp, "Die_npcfoe	%d\n",	deity->die_npcfoe );
	fprintf( fp, "Spell_aid		%d\n",	deity->spell_aid );
	fprintf( fp, "Dig_corpse	%d\n",	deity->dig_corpse );
	fprintf( fp, "Scorpse		%d\n",	deity->scorpse );
	fprintf( fp, "Savatar		%d\n",	deity->savatar );
	fprintf( fp, "Sdeityobj		%d\n",	deity->sdeityobj );
	fprintf( fp, "Srecall		%d\n",	deity->srecall );
	fprintf( fp, "Race		%d\n",	deity->race );
	fprintf( fp, "Class		%d\n",	deity->cl );
	fprintf( fp, "Element		%d\n",	deity->element );
	fprintf( fp, "Sex		%d\n",	deity->sex );
	fprintf( fp, "Avatar		%d\n",	deity->avatar );
	fprintf( fp, "Deityobj		%d\n",	deity->deityobj	);
	fprintf( fp, "Affected		%d\n",	deity->affected );
	fprintf( fp, "Npcrace		%d\n",	deity->npcrace );
	fprintf( fp, "Npcfoe		%d\n",	deity->npcfoe );
	fprintf( fp, "Suscept		%d\n",	deity->suscept );
	fprintf( fp, "End\n\n" );
	fprintf( fp, "#END\n" );
	fclose(fp);
     }
     return;
}

/* Read in actual deity data */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				      field = value;			\
				      fMatch = TRUE;			\
				      break;				\
				}

void fread_deity( DEITY_DATA *deity, FILE *fp )
{
     char buf[MAX_STRING_LENGTH];
     const char *word = NULL;
     bool fMatch = FALSE;

     for ( ; ; )
     {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    KEY( "Affected",	deity->affected,	fread_number( fp ) );
	    KEY( "Aid",		deity->aid,		fread_number( fp ) );
	    KEY( "Aid_spell",	deity->aid_spell,	fread_number( fp ) );
	    KEY( "Alignment",	deity->alignment,	fread_number( fp ) );
	    KEY( "Avatar",	deity->avatar,		fread_number( fp ) );
	    break;

	case 'B':
	    KEY( "Backstab",	deity->backstab,	fread_number( fp ) );
	    KEY( "Bury_corpse",	deity->bury_corpse,	fread_number( fp ) );
	    break;

	case 'C':
	    KEY( "Class",	deity->cl,		fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Deityobj",	deity->deityobj,	fread_number( fp ) );
	    KEY( "Description",	deity->description,	fread_string( fp ) );
	    KEY( "Die",		deity->die,		fread_number( fp ) );
	    KEY( "Die_npcrace",	deity->die_npcrace,	fread_number( fp ) );
	    KEY( "Die_npcfoe",	deity->die_npcfoe,	fread_number( fp ) );
	    KEY( "Dig_corpse",	deity->dig_corpse,	fread_number( fp ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if (!deity->name)
		  deity->name			= STRALLOC( "" );
		if (!deity->description)
		  deity->description		= STRALLOC( "" );
		return;
	    }
	    KEY( "Element",	deity->element,		fread_number( fp ) );
	    break;

	case 'F':
	    KEY( "Filename",	deity->filename,	fread_string_nohash( fp ) );
	    KEY( "Flee",	deity->flee,		fread_number( fp ) );
	    KEY( "Flee_npcrace",deity->flee_npcrace,	fread_number( fp ) );
	    KEY( "Flee_npcfoe",	deity->flee_npcfoe,	fread_number( fp ) );
	    break;

	case 'K':
	    KEY( "Kill",	deity->kill,		fread_number( fp ) );
	    KEY( "Kill_npcrace",deity->kill_npcrace,	fread_number( fp ) );
	    KEY( "Kill_npcfoe",	deity->kill_npcfoe,	fread_number( fp ) );
	    KEY( "Kill_magic",	deity->kill_magic,	fread_number( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	deity->name,		fread_string( fp ) );
	    KEY( "Npcfoe",	deity->npcfoe,		fread_number( fp ) );
	    KEY( "Npcrace",	deity->npcrace,		fread_number( fp ) );
	    break;

	case 'R':
	    KEY( "Race",	deity->race,		fread_number( fp ) );
	    break;

	case 'S':
	    KEY( "Sac",		deity->sac,		fread_number( fp ) );
	    KEY( "Savatar",	deity->savatar,		fread_number( fp ) );
	    KEY( "Scorpse",	deity->scorpse,		fread_number( fp ) );
	    KEY( "Sdeityobj",	deity->sdeityobj,	fread_number( fp ) );
	    KEY( "Srecall",	deity->srecall,		fread_number( fp ) );
	    KEY( "Sex",		deity->sex,		fread_number( fp ) );
	    KEY( "Spell_aid",   deity->spell_aid,	fread_number( fp ) );
	    KEY( "Steal",	deity->steal,		fread_number( fp ) );
	    KEY( "Suscept",	deity->suscept,		fread_number( fp ) );
	    break;

	case 'W':
	    KEY( "Worshippers",	deity->worshippers,	fread_number( fp ) );
	    break;
	}

	if ( !fMatch )
	{
	    sprintf( buf, "Fread_deity: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}

/* Load a deity file */

bool load_deity_file( const char *deityfile )
{
     char filename[256];
     DEITY_DATA *deity;
     FILE *fp;
     bool found;

     found = FALSE;
     sprintf( filename, "%s%s", DEITY_DIR, deityfile );

     if ( ( fp = fopen( filename, "r" ) ) != NULL )
     {
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_deity_file: # not found." );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "DEITY" ) )
	    {
		CREATE( deity, DEITY_DATA, 1 );
		fread_deity( deity, fp );
		LINK(deity, first_deity, last_deity, next, prev);
		found = TRUE;
		break;
	    }
	    else
	    {
		char buf[MAX_STRING_LENGTH];
		sprintf( buf, "Load_deity_file: bad section: %s.", word );
		bug( buf, 0 );
		break;
	    }
	}
	fclose( fp );
    }

    return found;
}

/* Load in all the deity files */

void load_deity( )
{
    FILE *fpList;
    const char *filename;
    char deitylist[256];
    char buf[MAX_STRING_LENGTH];

    first_deity = NULL;
    last_deity  = NULL;

    log_string_plus( "Loading deities...", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO );

    sprintf( deitylist, "%s%s", DEITY_DIR, DEITY_LIST );
    if ( ( fpList = fopen ( deitylist, "r" ) ) == NULL )
    {
	perror( deitylist );
	exit( 1 );
    }

    for ( ; ; )
    {
	filename = feof( fpList ) ? "$" : fread_word( fpList );
        log_string_plus( filename, LOG_NORMAL, LEVEL_LOG_CSET, SEV_SPAM);
	if ( filename[0] == '$' )
	   break;
	if ( !load_deity_file( filename ) )
	{
	  sprintf( buf, "Cannot load deity file: %s", filename );
	  bug( buf, 0 );
	}
    }
    fclose( fpList );
    log_string_plus( " Done deities ", LOG_NORMAL, LEVEL_LOG_CSET, SEV_SPAM );
    return;
}

void do_setdeity( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    DEITY_DATA *deity;
    int value;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    switch( ch->substate )
    {
        default:
		break;

	case SUB_RESTRICTED:
		send_to_char( "You cannot do this while in another command.\n\r", ch );
		return;

	case SUB_DEITYDESC:
		deity = (DEITY_DATA *)ch->dest_buf;
        	STRFREE( deity->description );
        	deity->description = copy_buffer( ch );
        	stop_editing( ch );
		save_deity( deity );
      		ch->substate = ch->tempnum;
        	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Usage: setdeity <deity> <field> <toggle>\n\r", ch );
	send_to_char( "\n\rField being one of:\n\r", ch );
	send_to_char( "filename name description type alignment worshippers npcfoe\n\r", ch );
	send_to_char( "deityobj race npcrace class element avatar sex affected suscept\n\r", ch );
	send_to_char( "\n\rFavor adjustments:\n\r", ch );
	send_to_char( "flee flee_npcrace kill kill_npcrace kill_magic\n\r", ch );
	send_to_char( "die die_npcrace dig_corpse bury_corpse spell_aid\n\r", ch);
	send_to_char( "steal backstab aid aid_spell sac kill_npcfoe\n\r", ch );
	send_to_char( "die_npcfoe flee_npcfoe\n\r", ch );
	send_to_char( "\n\rFavor requirements for supplicate:\n\r", ch );
	send_to_char( "scorpse savatar sdeityobj srecall\n\r", ch );
	return;
    }

    deity = get_deity( arg1 );
    if ( !deity )
    {
 	send_to_char( "No such deity.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
	STRFREE( deity->name );
	deity->name = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "filename" ) && get_trust( ch ) >= LEVEL_SUB_IMPLEM )
    {
	DISPOSE( deity->filename );
	deity->filename = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	write_deity_list( );
	return;
    }

    if ( !str_cmp( arg2, "description" ) )
    {
	if ( ch->substate == SUB_REPEATCMD )
	  ch->tempnum = SUB_REPEATCMD;
	else
          ch->tempnum = SUB_NONE;
        ch->substate = SUB_DEITYDESC;
        ch->dest_buf = deity;
        start_editing( ch, deity->description );
        return;
    }

    if ( !str_cmp( arg2, "alignment" ) )
    {
	deity->alignment = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "flee" ) )
    {
	deity->flee = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "flee_npcrace" ) )
    {
        deity->flee_npcrace = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "flee_npcfoe" ) )
    {
	deity->flee_npcfoe = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "kill" ) )
    {
        deity->kill = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "kill_npcrace" ) )
    {
        deity->kill_npcrace = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "kill_npcfoe" ) )
    {
	deity->kill_npcfoe = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "kill_magic" ) )
    {
        deity->kill_magic = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "sac" ) )
    {
        deity->sac = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "bury_corpse" ) )
    {
        deity->bury_corpse = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "aid_spell" ) )
    {
        deity->aid_spell = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "aid" ) )
    {
        deity->aid = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "steal" ) )
    {
        deity->steal = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "backstab" ) )
    {
        deity->backstab = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "die" ) )
    {
        deity->die = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "die_npcrace" ) )
    {
        deity->die_npcrace = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "die_npcfoe" ) )
    {
	deity->die_npcfoe = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "spell_aid" ) )
    {
        deity->spell_aid = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "dig_corpse" ) )
    {
        deity->dig_corpse = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "scorpse" ) )
    {
        deity->scorpse = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "savatar" ) )
    {
        deity->savatar = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "sdeityobj" ) )
    {
        deity->sdeityobj = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "srecall" ) )
    {
        deity->srecall = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "worshippers" ) )
    {
	deity->worshippers = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "deityobj" ) )
    {
	deity->deityobj = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "race" ) )
    {
	value = get_npc_race( argument );
	if ( value < 0 ) value = atoi( argument );
	if ( ( value < 0 ) || ( value >= MAX_RACE ) )
	{
	  deity->race = -1;
	  send_to_char( "No race set.\n\r", ch );
	  return;
	}
	deity->race = value;
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "npcrace" ) )
    {
	value = get_npc_race( argument );
	if ( value < 0 ) value = atoi( argument );
	if ( ( value < 0 ) || ( value >= MAX_RACE ) )
	{
	  send_to_char( "Invalid npc race.\n\r", ch );
	  return;
	}
	deity->npcrace = value;
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "npcfoe" ) )
    {
	value = get_npc_race( argument );
	if ( value < 0 ) value = atoi( argument );
	if ( ( value < 0 ) || ( value >= MAX_RACE ) )
	{
	    send_to_char( "Invalid npc race.\n\r", ch );
	    return;
	}
	deity->npcfoe = value;
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "class" ) )
    {
	deity->cl = atoi( argument );
	if ( ( deity->cl < 0 ) || ( deity->cl >= MAX_CLASS ) ) deity->cl = -1;
        send_to_char( "Done.\n\r", ch );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "suscept" ) )
    {
      bool fMatch = FALSE;

      while ( argument[0] != '\0' )
        {
           argument = one_argument( argument, arg3 );
           if ( !str_cmp ( arg3, "none" ) )
            {
            fMatch = TRUE;
            deity->suscept = 0;
            }
           else
            {
             value = get_risflag( arg3 );
             if ( value < 0 || value > 31 )
              ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
             else
             {
              TOGGLE_BIT( deity->suscept, 1 << value );
              fMatch = TRUE;
             }
            }
        }

        if ( fMatch ) ch_printf( ch, "Done.\n\r" );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "element" ) )
    {
      bool fMatch = FALSE;

      while ( argument[0] != '\0' )
        {
           argument = one_argument( argument, arg3 );
           if ( !str_cmp ( arg3, "none" ) )
            {
            fMatch = TRUE;
            deity->element = 0;
            }
           else
            {
             value = get_risflag( arg3 );
             if ( value < 0 || value > 31 )
              ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
             else
             {
              TOGGLE_BIT( deity->element, 1 << value );
              fMatch = TRUE;
             }
            }
        }

        if ( fMatch ) ch_printf( ch, "Done.\n\r" );
        save_deity( deity );
        return;
    }

    if ( !str_cmp( arg2, "avatar" ) )
    {
	deity->avatar = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "sex" ) )
    {
	deity->sex = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_deity( deity );
	return;
    }

    if ( !str_cmp( arg2, "affected" ) )
    {
      bool fMatch = FALSE;

      while ( argument[0] != '\0' )
        {
           argument = one_argument( argument, arg3 );
           if ( !str_cmp ( arg3, "none" ) )
            {
            fMatch = TRUE;
            deity->affected = 0;
            }
           else
            {
             value = get_aflag( arg3 );
             if ( value < 0 || value > 31 )
              ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
             else
             {
              TOGGLE_BIT( deity->affected, 1 << value );
              fMatch = TRUE;
             }
            }
        }

	if ( fMatch ) ch_printf( ch, "Done.\n\r" );
	save_deity( deity );
	return;
    }

    do_setdeity( ch, "" );
    return;
}


void do_showdeity( CHAR_DATA *ch, char *argument )
{
    DEITY_DATA *deity;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Usage: showdeity <deity>\n\r", ch );
	return;
    }

    deity = get_deity( argument );
    if ( !deity )
    {
	send_to_char( "No such deity.\n\r", ch );
	return;
    }

    ch_printf( ch, "Deity: %s\n\rFilename: %s\n\rDescription:\n\r%s\n\r", deity->name, deity->filename, deity->description );
    ch_printf( ch, "Alignment: %-6dNpcrace: %-9sNpcfoe: %s\n\r",
	deity->alignment,
	      ( deity->npcrace < 0 || deity->npcrace >= MAX_RACE ) ? "none" : race_table[deity->npcrace].race_name,
	( deity->npcfoe < 0 || deity->npcfoe >= MAX_RACE ) ? "none" : race_table[deity->npcfoe].race_name );
    ch_printf( ch, "Race: %-11sClass: %-11sSex: %s\n\r",
	( deity->race < 0 || deity->race >= MAX_RACE ) ? "none" : race_table[deity->race].race_name,
        ( deity->cl < 0 || deity->cl >= MAX_CLASS ) ? "none" : pc_class[deity->cl],
	deity->sex == -1	  ? "none"   :
        deity->sex == SEX_MALE    ? "male"   :
        deity->sex == SEX_FEMALE  ? "female" : "neutral" );
    ch_printf( ch, "Object: %-9dAvatar: %-10dWorshippers: %d\n\r",
	deity->deityobj, deity->avatar, deity->worshippers );
    ch_printf( ch, "\n\rAffected: %s\n\r", flag_string( deity->affected, a_flags ));
    ch_printf( ch, "Suscept: %s\n\r", flag_string( deity->suscept, ris_flags ));
    ch_printf( ch, "Element: %s\n\r", flag_string( deity->element, ris_flags ));
    ch_printf( ch, "\n\rFlee: %-11dFlee_npcrace: %-4dKill_npcrace: "
	"%-4dKill: %d\n\r", deity->flee, deity->flee_npcrace,
	deity->kill_npcrace, deity->kill );
    ch_printf( ch, "Kill_magic: %-5dSac: %-13dBury_corpse: %-5dAid_spell: "
 	"%d\n\r", deity->kill_magic, deity->sac, deity->bury_corpse,
	deity->aid_spell );
    ch_printf( ch, "Aid: %-12dSteal: %-11dBackstab: %-8dDie: %d\n\r",
	deity->aid, deity->steal, deity->backstab, deity->die );
    ch_printf( ch, "Die_npcrace: %-4dDig_corpse: %-6dSpell_aid: %-7dKill_npcfoe: %d\n\r",
	deity->die_npcrace, deity->dig_corpse, deity->spell_aid, deity->kill_npcfoe );
    ch_printf( ch, "Die_npcfoe: %-5dFlee_npcfoe: %d\n\r",
	deity->die_npcfoe, deity->flee_npcfoe );
    ch_printf( ch, "\n\rScorpse: %-8dSavatar: %-9dSdeityobj: %-7d"
	"Srecall: %d\n\r", deity->scorpse, deity->savatar,
	deity->sdeityobj, deity->srecall );
    return;
}

void do_makedeity( CHAR_DATA *ch, char *argument )
{
    char filename[256];
    DEITY_DATA *deity;
    bool found;

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "Usage: makedeity <deity name>\n\r", ch );
	return;
    }

    found = FALSE;
    sprintf( filename, "%s%s", DEITY_DIR, strlower(argument) );
    CREATE( deity, DEITY_DATA, 1 );
    LINK( deity, first_deity, last_deity, next, prev );
    deity->name		= STRALLOC( argument );
    deity->description  = STRALLOC( "" );
    deity->filename	= str_dup( filename );
    write_deity_list();
    save_deity(deity);

}

void do_devote( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DEITY_DATA *deity;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( GetMaxLevel(ch) < 10 )
    {
	send_to_char( "You are not yet prepared for such devotion (you must be at least level 10).\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Devote yourself to which deity?\n\r", ch );
	return;
    }

    if ( !str_cmp( arg, "none" ) )
    {
        AFFECT_DATA af;
        if ( !ch->pcdata->deity )
        {
            send_to_char( "You have already chosen to worship no deities.\n\r", ch );
            return;
        }
        --ch->pcdata->deity->worshippers;
        ch->pcdata->favor = -1000;
        send_to_char( "A terrible curse afflicts you as you forsake a deity!\n\r", ch );
        if ( IS_SET( ch->affected_by, ch->pcdata->deity->affected ) )
        {
            REMOVE_BIT( ch->affected_by, ch->pcdata->deity->affected );
        }
        if ( IS_SET( ch->resistant, ch->pcdata->deity->element ) )
        {
            REMOVE_BIT( ch->resistant, ch->pcdata->deity->element );
        }
        if ( IS_SET( ch->susceptible, ch->pcdata->deity->suscept ) )
        {
            REMOVE_BIT( ch->susceptible, ch->pcdata->deity->suscept );
        }
        if ( is_affected( ch, gsn_blindness ) ) affect_strip( ch, gsn_blindness );
        af.type         = gsn_blindness;
        af.location     = APPLY_HITROLL;
        af.modifier     = -4;
        af.duration     = (short int)(50.0 * DUR_CONV);
        af.bitvector    = AFF_BLIND;
        affect_to_char( ch, &af );
        save_deity(ch->pcdata->deity);
        send_to_char( "You cease to worship any deity.\n\r", ch );
        ch->pcdata->deity = NULL;
        STRFREE(ch->pcdata->deity_name);
        ch->pcdata->deity_name = STRALLOC( "" );
        save_char_obj( ch );
        return;
    }

    if ( ch->pcdata->deity )
    {
	send_to_char( "You are already devoted to a deity.\n\r", ch );
	return;
    }

    deity = get_deity( arg );
    if ( !deity )
    {
	send_to_char( "No such deity holds weight on this world.\n\r", ch );
	return;
    }

    if ( ( deity->cl != -1 )
         && ( !HAS_CLASS(ch, deity->cl) ) )
    {
	send_to_char( "That deity will not accept your worship due to your class.\n\r", ch );
	return;
    }

    if ( ( deity->sex != -1 )
         && ( deity->sex != ch->sex ) )
    {
	send_to_char( "That deity will not accept worshippers of your sex.\n\r", ch );
	return;
    }

    if ( ( deity->race != -1 )
         && ( deity->race != ch->race ) )
    {
	send_to_char( "That deity will not accept worshippers of your race.\n\r", ch );
	return;
    }

/*
    if ( ch->pcdata->deity )
    {
	AFFECT_DATA af;
	--ch->pcdata->deity->worshippers;
	ch->pcdata->favor = -1000;
	send_to_char( "A terrible curse afflicts you as you forsake a deity!\n\r", ch );
	if ( IS_SET( ch->affected_by, ch->pcdata->deity->affected ) )
	{
	   REMOVE_BIT( ch->affected_by, ch->pcdata->deity->affected );
	}
	if ( IS_SET( ch->resistant, ch->pcdata->deity->element ) )
	{
	   REMOVE_BIT( ch->resistant, ch->pcdata->deity->element );
	}

	if ( is_affected( ch, gsn_blindness ) ) affect_strip( ch, gsn_blindness );
	af.type		= gsn_blindness;
	af.location	= APPLY_HITROLL;
	af.modifier	= -4;
	af.duration	= 50 * DUR_CONV;
	af.bitvector	= AFF_BLIND;
	affect_to_char( ch, &af );
	save_deity(ch->pcdata->deity);
    }
*/

    STRFREE(ch->pcdata->deity_name);
    ch->pcdata->deity_name = QUICKLINK( deity->name );
    ch->pcdata->deity = deity;
    if ( !IS_SET( ch->affected_by, ch->pcdata->deity->affected ) )
    {
        SET_BIT( ch->affected_by, ch->pcdata->deity->affected );
    }
    if ( !IS_SET( ch->resistant, ch->pcdata->deity->element ) )
    {
        SET_BIT( ch->resistant, ch->pcdata->deity->element );
    }
    if ( !IS_SET( ch->susceptible, ch->pcdata->deity->suscept ) )
    {
        SET_BIT( ch->susceptible, ch->pcdata->deity->suscept );
    }
    act( AT_MAGIC, "Body and soul, you devote yourself to $t!", ch, ch->pcdata->deity_name, NULL, TO_CHAR );
    ++ch->pcdata->deity->worshippers;
    save_deity(ch->pcdata->deity);
    save_char_obj( ch );
    return;
}


void do_deities( CHAR_DATA *ch, char *argument )
{
    DEITY_DATA *deity;
    int count = 0;

    if ( argument[0] == '\0' )
    {
	set_char_color( AT_NOTE, ch );
	send_to_char( "For detailed information on a deity, try deities <deity>.\n\r", ch );
	send_to_char( "\n\rDeity			Worshippers\n\r", ch );
	for ( deity = first_deity; deity; deity = deity->next )
	{
	   ch_printf( ch, "%-14s	%19d\n\r", deity->name, deity->worshippers );
	   count++;
	}

	if ( !count )
	{
	   send_to_char( "There are no deities on this world.\n\r", ch );
	   return;
	}
	return;
     }

    deity = get_deity( argument );
    if ( !deity )
    {
	send_to_char( "That deity does not exist.\n\r", ch );
	return;
    }

    set_char_color( AT_NOTE, ch );
    ch_printf( ch, "Deity: %s\n\rDescription:\n\r%s", deity->name, deity->description );
    return;
}

void do_supplicate( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) || !ch->pcdata->deity )
    {
	send_to_char( "You have no deity to supplicate to.\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Supplicate for what?\n\r", ch );
	return;
    }

    if ( !str_cmp( argument, "corpse" ) && !IS_PKILL( ch ) )
    {
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	bool found;

	if ( ch->pcdata->favor < ch->pcdata->deity->scorpse )
	{
	   send_to_char( "You are not favored enough for a corpse retrieval.\n\r", ch );
	   return;
	}

	found = FALSE;
	sprintf(buf,"the corpse of %s", ch->name);
	for ( obj = first_object; obj; obj = obj->next )
	{
	   if ( obj->in_room &&
                !str_cmp( buf, obj->short_descr ) &&
                obj->pIndexData->ivnum == OBJ_VNUM_CORPSE_PC )
	   {
		found = TRUE;
		act( AT_MAGIC, "Your corpse appears suddenly, surrounded by a divine presence...", ch, NULL, NULL, TO_CHAR );
		act( AT_MAGIC, "$n's corpse appears suddenly, surrounded by a divine force...", ch, NULL, NULL, TO_ROOM );
		obj_from_room(obj);
		obj = obj_to_room(obj, ch->in_room);
		ch->pcdata->favor -= ch->pcdata->deity->scorpse;
	   }
	}

	if ( !found )
	{
	   send_to_char( "No corpse of yours litters the world...\n\r", ch );
	   return;
	}

	return;
   }

   if ( !str_cmp( argument, "avatar" ) )
   {
	CHAR_DATA *victim;

        if ( ch->pcdata->favor < ch->pcdata->deity->savatar )
        {
           send_to_char( "You are not favored enough for that.\n\r", ch );
           return;
        }

        if ( !(victim = create_mobile( ch->pcdata->deity->avatar )) )
        {
           send_to_char( "Your deity does not have an avatar to summon.\n\r", ch );
           return;
        }
        char_to_room( victim, ch->in_room );
        act( AT_MAGIC, "$n summons a powerful avatar!", ch, NULL, NULL, TO_ROOM );
        act( AT_MAGIC, "You summon a powerful avatar!", ch, NULL, NULL, TO_CHAR );
        mprog_birth_trigger(ch, victim);
        ch->pcdata->favor -= ch->pcdata->deity->savatar;
        return;
   }

   if ( !str_cmp( argument, "object" ) )
   {
	OBJ_DATA *obj;

	if ( ch->pcdata->favor < ch->pcdata->deity->sdeityobj )
	{
	   send_to_char( "You are not favored enough for that.\n\r", ch );
	   return;
	}

        if ( !(obj = create_object( ch->pcdata->deity->deityobj )) )
        {
            send_to_char( "Your deity does not have a sigil of worship.\n\r", ch );
            return;
	}
	if ( CAN_WEAR(obj, ITEM_TAKE) )
	   obj = obj_to_char( obj, ch );
	else
	   obj = obj_to_room( obj, ch->in_room );
	act( AT_MAGIC, "$n weaves $p from divine matter!", ch, obj, NULL, TO_ROOM );
	act( AT_MAGIC, "You weave $p from divine matter!", ch, obj, NULL, TO_CHAR );
	ch->pcdata->favor -= ch->pcdata->deity->sdeityobj;
	return;
   }

   if ( !str_cmp( argument, "recall" ) )
   {
	if ( ch->pcdata->favor < ch->pcdata->deity->srecall )
	{
	   send_to_char( "You do not have enough favor for such a supplication.\n\r", ch );
 	   return;
	}

	if ( ch->in_room == get_room_index(ROOM_VNUM_HELL) )
	{
	   send_to_char( "You have been forsaken!\n\r", ch );
	   return;
	}

    	act( AT_MAGIC, "$n disappears in a column of divine power.", ch, NULL, NULL, TO_ROOM );

        recall_char(ch);
    	if ( ch->mount )
    	{
            char_from_room( ch->mount );
            char_to_room( ch->mount, ch->in_room );
    	}
    	act( AT_MAGIC, "$n appears in the room from a column of divine mist.", ch, NULL, NULL, TO_ROOM );
    	do_look( ch, "auto" );
    	ch->pcdata->favor -= ch->pcdata->deity->srecall;
    	return;
    }

    send_to_char( "You cannot supplicate for that.\n\r", ch );
    return;
}

/*
Internal function to adjust favor.
Fields are:
0 = flee		5 = sac			10 = backstab
1 = flee_npcrace	6 = bury_corpse		11 = die
2 = kill		7 = aid_spell		12 = die_npcrace
3 = kill_npcrace		8 = aid			13 = spell_aid
4 = kill_magic		9 = steal		14 = dig_corpse
15 = die_npcfoe	       16 = flee_npcfoe         17 = kill_npcfoe
*/
void  adjust_favor( CHAR_DATA *ch, int field, int mod )
{
   if ( IS_NPC( ch ) ||
        !ch->pcdata->deity )
       return;

   if ( (ch->alignment - ch->pcdata->deity->alignment > 650 ||
         ch->alignment - ch->pcdata->deity->alignment < -650) &&
        ch->pcdata->deity->alignment != 0 )
   {
       ch->pcdata->favor -= 2;
       ch->pcdata->favor = URANGE( -1000, ch->pcdata->favor, 1000 );
       return;
   }

   if ( mod < 1 ) mod = 1;
   switch ( field )
   {
   case 0:  ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->flee / mod);	break;
   case 1:  ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->flee_npcrace / mod);	 break;
   case 2:  ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->kill / mod);	break;
   case 3:  ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->kill_npcrace / mod);	 break;
   case 4:  ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->kill_magic / mod);	 break;
   case 5:  ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->sac / mod);	break;
   case 6:  ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->bury_corpse / mod);	 break;
   case 7:  ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->aid_spell / mod);	 break;
   case 8:  ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->aid / mod);	break;
   case 9:  ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->steal / mod);	break;
   case 10: ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->backstab / mod); break;
   case 11: ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->die / mod);	break;
   case 12: ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->die_npcrace / mod);	 break;
   case 13: ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->spell_aid / mod);	 break;
   case 14: ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->dig_corpse / mod);	 break;
   case 15: ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->die_npcfoe / mod);	 break;
   case 16: ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->flee_npcfoe / mod);	 break;
   case 17: ch->pcdata->favor += number_fuzzy(ch->pcdata->deity->kill_npcfoe / mod);	 break;
   }
   ch->pcdata->favor = URANGE( -1000, ch->pcdata->favor, 1000 );
   return;
}
