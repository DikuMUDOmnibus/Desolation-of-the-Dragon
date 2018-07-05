/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider             |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops  |~'~.VxvxV.~'~*
 * ------------------------------------------------------------------------ *
 *			     Special clan module			    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: clans.c,v 1.28 2004/04/06 22:00:09 dotd Exp $";*/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* #include <stdlib.h> */
#include <time.h>
#include "mud.h"

DECLARE_DO_FUN(do_where);

#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];

CLAN_DATA * first_clan;
CLAN_DATA * last_clan;
COUNCIL_DATA * first_council;
COUNCIL_DATA * last_council;

/* local routines */
void	fread_clan	args( ( CLAN_DATA *clan, FILE *fp ) );
bool	load_clan_file	args( ( const char *clanfile ) );
void	write_clan_list	args( ( void ) );

void	fread_council	args( ( COUNCIL_DATA *council, FILE *fp ) );
bool	load_council_file	args( ( const char *councilfile ) );
void	write_council_list	args( ( void ) );

void free_clans(void)
{
    CLAN_DATA *c;

    while ((c=first_clan))
    {
	UNLINK(c,first_clan,last_clan,next,prev);
	if (c->filename)
	    DISPOSE(c->filename);
	if (c->name)
	    STRFREE(c->name);
        if (c->motto)
	    STRFREE(c->motto);
	if (c->symbol)
	    STRFREE(c->symbol);
	if (c->description)
	    STRFREE(c->description);
	if (c->deity)
	    STRFREE(c->deity);
	if (c->leader)
	    STRFREE(c->leader);
	if (c->number1)
            STRFREE(c->number1);
	if (c->number2)
            STRFREE(c->number2);
	if (c->member_list)
            STRFREE(c->member_list);
        DISPOSE(c);
    }
}

/*
 * Get pointer to clan structure from clan name.
 */
CLAN_DATA *get_clan( char *name )
{
    CLAN_DATA *clan;

    for ( clan = first_clan; clan; clan = clan->next )
       if ( !str_prefix( name, clan->name ) )
         return clan;
    return NULL;
}

COUNCIL_DATA *get_council( char *name )
{
    COUNCIL_DATA *council;

    for ( council = first_council; council; council = council->next )
       if ( !str_cmp( name, council->name ) )
         return council;
    return NULL;
}

void write_clan_list( )
{
    CLAN_DATA *tclan;
    FILE *fpout;
    char filename[256];

    sprintf( filename, "%s%s", CLAN_DIR, CLAN_LIST );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
	bug( "FATAL: cannot open clan.lst for writing!\n\r" );
 	return;
    }
    for ( tclan = first_clan; tclan; tclan = tclan->next )
	fprintf( fpout, "%s\n", tclan->filename );
    fprintf( fpout, "$\n" );
    FCLOSE( fpout );
}

void write_council_list( )
{
    COUNCIL_DATA *tcouncil;
    FILE *fpout;
    char filename[256];

    sprintf( filename, "%s%s", COUNCIL_DIR, COUNCIL_LIST );
    fpout = fopen( filename, "w" );
    if ( !fpout )
    {
	bug( "FATAL: cannot open council.lst for writing!\n\r" );
 	return;
    }
    for ( tcouncil = first_council; tcouncil; tcouncil = tcouncil->next )
	fprintf( fpout, "%s\n", tcouncil->filename );
    fprintf( fpout, "$\n" );
    FCLOSE( fpout );
}

/*
 * Save a clan's data to its data file
 */
void save_clan( CLAN_DATA *clan )
{
    FILE *fp;
    char filename[256];
    char buf[MAX_STRING_LENGTH];

    if ( !clan )
    {
	bug( "save_clan: null clan pointer!" );
	return;
    }

    if ( !clan->filename || clan->filename[0] == '\0' )
    {
	sprintf( buf, "save_clan: %s has no filename", clan->name );
	bug( buf, 0 );
	return;
    }

    sprintf( filename, "%s%s", CLAN_DIR, clan->filename );

    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
    	bug( "save_clan: fopen" );
    	perror( filename );
    }
    else
    {
	fprintf( fp, "#CLAN\n" );
	fprintf( fp, "Name         %s~\n",	clan->name		);
	fprintf( fp, "Filename     %s~\n",	clan->filename		);
	fprintf( fp, "Motto        %s~\n",	clan->motto		);
	fprintf( fp, "Symbol       %s~\n",	clan->symbol		);
	fprintf( fp, "Description  %s~\n",	clan->description	);
	fprintf( fp, "Deity        %s~\n",	clan->deity		);
	fprintf( fp, "Leader       %s~\n",	clan->leader		);
	fprintf( fp, "NumberOne    %s~\n",	clan->number1		);
	fprintf( fp, "NumberTwo    %s~\n",	clan->number2		);
	fprintf( fp, "PKills       %d\n",	clan->pkills		);
	fprintf( fp, "PDeaths      %d\n",	clan->pdeaths		);
	fprintf( fp, "MKills       %d\n",	clan->mkills		);
	fprintf( fp, "MDeaths      %d\n",	clan->mdeaths		);
	fprintf( fp, "IllegalPK    %d\n",	clan->illegal_pk	);
	fprintf( fp, "Score        %d\n",	clan->score		);
	fprintf( fp, "Type         %d\n",	clan->clan_type		);
	fprintf( fp, "Class        %d\n",	clan->cl		);
	fprintf( fp, "Favour       %d\n",	clan->favour		);
	fprintf( fp, "Strikes      %d\n",	clan->strikes		);
	fprintf( fp, "Members      %d\n",	clan->members		);
	fprintf( fp, "Alignment    %d\n",	clan->alignment		);
	fprintf( fp, "Board        %d\n",	clan->board		);
	fprintf( fp, "ClanObjOne   %d\n",	clan->clanobj1		);
	fprintf( fp, "ClanObjTwo   %d\n",	clan->clanobj2		);
	fprintf( fp, "ClanObjThree %d\n",	clan->clanobj3		);
	fprintf( fp, "Recall       %d\n",	clan->recall		);
	fprintf( fp, "Storeroom    %d\n",	clan->storeroom		);
	fprintf( fp, "GuardOne     %d\n",	clan->guard1		);
	fprintf( fp, "GuardTwo     %d\n",	clan->guard2		);
	fprintf( fp, "End\n\n"						);
	fprintf( fp, "#END\n"						);
    }
    FCLOSE( fp );
    return;
}

/*
 * Save a council's data to its data file
 */
void save_council( COUNCIL_DATA *council )
{
    FILE *fp;
    char filename[256];
    char buf[MAX_STRING_LENGTH];

    if ( !council )
    {
	bug( "save_council: null council pointer!" );
	return;
    }

    if ( !council->filename || council->filename[0] == '\0' )
    {
	sprintf( buf, "save_council: %s has no filename", council->name );
	bug( buf, 0 );
	return;
    }

    sprintf( filename, "%s%s", COUNCIL_DIR, council->filename );

    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
    	bug( "save_council: fopen" );
    	perror( filename );
    }
    else
    {
	fprintf( fp, "#COUNCIL\n" );
	fprintf( fp, "Name         %s~\n",	council->name		);
        fprintf( fp, "Filename     %s~\n",	council->filename	);
        if (council->description)
            fprintf( fp, "Description  %s~\n",	council->description	);
        if (council->head)
            fprintf( fp, "Head         %s~\n",	council->head		);
        if (council->members)
            fprintf( fp, "Members      %d\n",	council->members	);
        if (council->board)
            fprintf( fp, "Board        %d\n",	council->board		);
        if (council->meeting)
            fprintf( fp, "Meeting      %d\n",	council->meeting	);
        if (council->powers)
            fprintf( fp, "Powers       %s~\n",	council->powers		);
	fprintf( fp, "End\n\n"						);
	fprintf( fp, "#END\n"						);
    }
    FCLOSE( fp );
    return;
}


/*
 * Read in actual clan data.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void fread_clan( CLAN_DATA *clan, FILE *fp )
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
	    KEY( "Alignment",	clan->alignment,	fread_number( fp ) );
	    break;

	case 'B':
	    KEY( "Board",	clan->board,		fread_number( fp ) );
	    break;

	case 'C':
	    KEY( "ClanObjOne",	clan->clanobj1,		fread_number( fp ) );
	    KEY( "ClanObjTwo",	clan->clanobj2,		fread_number( fp ) );
	    KEY( "ClanObjThree",clan->clanobj3,		fread_number( fp ) );
	    KEY( "Class",	clan->cl,		fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Deity",	clan->deity,		fread_string( fp ) );
	    KEY( "Description",	clan->description,	fread_string( fp ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if (!clan->name)
		  clan->name		= STRALLOC( "" );
		if (!clan->leader)
		  clan->leader		= STRALLOC( "" );
		if (!clan->description)
		  clan->description 	= STRALLOC( "" );
		if (!clan->motto)
		  clan->motto		= STRALLOC( "" );
		if (!clan->symbol)
		  clan->symbol		= STRALLOC( "" );
		if (!clan->number1)
		  clan->number1		= STRALLOC( "" );
		if (!clan->number2)
		  clan->number2		= STRALLOC( "" );
		if (!clan->deity)
		  clan->deity		= STRALLOC( "" );
		return;
	    }
	    break;

	case 'F':
	    KEY( "Favour",	clan->favour,		fread_number( fp ) );
	    KEY( "Filename",	clan->filename,		fread_string_nohash( fp ) );

	case 'G':
	    KEY( "GuardOne",	clan->guard1,		fread_number( fp ) );
	    KEY( "GuardTwo",	clan->guard2,		fread_number( fp ) );
	    break;

	case 'I':
	    KEY( "IllegalPK",	clan->illegal_pk,	fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Leader",	clan->leader,		fread_string( fp ) );
	    break;

	case 'M':
	    KEY( "MDeaths",	clan->mdeaths,		fread_number( fp ) );
	    KEY( "Members",	clan->members,		fread_number( fp ) );
	    KEY( "MemberList",	clan->member_list,     	fread_string( fp ) );
	    KEY( "MKills",	clan->mkills,		fread_number( fp ) );
	    KEY( "Motto",	clan->motto,		fread_string( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	clan->name,		fread_string( fp ) );
	    KEY( "NumberOne",	clan->number1,		fread_string( fp ) );
	    KEY( "NumberTwo",	clan->number2,		fread_string( fp ) );
	    break;

	case 'P':
	    KEY( "PDeaths",	clan->pdeaths,		fread_number( fp ) );
	    KEY( "PKills",	clan->pkills,		fread_number( fp ) );
	    break;

	case 'R':
	    KEY( "Recall",	clan->recall,		fread_number( fp ) );
	    break;

	case 'S':
	    KEY( "Score",	clan->score,		fread_number( fp ) );
	    KEY( "Strikes",	clan->strikes,		fread_number( fp ) );
	    KEY( "Storeroom",	clan->storeroom,	fread_number( fp ) );
	    KEY( "Symbol",	clan->symbol,		fread_string( fp ) );
	    break;

	case 'T':
	    KEY( "Type",	clan->clan_type,	fread_number( fp ) );
	    break;
	}

	if ( !fMatch )
	{
	    sprintf( buf, "Fread_clan: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}

/*
 * Read in actual council data.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void fread_council( COUNCIL_DATA *council, FILE *fp )
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

	case 'B':
	    KEY( "Board",	council->board,		fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Description",	council->description,	fread_string( fp ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if (!council->name)
		  council->name		= STRALLOC( "" );
		if (!council->description)
		  council->description 	= STRALLOC( "" );
		if (!council->powers)
		  council->powers	= STRALLOC( "" );
		return;
	    }
	    break;

	case 'F':
	    KEY( "Filename",	council->filename,	fread_string_nohash( fp ) );
  	    break;

	case 'H':
	    KEY( "Head", 	council->head, 		fread_string( fp ) );
	    break;

	case 'M':
	    KEY( "Members",	council->members,	fread_number( fp ) );
	    KEY( "Meeting",   	council->meeting, 	fread_number( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	council->name,		fread_string( fp ) );
	    break;

	case 'P':
	    KEY( "Powers",	council->powers,	fread_string( fp ) );
	    break;
	}

	if ( !fMatch )
	{
	    sprintf( buf, "Fread_council: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}


/*
 * Load a clan file
 */

bool load_clan_file( const char *clanfile )
{
    char filename[256];
    CLAN_DATA *clan;
    FILE *fp;
    bool found;

    CREATE( clan, CLAN_DATA, 1 );

    found = FALSE;
    sprintf( filename, "%s%s", CLAN_DIR, clanfile );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {

	found = TRUE;
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
		bug( "Load_clan_file: # not found." );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "CLAN"	) )
	    {
	    	fread_clan( clan, fp );
	    	break;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		char buf[MAX_STRING_LENGTH];

		sprintf( buf, "Load_clan_file: bad section: %s.", word );
		bug( buf, 0 );
		break;
	    }
	}
	FCLOSE( fp );
    }

    if ( found )
    {
	ROOM_INDEX_DATA *storeroom;

	LINK( clan, first_clan, last_clan, next, prev );

	if ( clan->storeroom == 0
	|| (storeroom = get_room_index( clan->storeroom )) == NULL )
	{
	    log_string_plus( "Storeroom not found", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO );
	    return found;
	}

        if ( !IS_ROOM_FLAG( storeroom, ROOM_CLANSTOREROOM ) )
        {
            log_printf_plus( LOG_NORMAL, LEVEL_LOG_CSET, SEV_WARN,
                             "Setting ROOM_CLANSTOREROOM flag on room %d", clan->storeroom );
            SET_ROOM_FLAG( storeroom, ROOM_CLANSTOREROOM );
        }

	sprintf( filename, "%s%s.vault", CLAN_DIR, clan->filename );
	if ( ( fp = fopen( filename, "r" ) ) != NULL )
	{
	    int iNest;
	    OBJ_DATA *tobj, *tobj_next;

	    log_string_plus( "Loading clan storage room", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO );
	    rset_supermob(storeroom);
	    for ( iNest = 0; iNest < MAX_NEST; iNest++ )
		rgObjNest[iNest] = NULL;

	    found = TRUE;
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
		    bug( "Load_clan_vault: # not found." );
		    bug( clan->name, 0 );
		    break;
		}

		word = fread_word( fp );
		if ( !str_cmp( word, "OBJECT" ) )	/* Objects	*/
		  fread_obj  ( supermob, fp, OS_CARRY );
		else
		if ( !str_cmp( word, "END"    ) )	/* Done		*/
        	  break;
		else
		{
		    bug( "Load_clan_vault: bad section." );
		    bug( clan->name, 0 );
		    break;
		}
	    }
	    FCLOSE( fp );
	    for ( tobj = supermob->first_carrying; tobj; tobj = tobj_next )
	    {
		tobj_next = tobj->next_content;
		obj_from_char( tobj );
		obj_to_room( tobj, storeroom );
	    }
	    release_supermob();
	}
	else
	    log_string_plus( "Cannot open clan vault", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO );
    }
    else
      DISPOSE( clan );

    return found;
}

/*
 * Load a council file
 */

bool load_council_file( const char *councilfile )
{
    char filename[256];
    COUNCIL_DATA *council;
    FILE *fp;
    bool found;

    CREATE( council, COUNCIL_DATA, 1 );

    found = FALSE;
    sprintf( filename, "%s%s", COUNCIL_DIR, councilfile );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {

	found = TRUE;
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
		bug( "Load_council_file: # not found." );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "COUNCIL"	) )
	    {
	    	fread_council( council, fp );
	    	break;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
		bug( "Load_council_file: bad section." );
		break;
	    }
	}
	FCLOSE( fp );
    }

    if ( found )
      LINK( council, first_council, last_council, next, prev );

    else
      DISPOSE( council );

    return found;
}

/*
 * Load in all the clan files.
 */
void load_clans( )
{
    FILE *fpList;
    const char *filename;
    char clanlist[256];
    char buf[MAX_STRING_LENGTH];


    first_clan	= NULL;
    last_clan	= NULL;

    log_string_plus( "Loading clans...", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO );

    sprintf( clanlist, "%s%s", CLAN_DIR, CLAN_LIST );
    if ( ( fpList = fopen( clanlist, "r" ) ) == NULL )
    {
	perror( clanlist );
	exit( 1 );
    }

    for ( ; ; )
    {
	filename = feof( fpList ) ? "$" : fread_word( fpList );
	log_string_plus( filename, LOG_NORMAL, LEVEL_LOG_CSET, SEV_SPAM );
	if ( filename[0] == '$' )
	  break;

	if ( !load_clan_file( filename ) )
	{
	  sprintf( buf, "Cannot load clan file: %s", filename );
	  bug( buf, 0 );
	}
    }
    FCLOSE( fpList );
    log_string_plus(" Done clans ", LOG_NORMAL, LEVEL_LOG_CSET, SEV_SPAM );
    return;
}

/*
 * Load in all the council files.
 */
void load_councils( )
{
    FILE *fpList;
    const char *filename;
    char councillist[256];
    char buf[MAX_STRING_LENGTH];


    first_council	= NULL;
    last_council	= NULL;

    log_string_plus( "Loading councils...", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO );

    sprintf( councillist, "%s%s", COUNCIL_DIR, COUNCIL_LIST );
    if ( ( fpList = fopen( councillist, "r" ) ) == NULL )
    {
	perror( councillist );
	exit( 1 );
    }

    for ( ; ; )
    {
	filename = feof( fpList ) ? "$" : fread_word( fpList );
	log_string_plus( filename, LOG_NORMAL, LEVEL_LOG_CSET, SEV_SPAM );
	if ( filename[0] == '$' )
	  break;

	if ( !load_council_file( filename ) )
	{
	  sprintf( buf, "Cannot load council file: %s", filename );
	  bug( buf, 0 );
	}
    }
    FCLOSE( fpList );
    log_string_plus(" Done councils ", LOG_NORMAL, LEVEL_LOG_CSET, SEV_SPAM );
    return;
}

void do_make( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    clan = ch->pcdata->clan;

    if ( str_cmp( ch->name, clan->leader )
    &&   str_cmp( ch->name, clan->deity )
    &&  (clan->clan_type != CLAN_GUILD
    ||   str_cmp( ch->name, clan->number1 )) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Make what?\n\r", ch );
	return;
    }

    pObjIndex = get_obj_index( clan->clanobj1 );

    if ( !pObjIndex || !is_name( arg, pObjIndex->name ) )
    {
      pObjIndex = get_obj_index( clan->clanobj2 );
    }
    if ( !pObjIndex || !is_name( arg, pObjIndex->name ) )
    {
      pObjIndex = get_obj_index( clan->clanobj3 );
    }

    if ( !pObjIndex || !is_name( arg, pObjIndex->name ) )
    {
	send_to_char( "You don't know how to make that.\n\r", ch );
	return;
    }

    obj = create_object( pObjIndex->ivnum );
    SET_BIT( obj->extra_flags, ITEM_CLANOBJECT );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
      obj = obj_to_char( obj, ch );
    else
      obj = obj_to_room( obj, ch->in_room );
    act( AT_MAGIC, "$n makes $p!", ch, obj, NULL, TO_ROOM );
    act( AT_MAGIC, "You make $p!", ch, obj, NULL, TO_CHAR );
    return;
}

void do_induct( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    clan = ch->pcdata->clan;

    if ( (ch->pcdata && ch->pcdata->bestowments
    &&    is_name("induct", ch->pcdata->bestowments))
    ||   !str_cmp( ch->name, clan->deity   )
    ||   !str_cmp( ch->name, clan->leader  )
    ||   !str_cmp( ch->name, clan->number1 )
    ||   !str_cmp( ch->name, clan->number2 ) )
	;
    else
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Induct whom?\n\r", ch );
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

    if ( clan->clan_type == CLAN_GUILD )
    {
	if ( HAS_CLASS(victim, clan->cl))
	{
	    send_to_char( "This player's will is not in accordance with your guild.\n\r", ch);
            return;
	}
    }
    else
    {
	if ( GetMaxLevel(victim) < 10 )
	{
	    send_to_char( "This player is not worthy of joining yet.\n\r", ch );
	    return;
	}

	if ( GetMaxLevel(victim) > GetMaxLevel(ch) )
	{
	    send_to_char( "This player is too powerful for you to induct.\n\r", ch );
	    return;
	}
    }

    if ( victim->pcdata->clan )
    {
      if ( victim->pcdata->clan->clan_type == CLAN_ORDER )
      {
	if ( victim->pcdata->clan == clan )
	  send_to_char( "This player already belongs to your order!\n\r", ch );
	else
	  send_to_char( "This player already belongs to an order!\n\r", ch );
	return;
      }
      else
      if ( victim->pcdata->clan->clan_type == CLAN_GUILD )
      {
	if ( victim->pcdata->clan == clan )
	  send_to_char( "This player already belongs to your guild!\n\r", ch );
	else
	  send_to_char( "This player already belongs to an guild!\n\r", ch );
	return;
      }
      else
      {
	if ( victim->pcdata->clan == clan )
	  send_to_char( "This player already belongs to your clan!\n\r", ch );
	else
	  send_to_char( "This player already belongs to a clan!\n\r", ch );
	return;
      }
    }
    clan->members++;
    if ( clan->clan_type != CLAN_ORDER && clan->clan_type != CLAN_GUILD )
      SET_BIT(victim->speaks, LANG_CLAN);

    if ( clan->clan_type != CLAN_NOKILL && clan->clan_type != CLAN_ORDER
    &&   clan->clan_type != CLAN_GUILD )
      SET_BIT( victim->pcdata->flags, PCFLAG_DEADLY );

    if ( clan->clan_type == CLAN_GUILD )
    {
	int sn;

	for ( sn = 0; sn < top_sn; sn++ )
	{
	    if (skill_table[sn]->guild == clan->cl &&
		skill_table[sn]->name != NULL )
	    {
                victim->pcdata->learned[sn] = GET_ADEPT(victim, sn);
		ch_printf( victim, "%s instructs you in the ways of %s.\n\r", ch->name, skill_table[sn]->name);
	    }
	}
    }

    victim->pcdata->clan = clan;
    STRFREE(victim->pcdata->clan_name);
    victim->pcdata->clan_name = QUICKLINK( clan->name );
    act( AT_MAGIC, "You induct $N into $t", ch, clan->name, victim, TO_CHAR );
    act( AT_MAGIC, "$n inducts $N into $t", ch, clan->name, victim, TO_NOTVICT );
    act( AT_MAGIC, "$n inducts you into $t", ch, clan->name, victim, TO_VICT );
    save_char_obj( victim );
    return;
}

void do_council_induct( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    COUNCIL_DATA *council;

    if ( IS_NPC( ch ) || !ch->pcdata->council )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    council = ch->pcdata->council;

    if ( str_cmp( ch->name, council->head) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Induct whom into your council?\n\r", ch );
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

    if ( victim->pcdata->council )
    {
	send_to_char( "This player already belongs to a council!\n\r", ch );
	return;
    }

    council->members++;
    victim->pcdata->council = council;
    STRFREE(victim->pcdata->council_name);
    victim->pcdata->council_name = QUICKLINK( council->name );
    act( AT_MAGIC, "You induct $N into $t", ch, council->name, victim, TO_CHAR );
    act( AT_MAGIC, "$n inducts $N into $t", ch, council->name, victim, TO_ROOM );
    act( AT_MAGIC, "$n inducts you into $t", ch, council->name, victim, TO_VICT );
    save_char_obj( victim );
    save_council( council );
    return;
}

void do_outcast( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CLAN_DATA *clan;
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) || !ch->pcdata->clan )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    clan = ch->pcdata->clan;

    if ( (ch->pcdata && ch->pcdata->bestowments
    &&    is_name("outcast", ch->pcdata->bestowments))
    ||   !str_cmp( ch->name, clan->deity   )
    ||   !str_cmp( ch->name, clan->leader  )
    ||   !str_cmp( ch->name, clan->number1 )
    ||   !str_cmp( ch->name, clan->number2 ) )
	;
    else
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }


    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Outcast whom?\n\r", ch );
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

    if ( victim == ch )
    {
	if ( ch->pcdata->clan->clan_type == CLAN_ORDER )
	{
	    send_to_char( "Kick yourself out of your own order?\n\r", ch );
	    return;
	}
	else
	if ( ch->pcdata->clan->clan_type == CLAN_GUILD )
	{
	    send_to_char( "Kick yourself out of your own guild?\n\r", ch );
	    return;
	}
	else
	{
	    send_to_char( "Kick yourself out of your own clan?\n\r", ch );
	    return;
	}
    }

    if ( GetMaxLevel(victim) > GetMaxLevel(ch) )
    {
	send_to_char( "This player is too powerful for you to outcast.\n\r", ch );
	return;
    }

    if ( victim->pcdata->clan != ch->pcdata->clan )
    {
	if ( ch->pcdata->clan->clan_type == CLAN_ORDER )
	{
	    send_to_char( "This player does not belong to your order!\n\r", ch );
	    return;
	}
	else
	if ( ch->pcdata->clan->clan_type == CLAN_GUILD )
	{
	    send_to_char( "This player does not belong to your guild!\n\r", ch );
	    return;
	}
	else
	{
	    send_to_char( "This player does not belong to your clan!\n\r", ch );
	    return;
	}
    }

    if ( clan->clan_type == CLAN_GUILD )
    {
	int sn;

	for ( sn = 0; sn < top_sn; sn++ )
	    if ( skill_table[sn]->guild == victim->pcdata->clan->cl
	    &&   skill_table[sn]->name != NULL )
	    {
		victim->pcdata->learned[sn] = 0;
		ch_printf( ch, "You forget the ways of %s.\n\r", skill_table[sn]->name);
	    }
    }

    if ( victim->speaking & LANG_CLAN )
        victim->speaking = LANG_COMMON;
    REMOVE_BIT( victim->speaks, LANG_CLAN );
    --clan->members;
    if ( !str_cmp( victim->name, ch->pcdata->clan->number1 ) )
    {
	STRFREE( ch->pcdata->clan->number1 );
	ch->pcdata->clan->number1 = STRALLOC( "" );
    }
    if ( !str_cmp( victim->name, ch->pcdata->clan->number2 ) )
    {
	STRFREE( ch->pcdata->clan->number2 );
	ch->pcdata->clan->number2 = STRALLOC( "" );
    }
    victim->pcdata->clan = NULL;
    STRFREE(victim->pcdata->clan_name);
    victim->pcdata->clan_name = STRALLOC( "" );
    act( AT_MAGIC, "You outcast $N from $t", ch, clan->name, victim, TO_CHAR );
    act( AT_MAGIC, "$n outcasts $N from $t", ch, clan->name, victim, TO_ROOM );
    act( AT_MAGIC, "$n outcasts you from $t", ch, clan->name, victim, TO_VICT );
    if ( clan->clan_type != CLAN_GUILD )
    {
	sprintf(buf, "%s has been outcast from %s!", victim->name, clan->name);
	echo_to_all(AT_MAGIC, buf, ECHOTAR_ALL);
    }

/* Outcast flag setting removed by Narn.  It's useless now that deadlies
   remain deadly even on being cast out of a clan.
*/
/*    if ( clan->clan_type != CLAN_GUILD )
	SET_BIT(victim->act, PLR_OUTCAST);
*/
    save_char_obj( victim );	/* clan gets saved when pfile is saved */
    return;
}

void do_council_outcast( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    COUNCIL_DATA *council;

    if ( IS_NPC( ch ) || !ch->pcdata->council )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    council = ch->pcdata->council;

    if ( str_cmp( ch->name, council->head ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Outcast whom from your council?\n\r", ch );
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

    if ( victim == ch )
    {
	send_to_char( "Kick yourself out of your own council?\n\r", ch );
	return;
    }

    if ( victim->pcdata->council != ch->pcdata->council )
    {
	send_to_char( "This player does not belong to your council!\n\r", ch );
	return;
    }

    --council->members;
    victim->pcdata->council = NULL;
    STRFREE(victim->pcdata->council_name);
    victim->pcdata->council_name = STRALLOC( "" );
    act( AT_MAGIC, "You outcast $N from $t", ch, council->name, victim, TO_CHAR );
    act( AT_MAGIC, "$n outcasts $N from $t", ch, council->name, victim, TO_ROOM );
    act( AT_MAGIC, "$n outcasts you from $t", ch, council->name, victim, TO_VICT );
    save_char_obj( victim );
    save_council( council );
    return;
}

void do_setclan( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Usage: setclan <clan> <field> <deity|leader|number1|number2> <player>\n\r", ch );
	send_to_char( "\n\rField being one of:\n\r", ch );
	send_to_char( " deity leader number1 number2\n\r", ch );
	send_to_char( " members board recall storage\n\r", ch );
	send_to_char( " obj1 obj2 obj3 guard1 guard2\n\r", ch );
        send_to_char( " name filename motto desc symbol\n\r", ch );
        send_to_char( " favour strikes type align\n\r", ch );
        if ( get_trust( ch ) >= LEVEL_SUB_IMPLEM )
            send_to_char( " filename\n\r", ch );
	return;
    }

    clan = get_clan( arg1 );
    if ( !clan )
    {
	send_to_char( "No such clan.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "deity" ) )
    {
	STRFREE( clan->deity );
	clan->deity = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !str_cmp( arg2, "leader" ) )
    {
	STRFREE( clan->leader );
	clan->leader = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !str_cmp( arg2, "number1" ) )
    {
	STRFREE( clan->number1 );
	clan->number1 = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !str_cmp( arg2, "number2" ) )
    {
	STRFREE( clan->number2 );
	clan->number2 = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !str_cmp( arg2, "board" ) )
    {
	clan->board = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !str_cmp( arg2, "members" ) )
    {
	clan->members = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !str_cmp( arg2, "recall" ) )
    {
	clan->recall = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !str_cmp( arg2, "storage" ) )
    {
	clan->storeroom = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !str_cmp( arg2, "obj1" ) )
    {
	clan->clanobj1 = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !str_cmp( arg2, "obj2" ) )
    {
	clan->clanobj2 = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !str_cmp( arg2, "obj3" ) )
    {
	clan->clanobj3 = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !str_cmp( arg2, "guard1" ) )
    {
	clan->guard1 = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !str_cmp( arg2, "guard2" ) )
    {
	clan->guard2 = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !str_cmp( arg2, "motto" ) )
    {
	STRFREE( clan->motto );
	clan->motto = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !str_cmp( arg2, "desc" ) )
    {
	STRFREE( clan->description );
	clan->description = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !str_cmp( arg2, "align" ) )
    {
	clan->alignment = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !str_cmp( arg2, "type" ) )
    {
	if ( !str_cmp( argument, "order" ) )
	  clan->clan_type = CLAN_ORDER;
	else
	if ( !str_cmp( argument, "guild" ) )
	  clan->clan_type = CLAN_GUILD;
	else
	  clan->clan_type = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !str_cmp( arg2, "class" ) )
    {
	clan->cl = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
	STRFREE( clan->name );
	clan->name = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }

    if ( !str_cmp( arg2, "symbol" ) )
    {
	STRFREE( clan->symbol );
	clan->symbol = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	return;
    }
    if ( get_trust( ch ) < LEVEL_SUB_IMPLEM )
    {
	do_setclan( ch, "" );
	return;
    }

    if ( !str_cmp( arg2, "filename" ) && get_trust( ch ) >= LEVEL_SUB_IMPLEM )
    {
	if ( clan->filename )
	    DISPOSE( clan->filename );
	clan->filename = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	save_clan( clan );
	write_clan_list( );
	return;
    }

    do_setclan( ch, "" );
    return;
}

void do_setcouncil( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    COUNCIL_DATA *council;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Usage: setcouncil <council> <field> <deity|leader|number1|number2> <player>\n\r", ch );
	send_to_char( "\n\rField being one of:\n\r", ch );
	send_to_char( " head members board meeting\n\r", ch );
	if ( get_trust( ch ) >= LEVEL_GOD )
	  send_to_char( " name filename desc\n\r", ch );
        if ( get_trust( ch ) >= LEVEL_SUB_IMPLEM )
	  send_to_char( " powers\n\r", ch);
	return;
    }

    council = get_council( arg1 );
    if ( !council )
    {
	send_to_char( "No such council.\n\r", ch );
	return;
    }

    if ( !str_cmp( arg2, "head" ) )
    {
	STRFREE( council->head );
	council->head = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }

    if ( !str_cmp( arg2, "board" ) )
    {
	council->board = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }

    if ( !str_cmp( arg2, "members" ) )
    {
	council->members = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }

    if ( !str_cmp( arg2, "meeting" ) )
    {
	council->meeting = atoi( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }

    if ( get_trust( ch ) < LEVEL_GOD )
    {
	do_setcouncil( ch, "" );
	return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
	STRFREE( council->name );
	council->name = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }


    if ( !str_cmp( arg2, "filename" ) )
    {
	if ( council->filename )
	    DISPOSE( council->filename );
	council->filename = str_dup( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	write_council_list( );
	return;
    }

    if ( !str_cmp( arg2, "desc" ) )
    {
	STRFREE( council->description );
	council->description = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }

    if ( get_trust( ch ) < LEVEL_SUB_IMPLEM )
    {
	do_setcouncil( ch, "" );
	return;
    }

    if ( !str_cmp( arg2, "powers" ) )
    {
	STRFREE( council->powers );
	council->powers = STRALLOC( argument );
	send_to_char( "Done.\n\r", ch );
	save_council( council );
	return;
    }

    do_setcouncil( ch, "" );
    return;
}

void do_showclan( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Usage: showclan <clan>\n\r", ch );
	return;
    }

    clan = get_clan( argument );
    if ( !clan )
    {
	send_to_char( "No such clan.\n\r", ch );
	return;
    }

    ch_printf( ch, "%-5s      : %s\n\rFilename   : %s\n\rMotto      : %s\n\r",
               clan->clan_type == CLAN_ORDER ? "Order" :
               (clan->clan_type == CLAN_GUILD ? "Guild" : "Clan "),
               clan->name,
               clan->filename,
               clan->motto );
    ch_printf( ch, "Symbol    : %s\n\rDescription: %s\n\rDeity      : %s\n\rLeader     : %s\n\r",
               clan->symbol,
               clan->description,
               clan->deity,
               clan->leader );
    ch_printf( ch, "Number1    : %s\n\rNumber2    : %s\n\rPKills     : %6d    PDeaths   : %6d\n\r",
               clan->number1,
               clan->number2,
               clan->pkills,
               clan->pdeaths );
    ch_printf( ch, "MKills     : %6d    MDeaths   : %6d\n\r",
               clan->mkills,
               clan->mdeaths );
    ch_printf( ch, "IllegalPK  : %6d    Score     : %6d\n\r",
               clan->illegal_pk,
               clan->score );
    ch_printf( ch, "Type       : %6d    Favour    : %6d    Strikes : %6d\n\r",
               clan->clan_type,
               clan->favour,
               clan->strikes );
    ch_printf( ch, "Members    : %6d    Alignment : %6d    Class   : %6d\n\r",
               clan->members,
               clan->alignment,
               clan->cl );
    ch_printf( ch, "Board      : %6d\n\rObject1    : %6d    Object2   : %6d    Object3 : %6d\n\r",
               clan->board,
               clan->clanobj1,
               clan->clanobj2,
               clan->clanobj3 );
    ch_printf( ch, "Recall     : %6d    Storeroom : %6d\n\rGuard1     : %6d    Guard2    : %6d\n\r",
               clan->recall,
               clan->storeroom,
               clan->guard1,
               clan->guard2 );
    return;
}

void show_clan_to_pc( CHAR_DATA *ch, CLAN_DATA *clan )
{
    char s1[16],s2[16],s3[16],s4[16];
    sh_int ratio, weight;

    sprintf(s1,"%s",color_str(AT_SCORE,ch));
    sprintf(s2,"%s",color_str(AT_SCORE2,ch));
    sprintf(s3,"%s",color_str(AT_SCORE3,ch));
    sprintf(s4,"%s",color_str(AT_SCORE4,ch));

    ch_printf(ch, "%sClan: %s%s%s [%s%s].\n\r"
              "The clan is lead by %s%s%s",
              s1, s3, clan->name, s1,
              clan->symbol, s1,
              s3, clan->leader, s1);
    if ( clan->number1 && *clan->number1 )
    {
        if ( !clan->number2 && !*clan->number2)
            send_to_char(" and ", ch);
        else
            send_to_char(", ", ch);

        ch_printf(ch, "%s%s%s",
                  s3, clan->number1, s1);
    }
    if ( clan->number2 && *clan->number2 )
        ch_printf(ch, ", and %s%s%s",
                  s3, clan->number2, s1);
    send_to_char(".\n\r", ch);

    ch_printf(ch, "Clan motto: %s%s%s\n\r",
              s3, clan->motto, s1);

    ch_printf(ch, "Description of the clan: %s%s%s\n\r",
              s3, clan->description, s1);

    weight=2;
    ratio=0;
    while (ratio <= 100 && (clan->mdeaths*weight) < clan->mkills)
    {
        weight *= 2;
        ratio += 9;
    }
    ratio = URANGE(1, ratio, 100);
    ch_printf(ch, "Mobs killed: %s%d%s  Deaths by mobs: %s%d%s  Rating: %s%s%s\n\r",
              s2, clan->mkills, s1,
              s2, clan->mdeaths, s1,
              s3, how_good(ratio), s1);

    weight=2;
    ratio=0;
    while (ratio <= 100 && (clan->pdeaths*weight) < clan->pkills)
    {
        weight *= 2;
        ratio += 9;
    }
    ratio = URANGE(1, ratio, 100);
    ch_printf(ch, "Players killed: %s%d%s  Deaths by players: %s%d%s  Rating: %s%s%s\n\r",
              s2, clan->pkills, s1,
              s2, clan->pdeaths, s1,
              s3, how_good(ratio), s1);

}

void do_showcouncil( CHAR_DATA *ch, char *argument )
{
    COUNCIL_DATA *council;

    if ( IS_NPC( ch ) )
    {
	send_to_char( "Huh?\n\r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	send_to_char( "Usage: showcouncil <council>\n\r", ch );
	return;
    }

    council = get_council( argument );
    if ( !council )
    {
	send_to_char( "No such council.\n\r", ch );
	return;
    }

    ch_printf( ch, "Council    : %s\n\rFilename: %s\n\r",
               council->name?council->name:"(None)",
               council->filename?council->filename:"(None)" );
    ch_printf( ch, "Description: %s\n\rHead: %s\n\rMembers: %3d\n\r",
               council->description?council->description:"(None)",
               council->head?council->head:"(None)",
               council->members );
    ch_printf( ch, "Board: %5d\n\rMeeting: %5d\n\rPowers: %s\n\r",
               council->board,
               council->meeting,
               council->powers?council->powers:"(None)" );
    return;
}

void do_makeclan( CHAR_DATA *ch, char *argument )
{
    char filename[256];
    CLAN_DATA *clan;
    bool found;

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "Usage: makeclan <clan name>\n\r", ch );
	return;
    }

    found = FALSE;
    sprintf( filename, "%s%s", CLAN_DIR, strlower(argument) );

    CREATE( clan, CLAN_DATA, 1 );
    LINK( clan, first_clan, last_clan, next, prev );

    clan->name		= STRALLOC( argument );
    clan->motto		= STRALLOC( "" );
    clan->description	= STRALLOC( "" );
    clan->deity		= STRALLOC( "" );
    clan->leader	= STRALLOC( "" );
    clan->number1	= STRALLOC( "" );
    clan->number2	= STRALLOC( "" );
}

void do_makecouncil( CHAR_DATA *ch, char *argument )
{
    char filename[256];
    COUNCIL_DATA *council;
    bool found;

    if ( !argument || argument[0] == '\0' )
    {
	send_to_char( "Usage: makecouncil <council name>\n\r", ch );
	return;
    }

    found = FALSE;
    sprintf( filename, "%s%s", COUNCIL_DIR, strlower(argument) );

    CREATE( council, COUNCIL_DATA, 1 );
    LINK( council, first_council, last_council, next, prev );
    council->name		= STRALLOC( argument );
    council->head		= STRALLOC( "" );
    council->powers		= STRALLOC( "" );
}

void do_clans( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *clan;
    int count = 0;

    /* Switched deadly clan mobkills/mobdeaths to pkills -- Blodkai */
    set_char_color( AT_RED, ch );
    ch_printf( ch, "%-25s %-14s %-14s PKills\n\r", "Clan", "Deity", "Leader" );
    for ( clan = first_clan; clan; clan = clan->next )
    {
        if ( clan->clan_type == CLAN_ORDER || clan->clan_type == CLAN_GUILD )
          continue;
        set_char_color( AT_PLAIN, ch );
        ch_printf( ch, "%-25s %-14s %-14s", clan->name, clan->deity, clan->leader );
        set_char_color( AT_RED, ch );
        ch_printf( ch, " %7d\n\r", clan->pkills );
        count++;
    }

    if ( !count )
    {
	set_char_color( AT_RED, ch);
        send_to_char( "There are no clans currently formed.\n\r", ch );
	return;
    }
}

void do_cwhere( CHAR_DATA *ch, char *argument )
{
    if (IS_NPC(ch) || !ch->pcdata->clan)
    {
        send_to_char( "Huh?\n\r", ch);
        return;
    }

    do_where(ch, "_clan_");
}

void do_orders( CHAR_DATA *ch, char *argument )
{
    CLAN_DATA *order;
    int count = 0;

   /* Added displaying of mkills and mdeaths	- Brittany */
    set_char_color( AT_NOTE, ch );
    send_to_char( "Order            Deity          Leader        Mkills     Mdeaths\n\r", ch );
    for ( order = first_clan; order; order = order->next )
        if ( order->clan_type == CLAN_ORDER )
	{
	    ch_printf( ch, "%-16s %-14s %-14s %5d       %5d\n\r", order->name,
		order->deity, order->leader, order->mkills, order->mdeaths );
	    count++;
	}

    if ( !count )
    {
	send_to_char( "There are no Orders currently formed.\n\r", ch );
	return;
    }
}

void do_councils( CHAR_DATA *ch, char *argument)
{
    COUNCIL_DATA *council;

    if ( !first_council )
    {
	send_to_char( "There are no councils currently formed.\n\r", ch );
	return;
    }

    set_char_color( AT_NOTE, ch );
    send_to_char( "Name                  Head\n\r", ch);
    for ( council = first_council; council; council = council->next )
	ch_printf( ch, "%-21s %-14s\n\r", council->name, council->head);
}

void do_guilds( CHAR_DATA *ch, char *argument)
{
    CLAN_DATA *guild;
    int count = 0;

    /* Added guild mobkills/mobdeaths -- Blodkai */
    set_char_color( AT_NOTE, ch );
    send_to_char( "\n\rGuild                  Leader             Mkills      Mdeaths\n\r", ch);
    for ( guild = first_clan; guild; guild = guild->next )
        if ( guild->clan_type == CLAN_GUILD )
	{
	    ++count;
	    set_char_color( AT_YELLOW, ch );
	    ch_printf( ch, "%-20s   %-14s     %-6d       %6d\n\r", guild->name, guild->leader, guild->mkills, guild->mdeaths );
	}

    set_char_color( AT_NOTE, ch );
    if ( !count )
	send_to_char( "There are no Guilds currently formed.\n\r", ch );
    else
	ch_printf( ch, "%d guilds found.\n\r", count );
}

void do_shove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int exit_dir;
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    bool nogo;
    ROOM_INDEX_DATA *to_room;
    int percent = 0;
    int race_bonus = 0;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( IS_NPC(ch)
    || !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
    {
	send_to_char("Only deadly characters can shove.\n\r", ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Shove whom?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch);
	return;
    }

    if (victim == ch)
    {
	send_to_char("You shove yourself around, to no avail.\n\r", ch);
	return;
    }
    if ( IS_NPC(victim)
    || !IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) )
    {
	send_to_char("You can only shove deadly characters.\n\r", ch);
	return;
    }

    if ( GetMaxLevel(ch) - GetMaxLevel(victim) > 5
    ||   GetMaxLevel(victim) - GetMaxLevel(ch) > 5 )
    {
	send_to_char("There is too great an experience difference for you to even bother.\n\r", ch);
	return;
    }

    if ( (victim->position) != POS_STANDING )
    {
	act( AT_PLAIN, "$N isn't standing up.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( arg2[0] == '\0' )
    {
	send_to_char( "Shove them in which direction?\n\r", ch);
	return;
    }

    exit_dir = get_dir( arg2 );
    if ( IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    &&  get_timer(victim, TIMER_SHOVEDRAG) <= 0)
    {
	send_to_char("That character cannot be shoved right now.\n\r", ch);
	return;
    }
    victim->position = POS_SHOVE;
    nogo = FALSE;
    if ((pexit = get_exit(ch->in_room, exit_dir)) == NULL )
      nogo = TRUE;
    else
    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    && (!IS_AFFECTED(victim, AFF_PASS_DOOR)
    ||   IS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
      nogo = TRUE;
    if ( nogo )
    {
	send_to_char( "There's no exit in that direction.\n\r", ch );
        victim->position = POS_STANDING;
	return;
    }
    to_room = pexit->to_room;
    if (IS_SET(to_room->room_flags, ROOM_DEATH))
    {
      send_to_char("You cannot shove someone into a death trap.\n\r", ch);
      victim->position = POS_STANDING;
      return;
    }

    if (ch->in_room->area != to_room->area
    &&  !in_hard_range( victim, to_room->area ) )
    {
      send_to_char("That character cannot enter that area.\n\r", ch);
      victim->position = POS_STANDING;
      return;
    }

/* Check for class, assign percentage based on that. */
if (IS_ACTIVE(ch, CLASS_WARRIOR))
  percent = 70;
if (IS_ACTIVE(ch, CLASS_VAMPIRE))
  percent = 65;
if (IS_ACTIVE(ch, CLASS_RANGER))
  percent = 60;
if (IS_ACTIVE(ch, CLASS_DRUID))
  percent = 45;
if (IS_ACTIVE(ch, CLASS_CLERIC))
  percent = 35;
if (IS_ACTIVE(ch, CLASS_THIEF))
  percent = 30;
if (IS_ACTIVE(ch, CLASS_MAGE))
  percent = 15;

/* Add 3 points to percent for every str point above 15, subtract for
below 15 */

percent += ((get_curr_str(ch) - 15) * 3);

percent += (GetMaxLevel(ch) - GetMaxLevel(victim));

if (ch->race == RACE_ELF)
race_bonus = -3;

if (ch->race == RACE_DWARF)
race_bonus = 3;

if (ch->race == RACE_HALFLING)
race_bonus = -5;

if (ch->race == RACE_HALF_OGRE)
race_bonus = 5;

if (ch->race == RACE_HALF_ORC)
race_bonus = 7;

if (ch->race == RACE_HALF_ELF)
race_bonus = -2;

percent += race_bonus;

/* Debugging purposes - show percentage for testing */

/* sprintf(buf, "Shove percentage of %s = %d", ch->name, percent);
act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
*/

if (percent < number_percent( ))
{
  send_to_char("You failed.\n\r", ch);
  victim->position = POS_STANDING;
  return;
}
    act( AT_ACTION, "You shove $M.", ch, NULL, victim, TO_CHAR );
    act( AT_ACTION, "$n shoves you.", ch, NULL, victim, TO_VICT );
    move_char( victim, get_exit(ch->in_room,exit_dir), 0);
    if ( !char_died(victim) )
      victim->position = POS_STANDING;
    WAIT_STATE(ch, 12);
    /* Remove protection from shove/drag if char shoves -- Blodkai */
    if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE)
    &&   get_timer(ch, TIMER_SHOVEDRAG) <= 0 )
      add_timer( ch, TIMER_SHOVEDRAG, 10, NULL, 0 );
}

void do_drag( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int exit_dir;
    CHAR_DATA *victim;
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *to_room;
    bool nogo;
    int percent = 0;
    int race_bonus = 0;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( IS_NPC(ch)
    || !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
    {
	send_to_char("Only deadly characters can drag.\n\r", ch);
	return;
    }

    if ( arg[0] == '\0' )
    {
	send_to_char( "Drag whom?\n\r", ch);
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "They aren't here.\n\r", ch);
	return;
    }

    if ( victim == ch )
    {
	send_to_char("You take yourself by the scruff of your neck, but go nowhere.\n\r", ch);
	return;
    }

    if ( IS_NPC(victim)
         || !IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) )
    {
	send_to_char("You can only drag deadly characters.\n\r", ch);
	return;
    }

    if ( victim->fighting )
    {
        send_to_char( "You try, but can't get close enough.\n\r", ch);
        return;
    }

    if ( arg2[0] == '\0' )
    {
	send_to_char( "Drag them in which direction?\n\r", ch);
	return;
    }

    if ( GetMaxLevel(ch) - GetMaxLevel(victim) > 5
    ||   GetMaxLevel(victim) - GetMaxLevel(ch) > 5 )
    {
        send_to_char("There is too great an experience difference for you to even bother.\n\r", ch);
        return;
    }

    exit_dir = get_dir( arg2 );

    if ( IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    &&   get_timer( victim, TIMER_SHOVEDRAG ) <= 0)
    {
	send_to_char("That character cannot be dragged right now.\n\r", ch);
	return;
    }

    nogo = FALSE;
    if ((pexit = get_exit(ch->in_room, exit_dir)) == NULL )
      nogo = TRUE;
    else
    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    && (!IS_AFFECTED(victim, AFF_PASS_DOOR)
    ||   IS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
      nogo = TRUE;
    if ( nogo )
    {
	send_to_char( "There's no exit in that direction.\n\r", ch );
	return;
    }

    to_room = pexit->to_room;
    if (IS_SET(to_room->room_flags, ROOM_DEATH))
    {
      send_to_char("You cannot drag someone into a death trap.\n\r", ch);
      return;
    }

    if (ch->in_room->area != to_room->area
    && !in_hard_range( victim, to_room->area ) )
    {
      send_to_char("That character cannot enter that area.\n\r", ch);
      victim->position = POS_STANDING;
      return;
    }

/* Check for class, assign percentage based on that. */
if (IS_ACTIVE(ch, CLASS_WARRIOR))
  percent = 70;
if (IS_ACTIVE(ch, CLASS_VAMPIRE))
  percent = 65;
if (IS_ACTIVE(ch, CLASS_RANGER))
  percent = 60;
if (IS_ACTIVE(ch, CLASS_DRUID))
  percent = 45;
if (IS_ACTIVE(ch, CLASS_CLERIC))
  percent = 35;
if (IS_ACTIVE(ch, CLASS_THIEF))
  percent = 30;
if (IS_ACTIVE(ch, CLASS_MAGE))
  percent = 15;

/* Add 3 points to percent for every str point above 15, subtract for
below 15 */

percent += ((get_curr_str(ch) - 15) * 3);

percent += (GetMaxLevel(ch) - GetMaxLevel(victim));

if (ch->race == RACE_ELF)
race_bonus = -3;

if (ch->race == RACE_DWARF)
race_bonus = 3;

if (ch->race == RACE_HALFLING)
race_bonus = -5;

if (ch->race == RACE_HALF_OGRE)
race_bonus = 5;

if (ch->race == RACE_HALF_ORC)
race_bonus = 7;

if (ch->race == RACE_HALF_ELF)
race_bonus = -2;

percent += race_bonus;
/*
sprintf(buf, "Drag percentage of %s = %d", ch->name, percent);
act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
*/
if (percent < number_percent( ))
{
  send_to_char("You failed.\n\r", ch);
  victim->position = POS_STANDING;
  return;
}
    if ( victim->position < POS_STANDING )
    {
	sh_int temp;

	temp = victim->position;
	victim->position = POS_DRAG;
	act( AT_ACTION, "You drag $M into the next room.", ch, NULL, victim, TO_CHAR );
	act( AT_ACTION, "$n grabs your hair and drags you.", ch, NULL, victim, TO_VICT );
	move_char( victim, get_exit(ch->in_room,exit_dir), 0);
	if ( !char_died(victim) )
	  victim->position = temp;
/* Move ch to the room too.. they are doing dragging - Scryn */
	move_char( ch, get_exit(ch->in_room,exit_dir), 0);
	WAIT_STATE(ch, PULSE_VIOLENCE);
	return;
    }
    send_to_char("You cannot do that to someone who is standing.\n\r", ch);
    return;
}

