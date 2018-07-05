/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer and Heath Leach
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*static char rcsid[] = "$Id: ospecial.c,v 1.35 2004/04/06 22:00:10 dotd Exp $";*/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "currency.h"
#include "gsn.h"

DECLARE_DO_FUN(do_pull);
DECLARE_DO_FUN(do_drop);
DECLARE_DO_FUN(do_kill);
DECLARE_DO_FUN(do_wear);
DECLARE_DO_FUN(do_say);
DECLARE_DO_FUN(do_use);

#define SPEC SPECIAL_FUNC
#include "ospecial.h"
#undef SPEC

#define SPEC(func) \
    if (++i==2) {i=0;send_to_pager("\n\r",ch);} pager_printf(ch, "%-38s", #func)
void do_oslist( CHAR_DATA *ch, char *argument )
{
    int i=-1;

    set_pager_color(AT_PLAIN,ch);

    #include "ospecial.h"

    send_to_pager( "\n\r", ch );
}
#undef SPEC

#define SPEC(func) \
    if (obj->spec_fun == func && strstr(#func, argument)) \
    { ch_printf(ch, "u%-5d %5d: %s\n\r", obj->unum, obj->vnum, #func); match++; continue; }
void do_osfind( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int match=0;

    if (!argument)
    {
        send_to_char("osfind <name>\n\r", ch);
        return;
    }
    for (obj=first_object;obj;obj=obj->next)
    {
        if (!obj->spec_fun)
            continue;
#include "ospecial.h"
    }
    ch_printf(ch, "%d matches found.\n\r", match);
}
#undef SPEC

/*
 * Given a name, return the appropriate spec fun.
 */
#define SPEC(func) \
    if ( !str_cmp( name, #func ) ) return func
SPEC_FUN *o_spec_lookup( const char *name )
{
    #include "ospecial.h"
    return NULL;
}
#undef SPEC

/*
 * Given a pointer, return the appropriate spec fun text.
 */
#define SPEC(func) \
    if ( special == func ) return #func
char *o_lookup_spec( SPEC_FUN *special )
{
    #include "ospecial.h"
    return "";
}
#undef SPEC


SPECIAL_FUNC(spec_nodrop)
{
    if (type!=SFT_COMMAND || cmd->do_fun!=do_drop)
	return FALSE;

    if (IS_IMMORTAL(cmd_ch))
	send_to_char("It resists your attept to drop it, but you do anyway.\n\r", cmd_ch);
    else
	send_to_char("You cannot drop that.\n\r", cmd_ch);
    return TRUE;
}

char * const slot_values[6] =
{
    "&YLemon  ", "&OOrange ", "&RCherry ",
    "&GLime   ", "&WBar    ", "&YGold   "
};
SPECIAL_FUNC(spec_SlotMachine)
{
    OBJ_DATA *obj = (OBJ_DATA *)proc;
    char buf[MAX_INPUT_LENGTH];
    int c, i[3], ind=0, last;

    if (type==SFT_UPDATE && obj)
    {
        send_to_room("A slot machine twinkles with luring magics.\n\r",obj->in_room);
        return(TRUE);
    }

    if (type!=SFT_COMMAND || cmd->do_fun!=do_pull)
        return(FALSE);

    if (str_prefix(arg, "handle")) {
        send_to_char("You grope for something and pull it... Someone doesn't look too happy.\n\r", cmd_ch);
        return(TRUE);
    }

    if (GET_MONEY(cmd_ch,CURR_GOLD) < obj->value[4])
    {
        send_to_char("You don't have enough gold!\n\r", cmd_ch);
        return(TRUE);
    }
    else
    {
        send_to_char("You pull the handle and await the results...\n\r", cmd_ch);
    }

    WAIT_STATE(cmd_ch, PULSE_PER_SECOND);

    if (obj->value[5] <= obj->value[4])
        obj->value[5] = obj->value[4]*10;

    GET_MONEY(cmd_ch,CURR_GOLD) -= obj->value[4];
    obj->value[5] += obj->value[4];

    last = 0;
    for (c = 0; c < 3; c++)
    {
        int x = number_range(0, 40) + URANGE(-4, get_curr_lck(cmd_ch) - 16, 4);
        if (x < 8)
            i[c] = 0;
        else if (x < 8+7)
            i[c] = 1;
        else if (x < 8+7+6)
            i[c] = 2;
        else if (x < 8+7+6+5)
            i[c] = 3;
        else if (x < 8+7+6+5+4)
            i[c] = 4;
        else if (x < 8+7+6+5+4+3)
            i[c] = 5;
        else
            i[c] = last;
        send_to_char(slot_values[i[c]], cmd_ch);
        last = i[c];
    }
    send_to_char("&w\n\r", cmd_ch);

    /* if all three are same or cherry same same */
    if ((i[0] == i[1] || i[0] == 2) && i[1] == i[2])
    {
        act(AT_MAGIC,"$n wins some money from the slot machine!",cmd_ch,NULL,NULL,TO_ROOM);

        switch(i[0])
        {
        case 0:
            ind = obj->value[4]; /* Give them back what they put in */
            break;
        case 1:
            ind = obj->value[4]*5;
            break;
        case 2:
            ind = obj->value[4]*10;
            break;
        case 3:
            ind = obj->value[4]*20;
            break;
        case 4:
            ind = obj->value[4]*40;
            break;
        case 5:
            ind = obj->value[5]; /* Wow! We've won big! */
            if (i[0] == i[1])
                send_to_area("Magical telltales start popping off and twinkling lights flash everywhere!\n\r",cmd_ch->in_room->area);
            break;
        }
        if (i[0] == 2 && i[0] != i[1])
            ind /= 2;

        if (ind > obj->value[5])
            ind = obj->value[5]; /* Can only win as much as there is */

        sprintf(buf, "You won %d coins!\n\r", ind);
        send_to_char(buf, cmd_ch);

        GET_MONEY(cmd_ch,CURR_GOLD) += ind;
        obj->value[5] -= ind;
        return(TRUE);
    }

    send_to_char("Sorry, you lost.\n\r", cmd_ch);
    return(TRUE);
}

void do_bet(CHAR_DATA *ch, char *argument)
{
    send_to_char("You can't do that here.\n\r", ch);
}

#define ROULETTE_HOUSE_LIMIT   50000
#define ROULETTE_PLACES        31
#define ROULETTE_PLACE_RED     ROULETTE_PLACES+1
#define ROULETTE_PLACE_BLACK   ROULETTE_PLACES+2
SPECIAL_FUNC(spec_RouletteWheel)
{
    char arg2[MAX_INPUT_LENGTH];
    int currtype = CURR_GOLD;
    int bet=0,placement=0,roll=0;

    if (type!=SFT_COMMAND || cmd->do_fun!=do_bet)
        return(FALSE);

    arg = one_argument(arg, arg2);

    if (arg2[0] == '\0' || !arg || !*arg)
    {
        send_to_char("To play roulette, bet <coins> <0-31|red|black>\n\r", cmd_ch);
        return(TRUE);
    }

    bet = atoi(arg2);
    if (bet <= 0)
    {
        send_to_char("You must bet a positive amount.\n\r", cmd_ch);
        return(TRUE);
    }

    if (bet > ROULETTE_HOUSE_LIMIT && !IS_IMMORTAL(cmd_ch))
    {
        ch_printf(cmd_ch, "I'm sorry, but house rules say you can't bet more than %d coins.\n\r",
                  ROULETTE_HOUSE_LIMIT);
        return(TRUE);
    }
    if (GET_MONEY(cmd_ch, currtype) < bet)
    {
        ch_printf(cmd_ch, "You do not have that much %s.\n\r", curr_types[currtype]);
        return(TRUE);
    }

    if (!str_cmp(arg, "red"))
        placement = ROULETTE_PLACE_RED;
    else if (!str_cmp(arg, "black"))
        placement = ROULETTE_PLACE_BLACK;
    else
    {
        placement = atoi(arg);
        if (placement < 0 || placement > ROULETTE_PLACES)
        {
            send_to_char("Please pick red, black, or a number from 0 to 31.\n\r", cmd_ch);
            return(TRUE);
        }
    }

    GET_MONEY(cmd_ch, currtype) -= bet;
    roll = number_range(0, ROULETTE_PLACES);
    send_to_char("The dealer spins the wheel...\n\r", cmd_ch);
    send_to_char("The dealer drops the ball...\n\r", cmd_ch);

    if (roll==0)
        send_to_char("&WThe ball lands on &G00&W!\n\r", cmd_ch);
    else
        ch_printf(cmd_ch, "&WThe ball lands on %s %d&W!\n\r",
                  roll%2?"&zblack":"&rred", roll);

    if (placement==roll)
    {
        bet*=32;
        ch_printf(cmd_ch, "You picked %d, you win %d coins!!!\n\r", placement, bet);
    }
    else if (roll%2==0 && roll!=0 && placement==ROULETTE_PLACE_RED)
    {
        bet*=2;
        ch_printf(cmd_ch, "You bet on red, you win %d coins!!!\n\r", bet);
    }
    else if (roll%2!=0 && placement==ROULETTE_PLACE_BLACK)
    {
        bet*=2;
        ch_printf(cmd_ch, "You bet on black, you win %d coins!!!\n\r", bet);
    }
    else
    {
        send_to_char("You lost!\n\r", cmd_ch);
        bet=0;
    }

    if (bet>1000)
    {
        char buf[MAX_INPUT_LENGTH];
        sprintf(buf, "$n wins %d coins on the roulette wheel.", bet);
        act(AT_WHITE, buf, cmd_ch, NULL, NULL, TO_ROOM);
    }

    GET_MONEY(cmd_ch, currtype) += bet;

    set_char_color(AT_PLAIN, cmd_ch);
    return(TRUE);
}


const char *card_nums[13] =
{
    "ace", "two", "three", "four", "five", "six", "seven", "eight",
    "nine", "ten", "jack", "queen", "king"
};
const char *card_suites[4] =
{
    "hearts", "spades", "diamonds", "clubs"
};

static char *cardname(int card)
{
    static char retbuf[32];

    snprintf(retbuf, 32, "%s%s of %s&w",
             (card%2)==0?"&R":"&z",
             card_nums[(card%13)],
             card_suites[(card%4)]);

    return retbuf;
}

static int handtotal(char *hand)
{
    int x, y, tot=0, aces=0;

    for (x=0;x<strlen(hand);x++)
    {
        y = hand[x]-'a'+1;
        if (y<1 || y>13)
        {
            bug("spec_VideoBlackJack: hand has invalid card: %d", y);
            y = 10;
        }
        if (y == 1)
            aces++;
        else if (y > 10)
            tot += 10;
        else
            tot += y;
    }

    while (aces-- > 0)
    {
        if (tot <= 10)
            tot += 11;
        else
            tot += 1;
    }

    return tot;
}

#define BLACKJACK_CARDS 52
SPECIAL_FUNC(spec_VideoBlackJack)
{
    OBJ_DATA *obj = (OBJ_DATA *)proc;
    VAR_DATA *var;
    int x, currtype = CURR_GOLD, bet = 0;
    char deck[BLACKJACK_CARDS+1];
    char phand[BLACKJACK_CARDS], dhand[BLACKJACK_CARDS];
    char tbuf[2];

    if (type==SFT_UPDATE && obj && number_percent()<10)
    {
        send_to_room("A blackjack dealer looks at you expectantly.\n\r", obj->in_room);
        return TRUE;
    }

    if (type!=SFT_COMMAND)
        return FALSE;

    if (cmd->do_fun == do_bet)
    {
        char arg2[MAX_INPUT_LENGTH];
        int y;

        if (get_var(cmd_ch->vars, "VBJdeck"))
        {
            send_to_char("You are already playing a game, you cannot bet any more.\n\r", cmd_ch);
            return TRUE;
        }

        arg = one_argument(arg, arg2);
        if (!is_number(arg2))
        {
            send_to_char("Bet how much?\n\r", cmd_ch);
            return TRUE;
        }
        bet = atoi(arg2);

        if (arg && *arg)
            currtype = get_currency_type(arg);

        if (currtype == CURR_NONE || bet <= 0)
        {
            send_to_char("Bet how much?\n\r", cmd_ch);
            return TRUE;
        }

        if (GET_MONEY(cmd_ch, currtype) < bet)
        {
            ch_printf(cmd_ch, "You don't have that much %s.\n\r",
                      curr_types[currtype]);
            return TRUE;
        }

        cmd_ch->money[currtype] -= bet;

        set_var_int(&cmd_ch->vars, "VBJbet", bet);
        set_var_int(&cmd_ch->vars, "VBJcurr", currtype);

        for (x=0;x<BLACKJACK_CARDS;x++)
            deck[x]='c';
        deck[BLACKJACK_CARDS]='\0';

        if (bet > 5000)
        {
            char buf[32];
            sprintf(buf, "%d %s", bet, curr_types[currtype]);
            act(AT_PLAIN, "$n bets $t on a game of blackjack!", cmd_ch, buf, NULL, TO_ROOM);
        }

        send_to_char("You start a new game, and the dealer suffles the deck.\n\r", cmd_ch);

        phand[0]='\0';
        x = number_range(0,BLACKJACK_CARDS-1);
        sprintf(tbuf, "%c", 'a'+(x%13));
        strcat(phand, tbuf);
        deck[x] = '-';

        dhand[0]='\0';
        while ((y = number_range(0,BLACKJACK_CARDS-1)) == x) ;
        sprintf(tbuf, "%c", 'a'+(y%13));
        strcat(dhand, tbuf);
        deck[y] = '-';

        ch_printf(cmd_ch, "You are dealt a %s.\n\r", cardname(x));
        if (IS_IMMORTAL(cmd_ch))
            ch_printf(cmd_ch, "Dealer gets a %s.\n\r", cardname(y));

        set_var(&cmd_ch->vars, "VBJdeck", deck);
        set_var(&cmd_ch->vars, "VBJphand", phand);
        set_var(&cmd_ch->vars, "VBJdhand", dhand);
        return TRUE;
    }

    if (cmd->do_fun != do_kill &&
        cmd->do_fun != do_wear)
        return FALSE;

    if (!(var = get_var(cmd_ch->vars, "VBJdeck")))
        return FALSE;

    strncpy(deck, get_var_val_ch(cmd_ch, "VBJdeck"), BLACKJACK_CARDS+1);
    strncpy(phand, get_var_val_ch(cmd_ch, "VBJphand"), BLACKJACK_CARDS);
    strncpy(dhand, get_var_val_ch(cmd_ch, "VBJdhand"), BLACKJACK_CARDS);

    if (cmd->do_fun == do_kill)
    {
        int c = 0;
        x = number_range(0,BLACKJACK_CARDS-1);
        while (deck[x] == '-' && c < 50)
        {
            x = number_range(0,BLACKJACK_CARDS-1);
            c++;
        }

        if (deck[x] == '-')
        {
            bug("spec_VideoBlackJack: couldn't get a card!?!?");
            send_to_char("This machine is broken, please try another...\n\r", cmd_ch);
            return TRUE;
        }

        sprintf(tbuf, "%c", 'a'+(x%13));
        strcat(phand, tbuf);
        deck[x] = '-';

        ch_printf(cmd_ch, "You get a %s, your hand total is %d.\n\r",
                  cardname(x), handtotal(phand));

        if (handtotal(phand) > 21)
        {
            send_to_char("You bust.\n\r", cmd_ch);
            del_var(cmd_ch->vars, "VBJdeck");
            del_var(cmd_ch->vars, "VBJhand");
            del_var(cmd_ch->vars, "VBJcurr");
            del_var(cmd_ch->vars, "VBJbet");
            return TRUE;
        }
    }

    /* dealer takes a card/cards */
    while (handtotal(dhand) < 17)
    {
        int c = 0;
        x = number_range(0,BLACKJACK_CARDS-1);
        while (deck[x] == '-' && c < 50)
        {
            x = number_range(0,BLACKJACK_CARDS-1);
            c++;
        }

        if (deck[x] == '-')
        {
            bug("spec_VideoBlackJack: couldn't get a card!?!?");
            send_to_char("This machine is broken, please try another...\n\r", cmd_ch);
            return TRUE;
        }

        sprintf(tbuf, "%c", 'a'+(x%13));
        strcat(dhand, tbuf);
        deck[x] = '-';

        if (!IS_IMMORTAL(cmd_ch))
            ch_printf(cmd_ch, "Dealer gets a %s.\n\r",
                      cardname(x));
        else
            ch_printf(cmd_ch, "Dealer gets a %s, dealer has %d.\n\r",
                      cardname(x), handtotal(dhand));

        if (cmd->do_fun == do_kill)
            break;
    }

    if (cmd->do_fun == do_wear || handtotal(phand) > 21)
    {
        bet = UMAX(0, get_var_val_int_ch(cmd_ch, "VBJbet"));
        currtype = URANGE(CURR_GOLD, get_var_val_int_ch(cmd_ch, "VBJcurr"), MAX_CURR_TYPE-1);
        if (handtotal(dhand) > 21)
        {
            send_to_char("Dealer busts.\n\r", cmd_ch);
            bet *= 2;
        }
        else if (handtotal(phand) > handtotal(dhand))
        {
            send_to_char("You win.\n\r", cmd_ch);
            bet *= 2;
        }
        else if (handtotal(dhand) > handtotal(phand))
        {
            send_to_char("Dealer wins.\n\r", cmd_ch);
            bet = 0;
        }
        else
        {
            send_to_char("Tied.\n\r", cmd_ch);
        }

        if (bet)
        {
            if (bet > 5000)
            {
                char buf[32];
                sprintf(buf, "%d %s", bet, curr_types[currtype]);
                act(AT_PLAIN, "$n wins $t on a game of blackjack!", cmd_ch, buf, NULL, TO_ROOM);
            }
            ch_printf(cmd_ch, "You win %d %s coins!\n\r",
                      bet, curr_types[currtype]);

            GET_MONEY(cmd_ch, currtype) += bet;
        }

        del_var(cmd_ch->vars, "VBJdeck");
        del_var(cmd_ch->vars, "VBJphand");
        del_var(cmd_ch->vars, "VBJdhand");
        del_var(cmd_ch->vars, "VBJcurr");
        del_var(cmd_ch->vars, "VBJbet");
        return TRUE;
    }

    set_var(&cmd_ch->vars, "VBJdeck", deck);
    set_var(&cmd_ch->vars, "VBJphand", phand);
    set_var(&cmd_ch->vars, "VBJdhand", dhand);

    return TRUE;
}

const char *rainbow_colors[5] =
{
    "&R", "&Y", "&G", "&B", "&P"
};

SPECIAL_FUNC(spec_rainbow)
{
    CHAR_DATA *tch;
    char buf[MAX_INPUT_LENGTH];
    int x;

    if (type != SFT_COMMAND || cmd->do_fun != do_say)
        return FALSE;

    buf[0]='\0';
    for (x=0;x<strlen(arg);x++)
        sprintf(buf+strlen(buf), "%s%c", rainbow_colors[x%5], arg[x]);

    for (tch=cmd_ch->in_room->first_person;tch;tch=tch->next_in_room)
        if (cmd_ch!=tch && !IS_NPC(tch))
            break;

    if (!tch || cmd_ch==tch || IS_NPC(tch))
        return FALSE;

    for (tch=cmd_ch->in_room->first_person;tch;tch=tch->next_in_room)
        if (cmd_ch==tch)
            ch_printf(tch, "&WYou say '%s&W'\n\r", arg);
        else
            ch_printf(tch, "&W%s says '%s&W'\n\r", PERS(cmd_ch,tch), buf);

    return TRUE;
}

void base_berserk(CHAR_DATA *ch);
SPECIAL_FUNC(spec_BerserkerItem)
{
    OBJ_DATA *obj = (OBJ_DATA *)proc;

    if (type != SFT_UPDATE ||
        !obj->carried_by ||
        obj->wear_loc == WEAR_NONE)
        return FALSE;

    if (!who_fighting(obj->carried_by))
        return FALSE;

    if (is_affected(obj->carried_by, gsn_berserk))
        return FALSE;

    base_berserk(obj->carried_by);

    return TRUE;
}

SPECIAL_FUNC(spec_soap)
{
    OBJ_DATA *obj = (OBJ_DATA *)proc;
    AFFECT_DATA *paf, *paf_next;

    if (type != SFT_COMMAND || cmd->do_fun != do_use)
        return FALSE;

    if ( get_eq_char(cmd_ch, WEAR_HOLD) != obj )
        return FALSE;

    if ( !arg || !*arg || str_cmp(arg, "soap") )
        return FALSE;

    if (is_affected(cmd_ch, gsn_web))
    {
        for ( paf = cmd_ch->first_affect; paf; paf = paf_next )
        {
            paf_next = paf->next;

            if (paf->type != gsn_web)
                continue;

            affect_remove( cmd_ch, paf );
        }

        act(AT_PLAIN, "You wash some webbing off with $p.", cmd_ch, obj, NULL, TO_CHAR);
        act(AT_PLAIN, "$n washes some webbing off with $p.", cmd_ch, obj, NULL, TO_ROOM);
    }
    else
    {
        act(AT_PLAIN, "You give yourself a good lathering with $p.", cmd_ch, obj, NULL, TO_CHAR);
        act(AT_PLAIN, "$n gives $mself a good lathering with $p.", cmd_ch, obj, NULL, TO_ROOM);
    }

    if (--obj->value[0] <= 0)
    {
        act(AT_PLAIN, "That used up $p.", cmd_ch, obj, NULL, TO_CHAR);
        obj_from_char( obj );
        extract_obj( obj );
    }

    return TRUE;
}

SPECIAL_FUNC(spec_VideoPoker)
{
    return FALSE;
}

SPECIAL_FUNC(spec_EvilBlade)
{
    OBJ_DATA *obj = (OBJ_DATA *)proc;

    if ( obj )
        bug( "Obj with unfinished proc." );

    return FALSE;
}

SPECIAL_FUNC(spec_jive_box)
{
    OBJ_DATA *obj = (OBJ_DATA *)proc;

    if ( obj )
        bug( "Obj with unfinished proc." );

    return FALSE;
}

SPECIAL_FUNC(spec_BitterBlade)
{
    OBJ_DATA *obj = (OBJ_DATA *)proc;

    if ( obj )
        bug( "Obj with unfinished proc." );

    return FALSE;
}

SPECIAL_FUNC(spec_portal)
{
    OBJ_DATA *obj = (OBJ_DATA *)proc;

    if ( obj )
      bug( "Obj with unfinished proc." );

    return FALSE;
}

SPECIAL_FUNC(spec_scraps)
{
    OBJ_DATA *obj = (OBJ_DATA *)proc;

    if ( obj )
      bug( "Obj with unfinished proc." );

    return FALSE;
}

SPECIAL_FUNC(spec_antioch_grenade)
{
    OBJ_DATA *obj = (OBJ_DATA *)proc;

    if ( obj )
      bug( "Obj with unfinished proc." );

    return FALSE;
}

SPECIAL_FUNC(spec_GoodBlade)
{
    OBJ_DATA *obj = (OBJ_DATA *)proc;

    if ( obj )
      bug( "Obj with unfinished proc." );

    return FALSE;
}

SPECIAL_FUNC(spec_NeutralBlade)
{
    OBJ_DATA *obj = (OBJ_DATA *)proc;

    if ( obj )
      bug( "Obj with unfinished proc." );

    return FALSE;
}

SPECIAL_FUNC(spec_chainsaw)
{
    OBJ_DATA *obj = (OBJ_DATA *)proc;

    if ( obj )
      bug( "Obj with unfinished proc." );

    return FALSE;
}

SPECIAL_FUNC(spec_CrapTable)
{
    OBJ_DATA *obj = (OBJ_DATA *)proc;

    if ( obj )
      bug( "Obj with unfinished proc." );

    return FALSE;
}

