/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*static char rcsid[] = "$Id: rspecial.c,v 1.20 2003/01/26 23:43:56 dotd Exp $";*/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

DECLARE_DO_FUN(do_north);
DECLARE_DO_FUN(do_south);
DECLARE_DO_FUN(do_east);
DECLARE_DO_FUN(do_west);
DECLARE_DO_FUN(do_up);
DECLARE_DO_FUN(do_down);
DECLARE_DO_FUN(do_northeast);
DECLARE_DO_FUN(do_northwest);
DECLARE_DO_FUN(do_southeast);
DECLARE_DO_FUN(do_southwest);
DECLARE_DO_FUN(do_drop);
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_offer);
DECLARE_DO_FUN(do_rent);
DECLARE_DO_FUN(do_get);
DECLARE_DO_FUN(do_flee);
DECLARE_DO_FUN(do_clans);

#define SPEC SPECIAL_FUNC
#include "rspecial.h"
#undef SPEC

#define SPEC(func) \
    if (++i==2) {i=0;send_to_pager("\n\r",ch);} pager_printf(ch, "%-38s", #func)
void do_rslist( CHAR_DATA *ch, char *argument )
{
    int i=-1;

    set_pager_color(AT_PLAIN, ch);

    #include "rspecial.h"

    send_to_pager( "\n\r", ch );
}
#undef SPEC

#define SPEC(func) \
    if (room->spec_fun == func && strstr(#func, argument)) \
    { ch_printf(ch, "%5d: %s\n\r", room->vnum, #func); match++; continue; }
void do_rsfind( CHAR_DATA *ch, char *argument )
{
    int hash, match=0;
    ROOM_INDEX_DATA *room;
    extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
    
    if (!argument)
    {
        send_to_char("rsfind <name>\n\r", ch);
        return;
    }
    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
        for (room=room_index_hash[hash];room;room=room->next)
        {
            if (!room->spec_fun)
                continue;
#include "rspecial.h"
        }
    ch_printf(ch, "%d matches found.\n\r", match);
}
#undef SPEC

/*
 * Given a name, return the appropriate spec fun.
 */
#define SPEC(func) \
    if ( !str_cmp( name, #func ) ) return func
SPEC_FUN *r_spec_lookup( const char *name )
{
#include "rspecial.h"
    return NULL;
}
#undef SPEC

/*
 * Given a pointer, return the appropriate spec fun text.
 */
#define SPEC(func) \
    if ( special == func ) return #func
char *r_lookup_spec( SPEC_FUN *special )
{
#include "rspecial.h"
    return "";
}
#undef SPEC


SPECIAL_FUNC(spec_dump)
{
    ROOM_INDEX_DATA *rp=(ROOM_INDEX_DATA *)proc;
    OBJ_DATA *k;

    for (k=rp->first_content;k;k=rp->first_content)
	extract_obj(k);

    if (type != SFT_COMMAND || cmd->do_fun != do_drop)
	return FALSE;

    do_drop(cmd_ch, arg);

    for (k=rp->first_content;k;k=rp->first_content)
    {
        separate_obj(k);
	act(AT_PLAIN,"$p disappears under the rubble.",cmd_ch,k,NULL,TO_CHAR);
	act(AT_PLAIN,"$p disappears under the rubble.",cmd_ch,k,NULL,TO_ROOM);
	extract_obj(k);
    }

    return TRUE;
}

SPECIAL_FUNC(spec_donation)
{
    ROOM_INDEX_DATA *rp=(ROOM_INDEX_DATA *)proc;

    if (!IS_DONATION(rp))
        return FALSE;

    if (!cmd)
    {
	OBJ_DATA *k;
	send_to_room("A sign blinks, 'Welcome to donation!'\n\r", rp);
	for (k=rp->first_content;k;k=rp->first_content)
	{
	    obj_from_room(k);
	    obj_to_room(k,DONATION_ROOM());
	}
	return TRUE;
    }

    return FALSE;
}

bool is_house_owner(CHAR_DATA *ch, ROOM_INDEX_DATA *house)
{
    char housename[MAX_INPUT_LENGTH];
    unsigned int i;

    if (!ch || !house || house->spec_fun != spec_house)
	return(FALSE);

    strcpy(housename,house->name);

    for (i=0;i<strlen(housename);i++)
	if (housename[i]==' ' || housename[i]=='\'') break;

    strncpy(housename,house->name,i);
    if (strncmp(housename,GET_NAME(ch),strlen(GET_NAME(ch))))
    {
	send_to_char("Sorry, you'll have to find your own house.\n\r",ch);
	return(FALSE);
    }

    return(TRUE);
}

SPECIAL_FUNC(spec_house)
{
    ROOM_INDEX_DATA *rp=(ROOM_INDEX_DATA *)proc;

    if (type!=SFT_COMMAND || IS_NPC(cmd_ch))
	return(FALSE);

    if (cmd->do_fun==do_get)
    {
	OBJ_DATA *obj;

	if (strncmp(arg,"housekey",8))
	    return(FALSE);

        if (!is_house_owner(cmd_ch,rp))
        {
            send_to_char("You search all the secret places you would hide a key but find nothing.\n\r", cmd_ch);
            return(TRUE);
        }

	if (!(obj = create_object(rp->vnum)))
	{
	    send_to_char("You search all the secret places you would hide a key but find nothing.\n\r", cmd_ch);
	    return(TRUE);
	}

	obj = obj_to_char(obj, cmd_ch);
	send_to_char("You get your house key from it's secret hiding place.\n\r", cmd_ch);

	return(TRUE);
    }

    if (cmd->do_fun==do_rent)
    {
        if (!is_house_owner(cmd_ch,rp))
            return(FALSE);

        SET_BIT(rp->room_flags,ROOM_RECEPTION);
        do_rent(cmd_ch,"");
        REMOVE_BIT(rp->room_flags,ROOM_RECEPTION);

        return(TRUE);
    }

    if (cmd->do_fun==do_offer)
    {
        if (!is_house_owner(cmd_ch,rp))
            return(FALSE);

        SET_BIT(rp->room_flags,ROOM_RECEPTION);
        do_offer(cmd_ch,"");
        REMOVE_BIT(rp->room_flags,ROOM_RECEPTION);

        return(TRUE);
    }

    return(FALSE);
}

SPECIAL_FUNC(spec_clan_headquarters)
{
    ROOM_INDEX_DATA *rp=(ROOM_INDEX_DATA *)proc;
    CLAN_DATA *clan;
    void show_clan_to_pc( CHAR_DATA *ch, CLAN_DATA *clan );
    
    if (type!=SFT_COMMAND || IS_NPC(cmd_ch))
	return(FALSE);

    if (cmd->do_fun!=do_clans)
        return(FALSE);

    for ( clan = first_clan; clan; clan = clan->next )
        if ( strstr( rp->name, clan->name ) )
            break;

    if ( !clan || cmd_ch->pcdata->clan != clan || !strstr( rp->name, clan->name ) )
        return(FALSE);

    show_clan_to_pc(cmd_ch, clan);
    return(TRUE);
}

#define ROOM_VNUM_LAKEMEN_INN 8209
SPECIAL_FUNC(spec_desolation_lakementown)
{
    if (type != SFT_COMMAND || IS_NPC(cmd_ch))
        return FALSE;

    cmd_ch->pcdata->home = ROOM_VNUM_LAKEMEN_INN;

    return FALSE;
}

bool spec_challenge_prep_room(void *proc, CMDTYPE *cmd, char *arg, CHAR_DATA *cmd_ch, sh_int type, int cl)
{
    ROOM_INDEX_DATA *rp=(ROOM_INDEX_DATA *)proc, *chalroom;
    CHAR_DATA *chalmob;
    int vnum, fvnum = 0;

    if (type!=SFT_ARGUMENT || IS_NPC(cmd_ch))
        return FALSE;

    if (str_cmp(arg, "nod "))
        return FALSE;

    if (!IS_ACTIVE(cmd_ch, cl))
        return FALSE;

    if (GET_LEVEL(cmd_ch, cl) < 10)
    {
        send_to_char("You have no business here until you are level 10.\n\r", cmd_ch);
        return TRUE;
    }

    if (GET_EXP(cmd_ch) < exp_level(cmd_ch, GET_LEVEL(cmd_ch, cl)+1, cl))
    {
        send_to_char("You are not ready for your next challenge, gain more experience.\n\r", cmd_ch);
        return TRUE;
    }

    if (cmd_ch->first_carrying)
    {
        send_to_char("Please remove all your equipment before the challenge.\n\r", cmd_ch);
        return TRUE;
    }

    if (!(chalroom = get_room_index(rp->vnum+1)))
    {
        send_to_char("The challenge room is gone, contact a God.\n\r", cmd_ch);
        bug("spec_challenge_prep_room: challenge room is gone!");
        return TRUE;
    }

    if (chalroom->first_person)
    {
        send_to_char("The challenge room is busy, please try again later.\n\r", cmd_ch);
        return TRUE;
    }

    if (cl == CLASS_DRUID)
        fvnum = MOB_VNUM_DRUID_CHALLENGE;
    else if (cl == CLASS_MONK)
        fvnum = MOB_VNUM_MONK_CHALLENGE;

    vnum = fvnum + GET_LEVEL(cmd_ch, cl) - 10;
    vnum = URANGE(fvnum, vnum, fvnum+40);

    if (!(chalmob = create_mobile(vnum)))
    {
        send_to_char("The challenge has been called off, contact a God.\n\r", cmd_ch);
        bug("spec_challenge_prep_room: can't find challenge mob");
        return TRUE;
    }

    char_to_room(chalmob, chalroom);

    act(AT_PLAIN, "You are ushered into the challenge room.", cmd_ch, NULL, NULL, TO_CHAR);
    act(AT_PLAIN, "$n is ushered into the challenge room.", cmd_ch, NULL, NULL, TO_ROOM);

    char_from_room(cmd_ch);
    char_to_room(cmd_ch, chalroom);
    do_look(cmd_ch, "auto");

    return TRUE;
}

SPECIAL_FUNC(spec_druid_challenge_prep_room)
{
    return spec_challenge_prep_room(proc,cmd,arg,cmd_ch,type,CLASS_DRUID);
}

SPECIAL_FUNC(spec_monk_challenge_prep_room)
{
    return spec_challenge_prep_room(proc,cmd,arg,cmd_ch,type,CLASS_MONK);
}

bool spec_challenge_room(void *proc, CMDTYPE *cmd, char *arg, CHAR_DATA *cmd_ch, sh_int type, int cl)
{
    ROOM_INDEX_DATA *rp=(ROOM_INDEX_DATA *)proc, *preproom;
    CHAR_DATA *chalmob = NULL, *chalch = NULL, *vch;
    int fvnum = 0;

    if (type != SFT_UPDATE && type != SFT_COMMAND)
        return FALSE;

    if (cl == CLASS_DRUID)
        fvnum = MOB_VNUM_DRUID_CHALLENGE;
    else if (cl == CLASS_MONK)
        fvnum = MOB_VNUM_MONK_CHALLENGE;

    for ( vch = rp->first_person; vch; vch = vch->next_in_room )
        if (IS_IMMORTAL(vch))
            continue;
        else if (!IS_NPC(vch))
            chalch = vch;
        else if (vch->vnum >= fvnum ||
                 vch->vnum <= fvnum+40)
            chalmob = vch;

    if (!chalch && chalmob)
    {
        extract_char(chalmob, TRUE);
        return FALSE;
    }

    if (type != SFT_COMMAND)
        return FALSE;

    if (IS_NPC(cmd_ch) || IS_IMMORTAL(cmd_ch) || !IS_ACTIVE(cmd_ch, cl))
        return FALSE;

    if (GET_EXP(cmd_ch) < exp_level(cmd_ch, GET_LEVEL(cmd_ch, cl)+1, cl))
        return FALSE;

    preproom = get_room_index(rp->vnum-1);

    if (!chalmob)
    {
        send_to_char("You won the challenge!\n\r", cmd_ch);

        advance_level(cmd_ch, cl);

        char_from_room(cmd_ch);
        char_to_room(cmd_ch, preproom);
        do_look(cmd_ch, "auto");
        return TRUE;
    }

    if (cmd->do_fun == do_north ||
        cmd->do_fun == do_south ||
        cmd->do_fun == do_east ||
        cmd->do_fun == do_west ||
        cmd->do_fun == do_northeast ||
        cmd->do_fun == do_northwest ||
        cmd->do_fun == do_southeast ||
        cmd->do_fun == do_southwest ||
        cmd->do_fun == do_up ||
        cmd->do_fun == do_down ||
        cmd->do_fun == do_flee)
    {
        int loss;

        extract_char(chalmob, TRUE);

        loss = GET_EXP(cmd_ch) - exp_level(cmd_ch, GET_LEVEL(cmd_ch, cl), cl);
        loss /= 2;
        loss = UMIN(loss, 50000000);
        gain_exp(cmd_ch, -loss);

        ch_printf(cmd_ch, "You lost the challenge and %d experience.\n\r", loss);

        char_from_room(cmd_ch);
        char_to_room(cmd_ch, preproom);
        do_look(cmd_ch, "auto");
        return TRUE;
    }

    return FALSE;
}

SPECIAL_FUNC(spec_druid_challenge_room)
{
    return spec_challenge_room(proc,cmd,arg,cmd_ch,type,CLASS_DRUID);
}

SPECIAL_FUNC(spec_monk_challenge_room)
{
    return spec_challenge_room(proc,cmd,arg,cmd_ch,type,CLASS_MONK);
}

SPECIAL_FUNC(spec_StatueRoom)
{
    ROOM_INDEX_DATA *rp=(ROOM_INDEX_DATA *)proc;

    if (!cmd) {
	send_to_room("A sign blinks.\n\r", rp);
	return TRUE;
    }

    return FALSE;
}

