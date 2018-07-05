/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "mud.h"
#include "quest.h"


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
QUEST_DATA *first_quest;
QUEST_DATA *last_quest;
sh_int quests_active;

char *	const	quest_flags [32] =
{
    "active", "inorder", "autoreset", "allownpc", "onlyonce",
    "q6", "q7", "q8", "q9", "q10",
    "q11", "q12", "q13", "q14", "q15",
    "q16", "q17", "q18", "q19", "q20",
    "q21", "q22", "q23", "q24", "q25",
    "q26", "q27", "q28", "q29", "q30",
    "q31", "q32"
};

char *	const	task_flags [32] =
{
    "optional", "r2", "r3", "q4", "q5",
    "q6", "q7", "q8", "q9", "q10",
    "q11", "q12", "q13", "q14", "q15",
    "q16", "q17", "q18", "q19", "q20",
    "q21", "q22", "q23", "q24", "q25",
    "q26", "q27", "q28", "q29", "q30",
    "q31", "q32"
};

char *  const   quest_task_type_names [MAX_TASK_TYPE] =
{
    "mobfind", "objfind", "roomfind", "mobkill", "objgive", "objplace"
};

void free_quest_task(QUEST_DATA *quest, QUEST_TASK *task)
{
    if (task->completed_by)
        STRFREE(task->completed_by);
    if (task->description)
        DISPOSE(task->description);

    UNLINK(task, quest->first_task, quest->last_task, next, prev);
    DISPOSE(task);

    if (IS_QUEST_FLAG(quest, QUEST_ACTIVE) && !quest->first_task)
    {
        REMOVE_QUEST_FLAG(quest, QUEST_ACTIVE);
        quests_active--;
    }
}

void free_quest(QUEST_DATA *quest)
{
    QUEST_TASK *task;

    if (quest->name)
        STRFREE(quest->name);
    if (quest->owner)
        STRFREE(quest->owner);
    if (quest->creator)
        STRFREE(quest->creator);
    if (quest->description)
        DISPOSE(quest->description);
    if (quest->completed_by)
        STRFREE(quest->completed_by);

    while ((task = quest->first_task))
        free_quest_task(quest, task);

    if (IS_QUEST_FLAG(quest, QUEST_ACTIVE))
        quests_active--;

    UNLINK(quest, first_quest, last_quest, next, prev);
    DISPOSE(quest);
}

void free_quests(void)
{
    QUEST_DATA *quest;

    while ((quest = first_quest))
        free_quest(quest);
}

void reset_quest(QUEST_DATA *quest)
{
    QUEST_TASK *task;

    quest->completed = 0;
    if (quest->completed_by)
        STRFREE(quest->completed_by);
    for (task = quest->first_task; task; task = task->next)
    {
        if (task->completed_by)
            STRFREE(task->completed_by);
        task->completed = 0;
    }
    if (!IS_QUEST_FLAG(quest, QUEST_ACTIVE))
    {
        SET_QUEST_FLAG(quest, QUEST_ACTIVE);
        if (quest->first_task)
            quests_active++;
    }
}

void fread_quest_task(QUEST_TASK *task, FILE *fp)
{
    const char *word = NULL;
    bool fMatch = FALSE;

    for ( ; ; )
    {
        word   = feof( fp ) ? "EndTask" : fread_word( fp );
        fMatch = FALSE;

        switch ( UPPER(word[0]) )
        {
        case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;
        case 'E':
            if ( !str_cmp( word, "EndTask" ) )
                return;
            break;
        case 'C':
            KEY( "Completed",     task->completed,      fread_number( fp ) );
            KEY( "CompletedBy",   task->completed_by,   fread_string( fp ) );
            break;
        case 'D':
            KEY( "Description",   task->description,    fread_string_nohash( fp ) );
            break;
        case 'F':
            KEY( "Flags",         task->flags,          fread_number( fp ) );
            break;
        case 'G':
            KEY( "Glory",         task->glory,          fread_number( fp ) );
            break;
        case 'T':
            KEY( "Type",          task->type,           fread_number( fp ) );
            break;
        case 'V':
            KEY( "Vnum",          task->vnum1,          fread_number( fp ) );
            KEY( "Vnum1",         task->vnum1,          fread_number( fp ) );
            KEY( "Vnum2",         task->vnum2,          fread_number( fp ) );
            break;
        }

        if ( !fMatch )
        {
            bug( "fread_quest_task: no match: %s", word );
            fread_to_eol( fp );
        }
    }
}

void fread_quest(QUEST_DATA *quest, FILE *fp)
{
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
        case 'E':
            if ( !str_cmp( word, "End" ) )
                return;
            break;
        case 'C':
            KEY( "Creator",      quest->creator,        fread_string( fp ) );
            KEY( "Completed",    quest->completed,      fread_number( fp ) );
            KEY( "CompletedBy",  quest->completed_by,   fread_string( fp ) );
            break;
        case 'D':
            KEY( "Description",  quest->description,    fread_string_nohash( fp ) );
            break;
        case 'F':
            KEY( "Flags",        quest->flags,          fread_number( fp ) );
            break;
        case 'G':
            KEY( "Glory",        quest->glory,          fread_number( fp ) );
            break;
        case 'N':
            KEY( "Name",         quest->name,           fread_string( fp ) );
            break;
        case 'O':
            KEY( "Owner",        quest->owner,          fread_string( fp ) );
            break;
        case 'T':
            if ( !str_cmp( word, "Task" ) )
            {
                QUEST_TASK *task;
                CREATE(task, QUEST_TASK, 1);
                fread_quest_task(task, fp);
                LINK(task, quest->first_task, quest->last_task, next, prev);
                fMatch = TRUE;
                break;
            }
            break;
        }

        if ( !fMatch )
        {
            bug( "fread_quest: no match: %s", word );
            fread_to_eol( fp );
        }
    }
}

void load_quests(void)
{
    FILE *fp;
    QUEST_DATA *quest;
    char letter, *word;

    first_quest = NULL;
    last_quest = NULL;
    quests_active = 0;

    if (!(fp = fopen(QUEST_FILE, "r")))
        return;

    while (!feof(fp))
    {
        letter = fread_letter( fp );

        if ( letter == '*' )
        {
            fread_to_eol( fp );
            continue;
        }

        if ( letter != '#' )
        {
            bug( "load_quests: # not found (%c(%d) instead)", letter, letter );
            break;
        }

        word = fread_word( fp );
        if ( !str_cmp( word, "QUEST" ) )
        {
            CREATE(quest, QUEST_DATA, 1);
            fread_quest(quest, fp);
            LINK(quest, first_quest, last_quest, next, prev);
            if (IS_QUEST_FLAG(quest, QUEST_ACTIVE) &&
                quest->first_task)
                quests_active++;
            continue;
        }

        if ( !str_cmp( word, "END" ) )
            break;

        bug( "load_quests: bad section %s", word );
        break;
    }

    FCLOSE(fp);

    boot_log("load_quests: %d active quests", quests_active);
}

void fwrite_quest(FILE *fp, QUEST_DATA *quest)
{
    QUEST_TASK *task;

    fprintf(fp, "#QUEST\n");
    fprintf(fp, "Name          %s~\n", strip_cr(quest->name));
    if (quest->description)
        fprintf(fp, "Description   %s~\n", strip_cr(quest->description));
    fprintf(fp, "Creator       %s~\n", quest->creator);
    if (quest->owner)
        fprintf(fp, "Owner         %s~\n", quest->owner);
    if (quest->completed_by)
        fprintf(fp, "CompletedBy   %s~\n", quest->completed_by);
    if (quest->completed)
        fprintf(fp, "Completed     %ld\n", (unsigned long)quest->completed);
    if (quest->flags)
        fprintf(fp, "Flags         %d\n",  quest->flags);
    if (quest->glory)
        fprintf(fp, "Glroy         %d\n",  quest->glory);

    for (task = quest->first_task; task; task = task->next)
    {
        fprintf(fp, "Task\n");
        if (task->description)
            fprintf(fp, "Description   %s~\n", strip_cr(task->description));
        if (task->completed_by)
            fprintf(fp, "CompletedBy   %s~\n", task->completed_by);
        if (task->completed)
            fprintf(fp, "Completed     %ld\n", (unsigned long)task->completed);
        if (task->flags)
            fprintf(fp, "Flags         %d\n",  task->flags);
        fprintf(fp, "Type          %d\n",  task->type);
        if (task->vnum1)
            fprintf(fp, "Vnum1         %d\n",  task->vnum1);
        if (task->vnum2)
            fprintf(fp, "Vnum2         %d\n",  task->vnum2);
        if (task->glory)
            fprintf(fp, "Glory         %d\n",  task->glory);
        fprintf(fp, "EndTask\n");
    }
    fprintf(fp, "End\n\n");
}

void save_quests(void)
{
    FILE *fp;
    QUEST_DATA *quest;

    if (!(fp = fopen(QUEST_FILE, "w")))
    {
        bug("Unable to write quest data to " QUEST_FILE);
        return;
    }

    for (quest = first_quest; quest; quest = quest->next)
        fwrite_quest(fp, quest);

    fprintf(fp, "#END\n");

    FCLOSE(fp);
}

QUEST_DATA *find_quest(char *name)
{
    QUEST_DATA *quest;

    for (quest = first_quest; quest; quest = quest->next)
        if (!str_prefix(name, quest->name))
            return quest;

    return NULL;
}

int get_questflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, quest_flags[x] ) )
            return x;
    return -1;
}

int get_taskflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, task_flags[x] ) )
            return x;
    return -1;
}

int get_questtasktype( char *flag )
{
    int x;

    for ( x = 0; x < MAX_TASK_TYPE; x++ )
        if ( !str_cmp( flag, quest_task_type_names[x] ) )
            return x;
    return -1;
}

char *quest_onlyonce_makevarname(QUEST_DATA *quest)
{
    static char varname[MAX_VAR_NAME_LEN];
    unsigned int x, y = 0;

    strcpy(varname, "qoo_");
    x = strlen(varname);

    while (x < MAX_VAR_NAME_LEN-1 && y < strlen(quest->name))
    {
        if (isalnum(quest->name[y]))
            varname[x++] = quest->name[y++];
        else
            y++;
    }
    varname[x] = '\0';

    return varname;
}

void quest_onlyonce_add(CHAR_DATA *ch, QUEST_DATA *quest)
{
    char *varname;

    if (!(varname = quest_onlyonce_makevarname(quest)))
    {
        bug("quest_onlyonce_add: bad variable name");
        return;
    }

    if (get_var(ch->vars, varname))
    {
        bug("quest_onlyonce_add: ch already has onlyonce variable %s",
            varname);
        return;
    }

    set_var(&ch->vars, varname, "completed");
}

bool quest_onlyonce_check(CHAR_DATA *ch, QUEST_DATA *quest)
{
    char *varname;

    if (!IS_QUEST_FLAG(quest, QUEST_ONLYONCE))
        return FALSE;

    if (!(varname = quest_onlyonce_makevarname(quest)))
    {
        bug("quest_onlyonce_check: bad variable name");
        return FALSE;
    }

    if (check_var_equals(ch->vars, varname, "completed"))
        return TRUE;

    return FALSE;
}

void do_quests(CHAR_DATA *ch, char *argument)
{
    QUEST_DATA *quest;
    QUEST_TASK *task;
    int icnt = 0;
    char s1[16], s2[16], s3[16], s4[16];

    sprintf(s1, "%s", color_str(AT_SCORE, ch));
    sprintf(s2, "%s", color_str(AT_SCORE2, ch));
    sprintf(s3, "%s", color_str(AT_SCORE3, ch));
    sprintf(s4, "%s", color_str(AT_SCORE4, ch));

    if (!str_cmp(argument, "list"))
    {
        pager_printf(ch, "%-40.40s %-10.10s %-16.16s\n\r",
                     "[Quest Name]", "[Glory]", "[Creator]");
        for (quest = first_quest; quest; quest = quest->next)
        {
            if (!IS_QUEST_FLAG(quest, QUEST_ACTIVE) ||
                !quest->name || !quest->owner ||
                quest->completed)
                continue;

            if (str_cmp(quest->owner, GET_NAME(ch)))
                continue;

            if (quest_onlyonce_check(ch, quest))
                continue;

            pager_printf(ch, "%-40.40s %-10d %-16.16s\n\r",
                         quest->name?quest->name:"(none)",
                         quest->glory,
                         quest->creator?quest->creator:"(nobody)");
            icnt++;
        }

        pager_printf(ch, "\n\rQuests that anybody can complete:\n\r");
        for (quest = first_quest; quest; quest = quest->next)
        {
            if (!IS_QUEST_FLAG(quest, QUEST_ACTIVE) ||
                !quest->name || !quest->owner ||
                quest->completed)
                continue;

            if (str_cmp(quest->owner, "Anybody"))
                continue;

            if (quest_onlyonce_check(ch, quest))
                continue;

            pager_printf(ch, "%-40.40s %-10d %-16.16s\n\r",
                         quest->name,
                         quest->glory,
                         quest->creator?quest->creator:"(nobody)");
            icnt++;
        }
        pager_printf(ch, "%d quests available.\n\r", icnt);
        return;
    }

    if (!str_cmp(argument, "completed"))
    {
        pager_printf(ch, "%-40.40s %s\n\r",
                     "[Quest Name]", "[Completed]");
        for (quest = first_quest; quest; quest = quest->next)
        {
            if (!quest->name || !quest->owner ||
                !quest->completed ||
                str_cmp(quest->completed_by, GET_NAME(ch)) ||
                str_cmp(quest->owner, GET_NAME(ch)))
                continue;

            pager_printf(ch, "%-40.40s %.24s\n\r",
                         quest->name,
                         ctime(&quest->completed));
        }

        return;
    }

    if (argument && *argument && (quest = find_quest(argument)))
    {
        if (!quest->name || !quest->owner ||
            (str_cmp(quest->owner, GET_NAME(ch)) &&
             str_cmp(quest->owner, "Anybody")))
        {
            send_to_char("You cannot view that quest.\n\r", ch);
            return;
        }

        ch_printf(ch,
                  "%sName         : %s%s\n\r"
                  "%sCreator      : %s%s\n\r"
                  "%sOwner        : %s%s\n\r",
                  s1, s3, quest->name?quest->name:"(none)",
                  s1, s3, quest->creator?quest->creator:"(nobody)",
                  s1, s3, quest->owner?quest->owner:"(nobody)"
                 );

        if (quest->completed)
            ch_printf(ch,
                      "%sCompleted By : %s%s\n\r"
                      "%sCompleted    : %s%.24s\n\r",
                      s1, s3, quest->completed_by?quest->completed_by:"Nobody",
                      s1, s3, quest->completed?ctime(&quest->completed):"Never"
                     );

        if (quest->glory)
            ch_printf(ch,
                      "%sGlory Awarded: %s%d\n\r",
                      s1, s2, quest->glory
                     );


        icnt = 0;
	for (task = quest->first_task; task; task = task->next)
	    icnt++;

        ch_printf(ch,
                  "%sDescription  :\n\r%s%s\n\r"
                  "%sTasks (%s%d%s total):\n\r",
                  s1, s3, quest->description?quest->description:"(none)",
		  s1, s2, icnt, s1
                 );

        if (!quest->first_task)
        {
            send_to_char(" None\n\r", ch);
            return;
        }

        icnt = 0;
        for (task = quest->first_task; task; task = task->next)
        {
            if (IS_QUEST_FLAG(quest, QUEST_INORDER) &&
                task->completed)
                continue;

            ch_printf(ch,
                      " %s%2d%s) %s%s\n\r",
                      task->completed?s4:s2,
                      ++icnt, s1,
                      s3, task->description?task->description:"No Description"
                     );

            if (IS_QUEST_FLAG(quest, QUEST_INORDER) &&
                !IS_TASK_FLAG(task, TASK_OPTIONAL))
                break;
        }
        return;
    }

    send_to_char("Usage:\n\r"
                 "  quests list\n\r"
                 "  quests completed\n\r"
                 "  quests <name>\n\r",
                 ch);
}

void do_quest(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    QUEST_DATA *quest;
    QUEST_TASK *task;
    int icnt = 0;

    if (!IS_IMMORTAL(ch))
    {
        do_quests(ch, argument);
        return;
    }

    if (!argument || !*argument)
    {
        send_to_char("Usage:\n\r"
                     "  quest list\n\r"
                     "  quest create <name>\n\r\n\r"
                     "  quest <name> <command>\n\r"
                     "  Command being: delete show reset name desc owner flags glory task\n\r\n\r"
                     "  quest <name> task <number> <subcommand>\n\r"
                     "  Subcommand being: delete insert desc flags type vnum vnum2 glory\n\r",
                     ch);
        return;
    }

    if (!str_cmp(argument, "list"))
    {
        int acnt=0;

        pager_printf(ch, " %-40.40s %-16.16s %-16.16s\n\r",
                     "[Name]", "[Owner]", "[Creator]");
        for (quest = first_quest; quest; quest = quest->next)
        {
            pager_printf(ch, "%c%-40.40s %-16.16s %-16.16s\n\r",
                         (IS_QUEST_FLAG(quest, QUEST_ACTIVE) &&
                          quest->first_task)?'*':' ',
                         quest->name?quest->name:"(none)",
                         quest->owner?quest->owner:"(nobody)",
                         quest->creator?quest->creator:"(nobody)");
            if (IS_QUEST_FLAG(quest, QUEST_ACTIVE) &&
                quest->first_task)
                acnt++;
            icnt++;
        }
        pager_printf(ch, "%d matches found, %d active.\n\r", icnt, acnt);
        if (quests_active != acnt)
            bug("do_quests: quests_active (%d) is not equal to counted active quests (%d)", quests_active, acnt);
        return;
    }

    argument = one_argument(argument, arg);

    if (!str_cmp(arg, "create"))
    {
        CREATE(quest, QUEST_DATA, 1);
        LINK(quest, first_quest, last_quest, next, prev);

        if (argument && *argument)
            quest->name = STRALLOC(argument);
        else
            quest->name = STRALLOC("New Quest");
        quest->creator = STRALLOC(GET_NAME(ch));

        save_quests();

        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!(quest = find_quest(arg)))
    {
        send_to_char("Can't find that quest.\n\r", ch);
        return;
    }

    if (!argument || !*argument || !str_cmp(argument, "show"))
    {
        ch_printf(ch,
                  "Name        : %s\n\r"
                  "Description : %s\n\r"
                  "Creator     : %s\n\r"
                  "Owner       : %s\n\r"
                  "Completed By: %s\n\r"
                  "Completed   : %.24s\n\r"
                  "Glory       : %d\n\r"
                  "Flags       : %s\n\r"
                  "Tasks:\n\r",
                  quest->name?quest->name:"(none)",
                  quest->description?quest->description:"(none)",
                  quest->creator?quest->creator:"(nobody)",
                  quest->owner?quest->owner:"(nobody)",
                  quest->completed_by?quest->completed_by:"Nobody",
                  quest->completed?ctime(&quest->completed):"Never",
                  quest->glory,
                  flag_string(quest->flags, quest_flags));

        if (!quest->first_task)
        {
            send_to_char(" None\n\r", ch);
            return;
        }

        for (task = quest->first_task; task; task = task->next)
            ch_printf(ch,
                      " %2d) %-8.8s %-6d %-6d %3dG   Completed: %s, %s\n\r"
                      "     %s\n\r",
                      ++icnt,
                      quest_task_type_names[task->type],
                      task->vnum1,
                      task->vnum2,
                      task->glory,
                      task->completed_by?task->completed_by:"Nobody",
                      task->completed?sec_to_hms_short(current_time-task->completed):"Never",
                      task->description?task->description:"(none)"
                     );

        return;
    }

    if (!str_cmp(argument, "delete"))
    {
        free_quest(quest);
        send_to_char("Ok.\n\r", ch);
        save_quests();
        return;
    }

    if (!str_cmp(argument, "reset"))
    {
        reset_quest(quest);
        send_to_char("Ok.\n\r", ch);
        save_quests();
        return;
    }

    argument = one_argument(argument, arg);

    if (!argument || !*argument)
    {
        do_quest(ch, NULL);
        return;
    }

    if (!str_cmp(arg, "name"))
    {
        if (quest->name)
            STRFREE(quest->name);
        quest->name = STRALLOC(argument);
        save_quests();
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "owner"))
    {
        if (quest->owner)
            STRFREE(quest->owner);
        quest->owner = STRALLOC(argument);
        save_quests();
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "desc"))
    {
        if (quest->description)
            DISPOSE(quest->description);
        quest->description = str_dup(argument);
        save_quests();
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "flags"))
    {
        int value;

        if ( !argument || argument[0] == '\0' )
        {
            send_to_char("Available quest flags: ", ch);
            for ( value = 0; value < 32; value++ )
                ch_printf(ch, "%s ", quest_flags[value]);
            send_to_char("\n\r", ch);
        }

        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg );
            value = get_questflag( arg );
            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown quest flag: %s\n\r", arg );
            else
                TOGGLE_BIT(quest->flags, 1 << value);
        }

        quests_active=0;
        for (quest = first_quest; quest; quest = quest->next)
            if (IS_QUEST_FLAG(quest, QUEST_ACTIVE) &&
                quest->first_task)
                quests_active++;

        save_quests();
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "glory"))
    {
        quest->glory = atoi(argument);
        save_quests();
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "creator") && get_trust(ch) >= LEVEL_IMPLEMENTOR)
    {
        if (quest->creator)
            STRFREE(quest->creator);
        quest->creator = STRALLOC(argument);
        save_quests();
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "task"))
    {
        QUEST_TASK *temp_task;
        int tnum, count=0;

        argument = one_argument(argument, arg);
        tnum = atoi(arg);

        if (!argument || !*argument)
        {
            do_quest(ch, NULL);
            return;
        }
        argument = one_argument(argument, arg);

        for (task = quest->first_task; task; task = task->next)
        {
            count++;
            if (count == tnum)
                break;
        }
        if (!task || count != tnum)
        {
            if (!str_cmp(arg, "delete"))
            {
                send_to_char("Task not found.\n\r", ch);
                return;
            }
            send_to_char("Creating new task...\n\r", ch);
            CREATE(task, QUEST_TASK, 1);
            LINK(task, quest->first_task, quest->last_task, next, prev);
        }
        else if (!str_cmp(arg, "delete"))
        {
            free_quest_task(quest,task);
            save_quests();
            send_to_char("Ok.\n\r", ch);
            return;
        }
        else if (!str_cmp(arg, "insert"))
        {
            count = 0;
            send_to_char("Inserting new task...\n\r", ch);
            CREATE(task, QUEST_TASK, 1);
            for (temp_task = quest->first_task; temp_task; temp_task = temp_task->next)
            {
                count++;
                if (count >= tnum)
                {
                    INSERT(task, temp_task, quest->first_task, next, prev);
                    break;
                }
            }
            if (quest->first_task != task && !task->next && !task->prev)
                LINK(task, quest->first_task, quest->last_task, next, prev);

            argument = one_argument(argument, arg);
        }

        /*
         * be careful here, if any of the below task commands need to use
         * quest it'll be goofed up after this
         */
        quests_active=0;
        for (quest = first_quest; quest; quest = quest->next)
            if (IS_QUEST_FLAG(quest, QUEST_ACTIVE) &&
                quest->first_task)
                quests_active++;

        if (!str_cmp(arg, "desc"))
        {
            if (task->description)
                DISPOSE(task->description);
            task->description = str_dup(argument);
            save_quests();
            send_to_char("Ok.\n\r", ch);
            return;
        }

        if (!str_cmp(arg, "type"))
        {
            task->type = URANGE(0, get_questtasktype(argument), MAX_TASK_TYPE-1);
            save_quests();
            send_to_char("Ok.\n\r", ch);
            return;
        }

        if (!str_cmp(arg, "vnum") || !str_cmp(arg, "vnum1"))
        {
            task->vnum1 = atoi(argument);
            save_quests();
            send_to_char("Ok.\n\r", ch);
            return;
        }

        if (!str_cmp(arg, "vnum2"))
        {
            task->vnum2 = atoi(argument);
            save_quests();
            send_to_char("Ok.\n\r", ch);
            return;
        }

        if (!str_cmp(arg, "flags"))
        {
            int value;

            if ( !argument || argument[0] == '\0' )
            {
                send_to_char("Available task flags: ", ch);
                for ( value = 0; value < 32; value++ )
                    ch_printf(ch, "%s ", task_flags[value]);
                send_to_char("\n\r", ch);
            }

            while ( argument[0] != '\0' )
            {
                argument = one_argument( argument, arg );
                value = get_taskflag( arg );
                if ( value < 0 || value > 31 )
                    ch_printf( ch, "Unknown task flag: %s\n\r", arg );
                else
                    TOGGLE_BIT(task->flags, 1 << value);
            }

            save_quests();
            send_to_char("Ok.\n\r", ch);
            return;
        }


        if (!str_cmp(arg, "glory"))
        {
            task->glory = atoi(argument);
            save_quests();
            send_to_char("Ok.\n\r", ch);
            return;
        }

        if (arg[0] == '\0')
        {
            send_to_char("Default values set for task.\n\r", ch);
            return;
        }
    }

    do_quest(ch, NULL);
}

void complete_task(CHAR_DATA *ch, QUEST_DATA *quest, QUEST_TASK *task)
{
    QUEST_TASK *next_task;

    task->completed    = current_time;
    task->completed_by = STRALLOC(GET_NAME(ch));

    for (next_task = quest->first_task; next_task; next_task = next_task->next)
        if (!next_task->completed && !IS_TASK_FLAG(next_task, TASK_OPTIONAL))
            break;

    set_char_color(AT_GREEN, ch);
    if (next_task && !next_task->completed && !IS_TASK_FLAG(next_task, TASK_OPTIONAL))
    {
        send_to_char("You have completed part of a quest!\n\r", ch);
        if (task->glory != 0 && !IS_NPC(ch))
        {
            ch->pcdata->quest_curr  += task->glory;
            ch->pcdata->quest_accum += task->glory;
            ch_printf(ch, "You have earned %d glory!\n\r", task->glory);
        }

        if (IS_QUEST_FLAG(quest, QUEST_INORDER))
        {
            set_char_color(AT_YELLOW, ch);
            ch_printf(ch, "Your next task:\n\r%s\n\r",
                      next_task->description);
        }

        sprintf(log_buf, "%s completed a task from quest '%s'", GET_NAME(ch), quest->name);
        log_string_plus(log_buf, LOG_MONITOR, LEVEL_LOG_CSET, SEV_NOTICE);
    }
    else
    {
        quest->completed    = current_time;
        quest->completed_by = STRALLOC(GET_NAME(ch));
        if (IS_QUEST_FLAG(quest, QUEST_AUTORESET))
        {
            reset_quest(quest);
        }
        else
        {
            REMOVE_QUEST_FLAG(quest, QUEST_ACTIVE);
            quests_active--;
        }

        /* only allow players to do this quest once */
        if (IS_QUEST_FLAG(quest, QUEST_ONLYONCE))
            quest_onlyonce_add(ch, quest);
 
        send_to_char("You have completed a quest!\n\r", ch);
        if ((task->glory + quest->glory) != 0 && !IS_NPC(ch))
        {
            ch->pcdata->quest_curr  += (task->glory + quest->glory);
            ch->pcdata->quest_accum += (task->glory + quest->glory);
            ch_printf(ch, "You have earned %d glory!\n\r", task->glory + quest->glory);
        }

        sprintf(log_buf, "%s completed quest '%s'", GET_NAME(ch), quest->name);
        log_string_plus(log_buf, LOG_MONITOR, LEVEL_LOG_CSET, SEV_NOTICE);

        sprintf(log_buf, "%d active quests", quests_active);
        log_string_plus(log_buf, LOG_MONITOR, LEVEL_LOG_CSET, SEV_NOTICE);
    }

    save_quests();
}

static bool quest_trigger_vnumfind(CHAR_DATA *ch, int type, int vnum1, int vnum2)
{
    QUEST_DATA *quest;
    QUEST_TASK *task;
    bool completed = FALSE;

    if (!quests_active)
        return FALSE;

    for (quest = first_quest; quest; quest = quest->next)
    {
        if (!IS_QUEST_FLAG(quest, QUEST_ACTIVE))
            continue;

        if (!IS_QUEST_FLAG(quest, QUEST_ALLOWNPC) &&
            IS_NPC(ch))
            continue;

        if (str_cmp(quest->owner, "Anybody") &&
            str_cmp(quest->owner, GET_NAME(ch)))
            continue;

        if (quest_onlyonce_check(ch, quest))
            continue;

        for (task = quest->first_task; task; task = task->next)
        {
            if (task->completed)
                continue;

            if (task->type == type &&
                task->vnum1 == vnum1 &&
                task->vnum2 == vnum2)
            {
                complete_task(ch, quest, task);
                completed = TRUE;
                continue;
            }

            if (IS_QUEST_FLAG(quest, QUEST_INORDER) &&
                !IS_TASK_FLAG(task, TASK_OPTIONAL))
                break;
        }
    }

    return completed;
}

bool quest_trigger_objfind(CHAR_DATA *ch, OBJ_DATA *obj)
{
    if (!obj->pIndexData)
        return FALSE;

    if (quest_trigger_vnumfind(ch, QUEST_OBJ_FIND, obj->vnum, 0))
    {
        oprog_quest_trigger(ch, obj);
        return TRUE;
    }
    return FALSE;
}

bool quest_trigger_mobfind(CHAR_DATA *ch, CHAR_DATA *mob)
{
    if (!mob->pIndexData)
        return FALSE;
    if (quest_trigger_vnumfind(ch, QUEST_MOB_FIND, mob->vnum, 0))
    {
        mprog_quest_trigger(mob, ch);
        return TRUE;
    }
    return FALSE;
}

bool quest_trigger_roomfind(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
    if (quest_trigger_vnumfind(ch, QUEST_ROOM_FIND, room->vnum, 0))
    {
        rprog_quest_trigger(ch);
        return TRUE;
    }
    return FALSE;
}

bool quest_trigger_mobkill(CHAR_DATA *ch, CHAR_DATA *victim)
{
    if (!victim->pIndexData)
        return FALSE;
    if (quest_trigger_vnumfind(ch, QUEST_MOB_KILL, victim->vnum, 0))
    {
        mprog_quest_trigger(victim, ch);
        return TRUE;
    }
    return FALSE;
}

bool quest_trigger_objgive(CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *victim)
{
    if (!obj->pIndexData || !victim->pIndexData)
        return FALSE;
    if (quest_trigger_vnumfind(ch, QUEST_OBJ_GIVE, obj->vnum, victim->vnum))
    {
        oprog_quest_trigger(ch, obj);
        if (char_died(ch))
            return TRUE;
        mprog_quest_trigger(victim, ch);
        return TRUE;
    }
    return FALSE;
}

bool quest_trigger_objplace(CHAR_DATA *ch, OBJ_DATA *obj, ROOM_INDEX_DATA *room)
{
    if (!obj->pIndexData)
        return FALSE;
    if (quest_trigger_vnumfind(ch, QUEST_OBJ_PLACE, obj->vnum, room->vnum))
    {
        oprog_quest_trigger(ch, obj);
        if (char_died(ch))
            return TRUE;
        rprog_quest_trigger(ch);
        return TRUE;
    }
    return FALSE;
}
