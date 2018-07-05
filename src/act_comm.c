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
 *			   Player communication module			    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: act_comm.c,v 1.102 2004/04/06 22:00:08 dotd Exp $";*/

#include <sys/types.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "mud.h"
#include "gsn.h"

#include "mxp.h"
#include "property.h"
#include "currency.h"
#ifdef IRC
#include "irc.h"
#endif

DECLARE_DO_FUN(do_color);

/*
 *  Externals
 */
#ifdef IBUILD
void send_obj_page_to_char(CHAR_DATA * ch, OBJ_INDEX_DATA * idx, char page);
void send_room_page_to_char(CHAR_DATA * ch, ROOM_INDEX_DATA * idx, char page);
void send_page_to_char(CHAR_DATA * ch, MOB_INDEX_DATA * idx, char page);
void send_control_page_to_char(CHAR_DATA * ch, char page);
#endif

bool    is_house_owner          args( (CHAR_DATA *ch, ROOM_INDEX_DATA *house) );

/*
 * Local functions.
 */
char *  drunk_speech    args( ( const char *argument, CHAR_DATA *ch ) );

/* Text scrambler -- Altrag */
char *text_scramble( const char *argument, int modifier )
{
    static char arg[MAX_INPUT_LENGTH];
    sh_int position;
    sh_int conversion = 0;

    modifier %= number_range( 80, 300 ); /* Bitvectors get way too large #s */
    for ( position = 0; position < MAX_INPUT_LENGTH; position++ )
    {
        if ( argument[position] == '\0' )
        {
            arg[position] = '\0';
            return arg;
        }
        else if ( argument[position] >= 'A' && argument[position] <= 'Z' )
        {
            conversion = -conversion + position - modifier + argument[position] - 'A';
            conversion = number_range( conversion - 5, conversion + 5 );
            while ( conversion > 25 )
                conversion -= 26;
            while ( conversion < 0 )
                conversion += 26;
            arg[position] = conversion + 'A';
        }
        else if ( argument[position] >= 'a' && argument[position] <= 'z' )
        {
            conversion = -conversion + position - modifier + argument[position] - 'a';
            conversion = number_range( conversion - 5, conversion + 5 );
            while ( conversion > 25 )
                conversion -= 26;
            while ( conversion < 0 )
                conversion += 26;
            arg[position] = conversion + 'a';
        }
        else if ( argument[position] >= '0' && argument[position] <= '9' )
        {
            conversion = -conversion + position - modifier + argument[position] - '0';
            conversion = number_range( conversion - 2, conversion + 2 );
            while ( conversion > 9 )
                conversion -= 10;
            while ( conversion < 0 )
                conversion += 10;
            arg[position] = conversion + '0';
        }
        else
            arg[position] = argument[position];
    }
    arg[position] = '\0';
    return arg;
}


LANG_DATA *get_lang(const char *name)
{
    LANG_DATA *lng;

    for (lng = first_lang; lng; lng = lng->next)
	if (!str_cmp(lng->name, name))
	    return lng;
    return NULL;
}


/* percent = percent knowing the language. */
char *translate(int percent, const char *in, const char *name)
{
    LCNV_DATA *cnv;
    static char buf[256];
    char buf2[256];
    const char *pbuf;
    char *pbuf2 = buf2;
    LANG_DATA *lng;

    if ( percent > 99 || !str_cmp(name, "common") )
	return (char *) in;

    /* If we don't know this language... use "default" */
    if ( !(lng=get_lang(name)) )
	if ( !(lng = get_lang("default")) )
	    return (char *) in;

    for (pbuf = in; *pbuf;)
    {
	for (cnv = lng->first_precnv; cnv; cnv = cnv->next)
	{
	    if (!str_prefix(cnv->old, pbuf))
	    {
		if ( percent && (rand() % 100) < percent )
		{
		    strncpy(pbuf2, pbuf, cnv->olen);
		    pbuf2[cnv->olen] = '\0';
		    pbuf2 += cnv->olen;
		}
		else
		{
		    strcpy(pbuf2, cnv->newstr);
		    pbuf2 += cnv->nlen;
		}
		pbuf += cnv->olen;
		break;
	    }
	}
	if (!cnv)
	{
	    if (isalpha(*pbuf) && (!percent || (rand() % 100) > percent) )
	    {
		*pbuf2 = lng->alphabet[LOWER(*pbuf) - 'a'];
		if ( isupper(*pbuf) )
		    *pbuf2 = UPPER(*pbuf2);
	    }
	    else
		*pbuf2 = *pbuf;
	    pbuf++;
	    pbuf2++;
	}
    }
    *pbuf2 = '\0';
    for (pbuf = buf2, pbuf2 = buf; *pbuf;)
    {
	for (cnv = lng->first_cnv; cnv; cnv = cnv->next)
	    if (!str_prefix(cnv->old, pbuf))
	    {
		strcpy(pbuf2, cnv->newstr);
		pbuf += cnv->olen;
		pbuf2 += cnv->nlen;
		break;
	    }
	if (!cnv)
	    *(pbuf2++) = *(pbuf++);
    }
    *pbuf2 = '\0';
#if 0
    for (pbuf = in, pbuf2 = buf; *pbuf && *pbuf2; pbuf++, pbuf2++)
	if (isupper(*pbuf))
	    *pbuf2 = UPPER(*pbuf2);
    /* Attempt to align spacing.. */
	else if (isspace(*pbuf))
	    while (*pbuf2 && !isspace(*pbuf2))
		pbuf2++;
#endif
    return buf;
}

char *drunk_speech( const char *argument, CHAR_DATA *ch )
{
    const char *arg = argument;
    static char buf[MAX_INPUT_LENGTH*2];
    char buf1[MAX_INPUT_LENGTH*2];
    sh_int drunk;
    char *txt;
    char *txt1;

    if ( IS_NPC( ch ) || !ch->pcdata ) return (char *)arg;

    drunk = GET_COND(ch,COND_DRUNK);;

    if ( drunk <= 0 )
        return (char *)arg;

    buf[0] = '\0';
    buf1[0] = '\0';

    if ( !argument )
    {
        bug( "Drunk_speech: NULL argument" );
        return "";
    }

    /*
     if ( *arg == '\0' )
     return (char *) argument;
     */

    txt = buf;
    txt1 = buf1;

    while ( *arg != '\0' )
    {
        if ( toupper(*arg) == 'S' )
        {
            if ( number_percent() < drunk )		/* add 'h' after an 's' */
            {
                *txt++ = *arg;
                *txt++ = 'h';
            }
            else
                *txt++ = *arg;
        }
        else if ( toupper(*arg) == 'X' )
        {
            if ( number_percent() < drunk )
            {
                *txt++ = 'c', *txt++ = 's', *txt++ = 'h';
            }
            else
                *txt++ = *arg;
        }
        else if ( number_percent() < ( drunk / 5 ) )  /* slurred letters */
        {
            sh_int slurn = number_range( 1, 2 );
            sh_int currslur = 0;

            while ( currslur < slurn )
                *txt++ = *arg, currslur++;
        }
        else
            *txt++ = *arg;

        arg++;
    };

    *txt = '\0';

    txt = buf;

    while ( *txt != '\0' )   /* Let's mess with the string's caps */
    {
        if ( number_percent() < ( drunk / 2.5 ) )
        {
            if ( isupper(*txt) )
                *txt1 = tolower( *txt );
            else
                if ( islower(*txt) )
                    *txt1 = toupper( *txt );
                else
                    *txt1 = *txt;
        }
        else
            *txt1 = *txt;

        txt1++, txt++;
    };

    *txt1 = '\0';
    txt1 = buf1;
    txt = buf;

    while ( *txt1 != '\0' )   /* Let's make them stutter */
    {
        if ( *txt1 == ' ' )  /* If there's a space, then there's gotta be a */
        {			 /* along there somewhere soon */

            while ( *txt1 == ' ' )  /* Don't stutter on spaces */
                *txt++ = *txt1++;

            if ( ( number_percent() < ( drunk / 4 ) ) && *txt1 != '\0' )
            {
                sh_int offset = number_range( 0, 2 );
                sh_int pos = 0;

                while ( *txt1 != '\0' && pos < offset )
                    *txt++ = *txt1++, pos++;

                    if ( *txt1 == ' ' )  /* Make sure not to stutter a space after */
                    {		     /* the initial offset into the word */
                        *txt++ = *txt1++;
                        continue;
                    }

                    pos = 0;
                    offset = number_range( 2, 4 );
                    while (	*txt1 != '\0' && pos < offset )
                    {
                        *txt++ = *txt1;
                        pos++;
                        if ( *txt1 == ' ' || pos == offset )  /* Make sure we don't stick */
                        {		               /* A hyphen right before a space	*/
                            txt1--;
                            break;
                        }
                        *txt++ = '-';
                    }
                    if ( *txt1 != '\0' )
                        txt1++;
            }
        }
        else
            *txt++ = *txt1++;
    }

    *txt = '\0';

    return buf;
}

typedef struct channel_history_msg CHAN_HIST_MSG;
struct channel_history_msg
{
    CHAN_HIST_MSG *next;
    CHAN_HIST_MSG *prev;
    char *name;
    char *arg;
    sh_int invis_level;
};

struct channel_history
{
    CHAN_HIST_MSG *first;
    CHAN_HIST_MSG *last;
    unsigned int num_msgs;
}
channel_history[32] =
{
    { NULL, NULL, 0 },    { NULL, NULL, 0 },    { NULL, NULL, 0 },
    { NULL, NULL, 0 },    { NULL, NULL, 0 },    { NULL, NULL, 0 },
    { NULL, NULL, 0 },    { NULL, NULL, 0 },    { NULL, NULL, 0 },
    { NULL, NULL, 0 },    { NULL, NULL, 0 },    { NULL, NULL, 0 },
    { NULL, NULL, 0 },    { NULL, NULL, 0 },    { NULL, NULL, 0 },
    { NULL, NULL, 0 },    { NULL, NULL, 0 },    { NULL, NULL, 0 },
    { NULL, NULL, 0 },    { NULL, NULL, 0 },    { NULL, NULL, 0 },
    { NULL, NULL, 0 },    { NULL, NULL, 0 },    { NULL, NULL, 0 },
    { NULL, NULL, 0 },    { NULL, NULL, 0 },    { NULL, NULL, 0 },
    { NULL, NULL, 0 },    { NULL, NULL, 0 },    { NULL, NULL, 0 },
    { NULL, NULL, 0 },    { NULL, NULL, 0 }
};

void free_chan_hist(sh_int chist, CHAN_HIST_MSG *chan)
{
    STRFREE(chan->name);
    DISPOSE(chan->arg);
    UNLINK(chan, channel_history[chist].first, channel_history[chist].last, next, prev);
    DISPOSE(chan);
}

void add_channel_history( CHAR_DATA *ch, char *argument, int channel, const char *verb )
{
    CHAN_HIST_MSG *chan;
    CHAR_DATA *och;
    int chist;

    if (IS_NPC(ch) && !IS_ACT_FLAG(ch, ACT_POLYMORPHED))
	return;

    for (chist = 0; chist < 32; chist++)
        if ( channel == (1<<chist) )
            break;
    if ( channel != (1<<chist) )
        return;

    CREATE(chan, CHAN_HIST_MSG, 1);
    LINK(chan, channel_history[chist].first, channel_history[chist].last, next, prev);

    chan->name = STRALLOC(GET_NAME(ch));
    chan->arg = str_dup(argument);

    if (ch->desc && ch->desc->original)
	och = ch->desc->original;
    else
        och = ch;

    if (och->pcdata->wizinvis)
	chan->invis_level = och->pcdata->wizinvis;
    else if (IS_AFFECTED(och, AFF_INVISIBLE))
	chan->invis_level = 1;
    else
        chan->invis_level = 0;

    if (channel_history[chist].num_msgs++ > 30)
        free_chan_hist(chist, channel_history[chist].first);
}


void display_channel_history( CHAR_DATA *ch, int channel )
{
    CHAN_HIST_MSG *chan;

    int chist;

    for (chist = 0; chist < 32; chist++)
        if ( channel == (1<<chist) )
            break;
    if ( channel != (1<<chist) )
        return;

    for (chan = channel_history[chist].first; chan; chan = chan->next)
        if (chan->invis_level > GetMaxLevel(ch))
            pager_printf(ch, "Somebody: %s\n\r", chan->arg);
        else
            pager_printf(ch, "%s: %s\n\r", chan->name, chan->arg);
}


/*
 * Generic channel function.
 */
void talk_channel( CHAR_DATA *ch, char *argument, int channel, const char *verb )
{
    CHAR_DATA *victim = NULL, *vch;
    char col[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char word[MAX_INPUT_LENGTH];
    char ubuf[16];
    SOCIALTYPE *social = NULL;
    int position;
    sh_int AType;
    char *arg;
    char *socbuf_char = NULL, *socbuf_vict = NULL, *socbuf_other = NULL;
    int speaking = -1, lang;

    for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
        if ( ch->speaking & lang_array[lang] )
        {
            speaking = lang;
            break;
        }

    if ( IS_NPC( ch ) )
    {
        if ( channel == CHANNEL_CLAN )
        {
            send_to_char( "Mobs can't be in clans.\n\r", ch );
            return;
        }

        if ( channel == CHANNEL_ORDER )
        {
            send_to_char( "Mobs can't be in orders.\n\r", ch );
            return;
        }

        if ( channel == CHANNEL_COUNCIL )
        {
            send_to_char( "Mobs can't be in councils.\n\r", ch);
            return;
        }

        if ( channel == CHANNEL_GUILD )
        {   send_to_char( "Mobs can't be in guilds.\n\r", ch );
            return;
        }

        if ( IS_AFFECTED( ch, AFF_CHARM ) )
        {
            if ( ch->master )
                send_to_char( "I don't think so...\n\r", ch->master );
            return;
        }
    }

    if ( IS_ROOM_FLAG( ch->in_room, ROOM_SILENCE ) )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    if ( !argument || argument[0] == '\0' )
    {
        if (IS_IMMORTAL(ch))
        {
            display_channel_history(ch, channel);
            return;
        }
        sprintf( buf, "%s what?\n\r", verb );
        buf[0] = UPPER(buf[0]);
        send_to_char( buf, ch );	/* where'd this line go? */
        return;
    }

    if (IS_SILENCED(ch))
    {
        ch_printf( ch, "You can't %s.\n\r", verb );
        return;
    }

    if ( channel == CHANNEL_OOC && IS_PLR_FLAG( ch, PLR_NO_OOC ) )
    {
        send_to_char("You can't OOC.\n\r", ch);
        return;
    }

    REMOVE_BIT(ch->deaf, channel);

    switch ( channel )
    {
    default:
        AType = AT_GOSSIP;
        break;
    case CHANNEL_SHOUT:
        AType = AT_RED;
        break;
    case CHANNEL_TELLS:
        AType = AT_TELL;
        break;
    case CHANNEL_WARTALK:
        AType = AT_WARTALK;
        break;
    case CHANNEL_IMMTALK:
        AType = AT_BLUE;
        break;
    case CHANNEL_AVTALK:
        AType = AT_DGREEN;
        break;
    case CHANNEL_HIGHGOD:
        AType = AT_LBLUE;
        break;
    }

    if ( !str_cmp( argument, "on" ) )
    {
	CHECK_SUBRESTRICTED( ch );
        send_to_char("Channel mode on, use done to exit.\n\r", ch);
	ch->substate = SUB_REPEATCMD;
	if ( ch->pcdata )
	{
	    if ( ch->pcdata->subprompt )
		STRFREE( ch->pcdata->subprompt );
	    sprintf( buf, "&W<%s> ", verb );
	    if (ch->pcdata->prompt)
		strcat(buf, ch->pcdata->prompt);
	    ch->pcdata->subprompt = STRALLOC( buf );
	}
	return;
    }
    if ( !str_cmp( argument, "done" ) ||
	 !str_cmp( argument, "off" ) )
    {
	send_to_char( "Channel mode off.\n\r", ch );
	ch->substate = SUB_NONE;
	if ( ch->pcdata && ch->pcdata->subprompt )
	    STRFREE( ch->pcdata->subprompt );
	return;
    }

    arg = argument;
    arg = one_argument(arg,word);
    if (!IS_NPC(ch) &&
        ((*word=='@' && (social=find_social_exact(&word[1]))) ||
         (!*arg && (social=find_social_exact(word)))))
    {
        if (arg && *arg)
        {
            char name[MAX_INPUT_LENGTH];
            one_argument(arg,name);
            if ((victim = get_char_world(ch,name)))
                arg = one_argument(arg,name);
            if (!victim)
            {
                socbuf_char  = social->char_no_arg;
                socbuf_vict  = social->others_no_arg;
                socbuf_other = social->others_no_arg;
                if (!socbuf_char && !socbuf_other)
                    social = NULL;
            }
            else if (victim==ch)
            {
                socbuf_char  = social->char_auto;
                socbuf_vict  = social->others_auto;
                socbuf_other = social->others_auto;
                if (!socbuf_char && !socbuf_other)
                    social = NULL;
            }
            else if (!IS_NPC(victim))
            {
                socbuf_char  = social->char_found;
                socbuf_vict  = social->vict_found;
                socbuf_other = social->others_found;
                if (!socbuf_char && !socbuf_other && !socbuf_vict)
                    social = NULL;
            }
            else
                social = NULL;
        }
        else
        {
            socbuf_char  = social->char_no_arg;
            socbuf_vict  = social->others_no_arg;
            socbuf_other = social->others_no_arg;
            if (!socbuf_char && !socbuf_other)
                social = NULL;
        }
    }

    sprintf(col, "%s", color_str(AType, ch));
    if (social)
    {
        sprintf(buf, "%s%s[%s] %s%s",
                mxp_chan_str(ch, verb),
                col, verb, socbuf_char,
                mxp_chan_str_close(ch, verb));
    }
    else
        sprintf(buf, "%s%sYou %s '$t'%s",
                mxp_chan_str(ch, verb),
                col, verb,
                mxp_chan_str_close(ch, verb));

    act(AT_PLAIN, buf, ch, argument, victim, TO_CHAR);
    if (social && (socbuf_other == socbuf_char || !socbuf_other))
        return;

    add_channel_history(ch, argument, channel, verb);

    if ( IS_ROOM_FLAG( ch->in_room, ROOM_LOGSPEECH ) )
    {
        char roomlog[80];

        snprintf( roomlog, 80, ROOM_LOG_DIR "%d.log", ch->in_room->vnum );
        sprintf( log_buf, "%-16.16s %-10.10s %s",
                 IS_NPC( ch ) ? ch->short_descr : ch->name,
                 verb,
                 argument );

        append_to_file( roomlog, log_buf );
    }

#if USE_MUDMSG
    if ( channel == CHANNEL_IMMTALK )
        mud_message(ch, channel, argument);
#endif

#ifdef IRC
    if (channel == CHANNEL_OOC)
        irc_mud_to_channel(ch, "#OOC", argument);
    else if (channel == CHANNEL_IMMTALK)
        irc_mud_to_channel(ch, "#think", argument);
    else if (channel == CHANNEL_NEWBIE)
        irc_mud_to_channel(ch, "#newbie", argument);
#endif

    snprintf(ubuf, 15, "(u%d) ", ch->unum);

    for (vch = first_char; vch; vch = vch->next)
    {
        char *sbuf = argument;

        if (IS_NPC(vch))
            continue;

        if ( !vch->desc ||
             GET_CON_STATE(vch) != CON_PLAYING ||
             vch == ch ||
             IS_SET(vch->deaf, channel) )
            continue;

        if ( channel == CHANNEL_OOC && IS_PLR_FLAG( vch, PLR_NO_OOC ) )
            continue;
        if ( channel != CHANNEL_NEWBIE && NOT_AUTHED( vch ) )
            continue;
        if ( channel == CHANNEL_WARTALK && NOT_AUTHED( vch ) )
            continue;
        if ( channel == CHANNEL_AVTALK && !IS_HERO( vch ) )
            continue;
        if ( channel == CHANNEL_HIGHGOD && get_trust( vch ) < sysdata.muse_level )
            continue;
        if ( channel == CHANNEL_IMMTALK && get_trust( vch ) < sysdata.think_level )
            continue;
        if ( channel == CHANNEL_PRAY && !IS_IMMORTAL( vch ) )
            continue;

        /* Fix by Narn to let newbie council members see the newbie channel. */
        if ( channel == CHANNEL_NEWBIE  &&
             ( !IS_IMMORTAL(vch) && !NOT_AUTHED(vch) &&
               !( vch->pcdata->council &&
                  !str_cmp( vch->pcdata->council->name, "Newbie Council" ) ) ) )
            continue;

        if ( IS_ROOM_FLAG( vch->in_room, ROOM_SILENCE ) )
            continue;

        if ( channel == CHANNEL_GOSSIP &&
             vch->in_room->area != ch->in_room->area &&
             (IS_NPC(ch) || !IS_IMMORTAL(vch)) )
            continue;

        if ( channel == CHANNEL_CLAN || channel == CHANNEL_ORDER ||
             channel == CHANNEL_GUILD )
        {
            if ( IS_NPC( vch ) )
                continue;
            if ( vch->pcdata->clan != ch->pcdata->clan )
                continue;
        }

        if ( channel == CHANNEL_COUNCIL )
        {
            if ( IS_NPC( vch ) )
                continue;
            if ( vch->pcdata->council != ch->pcdata->council )
                continue;
        }

        position		= vch->position;

        if ( channel != CHANNEL_SHOUT && channel != CHANNEL_GOSSIP )
            vch->position	= POS_STANDING;

        if ( speaking != -1 && (!IS_NPC(ch) || ch->speaking) )
        {
            int speakswell = UMIN(knows_language(vch, ch->speaking, ch),
                                  knows_language(ch, ch->speaking, vch));

            if ( speakswell < 85 )
                sbuf = translate(speakswell, argument, lang_names[speaking]);
        }

        sprintf(col, "%s", color_str(AType, vch));

	MOBtrigger = FALSE;

        if (!social)
        {
            char name[MAX_INPUT_LENGTH];

            sprintf(name, "%s", PERS(ch, vch));
            name[0] = toupper(name[0]);

            if (IS_IMMORTAL(vch) && IS_NPC(ch))
                strcpy(buf, ubuf);
            else
                buf[0] = '\0';

            if ( channel == CHANNEL_IMMTALK || channel == CHANNEL_AVTALK ||
                 channel == CHANNEL_HIGHGOD )
            {
                sprintf(buf+strlen(buf), "%s%s::&W%s%s:: '$t%s'%s",
                        mxp_chan_str(vch, verb),
                        col, name, col, col,
                        mxp_chan_str_close(vch, verb));
                act(AT_PLAIN, buf, ch, argument, vch, TO_VICT);
            }
            else
            {
                sprintf(buf+strlen(buf), "%s%s[&W%s%s] %ss '$t%s'%s",
                        mxp_chan_str(vch, verb),
                        col, name, col, verb, col,
                        mxp_chan_str_close(vch, verb));
                act(AT_PLAIN, buf, ch, argument, vch, TO_VICT);
            }
        }
        else
        {
            if (vch==victim)
            {
                sprintf(buf, "%s%s[%s] %s%s",
                        mxp_chan_str(vch, verb),
                        col, verb, socbuf_vict,
                        mxp_chan_str_close(vch, verb));
                act(AT_PLAIN, buf, ch, NULL, vch, TO_VICT);
            }
            else
            {
                sprintf(buf, "%s%s[%s] %s%s",
                        mxp_chan_str(vch, verb),
                        col, verb, socbuf_other,
                        mxp_chan_str_close(vch, verb));
                act(AT_PLAIN, buf, ch, vch, victim, TO_THIRD);
            }

        }
        vch->position	= position;

    }

    if (social && arg && *arg)
        talk_channel(ch,arg,channel,verb);

    return;
}


void to_channel( const char *argument, sh_int log_type, sh_int level, sh_int severity )
{
    char buf[MAX_STRING_LENGTH], logfile[MAX_INPUT_LENGTH];
    int channel;
    char *verb;
    CHAR_DATA *vch;
    struct tm *now_time;

    if ( argument[0] == '\0' )
        return;

    verb    = sysdata.logdefs[log_type].name;
    channel = sysdata.logdefs[log_type].channel;

    sysdata.logdefs[log_type].num_logs++;

    sprintf(buf, "%24.24s %d.%d %s", (char *) ctime( &current_time ), level, severity, uncolorify(argument) );

    now_time = localtime(&current_time);

    sprintf(logfile, CHAN_LOG_DIR "%s.%d-%d",
            verb?verb:"None", now_time->tm_mon+1, now_time->tm_mday);

    append_to_file(logfile, buf);

    sprintf(buf, "%s-%d.%d: %s\r\n", verb?verb:"None", level, severity, uncolorify(argument) );

    for (vch = first_char; vch; vch = vch->next)
    {
        if (IS_NPC(vch))
            continue;

        if ( GET_CON_STATE(vch) != CON_PLAYING ||
             IS_SET(vch->deaf, channel) || get_trust(vch) < level )
            continue;

        if ( (!IS_PLR_FLAG(vch, PLR_AFK) &&
              vch->pcdata->log_severity[log_type] < severity) ||
             (IS_PLR_FLAG(vch, PLR_AFK) &&
              vch->pcdata->afk_log_severity[log_type] < severity) )
            continue;

        if ( get_trust(vch) < sysdata.log_level )
            if ( channel == CHANNEL_LOGPC ||
                 channel == CHANNEL_LOG ||
                 channel == CHANNEL_COMM ||
                 channel == CHANNEL_HIGHGOD ||
                 channel == CHANNEL_BUG ||
                 channel == CHANNEL_DEBUG ||
                 channel == CHANNEL_IMCDEBUG ||
                 channel == CHANNEL_MONITOR ||
                 channel == CHANNEL_HTTPD ||
                 channel == CHANNEL_MAGIC ||
                 channel == CHANNEL_IRC
               )
                continue;

        if ( get_trust(vch) < sysdata.build_level )
            if ( channel == CHANNEL_BUILD )
                continue;

        if ( get_trust( vch ) < sysdata.muse_level )
            if ( channel == CHANNEL_HIGHGOD )
                continue;

        if ( get_trust( vch ) < sysdata.think_level )
            if ( channel == CHANNEL_IMMTALK )
                continue;

        set_char_color( AT_LOG, vch );
        send_to_char( buf, vch );
    }

    return;
}


/*
 void do_auction( CHAR_DATA *ch, char *argument )
 {
 talk_channel( ch, argument, CHANNEL_AUCTION, "auction" );
 return;
 }
 */

void do_clantalk( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if ( IS_NPC( ch ) || !ch->pcdata->clan
         ||   ch->pcdata->clan->clan_type == CLAN_ORDER
         ||   ch->pcdata->clan->clan_type == CLAN_GUILD )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    talk_channel( ch, argument, CHANNEL_CLAN, "clantalk" );
    return;
}

void do_newbiechat( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch )
         || ( !NOT_AUTHED( ch ) && !IS_IMMORTAL(ch)
              && !( ch->pcdata->council &&
                    !str_cmp( ch->pcdata->council->name, "Newbie Council" ) ) ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    talk_channel( ch, argument, CHANNEL_NEWBIE, "newbiechat" );
    return;
}

void do_ordertalk( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if ( IS_NPC( ch ) || !ch->pcdata->clan
         || ch->pcdata->clan->clan_type != CLAN_ORDER )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    talk_channel( ch, argument, CHANNEL_ORDER, "ordertalk" );
    return;
}

void do_counciltalk( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if ( IS_NPC( ch ) || !ch->pcdata->council )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    talk_channel( ch, argument, CHANNEL_COUNCIL, "counciltalk" );
    return;
}

void do_guildtalk( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if ( IS_NPC( ch ) || !ch->pcdata->clan || ch->pcdata->clan->clan_type != CLAN_GUILD )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    talk_channel( ch, argument, CHANNEL_GUILD, "guildtalk" );
    return;
}

void do_music( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    talk_channel( ch, argument, CHANNEL_MUSIC, "music" );
    return;
}


void do_questtalk( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    talk_channel( ch, argument, CHANNEL_QUEST, "quest" );
    return;
}

void do_ask( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    talk_channel( ch, argument, CHANNEL_ASK, "ask" );
    return;
}

void do_answer( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    talk_channel( ch, argument, CHANNEL_ASK, "answer" );
    return;
}

void do_ooc( CHAR_DATA *ch, char *argument )
{
    if (!IS_NPC(ch) && IS_PLR_FLAG(ch, PLR_NO_TELL))
    {
        send_to_char("You can't do that.\n\r", ch);
        return;
    }
    talk_channel( ch, argument , CHANNEL_OOC, "OOC" );
}

void do_shout( CHAR_DATA *ch, char *argument )
{
    if (GET_MOVE(ch) < 10)
    {
        send_to_char("You are too exhausted to shout.\n\r", ch);
        return;
    }
    ch->move -= 10;
    talk_channel( ch, drunk_speech( argument, ch ), CHANNEL_SHOUT, "shout" );
}

void do_gossip( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    if (GetMaxLevel(ch)<5)
        send_to_char("Note: Gossip is only area-wide!\n\r(This message will self-destruct when you reach level 5)\n\r", ch);

    talk_channel( ch, drunk_speech( argument, ch ), CHANNEL_GOSSIP, "gossip");
    return;
}

void do_muse( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    talk_channel( ch, argument, CHANNEL_HIGHGOD, "muse" );
    return;
}


void do_think( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_IMMTALK, "think" );
    return;
}


void do_avtalk( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    talk_channel( ch, drunk_speech( argument, ch ), CHANNEL_AVTALK, "avtalk" );
    return;
}


void do_say( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *vch;
    int actflags;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Say what?\n\r", ch );
        return;
    }

    if (!IS_IMMORTAL(ch))
        if ( IS_ROOM_FLAG( ch->in_room, ROOM_SILENCE ) ||
             ch->in_room->sector_type == SECT_UNDERWATER )
        {
            send_to_char( "You can't do that here.\n\r", ch );
            return;
        }

    actflags = ch->act;
    if ( IS_NPC( ch ) ) REMOVE_BIT( ch->act, ACT_SECRETIVE );
    for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
    {
        char *sbuf = argument;
        int speaking = -1, lang;

        if ( vch == ch )
            continue;

#if 1
        for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
            if ( ch->speaking & lang_array[lang] )
            {
                speaking = lang;
                break;
            }
        if ( speaking != -1 && (!IS_NPC(ch) || ch->speaking) )
        {
            int speakswell = UMIN(knows_language(vch, ch->speaking, ch),
                                  knows_language(ch, ch->speaking, vch));

            if ( speakswell < 75 )
                sbuf = translate(speakswell, argument, lang_names[speaking]);
        }
#else
        if ( !knows_language(vch, ch->speaking, ch) &&
             (!IS_NPC(ch) || ch->speaking != 0) )
            sbuf = text_scramble(argument, ch->speaking);
#endif

        sbuf = drunk_speech( sbuf, ch );

        MOBtrigger = FALSE;
        if (sbuf != argument && ch->speaking != vch->speaking)
            sprintf(buf, "%s[&p$n%s] says in another language '$t%s'",
                    color_str(AT_SAY, vch), color_str(AT_SAY, vch), color_str(AT_SAY, vch));
        else
            sprintf(buf, "%s[&p$n%s] says '$t%s'",
                    color_str(AT_SAY, vch), color_str(AT_SAY, vch), color_str(AT_SAY, vch));
        act( AT_SAY, buf, ch, sbuf, vch, TO_VICT );
    }
    ch->act = actflags;
    MOBtrigger = FALSE;
    sprintf(buf, "%sYou say '$T%s'", color_str(AT_SAY, ch), color_str(AT_SAY, ch));
    act( AT_SAY, buf, ch, NULL, drunk_speech( argument, ch ), TO_CHAR );
    if ( IS_ROOM_FLAG( ch->in_room, ROOM_LOGSPEECH ) )
    {
        char roomlog[80];

        snprintf( roomlog, 80, ROOM_LOG_DIR "%d.log", ch->in_room->vnum );
        sprintf( log_buf, "%-16.16s %-10.10s %s",
                 IS_NPC( ch ) ? ch->short_descr : ch->name,
                 "say",
                 argument );

        append_to_file( roomlog, log_buf );
    }
    mprog_speech_trigger( argument, ch );
    if ( char_died(ch) )
        return;
    oprog_speech_trigger( argument, ch );
    if ( char_died(ch) )
        return;
    rprog_speech_trigger( argument, ch );
    return;
}


void do_whisper( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim, *tch;
    int position;
    CHAR_DATA *switched_victim = NULL;
    int speaking = -1, lang;

    for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
	if ( ch->speaking & lang_array[lang] )
	{
	    speaking = lang;
	    break;
	}

    REMOVE_BIT( ch->deaf, CHANNEL_WHISPER );
    if ( IS_ROOM_FLAG( ch->in_room, ROOM_SILENCE ) )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    if (IS_SILENCED(ch))
    {
        send_to_char( "You can't do that.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "Whisper what to whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL  ||
         (!NOT_AUTHED(ch) && NOT_AUTHED(victim) && !IS_IMMORTAL(ch) ) )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( ch == victim )
    {
        send_to_char( "You whisper something to yourself.\n\r", ch );
        return;
    }

    if (NOT_AUTHED(ch) && !NOT_AUTHED(victim) && !IS_IMMORTAL(victim) )
    {
        send_to_char( "They can't hear you because you are not authorized.\n\r", ch);
        return;
    }

    if ( !IS_NPC( victim ) &&
         victim->switched &&
         IS_IMMORTAL(victim) &&
         !IS_ACT_FLAG(victim->switched, ACT_POLYMORPHED) &&
         !IS_AFFECTED(victim->switched, AFF_POSSESS) )
    {
        send_to_char( "That player is switched.\n\r", ch );
        return;
    }
    else if ( !IS_NPC( victim ) &&
              victim->switched &&
              (IS_ACT_FLAG(victim->switched, ACT_POLYMORPHED) ||
               IS_AFFECTED(victim->switched, AFF_POSSESS) ) )
    {
        switched_victim = victim->switched;
    }
    else if ( !IS_NPC( victim ) && !victim->desc )
    {
        send_to_char( "That player is link-dead.\n\r", ch );
        return;
    }

    if ( IS_SET( victim->deaf, CHANNEL_WHISPER ) &&
         ( !IS_IMMORTAL( ch ) || ( get_trust( ch ) < get_trust( victim ) ) ) )
    {
        act( AT_PLAIN, "$E has $S whisper channel turned off.", ch, NULL, victim,
             TO_CHAR );
        return;
    }

    if (IS_SILENCED(victim))
    {
        send_to_char( "That player is silenced.  They will receive your message but can not respond.\n\r", ch );
    }

    if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
    {
        act( AT_PLAIN, "$E can't hear you.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( victim->desc		/* make sure desc exists first  -Thoric */
         &&   victim->desc->connected == CON_EDITING
         &&   get_trust(ch) < LEVEL_GOD )
    {
        act( AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR );
        return;
    }

    if (switched_victim)
        victim = switched_victim;

    act( AT_TELL, "$n whispers something to $N.", ch, NULL, victim, TO_ROOM );

    sprintf(buf,"You whisper to $N '$t%s'",color_str(AT_TELL,ch));
    act( AT_TELL, buf, ch, argument, victim, TO_CHAR );

    position		= victim->position;
    victim->position	= POS_STANDING;
    sprintf(buf,"[%s$n%s] whispers '$t%s'",
            color_str(AT_WHITE,victim),color_str(AT_TELL,victim),color_str(AT_TELL,victim));

    if ( speaking != -1 && (!IS_NPC(ch) || ch->speaking) )
    {
	int speakswell = UMIN(knows_language(victim, ch->speaking, ch),
			      knows_language(ch, ch->speaking, victim));

	if ( speakswell < 85 )
            act( AT_TELL, buf, ch,
                 translate(speakswell, argument, lang_names[speaking]), victim, TO_VICT );
        else
            act( AT_TELL, buf, ch, argument, victim, TO_VICT );
    }
    else
        act( AT_TELL, buf, ch, argument, victim, TO_VICT );

    victim->position	= position;
    victim->reply	= ch;

    for (tch = ch->in_room->first_person; tch; tch = tch->next_in_room)
    {
        if (!is_affected(tch, skill_lookup("eavesdrop")))
            continue;

        if ( speaking != -1 && (!IS_NPC(ch) || ch->speaking) )
        {
            int speakswell = UMIN(knows_language(tch, ch->speaking, ch),
                                  knows_language(ch, ch->speaking, tch));

            if ( speakswell < 85 )
                sprintf(buf, "%sYou overhear $n whispering to $N '%s%s'",
                        color_str(AT_TELL, tch),
                        translate(speakswell, argument, lang_names[speaking]),
                        color_str(AT_TELL,tch));
            else
                sprintf(buf, "%sYou overhear $n whispering to $N '%s%s'",
                        color_str(AT_TELL, tch),
                        argument,
                        color_str(AT_TELL,tch));
        }
        else
            sprintf(buf, "%sYou overhear $n whispering to $N '%s%s'",
                    color_str(AT_TELL, tch),
                    argument,
                    color_str(AT_TELL,tch));

        act( AT_TELL, buf, ch, tch, victim, TO_THIRD );
    }

    if ( IS_ROOM_FLAG( ch->in_room, ROOM_LOGSPEECH ) )
    {
        char roomlog[80];

        snprintf( roomlog, 80, ROOM_LOG_DIR "%d.log", ch->in_room->vnum );
        sprintf( log_buf, "%-16.16s %-10.10s %s (to %s)",
                 IS_NPC( ch ) ? ch->short_descr : ch->name,
                 "whisper",
                 argument,
                 IS_NPC( victim ) ? victim->short_descr : victim->name );

        append_to_file( roomlog, log_buf );
    }
    return;
}


void do_tell( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int position;
    CHAR_DATA *switched_victim = NULL;
    int speaking = -1, lang;

    for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
        if ( ch->speaking & lang_array[lang] )
	{
	    speaking = lang;
	    break;
	}

    REMOVE_BIT( ch->deaf, CHANNEL_TELLS );
    if ( IS_ROOM_FLAG( ch->in_room, ROOM_SILENCE ) )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    if (IS_SILENCED(ch) ||
        (!IS_NPC(ch) && IS_PLR_FLAG(ch, PLR_NO_TELL)))
    {
        send_to_char( "You can't do that.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "Tell whom what?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL  ||
         ( IS_NPC(victim) && victim->in_room != ch->in_room )  ||
         (!NOT_AUTHED(ch) && NOT_AUTHED(victim) && !IS_IMMORTAL(ch) ) )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( ch == victim )
    {
        send_to_char( "You have a nice little chat with yourself.\n\r", ch );
        return;
    }

    if (NOT_AUTHED(ch) && !NOT_AUTHED(victim) && !IS_IMMORTAL(victim) )
    {
        send_to_char( "They can't hear you because you are not authorized.\n\r", ch);
        return;
    }

    if ( !IS_NPC( victim ) && ( victim->switched ) &&
         ( get_trust( ch ) > LEVEL_AVATAR ) &&
         !IS_ACT_FLAG(victim->switched, ACT_POLYMORPHED) &&
         !IS_AFFECTED(victim->switched, AFF_POSSESS) )
    {
        send_to_char( "That player is switched.\n\r", ch );
        return;
    }
    else if ( !IS_NPC( victim ) && ( victim->switched ) &&
              (IS_ACT_FLAG(victim->switched, ACT_POLYMORPHED) ||
               IS_AFFECTED(victim->switched, AFF_POSSESS) ) )
    {
        switched_victim = victim->switched;
    }
    else if ( !IS_NPC( victim ) && ( !victim->desc ) )
    {
        send_to_char( "That player is link-dead.\n\r", ch );
        return;
    }

    if ( IS_SET( victim->deaf, CHANNEL_TELLS )
         && ( !IS_IMMORTAL( ch ) || ( get_trust( ch ) < get_trust( victim ) ) ) )
    {
        act( AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim,
             TO_CHAR );
        return;
    }

    if (IS_SILENCED(victim))
        send_to_char( "That player is silenced.  They will receive your message but can not respond.\n\r", ch );

    if ( (!IS_IMMORTAL(ch) && !IS_AWAKE(victim) )
         || (!IS_NPC(victim) && IS_ROOM_FLAG(victim->in_room, ROOM_SILENCE ) ) )
    {
        act( AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR );
        return;
    }

    if ( victim->desc		/* make sure desc exists first  -Thoric */
         &&   victim->desc->connected == CON_EDITING
         &&   get_trust(ch) < LEVEL_GOD )
    {
        act( AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR );
        return;
    }

    if(switched_victim)
        victim = switched_victim;


    sprintf(buf,"You tell $N '$t%s'",color_str(AT_TELL,ch));
    act( AT_TELL, buf, ch, argument, victim, TO_CHAR );
    position		= victim->position;
    victim->position	= POS_STANDING;

    sprintf(buf,"[%s$n%s] tells you '$t%s'",
            color_str(AT_WHITE,victim),color_str(AT_TELL,victim),color_str(AT_TELL,victim));
    if ( speaking != -1 && (!IS_NPC(ch) || ch->speaking) )
    {
	int speakswell = UMIN(knows_language(victim, ch->speaking, ch),
			      knows_language(ch, ch->speaking, victim));

	if ( speakswell < 85 )
	    act( AT_TELL, buf, ch, translate(speakswell, argument, lang_names[speaking]), victim, TO_VICT );
	else
	    act( AT_TELL, buf, ch, argument, victim, TO_VICT );
    }
    else
	act( AT_TELL, buf, ch, argument, victim, TO_VICT );

    victim->position	= position;
    victim->reply	= ch;
    if ( IS_ROOM_FLAG( ch->in_room, ROOM_LOGSPEECH ) )
    {
        char roomlog[80];

        snprintf( roomlog, 80, ROOM_LOG_DIR "%d.log", ch->in_room->vnum );
        sprintf( log_buf, "%-16.16s %-10.10s %s (to %s)",
                 IS_NPC( ch ) ? ch->short_descr : ch->name,
                 "tell",
                 argument,
                 IS_NPC( victim ) ? victim->short_descr : victim->name );

        append_to_file( roomlog, log_buf );

        if (ch->in_room != victim->in_room)
        {
            snprintf( roomlog, 80, ROOM_LOG_DIR "%d.log", victim->in_room->vnum );
            sprintf( log_buf, "%-16.16s %-10.10s %s (to %s)",
                     IS_NPC( ch ) ? ch->short_descr : ch->name,
                     "tell",
                     argument,
                     IS_NPC( victim ) ? victim->short_descr : victim->name );

            append_to_file( roomlog, log_buf );
        }

    }
    mprog_speech_trigger( argument, ch );
    return;
}



void do_reply( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int position;
    int speaking = -1, lang;

    for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
	if ( ch->speaking & lang_array[lang] )
	{
	    speaking = lang;
	    break;
	}

    REMOVE_BIT( ch->deaf, CHANNEL_TELLS );
    if ( IS_ROOM_FLAG( ch->in_room, ROOM_SILENCE ) )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    if ( ( victim = ch->reply ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if (!IS_NPC(ch))
    {
        if (IS_SILENCED(ch))
        {
            send_to_char( "Your message didn't get through.\n\r", ch );
            return;
        }

        if ( victim->switched
             && can_see( ch, victim ) && ( get_trust( ch ) > LEVEL_AVATAR ) )
        {
            send_to_char( "That player is switched.\n\r", ch );
            return;
        }
        else if ( !victim->desc )
        {
            send_to_char( "That player is link-dead.\n\r", ch );
            return;
        }

        if ( IS_PLR_FLAG (victim, PLR_AFK ) )
        {
            send_to_char( "That player is afk.\n\r", ch );
            return;
        }
    }
    if ( IS_SET( victim->deaf, CHANNEL_TELLS )
         && ( !IS_IMMORTAL( ch ) || ( get_trust( ch ) < get_trust( victim ) ) ) )
    {
        act( AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim,
             TO_CHAR );
        return;
    }

    if ( ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) ) ||
         ( !IS_NPC(victim) && IS_ROOM_FLAG( victim->in_room, ROOM_SILENCE ) ) )
    {
        act( AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR );
        return;
    }

    sprintf(buf,"You tell $N '$t%s'",color_str(AT_TELL,ch));
    act( AT_TELL, buf, ch, argument, victim, TO_CHAR );
    position		= victim->position;
    victim->position	= POS_STANDING;

    sprintf(buf,"[%s$n%s] tells you '$t%s'",
            color_str(AT_WHITE,victim),color_str(AT_TELL,victim),color_str(AT_TELL,victim));
    if ( speaking != -1 && (!IS_NPC(ch) || ch->speaking) )
    {
	int speakswell = UMIN(knows_language(victim, ch->speaking, ch),
			      knows_language(ch, ch->speaking, victim));

	if ( speakswell < 85 )
	    act( AT_TELL, buf, ch, translate(speakswell, argument, lang_names[speaking]), victim, TO_VICT );
	else
	    act( AT_TELL, buf, ch, argument, victim, TO_VICT );
    }
    else
	act( AT_TELL, buf, ch, argument, victim, TO_VICT );

    victim->position	= position;
    victim->reply	= ch;
    if ( IS_ROOM_FLAG( ch->in_room, ROOM_LOGSPEECH ) )
    {
        char roomlog[80];

        snprintf( roomlog, 80, ROOM_LOG_DIR "%d.log", ch->in_room->vnum );
        sprintf( log_buf, "%-16.16s %-10.10s %s (to %s)",
                 IS_NPC( ch ) ? ch->short_descr : ch->name,
                 "reply",
                 argument,
                 IS_NPC( victim ) ? victim->short_descr : victim->name );

        append_to_file( roomlog, log_buf );
        if (ch->in_room != victim->in_room)
        {
            snprintf( roomlog, 80, ROOM_LOG_DIR "%d.log", victim->in_room->vnum );
            sprintf( log_buf, "%-16.16s %-10.10s %s (to %s)",
                     IS_NPC( ch ) ? ch->short_descr : ch->name,
                     "reply",
                     argument,
                     IS_NPC( victim ) ? victim->short_descr : victim->name );

            append_to_file( roomlog, log_buf );
        }
    }

    return;
}



void do_emote( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;
    CHAR_DATA *vch;
    int actflags;
    int speaking = -1, lang;

    for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
	if ( ch->speaking & lang_array[lang] )
	{
	    speaking = lang;
	    break;
	}

    if ( !IS_NPC(ch) && IS_PLR_FLAG(ch, PLR_NO_EMOTE) )
    {
        send_to_char( "You can't show your emotions.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Emote what?\n\r", ch );
        return;
    }

    actflags = ch->act;
    if ( IS_NPC( ch ) ) REMOVE_BIT( ch->act, ACT_SECRETIVE );
    for ( plast = argument; *plast != '\0'; plast++ )
        ;

    strcpy( buf, argument );
    if ( isalpha(plast[-1]) )
        strcat( buf, "." );
    for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
    {
        char *sbuf = buf;

        if ( speaking != -1 && (!IS_NPC(ch) || ch->speaking) )
        {
            int speakswell = UMIN(knows_language(vch, ch->speaking, ch),
                                  knows_language(ch, ch->speaking, vch));

            if ( speakswell < 85 )
                sbuf = translate(speakswell, buf, lang_names[speaking]);
        }
        MOBtrigger = FALSE;
        act( AT_ACTION, "$n $t", ch, sbuf, vch, (vch == ch ? TO_CHAR : TO_VICT) );
    }
    /*    MOBtrigger = FALSE;
     act( AT_ACTION, "$n $T", ch, NULL, buf, TO_ROOM );
     MOBtrigger = FALSE;
     act( AT_ACTION, "$n $T", ch, NULL, buf, TO_CHAR );*/
    ch->act = actflags;
    if ( IS_ROOM_FLAG( ch->in_room, ROOM_LOGSPEECH ) )
    {
        char roomlog[80];

        snprintf( roomlog, 80, ROOM_LOG_DIR "%d.log", ch->in_room->vnum );
        sprintf( log_buf, "%-16.16s %-10.10s %s",
                 IS_NPC( ch ) ? ch->short_descr : ch->name,
                 "emote",
                 argument );

        append_to_file( roomlog, log_buf );
    }
    return;
}


void do_bug( CHAR_DATA *ch, char *argument )
{
    if (!argument || !*argument)
    {
        if (IS_IMMORTAL(ch))
        {
            show_file(ch, USER_BUG_FILE);
            return;
        }
        send_to_char("Bug what?\n\r", ch);
        return;
    }

    append_file( ch, USER_BUG_FILE, argument );
    log_printf_plus(LOG_BUG, LEVEL_IMMORTAL, SEV_NOTICE, "%s: %s", GET_NAME(ch), argument);
    send_to_char( "Ok.  Thanks.\n\r", ch );
    return;
}


void do_ide( CHAR_DATA *ch, char *argument )
{
    send_to_char("If you want to send an idea, type 'idea <message>'.\n\r", ch);
    send_to_char("If you want to identify an object and have the identify spell,\n\r", ch);
    send_to_char("Type 'cast identify <object>'.\n\r", ch);
    return;
}

void do_idea( CHAR_DATA *ch, char *argument )
{
    if (!argument || !*argument)
    {
        if (IS_IMMORTAL(ch))
        {
            show_file(ch, IDEA_FILE);
            return;
        }
        send_to_char("Idea what?\n\r", ch);
        return;
    }

    append_file( ch, IDEA_FILE, argument );
    send_to_char( "Ok.  Thanks.\n\r", ch );
    return;
}

void do_typo( CHAR_DATA *ch, char *argument )
{
    if (!argument || !*argument)
    {
        if (IS_IMMORTAL(ch))
        {
            show_file(ch, TYPO_FILE);
            return;
        }
        send_to_char("Typo what?\n\r", ch);
        return;
    }

    append_file( ch, TYPO_FILE, argument );
    send_to_char( "Ok.  Thanks.\n\r", ch );
    return;
}


void do_qui( CHAR_DATA *ch, char *argument )
{
    send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
    send_to_char( "It's a better idea to rent, if you want to keep your equipment.\n\r", ch );
    return;
}

void do_quit( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj, *next_obj;

    if (!IS_IMMORTAL(ch) && GetMaxLevel(ch)>1)
    {
        if (!argument || str_cmp(argument, "yes"))
        {
            send_to_char("Quiting does not save your char.\n\r", ch);
            send_to_char("If you don't wish to lose your eq, rent at an inn.\n\r", ch);
            send_to_char("Type 'quit yes' to overide.\n\r", ch);
            return;
        }
    }

    if ( IS_NPC(ch) && IS_ACT_FLAG(ch, ACT_POLYMORPHED))
    {
        send_to_char("You can't quit while polymorphed.\n\r", ch);
        return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
        return;

    if ( IS_NPC(ch) )
        return;

    if ( ch->position == POS_FIGHTING )
    {
        send_to_char( "No way! You are fighting.\n\r", ch );
        return;
    }

    if ( ch->position  < POS_STUNNED  )
    {
        send_to_char( "You're not DEAD yet.\n\r", ch );
        return;
    }

    if ( auction->item != NULL && ((ch == auction->buyer) || (ch == auction->seller) ) )
    {
        send_to_char("Wait until you have bought/sold the item on auction.\n\r", ch);
        return;
    }

    stop_memorizing(ch);

    send_to_char( "Your surroundings begin to fade as a mystical swirling vortex of colors\n\renvelops your body... When you come to, things are not as they were.\n\r\n\r", ch );
    act( AT_PLAIN, "$n has left the game.", ch, NULL, NULL, TO_ROOM );

    log_printf_plus( LOG_COMM, GetMaxLevel(ch), SEV_NOTICE, "%s has quit (played for %s).",
                     GET_NAME(ch), sec_to_hms_short(current_time-ch->logon) );

    if (ch->timer<=24)
    {
        for (obj = ch->first_carrying; obj; obj = next_obj) {
            next_obj = obj->next_content;
            if (obj->wear_loc != WEAR_NONE)
                remove_obj(ch, obj->wear_loc, TRUE);
            obj_from_char(obj);
            obj_to_room(obj, ch->in_room);
        }
    }

    quitting_char = ch;
    save_char_obj( ch );
    saving_char = NULL;
    extract_char( ch, TRUE );
}

void do_rent( CHAR_DATA *ch, char *argument )
{
    int rent = 0, tax = 0, type;
    CHAR_DATA *fch = NULL, *next = NULL;

    if ( !ch )
        return;

    if ( !IS_ROOM_FLAG( ch->in_room, ROOM_RECEPTION ) )
    {
        send_to_char("Sorry, but you can't do that here.\n\r", ch);
        return;
    }

    if ( IS_NPC(ch) && IS_ACT_FLAG(ch, ACT_POLYMORPHED))
    {
        send_to_char("You can't rent while polymorphed.\n\r", ch);
        return;
    }

    if ( IS_NPC(ch) )
        return;

    if ( ch->position == POS_FIGHTING )
    {
        send_to_char( "No way! You are fighting.\n\r", ch );
        return;
    }

    if ( ch->position  < POS_STUNNED  )
    {
        send_to_char( "You're not DEAD yet.\n\r", ch );
        return;
    }

    if ( auction->item != NULL && ((ch == auction->buyer) || (ch == auction->seller) ) )
    {
        send_to_char("Wait until you have bought/sold the item on auction.\n\r", ch);
        return;
    }

    stop_memorizing(ch);

    type = get_primary_curr(ch->in_room);
    rent = calc_rent(ch);
    tax  = property_tax(ch->in_room,rent,PROFIT_RENT);
    if (is_house_owner(ch, ch->in_room))
    {
        rent/=2;
        tax=0;
        send_to_char("Since you own a house, your rent is halved.\n\r", ch);
    }

    ch_printf(ch, "It will cost you %d coins per day to rent.\n\r",
              rent);
    ch_printf(ch, "It will cost you %d coins up front in taxes.\n\r",
              tax);

    if (GET_BALANCE(ch,type)+GET_MONEY(ch,type) < rent+tax)
    {
        send_to_char("You don't have enough money.\n\r",ch);
        return;
    }

    if (GET_BALANCE(ch,type) < tax)
    {
        tax -= ch->balance[type];
        ch->balance[type] = 0;
    }
    else
    {
        ch->balance[type] -= tax;
        tax = 0;
    }
    ch->money[type] -= tax;

    property_add_tax(ch->in_room,rent,type,PROFIT_RENT);

    send_to_char( "Your surroundings begin to fade as a mystical swirling vortex of colors\n\renvelops your body... When you come to, things are not as they were.\n\r\n\r", ch );
    act( AT_BYE, "$n has left the game.", ch, NULL, NULL, TO_ROOM );
    set_char_color( AT_GREY, ch);

    log_printf_plus( LOG_COMM, GetMaxLevel(ch), SEV_NOTICE,
                     "%s has rented (played for %s).",
                     ch->name, sec_to_hms_short(current_time-ch->logon) );

    ch->pcdata->home = ch->in_room->vnum;

    quitting_char = ch;
    save_char_obj( ch );
    saving_char = NULL;
    for (fch = ch->in_room->first_person; fch; fch = next)
    {
        next = fch->next_in_room;
        if (IS_NPC(fch) && fch != ch && fch->master == ch)
        {
            act( AT_BYE, "$N follows $n into $s room.", ch, NULL, fch, TO_ROOM );
            stop_follower(fch);
            extract_char( fch, TRUE);
        }
    }
    extract_char( ch, TRUE );
}


void do_offer( CHAR_DATA *ch, char *argument )
{
    int rent = 0, tax, type;

    if (!ch)
        return;

    if (!IS_ROOM_FLAG( ch->in_room, ROOM_RECEPTION))
    {
        send_to_char("Sorry, but you can't do that here.\n\r", ch);
        return;
    }

    if (IS_NPC(ch) && IS_ACT_FLAG(ch, ACT_POLYMORPHED))
    {
        send_to_char("You can't rent while polymorphed.\n\r", ch);
        return;
    }

    if (IS_NPC(ch))
        return;

    if (ch->position == POS_FIGHTING)
    {
        send_to_char("No way! You are fighting.\n\r", ch);
        return;
    }

    if (ch->position  < POS_STUNNED)
    {
        send_to_char("You're not DEAD yet.\n\r", ch);
        return;
    }

    type = get_primary_curr(ch->in_room);
    if (!(rent = calc_rent(ch)))
    {
        send_to_char("You have no rent.\n\r", ch);
        return;
    }

    tax  = property_tax(ch->in_room,rent,PROFIT_RENT);

    if (is_house_owner(ch, ch->in_room))
    {
        rent/=2;
        tax=0;
    }

    show_rent_list(ch);

    ch_printf(ch,
              "---------------------------------------------------------------\n\r"
              "%-40.40s %6d coins\n\r"
              "%-40.40s %6d coins/day\n\r",
              "Tax", tax,
              "Total", rent+tax);

    if (GET_BALANCE(ch,type)+GET_MONEY(ch,type) < rent+tax)
        ch_printf(ch, "You do not have enough %s.\n\r", curr_types[type]);
}


void send_rip_screen( CHAR_DATA *ch )
{
    FILE *rpfile;
    int num=0;
    char BUFF[MAX_STRING_LENGTH*2];

    if ((rpfile = fopen(RIPSCREEN_FILE,"r")) !=NULL) {
        while ((BUFF[num]=fgetc(rpfile)) != EOF)
            num++;
        fclose(rpfile);
        BUFF[num] = 0;
        send_to_char(BUFF, ch);
    }
}

void send_rip_title( CHAR_DATA *ch )
{
    FILE *rpfile;
    int num=0;
    char BUFF[MAX_STRING_LENGTH*2];

    if ((rpfile = fopen(RIPTITLE_FILE,"r")) !=NULL) {
        while ((BUFF[num]=fgetc(rpfile)) != EOF)
            num++;
        fclose(rpfile);
        BUFF[num] = 0;
        send_to_char(BUFF, ch);
    }
}

void send_ansi_title( CHAR_DATA *ch )
{
    FILE *rpfile;
    int num=0;
    char BUFF[MAX_STRING_LENGTH*2];

    if ((rpfile = fopen(ANSITITLE_FILE,"r")) !=NULL) {
        while ((BUFF[num]=fgetc(rpfile)) != EOF)
            num++;
        fclose(rpfile);
        BUFF[num] = 0;
        send_to_char(BUFF, ch);
    }
}

void send_ascii_title( CHAR_DATA *ch )
{
    FILE *rpfile;
    int num=0;
    char BUFF[MAX_STRING_LENGTH];

    if ((rpfile = fopen(ASCTITLE_FILE,"r")) !=NULL) {
        while ((BUFF[num]=fgetc(rpfile)) != EOF)
            num++;
        fclose(rpfile);
        BUFF[num] = 0;
        send_to_char(BUFF, ch);
    }
}

#ifdef IBUILD
void do_omenu( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: omenu <object> [page]  \n\r",     ch );
        send_to_char( "      Where:    <object> is a prototype object  \n\r",     ch );
        send_to_char( "            and  <page>  is an optional letter to select menu-pages\n\r",     ch );
        return;
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    /* can redit or something */

    ch->inter_type = OBJ_TYPE;
    ch->inter_substate = SUB_NORTH;
    if( ch->inter_editing != NULL) DISPOSE(ch->inter_editing);
    ch->inter_editing      =  str_dup(obj->pIndexData->name);
    sscanf(ch->inter_editing,"%s",ch->inter_editing);  /*one-arg*/
    ch->inter_editing_vnum =  obj->pIndexData->vnum;
    send_obj_page_to_char(ch, obj->pIndexData, arg2[0]);
}


void do_rmenu( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *idx;
    char arg1[MAX_INPUT_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );

    idx = ch->in_room;
    /* can redit or something */

    ch->inter_type = ROOM_TYPE;
    ch->inter_substate = SUB_NORTH;
    if( ch->inter_editing != NULL) DISPOSE(ch->inter_editing);
    ch->inter_editing      =  str_dup(idx->name);
    sscanf(ch->inter_editing,"%s",ch->inter_editing);  /*one-arg*/
    ch->inter_editing_vnum =  idx->vnum;
    send_room_page_to_char(ch, idx, arg1[0]);
}

void do_cmenu( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );

    ch->inter_type = CONTROL_TYPE;
    if( ch->inter_editing != NULL) DISPOSE(ch->inter_editing);
    ch->inter_editing      =  str_dup("Control Panel");
    sscanf(ch->inter_editing,"%s",ch->inter_editing);  /*one-arg*/
    send_control_page_to_char(ch, arg1[0]);
}


void do_mmenu( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: mmenu <victim> [page]  \n\r",     ch );
        send_to_char( "      Where:    <victim> is a prototype mob  \n\r",     ch );
        send_to_char( "            and  <page>  is an optional letter to select menu-pages\n\r",     ch );
        return;
    }


    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( !IS_NPC(victim) )
    {
        send_to_char( "Not on players.\n\r", ch );
        return;
    }

    if ( get_trust( ch ) < GetMaxLevel(victim) )
    {
        set_char_color( AT_IMMORT, ch );
        send_to_char( "Their godly glow prevents you from getting a good look .\n\r", ch );
        return;
    }
    ch->inter_type = MOB_TYPE;
    if( ch->inter_editing != NULL) DISPOSE(ch->inter_editing);
    ch->inter_editing      =  str_dup(arg1);
    sscanf(ch->inter_editing,"%s",ch->inter_editing);  /*one-arg*/
    ch->inter_editing_vnum =  victim->pIndexData->vnum;
    send_page_to_char(ch, victim->pIndexData, arg2[0]);
}
#endif

void do_rip( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Rip ON or OFF?\n\r", ch );
        return;
    }
    if ( !str_cmp(arg,"on") ) {
        send_rip_screen(ch);
        SET_BIT(ch->act,PLR_RIP);
        SET_BIT(ch->act,PLR_ANSI);
        return;
    }

    if ( !str_cmp(arg,"off") ) {
        REMOVE_BIT(ch->act,PLR_RIP);
        send_to_char( "!|*\n\rRIP now off...\n\r", ch );
        return;
    }
}

void do_ansi( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "ANSI ON or OFF?\n\r", ch );
        return;
    }
    if ( !str_cmp(arg,"on") ) {
        SET_BIT(ch->act,PLR_ANSI);
        set_char_color( AT_WHITE + AT_BLINK, ch);
        send_to_char( "ANSI ON!!!\n\r", ch);
        return;
    }

    if ( !str_cmp(arg,"off") ) {
        REMOVE_BIT(ch->act,PLR_ANSI);
        send_to_char( "Okay... ANSI support is now off\n\r", ch );
        return;
    }
}

void do_save( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) && IS_ACT_FLAG(ch, ACT_POLYMORPHED))
    {
        send_to_char("You can't save while polymorphed.\n\r", ch);
        return;
    }

    if ( IS_NPC(ch) )
        return;

    if ( GetMaxLevel(ch) < 2)
    {
        send_to_char("You can't save until level 2.\n\r", ch);
        return;
    }

    if ( !IS_SET( ch->affected_by, race_table[ch->race].affected ) )
        SET_BIT( ch->affected_by, race_table[ch->race].affected );
    if ( !IS_SET( ch->resistant, race_table[ch->race].resist ) )
        SET_BIT( ch->resistant, race_table[ch->race].resist );
    if ( !IS_SET( ch->susceptible, race_table[ch->race].suscept ) )
        SET_BIT( ch->susceptible, race_table[ch->race].suscept );

    if ( ch->pcdata->deity )
    {
        if ( !IS_SET( ch->affected_by, ch->pcdata->deity->affected ) )
            SET_BIT( ch->affected_by, ch->pcdata->deity->affected );
        if ( !IS_SET( ch->resistant, ch->pcdata->deity->element ) )
            SET_BIT( ch->resistant, ch->pcdata->deity->element );
        if ( !IS_SET( ch->susceptible, ch->pcdata->deity->suscept ) )
            SET_BIT( ch->susceptible, ch->pcdata->deity->suscept );
    }
    save_char_obj( ch );
    saving_char = NULL;
    send_to_char( "Ok.\n\r", ch );
    return;
}


/*
 * Something from original DikuMUD that Merc yanked out.
 * Used to prevent following loops, which can cause problems if people
 * follow in a loop through an exit leading back into the same room
 * (Which exists in many maze areas)			-Thoric
 */
bool circle_follow( CHAR_DATA *ch, CHAR_DATA *victim )
{
    CHAR_DATA *tmp;

    for ( tmp = victim; tmp; tmp = tmp->master )
        if ( tmp == ch )
            return TRUE;
    return FALSE;
}


void do_follow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Follow whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master )
    {
        act( AT_PLAIN, "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
        return;
    }

    if ( victim == ch )
    {
        if ( !ch->master )
        {
            send_to_char( "You already follow yourself.\n\r", ch );
            return;
        }
        stop_follower( ch );
        return;
    }
    /*
     if ( ( (GetMaxLevel(ch) - GetMaxLevel(victim) < -10)
     || ( GetMaxLevel(ch) - GetMaxLevel(victim) > 10 ) )
     &&   !IS_HERO(ch) )
     {
     send_to_char( "You are not of the right caliber to follow.\n\r", ch );
     return;
     }
     */
    if ( circle_follow( ch, victim ) )
    {
        send_to_char( "Following in loops is not allowed... sorry.\n\r", ch );
        return;
    }

    if ( ch->master )
        stop_follower( ch );

    add_follower( ch, victim );
    return;
}


bool too_many_followers(CHAR_DATA *ch)
{
    CHAR_DATA *vch;
    int max_followers, actual_fol = 0;

    max_followers = cha_app[get_curr_cha(ch)].charm;

    for (vch = first_char; vch; vch = vch->next)
        if (vch->master == ch && IS_AFFECTED(vch, AFF_CHARM))
            actual_fol++;

    if (actual_fol < max_followers)
        return FALSE;

    return TRUE;
}

void add_follower( CHAR_DATA *ch, CHAR_DATA *master )
{
    if ( ch->master )
    {
        bug( "Add_follower: non-null master." );
        return;
    }

    ch->master        = master;
    ch->leader        = NULL;

    if ( can_see( master, ch ) )
        act( AT_ACTION, "$n now follows you.", ch, NULL, master, TO_VICT );

    act( AT_ACTION, "You now follow $N.",  ch, NULL, master, TO_CHAR );

    return;
}


void stop_follower( CHAR_DATA *ch )
{
    if ( !ch->master )
    {
        bug( "Stop_follower: null master." );
        return;
    }

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
        REMOVE_BIT( ch->affected_by, AFF_CHARM );
        affect_strip( ch, gsn_charm_person );
    }

    if ( can_see( ch->master, ch ) )
        act( AT_ACTION, "$n stops following you.",     ch, NULL, ch->master, TO_VICT    );
    act( AT_ACTION, "You stop following $N.",      ch, NULL, ch->master, TO_CHAR    );

    ch->master = NULL;
    ch->leader = NULL;
    return;
}



void die_follower( CHAR_DATA *ch )
{
    CHAR_DATA *fch;

    if ( ch->master )
        stop_follower( ch );

    ch->leader = NULL;

    for ( fch = first_char; fch; fch = fch->next )
    {
        if ( fch->master == ch )
            stop_follower( fch );
        if ( fch->leader == ch )
            fch->leader = fch;
    }
    return;
}



void do_order( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char argbuf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;

    strcpy( argbuf, argument );
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "Order whom to do what?\n\r", ch );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        fAll   = TRUE;
        victim = NULL;
    }
    else
    {
        fAll   = FALSE;
        if ( ( victim = get_char_room( ch, arg ) ) == NULL )
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }

        if ( victim == ch )
        {
            send_to_char( "Aye aye, right away!\n\r", ch );
            return;
        }

        if ( !IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch )
        {
            if (IS_NPC(victim))
                do_say(victim, "Do it yourself!");
            else
                send_to_char("You can't do that.\n\r", ch);
            return;
        }
    }

    found = FALSE;
    for ( och = ch->in_room->first_person; och; och = och_next )
    {
        och_next = och->next_in_room;

        if ( IS_AFFECTED(och, AFF_CHARM)
             &&   och->master == ch
             && ( fAll || och == victim ) )
        {
            found = TRUE;
            act( AT_ACTION, "$n orders you to '$t'.", ch, argument, och, TO_VICT );
            interpret( och, argument );
        }
    }

    if ( found )
    {
        log_printf_plus( LOG_MONITOR, GetMaxLevel(ch), SEV_NOTICE, "%s: order %s.", ch->name, argbuf);
        send_to_char( "Ok.\n\r", ch );
        WAIT_STATE( ch, PULSE_VIOLENCE );
    }
    else
        send_to_char( "You have no followers here.\n\r", ch );
    return;
}

void do_guard( CHAR_DATA *ch, char *argument )
{
    bool off = FALSE;

    if (!IS_NPC(ch) || IS_ACT_FLAG(ch, ACT_POLYMORPHED))
    {
        send_to_char("Turn on autoassist instead (config +autoassist).\n\r",ch);
        return;
    }

    if (!ch->master)
    {
        send_to_char("You have no one to guard.\n\r", ch);
        return;
    }

    if (!argument || !*argument)
    {
        if (IS_ACT_FLAG(ch, ACT_GUARDIAN))
            off = TRUE;
        else
            off = FALSE;
    }
    else if (!str_cmp(argument, "on"))
        off = FALSE;
    else if (!str_cmp(argument, "off"))
        off = TRUE;
    else
    {
        send_to_char("Guard on or off?\n\r", ch->master);
        return;
    }

    if (off)
    {
        act(AT_PLAIN, "$n relaxes.", ch, NULL, NULL, TO_ROOM);
        act(AT_PLAIN, "You relax.", ch, NULL, NULL, TO_CHAR);
        REMOVE_ACT_FLAG(ch, ACT_GUARDIAN);
    }
    else
    {
        act(AT_PLAIN, "$n alertly watches you.", ch, NULL, ch->master, TO_VICT);
        act(AT_PLAIN, "$n alertly watches $N.", ch, NULL, ch->master, TO_NOTVICT);
        act(AT_PLAIN, "You alertly watch $N.", ch, NULL, ch->master, TO_CHAR);
        SET_ACT_FLAG(ch, ACT_GUARDIAN);
    }
}


void do_group( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim=NULL;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        switch (GET_INTF(ch))
        {
        case INT_SMAUG:
        case INT_MERC:
        case INT_ENVY: smaug_group( ch, argument ); break;
        default: dale_group( ch, argument ); break;
        }
        return;
    }

    if ( !str_cmp( arg, "disband" ))
    {
        CHAR_DATA *gch;
        int count = 0;

        if ( ch->leader || ch->master )
        {
            send_to_char( "You cannot disband a group if you're following someone.\n\r", ch );
            return;
        }

        for ( gch = first_char; gch; gch = gch->next )
        {
            if ( is_same_group( ch, gch )
                 && ( ch != gch ) )
            {
                gch->leader = NULL;
                gch->master = NULL;
                count++;
                send_to_char( "Your group is disbanded.\n\r", gch );
            }
        }

        if ( count == 0 )
            send_to_char( "You have no group members to disband.\n\r", ch );
        else
            send_to_char( "You disband your group.\n\r", ch );

        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        CHAR_DATA *rch;
        int count = 0;

        for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
        {
            if ( ch != rch
                 &&   !IS_NPC( rch )
                 &&   can_see( ch, rch )
                 &&   rch->master == ch
                 &&   !ch->master
                 &&   !ch->leader
                 &&   !is_same_group( rch, ch )
/*                 &&   IS_PKILL( ch ) == IS_PKILL( rch )*/
               )
            {
                rch->leader = ch;
                count++;
            }
        }

        if ( count == 0 )
            send_to_char( "You have no eligible group members.\n\r", ch );
        else
        {
            act( AT_ACTION, "$n groups $s followers.", ch, NULL, victim, TO_ROOM );
            send_to_char( "You group your followers.\n\r", ch );
        }
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( ch->master || ( ch->leader && ch->leader != ch ) )
    {
        send_to_char( "But you are following someone else!\n\r", ch );
        return;
    }

    if ( victim->master != ch && ch != victim )
    {
        act( AT_PLAIN, "$N isn't following you.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )
    {
        victim->leader = NULL;
        act( AT_ACTION, "$n removes $N from $s group.",   ch, NULL, victim, TO_NOTVICT );
        act( AT_ACTION, "$n removes you from $s group.",  ch, NULL, victim, TO_VICT    );
        act( AT_ACTION, "You remove $N from your group.", ch, NULL, victim, TO_CHAR    );
        return;
    }
    /*
    if ( !IS_NPC(victim) && IS_PKILL( ch ) != IS_PKILL( victim ) )
    {
        act( AT_PLAIN, "$N cannot join $n's group.",     ch, NULL, victim, TO_NOTVICT );
        act( AT_PLAIN, "You cannot join $n's group.",    ch, NULL, victim, TO_VICT    );
        act( AT_PLAIN, "$N cannot join your group.",     ch, NULL, victim, TO_CHAR    );
        return;
    }
    */
    victim->leader = ch;
    act( AT_ACTION, "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT );
    act( AT_ACTION, "You join $n's group.", ch, NULL, victim, TO_VICT    );
    act( AT_ACTION, "$N joins your group.", ch, NULL, victim, TO_CHAR    );
    return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    int members;
    int amount;
    int share;
    int extra, type;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || !argument )
    {
        send_to_char( "Split how much of what?\n\r", ch );
        return;
    }

    if (!(type=get_currency_type(argument)))
        type = DEFAULT_CURR;

    amount = atoi( arg );

    if ( amount < 0 )
    {
        send_to_char( "Your group wouldn't like that.\n\r", ch );
        return;
    }

    if ( amount == 0 )
    {
        send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
        return;
    }

    if ( GET_MONEY(ch,type) < amount )
    {
        ch_printf( ch, "You don't have that much %s.\n\r", curr_types[type] );
        return;
    }

    members = 0;
    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
        if ( is_same_group( gch, ch ) )
            members++;
    }


    if (( IS_PLR_FLAG(ch, PLR_AUTOGOLD)) && (members < 2))
        return;

    if ( members < 2 )
    {
        send_to_char( "Just keep it all.\n\r", ch );
        return;
    }

    share = amount / members;
    extra = amount % members;

    if ( share == 0 )
    {
        send_to_char( "Don't even bother, cheapskate.\n\r", ch );
        return;
    }

    GET_MONEY(ch,type) -= amount;
    GET_MONEY(ch,type) += share + extra;

    set_char_color( AT_GOLD, ch );
    ch_printf( ch,
               "You split %d %s coins.  Your share is %d %s coins.\n\r",
               amount, curr_types[type], share + extra, curr_types[type] );

    sprintf( buf, "$n splits %d %s coins.  Your share is %d %s coins.",
             amount, curr_types[type], share, curr_types[type] );

    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
        if ( gch != ch && is_same_group( gch, ch ) )
        {
            act( AT_GOLD, buf, ch, NULL, gch, TO_VICT );
            GET_MONEY(gch,type) += share;
        }
    }
    return;
}



void do_gtell( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *gch;
    int speaking = -1, lang;

    for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
	if ( ch->speaking & lang_array[lang] )
	{
	    speaking = lang;
	    break;
	}

    if ( argument[0] == '\0' )
    {
        send_to_char( "Tell your group what?\n\r", ch );
        return;
    }

    if ( IS_PLR_FLAG( ch, PLR_NO_TELL ) )
    {
        send_to_char( "Your message didn't get through!\n\r", ch );
        return;
    }

    /*
     * Note use of send_to_char, so gtell works on sleepers.
     */
    /*    sprintf( buf, "%s tells the group '%s'.\n\r", ch->name, argument );*/
    for ( gch = first_char; gch; gch = gch->next )
    {
        if ( is_same_group( gch, ch ) )
        {
	    if ( ch != gch ) {	/* Send a different message to the person doing the gtell */
		set_char_color( AT_GTELL, gch );
		/* Groups unscrambled regardless of clan language.  Other languages
                 still garble though. -- Altrag */
                if ( speaking != -1 && (!IS_NPC(ch) || ch->speaking) )
                {
                    int speakswell = UMIN(knows_language(gch, ch->speaking, ch),
                                          knows_language(ch, ch->speaking, gch));

                    if ( speakswell < 85 )
                        ch_printf( gch, "%s tells the group '%s'.\n\r", ch->name, translate(speakswell, argument, lang_names[speaking]) );
                    else
                        ch_printf( gch, "%s tells the group '%s'.\n\r", ch->name, argument );
                }
                else
                    ch_printf( gch, "%s tells the group '%s'.\n\r", ch->name, argument );
            } else {
                set_char_color( AT_GTELL, ch );
                ch_printf( ch, "You tell the group '%s'.\n\r", argument );	/* fix by Torin */
	    }
        }
    }

    return;
}


/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( ach->leader ) ach = ach->leader;
    if ( bch->leader ) bch = bch->leader;
    return ach == bch;
}


bool is_same_race_align( CHAR_DATA *ach, CHAR_DATA *bch )
{
    if ( GET_RACE(ach) == GET_RACE(bch) )
        return(TRUE);
    if ( IsGoodSide(ach) && IsGoodSide(bch) )
        return(TRUE);
    if ( IsBadSide(ach) && IsBadSide(bch) )
        return(TRUE);
    if ( IsNeutralSide(ach) && IsNeutralSide(bch) )
        return(TRUE);

    return(FALSE);
}

/*
 * this function sends raw argument over the AUCTION: channel
 * I am not too sure if this method is right..
 */

void talk_auction (char *argument)
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *vch;

    sprintf (buf,"Auction: %s", argument); /* last %s to reset color */

    for (vch = first_char; vch; vch = vch->next)
    {
        if (IS_NPC(vch))
            continue;

        if (GET_CON_STATE(vch) != CON_PLAYING ||
            IS_SET(vch->deaf, CHANNEL_AUCTION) ||
            IS_ROOM_FLAG(vch->in_room, ROOM_SILENCE) ||
            NOT_AUTHED(vch))
            continue;

        act( AT_GOSSIP, buf, vch, NULL, NULL, TO_CHAR );
    }
}

/*
 * Language support functions. -- Altrag
 * 07/01/96
 */
int knows_language( CHAR_DATA *ch, int language, CHAR_DATA *cch )
{
    sh_int sn;
    int max = 0, lang;

    if (IS_IMMORTAL(ch) || GetMaxLevel(ch) < 5)
        return 100;

    if (ch != cch && is_affected(ch, gsn_babel))
        return 0;

    if (IS_SET(race_table[GET_RACE(ch)].language, language))
        return 100;

    if ((IS_NPC(ch) || IS_NPC(cch)) && language & LANG_CLAN)
        return 100;

    if (ch != cch && (is_affected(ch, gsn_esp) || is_affected(ch, gsn_comprehend_lang)))
        return 100;

    if (language & LANG_CLAN &&
        ch->pcdata->clan != NULL &&
        ch->pcdata->clan == cch->pcdata->clan)
        return 100;

    for (lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++)
    {
        if (lang_array[lang] == LANG_CLAN)
            continue;

        if (IS_SET(language, lang_array[lang]) &&
            IS_SET(ch->speaks, lang_array[lang]) &&
            (sn = skill_lookup(lang_names[lang])))
            max = UMAX(max, LEARNED(ch, sn));
    }

    return URANGE(0, max, 100);
}

bool can_learn_lang( CHAR_DATA *ch, int language )
{
    if ( language & LANG_CLAN )
        return FALSE;
    if ( IS_NPC(ch) || IS_IMMORTAL(ch) )
        return FALSE;
    if ( race_table[ch->race].language & language )
        return FALSE;
    if ( ch->speaks & language )
    {
        int lang;

        for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
            if ( language & lang_array[lang] )
            {
                int sn;

                if ( !(VALID_LANGS & lang_array[lang]) )
                    return FALSE;
                if ( ( sn = skill_lookup( lang_names[lang] ) ) < 0 )
                {
                    bug( "Can_learn_lang: valid language without sn: %d", lang );
                    continue;
                }
                if ( LEARNED(ch, sn) >= GET_ADEPT(ch, sn) )
                    return FALSE;
            }
    }
    if ( VALID_LANGS & language )
        return TRUE;
    return FALSE;
}

/* Note: does not count racial language.  This is intentional (for now). */
int countlangs( int languages )
{
    int numlangs = 0;
    int looper;

    for ( looper = 0; lang_array[looper] != LANG_UNKNOWN; looper++ )
    {
        if ( lang_array[looper] == LANG_CLAN )
            continue;
        if ( languages & lang_array[looper] )
            numlangs++;
    }
    return numlangs;
}

void do_speak( CHAR_DATA *ch, char *argument )
{
    int langs;
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg );

    if ( !str_cmp( arg, "all" ) && IS_IMMORTAL( ch ) )
    {
        set_char_color( AT_SAY, ch );
        ch->speaking = ~LANG_CLAN;
        send_to_char( "Now speaking all languages.\n\r", ch );
        return;
    }
    for ( langs = 0; lang_array[langs] != LANG_UNKNOWN; langs++ )
        if ( !str_prefix( arg, lang_names[langs] ) )
            if ( knows_language( ch, lang_array[langs], ch ) )
            {
                if ( lang_array[langs] == LANG_CLAN &&
                     (IS_NPC(ch) || !ch->pcdata->clan) )
                    continue;
                ch->speaking = lang_array[langs];
                set_char_color( AT_SAY, ch );
                ch_printf( ch, "You now speak %s.\n\r", lang_names[langs] );
                return;
            }
    set_char_color( AT_SAY, ch );
    send_to_char( "You do not know that language.\n\r", ch );
}

void do_languages( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int lang;

    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' && !str_prefix( arg, "learn" ) &&
         !IS_IMMORTAL(ch) && !IS_NPC(ch) )
    {
        CHAR_DATA *sch;
        char arg2[MAX_INPUT_LENGTH];
        int sn;
        int prct;
        int prac;

        argument = one_argument( argument, arg2 );
        if ( arg2[0] == '\0' )
        {
            send_to_char( "Learn which language?\n\r", ch );
            return;
        }
        for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
        {
            if ( lang_array[lang] == LANG_CLAN )
                continue;
            if ( !str_prefix( arg2, lang_names[lang] ) )
                break;
        }
        if ( lang_array[lang] == LANG_UNKNOWN )
        {
            send_to_char( "That is not a language.\n\r", ch );
            return;
        }
        if ( !(VALID_LANGS & lang_array[lang]) )
        {
            send_to_char( "You may not learn that language.\n\r", ch );
            return;
        }
        if ( ( sn = skill_lookup( lang_names[lang] ) ) < 0 )
        {
            send_to_char( "That is not a language.\n\r", ch );
            return;
        }
        if ( race_table[ch->race].language & lang_array[lang] ||
             lang_array[lang] == LANG_COMMON ||
             LEARNED(ch, sn) >= GET_ADEPT(ch, sn) )
        {
            act( AT_PLAIN, "You are already fluent in $t.", ch,
                 lang_names[lang], NULL, TO_CHAR );
            return;
        }
        for ( sch = ch->in_room->first_person; sch; sch = sch->next )
            if ( IS_NPC(sch) && IS_ACT_FLAG(sch, ACT_SCHOLAR) &&
                 knows_language( sch, ch->speaking, ch ) &&
                 knows_language( sch, lang_array[lang], sch ) &&
                 (!sch->speaking || knows_language( ch, sch->speaking, sch )) )
                break;
        if ( !sch )
        {
            send_to_char( "There is no one who can teach that language here.\n\r", ch );
            return;
        }
        if ( countlangs(ch->speaks) >= (GetMaxLevel(ch) / 10) &&
             LEARNED(ch, sn) <= 0 )
        {
            act( AT_TELL, "$n tells you 'You may not learn a new language yet.'",
                 sch, NULL, ch, TO_VICT );
            return;
        }
        /* 0..16 cha = 2 pracs, 17..25 = 1 prac. -- Altrag */
        prac = 2 - (get_curr_cha(ch) / 17);
        if ( ch->practice < prac )
        {
            act( AT_TELL, "$n tells you 'You do not have enough practices.'",
                 sch, NULL, ch, TO_VICT );
            return;
        }
        ch->practice -= prac;
        /* Max 12% (5 + 4 + 3) at 24+ int and 21+ wis. -- Altrag */
        prct = 5 + (get_curr_int(ch) / 6) + (get_curr_wis(ch) / 7);
        ch->pcdata->learned[sn] += prct;
        ch->pcdata->learned[sn] = UMIN(LEARNED(ch, sn), GET_ADEPT(ch, sn));
        SET_BIT( ch->speaks, lang_array[lang] );
        if ( LEARNED(ch, sn) == prct )
            act( AT_PLAIN, "You begin lessons in $t.", ch, lang_names[lang],
                 NULL, TO_CHAR );
        else if ( LEARNED(ch, sn) < 60 )
            act( AT_PLAIN, "You continue lessons in $t.", ch, lang_names[lang],
                 NULL, TO_CHAR );
        else if ( LEARNED(ch, sn) < 60 + prct )
            act( AT_PLAIN, "You feel you can start communicating in $t.", ch,
                 lang_names[lang], NULL, TO_CHAR );
        else if ( LEARNED(ch, sn) < GET_ADEPT(ch, sn) )
            act( AT_PLAIN, "You become more fluent in $t.", ch,
                 lang_names[lang], NULL, TO_CHAR );
        else
            act( AT_PLAIN, "You now speak perfect $t.", ch, lang_names[lang],
                 NULL, TO_CHAR );
        return;
    }
    for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
        if ( knows_language( ch, lang_array[lang], ch ) )
        {
            if ( ch->speaking & lang_array[lang] ||
                 (IS_NPC(ch) && !ch->speaking) )
                set_char_color( AT_RED, ch );
            else
                set_char_color( AT_SAY, ch );
            send_to_char( lang_names[lang], ch );
            send_to_char( " ", ch );
        }
    send_to_char( "\n\r", ch );
    return;
}

void do_wartalk( CHAR_DATA *ch, char *argument )
{
    if (NOT_AUTHED(ch))
    {
        send_to_char("Huh?\n\r", ch);
        return;
    }
    talk_channel( ch, argument, CHANNEL_WARTALK, "war" );
    return;
}

void do_interface(CHAR_DATA *ch, char *argument)
{

    if (IS_NPC(ch)) {
        send_to_char("Creatures such as yourself can't see the world in any other way!\n\r", ch);
        return;
    }

    if (!str_cmp(argument, "dale")) {
        ch->pcdata->interface = INT_DALE;
        send_to_char("Interface set to DaleMUD!\n\r", ch);
        do_color(ch, "def");
    }
    else if (!str_cmp(argument, "silly")) {
        ch->pcdata->interface = INT_DALE;
        send_to_char("Interface set to SillyMUD!\n\r", ch);
        do_color(ch, "rmdesc grey");
        do_color(ch, "rmname grey");
    }
    else if (!str_cmp(argument, "smaug")) {
        ch->pcdata->interface = INT_SMAUG;
        send_to_char("Interface set to Smaug!\n\r", ch);
        do_color(ch, "rmdesc yellow");
        do_color(ch, "rmname white");
    }
    else if (!str_cmp(argument, "merc")) {
        ch->pcdata->interface = INT_ENVY;
        send_to_char("Interface set to Merc!\n\r", ch);
        do_color(ch, "rmdesc grey");
        do_color(ch, "rmname grey");
    }
    else if (!str_cmp(argument, "envy")) {
        ch->pcdata->interface = INT_ENVY;
        send_to_char("Interface set to Envy!\n\r", ch);
        do_color(ch, "rmdesc grey");
        do_color(ch, "rmname grey");
    }
    else if (!str_cmp(argument, "imp")) {
        if (get_trust(ch) >= LEVEL_SUPREME) {
            ch->pcdata->interface = INT_IMP;
            send_to_char("Interface set to IMPLEMENTOR!\n\r", ch);
            do_color(ch, "rmdesc grey");
            do_color(ch, "rmname grey");
        } else {
            send_to_char("Yeah right....\n\r", ch);
        }
    }
    else {
        ch->pcdata->interface = INT_DEFAULT;
        send_to_char("Interface set to Default!\n\r", ch);
        send_to_char("Interface can be: dale/silly, smaug, merc/envy\n\r",ch);
        do_color(ch, "def");
    }
}

void do_beep( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Beep whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    act(AT_PLAIN, "\a$N beeps you.", victim, NULL, ch, TO_CHAR);
    send_to_char("Ok.\n\r", ch);
}

#define MAX_POSE_CLASS 11
#define MAX_POSES      20

const int pose_classes[MAX_POSE_CLASS] =
{
    CLASS_MAGE, CLASS_CLERIC, CLASS_WARRIOR, CLASS_THIEF, CLASS_DRUID,
    CLASS_MONK, CLASS_BARBARIAN, CLASS_ARTIFICER, CLASS_PALADIN,
    CLASS_RANGER, CLASS_PSIONIST
};

struct pose_type
{
    int    level;                      /* minimum level for poser */
    char * poser_msg[MAX_POSE_CLASS];  /* message to poser        */
    char * room_msg[MAX_POSE_CLASS];   /* message to room         */
} pose_messages[MAX_POSES];

void load_poses()
{
    FILE *fp;
    char *line;
    int counter, x;

    if ( !( fp = fopen( POSE_FILE, "r" ) ) )
    {
        bug( "load_poses: Cannot open: %s", POSE_FILE );
 	exit(0);
    }

    for (counter=0;counter<MAX_POSES;counter++)
    {
        pose_messages[counter].level = fread_number(fp);

        if (pose_messages[counter].level < 0)
            break;

        for (x=0;x<MAX_POSE_CLASS;x++)
        {
            line = fread_line(fp);
            pose_messages[counter].poser_msg[x] = str_dup(strip_crlf(line));
            line = fread_line(fp);
            pose_messages[counter].room_msg[x]  = str_dup(strip_crlf(line));
        }
    }

    FCLOSE(fp);
}

void free_poses(void)
{
    int counter, x;

    for (counter=0;counter<MAX_POSES;counter++)
    {
        for (x=0;x<MAX_POSE_CLASS;x++)
        {
	    if (pose_messages[counter].poser_msg[x])
                DISPOSE(pose_messages[counter].poser_msg[x]);
            if (pose_messages[counter].room_msg[x])
                DISPOSE(pose_messages[counter].room_msg[x]);
        }
    }


}

void do_pose( CHAR_DATA *ch, char *argument )
{
    int to_pose;
    int counter;
    int lev, cl, x;

    lev = GetMaxLevel(ch);

    if (IS_NPC(ch) || lev < 5)
    {
        send_to_char("Pardon?\n\r", ch);
        return;
    }

    for (counter=0;counter<MAX_POSE_CLASS;counter++)
        if (IS_ACTIVE(ch, pose_classes[counter]))
            break;
    if (counter>=MAX_POSE_CLASS || !IS_ACTIVE(ch, pose_classes[counter]))
    {
        send_to_char("Your don't have any poses.\n\r", ch);
        return;
    }


    for (x=0;x<=100;x++)
    {
        cl = number_range(0, MAX_POSE_CLASS-1);
        if (IS_ACTIVE(ch, pose_classes[cl]) &&
            GET_LEVEL(ch, pose_classes[cl]) > 5)
            break;
    }
    if (x==100)
    {
        bug("do_pose: %s failed posing", GET_NAME(ch));
        send_to_char("You failed.\n\r", ch);
        return;
    }

    lev = GET_LEVEL(ch, pose_classes[cl]);

    for (counter = 0;
         counter < MAX_POSES &&
         (pose_messages[counter].level < lev) &&
         (pose_messages[counter].level > 0);
         counter++);

    to_pose = number_range(0, counter-1);

    act(AT_SOCIAL, pose_messages[to_pose].poser_msg[cl], ch, NULL, NULL, TO_CHAR);
    if (IS_IMMORTAL(ch))
        ch_printf(ch, "(%s)\n\r", pose_messages[to_pose].room_msg[cl]);
    act(AT_SOCIAL, pose_messages[to_pose].room_msg[cl], ch, NULL, NULL, TO_ROOM);
}

void do_pray( CHAR_DATA *ch, char *argument )
{
    talk_channel( ch, argument, CHANNEL_PRAY, "pray" );
}

void do_highfive(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char buf[MAX_INPUT_LENGTH];

    if (!*argument)
    {
        send_to_char("Highfive whom?\n\r", ch);
        return;
    }

    if (!(victim = get_char_room(ch, argument)))
    {
        if (!IS_IMMORTAL(ch) || !(victim = get_char_world(ch, argument)))
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }
    }

    if (victim == ch)
    {
        send_to_char("You clap your hands like you just don't care...\n\r", ch);
        return;
    }

    if (IS_IMMORTAL(ch) && IS_IMMORTAL(victim))
    {
        sprintf(buf, "Time stops for a moment as %s and %s high five.\n\r",
                GET_NAME(ch), GET_NAME(victim));
        echo_to_all(AT_IMMORT, buf, ECHOTAR_ALL);
    }

    act(AT_PLAIN, "$n gives you a high five.", ch, NULL, victim, TO_VICT);
    act(AT_PLAIN, "You give a hearty high five to $N.", ch, NULL, victim, TO_CHAR);
    act(AT_PLAIN, "$n gives $N a high five.", ch, NULL, victim, TO_NOTVICT);
}
