/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer and Heath Leach
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*static char rcsid[] = "$Id: idale.c,v 1.40 2004/04/06 22:00:10 dotd Exp $";*/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include "mud.h"
#include "gsn.h"
#include "currency.h"

#define DBW_IS_IMMORTAL(ch) ((ch) && IS_IMMORTAL(ch))

char *dale_buffered_who(CHAR_DATA *ch, char *argument)
{
    static char retbuf[MAX_STRING_LENGTH];

    DESCRIPTOR_DATA *d;
    CHAR_DATA      *person;
    char            buffer[MAX_STRING_LENGTH], tbuf[MAX_INPUT_LENGTH];
    int             count = 0;
    int             i;
    char            flags[20];
    char            name_mask[40];
    char s1[16], s2[16], s3[16], s4[16], s5[16];

    name_mask[0] = flags[0] = buffer[0] = tbuf[0] = '\0';
    retbuf[0] = '\0';

    sprintf(s1,"%s",color_str(AT_WHO,ch));
    sprintf(s2,"%s",color_str(AT_WHO2,ch));
    sprintf(s3,"%s",color_str(AT_WHO4,ch));
    sprintf(s4,"%s",color_str(AT_WHO,ch));
    sprintf(s5,"%s",color_str(AT_WHO3,ch));

    argument = one_argument(argument, tbuf);

    if (tbuf[0] == '-' && tbuf[1] != '\0')
        strncpy(flags, tbuf + 1, 19);
    else
        strncpy(name_mask, tbuf, 39);
    if (*argument) {
        argument = one_argument(argument, tbuf);
        if (tbuf[0] == '-' && tbuf[1] != '\0')
            strncpy(flags, tbuf + 1, 19);
        else
            strncpy(name_mask, tbuf, 39);
    }
    if (flags[0] == '\0' || !DBW_IS_IMMORTAL(ch)) {
        if (DBW_IS_IMMORTAL(ch)) {
            sprintf(buffer, "%sPlayers %s[%sGod Version -? for Help%s]\n\r---------------------------------\n\r",
                    s3,s2,s3,s2);
        } else {
            sprintf(buffer, "%sPlayers:\n\r--------\n\r",s2);
        }
        strcat(retbuf,buffer);
        for (d = first_descriptor; d; d = d->next) {
            person = (d->original ? d->original : d->character);
            if (!person)
                continue;
            if (can_see(ch, person) && person->in_room && \
                (!index(flags, 'g') || DBW_IS_IMMORTAL(person))) {
                if (is_name(PERS(person,ch), name_mask) || !name_mask[0]) {
		    char            statbuf[20];
                    /*
                    int x=0;
                    if (strlen(uncolorify(ParseAnsiColors(0, GET_RANK(person))))<20)
                        for (x=0;x<(10-(strlen(uncolorify(ParseAnsiColors(0, GET_RANK(person))))/2));x++)
                            statbuf[x] = ' ';
                    statbuf[x]='\0';
		    sprintf(buffer,"%s%s",statbuf,GET_RANK(person));
                    */
		    count++;
                    sprintf(buffer, "%s", center_str_color(GET_RANK(person), 20));
		    sprintf(statbuf, "%s[",s2);
                    if (IS_SET(person->act, PLR_AFK))
                        strcat(statbuf, "A");
                    else if (IS_SET(person->act2, PLR2_BUSY))
                        strcat(statbuf, "B");
                    else if (person->desc && person->desc->idle/PULSE_PER_SECOND>600)
                        strcat(statbuf, "I");
                    else if (person->leader)
                        strcat(statbuf, "G");
                    else if (IS_CLANNED(person))
                        strcat(statbuf, "C");
                    else
                        strcat(statbuf, "-");
                    if (person->desc && person->desc->connected > CON_PLAYING)
                        strcat(statbuf, "E");
                    else if (person->desc && person->desc->connected < CON_PLAYING)
                        strcat(statbuf, "N");
                    else
                        strcat(statbuf, "-");
                    strcat(statbuf, "]");
                    sprintf(tbuf, "%s%s %s%s %s\n\r", DBW_IS_IMMORTAL(person) ? color_str(AT_YELLOW,ch) : s5,
                            buffer, statbuf, s1, PERSLONG(person, ch));
                    /*send_to_pager(tbuf, ch);*/
                    strcat(retbuf,tbuf);
                }
            }
        }
        if (index(flags, 'g'))
            sprintf(tbuf, "\n\r%sTotal visible gods: %s%d\n\r",s3,s4,count);
        else
            sprintf(tbuf, "\n\r%sTotal visible players: %s%d\n\r",s3,s4,count);
        strcat(retbuf,tbuf);
    } else {
        int             listed = 0,
        lcount = 0,
        skip = FALSE;
	unsigned int l;
	char            ttbuf[MAX_INPUT_LENGTH];

        ttbuf[0] = '\0';
        sprintf(buffer, "%sPlayers %s[%sGod Version -? for Help%s]\n\r---------------------------------\n\r",s3,s2,s3,s2);

        if (index(flags, '?')) {
            strcat(retbuf,color_str(AT_PURPLE,ch));
            strcat(retbuf,buffer);
            strcat(retbuf,"[-]z=zone w=saving throws\n\r");
            strcat(retbuf,"[-]e=experience a=alignment q=gold m=gains n=deity b=odds k=kills\n\r");
            strcat(retbuf,"[-]i=idle l=levels t=title h=hit/mana/move s=stats r=race f=fighting\n\r");
            strcat(retbuf,"[-]d=linkdead g=God o=Mort [1]Mage[2]Cleric[3]War[4]Thief[5]Druid\n\r");
            strcat(retbuf,"[-][v]=invis lev [6]Monk[7]Barb[8]Sorc[9]Paladin[!]Ranger[@]Psi\n\r");
            strcat(retbuf,"\n\rStatus Bar Key:\n\r");
            strcat(retbuf,color_str(AT_DGREY,ch));
            strcat(retbuf,"---------------\n\r");
            strcat(retbuf,"A - AFK, B - Busy, I - Idle, G - Goruped, C - Clanned, E - Editing, N - New\n\r");
            return(retbuf);
        }
        for (person = first_char; person; person = person->next) {
            if ((!IS_NPC(person) || IS_SET(person->act, ACT_POLYMORPHED)) &&
                can_see(ch, person) &&
                (is_name(PERS(person,ch), name_mask) || !name_mask[0])) {
                count++;
                if (person->desc == NULL)
                    lcount++;
                skip = FALSE;
                if (index(flags, 'g') != NULL)
                    if (!DBW_IS_IMMORTAL(person))
                        skip = TRUE;
                if (index(flags, 'o') != NULL)
                    if (DBW_IS_IMMORTAL(person))
                        skip = TRUE;
                if (index(flags, '1') != NULL)
                    if (!HAS_CLASS(person, CLASS_MAGE))
                        skip = TRUE;
                if (index(flags, '2') != NULL)
                    if (!HAS_CLASS(person, CLASS_CLERIC))
                        skip = TRUE;
                if (index(flags, '3') != NULL)
                    if (!HAS_CLASS(person, CLASS_WARRIOR))
                        skip = TRUE;
                if (index(flags, '4') != NULL)
                    if (!HAS_CLASS(person, CLASS_THIEF))
                        skip = TRUE;
                if (index(flags, '5') != NULL)
                    if (!HAS_CLASS(person, CLASS_DRUID))
			skip = TRUE;
		if (index(flags, '6') != NULL)
		    if (!HAS_CLASS(person, CLASS_MONK))
			skip = TRUE;
		if (index(flags, '7') != NULL)
		    if (!HAS_CLASS(person, CLASS_BARBARIAN))
			skip = TRUE;
		if (index(flags, '8') != NULL)
		    if (!HAS_CLASS(person, CLASS_NECROMANCER))
			skip = TRUE;
		if (index(flags, '9') != NULL)
		    if (!HAS_CLASS(person, CLASS_PALADIN))
			skip = TRUE;
		if (index(flags, '!') != NULL)
		    if (!HAS_CLASS(person, CLASS_RANGER))
			skip = TRUE;
		if (index(flags, '@') != NULL)
		    if (!HAS_CLASS(person, CLASS_PSIONIST))
			skip = TRUE;
		if (index(flags, '#') != NULL)
                    if (!HAS_CLASS(person, CLASS_ARTIFICER))
                        skip = TRUE;

                if (!skip) {
                    if (person->desc == NULL) {
                        if (index(flags, 'd') != NULL) {
                            sprintf(tbuf, "%s[%-12s] ", s5, PERS(person,ch));
                            listed++;
                        }
                    } else {
                        if (IS_NPC(person) && IS_SET(person->act, ACT_POLYMORPHED)) {
                            sprintf(tbuf, "%s(%-12s)%s ", s2, PERS(person,ch), s5);
                            listed++;
                        } else {
                            sprintf(tbuf, "%s%-14s ", s5, PERS(person,ch));
                            listed++;
                        }
                    }
                    if ((person->desc != NULL) || (index(flags, 'd') != NULL)) {
                        for (l = 0; l < strlen(flags); l++) {
                            switch (flags[l]) {
                            case 'r':
                                {
                                    sprintf(ttbuf, " [%-15s", race_table[GET_RACE(person)].race_name);
                                    if (IsGoodSide(person))
                                        strcat(ttbuf, "    Good]  ");
                                    else if (IsBadSide(person))
                                        strcat(ttbuf, "    Evil]  ");
                                    else
                                        strcat(ttbuf, " Neutral]  ");
                                    strcat(tbuf, ttbuf);
                                    break;
                                }
                            case 'a':
                                sprintf(ttbuf, "Align:[%-5d] ", GET_ALIGN(person));
                                strcat(tbuf, ttbuf);
				break;
			    case 'e':
				sprintf(ttbuf, "Exp:[%12d] ", GET_EXP(person));
				strcat(tbuf, ttbuf);
				break;
			    case 'q':
				sprintf(ttbuf, "Gold:[%-9d] Bank:[%-9d] Worth:[%-3d%%]", GET_MONEY(person,CURR_GOLD), GET_BALANCE(person,CURR_GOLD), player_worth_percentage(person));
				strcat(tbuf, ttbuf);
				break;
			    case 'i':
				sprintf(ttbuf, "Idle:[%-3d] ", person->desc->idle / PULSE_PER_SECOND);
				strcat(tbuf, ttbuf);
				break;
			    case 'l':
				strcat(tbuf, "Level:[");
                                for (i = 0; i < MAX_CLASS; i++) {
                                  sprintf(ttbuf, "%2d", GET_LEVEL(person, i));
 				  strcat(tbuf, ttbuf);
                                  if (i < (MAX_CLASS-1)) {
				     strcat(tbuf, "\\");
                                 }
                                }
                                strcat(tbuf, "]");
				break;
			    case 'h':
				sprintf(ttbuf, "Hit:[%-4d/%-4d] Mana:[%-4d/%-4d] Move:[%-4d/%-4d] ", GET_HIT(person), GET_MAX_HIT(person), GET_MANA(person), GET_MAX_MANA(person), GET_MOVE(person), GET_MAX_MOVE(person));
				strcat(tbuf, ttbuf);
				break;
			    case 's':
				sprintf(ttbuf, "[S:%-2d I:%-2d W:%-2d C:%-2d D:%-2d CH:%-2d L:%-2d] ",
                                        get_curr_str(person), get_curr_int(person), get_curr_wis(person),
                                        get_curr_con(person), get_curr_dex(person), get_curr_cha(person),
                                        get_curr_lck(person));
				strcat(tbuf, ttbuf);
                                break;
			    case 'w':
                                sprintf(ttbuf, "[Psn:%-2d Wnd:%-2d Par:%-2d Bre:%-2d Spl:%-2d] ",
                                        person->saving_poison_death, person->saving_wand,
                                        person->saving_para_petri, person->saving_breath,
                                        person->saving_spell_staff);
				strcat(tbuf, ttbuf);
                                break;
                            case 'v':
                                sprintf(ttbuf, "INV:[");
                                if (DBW_IS_IMMORTAL(person) && !IS_NPC(person) && person->pcdata->wizinvis)
				    sprintf(ttbuf + strlen(ttbuf), "%-3d] ", person->pcdata->wizinvis);
				else if (IS_AFFECTED(person, AFF_INVISIBLE))
				    sprintf(ttbuf + strlen(ttbuf), "1  ] ");
				else
				    sprintf(ttbuf + strlen(ttbuf), "0  ] ");
				strcat(tbuf, ttbuf);
				break;
			    case 'f':
				if (who_fighting(person))
				    sprintf(ttbuf, "%s", PERS(who_fighting(person),ch));
				else
				    sprintf(ttbuf, "Nobody");
				strcat(tbuf, ttbuf);
                                break;
                            case 't':
                                sprintf(ttbuf, " %-16s ", GET_TITLE(person) ? GET_TITLE(person) : "(none)");
                                strcat(tbuf, ttbuf);
                                break;
                            case 'm':
                                sprintf(ttbuf, "Gains: [%2d+%2d/%2d+%2d/%2d+%2d] ",
                                         hit_gain(person),person->hit_regen,
                                        mana_gain(person),person->mana_regen,
                                        move_gain(person),person->move_regen);
                                strcat(tbuf, ttbuf);
				break;
			    case 'n':
                                sprintf(ttbuf, "Diety: [%12s] ", person->pcdata ? (person->pcdata->deity_name ? person->pcdata->deity_name : "(none)") : "(none)");
				strcat(tbuf, ttbuf);
				break;
			    case 'k':
				sprintf(ttbuf, "Kills: [%5d] Deaths: [%5d] ",
					person->pcdata->mkills, person->pcdata->mdeaths);
				strcat(tbuf, ttbuf);
				break;
			    case 'z':
                                sprintf(ttbuf, "Zone: [%s] ", ch->in_room->area?ch->in_room->area->name:"(noarea)");
				strcat(tbuf, ttbuf);
			    default:
				break;
                            }
                        }
                    }
                    if ((person->desc != NULL) || (index(flags, 'd') != NULL)) {
                        if (is_name(PERS(person,ch), name_mask) || !name_mask[0]) {
                            strcat(tbuf, "\n\r");
                            strcat(retbuf,tbuf);
			}
		    }
		}
	    }
        }

        if (listed <= 0)
            sprintf(tbuf, "\n\r%sNo Matches\n\r",s3);
        else {
            sprintf(tbuf, "\n\r%sTotal players / Link dead [%d/%d] (%2.0f%%)\n\r",
                    s3, count, lcount, ((float) lcount / (int) count) * 100);
        }
        strcat(retbuf,tbuf);
    }

    return(retbuf);
}

#undef DBW_IS_IMMORTAL

void dale_who(CHAR_DATA * ch, char *argument)
{
    char *buf;

    buf = dale_buffered_who(ch,argument);

    send_to_pager_color(buf,ch);
}

void dale_score(CHAR_DATA * ch, char *argument)
{
    char            buf[MAX_STRING_LENGTH];
    int  i;
    char s1[16],s2[16],s3[16],s4[16];

    sprintf(s1,"%s",color_str(AT_SCORE,ch));
    sprintf(s2,"%s",color_str(AT_SCORE2,ch));
    sprintf(s3,"%s",color_str(AT_SCORE3,ch));
    sprintf(s4,"%s",color_str(AT_SCORE4,ch));

    ch_printf(ch, "\n\r%sYou are %s%d%s years old.\n\r",
	s1,s2,get_age(ch),s1);

    ch_printf(ch, "You belong to the %s%s%s race.\n\r",
	s3,get_race_name(ch),s1);

    ch_printf(ch, "You have %s%d%s(%s%d%s) hit, %s%d%s(%s%d%s) mana, %s%d%s(%s%d%s) movement points.\n\r",
	s2,GET_HIT(ch),s1,
	s4,GET_MAX_HIT(ch),s1,
	s2,GET_MANA(ch),s1,
	s4,GET_MAX_MANA(ch),s1,
	s2,GET_MOVE(ch),s1,
	s4,GET_MAX_MOVE(ch),s1);

    if (IS_VAMPIRE(ch)) {
	ch_printf(ch,"Your blood level is: %s%d%s(%s%d%s).\n\r",
	    s2,GET_BLOOD(ch),s1,s4,GET_MAX_BLOOD(ch),s1);
    }

    ch_printf(ch, "You have %s%d%s practices left.\n\r",
	s2,ch->practice,s1);

    if (!IS_NPC(ch)) {
	ch_printf(ch, "You have died %s%d%s times and killed %s%d%s times.\n\r",
	    s2,ch->pcdata->mdeaths,s1,s2,ch->pcdata->mkills,s1);
    }

    if (ch->alignment > 900)
	sprintf(buf, "So good it makes you sick");
    else if (ch->alignment > 500)
	sprintf(buf, "A real goody-goody");
    else if (ch->alignment > 350)
	sprintf(buf, "Sweet and caring");
    else if (ch->alignment > 100)
	sprintf(buf, "Polite");
    else if (ch->alignment > 0)
        sprintf(buf, "Balanced");
    else if (ch->alignment == 0)
        sprintf(buf, "So neutral a druid would be ashamed");
    else if (ch->alignment > -100)
	sprintf(buf, "Annoying");
    else if (ch->alignment > -350)
	sprintf(buf, "Obnoxious as Hell");
    else if (ch->alignment > -500)
	sprintf(buf, "Not Nice at All");
    else if (ch->alignment > -900)
	sprintf(buf, "Really really Bad");
    else
	sprintf(buf, "So bad it makes you puke");

    ch_printf(ch, "Your alignment is: %s%s%s.\n\r",s3,buf,s1);

    ch_printf(ch, "You have scored %s%d%s exp.\n\r",
              s2,GET_EXP(ch),s1);

    for (i=0;i<MAX_CURR_TYPE;i++)
        if (GET_MONEY(ch,i) || GET_BALANCE(ch,i))
            ch_printf(ch, "You have %s%d%s %s coins on hand and %s%d%s in the bank.\n\r",
                      s2, GET_MONEY(ch,i), s1,
                      curr_types[i],
                      s2, GET_BALANCE(ch,i), s1);

    ch_printf(ch, "Your levels: ");
    for (i = 0; i < MAX_CLASS; ++i) {
        if IS_ACTIVE(ch, i)
           ch_printf(ch, "%s%s:%s%d ",s1,short_pc_class[i],s2,ch->levels[i]);
    }
    ch_printf(ch, "%s\n\r",s1);

    if (!IS_IMMORTAL(ch)) {
	ch_printf(ch, "Exp to next level: ");
	for (i = 0; i < MAX_CLASS; ++i) {
	    if (IS_ACTIVE(ch, i) && !HAD_CLASS(ch, i)) {
		ch_printf(ch, "%s%s:%s%ld ",
		    s1,short_pc_class[i],s2,
		    exp_level(ch, GET_LEVEL(ch, i)+1, i) - GET_EXP(ch));
	    }
	}
	ch_printf(ch, "%s\n\r", s1);
    }

    if ( get_trust(ch) >= LEVEL_IMMORTAL && get_trust(ch) != GetMaxLevel(ch) ) {
	ch_printf(ch, "You are trusted at level: %s%d%s\n\r",
	    s2,get_trust(ch),s1);
    }

    if (!IS_NPC(ch)) {
	ch_printf(ch, "This ranks you as: %s%s%s%s\n\r",
	    s3,ch->name,ch->pcdata->title,s1);
	ch_printf(ch, "Your introduction is: %s%s%s%s\n\r",
	    s1,s3,ch->intro_descr ? ch->intro_descr : "(not set - see help INTRO)",s1);
	ch_printf(ch, "You have been playing for: %s%s%s\n\r",
                  s2, sec_to_hms(ch->played + (current_time - ch->logon)), s1);
    }

    switch (ch->position)
    {
	case POS_DEAD:
		sprintf(buf, "pretty much dead");
		break;
	case POS_MORTAL:
		sprintf(buf, "mortally wounded");
		break;
	case POS_INCAP:
		sprintf(buf, "incapacitated");
		break;
	case POS_STUNNED:
		sprintf(buf, "stunned");
		break;
	case POS_SLEEPING:
		sprintf(buf, "sleeping");
		break;
	case POS_RESTING:
		sprintf(buf, "resting");
		break;
	case POS_STANDING:
		sprintf(buf, "standing");
		break;
	case POS_FIGHTING:
		sprintf(buf, "fighting");
		break;
	case POS_MOUNTED:
		sprintf(buf, "mounted");
		break;
        case POS_SITTING:
		sprintf(buf, "sitting");
		break;
        case POS_MEDITATING:
		sprintf(buf, "MEDITATING");
		break;
    }

    ch_printf(ch, "You are %s%s%s.\n\r",s3,buf,s1);
    return;
}

void dale_attrib(CHAR_DATA * ch, char *argument)
{
   char buf[MAX_STRING_LENGTH];
   AFFECT_DATA *paf;
   int iLang = 0;

   if (IS_NPC(ch))
     sprintf(buf, " ");
   else
       sprintf(buf, " %s%d%s years old, ",
               color_str(AT_SCORE2, ch), get_age(ch), color_str(AT_SCORE, ch));
   ch_printf(ch, "\n\r%sYou are%s%s%d%s inches, and %s%d%s lbs.\n\r"
             "You are carrying %s%d%s lbs of equipment.\n\r"
             "You are %s%s%s.\n\r",
             color_str(AT_SCORE, ch), buf, color_str(AT_SCORE2, ch),
             ch->height, color_str(AT_SCORE, ch), color_str(AT_SCORE2, ch),
             ch->weight, color_str(AT_SCORE, ch), color_str(AT_SCORE2, ch),
             carry_w(ch), color_str(AT_SCORE, ch), color_str(AT_SCORE3, ch),
             ArmorDesc(GET_AC(ch)), color_str(AT_SCORE, ch) );

   if (GetMaxLevel(ch) >= 15) {
       paint(AT_SCORE, ch, "You have ");
       paint(AT_SCORE2, ch, "%d ", get_curr_str(ch));
       paint(AT_SCORE, ch, "STR, ");
       paint(AT_SCORE2, ch, "%d ", get_curr_int(ch));
       paint(AT_SCORE, ch, "INT, ");
       paint(AT_SCORE2, ch, "%d ", get_curr_wis(ch));
       paint(AT_SCORE, ch, "WIS, ");
       paint(AT_SCORE2, ch, "%d ", get_curr_dex(ch));
       paint(AT_SCORE, ch, "DEX, ");
       paint(AT_SCORE2, ch, "%d ", get_curr_con(ch));
       paint(AT_SCORE, ch, "CON, ");
       paint(AT_SCORE2, ch, "%d ", get_curr_cha(ch));
       paint(AT_SCORE, ch, "CHR, ");
       paint(AT_SCORE2, ch, "%d ", get_curr_lck(ch));
       paint(AT_SCORE, ch, "LCK\n\r");
   }

   paint(AT_SCORE, ch, "Your hit and damage bonuses are ");
   paint(AT_SCORE3, ch, "%s ", RollDesc(ch->hitroll));
   paint(AT_SCORE, ch, "and ");
   paint(AT_SCORE3, ch, "%s ", RollDesc(ch->damroll));
   paint(AT_SCORE, ch, "respectively.\n\r");

   send_to_char("Languages: ", ch );
   for ( iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
       if ( knows_language( ch, lang_array[iLang], ch )
            ||  (IS_NPC(ch) && ch->speaks == 0) )
       {
           if ( lang_array[iLang] & ch->speaking )
               set_char_color( AT_SCORE3, ch );
           send_to_char( lang_names[iLang], ch );
           send_to_char( " ", ch );
           set_char_color( AT_SCORE, ch );
       }
   send_to_char( "\n\r", ch );


   ch_printf(ch, "\n\rAffecting Spells:\n\r");
   ch_printf(ch, "-----------------\n\r");
   if (ch->first_affect)
   {
       SKILLTYPE *sktmp;

       for (paf = ch->first_affect; paf; paf = paf->next)
       {
           if ( (sktmp=get_skilltype(paf->type)) == NULL )
               continue;
           sprintf(buf, "'%s%s%s'", color_str(AT_SCORE3, ch),  sktmp->name, color_str(AT_SCORE, ch));
           ch_printf(ch, "%sSpell : %-20s (%s%d hours%s)\n\r",
                     color_str(AT_SCORE, ch), buf, color_str(AT_SCORE2, ch),
                     paf->duration/(int)DUR_CONV, color_str(AT_SCORE, ch));
       }
   }


}

void dale_group( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *gch;
    CHAR_DATA *leader;

    leader = ch->leader ? ch->leader : ch;
    ch_printf( ch, "%s%s's%s group:\n\r",
               color_str(AT_SCORE3, ch), PERS(leader, ch), color_str(AT_SCORE, ch) );

    for ( gch = first_char; gch; gch = gch->next )
    {
        if ( is_same_group( gch, ch ) )
        {
            ch_printf( ch,
                       "%s%-50s%s HP:%s%2.0f%s%% %4s:%s%2.0f%s%% MV:%s%2.0f%s%%\n\r",
                       color_str(AT_SCORE3, ch),
                       capitalize( PERS(gch, ch) ),
                       color_str(AT_SCORE, ch),
                       color_str(AT_SCORE2, ch),
                       ((float)GET_HIT (gch)/GET_MAX_HIT (gch))*100+0.5,
                       color_str(AT_SCORE, ch),
                       IS_VAMPIRE(gch)?"BLD":"MANA",
                       color_str(AT_SCORE2, ch),
                       ((float)GET_MANA(gch)/GET_MAX_MANA(gch))*100+0.5,
                       color_str(AT_SCORE, ch),
                       color_str(AT_SCORE2, ch),
                       ((float)GET_MOVE(gch)/GET_MAX_MOVE(gch))*100+0.5,
                       color_str(AT_SCORE, ch)
                     );
        }
    }
}

char *how_good(int percent)
{
    static char buf[256];

    if (percent<0)
        strcpy(buf, " (forgotten)");
    else if (percent == 0)
        strcpy(buf, " (not learned)");
    else if (percent <= 10)
        strcpy(buf, " (awful)");
    else if (percent <= 20)
        strcpy(buf, " (terrible)");
    else if (percent <= 30)
        strcpy(buf, " (bad)");
    else if (percent <= 40)
        strcpy(buf, " (poor)");
    else if (percent <= 55)
        strcpy(buf, " (average)");
    else if (percent <= 60)
        strcpy(buf, " (tolerable)");
    else if (percent <= 70)
        strcpy(buf, " (fair)");
    else if (percent <= 80)
        strcpy(buf, " (good)");
    else if (percent <= 85)
        strcpy(buf, " (very good)");
    else if (percent <= 90)
        strcpy(buf, " (excellent)");
    else
        strcpy(buf, " (Superb)");

    return(buf);
}

void dale_prac_output( CHAR_DATA *ch, CHAR_DATA *is_at_gm )
{
    int	col, sn;
    sh_int lasttype, cnt;
    char buf[MAX_INPUT_LENGTH];

    col = cnt = 0;	lasttype = SKILL_SPELL;
    if (is_at_gm)
        sprintf(buf, "%d", is_at_gm->vnum);

    set_pager_color( AT_MAGIC, ch );
    for ( sn = 0; sn < top_sn; sn++ )
    {
        if ( !skill_table[sn]->name ||
             (!skill_table[sn]->skill_fun && !skill_table[sn]->spell_fun))
            continue;

        if (sn == gsn_drinking)
            continue;

        if (is_at_gm)
        {
            if (IS_ACT_FLAG(is_at_gm, ACT_TEACHER))
            {
                if (!skill_table[sn]->teachers ||
                    skill_table[sn]->teachers[0] == '\0' ||
                    !is_name(buf,skill_table[sn]->teachers))
                    continue;
            }
            else if (IS_ACT_FLAG(is_at_gm, ACT_PRACTICE))
            {
                if (skill_table[sn]->teachers &&
                    skill_table[sn]->teachers[0] != '\0')
                    continue;
                if (skill_table[sn]->skill_level[FirstActive(is_at_gm)] >= LEVEL_IMMORTAL ||
                    skill_table[sn]->skill_level[FirstActive(is_at_gm)] >
                    GetClassLevel(is_at_gm,FirstActive(is_at_gm)))
                    continue;
            }
        }
        else
        {
            /*
            is_ok = FALSE;
            for (i = 0; i < MAX_CLASS; ++i)
            {
                if (IS_ACTIVE(ch, i) &&
                    GET_LEVEL(ch, i) >= skill_table[sn]->skill_level[i])
                {
                    is_ok = TRUE;
                    break;
                }
            }
            if (!is_ok)
                continue;*/

            if ( LEARNED(ch, sn) == 0 )
                continue;
        }

        if ( SPELL_FLAG(skill_table[sn], SF_SECRETSKILL) )
            continue;

        if (!IS_IMMORTAL(ch) &&
            ( skill_table[sn]->guild != CLASS_NONE &&
              ( !IS_GUILDED(ch) ||
                (ch->pcdata->clan->cl != skill_table[sn]->guild) ) ) )
            continue;

        if (!CanUseSkill(ch,sn))
            continue;

        pager_printf( ch, "%-30s %-14s\n\r",
                      skill_table[sn]->name,
                      how_good(LEARNED(ch, sn)) );
    }


    pager_printf( ch, "You have %d practice sessions left.\n\r",
                  ch->practice );
}

