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
 * 			Table load/save Module				    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: tables.c,v 1.40 2004/04/06 22:00:11 dotd Exp $";*/

#include <time.h>
#include <stdio.h>
#include <string.h>
#include "mud.h"

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
#define ACMD DECLARE_DO_FUN
#define TABLES_C_ALL
#include "tables.h"
#undef TABLES_C_ALL
#undef ACMD

/*
 * Spell functions.
 * Defined in magic.c.
 */
#define ACMD DECLARE_SPELL_FUN
#define MAGIC_C_ALL
#include "magic.h"
#undef MAGIC_C_ALL
#undef ACMD

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


/* global variables */
int top_sn;
int top_herb;

SKILLTYPE *		skill_table	[MAX_SKILL];
struct	class_type *	class_table	[REAL_MAX_CLASS];
char *			title_table	[REAL_MAX_CLASS]
					[MAX_LEVEL+1]
					[2];
SKILLTYPE *		herb_table	[MAX_HERB];
LANG_DATA *		first_lang;
LANG_DATA *		last_lang;


char * const skill_tname[] =
{ "unknown", "Spell", "Skill", "Weapon", "Tongue", "Herb", "Lore", "PsiSpell" };


int get_skill_tname(char *tname)
{
    int x;

    for (x=1;x<SKILL_MAXTYPE;x++)
        if (!str_cmp(tname, skill_tname[x]))
            return x;

    return SKILL_UNKNOWN;
}

#define ACMD(func) if (!str_cmp( name, #func )) return func;
SPELL_FUN *spell_function( char *name )
{
    if ( !str_cmp( name, "reserved" ))		     return NULL;
    if ( !str_cmp( name, "spell_null" ))	     return spell_null;

#include "magic.h"

    return spell_notfound;
}

static DO_FUN *skill_function_a( const char *name )
{
#define TABLES_C_A
#include "tables.h"
#undef TABLES_C_A
    return skill_notfound;
}
static DO_FUN *skill_function_b( const char *name )
{
#define TABLES_C_B
#include "tables.h"
#undef TABLES_C_B
    return skill_notfound;
}
static DO_FUN *skill_function_c( const char *name )
{
#define TABLES_C_C
#include "tables.h"
#undef TABLES_C_C
    return skill_notfound;
}
static DO_FUN *skill_function_d( const char *name )
{
#define TABLES_C_D
#include "tables.h"
#undef TABLES_C_D
    return skill_notfound;
}
static DO_FUN *skill_function_e( const char *name )
{
#define TABLES_C_E
#include "tables.h"
#undef TABLES_C_E
    return skill_notfound;
}
static DO_FUN *skill_function_f( const char *name )
{
#define TABLES_C_F
#include "tables.h"
#undef TABLES_C_F
    return skill_notfound;
}
static DO_FUN *skill_function_g( const char *name )
{
#define TABLES_C_G
#include "tables.h"
#undef TABLES_C_G
    return skill_notfound;
}
static DO_FUN *skill_function_h( const char *name )
{
#define TABLES_C_H
#include "tables.h"
#undef TABLES_C_H
    return skill_notfound;
}
static DO_FUN *skill_function_i( const char *name )
{
#define TABLES_C_I
#include "tables.h"
#undef TABLES_C_I
    return skill_notfound;
}
static DO_FUN *skill_function_j( const char *name )
{
#define TABLES_C_J
#include "tables.h"
#undef TABLES_C_J
    return skill_notfound;
}
static DO_FUN *skill_function_k( const char *name )
{
#define TABLES_C_K
#include "tables.h"
#undef TABLES_C_K
    return skill_notfound;
}
static DO_FUN *skill_function_l( const char *name )
{
#define TABLES_C_L
#include "tables.h"
#undef TABLES_C_L
    return skill_notfound;
}
static DO_FUN *skill_function_m( const char *name )
{
#define TABLES_C_M
#include "tables.h"
#undef TABLES_C_M
    return skill_notfound;
}
static DO_FUN *skill_function_n( const char *name )
{
#define TABLES_C_N
#include "tables.h"
#undef TABLES_C_N
    return skill_notfound;
}
static DO_FUN *skill_function_o( const char *name )
{
#define TABLES_C_O
#include "tables.h"
#undef TABLES_C_O
    return skill_notfound;
}
static DO_FUN *skill_function_p( const char *name )
{
#define TABLES_C_P
#include "tables.h"
#undef TABLES_C_P
    return skill_notfound;
}
static DO_FUN *skill_function_q( const char *name )
{
#define TABLES_C_Q
#include "tables.h"
#undef TABLES_C_Q
    return skill_notfound;
}
static DO_FUN *skill_function_r( const char *name )
{
#define TABLES_C_R
#include "tables.h"
#undef TABLES_C_R
    return skill_notfound;
}
static DO_FUN *skill_function_s( const char *name )
{
#define TABLES_C_S
#include "tables.h"
#undef TABLES_C_S
    return skill_notfound;
}
static DO_FUN *skill_function_t( const char *name )
{
#define TABLES_C_T
#include "tables.h"
#undef TABLES_C_T
    return skill_notfound;
}
static DO_FUN *skill_function_u( const char *name )
{
#define TABLES_C_U
#include "tables.h"
#undef TABLES_C_U
    return skill_notfound;
}
static DO_FUN *skill_function_v( const char *name )
{
#define TABLES_C_V
#include "tables.h"
#undef TABLES_C_V
    return skill_notfound;
}
static DO_FUN *skill_function_w( const char *name )
{
#define TABLES_C_W
#include "tables.h"
#undef TABLES_C_W
    return skill_notfound;
}
static DO_FUN *skill_function_x( const char *name )
{
#define TABLES_C_X
#include "tables.h"
#undef TABLES_C_X
    return skill_notfound;
}
static DO_FUN *skill_function_y( const char *name )
{
#define TABLES_C_Y
#include "tables.h"
#undef TABLES_C_Y
    return skill_notfound;
}
static DO_FUN *skill_function_z( const char *name )
{
#define TABLES_C_Z
#include "tables.h"
#undef TABLES_C_Z
    return skill_notfound;
}
#undef ACMD

DO_FUN *skill_function( char *name )
{
    switch( name[3] )
    {
    case 'a': return(skill_function_a(name)); break;
    case 'b': return(skill_function_b(name)); break;
    case 'c': return(skill_function_c(name)); break;
    case 'd': return(skill_function_d(name)); break;
    case 'e': return(skill_function_e(name)); break;
    case 'f': return(skill_function_f(name)); break;
    case 'g': return(skill_function_g(name)); break;
    case 'h': return(skill_function_h(name)); break;
    case 'i': return(skill_function_i(name)); break;
    case 'j': return(skill_function_j(name)); break;
    case 'k': return(skill_function_k(name)); break;
    case 'l': return(skill_function_l(name)); break;
    case 'm': return(skill_function_m(name)); break;
    case 'n': return(skill_function_n(name)); break;
    case 'o': return(skill_function_o(name)); break;
    case 'p': return(skill_function_p(name)); break;
    case 'q': return(skill_function_q(name)); break;
    case 'r': return(skill_function_r(name)); break;
    case 's': return(skill_function_s(name)); break;
    case 't': return(skill_function_t(name)); break;
    case 'u': return(skill_function_u(name)); break;
    case 'v': return(skill_function_v(name)); break;
    case 'w': return(skill_function_w(name)); break;
    case 'x': return(skill_function_x(name)); break;
    case 'y': return(skill_function_y(name)); break;
    case 'z': return(skill_function_z(name)); break;
    }
    return skill_notfound;
}

char *spell_name( SPELL_FUN *spell )
{
    if ( spell == spell_smaug )		    return "spell_smaug";
    if ( spell == spell_null )		    return "spell_null";

#define ACMD(func) if ( spell == func ) return #func
#include "magic.h"
#undef ACMD

    return "reserved";
}

char *skill_name( DO_FUN *skill )
{
#define ACMD(func) if ( skill == func ) return #func
#define TABLES_C_ALL
#include "tables.h"
#undef TABLES_C_ALL
#undef ACMD

    return "reserved";
}

bool load_class_file( const char *fname )
{
    char buf[MAX_STRING_LENGTH];
    const char *word = NULL;
    bool fMatch = FALSE;
    struct class_type *clt;
    sh_int cl = CLASS_NONE;
    int tlev = 0, i=0;
    FILE *fp;

    sprintf( buf, "%s%s", CLASS_DIR, fname );
    if ( ( fp = fopen( buf, "r" ) ) == NULL )
    {
	perror( buf );
	return FALSE;
    }

    CREATE( clt, struct class_type, 1 );

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
	    KEY( "AttrPrime",	clt->attr_prime,	fread_number( fp )	);
	    KEY( "AttrString",	clt->attr_string,	fread_string( fp )	);
	    break;

	case 'C':
	    KEY( "Class",	cl,			fread_number( fp )	);
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		FCLOSE( fp );
		if ( cl < 0 || cl >= MAX_CLASS || !clt->who_name )
		{
                    boot_log("Load_class_file: Class (%s) bad/not found (%d)",
                             clt->who_name ? clt->who_name : "name not found", cl );
		    if ( clt->who_name )
			STRFREE( clt->who_name );
		    DISPOSE( clt );
		    return FALSE;
		}
                class_table[cl] = clt;
                boot_log( "Loaded class '%s'", clt->who_name );
                return TRUE;
	    }
	    KEY( "ExpBase",	clt->exp_base,	fread_number( fp )	);
	    KEY( "ExpPower",	clt->exp_power,	fread_number( fp )	);
	    break;

	case 'G':
	    KEY( "Guild",	clt->guild,		fread_number( fp )	);
	    break;

	case 'H':
	    KEY( "HpMax",	clt->hp_max,		fread_number( fp )	);
	    KEY( "HpMin",	clt->hp_min,		fread_number( fp )	);
	    KEY( "HpCLev",	clt->hp_const_lev,	fread_number( fp ) );
	    KEY( "HpCAdd",	clt->hp_const_add,	fread_number( fp ) );
	    break;

	case 'M':
	    KEY( "Mana",	clt->fMana,		fread_number( fp )	);
	    break;

	case 'N':
	    KEY( "Name",	clt->who_name,	fread_string( fp )	);
	    break;

	case 'S':
	    if ( !str_cmp( word, "Skill" ) )
	    {
                char *ln;
		int sn, lev=0, adp=0, mana=-1, beats=-1;

		word = fread_word( fp );

		ln = fread_line( fp );

		sscanf( ln, "%d %d %d %d", &lev, &adp, &mana, &beats );

		if ( lev == 0 && adp == 0 )
		{
		    lev = LEVEL_IMMORTAL;
		    adp = 95;
		}
		sn = skill_lookup( word );
		if ( cl < 0 || cl >= MAX_CLASS )
		{
		    boot_log( "load_class_file: Skill %s -- class bad/not found (%d)", word, cl );
                }
		else
		if ( !IS_VALID_SN(sn) )
		{
                    boot_log( "load_class_file: Skill %s unknown", word );
		}
		else
		{
		    skill_table[sn]->skill_level[cl] = lev;
		    skill_table[sn]->skill_adept[cl] = adp;
		    if ( mana != -1 )
			skill_table[sn]->class_mana[cl] = mana;
		    if ( beats != -1 )
			skill_table[sn]->class_beats[cl] = beats;
		}
		fMatch = TRUE;
		break;
	    }
	    KEY( "Skilladept",	clt->skill_adept,	fread_number( fp )	);
	    break;

	case 'T':
	    if ( !str_cmp( word, "Title" ) )
	    {
		if ( cl < 0 || cl >= MAX_CLASS )
		{
		    char *tmp;

                    boot_log( "load_class_file: Title -- clt bad/not found (%d)", cl );
		    tmp = fread_string_nohash( fp );
		    DISPOSE( tmp );
		    tmp = fread_string_nohash( fp );
		    DISPOSE( tmp );
		}
		else
		if ( tlev < MAX_LEVEL+1 )
		{
		    title_table[cl][tlev][0] = fread_string_nohash( fp );
		    title_table[cl][tlev][1] = fread_string_nohash( fp );
		    ++tlev;
		}
		else
		{
		    char *tmp;

		    boot_log( "load_class_file: Too many titles" );
		    tmp = fread_string_nohash( fp );
		    DISPOSE( tmp );
		    tmp = fread_string_nohash( fp );
		    DISPOSE( tmp );
		}
		fMatch = TRUE;
		break;
	    }
	    KEY( "Thac0",	clt->thac0_00,	fread_number( fp )	);
	    KEY( "Thac32",	clt->thac0_32,	fread_number( fp )	);
	    break;

	case 'W':
	    KEY( "Weapon",	clt->weapon,	fread_number( fp )	);
	    break;
	}
	if ( !fMatch )
	{
            boot_log( "load_class_file: no match: %s", word );
            fread_to_eol(fp);
	}
    }
    for (i=tlev;i<MAX_LEVEL;i++) {
      title_table[cl][i][0] = str_dup("UNKNOWN");
      title_table[cl][i][1] = str_dup("UNKNOWN");
    }
    return FALSE;
}

/*
 * Load in all the class files.
 */
void load_classes( )
{
    FILE *fpList;
    const char *filename;
    char classlist[256];

    sprintf( classlist, "%s%s", CLASS_DIR, CLASS_LIST );
    if ( ( fpList = fopen( classlist, "r" ) ) == NULL )
    {
        perror( classlist );
        exit( 1 );
    }

    for ( ; ; )
    {
        filename = feof( fpList ) ? "$" : fread_word( fpList );
        if ( filename[0] == '$' )
            break;

        if ( !load_class_file( filename ) )
        {
            boot_log( "Cannot load class file: %s", filename );
        }
    }

    FCLOSE( fpList );
    return;
}

void write_class_file( sh_int cl )
{
    FILE *fpout;
    char buf[MAX_STRING_LENGTH];
    char filename[MAX_INPUT_LENGTH];
    struct class_type *clt = class_table[cl];
    int x, y;

    sprintf( filename, "%s%s.class", CLASS_DIR, clt->who_name );
    if ( (fpout=fopen(filename, "w")) == NULL )
    {
	sprintf( buf, "Cannot open: %s for writing", filename );
	bug( buf, 0 );
	return;
    }
    fprintf( fpout, "Name        %s~\n",	clt->who_name		);
    fprintf( fpout, "Class       %d\n",		cl			);
    fprintf( fpout, "AttrPrime   %d\n",		clt->attr_prime	        );
    if (clt->attr_string)
	fprintf( fpout, "AttrString  %s~\n",       	clt->attr_string	);
    fprintf( fpout, "Weapon      %d\n",		clt->weapon		);
    fprintf( fpout, "Guild       %d\n",		clt->guild		);
    fprintf( fpout, "Skilladept  %d\n",		clt->skill_adept	);
    fprintf( fpout, "Thac0       %d\n",		clt->thac0_00		);
    fprintf( fpout, "Thac32      %d\n",		clt->thac0_32		);
    fprintf( fpout, "Hpmin       %d\n",		clt->hp_min		);
    fprintf( fpout, "Hpmax       %d\n",		clt->hp_max		);
    fprintf( fpout, "HpCLev      %d\n",		clt->hp_const_lev	);
    fprintf( fpout, "HpCAdd      %d\n",		clt->hp_const_add	);
    fprintf( fpout, "Mana        %d\n",		clt->fMana		);
    fprintf( fpout, "ExpBase     %ld\n",	clt->exp_base		);
    fprintf( fpout, "ExpPower    %ld\n",	clt->exp_power	);
    for ( x = 0; x < top_sn; x++ )
    {
        if ( !skill_table[x]->name || skill_table[x]->name[0] == '\0' )
            break;
        y = skill_table[x]->skill_level[cl];
        if ( y < LEVEL_IMMORTAL && y > 0 )
            fprintf( fpout, "Skill '%s' %d %d %d %d\n",
                     skill_table[x]->name,
                     skill_table[x]->skill_level[cl],
		     skill_table[x]->skill_adept[cl],
		     skill_table[x]->class_mana[cl],
		     skill_table[x]->class_beats[cl]);
    }
    for ( x = 0; x <= MAX_LEVEL; x++ )
        fprintf( fpout, "Title\n%s~\n%s~\n",
                 title_table[cl][x][0], title_table[cl][x][1] );
    fprintf( fpout, "End\n" );
    FCLOSE( fpout );
}

/*
 * Function used by qsort to sort skills
 */
int skill_comp( SKILLTYPE **sk1, SKILLTYPE **sk2 )
{
    SKILLTYPE *skill1 = (*sk1);
    SKILLTYPE *skill2 = (*sk2);

    if ( !skill1 && skill2 )
	return 1;
    if ( skill1 && !skill2 )
	return -1;
    if ( !skill1 && !skill2 )
	return 0;
    if ( skill1->type < skill2->type )
	return -1;
    if ( skill1->type > skill2->type )
	return 1;
    return strcmp( skill1->name, skill2->name );
}

int skill_comp_sn( SKILLTYPE **sk1, SKILLTYPE **sk2 )
{
    SKILLTYPE *skill1 = (*sk1);
    SKILLTYPE *skill2 = (*sk2);

    if ( !skill1 && skill2 )
	return 1;
    if ( skill1 && !skill2 )
	return -1;
    if ( !skill1 && !skill2 )
	return 0;
    if ( skill1->type < skill2->type )
	return -1;
    if ( skill1->type > skill2->type )
	return 1;
    return ( skill1->slot>skill2->slot );
}

/*
 * Sort the skill table with qsort
 */
void sort_skill_table()
{
    log_string_plus( "Sorting skill table...", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO );
    qsort( &skill_table[1], top_sn-1, sizeof( SKILLTYPE * ),
		(int(*)(const void *, const void *)) skill_comp );
}
void sort_skill_table_sn()
{
    log_string_plus( "Sorting skill table by sn...", LOG_NORMAL, LEVEL_LOG_CSET, SEV_INFO );
    qsort( &skill_table[1], top_sn-1, sizeof( SKILLTYPE * ),
		(int(*)(const void *, const void *)) skill_comp_sn );
}

/*
 * Write skill data to a file
 */
#if defined(USE_DB) || defined(START_DB)
void save_skill_table()
{
    db_insert_skills();
}

void save_herb_table()
{
    db_insert_herbs();
}

void save_commands()
{
    db_insert_commands();
}

void save_socials()
{
    db_insert_socials();
}

#else
void fwrite_skill( FILE *fpout, SKILLTYPE *skill )
{
    SMAUG_AFF *aff;

    fprintf( fpout, "Name         %s~\n",	skill->name	);
    fprintf( fpout, "Type         %s\n",	skill_tname[skill->type] );
    fprintf( fpout, "Flags        %d\n",	skill->flags	);

    /*
    skill->dam_type = SPELL_DAMAGE(skill);
    skill->act_type = SPELL_ACTION(skill);
    skill->power_type = SPELL_POWER(skill);
    skill->class_type = SPELL_CLASS(skill);
    REMOVE_BIT(skill->flags, BV00);
    REMOVE_BIT(skill->flags, BV01);
    REMOVE_BIT(skill->flags, BV02);
    REMOVE_BIT(skill->flags, BV03);
    REMOVE_BIT(skill->flags, BV04);
    REMOVE_BIT(skill->flags, BV05);
    REMOVE_BIT(skill->flags, BV06);
    REMOVE_BIT(skill->flags, BV07);
    REMOVE_BIT(skill->flags, BV08);
    REMOVE_BIT(skill->flags, BV09);
    REMOVE_BIT(skill->flags, BV10);
    */

    if ( SPELL_DAMAGE(skill) )
        fprintf( fpout, "DamType      %d\n",	skill->dam_type );
    if ( SPELL_ACTION(skill) )
        fprintf( fpout, "ActType      %d\n",	skill->act_type );
    if ( SPELL_POWER(skill) )
        fprintf( fpout, "PowerType    %d\n",	skill->power_type );
    if ( SPELL_CLASS(skill) )
        fprintf( fpout, "ClassType    %d\n",	skill->class_type );
    if ( SPELL_SAVE(skill) )
        fprintf( fpout, "SaveType     %d\n",	skill->save_type);

    if ( skill->target )
        fprintf( fpout, "Target       %d\n",	skill->target	);
    if ( skill->minimum_position )
        fprintf( fpout, "Minpos       %d\n",	skill->minimum_position );
    if ( skill->slot )
        fprintf( fpout, "Slot         %d\n",	skill->slot	);
    if ( skill->min_mana )
        fprintf( fpout, "Mana         %d\n",	skill->min_mana );
    if ( skill->beats )
	fprintf( fpout, "Rounds       %d\n",	skill->beats	);
    if ( skill->guild != -1 )
        fprintf( fpout, "Guild        %d\n",	skill->guild	);
    if ( skill->skill_fun )
        fprintf( fpout, "Code         %s\n",	skill_name(skill->skill_fun) );
    else
        if ( skill->spell_fun )
            fprintf( fpout, "Code         %s\n",	spell_name(skill->spell_fun) );
    fprintf( fpout, "Dammsg       %s~\n",	skill->noun_damage );
    if ( skill->msg_off && skill->msg_off[0] != '\0' )
        fprintf( fpout, "Wearoff      %s~\n",	skill->msg_off	);
    if ( skill->msg_off_room && skill->msg_off_room[0] != '\0' )
        fprintf( fpout, "Wearoffroom  %s~\n",	skill->msg_off_room );
    if ( skill->msg_off_soon && skill->msg_off_soon[0] != '\0' )
        fprintf( fpout, "Wearoffsoon  %s~\n",	skill->msg_off_soon );
    if ( skill->msg_off_soon_room && skill->msg_off_soon_room[0] != '\0' )
        fprintf( fpout, "WearoffsoonR %s~\n",	skill->msg_off_soon_room );

    if ( skill->hit_char && skill->hit_char[0] != '\0' )
        fprintf( fpout, "Hitchar      %s~\n",	skill->hit_char );
    if ( skill->hit_vict && skill->hit_vict[0] != '\0' )
        fprintf( fpout, "Hitvict      %s~\n",	skill->hit_vict );
    if ( skill->hit_room && skill->hit_room[0] != '\0' )
        fprintf( fpout, "Hitroom      %s~\n",	skill->hit_room );

    if ( skill->miss_char && skill->miss_char[0] != '\0' )
        fprintf( fpout, "Misschar     %s~\n",	skill->miss_char );
    if ( skill->miss_vict && skill->miss_vict[0] != '\0' )
        fprintf( fpout, "Missvict     %s~\n",	skill->miss_vict );
    if ( skill->miss_room && skill->miss_room[0] != '\0' )
        fprintf( fpout, "Missroom     %s~\n",	skill->miss_room );

    if ( skill->die_char && skill->die_char[0] != '\0' )
        fprintf( fpout, "Diechar      %s~\n",	skill->die_char );
    if ( skill->die_vict && skill->die_vict[0] != '\0' )
        fprintf( fpout, "Dievict      %s~\n",	skill->die_vict );
    if ( skill->die_room && skill->die_room[0] != '\0' )
        fprintf( fpout, "Dieroom      %s~\n",	skill->die_room );

    if ( skill->imm_char && skill->imm_char[0] != '\0' )
        fprintf( fpout, "Immchar      %s~\n",	skill->imm_char );
    if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
        fprintf( fpout, "Immvict      %s~\n",	skill->imm_vict );
    if ( skill->imm_room && skill->imm_room[0] != '\0' )
        fprintf( fpout, "Immroom      %s~\n",	skill->imm_room );

    if ( skill->abs_char && skill->abs_char[0] != '\0' )
        fprintf( fpout, "Abschar      %s~\n",	skill->abs_char );
    if ( skill->abs_vict && skill->abs_vict[0] != '\0' )
        fprintf( fpout, "Absvict      %s~\n",	skill->abs_vict );
    if ( skill->abs_room && skill->abs_room[0] != '\0' )
        fprintf( fpout, "Absroom      %s~\n",	skill->abs_room );

    if ( skill->corpse_string && skill->corpse_string[0] != '\0' )
    {
        fprintf( fpout, "CorpseStr    %s~\n",	skill->corpse_string );
        fprintf( fpout, "CorpseStg    %d\n",	skill->corpse_stage );
    }
    if ( skill->dice && skill->dice[0] != '\0' )
        fprintf( fpout, "Dice         %s~\n",	skill->dice );
    if ( skill->value )
        fprintf( fpout, "Value        %d\n",	skill->value );
    if ( skill->difficulty )
        fprintf( fpout, "Difficulty   %d\n",	skill->difficulty );
    if ( skill->participants )
        fprintf( fpout, "Participants %d\n",	skill->participants );

    if ( skill->part_start_char && skill->part_start_char[0] != '\0')
        fprintf( fpout, "PartStartCh  %s~\n",	skill->part_start_char );
    if ( skill->part_start_room && skill->part_start_room[0] != '\0')
        fprintf( fpout, "PartStartRm  %s~\n",	skill->part_start_room );

    if ( skill->part_end_char && skill->part_end_char[0] != '\0')
        fprintf( fpout, "PartEndCh    %s~\n",	skill->part_end_char );
    if ( skill->part_end_vict && skill->part_end_vict[0] != '\0')
        fprintf( fpout, "PartEndVict  %s~\n",	skill->part_end_vict );
    if ( skill->part_end_room && skill->part_end_room[0] != '\0')
        fprintf( fpout, "PartEndRm    %s~\n",	skill->part_end_room );
    if ( skill->part_end_caster && skill->part_end_caster[0] != '\0')
        fprintf( fpout, "PartEndCast  %s~\n",	skill->part_end_caster );

    if ( skill->part_miss_char && skill->part_miss_char[0] != '\0')
        fprintf( fpout, "PartMissCh   %s~\n",	skill->part_miss_char );
    if ( skill->part_miss_room && skill->part_miss_room[0] != '\0')
        fprintf( fpout, "PartMissRm   %s~\n",	skill->part_miss_room );

    if ( skill->part_abort_char && skill->part_abort_char[0] != '\0')
        fprintf( fpout, "PartAbortCh  %s~\n",	skill->part_abort_char );

    if ( skill->components && skill->components[0] != '\0' )
        fprintf( fpout, "Components   %s~\n",	skill->components );
    if ( skill->teachers && skill->teachers[0] != '\0' )
        fprintf( fpout, "Teachers     %s~\n",	skill->teachers );
    for ( aff = skill->affects; aff; aff = aff->next )
    {
        if (aff->location == APPLY_IMMUNESPELL && IS_VALID_SN(atoi(aff->modifier)))
            fprintf( fpout, "Affect       '%s' %d '%d' %d\n",
                     aff->duration, aff->location, skill_table[atoi(aff->modifier)]->slot, aff->bitvector );
        else
            fprintf( fpout, "Affect       '%s' %d '%s' %d\n",
                     aff->duration, aff->location, aff->modifier, aff->bitvector );
    }

    if ( skill->type != SKILL_HERB )
    {
        int y;
        int min = 1000;

        for ( y = 0; y < MAX_CLASS; y++ )
            if ( skill->skill_level[y] < min )
                min = skill->skill_level[y];

        fprintf( fpout, "Minlevel     %d\n",	min		);
    }
    fprintf( fpout, "End\n\n" );
}

/*
 * Save the skill table to disk
 */
void save_skill_table()
{
    int x;
    FILE *fpout;

    if ( (fpout=fopen( SKILL_FILE, "w" )) == NULL )
    {
	bug( "Cannot open skills.dat for writting" );
	perror( SKILL_FILE );
	return;
    }

    for ( x = 0; x < top_sn; x++ )
    {
	if ( !skill_table[x]->name || skill_table[x]->name[0] == '\0' )
	   break;
	fprintf( fpout, "#SKILL\n" );
	fwrite_skill( fpout, skill_table[x] );
    }
    fprintf( fpout, "#END\n" );
    FCLOSE( fpout );
}

/*
 * Save the herb table to disk
 */
void save_herb_table()
{
    int x;
    FILE *fpout;

    if ( (fpout=fopen( HERB_FILE, "w" )) == NULL )
    {
	bug( "Cannot open herbs.dat for writting" );
	perror( HERB_FILE );
	return;
    }

    for ( x = 0; x < top_herb; x++ )
    {
	if ( !herb_table[x]->name || herb_table[x]->name[0] == '\0' )
	   break;
	fprintf( fpout, "#HERB\n" );
	fwrite_skill( fpout, herb_table[x] );
    }
    fprintf( fpout, "#END\n" );
    FCLOSE( fpout );
}

/*
 * Save the socials to disk
 */
void save_socials()
{
    FILE *fpout;
    SOCIALTYPE *social;
    int x;

    if ( (fpout=fopen( SOCIAL_FILE, "w" )) == NULL )
    {
	bug( "Cannot open socials.dat for writting" );
	perror( SOCIAL_FILE );
	return;
    }

    for ( x = 0; x < 27; x++ )
    {
	for ( social = social_index[x]; social; social = social->next )
	{
	    if ( !social->name || social->name[0] == '\0' )
	    {
		bug( "Save_socials: blank social in hash bucket %d", x );
		continue;
	    }
	    fprintf( fpout, "#SOCIAL\n" );
            fprintf( fpout, "Name        %s~\n",	social->name );
            if ( social->position != POS_RESTING )
                fprintf( fpout, "Position    %d\n",     social->position );
            if ( social->mob_response )
                fprintf( fpout, "MobResponse %s~\n",    social->mob_response );
	    if ( social->char_no_arg )
		fprintf( fpout, "CharNoArg   %s~\n",	social->char_no_arg );
	    else
	        bug( "Save_socials: NULL char_no_arg in hash bucket %d", x );
	    if ( social->others_no_arg )
		fprintf( fpout, "OthersNoArg %s~\n",	social->others_no_arg );
	    if ( social->char_found )
		fprintf( fpout, "CharFound   %s~\n",	social->char_found );
	    if ( social->others_found )
		fprintf( fpout, "OthersFound %s~\n",	social->others_found );
	    if ( social->vict_found )
		fprintf( fpout, "VictFound   %s~\n",	social->vict_found );
	    if ( social->char_auto )
		fprintf( fpout, "CharAuto    %s~\n",	social->char_auto );
	    if ( social->others_auto )
		fprintf( fpout, "OthersAuto  %s~\n",	social->others_auto );
	    if ( social->not_found )
		fprintf( fpout, "NotFound    %s~\n",	social->not_found );
	    fprintf( fpout, "End\n\n" );
	}
    }
    fprintf( fpout, "#END\n" );
    FCLOSE( fpout );
}

/*
 * Save the commands to disk
 */
void save_commands()
{
    FILE *fpout;
    CMDTYPE *command;
    int x;

    if ( (fpout=fopen( COMMAND_FILE, "w" )) == NULL )
    {
	bug( "Cannot open commands.dat for writing" );
	perror( COMMAND_FILE );
	return;
    }

    for ( x = 0; x < 126; x++ )
    {
	for ( command = command_hash[x]; command; command = command->next )
	{
	    if ( !command->name || command->name[0] == '\0' )
	    {
		bug( "Save_commands: blank command in hash bucket %d", x );
		continue;
	    }
	    fprintf( fpout, "#COMMAND\n" );
	    fprintf( fpout, "Name        %s~\n", command->name		);
	    fprintf( fpout, "Code        %s\n",	 skill_name(command->do_fun) );
            if (command->position!=0)
                fprintf( fpout, "Position    %d\n",	 command->position	);
            if (command->level!=0)
                fprintf( fpout, "Level       %d\n",	 command->level		);
            if (command->log!=0)
                fprintf( fpout, "Log         %d\n",	 command->log		);
            if (command->flags!=0)
                fprintf( fpout, "Flags       %d\n",	 command->flags  	);
	    fprintf( fpout, "End\n\n" );
	}
    }
    fprintf( fpout, "#END\n" );
    FCLOSE( fpout );
}
#endif

#ifdef USE_DB

void load_skill_table()
{
    db_load_skill_table();
}

void load_herb_table()
{
    db_load_herb_table();
}

void load_commands()
{
    db_load_commands();
}

void load_socials()
{
    db_load_socials();
}

#else
SKILLTYPE *fread_skill( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    const char *word = NULL;
    bool fMatch = FALSE;
    SKILLTYPE *skill;
    int x;

    CREATE( skill, SKILLTYPE, 1 );
    for ( x = 0; x < MAX_CLASS; x++ )
    {
	skill->skill_level[x] = LEVEL_IMMORTAL;
	skill->skill_adept[x] = 95;
	skill->class_mana[x] = -1;
        skill->class_beats[x] = -1;
    }
    skill->guild = -1;

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
	    KEY( "Abschar",	skill->abs_char,	fread_string_nohash( fp ) );
            KEY( "Absroom",	skill->abs_room,	fread_string_nohash( fp ) );
	    KEY( "Absvict",	skill->abs_vict,	fread_string_nohash( fp ) );
	    KEY( "ActType",	skill->act_type,   	fread_number( fp ) );
	    if ( !str_cmp( word, "Affect" ) )
	    {
		SMAUG_AFF *aff;

		CREATE( aff, SMAUG_AFF, 1 );
		aff->duration = str_dup( fread_word( fp ) );
                aff->location = fread_number( fp );
                if (aff->location == APPLY_IMMUNESPELL)
                {
                    sprintf(buf, "%d", slot_lookup( atoi( fread_word( fp ) ) ));
                    aff->modifier = str_dup( buf );
                }
                else
                    aff->modifier = str_dup( fread_word( fp ) );
		aff->bitvector = fread_number( fp );
		aff->next = skill->affects;
		skill->affects = aff;
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'C':
	    if ( !str_cmp( word, "Class" ) )
	    {
/*
		class_index clt = fread_number( fp );

		skill->skill_level[clt] = fread_number( fp );
		skill->skill_adept[clt] = fread_number( fp );
		fMatch = TRUE;
		break;
*/
		bug("Throwback... skills.dat has Class entry...");
	    }
            KEY( "ClassType",	skill->class_type,   	fread_number( fp ) );
	    if ( !str_cmp( word, "Code" ) )
	    {
		SPELL_FUN *spellfun;
		DO_FUN	  *dofun;
		char	  *w = fread_word(fp);

		fMatch = TRUE;
		if ( (spellfun=spell_function(w)) != spell_notfound )
		   skill->spell_fun = spellfun;
		else
		if ( (dofun=skill_function(w)) != skill_notfound )
		   skill->skill_fun = dofun;
		else
		{
		   sprintf( buf, "fread_skill: unknown skill/spell %s", w );
		   bug( buf, 0 );
		   skill->spell_fun = spell_null;
		}
		break;
	    }
	    KEY( "Code",	skill->spell_fun, spell_function(fread_word(fp)) );
	    KEY( "Components",	skill->components,	fread_string_nohash( fp ) );
	    KEY( "CorpseStr",	skill->corpse_string,	fread_string_nohash( fp ) );
	    KEY( "CorpseStg",	skill->corpse_stage,	fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "DamType",	skill->dam_type,   	fread_number( fp ) );
            KEY( "Dammsg",	skill->noun_damage,	fread_string_nohash( fp ) );
	    KEY( "Dice",	skill->dice,		fread_string_nohash( fp ) );
	    KEY( "Diechar",	skill->die_char,	fread_string_nohash( fp ) );
	    KEY( "Dieroom",	skill->die_room,	fread_string_nohash( fp ) );
	    KEY( "Dievict",	skill->die_vict,	fread_string_nohash( fp ) );
	    KEY( "Difficulty",	skill->difficulty,	fread_number( fp ) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		for ( x = 0; x < MAX_CLASS; x++ )
		{
                    if (skill->class_mana[x] == -1)
			skill->class_mana[x] = skill->min_mana;
		    if (skill->class_beats[x] == -1)
			skill->class_beats[x] = skill->beats;
		}
		return skill;
	    }
	    break;

	case 'F':
	    KEY( "Flags",	skill->flags,		fread_number( fp ) );
	    break;

	case 'G':
	    KEY( "Guild",	skill->guild,		fread_number( fp ) );
	    break;

	case 'H':
	    KEY( "Hitchar",	skill->hit_char,	fread_string_nohash( fp ) );
	    KEY( "Hitroom",	skill->hit_room,	fread_string_nohash( fp ) );
	    KEY( "Hitvict",	skill->hit_vict,	fread_string_nohash( fp ) );
	    break;

	case 'I':
	    KEY( "Immchar",	skill->imm_char,	fread_string_nohash( fp ) );
	    KEY( "Immroom",	skill->imm_room,	fread_string_nohash( fp ) );
	    KEY( "Immvict",	skill->imm_vict,	fread_string_nohash( fp ) );
	    break;

	case 'M':
	    KEY( "Mana",	skill->min_mana,	fread_number( fp ) );
	    if ( !str_cmp( word, "Minlevel" ) )
	    {
		fread_to_eol( fp );
		fMatch = TRUE;
		break;
	    }
	    KEY( "Minpos",	skill->minimum_position, fread_number( fp ) );
	    KEY( "Misschar",	skill->miss_char,	fread_string_nohash( fp ) );
	    KEY( "Missroom",	skill->miss_room,	fread_string_nohash( fp ) );
	    KEY( "Missvict",	skill->miss_vict,	fread_string_nohash( fp ) );
	    break;

	case 'N':
            KEY( "Name",	skill->name,		fread_string_nohash( fp ) );
	    break;

	case 'P':
	    KEY( "Participants",skill->participants,	fread_number( fp ) );
	    KEY( "PartStartCh",	skill->part_start_char, fread_string_nohash( fp ) );
	    KEY( "PartStartRm",	skill->part_start_room, fread_string_nohash( fp ) );
	    KEY( "PartEndChar",	skill->part_end_char,   fread_string_nohash( fp ) );
	    KEY( "PartEndVict",	skill->part_end_vict,   fread_string_nohash( fp ) );
	    KEY( "PartEndRm",	skill->part_end_room,   fread_string_nohash( fp ) );
	    KEY( "PartEndCast",	skill->part_end_caster, fread_string_nohash( fp ) );
	    KEY( "PartMissCh",	skill->part_miss_char,  fread_string_nohash( fp ) );
	    KEY( "PartMissRm",	skill->part_miss_room,  fread_string_nohash( fp ) );
            KEY( "PartAbortCh",	skill->part_abort_char, fread_string_nohash( fp ) );
	    KEY( "PowerType",	skill->power_type,   	fread_number( fp ) );
	    break;

	case 'R':
	    KEY( "Rounds",	skill->beats,		fread_number( fp ) );
	    break;

	case 'S':
	    KEY( "Slot",	skill->slot,		fread_number( fp ) );
	    KEY( "Saves",	skill->save_type,   	fread_number( fp ) );
	    KEY( "SaveType",	skill->save_type,   	fread_number( fp ) );
	    break;

	case 'T':
	    KEY( "Target",	skill->target,		fread_number( fp ) );
	    KEY( "Teachers",	skill->teachers,	fread_string_nohash( fp ) );
	    KEY( "Type",	skill->type,            get_skill_tname(fread_word(fp))  );
	    break;

	case 'V':
	    KEY( "Value",	skill->value,		fread_number( fp ) );
	    break;

	case 'W':
	    KEY( "Wearoff",	skill->msg_off,		fread_string_nohash( fp ) );
	    KEY( "Wearoffroom",	skill->msg_off_room,	fread_string_nohash( fp ) );
	    KEY( "Wearoffsoon",	skill->msg_off_soon,	fread_string_nohash( fp ) );
	    KEY( "WearoffsoonR",skill->msg_off_soon_room, fread_string_nohash( fp ) );
	    break;
	}

	if ( !fMatch )
	{
            sprintf( buf, "Fread_skill: no match: %s", word );
            fread_to_eol(fp);
	    bug( buf, 0 );
	}
    }
}

void load_skill_table()
{
    FILE *fp;

    if ( ( fp = fopen( SKILL_FILE, "r" ) ) != NULL )
    {
	top_sn = 0;
	for ( ;; )
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
                bug( "Load_skill_table: # not found." );
		break;
	    }

	    word = fread_word( fp );
            if ( !str_cmp( word, "SKILL"      ) )
	    {
		if ( top_sn >= MAX_SKILL )
		{
		    bug( "load_skill_table: more skills than MAX_SKILL %d", MAX_SKILL );
		    FCLOSE( fp );
		    return;
		}
		skill_table[top_sn++] = fread_skill( fp );
		continue;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
                bug( "Load_skill_table: bad section." );
		continue;
	    }
	}
	FCLOSE( fp );
    }
    else
    {
	bug( "Cannot open skills.dat" );
 	exit(0);
    }
}

void load_herb_table()
{
    FILE *fp;

    if ( ( fp = fopen( HERB_FILE, "r" ) ) != NULL )
    {
	top_herb = 0;
	for ( ;; )
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
		bug( "Load_herb_table: # not found." );
		break;
	    }

	    word = fread_word( fp );
            if ( !str_cmp( word, "HERB"      ) )
	    {
		if ( top_herb >= MAX_HERB )
		{
		    bug( "load_herb_table: more herbs than MAX_HERB %d", MAX_HERB );
		    FCLOSE( fp );
		    return;
		}
		herb_table[top_herb++] = fread_skill( fp );
		if ( herb_table[top_herb-1]->slot == 0 )
		    herb_table[top_herb-1]->slot = top_herb-1;
		continue;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
                bug( "Load_herb_table: bad section." );
		continue;
	    }
	}
	FCLOSE( fp );
    }
    else
    {
	bug( "Cannot open herbs.dat" );
 	exit(0);
    }
}

void fread_social( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    const char *word = NULL;
    bool fMatch = FALSE;
    SOCIALTYPE *social;

    CREATE( social, SOCIALTYPE, 1 );

    social->position = POS_RESTING;

    for ( ;; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'C':
	    KEY( "CharNoArg",	social->char_no_arg,	fread_string_nohash(fp) );
	    KEY( "CharFound",	social->char_found,	fread_string_nohash(fp) );
	    KEY( "CharAuto",	social->char_auto,	fread_string_nohash(fp) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !social->name )
		{
		    bug( "Fread_social: Name not found" );
		    free_social( social );
		    return;
		}
		if ( !social->char_no_arg )
		{
		    bug( "Fread_social: CharNoArg not found" );
		    free_social( social );
		    return;
		}
		add_social( social );
		return;
	    }
	    break;

        case 'M':
            KEY( "MobResponse", social->mob_response,   fread_string_nohash(fp) );
            break;

	case 'N':
	    KEY( "Name",	social->name,		fread_string_nohash(fp) );
	    KEY( "NotFound",	social->not_found,	fread_string_nohash(fp) );
	    break;

	case 'O':
	    KEY( "OthersNoArg",	social->others_no_arg,	fread_string_nohash(fp) );
	    KEY( "OthersFound",	social->others_found,	fread_string_nohash(fp) );
	    KEY( "OthersAuto",	social->others_auto,	fread_string_nohash(fp) );
	    break;

	case 'P':
            KEY( "Position",	social->position,	fread_number(fp) );
            break;

	case 'V':
	    KEY( "VictFound",	social->vict_found,	fread_string_nohash(fp) );
	    break;
	}

	if ( !fMatch )
	{
            sprintf( buf, "Fread_social: no match: %s", word );
            fread_to_eol(fp);
	    bug( buf, 0 );
	}
    }
}

void load_socials()
{
    FILE *fp;

    if ( ( fp = fopen( SOCIAL_FILE, "r" ) ) != NULL )
    {
	top_sn = 0;
	for ( ;; )
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
                bug( "Load_socials: # not found." );
		break;
	    }

	    word = fread_word( fp );
            if ( !str_cmp( word, "SOCIAL"      ) )
	    {
                fread_social( fp );
	    	continue;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
                bug( "Load_socials: bad section." );
		continue;
	    }
	}
	FCLOSE( fp );
    }
    else
    {
	bug( "Cannot open %s", SOCIAL_FILE );
 	exit(0);
    }
}


void fread_command( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    const char *word = NULL;
    bool fMatch = FALSE;
    CMDTYPE *command;

    CREATE( command, CMDTYPE, 1 );

    command->position = 0;
    command->level    = 0;
    command->log      = 0;
    command->flags    = 0;

    for ( ;; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'C':
	    KEY( "Code",	command->do_fun,	skill_function(fread_word(fp)) );
	    break;

	case 'E':
	    if ( !str_cmp( word, "End" ) )
	    {
		if ( !command->name )
		{
		    bug( "Fread_command: Name not found" );
		    free_command( command );
		    return;
		}
		if ( !command->do_fun )
		{
		    bug( "Fread_command: Function not found" );
		    free_command( command );
		    return;
		}
		if ( command->do_fun == skill_notfound )
		{
		    command->level = MAX_LEVEL;
		}
		add_command( command );
		return;
	    }
	    break;

	case 'F':
            KEY( "Flags",	command->flags,		fread_number(fp) );
            break;

	case 'L':
	    KEY( "Level",	command->level,		fread_number(fp) );
	    KEY( "Log",		command->log,		fread_number(fp) );
	    break;

	case 'N':
	    KEY( "Name",	command->name,		fread_string_nohash(fp) );
	    break;

	case 'P':
	    KEY( "Position",	command->position,	fread_number(fp) );
	    break;
	}

	if ( !fMatch )
	{
            sprintf( buf, "Fread_command: no match: %s", word );
            fread_to_eol(fp);
	    bug( buf, 0 );
	}
    }
}

void load_commands()
{
    FILE *fp;

    if ( ( fp = fopen( COMMAND_FILE, "r" ) ) != NULL )
    {
	top_sn = 0;
	for ( ;; )
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
                bug( "Load_commands: # not found." );
		break;
	    }

	    word = fread_word( fp );
            if ( !str_cmp( word, "COMMAND"      ) )
	    {
                fread_command( fp );
	    	continue;
	    }
	    else
	    if ( !str_cmp( word, "END"	) )
	        break;
	    else
	    {
                bug( "Load_commands: bad section." );
		continue;
	    }
	}
	FCLOSE( fp );
    }
    else
    {
	bug( "Cannot open commands.dat" );
 	exit(0);
    }

}
#endif

void save_classes()
{
    sh_int x;

    for ( x = FIRST_CLASS; x < MAX_CLASS; x++ )
      write_class_file( x );
}

/*
 * Tongues / Languages loading/saving functions			-Altrag
 */
void fread_cnv(FILE * fp, LCNV_DATA **first_cnv, LCNV_DATA **last_cnv)
{
    LCNV_DATA *cnv;
    char letter;

    for (;;)
    {
	letter = fread_letter(fp);
	if (letter == '~' || letter == EOF)
	    break;
	ungetc(letter, fp);
	CREATE(cnv, LCNV_DATA, 1);

	cnv->old = str_dup(fread_word(fp));
	cnv->olen = strlen(cnv->old);
	cnv->newstr = str_dup(fread_word(fp));
	cnv->nlen = strlen(cnv->newstr);
	fread_to_eol(fp);
	LINK(cnv, *first_cnv, *last_cnv, next, prev);
    }
}

void load_tongues()
{
    FILE *fp;
    LANG_DATA *lng;
    char *word;
    char letter;

    if (!(fp=fopen(TONGUE_FILE, "r")))
    {
	perror("Load_tongues");
	return;
    }
    for (;;)
    {
	letter = fread_letter(fp);
	if (letter == EOF)
	{
	    fclose(fp);
	    return;
	}
	else if (letter == '*')
	{
	    fread_to_eol(fp);
	    continue;
	}
	else if (letter != '#')
	{
	    bug("Letter '%c' not #.", letter);
	    exit(0);
	}
	word = fread_word(fp);
	if (!str_cmp(word, "end"))
	{
	    fclose(fp);
	    return;
	}
	fread_to_eol(fp);
	CREATE(lng, LANG_DATA, 1);
	lng->name = STRALLOC(word);
	fread_cnv(fp, &lng->first_precnv, &lng->last_precnv);
	lng->alphabet = fread_string(fp);
	fread_cnv(fp, &lng->first_cnv, &lng->last_cnv);
	fread_to_eol(fp);
	LINK(lng, first_lang, last_lang, next, prev);
    }
    fclose(fp);
    return;
}

void fwrite_langs(void)
{
    FILE *fp;
    LANG_DATA *lng;
    LCNV_DATA *cnv;

    if (!(fp=fopen(TONGUE_FILE, "w")))
    {
	perror("fwrite_langs");
	return;
    }
    for (lng = first_lang; lng; lng = lng->next)
    {
	fprintf(fp, "#%s\n", lng->name);
	for (cnv = lng->first_precnv; cnv; cnv = cnv->next)
	    fprintf(fp, "'%s' '%s'\n", cnv->old, cnv->newstr);
	fprintf(fp, "~\n%s~\n", lng->alphabet);
	for (cnv = lng->first_cnv; cnv; cnv = cnv->next)
	    fprintf(fp, "'%s' '%s'\n", cnv->old, cnv->newstr);
	fprintf(fp, "\n");
    }
    fprintf(fp, "#end\n\n");
    fclose(fp);
    return;
}

void free_langs(void)
{
    LANG_DATA *lng;
    LCNV_DATA *cnv;

    while ((lng = first_lang))
    {
	UNLINK(lng, first_lang, last_lang, next, prev);
	STRFREE(lng->name);
        STRFREE(lng->alphabet);
	while ((cnv = lng->first_precnv))
	{
	    UNLINK(cnv, lng->first_precnv, lng->last_precnv, next, prev);
	    if (cnv->old)
		DISPOSE(cnv->old);
	    if (cnv->newstr)
		DISPOSE(cnv->newstr);
            DISPOSE(cnv);
	}
	while ((cnv = lng->first_cnv))
	{
	    UNLINK(cnv, lng->first_cnv, lng->last_cnv, next, prev);
	    if (cnv->old)
		DISPOSE(cnv->old);
	    if (cnv->newstr)
		DISPOSE(cnv->newstr);
            DISPOSE(cnv);
	}
	DISPOSE(lng);
    }
}
