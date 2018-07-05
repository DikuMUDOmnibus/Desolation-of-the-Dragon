/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "mud.h"
#include "justify.h"

/*
 MUD Internal Markup Language

 MIML Grammar: #keyword#arg1,arg2,argx#display if true#display if false#

 Valid keywords: aff, aff2, act, act2, plr, plr2, objroom, mobroom,
 drunk, mentalstate, spell, variable

 MIML is a system whereby the MUD can customize output to a player based on
 any number of things.  Room descriptions can, for example, be customized
 to show a player with true sight some secret, or if in the lair of a beast
 the beast is dead, the room can reflect that.

 MIML is a work in progress, please keep that in mind.

 Example:
 You are #aff#flying#flying above#standing in# an alley, between
 two buildings.  #aff#flying#Looking down, you see many piles of refuse.##
 The alley is quite dark, and you #aff#truesight#can see many forms scurrying
 about doing business in the underground#have trouble making out the path#.

 Notes: The #keyword#arg1,arg2,argx# portion should be on one line, otherwise
 the MIML interpreter will not recognize them because they will contain CRLF's.
 */

#define MIML_DELIMETER        '#'

typedef enum
{
    MIML_KEYW_INVALID, MIML_KEYW_AFF, MIML_KEYW_AFF2, MIML_KEYW_ACT,
    MIML_KEYW_ACT2, MIML_KEYW_PLR, MIML_KEYW_PLR2,
    MIML_KEYW_OBJROOM, MIML_KEYW_MOBROOM, MIML_KEYW_DRUNK,
    MIML_KEYW_MENTALSTATE, MIML_KEYW_SPELL, MIML_KEYW_VARIABLE,
    MIML_MAX_KEYWORDS
} miml_keyword_type;

char * const miml_keywords[MIML_MAX_KEYWORDS] =
{
    "invalid", "aff", "aff2", "act", "act2", "plr", "plr2",
    "objroom", "mobroom", "drunk", "mentalstate", "spell",
    "variable"
};

bool	mprog_seval		args( ( char* lhs, char* opr, char* rhs,
                                        CHAR_DATA *mob ) );
bool	mprog_veval		args( ( int lhs, char* opr, int rhs,
                                        CHAR_DATA *mob ) );

CHAR_DATA *miml_ch;

/* same as one_argumentx, but without the LOWER and size increased*/
char *miml_one_argumentx( char *argument, char *arg_first, char cEnd )
{
    sh_int count;

    count = 0;

    while ( isspace(*argument) )
	argument++;

    while ( *argument != '\0' || ++count >= MAX_INPUT_LENGTH )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first = *argument;
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

/* returns miml keyword type for a given str */
sh_int miml_keyword(char *str)
{
    char keyw[MAX_INPUT_LENGTH];
    int x;

    miml_one_argumentx(str, keyw, MIML_DELIMETER);

    if (keyw[0] == '\0')
        return MIML_KEYW_INVALID;

    for (x = 0; x < MIML_MAX_KEYWORDS; x++)
        if (!str_cmp(keyw, miml_keywords[x]))
            return x;

    return MIML_KEYW_INVALID;
}

/* the is the guts */
bool miml_check(sh_int keyword, char *args)
{
    int x, y;

    switch (keyword)
    {
    case MIML_KEYW_INVALID:
    case MIML_MAX_KEYWORDS:
        bug("miml_check: invalid keyword: %d", keyword);
        return FALSE;
        break;

    case MIML_KEYW_AFF:
        if ((x = get_aflag(args)) != -1)
            if (IS_AFFECTED(miml_ch, 1<<x))
                return TRUE;
        break;

    case MIML_KEYW_AFF2:
        if ((x = get_a2flag(args)) != -1)
            if (IS_AFFECTED2(miml_ch, 1<<x))
                return TRUE;
        break;

    case MIML_KEYW_ACT:
        if (IS_NPC(miml_ch) && (x = get_actflag(args)) != -1)
            if (IS_ACT_FLAG(miml_ch, 1<<x))
                return TRUE;
        break;

    case MIML_KEYW_ACT2:
        if (IS_NPC(miml_ch) && (x = get_act2flag(args)) != -1)
            if (IS_ACT2_FLAG(miml_ch, 1<<x))
                return TRUE;
        break;

    case MIML_KEYW_PLR:
        if (!IS_NPC(miml_ch) && (x = get_plrflag(args)) != -1)
            if (IS_PLR_FLAG(miml_ch, 1<<x))
                return TRUE;
        break;

    case MIML_KEYW_PLR2:
        if (!IS_NPC(miml_ch) && (x = get_plr2flag(args)) != -1)
            if (IS_PLR2_FLAG(miml_ch, 1<<x))
                return TRUE;
        break;

    case MIML_KEYW_OBJROOM:
        if (get_obj_list_rev(miml_ch, args, miml_ch->in_room->last_content))
            return TRUE;
        break;

    case MIML_KEYW_MOBROOM:
        if (get_char_room( miml_ch, args))
            return TRUE;
        break;

    case MIML_KEYW_DRUNK:
        x = atoi(args);
        if (GET_COND(miml_ch, COND_DRUNK) > x)
            return TRUE;
        break;

    case MIML_KEYW_MENTALSTATE:
        x = atoi(args);
        if (x < 0 && miml_ch->mental_state < x)
            return TRUE;
        if (x > 0 && miml_ch->mental_state > x)
            return TRUE;
        break;

    case MIML_KEYW_SPELL:
        x = skill_lookup(args);
        if (!IS_VALID_SN(x))
            x = atoi(args);
        if (!IS_VALID_SN(x))
            return FALSE;
        if (is_affected(miml_ch, x))
            return TRUE;
        break;

    case MIML_KEYW_VARIABLE:
        {
            char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
            char *cval = NULL;

            args = one_argument(args, arg1);
            args = one_argument(args, arg2);

            if (arg2[0] == '\0')
            {
                if (get_var(miml_ch->vars, arg1))
                    return TRUE;
                return FALSE;
            }

            if (arg1[0] == '#')
            {
                x = get_var_val_int_ch(miml_ch, arg1);
                y = dice_parse(miml_ch, GetMaxLevel(miml_ch), args);
                return mprog_veval(x, arg2, y, miml_ch);
            }

            if (!(cval = get_var_val_ch(miml_ch, arg1)))
                return FALSE;

            return mprog_seval(cval, arg2, args, miml_ch);

            break;
        }
    }

    return FALSE;
}

/* chews the miml code into bits, checking for sanity */
int miml_parse(char *str, char *outstr)
{
    char args[MAX_INPUT_LENGTH];
    char strtrue[MAX_INPUT_LENGTH];
    char strfalse[MAX_INPUT_LENGTH];
    sh_int keyword;
    int x, delims=0;

    outstr[0] = '\0';

    /* snarf keyword */
    keyword = miml_keyword(str+1);
    if (keyword == MIML_KEYW_INVALID)
        return 0;

    /* x will be the total length of the miml code, that we need to return */
    for (x = 0; x < strlen(str) && delims<5; x++)
        if (str[x] == MIML_DELIMETER)
            delims++;
    /* short circuit right now, if we don't have enough delimeters */
    if (delims<5)
        return 0;

    /* skip keyword */
    str = miml_one_argumentx(str+1, args, MIML_DELIMETER);
    if (str[0] == '\0')
        return 0;

    /* grab args for keyword */
    str = miml_one_argumentx(str, args, MIML_DELIMETER);
    if (str[0] == '\0')
        return 0;

    /* grab return str if cond is true */
    str = miml_one_argumentx(str, strtrue, MIML_DELIMETER);
    if (str[0] == '\0')
        return 0;

    /* grab return str if cond is false */
    str = miml_one_argumentx(str, strfalse, MIML_DELIMETER);

    if (miml_check(keyword, args))
        strcpy(outstr, strtrue);
    else
        strcpy(outstr, strfalse);

    return x-1;
}

/* feed me a string with miml codes, i'll feed you one all nice and parsed */
char *miml_process(char *str)
{
    static char outbuf[MAX_STRING_LENGTH];
    char tstr[MAX_INPUT_LENGTH];
    unsigned int x;
    int y=0, z;

    outbuf[0] = '\0';

    for (x = 0; x < strlen(str); x++)
    {
        if (str[x] == MIML_DELIMETER &&
            (z = miml_parse(str+x, tstr)) > 0)
        {
            x += z;
            y += strlen(tstr);
            strcat(outbuf, tstr);
            outbuf[y] = '\0';
            continue;
        }
        outbuf[y++] = str[x];
        outbuf[y] = '\0';
    }

    return outbuf;
}

void miml_to_char(char *str, CHAR_DATA *ch)
{
    char *mbuf;
    char *jbuf;

    miml_ch = ch;

    mbuf = miml_process(str);
    if (str_cmp(mbuf, str))
    {
        jbuf = Justify(mbuf, 75, justify_left);
        send_to_char(jbuf, ch);
        send_to_char("\n\r", ch);
    }
    else
        send_to_char(str, ch);

    miml_ch = NULL;
}
