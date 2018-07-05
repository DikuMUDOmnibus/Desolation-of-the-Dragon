/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider             |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops  |~'~.VxvxV.~'~*
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *		     Character saving and loading module		    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: save.c,v 1.82 2004/04/06 22:00:11 dotd Exp $";*/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "mud.h"
#include "gsn.h"
#include "recylist.h"
#include "christen.h"
#include "rareobj.h"
#include "currency.h"

extern int              char_affects;

/*
 * Increment with every major format change.
 */
#define SAVEVERSION	3

/*
 * Array to keep track of equipment temporarily.		-Thoric
 */
OBJ_DATA *save_equipment[MAX_WEAR][8];
CHAR_DATA *quitting_char, *loading_char, *saving_char;

int file_ver;

/*
 * Externals
 */
void fwrite_comments( CHAR_DATA *ch, FILE *fp );
void fread_comment( CHAR_DATA *ch, FILE *fp );

/*
 * Array of containers read for proper re-nesting of objects.
 */
static	OBJ_DATA *	rgObjNest	[MAX_NEST];

/*
 * Local functions.
 */
void	  fwrite_char	args( ( CHAR_DATA *ch, FILE *fp ) );
CHAR_DATA *fread_char	args( ( CHAR_DATA *ch, FILE *fp, bool preload ) );
void	  write_corpses	args( ( CHAR_DATA *ch, char *name ) );


/*
 * Un-equip character before saving to ensure proper	-Thoric
 * stats are saved in case of changes to or removal of EQ
 */
void de_equip_char( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int x,y;

    for ( x = 0; x < MAX_WEAR; x++ )
        for ( y = 0; y < MAX_LAYERS; y++ )
            save_equipment[x][y] = NULL;
    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
        if ( obj->wear_loc > -1 && obj->wear_loc < MAX_WEAR )
        {
            if ( char_ego(ch) >= item_ego(obj) )
            {
                for ( x = 0; x < MAX_LAYERS; x++ )
                    if ( !save_equipment[obj->wear_loc][x] )
                    {
                        save_equipment[obj->wear_loc][x] = obj;
                        break;
                    }
                if ( x == MAX_LAYERS )
                {
                    sprintf( buf, "%s had on more than %d layers of clothing in one location (%d): %s",
                             ch->name, MAX_LAYERS, obj->wear_loc, obj->name );
                    bug( buf, 0 );
                }
            }
            else
            {
                sprintf( buf, "%s had on %s:  ch ego = %d  obj ego = %d",
                         ch->name, obj->name,
                         char_ego(ch), item_ego(obj) );
                bug( buf, 0 );
            }
            unequip_char(ch, obj);
        }
}

/*
 * Re-equip character					-Thoric
 */
void re_equip_char( CHAR_DATA *ch )
{
    int x,y;

    for ( x = 0; x < MAX_WEAR; x++ )
        for ( y = 0; y < MAX_LAYERS; y++ )
            if ( save_equipment[x][y] != NULL )
            {
                if ( quitting_char != ch )
                    equip_char(ch, save_equipment[x][y], x);
                save_equipment[x][y] = NULL;
            }
            else
                break;
}


/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch )
{
    CHAR_DATA *follower, *nextinroom;
    char strsave[MAX_INPUT_LENGTH];
    char strback[MAX_INPUT_LENGTH];
    FILE *fp;

    if ( !ch )
    {
        bug( "Save_char_obj: null ch!" );
        return;
    }

    if ( IS_NPC(ch ) )
        return;

    if ( GetMaxLevel(ch) < 2 )
        return;

    saving_char = ch;
    /* save pc's clan's data while we're at it to keep the data in sync */
    if ( !IS_NPC(ch) && ch->pcdata->clan )
        save_clan( ch->pcdata->clan );

    /* save deity's data to keep it in sync -ren */
    if ( !IS_NPC(ch) && ch->pcdata->deity )
        save_deity( ch->pcdata->deity );

    if ( ch->desc && ch->desc->original )
        ch = ch->desc->original;

    de_equip_char( ch );

    ch->save_time = current_time;
    sprintf( strsave, "%s%c/%s", PLAYER_DIR, tolower(ch->name[0]),
             capitalize( ch->name ) );

    /*
     * Auto-backup pfile (can cause lag with high disk access situtations
     */
    if ( IS_SET( sysdata.save_flags, SV_BACKUP ) )
    {
        sprintf( strback, "%s%c/%s", BACKUP_DIR, tolower(ch->name[0]),
                 capitalize( ch->name ) );
        rename( strsave, strback );
    }

    /*
     * Save immortal stats, level & vnums for wizlist		-Thoric
     * and do_vnums command
     *
     * Also save the player flags so we the wizlist builder can see
     * who is a guest and who is retired.
     */
    if ( IS_IMMORTAL(ch) )
    {
        sprintf( strback, "%s%s", GOD_DIR, capitalize( ch->name ) );

        if ( ( fp = fopen( strback, "w" ) ) == NULL )
        {
            bug( "Save_god_level: fopen" );
            perror( strsave );
        }
        else
        {
            fprintf( fp, "Level        %d\n", GetMaxLevel(ch) );
            fprintf( fp, "Pcflags      %d\n", ch->pcdata->flags );
            if ( ch->pcdata->r_range_lo || ch->pcdata->r_range_hi )
                fprintf( fp, "RoomRange    %d %d\n", ch->pcdata->r_range_lo,
                         ch->pcdata->r_range_hi	);
            if ( ch->pcdata->o_range_lo || ch->pcdata->o_range_hi )
                fprintf( fp, "ObjRange     %d %d\n", ch->pcdata->o_range_lo,
                         ch->pcdata->o_range_hi	);
            if ( ch->pcdata->m_range_lo || ch->pcdata->m_range_hi )
                fprintf( fp, "MobRange     %d %d\n", ch->pcdata->m_range_lo,
                         ch->pcdata->m_range_hi	);
            FCLOSE( fp );
        }
    }

    if ( ( fp = fopen( strsave, "w" ) ) == NULL )
    {
        bug( "Save_char_obj: fopen" );
        send_to_char("Your character is having problems.\n\r", ch);
        perror( strsave );
    }
    else
    {
        fwrite_char( ch, fp );
        if ( ch->first_carrying )
            fwrite_obj( ch, ch->last_carrying, fp, 0, OS_CARRY );
        if ( ch->vars )
            fwrite_variables( ch->vars, fp );
        if ( ch->comments )                 /* comments */
            fwrite_comments( ch, fp );        /* comments */
        for (follower = ch->in_room->first_person; follower; follower = nextinroom)
        {
            nextinroom = follower->next_in_room;
            if ( (follower != ch) && (follower->master == ch) &&
                 IS_NPC(follower) )
            {
                fwrite_char(follower, fp);
                if (follower->first_carrying)
                    fwrite_obj( follower, follower->last_carrying, fp, 0, OS_CARRY );
                if (follower->vars)
                    fwrite_variables( follower->vars, fp );
            }
        }

#ifdef VTRACK
        if ( !IS_NPC(ch) && ch->pcdata->vtrack )
            fwrite_vtracks(ch, fp);
#endif

        fprintf( fp, "#END\n" );
        FCLOSE( fp );
    }

    ClassSpecificStuff(ch);

    re_equip_char( ch );

    write_corpses(ch, NULL);
    quitting_char = NULL;
    saving_char   = NULL;

    update_char_rare_obj(ch);

    return;
}



/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
    AFFECT_DATA *paf;
    ALIAS_DATA *pal;
    int i, sn, track, count;
    SKILLTYPE *skill = NULL;

    fprintf( fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER"		);

    fprintf( fp, "Version      %d\n",   SAVEVERSION		);
    if (IS_NPC(ch))
        fprintf( fp, "Vnum         %d\n",	ch->vnum	);
    fprintf( fp, "Name         %s~\n",	ch->name		);
    if ( ch->short_descr && ch->short_descr[0] != '\0' )
        fprintf( fp, "ShortDescr   %s~\n",	ch->short_descr	);
    if ( ch->long_descr && ch->long_descr[0] != '\0' )
        fprintf( fp, "LongDescr    %s~\n",	ch->long_descr	);
    if ( ch->description && ch->description[0] != '\0' )
        fprintf( fp, "Description  %s~\n",	ch->description	);
    if ( ch->intro_descr && ch->intro_descr[0] != '\0' )
        fprintf( fp, "IntroDescr    %s~\n",	ch->intro_descr	);
    fprintf( fp, "Sex          %d\n",	ch->sex			);
    fprintf( fp, "Race         %d\n",	ch->race		);
    fprintf( fp, "Languages    %d %d\n", ch->speaks, ch->speaking );

    for (i=0; i < MAX_CLASS; ++i) {
        if (HAS_CLASS(ch,i))
            fprintf( fp, "Level        %d %d %d\n", i,
                     ch->levels[i], ch->classes[i]);
    }

    if ( ch->trust )
        fprintf( fp, "Trust        %d\n",	ch->trust		);
    fprintf( fp, "Played       %d\n",
             ch->played + (int) (current_time - ch->logon)		);
    if (!IS_NPC(ch))
        fprintf( fp, "Room         %d\n", ch->pcdata->home);

    fprintf( fp, "HpManaMove   %d %d %d %d %d %d\n",
             GET_HIT(ch),  ch->max_hit,
             GET_MANA(ch), ch->max_mana,
             GET_MOVE(ch), 0 );
    fprintf( fp, "Regens       %d %d %d\n",
             ch->hit_regen, ch->mana_regen, ch->move_regen );

    if ( ch->gold2 )
    {
        GET_MONEY(ch,CURR_GOLD) += ch->gold2;
        ch->gold2 = 0;
    }
    if ( ch->bank2 )
    {
        GET_BALANCE(ch,CURR_GOLD) += ch->bank2;
        ch->bank2 = 0;
    }

    fprintf( fp, "Money       ");
    for (count=0; count < MAX_CURR_TYPE; count++ )
        fprintf(fp, " %d", GET_MONEY(ch,count)                          );
    fprintf(fp, " -1\n");
    fprintf( fp, "BankMoney   ");
    for (count=0; count < MAX_CURR_TYPE; count++ )
        fprintf(fp, " %d", GET_BALANCE(ch,count)                        );
    fprintf(fp, " -1\n");

    if ( !IS_NPC(ch) && (GET_INTF(ch) != INT_DEFAULT ) )
        fprintf( fp, "Interface    %d\n",   GET_INTF(ch)     	);
    if (ch->antimagicp)
        fprintf( fp, "AMP          %d\n",   ch->antimagicp             );
    if (ch->spellfail != 101)
        fprintf( fp, "Spellfail    %d\n",   ch->spellfail       );
    fprintf( fp, "Exp          %d\n",	ch->exp			);
    if ( ch->act )
        fprintf( fp, "Act          %d\n", ch->act			);
    if ( ch->act2 )
        fprintf( fp, "Act2         %d\n", ch->act2		);
    if ( ch->affected_by )
        fprintf( fp, "AffectedBy   %d\n",	ch->affected_by		);
    if ( ch->affected_by2 )
        fprintf( fp, "AffectedBy2  %d\n",	ch->affected_by2	);
    fprintf( fp, "Position     %d\n",
             ch->position == POS_FIGHTING ? POS_STANDING : ch->position );
    if ( ch->xflags )
        fprintf( fp, "Bodyparts    %d\n",	ch->xflags );

    fprintf( fp, "Practice     %d\n",	ch->practice		);
    fprintf( fp, "SavingThrows %d %d %d %d %d\n",
             ch->saving_poison_death,
             ch->saving_wand,
             ch->saving_para_petri,
             ch->saving_breath,
             ch->saving_spell_staff			);
    fprintf( fp, "Alignment    %d\n",	ch->alignment		);

    if (IS_NPC(ch)) {
        fprintf( fp, "Numattacks   %d\n",	ch->numattacks		);
    } else {
        fprintf( fp, "Favor        %d\n",	ch->pcdata->favor	);
        fprintf( fp, "Glory        %d\n",   ch->pcdata->quest_curr  );
        fprintf( fp, "MGlory       %d\n",   ch->pcdata->quest_accum );
    }
    fprintf( fp, "Hitroll      %d\n",	ch->hitroll		);
    fprintf( fp, "Damroll      %d\n",	ch->damroll		);
    fprintf( fp, "Armor        %d\n",	ch->armor		);
    if ( ch->wimpy )
        fprintf( fp, "Wimpy        %d\n",	ch->wimpy		);
    if ( ch->deaf )
        fprintf( fp, "Deaf         %d\n",	ch->deaf		);
    if ( !IS_NPC(ch) )
    {
        for(i=0;i<MAX_COLOR_TYPE;i++)
            if (ch->colors[i]!=def_color(i))
                fprintf( fp, "Color        %d %d\n", i, ch->colors[i]	);
    }
    if ( ch->resistant )
        fprintf( fp, "Resistant    %d\n",	ch->resistant		);
    if ( ch->immune )
        fprintf( fp, "Immune       %d\n",	ch->immune		);
    if ( ch->susceptible )
        fprintf( fp, "Susceptible  %d\n",	ch->susceptible		);
    if ( ch->absorb )
        fprintf( fp, "Absorb       %d\n",	ch->absorb		);

    if (IS_NPC(ch)) {
        fprintf( fp, "Mobinvis     %d\n",	ch->mobinvis		);
        fprintf( fp, "AttrPerm     %d %d %d %d %d %d %d\n",
                 ch->perm_str,
                 ch->perm_int,
                 ch->perm_wis,
                 ch->perm_dex,
                 ch->perm_con,
                 ch->perm_cha,
                 ch->perm_lck );

        fprintf( fp, "AttrMod      %d %d %d %d %d %d %d\n",
                 ch->mod_str,
                 ch->mod_int,
                 ch->mod_wis,
                 ch->mod_dex,
                 ch->mod_con,
                 ch->mod_cha,
                 ch->mod_lck );
    }
    else
    {
        if ( ch->pcdata->outcast_time )
            fprintf( fp, "Outcast_time %d\n",(int)ch->pcdata->outcast_time );
        if ( ch->pcdata->restore_time )
            fprintf( fp, "Restore_time %d\n",(int)ch->pcdata->restore_time );
        if ( IS_IMMORTAL(ch) )
        {
            if ( !ch->pcdata->time_immortal )
                ch->pcdata->time_immortal = current_time;
            fprintf( fp, "Time_Imm     %d\n",(int)ch->pcdata->time_immortal );
        }
        if ( ch->pcdata->time_to_die )
            fprintf( fp, "Time_To_Die  %d\n",(int)ch->pcdata->time_to_die );
        if ( !ch->pcdata->time_created )
            ch->pcdata->time_created = current_time;
        if ( ch->pcdata->time_created )
            fprintf( fp, "Time_Created %d\n",(int)ch->pcdata->time_created );

        /* increment times_played if logged in for more than 5 minutes but
           be careful to only do it once via inc_times_played */
        if ( !ch->pcdata->inc_times_played && ch->save_time > ch->logon+(5*60))
        {
            ch->pcdata->inc_times_played = TRUE;
            ch->pcdata->times_played++;
        }
        fprintf( fp, "Times_played %d\n", ch->pcdata->times_played );

        if ( ch->mental_state != -10 )
            fprintf( fp, "Mentalstate  %d\n",	ch->mental_state	);
        fprintf( fp, "Password     %s~\n",	ch->pcdata->pwd		);
        for ( pal = ch->pcdata->first_alias; pal; pal = pal->next )
        {
            if ( !pal->name || !pal->cmd || !*pal->name || !*pal->cmd )
                continue;
            fprintf( fp, "Alias        %s~ %s~ %d\n",
                     pal->name,
                     pal->cmd,
                     pal->flags
                   );
        }
        if ( ch->pcdata->bamfin && ch->pcdata->bamfin[0] != '\0' )
            fprintf( fp, "Bamfin       %s~\n",	ch->pcdata->bamfin	);
        if ( ch->pcdata->bamfout && ch->pcdata->bamfout[0] != '\0' )
            fprintf( fp, "Bamfout      %s~\n",	ch->pcdata->bamfout	);
        if ( ch->pcdata->rank && ch->pcdata->rank[0] != '\0' )
            fprintf( fp, "Rank         %s~\n",	ch->pcdata->rank	);
        if ( ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0' )
            fprintf( fp, "Bestowments  %s~\n", 	ch->pcdata->bestowments );
        fprintf( fp, "Title        %s~\n",	ch->pcdata->title	);
        if ( ch->pcdata->homepage && ch->pcdata->homepage[0] != '\0' )
            fprintf( fp, "Homepage     %s~\n",	ch->pcdata->homepage	);
        if ( ch->pcdata->bio && ch->pcdata->bio[0] != '\0' )
            fprintf( fp, "Bio          %s~\n",	ch->pcdata->bio 	);
        if ( ch->pcdata->authed_by && ch->pcdata->authed_by[0] != '\0' )
            fprintf( fp, "AuthedBy     %s~\n",	ch->pcdata->authed_by	);
        if ( ch->pcdata->min_snoop )
            fprintf( fp, "Minsnoop     %d\n",	ch->pcdata->min_snoop	);
        if ( ch->pcdata->prompt && *ch->pcdata->prompt )
            fprintf( fp, "Prompt       %s~\n",	ch->pcdata->prompt	);
        if ( ch->pcdata->pagerlen != 24 )
            fprintf( fp, "Pagerlen     %d\n",	ch->pcdata->pagerlen	);

        if ( IS_IMMORTAL( ch ) )
        {
            fprintf( fp, "WizInvis     %d\n",   ch->pcdata->wizinvis    );

            for (i=LOG_NORMAL;i<LOG_LAST;i++)
                if ( ch->pcdata->log_severity[i] != SEV_DEFAULT )
                    fprintf( fp, "LogSeverity  %d %s~\n",
                             ch->pcdata->log_severity[i],
                             sysdata.logdefs[i].name);
            for (i=LOG_NORMAL;i<LOG_LAST;i++)
                if ( ch->pcdata->afk_log_severity[i] != SEV_DEFAULT )
                    fprintf( fp, "AFKSeverity  %d %s~\n",
                             ch->pcdata->afk_log_severity[i],
                             sysdata.logdefs[i].name);
        }

        if ( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
            fprintf( fp, "RoomRange    %d %d\n", ch->pcdata->r_range_lo,
                     ch->pcdata->r_range_hi	);
        if ( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
            fprintf( fp, "ObjRange     %d %d\n", ch->pcdata->o_range_lo,
                     ch->pcdata->o_range_hi	);
        if ( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
            fprintf( fp, "MobRange     %d %d\n", ch->pcdata->m_range_lo,
                     ch->pcdata->m_range_hi	);

        if ( ch->pcdata->council)
            fprintf( fp, "Council      %s~\n", 	ch->pcdata->council_name );
        if ( ch->pcdata->deity_name && ch->pcdata->deity_name[0] != '\0' )
            fprintf( fp, "Deity	     %s~\n",	ch->pcdata->deity_name	 );
        if ( ch->pcdata->clan_name && ch->pcdata->clan_name[0] != '\0' )
            fprintf( fp, "Clan         %s~\n",	ch->pcdata->clan_name	);
        fprintf( fp, "Flags        %d\n",	ch->pcdata->flags	);
        if ( ch->pcdata->release_date > current_time )
            fprintf( fp, "Helled       %d %s~\n",
                     (int)ch->pcdata->release_date, ch->pcdata->helled_by );
        if ( ch->pcdata->pkills )
            fprintf( fp, "PKills       %d\n",	ch->pcdata->pkills	);
        if ( ch->pcdata->pdeaths )
            fprintf( fp, "PDeaths      %d\n",	ch->pcdata->pdeaths	);
        if ( get_timer( ch , TIMER_PKILLED)
             && ( get_timer( ch , TIMER_PKILLED) > 0 ) )
            fprintf( fp, "PTimer       %d\n",     get_timer(ch, TIMER_PKILLED));
        fprintf( fp, "MKills       %d\n",	ch->pcdata->mkills	);
        fprintf( fp, "MDeaths      %d\n",	ch->pcdata->mdeaths	);
        if ( ch->pcdata->illegal_pk )
            fprintf( fp, "IllegalPK    %d\n",	ch->pcdata->illegal_pk	);
        fprintf( fp, "AttrPerm     %d %d %d %d %d %d %d\n",
                 ch->perm_str,
                 ch->perm_int,
                 ch->perm_wis,
                 ch->perm_dex,
                 ch->perm_con,
                 ch->perm_cha,
                 ch->perm_lck );

        fprintf( fp, "AttrMod      %d %d %d %d %d %d %d\n",
                 ch->mod_str,
                 ch->mod_int,
                 ch->mod_wis,
                 ch->mod_dex,
                 ch->mod_con,
                 ch->mod_cha,
                 ch->mod_lck );

        fprintf( fp, "Condition    %d %d %d %d\n",
                 GET_COND(ch, 0),
                 GET_COND(ch, 1),
                 GET_COND(ch, 2),
                 GET_COND(ch, 3) );
        if ( ch->desc && ch->desc->host )
            fprintf( fp, "Site         %s\n", ch->desc->host );
        else
            fprintf( fp, "Site         (Link-Dead)\n" );
        if ( ch->desc && ch->desc->user )
            fprintf( fp, "User         %s\n", ch->desc->user );
        else
            fprintf( fp, "User         None\n" );

        for ( sn = 1; sn < top_sn; sn++ )
        {
            if ( !skill_table[sn]->name )
                continue;

            if ( LEARNED(ch, sn) > 0 )
                switch( skill_table[sn]->type )
                {
                default:
                    fprintf( fp, "Skill        %d '%s'\n",
			    LEARNED(ch, sn), skill_table[sn]->name );
                    break;
                case SKILL_SPELL:
		    fprintf( fp, "Spell        %d '%s'\n",
			    LEARNED(ch, sn), skill_table[sn]->name );
                    break;
                case SKILL_WEAPON:
		    fprintf( fp, "Weapon       %d '%s'\n",
			    LEARNED(ch, sn), skill_table[sn]->name );
		    break;
                case SKILL_TONGUE:
		    fprintf( fp, "Tongue       %d '%s'\n",
			    LEARNED(ch, sn), skill_table[sn]->name );
                    break;
                case SKILL_LORE:
                    fprintf( fp, "Lore         %d '%s'\n",
			    LEARNED(ch, sn), skill_table[sn]->name );
                    break;
                case SKILL_PSISPELL:
		    fprintf( fp, "PsiSpell     %d '%s'\n",
			    LEARNED(ch, sn), skill_table[sn]->name );
		    break;
                }

            if ( MEMORIZED(ch, sn) )
                fprintf( fp, "Memorized    %d '%s'\n",
                         MEMORIZED(ch, sn), skill_table[sn]->name );
        }
    }

    for ( paf = ch->first_affect; paf; paf = paf->next )
    {
        if ( paf->type >= 0 && (skill=get_skilltype(paf->type)) == NULL )
            continue;

        if ( paf->type >= 0 && paf->type < TYPE_PERSONAL )
            fprintf( fp, "AffectData   '%s' %3d %3d %3d %10d\n",
                     skill->name,
                     paf->duration,
                     paf->modifier,
                     paf->location,
                     paf->bitvector
                   );
        else
            fprintf( fp, "Affect       %3d %3d %3d %3d %10d\n",
                     paf->type,
                     paf->duration,
                     paf->modifier,
                     paf->location,
                     paf->bitvector
                   );
    }


    if (!IS_NPC(ch))
    {
       track = URANGE( 2, ((GetMaxLevel(ch)+3) * MAX_KILLTRACK)/LEVEL_AVATAR,MAX_KILLTRACK );
       for ( sn = 0; sn < track; sn++ )
       {
           if ( ch->pcdata->killed[sn].vnum == 0 )
               break;
           fprintf( fp, "Killed       %d %d\n",
                     ch->pcdata->killed[sn].vnum,
                     ch->pcdata->killed[sn].count );
       }

#ifdef I3
       i3save_char(ch, fp);
#endif
#ifdef IMC
       imc_savechar(ch, fp);
#endif
    }

    fprintf( fp, "End\n\n" );
    return;
}

/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest,
                 sh_int os_type )
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;
    sh_int wear, wear_loc, x;

    if ( iNest >= MAX_NEST )
    {
        bug( "fwrite_obj: iNest hit MAX_NEST %d", iNest );
        return;
    }

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->prev_content && os_type != OS_CORPSE )
        fwrite_obj( ch, obj->prev_content, fp, iNest, OS_CARRY );

    /*
     * Castrate storage characters.
     */
    if ( obj->item_type == ITEM_KEY && !IS_OBJ_STAT(obj, ITEM_CLANOBJECT) )
        return;

    /*
     * Catch deleted objects					-Thoric
     */
    if ( obj_extracted(obj) )
        return;

    /*
     * Do NOT save prototype items!				-Thoric
     */
    if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
        return;

    /*
     * Don't save objs with negative rent
     */
    if ( obj->rent < 0 )
        return;

    /* Corpse saving. -- Altrag */
    fprintf( fp, (os_type == OS_CORPSE ? "#CORPSE\n" : "#OBJECT\n") );

    fprintf( fp, "* %s\n", obj->name );
    if ( iNest )
        fprintf( fp, "Nest         %d\n",	iNest		     );
    if ( obj->count > 1 )
        fprintf( fp, "Count        %d\n",	obj->count	     );
    if ( QUICKMATCH( obj->name, obj->pIndexData->name ) == 0 )
        fprintf( fp, "Name         %s~\n",	obj->name	     );
    if ( QUICKMATCH( obj->short_descr, obj->pIndexData->short_descr ) == 0 )
        fprintf( fp, "ShortDescr   %s~\n",	obj->short_descr     );
    if ( QUICKMATCH( obj->description, obj->pIndexData->description ) == 0 )
        fprintf( fp, "Description  %s~\n",	obj->description     );
    if ( QUICKMATCH( obj->action_desc, obj->pIndexData->action_desc ) == 0 )
        fprintf( fp, "ActionDesc   %s~\n",	obj->action_desc     );
    fprintf( fp, "Vnum         %d\n",	obj->vnum	     );
    if ( obj->christened )
        fprintf( fp, "CVnum        %d\n",	obj->christened->cvnum);
    if ( os_type == OS_CORPSE && obj->in_room )
        fprintf( fp, "Room         %d\n",   obj->in_room->vnum       );
    if ( obj->extra_flags != obj->pIndexData->extra_flags )
        fprintf( fp, "ExtraFlags   %d\n",	obj->extra_flags     );
    if ( obj->extra_flags2 != obj->pIndexData->extra_flags2 )
        fprintf( fp, "ExtraFlags2  %d\n",	obj->extra_flags2    );
    if ( obj->magic_flags != obj->pIndexData->magic_flags )
        fprintf( fp, "MagicFlags   %d\n",	obj->magic_flags     );
    if ( obj->wear_flags != obj->pIndexData->wear_flags )
        fprintf( fp, "WearFlags    %d\n",	obj->wear_flags	     );
    wear_loc = -1;
    for ( wear = 0; wear < MAX_WEAR; wear++ )
        for ( x = 0; x < MAX_LAYERS; x++ )
            if ( obj == save_equipment[wear][x] )
            {
                wear_loc = wear;
                break;
            }
            else
                if ( !save_equipment[wear][x] )
                    break;
    if ( wear_loc != -1 )
        fprintf( fp, "WearLoc      %d\n",	wear_loc	     );
    if ( obj->item_type != obj->pIndexData->item_type )
        fprintf( fp, "ItemType     %d\n",	obj->item_type	     );
    if ( obj->weight != obj->pIndexData->weight )
        fprintf( fp, "Weight       %d\n",	obj->weight		     );
    if ( obj->timer )
        fprintf( fp, "Timer        %d\n",	obj->timer		     );
    if ( obj->cost != obj->pIndexData->cost )
        fprintf( fp, "Cost         %d\n",	obj->cost		     );
    if ( obj->currtype != obj->pIndexData->currtype )
        fprintf( fp, "CurrType     %d\n",	obj->currtype		     );
    if (is_rare_obj(obj))
        fprintf( fp, "Rent         %d\n",	obj->rent		     );
    if ( obj->value[0] || obj->value[1] || obj->value[2]
         ||   obj->value[3] || obj->value[4] || obj->value[5] )
        fprintf( fp, "Values       %d %d %d %d %d %d\n",
                 obj->value[0], obj->value[1], obj->value[2],
                 obj->value[3], obj->value[4], obj->value[5]     );

    switch ( obj->item_type )
    {
    case ITEM_PILL: /* was down there with staff and wand, wrongly - Scryn */
    case ITEM_POTION:
    case ITEM_SCROLL:
        if ( IS_VALID_SN(obj->value[1]) )
            fprintf( fp, "Spell 1      '%s'\n",
                     skill_table[obj->value[1]]->name );

        if ( IS_VALID_SN(obj->value[2]) )
            fprintf( fp, "Spell 2      '%s'\n",
                     skill_table[obj->value[2]]->name );

        if ( IS_VALID_SN(obj->value[3]) )
            fprintf( fp, "Spell 3      '%s'\n",
                     skill_table[obj->value[3]]->name );

        break;

    case ITEM_STAFF:
    case ITEM_WAND:
        if ( IS_VALID_SN(obj->value[3]) )
            fprintf( fp, "Spell 3      '%s'\n",
                     skill_table[obj->value[3]]->name );

        break;
    case ITEM_SALVE:
        if ( IS_VALID_SN(obj->value[4]) )
            fprintf( fp, "Spell 4      '%s'\n",
                     skill_table[obj->value[4]]->name );

        if ( IS_VALID_SN(obj->value[5]) )
            fprintf( fp, "Spell 5      '%s'\n",
                     skill_table[obj->value[5]]->name );
        break;
    }

    for ( paf = obj->first_affect; paf; paf = paf->next )
    {
        /*
         * Save extra object affects				-Thoric
         */
        if ( paf->type < 0 || paf->type >= top_sn )
        {
            fprintf( fp, "Affect       %d %d %d %d %d\n",
                     paf->type,
                     paf->duration,
                     ((paf->location == APPLY_WEAPONSPELL
                       || paf->location == APPLY_WEARSPELL
                       || paf->location == APPLY_REMOVESPELL
                       || paf->location == APPLY_EAT_SPELL
                       || paf->location == APPLY_IMMUNESPELL
                       || paf->location == APPLY_STRIPSN)
                      && IS_VALID_SN(paf->modifier))
                     ? skill_table[paf->modifier]->slot : paf->modifier,
                     paf->location,
                     paf->bitvector
                   );
        }
        else
            fprintf( fp, "AffectData   '%s' %d %d %d %d\n",
                     skill_table[paf->type]->name,
                     paf->duration,
                     ((paf->location == APPLY_WEAPONSPELL
                       || paf->location == APPLY_WEARSPELL
                       || paf->location == APPLY_REMOVESPELL
                       || paf->location == APPLY_EAT_SPELL
                       || paf->location == APPLY_IMMUNESPELL
                       || paf->location == APPLY_STRIPSN)
                      && IS_VALID_SN(paf->modifier))
                     ? skill_table[paf->modifier]->slot : paf->modifier,
                     paf->location,
                     paf->bitvector
                   );
    }

    for ( ed = obj->first_extradesc; ed; ed = ed->next )
        fprintf( fp, "ExtraDescr   %s~ %s~\n",
                 ed->keyword, ed->description );

    if ( obj->nickname )
        fprintf( fp, "Nickname     %s~\n",
                 obj->nickname );

    if ( obj->vars )
        fwrite_variables( obj->vars, fp );

    fprintf( fp, "End\n\n" );

    if ( obj->first_content )
        fwrite_obj( ch, obj->last_content, fp, iNest + 1, OS_CARRY );

    return;
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, char *name, bool preload,
                    ROOM_INDEX_DATA *PetRoom, bool loadpets )
{
    char strsave[MAX_INPUT_LENGTH];
    CHAR_DATA *ch, *mob = NULL, *lastloaded = NULL;
    FILE *fp;
    bool found;
    struct stat fst;
    int i, x;
    extern FILE *fpArea;
    extern char strArea[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];


    ch = new_char();

    for ( x = 0; x < MAX_WEAR; x++ )
        for ( i = 0; i < MAX_LAYERS; i++ )
            save_equipment[x][i] = NULL;
    clear_char( ch );
    loading_char = ch;

    CREATE( ch->pcdata, PC_DATA, 1 );
    d->character			= ch;
    ch->desc				= d;
    ch->name				= STRALLOC( name );
    ch->act				= PLR_BLANK
        | PLR_COMBINE
        | PLR_PROMPT;
    ch->act2				= 0;
    ch->gold2				= 0;
    ch->bank2				= 0;
    ch->pcdata->interface		= INT_DEFAULT;
    ch->perm_str			= 13;
    ch->perm_int			= 13;
    ch->perm_wis			= 13;
    ch->perm_dex			= 13;
    ch->perm_con			= 13;
    ch->perm_cha			= 13;
    ch->perm_lck			= 13;
    ch->pcdata->condition[COND_THIRST]	= MAX_COND_VAL;
    ch->pcdata->condition[COND_FULL]	= MAX_COND_VAL;
    ch->pcdata->condition[COND_BLOODTHIRST] = 10;
    ch->pcdata->wizinvis		= 0;
    for (i = LOG_NORMAL; i < LOG_LAST; i++)
    {
        ch->pcdata->log_severity[i]    	= SEV_DEFAULT;
        ch->pcdata->afk_log_severity[i] = SEV_DEFAULT;
    }
    ch->mental_state			= -10;
    ch->mobinvis			= 0;
    for(i = 0; i < MAX_SKILL; i++)
        ch->pcdata->learned[i]		= 0;
    ch->pcdata->release_date		= 0;
    ch->pcdata->helled_by		= NULL;
    ch->pcdata->outcast_time            = 0;
    ch->pcdata->time_to_die             = 0;
    ch->pcdata->time_created            = 0;
    ch->pcdata->time_immortal           = 0;
    ch->pcdata->inc_times_played        = FALSE;
    ch->pcdata->times_played            = 0;
    ch->saving_poison_death 		= 0;
    ch->saving_wand			= 0;
    ch->saving_para_petri		= 0;
    ch->saving_breath			= 0;
    ch->saving_spell_staff		= 0;
    ch->comments                        = NULL;    /* comments */
    ch->pcdata->pagerlen		= 24;

#ifdef IMC
    imc_initchar( ch );
#endif
#ifdef I3
    i3init_char( ch );
#endif

    found = FALSE;
    sprintf( strsave, "%s%c/%s", PLAYER_DIR, tolower(name[0]),
             capitalize( name ) );
    if ( stat( strsave, &fst ) != -1 )
    {
        if ( fst.st_size == 0 )
        {
            sprintf( strsave, "%s%c/%s", BACKUP_DIR, tolower(name[0]),
                     capitalize( name ) );
            send_to_char( "Restoring your backup player file...", ch );
        }
        else
        {
            sprintf( buf, "%s player data for: %s (%dK)",
                     preload ? "Preloading" : "Loading", ch->name,
                     (int) fst.st_size/1024 );
            log_string_plus( buf, LOG_COMM, LEVEL_GREATER, SEV_INFO );
        }
    }
    /* else no player file */

    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
        int iNest;

        for ( iNest = 0; iNest < MAX_NEST; iNest++ )
            rgObjNest[iNest] = NULL;

        found = TRUE;
        /* Cheat so that bug will show line #'s -- Altrag */
        fpArea = fp;
        strcpy(strArea, strsave);
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
                bug( "Load_char_obj: # not found, got %c instead in %s.",
                     letter, name );
                break;
            }

            word = fread_word( fp );
            if ( !str_cmp( word, "PLAYER" ) )
            {
                fread_char ( ch, fp, preload );
                lastloaded = ch;
                if ( preload )
                    break;
            }
            else if ( !str_cmp( word, "OBJECT" ) )	/* Objects	*/
            {
                if (!lastloaded) lastloaded = ch;
                fread_obj  ( lastloaded, fp, OS_CARRY );
            }
            else if ( !str_cmp( word, "VARIABLE" ) )
            {
                fread_variable(&ch->vars, fp);
            }
            else if ( !str_cmp( word, "COMMENT") )
            {
                fread_comment(ch, fp );		/* Comments	*/
            }
#ifdef VTRACK
            else if ( !str_cmp( word, "VTRACK" ) )
            {
                fread_vtracks(ch, fp);
            }
#endif
            else if ( !str_cmp( word, "END"    ) )	/* Done		*/
            {
                break;
            }
            else if ( !str_cmp( word, "MOB" ) )
            {

                if (!loadpets) {
                    bug( "Load_char_obj: !loadpets, skipping." );
                    break;
                }
                mob = fread_char(mob, fp, preload);
                if (mob) {
                    lastloaded = mob;
                    if (ch) add_follower(mob, ch);
                    if (PetRoom)
                    { char_to_room(mob, PetRoom); }
                    else if (ch)
                    { char_to_room(mob, get_room_index(ch->pcdata->home)); }
                    else extract_char(mob, TRUE);
                }
            }
            else
            {
                bug( "Load_char_obj: bad section %s for %s.",
                     word?word:"(null)", name );
                break;
            }
        }
        FCLOSE( fp );
        fpArea = NULL;
        strcpy(strArea, "$");
    }


    if ( !found )
    {
        if (d)
        {
#ifdef MXP
            if (d->mxp_detected)
                SET_PLR2_FLAG(ch, PLR2_MXP);
#endif
#ifdef MSP
            if (d->msp_detected)
                SET_PLR2_FLAG(ch, PLR2_MSP);
#endif
        }
        ch->short_descr			= STRALLOC( "" );
        ch->long_descr			= STRALLOC( "" );
        ch->description			= STRALLOC( "" );
        ch->intro_descr			= str_dup( "" );
        ch->editor			= NULL;
        ch->pcdata->clan_name		= STRALLOC( "" );
        ch->pcdata->clan		= NULL;
        ch->pcdata->council_name 	= STRALLOC( "" );
        ch->pcdata->council 		= NULL;
        ch->pcdata->deity_name		= STRALLOC( "" );
        ch->pcdata->deity		= NULL;
        ch->pcdata->first_alias		= NULL;
        ch->pcdata->last_alias		= NULL;
        ch->pcdata->pwd			= str_dup( "" );
        ch->pcdata->bamfin		= STRALLOC( "" );
        ch->pcdata->bamfout		= STRALLOC( "" );
        ch->pcdata->rank		= str_dup( "Mortal" );
        ch->pcdata->bestowments		= str_dup( "" );
        ch->pcdata->title		= STRALLOC( "" );
        ch->pcdata->homepage		= str_dup( "" );
        ch->pcdata->bio 		= STRALLOC( "" );
        ch->pcdata->authed_by		= STRALLOC( "" );
        ch->pcdata->prompt		= STRALLOC( "" );
        ch->pcdata->r_range_lo		= 0;
        ch->pcdata->r_range_hi		= 0;
        ch->pcdata->m_range_lo		= 0;
        ch->pcdata->m_range_hi		= 0;
        ch->pcdata->o_range_lo		= 0;
        ch->pcdata->o_range_hi		= 0;
        ch->pcdata->wizinvis		= 0;
        for (i = LOG_NORMAL; i < LOG_LAST; i++)
        {
            ch->pcdata->log_severity[i] = SEV_DEFAULT;
            ch->pcdata->afk_log_severity[i] = SEV_DEFAULT;
        }
        ch->pcdata->game_board		= NULL;
        ch->pcdata->time_created        = current_time;
        ch->pcdata->time_to_die         = 0;
        ch->pcdata->time_immortal       = 0;
        ch->pcdata->inc_times_played    = FALSE;
        ch->pcdata->times_played        = 0;
    }
    else
    {
        if ( !ch->pcdata->clan_name )
        {
            ch->pcdata->clan_name	= STRALLOC( "" );
            ch->pcdata->clan	= NULL;
        }
        if ( !ch->pcdata->council_name )
        {
            ch->pcdata->council_name = STRALLOC( "" );
            ch->pcdata->council 	= NULL;
        }
        if ( !ch->pcdata->deity_name )
        {
            ch->pcdata->deity_name = STRALLOC( "" );
            ch->pcdata->deity	 = NULL;
        }
        if ( !ch->pcdata->bio )
            ch->pcdata->bio	 = STRALLOC( "" );

        if ( !ch->pcdata->authed_by )
            ch->pcdata->authed_by	 = STRALLOC( "" );

        if ( !IS_NPC( ch ) && IS_IMMORTAL( ch ) )
        {
            assign_area( ch );
        }
        if ( file_ver > 1 )
        {
            for ( i = 0; i < MAX_WEAR; i++ )
                for ( x = 0; x < MAX_LAYERS; x++ )
                    if ( save_equipment[i][x] )
                    {
                        equip_char( ch, save_equipment[i][x], i );
                        save_equipment[i][x] = NULL;
                    }
                    else
                        break;
        }
        if (ch->trust > 0)
            log_printf_plus(LOG_DEBUG, GetMaxLevel(ch), SEV_SPAM, "load_char_obj: trust: %d", ch->trust);
        ClassSpecificStuff(ch);

        update_speaks(ch);
    }

#ifndef PLR2_AUTOGAIN
    if (IS_PLR2_FLAG(ch, BV05))
        REMOVE_PLR2_FLAG(ch, BV05);
#endif
    if (IS_PLR_FLAG(ch, PLR_AFK))
	REMOVE_PLR_FLAG(ch, PLR_AFK);

    loading_char = NULL;

    update_char_rare_obj(ch);

    return found;
}



/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
    if ( !str_cmp( word, literal ) )	\
    {					\
    field  = value;			\
    fMatch = TRUE;			\
    break;				\
    }

CHAR_DATA *fread_char( CHAR_DATA *ch, FILE *fp, bool preload )
{
    char buf[MAX_STRING_LENGTH];
    char *line = NULL;
    const char *word = NULL;
    int x1, x2, x3, x4, x5, x6, x7, i;
    sh_int killcnt;
    bool fMatch = FALSE;

    file_ver = 0;
    killcnt = 0;
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
            KEY( "Act",		ch->act,		fread_number( fp ) );
            KEY( "Act2",	ch->act2,		fread_number( fp ) );
            KEY( "AffectedBy",	ch->affected_by,	fread_number( fp ) );
            KEY( "AffectedBy2",	ch->affected_by2,	fread_number( fp ) );
            KEY( "Alignment",	ch->alignment,		fread_number( fp ) );
            KEY( "AMP",         ch->antimagicp,		fread_number( fp ) );
            KEY( "Armor",	ch->armor,		fread_number( fp ) );
            KEY( "Absorb",	ch->absorb,		fread_number( fp ) );
            KEY( "AuthedBy",	ch->pcdata->authed_by,	fread_string( fp ) );

            if ( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
            {
                AFFECT_DATA *paf;

                if ( preload )
                {
                    fMatch = TRUE;
                    fread_to_eol( fp );
                    break;
                }
                CREATE( paf, AFFECT_DATA, 1 );
                if ( !str_cmp( word, "Affect" ) )
                {
                    paf->type	= fread_number( fp );
                }
                else
                {
                    int sn;
                    char *sname = fread_word(fp);

                    if ( (sn=skill_lookup(sname)) < 0 )
                    {
                        if ( (sn=herb_lookup(sname)) < 0 )
                            bug( "Fread_char: unknown skill." );
                        else
                            sn += TYPE_HERB;
                    }
                    paf->type = sn;
                }

                paf->duration	= fread_number( fp );
                paf->modifier	= fread_number( fp );
                paf->location	= fread_number( fp );
                paf->bitvector	= fread_number( fp );
                LINK(paf, ch->first_affect, ch->last_affect, next, prev );
                char_affects++;
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "Alias" ) )
            {
                ALIAS_DATA *pal;

                if ( preload )
                {
                    fMatch = TRUE;
                    fread_to_eol( fp );
                    break;
                }
                CREATE( pal, ALIAS_DATA, 1 );

                pal->name	= fread_string_nohash( fp );
                pal->cmd	= fread_string_nohash( fp );

                line = fread_line( fp );
                x1=0;
                sscanf( line, "%d", &x1 );
                pal->flags = x1;

                LINK(pal, ch->pcdata->first_alias, ch->pcdata->last_alias, next, prev );
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "AttrMod"  ) )
            {
                line = fread_line( fp );
                x1=x2=x3=x4=x5=x6=x7=13;
                sscanf( line, "%d %d %d %d %d %d %d",
                        &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
                ch->mod_str = x1;
                ch->mod_int = x2;
                ch->mod_wis = x3;
                ch->mod_dex = x4;
                ch->mod_con = x5;
                ch->mod_cha = x6;
                ch->mod_lck = x7;
                if (!x7)
                    ch->mod_lck = 0;
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "AttrPerm" ) )
            {
                line = fread_line( fp );
                x1=x2=x3=x4=x5=x6=x7=0;
                sscanf( line, "%d %d %d %d %d %d %d",
                        &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
                ch->perm_str = x1;
                ch->perm_int = x2;
                ch->perm_wis = x3;
                ch->perm_dex = x4;
                ch->perm_con = x5;
                ch->perm_cha = x6;
                ch->perm_lck = x7;
                if (!x7 || x7 == 0)
                    ch->perm_lck = 13;
                fMatch = TRUE;
                break;
            }
            if ( !str_cmp( word, "AFKSeverity" ) )
            {
                char *logname;

                x1 = fread_number(fp);
                logname = fread_string_nohash(fp);

                for (i = LOG_NORMAL; i < LOG_LAST; i++)
                    if (!str_cmp(logname, sysdata.logdefs[i].name))
                    {
                        ch->pcdata->afk_log_severity[i] = x1;
                        break;
                    }

                DISPOSE(logname);

                fMatch = TRUE;
                break;
            }
            break;

        case 'B':
            KEY( "Bamfin",	ch->pcdata->bamfin,	fread_string( fp ) );
            KEY( "Bamfout",	ch->pcdata->bamfout,	fread_string( fp ) );
            KEY( "Bank",	ch->bank2,		fread_number( fp ) );
            KEY( "Bestowments", ch->pcdata->bestowments,fread_string_nohash( fp ) );
            KEY( "Bio",		ch->pcdata->bio,	fread_string( fp ) );
            KEY( "Bodyparts",	ch->xflags,		fread_number( fp ) );
            if ( !str_cmp( word, "BankMoney" ) )
            {
                x1=x2=0;
                while ((x1=fread_number(fp))>=0)
                    if (x2<MAX_CURR_TYPE)
                        GET_BALANCE(ch,x2++) = x1;
                fMatch = TRUE;
                break;
            }
            break;

        case 'C':
            if ( !str_cmp( word, "Clan" ) )
            {
                ch->pcdata->clan_name = fread_string( fp );

                if ( !preload
                     &&   ch->pcdata->clan_name[0] != '\0'
                     && ( ch->pcdata->clan = get_clan( ch->pcdata->clan_name )) == NULL )
                {
                    sprintf( buf, "Warning: the organization %s no longer exists, and therefore you no longer\n\rbelong to that organization.\n\r",
                             ch->pcdata->clan_name );
                    send_to_char( buf, ch );
                    STRFREE( ch->pcdata->clan_name );
                    ch->pcdata->clan_name = STRALLOC( "" );
                }
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "Color" ) )
            {
                x1=x2=0;
                line = fread_line( fp );
                sscanf( line, "%d %d",
                        &x1, &x2 );
                if ( x1 < 0 ||  x1 > MAX_COLOR_TYPE)
                {
                    bug( "Fread_char: Unknown color index." );
                    break;
                }
                ch->colors[x1] = x2;
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "Condition" ) )
            {
                line = fread_line( fp );
                sscanf( line, "%d %d %d %d",
                        &x1, &x2, &x3, &x4 );
                ch->pcdata->condition[0] = x1;
                ch->pcdata->condition[1] = x2;
                ch->pcdata->condition[2] = x3;
                ch->pcdata->condition[3] = x4;
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "Council" ) )
            {
                ch->pcdata->council_name = fread_string( fp );
                if ( !preload
                     &&   ch->pcdata->council_name[0] != '\0'
                     && ( ch->pcdata->council = get_council( ch->pcdata->council_name )) == NULL )
                {
                    sprintf( buf, "Warning: the council %s no longer exists, and herefore you no longer\n\rbelong to a council.\n\r",
                             ch->pcdata->council_name );
                    send_to_char( buf, ch );
                    STRFREE( ch->pcdata->council_name );
                    ch->pcdata->council_name = STRALLOC( "" );
                }
                fMatch = TRUE;
                break;
            }
            break;

        case 'D':
            KEY( "Damroll",	ch->damroll,		fread_number( fp ) );
            KEY( "Deaf",	ch->deaf,		fread_number( fp ) );
            if ( !str_cmp( word, "Deity" ) )
            {
                ch->pcdata->deity_name = fread_string( fp );

                if ( !preload
                     &&   ch->pcdata->deity_name[0] != '\0'
                     && ( ch->pcdata->deity = get_deity( ch->pcdata->deity_name )) == NULL )
                {
                    sprintf( buf, "Warning: the deity %s no longer exists.\n\r",
                             ch->pcdata->deity_name );
                    send_to_char( buf, ch );
                    STRFREE( ch->pcdata->deity_name );
                    ch->pcdata->deity_name = STRALLOC( "" );
                    ch->pcdata->favor = 0;
                }
                fMatch = TRUE;
                break;
            }
            KEY( "Description",	ch->description,	fread_string( fp ) );
            break;

            /* 'E' was moved to after 'S' */
        case 'F':
            KEY( "Favor",	ch->pcdata->favor,	fread_number( fp ) );
            KEY( "Flags",	ch->pcdata->flags,	fread_number( fp ) );
            break;

        case 'G':
            KEY( "Glory",       ch->pcdata->quest_curr, fread_number( fp ) );
            KEY( "Gold",	ch->gold2,		fread_number( fp ) );
            /* temporary measure */
            if ( !str_cmp( word, "Guild" ) )
            {
                ch->pcdata->clan_name = fread_string( fp );

                if ( !preload
                     &&   ch->pcdata->clan_name[0] != '\0'
                     && ( ch->pcdata->clan = get_clan( ch->pcdata->clan_name )) == NULL )
                {
                    sprintf( buf, "Warning: the organization %s no longer exists, and therefore you no longer\n\rbelong to that organization.\n\r",
                             ch->pcdata->clan_name );
                    send_to_char( buf, ch );
                    STRFREE( ch->pcdata->clan_name );
                    ch->pcdata->clan_name = STRALLOC( "" );
                }
                fMatch = TRUE;
                break;
            }
            break;

        case 'H':
            if ( !str_cmp(word, "Helled") )
            {
                ch->pcdata->release_date = fread_number(fp);
                ch->pcdata->helled_by = fread_string(fp);
                if ( ch->pcdata->release_date < current_time )
                {
                    STRFREE(ch->pcdata->helled_by);
                    ch->pcdata->helled_by = NULL;
                    ch->pcdata->release_date = 0;
                }
                fMatch = TRUE;
                break;
            }

            KEY( "Hitroll",	ch->hitroll,		fread_number( fp ) );
            KEY( "Homepage",	ch->pcdata->homepage,	fread_string_nohash( fp ) );

            if ( !str_cmp( word, "HpManaMove" ) )
            {
                GET_HIT(ch)	= fread_number( fp );
                ch->max_hit	= fread_number( fp );
                GET_MANA(ch)	= fread_number( fp );
                ch->max_mana	= fread_number( fp );
                GET_MOVE(ch)	= fread_number( fp );
                ch->max_move	= fread_number( fp );
                fMatch = TRUE;
                break;
            }
            break;

        case 'I':
            KEY( "IllegalPK",	ch->pcdata->illegal_pk,	fread_number( fp ) );
            KEY( "Immune",	ch->immune,		fread_number( fp ) );
            KEY( "Interface",   ch->pcdata->interface,  fread_number( fp ) );
            KEY( "IntroDescr",	ch->intro_descr,	fread_string_nohash( fp ) );
#ifdef IMC
	    if ((fMatch = imc_loadchar(ch, fp, word)))
                break;
#endif
#ifdef I3
	    if ((fMatch = i3load_char(ch, fp, word)))
                break;
#endif
            break;

        case 'K':
            if ( !str_cmp( word, "Killed" ) )
            {
                fMatch = TRUE;
                if ( killcnt >= MAX_KILLTRACK )
                    bug( "fread_char: killcnt (%d) >= MAX_KILLTRACK", killcnt );
                else
                {
                    ch->pcdata->killed[killcnt].vnum    = fread_number( fp );
                    ch->pcdata->killed[killcnt++].count = fread_number( fp );
                }
            }
            break;

        case 'L':
            if ( !str_cmp( word, "Level" ) )
            {
                i = fread_number( fp );
                if (i < 0 || i >= MAX_CLASS)
                {
                    log_printf_plus(LOG_BUG, GetMaxLevel(ch), SEV_NOTICE,
                                    "fread_char: Unknown class %d", i );
                    break;
                }
                ch->levels[i]     = fread_number( fp );
                ch->classes[i]    = fread_number( fp );
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "LogSeverity" ) )
            {
                char *logname;

                x1 = fread_number(fp);
                logname = fread_string_nohash(fp);

                for (i = LOG_NORMAL; i < LOG_LAST; i++)
                    if (!str_cmp(logname, sysdata.logdefs[i].name))
                    {
                        ch->pcdata->log_severity[i] = x1;
                        break;
                    }

                DISPOSE(logname);

                fMatch = TRUE;
                break;
            }

            KEY( "LongDescr",	ch->long_descr,		fread_string( fp ) );

            if ( !str_cmp( word, "Languages" ) )
            {
                ch->speaks = fread_number( fp );
                ch->speaking = fread_number( fp );
                fMatch = TRUE;
            }

            if ( !str_cmp( word, "Lore" ) )
            {
                int sn;
                int value;

                if ( preload )
                    word = "End";
                else
                {
                    value = fread_number( fp );

                    sn = bsearch_skill_exact( fread_word( fp ), gsn_first_lore, GSN_LAST_LORE );
                    if ( sn < 0 )
                        bug( "Fread_char: unknown lore." );
                    else
                    {
                        if ( ShouldSaveSkill(ch, sn) )
                            ch->pcdata->learned[sn] = value;
                        else
                        {
                            ch->pcdata->learned[sn] = 0;
                            ch->practice++;
                        }
                    }
                    fMatch = TRUE;
                }
                break;
            }
            break;

        case 'M':
            KEY( "MDeaths",	ch->pcdata->mdeaths,	fread_number( fp ) );
            KEY( "Mentalstate", ch->mental_state,	fread_number( fp ) );
            KEY( "MGlory",      ch->pcdata->quest_accum,fread_number( fp ) );
            KEY( "Minsnoop",	ch->pcdata->min_snoop,	fread_number( fp ) );
            KEY( "MKills",	ch->pcdata->mkills,	fread_number( fp ) );
            KEY( "Mobinvis",	ch->mobinvis,		fread_number( fp ) );
            if ( !str_cmp( word, "MobRange" ) )
            {
                if (!ch->pcdata->m_range_lo)
                    ch->pcdata->m_range_lo = fread_number( fp );
                if (!ch->pcdata->m_range_hi)
                    ch->pcdata->m_range_hi = fread_number( fp );
                fMatch = TRUE;
            }
            if ( !str_cmp( word, "Money" ) )
            {
                x1=x2=0;
                while ((x1=fread_number(fp))>=0)
                    if (x2<MAX_CURR_TYPE)
                        GET_MONEY(ch,x2++) = x1;
                if (ch->gold2)
                {
                    GET_MONEY(ch,CURR_GOLD) = ch->gold2;
                    ch->gold2    = 0;
                }
                GET_MONEY(ch,CURR_NONE) = 0;
                fMatch = TRUE;
                break;
            }
            if ( !str_cmp( word, "Memorized" ) )
            {
                int sn;
                int value;

                if ( preload )
                    word = "End";
                else
                {
                    value = fread_number( fp );
                    word = fread_word( fp );
                    if ( file_ver < 3 )
                        sn = skill_lookup( word );
                    else
                        sn = bsearch_skill_exact( word, gsn_first_spell, GSN_LAST_SPELL );
                    if ( sn < 0 )
                    {
                        bug( "Fread_char: unknown spell (memorized): %s", word );
                    }
                    else
                        ch->pcdata->memorized[sn] = UMAX(0, value);
                    fMatch = TRUE;
                    break;
                }
            }
            break;

        case 'N':
            KEY("Numattacks", 	ch->numattacks,		fread_number( fp ) );
            if ( !str_cmp( word, "Name" ) )
            {
                /*
                 * Name already set externally.
                 */
                fread_to_eol( fp );
                fMatch = TRUE;
                break;
            }
            break;

        case 'O':
            /*
             KEY("OldClass", ch->class, fread_number(fp));
             KEY( "OldLevel",	ch->level,		fread_number( fp ) );
             */
            KEY( "Outcast_time", ch->pcdata->outcast_time, fread_number( fp ) );
            if ( !str_cmp( word, "ObjRange" ) )
            {
                if (!ch->pcdata->o_range_lo)
                    ch->pcdata->o_range_lo = fread_number( fp );
                if (!ch->pcdata->o_range_hi)
                    ch->pcdata->o_range_hi = fread_number( fp );
                fMatch = TRUE;
            }
            break;

        case 'P':
            KEY( "Pagerlen",	ch->pcdata->pagerlen,	fread_number( fp ) );
            KEY( "Password",	ch->pcdata->pwd,	fread_string_nohash( fp ) );
            KEY( "PDeaths",	ch->pcdata->pdeaths,	fread_number( fp ) );
            KEY( "PKills",	ch->pcdata->pkills,	fread_number( fp ) );
            KEY( "Played",	ch->played,		fread_number( fp ) );
            KEY( "Position",	ch->position,		fread_number( fp ) );
            KEY( "Practice",	ch->practice,		fread_number( fp ) );
            KEY( "Prompt",	ch->pcdata->prompt,	fread_string( fp ) );
            if (!str_cmp ( word, "Password2" ) )
            {
                /* plain text password */
                if ( ch->pcdata->pwd )
                    DISPOSE( ch->pcdata->pwd );
                ch->pcdata->pwd =
                    str_dup( crypt(fread_string(fp), ch->name) );
                fMatch = TRUE;
                break;
            }

            if (!str_cmp ( word, "PTimer" ) )
            {
                add_timer( ch , TIMER_PKILLED, fread_number(fp), NULL, 0 );
                fMatch = TRUE;
                break;
            }
            if ( !str_cmp( word, "PsiSpell" ) )
            {
                int sn;
                int value;

                if ( preload )
                    word = "End";
                else
                {
                    value = fread_number( fp );
                    word = fread_word( fp );

                    sn = bsearch_skill_exact( word, gsn_first_psispell, GSN_LAST_PSISPELL );
                    if ( sn < 0 )
                    {
                        bug( "Fread_char: unknown psispell: %s", word );
                    }
		    else
		    {
                        if ( ShouldSaveSkill(ch, sn) )
                            ch->pcdata->learned[sn] = UMAX(0, value);
                        else
                        {
                            bug( "Fread_char: can't use: %s", word );
                            ch->pcdata->learned[sn] = 0;
                            ch->practice++;
                        }
                    }
                    fMatch = TRUE;
                    break;
                }
            }
            if ( str_cmp( word, "End" ) )
                break;
            break;

        case 'R':
            KEY( "Race",        ch->race,		fread_number( fp ) );
            KEY( "Rank",        ch->pcdata->rank,	fread_string_nohash( fp ) );
            KEY( "Resistant",	ch->resistant,		fread_number( fp ) );
            KEY( "Restore_time",ch->pcdata->restore_time, fread_number( fp ) );

            if ( !str_cmp( word, "Regens" ) )
            {
                ch->hit_regen	= fread_number( fp );
                ch->mana_regen	= fread_number( fp );
                ch->move_regen	= fread_number( fp );
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "Room" ) )
	    {
		int rvnum = fread_number( fp );

                load_area_demand( rvnum );

                ch->in_room = get_room_index( rvnum );
                if ( !ch->in_room || ch->in_room->vnum == ROOM_VNUM_LIMBO )
                {
                    if ( IsBadSide(ch) )
		    {
                        load_area_demand( ROOM_START_EVIL );
                        ch->in_room = get_room_index( ROOM_START_EVIL );
                        ch->pcdata->home = ROOM_START_EVIL;
                    }
                    else
                    {
                        load_area_demand( ROOM_START_GOOD );
                        ch->in_room = get_room_index( ROOM_START_GOOD );
                        ch->pcdata->home = ROOM_START_GOOD;
                    }
                }
		if ( !ch->in_room )
		{
		    load_area_demand( ROOM_VNUM_LIMBO );
		    ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
		}
                ch->pcdata->home = ch->in_room->vnum;
                fMatch = TRUE;
                break;
            }
            if ( !str_cmp( word, "RoomRange" ) )
            {
                if (!ch->pcdata->r_range_lo)
                    ch->pcdata->r_range_lo = fread_number( fp );
                if (!ch->pcdata->r_range_hi)
                    ch->pcdata->r_range_hi = fread_number( fp );
                fMatch = TRUE;
            }
            break;

        case 'S':
            KEY( "Sex",		ch->sex,		fread_number( fp ) );
            KEY( "ShortDescr",	ch->short_descr,	fread_string( fp ) );
            KEY( "Spellfail",   ch->spellfail,          fread_number( fp ) );
            KEY( "Susceptible",	ch->susceptible,	fread_number( fp ) );
            if ( !str_cmp( word, "SavingThrow" ) )
            {
                ch->saving_wand 	= fread_number( fp );
                ch->saving_poison_death = ch->saving_wand;
                ch->saving_para_petri 	= ch->saving_wand;
                ch->saving_breath 	= ch->saving_wand;
                ch->saving_spell_staff 	= ch->saving_wand;
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "SavingThrows" ) )
            {
                ch->saving_poison_death = fread_number( fp );
                ch->saving_wand 	= fread_number( fp );
                ch->saving_para_petri 	= fread_number( fp );
                ch->saving_breath 	= fread_number( fp );
                ch->saving_spell_staff 	= fread_number( fp );
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "Site" ) )
            {
                if ( !preload )
                {
                    sprintf( buf, "Last connected from: %s", fread_word( fp ) );
                    log_string_plus( buf, LOG_COMM, LEVEL_GREATER, SEV_INFO+1 );
                    send_to_char( buf, ch );
                    send_to_char( "\n\r", ch );
                }
                else
                    fread_to_eol( fp );
                fMatch = TRUE;
                if ( preload )
                    word = "End";
                else
                    break;
            }

            if ( !str_cmp( word, "Skill" ) )
            {
                int sn;
                int value;

                if ( preload )
                    word = "End";
                else
                {
                    value = fread_number( fp );
                    word = fread_word( fp );
                    if ( file_ver < 3 )
                        sn = skill_lookup( word );
                    else
                        sn = bsearch_skill_exact( word, gsn_first_skill, GSN_LAST_SKILL );
                    if ( sn < 0 )
                    {
                        bug( "Fread_char: unknown skill: %s", word );
                    }
                    else
                    {
                        if ( ShouldSaveSkill(ch, sn) )
                            ch->pcdata->learned[sn] = UMAX(0, value);
                        else
                        {
                            ch->pcdata->learned[sn] = 0;
                            ch->practice++;
                        }
                    }
                    fMatch = TRUE;
                    break;
                }
            }

            if ( !str_cmp( word, "Spell" ) )
            {
                int sn;
                int value;

                if ( preload )
                    word = "End";
                else
                {
                    value = fread_number( fp );
                    word = fread_word( fp );

                    sn = bsearch_skill_exact( word, gsn_first_spell, GSN_LAST_SPELL );
                    if ( sn < 0 )
                    {
                        bug( "Fread_char: unknown spell: %s", word );
                        sn = bsearch_skill_exact( word, gsn_first_skill, GSN_LAST_SKILL );
                        if ( sn < 0 )
                        {
                            bug( "Fread_char: unknown skill/spell: %s", word );
                            fread_to_eol( fp );
                            fMatch = TRUE;
                            break;
                        }
                    }
                    /* else */
                    {
                        if ( ShouldSaveSkill(ch, sn) )
                            ch->pcdata->learned[sn] = UMAX(0, value);
                        else
                        {
                            bug( "Fread_char: can't use: %s", word );
                            ch->pcdata->learned[sn] = 0;
                            ch->practice++;
                        }
                    }
                    fMatch = TRUE;
                    break;
                }
            }
            if ( str_cmp( word, "End" ) )
                break;

        case 'E':
            if ( !str_cmp( word, "End" ) )
            {
               if (!IS_NPC(ch)) {
                if (!ch->short_descr)
                    ch->short_descr	= STRALLOC( "" );
                if (!ch->long_descr)
                    ch->long_descr	= STRALLOC( "" );
                if (!ch->description)
                    ch->description	= STRALLOC( "" );
                if (!ch->intro_descr)
                    ch->intro_descr	= str_dup( "" );
                if (!ch->pcdata->pwd)
                    ch->pcdata->pwd	= str_dup( "" );
                if (!ch->pcdata->bamfin)
                    ch->pcdata->bamfin	= STRALLOC( "" );
                if (!ch->pcdata->bamfout)
                    ch->pcdata->bamfout	= STRALLOC( "" );
                if (!ch->pcdata->bio)
                    ch->pcdata->bio	= STRALLOC( "" );
                if (!ch->pcdata->rank)
                    ch->pcdata->rank	= str_dup( "" );
                if (!ch->pcdata->bestowments)
                    ch->pcdata->bestowments = str_dup( "" );
                if (!ch->pcdata->title)
                    ch->pcdata->title	= STRALLOC( "" );
                if (!ch->pcdata->homepage)
                    ch->pcdata->homepage	= str_dup( "" );
                if (!ch->pcdata->authed_by)
                    ch->pcdata->authed_by = STRALLOC( "" );
                if (!ch->pcdata->prompt )
                    ch->pcdata->prompt	= STRALLOC( "" );
                ch->editor		= NULL;
                killcnt = URANGE( 2, ((GetMaxLevel(ch)+3) * MAX_KILLTRACK)/LEVEL_AVATAR, MAX_KILLTRACK );
                if ( killcnt < MAX_KILLTRACK )
                    ch->pcdata->killed[killcnt].vnum = 0;

                /* no good for newbies at all */
                if ( !IS_IMMORTAL( ch ) && !ch->speaking )
                    ch->speaking = LANG_COMMON;
                /*	ch->speaking = race_table[ch->race].language; */
                if ( IS_IMMORTAL( ch ) )
                {
                    ch->speaks = ~0;
                    if ( ch->speaking == 0 )
                        ch->speaking = ~0;
                }
                if ( !ch->pcdata->prompt )
                    ch->pcdata->prompt = STRALLOC("");
                }
                return ch;
            }
            KEY( "Exp",		ch->exp,		fread_number( fp ) );
            break;

        case 'T':
            KEY( "Time_Created", ch->pcdata->time_created,  fread_number( fp ) );
            KEY( "Time_To_Die",  ch->pcdata->time_to_die,   fread_number( fp ) );
            KEY( "Time_Imm",     ch->pcdata->time_immortal, fread_number( fp ) );
            KEY( "Times_played", ch->pcdata->times_played,  fread_number( fp ) );
            if ( !str_cmp( word, "Tongue" ) )
            {
                int sn;
                int value;

                if ( preload )
                    word = "End";
                else
                {
                    value = fread_number( fp );

                    sn = bsearch_skill_exact( fread_word( fp ), gsn_first_tongue, GSN_LAST_TONGUE );
                    if ( sn < 0 )
                        bug( "Fread_char: unknown tongue." );
                    else
                    {
                        if ( ShouldSaveSkill(ch, sn) )
                            ch->pcdata->learned[sn] = UMAX(0, value);
                        else
                        {
                            ch->pcdata->learned[sn] = 0;
                            ch->practice++;
                        }
                    }
                    fMatch = TRUE;
                }
                break;
            }
            KEY( "Trust", ch->trust, fread_number( fp ) );
            /* Let no character be trusted higher than one below maxlevel -- Narn */
/*            ch->trust = UMIN( ch->trust, MAX_LEVEL ); */

            if ( !str_cmp( word, "Title" ) )
            {
                ch->pcdata->title = fread_string( fp );
                if ( isalpha(ch->pcdata->title[0])
                     ||   isdigit(ch->pcdata->title[0]) )
                {
                    sprintf( buf, " %s", ch->pcdata->title );
                    if ( ch->pcdata->title )
                        STRFREE( ch->pcdata->title );
                    ch->pcdata->title = STRALLOC( buf );
                }
                fMatch = TRUE;
                break;
            }

            break;

        case 'U':
            if ( !str_cmp( word, "User" ) )
            {
                if ( !preload )
                {
                    sprintf( buf, "Last connected as: %s", fread_word( fp ) );
                    log_string_plus( buf, LOG_COMM, LEVEL_GREATER, SEV_INFO+1 );
                }
                else
                    fread_to_eol( fp );
                fMatch = TRUE;
                if ( preload )
                    word = "End";
                else
                    break;
            }
            break;

        case 'V':
            if ( !str_cmp( word, "Vnum" ) )
            {
                if (ch) {
                    ch->pIndexData = get_mob_index( fread_number( fp ) );
                } else {
                    if ((ch = create_mobile( fread_number( fp ) )) == NULL)
                    {
                        bug( "Fread_char: bad vnum or error creating pet");
                        break;
                    }
                }
                fMatch = TRUE;
                break;
            }
            KEY( "Version",	file_ver,		fread_number( fp ) );
            break;

        case 'W':
            if ( !str_cmp( word, "Weapon" ) )
            {
                int sn;
                int value;

                if ( preload )
                    word = "End";
                else
                {
                    value = fread_number( fp );

                    sn = bsearch_skill_exact( fread_word( fp ), gsn_first_weapon, GSN_LAST_WEAPON );
                    if ( sn < 0 )
                        bug( "Fread_char: unknown weapon." );
                    else
                    {
                        if ( ShouldSaveSkill(ch, sn) )
                            ch->pcdata->learned[sn] = UMAX(0, value);
                        else
                        {
                            ch->pcdata->learned[sn] = 0;
                            ch->practice++;
                        }
                    }
                    fMatch = TRUE;
                }
                break;
            }
            KEY( "Wimpy",	ch->wimpy,		fread_number( fp ) );
            KEY( "WizInvis",	ch->pcdata->wizinvis,	fread_number( fp ) );
            break;
        }

        if ( !fMatch )
        {
            bug( "Fread_char: no match: %s", word );
            fread_to_eol( fp );
        }
    }
}


void fread_obj( CHAR_DATA *ch, FILE *fp, sh_int os_type )
{
    OBJ_DATA *obj;
    const char *word;
    int iNest;
    bool fMatch;
    bool fNest;
    bool fVnum;
    ROOM_INDEX_DATA *room = NULL;

    obj = new_obj();

    obj->count		= 1;
    obj->wear_loc	= -1;
    obj->weight		= 1;
    obj->timer          = 0;

    fNest		= TRUE;		/* Requiring a Nest 0 is a waste */
    fVnum		= TRUE;
    iNest		= 0;

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

        case '#':
            if ( !str_cmp( word, "#VARIABLE" ) )
            {
                fread_variable(&obj->vars, fp);
                fMatch = TRUE;
                break;
            }
            break;

        case 'A':
            KEY( "ActionDesc",	obj->action_desc,	fread_string( fp ) );
            if ( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
            {
                AFFECT_DATA *paf;
                int pafmod;

                CREATE( paf, AFFECT_DATA, 1 );
                if ( !str_cmp( word, "Affect" ) )
                {
                    paf->type	= fread_number( fp );
                }
                else
                {
                    int sn;

                    sn = skill_lookup( fread_word( fp ) );
                    if ( sn < 0 )
                        bug( "Fread_obj: unknown skill." );
                    else
                        paf->type = sn;
                }
                paf->duration	= fread_number( fp );
                pafmod		= fread_number( fp );
                paf->location	= fread_number( fp );
                paf->bitvector	= fread_number( fp );
                if ( paf->location == APPLY_WEAPONSPELL
                     ||   paf->location == APPLY_WEARSPELL
                     ||   paf->location == APPLY_REMOVESPELL
                     ||   paf->location == APPLY_EAT_SPELL
                     ||   paf->location == APPLY_IMMUNESPELL
                     ||   paf->location == APPLY_STRIPSN )
                     paf->modifier		= slot_lookup( pafmod );
                else
                    paf->modifier		= pafmod;
                LINK(paf, obj->first_affect, obj->last_affect, next, prev );
                fMatch				= TRUE;
                break;
            }
            break;

        case 'C':
            KEY( "Cost",	obj->cost,		fread_number( fp ) );
            KEY( "Count",	obj->count,		fread_number( fp ) );
            KEY( "CurrType",    obj->currtype,		fread_number( fp ) );
            if ( !str_cmp( word, "CVnum" ) )
            {
                int vnum;

                vnum = fread_number( fp );

                obj->christened = get_christen(vnum);

                fMatch = TRUE;
                break;
            }
            break;

        case 'D':
            KEY( "Description",	obj->description,	fread_string( fp ) );
            break;

        case 'E':
            KEY( "ExtraFlags",	obj->extra_flags,	fread_number( fp ) );
            KEY( "ExtraFlags2",	obj->extra_flags2,	fread_number( fp ) );

            if ( !str_cmp( word, "ExtraDescr" ) )
            {
                EXTRA_DESCR_DATA *ed;

                CREATE( ed, EXTRA_DESCR_DATA, 1 );
                ed->keyword		= fread_string( fp );
                ed->description		= fread_string( fp );
                LINK(ed, obj->first_extradesc, obj->last_extradesc, next, prev );
                fMatch 				= TRUE;
            }

            if ( !str_cmp( word, "End" ) )
            {
                if ( !fNest || !fVnum )
                {
                    bug( "Fread_obj: incomplete object: %s", obj->name?obj->name:"(null)" );
                    if ( obj->name )
                        STRFREE( obj->name        );
                    if ( obj->description )
                        STRFREE( obj->description );
                    if ( obj->short_descr )
                        STRFREE( obj->short_descr );
                    if ( obj->action_desc )
                        STRFREE( obj->action_desc );
                    DISPOSE( obj );
                    return;
                }
                else
                {
                    sh_int wear_loc = obj->wear_loc;

                    if ( !obj->name )
                        obj->name = QUICKLINK( obj->pIndexData->name );
                    if ( !obj->description )
                        obj->description = QUICKLINK( obj->pIndexData->description );
                    if ( !obj->short_descr )
                        obj->short_descr = QUICKLINK( obj->pIndexData->short_descr );
                    if ( !obj->action_desc )
                        obj->action_desc = QUICKLINK( obj->pIndexData->action_desc );
                    LINK(obj, first_object, last_object, next, prev );
                    obj->pIndexData->count += obj->count;
                    if ( fNest )
                        rgObjNest[iNest] = obj;
                    numobjsloaded += obj->count;
                    ++physicalobjects;
                    if ( file_ver > 1 || obj->wear_loc < -1
                         ||   obj->wear_loc >= MAX_WEAR )
                        obj->wear_loc = -1;
                    /* Corpse saving. -- Altrag */
                    if ( os_type == OS_CORPSE )
                    {
                        if ( !room )
                        {
                            bug( "Fread_obj: Corpse without room");
                            room = get_room_index(ROOM_VNUM_LIMBO);
                        }
                        obj = obj_to_room( obj, room );
                    }
                    else if ( iNest == 0 || rgObjNest[iNest] == NULL )
                    {
                        int slot = 0;
                        bool reslot = FALSE;

                        if ( file_ver > 1
                             &&   wear_loc > -1
                             &&   wear_loc < MAX_WEAR )
                        {
                            int x;

                            for ( x = 0; x < MAX_LAYERS; x++ )
                                if ( !save_equipment[wear_loc][x] )
                                {
                                    save_equipment[wear_loc][x] = obj;
                                    slot = x;
                                    reslot = TRUE;
                                    break;
                                }
                            if ( x == MAX_LAYERS )
                                bug( "Fread_obj: too many layers %d", wear_loc );
                        }
                        obj = obj_to_char( obj, ch );
                        if ( reslot )
                            save_equipment[wear_loc][slot] = obj;
                    }
                    else
                    {
                        if ( rgObjNest[iNest-1] )
                        {
                            separate_obj( rgObjNest[iNest-1] );
                            obj = obj_to_obj( obj, rgObjNest[iNest-1] );
                        }
                        else
                            bug( "Fread_obj: nest layer missing %d", iNest-1 );
                    }
                    if ( fNest )
                        rgObjNest[iNest] = obj;
                    return;
                }
            }
            break;

        case 'I':
            KEY( "ItemType",	obj->item_type,		fread_number( fp ) );
            break;

        case 'M':
            KEY( "MagicFlags",	obj->magic_flags,	fread_number( fp ) );
            break;

        case 'N':
            KEY( "Name",	obj->name,		fread_string( fp ) );
            KEY( "Nickname",	obj->nickname,		fread_string( fp ) );

            if ( !str_cmp( word, "Nest" ) )
            {
                iNest = fread_number( fp );
                if ( iNest < 0 || iNest >= MAX_NEST )
                {
                    bug( "Fread_obj: bad nest %d.", iNest );
                    iNest = 0;
                    fNest = FALSE;
                }
                fMatch = TRUE;
            }
            break;

        case 'R':
            KEY( "Rent",	obj->rent,		fread_number( fp ) );
            KEY( "Room", room, get_room_index(fread_number(fp)) );

        case 'S':
            KEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );

            if ( !str_cmp( word, "Spell" ) )
            {
                int iValue;
                int sn;

                iValue = fread_number( fp );
                sn     = skill_lookup( fread_word( fp ) );
                if ( iValue < 0 || iValue > 5 )
                    bug( "Fread_obj: bad iValue %d.", iValue );
                else if ( sn < 0 )
                    bug( "Fread_obj: unknown skill." );
                else
                    obj->value[iValue] = sn;
                fMatch = TRUE;
                break;
            }

            break;

        case 'T':
            KEY( "Timer",	obj->timer,		fread_number( fp ) );
            break;

        case 'V':
            if ( !str_cmp( word, "Values" ) )
            {
                int x1,x2,x3,x4,x5,x6;
                char *ln = fread_line( fp );

                x1=x2=x3=x4=x5=x6=0;
                sscanf( ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6 );
                /* clean up some garbage */
                if ( file_ver < 3 )
                    x5=x6=0;

                obj->value[0]	= x1;
                obj->value[1]	= x2;
                obj->value[2]	= x3;
                obj->value[3]	= x4;
                obj->value[4]	= x5;
                obj->value[5]	= x6;
                fMatch		= TRUE;
                break;
            }

            if ( !str_cmp( word, "Vnum" ) )
            {
                obj->vnum = fread_number( fp );
                /*  bug( "Fread_obj: bad vnum %d.", vnum );  */
                if ( ( obj->pIndexData = get_obj_index( obj->vnum ) ) == NULL )
                    fVnum = FALSE;
                else
                {
                    fVnum = TRUE;
                    obj->cost = obj->pIndexData->cost;
                    obj->currtype = obj->pIndexData->currtype;
                    obj->rent = obj->pIndexData->rent;
                    obj->weight = obj->pIndexData->weight;
                    obj->item_type = obj->pIndexData->item_type;
                    obj->wear_flags = obj->pIndexData->wear_flags;
                    obj->magic_flags = obj->pIndexData->magic_flags;
                    obj->extra_flags = obj->pIndexData->extra_flags;
                    obj->extra_flags2 = obj->pIndexData->extra_flags2;
                }
                fMatch = TRUE;
                break;
            }
            break;

        case 'W':
            KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
            KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
            KEY( "Weight",	obj->weight,		fread_number( fp ) );
            break;

        }

        if ( !fMatch )
        {
            EXTRA_DESCR_DATA *ed;
            AFFECT_DATA *paf;

            bug( "Fread_obj: no match %s", word?word:"(null)" );
            fread_to_eol( fp );
            if ( obj->name )
                STRFREE( obj->name        );
            if ( obj->description )
                STRFREE( obj->description );
            if ( obj->short_descr )
                STRFREE( obj->short_descr );
            if ( obj->action_desc )
                STRFREE( obj->action_desc );
            while ( (ed=obj->first_extradesc) != NULL )
            {
                STRFREE( ed->keyword );
                STRFREE( ed->description );
                UNLINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
                DISPOSE( ed );
            }
            while ( (paf=obj->first_affect) != NULL )
            {
                UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
                DISPOSE( paf );
            }
            DISPOSE( obj );
            return;
        }
    }
}

void set_alarm( long seconds )
{
    alarm( seconds );
}

/*
 * Based on last time modified, show when a player was last on	-Thoric
 */
void do_last( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_STRING_LENGTH];
    char arg [MAX_INPUT_LENGTH];
    char name[MAX_INPUT_LENGTH];
    struct stat fst;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        if (IS_IMMORTAL(ch))
        {
            show_file(ch, LASTLOG_FILE ".50");
            return;
        }
        send_to_char( "Usage: last <playername>\n\r", ch );
        return;
    }

    if (!check_parse_name(arg))
    {
        send_to_char("That is not a valid player name.\n\r", ch);
        return;
    }

    strcpy( name, capitalize(arg) );
    sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower(arg[0]), name );
    if ( stat( buf, &fst ) != -1 )
        sprintf( buf, "%s was last on: %s\r", name, ctime( &fst.st_mtime ) );
    else
        sprintf( buf, "%s was not found.\n\r", name );
    send_to_char( buf, ch );
}

void write_corpses( CHAR_DATA *ch, char *name )
{
    OBJ_DATA *corpse;
    FILE *fp = NULL;

    /* Name and ch support so that we dont have to have a char to save their
     corpses.. (ie: decayed corpses while offline) */
    if ( ch && IS_NPC(ch) )
    {
	bug( "Write_corpses: writing NPC corpse." );
        return;
    }
    if ( ch )
        name = ch->name;
    /* Go by vnum, less chance of screwups. -- Altrag */
    for ( corpse = first_object; corpse; corpse = corpse->next )
        if ( !corpse || !corpse->pIndexData )
            fprintf(stderr, "!corpse || !corpse->pIndexData in save.c\n");
        else if ( corpse->vnum == OBJ_VNUM_CORPSE_PC &&
                  corpse->in_room != NULL &&
                  !str_cmp(corpse->short_descr+14, name) )
        {
            if ( !fp )
            {
                char buf[127];

                sprintf(buf, "%s%s", CORPSE_DIR, capitalize(name));
                if ( !(fp = fopen(buf, "w")) )
                {
                    bug( "Write_corpses: Cannot open file." );
                    perror(buf);
                    return;
                }
            }
            fwrite_obj(ch, corpse, fp, 0, OS_CORPSE);
        }
    if ( fp )
    {
        fprintf(fp, "#END\n\n");
        FCLOSE(fp);
    }
    else
    {
        char buf[127];

        sprintf(buf, "%s%s", CORPSE_DIR, capitalize(name));
        remove(buf);
    }
    return;
}

void load_corpses( void )
{
    DIR *dp;
    struct dirent *de;
    extern FILE *fpArea;
    extern char strArea[MAX_INPUT_LENGTH];
    extern int falling;

    if ( !(dp = opendir(CORPSE_DIR)) )
    {
        bug( "Load_corpses: can't open CORPSE_DIR");
        perror(CORPSE_DIR);
        return;
    }

    falling = 1; /* Arbitrary, must be >0 though. */
    while ( (de = readdir(dp)) != NULL )
    {
        if ( de->d_name[0] != '.' )
        {
            sprintf(strArea, "%s%s", CORPSE_DIR, de->d_name );
            fprintf(stderr, "Corpse -> %s\n", strArea);
            if ( !(fpArea = fopen(strArea, "r")) )
            {
                perror(strArea);
                continue;
            }
            for ( ; ; )
            {
                char letter;
                char *word;

                letter = fread_letter( fpArea );
                if ( letter == '*' )
                {
                    fread_to_eol(fpArea);
                    continue;
                }
                if ( letter != '#' )
                {
                    bug( "Load_corpses: # not found, got %c instead", letter );
                    break;
                }
                word = fread_word( fpArea );
                if ( !str_cmp(word, "CORPSE" ) )
                    fread_obj( NULL, fpArea, OS_CORPSE );
                else if ( !str_cmp(word, "OBJECT" ) )
                    fread_obj( NULL, fpArea, OS_CARRY );
                else if ( !str_cmp( word, "END" ) )
                    break;
                else
                {
                    bug( "Load_corpses: bad section %s", word?word:"(null)" );
                    break;
                }
            }
            FCLOSE(fpArea);
        }
    }
    fpArea = NULL;
    strcpy(strArea, "$");
    closedir(dp);
    falling = 0;
    return;
}

void rent_recurse(CHAR_DATA *ch, OBJ_DATA *obj, int *rent, int type)
{
    OBJ_DATA *tobj;
    if (is_rare_obj(obj) || IS_OBJ_STAT2(obj, ITEM2_RENT))
    {
        int orent = obj->rent;
        orent = UMAX(1, obj->rent-10000);
        if (type==0)
            ch_printf(ch, "%-40.40s %6d coins/day (x%d)\n\r",
                      obj->short_descr, orent, obj->count);
        else
            *rent += orent*obj->count;
    }
    if (!obj->first_content)
        return;
    for (tobj=obj->first_content;tobj;tobj=tobj->next_content)
        rent_recurse(ch,tobj,rent,type);
}

void show_rent_list(CHAR_DATA *ch)
{
    OBJ_DATA *obj;

    for (obj=ch->first_carrying;obj;obj=obj->next_content)
        rent_recurse(ch,obj,NULL,0);
}

int calc_rent(CHAR_DATA *ch)
{
    OBJ_DATA *obj;
    int rent=0;

    if (GetMaxLevel(ch)<4)
        return 0;

    for (obj=ch->first_carrying;obj;obj=obj->next_content)
        rent_recurse(ch,obj,&rent,1);

    return(rent);
}


void process_rent(CHAR_DATA *ch)
{
    OBJ_DATA *obj, *prev_obj;
    char strsave[MAX_INPUT_LENGTH];
    struct stat fst;
    double days=0.0;
    int cost, rent=0, type;
    unsigned int x;
    time_t diff=0;

    if (!ch || !ch->pcdata)
        return;

    type = get_primary_curr(ch->in_room);

    rent=calc_rent(ch);
    ch_printf(ch,"\n\rYour rent is %d %s coins per day.\n\r",rent,curr_types[type]);

    strcpy(strsave,ch->in_room->name);
    for (x=0;x<strlen(strsave);x++)
        if (strsave[x]==' ' || strsave[x]=='\'')
            break;
    strncpy(strsave,ch->in_room->name,x);
    if (rent>0 && !strncmp(strsave,GET_NAME(ch),strlen(GET_NAME(ch))))
    {
        rent/=2;
        ch_printf(ch,"Since you own a house, your rent is halved.\n\r");
    }

    sprintf( strsave, "%s%c/%s", PLAYER_DIR, tolower(ch->name[0]),
             capitalize( ch->name ) );

    if ( stat( strsave, &fst ) != -1 )
    {
        diff=(time(0))-(fst.st_mtime);
        days=(double)diff/(double)(60*60*24);
        cost=(int)((double)rent*(double)days);
        ch_printf(ch,"You were gone for %s.\n\rYou owe %d %s coins in rent.\n\r",
                  sec_to_hms(diff), cost, curr_types[type]);

        if (cost >= GET_MONEY(ch,type))
        {
            cost -= ch->money[type];
            ch->money[type] = 0;
        }
        else
        {
            ch->money[type] -= cost;
            cost = 0;
        }

        if (cost > 0 && cost >= GET_BALANCE(ch,type))
        {
            cost -= ch->balance[type];
            ch->balance[type] = 0;
        }
        else
        {
            ch->balance[type] -= cost;
            cost = 0;
        }

        if (cost > 0)
        {
            set_char_color(AT_RED, ch);
            ch_printf(ch, "You can't pay your rent, your rare equipment has been confiscated.\n\r");
            log_printf_plus(LOG_MONITOR, LEVEL_LOG_CSET, SEV_NOTICE, "%s lost equipment due to rent", GET_NAME(ch));

            prev_obj = ch->last_carrying;

            while ( (obj = prev_obj) )
            {
                prev_obj = obj->prev_content;
                if (is_rare_obj(obj))
                    extract_obj( obj );
            }
            return;
        }

    }
}
