/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "mud.h"
#include "fcompress.h"
#include "justify.h"
#include "bugtrack.h"

#ifdef KEY
#undef KEY
#endif

#define KEY( literal, field, value )                                    \
                                if ( !str_cmp( word, literal ) )        \
                                {                                       \
                                    field  = value;                     \
                                    fMatch = TRUE;                      \
                                    break;                              \
                                }

DECLARE_DO_FUN(do_idea);
DECLARE_DO_FUN(do_typo);

BUGTRACK_DATA *first_bugtrack;
BUGTRACK_DATA *last_bugtrack;

const char *bugtrack_status_str[BUGTRACK_MAX_STATUS] =
{
    "Bug", "Fixed", "Not a Bug", "Duplicate", "Works For Me", "Assigned",
    "Feature Request", "Idea", "Typo", "Rejected"
};
const char *bugtrack_status_str_short[BUGTRACK_MAX_STATUS] =
{
    "BUG", "FIX", "NOT", "DUP", "WFM", "ASS", "FEA", "IDE", "TYP", "REJ"
};

void free_bugtrack(BUGTRACK_DATA *bt)
{
    if (bt->submitter)
        STRFREE(bt->submitter);
    if (bt->owner)
        STRFREE(bt->owner);
    if (bt->bugstr)
        DISPOSE(bt->bugstr);
    if (bt->text)
        DISPOSE(bt->text);

    UNLINK(bt, first_bugtrack, last_bugtrack, next, prev);
    DISPOSE(bt);
}

void free_bugtracks(void)
{
    BUGTRACK_DATA *bt;

    while ((bt = first_bugtrack))
        free_bugtrack(bt);
}

void fread_bugtrack(BUGTRACK_DATA *bt, gzFile gzfp)
{
    char linebuf[MAX_STRING_LENGTH];
    char wordbuf[MAX_INPUT_LENGTH];
    const char *word = NULL;
    char *line;
    bool fMatch = FALSE;

    for ( ; ; )
    {
        word   = gzeof( gzfp ) ? "End" : gz_fread_word( gzfp, wordbuf );
        fMatch = FALSE;

        switch ( UPPER(word[0]) )
        {
        case '*':
            fMatch = TRUE;
            gz_fread_to_eol( gzfp );
            break;
        case 'E':
            if ( !str_cmp( word, "End" ) )
                return;
            break;
        case 'B':
            KEY( "Bug",          bt->bugstr,         gz_fread_string_nohash( gzfp ) );
            break;
        case 'I':
            KEY( "Id",           bt->id,             gz_fread_number( gzfp ) );
            break;
        case 'O':
            KEY( "Owner",        bt->owner,          gz_fread_string( gzfp ) );
            break;
        case 'S':
            KEY( "Status",       bt->status,         gz_fread_number( gzfp ) );
            KEY( "Submitter",    bt->submitter,      gz_fread_string( gzfp ) );
            break;
        case 'T':
            KEY( "Text",         bt->text,           gz_fread_string_nohash( gzfp ) );
            if ( !str_cmp( word, "Times" ) )
            {
                line = gz_fread_line(gzfp, linebuf);
                sscanf( line, "%ld %ld %ld",
                        &bt->opened, &bt->closed, &bt->updated );
                fMatch = TRUE;
                break;
            }
            break;
        }

        if ( !fMatch )
        {
            bug( "fread_bugtrack: no match: %s", word );
            gz_fread_to_eol( gzfp );
        }

    }
}

void load_bugtracks(void)
{
    char wordbuf[MAX_INPUT_LENGTH];
    gzFile gzfp;
    BUGTRACK_DATA *bt;
    char letter, *word;
    int bugs = 0;

    first_bugtrack = NULL;
    last_bugtrack = NULL;

    if (!(gzfp = gzopen(BUGTRACK_FILE, "r")))
        return;

    while (!gzeof(gzfp))
    {
        letter = gz_fread_letter( gzfp );

        if (gzeof(gzfp))
            break;

        if ( letter == '$' )
            break;

        if ( letter == '*' )
        {
            gz_fread_to_eol( gzfp );
            continue;
        }

        if ( letter != '#' )
        {
            bug( "load_bugtracks: # not found (%c(%d) instead)", letter, letter );
            break;
        }

        word = gz_fread_word( gzfp, wordbuf );
        if ( !str_cmp( word, "BUG" ) )
        {
            CREATE(bt, BUGTRACK_DATA, 1);
            fread_bugtrack(bt, gzfp);
            bugs++;
            LINK(bt, first_bugtrack, last_bugtrack, next, prev);
            continue;
        }

        bug( "load_bugtracks: bad section %s", word );
        break;
    }

    gzclose(gzfp);
}

void fwrite_bugtrack(gzFile gzfp, BUGTRACK_DATA *bt)
{
    gzprintf(gzfp, "#BUG\n");
    gzprintf(gzfp, "Id          %d\n",
            bt->id);
    gzprintf(gzfp, "Status      %d\n",
            bt->status);
    gzprintf(gzfp, "Times       %ld %ld %ld\n",
            bt->opened,
            bt->closed,
            bt->updated);
    if (bt->submitter)
        gzprintf(gzfp, "Submitter   %s~\n",
                bt->submitter);
    if (bt->owner)
        gzprintf(gzfp, "Owner       %s~\n",
                bt->owner);
    if (bt->bugstr)
        gzprintf(gzfp, "Bug         %s~\n",
                bt->bugstr);
    if (bt->text)
        gzprintf(gzfp, "Text        %s~\n",
                strip_cr(bt->text));
    gzprintf(gzfp, "End\n");
}

void save_bugtracks(void)
{
    gzFile gzfp;
    BUGTRACK_DATA *bt;

    if (!(gzfp = gzopen(BUGTRACK_FILE, "w")))
    {
        bug("Unable to write bugtrack data to " BUGTRACK_FILE);
        return;
    }

    for (bt = first_bugtrack; bt; bt = bt->next)
        fwrite_bugtrack(gzfp, bt);
    gzprintf(gzfp, "$\n");

    gzclose(gzfp);
}

BUGTRACK_DATA *find_bugtrack(int id)
{
    BUGTRACK_DATA *bt;

    for (bt = first_bugtrack; bt; bt = bt->next)
        if (bt->id == id)
            return bt;

    return NULL;
}

sh_int find_bugtrack_status(char *status)
{
    sh_int st;

    for (st = BUGTRACK_FIRST_STATUS; st < BUGTRACK_MAX_STATUS; st++)
        if (!str_cmp(status, bugtrack_status_str[st]))
            return st;

    return BUGTRACK_INVALID_STATUS;
}

void bugtrack_add_text(BUGTRACK_DATA *bt, char *fmt, ...)
{
    char tbuf[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    va_list param;

    va_start(param, fmt);
    vsprintf(tbuf, fmt, param);
    va_end(param);

    if (bt->text)
        snprintf(buf, MAX_STRING_LENGTH-1, "%s-----------------------------------\n\r%s\n\r",
                 bt->text, tbuf);
    else
        snprintf(buf, MAX_STRING_LENGTH-1, "%s\n\r",
                 tbuf);

    if (bt->text)
        DISPOSE(bt->text);
    bt->text = str_dup(buf);
}

void bugtrack_check(CHAR_DATA *ch)
{
    BUGTRACK_DATA *bt;
    int submit = 0, own = 0, unass = 0, feature = 0;

    for (bt = first_bugtrack; bt; bt = bt->next)
    {
        if (bt->status == BUGTRACK_BUG ||
            bt->status == BUGTRACK_TYPO ||
            bt->status == BUGTRACK_IDEA)
        {
            unass++;
            if (bt->submitter && !str_cmp(bt->submitter, GET_NAME(ch)))
                submit++;
        }

        if (bt->status == BUGTRACK_ASSIGNED)
        {
            if (bt->owner && !str_cmp(bt->owner, GET_NAME(ch)))
                own++;
        }

        if (bt->status == BUGTRACK_FEATURE_REQUEST)
        {
            feature++;
        }
    }

    set_char_color(AT_GREEN, ch);
    if (feature)
        ch_printf(ch, "There are %d feature requests pending.\n\r", feature);
    if (submit)
        ch_printf(ch, "There are %d bugs submitted by you that are still unprocessed.\n\r", submit);
    if (own)
        ch_printf(ch, "There are %d open bugs assigned to you.\n\r", own);

    if (unass && IS_IMMORTAL(ch))
    {
        set_char_color(AT_RED, ch);
        ch_printf(ch, "There are %d open bugs that are unassigned, please check the queue.\n\r", unass);
    }
}

char *bugtrack_list_dupes(BUGTRACK_DATA *bt)
{
    BUGTRACK_DATA *temp_bt;
    char dbuf[MAX_STRING_LENGTH];
    static char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';
    sprintf(dbuf, "Duplicate of BugID %d", bt->id);

    for (temp_bt = first_bugtrack; temp_bt; temp_bt = temp_bt->next)
	if (strstr(temp_bt->text, dbuf))
            sprintf(buf+strlen(buf), "%d ", temp_bt->id);

    return buf;
}

void do_bugtrack(CHAR_DATA *ch, char *argument)
{
    BUGTRACK_DATA *bt = NULL;

    if (IS_NPC(ch))
    {
        bug("NPC using do_bugtrack: %s, %s",
            GET_NAME(ch), argument?argument:"(null)");
        send_to_char("NPC's can't use this.\n\r", ch);
        return;
    }

    if (!argument || !*argument)
    {
        if (!IS_IMMORTAL(ch))
        {
            send_to_char("Bug what?\n\r", ch);
            return;
        }
        send_to_char("Usage:\n\r"
                     "  bug list [all, mine, or any status]\n\r"
                     "  bug <BugID>\n\r"
                     "  bug file <new bug>\n\r"
                     "  bug <action> <BugID> [extra args]\n\r"
                     "Bug actions: take, fixed, notabug, worksforme, dupe, update\n\r",
                     ch);
        return;
    }

    if (IS_IMMORTAL(ch))
    {
        char arg[MAX_INPUT_LENGTH];
        char arg2[MAX_INPUT_LENGTH];
        char *argp;

        if (is_number(argument) && (bt = find_bugtrack(atoi(argument))))
        {
            ch_printf(ch,
		      "BugID:     %d\n\r"
		      "Dupes:     %s\n\r"
                      "Status:    %s\n\r"
                      "Owner:     %s\n\r"
                      "Submitter: %s\n\r"
                      "Opened:    %24.24s\n\r"
                      "Bug: %s\n\r"
                      "Text:\n\r%s\n\r",
		      bt->id,
		      bugtrack_list_dupes(bt),
                      bugtrack_status_str[bt->status],
                      bt->owner?bt->owner:"Nobody",
                      bt->submitter?bt->submitter:"Nobody",
                      ctime(&bt->opened),
                      bt->bugstr?bt->bugstr:"(null)",
                      bt->text?bt->text:"None"
                     );
            if (bt->updated)
                ch_printf(ch, "Updated:   %24.24s\n\r",
                          ctime(&bt->updated));
            if (bt->closed)
                ch_printf(ch, "Closed:    %24.24s\n\r",
                          ctime(&bt->closed));
            return;
        }

        argument = one_argument(argument, arg);

        if (!str_prefix(arg, "list"))
        {
            int count = 0, total = 0;

            for (bt = first_bugtrack; bt; bt = bt->next)
            {
                if (((!argument || !*argument) && BT_IS_OPEN(bt)) ||
                    (bt->owner && BT_IS_OPEN(bt) &&
                     !str_cmp(argument, "mine") && !str_cmp(bt->owner, GET_NAME(ch))) ||
                    bt->status == find_bugtrack_status(argument) ||
                    !str_cmp(argument, "all"))
                {
                    ch_printf(ch, "%s %5d: %-68.68s\n\r",
                              bugtrack_status_str_short[bt->status],
                              bt->id,
                              bt->bugstr?strip_crlf(bt->bugstr):"(null)");
                    count++;
                }
                total++;
            }
            ch_printf(ch, "%d/%d bugs listed.\n\r", count, total);
            return;
        }

        argp = argument;
        argp = one_argument(argp, arg2);

        if (arg[0] == '\0' || arg2[0] == '\0')
        {
            do_bugtrack(ch, NULL);
            return;
        }

        if (str_cmp(arg, "file") &&
            !(bt = find_bugtrack(atoi(arg2))))
        {
            send_to_char("Can't find that BugID.\n\r", ch);
            return;
        }

        if (!str_prefix(arg, "take"))
        {
            if (bt->owner)
                STRFREE(bt->owner);

            bt->owner = STRALLOC(GET_NAME(ch));
            bt->status = BUGTRACK_ASSIGNED;
            bugtrack_add_text(bt, "Bug assigned to: %s",
                              GET_NAME(ch));
            save_bugtracks();

            send_to_char("Ok.\n\r", ch);
            return;
        }
        if (!str_prefix(arg, "fixed"))
        {
            bt->status = BUGTRACK_FIXED;
            bt->closed = current_time;
            bugtrack_add_text(bt, "Bug fixed -- %s",
                              GET_NAME(ch));
            save_bugtracks();

            send_to_char("Ok.\n\r", ch);
            return;
        }
        if (!str_prefix(arg, "notabug"))
        {
            bt->status = BUGTRACK_NOTABUG;
            bt->closed = current_time;
            bugtrack_add_text(bt, "Not a bug -- %s", GET_NAME(ch));
            save_bugtracks();

            send_to_char("Ok.\n\r", ch);
            return;
        }
        if (!str_prefix(arg, "worksforme"))
        {
            bt->status = BUGTRACK_WORKSFORME;
            bt->closed = current_time;
            bugtrack_add_text(bt, "Works for me -- %s", GET_NAME(ch));
            save_bugtracks();

            send_to_char("Ok.\n\r", ch);
            return;
        }
        if (!str_prefix(arg, "dupe"))
        {
            int id;

            id = atoi(argp);

            if (!(find_bugtrack(id)))
            {
                send_to_char("Dupe needs a BugID as comment.\n\r", ch);
                return;
            }

            bt->status = BUGTRACK_DUPLICATE;
            bt->closed = current_time;
            bugtrack_add_text(bt, "Duplicate of BugID %d -- %s",
                              id, GET_NAME(ch));
            save_bugtracks();

            send_to_char("Ok.\n\r", ch);
            return;
        }

        if (!str_prefix(arg, "update"))
        {
            if (!argp || !*argp)
            {
                send_to_char("What do you want to put in the update?\n\r", ch);
                return;
            }

            bt->updated = current_time;
            bugtrack_add_text(bt, "Update by %s:\n\r%s",
                              GET_NAME(ch),
                              Justify(argp, 75, justify_left));
            save_bugtracks();

            send_to_char("Ok.\n\r", ch);
            return;
        }

        if (str_cmp(arg, "file"))
        {
            send_to_char("If you meant to file a bug, use bug file and minimum two words in comment.\n\r", ch);
            do_bugtrack(ch, NULL);
            return;
        }
    }

    log_printf_plus(LOG_BUG, LEVEL_IMMORTAL, SEV_NOTICE, "%s: %s", GET_NAME(ch), argument);

    CREATE(bt, BUGTRACK_DATA, 1);

    if (last_bugtrack)
        bt->id    = last_bugtrack->id+1; /* just make sure it's always sorted */
    else
        bt->id    = 1;

    if (ch->last_cmd == do_idea)
        bt->status    = BUGTRACK_IDEA;
    else if (ch->last_cmd == do_typo)
        bt->status    = BUGTRACK_TYPO;
    else
        bt->status    = BUGTRACK_BUG;

    bt->submitter = STRALLOC(GET_NAME(ch));
    bt->owner     = NULL;
    bt->bugstr    = str_dup(Justify(argument, 70, justify_left));
    bugtrack_add_text(bt, "Room: %d - %s",
                      ch->in_room->vnum, ch->in_room->name);
    bt->opened    = current_time;
    bt->closed    = 0;
    bt->updated   = 0;

    LINK(bt, first_bugtrack, last_bugtrack, next, prev);

    if (ch->last_cmd == do_idea)
        ch_printf(ch, "Idea noted, BugID is %d.  Thanks!\n\r", bt->id);
    else if (ch->last_cmd == do_typo)
        ch_printf(ch, "Typo noted, BugID is %d.  Thanks!\n\r", bt->id);
    else
        ch_printf(ch, "Bug created, BugID is %d.  Thanks!\n\r", bt->id);
    save_bugtracks();
}
