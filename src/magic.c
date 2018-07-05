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
 *			     Spell handling module			    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: magic.c,v 1.103 2004/04/06 22:00:10 dotd Exp $";*/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "mud.h"
#include "gsn.h"
#include "poly.h"
#include "rareobj.h"

DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_recall);
DECLARE_DO_FUN(do_cast);

extern bool fBootDb;
extern int top_affect;
extern int top_room_vnum;

/*
 * Local functions.
 */
void	say_spell	args( ( CHAR_DATA *ch, int sn ) );
CHAR_DATA *make_poly_mob args( (CHAR_DATA *ch, int vnum) );
ch_ret	spell_affect	args( ( int sn, int level, CHAR_DATA *ch, void *vo ) );
ch_ret	spell_affectchar args( ( int sn, int level, CHAR_DATA *ch, void *vo ) );

/*
 * Is immune to a damage type
 */
bool is_immune( CHAR_DATA *ch, SKILLTYPE *skill )
{
    AFFECT_DATA *paf;
    int sn;

    if (!ch)
	return FALSE;

    if (number_percent() < ch->antimagicp)
        return TRUE;

    switch( SPELL_DAMAGE(skill) )
    {
    case SD_FIRE:	     if (IS_IMMUNE(ch, RIS_FIRE))	 return TRUE;
    case SD_COLD:	     if (IS_IMMUNE(ch, RIS_COLD))	 return TRUE;
    case SD_ELECTRICITY:     if (IS_IMMUNE(ch, RIS_ELECTRICITY)) return TRUE;
    case SD_ENERGY:	     if (IS_IMMUNE(ch, RIS_ENERGY)) 	 return TRUE;
    case SD_ACID:	     if (IS_IMMUNE(ch, RIS_ACID))	 return TRUE;
    case SD_POISON:	     if (IS_IMMUNE(ch, RIS_POISON)) 	 return TRUE;
    case SD_DRAIN:	     if (IS_IMMUNE(ch, RIS_DRAIN))	 return TRUE;
    case SD_NONE: return FALSE;
    }

    if (!(sn = skill_lookup(skill->name)))
        return FALSE;

    for ( paf = ch->first_affect; paf; paf = paf->next )
        if ( paf->location == APPLY_IMMUNESPELL )
            if ( paf->modifier == sn )
                return TRUE;

    return FALSE;
}

/*
 * Lookup a skill by name, only stopping at skills the player has.
 */
int ch_slookup( CHAR_DATA *ch, const char *name )
{
    int sn;

    if ( IS_NPC(ch) && !IS_ACT_FLAG(ch, ACT_POLYMORPHED))
        return skill_lookup( name );
    for ( sn = 0; sn < top_sn; sn++ )
    {
        if ( !skill_table[sn]->name )
            break;
        if (  LEARNED(ch, sn) > 0
              &&    CanUseSkill(ch, sn)
              &&    LOWER(name[0]) == LOWER(skill_table[sn]->name[0])
              &&   !str_prefix( name, skill_table[sn]->name ) )
            return sn;
    }

    return -1;
}

/*
 * Lookup an herb by name.
 */
int herb_lookup( const char *name )
{
    int sn;

    for ( sn = 0; sn < top_herb; sn++ )
    {
        if ( !herb_table[sn] || !herb_table[sn]->name )
            return -1;
        if ( LOWER(name[0]) == LOWER(herb_table[sn]->name[0])
             &&  !str_prefix( name, herb_table[sn]->name ) )
            return sn;
    }
    return -1;
}

/*
 * Lookup a personal skill
 */
int personal_lookup( CHAR_DATA *ch, const char *name )
{
    return -1;
}

/*
 * Lookup a skill by name.
 */
int skill_lookup( const char *name )
{
    int sn;

    if ( (sn=bsearch_skill_exact(name, gsn_first_spell, GSN_LAST_SPELL)) != -1 )
        return sn;

    if ( (sn=bsearch_skill_exact(name, gsn_first_skill, GSN_LAST_SKILL)) != -1 )
        return sn;

    if ( (sn=bsearch_skill_exact(name, gsn_first_weapon, GSN_LAST_WEAPON)) != -1 )
        return sn;

    if ( (sn=bsearch_skill_exact(name, gsn_first_tongue, GSN_LAST_TONGUE)) != -1 )
        return sn;

    if ( (sn=bsearch_skill_exact(name, gsn_first_lore, GSN_LAST_LORE)) != -1 )
        return sn;

    if ( (sn=bsearch_skill_exact(name, gsn_first_psispell, GSN_LAST_PSISPELL)) != -1 )
	return sn;

    if ( (sn=bsearch_skill(name, gsn_first_spell, GSN_LAST_SPELL)) != -1 )
        return sn;

    if ( (sn=bsearch_skill(name, gsn_first_skill, GSN_LAST_SKILL)) != -1 )
        return sn;

    if ( (sn=bsearch_skill(name, gsn_first_weapon, GSN_LAST_WEAPON)) != -1 )
        return sn;

    if ( (sn=bsearch_skill(name, gsn_first_tongue, GSN_LAST_TONGUE)) != -1 )
        return sn;

    if ( (sn=bsearch_skill(name, gsn_first_lore, GSN_LAST_LORE)) != -1 )
        return sn;

    if ( (sn=bsearch_skill(name, gsn_first_psispell, GSN_LAST_PSISPELL)) != -1 )
        return sn;

    if ( gsn_top_sn < top_sn )
    {
        for ( sn = gsn_top_sn; sn < top_sn; sn++ )
        {
            if ( !skill_table[sn] || !skill_table[sn]->name )
                return -1;
            if ( LOWER(name[0]) == LOWER(skill_table[sn]->name[0]) &&
                 !str_cmp( name, skill_table[sn]->name ) )
                return sn;
        }

        for ( sn = gsn_top_sn; sn < top_sn; sn++ )
        {
            if ( !skill_table[sn] || !skill_table[sn]->name )
                return -1;
            if ( LOWER(name[0]) == LOWER(skill_table[sn]->name[0]) &&
                 !str_prefix( name, skill_table[sn]->name ) )
                return sn;
        }
        bug("skill_lookup: unknown skill: %s", name);
    }

    return -1;
}

/*
 * Return a skilltype pointer based on sn			-Thoric
 * Returns NULL if bad, unused or personal sn.
 */
SKILLTYPE *get_skilltype( int sn )
{
    if ( sn >= TYPE_PERSONAL )
        return NULL;
    if ( sn >= TYPE_HERB )
        return IS_VALID_HERB(sn-TYPE_HERB) ? herb_table[sn-TYPE_HERB] : NULL;
    if ( sn >= TYPE_HIT )
        return NULL;
    return IS_VALID_SN(sn) ? skill_table[sn] : NULL;
}

/*
 * Perform a binary search on a section of the skill table	-Thoric
 * Each different section of the skill table is sorted alphabetically
 */
int bsearch_skill( const char *name, int first, int top )
{
    int sn;

    for (;;)
    {
        sn = (first + top) >> 1;

        if ( LOWER(name[0]) == LOWER(skill_table[sn]->name[0])
             &&  !str_prefix(name, skill_table[sn]->name) )
            return sn;
        if (first >= top)
            return -1;
        if (strcmp(name, skill_table[sn]->name) < 1)
            top = sn - 1;
        else
            first = sn + 1;
    }
    return -1;
}

/*
 * Perform a binary search on a section of the skill table	-Thoric
 * Each different section of the skill table is sorted alphabetically
 * Check for exact matches only
 */
int bsearch_skill_exact( const char *name, int first, int top )
{
    int sn;

    for (;;)
    {
        sn = (first + top) >> 1;
        if ( !str_cmp(name, skill_table[sn]->name) )
            return sn;
        if (first >= top)
            return -1;
        if (strcmp(name, skill_table[sn]->name) < 1)
            top = sn - 1;
        else
            first = sn + 1;
    }
    return -1;
}

/*
 * Perform a binary search on a section of the skill table
 * Each different section of the skill table is sorted alphabetically
 * Only match skills player knows				-Thoric
 */
int ch_bsearch_skill( CHAR_DATA *ch, const char *name, int first, int top )
{
    int sn;

    for (;;)
    {
        sn = (first + top) >> 1;

        if ( LOWER(name[0]) == LOWER(skill_table[sn]->name[0])
             &&  !str_prefix(name, skill_table[sn]->name)
             &&   LEARNED(ch, sn) > 0
             &&   CanUseSkill(ch, sn) )
            return sn;
        if (first >= top)
            return -1;
        if (strcmp( name, skill_table[sn]->name) < 1)
            top = sn - 1;
        else
            first = sn + 1;
    }
    return -1;
}


int find_spell( CHAR_DATA *ch, const char *name, bool know )
{
    if ( IS_NPC(ch) || !know )
        return bsearch_skill( name, gsn_first_spell, GSN_LAST_SPELL );
    else
        return ch_bsearch_skill( ch, name, gsn_first_spell, GSN_LAST_SPELL  );
}

int find_skill( CHAR_DATA *ch, const char *name, bool know )
{
    if ( IS_NPC(ch) || !know )
        return bsearch_skill( name, gsn_first_skill, GSN_LAST_SKILL );
    else
        return ch_bsearch_skill( ch, name, gsn_first_skill, GSN_LAST_SKILL  );
}

int find_weapon( CHAR_DATA *ch, const char *name, bool know )
{
    if ( IS_NPC(ch) || !know )
        return bsearch_skill( name, gsn_first_weapon, GSN_LAST_WEAPON);
    else
        return ch_bsearch_skill( ch, name, gsn_first_weapon, GSN_LAST_WEAPON );
}

int find_tongue( CHAR_DATA *ch, const char *name, bool know )
{
    if ( IS_NPC(ch) || !know )
        return bsearch_skill( name, gsn_first_tongue, GSN_LAST_TONGUE );
    else
        return ch_bsearch_skill( ch, name, gsn_first_tongue, GSN_LAST_TONGUE );
}

int find_lore( CHAR_DATA *ch, const char *name, bool know )
{
    if ( IS_NPC(ch) || !know )
        return bsearch_skill( name, gsn_first_lore, GSN_LAST_LORE );
    else
        return ch_bsearch_skill( ch, name, gsn_first_lore, GSN_LAST_LORE );
}

int find_psispell( CHAR_DATA *ch, const char *name, bool know )
{
    if ( IS_NPC(ch) || !know )
        return bsearch_skill( name, gsn_first_psispell, GSN_LAST_PSISPELL );
    else
        return ch_bsearch_skill( ch, name, gsn_first_psispell, GSN_LAST_PSISPELL  );
}


/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int slot_lookup( int slot )
{
    int sn;

    if ( slot <= 0 )
        return -1;

    for ( sn = 0; sn < top_sn; sn++ )
        if ( slot == skill_table[sn]->slot )
            return sn;

    if ( fBootDb )
    {
        bug( "Slot_lookup: bad slot %d.", slot );
        /*	abort( );*/
    }

    return -1;
}

/*
 * Fancy message handling for a successful casting		-Thoric
 */
void successful_casting( SKILLTYPE *skill, CHAR_DATA *ch,
                         CHAR_DATA *victim, OBJ_DATA *obj )
{
    char buf[MAX_INPUT_LENGTH];
    unsigned int x;
    sh_int chitroom = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION);
    sh_int chit	    = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_HIT);
    sh_int chitme   = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_HITME);

    if ( skill->target != TAR_CHAR_OFFENSIVE )
    {
        chit = chitroom;
        chitme = chitroom;
    }

    if ( ch && ch != victim )
    {
        if ( skill->hit_char && skill->hit_char[0] != '\0' )
        {
            sprintf(buf, "%s", skill->hit_char);
            if ( obj && !victim && !(strstr(buf,"$P") || strstr(buf,"$p")))
                for (x=0;x<strlen(buf)-1;x++)
                {
                    if (buf[x]=='$' && buf[x+1]=='N')
                        buf[x+1]='p';
                    else if (buf[x]=='$' && buf[x+1]=='n')
                        buf[x+1]='p';
                }
            act( chit, buf, ch, obj, victim, TO_CHAR );
        }
        else
            if ( skill->type == SKILL_SPELL )
                act( chit, "Ok.", ch, NULL, NULL, TO_CHAR );
    }

    if ( ch && skill->hit_room && skill->hit_room[0] != '\0' )
    {
        sprintf(buf, "%s", skill->hit_room);
        if ( obj && !victim && !(strstr(buf,"$P") || strstr(buf,"$p")))
            for (x=0;x<strlen(buf)-1;x++)
            {
                if (buf[x]=='$' && buf[x+1]=='N')
                    buf[x+1]='p';
                else if (buf[x]=='$' && buf[x+1]=='n')
                    buf[x+1]='p';
            }
        act( chitroom, buf, ch, obj, victim, TO_NOTVICT );
    }

    if ( ch && victim && skill->hit_vict && skill->hit_vict[0] != '\0' )
    {
        if ( ch != victim )
            act( chitme, skill->hit_vict, ch, obj, victim, TO_VICT );
        else
            act( chitme, skill->hit_vict, ch, obj, victim, TO_CHAR );
    }
    else
        if ( ch && ch == victim && skill->type == SKILL_SPELL )
            act( chitme, "Ok.", ch, NULL, NULL, TO_CHAR );
}

/*
 * Fancy message handling for a failed casting			-Thoric
 */
void failed_casting( SKILLTYPE *skill, CHAR_DATA *ch,
                     CHAR_DATA *victim, OBJ_DATA *obj )
{
    sh_int chitroom = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION);
    sh_int chit	    = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_HIT);
    sh_int chitme   = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_HITME);

    if ( skill->target != TAR_CHAR_OFFENSIVE )
    {
        chit = chitroom;
        chitme = chitroom;
    }

    if ( ch && ch != victim )
    {
        if ( skill->miss_char && skill->miss_char[0] != '\0' )
            act( chit, skill->miss_char, ch, obj, victim, TO_CHAR );
        else
            if ( skill->type == SKILL_SPELL )
                act( chit, "You failed.", ch, NULL, NULL, TO_CHAR );
    }
    if ( ch && skill->miss_room && skill->miss_room[0] != '\0' )
        act( chitroom, skill->miss_room, ch, obj, victim, TO_NOTVICT );
    if ( ch && victim && skill->miss_vict && skill->miss_vict[0] != '\0' )
    {
        if ( ch != victim )
            act( chitme, skill->miss_vict, ch, obj, victim, TO_VICT );
        else
            act( chitme, skill->miss_vict, ch, obj, victim, TO_CHAR );
    }
    else
        if ( ch && ch == victim )
        {
            if ( skill->miss_char && skill->miss_char[0] != '\0' )
                act( chitme, skill->miss_char, ch, obj, victim, TO_CHAR );
            else
                if ( skill->type == SKILL_SPELL )
                    act( chitme, "You failed.", ch, NULL, NULL, TO_CHAR );
        }
}

/*
 * Fancy message handling for being immune to something		-Thoric
 */
void immune_casting( SKILLTYPE *skill, CHAR_DATA *ch,
                     CHAR_DATA *victim, OBJ_DATA *obj )
{
    sh_int chitroom = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION);
    sh_int chit	    = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_HIT);
    sh_int chitme   = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_HITME);

    if ( skill->target != TAR_CHAR_OFFENSIVE )
    {
        chit = chitroom;
        chitme = chitroom;
    }

    if ( ch && ch != victim )
    {
        if ( skill->imm_char && skill->imm_char[0] != '\0' )
            act( chit, skill->imm_char, ch, obj, victim, TO_CHAR );
        else
            if ( skill->miss_char && skill->miss_char[0] != '\0' )
                act( chit, skill->hit_char, ch, obj, victim, TO_CHAR );
            else
                if ( skill->type == SKILL_SPELL || skill->type == SKILL_SKILL )
                    act( chit, "That appears to have no effect.", ch, NULL, NULL, TO_CHAR );
    }
    if ( ch && skill->imm_room && skill->imm_room[0] != '\0' )
        act( chitroom, skill->imm_room, ch, obj, victim, TO_NOTVICT );
    else
        if ( ch && skill->miss_room && skill->miss_room[0] != '\0' )
            act( chitroom, skill->miss_room, ch, obj, victim, TO_NOTVICT );
    if ( ch && victim && skill->imm_vict && skill->imm_vict[0] != '\0' )
    {
        if ( ch != victim )
            act( chitme, skill->imm_vict, ch, obj, victim, TO_VICT );
        else
            act( chitme, skill->imm_vict, ch, obj, victim, TO_CHAR );
    }
    else
        if ( ch && victim && skill->miss_vict && skill->miss_vict[0] != '\0' )
        {
            if ( ch != victim )
                act( chitme, skill->miss_vict, ch, obj, victim, TO_VICT );
            else
                act( chitme, skill->miss_vict, ch, obj, victim, TO_CHAR );
        }
        else
            if ( ch && ch == victim )
            {
                if ( skill->imm_char && skill->imm_char[0] != '\0' )
                    act( chit, skill->imm_char, ch, obj, victim, TO_CHAR );
                else
                    if ( skill->miss_char && skill->miss_char[0] != '\0' )
                        act( chit, skill->hit_char, ch, obj, victim, TO_CHAR );
                    else
                        if ( skill->type == SKILL_SPELL || skill->type == SKILL_SKILL )
                            act( chit, "That appears to have no affect.", ch, NULL, NULL, TO_CHAR );
            }
}


/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA *ch, int sn )
{
    char buf  [MAX_STRING_LENGTH];
    char buf2 [MAX_STRING_LENGTH];
    CHAR_DATA *rch;
    char *pName;
    int iSyl;
    int length;
    SKILLTYPE *skill = get_skilltype( sn );

    struct syl_type
    {
        char *	old;
        char *	snew;
    };

    static const struct syl_type syl_table[] =
    {
        { " ",		" "		},
        { "ar",		"abra"		},
        { "au",		"kada"		},
        { "bless",	"fido"		},
        { "blind",	"nose"		},
        { "bur",	"mosa"		},
        { "cu",		"judi"		},
        { "de",		"oculo"		},
        { "en",		"unso"		},
        { "light",	"dies"		},
        { "lo",		"hi"		},
        { "mor",	"zak"		},
        { "move",	"sido"		},
        { "ness",	"lacri"		},
        { "ning",	"illa"		},
        { "per",	"duda"		},
        { "ra",		"gru"		},
        { "re",		"candus"	},
        { "son",	"sabru"		},
        { "tect",	"infra"		},
        { "tri",	"cula"		},
        { "ven",	"nofo"		},
        { "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
        { "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
        { "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
        { "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
        { "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
        { "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
        { "y", "l" }, { "z", "k" },
        { "", "" }
    };

    buf[0]	= '\0';
    for ( pName = skill->name; *pName != '\0'; pName += length )
    {
        for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ )
        {
            if ( !str_prefix( syl_table[iSyl].old, pName ) )
            {
                strcat( buf, syl_table[iSyl].snew );
                break;
            }
        }

        if ( length == 0 )
            length = 1;
    }

    sprintf( buf2, "$n utters the words, '%s'.", buf );
    sprintf( buf,  "$n utters the words, '%s'.", skill->name );

    for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
        if ( rch != ch )
            act( AT_MAGIC, IS_ACTIVE(rch, BestSkCl(ch, sn)) ? buf : buf2,
                 ch, NULL, rch, TO_VICT );

    }

    return;
}


/*
 * Make adjustments to saving throw based in RIS		-Thoric
 */
int ris_save( CHAR_DATA *ch, int percent, int ris )
{
    sh_int modifier;

    modifier = 10;
    if ( IS_SET(ch->absorb, ris ) )
        modifier -= 10;
    if ( IS_IMMUNE(ch, ris ) )
        modifier -= 10;
    if ( IS_SET(ch->resistant, ris ) )
        modifier -= 2;
    if ( IS_SET(ch->susceptible, ris ) )
        modifier += 2;
    if ( modifier <= 0 )
        return 1000;
    if ( modifier == 10 )
        return percent;
    return (percent * modifier) / 10;
}


/*								    -Thoric
 * Fancy dice expression parsing complete with order of operations,
 * simple exponent support, dice support as well as a few extra
 * variables: L = level, H = hp, M = mana, V = move, S = str, X = dex
 *            I = int, W = wis, C = con, A = cha, U = luck, A = age
 *
 * Used for spell dice parsing, ie: 3d8+L-6
 *
 */
int rd_parse(CHAR_DATA *ch, int level, char *exp)
{
    int x, y, len = 0;
    int lop = 0, gop = 0, eop = 0;
    char operation;
    char *sexp[2];
    int total = 0;

    /* take care of nulls coming in */
    if (!exp || !strlen(exp))
        return 0;

    /* get rid of brackets if they surround the entire expresion */
    if ((*exp == '(') && !index(exp+1,'(') && exp[strlen(exp)-1] == ')')
    {
        exp[strlen(exp)-1] = '\0';
        exp++;
    }

    /* check if the expresion is just a number */
    len = strlen(exp);
    if ( len == 1 && isalpha(exp[0]) )
    {
        /*log_printf("r1: %c",exp[0]);*/
        switch(exp[0]) {
        case 'L': case 'l':	return level;
        case 'H': case 'h':	return ch?GET_HIT(ch):0;
        case 'M': case 'm':	return ch?GET_MANA(ch):0;
        case 'V': case 'v':	return ch?GET_MOVE(ch):0;
        case 'S': case 's':	return ch?get_curr_str(ch):0;
        case 'I': case 'i':	return ch?get_curr_int(ch):0;
        case 'W': case 'w':	return ch?get_curr_wis(ch):0;
        case 'X': case 'x':	return ch?get_curr_dex(ch):0;
        case 'C': case 'c':	return ch?get_curr_con(ch):0;
        case 'A': case 'a':	return ch?get_curr_cha(ch):0;
        case 'U': case 'u':	return ch?get_curr_lck(ch):0;
        case 'Y': case 'y':	return ch?get_age(ch):0;
        case 'Z': case 'z':     return (int)DUR_CONV;
        }
    }

    for (x = 0; x < len; ++x)
        if (!isdigit(exp[x]) && !isspace(exp[x]))
            break;
    if (x == len)
    {
        /*log_printf("r2: %d",atoi(exp));*/
        return(atoi(exp));
    }
    /* break it into 2 parts */
    for (x = 0; x < strlen(exp); ++x)
        switch(exp[x]) {
        case '^':
            if (!total)
                eop = x;
            break;
        case '-': case '+':
            if (!total)
                lop = x;
            break;
        case '*': case '/': case '%': case 'd': case 'D':
        case '<': case '>': case '{': case '}': case '=':
            if (!total)
                gop =  x;
            break;
        case '(':
            ++total;
            break;
        case ')':
            --total;
            break;
        }
    if (lop) x = lop;
    else
        if (gop) x = gop;
        else
            x = eop;
    operation = exp[x];
    exp[x] = '\0';
    sexp[0] = exp;
    sexp[1] = (char *)(exp+x+1);

    /* work it out */
    /*log_printf("1: %s, %c, %s", sexp[0], operation, sexp[1]);*/
    total = rd_parse(ch, level, sexp[0]);
    /*log_printf("2: %d, %c, %s", total, operation, sexp[1]);*/
    switch(operation) {
    case '-':		total -= rd_parse(ch, level, sexp[1]);	break;
    case '+':		total += rd_parse(ch, level, sexp[1]);	break;
    case '*':		total *= rd_parse(ch, level, sexp[1]);	break;
    case '/':		y = rd_parse(ch, level, sexp[1]);
    if (y == 0)
        total = 0;
    else
        total /= y;
    break;
    case '%':		total %= rd_parse(ch, level, sexp[1]);	break;
    case 'd': case 'D':	total = dice( total, rd_parse(ch, level, sexp[1]) );	break;
    case '<':		total = (total < rd_parse(ch, level, sexp[1])); break;
    case '>':		total = (total > rd_parse(ch, level, sexp[1])); break;
    case '=':		total = (total == rd_parse(ch, level, sexp[1])); break;
    case '{':		total = UMIN( total, rd_parse(ch, level, sexp[1])); break;
    case '}':		total = UMAX( total, rd_parse(ch, level, sexp[1])); break;
    case '^':
        {
            int z = total;

            y = rd_parse(ch, level, sexp[1]);

            for (x = 1; x < y; ++x, z *= total);
            total = z;
            break;
        }
    }
    /*log_printf("r3: %d", total);*/
    return total;
}

/* wrapper function so as not to destroy exp */
int dice_parse(CHAR_DATA *ch, int level, char *dicestr)
{
    char buf[MAX_INPUT_LENGTH];

    strcpy( buf, dicestr );
    return rd_parse(ch, level, buf);
}

#if 1
/* Dale saving -Garil 10/5/2002 */
const sh_int saving_throws[REAL_MAX_CLASS][5][MAX_LEVEL] =
{
    /*
     * mage
     */
    {
        {16, 14, 14, 14, 14, 14, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 8, 6, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {15, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {17, 15, 15, 15, 15, 15, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 5, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {14, 12, 12, 12, 12, 12, 10, 10, 10, 10, 10, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    /*
     * cleric
     */
    {
        {11, 10, 10, 10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {16, 14, 14, 14, 13, 13, 13, 11, 11, 11, 10, 10, 10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {15, 13, 13, 13, 12, 12, 12, 10, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {18, 16, 16, 16, 15, 15, 15, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10, 8, 8, 7, 6, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {17, 15, 15, 15, 14, 14, 14, 12, 12, 12, 11, 11, 11, 10, 10, 10, 9, 9, 9, 7, 7, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    /*
     * thief
     */
    {
        {16, 14, 14, 13, 13, 11, 11, 10, 10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {18, 16, 16, 15, 15, 13, 13, 12, 12, 10, 10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {17, 15, 15, 14, 14, 12, 12, 11, 11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {20, 17, 17, 16, 16, 13, 13, 12, 12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {19, 17, 17, 16, 16, 14, 14, 13, 13, 11, 11, 10, 10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    /*
     * warrior
     */
    {
        {15, 13, 13, 13, 13, 12, 12, 12, 12, 11, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 9, 8, 7, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {16, 14, 14, 14, 14, 12, 12, 12, 12, 10, 10, 10, 10, 8, 8, 8, 8, 6, 6, 6, 6, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {14, 12, 12, 12, 12, 11, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 9, 8, 8, 8, 8, 7, 5, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {18, 16, 16, 16, 16, 15, 15, 15, 15, 14, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 12, 11, 9, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {17, 15, 15, 15, 15, 13, 13, 13, 13, 11, 11, 11, 11, 9, 9, 9, 9, 7, 7, 7, 7, 5, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    /*
     * vampire (same as warrior)
     */
    {
        {15, 13, 13, 13, 13, 12, 12, 12, 12, 11, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 9, 8, 7, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {16, 14, 14, 14, 14, 12, 12, 12, 12, 10, 10, 10, 10, 8, 8, 8, 8, 6, 6, 6, 6, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {14, 12, 12, 12, 12, 11, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 9, 8, 8, 8, 8, 7, 5, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {18, 16, 16, 16, 16, 15, 15, 15, 15, 14, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 12, 11, 9, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {17, 15, 15, 15, 15, 13, 13, 13, 13, 11, 11, 11, 11, 9, 9, 9, 9, 7, 7, 7, 7, 5, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    /*
     * druid
     */
    {
        {11, 10, 10, 10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {16, 14, 14, 14, 13, 13, 13, 11, 11, 11, 10, 10, 10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {15, 13, 13, 13, 12, 12, 12, 10, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {18, 16, 16, 16, 15, 15, 15, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10, 8, 8, 7, 6, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {17, 15, 15, 15, 14, 14, 14, 12, 12, 12, 11, 11, 11, 10, 10, 10, 9, 9, 9, 7, 7, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    /*
     * ranger (same as druid)
     */
    {
        {11, 10, 10, 10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {16, 14, 14, 14, 13, 13, 13, 11, 11, 11, 10, 10, 10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {15, 13, 13, 13, 12, 12, 12, 10, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {18, 16, 16, 16, 15, 15, 15, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10, 8, 8, 7, 6, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {17, 15, 15, 15, 14, 14, 14, 12, 12, 12, 11, 11, 11, 10, 10, 10, 9, 9, 9, 7, 7, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    /*
     * amazon (same as druid)
     */
    {
        {11, 10, 10, 10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {16, 14, 14, 14, 13, 13, 13, 11, 11, 11, 10, 10, 10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {15, 13, 13, 13, 12, 12, 12, 10, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {18, 16, 16, 16, 15, 15, 15, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10, 8, 8, 7, 6, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {17, 15, 15, 15, 14, 14, 14, 12, 12, 12, 11, 11, 11, 10, 10, 10, 9, 9, 9, 7, 7, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    /*
     * paladin (same as cleric)
     */
    {
        {11, 10, 10, 10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {16, 14, 14, 14, 13, 13, 13, 11, 11, 11, 10, 10, 10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {15, 13, 13, 13, 12, 12, 12, 10, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {18, 16, 16, 16, 15, 15, 15, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10, 8, 8, 7, 6, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {17, 15, 15, 15, 14, 14, 14, 12, 12, 12, 11, 11, 11, 10, 10, 10, 9, 9, 9, 7, 7, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    /*
     * barbarian
     */
    {
        {15, 13, 13, 13, 13, 12, 12, 12, 12, 11, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 9, 8, 7, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {16, 14, 14, 14, 14, 12, 12, 12, 12, 10, 10, 10, 10, 8, 8, 8, 8, 6, 6, 6, 6, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {14, 12, 12, 12, 12, 11, 11, 11, 11, 10, 10, 10, 10, 9, 9, 9, 9, 8, 8, 8, 8, 7, 5, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {18, 16, 16, 16, 16, 15, 15, 15, 15, 14, 14, 14, 14, 13, 13, 13, 13, 12, 12, 12, 12, 11, 9, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {17, 15, 15, 15, 15, 13, 13, 13, 13, 11, 11, 11, 11, 9, 9, 9, 9, 7, 7, 7, 7, 5, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    /*
     * psi
     */
    {
        {16, 14, 14, 13, 13, 11, 11, 10, 10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {18, 16, 16, 15, 15, 13, 13, 12, 12, 10, 10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {17, 15, 15, 14, 14, 12, 12, 11, 11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {20, 17, 17, 16, 16, 13, 13, 12, 12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {19, 17, 17, 16, 16, 14, 14, 13, 13, 11, 11, 10, 10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    /*
     * artificer (same as psi)
     */
    {
        {16, 14, 14, 13, 13, 11, 11, 10, 10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {18, 16, 16, 15, 15, 13, 13, 12, 12, 10, 10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {17, 15, 15, 14, 14, 12, 12, 11, 11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {20, 17, 17, 16, 16, 13, 13, 12, 12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {19, 17, 17, 16, 16, 14, 14, 13, 13, 11, 11, 10, 10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    /*
     * monk
     */
    {
        {16, 14, 14, 13, 13, 11, 11, 10, 10, 8, 8, 7, 7, 5, 5, 4, 4, 3, 3, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {18, 16, 16, 15, 15, 13, 13, 12, 12, 10, 10, 9, 9, 7, 7, 6, 6, 5, 5, 5, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {17, 15, 15, 14, 14, 12, 12, 11, 11, 9, 9, 8, 8, 6, 6, 5, 5, 4, 4, 4, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {20, 17, 17, 16, 16, 13, 13, 12, 12, 9, 9, 8, 8, 5, 5, 4, 4, 4, 4, 4, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {19, 17, 17, 16, 16, 14, 14, 13, 13, 11, 11, 10, 10, 8, 8, 7, 7, 6, 6, 6, 6, 4, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    /*
     * necromancer (same as sorcerer)
     */
    {
        {16, 14, 14, 14, 14, 14, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 8, 6, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {15, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {17, 15, 15, 15, 15, 15, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 5, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {14, 12, 12, 12, 12, 12, 10, 10, 10, 10, 10, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    /*
     * antipaladin (same as cleric)
     */
    {
        {11, 10, 10, 10, 9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {16, 14, 14, 14, 13, 13, 13, 11, 11, 11, 10, 10, 10, 9, 9, 9, 8, 8, 8, 6, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {15, 13, 13, 13, 12, 12, 12, 10, 10, 10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 5, 5, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {18, 16, 16, 16, 15, 15, 15, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10, 8, 8, 7, 6, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {17, 15, 15, 15, 14, 14, 14, 12, 12, 12, 11, 11, 11, 10, 10, 10, 9, 9, 9, 7, 7, 6, 5, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    },
    /*
     * sorcerer
     */
    {
        {16, 14, 14, 14, 14, 14, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 8, 6, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {15, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 5, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {17, 15, 15, 15, 15, 15, 13, 13, 13, 13, 13, 11, 11, 11, 11, 11, 9, 9, 9, 9, 9, 7, 5, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {14, 12, 12, 12, 12, 12, 10, 10, 10, 10, 10, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 4, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    }
};


/* level is unused, it's a smaug thing and we need the level and class, rather
   than passing it, just use the char's best */
bool saves_generic( int level, CHAR_DATA *victim, int victim_save, int save_type)
{
    int save = 0;

    if (GET_RACE(victim) == RACE_GOD ||
        IS_IMMORTAL(victim))
        return TRUE;

    if (!IS_NPC(victim) || IS_ACT_FLAG(victim, ACT_POLYMORPHED))
        save = saving_throws[BestMagicClass(victim)][save_type][GET_LEVEL(victim, BestMagicClass(victim))];

    return (UMAX(1, (victim_save + save)) < number_range(1, 20));
}

bool saves_poison_death( int level, CHAR_DATA *victim )
{
    return saves_generic(level, victim, victim->saving_poison_death, 0);
}
bool saves_wands( int level, CHAR_DATA *victim ) /* rod */
{
    return saves_generic(level, victim, victim->saving_wand, 1);
}
bool saves_para_petri( int level, CHAR_DATA *victim ) /* petri */
{
    return saves_generic(level, victim, victim->saving_para_petri, 2);
}
bool saves_breath( int level, CHAR_DATA *victim ) /* breath */
{
    return saves_generic(level, victim, victim->saving_breath, 3);
}
bool saves_spell_staff( int level, CHAR_DATA *victim ) /* spell */
{
    return saves_generic(level, victim, victim->saving_spell_staff, 4);
}
#else
/* the old smaug way -Garil 10/5/2002 */
/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_poison_death( int level, CHAR_DATA *victim )
{
    int save;

    if (!victim)
        return FALSE;

    save = 50 + ( GetMaxLevel(victim) - level - victim->saving_poison_death ) * 5;
    save = URANGE( 5, save, 95 );
    return chance( victim, save );
}
bool saves_wands( int level, CHAR_DATA *victim )
{
    int save;

    if (!victim)
        return FALSE;

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
        return TRUE;

    save = 50 + ( GetMaxLevel(victim) - level - victim->saving_wand ) * 5;
    save = URANGE( 5, save, 95 );
    return chance( victim, save );
}
bool saves_para_petri( int level, CHAR_DATA *victim )
{
    int save;

    if (!victim)
        return FALSE;

    save = 50 + ( GetMaxLevel(victim) - level - victim->saving_para_petri) * 5;
    save = URANGE( 5, save, 95 );
    return chance( victim, save );
}
bool saves_breath( int level, CHAR_DATA *victim )
{
    int save;

    if (!victim)
        return FALSE;

    save = 50 + ( GetMaxLevel(victim) - level - victim->saving_breath ) * 5;
    save = URANGE( 5, save, 95 );
    return chance( victim, save );
}
bool saves_spell_staff( int level, CHAR_DATA *victim )
{
    int save;

    if (!victim)
        return FALSE;

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
        return TRUE;

    if ( IS_NPC( victim ) && level > 10 )
        level -= 5;
    save = 50 + ( GetMaxLevel(victim) - level - victim->saving_spell_staff) * 5;
    save = URANGE( 5, save, 95 );
    return chance( victim, save );
}
#endif

/*
 * Process the spell's required components, if any		-Thoric
 * -----------------------------------------------
 * T###		check for item of type ###
 * V#####	check for item of vnum #####
 * Kword	check for item with keyword 'word'
 * G#####	check if player has ##### amount of gold
 * H####	check if player has #### amount of hitpoints
 *
 * Special operators:
 * ! spell fails if player has this
 * + don't consume this component
 * @ decrease component's value[0], and extract if it reaches 0
 * # decrease component's value[1], and extract if it reaches 0
 * $ decrease component's value[2], and extract if it reaches 0
 * % decrease component's value[3], and extract if it reaches 0
 * ^ decrease component's value[4], and extract if it reaches 0
 * & decrease component's value[5], and extract if it reaches 0
 */
bool process_spell_components( CHAR_DATA *ch, int sn )
{
    SKILLTYPE *skill	= get_skilltype(sn);
    char *comp		= skill->components;
    char *check;
    char arg[MAX_INPUT_LENGTH];
    bool consume, fail, found;
    int  val, value;
    OBJ_DATA *obj;

    /* if no components necessary, then everything is cool */
    if ( !comp || comp[0] == '\0' )
        return TRUE;

    while ( comp[0] != '\0' )
    {
        comp = one_argument( comp, arg );
        consume = TRUE;
        fail = found = FALSE;
        val = -1;
        switch( arg[1] )
        {
        default:	check = arg+1;				break;
        case '!':	check = arg+2;	fail = TRUE;		break;
        case '+':	check = arg+2;	consume = FALSE;	break;
        case '@':	check = arg+2;	val = 0;		break;
        case '#':	check = arg+2;	val = 1;		break;
        case '$':	check = arg+2;	val = 2;		break;
        case '%':	check = arg+2;	val = 3;		break;
        case '^':	check = arg+2;	val = 4;		break;
        case '&':	check = arg+2;	val = 5;		break;
        }
        value = atoi(check);
        obj = NULL;
        switch( UPPER(arg[0]) )
        {
        case 'T':
            for ( obj = ch->first_carrying; obj; obj = obj->next_content )
                if ( obj->item_type == value )
                {
                    if ( fail )
                    {
                        send_to_char( "Something disrupts the casting of this spell...\n\r", ch );
                        return FALSE;
                    }
                    found = TRUE;
                    break;
                }
            break;
        case 'V':
            for ( obj = ch->first_carrying; obj; obj = obj->next_content )
                if ( obj->vnum == value )
                {
                    if ( fail )
                    {
                        send_to_char( "Something disrupts the casting of this spell...\n\r", ch );
                        return FALSE;
                    }
                    found = TRUE;
                    break;
                }
            break;
        case 'K':
            for ( obj = ch->first_carrying; obj; obj = obj->next_content )
                if ( nifty_is_name( check, obj->name ) )
                {
                    if ( fail )
                    {
                        send_to_char( "Something disrupts the casting of this spell...\n\r", ch );
                        return FALSE;
                    }
                    found = TRUE;
                    break;
                }
            break;
        case 'G':
            if ( GET_MONEY(ch, DEFAULT_CURR) >= value )
            {
                if ( fail )
                {
                    send_to_char( "Something disrupts the casting of this spell...\n\r", ch );
                    return FALSE;
                }
                else
                {
                    if ( consume )
                    {
                        set_char_color( AT_GOLD, ch );
                        send_to_char( "You feel a little lighter...\n\r", ch );
                        GET_MONEY(ch, DEFAULT_CURR) -= value;
                    }
                    continue;
                }
            }
            break;
        case 'H':
            if ( GET_HIT(ch) >= value )
            {
                if ( fail )
                {
                    send_to_char( "Something disrupts the casting of this spell...\n\r", ch );
                    return FALSE;
                }
                else
                {
                    if ( consume )
                    {
                        set_char_color( AT_BLOOD, ch );
                        send_to_char( "You feel a little weaker...\n\r", ch );
                        GET_HIT(ch) -= value;
                        update_pos( ch );
                    }
                    continue;
                }
            }
            break;
        }
        /* having this component would make the spell fail... if we get
         here, then the caster didn't have that component */
        if ( fail )
            continue;
        if ( !found )
        {
            send_to_char( "Something is missing...\n\r", ch );
            return FALSE;
        }
        if ( obj )
        {
            if ( val >=0 && val < 6 )
            {
                separate_obj(obj);
                if ( obj->value[val] <= 0 )
                    return FALSE;
                else
                    if ( --obj->value[val] == 0 )
                    {
                        act( AT_MAGIC, "$p glows briefly, then disappears in a puff of smoke!", ch, obj, NULL, TO_CHAR );
                        act( AT_MAGIC, "$p glows briefly, then disappears in a puff of smoke!", ch, obj, NULL, TO_ROOM );
                        extract_obj( obj );
                    }
                    else
                        act( AT_MAGIC, "$p glows briefly and a whisp of smoke rises from it.", ch, obj, NULL, TO_CHAR );
            }
            else
                if ( consume )
                {
                    separate_obj(obj);
                    act( AT_MAGIC, "$p glows brightly, then disappears in a puff of smoke!", ch, obj, NULL, TO_CHAR );
                    act( AT_MAGIC, "$p glows brightly, then disappears in a puff of smoke!", ch, obj, NULL, TO_ROOM );
                    extract_obj( obj );
                }
                else
                {
                    int count = obj->count;

                    obj->count = 1;
                    act( AT_MAGIC, "$p glows briefly.", ch, obj, NULL, TO_CHAR );
                    obj->count = count;
                }
        }
    }
    return TRUE;
}

int pAbort;

/*
 * Locate targets.
 */
void *locate_targets( CHAR_DATA *ch, char *arg, int sn,
                      CHAR_DATA **victim, OBJ_DATA **obj )
{
    SKILLTYPE *skill = get_skilltype( sn );
    void *vo	= NULL;

    *victim	= NULL;
    *obj	= NULL;

    switch ( skill->target )
    {
    default:
        bug( "Do_cast: bad target for sn %d.", sn );
        return &pAbort;

    case TAR_IGNORE:
        break;

    case TAR_CHAR_OFFENSIVE:
        if ( arg[0] == '\0' )
        {
            if ( ( *victim = who_fighting( ch ) ) == NULL )
            {
                if (skill->skill_fun!=do_smaug_skill)
                    send_to_char( "Cast the spell on whom?\n\r", ch );
                else
                    send_to_char( "I don't see them anywhere!\n\r", ch);
                return &pAbort;
            }
        }
        else
        {
            if ( ( *victim = get_char_room( ch, arg ) ) == NULL )
            {
                send_to_char( "They aren't here.\n\r", ch );
                return &pAbort;
            }
        }

        if ( is_safe( ch, *victim ) )
            return &pAbort;

        if ( ch == *victim )
        {
            send_to_char( "Cast this on yourself?  Okay...\n\r", ch );
            /*
             send_to_char( "You can't do that to yourself.\n\r", ch );
             return &pAbort;
             */
        }

        if ( !IS_NPC(ch) )
        {
            if ( !IS_NPC(*victim) )
            {
                /*  Sheesh! can't do anything
                 send_to_char( "You can't do that on a player.\n\r", ch );
                 return &pAbort;
                 */
                /*
                 if( IS_SET(*victim->act, PLR_PK))
                 */
                if ( get_timer( ch, TIMER_PKILLED ) > 0 )
                {
                    send_to_char( "You have been killed in the last 5 minutes.\n\r", ch);
                    return &pAbort;
                }

                if ( get_timer( *victim, TIMER_PKILLED ) > 0 )
                {
                    send_to_char( "This player has been killed in the last 5 minutes.\n\r", ch );
                    return &pAbort;
                }

                if ( *victim != ch)
                    send_to_char( "You really shouldn't do this to another player...\n\r", ch );
            }

            if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == *victim )
            {
                send_to_char( "You can't do that on your own follower.\n\r", ch );
                return &pAbort;
            }
        }

        if ( check_illegal_pk(ch, *victim) )
        {
            send_to_char( "The gods will not permit you to cast offensive spells on them.\n\r", ch );
            return &pAbort;
        }

        if ( ch != *victim ) adjust_favor( ch, 4, 1 );
        vo = (void *) *victim;
        break;

    case TAR_CHAR_DEFENSIVE:
        if ( arg[0] == '\0' )
            *victim = ch;
        else
        {
            if ( ( *victim = get_char_room( ch, arg ) ) == NULL )
            {
                send_to_char( "They aren't here.\n\r", ch );
                return &pAbort;
            }
        }
        if ( ( *victim != ch )
             && !IS_NPC( *victim ) ) adjust_favor( ch, 7, 1 );
        if ( ( *victim != ch )
             && !IS_NPC( ch ) ) adjust_favor( *victim, 13, 1 );
        vo = (void *) *victim;
        break;

    case TAR_CHAR_SELF:
        if ( arg[0] != '\0' &&
             !IS_NPC(ch) &&
             !nifty_is_name( arg, ch->name ) &&
             !nifty_is_name(arg, "self") )
        {
            send_to_char( "You cannot cast this spell on another.\n\r", ch );
            return &pAbort;
        }

        vo = (void *) ch;
        break;

    case TAR_OBJ_INV:
        if ( arg[0] == '\0' )
        {
            send_to_char( "What should the spell be cast upon?\n\r", ch );
            return &pAbort;
        }

        if ( ( *obj = get_obj_carry( ch, arg ) ) == NULL )
        {
            send_to_char( "You are not carrying that.\n\r", ch );
            return &pAbort;
        }

        vo = (void *) *obj;
        break;

    case TAR_OBJ_ROOM:
        if ( arg[0] == '\0' )
        {
            send_to_char( "What should the spell be cast upon?\n\r", ch );
            return &pAbort;
        }

        if ( ( *obj = get_obj_list_rev( ch, arg, ch->in_room->last_content ) ) == NULL )
        {
            send_to_char( "You are not carrying that.\n\r", ch );
            return &pAbort;
        }

        vo = (void *) *obj;
        break;
    }

    return vo;
}


void do_mind( CHAR_DATA *ch, char *argument )
{
    do_cast( ch, argument );
}

/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;

/*
 * Cast a spell.  Multi-caster and component support by Thoric
 */
void do_cast( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    static char staticbuf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    void *vo = NULL;
    int mana;
    int blood;
    int sn;
    int max = 0;
    ch_ret retcode;
    bool dont_wait = FALSE;
    SKILLTYPE *skill = NULL;
    struct timeval time_used;
    bool mind = FALSE;

    retcode = rNONE;

    if (ch->last_cmd == do_mind)
        mind = TRUE;

    if (IS_SYSTEMFLAG(SYS_NOMAGIC))
    {
        send_to_char("You can't seem to remember how to cast!  The Gods must be angry!\n\r", ch);
        return;
    }

    switch( ch->substate )
    {
    default:
        /* no ordering charmed mobs to cast spells */
        if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
        {
            send_to_char( "You can't seem to do that right now...\n\r", ch );
            return;
        }

        if (IS_SET(ch->in_room->room_flags, ROOM_NO_MAGIC))
        {
            set_char_color( AT_MAGIC, ch );
            send_to_char("Ancient Magiks bar your path.\n\r", ch);
            if (IS_IMMORTAL(ch))
                send_to_char("However, your immortality overcomes them.\n\r", ch);
            else
                return;
        }

        target_name = one_argument( argument, arg1 );
        one_argument( target_name, arg2 );

        if ( arg1[0] == '\0' )
	{
	    if (mind)
		send_to_char( "Mind which what where?\n\r", ch );
            else
		send_to_char( "Cast which what where?\n\r", ch );
            if (IS_NPC(ch))
                log_printf_plus(LOG_MAGIC, UMAX(GetMaxLevel(ch),LEVEL_LOG_CSET), SEV_DEBUG,
                                "do_cast: %s (u%d) cast with no arguments",
                                ch->name, ch->unum);
            return;
        }

        if ( get_trust(ch) < LEVEL_GOD )
	{
	    if (mind)
		sn=find_psispell(ch, arg1, TRUE);
	    else
		sn=find_spell(ch, arg1, TRUE);
	    if (sn < 0)
	    {
		send_to_char( "You can't do that.\n\r", ch );
		return;
	    }

            if (!CanUseSkill(ch, sn))
            {
                send_to_char( "You can't use that.\n\r", ch );
                return;
            }
        }
        else
        {
            if ( (sn=skill_lookup(arg1)) < 0 )
            {
                send_to_char( "We didn't create that yet...\n\r", ch );
                return;
            }
            if ( sn >= MAX_SKILL )
            {
                send_to_char( "Hmm... that might hurt.\n\r", ch );
                return;
            }
        }

	if ( (skill=get_skilltype(sn)) == NULL )
	{
	    send_to_char( "Something is severely wrong with that one...\n\r", ch );
	    return;
	}

	if ( mind && skill->type != SKILL_PSISPELL )
	{
	    send_to_char( "That isn't a psionic skill.\n\r", ch );
	    return;
	}
	if ( !mind && skill->type != SKILL_SPELL )
	{
	    send_to_char( "That isn't a spell.\n\r", ch );
	    return;
	}
        /*
         * Something else removed by Merc			-Thoric
         */
        if ( ch->position < skill->minimum_position )
        {
            switch( ch->position )
            {
            default:
                send_to_char( "You can't concentrate enough.\n\r", ch );
                break;
            case POS_SITTING:
                log_printf_plus(LOG_MAGIC, LEVEL_IMMORTAL, SEV_DEBUG, "%s trying to cast %s while sitting.",GET_NAME(ch),skill->name);
                send_to_char( "You can't summon enough energy sitting down.\n\r", ch );
                break;
            case POS_RESTING:
                log_printf_plus(LOG_MAGIC, LEVEL_IMMORTAL, SEV_DEBUG, "%s trying to cast %s while resting.",GET_NAME(ch),skill->name);
                send_to_char( "You're too relaxed to use that.\n\r", ch );
                break;
            case POS_FIGHTING:
                if (IS_NPC(ch))
                    log_printf_plus(LOG_MAGIC, LEVEL_IMMORTAL, SEV_DEBUG, "%s trying to cast %s while fighting.",GET_NAME(ch),skill->name);
                send_to_char( "You can't concentrate enough while fighting!\n\r", ch );
                break;
	    case POS_SLEEPING:
		if (mind)
		    send_to_char( "You dream about great feats of mental prowess.\n\r", ch );
		else
		    send_to_char( "You dream about great feats of magic.\n\r", ch );
                break;
            }
            return;
        }

        if ( skill->spell_fun == spell_null )
        {
            send_to_char( "That's not a spell!\n\r", ch );
            bug("Null spell: %s", skill->name);
            return;
        }

        if ( !skill->spell_fun )
        {
            send_to_char( "You cannot use that... yet.\n\r", ch );
            bug("Spell with no fun: %s", skill->name);
            return;
        }

        /* psi's don't speak their spells */
        if (!mind)
        {
            if ( ch->in_room->sector_type == SECT_UNDERWATER )
            {
                send_to_char("Speak underwater, are you mad????\n\r", ch);
                return;
            }

            if ( IS_ROOM_FLAG( ch->in_room, ROOM_SILENCE ) )
            {
                send_to_char("You are in a silent zone, you can't make a sound!\n\r", ch);
                return;
            }

            if (IS_SILENCED(ch))
            {
                send_to_char("You are silenced, you can't make a sound!\n\r", ch);
                return;
            }
        }

        if ( !IS_NPC(ch)			/* fixed by Thoric */
             &&   !IS_IMMORTAL(ch)
             &&    skill->guild != CLASS_NONE
             &&  (!ch->pcdata->clan
                  || skill->guild != ch->pcdata->clan->cl) )
        {
            send_to_char( "That is only available to members of a certain guild.\n\r", ch);
            return;
        }

        mana = (IS_NPC(ch) || MEMORIZED(ch, sn)) ? 0 : skill->min_mana;
        /*
         * Locate targets.
         */
        vo = locate_targets( ch, arg2, sn, &victim, &obj );
        if ( vo == &pAbort )
            return;

        /*
         * Vampire spell casting				-Thoric
         */
        blood = UMAX(1, (mana+4) / 8);      /* NPCs don't have PCDatas. -- Altrag */
        if ( IS_VAMPIRE(ch) )
        {
            if (GET_COND(ch, COND_BLOODTHIRST) < blood)
            {
                send_to_char( "You don't have enough blood power.\n\r", ch );
                return;
            }
        }
        else
            if ( !IS_NPC(ch) && GET_MANA(ch) < mana )
            {
                send_to_char( "You don't have enough mana.\n\r", ch );
                return;
            }
        if ( skill->participants <= 1 )
            break;
        /* multi-participant spells			-Thoric */
        add_timer( ch, TIMER_DO_FUN,
                   UMAX(skill->beats/SPELL_BEATS_PER_ROUND, 1),
                   do_cast, 1 );

        if (skill->part_start_char && *skill->part_start_char)
            act( AT_MAGIC, skill->part_start_char, ch, NULL, NULL, TO_CHAR );
        else
            act( AT_MAGIC, "You begin to chant...", ch, NULL, NULL, TO_CHAR );
        if (skill->part_start_room && *skill->part_start_room)
            act( AT_MAGIC, skill->part_start_room, ch, NULL, NULL, TO_ROOM );
        else
            act( AT_MAGIC, "$n begins to chant...", ch, NULL, NULL, TO_ROOM );

        sprintf( staticbuf, "'%s' %s", arg2, target_name );
        ch->dest_buf = str_dup( staticbuf );
        ch->tempnum = sn;
        return;
    case SUB_TIMER_DO_ABORT:
        DISPOSE( ch->dest_buf );
        if ( IS_VALID_SN((sn = ch->tempnum)) )
        {
            if ( (skill=get_skilltype(sn)) == NULL )
            {
                send_to_char( "Something went wrong...\n\r", ch );
                bug( "do_cast: SUB_TIMER_DO_ABORT: bad sn %d", sn );
                return;
            }
            mana = IS_NPC(ch) ? 0 : UMAX(skill->min_mana,
                                         100 / UMAX(2,(2 + BestSkLv(ch, sn)) - skill->skill_level[BestSkCl(ch, sn)]));
            blood = UMAX(1, (mana+4) / 8);
            if ( IS_VAMPIRE(ch) )
                gain_condition( ch, COND_BLOODTHIRST, - UMAX(1, (blood >> 1)) );
            else if (!IS_IMMORTAL(ch))    /* so imms dont lose mana */
                GET_MANA(ch) -= (mana >> 1);
        }
        if (skill->part_abort_char && *skill->part_abort_char)
            act( AT_MAGIC, skill->part_abort_char, ch, NULL, NULL, TO_CHAR );
        else
            act( AT_MAGIC, "You stop chanting...", ch, NULL, NULL, TO_CHAR );
        /* should add chance of backfire here */
        return;
    case 1:
        sn = ch->tempnum;
        if ( (skill=get_skilltype(sn)) == NULL )
        {
            send_to_char( "Something went wrong...\n\r", ch );
            bug( "do_cast: substate 1: bad sn %d", sn );
            return;
        }
        if ( !ch->dest_buf || !IS_VALID_SN(sn) || skill->type != SKILL_SPELL )
        {
            send_to_char( "Something cancels out the spell!\n\r", ch );
            bug( "do_cast: ch->dest_buf NULL or bad sn (%d)", sn );
            return;
        }
        mana = IS_NPC(ch) ? 0 : UMAX(skill->min_mana,
                                     100 / UMAX(2,(2 + BestSkLv(ch, sn)) - skill->skill_level[BestSkCl(ch, sn)]) );
        blood = UMAX(1, (mana+4) / 8);
        strcpy( staticbuf, (char *)ch->dest_buf );
        target_name = one_argument(staticbuf, arg2);
        DISPOSE( ch->dest_buf );
        ch->substate = SUB_NONE;
        if ( skill->participants > 1 )
        {
            int cnt = 1;
            CHAR_DATA *tmp;
            TIMER *t = NULL;

            for ( tmp = ch->in_room->first_person; tmp; tmp = tmp->next_in_room )
                if (  tmp != ch
                      &&   (t = get_timerptr( tmp, TIMER_DO_FUN )) != NULL
                      &&    t->count >= 1 && t->do_fun == do_cast
                      &&    tmp->tempnum == sn && tmp->dest_buf
                      &&   !str_cmp( (char *)tmp->dest_buf, staticbuf ) )
                    ++cnt;

            if ( cnt >= skill->participants )
            {
                for ( tmp = ch->in_room->first_person; tmp; tmp = tmp->next_in_room )
                    if (  tmp != ch
                          &&   (t = get_timerptr( tmp, TIMER_DO_FUN )) != NULL
                          &&    t->count >= 1 && t->do_fun == do_cast
                          &&    tmp->tempnum == sn && tmp->dest_buf
                          &&   !str_cmp( (char *)tmp->dest_buf, staticbuf ) )
                    {
                        extract_timer( tmp, t );

                        if (skill->part_end_vict && *skill->part_end_vict)
                            act( AT_MAGIC, skill->part_end_vict, ch, NULL, tmp, TO_VICT );
                        else
                            act( AT_MAGIC, "You channel your energy into $n!", ch, NULL, tmp, TO_VICT );
                        if (skill->part_end_char && *skill->part_end_char)
                            act( AT_MAGIC, skill->part_end_char, ch, NULL, tmp, TO_CHAR );
                        else
                            act( AT_MAGIC, "$N channels $S energy into you!", ch, NULL, tmp, TO_CHAR );
                        if (skill->part_end_room && *skill->part_end_room)
                            act( AT_MAGIC, skill->part_end_room, ch, NULL, tmp, TO_NOTVICT );
                        else
                            act( AT_MAGIC, "$N channels $S energy into $n!", ch, NULL, tmp, TO_NOTVICT );

                        learn_from_success( tmp, sn );
                        if ( IS_VAMPIRE(ch) )
                            gain_condition( tmp, COND_BLOODTHIRST, - blood/2);
                        else if (!IS_IMMORTAL(tmp))
                            GET_MANA(tmp) -= mana;
                        tmp->substate = SUB_NONE;
                        tmp->tempnum = -1;
                        DISPOSE( tmp->dest_buf );
                    }
                dont_wait = TRUE;
                if (skill->part_end_caster && *skill->part_end_caster)
                    act( AT_MAGIC, skill->part_end_caster, ch, NULL, NULL, TO_CHAR );
                else
                    act( AT_MAGIC, "You concentrate all the energy into a burst of mystical words!", ch, NULL, NULL, TO_CHAR );
                vo = locate_targets( ch, arg2, sn, &victim, &obj );
                if ( vo == &pAbort )
                    return;
            }
            else
            {
                if (skill->part_miss_char && *skill->part_miss_char)
                    act( AT_MAGIC, skill->part_miss_char, ch, NULL, NULL, TO_CHAR );
                else
                    act( AT_MAGIC, "There was not enough power the spell to succeed...", ch, NULL, NULL, TO_CHAR );
                if (skill->part_miss_room && *skill->part_miss_room)
                    act( AT_MAGIC, skill->part_miss_room, ch, NULL, NULL, TO_ROOM );

                if ( IS_VAMPIRE(ch) )
                    gain_condition( ch, COND_BLOODTHIRST, - UMAX(1, blood / 2) );
                else if (!IS_IMMORTAL(ch))    /* so imms dont lose mana */
                    GET_MANA(ch) -= mana/2;
                learn_from_failure( ch, sn );
                return;
            }
        }
    }

    if ( str_cmp( skill->name, "ventriloquate" ) && !mind )
        say_spell( ch, sn );

    /*
     * Getting ready to cast... check for spell components	-Thoric
     */
    if ( !process_spell_components( ch, sn ) )
    {
        if ( IS_VAMPIRE(ch) )
            gain_condition( ch, COND_BLOODTHIRST, - UMAX(1, blood / 2) );
        else if (!IS_IMMORTAL(ch))    /* so imms dont lose mana */
            GET_MANA(ch) -= mana / 2;
        learn_from_failure( ch, sn );
        if ( !dont_wait )
            spell_lag(ch, sn);
        return;
    }

    if (IS_NPC(ch) && !IS_ACT_FLAG(ch, ACT_POLYMORPHED))
    {
        max = 1;
    }
    else
    {
        int cl;

        max = 0;

        for (cl = 0; cl < MAX_CLASS; cl++)
        {
            if (!IS_ACTIVE(ch, cl))
                continue;

            switch (cl)
            {
            default:
                break;
            case CLASS_MAGE:
            case CLASS_SORCERER:
                if (EqWBits(ch, ITEM_ANTI_MAGE))
                    max += 10;
                break;
            case CLASS_NECROMANCER:
                if (EqWBits2(ch, ITEM2_ANTI_NECROMANCER))
                    max += 10;
                break;
            case CLASS_CLERIC:
                if (EqWBits(ch, ITEM_ANTI_CLERIC))
                    max += 10;
                break;
            case CLASS_DRUID:
                if (EqWBits(ch, ITEM_ANTI_DRUID))
                    max += 10;
                break;
            case CLASS_PALADIN:
                if (EqWBits2(ch, ITEM2_ANTI_PALADIN))
                    max += 5;
                break;
            case CLASS_ANTIPALADIN:
                if (EqWBits2(ch, ITEM2_ANTI_APALADIN))
                    max += 5;
                break;
            case CLASS_PSIONIST:
                if (EqWBits2(ch, ITEM2_ANTI_PSI))
                    max += 1;
                break;
            case CLASS_RANGER:
                if (EqWBits2(ch, ITEM2_ANTI_RANGER))
                    max += 5;
                break;
            case CLASS_VAMPIRE:
                if (EqWBits(ch, ITEM_ANTI_VAMPIRE))
                    max += 10;
                break;
            case CLASS_ARTIFICER:
                if (EqWBits2(ch, ITEM2_ANTI_ARTIFICER))
                    max += 10;
                break;
            case CLASS_MONK:
                if (EqWBits2(ch, ITEM2_ANTI_MONK))
                    max += 1;
                break;
            case CLASS_THIEF:
                if (EqWBits(ch, ITEM_ANTI_THIEF))
                    max += 1;
                break;
            case CLASS_AMAZON:
                if (EqWBits2(ch, ITEM2_ANTI_AMAZON))
                    max += 1;
                break;
            case CLASS_WARRIOR:
                if (EqWBits(ch, ITEM_ANTI_WARRIOR))
                    max += 1;
                break;
            }
        }

        if (max > 0 && GetMaxLevel(ch)<5)
            send_to_char("Note: You are wearing anti-class eq, removing it will improve your spellfail.\n\r(This message will self-destruct when you reach level 5)\n\r", ch);

        max += ch->spellfail + (skill->difficulty * 5) + (GET_COND(ch, COND_DRUNK) * 2);

        log_printf_plus(LOG_DEBUG, LEVEL_IMMORTAL, SEV_DEBUG, "do_cast: %s spellfail: %d  sn: %d", GET_NAME(ch), max, sn);
    }

    if (IS_IMMORTAL(ch))
        max = 1;

    /* memorized spells don't use mana nor do they fail */
    if ( MEMORIZED(ch, sn) )
    {
        int adept;

        forget_spell(ch, sn);

        adept = GET_ADEPT(ch, sn);

        if (!IS_NPC(ch) && LEARNED(ch, sn) < adept)
            ch->pcdata->learned[sn] = UMIN( adept, LEARNED(ch, sn) + 1);
    }
    else
    {
        if (IS_ACTIVE(ch, CLASS_SORCERER))
        {
            int cl;

            for (cl=0;cl<MAX_CLASS;cl++)
            {
                if (cl == CLASS_SORCERER)
                    continue;
                if (CanUseSkillClass(ch, sn, cl))
                    break;
            }
            if (!CanUseSkillClass(ch, sn, cl) || cl == CLASS_SORCERER)
            {
                send_to_char("You must memorize that before you can cast it.\n\r", ch);
                return;
            }
            else if (cl < MAX_CLASS && !IS_IMMORTAL(ch))
            {
                bug("Sorcerer %s casting %s as %s",
                    GET_NAME(ch), skill->name, pc_class[cl]);
            }
        }

        if ( number_range(1, max) > LEARNED(ch, sn) )
        {
            /* Some more interesting loss of concentration messages  -Thoric */
            switch( number_bits(2) )
            {
            case 0:	/* too busy */
                if ( ch->fighting )
                    send_to_char( "This round of battle is too hectic to concentrate properly.\n\r", ch );
                else
                    send_to_char( "You lost your concentration.\n\r", ch );
                break;
            case 1:	/* irritation */
                if ( number_bits(2) == 0 )
                {
                    switch( number_bits(2) )
                    {
                    case 0: send_to_char( "A tickle in your nose prevents you from keeping your concentration.\n\r", ch ); break;
                    case 1: send_to_char( "An itch on your leg keeps you from properly casting your spell.\n\r", ch ); break;
                    case 2: send_to_char( "Something in your throat prevents you from uttering the proper phrase.\n\r", ch ); break;
                    case 3: send_to_char( "A twitch in your eye disrupts your concentration for a moment.\n\r", ch ); break;
                    }
                }
                else
                    send_to_char( "Something distracts you, and you lose your concentration.\n\r", ch );
                break;
            case 2:	/* not enough time */
                if ( ch->fighting )
                    send_to_char( "There wasn't enough time this round to complete the casting.\n\r", ch );
                else
                    send_to_char( "You lost your concentration.\n\r", ch );
                break;
            case 3:
                send_to_char( "You get a mental block mid-way through the casting.\n\r", ch );
                break;
            }
            if ( IS_VAMPIRE(ch) )
                gain_condition( ch, COND_BLOODTHIRST, - UMAX(1, blood) );
            else if (!IS_IMMORTAL(ch))    /* so imms dont lose mana */
                GET_MANA(ch) -= (mana >> 1);
            learn_from_failure( ch, sn );
            if ( !dont_wait )
                spell_lag(ch, sn);
            return;
        }

        if ( IS_VAMPIRE(ch) )
            gain_condition( ch, COND_BLOODTHIRST, - blood/2);
        else if (!IS_IMMORTAL(ch))
            GET_MANA(ch) -= mana;
    }

    if ( !dont_wait )
        spell_lag(ch, sn);

    if (!IS_NPC(ch))
        log_printf_plus(LOG_MAGIC, UMAX(GetMaxLevel(ch),LEVEL_LOG_CSET), SEV_DEBUG, "%s: Caster: %s, Level: %d, Learned: %d, Use: %d, Used: %d",
                        skill->name, GET_NAME(ch), BestSkLv(ch,sn), LEARNED(ch, sn), skill->min_mana, mana);

    /*
     * check for immunity to magic if victim is known...
     * and it is a TAR_CHAR_DEFENSIVE/SELF spell
     * otherwise spells will have to check themselves
     */
    if (victim && GET_AMAGICP(victim) > number_percent())
    {
        send_to_char( "Your magic was absorbed!\n\r", ch);
        return;
    }

    if ( (skill->target == TAR_CHAR_DEFENSIVE ||
          skill->target == TAR_CHAR_SELF) &&
         victim && IS_SET(victim->immune, RIS_MAGIC) )
    {
        immune_casting( skill, ch, victim, NULL );
        retcode = rSPELL_FAILED;
    }
    else if (victim && is_immune(victim, skill))
    {
        immune_casting( skill, ch, victim, NULL );
        log_printf_plus(LOG_MAGIC, UMAX(GetMaxLevel(ch), GetMaxLevel(victim)), SEV_DEBUG, "is_immune: Spell: %s, Caster: %s, Target: %s",
                        skill->name, GET_NAME(ch), GET_NAME(victim));
        retcode = rSPELL_FAILED;
    }
    else
    {
        start_timer(&time_used);
        retcode = (*skill->spell_fun) ( sn, BestSkLv(ch, sn), ch, vo );
        end_timer(&time_used);
        update_userec(&time_used, &skill->userec);
    }

    if ( retcode == rCHAR_DIED || retcode == rERROR || char_died(ch) )
        return;
    if ( retcode != rSPELL_FAILED )
        learn_from_success( ch, sn );
    else
        learn_from_failure( ch, sn );

    if (ch==supermob)
        return;

    /*
     * Fixed up a weird mess here, and added double safeguards	-Thoric
     */

    if ( skill->target == TAR_CHAR_OFFENSIVE
         &&   victim
         &&  !char_died(victim)
         &&	 victim != ch )
    {
        CHAR_DATA *vch, *vch_next;

        for ( vch = ch->in_room->first_person; vch; vch = vch_next )
        {
            vch_next = vch->next_in_room;

            if ( vch == victim )
            {
                if ( victim->master != ch
                     &&  !victim->fighting )
                    retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
                break;
            }
        }
    }

    return;
}


/*
 * Cast spells at targets using a magical object.
 */
ch_ret obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
    void *vo;
    ch_ret retcode = rNONE;
    int levdiff = GetAveLevel(ch) - level;
    SKILLTYPE *skill = get_skilltype( sn );
    struct timeval time_used;

    if ( sn == -1 )
        return retcode;
    if ( !skill || !skill->spell_fun )
    {
        bug( "Obj_cast_spell: bad sn %d.", sn );
        return rERROR;
    }

    if (IS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC))
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "The magic fizzles and dies, nothing happens...\n\r", ch );
        return rNONE;
    }

    /*
     * Basically this was added to cut down on level 5 players using level
     * 40 scrolls in battle too often ;)		-Thoric
     */
    if ( (skill->target == TAR_CHAR_OFFENSIVE
          ||    number_bits(7) == 1)	/* 1/128 chance if non-offensive */
         &&    skill->type != SKILL_HERB
         &&   !chance( ch, 95 + levdiff ) )
    {
        switch( number_bits(2) )
        {
        case 0: failed_casting( skill, ch, victim, NULL );	break;
        case 1:
            act( AT_MAGIC, "The $t spell backfires!", ch, skill->name, victim, TO_CHAR );
            if ( victim )
                act( AT_MAGIC, "$n's $t spell backfires!", ch, skill->name, victim, TO_VICT );
            act( AT_MAGIC, "$n's $t spell backfires!", ch, skill->name, victim, TO_NOTVICT );
            return damage( ch, ch, number_range( 1, level ), TYPE_UNDEFINED );
        case 2: failed_casting( skill, ch, victim, NULL );	break;
        case 3:
            act( AT_MAGIC, "The $t spell backfires!", ch, skill->name, victim, TO_CHAR );
            if ( victim )
                act( AT_MAGIC, "$n's $t spell backfires!", ch, skill->name, victim, TO_VICT );
            act( AT_MAGIC, "$n's $t spell backfires!", ch, skill->name, victim, TO_NOTVICT );
            return damage( ch, ch, number_range( 1, level ), TYPE_UNDEFINED );
        }
        return rNONE;
    }

    target_name = "";
    switch ( skill->target )
    {
    default:
        bug( "Obj_cast_spell: bad target for sn %d.", sn );
        return rERROR;

    case TAR_IGNORE:
        vo = NULL;
        if ( victim )
            target_name = victim->name;
        else
            if ( obj )
                target_name = obj->name;
        break;

    case TAR_CHAR_OFFENSIVE:
        if ( victim != ch )
        {
            if ( !victim )
                victim = who_fighting( ch );
            if ( !victim || !IS_NPC(victim) )
            {
                send_to_char( "You can't do that.\n\r", ch );
                return rNONE;
            }
        }
        if ( ch != victim && is_safe( ch, victim ) )
            return rNONE;
        vo = (void *) victim;
        break;

    case TAR_CHAR_DEFENSIVE:
        if ( victim == NULL )
            victim = ch;
        vo = (void *) victim;
        if ( skill->type != SKILL_HERB
             &&   IS_SET(victim->immune, RIS_MAGIC ) )
        {
            immune_casting( skill, ch, victim, NULL );
            return rNONE;
        }
        break;

    case TAR_CHAR_SELF:
        vo = (void *) ch;
        if ( skill->type != SKILL_HERB
             &&   IS_SET(ch->immune, RIS_MAGIC ) )
        {
            immune_casting( skill, ch, victim, NULL );
            return rNONE;
        }
        break;

    case TAR_OBJ_INV:
        if ( obj == NULL )
        {
            send_to_char( "You can't do that.\n\r", ch );
            return rNONE;
        }
        vo = (void *) obj;
        break;
    }

    start_timer(&time_used);
    retcode = (*skill->spell_fun) ( sn, level, ch, vo );
    end_timer(&time_used);
    update_userec(&time_used, &skill->userec);

    if ( retcode == rSPELL_FAILED )
        retcode = rNONE;

    if ( retcode == rCHAR_DIED || retcode == rERROR )
        return retcode;

    if ( char_died(ch) )
        return rCHAR_DIED;

    if ( skill->target == TAR_CHAR_OFFENSIVE
         &&   victim != ch
         &&  !char_died(victim) )
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

        for ( vch = ch->in_room->first_person; vch; vch = vch_next )
        {
            vch_next = vch->next_in_room;
            if ( victim == vch && !victim->fighting && victim->master != ch )
            {
                retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
                break;
            }
        }
    }

    return retcode;
}

/* this function is to handle changes in PULSE_PER_SECOND for spell lag */
void spell_lag(CHAR_DATA *ch, int sn)
{
    int pulses;

    SKILLTYPE *skill = get_skilltype(sn);

    if (!skill)
        return;

    /*
     * convert beats to pulses for pc's
     * npc's run at 1/second so formula is different
     */
    if (!IS_NPC(ch))
        pulses = (skill->beats*PULSE_VIOLENCE)/SPELL_BEATS_PER_ROUND;
    else
        pulses = (skill->beats*4)/SPELL_BEATS_PER_ROUND;

    WAIT_STATE(ch, pulses);

}

void make_charmie(CHAR_DATA *ch, CHAR_DATA *mob, int sn)
{
    AFFECT_DATA af;

    GET_ALIGN(mob) = GET_ALIGN(ch);
    REMOVE_ACT_FLAG(mob, ACT_META_AGGR);
    REMOVE_ACT_FLAG(mob, ACT_AGGRESSIVE);
    REMOVE_ACT_FLAG(mob, ACT_SENTINEL);

    if ( mob->master )
        stop_follower( mob );

    add_follower( mob, ch );

    af.type      = sn;
    af.duration  = (int)((24 * (UMAX(12, get_curr_cha(ch)) / 12)) * DUR_CONV);
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( mob, &af );
}


/*
 * Spell functions.
 */
ch_ret spell_acid_blast( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice( level, 6 );
    if ( saves_spell_staff( level, victim ) )
        dam /= 2;
    return damage( ch, victim, dam, sn );
}


ch_ret spell_blindness( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int tmp;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( SPELL_FLAG(skill, SF_PKSENSITIVE)
         &&  !IS_NPC(ch) && !IS_NPC(victim) )
        tmp = level/2;
    else
        tmp = level;

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }
    if ( is_affected(victim, gsn_blindness) ||
         IS_AFFECTED(victim, AFF_BLIND) ||
         saves_spell_staff( tmp, victim ) )
    {
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    af.type      = sn;
    af.duration  = (short int)((1 + (level / 2)) * DUR_CONV);
    af.bitvector = AFF_BLIND;

    if ( SPELL_POWER(skill) > SP_NONE )
        af.duration *= SPELL_POWER(skill);

    af.location  = APPLY_HITROLL;
    af.modifier  = -4;
    affect_to_char( victim, &af );

    af.location  = APPLY_AC;
    af.modifier  = 20;
    affect_to_char( victim, &af );

    set_char_color( AT_MAGIC, victim );
    send_to_char( "You have been blinded!\n\r", victim );
    if ( ch != victim )
        act(AT_MAGIC,"$n seems to be blinded!",victim,NULL,NULL,TO_ROOM);
    return rNONE;
}


ch_ret spell_burning_hands( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] =
    {
        0,
        0,  0,  0,  0,	14,	17, 20, 23, 26, 29,
        29, 29, 30, 30,	31,	31, 32, 32, 33, 33,
        34, 34, 35, 35,	36,	36, 37, 37, 38, 38,
        39, 39, 40, 40,	41,	41, 42, 42, 43, 43,
        44, 44, 45, 45, 46, 46, 47, 47, 48, 48,
        49, 49, 50, 50, 51, 51, 52, 52, 53, 53,
        54, 54, 55, 55, 56, 56, 57, 57, 58, 58
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell_staff( level, victim ) )
        dam /= 2;
    return damage( ch, victim, dam, sn );
}



ch_ret spell_call_lightning( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    bool ch_died;
    ch_ret retcode = 0;

    if ( !IS_OUTSIDE(ch) || ch->in_room->area->weather->precip<=0 )
    {
        send_to_char( "The proper atmospheric conditions are not at hand.\n\r", ch );
        return rSPELL_FAILED;
    }

    dam = dice(level/2, 8);

    set_char_color( AT_MAGIC, ch );
    send_to_char( "God's lightning strikes your foes!\n\r", ch );
    act( AT_MAGIC, "$n calls God's lightning to strike $s foes!",
         ch, NULL, NULL, TO_ROOM );

    ch_died = FALSE;
    for ( vch = first_char; vch; vch = vch_next )
    {
        vch_next	= vch->next;
        if ( !vch->in_room )
            continue;
        if ( vch->in_room == ch->in_room )
        {
            if ( !IS_NPC( vch ) && vch->pcdata->wizinvis >= LEVEL_IMMORTAL )
                continue;

            if ( vch != ch && ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) ) )
                retcode = damage( ch, vch, saves_spell_staff( level, vch ) ? dam/2 : dam, sn );
            if ( retcode == rCHAR_DIED || char_died(ch) )
                ch_died = TRUE;
            continue;
        }

        if ( !ch_died
             &&   vch->in_room->area == ch->in_room->area
             &&   IS_OUTSIDE(vch)
             &&   IS_AWAKE(vch) ) {
            set_char_color( AT_MAGIC, vch );
            send_to_char( "Lightning flashes in the sky.\n\r", vch );
        }
    }

    if ( ch_died )
        return rCHAR_DIED;
    else
        return rNONE;
}


ch_ret spell_change_sex( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }
    if ( is_affected( victim, sn ) )
        return rSPELL_FAILED;
    af.type      = sn;
    af.duration  = (int)(10 * level * DUR_CONV);
    af.location  = APPLY_SEX;
    do
    {
        af.modifier  = number_range( 0, 2 ) - victim->sex;
    }
    while ( af.modifier == 0 );
    af.bitvector = 0;
    affect_to_char( victim, &af );
    set_char_color( AT_MAGIC, victim );
    send_to_char( "You feel different.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return rNONE;
}


ch_ret spell_charm_person( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int saves;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( victim == ch )
    {
        send_to_char( "You like yourself even better!\n\r", ch );
        return rSPELL_FAILED;
    }

    if ( !IsPerson(victim) ||
         IS_SET( victim->immune, RIS_MAGIC ) ||
         IS_SET( victim->immune, RIS_CHARM ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    saves = ris_save( victim, level, RIS_CHARM );

    if ( IS_AFFECTED(victim, AFF_CHARM)
         ||   saves == 1000
         ||   IS_AFFECTED(ch, AFF_CHARM)
         ||   GetMaxLevel(victim) > level+10
         ||   circle_follow( victim, ch )
         ||   saves_spell_staff( saves, victim ) )
    {
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    if (too_many_followers(ch))
    {
        act(AT_MAGIC, "You are not charismatic enough to charm that many.", ch, NULL, victim, TO_CHAR);
        act(AT_MAGIC, "$n is not charismatic enough to charm $N.", ch, NULL, victim, TO_ROOM);
        return rSPELL_FAILED;
    }

    make_charmie(ch, victim, sn);

    act( AT_MAGIC, "Isn't $n just such a nice fellow?", ch, NULL, victim, TO_VICT );
    act( AT_MAGIC, "$N's eyes glaze over...", ch, NULL, victim, TO_ROOM );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );

    sprintf( log_buf, "%s has charmed %s.", ch->name, victim->name);
    log_string_plus( log_buf, LOG_MONITOR, GetMaxLevel(ch), SEV_NOTICE );

    return rNONE;
}


ch_ret spell_chill_touch( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int dam;

    dam = number_range( level, level*3 );

    if ( !saves_spell_staff( level, victim ) )
    {
        af.type      = sn;
        af.duration  = (int)(6 * DUR_CONV);
        af.location  = APPLY_STR;
        af.modifier  = -1;
        af.bitvector = 0;
        affect_join( victim, &af );
    }
    else
    {
        dam /= 2;
    }

    return damage( ch, victim, dam, sn );
}



ch_ret spell_colour_spray( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = 4 * level;

    if ( saves_spell_staff( level, victim ) )
        dam /= 2;

    return damage( ch, victim, dam, sn );
}


ch_ret spell_control_weather( int sn, int level, CHAR_DATA *ch, void *vo )
{
    SKILLTYPE *skill = get_skilltype(sn);
    WEATHER_DATA *weath;
    int change;

    if ( !IS_OUTSIDE(ch) )
    {
        send_to_char( "You need to be outside.\n\r", ch );
        return rSPELL_FAILED;
    }

    weath = ch->in_room->area->weather;

    change = number_range(-rand_factor, rand_factor) +
        (GetMaxLevel(ch)*3)/(2*max_vector);

    if ( !str_cmp( target_name, "warmer" ) )
        weath->temp_vector += change;
    else if ( !str_cmp( target_name, "colder" ) )
        weath->temp_vector -= change;
    else if(!str_cmp(target_name, "wetter"))
        weath->precip_vector += change;
    else if(!str_cmp(target_name, "drier"))
        weath->precip_vector -= change;
    else if(!str_cmp(target_name, "windier"))
        weath->wind_vector += change;
    else if(!str_cmp(target_name, "calmer"))
        weath->wind_vector -= change;
    else
    {
        send_to_char ("Do you want it to get warmer, colder, wetter, "
                      "drier, windier, or calmer?\n\r", ch );
        return rSPELL_FAILED;
    }
    weath->temp_vector = URANGE(-max_vector,
                                weath->temp_vector, max_vector);
    weath->precip_vector = URANGE(-max_vector,
                                  weath->precip_vector, max_vector);
    weath->wind_vector = URANGE(-max_vector,
                                weath->wind_vector, max_vector);

    successful_casting( skill, ch, NULL, NULL );
    return rNONE;
}


ch_ret spell_create_food( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *mushroom;

    mushroom = create_object( OBJ_VNUM_MUSHROOM );
    if (!mushroom)
    {
        act( AT_MAGIC, "Something blinks in then out of existance.", ch, NULL, NULL, TO_ROOM );
        act( AT_MAGIC, "Something blinks in then out of existance.", ch, NULL, NULL, TO_CHAR );
        return rNONE;
    }
    mushroom->value[0] = 5 + level;
    act( AT_MAGIC, "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM );
    act( AT_MAGIC, "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR );
    mushroom = obj_to_room( mushroom, ch->in_room );
    return rNONE;
}


ch_ret spell_create_water( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int water;

    if ( obj->item_type != ITEM_DRINK_CON )
    {
        send_to_char( "It is unable to hold water.\n\r", ch );
        return rSPELL_FAILED;
    }

    if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
    {
        send_to_char( "It contains some other liquid.\n\r", ch );
        return rSPELL_FAILED;
    }

    water = UMIN(
                 level * (ch->in_room->area->weather->precip > 0 ? 4 : 2),
                 obj->value[0] - obj->value[1]
                );

    if ( water > 0 )
    {
        separate_obj(obj);
        obj->value[2] = LIQ_WATER;
        obj->value[1] += water;
        if ( !is_name( "water", obj->name ) )
        {
            char buf[MAX_STRING_LENGTH];

            sprintf( buf, "%s water", obj->name );
            STRFREE( obj->name );
            obj->name = STRALLOC( buf );
        }
        act( AT_MAGIC, "$p is filled.", ch, obj, NULL, TO_CHAR );
    }

    return rNONE;
}


ch_ret spell_cure_blindness( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    if ( !is_affected( victim, gsn_blindness ) )
        return rSPELL_FAILED;
    affect_strip( victim, gsn_blindness );
    set_char_color( AT_MAGIC, victim);
    send_to_char( "Your vision returns!\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return rNONE;
}


ch_ret spell_cure_poison( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    if ( is_affected( victim, gsn_poison ) )
    {
        affect_strip( victim, gsn_poison );
        act( AT_MAGIC, "$N looks better.", ch, NULL, victim, TO_ROOM );
        set_char_color( AT_MAGIC, victim);
        send_to_char( "A warm feeling runs through your body.\n\r", victim );
        victim->mental_state = URANGE( -100, victim->mental_state, -10 );
        send_to_char( "Ok.\n\r", ch );
        return rNONE;
    }
    else
        return rSPELL_FAILED;
}


ch_ret spell_curse( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = NULL;
    CHAR_DATA *victim = NULL;
    AFFECT_DATA af;
    SKILLTYPE *skill = get_skilltype(sn);

    if ((obj = get_obj_carry(ch, target_name)))
    {
        SET_OBJ_STAT(obj, ITEM_ANTI_GOOD);
        SET_OBJ_STAT(obj, ITEM_NODROP);
        SET_OBJ_STAT(obj, ITEM_NOREMOVE);

        /* LOWER ATTACK DICE BY -1 */
        if(obj->item_type == ITEM_WEAPON)
            obj->value[2]--;
        act(AT_MAGIC, "$p glows red.", ch, obj, NULL, TO_CHAR);

        return rNONE;
    }

    if (!(victim = get_char_room(ch, target_name)))
    {
        act(AT_MAGIC, "Nobody here by that name.", ch, NULL, NULL, TO_CHAR);
        return rNONE;
    }

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    if ( saves_spell_staff(level, victim) ||
         is_affected(victim, gsn_curse))
    {
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    af.type      = sn;
    af.duration  = (int)(24*7*DUR_CONV);
    af.modifier  = -1;
    af.location  = APPLY_HITROLL;
    af.bitvector = AFF_CURSE;
    affect_to_char(victim, &af);

    af.location = APPLY_SAVING_PARA;
    af.modifier = 1;
    affect_to_char(victim, &af);

    successful_casting( skill, ch, victim, NULL );

    if (IS_NPC(victim) && !who_fighting(victim))
        set_fighting(victim,ch);

    if (!IS_NPC(ch))
        ch->alignment -= 2;

    return rNONE;
}


ch_ret spell_detect_poison( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    set_char_color( AT_MAGIC, ch);
    if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
    {
        if ( obj->value[3] != 0 )
            send_to_char( "You smell poisonous fumes.\n\r", ch );
        else
            send_to_char( "It looks very delicious.\n\r", ch );
    }
    else
    {
        send_to_char( "It doesn't look poisoned.\n\r", ch );
    }

    return rNONE;
}


ch_ret spell_dispel_evil( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( !IS_NPC(ch) && IS_EVIL(ch) )
        victim = ch;

    if ( IS_GOOD(victim) )
    {
        act( AT_MAGIC, "Good protects $N.", ch, NULL, victim, TO_ROOM );
        return rSPELL_FAILED;
    }

    if ( IS_NEUTRAL(victim) )
    {
        act( AT_MAGIC, "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
        return rSPELL_FAILED;
    }

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    if ( saves_spell_staff( level, victim ) )
    {
        act(AT_MAGIC,"$N resists the attack.",ch,NULL,victim,TO_CHAR);
        act(AT_MAGIC,"You resist $n's attack.",ch,NULL,victim,TO_VICT);
        return damage( ch, victim, 1, sn );
    }

    act(AT_MAGIC,"$n forces $N from this plane.",ch,NULL,victim,TO_ROOM);
    act(AT_MAGIC,"You force $N from this plane.",ch,NULL,victim,TO_CHAR);
    act(AT_MAGIC,"$n forces you from this plane.",ch,NULL,victim,TO_VICT);
    gain_exp(ch,UMIN(GET_EXP(victim)/2,50000));
    extract_char(victim, FALSE);
    return rCHAR_DIED;
}


ch_ret spell_dispel_good( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( !IS_NPC(ch) && IS_GOOD(ch) )
        victim = ch;

    if ( IS_EVIL(victim) )
    {
        act( AT_MAGIC, "Evil protects $N.", ch, NULL, victim, TO_ROOM );
        return rSPELL_FAILED;
    }

    if ( IS_NEUTRAL(victim) )
    {
        act( AT_MAGIC, "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
        return rSPELL_FAILED;
    }

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    if ( saves_spell_staff( level, victim ) )
    {
        act(AT_MAGIC,"$N resists the attack.",ch,NULL,victim,TO_CHAR);
        act(AT_MAGIC,"You resist $n's attack.",ch,NULL,victim,TO_VICT);
        return damage( ch, victim, 1, sn );
    }

    act(AT_MAGIC,"$n forces $N from this plane.",ch,NULL,victim,TO_ROOM);
    act(AT_MAGIC,"You force $N from this plane.",ch,NULL,victim,TO_CHAR);
    act(AT_MAGIC,"$n forces you from this plane.",ch,NULL,victim,TO_VICT);
    gain_exp(ch,UMIN(GET_EXP(victim)/2,50000));
    extract_char(victim, FALSE);
    return rCHAR_DIED;
}


ch_ret spell_dispel_magic( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    SKILLTYPE *skill = get_skilltype(sn);
    int hitp = 0, tmpgsn;

    if (target_name[0] == '\0')
    {
        send_to_char("Cast this where?\n\r", ch);
        return rSPELL_FAILED;
    }

    victim = get_char_room(ch, target_name);
    obj = get_obj_carry(ch, target_name);
    if (!victim && !obj)
    {
        send_to_char( "I can't find one of those anywhere!\n\r", ch );
        return rSPELL_FAILED;
    }

    if (obj)
    {
        if (IS_OBJ_STAT(obj, ITEM_INVIS))
        {
            act(AT_MAGIC, "$p fades into existance.",ch,obj,NULL,TO_CHAR);
            act(AT_MAGIC, "$p fades into existance.",ch,obj,NULL,TO_ROOM);
            REMOVE_OBJ_STAT(obj, ITEM_INVIS);
        }
        if (IS_OBJ_STAT(obj, ITEM_BLESS))
        {
            act(AT_MAGIC, "$p looks less blessed.",ch,obj,NULL,TO_CHAR);
            act(AT_MAGIC, "$p looks less blessed.",ch,obj,NULL,TO_ROOM);
            REMOVE_OBJ_STAT(obj, ITEM_BLESS);
        }
        if (level >= 45 && IS_OBJ_STAT(obj, ITEM_MAGIC))
        {
            AFFECT_DATA *paf, *paf_next;
            REMOVE_OBJ_STAT(obj, ITEM_MAGIC);
            act(AT_MAGIC, "The glow about $p fades.",ch,obj,NULL,TO_CHAR);
            act(AT_MAGIC, "The glow about $p fades.",ch,obj,NULL,TO_ROOM);
            for (paf = obj->first_affect; paf; paf = paf_next)
            {
                paf_next = paf->next;
                UNLINK(paf, obj->first_affect, obj->last_affect, next, prev);
                DISPOSE(paf);
                top_affect--;
            }
        }
        if (level >= 30 && IS_OBJ_STAT(obj, ITEM_NODROP))
        {
            act(AT_MAGIC, "$p looks less annoying.",ch,obj,NULL,TO_CHAR);
            act(AT_MAGIC, "$p looks less annoying.",ch,obj,NULL,TO_ROOM);
            REMOVE_OBJ_STAT(obj, ITEM_NODROP);
        }
        if (level >= 56)
        {
            act(AT_MAGIC, "$p looks less restrictive.",ch,obj,NULL,TO_CHAR);
            act(AT_MAGIC, "$p looks less restrictive.",ch,obj,NULL,TO_ROOM);
            REMOVE_OBJ_STAT(obj, ITEM_ANTI_GOOD);
            REMOVE_OBJ_STAT(obj, ITEM_ANTI_EVIL);
            REMOVE_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL);
        }
        if (level >= 58 && IS_OBJ_STAT(obj, ITEM_GLOW))
        {
            act(AT_MAGIC, "$p stops glowing.",ch,obj,NULL,TO_CHAR);
            act(AT_MAGIC, "$p stops glowing.",ch,obj,NULL,TO_ROOM);
            REMOVE_OBJ_STAT(obj, ITEM_GLOW);
        }
        if (level >= 60 && IS_OBJ_STAT(obj, ITEM_HUM))
        {
            act(AT_MAGIC, "$p becomes silent.",ch,obj,NULL,TO_CHAR);
            act(AT_MAGIC, "$p becomes silent.",ch,obj,NULL,TO_ROOM);
            REMOVE_OBJ_STAT(obj, ITEM_HUM);
        }
        send_to_char("Ok.\n\r", ch);
        return rNONE;
    }

    if (victim)
    {
        AFFECT_DATA *paf;
        bool yes = FALSE;

        if ( IS_SET( victim->immune, RIS_MAGIC ) )
        {
            immune_casting( skill, ch, victim, NULL );
            return rSPELL_FAILED;
        }

        if (GetMaxLevel(victim) <= GetMaxLevel(ch))
            yes = TRUE;

#define CHECK_DISPEL(sn,str,str2) \
    if ((paf=is_affected(victim,(sn)))) \
    if (yes || !saves_spell_staff(level,victim)) { \
    affect_remove(victim,paf); \
    send_to_char((str),victim); \
    act(AT_MAGIC,(str2),victim,NULL,NULL,TO_ROOM); \
    }
#define CHECK_DISPEL2(aff,str,str2) \
    if (IS_AFFECTED(victim,(aff))) \
    if (yes || !saves_spell_staff(level,victim)) { \
    REMOVE_AFFECTED(victim,(aff)); \
    send_to_char((str),victim); \
    act(AT_MAGIC,(str2),victim,NULL,NULL,TO_ROOM); \
    }
#define CHECK_DISPEL3(sn,str,str2) \
    if ((is_affected(victim,(sn)))) \
    if (yes || !saves_spell_staff(level,victim)) { \
    hitp = (GET_HIT(victim) * 100) / GET_MAX_HIT(victim); \
    while ((paf=is_affected(victim, (sn)))) \
           affect_remove(victim,paf); \
    if ((sn) == gsn_poly) \
       GET_HIT(victim) = (GET_MAX_HIT(victim) * hitp) / 100; \
    send_to_char((str),victim); \
    act(AT_MAGIC,(str2),victim,NULL,NULL,TO_ROOM); \
    }

        CHECK_DISPEL(gsn_detect_invis,
                     "You feel less perceptive.\n\r",
                     "");
        CHECK_DISPEL(gsn_invis,
                     "You feel exposed.\n\r",
                     "");
        CHECK_DISPEL(gsn_enlarge,
                     "You feel smaller.\n\r",
                     "");
        CHECK_DISPEL(gsn_detect_evil,
                     "You feel less morally alert.\n\r",
                     "");
        CHECK_DISPEL(gsn_detect_magic,
                     "You stop noticing the magic in your life.\n\r",
                     "");
        CHECK_DISPEL(gsn_sense_life,
                     "You feel less in touch with living things.\n\r",
                     "");
        if (victim != ch && is_affected(victim,gsn_sanctuary))
            one_hit(victim,ch,TYPE_UNDEFINED);
        CHECK_DISPEL(gsn_sanctuary,
                     "You don't feel so invulnerable anymore.\n\r",
                     "The white glow around $n's body fades.");
        if (victim != ch && IS_AFFECTED(victim,AFF_SANCTUARY))
            one_hit(victim,ch,TYPE_UNDEFINED);
        CHECK_DISPEL2(AFF_SANCTUARY,
                      "You don't feel so invulnerable anymore.\n\r",
                      "The white glow around $n's body fades.");
        CHECK_DISPEL(gsn_protection_from_evil,
                     "You feel less morally protected.\n\r",
                     "");
        CHECK_DISPEL(gsn_infravision,
                     "Your sight grows dimmer.\n\r",
                     "");
        CHECK_DISPEL(gsn_sleep,
                     "You don't feel so tired.\n\r",
                     "");
        CHECK_DISPEL(gsn_charm_person,
                     "You feel less enthused about your master.\n\r",
                     "");
        CHECK_DISPEL(gsn_weakness,
                     "You don't feel so weak.\n\r",
                     "");
        CHECK_DISPEL(gsn_strength,
                     "You don't feel so strong.\n\r",
                     "");
        CHECK_DISPEL(gsn_armor,
                     "You don't feel so well protected.\n\r",
                     "");
        CHECK_DISPEL(gsn_detect_poison,
                     "You don't feel so sensitive to fumes.\n\r",
                     "");
        CHECK_DISPEL(gsn_bless,
                     "You don't feel so blessed.\n\r",
                     "");
        CHECK_DISPEL(gsn_bless,
                     "",
                     "");
        CHECK_DISPEL(gsn_fly,
                     "You don't feel lighter than air anymore.\n\r",
                     "");
        CHECK_DISPEL(gsn_water_breath,
                     "You don't feel so fishy anymore.\n\r",
                     "");
        if (victim != ch && is_affected(victim,gsn_fireshield))
            one_hit(victim,ch,TYPE_UNDEFINED);
        CHECK_DISPEL(gsn_fireshield,
                     "You don't feel so fiery anymore.\n\r",
                     "The fiery aura around $n's body fades.");
        if ((tmpgsn = skill_lookup("flameshroud")) > 0)
        {
            if (victim != ch && is_affected(victim,tmpgsn))
                one_hit(victim,ch,TYPE_UNDEFINED);
            CHECK_DISPEL(tmpgsn,
                         "You don't feel so fiery anymore.\n\r",
                         "The fiery aura around $n's body fades.");
        }
        if (victim != ch && IS_AFFECTED(victim,AFF_FIRESHIELD))
            one_hit(victim,ch,TYPE_UNDEFINED);
        CHECK_DISPEL2(AFF_FIRESHIELD,
                      "You don't feel so fiery anymore.\n\r",
                      "The fiery aura around $n's body fades.");
        CHECK_DISPEL(gsn_faerie_fire,
                     "You don't feel so pink anymore.\n\r",
                     "The pink glow around $n's body fades.");
        CHECK_DISPEL(gsn_minor_track,
                     "You lose the trail.\n\r",
                     "");
        CHECK_DISPEL(gsn_major_track,
                     "You lose the trail.\n\r",
                     "");
        CHECK_DISPEL(gsn_web,
                     "You don't feel so sticky anymore.\n\r",
                     "");
        CHECK_DISPEL(gsn_silence,
                     "You don't feel so quiet anymore",
                     "");
        CHECK_DISPEL(gsn_tree_travel,
                     "You don't feel so in touch with trees anymore.\n\r",
                     "");
        CHECK_DISPEL(gsn_haste,
                     "You don't feel so fast anymore.\n\r",
                     "");
        CHECK_DISPEL(gsn_slow,
                     "You don't feel so slow anymore.\n\r",
                     "");
        CHECK_DISPEL(gsn_barkskin,
                     "You don't feel so barky anymore.\n\r",
                     "");
        CHECK_DISPEL(gsn_stone_skin,
                     "Your skin feels softer.\n\r",
                     "");
        CHECK_DISPEL(gsn_aid,
                     "You feel less aided.\n\r",
                     "");
        CHECK_DISPEL(gsn_shield,
                     "You feel less shielded.\n\r",
                     "");
        CHECK_DISPEL(gsn_true_sight,
                     "You feel less keen.\n\r",
                     "");
        CHECK_DISPEL(gsn_invis_to_animals,
                     "You feel exposed.\n\r",
                     "");
        CHECK_DISPEL(gsn_dragon_ride,
                     "You feel more afraid of dragons.\n\r",
                     "");
        CHECK_DISPEL3(gsn_poly,
                     "You revert to your origional form.\n\r",
                     "$n reverts back to $s normal form.");
        CHECK_DISPEL(gsn_darkness,
                     "Your globe of darkness vanishes.\n\r",
                     "");
        CHECK_DISPEL(gsn_minor_invulnerability,
                     "You see your globe of protection vanish.\n\r",
                     "");
        CHECK_DISPEL(gsn_major_invulnerability,
                     "You see your globe of protection vanish.\n\r",
                     "");
        if ((tmpgsn = skill_lookup("dome of protection")) > 0)
        {
            if (victim != ch && is_affected(victim,tmpgsn))
                one_hit(victim,ch,TYPE_UNDEFINED);
            CHECK_DISPEL(tmpgsn,
                         "Your dome of protection fades.\n\r",
                         "A glow about $n's body fades.");
        }
        CHECK_DISPEL(gsn_protection_from_energy_drain,
                     "You feel fearful of vampiric creatures.\n\r",
                     "");
        CHECK_DISPEL(gsn_wizardeye,
                     "Your wizardeye disappears with a *pop*.\n\r",
                     "");
        CHECK_DISPEL(gsn_protection_from_breath,
                     "You feel the urge to avoid dragons.\n\r",
                     "");
        CHECK_DISPEL(gsn_protection_from_fire_breath,
                     "You feel the urge to avoid fire dragons.\n\r",
                     "");
        CHECK_DISPEL(gsn_protection_from_frost_breath,
                     "You feel the urge to avoid frost dragons.\n\r",
                     "");
        CHECK_DISPEL(gsn_protection_from_electric_breath,
                     "You feel the urge to avoid electric dragons.\n\r",
                     "");
        CHECK_DISPEL(gsn_protection_from_acid_breath,
                     "You feel the urge to avoid acid dragons.\n\r",
                     "");
        CHECK_DISPEL(gsn_protection_from_gas_breath,
                     "You feel the urge to avoid gas dragons.\n\r",
                     "");
        if (IS_IMMORTAL(ch))
        {
            CHECK_DISPEL(gsn_anti_magic_shell,
                         "Your antimagic shell fizzles out.\n\r",
                         "");
            CHECK_DISPEL(gsn_blindness,
                         "Your vision returns.\n\r",
                         "");
            CHECK_DISPEL(gsn_paralyze,
                         "You feel freedom of movement.\n\r",
                         "");
            CHECK_DISPEL(gsn_poison,
                         "You feel better.\n\r",
                         "");
            CHECK_DISPEL(gsn_curse,
                         "You feel free.\n\r",
                         "");
            CHECK_DISPEL(gsn_curse,
                         "",
                         "");
        }

#undef CHECK_DISPEL
#undef CHECK_DISPEL2
#undef CHECK_DISPEL3

        send_to_char("Ok.\n\r", ch);
        return rNONE;
    }

    return rNONE;
}



ch_ret spell_earthquake( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    bool ch_died;
    ch_ret retcode;
    SKILLTYPE *skill = get_skilltype(sn);

    ch_died = FALSE;
    retcode = rNONE;

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
        failed_casting( skill, ch, NULL, NULL );
        return rSPELL_FAILED;
    }

    act( AT_MAGIC, "The earth trembles beneath your feet!", ch, NULL, NULL, TO_CHAR );
    act( AT_MAGIC, "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM );

    for ( vch = first_char; vch; vch = vch_next )
    {
        vch_next	= vch->next;
        if ( !vch->in_room )
            continue;

        if ( vch->in_room == ch->in_room )
        {
            if ( !IS_NPC( vch ) && vch->pcdata->wizinvis >= LEVEL_IMMORTAL )
                continue;

            if (vch->master == ch ||
                ch->master == vch ||
                is_same_group(ch, vch) ||
                IS_AFFECTED(vch, AFF_FLYING))
                continue;

            if ( vch != ch && ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) ) )
                retcode = damage( ch, vch, \
                                  GetMaxLevel(vch)<4?(GET_MAX_HIT(vch)*12):(level + dice(1, 4) + 1), \
                                  sn );
            if ( retcode == rCHAR_DIED || char_died(ch) )
            {
                ch_died = TRUE;
                continue;
            }
            if ( retcode == rVICT_DIED )
                continue;
        }

        if ( !ch_died && vch->in_room->area == ch->in_room->area )
        {
            set_char_color( AT_MAGIC, vch );
            send_to_char( "The earth trembles...\n\r", vch );
        }
    }

    if ( ch_died )
        return rCHAR_DIED;
    else
        return rNONE;
}


ch_ret spell_enchant_weapon( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;
    SKILLTYPE *skill = get_skilltype(sn);
    int mod;

    if ( obj->item_type != ITEM_WEAPON ||
         IS_OBJ_STAT(obj, ITEM_MAGIC) ||
         IS_OBJ_STAT(obj, ITEM_BLESS) ||
         IS_OBJ_STAT(obj, ITEM_GLOW) ||
         IS_OBJ_STAT(obj, ITEM_HUM) ||
         obj->first_affect )
        return rSPELL_FAILED;

    separate_obj(obj);

    mod = 1;
    if (level > 17 )
        mod += 1;
    if (level > 40 )
        mod += 1;
    if (level > 50 )
        mod += 1;
    if (level > 60 )
        mod += 1;
    if (level > 70 )
        mod += 1;

    if (SPELL_POWER(skill) == SP_MINOR)
        mod += 1;
    else if (SPELL_POWER(skill) == SP_GREATER)
        mod += 2;
    else if (IS_IMMORTAL(ch) || SPELL_POWER(skill) == SP_MAJOR)
        mod += 3;

    if (SPELL_CLASS(skill) == SC_DEATH)
        mod = -mod;

    CREATE( paf, AFFECT_DATA, 1 );
    paf->type		= -1;
    paf->duration	= -1;
    paf->location	= APPLY_HITROLL;
    paf->modifier	= mod;
    paf->bitvector	= 0;
    LINK( paf, obj->first_affect, obj->last_affect, next, prev );

    CREATE( paf, AFFECT_DATA, 1 );
    paf->type		= -1;
    paf->duration	= -1;
    paf->location	= APPLY_DAMROLL;
    paf->modifier	= mod;
    paf->bitvector	= 0;
    LINK( paf, obj->first_affect, obj->last_affect, next, prev );

    SET_BIT(obj->extra_flags, ITEM_MAGIC);
    if ( IS_GOOD(ch) )
    {
        SET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
        act( AT_BLUE, "$p glows blue.", ch, obj, NULL, TO_CHAR );
    }
    else if ( IS_EVIL(ch) )
    {
        SET_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
        act( AT_RED, "$p glows red.", ch, obj, NULL, TO_CHAR );
    }
    else
    {
        SET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
        SET_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
        act( AT_YELLOW, "$p glows yellow.", ch, obj, NULL, TO_CHAR );
    }

    send_to_char( "Ok.\n\r", ch );
    return rNONE;
}

ch_ret spell_enchant_armor( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA *paf;
    SKILLTYPE *skill = get_skilltype(sn);
    int mod;

    if ( obj->item_type != ITEM_ARMOR ||
         IS_OBJ_STAT(obj, ITEM_MAGIC) ||
         IS_OBJ_STAT(obj, ITEM_BLESS) ||
         IS_OBJ_STAT(obj, ITEM_GLOW) ||
         IS_OBJ_STAT(obj, ITEM_HUM) )
        return rSPELL_FAILED;

    separate_obj(obj);

    mod = -1;
    if (level > 19 )
        mod -= 1;
    if (level > 39 )
        mod -= 1;
    if (level > 49 )
        mod -= 1;
    if (level > 59 )
        mod -= 1;
    if (level > 70 )
        mod -= 1;

    if (SPELL_POWER(skill) == SP_MINOR)
        mod -= 1;
    else if (SPELL_POWER(skill) == SP_GREATER)
        mod -= 2;
    else if (IS_IMMORTAL(ch) || SPELL_POWER(skill) == SP_MAJOR)
        mod -= 3;

    if (SPELL_CLASS(skill) == SC_DEATH)
        mod = -mod;

    CREATE( paf, AFFECT_DATA, 1 );
    paf->type		= -1;
    paf->duration	= -1;
    paf->location	= APPLY_SAVING_ALL;
    paf->modifier	= mod;
    paf->bitvector	= 0;
    LINK( paf, obj->first_affect, obj->last_affect, next, prev );

    mod = -1;
    if (level > 30 )
        mod -= 1;
    if (level > 50 )
        mod -= 1;
    if (level > 59 )
        mod -= 1;
    if (level > 70 )
        mod -= 1;

    if (SPELL_POWER(skill) == SP_MINOR)
        mod -= 1;
    else if (SPELL_POWER(skill) == SP_GREATER)
        mod -= 2;
    else if (IS_IMMORTAL(ch) || SPELL_POWER(skill) == SP_MAJOR)
        mod -= 3;

    if (SPELL_CLASS(skill) == SC_DEATH)
        mod = -mod;

    CREATE( paf, AFFECT_DATA, 1 );
    paf->type		= -1;
    paf->duration	= -1;
    paf->location	= APPLY_AC;
    paf->modifier	= mod;
    paf->bitvector	= 0;
    LINK( paf, obj->first_affect, obj->last_affect, next, prev );

    SET_BIT(obj->extra_flags, ITEM_MAGIC);
    if ( IS_GOOD(ch) )
    {
        SET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
        act( AT_BLUE, "$p glows blue.", ch, obj, NULL, TO_CHAR );
    }
    else if ( IS_EVIL(ch) )
    {
        SET_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
        act( AT_RED, "$p glows red.", ch, obj, NULL, TO_CHAR );
    }
    else
    {
        SET_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
        SET_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
        act( AT_YELLOW, "$p glows yellow.", ch, obj, NULL, TO_CHAR );
    }

    return rNONE;
}



/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
ch_ret spell_energy_drain( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    int saves;
    SKILLTYPE *skill = get_skilltype(sn);

    if (!(IS_NPC(ch) && IS_SET(ch->act2, ACT2_MASTER_VAMPIRE)))
    {
        if ( IS_SET( victim->immune, RIS_DRAIN ) )
        {
            immune_casting( skill, ch, victim, NULL );
            return rSPELL_FAILED;
        }

        saves = ris_save( victim, GetMaxLevel(victim), RIS_DRAIN );
        if ( saves == 1000 || saves_spell_staff( saves, victim ) )
        {
            failed_casting( skill, ch, victim, NULL ); /* SB */
            return rSPELL_FAILED;
        }
    }

    ch->alignment = UMAX(-1000, ch->alignment - 200);
    if ( GetMaxLevel(victim) <= 1 )
        dam		 = GET_HIT(victim) * 12;
    else
    {
        gain_exp( victim, 0 - number_range( level / 2, 3 * level / 2 ) );
        dam			 = 1;
    }

    if ( GET_HIT(ch) > GET_MAX_HIT(ch) )
        GET_HIT(ch) = GET_MAX_HIT(ch);
    return damage( ch, victim, dam, sn );
}



ch_ret spell_fireball( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch, *vch_next;
    bool ch_died = FALSE;
    ch_ret retcode = rNONE;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
        failed_casting( skill, ch, NULL, NULL );
        return rSPELL_FAILED;
    }

    for ( vch = first_char; vch; vch = vch_next )
    {
        vch_next	= vch->next;
        if ( !vch->in_room )
            continue;
        if ( vch->in_room == ch->in_room )
        {
            if ( !IS_NPC( vch ) && vch->pcdata->wizinvis >= LEVEL_IMMORTAL )
                continue;

            if (vch->master == ch ||
                ch->master == vch ||
                is_same_group(ch, vch))
                continue;

            if ( vch != ch && ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) ) )
                retcode = damage( ch, vch, \
                                  !saves_spell_staff(level,vch)?dice(level,8):(dice(level,8)/2), \
                                  sn );
            if ( retcode == rCHAR_DIED || char_died(ch) )
            {
                ch_died = TRUE;
                continue;
            }
            if ( char_died(vch) )
                continue;
        }

        if ( !ch_died && vch->in_room->area == ch->in_room->area &&
             vch->in_room != ch->in_room )
        {
            set_char_color( AT_MAGIC, vch );
            send_to_char( "You feel a blast of hot air...\n\r", vch );
        }
    }

    if ( ch_died )
        return rCHAR_DIED;
    else
        return rNONE;
}


ch_ret spell_flamestrike( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    dam = dice(6, 8);
    if ( saves_spell_staff( level, victim ) )
        dam /= 2;
    return damage( ch, victim, dam, sn );
}


ch_ret spell_faerie_fire( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) )
    {
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }
    af.type      = sn;
    af.duration  = (int)(level * DUR_CONV);
    af.location  = APPLY_AC;
    af.modifier  = 20;
    af.bitvector = AFF_FAERIE_FIRE;
    affect_to_char( victim, &af );
    act( AT_PINK, "$n points at $N.", ch, NULL, victim, TO_ROOM);
    act( AT_PINK, "You point at $N.", ch, NULL, victim, TO_CHAR);
    act( AT_PINK, "You are surrounded by a pink outline.", victim, NULL, NULL, TO_CHAR );
    act( AT_PINK, "$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM );
    return rNONE;
}



ch_ret spell_faerie_fog( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *ich;

    act( AT_MAGIC, "$n snaps $s fingers, and a cloud of purple smoke billows forth.", ch, NULL, NULL, TO_ROOM );
    act( AT_MAGIC, "You snap your fingers, and a cloud of purple smoke billows forth.", ch, NULL, NULL, TO_CHAR );

    for ( ich = ch->in_room->first_person; ich; ich = ich->next_in_room )
    {
        if ( !IS_NPC(ich) && ich->pcdata->wizinvis )
            continue;

        if (ich->master == ch ||
            ch->master == ich ||
            is_same_group(ch, ich))
            continue;

        if ( ich == ch || saves_spell_staff( level, ich ) )
            continue;

        affect_strip ( ich, gsn_invis			);
        affect_strip ( ich, gsn_group_invis		);
        affect_strip ( ich, gsn_sneak			);
        REMOVE_BIT   ( ich->affected_by, AFF_HIDE	);
        REMOVE_BIT   ( ich->affected_by, AFF_INVISIBLE	);
        REMOVE_BIT   ( ich->affected_by, AFF_SNEAK	);
        act( AT_MAGIC, "$n is revealed!", ich, NULL, NULL, TO_ROOM );
        act( AT_MAGIC, "You are revealed!", ich, NULL, NULL, TO_CHAR );
    }
    return rNONE;
}


ch_ret spell_gate( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *targetRoom, *fromRoom;
    int targetRoomVnum;
    OBJ_DATA *portalObj, *portalObj2;
    EXIT_DATA *pexit, *pexit2;
    char buf[MAX_STRING_LENGTH];

    if ( (victim = get_char_world(ch, target_name)) == NULL )
    {
        send_to_char("The magic cannot find the target.\n\r", ch);
        return rNONE;
    }

    if ( victim == ch )
    {
        send_to_char("You can't create a portal to yourself.\n\r", ch);
        return rNONE;
    }

    if (victim->in_room == ch->in_room)
    {
        send_to_char("They are right beside you!\n\r", ch);
        return rNONE;
    }

    if (IS_SYSTEMFLAG(SYS_NOPORTAL))
    {
        send_to_char("Everything appears to have gone right, but no gateway appears.\n\r", ch);
        return rNONE;
    }

    /* Gate is supposed to allow extra-planar travel */
    /*
    if ( !IsExtraPlanar(ch) &&
         is_other_plane(ch->in_room, victim->in_room))
    {
        send_to_char("The planes obscure your magic.\n\r", ch);
        if (IS_IMMORTAL(ch))
            send_to_char("However, your immortality overcomes the obscurity.\n\r", ch);
        else
            return rNONE;
    }
    */

    if ( (IS_SET(ch->in_room->room_flags, ROOM_NO_ASTRAL       ) ||
          IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL       ) ||
          IS_SET(ch->in_room->room_flags, ROOM_PROTOTYPE       ) ||
          IS_SET(victim->in_room->room_flags, ROOM_NO_ASTRAL   ) ||
          IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL   ) ||
          IS_SET(victim->in_room->room_flags, ROOM_PROTOTYPE   ) ||
          IS_SET(victim->in_room->room_flags, ROOM_DEATH  ) ||
          (IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE)) ||
          in_arena(victim) || in_arena(ch) ||
          !victim->in_room                                        ))
    {
        send_to_char("Eldritch wizardry obstucts thee.\n\r",ch);
        if (IS_IMMORTAL(ch))
            send_to_char("However, your immortality overcomes it.\n\r", ch);
        else
            return rNONE;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_SUMMON)    ||
         IS_SET(victim->in_room->room_flags, ROOM_NO_SUMMON)  )
    {
        send_to_char("Ancient Magiks bar your path.\n\r",ch);
        if (IS_IMMORTAL(ch))
            send_to_char("However, your immortality overcomes them.\n\r", ch);
        else
            return rNONE;
    }

    if (!IS_NPC(victim) && IS_IMMORTAL(victim) && !IS_IMMORTAL(ch))
    {
	send_to_char("They wouldn't like that much.\n\r", ch);
        if (can_see(victim, ch))
	    ch_printf(victim, "%s just tried gating to you.\n\r", GET_NAME(ch));
        return rNONE;
    }

    targetRoomVnum = victim->in_room->vnum;
    fromRoom = ch->in_room;
    targetRoom = victim->in_room;

    for ( pexit = fromRoom->first_exit; pexit; pexit = pexit->next )
    {
        if ( pexit->vdir == DIR_PORTAL )
        {
            send_to_char("Magic collides with magic and your spell fizzles.\n\r",ch);
            act( AT_MAGIC, "The portal crackles loudly, throwing off bright blue sparks.", ch, NULL, NULL, TO_ROOM );
            act( AT_MAGIC, "The portal crackles loudly, throwing off bright blue sparks.", ch, NULL, NULL, TO_CHAR );
            return rNONE;
        }
    }

    for ( pexit2 = targetRoom->first_exit; pexit2; pexit2 = pexit2->next )
    {
        if ( pexit2->vdir == DIR_PORTAL )
        {
            send_to_char("Magic collides with magic and your spell fizzles.\n\r",ch);
            act( AT_MAGIC, "The portal crackles loudly, throwing off bright blue sparks.", ch, NULL, NULL, TO_ROOM );
            act( AT_MAGIC, "The portal crackles loudly, throwing off bright blue sparks.", ch, NULL, NULL, TO_CHAR );
            return rNONE;
        }
    }

    pexit = make_exit( fromRoom, targetRoom, DIR_PORTAL );
    pexit->keyword      = STRALLOC( "portal" );
    pexit->description  = STRALLOC( "Through the mists of the portal, you can faintly see...\n\r" );
    pexit->key          = 0;
    pexit->exit_info    = EX_PORTAL | EX_xENTER | EX_HIDDEN | EX_xLOOK;
    pexit->vnum         = targetRoomVnum;

    portalObj = create_object( OBJ_VNUM_PORTAL );
    if (!portalObj) return rNONE;
    portalObj->item_type = ITEM_PORTAL;
    portalObj->timer = 3;
    sprintf( buf, "A portal made by %s", ch->name );
    STRFREE( portalObj->short_descr );
    portalObj->short_descr = STRALLOC( buf );
    portalObj = obj_to_room( portalObj, ch->in_room );

    act( AT_MAGIC, "A misty portal suddenly appears.", ch, NULL, NULL, TO_ROOM );
    act( AT_MAGIC, "A misty portal suddenly appears.", ch, NULL, NULL, TO_CHAR );

    pexit2 = make_exit( targetRoom, fromRoom, DIR_PORTAL );
    pexit2->keyword      = STRALLOC( "portal" );
    pexit2->description  = STRALLOC( "Through the mists of the portal, you can faintly see...\n\r" );
    pexit2->key          = 0;
    pexit2->exit_info    = EX_PORTAL | EX_xENTER | EX_HIDDEN | EX_xLOOK;
    pexit2->vnum         = ch->in_room->vnum;

    portalObj2 = create_object( OBJ_VNUM_PORTAL );
    if (!portalObj2) return rNONE;
    portalObj2->item_type = ITEM_PORTAL;
    portalObj2->timer = 3;
    sprintf( buf, "A portal made by %s", ch->name );
    STRFREE( portalObj2->short_descr );
    portalObj2->short_descr = STRALLOC( buf );
    portalObj2 = obj_to_room( portalObj2, targetRoom );

    act( AT_MAGIC, "A misty portal suddenly appears.", victim, NULL, NULL, TO_ROOM );
    act( AT_MAGIC, "A misty portal suddenly appears.", victim, NULL, NULL, TO_CHAR );

    return rNONE;
}


ch_ret spell_harm( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    dam = UMIN( 100, GET_HIT(victim) - dice(1,4) );
    if ( GET_RACE(victim) == RACE_GOD )
        dam = 0;
    else if ( dam < 0 )
        dam = 100;

    return damage( ch, victim, dam, sn );
}

ch_ret spell_know_monster( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    AFFECT_DATA *paf;
    SKILLTYPE *sktmp;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( ( victim = get_char_room( ch, target_name ) ) != NULL )
    {

        if ( IS_SET( victim->immune, RIS_MAGIC ) )
        {
            immune_casting( skill, ch, victim, NULL );
            return rSPELL_FAILED;
        }

        if ( IS_NPC(victim) )
        {
            ch_printf(ch, "%s appears to be between level %d and %d.\n\r",
                      victim->name,
                      GetAveLevel(victim) - (GetAveLevel(victim) % 5),
                      GetAveLevel(victim) - (GetAveLevel(victim) % 5) + 5);
        }
        else
        {
            ch_printf(ch, "%s appears to be level %d.\n\r", victim->name, GetAveLevel(victim) );
        }

        ch_printf(ch, "%s looks like %s, and follows the ways of the %s.\n\r",
                  victim->name, aoran(get_race_name(victim)), get_class_name(victim) );

        if ( (chance(ch, 50) && GetAveLevel(ch) >= GetMaxLevel(victim) + 10 )
             ||    IS_IMMORTAL(ch) )
        {
            ch_printf(ch, "%s appears to be affected by: ", victim->name);

            if (!victim->first_affect)
            {
                send_to_char( "nothing.\n\r", ch );
                return rNONE;
            }

            for ( paf = victim->first_affect; paf; paf = paf->next )
            {
                if (victim->first_affect != victim->last_affect)
                {
                    if( paf != victim->last_affect && (sktmp=get_skilltype(paf->type)) != NULL )
                        ch_printf( ch, "%s, ", sktmp->name );

                    if( paf == victim->last_affect && (sktmp=get_skilltype(paf->type)) != NULL )
                    {
                        ch_printf( ch, "and %s.\n\r", sktmp->name );
                        return rNONE;
                    }
                }
                else
                {
                    if ( (sktmp=get_skilltype(paf->type)) != NULL )
                        ch_printf( ch, "%s.\n\r", sktmp->name );
                    else
                        send_to_char( "\n\r", ch );
                    return rNONE;
                }
            }
        }
    }

    else
    {
        ch_printf(ch, "You can't find %s!\n\r", target_name );
        return rSPELL_FAILED;
    }
    return rNONE;

}


ch_ret spell_identify( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj;
    CHAR_DATA *mob;
    AFFECT_DATA *paf;
    SKILLTYPE *sktmp;

    if ( target_name[0] == '\0' )
    {
        send_to_char( "What should the spell be cast upon?\n\r", ch );
        return rSPELL_FAILED;
    }

    mob = get_char_room(ch, target_name);
    obj = get_obj_carry(ch, target_name);
    if (!mob && !obj)
    {
        send_to_char( "I can't find one of those anywhere!\n\r", ch );
        return rSPELL_FAILED;
    }

    set_char_color( AT_MAGIC, ch );
    if (obj)
    {
        ch_printf(ch, "You feel informed:\n\rObject '%s', Item type: %s\n\r",
                  obj->name, item_type_name(obj));
        /* affected bits? */

        if (obj->extra_flags || obj->extra_flags2 || obj->magic_flags)
        {
            ch_printf(ch, "Item is: %s ",
                      obj->extra_flags?flag_string(obj->extra_flags, o_flags):" ");
	    ch_printf(ch, "%s ",
                      obj->extra_flags2?flag_string(obj->extra_flags2, o2_flags):" ");
	    ch_printf(ch, "%s\n\r",
                      obj->magic_flags?flag_string(obj->magic_flags, mag_flags):" ");
        }
        ch_printf(ch, "Weight: %d, Value: %d, Rent cost: %d  %s\n\r",
                  obj->weight, obj->cost, obj->rent,
                  is_rare_obj(obj)?"[RARE]":" ");
        switch (obj->item_type)
        {
        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
            ch_printf(ch, "Level %d spells of:\n\r", obj->value[0]);
            if (obj->value[1] >=0 && (sktmp=get_skilltype(obj->value[1])) != NULL)
                ch_printf(ch, "%s\n\r", sktmp->name);
            if (obj->value[2] >=0 && (sktmp=get_skilltype(obj->value[2])) != NULL)
                ch_printf(ch, "%s\n\r", sktmp->name);
            if (obj->value[3] >=0 && (sktmp=get_skilltype(obj->value[3])) != NULL)
                ch_printf(ch, "%s\n\r", sktmp->name);
            break;
        case ITEM_WAND:
        case ITEM_STAFF:
            ch_printf(ch, "Has %d charges, with %d charges left.\n\r",
                      obj->value[1], obj->value[2]);
            if (obj->value[3] >= 0 && (sktmp=get_skilltype(obj->value[3])) != NULL)
                ch_printf(ch, "Level %d spell of:\n\r%s\n\r",
                          obj->value[0], sktmp->name);
            break;
        case ITEM_MATERIAL:
            ch_printf(ch, "Is %s grade %s.\n\r", mat_qname[MGRADE(obj)],
                                                 mat_name[MTYPE(obj)]);
            break;
        case ITEM_WEAPON:
            ch_printf(ch, "Damage Dice is '%dD%d'\n\r",
                      obj->value[1], obj->value[2]);
            break;
        case ITEM_ARMOR:
            ch_printf(ch, "AC-apply is %d\n\r",
                      obj->value[0]);
            break;
        }
        /*
        for (paf=obj->pIndexData->first_affect; paf; paf=paf->next)
            showaffect(ch, paf);
        */
        for (paf=obj->first_affect; paf; paf=paf->next)
            showaffect(ch, paf);

        return rNONE;
    }

    if (mob)
    {
        if (!IS_NPC(mob))
        {
            if (IS_IMMORTAL(mob) && !IS_IMMORTAL(ch))
            {
                return rSPELL_FAILED;
            }
            ch_printf(ch, "%d Years,  %d Months,  %d Days,  %d Hours old.\n\r",
                      get_age(mob), get_age_month(mob), get_age_day(mob), get_age_hour(mob));
            ch_printf(ch, "Height %din  Weight %dpounds\n\r",
                      mob->height, mob->weight);
            ch_printf(ch, "Armor Class: %d\n\rAlignment: %d\n\rSpellfail: %d\n\r",
                      GET_AC(mob), mob->alignment, mob->spellfail);
            if (level>10)
                ch_printf(ch, "Attacks: %1.3f\n\r",
                          (float)mob->numattacks/1000);
            if (level>30)
                ch_printf(ch, "Str %d, Int %d, Wis %d, Dex %d, Con %d, Ch %d, Lck %d\n\r",
                          get_curr_str(mob), get_curr_int(mob), get_curr_wis(mob),
                          get_curr_dex(mob), get_curr_con(mob), get_curr_cha(mob),
                          get_curr_lck(mob));
        }
        else
        {
            send_to_char("You learn nothing new.\n\r", ch);
        }
        return rNONE;
    }

    if (!IS_IMMORTAL(ch))
    {
        send_to_char("You are overcome by a wave of exhaustion.\n\r", ch);
        act(AT_MAGIC,"$n slumps to the ground, exhausted.",ch,NULL,NULL,TO_ROOM);
        WAIT_STATE( ch, PULSE_VIOLENCE *3 );
    }

    return rSPELL_FAILED;
}



ch_ret spell_invis( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    SKILLTYPE *skill = get_skilltype(sn);

    /* Modifications on 1/2/96 to work on player/object - Scryn */

    if (target_name[0] == '\0')
        victim = ch;
    else
        victim = get_char_room(ch, target_name);

    if( victim )
    {
        AFFECT_DATA af;

        if ( IS_SET( victim->immune, RIS_MAGIC ) )
        {
            immune_casting( skill, ch, victim, NULL );
            return rSPELL_FAILED;
        }

        if ( IS_AFFECTED(victim, AFF_INVISIBLE) )
        {
            failed_casting( skill, ch, victim, NULL );
            return rSPELL_FAILED;
        }

        act( AT_MAGIC, "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );
        af.type      = sn;
        af.duration  = (int)(((level / 4) + 12) * DUR_CONV);
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = AFF_INVISIBLE;
        affect_to_char( victim, &af );
        act( AT_MAGIC, "You fade out of existence.", victim, NULL, NULL, TO_CHAR );
        return rNONE;
    }
    else
    {
        OBJ_DATA *obj;

        obj = get_obj_carry( ch, target_name );

        if (obj)
        {
            if ( IS_OBJ_STAT(obj, ITEM_INVIS)
                 ||   chance(ch, 40 + level / 10))
            {
                failed_casting( skill, ch, NULL, NULL );
                return rSPELL_FAILED;
            }

            SET_BIT( obj->extra_flags, ITEM_INVIS );
            act( AT_MAGIC, "$p fades out of existence.", ch, obj, NULL, TO_CHAR );
            return rNONE;
        }
    }
    ch_printf(ch, "You can't find %s!\n\r", target_name);
    return rSPELL_FAILED;
}



ch_ret spell_know_alignment( int sn, int level, CHAR_DATA *ch, void *vo )
{
    char *msg;
    int ap;
    SKILLTYPE *skill = get_skilltype(sn);
    CHAR_DATA *victim;

    victim = get_char_room(ch, target_name);

    if ( !victim )
    {
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    ap = victim->alignment;

    if ( ap >  700 ) msg = "$N has an aura as white as the driven snow.";
    else if ( ap >  350 ) msg = "$N is of excellent moral character.";
    else if ( ap >  100 ) msg = "$N is often kind and thoughtful.";
    else if ( ap > -100 ) msg = "$N doesn't have a firm moral commitment.";
    else if ( ap > -350 ) msg = "$N lies to $S friends.";
    else if ( ap > -700 ) msg = "$N is of questionable moral character.";
    else msg = "$N has an aura as red as freshly spilled blood.";

    act( AT_MAGIC, msg, ch, NULL, victim, TO_CHAR );
    return rNONE;
}


ch_ret spell_lightning_bolt( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] =
    {
        0,
        0,  0,  0,  0,  0,	 0,  0,  0, 25, 28,
        31, 34, 37, 40, 40,	41, 42, 42, 43, 44,
        44, 45, 46, 46, 47,	48, 48, 49, 50, 50,
        51, 52, 52, 53, 54,	54, 55, 56, 56, 57,
        58, 58, 59, 60, 60, 61, 62, 62, 63, 64,
        64, 65, 65, 66, 66, 67, 68, 68, 69, 69,
        70, 71, 71, 72, 72, 73, 73, 74, 75, 75
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell_staff( level, victim ) )
        dam /= 2;
    return damage( ch, victim, dam, sn );
}



ch_ret spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo )
{
    char buf[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int cnt;

    found = FALSE;
    for ( obj = first_object; obj; obj = obj->next )
    {
        if ( !can_see_obj( ch, obj ) || !nifty_is_name( target_name, obj->name ) )
            continue;
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) && !IS_IMMORTAL(ch) )
            continue;

        found = TRUE;

        for ( cnt = 0, in_obj = obj;
              in_obj->in_obj && cnt < 100;
              in_obj = in_obj->in_obj, ++cnt )
            ;
        if ( cnt >= MAX_NEST )
        {
            sprintf( buf, "spell_locate_obj: object [%d] %s is nested more than %d times!",
                     obj->vnum, obj->short_descr, MAX_NEST );
            bug( buf, 0 );
            continue;
        }

        if ( in_obj->carried_by )
        {
            if ( IS_IMMORTAL( in_obj->carried_by )
                 && !IS_NPC( in_obj->carried_by )
                 && ( get_trust( ch ) < in_obj->carried_by->pcdata->wizinvis )
                 && in_obj->carried_by->pcdata->wizinvis )
                continue;

            sprintf( buf, "%s carried by %s.\n\r",
                     obj_short(obj), PERS(in_obj->carried_by, ch) );
        }
        else
        {
            sprintf( buf, "%s in %s.\n\r",
                     obj_short(obj), in_obj->in_room == NULL
                     ? "somewhere" : in_obj->in_room->name );
        }

        buf[0] = UPPER(buf[0]);
        set_pager_color( AT_MAGIC, ch );
        send_to_pager( buf, ch );
    }

    if ( !found )
    {
        send_to_char( "Nothing like that exists.\n\r", ch );
        return rSPELL_FAILED;
    }
    return rNONE;
}



ch_ret spell_pass_door( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    if ( IS_AFFECTED(victim, AFF_PASS_DOOR) )
    {
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }
    af.type      = sn;
    af.duration  = (int)(number_fuzzy( level / 4 ) * DUR_CONV);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );
    act( AT_MAGIC, "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
    act( AT_MAGIC, "You turn translucent.", victim, NULL, NULL, TO_CHAR );
    return rNONE;
}



ch_ret spell_poison( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    int saves;
    bool first = TRUE;

    saves = ris_save( victim, level, RIS_POISON );
    if ( saves == 1000 || saves_poison_death( saves, victim ) )
        return rSPELL_FAILED;
    if ( IS_AFFECTED( victim, AFF_POISON ) )
        first = FALSE;
    af.type      = sn;
    af.duration  = (int)(level * DUR_CONV);
    af.location  = APPLY_STR;
    af.modifier  = -2;
    af.bitvector = AFF_POISON;
    affect_join( victim, &af );
    set_char_color( AT_MAGIC, victim );
    send_to_char( "You feel very sick.\n\r", victim );
    victim->mental_state = URANGE( 20, victim->mental_state
                                   + (first ? 5 : 0), 100 );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return rNONE;
}


ch_ret spell_remove_trap( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj;
    OBJ_DATA *trap;
    bool found;
    int retcode;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( !target_name || target_name[0] == '\0' )
    {
        send_to_char( "Remove trap on what?\n\r", ch );
        return rSPELL_FAILED;
    }

    found = FALSE;

    if ( !ch->in_room->first_content )
    {
        send_to_char( "You can't find that here.\n\r", ch );
        return rNONE;
    }

    for ( obj = ch->in_room->first_content; obj; obj = obj->next_content )
        if ( can_see_obj( ch, obj ) && nifty_is_name( target_name, obj->name ) )
        {
            found = TRUE;
            break;
        }

    if ( !found )
    {
        send_to_char( "You can't find that here.\n\r", ch );
        return rSPELL_FAILED;
    }

    if ( (trap = get_trap( obj )) == NULL )
    {
        failed_casting( skill, ch, NULL, NULL );
        return rSPELL_FAILED;
    }


    if ( chance(ch, 70 + get_curr_wis(ch)) )
    {
        send_to_char( "Ooops!\n\r", ch );
        retcode = spring_trap(ch, trap);
        if ( retcode == rNONE )
            retcode = rSPELL_FAILED;
        return retcode;
    }

    extract_obj( trap );

    successful_casting( skill, ch, NULL, NULL );
    return rNONE;
}


ch_ret spell_shocking_grasp( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const int dam_each[] =
    {
        0,
        0,  0,  0,  0,  0,	 0, 20, 25, 29, 33,
        36, 39, 39, 39, 40,	40, 41, 41, 42, 42,
        43, 43, 44, 44, 45,	45, 46, 46, 47, 47,
        48, 48, 49, 49, 50,	50, 51, 51, 52, 52,
        53, 53, 54, 54, 55, 55, 56, 56, 57, 57,
        58, 58, 59, 59, 60, 60, 61, 61, 62, 62,
        63, 63, 64, 64, 65, 65, 66, 66, 67, 67
    };
    int dam;

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell_staff( level, victim ) )
        dam /= 2;
    return damage( ch, victim, dam, sn );
}



ch_ret spell_sleep( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af;
    int retcode;
    int saves;
    int tmp;
    CHAR_DATA *victim;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( ( victim = get_char_room( ch, target_name ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return rSPELL_FAILED;
    }

    if ( !IS_NPC(victim) && victim->fighting )
    {
        send_to_char( "You cannot sleep a fighting player.\n\r", ch );
        return rSPELL_FAILED;
    }

    if ( is_safe(ch, victim) )
        return rSPELL_FAILED;

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    if ( SPELL_FLAG(skill, SF_PKSENSITIVE)
         &&  !IS_NPC(ch) && !IS_NPC(victim) )
        tmp = level/2;
    else
        tmp = level;

    if ( IS_AFFECTED(victim, AFF_SLEEP)
         ||	(saves=ris_save(victim, tmp, RIS_SLEEP)) == 1000
         ||   level < GetMaxLevel(victim)
         ||  (victim != ch && IS_SET(victim->in_room->room_flags, ROOM_SAFE))
         ||   saves_spell_staff( saves, victim ) )
    {
        failed_casting( skill, ch, victim, NULL );
        if ( ch == victim )
            return rSPELL_FAILED;
        if ( !victim->fighting )
        {
            retcode = multi_hit( victim, ch, TYPE_UNDEFINED );
            if ( retcode == rNONE )
                retcode = rSPELL_FAILED;
            return retcode;
        }
    }
    af.type      = sn;
    af.duration  = (int)((4 + level) * DUR_CONV);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );

    /* Added by Narn at the request of Dominus. */
    if ( !IS_NPC( victim ) )
    {
        sprintf( log_buf, "%s has cast sleep on %s.", ch->name, victim->name );
        log_string_plus( log_buf, LOG_MONITOR, GetMaxLevel(ch), SEV_NOTICE );
        log_string_plus( log_buf, LOG_MAGIC, UMAX(LEVEL_LOG_CSET, GetMaxLevel(ch) ), SEV_INFO );
    }

    if ( IS_AWAKE(victim) )
    {
        act( AT_MAGIC, "You feel very sleepy ..... zzzzzz.", victim, NULL, NULL, TO_CHAR );
        act( AT_MAGIC, "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
        victim->position = POS_SLEEPING;
    }
    if ( IS_NPC( victim ) )
        start_hating( victim, ch );

    return rNONE;
}



ch_ret spell_summon( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    SKILLTYPE *skill = get_skilltype(sn);

    if (IS_SYSTEMFLAG(SYS_NOSUMMON))
    {
        send_to_char("Everything appears to have gone right, but nothing happens.\n\r", ch);
        return rNONE;
    }

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
         ||   victim == ch
         ||   !victim->in_room
         ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
         ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
         ||   IS_SET(victim->in_room->room_flags, ROOM_NO_SUMMON)
         ||   IS_SET(ch->in_room->room_flags, ROOM_NO_SUMMON)
         ||   victim->fighting
         ||  (IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE))
         ||  (IS_NPC(victim) && saves_spell_staff( level, victim ))
         ||   !in_hard_range( victim, ch->in_room->area )
         ||  ( !IS_NPC( ch ) && !CAN_PKILL( ch ) && IS_PKILL( victim ) )
         ||  (IS_SET(ch->in_room->area->flags, AFLAG_NOPKILL) && IS_PKILL(victim))
         ||  ( !IS_NPC(ch) && !IS_NPC(victim) && IS_SET(victim->pcdata->flags, PCFLAG_NOSUMMON) ) )
    {
        failed_casting( skill, ch, victim, NULL );
        if (victim)
        {
            set_char_color( AT_MAGIC, victim );
            send_to_char( "You feel strange, but it passes.\n\r", victim );
        }
        return rSPELL_FAILED;
    }

    if ( !IsExtraPlanar(ch) &&
         is_other_plane(ch->in_room, victim->in_room))
    {
        send_to_char("The planes obscure your magic.\n\r", ch);
        if (IS_IMMORTAL(ch))
            send_to_char("However, your immortality overcomes it.\n\r", ch);
        else
            return rNONE;
    }

    if ( ch->in_room->area != victim->in_room->area )
    {
        if ( ( (IS_NPC(ch) != IS_NPC(victim)) && chance(ch, 30) )
             ||   ( (IS_NPC(ch) == IS_NPC(victim)) && chance(ch, 60) ) )
        {
            failed_casting( skill, ch, victim, NULL );
            set_char_color( AT_MAGIC, victim );
            send_to_char( "You feel a strange pulling sensation...\n\r", victim );
            return rSPELL_FAILED;
        }
    }

    if ( !IS_NPC( victim ) )
    {
        act( AT_MAGIC, "You feel a wave of nausea overcome you...", ch, NULL,
             NULL, TO_CHAR );
        act( AT_MAGIC, "$n collapses, stunned!", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_STUNNED;

        sprintf( log_buf, "%s summoned %s to room %d.", ch->name,
                 victim->name,
                 ch->in_room->vnum );
        log_string_plus( log_buf, LOG_MONITOR, GetMaxLevel(ch), SEV_NOTICE );
    }

    act( AT_MAGIC, "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, ch->in_room );
    act( AT_MAGIC, "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
    act( AT_MAGIC, "$N has summoned you!", victim, NULL, ch,   TO_CHAR );
    do_look( victim, "auto" );
    return rNONE;
}

ch_ret spell_astral_walk( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *gch, *gch_next;
    ROOM_INDEX_DATA *astral;

    astral = get_room_index( ROOM_VNUM_ASTRAL_ENTRANCE );

    if ( ch->in_room == astral )
	return rNONE;

    if ( IS_SET (ch->in_room->room_flags, ROOM_NO_ASTRAL) ||
         IS_SET (ch->in_room->area->flags, AFLAG_ARENA)   ||
         IS_SET (ch->in_room->area->flags, AFLAG_NOPKILL) ||
         IS_SET (ch->in_room->room_flags, ROOM_SAFE) )
    {
        send_to_char("You can't access the astral plane from here.\n", ch);
        return rNONE;
    }

    if (!astral || IS_SYSTEMFLAG(SYS_NOPORTAL))
    {
        send_to_char("Everything appears to have gone right, but you are not on the astral plane.\n\r", ch);
        return rNONE;
    }

    for ( gch = ch->in_room->first_person; gch; gch = gch = gch_next )
    {
        gch_next = gch->next_in_room;

        if ( char_died(gch) ||
             (IS_NPC(gch) && IS_SET (gch->act, ACT_PROTOTYPE)) ||
             !is_same_group(ch, gch) ||
             gch->position == POS_FIGHTING )
            continue;

        act( AT_MAGIC, "$n wavers, fades and dissappears.", gch, NULL, NULL, TO_ROOM );
        char_from_room(gch);
        char_to_room(gch, astral);
        act( AT_MAGIC, "$n wavers into existence.", gch, NULL, NULL, TO_ROOM );
        do_look( gch, "auto" );
    }

    return rNONE;
}

ch_ret spell_teleport( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    ROOM_INDEX_DATA *pRoomIndex;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( !victim
         || !victim->in_room
         ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
         || ( !IS_NPC(ch) && victim->fighting )
         || ( victim != ch
              && ( saves_spell_staff( level, victim ) || saves_wands( level, victim ) ) ) )
    {
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    for ( ; ; )
    {
        pRoomIndex = get_room_index( number_range( 0, top_room_vnum ) );
        if ( pRoomIndex )
            if ( !IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE) &&
                 !IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) &&
                 !IS_SET(pRoomIndex->room_flags, ROOM_PROTOTYPE) &&
                 !IS_SET(pRoomIndex->room_flags, ROOM_NO_RECALL) &&
                 in_hard_range( ch, pRoomIndex->area ) )
                break;
    }

    successful_casting( skill, ch, victim, NULL );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    act( AT_MAGIC, "$n slowly fades into view.", victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );
    return rNONE;
}



ch_ret spell_ventriloquate( int sn, int level, CHAR_DATA *ch, void *vo )
{
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char speaker[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;

    target_name = one_argument( target_name, speaker );

    sprintf( buf1, "%s says '%s'.\n\r",              speaker, target_name );
    sprintf( buf2, "Someone makes %s say '%s'.\n\r", speaker, target_name );
    buf1[0] = UPPER(buf1[0]);

    for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
    {
        if ( !is_name( speaker, vch->name ) ) {
            set_char_color( AT_SAY, vch );
            send_to_char( saves_spell_staff( level, vch ) ? buf2 : buf1, vch );
        }
    }

    return rNONE;
}



ch_ret spell_weaken( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }
    if ( is_affected( victim, sn ) || saves_wands( level, victim ) )
        return rSPELL_FAILED;
    af.type      = sn;
    af.duration  = (int)(level / 2 * DUR_CONV);
    af.location  = APPLY_STR;
    af.modifier  = -2;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    set_char_color( AT_MAGIC, victim );
    send_to_char( "You feel weaker.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return rNONE;
}



/*
 * A spell as it should be				-Thoric
 */
ch_ret spell_word_of_recall( int sn, int level, CHAR_DATA *ch, void *vo )
{
    do_recall( (CHAR_DATA *) vo, "" );
    return rNONE;
}


/*
 * NPC spells.
 */
ch_ret spell_acid_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose;
    OBJ_DATA *obj_next;
    int dam;

    successful_casting( get_skilltype(sn), ch, victim, NULL );

    if ( chance(ch, 2 * level) &&
         !saves_breath( level, victim ) &&
         !is_affected(victim, gsn_protection_from_breath) &&
         !is_affected(victim, gsn_protection_from_acid_breath) )
    {
        for ( obj_lose = victim->first_carrying; obj_lose; obj_lose = obj_next )
        {
            int iWear;

            obj_next = obj_lose->next_content;

            if (ItemSave(obj_lose, sn))
                continue;

            switch ( obj_lose->item_type )
            {
            case ITEM_ARMOR:
                if ( obj_lose->value[0] > 0 )
                {
                    separate_obj(obj_lose);
                    act( AT_DAMAGE, "$p is pitted and etched!",
                         victim, obj_lose, NULL, TO_CHAR );
                    if ( ( iWear = obj_lose->wear_loc ) != WEAR_NONE )
                        victim->armor -= apply_ac( obj_lose, iWear );
                    obj_lose->value[0] -= 1;
                    obj_lose->cost      = 0;
                    if ( iWear != WEAR_NONE )
                        victim->armor += apply_ac( obj_lose, iWear );
                }
                break;

            case ITEM_CONTAINER:
                separate_obj( obj_lose );
                act( AT_DAMAGE, "$p fumes and dissolves!",
                     victim, obj_lose, NULL, TO_CHAR );
                act( AT_OBJECT, "The contents of $p spill out onto the ground.",
                     victim, obj_lose, NULL, TO_ROOM );
                act( AT_OBJECT, "The contents of $p spill out onto the ground.",
                     victim, obj_lose, NULL, TO_CHAR );
                empty_obj( obj_lose, NULL, victim->in_room );
                extract_obj( obj_lose );
                break;
            }
        }
    }

    dam = UMAX(10, ((GET_MAX_HIT(ch) * level) / GetMaxLevel(victim)));
    if ( saves_breath( level, victim ) )
        dam /= 2;
    if ( is_affected(ch, gsn_protection_from_acid_breath) )
        dam /= 4;
    return damage( ch, victim, dam, sn );
}



ch_ret spell_fire_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose;
    OBJ_DATA *obj_next;
    int dam;

    successful_casting( get_skilltype(sn), ch, victim, NULL );

    if ( chance(ch, 2 * level) &&
         !saves_breath( level, victim ) &&
         !is_affected(victim, gsn_protection_from_breath) &&
         !is_affected(victim, gsn_protection_from_fire_breath) )
    {
        for ( obj_lose = victim->first_carrying; obj_lose;
              obj_lose = obj_next )
        {
            char *msg;

            obj_next = obj_lose->next_content;

            if (ItemSave(obj_lose, sn))
                continue;

            switch ( obj_lose->item_type )
            {
            default:             continue;
            case ITEM_CONTAINER: msg = "$p ignites and burns!";   break;
            case ITEM_POTION:    msg = "$p bubbles and boils!";   break;
            case ITEM_SCROLL:    msg = "$p crackles and burns!";  break;
            case ITEM_STAFF:     msg = "$p smokes and chars!";    break;
            case ITEM_WAND:      msg = "$p sparks and sputters!"; break;
            case ITEM_FOOD:      msg = "$p blackens and crisps!"; break;
            case ITEM_PILL:      msg = "$p melts and drips!";     break;
            }

            separate_obj( obj_lose );
            act( AT_DAMAGE, msg, victim, obj_lose, NULL, TO_CHAR );
            if ( obj_lose->item_type == ITEM_CONTAINER )
            {
                act( AT_OBJECT, "The contents of $p spill out onto the ground.",
                     victim, obj_lose, NULL, TO_ROOM );
                act( AT_OBJECT, "The contents of $p spill out onto the ground.",
                     victim, obj_lose, NULL, TO_CHAR );
                empty_obj( obj_lose, NULL, victim->in_room );
            }
            extract_obj( obj_lose );
        }
    }


    dam = UMAX(10, ((GET_MAX_HIT(ch) * level) / GetMaxLevel(victim)));
    if ( saves_breath( level, victim ) )
        dam /= 2;
    if ( is_affected(ch, gsn_protection_from_fire_breath) )
        dam /= 4;
    return damage( ch, victim, dam, sn );
}


ch_ret spell_frost_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj_lose;
    OBJ_DATA *obj_next;
    int dam;

    successful_casting( get_skilltype(sn), ch, victim, NULL );

    if ( chance( ch, 2 * level ) &&
         !saves_breath( level, victim ) &&
         !is_affected(victim, gsn_protection_from_breath) &&
         !is_affected(victim, gsn_protection_from_frost_breath) )
    {
        for ( obj_lose = victim->first_carrying; obj_lose;
              obj_lose = obj_next )
        {
            char *msg;

            obj_next = obj_lose->next_content;

            if (ItemSave(obj_lose, sn))
                continue;

            switch ( obj_lose->item_type )
            {
            default:            continue;
            case ITEM_CONTAINER:
            case ITEM_DRINK_CON:
            case ITEM_POTION:   msg = "$p freezes and shatters!"; break;
            }

            separate_obj( obj_lose );
            act( AT_DAMAGE, msg, victim, obj_lose, NULL, TO_CHAR );
            if ( obj_lose->item_type == ITEM_CONTAINER )
            {
                act( AT_OBJECT, "The contents of $p spill out onto the ground.",
                     victim, obj_lose, NULL, TO_ROOM );
                act( AT_OBJECT, "The contents of $p spill out onto the ground.",
                     victim, obj_lose, NULL, TO_CHAR );
                empty_obj( obj_lose, NULL, victim->in_room );
            }
            extract_obj( obj_lose );
        }
    }

    dam = UMAX(10, ((GET_MAX_HIT(ch) * level) / GetMaxLevel(victim)));
    if ( saves_breath( level, victim ) )
        dam /= 2;
    if ( is_affected(ch, gsn_protection_from_frost_breath) )
        dam /= 4;
    return damage( ch, victim, dam, sn );
}


ch_ret spell_gas_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    bool ch_died;

    ch_died = FALSE;

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "You fail to breathe.\n\r", ch );
        return rNONE;
    }

    for ( vch = ch->in_room->first_person; vch; vch = vch_next )
    {
        vch_next = vch->next_in_room;
        if ( !IS_NPC( vch ) && vch->pcdata->wizinvis >= LEVEL_IMMORTAL )
            continue;

        if (vch->master == ch ||
            ch->master == vch ||
            is_same_group(ch, vch))
            continue;

        if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
        {
            successful_casting( get_skilltype(sn), ch, vch, NULL );

            dam = UMAX(10, ((GET_MAX_HIT(ch) * level) / GetMaxLevel(vch)));
            if ( saves_breath( level, vch ) )
                dam /= 2;
            if ( is_affected(ch, gsn_protection_from_gas_breath) )
                dam /= 4;
            if ( damage( ch, vch, dam, sn ) == rCHAR_DIED || char_died(vch) )
                ch_died = TRUE;
        }
    }
    if ( ch_died )
        return rCHAR_DIED;
    else
        return rNONE;
}



ch_ret spell_lightning_breath( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    successful_casting( get_skilltype(sn), ch, victim, NULL );

    dam = UMAX(10, ((GET_MAX_HIT(ch) * level) / GetMaxLevel(victim)));
    if ( saves_breath( level, victim ) )
        dam /= 2;
    if ( is_affected(ch, gsn_protection_from_electric_breath) )
        dam /= 4;
    return damage( ch, victim, dam, sn );
}

ch_ret spell_null( int sn, int level, CHAR_DATA *ch, void *vo )
{
    send_to_char( "That's not a spell!\n\r", ch );
    log_printf_plus(LOG_MAGIC, LEVEL_LOG_CSET, SEV_ERR, "Spell with null fun: %d, cast by: %s", sn, GET_NAME(ch));
    return rNONE;
}

/* don't remove, may look redundant, but is important */
ch_ret spell_notfound( int sn, int level, CHAR_DATA *ch, void *vo )
{
    send_to_char( "That's not a spell!\n\r", ch );
    log_printf_plus(LOG_MAGIC, LEVEL_LOG_CSET, SEV_ERR, "Spell with notfound fun: %d, cast by: %s", sn, GET_NAME(ch));
    return rNONE;
}


/*
 *   Haus' Spell Additions
 *
 */

/* to do: portal           (like mpcreatepassage)
 *        enchant armour?  (say -1/-2/-3 ac )
 *        sharpness        (makes weapon of caster's level)
 *        repair           (repairs armor)
 *        blood burn       (offensive)  * name: net book of spells *
 *        spirit scream    (offensive)  * name: net book of spells *
 *        something about saltpeter or brimstone
 */

/* Working on DM's transport eq suggestion - Scryn 8/13 */
ch_ret spell_transport( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char arg3[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    SKILLTYPE *skill = get_skilltype(sn);

    target_name = one_argument(target_name, arg3 );

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
         ||   victim == ch
         ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
         ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
         ||   IS_SET(victim->in_room->room_flags, ROOM_NO_ASTRAL)
         ||   IS_SET(victim->in_room->room_flags, ROOM_DEATH)
         ||   IS_SET(victim->in_room->room_flags, ROOM_PROTOTYPE)
         ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
         ||   GetAveLevel(victim) >= level + 15
         ||	(IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE))
         ||  (IS_NPC(victim) && saves_spell_staff( level, victim )) )
    {
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }


    if (victim->in_room == ch->in_room)
    {
        send_to_char("They are right beside you!\n\r", ch);
        return rSPELL_FAILED;
    }

    if ( (obj = get_obj_carry( ch, arg3 ) ) == NULL
         || ( carry_w(victim) + get_obj_weight ( obj ) ) > can_carry_w(victim)
         ||	(IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE)))
    {
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    separate_obj(obj);  /* altrag shoots, haus alley-oops! */

    if ( IS_OBJ_STAT( obj, ITEM_NODROP ) )
    {
        send_to_char( "You can't seem to let go of it.\n\r", ch );
        return rSPELL_FAILED;   /* nice catch, caine */
    }

    if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE )
         &&   get_trust( victim ) < LEVEL_IMMORTAL )
    {
        send_to_char( "That item is not for mortal hands to touch!\n\r", ch );
        return rSPELL_FAILED;   /* Thoric */
    }

    act( AT_MAGIC, "$p slowly dematerializes...", ch, obj, NULL, TO_CHAR );
    act( AT_MAGIC, "$p slowly dematerializes from $n's hands..", ch, obj, NULL, TO_ROOM );
    obj_from_char( obj );
    obj_to_char( obj, victim );
    act( AT_MAGIC, "$p from $n appears in your hands!", ch, obj, victim, TO_VICT );
    act( AT_MAGIC, "$p appears in $n's hands!", victim, obj, NULL, TO_ROOM );
    if ( IS_SET( sysdata.save_flags, SV_GIVE ) )
        save_char_obj(ch);
    if ( IS_SET( sysdata.save_flags, SV_RECEIVE ) )
        save_char_obj(victim);
    return rNONE;
}


ch_ret spell_portal( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *targetRoom, *fromRoom;
    int targetRoomVnum;
    OBJ_DATA *portalObj;
    EXIT_DATA *pexit;
    char buf[MAX_STRING_LENGTH];

    if ( (victim = get_char_world(ch, target_name)) == NULL )
    {
        send_to_char("The magic cannot find the target.\n\r", ch);
        return rNONE;
    }

    if ( victim == ch )
    {
        send_to_char("You can't create a portal to yourself.\n\r", ch);
        return rNONE;
    }

    if (victim->in_room == ch->in_room)
    {
        send_to_char("They are right beside you!\n\r", ch);
        return rNONE;
    }

    if (IS_SYSTEMFLAG(SYS_NOPORTAL))
    {
        send_to_char("Everything appears to have gone right, but no portal appears.\n\r", ch);
        return rNONE;
    }

    if ( !IsExtraPlanar(ch) &&
         is_other_plane(ch->in_room, victim->in_room))
    {
        send_to_char("The planes obscure your magic.\n\r", ch);
        if (IS_IMMORTAL(ch))
            send_to_char("However, your immortality overcomes it.\n\r", ch);
        else
            return rNONE;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_ASTRAL       ) ||
         IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL       ) ||
         IS_SET(ch->in_room->room_flags, ROOM_PROTOTYPE       ) ||
         IS_SET(ch->in_room->room_flags, ROOM_ARENA           ) ||
         IS_SET(victim->in_room->room_flags, ROOM_ARENA       ) ||
         IS_SET(victim->in_room->room_flags, ROOM_NO_ASTRAL   ) ||
         IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL   ) ||
         IS_SET(victim->in_room->room_flags, ROOM_PROTOTYPE   ) ||
         IS_SET(victim->in_room->room_flags, ROOM_DEATH   ) ||
         (IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE)) ||
         !victim->in_room                                        )
    {
        send_to_char("Eldritch wizardry obstucts thee.\n\r",ch);
        if (IS_IMMORTAL(ch))
            send_to_char("However, your immortality overcomes it.\n\r", ch);
        else
            return rNONE;
    }

    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_SUMMON)    ||
         IS_SET(victim->in_room->room_flags, ROOM_NO_SUMMON)  )
    {
        send_to_char("Ancient Magiks bar your path.\n\r",ch);
        if (IS_IMMORTAL(ch))
            send_to_char("However, your immortality overcomes them.\n\r", ch);
        else
            return rNONE;
    }

    if (!IS_NPC(victim) && IS_IMMORTAL(victim) && !IS_IMMORTAL(ch))
    {
	send_to_char("They wouldn't like that much.\n\r", ch);
        if (can_see(victim, ch))
	    ch_printf(victim, "%s just tried portaling to you.\n\r", GET_NAME(ch));
        return rNONE;
    }

    targetRoomVnum = victim->in_room->vnum;
    fromRoom = ch->in_room;
    targetRoom = victim->in_room;

    for ( pexit = fromRoom->first_exit; pexit; pexit = pexit->next )
    {
        if ( pexit->vdir == DIR_PORTAL )
        {
            send_to_char("Magic collides with magic and your spell fizzles.\n\r",ch);
            act( AT_MAGIC, "The portal crackles loudly, throwing off bright blue sparks.", ch, NULL, NULL, TO_ROOM );
            act( AT_MAGIC, "The portal crackles loudly, throwing off bright blue sparks.", ch, NULL, NULL, TO_CHAR );
            return rNONE;
        }
    }

    pexit = make_exit( fromRoom, targetRoom, DIR_PORTAL );
    pexit->keyword      = STRALLOC( "portal" );
    pexit->description  = STRALLOC( "Through the mists of the portal, you can faintly see...\n\r" );
    pexit->key          = 0;
    pexit->exit_info    = EX_PORTAL | EX_xENTER | EX_HIDDEN | EX_xLOOK;
    pexit->vnum         = targetRoomVnum;

    portalObj = create_object( OBJ_VNUM_PORTAL );
    if (!portalObj) return rNONE;
    portalObj->item_type = ITEM_PORTAL;
    portalObj->timer = 3;
    sprintf( buf, "A portal made by %s", ch->name );
    STRFREE( portalObj->short_descr );
    portalObj->short_descr = STRALLOC( buf );
    portalObj = obj_to_room( portalObj, ch->in_room );

    act( AT_MAGIC, "A misty portal suddenly appears.", ch, NULL, NULL, TO_ROOM );
    act( AT_MAGIC, "A misty portal suddenly appears.", ch, NULL, NULL, TO_CHAR );

    return rNONE;
}


ch_ret spell_farsight( int sn, int level, CHAR_DATA *ch, void *vo )
{
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA *victim;
    SKILLTYPE *skill = get_skilltype(sn);

    /* The spell fails if the victim isn't playing, the victim is the caster,
     the target room has private, solitary, noastral, death or proto flags,
     the caster's room is norecall, the victim is too high in level, the
     victim is a proto mob, the victim makes the saving throw or the pkill
     flag on the caster is not the same as on the victim.  Got it?
     */
    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
         ||   victim == ch
         ||   !victim->in_room
         ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
         ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
         ||   IS_SET(victim->in_room->room_flags, ROOM_NO_ASTRAL)
         ||   IS_SET(victim->in_room->room_flags, ROOM_DEATH)
         ||   IS_SET(victim->in_room->room_flags, ROOM_PROTOTYPE)
         ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
         ||   GetAveLevel(victim) >= level + 15
         ||	(IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE))
         ||  (IS_NPC(victim) && saves_spell_staff( level, victim ))
         ||  (!IS_NPC(victim) && CAN_PKILL(ch) != CAN_PKILL(victim) ) )
    {
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    location = victim->in_room;
    if (!location)
    {
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }
    successful_casting( skill, ch, victim, NULL );
    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, location );
    do_look( ch, "auto" );
    char_from_room( ch );
    char_to_room( ch, original );
    return rNONE;
}

ch_ret spell_recharge( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    if ( obj->item_type == ITEM_STAFF
         ||   obj->item_type == ITEM_WAND)
    {
        separate_obj(obj);
        if ( obj->value[2] == obj->value[1]
             ||   obj->value[1] > (obj->pIndexData->value[1] * 4) )
        {
            act( AT_PLAIN, "$p bursts into flames, injuring you!", ch, obj, NULL, TO_CHAR );
            act( AT_PLAIN, "$p bursts into flames, charring $n!", ch, obj, NULL, TO_ROOM);
            extract_obj(obj);
            if ( damage(ch, ch, number_fuzzy(15) * 2, TYPE_UNDEFINED) == rCHAR_DIED
                 ||   char_died(ch) )
                return rCHAR_DIED;
            else
                return rSPELL_FAILED;
        }

        if ( chance(ch, 2) )
        {
            act( AT_PLAIN, "$p glows with a blinding magical luminescence.",
                 ch, obj, NULL, TO_CHAR);
            obj->value[1] *= 2;
            obj->value[2] = obj->value[1];
            return rNONE;
        }
        else
            if ( chance(ch, 5) )
            {
                act( AT_PLAIN, "$p glows brightly for a few seconds...",
                     ch, obj, NULL, TO_CHAR);
                obj->value[2] = obj->value[1];
                return rNONE;
            }
            else
                if ( chance(ch, 10) )
                {
                    act( AT_PLAIN, "$p disintegrates into a void.", ch, obj, NULL, TO_CHAR);
                    act( AT_PLAIN, "$n's attempt at recharging fails, and $p disintegrates.",
                         ch, obj, NULL, TO_ROOM);
                    extract_obj(obj);
                    return rSPELL_FAILED;
                }
                else
                    if ( chance(ch, 50 - (GetAveLevel(ch)/2) ) )
                    {
                        send_to_char("Nothing happens.\n\r", ch);
                        return rSPELL_FAILED;
                    }
                    else
                    {
                        act( AT_MAGIC, "$p feels warm to the touch.", ch, obj, NULL, TO_CHAR);
                        --obj->value[1];
                        obj->value[2] = obj->value[1];
                        return rNONE;
                    }
    }
    else
    {
        send_to_char( "You can't recharge that!\n\r", ch);
        return rSPELL_FAILED;
    }
}

/*
 * Idea from AD&D 2nd edition player's handbook (c)1989 TSR Hobbies Inc.
 * -Thoric
 */
ch_ret spell_plant_pass( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
         ||   victim == ch
         ||   !victim->in_room
         ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
         ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
         ||   IS_SET(victim->in_room->room_flags, ROOM_NO_ASTRAL)
         ||   IS_SET(victim->in_room->room_flags, ROOM_DEATH)
         ||   IS_SET(victim->in_room->room_flags, ROOM_PROTOTYPE)
         ||  (victim->in_room->sector_type != SECT_FOREST
              &&   victim->in_room->sector_type != SECT_FIELD)
         ||  (ch->in_room->sector_type     != SECT_FOREST
              &&   ch->in_room->sector_type     != SECT_FIELD)
         ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
         ||   GetAveLevel(victim) >= level + 15
         ||	(IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE))
         ||  (IS_NPC(victim) && saves_spell_staff( level, victim ))
         ||  !in_hard_range( ch, victim->in_room->area )
         ||  (IS_SET(victim->in_room->area->flags, AFLAG_NOPKILL) && IS_PKILL(ch)))
    {
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    if ( ch->in_room->sector_type == SECT_FOREST )
        act( AT_MAGIC, "$n melds into a nearby tree!", ch, NULL, NULL, TO_ROOM );
    else
        act( AT_MAGIC, "$n melds into the grass!", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, victim->in_room );
    if ( ch->in_room->sector_type == SECT_FOREST )
        act( AT_MAGIC, "$n appears from behind a nearby tree!", ch, NULL, NULL, TO_ROOM );
    else
        act( AT_MAGIC, "$n grows up from the grass!", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    return rNONE;
}

/*
 * Vampire version of astral_walk				-Thoric
 */
ch_ret spell_mist_walk( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    bool allowday;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
         ||   victim == ch )
    {
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    if ( IS_PKILL(ch)
         &&   ch->pcdata->condition[COND_BLOODTHIRST] > 22 )
        allowday = TRUE;
    else
        allowday = FALSE;

    if ( (time_info.hour < 21 && time_info.hour > 5 && !allowday )
         ||   !victim->in_room
         ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
         ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
         ||   IS_SET(victim->in_room->room_flags, ROOM_NO_ASTRAL)
         ||   IS_SET(victim->in_room->room_flags, ROOM_DEATH)
         ||   IS_SET(victim->in_room->room_flags, ROOM_PROTOTYPE)
         ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
         ||   GetAveLevel(victim) >= level + 15
         ||	(IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE))
         ||  (IS_NPC(victim) && saves_spell_staff( level, victim ))
         ||  !in_hard_range( ch, victim->in_room->area )
         ||  (IS_SET(victim->in_room->area->flags, AFLAG_NOPKILL) && IS_PKILL(ch)))
    {
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    /* Subtract 22 extra bp for mist walk from 0500 to 2100 SB */
    if  ( time_info.hour < 21 && time_info.hour > 5 && !IS_NPC(ch) )
        gain_condition( ch, COND_BLOODTHIRST, - 22 );

    act( AT_DGREEN, "$n dissolves into a cloud of glowing mist, then vanishes!",
         ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, victim->in_room );
    act( AT_DGREEN, "A cloud of glowing mist engulfs you, then withdraws to unveil $n!",
         ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    return rNONE;
}

/*
 * Cleric version of astral_walk				-Thoric
 */
ch_ret spell_solar_flight( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
         ||   victim == ch
         ||  (time_info.hour > 18 || time_info.hour < 8)
         ||   !victim->in_room
         ||  !IS_OUTSIDE(ch)
         ||  !IS_OUTSIDE(victim)
         ||   ch->in_room->area->weather->precip >  0
         ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
         ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
         ||   IS_SET(victim->in_room->room_flags, ROOM_NO_ASTRAL)
         ||   IS_SET(victim->in_room->room_flags, ROOM_DEATH)
         ||   IS_SET(victim->in_room->room_flags, ROOM_PROTOTYPE)
         ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
         ||   GetAveLevel(victim) >= level + 15
         ||	(IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE))
         ||  (IS_NPC(victim) && saves_spell_staff( level, victim ))
         ||  !in_hard_range( ch, victim->in_room->area )
         ||  (IS_SET(victim->in_room->area->flags, AFLAG_NOPKILL) && IS_PKILL(ch)))
    {
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    act( AT_MAGIC, "$n disappears in a blinding flash of light!",
         ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, victim->in_room );
    act( AT_MAGIC, "$n appears in a blinding flash of light!",
         ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    return rNONE;
}

/* Scryn 2/2/96 */
ch_ret spell_remove_invis( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( target_name[0] == '\0' )
    {
        send_to_char( "What should the spell be cast upon?\n\r", ch );
        return rSPELL_FAILED;
    }

    obj = get_obj_carry( ch, target_name );

    if ( obj )
    {
        if ( !IS_OBJ_STAT(obj, ITEM_INVIS) )
            return rSPELL_FAILED;

        REMOVE_BIT(obj->extra_flags, ITEM_INVIS);
        act( AT_MAGIC, "$p becomes visible again.", ch, obj, NULL, TO_CHAR );

        send_to_char( "Ok.\n\r", ch );
        return rNONE;
    }
    else
    {
        CHAR_DATA *victim;

        victim = get_char_room(ch, target_name);

        if (victim)
        {
            if(!can_see(ch, victim))
            {
                ch_printf(ch, "You don't see %s!\n\r", target_name);
                return rSPELL_FAILED;
            }

            if(!IS_AFFECTED(victim, AFF_INVISIBLE))
            {
                send_to_char("They are not invisible!\n\r", ch);
                return rSPELL_FAILED;
            }

            if( is_safe(ch, victim) )
            {
                failed_casting( skill, ch, victim, NULL );
                return rSPELL_FAILED;
            }

            if ( IS_SET( victim->immune, RIS_MAGIC ) )
            {
                immune_casting( skill, ch, victim, NULL );
                return rSPELL_FAILED;
            }
            if( !IS_NPC(victim) )
            {
                if( chance(ch, 50) && BestSkLv(ch, sn) + 10 < GetMaxLevel(victim))
                {
                    failed_casting( skill, ch, victim, NULL );
                    return rSPELL_FAILED;
                }
                else
                    check_illegal_pk(ch, victim);
            }

            else
            {
                if( chance(ch, 50) && BestSkLv(ch, sn) + 15 < GetMaxLevel(victim))
                {
                    failed_casting( skill, ch, victim, NULL );
                    return rSPELL_FAILED;
                }
            }

            affect_strip ( victim, gsn_invis                        );
            affect_strip ( victim, gsn_group_invis                   );
            REMOVE_BIT   ( victim->affected_by, AFF_INVISIBLE       );
            send_to_char( "Ok.\n\r", ch );
            return rNONE;
        }

        ch_printf(ch, "You can't find %s!\n\r", target_name);
        return rSPELL_FAILED;
    }
}

/*
 * Animate Dead: Scryn 3/2/96
 * Modifications by Altrag 16/2/96
 */
ch_ret spell_animate_dead( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *mob;
    OBJ_DATA  *corpse;
    OBJ_DATA  *corpse_next;
    OBJ_DATA  *obj;
    OBJ_DATA  *obj_next;
    bool      found;
    MOB_INDEX_DATA *pMobIndex;
    AFFECT_DATA af;
    char       buf[MAX_STRING_LENGTH];
    SKILLTYPE *skill = get_skilltype(sn);

    found = FALSE;

    for (corpse = ch->in_room->first_content; corpse; corpse = corpse_next)
    {
        corpse_next = corpse->next_content;

        if (corpse->item_type == ITEM_CORPSE_NPC && corpse->cost != -MOB_VNUM_ANIMATED_CORPSE)
        {
            found = TRUE;
            break;
        }
    }

    if( !found )
    {
        send_to_char("You cannot find a suitable corpse here.\n\r", ch);
        return rSPELL_FAILED;
    }

    if ( !mob_exists_index(MOB_VNUM_ANIMATED_CORPSE) )
    {
        bug("Vnum %d not found for spell_animate_dead!", MOB_VNUM_ANIMATED_CORPSE);
        return rNONE;
    }


    if ( (pMobIndex = get_mob_index((sh_int) abs(corpse->cost) )) == NULL )
    {
        bug("Can not find mob for cost of corpse, spell_animate_dead");
        return rSPELL_FAILED;
    }

    if ( !IS_NPC(ch) )
    {
        if ( IS_VAMPIRE(ch) )
        {
            if ( !IS_IMMORTAL(ch) && ch->pcdata->condition[COND_BLOODTHIRST] -
                 (MIGetMaxLevel(pMobIndex)/3) < 0 )
            {
                send_to_char("You do not have enough blood power to reanimate this"
                             " corpse.\n\r", ch );
                return rSPELL_FAILED;
            }
            gain_condition(ch, COND_BLOODTHIRST, MIGetMaxLevel(pMobIndex)/3);
        }
        else if ( GET_MANA(ch) - (MIGetMaxLevel(pMobIndex)*4) < 0 )
        {
            send_to_char("You do not have enough mana to reanimate this "
                         "corpse.\n\r", ch);
            return rSPELL_FAILED;
        }
        else
            GET_MANA(ch) -= (MIGetMaxLevel(pMobIndex)*4);
    }

    if ( IS_IMMORTAL(ch) || ( chance(ch, 75) &&
                              MIGetMaxLevel(pMobIndex) - BestSkLv(ch, sn) < 10 ) )
    {
        mob = create_mobile( MOB_VNUM_ANIMATED_CORPSE );
        char_to_room( mob, ch->in_room );
        mob->levels[CLASS_WARRIOR] 	 = UMIN(GetMaxLevel(ch) / 2, MIGetMaxLevel(pMobIndex));
        mob->classes[CLASS_WARRIOR] = 1;
        mob->race  	 = pMobIndex->race;	/* should be undead */

        /* Fix so mobs wont have 0 hps and crash mud - Scryn 2/20/96 */
        if (!pMobIndex->hitnodice)
            mob->max_hit      = MIGetMaxLevel(pMobIndex) * 8 + number_range(
                                                                            MIGetMaxLevel(pMobIndex) * MIGetMaxLevel(pMobIndex) / 4,
                                                                            MIGetMaxLevel(pMobIndex) * MIGetMaxLevel(pMobIndex) );
        else
            mob->max_hit     = dice(pMobIndex->hitnodice, pMobIndex->hitsizedice)
                + pMobIndex->hitplus;
        mob->max_hit	 = UMAX( URANGE( mob->max_hit / 4,
                                         (mob->max_hit * corpse->value[3]) / 100,
                                         BestSkLv(ch, sn) * dice(20,10)), 1 );


        GET_HIT(mob)      = GET_MAX_HIT(mob);
        mob->damroll      = BestSkLv(ch, sn) / 8;
        mob->hitroll      = BestSkLv(ch, sn) / 6;
        mob->alignment    = ch->alignment;
        mob->exp          = 0;

        act(AT_MAGIC, "$n makes $T rise from the grave!", ch, NULL, pMobIndex->short_descr, TO_ROOM);
        act(AT_MAGIC, "You make $T rise from the grave!", ch, NULL, pMobIndex->short_descr, TO_CHAR);

        sprintf(buf, "animated corpse %s", pMobIndex->player_name);
        STRFREE(mob->name);
        mob->name = STRALLOC(buf);

        sprintf(buf, "The animated corpse of %s", pMobIndex->short_descr);
        STRFREE(mob->short_descr);
        mob->short_descr = STRALLOC(buf);

        sprintf(buf, "An animated corpse of %s struggles with the horror of its undeath.\n\r", pMobIndex->short_descr);
        STRFREE(mob->long_descr);
        mob->long_descr = STRALLOC(buf);
        add_follower( mob, ch );
        af.type      = sn;
        af.duration  = (int)((number_fuzzy( (level + 1) / 4 ) + 1) * DUR_CONV);
        af.location  = 0;
        af.modifier  = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char( mob, &af );

        if (corpse->first_content)
            for( obj = corpse->first_content; obj; obj = obj_next)
            {
                obj_next = obj->next_content;
                obj_from_obj(obj);
                obj_to_room(obj, corpse->in_room);
            }

        separate_obj(corpse);
        extract_obj(corpse);
        return rNONE;
    }

    failed_casting( skill, ch, NULL, NULL );
    return rSPELL_FAILED;
}

/* Works now.. -- Altrag */
ch_ret spell_possess( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA af;
    SKILLTYPE *skill = get_skilltype(sn);

    if (ch->desc->original)
    {
        send_to_char("You are not in your original state.\n\r", ch);
        return rSPELL_FAILED;
    }

    if ( (victim = get_char_room( ch, target_name ) ) == NULL)
    {
        send_to_char("They aren't here!\n\r", ch);
        return rSPELL_FAILED;
    }

    if (victim == ch)
    {
        send_to_char("You can't possess yourself!\n\r", ch);
        return rSPELL_FAILED;
    }

    if (!IS_NPC(victim))
    {
        send_to_char("You can't possess another player!\n\r", ch);
        return rSPELL_FAILED;
    }

    if (victim->desc)
    {
        ch_printf(ch, "%s is already possessed.\n\r", victim->short_descr);
        return rSPELL_FAILED;
    }

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    if ( IS_AFFECTED(victim, AFF_POSSESS)
         ||   level < (GetMaxLevel(victim) + 30)
         ||  victim->desc
         ||  !chance(ch, 25) )
    {
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    af.type      = sn;
    af.duration  = 20 + (BestSkLv(ch, sn) - GetMaxLevel(victim)) / 2;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = AFF_POSSESS;
    affect_to_char( victim, &af );

    sprintf(buf, "You have possessed %s!\n\r", victim->short_descr);

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    ch->switched        = victim;
    send_to_char( buf, victim );

    return rNONE;

}

/* Ignores pickproofs, but can't unlock containers. -- Altrag 17/2/96 */
ch_ret spell_knock( int sn, int level, CHAR_DATA *ch, void *vo )
{
    EXIT_DATA *pexit;
    SKILLTYPE *skill = get_skilltype(sn);

    set_char_color(AT_MAGIC, ch);
    /*
     * shouldn't know why it didn't work, and shouldn't work on pickproof
     * exits.  -Thoric
     */
    if ( !(pexit=find_door(ch, target_name, FALSE))
         ||   !IS_SET(pexit->exit_info, EX_CLOSED)
         ||   !IS_SET(pexit->exit_info, EX_LOCKED)
         ||    IS_SET(pexit->exit_info, EX_PICKPROOF) )
    {
        failed_casting( skill, ch, NULL, NULL );
        return rSPELL_FAILED;
    }
    REMOVE_BIT(pexit->exit_info, EX_LOCKED);
    send_to_char( "*Click*\n\r", ch );
    if ( pexit->rexit && pexit->rexit->to_room == ch->in_room )
        REMOVE_BIT( pexit->rexit->exit_info, EX_LOCKED );
    check_room_for_traps( ch, TRAP_UNLOCK | trap_door[pexit->vdir] );
    return rNONE;
}

/* Tells to sleepers in are. -- Altrag 17/2/96 */
ch_ret spell_dream( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    target_name = one_argument(target_name, arg);
    set_char_color(AT_MAGIC, ch);
    if ( !(victim = get_char_world(ch, arg)) ||
         victim->in_room->area != ch->in_room->area )
    {
        send_to_char("They aren't here.\n\r", ch);
        return rSPELL_FAILED;
    }
    if ( victim->position != POS_SLEEPING )
    {
        send_to_char("They aren't asleep.\n\r", ch);
        return rSPELL_FAILED;
    }
    if ( !target_name )
    {
        send_to_char("What do you want them to dream about?\n\r", ch );
        return rSPELL_FAILED;
    }

    set_char_color(AT_TELL, victim);
    ch_printf(victim, "You have dreams about %s telling you '%s'.\n\r",
              PERS(ch, victim), target_name);
    send_to_char("Ok.\n\r", ch);
    return rNONE;
}

#define  GET_SEX(ch) ((ch)->sex)
#define  ROOM_VNUM_POLY_STORAGE 4
ch_ret spell_polymorph( int sn, int level, CHAR_DATA *ch, void *vo )
{
    int poly_vnum;
    CHAR_DATA *poly_mob;
    int i, hitp;
    AFFECT_DATA af;
    OBJ_DATA *obj, *next_obj;

    if (IS_NPC(ch))
    {
        send_to_char("Mobs can't polymorph!\n\r", ch);
        return rSPELL_FAILED;
    }

    if (is_affected(ch, gsn_poly))
    {
        send_to_char("You are not in your original state.\n\r", ch);
        return rSPELL_FAILED;
    }

    poly_vnum = 0;

    for (i = MAX_POLY; i >= 0; i--)
        if (!str_cmp(target_name, PolyList[i].name) &&
            HAS_CLASS(ch, PolyList[i].cl) &&
            (GET_LEVEL(ch, PolyList[i].cl) >= PolyList[i].level))
        {
            poly_vnum = PolyList[i].number;
            break;
        }

    if (!poly_vnum){
        send_to_char("You can't find any of those!\n\r", ch);
        return rSPELL_FAILED;
    }

    poly_mob = make_poly_mob(ch, poly_vnum);
    if(!poly_mob)
    {
        bug("Spell_polymorph: null polymob!");
        return rSPELL_FAILED;
    }

    for (obj = ch->first_carrying; obj; obj = next_obj) {
        next_obj = obj->next_content;
        if (obj->wear_loc != WEAR_NONE)
            remove_obj(ch, obj->wear_loc, TRUE);
    }

    af.type      = gsn_poly;
    af.duration  = -1;
    af.bitvector = 0;

    if (poly_mob->damroll != ch->damroll)
    {
        af.location  = APPLY_DAMROLL;
        af.modifier  = poly_mob->damroll - ch->damroll;
        affect_to_char( ch, &af );
    }

    if (GET_RACE(ch) != GET_RACE(poly_mob))
    {
        af.location  = APPLY_RACE;
        af.modifier  = GET_RACE(poly_mob) - GET_RACE(ch);
        affect_to_char( ch, &af );
    }

    if (ch->affected_by != poly_mob->affected_by)
    {
        af.location  = APPLY_AFFECT;
        af.modifier  = poly_mob->affected_by;
        affect_to_char( ch, &af );
    }

    if (ch->affected_by2 != poly_mob->affected_by2)
    {
        af.location  = APPLY_AFF2;
        af.modifier  = poly_mob->affected_by2;
        affect_to_char( ch, &af );
    }
    if (ch->immune != poly_mob->immune)
    {
        af.location  = APPLY_IMMUNE;
        af.modifier  = poly_mob->immune;
        affect_to_char( ch, &af );
    }

    if (ch->susceptible != poly_mob->susceptible)
    {
        af.location  = APPLY_SUSCEPTIBLE;
        af.modifier  = poly_mob->susceptible;
        affect_to_char( ch, &af );
    }

    if (ch->absorb != poly_mob->absorb)
    {
        af.location  = APPLY_ABSORB;
        af.modifier  = poly_mob->absorb;
        affect_to_char( ch, &af );
    }

    if (ch->resistant != poly_mob->resistant)
    {
        af.location  = APPLY_RESISTANT;
        af.modifier  = poly_mob->resistant;
        affect_to_char( ch, &af );
    }
    if (get_curr_str(ch) != get_curr_str(poly_mob))
    {
        af.location  = APPLY_STR;
        af.modifier  = get_curr_str(poly_mob) - get_curr_str(ch);
        affect_to_char( ch, &af );
    }
    if (get_curr_dex(ch) != get_curr_dex(poly_mob))
    {
        af.location  = APPLY_DEX;
        af.modifier  = get_curr_dex(poly_mob) - get_curr_dex(ch);
        affect_to_char( ch, &af );
    }
    if (get_curr_con(ch) != get_curr_con(poly_mob))
    {
        af.location  = APPLY_CON;
        af.modifier  = get_curr_con(poly_mob) - get_curr_con(ch);
        affect_to_char( ch, &af );
    }
    if (GET_SEX(ch) != GET_SEX(poly_mob))
    {
        af.location  = APPLY_SEX;
        af.modifier  = GET_SEX(poly_mob) - GET_SEX(ch);
        affect_to_char( ch, &af );
    }
    if (ch->numattacks != poly_mob->numattacks)
    {
        af.location  = APPLY_NUMATTACKS;
        af.modifier  = poly_mob->numattacks - ch->numattacks;
        affect_to_char( ch, &af );
    }
    if (ch->barenumdie != poly_mob->barenumdie)
    {
        af.location  = APPLY_BARENUMDIE;
        af.modifier  = poly_mob->barenumdie - ch->barenumdie;
        affect_to_char( ch, &af );
    }
    if (ch->baresizedie != poly_mob->baresizedie)
    {
        af.location  = APPLY_BARESIZDIE;
        af.modifier  = poly_mob->baresizedie - ch->baresizedie;
        affect_to_char( ch, &af );
    }
    if (ch->height != poly_mob->height)
    {
        af.location  = APPLY_HEIGHT;
        af.modifier  = poly_mob->height - ch->height;
        affect_to_char( ch, &af );
    }
    if (ch->weight != poly_mob->weight)
    {
        af.location  = APPLY_WEIGHT;
        af.modifier  = poly_mob->weight - ch->weight;
        affect_to_char( ch, &af );
    }
    if (ch->antimagicp != poly_mob->antimagicp)
    {
        af.location  = APPLY_ANTIMAGIC;
        af.modifier  = poly_mob->antimagicp - ch->antimagicp;
        affect_to_char( ch, &af );
    }
    if (ch->armor != poly_mob->armor)
    {
        af.location  = APPLY_AC;
        af.modifier  = poly_mob->armor - ch->armor;
        affect_to_char( ch, &af );
    }
    if (GET_MAX_HIT(ch) != GET_MAX_HIT(poly_mob))
    {
        hitp = (GET_HIT(ch) * 100) / GET_MAX_HIT(ch);
        af.location  = APPLY_HIT;
        af.modifier  = GET_MAX_HIT(poly_mob) - GET_MAX_HIT(ch);
        affect_to_char( ch, &af );
        GET_HIT(ch) = (GET_MAX_HIT(ch) * hitp) / 100;
    }

    successful_casting( get_skilltype(gsn_poly), ch, poly_mob, NULL );

    char_to_room(poly_mob, get_room_index(ROOM_VNUM_POLY_STORAGE));
    extract_char(poly_mob, TRUE);

    if (!is_affected(ch, gsn_poly))
    {
        send_to_char("There would be no reason for that.\n\r", ch);
        return rSPELL_FAILED;
    }

    return rNONE;
}

void switch_stuff(CHAR_DATA *ch, CHAR_DATA *oc)
{
    OBJ_DATA *obj, *next_obj;
    int i;
    float f;

    for (obj = ch->first_carrying; obj; obj = next_obj) {
        next_obj = obj->next_content;
        if (obj->wear_loc != WEAR_NONE)
            remove_obj(ch, obj->wear_loc, TRUE);
        obj_from_char(obj);
        obj_to_char(obj, oc);
    }
    GET_ALIGN(oc) = GET_ALIGN(ch);
    for (i=0;i<MAX_CURR_TYPE;i++)
    {
        GET_MONEY(oc,i) = GET_MONEY(ch, i);
        GET_BALANCE(oc,i) = GET_BALANCE(ch, i);
    }
    GET_MANA(oc) = GET_MANA(ch);
    f = GET_MAX_HIT(oc) * GET_HIT(ch) / GET_MAX_HIT(ch);
    GET_HIT(oc) = (sh_int) f;
    for (i = 0; i < MAX_SKILL; i++)
        oc->pcdata->learned[i] = ch->pcdata->learned[i];
    if (IS_NPC(oc)) {
        GET_EXP(oc) = GET_EXP(ch);
        oc->max_mana = GET_MAX_MANA(ch);
        for (i = FIRST_CLASS; i < MAX_CLASS; i++) {
            oc->classes[i] = ch->classes[i];
            oc->levels[i] = ch->levels[i];
        }
    } else {
        gain_exp(oc, GET_EXP(ch) - GET_EXP(oc));
    }
}

CHAR_DATA *make_poly_mob(CHAR_DATA *ch, int vnum)
{
    CHAR_DATA *mob;

    if(!ch)
    {
        bug("Make_poly_mob: null ch!");
        return NULL;
    }

    if ( (mob = create_mobile( vnum ) ) == NULL )
    {
        bug("Make_poly_mob: Can't find mob %d", vnum);
        return NULL;
    }

    SET_BIT(mob->act, ACT_POLYMORPHED);
    SET_BIT(mob->act, ACT_SENTINEL);
    REMOVE_BIT(mob->act, ACT_AGGRESSIVE);
    CREATE(mob->pcdata, PC_DATA, 1);
    return mob;
}

void do_revert(CHAR_DATA *ch, char *argument)
{

    int hitp;
    AFFECT_DATA *paf;


    if ( !is_affected(ch, gsn_poly) )
    {
        send_to_char("You are not polymorphed.\n\r", ch);
        return;
    }
    send_to_char("You revert to your origional form.\n\r",ch);
    act(AT_MAGIC,"$n reverts back to $s normal form.\n\r",ch,NULL,NULL,TO_ROOM);

    hitp = (GET_HIT(ch) * 100) / GET_MAX_HIT(ch);
    while ((paf=is_affected(ch, gsn_poly)))
           affect_remove(ch,paf);
    GET_HIT(ch) = (GET_MAX_HIT(ch) * hitp) / 100;

}
/* Added spells spiral_blast, scorching surge,
 nostrum, and astral   by SB for Augurer class
 7/10/96 */
ch_ret spell_spiral_blast( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    int hpch;
    bool ch_died;

    ch_died = FALSE;

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
        set_char_color( AT_MAGIC, ch );
        send_to_char( "You fail to breathe.\n\r", ch );
        return rNONE;
    }

    for ( vch = ch->in_room->first_person; vch; vch = vch_next )
    {
        vch_next = vch->next_in_room;
        if ( !IS_NPC( vch ) && vch->pcdata->wizinvis >= LEVEL_IMMORTAL )
            continue;

        if (vch->master == ch ||
            ch->master == vch ||
            is_same_group(ch, vch))
            continue;

        if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
        {
            act( AT_MAGIC, "Swirling colours radiate from $n"
                 ", encompassing $N.",
                 ch, ch, vch, TO_ROOM );
            act( AT_MAGIC, "Swirling colours radiate from you,"
                 " encompassing $N",
                 ch, ch, vch , TO_CHAR );

            hpch = UMAX( 10, GET_HIT(ch) );
            dam  = number_range( hpch/14+1, hpch/7 );
            if ( saves_breath( level, vch ) )
                dam /= 2;
            if ( damage( ch, vch, dam, sn ) == rCHAR_DIED ||
                 char_died(ch) )
                ch_died = TRUE;
        }
    }

    if ( ch_died )
        return rCHAR_DIED;
    else
        return rNONE;
}

ch_ret spell_scorching_surge( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    static const sh_int dam_each[] =
    {
        0,
        0,   0,   0,   0,   0,          0,   0,   0,   0,   0,
        0,   0,   0,   0,  30,         35,  40,  45,  50,  55,
        60,  65,  70,  75,  80,         82,  84,  86,  88,  90,
        92,  94,  96,  98, 100,   	102, 104, 106, 108, 110,
        112, 114, 116, 118, 120,       122, 124, 126, 128, 130,
        132, 134, 136, 138, 140,  	142, 144, 146, 148, 150,
        152, 154, 156, 158, 160,   	162, 164, 166, 168, 170
    };
    int dam;

    level       = UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level       = UMAX(0, level);
    dam         = number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell_staff( level, victim ) )
        dam /= 2;
    act( AT_MAGIC, "A fiery current lashes through $n's body!",
         ch, NULL, NULL, TO_ROOM );
    act( AT_MAGIC, "A fiery current lashes through your body!",
         ch, NULL, NULL, TO_CHAR );
    return damage( ch, victim, (int)(dam*1.4), sn );
}


ch_ret spell_helical_flow( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL
         ||   victim == ch
         ||   !victim->in_room
         ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
         ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
         ||   IS_SET(victim->in_room->room_flags, ROOM_NO_ASTRAL)
         ||   IS_SET(victim->in_room->room_flags, ROOM_DEATH)
         ||   IS_SET(victim->in_room->room_flags, ROOM_PROTOTYPE)
         ||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
         ||  GetAveLevel(victim) >= level + 15
         ||  (IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE))
         ||  (IS_NPC(victim) && saves_spell_staff( level, victim ))
         ||  !in_hard_range( ch, victim->in_room->area )
         ||  (IS_SET(victim->in_room->area->flags, AFLAG_NOPKILL) && IS_PKILL(ch)))
    {
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    act( AT_MAGIC, "$n coils into an ascending column of colour,"
         " vanishing into thin air.", ch, NULL, NULL,
         TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, victim->in_room );
    act( AT_MAGIC, "A coil of colours descends from above, "
         "revealing $n as it dissipates.", ch, NULL,
         NULL, TO_ROOM );
    do_look( ch, "auto" );
    return rNONE;
}


/*******************************************************
 * Everything after this point is part of SMAUG SPELLS *
 *******************************************************/

/*
 * saving throw check						-Thoric
 */
bool check_save( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim )
{
    SKILLTYPE *skill = get_skilltype(sn);
    bool saved = FALSE;

    if ( SPELL_FLAG(skill, SF_PKSENSITIVE)
         &&  !IS_NPC(ch) && !IS_NPC(victim) )
        level /= 2;

    if ( SPELL_SAVE(skill) )
        switch( SPELL_SAVE(skill) )
        {
        case SS_POISON_DEATH:
            saved = saves_poison_death(level, victim);	break;
        case SS_ROD_WANDS:
            saved = saves_wands(level, victim);		break;
        case SS_PARA_PETRI:
            saved = saves_para_petri(level, victim);	break;
        case SS_BREATH:
            saved = saves_breath(level, victim);	break;
        case SS_SPELL_STAFF:
            saved = saves_spell_staff(level, victim);	break;
        case SS_NONE:
            saved = FALSE;
        }
    return saved;
}

/*
 * Generic offensive spell damage attack			-Thoric
 */
ch_ret spell_attack( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    SKILLTYPE *skill = get_skilltype(sn);
    bool saved = check_save( sn, level, ch, victim );
    int dam;
    ch_ret retcode;

    if ( saved && !SPELL_FLAG( skill, SF_SAVE_HALF_DAMAGE ) )
    {
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }
    if ( skill->dice )
        dam = UMAX( 0, dice_parse( ch, level, skill->dice ) );
    else
        dam = dice( 1, level/2 );
    if ( saved )
        dam /= 2;
    retcode = damage( ch, victim, dam, sn );
    if ( retcode == rNONE && skill->affects
         &&  !char_died(ch) && !char_died(victim) )
        retcode = spell_affectchar( sn, level, ch, victim );
    return retcode;
}

/*
 * Generic area attack						-Thoric
 */
ch_ret spell_area_attack( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch, *vch_next;
    SKILLTYPE *skill = get_skilltype(sn);
    bool saved;
    bool affects;
    int dam;
    bool ch_died = FALSE;
    ch_ret retcode = 0;

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
        failed_casting( skill, ch, NULL, NULL );
        return rSPELL_FAILED;
    }

    affects = (skill->affects ? TRUE : FALSE);
    if ( skill->hit_char && skill->hit_char[0] != '\0' )
        if ( !strstr(skill->hit_char, "$N") )
            act( AT_MAGIC, skill->hit_char, ch, NULL, NULL, TO_CHAR );
    if ( skill->hit_room && skill->hit_room[0] != '\0' )
        if ( !strstr(skill->hit_room, "$N") )
            act( AT_MAGIC, skill->hit_room, ch, NULL, NULL, TO_ROOM );

    for ( vch = ch->in_room->first_person; vch; vch = vch_next )
    {
        vch_next = vch->next_in_room;

        if ( !IS_NPC( vch ) && vch->pcdata->wizinvis >= LEVEL_IMMORTAL )
            continue;

        if (vch->master == ch ||
            ch->master == vch ||
            is_same_group(ch, vch))
            continue;

        if ( vch != ch && ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) ) )
        {
            saved = check_save( sn, level, ch, vch );
            if ( saved && !SPELL_FLAG( skill, SF_SAVE_HALF_DAMAGE ) )
            {
                failed_casting( skill, ch, vch, NULL );
                dam = 0;
            }
            else
                if ( skill->dice )
                    dam = dice_parse(ch, level, skill->dice);
                else
                    dam = dice( 1, level/2 );
            if ( saved && SPELL_FLAG( skill, SF_SAVE_HALF_DAMAGE ) )
                dam /= 2;
            retcode = damage( ch, vch, dam, sn );
        }
        if ( retcode == rNONE && affects && !char_died(ch) && !char_died(vch) )
            retcode = spell_affectchar( sn, level, ch, vch );
        if ( retcode == rCHAR_DIED || char_died(ch) )
        {
            ch_died = TRUE;
            break;
        }
    }
    return retcode;
}


ch_ret spell_affectchar( int sn, int level, CHAR_DATA *ch, void *vo )
{
    AFFECT_DATA af, *paf;
    SMAUG_AFF *saf;
    SKILLTYPE *skill = get_skilltype(sn);
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int saves;
    ch_ret retcode = rNONE;

    if ( SPELL_FLAG( skill, SF_RECASTABLE ) )
        affect_strip( victim, sn );
    for ( saf = skill->affects; saf; saf = saf->next )
    {
        if ( saf->location >= REVERSE_APPLY )
        {
            bug("reverse_apply sn: %d",sn);
            victim = ch;
        }
        else
            victim = (CHAR_DATA *) vo;
        /* Check if char has this bitvector already */
        if ( (af.bitvector=saf->bitvector) != 0
             && saf->location < REVERSE_APPLY
             && IS_AFFECTED( victim, af.bitvector )
             && !SPELL_FLAG( skill, SF_ACCUMULATIVE ) )
            continue;
        /*
         * necessary for affect_strip to work properly...
         */
        switch ( af.bitvector )
        {
        default:
            af.type = sn;			break;
        case AFF_POISON:
            af.type = gsn_poison;
            saves = ris_save( victim, level, RIS_POISON );
            if ( saves == 1000 )
            {
                retcode = rVICT_IMMUNE;
                if ( SPELL_FLAG(skill, SF_STOPONFAIL) )
                    return retcode;
                continue;
            }
            if ( saves_poison_death( saves, victim ) )
            {
                if ( SPELL_FLAG(skill, SF_STOPONFAIL) )
                    return retcode;
                continue;
            }
            victim->mental_state = URANGE( 30, victim->mental_state + 2, 100 );
            break;
        case AFF_BLIND:
            af.type = gsn_blindness;
            break;
        case AFF_CURSE:
            af.type = gsn_curse;
            break;
        case AFF_INVISIBLE:
            af.type = gsn_invis;
            break;
        case AFF_SLEEP:
            af.type = gsn_sleep;
            saves = ris_save( victim, level, RIS_SLEEP );
            if ( saves == 1000 )
            {
                retcode = rVICT_IMMUNE;
                if ( SPELL_FLAG(skill, SF_STOPONFAIL) )
                    return retcode;
                continue;
            }
            break;
        case AFF_CHARM:
            af.type = gsn_charm_person;
            saves = ris_save( victim, level, RIS_CHARM );
            if ( saves == 1000 )
            {
                retcode = rVICT_IMMUNE;
                if ( SPELL_FLAG(skill, SF_STOPONFAIL) )
                    return retcode;
                continue;
            }
            break;
        case AFF_POSSESS:
            af.type = gsn_possess;		break;
        }
        af.duration  = dice_parse(ch, level, saf->duration);
        af.modifier  = dice_parse(ch, level, saf->modifier);
        af.location  = saf->location % REVERSE_APPLY;
        if ( af.duration == 0 )
        {
            /*int xp_gain;*/

            switch( af.location )
            {
            case APPLY_HIT:
                GET_HIT(victim) = URANGE( 0, GET_HIT(victim) + af.modifier, GET_MAX_HIT(victim) );
                update_pos( victim );
#if 0
                if ( (af.modifier > 0 && ch->fighting && ch->fighting->who == victim)
                     ||   (af.modifier > 0 && victim->fighting && victim->fighting->who == ch) )
                {
                    xp_gain = (int) (ch->fighting->xp * af.modifier*2) / victim->max_hit;
                    gain_exp( ch, 0 - xp_gain );
                }
#endif
                break;
            case APPLY_MANA:
                GET_MANA(victim) = URANGE( 0, GET_MANA(victim) + af.modifier, GET_MAX_MANA(victim) );
                update_pos( victim );
                break;
            case APPLY_MOVE:
                GET_MOVE(victim) = URANGE( 0, GET_MOVE(victim) + af.modifier, GET_MAX_MOVE(victim) );
                update_pos( victim );
                break;
            case APPLY_FULL:
                victim->pcdata->condition[COND_FULL] = URANGE( 0, GET_COND(victim, COND_FULL) + af.modifier, MAX_COND_VAL );
                update_pos( victim );
                break;
            case APPLY_DRUNK:
                victim->pcdata->condition[COND_DRUNK] = URANGE( 0, GET_COND(victim, COND_DRUNK) + af.modifier, MAX_COND_VAL );
                update_pos( victim );
                break;
            case APPLY_THIRST:
                victim->pcdata->condition[COND_THIRST] = URANGE( 0, GET_COND(victim, COND_THIRST) + af.modifier, MAX_COND_VAL );
                update_pos( victim );
                break;

            case APPLY_MENTALSTATE:
                victim->mental_state = URANGE( 0, victim->mental_state + af.modifier, 100 );
                update_pos( victim );
                break;

            default:
                if (saf->location >= REVERSE_APPLY)
                    while ((paf=is_affected(victim, af.type)))
                        affect_remove(victim, paf);
                else
                    affect_modify( victim, &af, TRUE );
                break;
            }
        }
        else
            if ( SPELL_FLAG( skill, SF_ACCUMULATIVE ) )
                affect_join( victim, &af );
            else
                affect_to_char( victim, &af );
    }
    update_pos( victim );
    return retcode;
}


/*
 * Generic spell affect						-Thoric
 */
ch_ret spell_affect( int sn, int level, CHAR_DATA *ch, void *vo )
{
    SMAUG_AFF *saf;
    SKILLTYPE *skill = get_skilltype(sn);
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    bool groupsp;
    bool areasp;
    bool hitchar = FALSE, hitroom = FALSE, hitvict = FALSE;
    ch_ret retcode;

    if ( !skill->affects )
    {
        bug( "spell_affect has no affects sn %d", sn );
        return rNONE;
    }
    if ( SPELL_FLAG(skill, SF_GROUPSPELL) )
        groupsp = TRUE;
    else
        groupsp = FALSE;

    if ( SPELL_FLAG(skill, SF_AREA ) )
        areasp = TRUE;
    else
        areasp = FALSE;
    if ( !groupsp && !areasp )
    {
        /* Can't find a victim */
        if ( !victim )
        {
            failed_casting( skill, ch, victim, NULL );
            return rSPELL_FAILED;
        }

        if ( (skill->type != SKILL_HERB
              &&    IS_SET( victim->immune, RIS_MAGIC ))
             ||    is_immune( victim, skill ) )
        {
            immune_casting( skill, ch, victim, NULL );
            return rSPELL_FAILED;
        }

        /* Spell is already on this guy */
        if ( is_affected( victim, sn )
             &&  !SPELL_FLAG( skill, SF_ACCUMULATIVE )
             &&  !SPELL_FLAG( skill, SF_RECASTABLE ) )
        {
            /*failed_casting( skill, ch, victim, NULL );*/
            send_to_char("This spell is not recastable or accumulative.\n\r",ch);
            return rSPELL_FAILED;
        }

        if ( (saf = skill->affects) && !saf->next
             &&    saf->location == APPLY_STRIPSN
             &&   !is_affected( victim, dice_parse(ch, level, saf->modifier) ) )
        {
            failed_casting( skill, ch, victim, NULL );
            return rSPELL_FAILED;
        }

        if ( check_save( sn, level, ch, victim ) )
        {
            failed_casting( skill, ch, victim, NULL );
            return rSPELL_FAILED;
        }
    }
    else
    {
        if ( skill->hit_char && skill->hit_char[0] != '\0' )
        {
            if ( strstr(skill->hit_char, "$N") )
                hitchar = TRUE;
            else
                act( AT_MAGIC, skill->hit_char, ch, NULL, NULL, TO_CHAR );
        }
        if ( skill->hit_room && skill->hit_room[0] != '\0' )
        {
            if ( strstr(skill->hit_room, "$N") )
                hitroom = TRUE;
            else
                act( AT_MAGIC, skill->hit_room, ch, NULL, NULL, TO_ROOM );
        }
        if ( skill->hit_vict && skill->hit_vict[0] != '\0' )
            hitvict = TRUE;
        if ( victim )
            victim = victim->in_room->first_person;
        else
            victim = ch->in_room->first_person;
    }
    if ( !victim )
    {
        bug( "spell_affect: could not find victim: sn %d", sn );
        failed_casting( skill, ch, victim, NULL );
        return rSPELL_FAILED;
    }

    for ( ; victim; victim = victim->next_in_room )
    {
        if ( groupsp || areasp )
        {
            if ((groupsp && !is_same_group( victim, ch ))
                ||	 IS_SET( victim->immune, RIS_MAGIC )
                ||   is_immune( victim, skill )
                ||   check_save(sn, level, ch, victim)
                || (!SPELL_FLAG(skill, SF_RECASTABLE) && is_affected(victim, sn)))
                continue;

            if ( hitvict && ch != victim )
            {
                act( AT_MAGIC, skill->hit_vict, ch, NULL, victim, TO_VICT );
                if ( hitroom )
                {
                    act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_NOTVICT );
                    act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_CHAR );
                }
            }
            else
                if ( hitroom )
                    act( AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_ROOM );
            if ( ch == victim )
            {
                if ( hitvict )
                    act( AT_MAGIC, skill->hit_vict, ch, NULL, ch, TO_CHAR );
                else
                    if ( hitchar )
                        act( AT_MAGIC, skill->hit_char, ch, NULL, ch, TO_CHAR );
            }
            else
                if ( hitchar )
                    act( AT_MAGIC, skill->hit_char, ch, NULL, victim, TO_CHAR );
        }
        retcode = spell_affectchar( sn, level, ch, victim );
        if ( !groupsp && !areasp )
        {
            if ( retcode == rVICT_IMMUNE )
                immune_casting( skill, ch, victim, NULL );
            else
                successful_casting( skill, ch, victim, NULL );
            break;
        }
    }
    return rNONE;
}

/*
 * Generic inventory object spell				-Thoric
 */
ch_ret spell_obj_inv( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    SKILLTYPE *skill = get_skilltype(sn);
    OBJ_DATA *clone;

    if ( !obj )
    {
        failed_casting( skill, ch, NULL, NULL );
        return rNONE;
    }

    switch( SPELL_ACTION(skill) )
    {
    case SA_NONE:
        return rNONE;

    case SA_CREATE:
        if ( SPELL_FLAG(skill, SF_WATER) )	/* create water */
        {
            int water;

            if ( obj->item_type != ITEM_DRINK_CON )
            {
                send_to_char( "It is unable to hold water.\n\r", ch );
                return rSPELL_FAILED;
            }

            if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
            {
                send_to_char( "It contains some other liquid.\n\r", ch );
                return rSPELL_FAILED;
            }

            water = UMIN( (skill->dice ? dice_parse(ch, level, skill->dice) : level)
                          * (ch->in_room->area->weather->precip >= 0 ? 2 : 1),
                          obj->value[0] - obj->value[1] );

            if ( water > 0 )
            {
                separate_obj(obj);
                obj->value[2] = LIQ_WATER;
                obj->value[1] += water;
                if ( !is_name( "water", obj->name ) )
                {
                    char buf[MAX_STRING_LENGTH];

                    sprintf( buf, "%s water", obj->name );
                    STRFREE( obj->name );
                    obj->name = STRALLOC( buf );
                }
            }
            successful_casting( skill, ch, NULL, obj );
            return rNONE;
        }
        if ( SPELL_DAMAGE(skill) == SD_FIRE )	/* burn object */
        {
            /* return rNONE; */
        }
        if ( SPELL_DAMAGE(skill) == SD_POISON	/* poison object */
             ||   SPELL_CLASS(skill)  == SC_DEATH )
        {
            switch( obj->item_type )
            {
            default:
                failed_casting( skill, ch, NULL, obj );
                break;
            case ITEM_FOOD:
            case ITEM_DRINK_CON:
                separate_obj(obj);
                obj->value[3] = 1;
                successful_casting( skill, ch, NULL, obj );
                break;
            }
            return rNONE;
        }
        if ( SPELL_CLASS(skill) == SC_LIFE	/* purify */
             &&  (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON) )
        {
            switch( obj->item_type )
            {
            default:
                failed_casting( skill, ch, NULL, obj );
                break;
            case ITEM_FOOD:
            case ITEM_DRINK_CON:
                separate_obj(obj);
                obj->value[3] = 0;
                successful_casting( skill, ch, NULL, obj );
                break;
            }
            return rNONE;
        }

        if ( SPELL_CLASS(skill) != SC_NONE )
        {
            failed_casting( skill, ch, NULL, obj );
            return rNONE;
        }
        switch( SPELL_POWER(skill) )		/* clone object */
        {

        default:
        case SP_NONE:
            if ( BestSkLv(ch, sn) - number_range(1,40) < 10
                 ||   obj->cost > BestSkLv(ch, sn) * get_curr_int(ch) * get_curr_wis(ch) )
            {
                failed_casting( skill, ch, NULL, obj );
                return rNONE;
            }
            break;
        case SP_MINOR:
            if ( BestSkLv(ch, sn) - number_range(1,40) < 20
                 ||   obj->cost > BestSkLv(ch, sn) * get_curr_int(ch) / 5 )
            {
                failed_casting( skill, ch, NULL, obj );
                return rNONE;
            }
            break;
        case SP_GREATER:
            if ( BestSkLv(ch, sn) - number_range(1,40) < 5
                 ||   obj->cost > BestSkLv(ch, sn) * 10 * get_curr_int(ch) * get_curr_wis(ch) )
            {
                failed_casting( skill, ch, NULL, obj );
                return rNONE;
            }
            break;
        case SP_MAJOR:
            if ( BestSkLv(ch, sn) - number_range(1,40) < 0
                 ||   obj->cost > BestSkLv(ch, sn) * 50 * get_curr_int(ch) * get_curr_wis(ch) )
            {
                failed_casting( skill, ch, NULL, obj );
                return rNONE;
            }
            break;
        }
        clone = clone_object(obj);
        clone->timer = skill->dice ? dice_parse(ch, level, skill->dice) : 0;
        obj_to_char( clone, ch );
        successful_casting( skill, ch, NULL, obj );
        return rNONE;

    case SA_DESTROY:
    case SA_RESIST:
    case SA_SUSCEPT:
    case SA_DIVINATE:
        if ( SPELL_DAMAGE(skill) == SD_POISON ) /* detect poison */
        {
            if ( obj->item_type == ITEM_DRINK_CON
                 ||   obj->item_type == ITEM_FOOD )
            {
                if ( obj->value[3] != 0 )
                    send_to_char( "You smell poisonous fumes.\n\r", ch );
                else
                    send_to_char( "It looks very delicious.\n\r", ch );
            }
            else
                send_to_char( "It doesn't look poisoned.\n\r", ch );
            return rNONE;
        }
        return rNONE;
    case SA_OBSCURE:			/* make obj invis */
        if ( SPELL_CLASS(skill) == SC_ILLUSION )
        {
            if ( IS_OBJ_STAT(obj, ITEM_INVIS)
                 || chance(ch, skill->dice ? dice_parse(ch, level, skill->dice) : 20))
            {
                failed_casting( skill, ch, NULL, NULL );
                return rSPELL_FAILED;
            }
            successful_casting( skill, ch, NULL, obj );
            SET_BIT(obj->extra_flags, ITEM_INVIS);
        }
        else if ( SPELL_CLASS(skill) == SC_DEATH )
        {
            if ( IS_OBJ_STAT(obj, ITEM_NODROP) || IS_OBJ_STAT(obj, ITEM_NOREMOVE)
                 || chance(ch, skill->dice ? dice_parse(ch, level, skill->dice) : 20))
            {
                failed_casting( skill, ch, NULL, NULL );
                return rSPELL_FAILED;
            }
            successful_casting( skill, ch, NULL, obj );
            SET_BIT(obj->extra_flags, ITEM_NODROP);
            SET_BIT(obj->extra_flags, ITEM_NOREMOVE);
        }
        else if ( SPELL_CLASS(skill) == SC_LIFE )
        {
            if ( IS_OBJ_STAT(obj, ITEM_NODROP) || IS_OBJ_STAT(obj, ITEM_NOREMOVE)
                 || chance(ch, skill->dice ? dice_parse(ch, level, skill->dice) : 20))
            {
                successful_casting( skill, ch, NULL, obj );
                REMOVE_BIT(obj->extra_flags, ITEM_NODROP);
                REMOVE_BIT(obj->extra_flags, ITEM_NOREMOVE);
            } else {
                failed_casting( skill, ch, NULL, NULL );
                return rSPELL_FAILED;
            }
        }
        return rNONE;

    case SA_CHANGE:
        return rNONE;
    }
    return rNONE;
}

/*
 * Generic object creating spell				-Thoric
 */
ch_ret spell_create_obj( int sn, int level, CHAR_DATA *ch, void *vo )
{
    SKILLTYPE *skill = get_skilltype(sn);
    int vnum = skill->value;
    OBJ_DATA *obj;

    /*
     * Add predetermined objects here
     */
    if (vnum==0)
    {
        if (SPELL_POWER(skill) == SP_NONE)
        {
            if ( !str_cmp( target_name, "sword" ) )
                vnum = OBJ_VNUM_SCHOOL_SWORD;
            else if ( !str_cmp( target_name, "shield" ) )
                vnum = OBJ_VNUM_SCHOOL_SHIELD;
        }
        else if (SPELL_POWER(skill) == SP_MINOR) /* minor creation */
        {
            if (!str_cmp(target_name, "sword"))
                vnum = OBJ_VNUM_MINORC_LONG_SWORD;
            else if (!str_cmp(target_name, "shield"))
                vnum = OBJ_VNUM_MINORC_SHIELD;
            else if (!str_cmp(target_name, "raft"))
                vnum = OBJ_VNUM_MINORC_RAFT;
            else if (!str_cmp(target_name, "bag"))
                vnum = OBJ_VNUM_MINORC_BAG;
            else if (!str_cmp(target_name, "barrel"))
                vnum = OBJ_VNUM_MINORC_WATER_BARREL;
            else if (!str_cmp(target_name, "bread"))
                vnum = OBJ_VNUM_MINORC_BREAD;
        }
        else if (SPELL_POWER(skill) == SP_MAJOR) /* major creation */
        {
            if (!str_cmp(target_name, "boots"))
                vnum = OBJ_VNUM_MAJORC_BOOTS;
            else if (!str_cmp(target_name, "leggings"))
                vnum = OBJ_VNUM_MAJORC_LEGGINGS;
            else if (!str_cmp(target_name, "sleeves"))
                vnum = OBJ_VNUM_MAJORC_SLEEVES;
            else if (!str_cmp(target_name, "helmet"))
                vnum = OBJ_VNUM_MAJORC_HELMET;
            else if (!str_cmp(target_name, "breast"))
                vnum = OBJ_VNUM_MAJORC_BREAST;
            else if (!str_cmp(target_name, "gloves"))
                vnum = OBJ_VNUM_MAJORC_GLOVES;
        }
        else if (SPELL_POWER(skill) == SP_GREATER) /* greater creation? */
        {
	    log_printf_plus(LOG_MAGIC, LEVEL_LOG_CSET, SEV_DEBUG, "greater creation?");

        }
	else
        {
	    log_printf_plus(LOG_MAGIC, LEVEL_LOG_CSET, SEV_ERR, "unknown power for obj creation");

        }
    }

    if ( !vnum ||
         (obj=create_object(vnum)) == NULL )
    {
        log_printf_plus(LOG_MAGIC, LEVEL_LOG_CSET, SEV_ERR, "Bad vnum in creation spell: %d", vnum);
        failed_casting( skill, ch, NULL, NULL );
        return rNONE;
    }

    /*obj->timer = skill->dice ? dice_parse( ch, level, skill->dice ) : 0;*/
    successful_casting( skill, ch, NULL, obj );

    if (SPELL_POWER(skill) != SP_NONE)
    {
        act(AT_MAGIC, "In a flash of light, $p appears.", ch, obj, NULL, TO_ROOM);
        act(AT_MAGIC, "In a flash of light, $p appears.", ch, obj, NULL, TO_CHAR);
        obj_to_room( obj, ch->in_room );
    }
    else
    {
        if ( CAN_WEAR(obj, ITEM_TAKE) )
            obj_to_char( obj, ch );
        else
            obj_to_room( obj, ch->in_room );
    }

    return rNONE;
}

/*
 * Generic mob creating spell					-Thoric
 */
ch_ret spell_create_mob( int sn, int level, CHAR_DATA *ch, void *vo )
{
    SKILLTYPE *skill = get_skilltype(sn);
    int lvl;
    int vnum = skill->value;
    CHAR_DATA *mob = NULL;

    /* set maximum mob level */
    switch( SPELL_POWER(skill) )
    {
    case SP_NONE:	 lvl = 20;	break;
    case SP_MINOR:	 lvl = 5;	break;
    case SP_GREATER:     lvl = level/2; break;
    case SP_MAJOR:
        lvl = level;	break;
    }

    /*
     * Add predetermined mobiles here
     */
    if ( vnum == 0 )
    {
        if ( !str_cmp( target_name, "cityguard" ) )
            vnum = MOB_VNUM_CITYGUARD;
        if ( !str_cmp( target_name, "vampire" ) )
            vnum = MOB_VNUM_VAMPIRE;
    }

    if ((mob=create_mobile(vnum)) == NULL )
    {
        failed_casting( skill, ch, NULL, NULL );
        return rNONE;
    }

    successful_casting( skill, ch, mob, NULL );
    char_to_room( mob, ch->in_room );

    if (too_many_followers(ch))
    {
        act(AT_MAGIC, "$N takes one look at the size of your posse and just says no!", ch, NULL, mob, TO_CHAR);
        act(AT_MAGIC, "$N takes one look at the size of $n's posse and just says no!", ch, NULL, mob, TO_ROOM);
    }
    else
        make_charmie(ch, mob, sn);

    mprog_birth_trigger(ch, mob);

    return rNONE;
}

/*
 * Generic handler for new "SMAUG" spells			-Thoric
 */
ch_ret spell_smaug( int sn, int level, CHAR_DATA *ch, void *vo )
{
    SKILLTYPE *skill = get_skilltype(sn);

    if (IS_SYSTEMFLAG(SYS_NOMAGIC))
        return rSPELL_FAILED;

    switch( skill->target )
    {
    case TAR_IGNORE:
        /* offensive area spell */
        if ( SPELL_FLAG(skill, SF_AREA)
             && ((SPELL_ACTION(skill) == SA_DESTROY
                  &&   SPELL_CLASS(skill) == SC_LIFE)
                 ||  (SPELL_ACTION(skill) == SA_CREATE
                      &&   SPELL_CLASS(skill) == SC_DEATH)))
            return spell_area_attack( sn, level, ch, vo );

        if ( SPELL_ACTION(skill) == SA_CREATE )
        {
            if ( SPELL_FLAG(skill, SF_OBJECT) )	/* create object */
                return spell_create_obj( sn, level, ch,  vo );
            if ( SPELL_CLASS(skill) == SC_LIFE )	/* create mob */
                return spell_create_mob( sn, level, ch,  vo );
        }

        if ( target_name && target_name[0] != '\0' )
        {
            if ( (SPELL_ACTION(skill) == SA_CHANGE) &&
                 (SPELL_CLASS(skill) == SC_SUMMON) &&
                 SPELL_FLAG(skill, SF_DISTANT) &&
                 SPELL_FLAG(skill, SF_CHARACTER) )
                return spell_summon(sn, level, ch, get_char_world(ch, target_name));

            /* affect a distant player */
            if ( SPELL_FLAG(skill, SF_DISTANT)
                 &&   SPELL_FLAG(skill, SF_CHARACTER))
                return spell_affect(sn, level, ch, get_char_world( ch, target_name ));

            if ( get_obj_carry(ch, target_name) && SPELL_FLAG(skill, SF_CHARACTER))
                return spell_obj_inv(sn, level, ch, get_obj_carry(ch, target_name));

            if ( get_obj_wear(ch, target_name) && SPELL_FLAG(skill, SF_CHARACTER))
                return spell_obj_inv(sn, level, ch, get_obj_wear(ch, target_name));

            /* affect a player in this room (should have been TAR_CHAR_XXX) */
            if ( SPELL_FLAG(skill, SF_CHARACTER) )
                return spell_affect(sn, level, ch, get_char_room( ch, target_name ));
        }

        if ( SPELL_FLAG(skill, SF_CHARACTER) )
            return spell_affect(sn, level, ch, ch);

        /* will fail, or be an area/group affect */
        return spell_affect( sn, level, ch, vo );

    case TAR_CHAR_OFFENSIVE:
        /* a regular damage inflicting spell attack */
        if ((SPELL_ACTION(skill) == SA_DESTROY
             &&   SPELL_CLASS(skill) == SC_LIFE)
            ||  (SPELL_ACTION(skill) == SA_CREATE
                 &&   SPELL_CLASS(skill) == SC_DEATH)  )
            return spell_attack( sn, level, ch, vo );

        /* a nasty spell affect */
        return spell_affect( sn, level, ch, vo );

    case TAR_CHAR_DEFENSIVE:
    case TAR_CHAR_SELF:
        if ( vo )
        {
            CHAR_DATA *victim = (CHAR_DATA *) vo;

            if ( SPELL_ACTION(skill) == SA_DESTROY )
            {

                /* cure poison */
                if ( SPELL_DAMAGE(skill) == SD_POISON )
                {
                    if ( is_affected( victim, gsn_poison ) )
                    {
                        affect_strip( victim, gsn_poison );
                        victim->mental_state = URANGE( -100, victim->mental_state, -10 );
                        successful_casting( skill, ch, victim, NULL );
                        return rNONE;
                    }
                    failed_casting( skill, ch, victim, NULL );
                    return rSPELL_FAILED;
                }
                /* cure blindness */
                if ( SPELL_CLASS(skill) == SC_ILLUSION )
                {
                    if ( is_affected( victim, gsn_blindness ) )
                    {
                        affect_strip( victim, gsn_blindness );
                        successful_casting( skill, ch, victim, NULL );
                        return rNONE;
                    }
                    failed_casting( skill, ch, victim, NULL );
                    return rSPELL_FAILED;
                }
            }
            else if ( SPELL_ACTION(skill) == SA_RESIST )
            {
                /* slow poison */
                if ( SPELL_DAMAGE(skill) == SD_POISON )
                {
                    /* NOT DONE */
                    failed_casting( skill, ch, victim, NULL );
                    return rSPELL_FAILED;
                }
            }
            else if ( SPELL_ACTION(skill) == SA_DIVINATE )
            {
                /* detect poison */
                if ( SPELL_DAMAGE(skill) == SD_POISON )
                {
                    /* NOT DONE */
                    failed_casting( skill, ch, victim, NULL );
                    return rSPELL_FAILED;
                }
            }

        }
        return spell_affect( sn, level, ch, vo );

    case TAR_OBJ_INV:
        return spell_obj_inv( sn, level, ch, vo );
    }
    return rNONE;
}



/* Haus' new, new mage spells follow */

/*
 *  4 Energy Spells
 */
ch_ret spell_ethereal_fist( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level       = UMAX(0, level);
    level       = UMIN(35, level);
    dam         = level*number_range( 1, 6 )-31;
    dam         = UMAX(0,dam);

    if ( saves_spell_staff( level, victim ) )
        dam = 0;

    act( AT_MAGIC, "A fist of black, otherworldly ether rams into $N, leaving $M looking stunned!"
         , ch, NULL,
         victim, TO_NOTVICT );
    return damage( ch, victim, dam, sn );
}


ch_ret spell_spectral_furor( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level	= UMAX(0, level);
    level	= UMIN(16, level);
    dam		= level*number_range( 1, 7 )+7;
    if ( saves_spell_staff( level, victim ) )
        dam /= 2;
    act( AT_MAGIC, "The fabric of the cosmos strains in fury about $N!"
         , ch, NULL,
         victim, TO_NOTVICT );
    return damage( ch, victim, dam, sn );
}

ch_ret spell_hand_of_chaos( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level       = UMAX(0, level);
    level       = UMIN(18, level);
    dam         = level*number_range( 1, 7 )+9;

    if ( saves_spell_staff( level, victim ) )
        dam = 0;
    act( AT_MAGIC, "$N is grasped by an incomprehensible hand of chaos!"
         , ch, NULL,
         victim, TO_NOTVICT );
    return damage( ch, victim, dam, sn );
}


ch_ret spell_disruption( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level	= UMAX(0, level);
    level	= UMIN(14, level);
    dam		= level*number_range( 1, 6 )+8;

    if ( saves_spell_staff( level, victim ) )
        dam = 0;
    act( AT_MAGIC, "A weird energy encompasses $N, causing you to question $S continued existence."
         , ch, NULL,
         victim, TO_NOTVICT );
    return damage( ch, victim, dam, sn );
}

ch_ret spell_sonic_resonance( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level       = UMAX(0, level);
    level       = UMIN(23, level);
    dam         = level*number_range( 1, 8 );

    if ( saves_spell_staff( level, victim ) )
        dam = dam*3/4;
    act( AT_MAGIC, "A cylinder of kinetic energy enshrouds $N causing $S to resonate."
         , ch, NULL,
         victim, TO_NOTVICT );
    return damage( ch, victim, dam, sn );
}

/*
 * 3 Mentalstate spells
 */
ch_ret spell_mind_wrack( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    /* decrement mentalstate by up to 50 */

    level	= UMAX(0, level);
    dam		= number_range( 0, 0 );
    if ( saves_spell_staff( level, victim ) )
        dam /= 2;
    act( AT_MAGIC, "$n stares intently at $N, causing $N to seem very lethargic."
         , ch, NULL,
         victim, TO_NOTVICT );
    return damage( ch, victim, dam, sn );
}

ch_ret spell_mind_wrench( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    /* increment mentalstate by up to 50 */

    level	= UMAX(0, level);
    dam		= number_range( 0, 0 );
    if ( saves_spell_staff( level, victim ) )
        dam /= 2;
    act( AT_MAGIC, "$n stares intently at $N, causing $N to seem very hyperactive."
         , ch, NULL,
         victim, TO_NOTVICT );
    return damage( ch, victim, dam, sn );
}


/* Non-offensive spell! */
ch_ret spell_revive( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    /* set mentalstate to mentalstate/2 */
    level	= UMAX(0, level);
    dam		= number_range( 0, 0 );
    if ( saves_spell_staff( level, victim ) )
        dam /= 2;
    act( AT_MAGIC, "$n concentrates intently, and begins looking more centered."
         , ch, NULL,
         victim, TO_NOTVICT );
    return damage( ch, victim, dam, sn );
}

/*
 * n Acid Spells
 */
ch_ret spell_sulfurous_spray( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level       = UMAX(0, level);
    level       = UMIN(19, level);
    dam         = 2*level*number_range( 1, 7 )+11;

    if ( saves_spell_staff( level, victim ) )
        dam /= 4;
    act( AT_MAGIC, "A stinking spray of sulfurous liquid rains down on $N."
         , ch, NULL,
         victim, TO_NOTVICT );
    return damage( ch, victim, dam, sn );
}

ch_ret spell_caustic_fount( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level       = UMAX(0, level);
    level       = UMIN(42, level);
    dam         = 2*level*number_range( 1, 6 )-31;
    dam         = UMAX(0,dam);

    if ( saves_spell_staff( level, victim ) )
        dam = dam*3/4;
    act( AT_MAGIC, "A fountain of caustic liquid forms below $N.  The smell of $S degenerating tissues is revolting! "
         , ch, NULL,
         victim, TO_NOTVICT );
    return damage( ch, victim, dam, sn );
}

ch_ret spell_acetum_primus( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level       = UMAX(0, level);
    dam         = 2*level*number_range( 1, 4 )+7;

    if ( saves_spell_staff( level, victim ) )
        dam = 3*dam/4;
    act( AT_MAGIC, "A cloak of primal acid enshrouds $N, sparks form as it consumes all it touches. "
         , ch, NULL,
         victim, TO_NOTVICT );
    return damage( ch, victim, dam, sn );
}

/*
 *  Electrical
 */

ch_ret spell_galvanic_whip( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level	= UMAX(0, level);
    level	= UMIN(10, level);
    dam		= level*number_range( 1, 6 )+5;

    if ( saves_spell_staff( level, victim ) )
        dam /= 2;
    act( AT_MAGIC, "$n conjures a whip of ionized particles, which lashes ferociously at $N."
         , ch, NULL,
         victim, TO_NOTVICT );
    return damage( ch, victim, dam, sn );
}

ch_ret spell_magnetic_thrust( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level       = UMAX(0, level);
    level       = UMIN(29, level);
    dam         = (5*level*number_range( 1, 6 )) +16;

    if ( saves_spell_staff( level, victim ) )
        dam /= 2;
    act( AT_MAGIC, "An unseen energy moves nearby, causing your hair to stand on end!"
         , ch, NULL,
         victim, TO_NOTVICT );
    return damage( ch, victim, dam, sn );
}

ch_ret spell_quantum_spike( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,l;

    level       = UMAX(0, level);
    l 		= UMAX(1,level-40);
    dam         = l*number_range( 1,40)+145;

    if ( saves_spell_staff( level, victim ) )
        dam /= 2;
    act( AT_MAGIC, "$N seems to dissolve into tiny unconnected particles, then is painfully reassembled."
         , ch, NULL,
         victim, TO_NOTVICT );
    return damage( ch, victim, dam, sn );
}

/*
 * Black-magicish guys
 */

/* L2 Mage Spell */
ch_ret spell_black_hand( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level	= UMAX(0, level);
    level	= UMIN(5, level);
    dam		= level*number_range( 1, 6 )+3;

    if ( saves_poison_death( level, victim ) )
        dam /= 4;
    act( AT_MAGIC, "$n conjures a mystical hand, which swoops menacingly at $N."
         , ch, NULL,
         victim, TO_NOTVICT );
    return damage( ch, victim, dam, sn );
}

ch_ret spell_black_fist( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;

    level       = UMAX(0, level);
    level       = UMIN(30, level);
    dam         = level*number_range( 1, 9 )+4;

    if ( saves_poison_death( level, victim ) )
        dam /= 4;
    act( AT_MAGIC, "$n conjures a mystical fist, which swoops menacingly at $N."
         , ch, NULL,
         victim, TO_NOTVICT );
    return damage( ch, victim, dam, sn );
}

ch_ret spell_black_lightning( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam,l;

    level       = UMAX(0, level);
    l 		= UMAX(1,level-40);
    dam         = l*number_range(1,50)+135;

    if ( saves_poison_death( level, victim ) )
        dam /= 4;
    act( AT_MAGIC, "$n conjures a mystical black thundercloud directly over $N's head."
         , ch, NULL,
         victim, TO_NOTVICT );
    return damage( ch, victim, dam, sn );
}

ch_ret spell_midas_touch( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = NULL;
    int val;
    OBJ_DATA *obj = (OBJ_DATA *) vo;

    separate_obj(obj);  /* nice, alty :) */

    if ( IS_OBJ_STAT( obj, ITEM_NODROP ) )
    {
        send_to_char( "You can't seem to let go of it.\n\r", ch );
        return rSPELL_FAILED;
    }

    if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE )
         &&   get_trust( victim ) < LEVEL_IMMORTAL )
    {
        send_to_char( "That item is not for mortal hands to touch!\n\r", ch );
        return rSPELL_FAILED;   /* Thoric */
    }

    if ( !CAN_WEAR(obj, ITEM_TAKE)
         || ( obj->item_type == ITEM_CORPSE_NPC)
         || ( obj->item_type == ITEM_CORPSE_PC )
       )
    {
        send_to_char( "Your cannot seem to turn this item to gold!", ch);
        return rNONE;
    }

    val = obj->cost/2;
    val = UMAX(0, val);

    if(  obj->item_type==ITEM_WEAPON ){
        switch( number_bits(2) )
        {
        case 0: victim = get_char_world( ch, "shmalnoth");	break;
        case 1:
        case 2:
        case 3: victim = get_char_world( ch, "shmalnak" ); break;
        }
    } else if (  obj->item_type==ITEM_ARMOR ){
        switch( number_bits(2) )
        {
        case 0: victim = get_char_world( ch, "shmalnoth");	break;
        case 1:
        case 2:
        case 3: victim = get_char_world( ch, "crafter" ); break;
        }
    } else if (  obj->item_type==ITEM_SCROLL ){
        victim = get_char_world( ch, "tatorious" );
    } else if (  obj->item_type==ITEM_STAFF ){
        victim = get_char_world( ch, "tatorious" );
    } else if (  obj->item_type==ITEM_WAND ){
        victim = get_char_world( ch, "tatorious" );
    } else {
        victim = NULL;
    }

    if (  victim == NULL )
    {
        GET_MONEY(ch,DEFAULT_CURR) += val;

        if ( obj_extracted(obj) )
            return rNONE;
        extract_obj( obj );
        send_to_char( "Ok.", ch);
        return rNONE;
    }


    if ( ( carry_w(victim) + get_obj_weight ( obj ) ) > can_carry_w(victim)
         ||	(IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE)))
    {
        GET_MONEY(ch,DEFAULT_CURR) += val;

        if ( obj_extracted(obj) )
            return rNONE;
        extract_obj( obj );
        send_to_char( "Ok.", ch);
        return rNONE;
    }


    GET_MONEY(ch,DEFAULT_CURR) += val;
    obj_from_char( obj );
    obj_to_char( obj, victim );

    send_to_char( "You transmogrify the item to gold!\n\r", ch );
    return rNONE;
}
