/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer and Heath Leach
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*static char rcsid[] = "$Id: magic2.c,v 1.53 2004/04/06 22:00:10 dotd Exp $";*/

#include <stdio.h>
#include <string.h>

#include "mud.h"
#include "gsn.h"

DECLARE_DO_FUN(do_stand);
DECLARE_DO_FUN(do_flee);

DECLARE_SPELL_FUN(spell_poison);
DECLARE_SPELL_FUN(spell_teleport);
DECLARE_SPELL_FUN(spell_blindness);

extern char *target_name;

/*
 * magic.c functions
 */
void successful_casting  args( ( SKILLTYPE *skill, CHAR_DATA *ch,
                                 CHAR_DATA *victim, OBJ_DATA *obj ) );
void failed_casting      args( ( SKILLTYPE *skill, CHAR_DATA *ch,
                                 CHAR_DATA *victim, OBJ_DATA *obj ) );
void immune_casting      args( ( SKILLTYPE *skill, CHAR_DATA *ch,
                                 CHAR_DATA *victim, OBJ_DATA *obj ) );
int  ris_save            args( ( CHAR_DATA *ch, int chance, int ris ) );
void make_charmie        args( ( CHAR_DATA *ch, CHAR_DATA *mob, int sn ) );


ch_ret spell_pword_kill( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int max = 80;

    max += level;
    max += level/2;

    if (GET_MAX_HIT(victim) <= max || GetMaxLevel(ch) > LEVEL_IMMORTAL)
        return damage(ch, victim, GET_MAX_HIT(victim)*4, sn);
    else
        send_to_char("They are too powerful to destroy this way.\n\r", ch);

    return rNONE;
}

ch_ret spell_chain_lightning( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *vch_next, *vch, *victim = (CHAR_DATA *) vo;
    bool ch_died = FALSE;
    ch_ret retcode = rNONE;
    int dam = dice(level,6);

    if (victim)
        damage(ch, victim, dam, sn);

    for ( vch = first_char; vch; vch = vch_next )
    {
        vch_next	= vch->next;

        if ( char_died(vch) || !vch->in_room || victim==vch || ch==vch )
            continue;

        if ( vch->in_room == ch->in_room )
        {
            if ( !IS_NPC( vch ) && vch->pcdata->wizinvis >= LEVEL_IMMORTAL )
                continue;

            if (vch->master == ch ||
                ch->master == vch ||
                is_same_group(ch, vch))
                continue;

            dam = dice(UMAX(1,level-1),6);

            if ( ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) ) )
                retcode = damage( ch, vch, dam, sn );
            if ( retcode == rCHAR_DIED || char_died(ch) )
            {
                ch_died = TRUE;
                continue;
            }
            if ( char_died(vch) )
                continue;
        }
        else if ( vch->in_room->area == ch->in_room->area )
        {
            set_char_color( AT_MAGIC, vch );
            send_to_char( "You hear a loud thunderclap...\n\r", vch );
        }
    }

    if ( ch_died )
        return rCHAR_DIED;
    else
        return rNONE;
}

/* gilgamesh */

ch_ret spell_strength( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    AFFECT_DATA af;
    SKILLTYPE *skill = get_skilltype(sn);
    int dam;

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rVICT_IMMUNE;
    }

    if ( is_affected( victim, sn ))
    {
        act( AT_MAGIC, "Nothing new seems to happen.", ch, NULL, victim, TO_CHAR );
        return rNONE;
    }

    successful_casting( skill, ch, victim, NULL );

    if (IS_ACTIVE(victim, CLASS_WARRIOR) ||
        IS_ACTIVE(victim, CLASS_BARBARIAN) ||
        IS_ACTIVE(victim, CLASS_RANGER) ||
        IS_ACTIVE(victim, CLASS_PALADIN) ||
        IS_ACTIVE(victim, CLASS_ANTIPALADIN))
        dam = dice(1,8);
    else if (IS_ACTIVE(victim, CLASS_CLERIC) ||
             IS_ACTIVE(victim, CLASS_THIEF) ||
             IS_NPC(victim))
        dam = dice(1,6);
    else
        dam = dice(1,4);

    if (IS_IMMORTAL(ch))
        dam = 25 - get_curr_str(victim);

    af.type      = sn;
    af.duration  = (int)((2*level)*DUR_CONV);
    af.modifier  = dam;
    af.location  = APPLY_STR;
    af.bitvector = 0;
    affect_to_char(victim, &af);
    return rNONE;
}

ch_ret spell_second_wind( int sn, int level, CHAR_DATA *ch, void *vo )
{
    int dam;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rVICT_IMMUNE;
    }

    dam = dice(level,8)+level;

    if ( (dam + GET_MOVE(victim)) > move_limit(victim) )
        GET_MOVE(victim) = move_limit(victim);
    else
        GET_MOVE(victim) += dam;

    successful_casting ( skill, ch, victim, NULL);
    return rNONE;
}

ch_ret spell_refresh( int sn, int level, CHAR_DATA *ch, void *vo )
{
    int dam;
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rVICT_IMMUNE;
    }

    dam = dice(level,4)+level;

    if ( (dam + GET_MOVE(victim)) > move_limit(victim) )
        GET_MOVE(victim) = move_limit(victim);
    else
        GET_MOVE(victim) += dam;

    successful_casting ( skill, ch, victim, NULL);
    return rNONE;

}

ch_ret spell_pword_blind( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rVICT_IMMUNE;
    }

    if (IS_AFFECTED(victim, AFF_BLIND) )
    {
        act( AT_MAGIC, "$N is already blind.", ch, NULL, victim, TO_CHAR );
        return rSPELL_FAILED;
    }

    if (GET_MAX_HIT(victim) <= 100 || IS_IMMORTAL(ch))
    {
        SET_BIT(victim->affected_by, AFF_BLIND);
        successful_casting ( skill, ch, victim, NULL);
        return rNONE;
    }

    act( AT_MAGIC, "$N is too powerful to blind this way.", ch, NULL, victim, TO_CHAR );
    return rVICT_IMMUNE;
}

ch_ret spell_magic_missile( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    int dam;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rVICT_IMMUNE;
    }

    if ( is_affected( victim, gsn_shield ) )
    {
        act( AT_MAGIC, "$N's shield deflects the magic missile from $n!", ch, NULL, victim, TO_NOTVICT );
        act( AT_MAGIC, "$N's shield deflects your magic missile!",ch, NULL, victim, TO_CHAR );
        act( AT_MAGIC, "Your shield deflects $n's magic missile!",ch, NULL, victim, TO_VICT );
        dam = 0;
    } else {
        dam = dice(UMAX((level / 2),1),4)+UMAX((level / 2),1);
    }

    return damage( ch, victim, dam, sn );
}

CHAR_DATA *world_find_pc(char *arg)
{
    CHAR_DATA *wch;
    for ( wch = first_char; wch; wch = wch->next )
        if ( nifty_is_name( arg, wch->name ) && !IS_NPC(wch))
            return wch;
    return NULL;
}


ch_ret spell_resurection( int sn, int level, CHAR_DATA *ch, void *vo)
{

    OBJ_DATA *obj = (OBJ_DATA *) vo;
    OBJ_DATA *obj_content;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int mob_vnum;
    int nr,i;
    SKILLTYPE *skill = get_skilltype(sn);

    if (!obj) {
        send_to_char("You can't find any of those!\n\r", ch);
        return rSPELL_FAILED;
    }

    if ((obj->item_type != ITEM_CORPSE_PC) && (obj->item_type != ITEM_CORPSE_NPC)) {
        send_to_char("You can't resurrect that!\n\r", ch);
        return rSPELL_FAILED;
    }

    if (obj->vnum == OBJ_VNUM_CORPSE_PC)
    {
        if (SPELL_POWER(skill) != SP_GREATER)
        {
            if (GET_MONEY(ch,DEFAULT_CURR) < 75000)
            {
                send_to_char("You can't make a sufficient sacrifice!\n\r", ch);
                return rSPELL_FAILED;
            }
        }
        sscanf(obj->name, "%s %s", buf, buf2);
        if (strcmp(buf, "corpse"))
            return rSPELL_FAILED;
        victim = world_find_pc(buf2);
        if (!victim) {
            send_to_char("Their spirit cannot be found.\n\r", ch);
            return rSPELL_FAILED;
        }
        if (!IS_SET(victim->act2, PLR2_DIED)) {
            send_to_char("They have to be dead first, silly!\n\r", ch);
            return rSPELL_FAILED;
        }
        if (victim->perm_con < 4) {
            send_to_char("Their spirit is too weak.\n\r", ch);
            return rSPELL_FAILED;
        }
        if (GetMaxLevel(victim)>=51)
            GET_EXP(victim) = (int)((float)GET_EXP(victim) / 0.65);
        else if (GetMaxLevel(victim)>=40)
            GET_EXP(victim) = (int)((float)GET_EXP(victim) / 0.70);
        else if (GetMaxLevel(victim)>=25)
            GET_EXP(victim) = (int)((float)GET_EXP(victim) / 0.80);
        else if (GetMaxLevel(victim)>=15)
            GET_EXP(victim) = (int)((float)GET_EXP(victim) / 0.85);
        else if (GetMaxLevel(victim)>=5)
            GET_EXP(victim) = (int)((float)GET_EXP(victim) / 0.90);
        else
            GET_EXP(victim) = (int)((float)GET_EXP(victim) / 0.95);
        if (SPELL_POWER(skill) != SP_GREATER)
            GET_MONEY(ch,DEFAULT_CURR) -= 75000;
        for (i=0;i<MAX_CURR_TYPE;i++)
            GET_MONEY(victim,i) = 0;
        GET_MANA(victim) = 1;
        GET_HIT(victim) = 1;
        GET_MOVE(victim) = 1;
        victim->perm_con -= 1;
        char_from_room(victim);
    } else {
        if (SPELL_POWER(skill) != SP_GREATER) {
            if (GET_MONEY(ch,DEFAULT_CURR) < 25000) {
                send_to_char("You can't make a sufficient sacrifice!\n\r", ch);
                return rSPELL_FAILED;
            }
        }
        mob_vnum = 0 - obj->cost;
        victim = create_mobile(mob_vnum);
        if (!victim) {
            send_to_char("Their spirit is not strong enough!\n\r", ch);
            return rSPELL_FAILED;
        }
        if (SPELL_POWER(skill) != SP_GREATER)
            GET_MONEY(ch,DEFAULT_CURR) -= 25000;
        for (i=0;i<MAX_CURR_TYPE;i++)
            GET_MONEY(victim,i) = 0;
        GET_HIT(victim) = 1;
        GET_MANA(victim) = 1;
        GET_MOVE(victim) = 1;
        SET_BIT(victim->act, ACT_IS_NPC);
        SET_BIT(victim->act, ACT_NICE_THIEF);
        SET_BIT(victim->act, ACT_SENTINEL);
        REMOVE_BIT(victim->act, ACT_AGGRESSIVE);
        REMOVE_BIT(victim->act, ACT_SCAVENGER);
    }
    if (SPELL_POWER(skill) == SP_GREATER) {
        nr = RACE_HALFBREED;
        while ((nr < RACE_HUMAN) || (nr > RACE_DRAGON && nr < RACE_ORC) ||
               (nr > RACE_SLIME && nr < RACE_SNAKE) ||
               (nr > RACE_VEGGIE && nr < RACE_GOBLIN) ||
               (nr > RACE_DROW && nr < RACE_SKEXIE) ||
               (nr > RACE_HORSE && nr < RACE_GIANT_HILL) ||
               (nr > RACE_DRAGON_BRASS && nr < RACE_HALF_ELF) ||
               (nr > RACE_SEA_ELF)) {
            nr = number_range(RACE_HUMAN, RACE_SEA_ELF);
        }
        GET_RACE(victim) = nr;
        victim->xflags = race_bodyparts(victim);
    }
    char_to_room(victim, ch->in_room);

    ch_printf(ch, "The gods have answered your prayers and have resurrected %s!\n\r", PERS(victim, ch));

    while ((obj_content = obj->last_content) != NULL) {
        obj_from_obj(obj_content);
        obj_to_char(obj_content, victim);
    }

    extract_obj(obj);

    if (((IS_NPC(victim)) && ( !IS_SET( victim->immune, RIS_MAGIC ) &&
                               !IS_SET( victim->immune, RIS_CHARM ) )
         && (ris_save( victim, level, RIS_CHARM ) != 1000)
         && (GetMaxLevel(victim) <= level)
         && !circle_follow( victim, ch )
         && !saves_spell_staff( ris_save(victim, level, RIS_CHARM), victim )
        ) || (IS_IMMORTAL(ch) && IS_NPC(victim)))
    {
        AFFECT_DATA af;
        add_follower( victim, ch );
        af.type      = sn;
        af.duration  = (int)(24*18 * DUR_CONV);
        af.location  = 0;
        af.modifier  = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char( victim, &af );
        do_stand(victim, "");
        sprintf(buf, "kneel %s", GET_NAME(ch));
        interpret(victim, buf);
    } else {
        do_stand(victim, "");
        sprintf(buf, "thank %s", GET_NAME(ch));
        interpret(victim, buf);
    }

    return rNONE;

}

ch_ret spell_monsum( int sn, int level, CHAR_DATA *ch, void *vo )
{
    SKILLTYPE *skill = get_skilltype(sn);
    CHAR_DATA *mob = NULL;
    int vnum = 0;


    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE) ||
        IS_SET(ch->in_room->room_flags, ROOM_PRIVATE) ||
        IS_SET(ch->in_room->room_flags, ROOM_SOLITARY) ||
        IS_SET(ch->in_room->room_flags, ROOM_NO_SUMMON))
    {
        send_to_char("Ancient Magiks bar your path.\n\r", ch);
        if (IS_IMMORTAL(ch))
            send_to_char("However, your immortality overcomes them.\n\r", ch);
        else
            return rSPELL_FAILED;
    }

    switch( SPELL_POWER(skill) )
    {
    default:
    case SP_NONE:
        if (SPELL_CLASS(skill) == SC_NONE)
            vnum = 16034;
        else
            vnum = number_range(1,10) + 240;
        break;
    case SP_MINOR:
        if (SPELL_CLASS(skill) == SC_NONE)
            vnum = 9191;
        else
            vnum = number_range(1,10) + 250;
        break;
    case SP_GREATER:
        if (SPELL_CLASS(skill) == SC_NONE)
            vnum = number_range(1,10) + 220;
        else
            vnum = number_range(1,10) + 260;
        break;
    case SP_MAJOR:
        if (SPELL_CLASS(skill) == SC_NONE)
            vnum = number_range(1,10) + 230;
        else
            vnum = 0; /* would be monsum eight if it existed */
        break;
    }


    if ( !vnum ||
         (mob=create_mobile(vnum)) == NULL )
    {
        failed_casting( skill, ch, NULL, NULL );
        return rNONE;
    }

    successful_casting( skill, ch, mob, NULL );
    char_to_room( mob, ch->in_room );

    if (too_many_followers(ch))
    {
        act(AT_MAGIC, "$N takes one look at the size of your posse and just says no!", ch, NULL, mob, TO_CHAR);
        act(AT_MAGIC, "$N takes one look at the size of $n's posse and just says no!", ch, NULL, mob, TO_ROOM);
    }
    else
        make_charmie(ch, mob, sn);

    mprog_birth_trigger(ch, mob);

    return rNONE;
}


ch_ret spell_conjure_elemental( int sn, int level, CHAR_DATA *ch, void *vo )
{
    SKILLTYPE *skill = get_skilltype(sn);
    CHAR_DATA *mob = NULL;
    OBJ_DATA *obj;
    int vnum = 0, objv = 0;


    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE) ||
        IS_SET(ch->in_room->room_flags, ROOM_PRIVATE) ||
        IS_SET(ch->in_room->room_flags, ROOM_SOLITARY) ||
        IS_SET(ch->in_room->room_flags, ROOM_NO_SUMMON))
    {
        send_to_char("Ancient Magiks bar your path.\n\r", ch);
        if (IS_IMMORTAL(ch))
            send_to_char("However, your immortality overcomes them.\n\r", ch);
        else
            return rSPELL_FAILED;
    }

    if (!str_cmp(target_name, "fire"))
    {
        vnum = MOB_VNUM_FIRE_ELEMENTAL;
        objv = OBJ_VNUM_RED_STONE;
    }
    else if (!str_cmp(target_name, "water"))
    {
        vnum = MOB_VNUM_WATER_ELEMENTAL;
        objv = OBJ_VNUM_PALE_BLUE_STONE;
    }
    else if (!str_cmp(target_name, "air"))
    {
        vnum = MOB_VNUM_AIR_ELEMENTAL;
        objv = OBJ_VNUM_GREY_STONE;
    }
    else if (!str_cmp(target_name, "earth"))
    {
        vnum = MOB_VNUM_EARTH_ELEMENTAL;
        objv = OBJ_VNUM_CLEAR_STONE;
    }

    if ( !objv ||
         (obj=get_eq_char(ch,WEAR_HOLD))==NULL ||
         obj->vnum!=objv )
    {
        failed_casting(skill, ch, NULL, NULL);
        return rNONE;
    }

    if ( !vnum ||
         (mob=create_mobile(vnum))==NULL )
    {
        failed_casting(skill, ch, NULL, NULL);
        return rNONE;
    }

    successful_casting( skill, ch, mob, NULL );
    char_to_room( mob, ch->in_room );

    if (too_many_followers(ch))
    {
        act(AT_MAGIC, "$N takes one look at the size of your posse and just says no!", ch, NULL, mob, TO_CHAR);
        act(AT_MAGIC, "$N takes one look at the size of $n's posse and just says no!", ch, NULL, mob, TO_ROOM);
    }
    else
        make_charmie(ch, mob, sn);

    mprog_birth_trigger(ch, mob);

    act(AT_FIRE, "$p bursts into flame and disintegrates!", ch, obj, NULL, TO_ROOM);
    act(AT_FIRE, "$p bursts into flame and disintegrates!", ch, obj, NULL, TO_CHAR);

    unequip_char(ch, obj);
    separate_obj(obj);
    obj_from_char(obj);
    extract_obj(obj);

    return rNONE;
}


ch_ret spell_cacaodemon( int sn, int level, CHAR_DATA *ch, void *vo )
{
    SKILLTYPE *skill = get_skilltype(sn);
    CHAR_DATA *mob = NULL;
    OBJ_DATA *obj;
    int vnum = 0, objv = 0;


    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE) ||
        IS_SET(ch->in_room->room_flags, ROOM_PRIVATE) ||
        IS_SET(ch->in_room->room_flags, ROOM_SOLITARY) ||
        IS_SET(ch->in_room->room_flags, ROOM_NO_SUMMON))
    {
        send_to_char("Ancient Magiks bar your path.\n\r", ch);
        if (IS_IMMORTAL(ch))
            send_to_char("However, your immortality overcomes them.\n\r", ch);
        else
            return rSPELL_FAILED;
    }

    if (!str_cmp(target_name, "one"))
    {
        vnum = MOB_VNUM_DEMON_TYPE_I;
        objv = OBJ_VNUM_DEMON_TYPE_I;
    }
    else if (!str_cmp(target_name, "two"))
    {
        vnum = MOB_VNUM_DEMON_TYPE_II;
        objv = OBJ_VNUM_DEMON_TYPE_II;
    }
    else if (!str_cmp(target_name, "three"))
    {
        vnum = MOB_VNUM_DEMON_TYPE_III;
        objv = OBJ_VNUM_DEMON_TYPE_III;
    }
    else if (!str_cmp(target_name, "four"))
    {
        vnum = MOB_VNUM_DEMON_TYPE_IV;
        objv = OBJ_VNUM_DEMON_TYPE_IV;
    }
    else if (!str_cmp(target_name, "five"))
    {
        vnum = MOB_VNUM_DEMON_TYPE_V;
        objv = OBJ_VNUM_DEMON_TYPE_V;
    }
    else if (!str_cmp(target_name, "six"))
    {
        vnum = MOB_VNUM_DEMON_TYPE_VI;
        objv = OBJ_VNUM_DEMON_TYPE_VI;
    }

    if ( !objv ||
         (obj=get_eq_char(ch,WEAR_WIELD))==NULL ||
         obj->vnum!=objv )
    {
        failed_casting(skill, ch, NULL, NULL);
        return rNONE;
    }

    if ( !vnum ||
         (mob=create_mobile(vnum))==NULL )
    {
        failed_casting(skill, ch, NULL, NULL);
        return rNONE;
    }

    act(AT_BLACK, "$n gestures, and a black cloud of smoke appears.", ch, NULL, NULL, TO_ROOM);
    act(AT_BLACK, "$n gestures, and a black cloud of smoke appears.", ch, NULL, NULL, TO_CHAR);
    if (GET_LEVEL(ch, CLASS_CLERIC) > 40 && IS_EVIL(ch))
    {
        act(AT_BLACK, "$p smokes briefly.", ch, obj, NULL, TO_ROOM);
        act(AT_BLACK, "$p smokes briefly.", ch, obj, NULL, TO_CHAR);
        obj->cost /= 2;
        if (obj->cost < 100)
        {
            act(AT_FIRE, "$p bursts into flame and disintegrates!",
                ch, obj, NULL, TO_ROOM);
            act(AT_FIRE, "$p bursts into flame and disintegrates!",
                ch, obj, NULL, TO_CHAR);
            unequip_char(ch, obj);
            separate_obj(obj);
            obj_from_char(obj);
            extract_obj(obj);
        }
    }
    else
    {
        act(AT_FIRE, "$p bursts into flame and disintegrates!", ch, obj, NULL, TO_ROOM);
        act(AT_FIRE, "$p bursts into flame and disintegrates!", ch, obj, NULL, TO_CHAR);
        unequip_char(ch, obj);
        separate_obj(obj);
        obj_from_char(obj);
        extract_obj(obj);
        GET_ALIGN(ch)-=5;
    }
    act(AT_MAGIC, "With an evil laugh, $N emerges from the smoke.", ch, NULL, mob, TO_NOTVICT);

    successful_casting( skill, ch, mob, NULL );
    char_to_room( mob, ch->in_room );

    if (too_many_followers(ch))
    {
        act(AT_MAGIC, "$N takes one look at the size of your posse and just says no!", ch, NULL, mob, TO_CHAR);
        act(AT_MAGIC, "$N takes one look at the size of $n's posse and just says no!", ch, NULL, mob, TO_ROOM);
    }
    else
        make_charmie(ch, mob, sn);

    mprog_birth_trigger(ch, mob);

    return rNONE;
}

ch_ret spell_animate_rock(int sn, int level, CHAR_DATA *ch, void *vo)
{
    SKILLTYPE *skill = get_skilltype(sn);
    int vnum = MOB_VNUM_ANIMATE_ROCK;
    OBJ_DATA *obj;
    CHAR_DATA *mob = NULL;

    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE) ||
        IS_SET(ch->in_room->room_flags, ROOM_PRIVATE) ||
        IS_SET(ch->in_room->room_flags, ROOM_SOLITARY) ||
        IS_SET(ch->in_room->room_flags, ROOM_NO_SUMMON))
    {
        send_to_char("Ancient Magiks bar your path.\n\r", ch);
        if (IS_IMMORTAL(ch))
            send_to_char("However, your immortality overcomes them.\n\r", ch);
        else
            return rSPELL_FAILED;
    }

    if (!(obj=get_obj_here(ch, target_name)))
    {
        send_to_char("You don't see that here.\n\r", ch);
        return rSPELL_FAILED;
    }

    if (obj->item_type != ITEM_ROCK)
    {

        send_to_char("You can only cast this on rocks.\n\r", ch);
        return rSPELL_FAILED;
    }

    if (obj->weight > 20)
        vnum++;
    if (obj->weight > 40)
        vnum++;
    if (obj->weight > 80)
        vnum++;
    if (obj->weight > 160)
        vnum++;
    if (obj->weight > 320)
        vnum++;

    if ( !vnum ||
         (mob=create_mobile(vnum)) == NULL )
    {
        failed_casting( skill, ch, NULL, NULL );
        return rNONE;
    }

    successful_casting( skill, ch, mob, NULL );
    char_to_room( mob, ch->in_room );

    if (too_many_followers(ch))
    {
        act(AT_MAGIC, "$N takes one look at the size of your posse and just says no!", ch, NULL, mob, TO_CHAR);
        act(AT_MAGIC, "$N takes one look at the size of $n's posse and just says no!", ch, NULL, mob, TO_ROOM);
    }
    else
        make_charmie(ch, mob, sn);

    mprog_birth_trigger(ch, mob);

    return rNONE;
}

ch_ret spell_golem(int sn, int level, CHAR_DATA *ch, void *vo)
{
    SKILLTYPE *skill = get_skilltype(sn);
    CHAR_DATA *mob = NULL;
    OBJ_DATA *obj;
    int vnum = MOB_VNUM_ARMOR_GOLEM, armor;
    OBJ_DATA *helmet=NULL, *boots=NULL, *breast=NULL,
        *pants=NULL, *sleeves=NULL, *gloves=NULL;

    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE) ||
        IS_SET(ch->in_room->room_flags, ROOM_PRIVATE) ||
        IS_SET(ch->in_room->room_flags, ROOM_SOLITARY) ||
        IS_SET(ch->in_room->room_flags, ROOM_NO_SUMMON))
    {
        send_to_char("Ancient Magiks bar your path.\n\r", ch);
        if (IS_IMMORTAL(ch))
            send_to_char("However, your immortality overcomes them.\n\r", ch);
        else
            return rSPELL_FAILED;
    }

    for (obj=ch->in_room->first_content;obj;obj=obj->next_content)
    {
        if (obj->item_type != ITEM_ARMOR)
            continue;
        if (!helmet && IS_SET(obj->wear_flags, ITEM_WEAR_HEAD))
            helmet=obj;
        else if (!boots && IS_SET(obj->wear_flags, ITEM_WEAR_FEET))
            boots=obj;
        else if (!breast && IS_SET(obj->wear_flags, ITEM_WEAR_BODY))
            breast=obj;
        else if (!pants && IS_SET(obj->wear_flags, ITEM_WEAR_LEGS))
            pants=obj;
        else if (!sleeves && IS_SET(obj->wear_flags, ITEM_WEAR_ARMS))
            sleeves=obj;
        else if (!gloves && IS_SET(obj->wear_flags, ITEM_WEAR_HANDS))
            gloves=obj;
    }

    if (!helmet || !boots || !breast || !pants || !sleeves || !gloves)
    {
        send_to_char("You don't have the required pieces of armor here.\n\r", ch);
        return rSPELL_FAILED;
    }

    if ( !vnum ||
         (mob=create_mobile(vnum))==NULL )
    {
        failed_casting(skill, ch, NULL, NULL);
        return rNONE;
    }

    successful_casting( skill, ch, mob, NULL );
    char_to_room( mob, ch->in_room );

    act(AT_MAGIC, "$n waves $s hand over a pile of armor on the floor.", ch, NULL, NULL, TO_ROOM);
    act(AT_MAGIC, "You wave your hands over the pile of armor.", ch, NULL,NULL, TO_CHAR);
    act(AT_MAGIC, "The armor flys together to form a humanoid figure, $N!", ch, NULL, mob, TO_ROOM);
    act(AT_MAGIC, "$N is quickly assembled from the pieces!", ch, NULL, mob, TO_CHAR);

    if (too_many_followers(ch))
    {
        act(AT_MAGIC, "$N takes one look at the size of your posse and just says no!", ch, NULL, mob, TO_CHAR);
        act(AT_MAGIC, "$N takes one look at the size of $n's posse and just says no!", ch, NULL, mob, TO_ROOM);
    }
    else
        make_charmie(ch, mob, sn);

    armor = helmet->value[0];
    armor += boots->value[0];
    armor += gloves->value[0];
    armor += (pants->value[0]*2);
    armor += (sleeves->value[0]*2);
    armor += (breast->value[0]*3);
    mob->armor -= armor;

    mob->max_hit = dice( (armor/6), 10) + GetMaxLevel(ch);
    GET_HIT(mob) = GET_MAX_HIT(mob);

    GET_LEVEL(mob, CLASS_WARRIOR) = (armor/6);
    mob->exp = 0;

    if (GET_LEVEL(mob, CLASS_WARRIOR) > 10)
        mob->numattacks += 500;

    add_obj_affects(mob, helmet);
    add_obj_affects(mob, boots);
    add_obj_affects(mob, gloves);
    add_obj_affects(mob, pants);
    add_obj_affects(mob, sleeves);
    add_obj_affects(mob, breast);

    mprog_birth_trigger(ch, mob);

    extract_obj(helmet);
    extract_obj(gloves);
    extract_obj(boots);
    extract_obj(breast);
    extract_obj(sleeves);
    extract_obj(pants);

    return rNONE;
}

ch_ret spell_warp_weapon(int sn, int level, CHAR_DATA *ch, void *vo)
{
    SKILLTYPE *skill = get_skilltype(sn);
    CHAR_DATA *victim = NULL;
    OBJ_DATA *obj = NULL;

    if ((victim = get_char_room(ch, target_name)))
        obj = get_eq_char(victim, WEAR_WIELD);
    else
        obj = get_obj_carry(ch, target_name);

    if (!obj)
        return rSPELL_FAILED;

    successful_casting( skill, ch, victim, obj );
    act(AT_MAGIC, "$p is warped and twisted by the power of the spell.",
        ch, obj, NULL, TO_CHAR);
    act(AT_MAGIC, "$p is warped and twisted by the power of the spell.",
        ch, obj, NULL, TO_ROOM);
    damage_obj(obj);

    if (victim && !victim->fighting)
        set_fighting(victim, ch);

    return rNONE;
}

ch_ret spell_web(int sn, int level, CHAR_DATA *ch, void *vo)
{
    SKILLTYPE *skill = get_skilltype(sn);
    CHAR_DATA *victim = (CHAR_DATA *)vo;
    bool big = FALSE, failed = FALSE, pissed = FALSE;

    switch (GET_RACE(victim))
    {
    case RACE_GHOST:
    case RACE_SLIME:
    case RACE_ARACHNID:
        failed = TRUE;
    }

    if (saves_para_petri(level, victim))
        failed = TRUE;

    if (IS_ACT_FLAG(victim, ACT_HUGE))
        big = TRUE;

    if (room_is_inside(ch->in_room))
    {
        if (number_range(0, 4) < (failed ? (big ? 2 : 1) : 3))
            pissed = TRUE;
    }
    else
    {
        if (number_range(0, 4) < (failed ? (big ? 3 : 2) : (big ? 5 : 3)))
            pissed = TRUE;
    }

    if (!failed)
    {
        AFFECT_DATA af;

        af.type      = sn;
        af.duration  = (int)(level * DUR_CONV);
        af.bitvector = 0;
        af.location  = APPLY_MOVE;
        af.modifier  = -50;
        affect_to_char( victim, &af );

        successful_casting(skill, ch, victim, NULL);

        if (!pissed)
        {
            act(AT_MAGIC, "You are stuck in a sticky webbing!",
                ch, NULL, victim, TO_VICT);
            act(AT_MAGIC, "$N is stuck in a sticky webbing!",
                ch, NULL, victim,TO_NOTVICT);
            act(AT_MAGIC, "You wrap $N in a sticky webbing!",
                ch, NULL, victim, TO_CHAR);
        }
        else
        {
            act(AT_MAGIC, "You are wrapped in webs, but they don't stop you!",
                ch, NULL, victim, TO_VICT);
            act(AT_MAGIC, "$N attacks, paying little heed to the webs that slow it.",
                ch, NULL, victim, TO_NOTVICT);
            act(AT_MAGIC, "You only manage to piss off $N with your webs, ack!",
                ch, NULL, victim, TO_CHAR);
        }
    }
    else
    {
        if (pissed)
        {
            act(AT_MAGIC, "You are almost caught in a sticky webbing, GRRRR!",
                ch, NULL, victim, TO_VICT);
            act(AT_MAGIC, "$N growls and dodges $n's sticky webbing!",
                ch, NULL, victim,TO_NOTVICT);
            act(AT_MAGIC, "You miss $N with your sticky webbing!  Uh oh, it's mad.",
                ch, NULL, victim, TO_CHAR);
        }
        else
        {
            act(AT_MAGIC, "You watch with amusement as $n casts web about the room.",
                ch, NULL, victim, TO_VICT);
            act(AT_MAGIC, "$n misses $N with the webs!",
                ch, NULL, victim, TO_NOTVICT);
            act(AT_MAGIC, "You miss with your webs, but $N doesn't seem to notice.",
                ch, NULL, victim, TO_CHAR);
        }
    }

    if (!victim->fighting && pissed)
        set_fighting(victim, ch);

    return rNONE;
}

ch_ret spell_gust_of_wind(int sn, int level, CHAR_DATA *ch, void *vo)
{
    CHAR_DATA *vch, *vch_next;

    act(AT_MAGIC, "You wave your hands, and a gust of wind boils forth!", ch, NULL, NULL, TO_CHAR);
    act(AT_MAGIC, "$n sends a gust of wind towards you!", ch, NULL, NULL, TO_ROOM);

    for ( vch = ch->in_room->first_person; vch; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if ( IS_SET( vch->immune, RIS_MAGIC ) )
            continue;

        if ( ch == vch || IS_IMMORTAL(vch) || is_same_group(ch, vch) ||
             saves_spell_staff(level, vch) )
        {
            act(AT_MAGIC, "You were able to avoid the swirling gust.", ch, NULL, vch, TO_VICT);
            continue;
        }

        act(AT_MAGIC, "Your gust slams into $N.", ch, NULL, vch, TO_CHAR);
        vch->position = POS_SITTING;
        WAIT_STATE(vch, PULSE_VIOLENCE);
    }

    return rNONE;
}

ch_ret spell_haste(int sn, int level, CHAR_DATA *ch, void *vo)
{
    CHAR_DATA *victim = (CHAR_DATA *)vo;
    SKILLTYPE *skill = get_skilltype(sn);
    AFFECT_DATA af;

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rVICT_IMMUNE;
    }

    if (is_affected(victim, sn))
    {
        act(AT_MAGIC, "$N is already moving fast!", ch , NULL, victim, TO_CHAR);
        return rNONE;
    }

    if (IS_IMMUNE(victim, RIS_HOLD))
    {
        immune_casting(skill, ch, victim, NULL);
        if (ch!=victim && !is_same_group(ch, victim) && IS_NPC(victim))
            return damage(ch, victim, 0, sn);
        return rVICT_IMMUNE;
    }

    af.type      = sn;
    af.duration  = level * DUR_CONV;
    af.modifier  = victim->numattacks;
    af.location  = APPLY_NUMATTACKS;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    successful_casting(skill, ch, victim, NULL);

    /* find out a good way to age the char by 1 year */

    if (ch!=victim &&
        !is_same_group(ch, victim) &&
        victim->master != ch &&
        IS_NPC(victim))
        return damage(ch, victim, 0, sn);

    return rNONE;
}

ch_ret spell_slowness(int sn, int level, CHAR_DATA *ch, void *vo)
{
    CHAR_DATA *victim = (CHAR_DATA *)vo;
    SKILLTYPE *skill = get_skilltype(sn);
    AFFECT_DATA af;

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rVICT_IMMUNE;
    }

    if (is_affected(victim, sn))
    {
        act(AT_MAGIC, "$N is already slowed!", ch , NULL, victim, TO_CHAR);
        return rNONE;
    }

    if (IS_IMMUNE(victim, RIS_HOLD))
    {
        immune_casting(skill, ch, victim, NULL);
        if (ch!=victim && !is_same_group(ch, victim) && IS_NPC(victim))
            return damage(ch, victim, 0, sn);
        return rVICT_IMMUNE;
    }

    af.type      = sn;
    af.duration  = level * DUR_CONV;
    af.modifier  = -1;
    af.location  = APPLY_NUMATTACKS;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    successful_casting(skill, ch, victim, NULL);

    if (ch!=victim &&
        !is_same_group(ch, victim) &&
        victim->master != ch &&
        IS_NPC(victim))
        return damage(ch, victim, 0, sn);

    return rNONE;
}

ch_ret spell_calm(int sn, int level, CHAR_DATA *ch, void *vo)
{
    CHAR_DATA *victim = (CHAR_DATA *)vo;
    SKILLTYPE *skill = get_skilltype(sn);

    if (!IS_NPC(victim))
    {
        send_to_char("They are already calm.\n\r", ch);
        return rNONE;
    }

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rVICT_IMMUNE;
    }

    if (IS_ACT_FLAG(victim, ACT_AGGRESSIVE) &&
        HitOrMiss(ch, victim, CalcThaco(ch)))
    {
        REMOVE_ACT_FLAG(victim, ACT_AGGRESSIVE);
        successful_casting(skill, ch, victim, NULL);
        return rNONE;
    }

    failed_casting(skill, ch, victim, NULL);
    return rSPELL_FAILED;
}

ch_ret spell_fear(int sn, int level, CHAR_DATA *ch, void *vo)
{
    CHAR_DATA *victim = (CHAR_DATA *)vo;
    SKILLTYPE *skill = get_skilltype(sn);

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rVICT_IMMUNE;
    }

    if (GetMaxLevel(ch) < GetMaxLevel(victim)-2 ||
        saves_spell_staff(level, victim))
    {
        failed_casting(skill, ch, victim, NULL);
        return rSPELL_FAILED;
    }

    successful_casting(skill, ch, victim, NULL);
    do_flee(victim, NULL);

    return rNONE;
}

void stop_memorizing(CHAR_DATA *ch)
{
    AFFECT_DATA *paf, *paf_next;

    if (!is_affected(ch, gsn_memorize))
        return;

    send_to_char("You are interrupted from your memorizing.\n\r", ch);

    for ( paf = ch->first_affect; paf; paf = paf_next )
    {
        paf_next = paf->next;

        if (paf->type != gsn_memorize)
            continue;

        affect_remove( ch, paf );
    }

}

void do_forget(CHAR_DATA *ch, char *argument)
{
    char spell_name[MAX_INPUT_LENGTH];
    int sn, num;

    if (IS_NPC(ch) && !IS_ACT_FLAG(ch, ACT_POLYMORPHED))
        return;

    if (!*argument)
    {
        send_to_char("Forget what spell?\n\r", ch);
        return;
    }

    argument = one_argument(argument, spell_name);
    if ((sn=find_spell(ch, spell_name, TRUE)) < 0)
    {
        send_to_char("You don't know that spell.\n\r", ch);
        return;
    }

    if (!MEMORIZED(ch, sn))
    {
        send_to_char("You don't have that spell memorized.\n\r", ch);
        return;
    }

    num = atoi(argument);
    num = URANGE(1, num, MEMORIZED(ch, sn));

    ch->pcdata->memorized[sn] -= num;
    send_to_char("You forget a spell, but which one?\n\r", ch);
}

void do_memorize(CHAR_DATA *ch, char *argument)
{
    SKILLTYPE *skill = NULL;
    AFFECT_DATA af;
    char spell_name[MAX_INPUT_LENGTH];
    int sn, numtomem, duration, slev, x;

    if (IS_NPC(ch) && !IS_ACT_FLAG(ch, ACT_POLYMORPHED))
        return;

    if (!*argument)
    {
        int i;

        send_to_pager("Memorize 'spell name' [number to memorize]\n\r", ch);
        pager_printf(ch, "You can memorize one spell %d times, with a total of %d spells memorized.\n\r",
                     max_can_memorize_spell(ch,0), max_can_memorize(ch));
        pager_printf(ch, "You currently have %d spells memorized.\n\r",
                     total_memorized(ch));
        send_to_pager("Your spellbook holds these spells:\n\r", ch);


        for(i=0; i<MAX_SKILL; i++)
        {
            if (!MEMORIZED(ch, i))
                continue;

            if ( (skill=get_skilltype(i)) == NULL )
                continue;

            ch_printf(ch, "[%-2d] %-30s %-14s",
                      skill->skill_level[CLASS_SORCERER],
                      skill->name,
                      how_good(LEARNED(ch, i)));

            /*if (is_specialized(ch, i))
             send_to_char(" (special)", ch);*/


            ch_printf(ch, " x%d\n\r", MEMORIZED(ch, i));
        }
        return;
    }

    if (IS_SYSTEMFLAG(SYS_NOMAGIC))
    {
        send_to_char("You can't seem to memorize anything!  The Gods must be angry!\n\r", ch);
        return;
    }

    if (!IsHumanoid(ch) && !IsDragon(ch))
    {
        send_to_char("Sorry, you don't have the right form for that.\n\r", ch);
        return;
    }

    if (is_affected(ch, gsn_memorize))
    {
        send_to_char("You can only learn one spell at a time.\n\r",ch);
        return;
    }

    if (GET_POS(ch) > POS_RESTING)
    {
        send_to_char("You must be resting to memorize.\n\r", ch);
        return;
    }

    argument = one_argument(argument, spell_name);
    if ((sn=find_spell(ch, spell_name, TRUE)) < 0)
    {
        /* handle memorizing scrolls */
#if 0
        for (scroll=ch->carrying;
             scroll && scroll->obj_flags.type_flag!=ITEM_SCROLL;
             scroll=scroll->next_content) {
            if (scroll->obj_flags.value[1]==spl || scroll->obj_flags.value[2]==spl || scroll->obj_flags.value[3]==spl)
                break;
        }
        {
            send_to_char("You memorize a scroll which freezes and falls to the ground in bits.\n\r",ch);
            obj_from_char(scroll);
            extract_obj(scroll);
            scr=1;
            ch->skills[spl].learned += (int)(GET_INT(ch)/2);
        }
#endif
        send_to_char("You flip through your spell book but do not find that spell.\n\r",ch);
        return;
    }

    if (MEMORIZED(ch, sn) < 0)
    {
        send_to_char("You cannot do that right now...\n\r", ch);
        log_printf_plus(LOG_NORMAL, LEVEL_IMMORTAL, SEV_ERR,
                        "%s memorized < 0 of %s.", GET_NAME(ch), skill->name);

        return;
    }

    if ( (skill=get_skilltype(sn)) == NULL )
    {
        send_to_char( "You can't do that right now...\n\r", ch );
        return;
    }

    if (!IS_ACTIVE(ch, CLASS_SORCERER) ||
        GET_LEVEL(ch, CLASS_SORCERER) < skill->skill_level[CLASS_SORCERER])
    {
        send_to_char( "You can't memorize that.\n\r", ch );
        return;
    }

    numtomem = atoi(argument);
    numtomem = UMAX(1, numtomem);

    if (numtomem > max_can_memorize_spell(ch, sn)-MEMORIZED(ch, sn))
    {
        if (numtomem == 1)
            send_to_char("You cannot memorize that spell any further.\n\r", ch);
        else
            send_to_char("You cannot memorize that spell that much.\n\r", ch);
        return;
    }

    slev = skill->skill_level[CLASS_SORCERER];

    duration = numtomem;
    duration += ((GET_ADEPT(ch, sn) - LEARNED(ch, sn)) / 2);

    if (slev <= 5)
        duration = 0.1*(float)duration;
    else if (slev <= 10)
        duration = 0.3*(float)duration;
    else if (slev <= 25)
        duration = 0.5*(float)duration;
    else if (slev <= 45)
        duration = 0.7*(float)duration;
    else if (slev <= 47)
        duration = duration;
    else
        duration = 1.5*(float)duration;

#ifdef MUD_DEBUG
    log_printf_plus(LOG_DEBUG, LEVEL_IMMORTAL, SEV_DEBUG, "do_memorize: %s duration %d learned %d", GET_NAME(ch), duration, LEARNED(ch, sn));
#endif

    for (x=0; x<numtomem; x++)
    {
        af.type     = gsn_memorize;
        af.duration = duration+x;
        af.modifier = sn;
        af.location = APPLY_NUMMEM;
        af.bitvector = 0;
        affect_to_char(ch, &af);
    }

    send_to_char("You flip open your spell book then begin to read and meditate.\n\r", ch);
    act(AT_MAGIC, "$n opens a spell book then begins to read and meditate.", ch, NULL, NULL, TO_ROOM);
}

ch_ret spell_remove_curse(int sn, int level, CHAR_DATA *ch, void *vo)
{
    OBJ_DATA *obj = NULL;
    CHAR_DATA *victim = NULL;
    SKILLTYPE *skill = get_skilltype(sn);
    bool found = FALSE;

    if ((obj = get_obj_carry(ch, target_name)))
    {
        if (IS_OBJ_STAT(obj, ITEM_NODROP))
        {
            act(AT_MAGIC, "$p briefly glows blue.", ch, obj, NULL, TO_CHAR);
            act(AT_MAGIC, "$p, held by $n, briefly glows blue.", ch, obj, NULL, TO_ROOM);
            REMOVE_OBJ_STAT(obj, ITEM_NODROP);
            return rNONE;
        }
        if (IS_OBJ_STAT(obj, ITEM_NOREMOVE))
        {
            act(AT_MAGIC, "$p briefly glows green.", ch, obj, NULL, TO_CHAR);
            act(AT_MAGIC, "$p, held by $n, briefly glows green.", ch, obj, NULL, TO_ROOM);
            REMOVE_OBJ_STAT(obj, ITEM_NOREMOVE);
            return rNONE;
        }
        act(AT_MAGIC, "Nothing noticable happened...", ch, NULL, NULL, TO_CHAR);
        return rNONE;
    }

    if (!(victim = get_char_room(ch, target_name)))
    {
        act(AT_MAGIC, "Nobody here by that name.", ch, NULL, NULL, TO_CHAR);
        return rNONE;
    }

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
        immune_casting( skill, ch, victim, NULL );
        return rVICT_IMMUNE;
    }

    if (is_affected(victim, gsn_curse))
    {
        AFFECT_DATA *paf;

        act(AT_MAGIC, "$n briefly glows red, then blue.", victim, NULL, NULL, TO_ROOM);
        act(AT_MAGIC, "You feel somehow ... different...", victim, NULL, NULL, TO_CHAR);

        while ((paf=is_affected(victim, gsn_curse)))
            affect_remove(victim,paf);

        return rNONE;
    }

    for ( obj = victim->last_carrying; obj; obj = obj->prev_content )
    {
        if (IS_OBJ_STAT(obj, ITEM_NODROP))
        {
            act(AT_MAGIC, "$p briefly glows blue.", victim, obj, NULL, TO_CHAR);
            act(AT_MAGIC, "$p, held by $n, briefly glows blue.", victim, obj, NULL, TO_ROOM);
	    REMOVE_OBJ_STAT(obj, ITEM_NODROP);
            found = TRUE;
        }
        if (IS_OBJ_STAT(obj, ITEM_NOREMOVE))
        {
            act(AT_MAGIC, "$p briefly glows green.", victim, obj, NULL, TO_CHAR);
            act(AT_MAGIC, "$p, held by $n, briefly glows green.", victim, obj, NULL, TO_ROOM);
	    REMOVE_OBJ_STAT(obj, ITEM_NOREMOVE);
            found = TRUE;
        }
    }

    if (!found)
	act(AT_MAGIC, "Nothing noticable happened...", ch, NULL, NULL, TO_CHAR);
    return rNONE;
}


ch_ret spell_animal_friendship(int sn, int level, CHAR_DATA *ch, void *vo)
{
    CHAR_DATA *victim = (CHAR_DATA *)vo;
    SKILLTYPE *skill = get_skilltype(sn);
    AFFECT_DATA *paf;

    if (!IsAnimal(victim) || IS_IMMUNE(victim, RIS_CHARM))
    {
        immune_casting(skill, ch, victim, NULL);
        return rVICT_IMMUNE;
    }

    if (saves_spell_staff(level, victim) ||
        IS_AFFECTED(ch, AFF_CHARM) ||
        circle_follow( victim, ch ) ||
        GetMaxLevel(victim) > 10+GetMaxLevel(ch))
    {
        failed_casting(skill, ch, victim, NULL);
        return rSPELL_FAILED;
    }

    successful_casting( skill, ch, victim, NULL );

    if (too_many_followers(ch))
    {
        act(AT_MAGIC, "$N takes one look at the size of your posse and just says no!", ch, NULL, victim, TO_CHAR);
        act(AT_MAGIC, "$N takes one look at the size of $n's posse and just says no!", ch, NULL, victim, TO_ROOM);
    }
    else
        make_charmie(ch, victim, sn);

    for (paf = victim->first_affect; paf; paf = paf->next)
        if (paf->type == sn)
            paf->duration = (int)(DUR_CONV * 24);

    return rNONE;
}

ch_ret spell_charm_vegetable(int sn, int level, CHAR_DATA *ch, void *vo)
{
    CHAR_DATA *victim = (CHAR_DATA *)vo;
    SKILLTYPE *skill = get_skilltype(sn);
    AFFECT_DATA *paf;

    if (!IsVeggie(victim) || IS_IMMUNE(victim, RIS_CHARM))
    {
        immune_casting(skill, ch, victim, NULL);
        return rVICT_IMMUNE;
    }

    if (saves_para_petri(level, victim) ||
        IS_AFFECTED(ch, AFF_CHARM) ||
        circle_follow( victim, ch ) ||
        GetMaxLevel(victim) > 10+GetMaxLevel(ch))
    {
        failed_casting(skill, ch, victim, NULL);
        return rSPELL_FAILED;
    }

    successful_casting( skill, ch, victim, NULL );

    if (too_many_followers(ch))
    {
        act(AT_MAGIC, "$N takes one look at the size of your posse and just says no!", ch, NULL, victim, TO_CHAR);
        act(AT_MAGIC, "$N takes one look at the size of $n's posse and just says no!", ch, NULL, victim, TO_ROOM);
    }
    else
        make_charmie(ch, victim, sn);

    for (paf = victim->first_affect; paf; paf = paf->next)
        if (paf->type == sn)
            paf->duration = (int)(DUR_CONV * 24);

    return rNONE;
}

ch_ret spell_shillelagh(int sn, int level, CHAR_DATA *ch, void *vo)
{
    OBJ_DATA *obj = (OBJ_DATA *)vo;
    AFFECT_DATA *paf;
    SKILLTYPE *skill = get_skilltype(sn);

    separate_obj(obj);

    if (obj->item_type != ITEM_WEAPON ||
        !is_name("club", obj->name))
    {
        send_to_char("You can only cast that on clubs.\n\r", ch);
        return rNONE;
    }

    if (IS_OBJ_STAT(obj, ITEM_MAGIC))
    {
        immune_casting( skill, ch, NULL, obj );
        return rNONE;
    }

    successful_casting( skill, ch, NULL, obj );

    SET_OBJ_STAT(obj, ITEM_MAGIC);
    SET_OBJ_STAT(obj, ITEM_GLOW);

    if (obj->value[1] < 2)
        obj->value[1] = 2;
    if (obj->value[2] < 4)
        obj->value[2] = 4;

    for ( paf = obj->first_affect; paf; paf = paf->next )
        if (paf->location == APPLY_HITROLL)
        {
            paf->modifier += 1;
            break;
        }

    if (!paf || paf->location != APPLY_HITROLL)
    {
        CREATE( paf, AFFECT_DATA, 1 );
        paf->type       = sn;
        paf->duration	= -1;
        paf->location	= APPLY_HITROLL;
        paf->modifier	= 1;
        paf->bitvector	= 0;
        LINK( paf, obj->first_affect, obj->last_affect, next, prev );
    }

    for ( paf = obj->first_affect; paf; paf = paf->next )
        if (paf->location == APPLY_DAMROLL)
        {
            paf->modifier += 1;
            break;
        }

    if (!paf || paf->location != APPLY_DAMROLL)
    {
        CREATE( paf, AFFECT_DATA, 1 );
        paf->type       = sn;
        paf->duration	= -1;
        paf->location	= APPLY_DAMROLL;
        paf->modifier	= 1;
        paf->bitvector	= 0;
        LINK( paf, obj->first_affect, obj->last_affect, next, prev );
    }

    return rNONE;
}

ch_ret spell_changestaff(int sn, int level, CHAR_DATA *ch, void *vo)
{
    CHAR_DATA *tree;
    OBJ_DATA *obj = (OBJ_DATA *)vo;
    AFFECT_DATA *paf;
    SKILLTYPE *skill = get_skilltype(sn);

    separate_obj(obj);

    if (obj->item_type != ITEM_STAFF)
    {
        send_to_char("You can only cast that on staves.\n\r", ch);
        return rNONE;
    }

    if (!obj->value[2])
    {
        failed_casting( skill, ch, NULL, obj );
        return rSPELL_FAILED;
    }

    if (!(tree = create_mobile(MOB_VNUM_CHANGESTAFF_TREE)))
    {
        log_printf_plus(LOG_NORMAL, LEVEL_IMMORTAL, SEV_ERR,
                        "changestaff mob #%d does not exist.", MOB_VNUM_CHANGESTAFF_TREE);
        send_to_char( "Please report to an immortal, there are no trees.\n\r", ch );
        return rNONE;
    }

    char_to_room(tree, ch->in_room);

    successful_casting( skill, ch, tree, obj );

    if (too_many_followers(ch))
    {
        act(AT_MAGIC, "$N takes one look at the size of your posse and just says no!", ch, NULL, tree, TO_CHAR);
        act(AT_MAGIC, "$N takes one look at the size of $n's posse and just says no!", ch, NULL, tree, TO_ROOM);
    }
    else
        make_charmie(ch, tree, sn);

    for (paf = tree->first_affect; paf; paf = paf->next)
        if (paf->type == sn)
            paf->duration = (int)(DUR_CONV * obj->value[2]);

    extract_obj(obj);

    mprog_birth_trigger(ch, tree);

    return rNONE;
}

ch_ret spell_acid_rain(int sn, int level, CHAR_DATA *ch, void *vo)
{
    SKILLTYPE *skill = get_skilltype(sn);
    AREA_DATA *tarea;
    ROOM_INDEX_DATA *room, *was_in_room;
    CHAR_DATA *vch, *vch_next;
    int vnum, dam;

    tarea = ch->in_room->area;

    dam = dice(level/4, 2);

    for ( vnum = tarea->low_r_vnum; vnum <= tarea->hi_r_vnum; vnum++ )
    {
        if ( (room = get_room_index( vnum )) == NULL )
            continue;

        if (room_is_inside(room) ||
            IS_ROOM_FLAG(room, ROOM_NO_MAGIC))
            continue;

        was_in_room = ch->in_room;
        ch->in_room = room;
        act( AT_MAGIC, skill->imm_room, ch, NULL, NULL, TO_ROOM );
        ch->in_room = was_in_room;

        for (vch = room->first_person; vch; vch = vch_next)
        {
            vch_next = vch->next_in_room;

            if (!IS_NPC(vch) ||
		char_died(vch) ||
                saves_spell_staff(level, vch))
                continue;

            act( AT_MAGIC, skill->hit_vict, ch, NULL, vch, TO_VICT );
            damage( ch, vch, dam, sn );
        }
    }
    act( AT_MAGIC, skill->hit_char, ch, NULL, NULL, TO_CHAR );
    act( AT_MAGIC, skill->hit_room, ch, NULL, NULL, TO_ROOM );

    return rNONE;
}

ch_ret spell_prismatic_spray(int sn, int level, CHAR_DATA *ch, void *vo)
{
    SKILLTYPE *skill = get_skilltype(sn);
    CHAR_DATA *vch, *vch_next;

    /* spell effects
       red     1 40 dam
       orange  2 80
       yellow  3 100
       green   4 poison
       blue    5 petrify
       indigo  6 feeble
       violet  7 teleport
     */

    successful_casting( skill, ch, NULL, NULL );

    for ( vch = ch->in_room->first_person; vch; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if (ch == vch || IS_IMMORTAL(vch) || char_died(vch) ||
            is_same_group(ch, vch) || saves_spell_staff(level, vch))
            continue;

        switch(number_range(1,7))
        {
        case 1:
            act(AT_MAGIC, "$n is hit by a red shaft of light!", vch, NULL, NULL, TO_ROOM);
            damage( ch, vch, 40, sn );
            break;
        case 2:
            act(AT_MAGIC, "$n is hit by a orange shaft of light!", vch, NULL, NULL, TO_ROOM);
            damage( ch, vch, 80, sn );
            break;
        case 3:
            act(AT_MAGIC, "$n is hit by a yellow shaft of light!", vch, NULL, NULL, TO_ROOM);
            damage( ch, vch, 100, sn );
            break;
        case 4:
            act(AT_MAGIC, "$n is hit by a green shaft of light!", vch, NULL, NULL, TO_ROOM);
            spell_poison(sn, level, ch, vch );
            break;
        case 5:
            act(AT_MAGIC, "$n is hit by a blue shaft of light!", vch, NULL, NULL, TO_ROOM);
            spell_smaug( gsn_paralyze, level, ch, vch );
            break;
        case 6:
            {
                int fsn;
                if ((fsn = skill_lookup("feeblemind")))
                {
                    act(AT_MAGIC, "$N is hit by a indigo shaft of light!", ch, NULL, vch, TO_ROOM);
                    spell_smaug( fsn, level, ch, vch );
                    break;
                }
            }
        case 7:
            act(AT_MAGIC, "$N is hit by a violet shaft of light!", ch, NULL, vch, TO_ROOM);
            spell_teleport(sn, level, ch, vch );
            break;
        }
    }

    return rNONE;
}

ch_ret spell_holy_word(int sn, int level, CHAR_DATA *ch, void *vo)
{
    SKILLTYPE *skill = get_skilltype(sn);
    CHAR_DATA *vch, *vch_next;
    int lev;

    if ((IS_GOOD(ch) && !str_cmp(skill->name, "unholy word")) ||
        (IS_EVIL(ch) && !str_cmp(skill->name, "holy word")))
        return rSPELL_FAILED;

    for ( vch = ch->in_room->first_person; vch; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if (ch == vch || IS_IMMORTAL(vch) || char_died(vch) ||
            is_same_group(ch, vch) || saves_spell_staff(level, vch))
            continue;

        if ((IS_GOOD(ch) && IS_GOOD(vch)) ||
            (IS_EVIL(ch) && IS_EVIL(vch)))
            continue;

        lev = GetMaxLevel(vch);

        successful_casting( skill, ch, vch, NULL );

        if (lev <= 4)
            damage( ch, vch, GET_MAX_HIT(vch)*10, sn );
        else if (lev <= 8)
        {
            damage( ch, vch, 1, sn );
            spell_smaug( gsn_paralyze, level, ch, vch );
        }
        else if (lev <= 12)
        {
            damage( ch, vch, 1, sn );
            spell_blindness(sn, level, ch, vch );
         }
        else if (lev <= 16)
        {
            damage( ch, vch, 1, sn );
            vch->position = POS_STUNNED;
        }
        else
            damage( ch, vch, 1, sn );
    }

    return rNONE;
}

ch_ret spell_find_familiar(int sn, int level, CHAR_DATA *ch, void *vo)
{
    SKILLTYPE *skill = get_skilltype(sn);
    CHAR_DATA *mob;
    AFFECT_DATA af;
    int vnum;

    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE) ||
        IS_SET(ch->in_room->room_flags, ROOM_PRIVATE) ||
        IS_SET(ch->in_room->room_flags, ROOM_SOLITARY) ||
        IS_SET(ch->in_room->room_flags, ROOM_NO_SUMMON))
    {
        send_to_char("Ancient Magiks bar your path.\n\r", ch);
        if (IS_IMMORTAL(ch))
            send_to_char("However, your immortality overcomes them.\n\r", ch);
        else
            return rSPELL_FAILED;
    }

    if (level < 2)
        vnum = MOB_VNUM_FAMILIAR_FIRST;
    else if (level < 4)
        vnum = MOB_VNUM_FAMILIAR_SECOND;
    else if (level < 6)
        vnum = MOB_VNUM_FAMILIAR_THIRD;
    else if (level < 8)
        vnum = MOB_VNUM_FAMILIAR_FOURTH;
    else
        vnum = MOB_VNUM_FAMILIAR_FIFTH;

    if ( (mob=create_mobile(vnum)) == NULL )
    {
        failed_casting( skill, ch, NULL, NULL );
        return rNONE;
    }

    af.type      = sn;
    af.duration  = (int)(24 * DUR_CONV);
    af.bitvector = 0;
    af.location  = APPLY_AC;
    af.modifier  = -5;
    affect_to_char( ch, &af );

    successful_casting( skill, ch, mob, NULL );
    char_to_room( mob, ch->in_room );

    if (too_many_followers(ch))
    {
        act(AT_MAGIC, "$N takes one look at the size of your posse and just says no!", ch, NULL, mob, TO_CHAR);
        act(AT_MAGIC, "$N takes one look at the size of $n's posse and just says no!", ch, NULL, mob, TO_ROOM);
    }
    else
        make_charmie(ch, mob, sn);

    mprog_birth_trigger(ch, mob);

    return rNONE;
}

ch_ret spell_dust_devil(int sn, int level, CHAR_DATA *ch, void *vo)
{
    SKILLTYPE *skill = get_skilltype(sn);
    CHAR_DATA *mob;
    AFFECT_DATA af;

    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE) ||
        IS_SET(ch->in_room->room_flags, ROOM_PRIVATE) ||
        IS_SET(ch->in_room->room_flags, ROOM_SOLITARY) ||
        IS_SET(ch->in_room->room_flags, ROOM_NO_SUMMON))
    {
        send_to_char("Ancient Magiks bar your path.\n\r", ch);
        if (IS_IMMORTAL(ch))
            send_to_char("However, your immortality overcomes them.\n\r", ch);
        else
            return rSPELL_FAILED;
    }

    if (room_is_inside(ch->in_room))
    {
        send_to_char("You cannot cast this indoors.\n\r", ch);
        return rSPELL_FAILED;
    }


    if ( (mob=create_mobile(MOB_VNUM_DUST_DEVIL)) == NULL )
    {
        failed_casting( skill, ch, NULL, NULL );
        return rNONE;
    }

    af.type      = sn;
    af.duration  = (int)(24 * DUR_CONV);
    af.bitvector = 0;
    af.location  = APPLY_AC;
    af.modifier  = -1;
    affect_to_char( ch, &af );

    successful_casting( skill, ch, mob, NULL );
    char_to_room( mob, ch->in_room );

    if (too_many_followers(ch))
    {
        act(AT_MAGIC, "$N takes one look at the size of your posse and just says no!", ch, NULL, mob, TO_CHAR);
        act(AT_MAGIC, "$N takes one look at the size of $n's posse and just says no!", ch, NULL, mob, TO_ROOM);
    }
    else
        make_charmie(ch, mob, sn);

    mprog_birth_trigger(ch, mob);

    return rNONE;
}

ch_ret spell_disintegrate(int sn, int level, CHAR_DATA *ch, void *vo)
{
    SKILLTYPE *skill = get_skilltype(sn);
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj, *prev_obj;

    prev_obj = victim->last_carrying;

    if ( is_safe( ch, victim ) )
    {
	failed_casting( skill, ch, victim, NULL );
        return rNONE;
    }

    if ( !in_arena( victim ) )
	while ( (obj = prev_obj) )
	{
	    separate_obj(obj);
	    prev_obj = obj->prev_content;
	    if (ItemSave(obj, sn))
	    {
		act(AT_GREEN, "$p resists the disintegration ray completely!", ch, obj, victim, TO_VICT);
		act(AT_GREEN, "$p carried by $N, resists $n's disintegration ray!", ch, obj, victim, TO_NOTVICT);
		continue;
	    }

	    act(AT_GREEN, "$p turns red hot, then it disappears in a puff of smoke!", ch, obj, victim, TO_CHAR);
	    act(AT_GREEN, "$p turns red hot, then it disappears in a puff of smoke!", ch, obj, victim, TO_NOTVICT);
	    act(AT_GREEN, "$p turns red hot, then it disappears in a puff of smoke!", ch, obj, victim, TO_VICT);
	    extract_obj(obj);
	}

    return damage(ch, victim, dice(level, 10), sn);
}

ch_ret spell_flame_blade(int sn, int level, CHAR_DATA *ch, void *vo)
{
    OBJ_DATA *blade;
    AFFECT_DATA *paf;
    SKILLTYPE *skill = get_skilltype(sn);

    if (get_eq_char( ch, WEAR_DUAL_WIELD ))
    {
        send_to_char("You can't be wielding a weapon.\n\r", ch);
        return rNONE;
    }

    blade = create_object( OBJ_VNUM_MINORC_LONG_SWORD );

    if (!blade)
    {
        failed_casting( skill, ch, NULL, NULL );
        return rNONE;
    }

    STRFREE( blade->name );
    blade->name = STRALLOC( "druid flame blade" );

    STRFREE( blade->short_descr );
    blade->short_descr = STRALLOC( "a flame blade" );

    STRFREE( blade->description );
    blade->description = STRALLOC( "A flame blade burns brightly here." );

    blade->value[0] = INIT_WEAPON_CONDITION;
    blade->value[1] = 1;
    blade->value[2] = 4;

    SET_BIT(blade->extra_flags, ITEM_MAGIC);

    CREATE( paf, AFFECT_DATA, 1 );
    paf->type		= -1;
    paf->duration	= -1;
    paf->location	= APPLY_DAMROLL;
    paf->modifier	= 4+GET_LEVEL(ch, CLASS_DRUID)/8;
    paf->bitvector	= 0;
    LINK( paf, blade->first_affect, blade->last_affect, next, prev );

    blade = obj_to_char( blade, ch );
    equip_char(ch, blade, WEAR_WIELD);

    successful_casting( skill, ch, NULL, blade );

    return rNONE;
}

ch_ret spell_elemental_servant(int sn, int level, CHAR_DATA *ch, void *vo)
{
    SKILLTYPE *skill = get_skilltype(sn);
    CHAR_DATA *mob;
    AFFECT_DATA af;
    int vnum;

    if (IS_SET(ch->in_room->room_flags, ROOM_SAFE) ||
        IS_SET(ch->in_room->room_flags, ROOM_PRIVATE) ||
        IS_SET(ch->in_room->room_flags, ROOM_SOLITARY) ||
        IS_SET(ch->in_room->room_flags, ROOM_NO_SUMMON))
    {
        send_to_char("Ancient Magiks bar your path.\n\r", ch);
        if (IS_IMMORTAL(ch))
            send_to_char("However, your immortality overcomes them.\n\r", ch);
        else
            return rSPELL_FAILED;
    }

    if (!str_cmp(skill->name, "fire servant"))
        vnum = MOB_VNUM_FIRE_SERVANT;
    else if (!str_cmp(skill->name, "wind servant"))
        vnum = MOB_VNUM_WIND_SERVANT;
    else if (!str_cmp(skill->name, "water servant"))
        vnum = MOB_VNUM_WATER_SERVANT;
    else if (!str_cmp(skill->name, "earth servant"))
        vnum = MOB_VNUM_EARTH_SERVANT;
    else
        vnum = MOB_VNUM_FIRE_SERVANT;

    if ( (mob=create_mobile(vnum)) == NULL )
    {
        failed_casting( skill, ch, NULL, NULL );
        return rNONE;
    }

    af.type      = sn;
    af.duration  = (int)(24 * DUR_CONV);
    af.bitvector = 0;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    affect_to_char( ch, &af );

    successful_casting( skill, ch, mob, NULL );
    char_to_room( mob, ch->in_room );

    if (too_many_followers(ch))
    {
        act(AT_MAGIC, "$N takes one look at the size of your posse and just says no!", ch, NULL, mob, TO_CHAR);
        act(AT_MAGIC, "$N takes one look at the size of $n's posse and just says no!", ch, NULL, mob, TO_ROOM);
    }
    else
        make_charmie(ch, mob, sn);

    mprog_birth_trigger(ch, mob);

    return rNONE;
}


ch_ret spell_command(int sn, int level, CHAR_DATA *ch, void *vo)
{

    return rNONE;
}


ch_ret spell_silence(int sn, int level, CHAR_DATA *ch, void *vo)
{

    return rNONE;
}



