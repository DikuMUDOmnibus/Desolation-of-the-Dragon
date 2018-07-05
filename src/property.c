/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

#include <stdio.h>
#ifdef WINDOWS
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#endif

#include "mud.h"
#define SPEC SPECIAL_FUNC
#include "mspecial.h"
#undef SPEC
#include "property.h"
#include "currency.h"

DECLARE_DO_FUN(do_propset);
DECLARE_DO_FUN(do_say);

#define FP_PREC 100.0

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

char *	const	property_flags [] =
{
    "forsale", "bv01", "bv02", "bv03", "bv04", "bv05",
    "bv06", "bv07", "bv08", "bv09", "bv10", "bv11", "bv12",
    "bv13", "bv14", "bv15", "bv16", "bv17", "bv18", "bv19",
    "bv20", "bv21", "bv22", "bv23", "bv24", "bv25", "bv26",
    "bv27", "bv28", "bv29", "bv30", "bv31"
};

PROPERTY_DATA   *       first_property;
PROPERTY_DATA   *       last_property;

PROPERTY_DATA *make_property( void )
{
    PROPERTY_DATA *prop;
    int x;

    CREATE(prop,PROPERTY_DATA,1);
    prop->vnum = 0;
    prop->value = 0;
    prop->currtype = DEFAULT_CURR;
    prop->flags = 0;
    prop->profit_buy = 0;
    prop->profit_rent = 0;
    for (x=0;x<MAX_CURR_TYPE;x++)
        prop->money[x] = 0;
    prop->owner = NULL;
    prop->name = NULL;
    prop->next_property = NULL;
    prop->prev_property = NULL;
    prop->raw_goods = NULL;
    LINK(prop,first_property,last_property,next_property,prev_property);
    return prop;
}

static void save_properties( void )
{
    PROPERTY_DATA *prop;
    FILE *fp;
    int count;

    if ( ( fp = fopen(PROPERTY_FILE,"w") ) == NULL )
    {
        bug("Unable to write property file: %s",PROPERTY_FILE);
        return;
    }

    for (prop=first_property;prop;prop=prop->next_property)
    {
        if (!prop->name && !prop->owner)
            continue;
        fprintf(fp, "#PROPERTY\n");
        if (prop->vnum)
            fprintf(fp, "Vnum           %d\n", prop->vnum);
        if (prop->value)
            fprintf(fp, "Value          %d\n", prop->value);
        if (prop->currtype)
            fprintf(fp, "Currtype       %d\n", prop->currtype);
        if (prop->profit_buy)
            fprintf(fp, "ProfitBuy      %d\n", prop->profit_buy);
        if (prop->profit_rent)
            fprintf(fp, "ProfitRent     %d\n", prop->profit_rent);
        if (prop->flags)
            fprintf(fp, "Flags          %d\n", prop->flags);
        fprintf( fp, "Money          ");
        for (count=0; count < MAX_CURR_TYPE; count++ )
            fprintf(fp, " %d", prop->money[count]);
        fprintf(fp, " -1\n");
        if (prop->owner && *prop->owner != '\0')
            fprintf(fp, "Owner          %s~\n", prop->owner);
        if (prop->name && *prop->name != '\0')
            fprintf(fp, "Name           %s~\n", prop->name);
        fprintf(fp, "End\n\n");
    }
    fprintf(fp, "#END\n");

    fclose(fp);
}

static void fread_property( PROPERTY_DATA *prop, FILE *fp )
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
            KEY( "Currtype",    prop->currtype,         fread_number( fp ) );
            break;
        case 'F':
            KEY( "Flags",       prop->flags,            fread_number( fp ) );
            break;
        case 'G':
            KEY( "Gold",        prop->money[DEFAULT_CURR], fread_number( fp ) );
            break;
        case 'N':
            KEY( "Name",        prop->name,             fread_string_nohash( fp ) );
            break;
        case 'M':
            if ( !str_cmp( word, "Money" ) )
            {
                int x1=0,x2=0;
                while ((x1=fread_number(fp))>=0 && x2<MAX_CURR_TYPE)
                    prop->money[x2++] = x1;
                fread_to_eol(fp);
                fMatch = TRUE;
                break;
            }
            break;
        case 'O':
            KEY( "Owner",       prop->owner,            fread_string_nohash( fp ) );
            break;
        case 'P':
            KEY( "ProfitBuy",   prop->profit_buy,       fread_number( fp ) );
            KEY( "ProfitRent",  prop->profit_rent,      fread_number( fp ) );
            break;
        case 'V':
            KEY( "Value",       prop->value,            fread_number( fp ) );
            KEY( "Vnum",        prop->vnum,             fread_number( fp ) );
            break;
        }

        if ( !fMatch )
        {
            bug( "Fread_property: no match: %s", word );
            fread_to_eol( fp );
        }
    }
}

void load_properties( void )
{
    FILE *fp;
    PROPERTY_DATA *prop;
    bool found = FALSE;

    if ( ( fp = fopen( PROPERTY_FILE, "r" ) ) != NULL )
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
                bug( "load_property: # not found." );
                break;
            }

            word = fread_word( fp );
            if ( !str_cmp( word, "PROPERTY" ) )
            {
                prop = make_property();
                fread_property( prop, fp );
                continue;
            }
            else if ( !str_cmp( word, "END" ) )
                break;
            else
            {
                bug( "load_properties: bad section." );
                break;
            }
        }
        fclose( fp );
    }
}

static void free_property( PROPERTY_DATA *prop )
{
    if (prop->owner)
        DISPOSE(prop->owner);
    if (prop->name)
        DISPOSE(prop->name);
    UNLINK(prop,first_property,last_property,next_property,prev_property);
    DISPOSE(prop);
}

void free_properties( void )
{
    PROPERTY_DATA *prop;

    while ((prop=first_property) != NULL)
        free_property(prop);

    first_property = NULL;
    last_property = NULL;
}

static PROPERTY_DATA *find_property( char *name )
{
    PROPERTY_DATA *prop;

    if (!first_property)
        return NULL;

    for (prop=first_property;prop;prop=prop->next_property)
        if (!str_prefix(name,prop->name) || (is_number(name) && prop->vnum==atoi(name)))
            return prop;

    return NULL;
}

OBJ_DATA *create_deed( PROPERTY_DATA *prop )
{
    ROOM_INDEX_DATA *room;
    OBJ_DATA *obj;
    char buf[MAX_STRING_LENGTH];

    if ( !(room=get_room_index(prop->vnum)) )
    {
        bug( "Create_deed: room %d does not exist.", prop->vnum );
        return NULL;
    }

    if ( !obj_exists_index(OBJ_VNUM_DEED) )
    {
        bug( "Create_deed: deed %d does not exist.", OBJ_VNUM_DEED );
        return NULL;
    }

    obj = create_object( OBJ_VNUM_DEED );

    if ( !obj )
    {
        bug( "Create_deed: error creating object." );
        return NULL;
    }

    sprintf( buf, obj->name, prop->vnum, room->name );
    STRFREE( obj->name );
    obj->name = STRALLOC( buf );

    sprintf( buf, obj->short_descr, room->name );
    STRFREE( obj->short_descr );
    obj->short_descr  = STRALLOC( buf );

    sprintf( buf, obj->description, room->name );
    STRFREE( obj->description );
    obj->description  = STRALLOC( buf );

    obj->value[0] = prop->vnum;

    return obj;
}

int property_tax(ROOM_INDEX_DATA *room, int cost, int profit_type)
{
    PROPERTY_DATA *prop;
    char buf[16];
    int profit;

    sprintf(buf, "%d", room->vnum);

    if (!(prop=find_property(buf)))
        return 0;

    switch (profit_type)
    {
    case PROFIT_RENT:
        profit = prop->profit_rent; break;
    case PROFIT_BUY:
        profit = prop->profit_buy; break;
    case PROFIT_SELL:
    default:
        return 0; break;
    }

    return (int)(cost * ((float)profit/(FP_PREC*100.0)));
}

void property_add_tax(ROOM_INDEX_DATA *room, int cost, int type, int profit_type)
{
    PROPERTY_DATA *prop;
    char buf[16];

    sprintf(buf, "%d", room->vnum);

    if (!(prop=find_property(buf)))
        return;

    prop->money[type] += property_tax(room,cost,profit_type);
    save_properties();
}

bool owns_prop(CHAR_DATA *ch, PROPERTY_DATA *prop)
{
    if (prop->owner && !str_cmp(prop->owner,GET_NAME(ch)))
        return TRUE;

    return FALSE;
}

void do_makeproperty( CHAR_DATA *ch, char *argument )
{
    PROPERTY_DATA *prop;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (!arg2 || arg2[0] == '\0')
    {
        send_to_char("Syntax: makeproperty <vnum> <value> [name]\n\r", ch);
        return;
    }

    if ( !is_number(arg1) || !is_number(arg2) )
    {
        send_to_char("Invalid arguments.\n\r", ch);
        do_makeproperty(ch,"");
        return;
    }
    prop = make_property();
    prop->vnum = atoi(arg1);
    prop->value = atoi(arg2);
    prop->owner = str_dup("None");
    if (argument && *argument != '\0')
    {
        if (prop->name)
            DISPOSE(prop->name);
        prop->name = str_dup(argument);
    }
    save_properties();
    send_to_char("Ok.\n\r", ch);
}

void do_property( CHAR_DATA *ch, char *argument )
{
    PROPERTY_DATA *prop;
    char arg1[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];

    if (!first_property)
    {
        send_to_char("There are no properties.\n\r", ch);
        return;
    }

    if (!argument || argument[0] == '\0')
    {
        send_to_char("Syntax: property [field]\n\r\n\r"
                     "Field being one of:\n\r  owned buy sell tax rent collect\n\r",ch);
        if (IS_IMMORTAL(ch))
            send_to_char("Additional fields for immortals:\n\r  list\n\r", ch);
        return;
    }

    if ( !str_prefix(argument, "list") && IS_IMMORTAL(ch) )
    {
        bool found = FALSE;
        send_to_char("--------- Property Not For Sale ---------\n\r", ch);
        for (prop=first_property;prop;prop=prop->next_property)
        {
            if (IS_SET(prop->flags, PROPERTY_FORSALE) || owns_prop(ch, prop))
                continue;
            found = TRUE;
            ch_printf(ch, "%-5d:Name: %-15.15s Owner: %-10.10s Value: %-9d Rates: %.2f/%.2f\n\r",
                      prop->vnum, prop->name, prop->owner, prop->value,
                      (float)prop->profit_buy/FP_PREC,
                      (float)prop->profit_rent/FP_PREC);
        }
        if (!found)
            send_to_char("None.\n\r", ch);
        found = FALSE;
        send_to_char("----------- Property For Sale -----------\n\r", ch);
        for (prop=first_property;prop;prop=prop->next_property)
        {
            if (!IS_SET(prop->flags, PROPERTY_FORSALE))
                continue;
            found = TRUE;
            ch_printf(ch, "%-5d:Name: %-15.15s Owner: %-10.10s Value: %-9d Rates: %.2f/%.2f\n\r",
                      prop->vnum, prop->name, prop->owner, prop->value,
                      (float)prop->profit_buy/FP_PREC,
                      (float)prop->profit_rent/FP_PREC);
        }
        if (!found)
            send_to_char("None.\n\r", ch);
        found = FALSE;
        send_to_char("----------- Property You Own ------------\n\r", ch);
        for (prop=first_property;prop;prop=prop->next_property)
        {
            if (!owns_prop(ch, prop))
                continue;
            found = TRUE;
            ch_printf(ch, "%-5d:Name: %-33.33s Value: %-9d Rates: %.2f/%.2f\n\r",
                      prop->vnum, prop->name, prop->value,
                      (float)prop->profit_buy/FP_PREC,
                      (float)prop->profit_rent/FP_PREC);
        }
        if (!found)
            send_to_char("None.\n\r", ch);
        return;
    }

    if ( !str_prefix(argument, "owned") )
    {
        int count;
        bool owned = FALSE;

        for (prop=first_property;prop;prop=prop->next_property)
            if (owns_prop(ch, prop))
            {
                owned = TRUE;
                ch_printf(ch, "Name: %-30s  Value: %d%c  Rates: %.2f/%.2f\n\r",
                          prop->name,
                          prop->value,
                          IS_SET(prop->flags,PROPERTY_FORSALE)?'*':' ',
                          (float)prop->profit_buy/FP_PREC,
                          (float)prop->profit_rent/FP_PREC);
                for (count=FIRST_CURR; count <= LAST_CURR; count++ )
                    if (prop->money[count])
                        ch_printf(ch, "  %s: %d\n\r",
                                  cap_curr_types[count],
                                  prop->money[count]);
            }
        if ( !owned )
            send_to_char("You don't own any properties.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg1);

    if (arg1[0] == '\0')
    {
        send_to_char("Syntax: property [field]\n\r\n\r"
                     "Field being one of:\n\r  list owned buy sell tax rent collect\n\r",ch);
        return;
    }

    sprintf(buf, "%d", ch->in_room->vnum);

    if (!(prop=find_property(buf)))
    {
        send_to_char("This is not a property.\n\r", ch);
        return;
    }

    if ( !str_cmp(arg1, "buy") )
    {
        OBJ_DATA *obj;

        if (owns_prop(ch, prop))
        {
            if (IS_SET(prop->flags, PROPERTY_FORSALE))
            {
                sprintf(buf, "sell \"%s\"", prop->name);

                do_property(ch, buf);
                return;
            }
            send_to_char("You already own it.\n\r", ch);
            return;
        }

        if (prop->owner && str_cmp(prop->owner, "None") &&
            !IS_SET(prop->flags, PROPERTY_FORSALE))
        {
            send_to_char("You can't buy that property.\n\r",ch);
            return;
        }
        if (prop->value < 0)
        {
            send_to_char("You cannot buy that property, it has no value.\n\r",ch);
            return;
        }
        if (GET_MONEY(ch,prop->currtype) < prop->value)
        {
            send_to_char("You can't afford it.\n\r",ch);
            return;
        }
        GET_MONEY(ch,prop->currtype) -= prop->value;
        if (prop->owner)
            DISPOSE(prop->owner);
        prop->owner = str_dup(GET_NAME(ch));
        REMOVE_BIT(prop->flags, PROPERTY_FORSALE);
        save_properties();
        if ((obj = create_deed(prop)))
        {
            (void)obj_to_char(obj,ch);
            ch_printf(ch, "You buy '%s' and are given the deed.\n\r", prop->name);
        }
        else
            ch_printf(ch, "You buy '%s' but will have to get the deed from the mayor's office.\n\r", prop->name);
        return;
    }

    if (!owns_prop(ch, prop))
    {
        send_to_char("That is not your property.\n\r", ch);
        return;
    }

    if ( !str_prefix(arg1, "sell") )
    {
        TOGGLE_BIT(prop->flags, PROPERTY_FORSALE);
        if (IS_SET(prop->flags, PROPERTY_FORSALE))
            ch_printf(ch, "You have placed '%s' up for sale.\n\r", prop->name);
        else
            ch_printf(ch, "You have taken '%s' off the market.\n\r", prop->name);
        save_properties();
        return;
    }

    if ( !str_prefix(arg1, "tax") )
    {
        char arg3[MAX_INPUT_LENGTH];
        argument = one_argument(argument, arg3);
        sprintf(buf, "%d buy %s", prop->vnum, arg3);
        if (arg3[0] != '\0')
        {
            do_propset(ch,buf);
            return;
        }
    }

    if ( !str_prefix(arg1, "rent") )
    {
        char arg3[MAX_INPUT_LENGTH];
        argument = one_argument(argument, arg3);
        sprintf(buf, "%d rent %s", prop->vnum, arg3);
        if (arg3[0] != '\0')
        {
            do_propset(ch,buf);
            return;
        }
    }

    if ( !str_prefix(arg1, "collect") )
    {
        int x;
        bool found = FALSE;

        for (x=FIRST_CURR;x<=LAST_CURR;x++)
        {
            if ( prop->money[x] < 0 )
            {
                int paid;

                if ( (GET_MONEY(ch,x)+prop->money[x]) < 0)
                    paid = GET_MONEY(ch,x);
                else
                    paid = prop->money[x]*-1;

                ch_printf(ch, "You paid %d %s coin%s.\n\r",
                          paid,
                          curr_types[x],
                          paid==1?"":"s");

                prop->money[x] -= paid;
                GET_MONEY(ch,x) -= paid;

                if (prop->money[x] < 0)
                    ch_printf(ch, "You still owe %d %s coin%s.\n\r",
                              prop->money[x]*-1,
                              curr_types[x],
                              prop->money[x]==-1?"":"s");
                found = TRUE;
            }
            else if (prop->money[x] > 0)
            {
                ch_printf(ch, "You collected %d %s coin%s.\n\r",
                          prop->money[x],
                          curr_types[x],
                          prop->money[x]==1?"":"s");

                GET_MONEY(ch,x) += prop->money[x];
                prop->money[x] = 0;
                found = TRUE;
            }
        }

        if (!found)
            send_to_char("You collected no money.\n\r", ch);

        return;
    }

    send_to_char("Unknown field.\n\r", ch);
    do_property(ch,"");
}

void do_propset( CHAR_DATA *ch, char *argument )
{
    PROPERTY_DATA *prop;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH];
    int type;

    if ( !str_cmp(argument, "save") )
    {
        save_properties();
        send_to_char("Ok.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char("Syntax: propset save\n\r", ch);
        send_to_char("Syntax: propset <property vnum> <field>\n\r\n\r", ch);
        send_to_char("Field being one of:\n\r  name owner value vnum buy rent delete", ch);
        for (type=FIRST_CURR;type<=LAST_CURR;type++)
            ch_printf(ch, " %s", curr_types[type]);
        send_to_char( "\n\r", ch );

        return;
    }

    if (!(prop=find_property(arg1)))
    {
        send_to_char("Unable to find that property.\n\r", ch);
        return;
    }

    if ( !str_cmp(arg2, "delete") )
    {
        free_property(prop);
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (argument[0] == '\0')
    {
        do_propset(ch,"");
        return;
    }

    if ( !str_cmp(arg2, "name") )
    {
        if (prop->name)
            DISPOSE(prop->name);
        prop->name = str_dup(argument);
        send_to_char("Ok.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg3);

    if ( !str_cmp(arg2, "owner") )
    {
        if (prop->owner)
            DISPOSE(prop->owner);
        prop->owner = str_dup(capitalize(arg3));
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !is_number(arg3) )
    {
        ch_printf(ch, "The argument to '%s' must be a number.\n\r", arg2);
        return;
    }

    if ( !str_cmp(arg2, "value") )
    {
        prop->value = atoi(arg3);
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp(arg2, "vnum") )
    {
        prop->vnum = atoi(arg3);
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp(arg2, "buy") || !str_cmp(arg2, "profitbuy") )
    {
        prop->profit_buy = (int)(FP_PREC*atof(arg3));
        ch_printf(ch, "Setting the tax on renting to %.2f\n\r", (float)prop->profit_buy/FP_PREC);
        return;
    }

    if ( !str_cmp(arg2, "rent") || !str_cmp(arg2, "profitrent") )
    {
        prop->profit_rent = (int)(FP_PREC*atof(arg3));
        ch_printf(ch, "Setting the tax on renting to %.2f\n\r", (float)prop->profit_rent/FP_PREC);
        return;
    }

    if ( (type=get_currency_type(arg2)) )
    {
        prop->money[type] = atoi(arg3);
        send_to_char("Ok.\n\r", ch);
        return;
    }

    send_to_char("Unknown field.\n\r", ch);
    do_propset(ch,"");
}

void do_propstat( CHAR_DATA *ch, char *argument )
{
    PROPERTY_DATA *prop;
    ROOM_INDEX_DATA *roomi;
    int type;

    if (!argument || !*argument)
    {
        send_to_char("Syntax: propstat <property>\n\r", ch);
        return;
    }

    if (!(prop=find_property(argument)))
    {
        send_to_char("Unable to find that property.\n\r", ch);
        return;
    }

    roomi = get_room_index(prop->vnum);
    ch_printf(ch, "Property status:\n\rName: %s\n\rRoom name: %s\n\rLocation: %s\n\rOwner: %s\n\rValue: %d\n\rFlags: %s\n\rMoney:",
              prop->name,
              roomi?roomi->name:"(no room)",
              roomi?(roomi->area?roomi->area->name:"(no location)"):"(no location)",
              prop->owner,
              prop->value,
              flag_string(prop->flags, property_flags));
    for (type=0;type<MAX_CURR_TYPE;type++)
        ch_printf(ch, " %d", prop->money[type]);
    ch_printf(ch, "\n\rProfit buy: %.2f%%  Profit rent: %.2f%%\n\r",
              (float)prop->profit_buy/FP_PREC,
              (float)prop->profit_rent/FP_PREC );
}


SPECIAL_FUNC(spec_realtor)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    PROPERTY_DATA *prop;
    bool found = FALSE;

    if (who_fighting(ch))
        return spec_warrior(proc,cmd,arg,cmd_ch,type);

    if (type == SFT_UPDATE)
    {
        /* handle running around placing 'for sale' signs here */
        return FALSE;
    }

    if (type != SFT_COMMAND)
        return FALSE;


    if (cmd->do_fun != do_property)
        return FALSE;

    if (arg && *arg != '\0')
    {
        do_say(ch, "Just type 'property' to see my list.");
        return TRUE;
    }

    act( AT_PLAIN, "You show $N your list of properties.", ch, NULL, cmd_ch, TO_CHAR );
    act( AT_PLAIN, "$n shows you $s list of properties:", ch, NULL, cmd_ch, TO_VICT );
    act( AT_PLAIN, "$n shows $N $s list of properties.", ch, NULL, cmd_ch, TO_NOTVICT );

    send_to_char("--------- Property Not For Sale ---------\n\r", cmd_ch);
    for (prop=first_property;prop;prop=prop->next_property)
    {
        if (IS_SET(prop->flags, PROPERTY_FORSALE) || owns_prop(cmd_ch, prop))
            continue;
        found = TRUE;
        ch_printf(cmd_ch, "Name: %-38.38s Owner: %-10.10s Value: %-9d\n\r",
                  prop->name, prop->owner, prop->value);
    }
    if (!found)
        send_to_char("None.\n\r", cmd_ch);
    found = FALSE;
    send_to_char("----------- Property For Sale -----------\n\r", cmd_ch);
    for (prop=first_property;prop;prop=prop->next_property)
    {
        if (!IS_SET(prop->flags, PROPERTY_FORSALE))
            continue;
        found = TRUE;
        ch_printf(cmd_ch, "Name: %-38.38s Owner: %-10.10s Value: %-9d\n\r",
                  prop->name, prop->owner, prop->value);
    }
    if (!found)
        send_to_char("None.\n\r", cmd_ch);
    found = FALSE;
    send_to_char("----------- Property You Own ------------\n\r", cmd_ch);
    for (prop=first_property;prop;prop=prop->next_property)
    {
        if (!owns_prop(cmd_ch, prop))
            continue;
        found = TRUE;
        ch_printf(cmd_ch, "Name: %-38.38s Value: %-9d Rates: %.2f/%.2f\n\r",
                  prop->name, prop->value,
                  (float)prop->profit_buy/FP_PREC,
                  (float)prop->profit_rent/FP_PREC);
    }
    if (!found)
        send_to_char("None.\n\r", cmd_ch);

    return TRUE;
}

