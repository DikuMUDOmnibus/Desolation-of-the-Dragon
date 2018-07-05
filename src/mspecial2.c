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
#include "gsn.h"

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
DECLARE_DO_FUN(do_gossip);
DECLARE_DO_FUN(do_shout);
DECLARE_DO_FUN(do_open);
DECLARE_DO_FUN(do_close);
DECLARE_DO_FUN(do_emote);
DECLARE_DO_FUN(do_say);
DECLARE_DO_FUN(do_tell);
DECLARE_DO_FUN(do_cast);
DECLARE_DO_FUN(do_enter);
DECLARE_DO_FUN(do_list);
DECLARE_DO_FUN(do_buy);

#define SPEC SPECIAL_FUNC
#include "mspecial.h"
#undef SPEC

bool summon_if_hating args( ( CHAR_DATA *ch ) );

#define VAR_SISYPHUS "sisyphus"
SPECIAL_FUNC(spec_sisyphus)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc, *vch;
    VAR_DATA *var;
    bool level_too_high = FALSE;

    if (type == SFT_UPDATE)
        return spec_warrior(proc,cmd,arg,cmd_ch,type);

    if (type != SFT_COMMAND || ch == cmd_ch)
        return FALSE;

    if (GetMaxLevel(cmd_ch)>15 && !IS_IMMORTAL(cmd_ch))
        level_too_high = TRUE;
    else
    {
        for (vch = cmd_ch->in_room->first_person; vch; vch = vch->next_in_room)
            if (is_same_group(cmd_ch, vch) && GetMaxLevel(vch)>15)
            {
                level_too_high = TRUE;
                break;
            }
    }

    if (!level_too_high)
    {
        if (cmd->do_fun == do_west)
        {
            do_open(ch, "gate");
            do_emote(ch, "nods approvingly.");
            do_west(cmd_ch, "");
            do_close(ch, "gate");
            return TRUE;
        }
        return FALSE;
    }

    if (!(vch = cmd_ch->master))
        vch = cmd_ch;

    if (!(var = get_var(vch->vars, VAR_SISYPHUS)))
    {
	set_var(&vch->vars, VAR_SISYPHUS, "0");
	var = get_var(vch->vars, VAR_SISYPHUS);
	if (!var)
	{
	    bug("spec_sisyphus: !var");
	    return FALSE;
	}
    }

    if (cmd->do_fun == do_west)
    {
	switch (var->val[0])
        {
	case '0':
        interpret(ch, "shake");
        act(AT_PLAIN, "$n blocks $N's way.", ch, NULL, cmd_ch, TO_NOTVICT);
        do_say(ch, "First, you'll have to get past me!");
	set_var(&vch->vars, VAR_SISYPHUS, "1");
	break;
	case '1':
        act(AT_PLAIN, "$n grabs $N and shoves $M away from the gate.", ch, NULL, cmd_ch, TO_NOTVICT);
	do_say(ch, "I told you, you go through me or you don't go at all!");
	set_var(&vch->vars, VAR_SISYPHUS, "2");
	break;
	case '2':
        act(AT_PLAIN, "$n roughly grabs $N and shoves $M to the ground.", ch, NULL, cmd_ch, TO_NOTVICT);
	cmd_ch->position = POS_SITTING;
	do_say(ch, "This is your last warning, leave or I will violently remove you.");
	set_var(&vch->vars, VAR_SISYPHUS, "3");
	break;
	case '3':
	start_hating(ch, vch);
	set_var(&vch->vars, VAR_SISYPHUS, "1");
	break;
	}
        return TRUE;
    }

    if (cmd->do_fun == do_open)
    {
        act(AT_PLAIN, "$n stops $N.", ch, NULL, cmd_ch, TO_NOTVICT);
        do_say(ch, "I don't think so.");
        interpret(ch, "grin");
        return TRUE;
    }

    return FALSE;
}

SPECIAL_FUNC(spec_StatTeller)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (type == SFT_UPDATE && number_percent()<10)
    {
        do_say(ch, "Fortunes read, only 1000 gold coins!");
        return TRUE;
    }

    if (type != SFT_COMMAND)
        return FALSE;

    if (cmd->do_fun == do_list)
    {
        do_say(ch, "I will read your fortune, and tell you three of your stats.");
        return TRUE;
    }

    if (cmd->do_fun == do_buy)
    {
        char buf[MAX_INPUT_LENGTH];
        int x[3],y;

        x[0] = number_range(1,7);
        x[1] = number_range(1,7);
        x[2] = number_range(1,7);
        while (x[1]==x[0])
            x[1] = number_range(1,7);
        while (x[2]==x[1] || x[2]==x[0])
            x[2] = number_range(1,7);

        sprintf(buf, "%s Here is your fortune: ", PERS(cmd_ch,ch));
        for (y=0;y<3;y++)
            switch (x[y])
            {
            case 1:
                sprintf(buf+strlen(buf),"STR: %d ",get_curr_str(cmd_ch));
                break;
            case 2:
                sprintf(buf+strlen(buf),"WIS: %d ",get_curr_wis(cmd_ch));
                break;
            case 3:
                sprintf(buf+strlen(buf),"INT: %d ",get_curr_int(cmd_ch));
                break;
            case 4:
                sprintf(buf+strlen(buf),"CON: %d ",get_curr_con(cmd_ch));
                break;
            case 5:
                sprintf(buf+strlen(buf),"DEX: %d ",get_curr_dex(cmd_ch));
                break;
            case 6:
                sprintf(buf+strlen(buf),"LCK: %d ",get_curr_lck(cmd_ch));
                break;
            case 7:
                sprintf(buf+strlen(buf),"CHR: %d ",get_curr_cha(cmd_ch));
                break;
            }

        do_tell(ch, buf);
        return TRUE;
    }

    return FALSE;
}

SPECIAL_FUNC(spec_coldcaster)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    char buf[MAX_INPUT_LENGTH];
    char *spell = "";
    int sn;

    if (type != SFT_UPDATE)
        return FALSE;

    if (!ch->fighting)
    {
        if (!is_affected(ch,gsn_protection_from_cold))
            spell="protection from cold";
        else
            return FALSE;
    }
    else
    {
        switch (number_range(0,3))
        {
        case 2:
            spell = "cone of cold";
            break;
        case 3:
            if (is_affected(who_fighting(ch), gsn_protection_from_cold))
                spell = "dispel magic";
            else
                spell = "ice storm";
            break;
        default:
            spell = "chill touch";
            break;
        }
    }

    if (!spell)
        return FALSE;

    if (!(sn = skill_lookup(spell)))
    {
        bug("spec_coldcaster: spell not found %s", spell);
        return FALSE;
    }
    sprintf(buf, "'%s'", spell);
    do_cast(ch, buf);
    return TRUE;
}

SPECIAL_FUNC(spec_firecaster)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    char buf[MAX_INPUT_LENGTH];
    char *spell = "";
    int sn;

    if (type != SFT_UPDATE)
        return FALSE;

    if (!ch->fighting)
    {
        if (!is_affected(ch,gsn_protection_from_fire))
            spell="protection from fire";
        else if (!is_affected(ch,gsn_fireshield))
            spell="fireshield";
        else
            return FALSE;
    }
    else
    {
        switch (number_range(0,5))
        {
        case 2:
            spell = "firestorm";
            break;
        case 3:
            spell = "flamestrike";
            break;
        case 4:
            if (is_affected(who_fighting(ch), gsn_protection_from_fire))
                spell = "dispel magic";
            else
                spell = "fireball";
            break;
        case 5:
            if (is_affected(who_fighting(ch), gsn_fireshield))
                spell = "dispel magic";
            else
                spell = "incendiary cloud";
            break;
        default:
            if (!is_affected(who_fighting(ch), gsn_faerie_fire))
                spell = "faerie fire";
            else
                spell = "burning hands";
            break;
        }
    }

    if (!spell)
        return FALSE;

    if (!(sn = skill_lookup(spell)))
    {
        bug("spec_firecaster: spell not found %s", spell);
        return FALSE;
    }
    sprintf(buf, "'%s'", spell);
    do_cast(ch, buf);
    return TRUE;
}

SPECIAL_FUNC(spec_psi)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    CHAR_DATA *victim, *cast_on = NULL;
    char *spell=NULL, buf[MAX_INPUT_LENGTH];
    int sn, position;

    if (type!=SFT_UPDATE ||
        IS_SET(ch->in_room->room_flags,ROOM_NO_MAGIC) ||
        ch->wait)
        return FALSE;

    if (!(victim=who_fighting(ch)))
    {
        victim=ch;
        cast_on=ch;
        if (!is_affected(ch,gsn_psishield)) {
            spell="psishield";
        } else if (!is_affected(ch,gsn_psistrength)) {
            spell="psistrength";
        } else if (!is_affected(ch,gsn_mindblank)) {
            spell="mindblank";
        } else if (!is_affected(ch,gsn_great_sight)) {
            spell="greatsight";
        } else if (!is_affected(ch,gsn_tower_of_iron_will)) {
            spell="towerofironwill";
        } else if (summon_if_hating(ch)) {
            return TRUE;
        } else if (!is_affected(ch,gsn_invis)) {
            spell="psiinvis";
        } else if (!is_affected(ch,gsn_chameleon)) {
            spell="chameleon";
        } else
            return FALSE;
    }

    if (!spell)
        return FALSE;

    if (!(sn = skill_lookup(spell)))
    {
        bug("spec_psi: spell not found %s", spell);
        return FALSE;
    }

    position = ch->position;
    ch->position = POS_STANDING;

#ifdef MUD_DEBUG
    log_printf_plus(LOG_DEBUG, LEVEL_IMMORTAL, SEV_DEBUG, "spec_psi: %s '%s'", GET_NAME(ch), spell);
#endif

    if (skill_table[sn]->type == SKILL_SKILL)
    {
        sprintf(buf, "%s %s", spell, cast_on==ch?"":cast_on?GET_NAME(cast_on):"");
        interpret(ch, buf);
    }
    else if (skill_table[sn]->type == SKILL_SPELL)
    {
        sprintf(buf, "'%s' %s", spell, cast_on==ch?"self":cast_on?GET_NAME(cast_on):"");
        do_cast(ch, buf);
    }
    else
        bug("spec_psi: spell %s not skill or spell", spell);

    ch->position = position;

    return TRUE;

#if 0
    {
        if (PSI_CAN(SKILL_CELL_ADJUSTMENT, PML) && (cmana > Qmana) &&
            (mob->points.hit < hpcan) )
        {
            do_say(mob,"That was too close for comfort.",1);
            mind_teleport(PML,mob,mob,NULL);
            mind_cell_adjustment(PML, mob, mob, NULL);
            return(TRUE);
        }
        if (cmana <= Qmana)
            if ((mob->points.hit>(hpcan+1))&&(mob->skills[SKILL_CANIBALIZE].learned))
            {
                if ((cmana + 2*(mob->points.hit - hpcan)) >= mob->points.max_mana)
                    sprintf(buf,"24"); /*Qmana=51>=cm, cm+(2*24) <= 99 */
                else sprintf(buf,"%d",(mob->points.hit - hpcan -1));
                do_canibalize(mob,buf,1);
                return(TRUE);
            }
            else if (mob->skills[SKILL_MEDITATE].learned)
            {
                do_meditate(mob,mob->player.name,1);
                return(TRUE);
            }
        if (IS_SET(mob->hatefield, HATE_CHAR) && (mob->points.hit>hpcan))
        {
            do_say(mob,"It's payback time!",1);
            mob->points.mana = 100;
            if (PSI_CAN(SKILL_PORTAL,PML) || PSI_CAN(SKILL_SUMMON,PML))
                return(Summoner(mob,0,NULL,mob,0));
        }
        return(TRUE);
    } /* end peace time castings */
    else
    { /*they are fighting someone, do something nasty to them!*/
        mob->points.mana = 100; /*some psi combat spells still cost mana,
        set to max mana start of every round of combat*/
        targ = mob->specials.fighting;
        if ((mob->points.max_hit-hpcan) > (1.5 * mob->points.hit) )
        {
            if (!mob->skills[SKILL_PSI_TELEPORT].learned || (!IsOnPmp(mob->in_room)))
            {
                act("$n looks around frantically.",0,mob,0,0,TO_ROOM);
                command_interpreter(mob,"flee");
                return(TRUE);
            }
            act("$n screams defiantly, 'I'll get you yet, $N!'",0,mob,0,targ,TO_ROOM);
            mind_teleport(PML,mob,mob,0);
            return(TRUE);
        }
        group = ((targ->followers || targ->master) ? TRUE : FALSE);
        log(buf);
        if (group && (dice(1,2)-1)) group = FALSE;
        if (!group)
        { /*not fighting a group, or has selected person fighting, for spec*/
            if ((dice(1,2)-1)) /* do special attack 50% of time */
            {
                if (IS_SET(targ->player.class,CLASS_MAGIC_USER|CLASS_CLERIC))
                    if ((dice(1,2)-1))
                        CAST_OR_BLAST(mob,targ,SKILL_TELEKINESIS);
                    else CAST_OR_BLAST(mob,targ,SKILL_MIND_WIPE);
                /*special attack for psi & sorc opponents */
                else if (IS_SET(targ->player.class,CLASS_PSI|CLASS_SORCERER))
                    if (affected_by_spell(targ,SPELL_FEEBLEMIND))
                        CAST_OR_BLAST(mob,targ,SKILL_DISINTEGRATE);
                    else CAST_OR_BLAST(mob,targ,SKILL_MIND_WIPE);
                /*special attack for fighter subtypes & thieves*/
                else if ((GetMaxLevel(targ) < 20) && (dice(1,2)-1))
                    CAST_OR_BLAST(mob,targ,SKILL_PSI_TELEPORT);
                else CAST_OR_BLAST(mob,targ,SKILL_DISINTEGRATE);
            }
            else C_OR_B(mob,targ); /* norm attack, psychic crush or psionic blast*/
        }
        else if (mob->skills[SKILL_ULTRA_BLAST].learned)
            mind_ultra_blast(PML,mob,targ,NULL);
        else if (mob->skills[SKILL_MIND_BURN].learned)
            mind_burn(PML,mob,targ,NULL);
        else do_blast(mob,targ->player.name,1);
        return(TRUE);
    } /* end of fighting stuff */


    return FALSE;
#endif
}

SPECIAL_FUNC(spec_regenerator)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    void char_regen(CHAR_DATA *ch);

    if (type != SFT_UPDATE)
        return FALSE;

    char_regen(ch);
    return FALSE;
}

/* future - write procs for each race - Garil 03/11/2001 */
SPECIAL_FUNC(spec_racial_specifics)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    switch (GET_RACE(ch))
    {
    case RACE_DRAGON:
    case RACE_DRAGON_RED:
    case RACE_DRAGON_BLACK:
    case RACE_DRAGON_GREEN:
    case RACE_DRAGON_WHITE:
    case RACE_DRAGON_BLUE:
    case RACE_DRAGON_SILVER:
    case RACE_DRAGON_GOLD:
    case RACE_DRAGON_BRONZE:
    case RACE_DRAGON_COPPER:
    case RACE_DRAGON_BRASS:
        return spec_dragon( ch, cmd, arg, cmd_ch, type );
    }

    return FALSE;
}


bool block_dir(CHAR_DATA *ch, CHAR_DATA *vch, CMDTYPE *cmd, DO_FUN *bcmd, sh_int type)
{
    if (type != SFT_COMMAND)
        return FALSE;

    if (cmd->do_fun==bcmd)
    {
        act(AT_PLAIN,"$n shakes $s head at you and blocks your way.", ch, 0, vch, TO_VICT);
        act(AT_PLAIN,"$n shakes $s head at $N and blocks $S way.", ch, 0, vch, TO_NOTVICT);
        return TRUE;
    }
    return FALSE;
}

SPECIAL_FUNC(spec_block_north)
{ return block_dir((CHAR_DATA *)proc, cmd_ch, cmd, do_north, type); }
SPECIAL_FUNC(spec_block_south)
{ return block_dir((CHAR_DATA *)proc, cmd_ch, cmd, do_south, type); }
SPECIAL_FUNC(spec_block_east)
{ return block_dir((CHAR_DATA *)proc, cmd_ch, cmd, do_east, type); }
SPECIAL_FUNC(spec_block_west)
{ return block_dir((CHAR_DATA *)proc, cmd_ch, cmd, do_west, type); }
SPECIAL_FUNC(spec_block_up)
{ return block_dir((CHAR_DATA *)proc, cmd_ch, cmd, do_up, type); }
SPECIAL_FUNC(spec_block_down)
{ return block_dir((CHAR_DATA *)proc, cmd_ch, cmd, do_down, type); }
SPECIAL_FUNC(spec_block_northeast)
{ return block_dir((CHAR_DATA *)proc, cmd_ch, cmd, do_northeast, type); }
SPECIAL_FUNC(spec_block_southeast)
{ return block_dir((CHAR_DATA *)proc, cmd_ch, cmd, do_southeast, type); }
SPECIAL_FUNC(spec_block_northwest)
{ return block_dir((CHAR_DATA *)proc, cmd_ch, cmd, do_northwest, type); }
SPECIAL_FUNC(spec_block_southwest)
{ return block_dir((CHAR_DATA *)proc, cmd_ch, cmd, do_southwest, type); }


DO_FUN *dir_commands[LAST_NORMAL_DIR+1] =
{
    do_north,
    do_east,
    do_south,
    do_west,
    do_up,
    do_down,
    do_northeast,
    do_northwest,
    do_southeast,
    do_southwest,
    do_enter /* somewhere */
};

SPECIAL_FUNC(spec_clan_guard)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;
    CLAN_DATA *clan;
    EXIT_DATA *pexit;

    if (type != SFT_COMMAND || !IS_AWAKE(ch) ||
        IS_NPC(cmd_ch) || !can_see(ch, cmd_ch))
        return FALSE;

    for (clan = first_clan; clan; clan = clan->next)
        if (clan->guard1 == ch->vnum ||
            clan->guard2 == ch->vnum)
            break;

    if (!clan ||
        (clan->guard1 != ch->vnum &&
         clan->guard2 != ch->vnum))
        return FALSE;

    if (cmd_ch->pcdata->clan == clan)
        return FALSE;

    for (pexit = ch->in_room->first_exit; pexit; pexit = pexit->next)
        if (pexit->to_room->vnum > ch->in_room->vnum &&
            pexit->to_room->area == ch->in_room->area &&
            cmd->do_fun == dir_commands[pexit->vdir])
        {
            do_say(ch, "I'm sorry, but you're not allowed to go in there.");
            return TRUE;
        }

    return FALSE;
}

SPECIAL_FUNC(spec_homer_simpson)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (type != SFT_UPDATE || !IS_AWAKE(ch))
        return FALSE;

    if (ch->tempnum == 1) {
        do_shout(ch, "Marge... I have to pee...");
        do_emote(ch, "does a little dance.");
        ch->tempnum = 0;
        return TRUE;
    }

    switch (number_range(0, 200))
    {
    case 0:
        do_say(ch, "mmmmm..... beer.....");
        return TRUE;
    case 1:
        do_emote(ch,"looks into a fish pond.");
        do_say(ch, "mmmmm.....  unprocessed fish sticks.....");
        return TRUE;
    case 2:
        do_say(ch, "mmmmm.....  forbidden donut..... uhhhhhh....");
        return TRUE;
    case 3:
        do_say(ch, "Don't eat me! I have a wife and kids, eat them!");
        return TRUE;
    case 4:
        do_say(ch, "mmmmm..... hog fat.....");
        return TRUE;
    case 5:
        do_say(ch, "mmmmm..... 64 slices of american cheese.....");
        return TRUE;
    case 6:
        do_emote(ch, "falls to the ground.");
        if (number_range(0,40)==0)
            do_shout(ch, "...damn you!  Dam you all to hell!");
        else
            do_say(ch, "...damn you!  Dam you all to hell!");
        return TRUE;
    case 7:
        do_say(ch,"I choose to waive that right...");
        do_emote(ch,"screams a girlish scream!");
        return TRUE;
    case 8:
        do_say(ch, "You mean I shaved my bikini zone for nothing?!?");
        return TRUE;
    case 9:
        do_say(ch, "hehehe, where's the beef?");
        return TRUE;
    case 10:
        {
#if 0
            CHAR_DATA *tmp_ch;
            char buf[80];

            tmp_ch = (struct char_data *)FindAnyVictim(ch);
            if (!IS_NPC(ch))
            {
                snprintf(buf, 79, "mmmmm..... %s rations.....", PERS(tmp_ch,ch));
                do_say(ch, buf);
                return TRUE;
            } else
#endif
                return FALSE;
        }
    case 11:
        if (!number_range(0,20))
            do_gossip(ch, "NNNNNnnnnoooooOOOoooooo!!!");
        return TRUE;
    case 12:
        do_say(ch, "mmmmm.....  The Erotic Adventures of Homerclees.....");
        return TRUE;
    case 13:
        do_say(ch, "Awww...  I wore my extra loose pants for nothing...");
        return TRUE;
    case 14:
        do_say(ch, "How is education supposed to make me smarter?");
        return TRUE;
    case 15:
        do_say(ch, "English, who needs that?");
        return TRUE;
    case 16:
        do_say(ch, "Must destroy mankind...");
        do_emote(ch,"gets distracted.");
        do_say(ch, "mmmmm..... donuts.....");
        return TRUE;
    case 17:
        do_say(ch, "Is there any frontal nudity?");
        return TRUE;
    case 18:
        do_say(ch, "mmmmm..... the girls of the internet.....");
        return TRUE;
    case 19:
        do_say(ch, "You want the truth?  You can't handle the truth!  Cause when you put your hand in a puddle of goo, and you realize it's your partner's face, you can't handle the truth!");
        return TRUE;
    case 20:
        do_say(ch, "Marge... The bee bit my bottom...");
        return TRUE;
    case 21:
        do_emote(ch,"sighs.");
        do_say(ch, "My gastronomic capacity knows no satiety.");
        return TRUE;
    case 22:
        do_say(ch, "Inanimate huh?  I'll show him inanimate!");
        do_emote(ch, "freezes.");
        return TRUE;
    case 23:
        do_say(ch, "I'm the plowingest guy in the USA!");
        return TRUE;
    case 24:
        do_say(ch, "Me loose brain? uh oh.");
        return TRUE;
        break;
    case 25:
        do_say(ch, "Ooo, I'd sell my soul for a donut.");
        return TRUE;
        break;
    case 26:
        if (number_range(0,10)==0)
            do_gossip(ch, "I am so smart.  S-M-R-T");
        else
            do_say(ch, "I am so smart.  S-M-R-T");
        return TRUE;
    case 27:
        do_emote(ch, "begins to sing to the Flinstones theme.");
        do_say(ch, "Simpson, Homer Simpson, he's the greatest guy in history!");
        do_say(ch, "From the, town of Springfield, he's about to hit a chestnut tree!");
        do_emote(ch, "screams a girlish scream.");
        do_emote(ch, "smashes his car into a chestnut tree.");
        return TRUE;
    case 28:
        if (!number_range(0,10))
            do_gossip(ch, "Why you little!!!");
        else
            do_say(ch, "Why you little!!!");
        do_emote(ch, "begins choking Bart.");
        return TRUE;
    case 29:
        do_emote(ch, "points at you!");
        do_say(ch, "NEEEEEEERRRRRRRRDDDDDDDDD!!!!");
        return TRUE;
    case 30:
        do_say(ch, "Donuts, is there any thing they can't do?");
        return TRUE;
    case 31:
        do_say(ch, "You're the suckiest bunch of sucks that ever sucked.");
        return TRUE;
    case 32:
        if (number_range(0,5)==1)
            do_gossip(ch, "Television.  Teacher.  Mother.  Secret Lover.");
        else
            do_say(ch, "Television.  Teacher.  Mother.  Secret Lover.");
        return TRUE;
    case 33:
        if (number_range(0,10)==1)
            do_gossip(ch, "Can't talk, *I* have a class to teach!");
        else
            do_say(ch, "Can't talk, *I* have a class to teach!");
        return TRUE;
    case 34:
        do_say(ch, "The Homer broadcasting system is on the air!");
        return TRUE;
    case 35:
        do_say(ch, "I'm an obese man trapped in a fat man's body.");
        return TRUE;
    case 36:
        do_say(ch, "But Marge!  It works on any Ayatollah!");
        return TRUE;
    case 37:
        do_say(ch, "Let the bears pay the bear tax.  I pay the homer tax.");
        return TRUE;
    case 38:
        do_say(ch, "Sweet merciful crap!");
        return TRUE;
    case 39:
        do_say(ch, "If it's brown, drink it down, if it's black send it back!");
        return TRUE;
    case 40:
        do_say(ch, "It's hip to be square.");
        return TRUE;
    case 41:
        if (number_range(0,10)==1) {
            do_gossip(ch, "Doh! Nuts.");
            return TRUE;
        }
        break;
    case 42:
        if (number_range(0,10)==1) {
            do_gossip(ch, "Ohuhuhuh, John Denver.");
            return TRUE;
        }
        break;
    case 43:
        do_say(ch, "Stupid President!");
        return TRUE;
    case 44:
        do_say(ch, "Now excuse me while I kiss the sky.");
        return TRUE;
    case 45:
        if (!number_range(0,30))
            do_shout(ch, "Nacho Nacho Man!  I want to be a nacho man!");
        return TRUE;
    case 46:
        do_say(ch, "ewwwww...  dog water....");
        return TRUE;
    case 47:
        if (number_range(0,40) == 0) {
            do_shout(ch, "Oh, a graduate student...  huh?");
            return TRUE;
        }
        break;
    case 48:
        do_say(ch, "Hey, there's a NEW Mexico.");
        return TRUE;
    case 49:
        do_say(ch, "Quiet!  I can't hear myself think!");
        return TRUE;
    case 50:
        do_say(ch, "Where, I can't see!");
        do_emote(ch, "runs in circles trying to read the back of his head.");
        return TRUE;
    case 51:
        do_emote(ch, "wiggles his butt.");
        return TRUE;
    case 52:
        if (!number_range(0,20))
            do_gossip(ch, "A woman is a lot like a beer, once you drink one, you want another... and another...");
        return TRUE;
    case 53:
        do_say(ch, "No TV and no beer make Homer crazy.");
        return TRUE;
    case 54:
        do_say(ch,"This is not happening... This is not happening...");
        if (!number_range(0,40))
            do_shout(ch, "NOOOOOOOOOO!!!");
        return TRUE;
    case 55:
        if (!number_range(0,15))
            do_gossip(ch, "Marge!  Where did I put my car keys?");
        return TRUE;
    case 56:
        if (!number_range(0,15))
            do_gossip(ch, "Shut up Flanders!");
        return TRUE;
    case 57:
        do_say(ch, "Stupid Flanders...");
        return TRUE;
    case 58:
        if (!number_range(0,30))
            do_shout(ch, "WOOHOO!");
        return TRUE;
    case 59:
        do_say(ch, "I dunno...");
        return TRUE;
    case 60:
        do_say(ch, "With twenty dollars, you can buy many peanuts. Really? Explain how!?");
    case 61:
        do_say(ch, "Money can be exchanged for goods and services!  Woohoo!");
        return TRUE;
    case 62:
        do_say(ch, "My baloney has a first name, it's H-O-M-E-R!");
        do_say(ch, "My baloney has a second name, it's H-O-M-E-R!");
        return TRUE;
    case 63:
        do_emote(ch, "offers you a donut.");
        do_say(ch, "It has purple stuff in it.  Purple is a fruit.");
        return TRUE;
    case 64:
        do_say(ch, "Rock stars.  Is there anything they don't know?");
        return TRUE;
    case 65:
        do_say(ch, "mmmmm..... sacrilicious....");
        return TRUE;
    case 66:
        do_say(ch, "Professional athletes...  Always wanting more.");
        return TRUE;
    case 67:
        do_say(ch,"If this was a more perfect world, we'd all be known and the Flimpsons.");
        return TRUE;
    case 68:
        do_say(ch,"I've come back to the time when dinosaurs wern't just confined to zoos.");
        return TRUE;
    case 69:
        do_say(ch,"Stupid cheap weatherstriping!");
        return TRUE;
    case 70:
        do_say(ch,"mmmmm.....  foot-long chili dog.....");
        return TRUE;
    case 80:
        do_say(ch,"Vampires are make-believe, like elves, gremlins and eskimos!");
        return TRUE;
    case 81:
        do_say(ch,"mmmmm.....  open glove golf-wedge.....");
        return TRUE;
    case 82:
        do_say(ch,"I'm in a place where I don't know where I am.");
        return TRUE;
    case 83:
        do_say(ch,"mmmmm.....  homemade prozac...... needs more ice-cream....");
        return TRUE;
    default:
        return FALSE;
    }

    return FALSE;
}

SPECIAL_FUNC(spec_lich_church)
{
    if (number_percent() < 50)
        return spec_mage(proc,cmd,arg,cmd_ch,type);
    return spec_shadow(proc,cmd,arg,cmd_ch,type);
}


SPECIAL_FUNC(spec_dogpack)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_andy_wilcox)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
       do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_eric_johnson)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_GameGuard)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_GreyParamedic)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_AmberParamedic)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_replicant)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_Tytan)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_AbbarachDragon)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_RustMonster)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_temple_labrynth_liar)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_temple_labrynth_sentry)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_zombie_master)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_delivery_elf)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_delivery_beast)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_Keftab)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_StormGiant)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_NewThalosMayor)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_Tyrannosaurus_swallower)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_lattimore)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_trapper)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_trogcook)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_shaman)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_golgar)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_ghostsoldier)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_Valik)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_guardian)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_web_slinger)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_snake_avt)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_virgin_sac)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_PrisonGuard)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_acid_monster)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_magic_user_imp)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_snake_guardians)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_death_knight)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_DogCatcher)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_raven_iron_golem)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_druid_protector)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_DruidAttackSpells)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_Summoner)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_Samah)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_MakeQuest)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_AbyssGateKeeper)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_creeping_death)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_BreathWeapon)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_sailor)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_Devil)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_Demon)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_DruidChallenger)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_MonkChallenger)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_attack_rats)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_DragonHunterLeader)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_HuntingMercenary)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_DwarvenMiners)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_real_rabbit)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_real_fox)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_archer_instructor)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_archer)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_Beholder)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_Slavalis)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_AcidBlob)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_lizardman_shaman)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_village_princess)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_strahd_vampire)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_strahd_zombie)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_banshee)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_baby_bear)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_timnus)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_winger)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_Barbarian)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_goblin_sentry)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_medusa)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_Asmodeus)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_horsetele)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_hpriest)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_archbishop_gpyr2)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_refresher)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_horsemen_welcomer)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_sargust_pub)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_EnchanterGuy)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}

SPECIAL_FUNC(spec_lottery_man)
{
    CHAR_DATA *ch = (CHAR_DATA *)proc;

    if (number_bits(10)==1)
        do_say(ch,"I've got an unfinished mob proc!");

    return FALSE;
}
