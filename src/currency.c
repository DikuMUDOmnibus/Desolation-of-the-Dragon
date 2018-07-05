/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*static char rcsid[] = "$Id: currency.c,v 1.27 2003/12/21 17:20:54 dotd Exp $";*/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mud.h"
#include "currency.h"

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

#define CURRENCY_FILE               SYSTEM_DIR "currency.dat"

char *  const   curr_types [] =
{
    "none", "gold", "silver", "bronze", "mithril"
};

char *  const   cap_curr_types [] =
{
    "None", "Gold", "Silver", "Bronze", "Mithril"
};

CURR_INDEX_DATA *first_curr_index;
CURR_INDEX_DATA *last_curr_index;

CURR_INDEX_DATA *mud_curr_index;

int get_currency_type(char *type)
{
    int x;

    if (!str_cmp(type, "coin") || !str_cmp(type, "coins"))
        return DEFAULT_CURR;

    for ( x = 0; x < MAX_CURR_TYPE; x++ )
      if ( !str_cmp( type, curr_types[x] ) )
        return x;

    return CURR_NONE;
}

float get_worth(CURRENCY_DATA *c1, CURRENCY_DATA *c2)
{
    if (!c1 || !c2 || c2->tsiints == 0)
        return 0.0;

    return ((float)c1->tsiints/(float)c2->tsiints);
}

sh_int get_primary_curr(ROOM_INDEX_DATA *room)
{
    if (!room)
        return DEFAULT_CURR;

    if (room->currindex)
        return room->currindex->primary;
    if (room->area->currindex)
        return room->area->currindex->primary;
    return mud_curr_index->primary;
}

char *get_primary_curr_str(ROOM_INDEX_DATA *room)
{
    return cap_curr_types[get_primary_curr(room)];
}

void free_currency(CURR_INDEX_DATA *currindex, CURRENCY_DATA *curr)
{
    UNLINK(curr,currindex->first_currency,currindex->last_currency,next_currency,prev_currency);
    DISPOSE(curr);
}

void free_currindex(CURR_INDEX_DATA *currindex)
{
    CURRENCY_DATA *curr;

    if (!currindex)
        return;

    UNLINK(currindex,first_curr_index,last_curr_index,next_currindex,prev_currindex);

    while ((curr=currindex->first_currency))
        free_currency(currindex,curr);

    if (currindex->name)
        DISPOSE(currindex->name);

    DISPOSE(currindex);
}

void free_currencies(void)
{
    while (first_curr_index)
        free_currindex(first_curr_index);
}

CURR_INDEX_DATA *make_currindex( void )
{
    CURR_INDEX_DATA *currindex;

    CREATE(currindex,CURR_INDEX_DATA,1);

    currindex->vnum           = 0;
    currindex->primary        = CURR_NONE;
    currindex->name           = str_dup("Unnamed");
    currindex->first_currency = NULL;
    currindex->last_currency  = NULL;

    LINK(currindex,first_curr_index,last_curr_index,next_currindex,prev_currindex);

    return currindex;
}

CURRENCY_DATA *make_currency( CURR_INDEX_DATA *currindex )
{
    CURRENCY_DATA *curr;

    CREATE(curr,CURRENCY_DATA,1);

    curr->type    = CURR_NONE;
    curr->tsiints = 0;

    LINK(curr,currindex->first_currency,currindex->last_currency,next_currency,prev_currency);

    return curr;
}

CURR_INDEX_DATA *find_currindex_vnum( int vnum )
{
    CURR_INDEX_DATA *currindex;

    if (!first_curr_index)
        return NULL;

    for (currindex=first_curr_index;currindex;currindex=currindex->next_currindex)
        if (currindex->vnum==vnum)
            return currindex;

    return NULL;
}

CURRENCY_DATA *find_currency(CURR_INDEX_DATA *currindex, int type)
{
    CURRENCY_DATA *curr;

    if (!currindex || !currindex->first_currency)
        return NULL;

    for (curr=currindex->first_currency;curr;curr=curr->next_currency)
        if (curr->type == type)
            return curr;

    return NULL;
}

void init_mud_curr_index(void)
{
    CURRENCY_DATA *c;
    int x;

    mud_curr_index = make_currindex();

    mud_curr_index->primary = DEFAULT_CURR;
    DISPOSE(mud_curr_index->name);
    mud_curr_index->name = str_dup("MUD-Wide Currency");

    for (x=0;x<MAX_CURR_TYPE;x++)
    {
        c = make_currency(mud_curr_index);
        c->type = x;
        switch (x)
        {
        case CURR_GOLD:
            c->tsiints = 500;
            break;
        case CURR_SILVER:
            c->tsiints = 200;
            break;
        case CURR_BRONZE:
            c->tsiints = 100;
            break;
        case CURR_MITHRIL:
            c->tsiints = 10000;
            break;
        default:
            c->tsiints = 150;
            break;
        }
    }
}


int convert_curr(ROOM_INDEX_DATA *room, int amount, int fromtype, int totype)
{
    CURR_INDEX_DATA *rc=NULL, *ac=NULL;
    CURRENCY_DATA *from, *to;

    if (fromtype == totype)
        return amount;

    if (room)
    {
        rc = room->currindex;
        ac = room->area->currindex;
    }

    if (!rc || !(from = find_currency(rc, fromtype)))
        if (!ac || !(from = find_currency(ac, fromtype)))
            from = find_currency(mud_curr_index, fromtype);

    if (!rc || !(to = find_currency(rc, totype)))
        if (!ac || !(to = find_currency(ac, totype)))
            to = find_currency(mud_curr_index, totype);

    if (!from || !to)
    {
        bug("Currency conversion returns -1 (%d to %d) for room %d.",
            fromtype, totype, room?room->vnum:0);
        return -1;
    }

    if (from->tsiints == to->tsiints)
        return amount;

    return (int)((float)amount*get_worth(from,to));
}

int obj_primary_curr_value(ROOM_INDEX_DATA *room, OBJ_DATA *obj)
{
    if (!obj)
    {
        bug("No obj given to obj_primary_curr_value.");
        return -1;
    }

    return convert_curr(room, obj->cost, obj->currtype, get_primary_curr(room));
}

void assign_currindex(ROOM_INDEX_DATA *room)
{
    if (!room)
    {
        bug("assign_currindex: No room.");
        return;
    }

    if (room->currvnum)
    {
        if ( (room->currindex = find_currindex_vnum(room->currvnum)) )
            return;
        bug("assign_currindex: room %d has invalid currvnum %d.",
            room->vnum, room->currvnum);
    }

    if (room->area && room->area->currvnum)
    {
        if ( (room->currindex = find_currindex_vnum(room->area->currvnum)) )
            return;
        bug("assign_currindex: area %s has invalid currvnum %d.",
            room->area->name, room->currvnum);
    }

    room->currindex = mud_curr_index;
}

static void save_currency( void )
{
    CURR_INDEX_DATA *currindex;
    CURRENCY_DATA *curr;
    FILE *fp;

    if ( ( fp = fopen(CURRENCY_FILE,"w") ) == NULL )
    {
        bug("Unable to write currency file: %s", CURRENCY_FILE);
        return;
    }

    for (currindex=first_curr_index;currindex;currindex=currindex->next_currindex)
    {
        if (currindex->vnum==0)
            continue;
        fprintf(fp, "#CURRINDEX\n");
        fprintf(fp, "Vnum           %d\n", currindex->vnum);
        fprintf(fp, "Primary        %d\n", currindex->primary);
        fprintf(fp, "Name           %s~\n", currindex->name);
        fprintf(fp, "Charge         %d\n", currindex->charge);
        for (curr=currindex->first_currency;curr;curr=curr->next_currency)
        {
            fprintf(fp, "Currency       %d %d %d\n",
                    curr->tsiints, curr->type, curr->charge);
        }
        fprintf(fp, "End\n\n");
    }
    fprintf(fp, "#END\n");

    fclose(fp);
}

static void fread_currindex( CURR_INDEX_DATA *currindex, FILE *fp )
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
            KEY( "Charge",      currindex->charge,          fread_number( fp ) );
            if (!str_cmp(word, "Currency"))
            {
                CURRENCY_DATA *curr;

                curr = make_currency(currindex);
                curr->tsiints = fread_number(fp);
                curr->type    = fread_number(fp);
                curr->charge  = fread_number(fp);

                fMatch = TRUE;
                break;
            }
            break;
        case 'N':
            KEY( "Name",        currindex->name,            fread_string_nohash( fp ) );
            break;
        case 'P':
            KEY( "Primary",     currindex->primary,         fread_number( fp ) );
            break;
        case 'V':
            KEY( "Vnum",        currindex->vnum,            fread_number( fp ) );
            break;
        }

        if ( !fMatch )
        {
            bug( "Fread_currency: no match: %s", word );
            fread_to_eol( fp );
        }
    }
}

void load_currency( void )
{
    FILE *fp;
    CURR_INDEX_DATA *currindex;
    bool found = FALSE;

    init_mud_curr_index();

    if ( ( fp = fopen( CURRENCY_FILE, "r" ) ) != NULL )
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
                bug( "load_currency: # not found." );
                break;
            }

            word = fread_word( fp );
            if ( !str_cmp( word, "CURRINDEX" ) )
            {
                currindex = make_currindex();
                DISPOSE(currindex->name);
                fread_currindex( currindex, fp );
                continue;
            }
            else if ( !str_cmp( word, "END" ) )
                break;
            else
            {
                bug( "load_currency: bad section." );
                break;
            }
        }
        fclose( fp );
    }
}

void do_listcurrency(CHAR_DATA *ch, char *argument)
{
    CURR_INDEX_DATA *currindex;
    CURRENCY_DATA *curr;

    if (!first_curr_index)
    {
        send_to_char("There are no currencies.\n\r", ch);
        return;
    }

    for (currindex=first_curr_index;currindex;currindex=currindex->next_currindex)
    {
        ch_printf(ch, "%-5d: %-41s Charge: %d%%\n\r",
                  currindex->vnum, currindex->name, currindex->charge);
        if (!currindex->first_currency)
            send_to_char(" This currency index has no currency.\n\r", ch);
        else
            for (curr=currindex->first_currency;curr;curr=curr->next_currency)
                ch_printf(ch, " %cCurrency: %-20s Tsiints: %-5d  Charge: %d%%\n\r",
                          curr->type==currindex->primary?'*':' ',
                          curr_types[curr->type], curr->tsiints, curr->charge);
    }
}

void do_setcurrency(CHAR_DATA *ch, char *argument)
{
    CURR_INDEX_DATA *currindex;
    CURRENCY_DATA *curr;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH],
        arg3[MAX_INPUT_LENGTH], arg4[MAX_INPUT_LENGTH];

    if ( !str_cmp(argument, "save") )
    {
        save_currency();
        send_to_char("Ok.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char("Syntax: setcurrency save\n\r", ch);
        send_to_char("Syntax: setcurrency <currindex vnum> <field> [arguments]\n\r\n\r", ch);
        send_to_char("Field being one of:\n\r  create vnum name primary charge save delete\n\r", ch);
        send_to_char("Field may also be a currency type:\n\r", ch);
        send_to_char("Syntax: setcurrency <currindex vnum> <type> <field> [arguments]\n\r", ch);
        send_to_char("Field being one of:\n\r  tsiints charge delete\n\r", ch);
        return;
    }

    if ( !str_cmp(arg2, "create") )
    {
        int x = atoi(arg1);
        if (find_currindex_vnum(x))
        {
            send_to_char("A currency index with that vnum already exists.\n\r", ch);
            return;
        }

        currindex = make_currindex();
        currindex->vnum = x;

        if (argument && *argument != '\0')
        {
            if (currindex->name)
                DISPOSE(currindex->name);
            currindex->name = str_dup(argument);
        }

        save_currency();

        assign_currindex(get_room_index(currindex->vnum));

        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!(currindex=find_currindex_vnum(atoi(arg1))))
    {
        send_to_char("Unable to find that currindex.\n\r", ch);
        return;
    }

    if ( !str_cmp(arg2, "delete") )
    {
        free_currindex(currindex);
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (argument[0] == '\0')
    {
        do_setcurrency(ch, "");
        return;
    }

    if ( !str_cmp(arg2, "name") )
    {
        if (currindex->name)
            DISPOSE(currindex->name);
        currindex->name = str_dup(argument);
        send_to_char("Ok.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg3);

    if ( !str_cmp(arg2, "primary") )
    {
        currindex->primary = get_currency_type(arg3);
        for (curr=currindex->first_currency;curr;curr=curr->next_currency)
            if (curr->type == currindex->primary)
                break;
        if (!curr || curr->type != currindex->primary)
        {
            currindex->primary = CURR_NONE;
            send_to_char("Please use makecurrency before you set that as a primary.\n\r", ch);
            return;
        }
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp(arg2, "vnum") )
    {
        if (!is_number(arg3))
        {
            send_to_char("Vnum must be a number.\n\r", ch);
            return;
        }
        if (find_currindex_vnum(atoi(arg3)))
        {
            send_to_char("There is another currindex with that vnum.\n\r", ch);
            return;
        }
        currindex->vnum = atoi(arg3);
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp(arg2, "charge") )
    {
        currindex->charge = URANGE(0,atoi(arg3),100);
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!(curr=find_currency(currindex,get_currency_type(arg2))))
    {
        int type, x;

        if ((type = get_currency_type(arg2)) == CURR_NONE)
        {
            send_to_char("That is an invalid currency type.\n\r", ch);
            return;
        }

        send_to_char("Creating currency.\n\r", ch);

        x = atoi(arg1);
        curr = make_currency(currindex);
        curr->tsiints = x;
        curr->type = type;

        save_currency();
        return;
    }

    if ( !str_cmp(arg3, "delete") )
    {
        if (currindex->primary == curr->type)
        {
            send_to_char("Primary currency deleted.\n\r", ch);
            currindex->primary = CURR_NONE;
        }
        free_currency(currindex,curr);
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (argument[0] == '\0')
    {
        do_setcurrency(ch, "");
        return;
    }

    argument = one_argument(argument, arg4);

    if ( !is_number(arg4) )
    {
        send_to_char("The fourth argument must be a number.\n\r",ch);
        return;
    }

    if ( !str_cmp(arg3, "tsiints") )
    {
        curr->tsiints = atoi(arg4);
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp(arg3, "charge") )
    {
        curr->charge = UMIN(atoi(arg4),100);
        send_to_char("Ok.\n\r", ch);
        return;
    }

    do_setcurrency(ch, "");
}

void do_exchange(CHAR_DATA *ch, char *argument)
{
    CURRENCY_DATA *c1, *c2, *c3 = NULL;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH],\
    arg3[MAX_INPUT_LENGTH];
    int amount, x, y, z;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);

    if (!IS_SET(ch->in_room->room_flags, ROOM_BANK))
    {
        send_to_char("You can only exchange currency in banks.\n\r", ch);
        return;
    }

    if (!ch->in_room->currindex || !ch->in_room->currindex->first_currency)
    {
        send_to_char("Sorry, but this bank does not exchange currency.\n\r", ch);
        return;
    }

    c1 = ch->in_room->currindex->first_currency;

    if (!c1->next_currency)
    {
        send_to_char("Sorry, but this bank does not exchange currency.\n\r", ch);
        return;
    }

    for (c2=c1;c2;c2=c2->next_currency)
        if (c2->type==ch->in_room->currindex->primary)
            break;

    if (c2 && c2->type!=ch->in_room->currindex->primary)
        c2=c1->next_currency;
    else
    {
        if (c2)
            c1=c2;
        c2=ch->in_room->currindex->first_currency;
        if (c1==c2)
            c2=c2->next_currency;
    }



    /* Tsiin's waste of time ;) Makes a neat little grid, bleh */
#if 0
    send_to_char("            ", ch);
    for (c3=ch->in_room->currindex->first_currency;c3;c3=c3->next_currency)
        ch_printf(ch,"   %-10.10s", curr_types[c3->type]);
    send_to_char("\n\r           +", ch);
    for (c3=ch->in_room->currindex->first_currency;c3;c3=c3->next_currency)
        send_to_char("------------+", ch);
    send_to_char("\n\r", ch);
    for (c1=ch->in_room->currindex->first_currency;c1;c1=c1->next_currency)
    {
        ch_printf(ch, "%10.10s |", curr_types[c1->type]);
        for (c2=ch->in_room->currindex->first_currency;c2;c2=c2->next_currency)
        {
            char tbuf[32];
            sprintf(tbuf, "%1.5f", get_worth(c2,c1));
            if (c1==c2)
                send_to_char("            |",ch);
            else
                ch_printf(ch, " %10.10s |", tbuf);
        }
        send_to_char("\n\r           +", ch);
        for (c3=ch->in_room->currindex->first_currency;c3;c3=c3->next_currency)
            send_to_char("------------+", ch);
        send_to_char("\n\r", ch);
    }
    return;
#endif


    /*
    for (c2=c1;c2;c2=c2->next_currency)
        for (c3=c1;c3;c3=c3->next_currency)
        {
            if (c2==c3)
                continue;
            ch_printf(ch, "%d %s piece%s for %d %s piece%s\n\r",
                      c2->tsiints,
                      curr_types[c3->type],
                      c2->tsiints>1?"s":"",
                      c3->tsiints,
                      curr_types[c2->type],
                      c3->tsiints>1?"s":"");
                      }
                      */
    if (arg1[0] == '\0')
    {
        ch_printf(ch, "%-10.10s: 1.000\n\r",
                  curr_types[c1->type]);
        for (c3=c2;c3;c3=c3->next_currency)
        {
            if (c3==c1 || c3->type == CURR_NONE)
                continue;
            ch_printf(ch, "%-10.10s: %.3f x %s\n\r",
                      curr_types[c3->type],
                      get_worth(c3,c1),
                      curr_types[c1->type]);
        }
        return;
    }

    if (!is_number(arg1) || arg2[0] == '\0' || arg3[0] == '\0')
    {
        send_to_char("Usage: exchange <amount> <from> <to>\n\r", ch);
        send_to_char("Example: exchange 100 gold silver\n\r", ch);
        return;
    }

    if ((amount=atoi(arg1))<=0)
    {
        send_to_char("That is not a valid amount to exchange.\n\r", ch);
        return;
    }

    if (!(c2=find_currency(ch->in_room->currindex,get_currency_type(arg2))) ||
        !(c3=find_currency(ch->in_room->currindex,get_currency_type(arg3))))
    {
        send_to_char("This bank does not exchange that type of currency.\n\r", ch);
        return;
    }

    if (c3->type == CURR_NONE)
    {
        send_to_char("That is not a valid currency to exchange to.\n\r", ch);
        return;
    }

    if (c2==c3 || amount*c2->tsiints<c3->tsiints)
    {
        send_to_char("Why not just give money to the bank?\n\r", ch);
        return;
    }

    if (GET_MONEY(ch,c2->type)<amount)
    {
        ch_printf(ch, "You don't have that much %s.\n\r",
                  curr_types[c2->type]);
        return;
    }

    if (ch->in_room->currindex->charge+c3->charge>100)
    {
        send_to_char("The banker is trying to rip you off, you decide to keep your money.\n\r", ch);
        return;
    }

    GET_MONEY(ch,c2->type) -= amount;

    y=0;
    for (x=c3->tsiints;x;x=x+c3->tsiints)
        if (x <= (amount*c2->tsiints))
            y++;
        else
            break;

    x=0;
    for (z=((amount*c2->tsiints)-(y*c3->tsiints));z;z=z-c2->tsiints)
        if (z >= c2->tsiints)
            x++;
        else
            break;

    GET_MONEY(ch,c3->type) += y;
    GET_MONEY(ch,c2->type) += x;
    ch_printf(ch, "You get %d %s with %d %s leftover.\n\r",
              y, curr_types[c3->type],
              x, curr_types[c2->type]);

    x = (int)((float)y*(float)((float)(ch->in_room->currindex->charge+c3->charge)/100.0));
    ch_printf(ch, "You are charged %d %s for the transaction.\n\r",
              x, curr_types[c3->type]);
}

/* returns the cost of an object in currtype currency */
int obj_cost(ROOM_INDEX_DATA *room, OBJ_DATA *obj, int currtype)
{
    return convert_curr(room, obj->cost, obj->currtype, currtype);
}

/* coins per pound */
const int money_weights[MAX_CURR_TYPE] =
{
    1, 1000, 1500, 2000, 750
};

int money_weight(int amount, int type)
{
    if (type < 0 ||type >= MAX_CURR_TYPE)
        return(1);
    return(amount/money_weights[type]);
}

int max_carry_money(CHAR_DATA *ch, int type)
{
    int amount = can_carry_w(ch) - carry_w(ch);

    amount = URANGE(0, amount, 1000000);

    return(amount*money_weights[type]);
}

int player_worth_percentage(CHAR_DATA *ch)
{
    CHAR_DATA *vch;
    double worth=0, mudworth=0, t;
    int type;

    for (vch = first_char; vch; vch = vch->next)
    {
        if (IS_NPC(vch))
            continue;

        t = 0.0;
        for (type = FIRST_CURR; type <= LAST_CURR; type++)
        {
            if (GET_MONEY(vch, type))
                t += convert_curr(NULL, GET_MONEY(vch, type), type, CURR_MITHRIL);
            if (GET_BALANCE(vch, type))
                t += convert_curr(NULL, GET_BALANCE(vch, type), type, CURR_MITHRIL);
        }
        mudworth += t;
        if (vch == ch)
            worth = t;
    }

    return((int)(worth/mudworth*100.0));
}

int horde_worth(CHAR_DATA *ch, int amount)
{
    int percentage = 100 - player_worth_percentage(ch);

    /* give high percentage people 5% at least */
    percentage = URANGE(5, percentage, 100);

    return (amount*percentage/100);
}

void do_balance( CHAR_DATA *ch, char *argument )
{
    int type;

    if (!IS_ROOM_FLAG(ch->in_room,ROOM_BANK))
    {
        send_to_char("You are not at the bank.\n\r",ch);
        return;
    }

    for (type=0;type<MAX_CURR_TYPE;type++)
        ch_printf(ch,"You have %d %s coins.\n\r",GET_BALANCE(ch,type),curr_types[type]);
}

void do_withdrawl( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int money=0, type=DEFAULT_CURR;

    if (!IS_ROOM_FLAG(ch->in_room,ROOM_BANK))
    {
        send_to_char("You are not at the bank.\n\r",ch);
        return;
    }

    argument = one_argument(argument, arg);

    if ( !str_cmp(arg, "all") )
    {
        money = -1;
    }
    else if ( !isdigit(arg[0]) )
    {
        send_to_char("You can only withdraw coins.\n\r",ch);
        return;
    }
    else
    {
        money = atoi(arg);
    }

    if (argument && argument[0])
        type = get_currency_type(argument);

    if (type == CURR_NONE)
    {
        send_to_char("You don't have any of that kind of coin.\n\r", ch);
        return;
    }

    if (money == -1)
        money = UMIN(max_carry_money(ch, type), GET_BALANCE(ch, type));

    if (money <= 0)
    {
        send_to_char("You can't do that.\n\r",ch);
        return;
    }

    if (money>GET_BALANCE(ch,type))
    {
        ch_printf(ch,"You don't have that much %s in the bank.\n\r",curr_types[type]);
        return;
    }

    if (money_weight(money,type) + carry_w(ch) >= can_carry_w(ch))
    {
        send_to_char("You can't carry that much.\n\r", ch);
        return;
    }

    GET_MONEY(ch,type)   +=money;
    GET_BALANCE(ch,type) -=money;

    ch_printf(ch,"You took %d %s coins from the bank, bringing your balance to %d.\n\r",
              money,curr_types[type],GET_BALANCE(ch,type));
}

void do_deposit( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int money=0, type=DEFAULT_CURR;

    if (!IS_ROOM_FLAG(ch->in_room,ROOM_BANK))
    {
        send_to_char("You are not at the bank.\n\r",ch);
        return;
    }

    argument = one_argument(argument, arg);

    if ( !isdigit(arg[0]) )
    {
        send_to_char("You can only deposit coins.\n\r",ch);
        return;
    }

    money=atoi(arg);
    if (argument && argument[0])
        type=get_currency_type(argument);

    if (money<=0)
    {
        send_to_char("You can't do that.\n\r",ch);
        return;
    }
    if (money>GET_MONEY(ch,type))
    {
        ch_printf(ch,"You don't have that much %s.\n\r",curr_types[type]);
        return;
    }
    GET_MONEY(ch,type)   -=money;
    GET_BALANCE(ch,type) +=money;

    ch_printf(ch,"You put %d %s coins in the bank, bringing your balance to %d.\n\r",
              money,curr_types[type],GET_BALANCE(ch,type));
}
