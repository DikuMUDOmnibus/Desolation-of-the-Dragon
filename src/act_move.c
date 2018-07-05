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
 *			   Player movement module			    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: act_move.c,v 1.36 2004/04/06 22:00:08 dotd Exp $";*/


#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "mud.h"
#include "gsn.h"
#include "quest.h"

DECLARE_DO_FUN(do_look);

const	sh_int	movement_loss	[SECT_MAX]	=
{
    0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0
};

char *	const	dir_name_str	[LAST_NORMAL_DIR+1]		=
{
    "North", "East", "South", "West", "Up", "Down",
    "NorthEast", "NorthWest", "SouthEast", "SouthWest", "Somewhere"
};

const	int	trap_door	[]		=
{
    TRAP_N, TRAP_E, TRAP_S, TRAP_W, TRAP_U, TRAP_D,
    TRAP_NE, TRAP_NW, TRAP_SE, TRAP_SW
};


const	sh_int	rev_dir		[LAST_NORMAL_DIR+1]		=
{
    2, 3, 0, 1, 5, 4, 9, 8, 7, 6, 10
};


ROOM_INDEX_DATA * vroom_hash [64];


/*
 * Local functions.
 */
bool	has_key		args( ( CHAR_DATA *ch, int key ) );


char *	const		sect_names[SECT_MAX][2] =
{
    { "In a room","inside"	},	{ "In a city",	"cities"	},
    { "In a field","fields"	},	{ "In a forest","forests"	},
    { "hill",	"hills"		},	{ "On a mountain","mountains"	},
    { "In the water","waters"	},	{ "In rough water","waters"	},
    { "Underwater", "underwaters" },	{ "In the air",	"air"		},
    { "In a desert","deserts"	},	{ "Somewhere",	"unknown"	},
    { "ocean floor", "ocean floor" },	{ "underground", "underground"	},
    { "In a tree", "trees"      },      { "In a fire", "fire"           },
    { "In quicksand", "quicksand" },    { "In the ether", "ether"       },
    { "On a glacier", "glacier" },      { "In solid rock", "earth"      }
};


const	int		sent_total[SECT_MAX]		=
{
    3, 5, 4, 4, 1, 1, 1, 1, 1, 2, 2, 25, 1, 1, 1, 1, 1, 1, 1, 1
};

char *	const		room_sents[SECT_MAX][25]	=
{
    {
        "rough hewn walls of granite with the occasional spider crawling around",
        "signs of a recent battle from the bloodstains on the floor",
        "a damp musty odour not unlike rotting vegetation"
    },

    {
        "the occasional stray digging through some garbage",
        "merchants trying to lure customers to their tents",
        "some street people putting on an interesting display of talent",
        "an argument between a customer and a merchant about the price of an item",
        "several shady figures talking down a dark alleyway"
    },

    {
        "sparce patches of brush and shrubs",
        "a small cluster of trees far off in the distance",
        "grassy fields as far as the eye can see",
        "a wide variety of weeds and wildflowers"
    },

    {
        "tall, dark evergreens prevent you from seeing very far",
        "many huge oak trees that look several hundred years old",
        "a solitary lonely weeping willow",
        "a patch of bright white birch trees slender and tall"
    },

    {
        "rolling hills lightly speckled with violet wildflowers"
    },

    {
        "the rocky mountain pass offers many hiding places"
    },

    {
        "the water is smooth as glass"
    },

    {
        "rough waves splash about angrily"
    },

    {
        "a small school of fish"
    },

    {
        "the land far below",
        "a misty haze of clouds"
    },

    {
        "sand as far as the eye can see",
        "an oasis far in the distance"
    },

    {
        "nothing unusual",	"nothing unusual",	"nothing unusual",
        "nothing unusual",	"nothing unusual",	"nothing unusual",
        "nothing unusual",	"nothing unusual",	"nothing unusual",
        "nothing unusual",	"nothing unusual",	"nothing unusual",
        "nothing unusual",	"nothing unusual",	"nothing unusual",
        "nothing unusual",	"nothing unusual",	"nothing unusual",
        "nothing unusual",	"nothing unusual",	"nothing unusual",
        "nothing unusual",	"nothing unusual",	"nothing unusual",
        "nothing unusual",
    },

    {
        "rocks and coral which litter the ocean floor."
    },

    {
        "a lengthy tunnel of rock."
    },

    {
        "tree"
    },

    {
        "fire"
    },

    {
        "quicksand"
    },

    {
        "ether"
    },

    {
        "glacier"
    },

    {
        "earth"
    }

};

char *grab_word( char *argument, char *arg_first )
{
    char cEnd;
    sh_int count;

    count = 0;

    while ( isspace(*argument) )
        argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;

    while ( *argument != '\0' || ++count >= 255 )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        *arg_first++ = *argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
        argument++;

    return argument;
}

char *wordwrap( char *txt, sh_int wrap )
{
    static char buf[MAX_STRING_LENGTH];
    char *bufp;

    buf[0] = '\0';
    bufp = buf;
    if ( txt != NULL )
    {
        char line[MAX_STRING_LENGTH];
        char temp[MAX_STRING_LENGTH];
        char *ptr, *p;
        int ln, x;

        ++bufp;
        line[0] = '\0';
        ptr = txt;
        while ( *ptr )
        {
            ptr = grab_word( ptr, temp );
            ln = strlen( line );  x = strlen( temp );
            if ( (ln + x + 1) < wrap )
            {
                if ( line[ln-1] == '.' )
                    strcat( line, "  " );
                else
                    strcat( line, " " );
                strcat( line, temp );
                p = strchr( line, '\n' );
                if ( !p )
                    p = strchr( line, '\r' );
                if ( p )
                {
                    strcat( buf, line );
                    line[0] = '\0';
                }
            }
            else
            {
                strcat( line, "\r\n" );
                strcat( buf, line );
                strcpy( line, temp );
            }
        }
        if ( line[0] != '\0' )
            strcat( buf, line );
    }
    return bufp;
}

void decorate_room( ROOM_INDEX_DATA *room )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int nRand;
    int iRand, len;
    int previous[8];
    int sector = room->sector_type;
    char *pre=NULL, *post=NULL;

    if ( room->name )
        STRFREE( room->name );
    if ( room->description )
        STRFREE( room->description );

    /*
     room->name	  = STRALLOC( "In a virtual room" );
     room->description = STRALLOC( "You're on a pathway.\n\r" );
     */

    room->name	= STRALLOC( sect_names[sector][0] );
    buf[0] = '\0';
    nRand = number_range( 1, UMIN(8,sent_total[sector]) );

    for ( iRand = 0; iRand < nRand; iRand++ )
        previous[iRand] = -1;

    for ( iRand = 0; iRand < nRand; iRand++ )
    {
        while ( previous[iRand] == -1 )
        {
            int x, z;

            x = number_range( 0, sent_total[sector]-1 );

            for ( z = 0; z < iRand; z++ )
                if ( previous[z] == x )
                    break;

            if ( z < iRand )
                continue;

            previous[iRand] = x;

            len = strlen(buf);
            if ( len == 0 )
            {
                switch( number_range(1, 2 * (iRand == nRand -1) ? 1 : 2) )
                {
                case 1:	pre = "You notice ";	post = ".";	 break;
                case 2: pre = "You see ";	post = ".";	 break;
                case 3: pre = "You see ";	post = ", and "; break;
                case 4: pre = "You notice ";	post = ", and "; break;
                }
                sprintf( buf2, "%s%s%s", pre, room_sents[sector][x], post );
            }
            else
                if ( iRand != nRand -1 )
                {
                    if ( buf[len-1] == '.' )
                        switch( number_range(0, 3) )
                        {
                        case 0:	pre = "you notice ";	post = ".";	 break;
                        case 1: pre = "you see ";	post = ", and "; break;
                        case 2: pre = "you see ";	post = ".";	 break;
                        case 3: pre = "over yonder ";	post = ", and "; break;
                        }
                    else
                        switch( number_range(0, 3) )
                        {
                        case 0:	pre = ""; post = ".";			break;
                        case 1:	pre = ""; post = " not too far away.";	break;
                        case 2: pre = ""; post = ", and ";		break;
                        case 3: pre = ""; post = " nearby.";		break;
                        }
                    sprintf( buf2, "%s%s%s", pre, room_sents[sector][x], post );
                }
                else
                    sprintf( buf2, "%s.", room_sents[sector][x] );
            if ( len > 5 && buf[len-1] == '.' )
            {
                strcat( buf, "  " );
                buf2[0] = UPPER(buf2[0] );
            }
            else
                if ( len == 0 )
                    buf2[0] = UPPER(buf2[0] );
            strcat( buf, buf2 );
        }
    }
    sprintf( buf2, "%s\n\r", wordwrap(buf, 78) );
    room->description = STRALLOC( buf2 );
}

/*
 * Remove any unused virtual rooms				-Thoric
 */
void clear_vrooms( )
{
    int hash;
    ROOM_INDEX_DATA *room, *room_next, *prev;

    for ( hash = 0; hash < 64; hash++ )
    {
        while ( vroom_hash[hash]
                &&     !vroom_hash[hash]->first_person
                &&     !vroom_hash[hash]->first_content )
        {
            room = vroom_hash[hash];
            vroom_hash[hash] = room->next;
            clean_room( room );
            DISPOSE( room );
            --top_vroom;
        }
        prev = NULL;
        for ( room = vroom_hash[hash]; room; room = room_next )
        {
            room_next = room->next;
            if ( !room->first_person && !room->first_content )
            {
                if ( prev )
                    prev->next = room_next;
                clean_room( room );
                DISPOSE( room );
                --top_vroom;
            }
            if ( room )
                prev = room;
        }
    }
}

char *exit_name(EXIT_DATA *pexit)
{
    if (pexit->vdir >= 0 && pexit->vdir <= LAST_NORMAL_DIR)
	return dir_name_str[pexit->vdir];

    if (pexit->keyword && *pexit->keyword)
	return pexit->keyword;

    bug("exit_name: no name for exit");
    return "(Arbitrary)";
}

char *dir_name(sh_int dir)
{
    if (dir >= 0 && dir <= LAST_NORMAL_DIR)
	return dir_name_str[dir];

    return "(Arbitrary)";
}
/*
 * Function to get the equivelant exit of DIR 0-MAXDIR out of linked list.
 * Made to allow old-style diku-merc exit functions to work.	-Thoric
 */
EXIT_DATA *get_exit( ROOM_INDEX_DATA *room, sh_int dir )
{
    EXIT_DATA *xit;

    if ( !room )
    {
        bug( "Get_exit: NULL room" );
        return NULL;
    }

    for (xit = room->first_exit; xit; xit = xit->next )
        if ( xit->vdir == dir )
            return xit;
    return NULL;
}

/*
 * Function to get an exit, leading the the specified room
 */
EXIT_DATA *get_exit_to( ROOM_INDEX_DATA *room, sh_int dir, int vnum )
{
    EXIT_DATA *xit;

    if ( !room )
    {
        bug( "Get_exit: NULL room" );
        return NULL;
    }

    for (xit = room->first_exit; xit; xit = xit->next )
        if ( xit->vdir == dir && xit->vnum == vnum )
            return xit;
    return NULL;
}

/*
 * Function to get the nth exit of a room			-Thoric
 */
EXIT_DATA *get_exit_num( ROOM_INDEX_DATA *room, sh_int count )
{
    EXIT_DATA *xit;
    int cnt;

    if ( !room )
    {
        bug( "Get_exit: NULL room" );
        return NULL;
    }

    for (cnt = 0, xit = room->first_exit; xit; xit = xit->next )
        if ( ++cnt == count )
            return xit;
    return NULL;
}


/*
 * Modify movement due to encumbrance				-Thoric
 */
sh_int encumbrance( CHAR_DATA *ch, sh_int move )
{
    int cur, max;

    max = can_carry_w(ch);
    cur = carry_w(ch);
    if ( cur >= max )
        return move * 4;
    else if ( cur >= max * 0.95 )
        return (int)(move * 3.5);
    else if ( cur >= max * 0.90 )
        return move * 3;
    else if ( cur >= max * 0.85 )
        return (int)(move * 2.5);
    else if ( cur >= max * 0.80 )
        return move * 2;
    else if ( cur >= max * 0.75 )
        return (int)(move * 1.5);
    else
        return move;
}


/*
 * Check to see if a character can fall down, checks for looping   -Thoric
 */
bool will_fall( CHAR_DATA *ch, int fall )
{
    if ( IS_SET( ch->in_room->room_flags, ROOM_NOFLOOR ) &&
         get_exit(ch->in_room, DIR_DOWN) &&
         (!IS_AFFECTED( ch, AFF_FLYING ) ||
          ( ch->mount && !IS_AFFECTED( ch->mount, AFF_FLYING ) ) ) )
    {
        if ( fall > 80 )
        {
            bug( "Falling (in a loop?) more than 80 rooms: vnum %d", ch->in_room->vnum );
            char_from_room( ch );
            char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
            fall = 0;
            return TRUE;
        }
        set_char_color( AT_FALLING, ch );
        send_to_char( "You're falling down...\n\r", ch );
        move_char( ch, get_exit(ch->in_room, DIR_DOWN), ++fall );
        return TRUE;
    }
    return FALSE;
}

bool will_drown( CHAR_DATA *ch, ROOM_INDEX_DATA *room )
{
    if (IS_IMMORTAL(ch))
        return FALSE;

    if (IS_AFFECTED(ch, AFF_AQUA_BREATH))
        return FALSE;

    if (room->sector_type == SECT_UNDERWATER ||
        room->sector_type == SECT_OCEANFLOOR)
        return TRUE;

    if (IS_AFFECTED(ch, AFF_FLYING) ||
        IS_AFFECTED(ch, AFF_FLOATING))
        return FALSE;

    if (room->sector_type == SECT_WATER_NOSWIM)
        return TRUE;

    if (room->sector_type == SECT_WATER_SWIM &&
        !LEARNED(ch, gsn_swim))
        return TRUE;

    return FALSE;
}

/*
 * create a 'virtual' room					-Thoric
 */
ROOM_INDEX_DATA *generate_exit( ROOM_INDEX_DATA *in_room, EXIT_DATA **pexit )
{
    EXIT_DATA *xit, *bxit;
    EXIT_DATA *orig_exit = (EXIT_DATA *) *pexit;
    ROOM_INDEX_DATA *room, *backroom;
    int brvnum;
    int serial;
    int roomnum;
    int distance = -1;
    int vdir = orig_exit->vdir;
    int rdir = orig_exit->rdir;
    sh_int hash;
    bool found = FALSE;

    if ( in_room->vnum > 1048576000 )	/* room is virtual */
    {
        serial = in_room->vnum;
        roomnum = in_room->tele_vnum;
        if ( (serial & 2097152000) == orig_exit->vnum )
        {
            brvnum = serial >> 16;
            --roomnum;
            distance = roomnum;
        }
        else
        {
            brvnum = serial & 2097152000;
            ++roomnum;
            distance = orig_exit->distance - 1;
        }
        backroom = get_room_index( brvnum );
    }
    else
    {
        int r1 = in_room->vnum;
        int r2 = orig_exit->vnum;

        brvnum = r1;
        backroom = in_room;
        serial = (UMAX( r1, r2 ) << 16) | UMIN( r1, r2 );
        distance = orig_exit->distance - 1;
        roomnum = r1 < r2 ? 1 : distance;
    }
    hash = serial % 64;

    for ( room = vroom_hash[hash]; room; room = room->next )
        if ( room->vnum == serial && room->tele_vnum == roomnum )
        {
            found = TRUE;
            break;
        }
    if ( !found )
    {
        CREATE( room, ROOM_INDEX_DATA, 1 );
        room->area	  = in_room->area;
        room->vnum	  = serial;
        room->tele_vnum	  = roomnum;
        room->sector_type = in_room->sector_type;
        room->room_flags  = in_room->room_flags;
        decorate_room( room );
        room->next	  = vroom_hash[hash];
        vroom_hash[hash]  = room;
        ++top_vroom;
    }
    if ( !found || (xit=get_exit(room, vdir))==NULL )
    {
        xit = make_exit(room, orig_exit->to_room, vdir);
        xit->keyword		= STRALLOC( "" );
        xit->description	= STRALLOC( "" );
        xit->key		= 0;
        xit->distance = distance;
    }
    if ( !found )
    {
        bxit = make_exit(room, backroom, rdir);
        bxit->keyword		= STRALLOC( "" );
        bxit->description	= STRALLOC( "" );
        bxit->key		= 0;
        if ( (serial & 2097152000) != orig_exit->vnum )
            bxit->distance = roomnum;
        else
        {
            EXIT_DATA *tmp = get_exit( backroom, vdir );
            int fulldist = tmp->distance;

            bxit->distance = fulldist - distance;
        }
    }
    *pexit = xit;
    return room;
}

ch_ret move_char( CHAR_DATA *ch, EXIT_DATA *pexit, int fall )
{
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    ROOM_INDEX_DATA *from_room;
    char buf[MAX_STRING_LENGTH];
    char txt[MAX_INPUT_LENGTH];
    char *dtxt;
    ch_ret retcode;
    sh_int door, distance;
    bool drunk = FALSE;
    bool brief = FALSE;

    if ( pexit && !pexit->to_room )
	load_area_demand( pexit->vnum );

    if ( !IS_NPC( ch ) )
        if ( IS_DRUNK( ch, 2 ) && ( ch->position != POS_SHOVE )
             && ( ch->position != POS_DRAG ) )
            drunk = TRUE;

    if ( drunk && !fall )
    {
        door = number_door();
        pexit = get_exit( ch->in_room, door );
    }

#ifdef DEBUG
    if ( pexit )
    {
        sprintf( log_buf, "move_char: %s to door %d", ch->name, pexit->vdir );
        log_string_plus( log_buf, LOG_DEBUG, LEVEL_LOG_CSET, SEV_SPAM );
    }
#endif

    retcode = rNONE;
    txt[0] = '\0';

    if ( IS_NPC(ch) && IS_SET( ch->act, ACT_MOUNTED ) )
        return retcode;

    in_room = ch->in_room;
    from_room = in_room;
    if ( !pexit || (to_room = pexit->to_room) == NULL )
    {
        if ( drunk )
            send_to_char( "You hit a wall in your drunken state.\n\r", ch );
        else
            send_to_char( "Alas, you cannot go that way.\n\r", ch );
        return rSTOP;
    }

    door = pexit->vdir;
    distance = pexit->distance;

    /*
     * Exit is only a "window", there is no way to travel in that direction
     * unless it's a door with a window in it		-Thoric
     */
    if ( IS_SET( pexit->exit_info, EX_WINDOW )
         &&  !IS_SET( pexit->exit_info, EX_ISDOOR ) )
    {
        send_to_char( "Alas, you cannot go that way.\n\r", ch );
        return rSTOP;
    }

    if (  IS_SET(pexit->exit_info, EX_PORTAL)
          && IS_NPC(ch) )
    {
        act( AT_PLAIN, "Mobs can't use portals.", ch, NULL, NULL, TO_CHAR );
        return rSTOP;
    }

    if ( IS_SET(pexit->exit_info, EX_NOMOB)
         && IS_NPC(ch) )
    {
        act( AT_PLAIN, "Mobs can't enter there.", ch, NULL, NULL, TO_CHAR );
        return rSTOP;
    }

    if (is_affected(ch, gsn_web))
    {
        if (saves_para_petri(GetMaxLevel(ch), ch))
        {
            send_to_char("You are entrapped in sticky webs!\n\r", ch);
            send_to_char("Your struggles only entrap you further!\n\r", ch);
            act(AT_PLAIN, "$n struggles against webs that bind $s.", ch, NULL, NULL, TO_ROOM);
            WAIT_STATE(ch, PULSE_VIOLENCE*5);
            GET_MOVE(ch) = 0;
        }
        else
        {
            send_to_char("You briefly pull free from the sticky webbing!\n\r", ch);
            act(AT_PLAIN, "$n briefly pulls free from the webs that bind $s.", ch, NULL, NULL, TO_ROOM);
            WAIT_STATE(ch, PULSE_VIOLENCE);
            GET_MOVE(ch) -= 50;
        }
        return rSTOP;
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
         && (!IS_AFFECTED(ch, AFF_PASS_DOOR)
             ||   IS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
    {
        if ( !IS_SET( pexit->exit_info, EX_SECRET )
             &&   !IS_SET( pexit->exit_info, EX_DIG ) )
        {
            if ( drunk )
            {
                act( AT_PLAIN, "$n runs into the $d in $s drunken state.", ch,
                     NULL, pexit->keyword, TO_ROOM );
                act( AT_PLAIN, "You run into the $d in your drunken state.", ch,
                     NULL, pexit->keyword, TO_CHAR );
            }
            else
                act( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
        }
        else
        {
            if ( drunk )
                send_to_char( "You hit a wall in your drunken state.\n\r", ch );
            else
                send_to_char( "Alas, you cannot go that way.\n\r", ch );
        }

        return rSTOP;
    }

    if (to_room->sector_type == SECT_TREE &&
	!IS_IMMORTAL(ch) &&
	!is_affected(ch, gsn_tree_travel))
    {
	send_to_char("The trees are too dense to go that way.\n\r", ch);
	return rSTOP;
    }

    /*
     * Crazy virtual room idea, created upon demand.		-Thoric
     */
    if ( distance > 1 )
        if ( (to_room=generate_exit(in_room, &pexit)) == NULL )
            send_to_char( "Alas, you cannot go that way.\n\r", ch );

    if ( !fall
         &&   IS_AFFECTED(ch, AFF_CHARM)
         &&   ch->master
         &&   in_room == ch->master->in_room )
    {
        send_to_char( "What?  And leave your beloved master?\n\r", ch );
        return rSTOP;
    }

    if ( room_is_private( to_room ) )
    {
        send_to_char( "That room is private right now.\n\r", ch );
        return rSTOP;
    }

    if ( (GET_POS(ch) == POS_MEDITATING) &&
        !(IS_SET(ch->affected_by, AFF_FLOATING)) )
    {
        send_to_char( "You are too busy meditating.\n\r", ch );
        return rSTOP;
    }

    if ( !IS_IMMORTAL(ch)
         &&  !IS_NPC(ch)
         &&  ch->in_room->area != to_room->area )
    {
        if ( to_room->area->low_hard_range > 0 &&
             GetMaxLevel(ch) < to_room->area->low_hard_range )
        {
            set_char_color( AT_TELL, ch );
            switch( to_room->area->low_hard_range - GetMaxLevel(ch) )
            {
            case 1:
                send_to_char( "A voice in your mind says, 'You are nearly ready to go that way...'\n\r", ch );
                break;
            case 2:
                send_to_char( "A voice in your mind says, 'Soon you shall be ready to travel down this path... soon.'\n\r", ch );
                break;
            case 3:
                send_to_char( "A voice in your mind says, 'You are not ready to go down that path... yet.'.\n\r", ch);
                break;
            default:
                send_to_char( "A voice in your mind says, 'You are not ready to go down that path.'.\n\r", ch);
            }
            return rNONE;
        }
        else
            if ( to_room->area->hi_hard_range > 0 &&
                 GetMinLevel(ch) > to_room->area->hi_hard_range )
            {
                set_char_color( AT_TELL, ch );
                send_to_char( "A voice in your mind says, 'There is nothing more for you down that path.'\n\r", ch );
                return rNONE;
            }
    }

    if ( !fall )
    {
        int move;

        /*
         * Prevent deadlies from entering a nopkill-flagged area from a
         * non-flagged area, but allow them to move around if already
         * inside a nopkill area. - Blodkai
         */

        if (  IS_SET( to_room->area->flags, AFLAG_NOPKILL )
              &&   !IS_SET( ch->in_room->area->flags, AFLAG_NOPKILL )
              && (  IS_PKILL (ch) && !IS_IMMORTAL (ch) ) )
        {
            set_char_color( AT_MAGIC, ch );
            send_to_char( "\n\rA godly force forbids deadly characters from entering that area...\n\r", ch);
            return rSTOP;
        }

        if ( !IS_SET( pexit->exit_info, EX_PORTAL ) || pexit->vdir != DIR_PORTAL )
        {
            if ( in_room->sector_type == SECT_AIR
                 ||   to_room->sector_type == SECT_AIR
                 ||   IS_SET( pexit->exit_info, EX_FLY ) )
            {
                if ( ch->mount && !IS_AFFECTED( ch->mount, AFF_FLYING ) )
                {
                    send_to_char( "Your mount can't fly.\n\r", ch );
                    return rSTOP;
                }
                if ( !ch->mount && !IS_AFFECTED(ch, AFF_FLYING) )
                {
                    send_to_char( "You'd need to fly to go there.\n\r", ch );
                    return rSTOP;
                }
            }

            if ( !ch->mount && will_drown(ch, pexit->to_room) )
            {
                OBJ_DATA *obj;
                bool found = FALSE;

                if (pexit->to_room->sector_type == SECT_WATER_NOSWIM ||
                    pexit->to_room->sector_type == SECT_WATER_SWIM)
                {
                    /*
                     * Look for a boat.
                     */
                    if ( !found )
                        for ( obj = ch->first_carrying; obj; obj = obj->next_content )
                        {
                            if ( obj->item_type == ITEM_BOAT )
                            {
                                found = TRUE;
                                if ( drunk )
                                    strcpy(txt, "paddles unevenly");
                                else
                                    strcpy(txt, "paddles");
                                break;
                            }
                        }

                    if ( !found )
                    {
                        send_to_char( "You would need a boat to go there.\n\r", ch );
                        return rSTOP;
                    }
                }
                if (pexit->to_room->sector_type == SECT_OCEANFLOOR ||
                    pexit->to_room->sector_type == SECT_UNDERWATER)
                {
                    send_to_char( "You can't breathe underwater.\n\r", ch );
                    return rSTOP;
                }
            }
            if ( ch->mount && will_drown(ch->mount, pexit->to_room) )
            {
                send_to_char( "Your mount refuses to go that way.\n\r", ch );
                return rSTOP;
            }

            if ( IS_SET( pexit->exit_info, EX_CLIMB ) )
            {
                bool found;

                found = FALSE;
                if ( ch->mount && IS_AFFECTED( ch->mount, AFF_FLYING ) )
                    found = TRUE;
                else
                    if ( IS_AFFECTED(ch, AFF_FLYING) )
                        found = TRUE;

                if ( !found && !ch->mount )
                {
                    if ( ( !IS_NPC(ch) && number_percent( ) > LEARNED(ch, gsn_climb) )
                         ||      drunk || ch->mental_state < -90 )
                    {
                        send_to_char( "You start to climb... but lose your grip and fall!\n\r", ch);
                        learn_from_failure( ch, gsn_climb );
                        if ( pexit->vdir == DIR_DOWN )
                        {
                            retcode = move_char( ch, pexit, 1 );
                            return retcode;
                        }
                        set_char_color( AT_HURT, ch );
                        send_to_char( "OUCH! You hit the ground!\n\r", ch );
                        WAIT_STATE( ch, PULSE_VIOLENCE*6 );
                        retcode = damage( ch, ch, (pexit->vdir == DIR_UP ? 10 : 5),
                                          TYPE_UNDEFINED );
                        return retcode;
                    }
                    found = TRUE;
                    learn_from_success( ch, gsn_climb );
                    spell_lag(ch, gsn_climb);
                    strcpy(txt, "climbs");
                }

                if ( !found )
                {
                    send_to_char( "You can't climb.\n\r", ch );
                    return rSTOP;
                }
            }
        }
        if ( ch->mount )
        {
            switch (ch->mount->position)
            {
            case POS_DEAD:
                send_to_char( "Your mount is dead!\n\r", ch );
                return rSTOP;
                break;

            case POS_MORTAL:
            case POS_INCAP:
                send_to_char( "Your mount is hurt far too badly to move.\n\r", ch );
                return rSTOP;
                break;

            case POS_STUNNED:
                send_to_char( "Your mount is too stunned to do that.\n\r", ch );
                return rSTOP;
                break;

            case POS_SLEEPING:
                send_to_char( "Your mount is sleeping.\n\r", ch );
                return rSTOP;
                break;

            case POS_RESTING:
                send_to_char( "Your mount is resting.\n\r", ch);
                return rSTOP;
                break;

            case POS_SITTING:
                send_to_char( "Your mount is sitting down.\n\r", ch);
                return rSTOP;
                break;

            case POS_MEDITATING:
                send_to_char( "Your mount is meditating.\n\r", ch);
                return rSTOP;
                break;


            default:
                break;
            }

            if ( IS_AFFECTED(ch->mount, AFF_FLYING) ||
                 IS_AFFECTED(ch->mount, AFF_FLOATING) ||
                 (is_affected(ch->mount, gsn_travelling) &&
                  !room_is_inside(ch->in_room)))
                move = 0;
            else
                move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)];
            if ( GET_MOVE(ch->mount) < move )
            {
                send_to_char( "Your mount is too exhausted.\n\r", ch );
                return rSTOP;
            }
        }
        else
        {
            if ( IS_AFFECTED(ch, AFF_FLYING) ||
                 IS_AFFECTED(ch, AFF_FLOATING) ||
                 (is_affected(ch, gsn_travelling) &&
                  !room_is_inside(ch->in_room)))
                move = 0;
            else
                move = encumbrance( ch, movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)] );
            if ( GET_MOVE(ch) < move )
            {
                send_to_char( "You are too exhausted.\n\r", ch );
                return rSTOP;
            }
        }

        WAIT_STATE( ch, move );
        if ( ch->mount )
            GET_MOVE(ch->mount) -= move;
        else
            GET_MOVE(ch) -= move;
    }

    /*
     * Check if player can fit in the room
     */
    if ( to_room->tunnel > 0 )
    {
        CHAR_DATA *ctmp;
        int count = ch->mount ? 1 : 0;

        for ( ctmp = to_room->first_person; ctmp; ctmp = ctmp->next_in_room )
            if ( ++count >= to_room->tunnel )
            {
                if ( ch->mount && count == to_room->tunnel )
                    send_to_char( "There is no room for both you and your mount in there.\n\r", ch );
                else
                    send_to_char( "There is no room for you in there.\n\r", ch );
                return rSTOP;
            }
    }

    /* check for traps on exit - later */

    if ( !IS_AFFECTED(ch, AFF_SNEAK)
         && ( IS_NPC(ch) || !ch->pcdata->wizinvis ) )
    {
        if ( fall )
            strcpy(txt, "falls");
        else if ( !txt[0] )
        {
            if ( ch->mount )
            {
                if ( IS_AFFECTED( ch->mount, AFF_FLOATING ) )
                    strcpy(txt, "floats");
                else if ( IS_AFFECTED( ch->mount, AFF_FLYING ) )
                    strcpy(txt, "flys");
                else
                    strcpy(txt, "rides");
            }
            else if ( IS_AFFECTED( ch, AFF_FLOATING ) )
            {
                if ( drunk )
                    strcpy(txt, "floats unsteadily");
                else
                    strcpy(txt, "floats");
            }
            else if ( IS_AFFECTED( ch, AFF_FLYING ) )
            {
                if ( drunk )
                    strcpy(txt, "flys shakily");
                else
                    strcpy(txt, "flys");
            }
            else if ( ch->position == POS_SHOVE )
                strcpy(txt, "is shoved");
            else if ( ch->position == POS_DRAG )
                strcpy(txt, "is dragged");
            else if ( drunk )
                strcpy(txt, "stumbles drunkenly");
            else
                strcpy(txt, race_exit_msgs[GET_RACE(ch)]);
        }
        if ( ch->mount )
        {
            sprintf( buf, "$n %s %s upon $N.", txt, dir_name(door) );
            act( AT_ACTION, buf, ch, NULL, ch->mount, TO_NOTVICT );
        }
        else
        {
            sprintf( buf, "$n %s $T.", txt );
            act( AT_ACTION, buf, ch, NULL, dir_name(door), TO_ROOM );
        }
    }

    rprog_leave_trigger( ch );
    if( char_died(ch) )
        return global_retcode;

    mprog_leave_trigger( ch );
    if( char_died(ch) )
        return global_retcode;

    char_from_room( ch );
    if ( ch->mount )
    {
        rprog_leave_trigger( ch->mount );
        if( char_died(ch) )
            return global_retcode;

        mprog_leave_trigger( ch->mount );
        if( char_died(ch) )
            return global_retcode;

        if( ch->mount )
        {
            char_from_room( ch->mount );
            char_to_room( ch->mount, to_room );
        }
    }

    char_to_room( ch, to_room );

    quest_trigger_roomfind(ch, ch->in_room);

    if ( !IS_AFFECTED(ch, AFF_SNEAK)
         && ( IS_NPC(ch) || !ch->pcdata->wizinvis ) )
    {
        if ( fall )
            strcpy(txt, "falls");
        else if ( ch->mount )
        {
            if ( IS_AFFECTED( ch->mount, AFF_FLOATING ) )
                strcpy(txt, "floats in");
            else
                if ( IS_AFFECTED( ch->mount, AFF_FLYING ) )
                    strcpy(txt, "flys in");
                else
                    strcpy(txt, "rides in");
        }
        else if ( IS_AFFECTED( ch, AFF_FLOATING ) )
        {
            if ( drunk )
                strcpy(txt, "floats in unsteadily");
            else
                strcpy(txt, "floats in");
        }
        else if ( IS_AFFECTED( ch, AFF_FLYING ) )
        {
            if ( drunk )
                strcpy(txt, "flys in shakily");
            else
                strcpy(txt, "flys in");
        }
        else if ( ch->position == POS_SHOVE )
            strcpy(txt, "is shoved in");
        else if ( ch->position == POS_DRAG )
            strcpy(txt, "is dragged in");
        else
        {
            if ( drunk )
                strcpy(txt, "stumbles drunkenly in");
            else
                sprintf(txt, "%s in", race_exit_msgs[GET_RACE(ch)]);
        }
        switch( door )
        {
        default: dtxt = "somewhere";	break;
        case 0:  dtxt = "the south";	break;
        case 1:  dtxt = "the west";	break;
        case 2:  dtxt = "the north";	break;
        case 3:  dtxt = "the east";	break;
        case 4:  dtxt = "below";		break;
        case 5:  dtxt = "above";		break;
        case 6:  dtxt = "the south-west";	break;
        case 7:  dtxt = "the south-east";	break;
        case 8:  dtxt = "the north-west";	break;
        case 9:  dtxt = "the north-east";	break;
        }
        if ( ch->mount )
        {
            sprintf( buf, "$n %s from %s upon $N.", txt, dtxt );
            act( AT_ACTION, buf, ch, NULL, ch->mount, TO_ROOM );
        }
        else
        {
            sprintf( buf, "$n %s from %s.", txt, dtxt );
            act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
        }
    }

    if ( !IS_IMMORTAL(ch)
         &&  !IS_NPC(ch)
         &&  ch->in_room->area != to_room->area )
    {
        if ( GetMaxLevel(ch) < to_room->area->low_soft_range )
        {
            set_char_color( AT_MAGIC, ch );
            send_to_char("You feel uncomfortable being in this strange land...\n\r", ch);
        }
        else
            if ( GetMaxLevel(ch) > to_room->area->hi_soft_range )
            {
                set_char_color( AT_MAGIC, ch );
                send_to_char("You feel there is not much to gain visiting this place...\n\r", ch);
            }
    }

    /* Make sure everyone sees the room description of death traps. */
    if ( IS_SET( ch->in_room->room_flags, ROOM_DEATH ) && !IS_IMMORTAL( ch ) )
    {
        if( IS_SET( ch->act, PLR_BRIEF ) )
            brief = TRUE;
        REMOVE_BIT( ch->act, PLR_BRIEF );
    }

    do_look( ch, "auto" );
    if ( brief )
        SET_BIT( ch->act, PLR_BRIEF );

    /*
     * Put good-old EQ-munching death traps back in!		-Thoric
     */
    if ( IS_SET( ch->in_room->room_flags, ROOM_DEATH ) && !IS_IMMORTAL( ch ) )
    {
        act( AT_DEAD, "$n falls prey to a terrible death!", ch, NULL, NULL, TO_ROOM );
        set_char_color( AT_DEAD, ch );
        send_to_char( "Oopsie... you're dead!\n\r", ch );
        sprintf(log_buf, "%s hit a DEATH TRAP in room %d!",
                ch->name, ch->in_room->vnum );
        log_string_plus(log_buf,LOG_MONITOR,LEVEL_IMMORTAL,SEV_CRIT);
        extract_char( ch, FALSE );
        return rCHAR_DIED;
    }

    /* BIG ugly looping problem here when the character is mptransed back
     to the starting room.  To avoid this, check how many chars are in
     the room at the start and stop processing followers after doing
     the right number of them.  -- Narn
     */
    if ( !fall )
    {
        CHAR_DATA *fch;
        CHAR_DATA *nextinroom;
        int chars = 0, count = 0;

        for ( fch = from_room->first_person; fch; fch = fch->next_in_room )
            chars++;

        for ( fch = from_room->first_person; fch && ( count < chars ); fch = nextinroom )
        {
            nextinroom = fch->next_in_room;
            count++;
            if ( fch != ch		/* loop room bug fix here by Thoric */
                 && fch->master == ch
                 && fch->position == POS_STANDING )
            {
                act( AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR );
                move_char( fch, pexit, 0 );
            }
        }
    }

    if ( ch->in_room->first_content )
        retcode = check_room_for_traps( ch, TRAP_ENTER_ROOM );
    if ( retcode != rNONE )
        return retcode;

    if ( char_died(ch) )
        return retcode;

    mprog_entry_trigger( ch );
    if ( char_died(ch) )
        return retcode;

    rprog_enter_trigger( ch );
    if ( char_died(ch) )
        return retcode;

    mprog_greet_trigger( ch );
    if ( char_died(ch) )
        return retcode;

    oprog_greet_trigger( ch );
    if ( char_died(ch) )
        return retcode;

    if (!will_fall( ch, fall )
        &&   fall > 0 )
    {
        if (!IS_AFFECTED( ch, AFF_FLOATING )
            || ( ch->mount && !IS_AFFECTED( ch->mount, AFF_FLOATING ) ) )
        {
            set_char_color( AT_HURT, ch );
            send_to_char( "OUCH! You hit the ground!\n\r", ch );
            WAIT_STATE( ch, PULSE_VIOLENCE*6 );
            retcode = damage( ch, ch, 20 * fall, TYPE_UNDEFINED );
        }
        else
        {
            set_char_color( AT_MAGIC, ch );
            send_to_char( "You lightly float down to the ground.\n\r", ch );
        }
    }
    return retcode;
}


void do_north( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_NORTH), 0 );
    return;
}


void do_east( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_EAST), 0 );
    return;
}


void do_south( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_SOUTH), 0 );
    return;
}


void do_west( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_WEST), 0 );
    return;
}


void do_up( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_UP), 0 );
    return;
}


void do_down( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_DOWN), 0 );
    return;
}

void do_northeast( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_NORTHEAST), 0 );
    return;
}

void do_northwest( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_NORTHWEST), 0 );
    return;
}

void do_southeast( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_SOUTHEAST), 0 );
    return;
}

void do_southwest( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_SOUTHWEST), 0 );
    return;
}

EXIT_DATA *find_door( CHAR_DATA *ch, char *arg, bool quiet )
{
    EXIT_DATA *pexit;
    int door;

    if (arg == NULL || !str_cmp(arg,""))
        return NULL;

    pexit = NULL;
    if ( !str_cmp( arg, "n"  ) || !str_cmp( arg, "north"	  ) ) door = DIR_NORTH;
    else if ( !str_cmp( arg, "e"  ) || !str_cmp( arg, "east"	  ) ) door = DIR_EAST;
    else if ( !str_cmp( arg, "s"  ) || !str_cmp( arg, "south"	  ) ) door = DIR_SOUTH;
    else if ( !str_cmp( arg, "w"  ) || !str_cmp( arg, "west"	  ) ) door = DIR_WEST;
    else if ( !str_cmp( arg, "u"  ) || !str_cmp( arg, "up"	  ) ) door = DIR_UP;
    else if ( !str_cmp( arg, "d"  ) || !str_cmp( arg, "down"	  ) ) door = DIR_DOWN;
    else if ( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast" ) ) door = DIR_NORTHEAST;
    else if ( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest" ) ) door = DIR_NORTHWEST;
    else if ( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast" ) ) door = DIR_SOUTHEAST;
    else if ( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest" ) ) door = DIR_SOUTHWEST;
    else if ( !str_cmp( arg, "portal" ) ) door = DIR_PORTAL;
    else
    {
        for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
        {
            if ( (quiet || IS_SET(pexit->exit_info, EX_ISDOOR))
                 &&    pexit->keyword
                 &&    nifty_is_name( arg, pexit->keyword ) )
                return pexit;
        }
        if ( !quiet )
            act( AT_PLAIN, "You see no $T here.", ch, NULL, arg, TO_CHAR );
        return NULL;
    }

    if ( (pexit = get_exit( ch->in_room, door )) == NULL )
    {
        if ( !quiet)
            act( AT_PLAIN, "You see no $T here.", ch, NULL, arg, TO_CHAR );
        return NULL;
    }

    if ( quiet )
        return pexit;

    if ( IS_SET(pexit->exit_info, EX_SECRET) )
    {
        act( AT_PLAIN, "You see no $T here.", ch, NULL, arg, TO_CHAR );
        return NULL;
    }

    if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
        send_to_char( "You can't do that.\n\r", ch );
        return NULL;
    }

    return pexit;
}


void toggle_bexit_flag( EXIT_DATA *pexit, int flag )
{
    EXIT_DATA *pexit_rev;

    TOGGLE_BIT(pexit->exit_info, flag);
    if ( (pexit_rev = pexit->rexit) != NULL
         &&   pexit_rev != pexit )
        TOGGLE_BIT( pexit_rev->exit_info, flag );
}

void set_bexit_flag( EXIT_DATA *pexit, int flag )
{
    EXIT_DATA *pexit_rev;

    SET_BIT(pexit->exit_info, flag);
    if ( (pexit_rev = pexit->rexit) != NULL
         &&   pexit_rev != pexit )
        SET_BIT( pexit_rev->exit_info, flag );
}

void remove_bexit_flag( EXIT_DATA *pexit, int flag )
{
    EXIT_DATA *pexit_rev;

    REMOVE_BIT(pexit->exit_info, flag);
    if ( (pexit_rev = pexit->rexit) != NULL
         &&   pexit_rev != pexit )
        REMOVE_BIT( pexit_rev->exit_info, flag );
}

void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    EXIT_DATA *pexit;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Open what?\n\r", ch );
        return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
        /* 'open door' */
        EXIT_DATA *pexit_rev;

        if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
        { send_to_char( "You can't do that.\n\r",      ch ); return; }
        if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
        { send_to_char( "It's already open.\n\r",      ch ); return; }
        if (  IS_SET(pexit->exit_info, EX_LOCKED) )
        { send_to_char( "It's locked.\n\r",            ch ); return; }

        if ( !IS_SET(pexit->exit_info, EX_SECRET)
             ||   (pexit->keyword && nifty_is_name( arg, pexit->keyword )) )
        {
            act( AT_ACTION, "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM );
            act( AT_ACTION, "You open the $d.", ch, NULL, pexit->keyword, TO_CHAR );
            if ( (pexit_rev = pexit->rexit) != NULL
                 &&   pexit_rev->to_room == ch->in_room )
            {
                CHAR_DATA *rch;

                for ( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )
                    act( AT_ACTION, "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
            }
            remove_bexit_flag( pexit, EX_CLOSED );
            if ( (door=pexit->vdir) >= 0 && door < 10 )
                check_room_for_traps( ch, trap_door[door]);
            return;
        }
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
        /* 'open object' */
        if ( obj->item_type != ITEM_CONTAINER )
        {
            ch_printf( ch, "%s is not a container.\n\r", capitalize( obj->short_descr ) );
            return;
        }
        if ( !IS_SET(obj->value[1], CONT_CLOSED) )
        {
            ch_printf( ch, "%s is already open.\n\r", capitalize( obj->short_descr ) );
            return;
        }
        if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
        {
            ch_printf( ch, "%s cannot be opened or closed.\n\r", capitalize( obj->short_descr ) );
            return;
        }
        if ( IS_SET(obj->value[1], CONT_LOCKED) )
        {
            ch_printf( ch, "%s is locked.\n\r", capitalize( obj->short_descr ) );
            return;
        }

        REMOVE_BIT(obj->value[1], CONT_CLOSED);
        act( AT_ACTION, "You open $p.", ch, obj, NULL, TO_CHAR );
        act( AT_ACTION, "$n opens $p.", ch, obj, NULL, TO_ROOM );
        check_for_trap( ch, obj, TRAP_OPEN );
        return;
    }

    ch_printf( ch, "You see no %s here.\n\r", arg );
    return;
}



void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    EXIT_DATA *pexit;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Close what?\n\r", ch );
        return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
        /* 'close door' */
        EXIT_DATA *pexit_rev;

        if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
        { send_to_char( "You can't do that.\n\r",      ch ); return; }
        if ( IS_SET(pexit->exit_info, EX_CLOSED) )
        { send_to_char( "It's already closed.\n\r",    ch ); return; }

        act( AT_ACTION, "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM );
        act( AT_ACTION, "You close the $d.", ch, NULL, pexit->keyword, TO_CHAR );

        /* close the other side */
        if ( ( pexit_rev = pexit->rexit ) != NULL
             &&   pexit_rev->to_room == ch->in_room )
        {
            CHAR_DATA *rch;

            SET_BIT( pexit_rev->exit_info, EX_CLOSED );
            for ( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )
                act( AT_ACTION, "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
        }
        set_bexit_flag( pexit, EX_CLOSED );
        if ( (door=pexit->vdir) >= 0 && door < 10 )
            check_room_for_traps( ch, trap_door[door]);
        return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
        /* 'close object' */
        if ( obj->item_type != ITEM_CONTAINER )
        {
            ch_printf( ch, "%s is not a container.\n\r", capitalize( obj->short_descr ) );
            return;
        }
        if ( IS_SET(obj->value[1], CONT_CLOSED) )
        {
            ch_printf( ch, "%s is already closed.\n\r", capitalize( obj->short_descr ) );
            return;
        }
        if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
        {
            ch_printf( ch, "%s cannot be opened or closed.\n\r", capitalize( obj->short_descr ) );
            return;
        }

        SET_BIT(obj->value[1], CONT_CLOSED);
        act( AT_ACTION, "You close $p.", ch, obj, NULL, TO_CHAR );
        act( AT_ACTION, "$n closes $p.", ch, obj, NULL, TO_ROOM );
        check_for_trap( ch, obj, TRAP_CLOSE );
        return;
    }

    ch_printf( ch, "You see no %s here.\n\r", arg );
    return;
}


bool has_key( CHAR_DATA *ch, int key )
{
    OBJ_DATA *obj;

    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
        if ( obj->vnum == key || obj->value[0] == key )
            return TRUE;

    return FALSE;
}


void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    EXIT_DATA *pexit;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Lock what?\n\r", ch );
        return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
        /* 'lock door' */

        if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
        { send_to_char( "You can't do that.\n\r",      ch ); return; }
        if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
        { send_to_char( "It's not closed.\n\r",        ch ); return; }
        if ( pexit->key < 0 )
        { send_to_char( "It can't be locked.\n\r",     ch ); return; }
        if ( !has_key( ch, pexit->key) )
        { send_to_char( "You lack the key.\n\r",       ch ); return; }
        if ( IS_SET(pexit->exit_info, EX_LOCKED) )
        { send_to_char( "It's already locked.\n\r",    ch ); return; }

        if ( !IS_SET(pexit->exit_info, EX_SECRET)
             ||   (pexit->keyword && nifty_is_name( arg, pexit->keyword )) )
        {
            send_to_char( "*Click*\n\r", ch );
            act( AT_ACTION, "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
            set_bexit_flag( pexit, EX_LOCKED );
            return;
        }
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
        /* 'lock object' */
        if ( obj->item_type != ITEM_CONTAINER )
        { send_to_char( "That's not a container.\n\r", ch ); return; }
        if ( !IS_SET(obj->value[1], CONT_CLOSED) )
        { send_to_char( "It's not closed.\n\r",        ch ); return; }
        if ( obj->value[2] < 0 )
        { send_to_char( "It can't be locked.\n\r",     ch ); return; }
        if ( !has_key( ch, obj->value[2] ) )
        { send_to_char( "You lack the key.\n\r",       ch ); return; }
        if ( IS_SET(obj->value[1], CONT_LOCKED) )
        { send_to_char( "It's already locked.\n\r",    ch ); return; }

        SET_BIT(obj->value[1], CONT_LOCKED);
        send_to_char( "*Click*\n\r", ch );
        act( AT_ACTION, "$n locks $p.", ch, obj, NULL, TO_ROOM );
        return;
    }

    ch_printf( ch, "You see no %s here.\n\r", arg );
    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    EXIT_DATA *pexit;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Unlock what?\n\r", ch );
        return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
        /* 'unlock door' */

        if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
        { send_to_char( "You can't do that.\n\r",      ch ); return; }
        if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
        { send_to_char( "It's not closed.\n\r",        ch ); return; }
        if ( pexit->key < 0 )
        { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
        if ( !has_key( ch, pexit->key) )
        { send_to_char( "You lack the key.\n\r",       ch ); return; }
        if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
        { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

        if ( !IS_SET(pexit->exit_info, EX_SECRET)
             ||   (pexit->keyword && nifty_is_name( arg, pexit->keyword )) )
        {
            send_to_char( "*Click*\n\r", ch );
            act( AT_ACTION, "$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
            remove_bexit_flag( pexit, EX_LOCKED );
            return;
        }
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
        /* 'unlock object' */
        if ( obj->item_type != ITEM_CONTAINER )
        { send_to_char( "That's not a container.\n\r", ch ); return; }
        if ( !IS_SET(obj->value[1], CONT_CLOSED) )
        { send_to_char( "It's not closed.\n\r",        ch ); return; }
        if ( obj->value[2] < 0 )
        { send_to_char( "It can't be unlocked.\n\r",   ch ); return; }
        if ( !has_key( ch, obj->value[2] ) )
        { send_to_char( "You lack the key.\n\r",       ch ); return; }
        if ( !IS_SET(obj->value[1], CONT_LOCKED) )
        { send_to_char( "It's already unlocked.\n\r",  ch ); return; }

        REMOVE_BIT(obj->value[1], CONT_LOCKED);
        send_to_char( "*Click*\n\r", ch );
        act( AT_ACTION, "$n unlocks $p.", ch, obj, NULL, TO_ROOM );
        return;
    }

    ch_printf( ch, "You see no %s here.\n\r", arg );
    return;
}

void do_bashdoor( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *gch;
    EXIT_DATA *pexit;
    char       arg [ MAX_INPUT_LENGTH ];

    if ( !IS_NPC( ch ) && !CanUseSkill(ch, gsn_doorbash) )
    {
        send_to_char( "You're not enough of a warrior to bash doors!\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Bash what?\n\r", ch );
        return;
    }

    if ( ch->fighting )
    {
        send_to_char( "You can't break off your fight.\n\r", ch );
        return;
    }

    if ( ( pexit = find_door( ch, arg, FALSE ) ) != NULL )
    {
        ROOM_INDEX_DATA *to_room;
        EXIT_DATA       *pexit_rev = NULL;
        char	    *keyword;
        int percent;

        if ( !IS_SET( pexit->exit_info, EX_CLOSED ) )
        {
            send_to_char( "Calm down.  It is already open.\n\r", ch );
            return;
        }

        spell_lag(ch, gsn_doorbash);

        if ( IS_SET( pexit->exit_info, EX_SECRET ) )
            keyword = "wall";
        else
            keyword = pexit->keyword;
        if ( !IS_NPC(ch) )
            percent = LEARNED(ch, gsn_doorbash) / 2;
        else
            percent = 90;
        if ( IS_SET( pexit->exit_info, EX_LOCKED ) )
            percent /= 3;

        if ( !IS_SET( pexit->exit_info, EX_BASHPROOF )
             &&   GET_MOVE(ch) >= 15
             &&   number_percent( ) < ( percent + 4 * ( get_curr_str( ch ) - 19 ) ) )
        {
            REMOVE_BIT( pexit->exit_info, EX_CLOSED );
            if ( IS_SET( pexit->exit_info, EX_LOCKED ) )
                REMOVE_BIT( pexit->exit_info, EX_LOCKED );
            SET_BIT( pexit->exit_info, EX_BASHED );

            act(AT_SKILL, "Crash!  You bashed open the $d!", ch, NULL, keyword, TO_CHAR );
            act(AT_SKILL, "$n bashes open the $d!",          ch, NULL, keyword, TO_ROOM );
            learn_from_success(ch, gsn_doorbash);

            if ( (to_room = pexit->to_room) != NULL
                 &&   (pexit_rev = pexit->rexit) != NULL
                 &&    pexit_rev->to_room	== ch->in_room )
            {
                CHAR_DATA *rch;

                REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
                if ( IS_SET( pexit_rev->exit_info, EX_LOCKED ) )
                    REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
                SET_BIT( pexit_rev->exit_info, EX_BASHED );

                for ( rch = to_room->first_person; rch; rch = rch->next_in_room )
                {
                    act(AT_SKILL, "The $d crashes open!",
                        rch, NULL, pexit_rev->keyword, TO_CHAR );
                }
            }
            damage( ch, ch, ( GET_MAX_HIT(ch) / 20 ), gsn_doorbash );

        }
        else
        {
            act(AT_SKILL, "WHAAAAM!!!  You bash against the $d, but it doesn't budge.",
                ch, NULL, keyword, TO_CHAR );
            act(AT_SKILL, "WHAAAAM!!!  $n bashes against the $d, but it holds strong.",
                ch, NULL, keyword, TO_ROOM );
            damage( ch, ch, ( GET_MAX_HIT(ch) / 20 ) + 10, gsn_doorbash );
            learn_from_failure(ch, gsn_doorbash);
        }
    }
    else
    {
        act(AT_SKILL, "WHAAAAM!!!  You bash against the wall, but it doesn't budge.",
            ch, NULL, NULL, TO_CHAR );
        act(AT_SKILL, "WHAAAAM!!!  $n bashes against the wall, but it holds strong.",
            ch, NULL, NULL, TO_ROOM );
        damage( ch, ch, ( GET_MAX_HIT(ch) / 20 ) + 10, gsn_doorbash );
        learn_from_failure(ch, gsn_doorbash);
    }
    if ( !char_died( ch ) )
        for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
        {
            if ( IS_AWAKE( gch )
                 && !gch->fighting
                 && ( IS_NPC( gch ) && !IS_AFFECTED( gch, AFF_CHARM ) )
                 && ( GetMaxLevel(ch) - GetMaxLevel(gch) <= 4 )
                 && number_bits( 2 ) == 0 )
                multi_hit( gch, ch, TYPE_UNDEFINED );
        }

    return;
}


void do_stand( CHAR_DATA *ch, char *argument )
{
    stop_memorizing(ch);

    switch ( ch->position )
    {
    case POS_SLEEPING:
        if ( IS_AFFECTED(ch, AFF_SLEEP) )
        { send_to_char( "You can't seem to wake up!\n\r", ch ); return; }

        send_to_char( "You wake and climb quickly to your feet.\n\r", ch );
        act( AT_ACTION, "$n arises from $s slumber.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_STANDING;
        break;

    case POS_RESTING:
        send_to_char( "You gather yourself and stand up.\n\r", ch );
        act( AT_ACTION, "$n rises from $s rest.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_STANDING;
        break;

    case POS_SITTING:
    case POS_MEDITATING:
        send_to_char( "You move quickly to your feet.\n\r", ch );
        act( AT_ACTION, "$n rises quickly to $s feet.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_STANDING;
        break;

    case POS_STANDING:
        send_to_char( "You are already standing.\n\r", ch );
        break;

    case POS_FIGHTING:
        send_to_char( "You are already fighting!\n\r", ch );
        break;
    }

    return;
}


void do_sit( CHAR_DATA *ch, char *argument )
{
    stop_memorizing(ch);

    switch ( ch->position )
    {
    case POS_SLEEPING:
        if ( IS_AFFECTED(ch, AFF_SLEEP) )
        { send_to_char( "You can't seem to wake up!\n\r", ch ); return; }

        send_to_char( "You wake and sit up.\n\r", ch );
        act( AT_ACTION, "$n wakes and sits up.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_SITTING;
        break;

    case POS_RESTING:
        send_to_char( "You stop resting and sit up.\n\r", ch );
        act( AT_ACTION, "$n stops resting and sits up.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_SITTING;
        break;

    case POS_MEDITATING:
        send_to_char( "You stop meditating and relax.\n\r", ch );
        act( AT_ACTION, "$n stops humming and sits up.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_SITTING;
        break;

    case POS_STANDING:
        send_to_char( "You sit down.\n\r", ch );
        act( AT_ACTION, "$n sits down.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_SITTING;
        break;
    case POS_SITTING:
        send_to_char( "You are already sitting.\n\r", ch );
        return;

    case POS_FIGHTING:
        send_to_char( "You are busy fighting!\n\r", ch );
        return;
    case POS_MOUNTED:
        send_to_char( "You are already sitting - on your mount.\n\r", ch );
        return;
    }

    rprog_rest_trigger( ch );
    return;
}


void do_rest( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
        if ( IS_AFFECTED(ch, AFF_SLEEP) )
        { send_to_char( "You can't seem to wake up!\n\r", ch ); return; }

        send_to_char( "You rouse from your slumber.\n\r", ch );
        act( AT_ACTION, "$n rouses from $s slumber.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_RESTING;
        break;

    case POS_RESTING:
        send_to_char( "You are already resting.\n\r", ch );
        return;

    case POS_STANDING:
        send_to_char( "You sprawl out haphazardly.\n\r", ch );
        act( AT_ACTION, "$n sprawls out haphazardly.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_RESTING;
        break;

    case POS_SITTING:
    case POS_MEDITATING:
        send_to_char( "You lie back and sprawl out to rest.\n\r", ch );
        act( AT_ACTION, "$n lies back and sprawls out to rest.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_RESTING;
        break;

    case POS_FIGHTING:
        send_to_char( "You are busy fighting!\n\r", ch );
        return;
    case POS_MOUNTED:
        send_to_char( "You'd better dismount first.\n\r", ch );
        return;
    }

    rprog_rest_trigger( ch );
    return;
}


void do_sleep( CHAR_DATA *ch, char *argument )
{
    stop_memorizing(ch);

    switch ( ch->position )
    {
    case POS_SLEEPING:
        send_to_char( "You are already sleeping.\n\r", ch );
        return;

    case POS_RESTING:
        if ( ch->mental_state > 30 && (number_percent()+10) < ch->mental_state )
        {
            send_to_char( "You just can't seem to calm yourself down enough to sleep.\n\r", ch );
            act( AT_ACTION, "$n closes $s eyes for a few moments, but just can't seem to go to sleep.", ch, NULL, NULL, TO_ROOM );
            return;
        }
        send_to_char( "You close your eyes and drift into slumber.\n\r", ch );
        act( AT_ACTION, "$n closes $s eyes and drifts into a deep slumber.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_SLEEPING;
        break;

    case POS_MEDITATING:
    case POS_SITTING:
        if ( ch->mental_state > 30 && (number_percent()+5) < ch->mental_state )
        {
            send_to_char( "You just can't seem to calm yourself down enough to sleep.\n\r", ch );
            act( AT_ACTION, "$n closes $s eyes for a few moments, but just can't seem to go to sleep.", ch, NULL, NULL, TO_ROOM );
            return;
        }
        send_to_char( "You slump over and fall dead asleep.\n\r", ch );
        act( AT_ACTION, "$n nods off and slowly slumps over, dead asleep.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_SLEEPING;
        break;

    case POS_STANDING:
        if ( ch->mental_state > 30 && number_percent() < ch->mental_state )
        {
            send_to_char( "You just can't seem to calm yourself down enough to sleep.\n\r", ch );
            act( AT_ACTION, "$n closes $s eyes for a few moments, but just can't seem to go to sleep.", ch, NULL, NULL, TO_ROOM );
            return;
        }
        send_to_char( "You collapse into a deep sleep.\n\r", ch );
        act( AT_ACTION, "$n collapses into a deep sleep.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_SLEEPING;
        break;

    case POS_FIGHTING:
        send_to_char( "You are busy fighting!\n\r", ch );
        return;
    case POS_MOUNTED:
        send_to_char( "You really should dismount first.\n\r", ch );
        return;
    }

    if (IS_ACTIVE(ch, CLASS_PSIONIST) && GET_LEVEL(ch, CLASS_PSIONIST)<10)
        send_to_char("(Remember: to enter the psionist guild, type psitrain when sleeping)\n\r", ch);

    rprog_sleep_trigger( ch );
    return;
}


void do_wake( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    { do_stand( ch, argument ); return; }

    if ( !IS_AWAKE(ch) )
    { send_to_char( "You are asleep yourself!\n\r",       ch ); return; }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    { send_to_char( "They aren't here.\n\r",              ch ); return; }

    if ( IS_AWAKE(victim) )
    { act( AT_PLAIN, "$N is already awake.", ch, NULL, victim, TO_CHAR ); return; }

    if ( IS_AFFECTED(victim, AFF_SLEEP) || victim->position < POS_SLEEPING )
    { act( AT_PLAIN, "You can't seem to wake $M!",  ch, NULL, victim, TO_CHAR );  return; }

    act( AT_ACTION, "You wake $M.", ch, NULL, victim, TO_CHAR );
    victim->position = POS_STANDING;
    act( AT_ACTION, "$n wakes you.", ch, NULL, victim, TO_VICT );
    return;
}


/*
 * teleport a character to another room
 */
void teleportch( CHAR_DATA *ch, ROOM_INDEX_DATA *room, bool show )
{
    if ( room_is_private( room ) )
        return;
    if ( IS_NPC( ch ) )
        return;

    act( AT_ACTION, "$n disappears suddenly!", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, room );
    act( AT_ACTION, "$n arrives suddenly!", ch, NULL, NULL, TO_ROOM );
    if ( show )
        do_look( ch, "auto" );
    if ( IS_SET( ch->in_room->room_flags, ROOM_DEATH ) && !IS_IMMORTAL( ch ) )
    {
        act( AT_DEAD, "$n falls prey to a terrible death!", ch, NULL, NULL, TO_ROOM );
        set_char_color( AT_DEAD, ch );
        send_to_char( "Oopsie... you're dead!\n\r", ch );
        sprintf(log_buf, "%s hit a DEATH TRAP in room %d!",
                ch->name, ch->in_room->vnum );
        log_string_plus(log_buf,LOG_MONITOR,LEVEL_IMMORTAL,SEV_CRIT);
        extract_char( ch, FALSE );
    }
}

void teleport( CHAR_DATA *ch, int room, int flags )
{
    CHAR_DATA *nch, *nch_next;
    ROOM_INDEX_DATA *pRoomIndex;
    bool show;

    pRoomIndex = get_room_index( room );
    if ( !pRoomIndex || !room )
    {
        bug( "teleport: bad room vnum %d", room );
        return;
    }

    if (ch->in_room->vnum == room)
    {
        bug( "teleport: ch already in room vnum %d", room );
        return;
    }

    if ( IS_SET( flags, TELE_SHOWDESC ) )
        show = TRUE;
    else
        show = FALSE;
    if ( !IS_SET( flags, TELE_TRANSALL ) )
    {
        teleportch( ch, pRoomIndex, show );
        return;
    }
    for ( nch = ch->in_room->first_person; nch; nch = nch_next )
    {
        nch_next = nch->next_in_room;
        teleportch( nch, pRoomIndex, show );
    }
}

/*
 * "Climb" in a certain direction.				-Thoric
 */
void do_climb( CHAR_DATA *ch, char *argument )
{
    EXIT_DATA *pexit;
    bool found;

    found = FALSE;
    if ( argument[0] == '\0' )
    {
        for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
            if ( IS_SET( pexit->exit_info, EX_xCLIMB ) )
            {
                move_char( ch, pexit, 0 );
                return;
            }
        send_to_char( "You cannot climb here.\n\r", ch );
        return;
    }

    if ( (pexit = find_door( ch, argument, TRUE )) != NULL
         &&   IS_SET( pexit->exit_info, EX_xCLIMB ))
    {
        move_char( ch, pexit, 0 );
        return;
    }
    send_to_char( "You cannot climb there.\n\r", ch );
    return;
}

/*
 * "enter" something (moves through an exit)			-Thoric
 */
void do_enter( CHAR_DATA *ch, char *argument )
{
    EXIT_DATA *pexit;
    bool found;

    found = FALSE;
    if ( argument[0] == '\0' )
    {
        for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
            if ( IS_SET( pexit->exit_info, EX_xENTER ) )
            {
                move_char( ch, pexit, 0 );
                return;
            }
        send_to_char( "You cannot find an entrance here.\n\r", ch );
        return;
    }

    if ( (pexit = find_door( ch, argument, TRUE )) != NULL
         &&   IS_SET( pexit->exit_info, EX_xENTER ))
    {
        move_char( ch, pexit, 0 );
        return;
    }
    send_to_char( "You cannot find that here.\n\r", ch );
    return;
}

/*
 * Leave through an exit.					-Thoric
 */
void do_leave( CHAR_DATA *ch, char *argument )
{
    EXIT_DATA *pexit;
    bool found;

    found = FALSE;
    if ( argument[0] == '\0' )
    {
        for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
            if ( IS_SET( pexit->exit_info, EX_xLEAVE ) )
            {
                move_char( ch, pexit, 0 );
                return;
            }
        send_to_char( "You cannot find an exit here.\n\r", ch );
        return;
    }

    if ( (pexit = find_door( ch, argument, TRUE )) != NULL
         &&   IS_SET( pexit->exit_info, EX_xLEAVE ))
    {
        move_char( ch, pexit, 0 );
        return;
    }
    send_to_char( "You cannot find that exit here.\n\r", ch );
    return;
}

void do_run( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *from_room, *in_room;
    EXIT_DATA *pexit;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Run where?\n\r", ch );
        return;
    }

    if ( ch->position != POS_STANDING )
    {
        send_to_char( "You are not in the correct position for that.\n\r", ch );
        return;
    }

    from_room = ch->in_room;

    while ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
        if ( ch->move < 1 )
        {
            send_to_char( "You are too exhausted to run anymore.\n\r", ch );
            ch->move = 0;
            break;
        }
        ch->move -= 5;

        in_room = ch->in_room;

	interpret(ch, exit_name(pexit));

        if (ch->in_room == in_room)
            break;
    }

    if ( ch->in_room == from_room )
    {
        send_to_char( "You try to run but don't get anywhere.\n\r", ch );
        act( AT_ACTION, "$n tries to run but doesn't get anywhere.", ch, NULL, NULL, TO_ROOM );
        return;
    }

    send_to_char( "You slow down after your run.\n\r", ch );
    act( AT_ACTION, "$n slows down after $s run.", ch, NULL, NULL, TO_ROOM );
}

#define PSITRAIN_DEATH 36600
#define PSITRAIN_START 36601
#define PSITRAIN_MOB   36600

void do_psitrain(CHAR_DATA *ch, char *argument)
{
    ROOM_INDEX_DATA *deathroom;
    CHAR_DATA *tchar;
    char buf[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
        return;

    if (!IS_ACTIVE(ch, CLASS_PSIONIST))
    {
        send_to_char("Pardon?\n\r", ch);
        return;
    }

    if ((GET_POS(ch) != POS_SLEEPING) &&
        (GET_POS(ch) != POS_MEDITATING))
    {
        send_to_char("You need to be more relaxed.\n\r", ch);
        return;
    }

    if (!(deathroom = get_room_index(PSITRAIN_DEATH)))
    {
        bug("No psitrain death room.");
        send_to_char("You can't do that right now.\n\r", ch);
        return;
    }

    sprintf(buf, "%s psitrain", GET_NAME(ch));

    if (ch->in_room->area == deathroom->area)
    {
        tchar = get_char_world(ch, buf);
        if (!tchar)
        {
            send_to_char("You could not find your way back to your body!\n\r", ch);
            char_from_room(ch);
            char_to_room(ch, deathroom);
            if (GET_POS(ch) != POS_SLEEPING)
                do_look(ch, "auto");
            return;
        } else {
            char_from_room(ch);
            char_to_room(ch, tchar->in_room);
            extract_char(tchar, TRUE);
            if (GET_POS(ch) != POS_SLEEPING)
                do_look(ch, "auto");
        }
    } else {
        tchar = create_mobile(PSITRAIN_MOB);
        char_to_room(tchar, ch->in_room);
        if (tchar->name)
            STRFREE(tchar->name);
        tchar->name = STRALLOC(buf);

        if (tchar->long_descr)
            STRFREE(tchar->long_descr);
        sprintf(buf, "%s's body is here, but as for the mind, who knows?",
                GET_NAME(ch));
        tchar->long_descr = STRALLOC(buf);

        tchar->max_hit = 1;
        GET_HIT(tchar) = 1;

        if (!IS_SET(tchar->affected_by2, AFF2_BEACON))
            SET_BIT(tchar->affected_by2, AFF2_BEACON);

        if (!IS_SET(tchar->affected_by, AFF_PARALYSIS))
            SET_BIT(tchar->affected_by, AFF_PARALYSIS);

        char_from_room(ch);
        char_to_room(ch, get_room_index(PSITRAIN_START));
        do_stand(ch, "");
        do_look(ch, "auto");

        if (GET_LEVEL(ch, CLASS_PSIONIST)<10)
            send_to_char("\n\r(Remember: to return to the normal world, type psitrain)\n\r", ch);

        return;
    }
}

void do_tag( CHAR_DATA *ch, char *argument )
{
    send_to_char("You can't do that!\n\r", ch);
}
