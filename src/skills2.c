/******************************************************
          Desolation of the Dragon MUD II
          (C) 1997-2002  Jesse DeFer and Heath Leach
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*static char rcsid[] = "$Id: skills2.c,v 1.53 2004/04/06 22:00:11 dotd Exp $";*/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "mud.h"
#include "gsn.h"

DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_flee);

DECLARE_SPELL_FUN(spell_blindness);
DECLARE_SPELL_FUN(spell_strength);
DECLARE_SPELL_FUN(spell_refresh);
DECLARE_SPELL_FUN(spell_cure_poison);
DECLARE_SPELL_FUN(spell_disintegrate);
DECLARE_SPELL_FUN(spell_teleport);

/* from magic.c */
void failed_casting( SKILLTYPE *skill, CHAR_DATA *ch,
                     CHAR_DATA *victim, OBJ_DATA *obj );

void successful_casting( SKILLTYPE *skill, CHAR_DATA *ch,
                     CHAR_DATA *victim, OBJ_DATA *obj );

int ris_save( CHAR_DATA *ch, int percent, int ris );


void do_blast( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int potence=0,level,dam=0;
    AFFECT_DATA af;

    if (is_affected(ch, gsn_mindwipe) ||
        is_affected(ch, gsn_feeblemind))
    {
        send_to_char("Der, what is that?\n\r", ch);
        return;
    }

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
        if ( argument[0] == '\0'
             ||  (victim=get_char_room(ch, argument)) == NULL )
        {
            send_to_char("Exactly whom did you wish to blast?\n\r",ch);
            return;
        }
    }

    if (victim==ch)    {
        send_to_char ("Blast yourself? Your mother would be sad!\n\r",ch);
        return;
    }

    if (IS_IMMORTAL(victim)) {
        send_to_char ("They ignore your attempt at humor!\n\r",ch);
        return;
    }

    if (GET_MANA(ch)<25 && !IS_IMMORTAL(ch))   {
        send_to_char("Your mind is not up to the challenge at the moment.\n\r",ch);
        return;
    }

    if ( is_affected( victim, gsn_tower_of_iron_will ) )
    {
        act( AT_MAGIC, "$N's psionic attack is ignored by $n!", ch, NULL, victim, TO_NOTVICT );
        act( AT_MAGIC, "$N's psionic protections shield against your attack!",ch, NULL, victim, TO_CHAR );
        act( AT_MAGIC, "Your psionic protections protect you from $n's attack!",ch, NULL, victim, TO_VICT );
        global_retcode = damage( ch, victim, 0, gsn_blast );
        return;
    }

    if ( IS_NPC(ch)
         || number_percent( ) < LEARNED(ch, gsn_blast) )
    {
        act(AT_MAGIC, "$n focuses $s mind on $N's mind.", ch, NULL, victim, TO_ROOM);
        act(AT_MAGIC, "$n blasts your mind with psychic energy.", ch, NULL, victim, TO_VICT);
        act(AT_MAGIC,"You blast $N's mind with a psionic blast of energy!", ch, NULL, victim, TO_CHAR);
        learn_from_success( ch, gsn_blast );
    }
    else
    {
        GET_MANA(ch) -= 12;
        send_to_char("You try and focus your energy but it fizzles!\n\r",ch);
        spell_lag(ch, gsn_blast);
        learn_from_failure( ch, gsn_blast );
        global_retcode = damage( ch, victim, 0, gsn_blast );
        return;
    }

    GET_MANA(ch) -= 25;
    level=GET_LEVEL(ch,CLASS_PSIONIST);
    if (level>0)        potence+=2; /* 2 */
    if (level>4)        potence++;  /* 3 */
    if (level>7)        potence++;  /* 4 */
    if (level>10)       potence++;  /* 5 */
    if (level>20)       potence+=2; /* 7 */
    if (level>30)       potence+=2; /* 9 */
    if (level>40)       potence+=2; /* 11 */
    if (level>49)       potence++;  /* 12 */
    if (level>50)       potence+=2; /* 14 */

    if (number_range(1,LEVEL_HERO) < GetMaxLevel(victim))
        potence--;

    if (potence < 0)
        potence = 0;

    if (potence > 14)
        potence = 14;

    switch(potence)     {
    case 0:
        /* level 0 */
        dam=1;
        break;
    case 1:
        /* level 1 weak */
        dam=dice(1,4);
        break;
    case 2:
        /* levels 1-4 strong */
        /* levels 5-7 weak   */
        dam=dice(2,5);
        break;
    case 3:
        /* levels 5-7 strong */
        /* levels 8-10 weak  */
        dam=dice(3,4);
        break;
    case 4:
        /* levels 8-10 strong */
        /* levels 11-20 weak  */
        dam=dice(4,4);
        break;
    case 5:
        /* levels 11-20 strong */
        dam=20;
        if (!IS_AFFECTED(ch,AFF_BLIND))         {
            af.type = gsn_blast;
            af.duration = 5 * DUR_CONV;
            af.modifier = -4;
            af.location = APPLY_HITROLL;
            af.bitvector = AFF_BLIND;
            affect_to_char(victim, &af);
            af.location = APPLY_AC;
            af.modifier = 20;
            affect_to_char(victim, &af);
        }
        break;
    case 6:
        /* levels 21-30 weak */
        dam=20;
        break;
    case 7:
        /* levels 21-30 strong */
        dam=35;
        if (!IS_AFFECTED(ch,AFF_BLIND))         {
            af.type = gsn_blast;
            af.duration = 5 * DUR_CONV;
            af.modifier = -4;
            af.location = APPLY_HITROLL;
            af.bitvector = AFF_BLIND;
            affect_to_char(victim, &af);
            af.location = APPLY_AC;
            af.modifier = 20;
            affect_to_char(victim, &af);
        }
        if (GET_POS(victim)>POS_STUNNED)
            GET_POS(victim)=POS_STUNNED;
        break;
    case 8:
        /* levels 31-40 weak */
        dam=50;
        break;
    case 9:
        /* levels 31-40 strong */
        dam=70;
        if (GET_POS(victim)>POS_STUNNED)
            GET_POS(victim)=POS_STUNNED;
        if (GET_HITROLL(victim)>-50)         {
            af.type = gsn_blast;
            af.duration = 5 * DUR_CONV;
            af.modifier = -5;
            af.location = APPLY_HITROLL;
            af.bitvector = 0;
            affect_join(victim, &af);
        }
        break;
    case 10:
        /* levels 41-49 weak */
        dam=75;
        break;
    case 11:
        /* levels 41-49 strong */
        /* level 50 weak */
        dam=100;
        if (GET_POS(victim)>POS_STUNNED)
            GET_POS(victim)=POS_STUNNED;
        if (GET_HITROLL(victim)>-50)         {
            af.type = gsn_blast;
            af.duration = 5 * DUR_CONV;
            af.modifier = -10;
            af.location = APPLY_HITROLL;
            af.bitvector = 0;
            affect_join(victim, &af);
        }
        break;
    case 12:
        /* level 50 strong */
        dam=150;
        if (GET_POS(victim)>POS_STUNNED)
            GET_POS(victim)=POS_STUNNED;
        if ((!IS_SET(victim->immune,RIS_HOLD)) &&
            (!IS_AFFECTED(victim,AFF_PARALYSIS))) {
            af.type = gsn_blast;
            af.duration = level * DUR_CONV;
            af.modifier = 0;
            af.location = APPLY_NONE;
            af.bitvector = AFF_PARALYSIS;
            affect_join(victim, &af);
        }
        break;
    case 13:
    case 14:
       /* levels 51+ weak */
       /* levels 51+ strong */
        dam=200;
        if (GET_POS(victim)>POS_STUNNED)
            GET_POS(victim)=POS_STUNNED;
        if ((!IS_SET(victim->immune,RIS_HOLD)) &&
            (!IS_AFFECTED(victim,AFF_PARALYSIS))) {
            af.type = gsn_blast;
            af.duration = 100 * DUR_CONV;
            af.modifier = 0;
            af.location = APPLY_NONE;
            af.bitvector = AFF_PARALYSIS;
            affect_join(victim, &af);
        }
        send_to_char("Your brain is turned to jelly!\n\r",victim);
        act(AT_MAGIC,"You turn $N's brain to jelly!",ch,0,victim,TO_CHAR);
        break;
    }

    global_retcode = damage( ch, victim, dam, gsn_blast );

    spell_lag(ch, gsn_blast);
}

void do_ultra_blast( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch, *vch_next;
    int dam, level;
    int gsn_ultra_blast = skill_lookup("ultrablast");

    if (is_affected(ch, gsn_mindwipe) ||
        is_affected(ch, gsn_feeblemind))
    {
        send_to_char("Der, what is that?\n\r", ch);
        return;
    }

    level = GetMaxLevel(ch);

    act(AT_MAGIC, "You blast out a massive wave of destructive psionic energy!", ch, NULL, NULL, TO_CHAR);
    act(AT_MAGIC, "$n blasts out a massive wave of destructive psionic energy!", ch, NULL, NULL, TO_ROOM);

    for(vch = ch->in_room->first_person; vch; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        dam = dice(level, 4) + level;

        if (vch == ch ||
            is_same_group(ch, vch) ||
            IS_IMMORTAL(vch))
            continue;

        if (saves_spell_staff(GetMaxLevel(ch), vch))
        {
            dam /= 2;
            if (is_affected(vch, gsn_tower_of_iron_will))
                dam = 0;
        }
        else if (is_affected(vch, gsn_tower_of_iron_will))
            dam /= 2;

        damage( ch, vch, dam, gsn_ultra_blast );
    }

    learn_from_success(ch, gsn_ultra_blast);
    spell_lag(ch, gsn_ultra_blast);
}

void do_psiscry( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA *victim;
    SKILLTYPE *skill = get_skilltype(gsn_scry);

    if (is_affected(ch, gsn_mindwipe) ||
        is_affected(ch, gsn_feeblemind))
    {
        send_to_char("Der, what is that?\n\r", ch);
        return;
    }

    spell_lag(ch, gsn_scry);

    if ( (victim = get_char_world( ch, argument )) == NULL ||
         !victim->in_room ||
         IS_SET(victim->in_room->room_flags, ROOM_PRIVATE) ||
         IS_SET(victim->in_room->room_flags, ROOM_SOLITARY) ||
         IS_SET(victim->in_room->room_flags, ROOM_DEATH) ||
         IS_SET(victim->in_room->room_flags, ROOM_PROTOTYPE) ||
         (IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE)) ||
         !(location = victim->in_room) ||
         (!IS_IMMORTAL(ch) && IS_IMMORTAL(victim))
       )
    {
        failed_casting( skill, ch, victim, NULL );
        return;
    }
    if (number_percent() > LEARNED(ch, gsn_scry))
    {
        failed_casting( skill, ch, victim, NULL );
        learn_from_failure(ch, gsn_scry);
        return;
    }

    send_to_char("You close your eyes and envision your target.\n\r", ch);
    if (IS_ACTIVE(victim, CLASS_PSIONIST) &&
        is_affected(victim, gsn_great_sight))
        send_to_char("You feel as though someone is watching you from afar.\n\r", victim);

    successful_casting( skill, ch, victim, NULL );
    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, location );
    do_look( ch, "auto" );
    char_from_room( ch );
    char_to_room( ch, original );
    return;
}

void do_doorway( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    SKILLTYPE *skill = get_skilltype(gsn_doorway);

    if (is_affected(ch, gsn_mindwipe) ||
        is_affected(ch, gsn_feeblemind))
    {
        send_to_char("Der, what is that?\n\r", ch);
        return;
    }

    spell_lag(ch, gsn_doorway);

    if ( (victim = get_char_world( ch, argument )) == NULL )
    {
        send_to_char("You can't sense that person anywhere.\n\r", ch);
        return;
    }

    if ( victim->in_room == ch->in_room )
    {
        send_to_char("They're standing right next to you.\n\r", ch);
        return;
    }

    if ( !victim->in_room ||
         IS_SET(victim->in_room->room_flags, ROOM_PRIVATE) ||
         IS_SET(victim->in_room->room_flags, ROOM_SOLITARY) ||
         IS_SET(victim->in_room->room_flags, ROOM_NO_ASTRAL) ||
         IS_SET(victim->in_room->room_flags, ROOM_DEATH) ||
         IS_SET(victim->in_room->room_flags, ROOM_PROTOTYPE) ||
         IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL) ||
         IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL) ||
         (!IsExtraPlanar(ch) && is_other_plane(ch->in_room, victim->in_room)) ||
         (IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE)) ||
         (!IS_IMMORTAL(ch) && IS_IMMORTAL(victim))
       )
    {
        send_to_char("Your mind is not yet strong enough to attempt such a jump.\n\r", ch);
        return;
    }

    if (IS_SYSTEMFLAG(SYS_NOPORTAL))
    {
        send_to_char("The planes are fuzzy, your doorway goes nowhere.\n\r", ch);
        return;
    }

    if (!IS_NPC(victim) && IS_IMMORTAL(victim) && !IS_IMMORTAL(ch))
    {
	send_to_char("They wouldn't like that much.\n\r", ch);
        if (can_see(victim, ch))
	    ch_printf(victim, "%s just tried dorwaying to you.\n\r", GET_NAME(ch));
        return;
    }

    if (number_percent() > LEARNED(ch, gsn_doorway))
    {
        failed_casting( skill, ch, victim, NULL );
        learn_from_failure(ch, gsn_doorway);
        return;
    }

    successful_casting( skill, ch, victim, NULL );
    char_from_room( ch );
    char_to_room( ch, victim->in_room );
    act( AT_MAGIC, "$n leaps from a portal in the sky.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    return;
}

void do_portal( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    SKILLTYPE *skill = get_skilltype(gsn_psiportal);
    CHAR_DATA *gch, *gch_next;

    if (is_affected(ch, gsn_mindwipe) ||
        is_affected(ch, gsn_feeblemind))
    {
        send_to_char("Der, what is that?\n\r", ch);
        return;
    }

    spell_lag(ch, gsn_psiportal);

    if ( (victim = get_char_world( ch, argument )) == NULL )
    {
        send_to_char("You can't sense that person anywhere.\n\r", ch);
        return;
    }

    if ( victim->in_room == ch->in_room )
    {
        send_to_char("They're standing right next to you.\n\r", ch);
        return;
    }

    if ( !victim->in_room ||
         IS_SET(victim->in_room->room_flags, ROOM_PRIVATE) ||
         IS_SET(victim->in_room->room_flags, ROOM_SOLITARY) ||
         IS_SET(victim->in_room->room_flags, ROOM_NO_ASTRAL) ||
         IS_SET(victim->in_room->room_flags, ROOM_DEATH) ||
         IS_SET(victim->in_room->room_flags, ROOM_PROTOTYPE) ||
         IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL) ||
         IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL) ||
         (!IsExtraPlanar(ch) && is_other_plane(ch->in_room, victim->in_room)) ||
         (IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE)) ||
         (!IS_IMMORTAL(ch) && IS_IMMORTAL(victim)) )
    {
        send_to_char("Your mind is not yet strong enough to attempt such a jump.\n\r", ch);
        return;
    }

    if (IS_SYSTEMFLAG(SYS_NOPORTAL))
    {
        send_to_char("The planes are fuzzy, your portal goes nowhere.\n\r", ch);
        return;
    }

    if (!IS_NPC(victim) && IS_IMMORTAL(victim) && !IS_IMMORTAL(ch))
    {
	send_to_char("They wouldn't like that much.\n\r", ch);
        if (can_see(victim, ch))
	    ch_printf(victim, "%s just tried psiportaling to you.\n\r", GET_NAME(ch));
        return;
    }

    if (number_percent() > LEARNED(ch, gsn_psiportal))
    {
        failed_casting( skill, ch, victim, NULL );
        learn_from_failure(ch, gsn_psiportal);
        return;
    }

    for ( gch = ch->in_room->first_person; gch; gch = gch = gch_next )
    {
        gch_next = gch->next_in_room;

        if ( IS_NPC(gch) && IS_SET (gch->act, ACT_PROTOTYPE) )
            continue;

        if ( is_same_group(ch, gch) && gch->position != POS_FIGHTING )
        {
            act( AT_MAGIC, "$n jumps through a portal and dissappears.", gch, NULL, NULL, TO_ROOM );
            char_from_room(gch);
            char_to_room(gch, victim->in_room);
            act( AT_MAGIC, "$n jumps from a portal in the sky.", gch, NULL, NULL, TO_ROOM );
            do_look( gch, "auto" );
        }
    }
}

void do_tan(CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *forge = NULL;
    OBJ_DATA *corpse = NULL;
    OBJ_DATA *hide = NULL;
    AFFECT_DATA *paf;
    char itemname[MAX_INPUT_LENGTH], itemtype[MAX_INPUT_LENGTH], hidetype[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int percent=0, acapply=0, acbonus=0, lev=0, gsn_forge=0;

    if (IS_NPC(ch) && !IS_ACT_FLAG(ch, ACT_POLYMORPHED))
        return;

    if (ch->mount) {
        send_to_char("Not from this mount you cannot!\n\r",ch);
        return;
    }

    if (!CanUseSkill(ch, gsn_tan)) {
        send_to_char("What do you think you are, A tanner?\n\r",ch);
        return;
    }

    argument = one_argument(argument, itemname);
    argument = one_argument(argument, itemtype);

    if (!*itemname) {
        send_to_char("Tan what?\n\r",ch);
        return;
    }

    if (!*itemtype) {
        send_to_char("I see that, but what do you wanna make?\n\r",ch);
        return;
    }
    gsn_forge = skill_lookup("forge");

    if (!(corpse = get_obj_here(ch, itemname)))
    {
        if (CanUseSkill(ch, gsn_forge))
        {
            if (!(corpse = get_obj_carry(ch, itemname)))
            {
                send_to_char("You need materials of some kind!\n\r", ch);
                return;
            }
            else send_to_char("You combine your tanning and forging skills!\n\r", ch);
        } else {
            send_to_char("Where did that carcuss go?\n\r",ch);
            return;
        }
    }

    if (corpse->item_type != ITEM_CORPSE_PC && corpse->item_type != ITEM_CORPSE_NPC)
    {
        if (CanUseSkill(ch, gsn_forge))
        {
            if (!(forge = get_obj_here(ch, "forge")))
            {
                send_to_char("You need a forge to complete the process.\n\r",ch);
                return;
            } else if (forge->item_type != ITEM_FORGE)
            {
                send_to_char("This forge is inoperable!\n\r",ch);
                return;
            }

            if (corpse->item_type != ITEM_MATERIAL)
            {
                send_to_char("Those materials ar insuficient.\n\r", ch);
                return;
            }
        } else {
            send_to_char("That is not a corpse, you cannot tan it.\n\r", ch);
            return;
        }
    }

    /* rent==0 means you can carve corpse */
    if ((corpse->item_type != ITEM_MATERIAL) &&
        corpse->rent > 0 && !IS_IMMORTAL(ch))
    {
        send_to_char("Sorry, nothing left of the carcuss to make a item with.\n\r",ch);
        return;
    }

    if (corpse->item_type == ITEM_MATERIAL)
        if (MTYPE(corpse) == MAT_WORTHLESS ||
            MGRADE(corpse) == QUAL_WORTHLESS)
        {
            send_to_char("These materials are just scraps, not enough for an attmpt.\n\r", ch);
            return;
        }

    percent = number_percent();

    if (percent > LEARNED(ch, gsn_tan))
    {
        if (corpse->item_type == ITEM_MATERIAL)
        {
            MGRADE(corpse)--;
            sprintf(buf,"You hack at %s but manage to only weaken the temper.\n\r",
                    corpse->short_descr);
            send_to_char(buf,ch);

            sprintf(buf,"$n tries to forge %s into armor, but fails.",
                    corpse->short_descr);
            act(AT_SKILL, buf, ch, NULL, NULL, TO_ROOM);
        } else {

            corpse->rent++;

            sprintf(buf,"You hack at %s but manage to only destroy the hide.\n\r",
                    corpse->short_descr);
            send_to_char(buf,ch);

            sprintf(buf,"$n tries to skins %s for it's hide, but destroys it.",
                    corpse->short_descr);
            act(AT_SKILL, buf, ch, NULL, NULL, TO_ROOM);
        }
        learn_from_failure(ch, gsn_tan);
        spell_lag(ch, gsn_tan);
        return;
    }

    lev = (corpse->item_type == ITEM_MATERIAL) ?
        GetClassLevel(ch, CLASS_ARTIFICER) : corpse->value[5];

    if (corpse->item_type == ITEM_MATERIAL)
    {
        sprintf(hidetype, "%s", mat_name[MTYPE(corpse)]);
        acapply += mat_acplus[MTYPE(corpse)];
        acbonus += mat_acplus[MTYPE(corpse)];
    }
    else
    {
        switch(corpse->value[4])
        {
        case RACE_HALFBREED :
            sprintf(hidetype,"spotted leather");
            break;
        case RACE_HUMAN     :
            sprintf(hidetype,"human leather");
            lev=(int)lev/2;
            break;
        case RACE_ELVEN     :
            sprintf(hidetype,"elf hide");
            lev=(int)lev/2;
            break;
        case RACE_DWARF     :
            sprintf(hidetype,"dwarf hide");
            lev=(int)lev/2;
            break;
        case RACE_HALFLING  :
            sprintf(hidetype,"halfing hide");
            lev=(int)lev/2;
            break;
        case RACE_GNOME     :
            sprintf(hidetype,"gnome hide");
            lev=(int)lev/2;
            break;
        case RACE_REPTILE  :
            sprintf(hidetype,"reptile hide");
            break;
        case RACE_SPECIAL  :
        case RACE_LYCANTH  :
            sprintf(hidetype,"leather");
            break;
        case RACE_DRAGON   :
            sprintf(hidetype,"dragon hide");
            break;
        case RACE_UNDEAD   :
        case RACE_UNDEAD_VAMPIRE :
        case RACE_UNDEAD_LICH    :
        case RACE_UNDEAD_WIGHT   :
        case RACE_UNDEAD_GHAST   :
        case RACE_UNDEAD_SPECTRE :
        case RACE_UNDEAD_ZOMBIE  :
        case RACE_UNDEAD_SKELETON :
        case RACE_UNDEAD_GHOUL    :
            sprintf(hidetype,"rotting hide");
            lev=(int)lev/2;
            break;
        case RACE_ORC      :
            sprintf(hidetype,"orc hide");
            lev=(int)lev/2;
            break;
        case RACE_INSECT   :
            sprintf(hidetype,"insectiod hide");
            break;
        case RACE_ARACHNID :
            sprintf(hidetype,"hairy leather");
            lev=(int)lev/2;
            break;
        case RACE_DINOSAUR :
            sprintf(hidetype,"thick leather");
            break;
        case RACE_FISH     :
            sprintf(hidetype,"fishy hide");
            break;
        case RACE_BIRD     :
            sprintf(hidetype,"feathery hide");
            lev=(int)lev/2;
            break;
        case RACE_GIANT    :
            sprintf(hidetype,"giantish hide");
            break;
        case RACE_PREDATOR :
            sprintf(hidetype,"leather");
            break;
        case RACE_PARASITE :
            sprintf(hidetype,"celulite");
            break;
        case RACE_SLIME    :
            sprintf(hidetype,"jelly");
            lev=(int)lev/2;
            break;
        case RACE_DEMON    :
            sprintf(hidetype,"demon hide");
            break;
        case RACE_SNAKE    :
            sprintf(hidetype,"snake hide");
            break;
        case RACE_HERBIV   :
            sprintf(hidetype,"leather");
            break;
        case RACE_TREE     :
            sprintf(hidetype,"bark hide");
            break;
        case RACE_VEGGIE   :
            sprintf(hidetype,"green hide");
            break;
        case RACE_ELEMENT  :
            sprintf(hidetype,"pure hide");
            break;
        case RACE_PLANAR   :
            sprintf(hidetype,"astralion");
            break;
        case RACE_DEVIL    :
            sprintf(hidetype,"devil hide");
            break;
        case RACE_GHOST    :
            sprintf(hidetype,"ghostly hide");
            break;

        case RACE_GOBLIN   :
            sprintf(hidetype,"goblin hide");
            lev=(int)lev/2;
            break;
        case RACE_TROLL    :
            sprintf(hidetype,"troll leather");
            break;
        case RACE_VEGMAN   :
            sprintf(hidetype,"green hide");
            break;
        case RACE_MFLAYER  :
            sprintf(hidetype,"mindflayer hide");
            break;
        case RACE_PRIMATE  :
            sprintf(hidetype,"leather");
            break;
        case RACE_ENFAN    :
            sprintf(hidetype,"enfan hide");
            lev=(int)lev/2;
            break;
        case RACE_DROW     :
            sprintf(hidetype,"drow hide");
            lev=(int)lev/2;
            break;
        case RACE_GOLEM    :
        case RACE_SKEXIE   :
        case RACE_TROGMAN  :
        case RACE_LIZARDMAN:
        case RACE_PATRYN   :
        case RACE_LABRAT   :
        case RACE_SARTAN   :
            sprintf(hidetype,"leather");
            break;
        case RACE_TYTAN   :
            sprintf(hidetype,"tytan hide");
            break;
        case RACE_SMURF    :
            sprintf(hidetype,"leather");
            break;
        case RACE_ROO      :
            sprintf(hidetype,"roo hide");
            break;
        case RACE_HORSE    :
        case RACE_DRAAGDIM :
            sprintf(hidetype,"leather");
            break;
        case RACE_ASTRAL   :
            sprintf(hidetype,"strange hide");
            break;
        case RACE_GOD      :
            sprintf(hidetype,"titanium");
            break;
        case RACE_GIANT_HILL   :
            sprintf(hidetype,"hill giant hide");
            break;
        case RACE_GIANT_FROST  :
            sprintf(hidetype,"frost giant hide");
            break;
        case RACE_GIANT_FIRE   :
            sprintf(hidetype,"fire giant hide");
            break;
        case RACE_GIANT_CLOUD  :
            sprintf(hidetype,"cloud giant hide");
            break;
        case RACE_GIANT_STORM  :
            sprintf(hidetype,"storm giant hide");
            break;
        case RACE_GIANT_STONE  :
            sprintf(hidetype,"stone giant hide");
            acapply++;
            acbonus++;
            break;
        case RACE_DRAGON_RED   :
            sprintf(hidetype,"red dragon hide");
            acapply+=2;
            acbonus+=3;
            break;
        case RACE_DRAGON_BLACK :
            sprintf(hidetype,"black dragon hide");
            acapply++;
            acbonus++;
            break;
        case RACE_DRAGON_GREEN :
            sprintf(hidetype,"green dragon hide");
            acapply++;
            acbonus++;
            break;
        case RACE_DRAGON_WHITE :
            sprintf(hidetype,"white dragon hide");
            acapply++;
            acbonus++;
            break;
        case RACE_DRAGON_BLUE  :
            sprintf(hidetype,"blue dragon hide");
            acapply++;
            acbonus++;
            break;
        case RACE_DRAGON_SILVER:
            sprintf(hidetype,"silver dragon hide");
            acapply+=2;
            acbonus+=2;
            break;
        case RACE_DRAGON_GOLD  :
            sprintf(hidetype,"gold dragon hide");
            acapply+=2;
            acbonus+=3;
            break;
        case RACE_DRAGON_BRONZE:
            sprintf(hidetype,"bronze dragon hide");
            acapply++;
            acbonus+=2;
            break;
        case RACE_DRAGON_COPPER:
            sprintf(hidetype,"copper dragon hide");
            acapply++;
            acbonus+=2;
            break;
        case RACE_DRAGON_BRASS :
            sprintf(hidetype,"brass dragon hide");
            acapply++;
            acbonus+=2;
            break;
        default:
            sprintf(hidetype,"leather");
            break;

        }
    }

    acbonus+=(int)lev/10;
    acapply+=(int)lev/10;


    if (IS_ACTIVE(ch,CLASS_RANGER))
        acbonus+=1;

    acbonus = URANGE(0, acbonus, 6);
    acapply = URANGE(0, acapply, 6);

    if (!str_cmp(itemtype,"shield"))
    {
        hide = create_object(OBJ_VNUM_TAN_SHIELD);
        acapply++;
        acbonus++;
        strcat(hidetype," shield");
    }
    else if (!str_cmp(itemtype,"jacket"))
    {
        hide = create_object(OBJ_VNUM_TAN_JACKET);
        acapply+=5;
        acbonus+=2;
        strcat(hidetype," jacket");
    }
    else if (!str_cmp(itemtype,"boots"))
    {
        hide = create_object(OBJ_VNUM_TAN_BOOTS);
        acapply--;
        if (acapply <0) acapply=0;
        acbonus--;
        if (acbonus <0) acbonus=0;
        strcat(hidetype," pair of boots");
    }
    else if (!str_cmp(itemtype,"gloves"))
    {
        hide = create_object(OBJ_VNUM_TAN_GLOVES);
        acapply--;
        if (acapply<0) acapply=0;
        acbonus--;
        if (acbonus<0) acbonus=0;
        strcat(hidetype," pair of gloves");
    }
    else if (!str_cmp(itemtype,"leggings"))
    {
        hide = create_object(OBJ_VNUM_TAN_LEGGINGS);
        acapply++;
        acbonus++;
        strcat(hidetype," set of leggings");
    }
    else if (!str_cmp(itemtype,"sleeves"))
    {
        hide = create_object(OBJ_VNUM_TAN_SLEEVES);
        acapply++;
        acbonus++;
        strcat(hidetype," set of sleeves");
    }
    else  if (!str_cmp(itemtype,"helmet"))
    {
        hide = create_object(OBJ_VNUM_TAN_HELMET);
        acapply--;
        if (acapply<0) acapply=0;
        acbonus--;
        if (acbonus<0) acbonus=0;
        strcat(hidetype," helmet");
    }
    else  if (!str_cmp(itemtype,"bag"))
    {
        hide = create_object(OBJ_VNUM_TAN_BAG);
        strcat(hidetype," bag");
    }  else  {
        send_to_char("Illegal type of equipment!\n\r",ch);
        return;
    }
    if (!hide)
    {
        bug("Tan objects missing.");
        send_to_char("You messed up the hide and it's useless.\n\r", ch);
        return;
    }
    obj_to_char(hide, ch);

    sprintf( buf, "%s", hidetype );
    STRFREE(hide->name);
    hide->name = STRALLOC(buf);

    sprintf( buf, "a %s", hidetype );
    STRFREE(hide->short_descr);
    hide->short_descr = STRALLOC(buf);

    sprintf( buf, "A %s lies here.", hidetype );
    STRFREE(hide->description);
    hide->description = STRALLOC(buf);

    if (str_cmp(itemtype,"bag"))
    {
        hide->value[0] = acapply;
        hide->value[1] = (int)(lev/10);

        /* add in AC bonus here */
        CREATE( paf, AFFECT_DATA, 1 );
        paf->type		= gsn_tan;
        paf->duration		= -1;
        paf->location		= APPLY_AC;
        paf->modifier		= -acbonus;
        paf->bitvector		= 0;
        LINK( paf, hide->first_affect, hide->last_affect, next, prev );

        /*
         sprintf(buf,"%s aff1 %d 17",itemtype,0-acbonus);
         do_ooedit(ch,buf,0);	 */
    } 		/* was not a bag ^ */

    if (corpse->item_type == ITEM_MATERIAL)
    { MGRADE(corpse) = 0; } else { corpse->rent++; }

    sprintf(buf,"You hack at the %s and finally make the %s.\n\r",
            corpse->short_descr, itemtype);
    send_to_char(buf,ch);

    sprintf(buf,"$n skins %s for it's hide.",
            corpse->short_descr);
    act(AT_SKILL, buf, ch, NULL, NULL, TO_ROOM);

    spell_lag(ch, gsn_tan);
    return;
}

void do_canibalize(CHAR_DATA *ch, char *argument )
{
    long hit_points,mana_points;
    int sn = gsn_canibalize;
    int percent;

    if (is_affected(ch, gsn_mindwipe) ||
        is_affected(ch, gsn_feeblemind))
    {
        send_to_char("Der, what is that?\n\r", ch);
        return;
    }

    hit_points = atoi(argument);

    if ((hit_points <1) || (hit_points > 65535)) {
        send_to_char("Invalid number to canibalize.\n\r",ch);
        return;
    }

    mana_points = (hit_points * 2);

    if (GET_HIT(ch) < (hit_points+5))    {
        send_to_char ("You don't have enough physical stamina to canibalize.\n\r",ch);
        return;
    }

    if ( (GET_MANA(ch)+mana_points) > (GET_MAX_MANA(ch)) )    {
        send_to_char ("Your mind cannot handle that much extra energy.\n\r",ch);
        return;
    }

    percent = number_percent();

    if (percent > LEARNED(ch, sn) )    {
        send_to_char ("You try to canibalize your stamina but the energy escapes before you can harness it.\n\r",ch);
        act(AT_DAMAGE,"$n yelps in pain.",ch,NULL,NULL,TO_ROOM);
        GET_HIT(ch) -= hit_points;
        learn_from_failure(ch, sn);
        WAIT_STATE(ch, PULSE_VIOLENCE*3);
        return;
    }

    send_to_char ("You sucessfully convert your stamina to Mental power.\n\r",ch);
    act(AT_DAMAGE,"$n briefly is surrounded by a red aura.",ch,NULL,NULL,TO_ROOM);
    GET_HIT(ch) -= hit_points;
    GET_MANA(ch) += mana_points;

    WAIT_STATE(ch, PULSE_VIOLENCE*3);
}

void do_quivering_palm(CHAR_DATA *ch, char *argument)
{
    AFFECT_DATA paf;
    CHAR_DATA *victim;
    SKILLTYPE *skill = get_skilltype(gsn_quivering_palm);

    if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    {
        ch_printf(ch, "You can't find a %s anywhere!\n\r", argument);
        return;
    }

    if (is_affected(ch, gsn_quivering_palm))
    {
        send_to_char("You can only do this once per week\n\r", ch);
        return;
    }

    if (ch == victim)
    {
        ch_printf(ch, "Okay... You kill yourself, duh!\n\r");
        return;
    }

    /*
     if (ch->attackers > 3) {
     send_to_char("There's no room to use that skill!\n\r",ch);
     return;
     }

     if (victim->attackers >= 3) {
     send_to_char("You can't get close enough\n\r", ch);
     return;
     }*/

    if (!IsHumanoid(victim) )
    {
        send_to_char("You can only do this to humanoid opponents.\n\r", ch);
        return;
    }

    send_to_char("You begin to work on the vibrations.\n\r", ch);

    if ((GetMaxLevel(victim) >= GetMaxLevel(ch))
        || GET_MAX_HIT(victim) > (GET_MAX_HIT(ch)*2))
    {
        failed_casting( skill, ch, victim, NULL );
        damage(ch, victim, 0, gsn_quivering_palm);
        WAIT_STATE(ch, PULSE_VIOLENCE*3);
        return;
    }

    if (HitOrMiss(ch, victim, CalcThaco(ch)))
    {
        learn_from_success(ch, gsn_quivering_palm);
        successful_casting( skill, ch, victim, NULL );
        damage(ch, victim, GET_MAX_HIT(victim)*5, gsn_quivering_palm);
    }
    else
    {
        failed_casting( skill, ch, victim, NULL );
        learn_from_failure(ch, gsn_quivering_palm);
    }

    spell_lag(ch, gsn_quivering_palm);

    paf.type      = gsn_quivering_palm;
    paf.duration  = (int)(24*DUR_CONV);
    paf.location  = APPLY_NONE;
    paf.modifier  = 0;
    paf.bitvector = 0;
    affect_to_char(ch, &paf);
}

void do_hypnotize(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vict;
    AFFECT_DATA paf;
    int gsn_hypno = skill_lookup("hypnotize"), saves;

    if (is_affected(ch, gsn_mindwipe) ||
        is_affected(ch, gsn_feeblemind))
    {
        send_to_char("Der, what is that?\n\r", ch);
        return;
    }

    if (!CanUseSkill(ch, gsn_hypno))
    {
        send_to_char("What do you think you are, a hypnotist?\n\r", ch);
        return;
    }

    vict = get_char_room(ch, argument);
    if (!vict)
    {
        send_to_char("Hrmmm.... They seem to have left!\n\r", ch);
        return;
    }
    if (vict == ch)
    {
        send_to_char("You obey your every command!\n\r", ch);
        return;
    }
    if (GET_MANA(ch) < 25)
    {
        send_to_char("Your mind needs some rest first.\n\r", ch);
        return;
    }
    if (number_percent() > LEARNED(ch, gsn_hypno))
    {
        send_to_char("Your attempt at hypnosis was laughable.\n\r", ch);
        send_to_char("You feel that someone is messing with your mind.\n\r", vict);
        GET_MANA(ch)-=12;
        learn_from_failure(ch, gsn_hypno);
        WAIT_STATE(ch, PULSE_VIOLENCE*3);
        return;
    }
    if (GetMaxLevel(ch) < GetMaxLevel(vict))
    {
        send_to_char("You would probably just get your head bashed in!\n\r", ch);
        GET_MANA(ch)-=12;
        learn_from_failure(ch, gsn_hypno);
        return;
    }

    saves = ris_save( vict, GetMaxLevel(ch), RIS_CHARM );
    log_printf_plus(LOG_MAGIC, GetMaxLevel(ch), SEV_DEBUG,
                    "hypnotize: %s saves %d (%s)", GET_NAME(vict), saves, GET_NAME(ch));

    if ( IS_AFFECTED(vict, AFF_CHARM)
         ||   saves == 1000
         ||   IS_AFFECTED(ch, AFF_CHARM)
         ||   circle_follow( vict, ch )
         ||   saves_spell_staff(saves, vict))
    {
        send_to_char("You could not hypnotize this person.\n\r", ch);
        GET_MANA(ch)-=12;
        learn_from_failure(ch, gsn_hypno);
        start_hating(vict, ch);
        set_fighting(vict, ch);
        return;
    }

    if (IS_NPC(vict))
    {
        REMOVE_BIT(vict->act, ACT_AGGRESSIVE);
        REMOVE_BIT(vict->act, ACT_META_AGGR);
        SET_BIT(vict->act, ACT_SENTINEL);
    }

    if (vict->master)
        stop_follower(ch);
    add_follower(vict, ch);

    paf.type      = gsn_hypno;
    paf.duration  = 36 * DUR_CONV;
    paf.location  = APPLY_NONE;
    paf.modifier  = 0;
    paf.bitvector = AFF_CHARM;
    affect_to_char(vict, &paf);

    WAIT_STATE(ch, PULSE_VIOLENCE*3);
}

void do_blessing(CHAR_DATA *ch, char *argument)
{
    int rating,factor,level;
    CHAR_DATA *test, *victim;
    AFFECT_DATA af;
    int gsn_blessing = skill_lookup("blessing");

    if (!gsn_blessing)
        return;

    if (!CanUseSkill(ch, gsn_blessing))
    {
        send_to_char("You don't know how to do that.\n\r", ch);
        return;
    }

    if (GET_MANA(ch)<GET_LEVEL(ch,CLASS_PALADIN)*2)
    {
        send_to_char("You haven't the spiritual resources to do that now.\n\r",ch);
        return;
    }

    if (is_affected(ch, gsn_blessing))
    {
        send_to_char("You can only request a blessing from your deity once every 3 days.\n\r",ch);
        return;
    }

    if (!(victim=get_char_room(ch,argument)))
    {
        send_to_char ("WHO do you wish to bless?\n\r",ch);
        return;
    }

    if (number_percent() > LEARNED(ch, gsn_blessing))
    {
        send_to_char ("You fail in the bestow your gods blessing.\n\r",ch);
        learn_from_failure(ch, gsn_blessing);
        WAIT_STATE(ch, PULSE_VIOLENCE*2);
        return;
    }

    ch->mana -= GET_LEVEL(ch,CLASS_PALADIN)*2;
    factor=0;
    if (ch==victim)
        factor++;
    if (GET_ALIGN(victim) > 350)
        factor++;
    if (GET_ALIGN(victim) == 1000)
        factor++;
    level = GET_LEVEL(ch,CLASS_PALADIN);
    rating = (int)((level)*(GET_ALIGN(ch))/1000)+factor;
    factor=0;
    for (test=ch->in_room->first_person;test;test=test->next_in_room)
    {
        if (test==ch)
            continue;
        if (is_same_group(ch, test))
            factor++;
    }
    rating += UMIN(factor,3);

    if (rating<0)
    {
        send_to_char("You are so despised by your god that he punishes you!\n\r",ch);
        spell_blindness( gsn_blindness, GET_LEVEL(ch, CLASS_PALADIN), ch, victim );
        spell_smaug( gsn_paralyze, GET_LEVEL(ch, CLASS_PALADIN), ch, victim );
        return;
    }
    if (rating==0)
    {
        send_to_char("There's no one in your group to bless.", ch);
        return;
    }
    if (!is_affected(victim, gsn_bless))
        spell_smaug( gsn_bless, GET_LEVEL(ch, CLASS_PALADIN), ch, victim );

    if (rating>1 && !is_affected(victim, gsn_armor))
        spell_smaug( gsn_armor, GET_LEVEL(ch, CLASS_PALADIN), ch, victim );

    if (rating>4 && !is_affected(victim, gsn_strength))
        spell_strength( gsn_strength, GET_LEVEL(ch, CLASS_PALADIN), ch, victim );

    if (rating>6)
        spell_refresh( gsn_refresh, GET_LEVEL(ch, CLASS_PALADIN), ch, victim );

    if (rating>9 && !is_affected(victim, gsn_sense_life))
        spell_smaug( gsn_sense_life, GET_LEVEL(ch, CLASS_PALADIN), ch, victim );

    if (rating>14 && !is_affected(victim, gsn_true_sight))
        spell_smaug( gsn_true_sight, GET_LEVEL(ch, CLASS_PALADIN), ch, victim );

    if (rating>19)
        spell_smaug( gsn_cure_critical, GET_LEVEL(ch, CLASS_PALADIN), ch, victim );

    if (rating>24 && !is_affected(victim, gsn_sanctuary))
        spell_smaug( gsn_sanctuary, GET_LEVEL(ch, CLASS_PALADIN), ch, victim );

    if (rating>29)
        spell_smaug( gsn_heal, GET_LEVEL(ch, CLASS_PALADIN), ch, victim );

    if (rating>34)
    {
        int gsn_remove_paralysis = skill_lookup("remove paralysis");
        spell_cure_poison( gsn_cure_poison, GET_LEVEL(ch, CLASS_PALADIN), ch, victim );
        spell_smaug( gsn_remove_paralysis, GET_LEVEL(ch, CLASS_PALADIN), ch, victim );
    }

    if (rating>39)
        spell_smaug( gsn_heal, GET_LEVEL(ch, CLASS_PALADIN), ch, victim );

    if (rating>44 && !IS_NPC(victim))
    {
        if (victim->pcdata->condition[COND_FULL] != -1)
            victim->pcdata->condition[COND_FULL] = MAX_COND_VAL;
        if (victim->pcdata->condition[COND_THIRST] != -1)
            victim->pcdata->condition[COND_THIRST] = MAX_COND_VAL;
    }

    if (rating>54)
    {
        spell_smaug( gsn_heal, GET_LEVEL(ch, CLASS_PALADIN), ch, victim );
        send_to_char ("An awesome feeling of holy power overcomes you!\n\r", victim);
    }

    log_printf_plus(LOG_MAGIC, LEVEL_IMMORTAL, SEV_DEBUG, "%s blessing rating is %d", GET_NAME(ch), rating);

    if (ch!=victim)
    {
        act(AT_WHITE, "$n asks $s deity to bless $N!", ch, NULL, victim, TO_NOTVICT);
        act(AT_WHITE, "You pray for a blessing on $N!", ch, NULL, victim, TO_CHAR);
        act(AT_WHITE, "$n's deity blesses you!", ch, NULL, victim, TO_VICT);
    }
    else
        act(AT_WHITE, "$n asks $s deity to bless $m!", ch, NULL, NULL, TO_ROOM);

    af.type      = gsn_blessing;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    af.duration  = 24*3*DUR_CONV;    /* once every three days */
    affect_to_char(ch, &af);

    WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

void do_lay_on_hands(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    int wounds, healing;
    int gsn_lay_on_hands = skill_lookup("lay on hands");

    if (!gsn_lay_on_hands)
        return;

    if (!CanUseSkill(ch, gsn_lay_on_hands))
    {
        send_to_char("You don't know how to do that.\n\r", ch);
        return;
    }

    if (!(victim=get_char_room(ch, argument)))
    {
        send_to_char("Your hands cannot reach that person.\n\r",ch);
        return;
    }

    if (is_affected(ch, gsn_lay_on_hands))
    {
        send_to_char("You have already healed once today.\n\r",ch);
        return;
    }

    wounds=GET_MAX_HIT(victim)-GET_HIT(victim);

    if (!wounds)
    {
        send_to_char("Don't try to heal what ain't hurt!\n\r",ch);
        return;
    }

    if (LEARNED(ch, gsn_lay_on_hands) < number_percent())
    {
        send_to_char("You cannot seem to call on your deity right now.\n\r",ch);
        learn_from_failure(ch, gsn_lay_on_hands);
        WAIT_STATE(ch, PULSE_VIOLENCE);
        return;
    }

    act(AT_WHITE, "$n lays hands on $N.", ch, NULL, victim, TO_NOTVICT);
    act(AT_WHITE, "You lay hands on $N.", ch, NULL, victim, TO_CHAR);
    act(AT_WHITE, "$n lays hands on you.",ch, NULL, victim, TO_VICT);

    if (GET_ALIGN(victim)<0)
    {
        act(AT_DGREY, "You are too evil to benefit from this treatment.", ch, NULL, victim, TO_VICT);
        act(AT_DGREY, "$n is too evil to benefit from this treatment.", victim, NULL, ch, TO_ROOM);
        return;
    }

    if (GET_ALIGN(ch)<350) /* should never be since they get converted */
        healing = GET_LEVEL(ch, CLASS_PALADIN); /* after 349 */
    else if (GET_ALIGN(ch)>=900)
        healing = GET_LEVEL(ch, CLASS_PALADIN)*3;
    else
        healing = GET_LEVEL(ch, CLASS_PALADIN)*2;

    if (GET_ALIGN(ch)==1000 && number_percent()>(100-get_curr_lck(ch)))
    {
        act(AT_WHITE, "God favors your actions, and doubles your healing power!", ch, NULL, NULL, TO_CHAR);
        act(AT_WHITE, "God blesses $n, and $m healing power is doubled!",ch, NULL, NULL, TO_ROOM);
        healing *= 2;
        log_printf_plus(LOG_MAGIC, LEVEL_IMMORTAL, SEV_INFO, "%s is lucky, lay on hands doubled for %d hp!", GET_NAME(ch), healing);
    }

    if (healing>wounds)
        GET_HIT(victim) = GET_MAX_HIT(victim);
    else
        GET_HIT(victim) += healing;

    af.type = gsn_lay_on_hands;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    af.duration  = 24 * DUR_CONV;
    affect_to_char(ch, &af);

    WAIT_STATE(ch, PULSE_VIOLENCE);
}

void do_warcry(CHAR_DATA *ch, char *argument)
{
    int dam, dif, level;
    CHAR_DATA *victim;
    int gsn_warcry = skill_lookup("warcry");

    if (!gsn_warcry)
        return;

    if (GET_ALIGN(ch) < 350)
    {
        send_to_char("You're too ashamed of your behavior to warcry.\n\r",ch);
        return;
    }

    if (ch->fighting)
        victim = who_fighting(ch);
    else if (!(victim=get_char_room(ch,argument)))
    {
        send_to_char("You bellow at the top of your lungs, to bad your victim wasn't here to hear it.\n\r",ch);
        return;
    }

    if (IS_IMMORTAL(victim))
    {
        send_to_char("The gods are not impressed by people shouting at them.\n", ch);
        send_to_area("The earth trembles.\n\r", ch->in_room->area);
        return;
    }

    if (is_safe(ch,victim))
    {
        send_to_char("You warcry is completely silenced by the tranquility of this room.\n\r", ch);
        return;
    }

    if (LEARNED(ch, gsn_warcry) < number_percent())
    {
        send_to_char("Your mighty warcry emerges from your throat as a tiny squeak.\n\r", ch);
        learn_from_failure(ch, gsn_warcry);
        WAIT_STATE(ch, PULSE_VIOLENCE*3);
        set_fighting(victim, ch); /* make'em fight even if he fails */
    }
    else
    {
        if (!IS_NPC(victim))
        {
            act(AT_WHITE, "$n surprises you with a painful warcry!", ch, NULL, victim, TO_VICT);
        }

        dif=(level=GET_LEVEL(ch,CLASS_PALADIN)-GetMaxLevel(victim));
        if (dif>19)
        {
            spell_smaug( gsn_paralyze, GET_LEVEL(ch, CLASS_PALADIN), ch, victim );
            dam = (int)(level*2.5);
        }
        else if (dif>14)
            dam = (int)(level*2.5);
        else if (dif>10)
            dam = (int)(level*2);
        else if (dif>6)
            dam = (int)(level*1.5);
        else if (dif>-6)
            dam = (int)(level);
        else if (dif>-11)
            dam = (int)(level*.5);
        else
            dam = 0;

        if (saves_spell_staff(GET_LEVEL(ch, CLASS_PALADIN), victim) )
            dam /= 2;
        act(AT_WHITE, "You are attacked by $n who shouts a heroic warcry!", ch, NULL, victim, TO_VICT);
        act(AT_WHITE, "$n screams a warcry at $N with a tremendous fury!", ch, NULL, victim, TO_ROOM);
        act(AT_WHITE, "You fly into battle $N with a holy warcry!", ch, NULL, victim, TO_CHAR);
        damage(ch, victim, dam, gsn_warcry);
        if (!ch->fighting)
            set_fighting(ch, victim);
        WAIT_STATE(ch, PULSE_VIOLENCE*3);
    }

}

void do_find_food(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    SKILLTYPE *skill;
    int sn = skill_lookup("findfood");

    if (!IS_OUTSIDE(ch))
    {
        send_to_char("You must be outside, in the wilderness to find food.\n\r",ch);
        return;
    }

    WAIT_STATE(ch, PULSE_VIOLENCE*3);

    skill = get_skilltype(sn);
    if (!skill)
    {
        send_to_char("You can't seem to do that...\n\r", ch);
        bug("Can't find sn %d (find food)", sn);
        return;
    }

    if (number_range(1,101) > LEARNED(ch, sn) || !obj_exists_index(OBJ_VNUM_FIND_FOOD))
    {
        act(AT_SKILL, skill->miss_char, ch, NULL, NULL, TO_CHAR);
        act(AT_SKILL, skill->miss_room, ch, NULL, NULL, TO_ROOM);
        learn_from_failure(ch, sn);
        return;
    }

    obj = create_object(OBJ_VNUM_FIND_FOOD);
    if (!obj)
    {
        send_to_char("You can't seem to do that...\n\r", ch);
        bug("Can't create obj (find food)");
        return;
    }

    act(AT_SKILL, skill->hit_char, ch, obj, NULL, TO_CHAR);
    act(AT_SKILL, skill->hit_room, ch, obj, NULL, TO_ROOM);

    obj_to_char(obj, ch);
}

void do_find_water(CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    SKILLTYPE *skill;
    int sn = skill_lookup("findwater");

    if (!IS_OUTSIDE(ch))
    {
        send_to_char("You must be outside, in the wilderness to find water.\n\r",ch);
        return;
    }

    WAIT_STATE(ch, PULSE_VIOLENCE*3);

    skill = get_skilltype(sn);
    if (!skill)
    {
        send_to_char("You can't seem to do that...\n\r", ch);
        bug("Can't find sn %d (find water)", sn);
        return;
    }

    if (number_range(1,101) > LEARNED(ch, sn) || !get_obj_index(OBJ_VNUM_FIND_WATER))
    {
        act(AT_SKILL, skill->miss_char, ch, NULL, NULL, TO_CHAR);
        act(AT_SKILL, skill->miss_room, ch, NULL, NULL, TO_ROOM);
        learn_from_failure(ch, sn);
        return;
    }

    obj = create_object(OBJ_VNUM_FIND_WATER);
    if (!obj)
    {
        send_to_char("You can't seem to do that...\n\r", ch);
        bug("Can't create obj (find water)");
        return;
    }

    act(AT_SKILL, skill->hit_char, ch, obj, NULL, TO_CHAR);
    act(AT_SKILL, skill->hit_room, ch, obj, NULL, TO_ROOM);

    obj_to_char(obj, ch);
}

void do_carve(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *food;
    SKILLTYPE *skill;
    int sn = skill_lookup("carve");

    if (ch->mount)
    {
        send_to_char("Not from this mount you cannot!\n\r",ch);
        return;
    }

    if (!CanUseSkill(ch, sn))
    {
        send_to_char("You don't know how to do that.\n\r",ch);
        return;
    }

    if (!(corpse = get_obj_here(ch, argument)))
    {
        send_to_char("That's not here.\n\r",ch);
        return;
    }

    if (corpse->item_type != ITEM_CORPSE_NPC &&
        corpse->item_type != ITEM_CORPSE_PC)
    {
        send_to_char("You can't carve that!\n\r",ch);
        return;
    }

    if (corpse->weight<70)
    {
        send_to_char("There is no good meat left on it.\n\r",ch);
        return;
    }

    if (LEARNED(ch, sn) < number_percent())
    {
        send_to_char ("You can't seem to locate the choicest parts of the corpse.\n\r",ch);
        learn_from_failure( ch, sn );
        spell_lag(ch, sn);
        return;
    }

    food = create_object(OBJ_VNUM_FIND_FOOD);
    if (!food)
    {
        send_to_char("You can't seem to do that...\n\r", ch);
        bug("Can't create obj (carve)");
        return;
    }

    skill = get_skilltype(sn);

    act(AT_SKILL, skill->hit_char, ch, corpse, NULL, TO_CHAR);
    act(AT_SKILL, skill->hit_room, ch, corpse, NULL, TO_ROOM);

    obj_to_char(food, ch);

    STRFREE(food->name);
    food->name = STRALLOC("ration slice filet food");

    sprintf( buf, "a ration of %s", corpse->short_descr+14);
    STRFREE(food->short_descr);
    food->short_descr = STRALLOC(buf);

    sprintf( buf, "A ration made from %s lies here.", corpse->short_descr+14 );
    STRFREE(food->description);
    food->description = STRALLOC(buf);
}

void do_psidisintegrate( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int gsn_disintegrate = skill_lookup("disintegrate");

    if (is_affected(ch, gsn_mindwipe) ||
        is_affected(ch, gsn_feeblemind))
    {
        send_to_char("Der, what is that?\n\r", ch);
        return;
    }

    if ( (victim = get_char_room(ch, argument)) == NULL &&
         (victim = who_fighting(ch)) == NULL)
    {
        send_to_char("Disintegrate who?\n\r", ch);
        return;
    }

    if (victim == ch)
    {
        send_to_char("You cringe just thinking about it.\n\r", ch);
        return;
    }

    spell_disintegrate( gsn_disintegrate, GET_LEVEL(ch, CLASS_PSIONIST), ch, victim );
    spell_lag(ch, gsn_disintegrate);
}

void do_bellow(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vch;
    SKILLTYPE *skill;
    int sn = skill_lookup("bellow");

    if ( ch->in_room->sector_type == SECT_UNDERWATER )
    {
        send_to_char("Bellow underwater, are you mad????\n\r", ch);
        return;
    }

    if ( IS_ROOM_FLAG( ch->in_room, ROOM_SILENCE ) )
    {
        send_to_char("You are in a silent zone, you can't make a sound!\n\r", ch);
        return;
    }

    if (IS_SILENCED(ch))
    {
        send_to_char("You are silenced, you can't make a sound!\n\r", ch);
        return;
    }

    skill = get_skilltype(sn);

    if (LEARNED(ch, sn) < number_percent())
    {
        failed_casting( skill, ch, NULL, NULL );
        spell_lag(ch, sn);
        return;
    }

    successful_casting( skill, ch, NULL, NULL );

    for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
    {
        if (is_same_group(ch, vch) ||
            vch->master == ch ||
            IS_IMMORTAL(vch) ||
            char_died(vch))
            continue;

        if (GetMaxLevel(vch)-3 > GetMaxLevel(ch) ||
            saves_para_petri(GetMaxLevel(ch), vch))
        {
            act(AT_PLAIN, "$N doesn't appreciate your bellowing.", ch, NULL, vch, TO_CHAR);
            start_hating(vch, ch);
            set_fighting(vch, ch);
            continue;
        }

        start_fearing(vch, ch);

        if (GetMaxLevel(ch) + number_range(1,40) > 70)
        {
            act(AT_PLAIN, "You stunned $N!", ch, NULL, vch, TO_CHAR);
            act(AT_PLAIN, "$n stuns $N with a loud bellow!", ch, NULL, vch, TO_ROOM);
            vch->position = POS_STUNNED;
            spell_lag(vch, sn*2);
            continue;
        }

        act(AT_PLAIN, "You scared $N to death with your bellow!", ch, NULL, vch, TO_CHAR);
        act(AT_PLAIN, "$n scared $N with a loud bellow!", ch, NULL, vch, TO_ROOM);
        do_flee(vch,NULL);
    }

    spell_lag(ch, sn);
}

void do_psicrush( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    SKILLTYPE *skill;
    char name[MAX_INPUT_LENGTH];
    int dam, level, sn = skill_lookup("psicrush");

    one_argument(argument,name);

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
        if ( argument[0] == '\0'
             ||  (victim=get_char_room(ch, argument)) == NULL )
        {
            send_to_char("Exactly whom did you wish to crush?\n\r",ch);
            return;
        }
    }

    if (victim==ch)
    {
        send_to_char ("Psychic crush yourself? What would your teacher say?\n\r",ch);
        return;
    }

    if (IS_IMMORTAL(victim))
    {
        send_to_char ("Your mind can't fathom the intellect you are dealing with!\n\r",ch);
        return;
    }

    level=GET_LEVEL(ch,CLASS_PSIONIST);

    dam = dice(level, 6) + (int)(level/2);

    if (saves_spell_staff(GetMaxLevel(ch), victim))
    {
        dam /= 2;
        if (is_affected(victim, gsn_tower_of_iron_will))
            dam = 0;
    }
    else if (is_affected(victim, gsn_tower_of_iron_will))
        dam /= 2;

    if ((skill = get_skilltype(sn)))
        successful_casting( skill, ch, victim, NULL );

    damage(ch, victim, dam, sn);
    return;
}

void do_chameleon( CHAR_DATA *ch, char *argument )
{
    int sn = skill_lookup("chameleon");

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    if ( ch->mount )
    {
        send_to_char( "You can't do that while mounted.\n\r", ch );
        return;
    }

    send_to_char( "You attempt to change your color to match your surroundings.\n\r", ch );

    if ( IS_AFFECTED(ch, AFF_HIDE) )
        REMOVE_BIT(ch->affected_by, AFF_HIDE);

    if ( IS_NPC(ch) || number_percent( ) < ch->pcdata->learned[sn] )
    {
        SET_BIT(ch->affected_by, AFF_HIDE);
        learn_from_success( ch, sn );
    }
    else
        learn_from_failure( ch, sn );

    return;
}

void do_deathfield( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch, *vch_next;
    int dam;
    int sn = skill_lookup("deathfield");

    act(AT_MAGIC, "You begin to suck the life out of everything around you!", ch, NULL, NULL, TO_CHAR);
    act(AT_MAGIC, "$n sends out a shockwave all around him that makes you feel deathly ill!", ch, NULL, NULL, TO_ROOM);

    dam = (int) (GET_HIT(ch) / 2);

    for(vch = ch->in_room->first_person; vch; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if (IS_IMMORTAL(vch)||
            is_same_group(ch, vch) )
            continue;

        if (saves_spell_staff(GetMaxLevel(ch), vch))
        {
            dam /= 4;
        }
        if (is_affected(vch, gsn_tower_of_iron_will))
            dam /= 2;

        damage( ch, vch, dam, sn );

    }
    learn_from_success(ch, sn);
    spell_lag(ch, sn);
}


void do_detonate( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch, *vch_next;
    OBJ_DATA *target = NULL;
    char itemname[MAX_INPUT_LENGTH];
    int dam;
    int sn = skill_lookup("detonate");

    argument = one_argument(argument, itemname);
    if (!(target = get_obj_here(ch, itemname)))
    {
        act(AT_MAGIC, "You don't see one of those anywhere...", ch, NULL, NULL, TO_CHAR);
        return;
    }

    act(AT_MAGIC, "You use your psychic powers to blow $p to peices!", ch, target, NULL, TO_CHAR);
    act(AT_MAGIC, "$n blows up $p sending shrapnel flying everywhere!", ch, target, NULL, TO_ROOM);

    dam = dice(1, target->weight);

    extract_obj(target);

    for(vch = ch->in_room->first_person; vch; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if (vch == ch ||
            IS_IMMORTAL(vch)||
            is_same_group(ch, vch) )
            continue;

        damage( ch, vch, dam, sn );

    }
    learn_from_success(ch, sn);
    spell_lag(ch, sn);
}


void do_intuit( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    CHAR_DATA *mob;
    AFFECT_DATA *paf;
    SKILLTYPE *sktmp;
    char targetname[MAX_INPUT_LENGTH];
    int level = GetMaxLevel(ch);
    int sn = skill_lookup("intuit");

    argument = one_argument(argument, targetname);

    mob = get_char_room(ch, targetname);
    obj = get_obj_carry(ch, targetname);
    if (!mob && !obj)
    {
        send_to_char( "I can't find one of those anywhere!\n\r", ch );
        return;
    }

    set_char_color( AT_MAGIC, ch );
    if (obj)
    {
        ch_printf(ch, "You feel informed:\n\rObject '%s', Item type: %s\n\r",
                  obj->name, item_type_name(obj));
        /* affected bits? */

        if (obj->extra_flags || obj->extra_flags2 || obj->magic_flags)
        {
            ch_printf(ch, "Item is: %s ",
                      obj->extra_flags?flag_string(obj->extra_flags, o_flags):" ");
            ch_printf(ch, "%s ",
                      obj->extra_flags2?flag_string(obj->extra_flags2, o2_flags):" ");
            ch_printf(ch, "%s\n\r",
                      obj->magic_flags?flag_string(obj->magic_flags, mag_flags):" ");
        }
        ch_printf(ch, "Weight: %d, Value: %d, Rent cost: %d  %s\n\r",
                  obj->weight, obj->cost, obj->rent,
                  obj->rent>MIN_OBJ_RENT?"[RARE]":" ");
        switch (obj->item_type)
        {
        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
            ch_printf(ch, "Level %d spells of:\n\r", obj->value[0]);
            if (obj->value[1] >=0 && (sktmp=get_skilltype(obj->value[1])) != NULL)
                ch_printf(ch, "%s\n\r", sktmp->name);
            if (obj->value[2] >=0 && (sktmp=get_skilltype(obj->value[2])) != NULL)
                ch_printf(ch, "%s\n\r", sktmp->name);
            if (obj->value[3] >=0 && (sktmp=get_skilltype(obj->value[3])) != NULL)
                ch_printf(ch, "%s\n\r", sktmp->name);
            break;
        case ITEM_WAND:
        case ITEM_STAFF:
            ch_printf(ch, "Has %d charges, with %d charges left.\n\r",
                      obj->value[1], obj->value[2]);
            if (obj->value[3] >= 0 && (sktmp=get_skilltype(obj->value[3])) != NULL)
                ch_printf(ch, "Level %d spell of:\n\r%s\n\r",
                          obj->value[0], sktmp->name);
            break;
        case ITEM_MATERIAL:
            ch_printf(ch, "Is %s grade %s.\n\r", mat_qname[MGRADE(obj)],
                      mat_name[MTYPE(obj)]);
            break;
        case ITEM_WEAPON:
            ch_printf(ch, "Damage Dice is '%dD%d'\n\r",
                      obj->value[1], obj->value[2]);
            break;
        case ITEM_ARMOR:
            ch_printf(ch, "AC-apply is %d\n\r",
                      obj->value[0]);
            break;
        }
        /*
        for (paf=obj->pIndexData->first_affect; paf; paf=paf->next)
            showaffect(ch, paf);
        */
        for (paf=obj->first_affect; paf; paf=paf->next)
            showaffect(ch, paf);

        return;
    }

    if (mob)
    {
        if (IS_IMMORTAL(mob) && !IS_IMMORTAL(ch))
        {
            return;
        }
        if (!IS_NPC(mob))
        {
            ch_printf(ch, "%d Years,  %d Months,  %d Days,  %d Hours old.\n\r",
                      get_age(mob), get_age_month(mob), get_age_day(mob), get_age_hour(mob));
        }
        ch_printf(ch, "Height %din  Weight %dpounds\n\r",
                  mob->height, mob->weight);
        ch_printf(ch, "Armor Class: %d\n\rAlignment: %d\n\rSpellfail: %d\n\r",
                  GET_AC(mob), mob->alignment, mob->spellfail);
        if (level>10)
            ch_printf(ch, "Attacks: %1.3f\n\r",
                      (float)mob->numattacks/1000);
        if (level>30)
            ch_printf(ch, "Str %d, Int %d, Wis %d, Dex %d, Con %d, Ch %d, Lck %d\n\r",
                      get_curr_str(mob), get_curr_int(mob), get_curr_wis(mob),
                      get_curr_dex(mob), get_curr_con(mob), get_curr_cha(mob),
                      get_curr_lck(mob));

    }

    learn_from_success(ch, sn);
}


void do_psisummon( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    int sn = skill_lookup("psisummon");
    SKILLTYPE *skill = get_skilltype(sn);

    if (IS_SYSTEMFLAG(SYS_NOSUMMON))
    {
        send_to_char("Everything appears to have gone right, but nothing happens.\n\r", ch);
        return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL
         ||   victim == ch
         ||   !victim->in_room
         ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
         ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
         ||   IS_SET(victim->in_room->room_flags, ROOM_NO_SUMMON)
         ||   IS_SET(ch->in_room->room_flags, ROOM_NO_SUMMON)
         ||   victim->fighting
         ||  GetAveLevel(victim) >= GetAveLevel(ch) + 15
         ||  (IS_NPC(victim) && IS_SET(victim->act, ACT_PROTOTYPE))
         ||   !in_hard_range( victim, ch->in_room->area )
         ||  ( !IS_NPC( ch ) && !CAN_PKILL( ch ) && IS_PKILL( victim ) )
         ||  (IS_SET(victim->in_room->area->flags, AFLAG_NOPKILL) && IS_PKILL(ch))
         ||  ( !IS_NPC(ch) && !IS_NPC(victim) && IS_SET(victim->pcdata->flags, PCFLAG_NOSUMMON) ) )
    {
        send_to_char("You fail to get a good psychic contact with your target.\n\r", ch);
        act( AT_MAGIC, "$n seems to fade from reality briefly, then returns!", ch, NULL, NULL, TO_ROOM );
        failed_casting( skill, ch, victim, NULL );
        learn_from_failure(ch, sn);
        return;
    }

    if (number_percent() > LEARNED(ch, sn))
    {
        send_to_char("You fail at your attempt to summon.\n\r", ch);
        failed_casting( skill, ch, victim, NULL );
        learn_from_failure(ch, sn);
        return;
    }

    if ( !IsExtraPlanar(ch) &&
         is_other_plane(ch->in_room, victim->in_room))
    {
        send_to_char("The planes obscure your psychic powers.\n\r", ch);
        learn_from_failure(ch, sn);
        if (IS_IMMORTAL(ch))
            send_to_char("However, your immortality overcomes it.\n\r", ch);
        else
            return;
    }

    act( AT_MAGIC, "$n fades quickly from reality.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, ch->in_room );
    act( AT_MAGIC, "$n fades into existance right before your eyes.", victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );
    return;
}

void do_psiteleport(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    int sn = skill_lookup("psiteleport");

    if (is_affected(ch, gsn_mindwipe) ||
        is_affected(ch, gsn_feeblemind))
    {
        send_to_char("Der, what is that?\n\r", ch);
        return;
    }

    if ( IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You can't concentrate enough for that.\n\r", ch );
        return;
    }

    if ( !argument || *argument == '\0' ||
         (victim=get_char_room(ch, argument)) == NULL )
    {
        send_to_char("Exactly whom did you wish to teleport?\n\r",ch);
        return;
    }

    if (IS_IMMORTAL(victim))
    {
        send_to_char ("They ignore your attempt at humor!\n\r",ch);
        return;
    }

    spell_teleport(sn, BestSkLv(ch, sn), ch, victim);
}

void do_first_aid(CHAR_DATA *ch, char *argument)
{
}

void do_feign_death(CHAR_DATA *ch, char *argument)
{
}

