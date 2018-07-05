/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer and Heath Leach
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*static char rcsid[] = "$Id: multclas.c,v 1.22 2004/04/06 22:00:10 dotd Exp $";*/

#include <stdio.h>
#include <string.h>

#include "mud.h"

sh_int LastActive(CHAR_DATA *ch)
{
    sh_int i;

    for (i = MAX_CLASS-1; i >= FIRST_CLASS; --i)
        if (IS_ACTIVE(ch, i))
            return(i);
    bug("%s has no class in LastActive!", GET_NAME(ch));
    return(CLASS_NONE);
}

int GetClassLevel(CHAR_DATA *ch, sh_int cl)
{
    if (IS_ACTIVE(ch, cl))
        return(GET_LEVEL(ch, cl));
    return(0);
}

bool OnlyClass(CHAR_DATA *ch, sh_int cl)
{
    sh_int i;

    for (i=FIRST_CLASS; i<MAX_CLASS; ++i)
        if (GetClassLevel(ch, i) != 0)
            if (i != cl)
                return(FALSE);
    return(TRUE);
}


int HowManyClasses(CHAR_DATA *ch)
{
    sh_int i;
    int tot=0;

    for (i=FIRST_CLASS;i<MAX_CLASS;i++)
        if (IS_ACTIVE(ch, i) && !HAD_CLASS(ch, i))
            tot++;

    return(UMAX(1,tot));
}

int HowManyClassesPlus(CHAR_DATA *ch)
{
    sh_int i;
    int tot=0;

    for (i=FIRST_CLASS;i<MAX_CLASS;i++)
    {
        if (IS_ACTIVE(ch, i) || HAD_CLASS(ch, i))
        {
            tot++;
        }
    } /* end for */

    return(tot);
}


int BestFightingClass(CHAR_DATA *ch)
{
    if (IS_ACTIVE(ch, CLASS_BARBARIAN))
        return(CLASS_BARBARIAN);
    if (IS_ACTIVE(ch, CLASS_PALADIN))
        return(CLASS_PALADIN);
    if (IS_ACTIVE(ch, CLASS_ANTIPALADIN))
        return(CLASS_ANTIPALADIN);
    if (IS_ACTIVE(ch, CLASS_WARRIOR))
        return(CLASS_WARRIOR);
    if (IS_ACTIVE(ch, CLASS_AMAZON))
        return(CLASS_AMAZON);
    if (IS_ACTIVE(ch, CLASS_RANGER))
        return(CLASS_RANGER);
    if (IS_ACTIVE(ch, CLASS_MONK))
        return(CLASS_MONK);
    if (IS_ACTIVE(ch, CLASS_VAMPIRE))
        return(CLASS_VAMPIRE);
    if (IS_ACTIVE(ch, CLASS_CLERIC))
        return(CLASS_CLERIC);
    if (IS_ACTIVE(ch, CLASS_DRUID))
        return(CLASS_DRUID);
    if (IS_ACTIVE(ch, CLASS_THIEF))
        return(CLASS_THIEF);
    if (IS_ACTIVE(ch, CLASS_ARTIFICER))
        return(CLASS_ARTIFICER);
    if (IS_ACTIVE(ch, CLASS_PSIONIST))
        return(CLASS_PSIONIST);
    if (IS_ACTIVE(ch, CLASS_NECROMANCER))
        return(CLASS_NECROMANCER);
    if (IS_ACTIVE(ch, CLASS_SORCERER))
        return(CLASS_SORCERER);
    if (IS_ACTIVE(ch, CLASS_MAGE))
        return(CLASS_MAGE);

    log_printf_plus(LOG_BUG, LEVEL_LOG_CSET, SEV_ERR, "Error in BestFightingClass, %s has no class! Returning first active class\n\r", GET_NAME(ch));
    return(FirstActive(ch));
}

int BestThiefClass(CHAR_DATA *ch)
{
    if (IS_ACTIVE(ch, CLASS_THIEF))
        return(CLASS_THIEF);
    if (IS_ACTIVE(ch, CLASS_RANGER))
        return(CLASS_RANGER);
    if (IS_ACTIVE(ch, CLASS_VAMPIRE))
        return(CLASS_VAMPIRE);
    if (IS_ACTIVE(ch, CLASS_AMAZON))
        return(CLASS_AMAZON);
    if (IS_ACTIVE(ch, CLASS_SORCERER))
        return(CLASS_SORCERER);
    if (IS_ACTIVE(ch, CLASS_MAGE))
        return(CLASS_MAGE);
    if (IS_ACTIVE(ch, CLASS_WARRIOR))
        return(CLASS_WARRIOR);
    if (IS_ACTIVE(ch, CLASS_DRUID))
        return(CLASS_DRUID);
    if (IS_ACTIVE(ch, CLASS_CLERIC))
        return(CLASS_CLERIC);

    sprintf(log_buf, "Error in BestThiefClass, %s has no class!", GET_NAME(ch));
    log_string_plus(log_buf, LOG_BUG, LEVEL_LOG_CSET, SEV_ERR);
    log_string_plus("Returning first active class", LOG_BUG, LEVEL_LOG_CSET, SEV_ERR);
    return(FirstActive(ch));

}

int BestMagicClass(CHAR_DATA *ch)
{
    if (IS_ACTIVE(ch, CLASS_SORCERER))
        return(CLASS_SORCERER);
    if (IS_ACTIVE(ch, CLASS_MAGE))
        return(CLASS_MAGE);
    if (IS_ACTIVE(ch, CLASS_CLERIC))
        return(CLASS_CLERIC);
    if (IS_ACTIVE(ch, CLASS_DRUID))
        return(CLASS_DRUID);
    if (IS_ACTIVE(ch, CLASS_ARTIFICER))
        return(CLASS_ARTIFICER);
    if (IS_ACTIVE(ch, CLASS_NECROMANCER))
        return(CLASS_NECROMANCER);
    if (IS_ACTIVE(ch, CLASS_VAMPIRE))
        return(CLASS_VAMPIRE);
    if (IS_ACTIVE(ch, CLASS_PALADIN))
        return(CLASS_PALADIN);
    if (IS_ACTIVE(ch, CLASS_ANTIPALADIN))
        return(CLASS_ANTIPALADIN);
    if (IS_ACTIVE(ch, CLASS_RANGER))
        return(CLASS_RANGER);
    if (IS_ACTIVE(ch, CLASS_MONK))
        return(CLASS_MONK);
    if (IS_ACTIVE(ch, CLASS_PSIONIST))
        return(CLASS_PSIONIST);
    if (IS_ACTIVE(ch, CLASS_THIEF))
        return(CLASS_THIEF);
    if (IS_ACTIVE(ch, CLASS_AMAZON))
        return(CLASS_AMAZON);
    if (IS_ACTIVE(ch, CLASS_WARRIOR))
        return(CLASS_WARRIOR);
    if (IS_ACTIVE(ch, CLASS_BARBARIAN))
        return(CLASS_BARBARIAN);

    sprintf(log_buf, "Error in BestMagicClass, %s has no class!", GET_NAME(ch));
    log_string_plus(log_buf, LOG_BUG, LEVEL_LOG_CSET, SEV_ERR);
    log_string_plus("Returning first active class", LOG_BUG, LEVEL_LOG_CSET, SEV_ERR);
    return(FirstActive(ch));
}

int GetALevel(CHAR_DATA *ch, int which)
{
    int ind[REAL_MAX_CLASS],k;
    sh_int i, j;

    for (i=FIRST_CLASS; i<MAX_CLASS; i++) {
        ind[i] = GET_LEVEL(ch,i);
    }

    for (i = FIRST_CLASS; i< MAX_CLASS-1; i++) {
        for (j=i+1;j< MAX_CLASS;j++) {
            if (ind[j] > ind[i]) {
                k = ind[i];
                ind[i] = ind[j];
                ind[j] = k;
            }
        }
    }

    if (which > -1 && which < 4) {
        return(ind[which]);
    }
    return(0);
}

int GetSecMaxLev(CHAR_DATA *ch)
{
    return(GetALevel(ch, 2));
}

int GetThirdMaxLev(CHAR_DATA *ch)
{
    return(GetALevel(ch, 3));
}

int GetMaxLevel(CHAR_DATA *ch)
{
    int max = 0;
    sh_int i;

    for (i=FIRST_CLASS; i<MAX_CLASS; i++)
        if (IS_ACTIVE(ch, i))
            max = UMAX(max, GET_LEVEL(ch, i));

    if (ch && IS_NPC(ch))
        max = UMAX(1,max);

    return(max);
}

int GetMinLevel(CHAR_DATA *ch)
{
    int min = MAX_LEVEL;
    sh_int i;

    for (i=FIRST_CLASS; i<MAX_CLASS; i++)
        if (IS_ACTIVE(ch, i))
            min = UMIN(min, GET_LEVEL(ch, i));

    if (ch && IS_NPC(ch))
        min = UMIN(MAX_LEVEL,min);

    return(min);
}

int MIGetMaxLevel(MOB_INDEX_DATA *ch)
{
    int max = 0;
    sh_int i;

    for (i = FIRST_CLASS; i < MAX_CLASS; i++)
        if (GET_LEVEL(ch, i) > max)
            max = GET_LEVEL(ch,i);

    return(max);
}

int GetMaxClass(CHAR_DATA *ch)
{
    sh_int max=0, i;

    for (i = FIRST_CLASS; i< MAX_CLASS; i++) {
        if (GET_LEVEL(ch, i) > GET_LEVEL(ch, max))
            max = i;
    }

    return(max);
}

int GetAveLevel(CHAR_DATA *ch)
{
    int tot=0, cnt=0;
    sh_int i;

    for (i=FIRST_CLASS; i< MAX_CLASS; i++) {
        if (IS_ACTIVE(ch, i)) {
            tot += GET_LEVEL(ch,i);
            ++cnt;
        }
    }

    if (cnt==0)
        return(0);

    tot /= cnt;
    return(tot);
}

int GetTotLevel(CHAR_DATA *ch)
{
    int max=0;
    sh_int i;

    for (i=FIRST_CLASS; i< MAX_CLASS; i++)
        if (IS_ACTIVE(ch, i))
            max += GET_LEVEL(ch,i);

    return(max);

}

void StartLevels(CHAR_DATA *ch)
{
    sh_int i;

    for (i=FIRST_CLASS; i < MAX_CLASS; ++i) {
        if (IS_ACTIVE(ch, i)) {
            GET_LEVEL(ch, i) = 1;
        } else {
            GET_LEVEL(ch, i) = 0;
        }
    }

    ch->trust = 0;

}

char *GetClassString(CHAR_DATA *ch)
{
    static char buf[MAX_STRING_LENGTH];
    sh_int i;

    buf[0] = '\0';

    for (i = FIRST_CLASS; i < MAX_CLASS; ++i) {
        if (IS_ACTIVE(ch, i) && !HAD_CLASS(ch, i)) {
            strcat(buf, short_pc_class[i]);
            strcat(buf, "/");
        }
    }

    if (strlen(buf))
        buf[strlen(buf)-1] = '\0';

    return(buf);
}

char *GetLevelString(CHAR_DATA *ch)
{
    static char buf[MAX_STRING_LENGTH];
    char tmpbuf[8];
    sh_int i;

    buf[0] = '\0';

    for (i = FIRST_CLASS; i < MAX_CLASS; ++i) {
        if (IS_ACTIVE(ch, i) && !HAD_CLASS(ch, i)) {
            sprintf(tmpbuf, "%d", ch->levels[i]);
            strcat(buf, tmpbuf);
            strcat(buf, "/");
        }
    }

    if (strlen(buf))
        buf[strlen(buf)-1] = '\0';

    return(buf);
}

char *GetTitleString(CHAR_DATA *ch)
{
#if 0
    return("Title string disabled for now");
#else
    static char buf[MAX_STRING_LENGTH];
    sh_int i;

    buf[0] = '\0';

    for (i = FirstActive(ch); i <= LastActive(ch); ++i) {
        if (IS_ACTIVE(ch, i) && !HAD_CLASS(ch, i)) {
            if (!title_table[i][GET_LEVEL(ch,i)] ||
                !title_table[i][GET_LEVEL(ch,i)][ch->sex==SEX_FEMALE?1:0])
                continue;
            strcat(buf, title_table[i] [GET_LEVEL(ch, i)]
                   [ch->sex == SEX_FEMALE ? 1 : 0]);
            if (i != LastActive(ch))
                strcat(buf, "/");
        }
    }
    if (buf[strlen(buf)] == '/')
        buf[strlen(buf)] = '\0';

    return(buf);
#endif
}


bool ShouldSaveSkill(CHAR_DATA *ch, int skill)
{
    sh_int i;

    if (!skill)
	return(FALSE);

    if (IS_IMMORTAL(ch))
        return(TRUE);

    if (IS_NPC(ch) && !IS_SET(ch->act, ACT_POLYMORPHED))
        return(TRUE);

    for (i = FIRST_CLASS; i < MAX_CLASS; ++i)
        if (HAS_CLASS(ch, i))
            if ((GET_LEVEL(ch, i) >= skill_table[skill]->skill_level[i]) &&
                !IS_FALLEN(ch, i))
                return(TRUE);

    return(FALSE);
}

bool CanUseSkill(CHAR_DATA *ch, int skill)
{
    sh_int i;

    if (!skill)
	return(FALSE);

    if (IS_IMMORTAL(ch))
        return(TRUE);

    if (IS_NPC(ch) && !IS_SET(ch->act, ACT_POLYMORPHED))
        return(TRUE);

    for (i = FIRST_CLASS; i < MAX_CLASS; ++i)
        if (IS_ACTIVE(ch, i))
            if ((GET_LEVEL(ch, i) >= skill_table[skill]->skill_level[i]) &&
                !IS_FALLEN(ch, i))
                return(TRUE);

    return(FALSE);
}

bool CanUseSkillClass(CHAR_DATA *ch, int skill, sh_int cl)
{
    if (!skill)
	return(FALSE);

    if (IS_IMMORTAL(ch))
        return(TRUE);

    if (IS_NPC(ch) && !IS_SET(ch->act, ACT_POLYMORPHED))
        return(TRUE);

    if (!IS_ACTIVE(ch, cl))
        return(FALSE);

    if (GET_LEVEL(ch, cl) >= skill_table[skill]->skill_level[cl])
        return(TRUE);

    return(FALSE);
}

int LowSkLv(CHAR_DATA *ch, int skill)
{
    sh_int i;
    int low = MAX_LEVEL;

    for (i = FIRST_CLASS; i < MAX_CLASS; ++i)
        if (IS_ACTIVE(ch, i))
            if (skill_table[skill]->skill_level[i] < low)
                low = skill_table[skill]->skill_level[i];
    return(low);
}

int LowSkCl(CHAR_DATA *ch, int skill)
{
    sh_int i;
    sh_int cl = CLASS_NONE;

    for (i = FIRST_CLASS; i < MAX_CLASS; ++i)
        if (IS_ACTIVE(ch, i)) {
            if (cl == CLASS_NONE) {
                cl = i;
            } else
                if (skill_table[skill]->skill_level[i] <
                    skill_table[skill]->skill_level[cl])
                    cl = i;
        }
    return(cl);
}

sh_int FirstActive(CHAR_DATA *ch)
{
    sh_int i;
    char buf[MAX_INPUT_LENGTH];

    for (i = FIRST_CLASS; i < MAX_CLASS; ++i)
        if (IS_ACTIVE(ch, i))
            return(i);
    sprintf(buf, "%s has no class in FirstActive!", GET_NAME(ch));
    log_string_plus(buf, LOG_BUG, LEVEL_LOG_CSET, SEV_CRIT);
    return(CLASS_NONE);
}

sh_int MIFirstActive(MOB_INDEX_DATA *ch)
{
    sh_int i;

    for (i = FIRST_CLASS; i < MAX_CLASS; ++i)
        if (IS_ACTIVE(ch, i))
            return(i);
    return(CLASS_NONE);
}

sh_int BestSkCl(CHAR_DATA *ch, int skill)
{
    sh_int i, cur;

    cur = FirstActive(ch);

    if (!IS_VALID_SN(skill))
        return(cur);

    for (i = cur + 1; i < MAX_CLASS; ++i)
        if (IS_ACTIVE(ch, i))
            if ((GET_LEVEL(ch, i) >= skill_table[skill]->skill_level[i]) &&
                (GET_LEVEL(ch, cur) < GET_LEVEL(ch, i)) )
                cur = i;

    return(cur);
}

int BestSkLv(CHAR_DATA *ch, int skill)
{
    return(ch->levels[BestSkCl(ch, skill)]);
}

void ClassSpecificStuff(CHAR_DATA *ch)
{
    AFFECT_DATA *paf;
    int x=0;

    if (IS_NPC(ch))
        return;

    if (IsDragon(ch))
        ch->armor = UMIN(-150, ch->armor);


    if (IS_ACTIVE(ch, CLASS_BARBARIAN)) {
        x = 50 * UMIN(40, GET_LEVEL(ch, CLASS_BARBARIAN));
    } else if (IS_ACTIVE(ch, CLASS_WARRIOR)) {
        x = 50 * UMIN(40, GET_LEVEL(ch, CLASS_WARRIOR));
    } else if (IS_ACTIVE(ch, CLASS_PALADIN)) {
        x = 50 * UMIN(40, GET_LEVEL(ch, CLASS_PALADIN));
    } else if (IS_ACTIVE(ch, CLASS_RANGER)) {
        x = 50 * UMIN(40, GET_LEVEL(ch, CLASS_RANGER));
    }
    ch->numattacks = 1000 + x;

    if (IS_ACTIVE(ch, CLASS_MONK))
    {
        x = (int)((62.5 * GET_LEVEL(ch, CLASS_MONK)));
        ch->numattacks = 1000 + x;
    }
    else
        ch->numattacks = URANGE(1000, ch->numattacks, 3000);

    for (paf = ch->first_affect; paf; paf = paf->next)
        if (paf->location == APPLY_NUMATTACKS)
            ch->numattacks += paf->modifier;

    if (IS_ACTIVE(ch,CLASS_MONK))
        switch (GET_LEVEL(ch, CLASS_MONK))
        {
        case 1:
        case 2:
        case 3:
            ch->barenumdie = 1;
            ch->baresizedie = 3;
            break;
        case 4:
        case 5:
            ch->barenumdie = 1;
            ch->baresizedie = 4;
            break;
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
            ch->barenumdie = 1;
            ch->baresizedie = 6;
            break;
        case 12:
        case 13:
        case 14:
            ch->barenumdie = 2;
            ch->baresizedie = 3;
            break;
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
            ch->barenumdie = 2;
            ch->baresizedie = 4;
            break;
        case 20:
        case 21:
            ch->barenumdie = 3;
            ch->baresizedie = 3;
            break;
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
            ch->barenumdie = 3;
            ch->baresizedie = 4;
            break;
        case 27:
        case 28:
        case 29:
            ch->barenumdie = 4;
            ch->baresizedie = 3;
            break;
        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
            ch->barenumdie = 4;
            ch->baresizedie = 4;
            break;
        case 35:
        case 36:
            ch->barenumdie = 4;
            ch->baresizedie = 5;
            break;
        case 37:
        case 38:
        case 39:
        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
            ch->barenumdie = 5;
            ch->baresizedie = 4;
            break;
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
            ch->barenumdie = 5;
            ch->baresizedie = 5;
            break;
        case 50:
            ch->barenumdie = 7;
            ch->baresizedie = 4;
            break;
        case 51:
        case 52:
        case 53:
        case 54:
        case 55:
            ch->barenumdie = 8;
            ch->baresizedie = 4;
            break;
        default:
            ch->barenumdie = 8;
            ch->baresizedie = 5;
            break;
        }

    if (IS_ACTIVE(ch, CLASS_BARBARIAN) && !IS_ACTIVE(ch, CLASS_MONK)
        && GET_LEVEL(ch, CLASS_BARBARIAN)>50) {
        switch (GET_LEVEL(ch, CLASS_BARBARIAN)) {
        case 51:
        case 52:
        case 53:
            ch->barenumdie = 2;
            ch->baresizedie = 2;
            break;
        case 54:
        case 55:
        case 56:
            ch->barenumdie = 3;
            ch->baresizedie = 2;
            break;
        case 57:
        case 58:
        case 59:
            ch->barenumdie = 4;
            ch->baresizedie = 2;
            break;
        default:
            ch->barenumdie = 5;
            ch->baresizedie = 2;
            break;
        }
    }
    if (IS_ACTIVE(ch, CLASS_MONK)) {
        if (GET_LEVEL(ch, CLASS_MONK) > 10)
            SET_BIT(ch->resistant, RIS_HOLD);
        if (GET_LEVEL(ch, CLASS_MONK) > 18)
            SET_BIT(ch->immune, RIS_CHARM);
        if (GET_LEVEL(ch, CLASS_MONK) > 22)
            SET_BIT(ch->resistant, RIS_POISON);
        if (GET_LEVEL(ch, CLASS_MONK) > 36)
            SET_BIT(ch->resistant, RIS_CHARM);
        ch->armor = 100 - UMIN(150, (GET_LEVEL(ch, CLASS_MONK) * 5));
        ch->max_move = GET_LEVEL(ch, CLASS_MONK);
    }
    if (IS_ACTIVE(ch, CLASS_DRUID)) {
        if (GET_LEVEL(ch, CLASS_DRUID) >= 14)
            SET_BIT(ch->immune, RIS_CHARM);
        if (GET_LEVEL(ch, CLASS_DRUID) >= 32)
            SET_BIT(ch->resistant, RIS_POISON);
    }
    if (OnlyClass(ch, CLASS_ARTIFICER)) {
        if (GET_LEVEL(ch, CLASS_ARTIFICER) >= 10)
            SET_BIT(ch->resistant, RIS_COLD);
        if (GET_LEVEL(ch, CLASS_ARTIFICER) >= 20)
            SET_BIT(ch->resistant, RIS_FIRE);
        if (GET_LEVEL(ch, CLASS_ARTIFICER) >= 50)
            SET_BIT(ch->immune, RIS_COLD);
        if (GET_LEVEL(ch, CLASS_ARTIFICER) >= 51)
            SET_BIT(ch->immune, RIS_FIRE);
        if (GET_LEVEL(ch, CLASS_ARTIFICER) >= 60)
            SET_BIT(ch->resistant, RIS_ENERGY);
        if (GET_LEVEL(ch, CLASS_ARTIFICER) <= 50)
            SET_BIT(ch->susceptible, RIS_SLEEP);
    }
}

/* sorcerer stuff */
int total_memorized(CHAR_DATA *ch)
{
    sh_int i;
    int total=0;

    for (i = FIRST_CLASS; i < MAX_SKILL; i++)
        total += MEMORIZED(ch, i);

    return(total);
}


void forget_spell(CHAR_DATA *ch, int sn)
{
    if (IS_NPC(ch))
	return;

    if (MEMORIZED(ch, sn))
        ch->pcdata->memorized[sn] -= 1;
}

int max_can_memorize(CHAR_DATA *ch)
{
    int i;

    if (OnlyClass(ch, CLASS_SORCERER))
        i=GET_LEVEL(ch, CLASS_SORCERER)*2;
    else
        i=(int)(GET_LEVEL(ch, CLASS_SORCERER)*2/HowManyClasses(ch)*0.5);

    i += (int)int_app[get_curr_int(ch)].learn/2;
    i += (int)get_curr_wis(ch)/6;

    return(i);
}

/* return the amount max a person can memorize a single spell */
int max_can_memorize_spell(CHAR_DATA *ch, int sn)
{
    int canmem;

    if (OnlyClass(ch,CLASS_SORCERER))
        canmem = 2;
    else
        canmem = 0;

    if (get_curr_int(ch) > 18)
        canmem += (get_curr_int(ch)-18);  /* +1 spell per intel over 18 */

    canmem++;

    if (GET_LEVEL(ch, CLASS_SORCERER) < 4)
        return(3+canmem);
    if (GET_LEVEL(ch, CLASS_SORCERER) < 11)
        return(3+canmem);
    if (GET_LEVEL(ch, CLASS_SORCERER) < 18)
        return(3+canmem);
    if (GET_LEVEL(ch, CLASS_SORCERER) < 21)
        return(4+canmem);
    if (GET_LEVEL(ch, CLASS_SORCERER) < 26)
        return(4+canmem);
    if (GET_LEVEL(ch, CLASS_SORCERER) < 21)
        return(5+canmem);
    if (GET_LEVEL(ch, CLASS_SORCERER) < 23)
        return(5+canmem);
    if (GET_LEVEL(ch, CLASS_SORCERER) < 26)
        return(6+canmem);
    if (GET_LEVEL(ch, CLASS_SORCERER) < 28)
        return(6+canmem);
    if (GET_LEVEL(ch, CLASS_SORCERER) < 31)
        return(7+canmem);
    if (GET_LEVEL(ch, CLASS_SORCERER) < 34)
        return(7+canmem);
    if (GET_LEVEL(ch, CLASS_SORCERER) < 36)
        return(7+canmem);
    if (GET_LEVEL(ch, CLASS_SORCERER) < 41)
        return(8+canmem);
    if (GET_LEVEL(ch, CLASS_SORCERER) < 46)
        return(9+canmem);
    if (GET_LEVEL(ch, CLASS_SORCERER) < 51)
        return(10+canmem);
    if (GET_LEVEL(ch, CLASS_SORCERER) < 56)
        return(11+canmem);
    if (GET_LEVEL(ch, CLASS_SORCERER) < 61)
        return(12+canmem);

    /* imms */
    return(100);
}

bool can_gain_level( CHAR_DATA *ch, sh_int cl )
{
    if (!IS_ACTIVE(ch, cl))
        return FALSE;

    if (GET_LEVEL(ch,cl) >= LEVEL_AVATAR)
        return FALSE;

    if (GET_LEVEL(ch,cl) < RacialMax[GET_RACE(ch)][cl])
        return TRUE;

    return FALSE;
}

int get_numattacks(CHAR_DATA *ch)
{
    int x;

    if (IS_ACTIVE(ch, CLASS_MONK) && get_eq_char(ch, WEAR_WIELD))
        return 1000;

    if (ch->numattacks > 20)
        x = ch->numattacks;
    else
        x = ch->numattacks * 1000;

    return x;
}
