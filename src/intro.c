/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2001  Jesse DeFer and Heath Leach
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*static char rcsid[] = "$Id: intro.c,v 1.18 2004/03/31 02:19:53 dotd Exp $";*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "mud.h"

void free_intros(CHAR_DATA *ch)
{
    INTRO_DATA *intro;

    while ((intro = ch->first_intro))
    {
        UNLINK(intro, ch->first_intro, ch->last_intro, next, prev);
        free_intro(intro);
    }
}

void free_intro(INTRO_DATA *intro)
{
    if (intro->real_name)
        STRFREE(intro->real_name);
    if (intro->intro_name)
        STRFREE(intro->intro_name);
    if (intro->recog_name)
        STRFREE(intro->recog_name);
    DISPOSE(intro);
}

void fwrite_introductions(CHAR_DATA *ch, FILE *fp)
{
    INTRO_DATA *intro;

    if (!ch->first_intro)
        return;

    fprintf(fp, "#INTRODUCTIONS\n");

    for (intro = ch->first_intro; intro; intro = intro->next)
        fprintf(fp, "%ld '%s' '%s' '%s'\n",
                (long)intro->when,
                intro->real_name,
                intro->intro_name?intro->intro_name:"",
                intro->recog_name?intro->recog_name:"");

    fprintf(fp, "End\n\n");
}

INTRO_DATA *new_intro(CHAR_DATA *ch, const char *real_name)
{
    INTRO_DATA *intro;

    CREATE(intro, INTRO_DATA, 1);

    intro->real_name = STRALLOC(real_name);

    LINK(intro, ch->first_intro, ch->last_intro, next, prev);

    return intro;
}

void update_intro(INTRO_DATA *intro, char *new_intro_name)
{
    if (!intro || !new_intro_name)
        return;

    if (intro->intro_name)
        STRFREE(intro->intro_name);
    if (*new_intro_name)
        intro->intro_name = STRALLOC(new_intro_name);
}

void update_recog(INTRO_DATA *intro, char *new_recog_name)
{
    if (!intro || !new_recog_name)
        return;

    if (intro->recog_name)
        STRFREE(intro->recog_name);
    if (*new_recog_name)
        intro->recog_name = STRALLOC(new_recog_name);
}

void fread_introductions(CHAR_DATA *ch, FILE *fp)
{
    INTRO_DATA *intro = NULL;
    const char *word;
    char *str;
    bool fMatch;

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
        case 'E':
            if ( !str_cmp( word, "End" ) )
                return;
            break;

        default:
            intro = new_intro(ch, word);
            intro->when = fread_number( fp );
            str = fread_word( fp );
            update_intro(intro, str);
            str = fread_word( fp );
            update_recog(intro, str);
            break;
        }
    }
}

INTRO_DATA *get_intro(CHAR_DATA *ch, char *who)
{
    INTRO_DATA *intro;

    if (!ch || !who)
        return(NULL);

    for (intro = ch->first_intro; intro; intro = intro->next)
        if (!str_cmp(intro->real_name, who))
            return(intro);

    return(NULL);
}

char *PERS(CHAR_DATA *ch, CHAR_DATA *looker)
{
    INTRO_DATA *intro;

    if (!can_see(looker, ch))
        return("someone");

    if (IS_NPC(ch))
        return(ch->short_descr);

    if (sysdata.intro_disabled || ch == looker || !looker || IS_IMMORTAL(looker))
        return(GET_NAME(ch));

    if (!ch->intro_descr || ch->intro_descr[0] == '\0')
        return(GET_NAME(ch));

    if (!(intro = get_intro(looker, GET_NAME(ch))))
        return(ch->intro_descr);

    if (intro->recog_name)
        return(intro->recog_name);

    if (intro->intro_name)
        return(intro->intro_name);

    bug("PERS: no intro or recog names, ch: %s, looker: %s",
	GET_NAME(ch), looker?GET_NAME(looker):"Null");
    return("someone");
}

char *PERSLONG(CHAR_DATA *ch, CHAR_DATA *looker)
{
    static char buf[MAX_INPUT_LENGTH];
    INTRO_DATA *intro;

    if (!can_see(looker, ch))
        return("someone");

    if (IS_NPC(ch))
        return(ch->long_descr);

    sprintf(buf, "%s%s", GET_NAME(ch), GET_TITLE(ch));

    if (sysdata.intro_disabled || ch == looker || !looker || IS_IMMORTAL(looker))
        return(buf);

    if (!ch->intro_descr || ch->intro_descr[0] == '\0')
        return(buf);

    if (!(intro = get_intro(looker, GET_NAME(ch))))
        return(ch->intro_descr);

    if (intro->recog_name)
    {
        sprintf(buf, "%s%s", intro->recog_name, GET_TITLE(ch));
        return(buf);
    }

    if (intro->intro_name)
    {
        sprintf(buf, "%s%s", intro->intro_name, GET_TITLE(ch));
        return(buf);
    }

    bug("PERSLONG: no intro or recog names, ch: %s, looker: %s",
        GET_NAME(ch), GET_NAME(looker));
    return("someone");
}

void do_introduce( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    INTRO_DATA *intro;
    char *intro_name;

    argument = one_argument(argument, arg);

    if ( arg[0] == '\0' )
    {
        send_to_char( "Introduce yourself to whom?\n\r", ch );
        return;
    }

    if ( IS_IMMORTAL(ch) && !str_cmp(arg, "list") )
    {
        if (!(victim = get_char_world(ch, argument)))
            victim = ch;

        ch_printf(ch, "%-16.16s %-30.30s %-30.30s\n\r",
                  "[Real Name]", "[Intro Name]", "[Recog Name]");
        for (intro = victim->first_intro; intro; intro = intro->next)
            ch_printf(ch, "%-16.16s %-30.30s %-30.30s\n\r",
                      intro->real_name,
                      intro->intro_name,
                      intro->recog_name);
        return;
    }

    if ( !str_cmp(arg, "setintro") && argument[0] != '\0' )
    {
        if ( GetMaxLevel(ch)>1 && !IS_IMMORTAL(ch) &&
             ch->intro_descr && *ch->intro_descr )
        {
            send_to_char("You cannot set your intro.\n\r", ch);
            return;
        }
        if (strlen(argument) > 32)
        {
            send_to_char("That description is too long.\n\r", ch);
            return;
        }
        if (ch->intro_descr)
            DISPOSE(ch->intro_descr);
        ch->intro_descr = str_dup(argument);
        ch_printf(ch, "Your intro is now: '%s'\n\r", ch->intro_descr);
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "You can't seem to find them.\n\r", ch );
        return;
    }

    if ( ch == victim )
    {
        send_to_char("You already know yourself.\n\r", ch);
        return;
    }

    if ( (intro = get_intro(victim, GET_NAME(ch))) )
    {
        send_to_char("You have already introduced yourself to them.\n\r", ch);
        return;
    }

    if (argument && argument[0] != '\0' && IS_IMMORTAL(ch))
    {
        if (strlen(argument) > 32)
        {
            send_to_char("That name is too long.\n\r", ch);
            return;
        }
        intro_name = argument;
    }
    else
        intro_name = GET_NAME(ch);

    act(AT_PLAIN, "$n introduces $mself to you as $t.", ch, intro_name, victim, TO_VICT);
    act(AT_PLAIN, "You introduce yourself to $N as $t.", ch, intro_name, victim, TO_CHAR);

    intro = new_intro(victim, (const char *)GET_NAME(ch));
    update_intro(intro, intro_name);
}

void do_recognize(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    INTRO_DATA *intro;
    char *recog_name;

    argument = one_argument(argument, arg);

    if ( arg[0] == '\0' )
    {
        send_to_char( "Recognize whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "You can't seem to find them.\n\r", ch );
        return;
    }

    if ( ch == victim )
    {
        send_to_char("Epiphany!  You recognize your true self ", ch);
        if (IsGoodSide(ch))
            send_to_char("as a whiney do-gooder!", ch);
        else if (IsBadSide(ch))
            send_to_char("as a pathetic low-life!", ch);
        else
            send_to_char("as an unreliable oaf!", ch);
        send_to_char("\n\rBut that's besides the point, try recognizing somebody else.\n\r", ch);
        return;
    }

    if (!(intro = get_intro(victim, GET_NAME(ch))))
        intro = new_intro(victim, GET_NAME(ch));

    if (argument && argument[0] != '\0')
    {
        if (strlen(argument) > 32)
        {
            send_to_char("That name is too long.\n\r", ch);
            return;
        }
        recog_name = argument;
    }
    else if (intro && intro->intro_name)
        recog_name = intro->intro_name;
    else
    {
        send_to_char("Once you recognize somebody, you can't forget them on purpose.\n\r", ch);
        return;
    }

    act(AT_PLAIN, "You recognize $N as $t.", ch, recog_name, victim, TO_CHAR);

    update_recog(intro, recog_name);
}
