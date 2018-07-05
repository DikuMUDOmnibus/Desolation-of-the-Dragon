/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*static char rcsid[] = "$Id: commodity.c,v 1.11 2003/12/21 17:20:54 dotd Exp $";*/

#include <stdio.h>

#include "mud.h"
#include "currency.h"

DECLARE_DO_FUN(do_noteroom);

#ifdef KEY
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

#define COMMODITY_FILE	SYSTEM_DIR "commodity.dat"

typedef struct commodity_data COMMODITY_DATA;

struct commodity_data
{
    char *name;
    int value;
    int amount;
    int currtype;
    COMMODITY_DATA *next_commodity;
    COMMODITY_DATA *prev_commodity;
};

COMMODITY_DATA *first_commodity;
COMMODITY_DATA *last_commodity;


COMMODITY_DATA *make_commodity( void )
{
    COMMODITY_DATA *commodity;
    CREATE(commodity,COMMODITY_DATA,1);
    commodity->value = 0;
    commodity->amount = 0;
    commodity->name = NULL;
    commodity->next_commodity = NULL;
    commodity->prev_commodity = NULL;
    LINK(commodity,first_commodity,last_commodity,next_commodity,prev_commodity);
    return commodity;
}

static void save_commodities(void)
{
    COMMODITY_DATA *commodity;
    FILE *fp;

    if ( ( fp = fopen(COMMODITY_FILE,"w") ) == NULL )
    {
	bug("Unable to write commodity file: %s",COMMODITY_FILE);
	return;
    }

    for (commodity=first_commodity;commodity;commodity=commodity->next_commodity)
    {
        if (!commodity->name)
            continue;
        fprintf(fp, "#COMMODITY\n");
        if (commodity->value)
            fprintf(fp, "Value          %d\n", commodity->value);
        if (commodity->amount)
            fprintf(fp, "Ammount        %d\n", commodity->amount);
        if (commodity->currtype!=DEFAULT_CURR)
            fprintf(fp, "Currtype       %d\n", commodity->currtype);
        if (commodity->name && *commodity->name != '\0')
            fprintf(fp, "Name           %s~\n", commodity->name);
        fprintf(fp, "End\n\n");
    }
    fprintf(fp, "#END\n");

    fclose(fp);
}

static void fread_commodity(COMMODITY_DATA *commodity, FILE *fp)
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
        case 'A':
            KEY( "Ammount",	commodity->amount,		fread_number( fp ) );
            break;
        case 'C':
            KEY( "Currtype",	commodity->currtype,		fread_number( fp ) );
            break;
	case 'E':
	    if ( !str_cmp( word, "End" ) )
		return;
	    break;
        case 'N':
	    KEY( "Name",	commodity->name,		fread_string_nohash( fp ) );
	    break;
        case 'V':
	    KEY( "Value",	commodity->value,		fread_number( fp ) );
	    break;
	}
	
	if ( !fMatch )
	{
            bug( "Fread_commodity: no match: %s", word );
            fread_to_eol( fp );
        }
    }
}

void load_commodities(void)
{
    FILE *fp;
    COMMODITY_DATA *commodity;
    bool found = FALSE;

    if ( ( fp = fopen( COMMODITY_FILE, "r" ) ) != NULL )
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
		bug( "load_commodity: # not found." );
		break;
	    }

	    word = fread_word( fp );
	    if ( !str_cmp( word, "COMMODITY" ) )
	    {
		commodity = make_commodity();
	    	fread_commodity( commodity, fp );
		continue;
	    }
	    else if ( !str_cmp( word, "END" ) )
	        break;
	    else
	    {
		bug( "load_commodities: bad section." );
		break;
	    }
	}
	fclose( fp );
    }
}

static void free_commodity( COMMODITY_DATA *commodity )
{
    if (commodity->name)
	DISPOSE(commodity->name);
    UNLINK(commodity,first_commodity,last_commodity,next_commodity,prev_commodity);
    DISPOSE(commodity);
}

void free_commodities( void )
{
    COMMODITY_DATA *commodity;

    while ((commodity=first_commodity) != NULL)
	free_commodity(commodity);

    first_commodity = NULL;
    last_commodity = NULL;
}

static COMMODITY_DATA *find_commodity( char *name )
{
    COMMODITY_DATA *commodity;

    if (!first_commodity)
	return NULL;

    for (commodity=first_commodity;commodity;commodity=commodity->next_commodity)
	if (!str_prefix(name,commodity->name))
	    return commodity;

    return NULL;
}

static void list_commodities(CHAR_DATA *ch)
{
    COMMODITY_DATA *commodity;

    if (!first_commodity)
    {
        send_to_char("There are no commodities to list.\n\r", ch);
        return;
    }
    
    ch_printf(ch, "%-20s %-5s %-5s\n\r", "Commodity Name", "Value", "Ammount");

    for (commodity=first_commodity;commodity;commodity=commodity->next_commodity)
        ch_printf(ch, "%-20s %-5d %-5d\n\r",
                  commodity->name, commodity->value, commodity->amount);
}

SPECIAL_FUNC(spec_commodity_board)
{
    if (type!=SFT_COMMAND)
	return FALSE;

    if (cmd->do_fun!=do_noteroom)
        return FALSE;

    list_commodities(cmd_ch);

    return TRUE;
}

void do_makecommodity(CHAR_DATA *ch, char *argument)
{
    COMMODITY_DATA *commodity;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (!arg2 || arg2[0] == '\0')
    {
	send_to_char("Syntax: makecommodity <value> <amount> <name>\n\r", ch);
	return;
    }

    if ( !is_number(arg1) || !is_number(arg2) )
    {
	send_to_char("Invalid format.\n\r", ch);
	send_to_char("Syntax: makecommodity <value> <amount> <name>\n\r", ch);
	return;
    }
    commodity = make_commodity();
    commodity->value = atoi(arg1);
    commodity->amount = atoi(arg2);
    if (argument && *argument != '\0')
    {
	if (commodity->name)
	    DISPOSE(commodity->name);
	commodity->name = str_dup(argument);
    }
    save_commodities();
    send_to_char("Ok.\n\r", ch);
}

void do_commodity( CHAR_DATA *ch, char *argument )
{
    COMMODITY_DATA *commodity;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    if ( !str_cmp(argument, "list") )
    {
        char s1[16], s2[16], s3[16];
        sprintf(s1, "%s", color_str(AT_SCORE,ch));
        sprintf(s2, "%s", color_str(AT_SCORE2,ch));
        sprintf(s3, "%s", color_str(AT_SCORE3,ch));
        for (commodity=first_commodity;commodity;commodity=commodity->next_commodity)
            ch_printf(ch, "%sName: %s%-30s%s  Value: %s%d%s  Amount: %s%d/%d\n\r",
                      s1, s3, commodity->name, s1,
                      s2, commodity->value, s1,
                      s2, commodity->amount, commodity->currtype );
        return;
    }
    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char("Syntax: commodity [field] <name>\n\r\n\r"
                     "Field being one of:\n\r  buy sell list\n\r",ch);
        return;
    }

    if (!(commodity=find_commodity(arg2)))
    {
	send_to_char("Unable to find that commodity.\n\r", ch);
	return;
    }

    if ( !str_cmp(arg1, "buy") )
    {
	if (commodity->value < 0)
	{
	    send_to_char("You cannot buy that commodity.\n\r",ch);
	    return;
	}
        if (GET_MONEY(ch,commodity->currtype) < commodity->value)
        {
            send_to_char("You can't afford it.\n\r",ch);
            return;
        }
        GET_MONEY(ch,commodity->currtype) -= commodity->value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp(arg1, "sell") )
    {
        GET_MONEY(ch, commodity->currtype) += commodity->value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    send_to_char("Syntax: commodity [field] <name>\n\r\n\r"
                 "Field being one of:\n\r  buy sell list\n\r",ch);

}

void do_comset( CHAR_DATA *ch, char *argument )
{
    COMMODITY_DATA *commodity;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH];

    if ( !str_cmp(argument, "save") )
    {
	save_commodities();
	send_to_char("Ok.\n\r", ch);
	return;
    }

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0')
    {
	send_to_char("Syntax: comset save\n\r", ch);
	send_to_char("Syntax: comset <commodity name> <field>\n\r\n\r", ch);
	send_to_char("Field being one of:\n\r  name value amount currtype delete\n\r", ch);
	return;
    }

    if (!(commodity=find_commodity(arg1)))
    {
	send_to_char("Unable to find that commodity.\n\r", ch);
	return;
    }

    if ( !str_cmp(arg2, "delete") )
    {
        free_commodity(commodity);
        send_to_char("Ok.\n\r", ch);
        return;
    }
    
    if (argument[0] == '\0')
    {
        do_comset(ch,"");
        return;
    }

    if ( !str_cmp(arg2, "name") )
    {
	if (commodity->name)
	    DISPOSE(commodity->name);
	commodity->name = str_dup(argument);
	send_to_char("Ok.\n\r", ch);
	return;
    }

    if ( !str_cmp(arg2, "currtype") )
    {
	commodity->currtype = get_currency_type(argument);
	send_to_char("Ok.\n\r", ch);
        return;
    }

    argument = one_argument(argument, arg3);

    if ( !is_number(arg3) )
    {
	send_to_char("The third argument must be a number.\n\r",ch);
	return;
    }

    if ( !str_cmp(arg2, "value") )
    {
	commodity->value = atoi(arg3);
	send_to_char("Ok.\n\r", ch);
	return;
    }

    if ( !str_cmp(arg2, "amount") )
    {
	commodity->amount = atoi(arg3);
	send_to_char("Ok.\n\r", ch);
        return;
    }

    do_comset(ch,"");
}
