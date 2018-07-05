/******************************************************
 Desolation of the Dragon MUD II
 (C) 1997-2003  Jesse DeFer
 http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*
 * Acro for SMAUG 1.x            http://dotd.com
 * (C) Jesse DeFer, dotd@dotd.com, dotd.com 4000
 *
 * Acro is a multiplayer game involving acronyms.  The computer gives you
 * an acronym, and you have to expand it into a real sentence that would fit
 * the acronym.  Every player submits their own sentence and then everybody
 * votes on the best one.  The person with the most votes wins the round, and
 * the game continues with a new acronym.  There is no limit to the number of
 * players, minimum suggested is 3 (because two players will always tie).
 *
 * Installation instructions:
 * 1. Copy the Acro source to your SMAUG src dir
 * 2. Add it to your makefile to be compiled/linked with SMAUG
 * 3. Add do_acro to mud.h, tables.c, and commands.dat
 * 4. In update.c, add acro_check() inside the block that handles seconds
 * 5. Update ACRO_COMMANDNAME to the same name you created your command as
 * 6. Check the defines after these comments
 * 7. Add lines 2-4 of this file to HELP ACRO (the copyright)
 * 8. Add remove_acro_player(GET_NAME(ch)); to free_char()
 * 9. Optionally add free_acro_players() to your MUD exit routines (provided
 *    for those of you who use debugging mallocs, otherwise you can skip it)
 *
 * IMC Installation instructions:
 * AntiFreeze CL-2 and later:
 * 1. init_acro(); to the mud's boot sequence (boot_db is a good choice)
 * Previous to AntiFreeze CL-1:
 * 1. Paste the following into imc.c after "tell" in imc_recv()
 else if( !strcasecmp( p->type, "acro" ) )
 {
 void imc_recv_acro(imc_char_data *from, PACKET *p);
 imc_recv_acro( &d, p );
 }
 * 2. Uncomment the #define USE_IMC below
 */

/* Changes:
 * 0.01:
 *      initial release
 * 0.02:
 *      smarter acronym generator
 * 0.03:
 *      short circuit voting/submit phases if everybody has voted/submitted
 * 0.04:
 *      add optional log file
 * 0.05:
 *      remove dependence on supermob, fix word capitalization code
 * 0.06:
 *      IMC2 support
 * 0.07:
 *      Upgrade to IMC2 AntiFreeze CL-2 (this means you)
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "mud.h"

#define ACRO_VERSION "0.07"
#define ACRO_INFO "SMAUG Acro v" ACRO_VERSION ", by Jesse DeFer, jdefer@dotd.com"
#define ACRO_CVSID "$Id: acro.c,v 1.27 2004/04/01 02:56:03 dotd Exp $"
#define ACRO_COMMANDNAME "acro"

/* define this to log acros and scores to a file, comment out to disable */
#define ACRO_FILE SYSTEM_DIR "acro.log"

/* minimum length of generated acronyms in letters */
#define ACRO_MIN_LENGTH 3
/* maximum length of generated acronyms in letters */
#define ACRO_MAX_LENGTH 6

/* length of time to accept new sentences */
#define ACRO_ACCEPTING_TIME     90
/* length of time to allow voting */
#define ACRO_VOTING_TIME        30
/* length of time before a new game starts */
#define ACRO_BETWEEN_ROUND_TIME 10

/* minimum players before the game starts */
#define ACRO_MIN_PLAYERS             3
/* allow people to change their sentence as many times as they want before voting */
/*#define ACRO_ALLOW_ENTRY_CHANGES     1*/
/* allow people to vote for their own entry */
/*#define ACRO_ALLOW_VOTE_SELF         1*/
/* how many sentences must be submitted before voting can take place */
#define ACRO_SUBMISSIONS_FOR_VOTING  1
/* how many rounds in which there are no submissions before the game ends */
#define ACRO_IDLE_ROUNDS_BEFORE_END  2
/* auto capitalize entries, comment out to disable */
#define ACRO_PRETY_PRINT_ENTRY

/* IMC2 support for acro, comment out to disable */
/*#define USE_IMC*/

#ifndef GET_NAME
#define GET_NAME(ch) (ch)->name
#endif

void do_acro(CHAR_DATA *ch, char *argument);

typedef struct acro_player ACRO_PLAYER;

struct acro_player
{
    ACRO_PLAYER *next;
    ACRO_PLAYER *prev;

    char *name;
    char *entry;
#ifdef USE_IMC
    char *server;
#endif
    int votes;
    bool voted;
    int idle_rounds;
    bool newplayer;
    int wins;
    int ties;
    int rounds;
};

typedef enum
{
    ACRO_IDLE, ACRO_BETWEEN_ROUNDS, ACRO_ACCEPTING, ACRO_VOTING
} acro_state_type;

#define ACRO_POOL_LAST 71
char acro_letter_pool[ACRO_POOL_LAST+1] =
{
    'A', 'A', 'A', 'A', 'A', 'B', 'C', 'D', 'E', 'E',
    'E', 'E', 'E', 'F', 'F', 'F', 'G', 'G', 'H', 'H',
    'H', 'H', 'H', 'I', 'I', 'I', 'I', 'I', 'J', 'J',
    'K', 'L', 'L', 'M', 'M', 'N', 'N', 'N', 'N', 'N',
    'O', 'O', 'O', 'O', 'O', 'P', 'Q', 'R', 'R', 'R',
    'R', 'R', 'S', 'S', 'S', 'S', 'S', 'T', 'T', 'T',
    'T', 'T', 'U', 'U', 'U', 'V', 'W', 'X', 'Y', 'Y',
    'Y', 'Z'
};

char acro[ACRO_MAX_LENGTH+1];
acro_state_type acro_state = ACRO_IDLE;
time_t acro_time;
int acro_idle_rounds = 0;

ACRO_PLAYER *first_ap = NULL;
ACRO_PLAYER *last_ap = NULL;
int acro_num_players = 0;

#ifdef USE_IMC
#include "imcsdk.h"
#endif

static ACRO_PLAYER *find_acro_player(char *name)
{
    ACRO_PLAYER *ap;

    for (ap = first_ap; ap; ap = ap->next)
	if (!str_cmp(ap->name, name))
	    return ap;

    return NULL;
}

static ACRO_PLAYER *get_acro_player(char *name)
{
    ACRO_PLAYER *ap;

    if ((ap = find_acro_player(name)))
        return ap;

    CREATE(ap, ACRO_PLAYER, 1);

    ap->name  = str_dup(name);
    ap->entry = NULL;
#ifdef USE_IMC
    ap->server = NULL;
#endif
    ap->votes = 0;
    ap->voted = FALSE;
    ap->newplayer = TRUE;
    ap->wins  = 0;
    ap->ties  = 0;
    ap->rounds= 0;

    ap->next  = NULL;
    ap->prev  = NULL;

    acro_num_players++;

    LINK(ap, first_ap, last_ap, next, prev);

    return ap;
}

void remove_acro_player(char *name)
{
    ACRO_PLAYER *ap;

    if (!(ap = find_acro_player(name)))
        return;

    UNLINK(ap, first_ap, last_ap, next, prev);
    DISPOSE(ap->name);
#ifdef USE_IMC
    if (ap->server)
	DISPOSE(ap->server);
#endif
    DISPOSE(ap);

    acro_num_players--;
}

void remove_acro_players(void)
{
    while (first_ap)
        remove_acro_player(first_ap->name);
}

static void clean_player_entries(void)
{
    ACRO_PLAYER *ap;

    for (ap = first_ap; ap; ap = ap->next)
    {
        if (ap->entry)
        {
            DISPOSE(ap->entry);
            ap->entry = NULL;
        }
        ap->votes = 0;
        ap->voted = FALSE;
        ap->newplayer = FALSE;
    }
}

static int generate_acro(char *acro)
{
    int len = number_range(ACRO_MIN_LENGTH, ACRO_MAX_LENGTH);
    int x;

    for (x=0;x<len;x++)
        acro[x] = acro_letter_pool[number_range(0,ACRO_POOL_LAST)];
    acro[x] = '\0';

    return len;
}

static CHAR_DATA *get_char_for_ap(ACRO_PLAYER *ap)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *ch;

    for ( d = first_descriptor; d; d = d->next )
    {
        ch = d->character?d->character:d->original;
        if (!ch)
            continue;

        if (!str_cmp(GET_NAME(ch), ap->name))
            return ch;
    }

    return NULL;
}

#ifdef USE_IMC
void imc_send_acro(CHAR_DATA *ch, char *to, char *command, char *argument)
{
    PACKET out;

    if ( imc_active<IA_UP )
        return;

    setdata(&out, imc_getdata(ch));

    imcstrlcpy( out.to, to, IMC_NAME_LENGTH );
    imcstrlcpy( out.type, "acro", IMC_TYPE_LENGTH );
    imc_addkey( &out, "command", command );
    if (argument && *argument!='\0')
	imc_addkey( &out, "argument", argument );

    imc_send(&out);
    imc_freedata(&out);
}
#endif

static void announce_acro(const char *str, ...)
{
    CHAR_DATA *ch;
    ACRO_PLAYER *ap;
    va_list param;
    char buf[MAX_STRING_LENGTH];

    va_start( param, str );
    vsprintf( buf, str, param );
    va_end( param );

    for (ap = first_ap; ap; ap = ap->next)
    {
#ifdef USE_IMC
	if (strchr(ap->name, '@'))
	{
	    imc_send_acro(NULL, ap->name, "message", buf);
            continue;
	}
#endif
        if (!(ch = get_char_for_ap(ap)))
            continue;

	send_to_char(buf, ch);
    }

    /* send announce to instead maybe? */
}

static void acro_new_round(void)
{
    clean_player_entries();
    generate_acro(acro);
    acro_time = current_time + ACRO_ACCEPTING_TIME;
    acro_state = ACRO_ACCEPTING;
    announce_acro("You have %d seconds to submit an entry for: %s\n\r",
                  acro_time - current_time, acro);
}

static void acro_announce_scores(void)
{
    ACRO_PLAYER *ap;
    char buf[MAX_INPUT_LENGTH];

#ifdef ACRO_FILE
    sprintf(buf, "--------------------------------------------\nAcro: %s\n", acro);
    append_to_file(ACRO_FILE, buf);
#endif

    for (ap = first_ap; ap; ap = ap->next)
        if (ap->entry)
        {
            sprintf(buf, "%-30s  %2d votes  %s\n\r",
                    ap->name, ap->votes, ap->entry);
            announce_acro(buf);
#ifdef ACRO_FILE
            append_to_file(ACRO_FILE, strip_lf(buf));
#endif
        }
}

bool valid_acro(char *acro, char *str)
{
    char *s = str, *a = acro;

    if (tolower(*s) != tolower(*a))
        return FALSE;

    a++;

    if (!*a)
    {
        while (*s)
            if (*s++ == ' ')
                return FALSE;
        return TRUE;
    }

    while (*s)
    {
        if (*s == ' ')
        {
            s++;

            if (!*s || tolower(*s) != tolower(*a))
                return FALSE;

            a++;

            if (!*a)
            {
                while (*s)
                    if (*s++ == ' ')
                        return FALSE;
                return TRUE;
            }
        }
        s++;
    }

    return FALSE;
}

void send_to_ap(char *txt, ACRO_PLAYER *ap)
{
    CHAR_DATA *ch;

    if (!ap)
        return;

    if ((ch = get_char_for_ap(ap)))
	send_to_char(txt, ch);
#ifdef USE_IMC
    else if (strchr(ap->name, '@'))
	imc_send_acro(NULL, ap->name, "message", txt);
#endif
    else
	bug("send_to_ap: unable to find location to send to.");
}

void ap_printf(ACRO_PLAYER *ap, char *fmt, ...)
{
    char buf[MAX_STRING_LENGTH];
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);

    send_to_ap(buf, ap);
}

void acro_command(char *name, char *argument)
{
    ACRO_PLAYER *ap, *tap;
#ifdef ACRO_PRETTY_PRINT_ENTRY
    bool ls;
#endif

    if (argument && *argument)
    {
	if (!str_cmp(argument, "quit"))
	{
	    if ((ap = find_acro_player(name)))
	    {
		send_to_ap("You have quit playing acro.\n\r", ap);
		announce_acro("%s has quit playing acro.\n\r", name);
		remove_acro_player(name);
		return;
	    }
            return;
	}
	if (!str_cmp(argument, "scores"))
	{
	    if (!(ap = find_acro_player(name)))
		return;

	    ap_printf(ap, "%s\n\r%s\n\r\n\r",
		      ACRO_INFO, ACRO_CVSID);

	    for (tap = first_ap; tap; tap = tap->next)
		ap_printf(ap, "%-30s  %2d wins  %2d ties  %2d rounds\n\r",
			  tap->name, tap->wins, tap->ties, tap->rounds);
	    return;
	}
    }

    if (acro_state == ACRO_IDLE)
    {
        ap = get_acro_player(name);

        if (acro_num_players >= ACRO_MIN_PLAYERS)
        {
            acro_new_round();
            acro_idle_rounds = 0;
        }
        else
	    ap_printf(ap, "A new game of acro will start when there are at least %d more players.\n\r",
		      ACRO_MIN_PLAYERS - acro_num_players);
        return;
    }

    if (acro_state == ACRO_BETWEEN_ROUNDS)
    {
        ap = get_acro_player(name);
        ap->newplayer = FALSE;
	ap_printf(ap, "The next round will start in %d seconds.\n\r",
		    acro_time - current_time, acro);
        return;
    }

    if (acro_state == ACRO_ACCEPTING)
    {
	ap = find_acro_player(name);

	if (!argument || !*argument)
	{
#ifndef ACRO_ALLOW_ENTRY_CHANGES
	    if (ap && ap->entry)
		ap_printf(ap, "There are %d seconds until voting starts for: %s\n\r",
			    acro_time - current_time, acro);
	    else
#endif
		ap_printf(ap, "You have %d seconds to submit an entry for: %s\n\r",
			    acro_time - current_time, acro);
	    return;
	}

	ap = get_acro_player(name);

	if (!valid_acro(acro, argument))
	{
	    ap_printf(ap, "That doesn't fit the acronym '%s'.\n\r", acro);
	    return;
	}

	if (ap->entry)
	{
#ifndef ACRO_ALLOW_ENTRY_CHANGES
	    send_to_ap("You already have an entry, you can't change it.\n\r", ap);
	    return;
#else
	    DISPOSE(ap->entry);
#endif
	}

	ap->entry = str_dup(argument);
	ap->idle_rounds = 0;
	ap->newplayer = FALSE;

#ifdef ACRO_PRETTY_PRINT_ENTRY
	ls = FALSE;
	ap->entry[0] = UPPER(ap->entry[0]);
	for (num=1;num<strlen(ap->entry);num++)
	    if (ap->entry[num]==' ')
		ls=TRUE;
	    else if (ls)
	    {
		ap->entry[num] = UPPER(ap->entry[num]);
		ls=FALSE;
	    }
#endif

	ap_printf(ap, "You have submitted a good acro, there are %d seconds until voting.\n\r",
		  acro_time - current_time);
	return;
    }

    if (acro_state == ACRO_VOTING)
    {
        int choice, num;

        num = 1;

	ap = get_acro_player(name);

        if (!argument || !*argument)
        {
	    ap_printf(ap, "Choices (%d seconds left to vote):\n\r",
		      acro_time - current_time);

	    for (tap = first_ap; tap; tap = tap->next)
		if (tap->entry)
		    ap_printf(ap, "%2d. %s\n\r", num++, tap->entry);

            return;
        }

        if (ap->newplayer)
        {
            send_to_ap("Please wait until the next round starts.\n\r", ap);
            return;
        }

        if (!ap->entry)
        {
	    send_to_ap("You cannot vote unless you submitted an entry.\n\r", ap);
            return;
        }

        if (ap->voted)
        {
            send_to_ap("You have already voted for this round.\n\r", ap);
            return;
        }

        if (!is_number(argument))
        {
	    send_to_ap("Please pick a number to vote for.\n\r", ap);
            return;
        }

        choice = atoi(argument);

        for (tap = first_ap; tap; tap = tap->next)
	    if (tap->entry)
	    {
                if (num == choice)
                    break;
                else
                    num++;
	    }

        if (!tap || num != choice)
        {
            send_to_ap("That choice is out of range, please try again.\n\r", ap);
            return;
        }

#ifndef ACRO_ALLOW_VOTE_SELF
        if (ap == tap)
        {
	    send_to_ap("You cannot vote for your own entry.\n\r", ap);
            return;
        }
#endif

        ap->voted = TRUE;
        tap->votes++;

	ap_printf(ap, "Good choice!  %d seconds until voting is complete.\n\r",
		  acro_time - current_time);
	return;
    }

    bug("acro_command: invalid state %d", acro_state);
}

void do_acro(CHAR_DATA *ch, char *argument)
{
#ifdef USE_IMC
    ACRO_PLAYER *ap;
#endif

    if (IS_NPC(ch))
    {
        send_to_char("No NPC's.\n\r", ch);
        return;
    }

#ifdef USE_IMC
    if ((ap = find_acro_player(GET_NAME(ch))) && ap->server)
    {
	if (!str_cmp(argument, "quit"))
	    imc_send_acro(ch, ap->server, "quit", GET_NAME(ch));

	imc_send_acro(ch, ap->server, "game", argument);
        return;
    }

    if (argument && *argument!='\0' && !str_prefix("join ", argument))
    {
	char buf[MAX_INPUT_LENGTH];

	if ((ap = find_acro_player(GET_NAME(ch))))
	{
	    send_to_char("You are already in the game.\n\r", ch);
	    return;
	}

	argument = one_argument(argument, buf);

	if (!argument || !*argument)
	{
	    send_to_char("Join which server?\n\r", ch);
	    return;
	}

	if ( !check_mud( ch, argument ) )
	    return;

	ap = get_acro_player(GET_NAME(ch));

	sprintf(buf, "*@%s", argument);
	ap->server = str_dup(buf);

        send_to_char("Joining server...\n\r", ch);

	imc_send_acro(ch, ap->server, "join", NULL);
	return;
    }
#endif

    acro_command(GET_NAME(ch), argument);
}

void acro_check(void)
{
    ACRO_PLAYER *ap, *ap_next;
    int num;

    if (acro_time >= current_time)
        return;

    if (!first_ap)
    {
	acro_state = ACRO_IDLE;
	return;
    }

    if (acro_state == ACRO_IDLE)
	return;

    if (acro_state == ACRO_BETWEEN_ROUNDS)
    {
	if (acro_num_players < ACRO_MIN_PLAYERS)
	{
	    announce_acro("There are not enough players for a new round.\n\r");
	    clean_player_entries();
	    acro_state = ACRO_IDLE;
	    return;
	}

	acro_new_round();
	acro_idle_rounds = 0;
	return;
    }

    if (acro_state == ACRO_ACCEPTING)
    {
	CHAR_DATA *player;
	announce_acro("Submission phase ended.\n\r");

	num = 0;
	for (ap = first_ap; ap; ap = ap_next)
	{
	    ap_next = ap->next;
	    if (ap->entry)
		num++;
	    else if (++ap->idle_rounds > ACRO_IDLE_ROUNDS_BEFORE_END)
	    {
		player = get_char_for_ap(ap);
		remove_acro_player(ap->name);
		if (player)
		{
		    announce_acro("%s has idled out and has been removed from the game.\n\r", GET_NAME(player));
		    send_to_char("You have idled out and have been removed from the game of Acro.\n\r", player);
		}
	    }
	}

	if (num < ACRO_SUBMISSIONS_FOR_VOTING)
	{
	    announce_acro("There were not enough submissions, ");
	    if (++acro_idle_rounds > ACRO_IDLE_ROUNDS_BEFORE_END)
	    {
		announce_acro("this game of Acro is now ending.\n\r");
		acro_state = ACRO_IDLE;
		return;
	    }

	    announce_acro("Acro will continue to a new round.\n\r");
	    acro_new_round();
	    return;
	}

	announce_acro("Voting will now commence, you have %d seconds, please choose:\n\r", ACRO_VOTING_TIME);

	num = 1;
	for (ap = first_ap; ap; ap = ap->next)
	    if (ap->entry)
		announce_acro("%2d. %s\n\r", num++, ap->entry);

	acro_state = ACRO_VOTING;
	acro_time = current_time + ACRO_VOTING_TIME;
	acro_idle_rounds = 0;
	return;
    }

    if (acro_state == ACRO_VOTING)
    {
	ACRO_PLAYER *maxap = first_ap;
	bool tie = FALSE;

	for (ap = first_ap; ap; ap = ap->next)
	    if (ap->votes > maxap->votes)
		maxap = ap;

	for (ap = first_ap; ap; ap = ap->next)
	    if (maxap->votes == ap->votes && maxap != ap)
	    {
		tie = TRUE;
		break;
	    }

	acro_announce_scores();

	if (maxap->votes == 0)
	    announce_acro("Nobody wins because nobody voted!\n\r");
	else if (!tie)
	{
	    announce_acro("%s wins the round with %d votes!\n\r", maxap->name, maxap->votes);
	    maxap->wins++;
	}
	else
	{
	    announce_acro("The round was tied with %d votes for: ", maxap->votes);
	    for (ap = first_ap; ap; ap = ap->next)
		if (maxap->votes == ap->votes)
		{
		    announce_acro("%s ", ap->name);
		    ap->ties++;
		}
	    announce_acro("\n\r");
	}

	for (ap = first_ap; ap; ap = ap->next)
	    if (!ap->newplayer)
		ap->rounds++;

	announce_acro("A new round will start in %d seconds.\n\r", ACRO_BETWEEN_ROUND_TIME);

	acro_state = ACRO_BETWEEN_ROUNDS;
	acro_time = current_time + ACRO_BETWEEN_ROUND_TIME;
	return;
    }
}

#ifdef USE_IMC
void imc_recv_acro(imc_char_data *from, PACKET *p)
{
    ACRO_PLAYER *ap;
    CHAR_DATA *victim;
    char *command, *argument;

    command = imc_getkey( p, "command", NULL );
    argument = imc_getkey( p, "argument", NULL );

    if (!command || !*command)
    {
	sprintf(log_buf, "Empty acro command from: %s", from->name);
	log_string_plus(log_buf, LOG_IMC, LEVEL_LOG_CSET, SEV_ERR);
        return;
    }

    if (!str_cmp(command, "join"))
    {
	ap = get_acro_player(from->name);

	if (ap->newplayer)
	    imc_send_acro(NULL, from->name, "message", "You have joined the game.\n\r");
	else
	    imc_send_acro(NULL, from->name, "message", "You are already in the game.\n\r");
        return;
    }

    if (!str_cmp(command, "quit"))
    {
	if (argument && *argument && find_acro_player(argument))
	{
	    remove_acro_player(argument);
	    imc_send_acro(NULL, argument, "message", "You have quit the game.\n\r");
	}
	if (find_acro_player(from->name))
	{
	    remove_acro_player(from->name);
	    imc_send_acro(NULL, from->name, "message", "You have quit the game.\n\r");
	}
        return;
    }

    if (!str_cmp(command, "game"))
    {
        acro_command(from->name, argument);
        return;
    }

    if (!(victim = imc_find_user(p->to)))
    {
        remove_acro_player(p->to);
	imc_send_acro(NULL, "*", "quit", p->to);
        return;
    }

    if (!str_cmp(command, "message"))
    {
        ch_printf(victim, "%s", argument);
        return;
    }

    sprintf(log_buf, "Unknown acro command from: %s, command: %s", from->name, command);
    log_string_plus(log_buf, LOG_IMC, LEVEL_LOG_CSET, SEV_ERR);
}
#endif

void init_acro(void)
{
#ifdef USE_IMC
    imc_register_packet_handler("acro", imc_recv_acro);
#endif
}
