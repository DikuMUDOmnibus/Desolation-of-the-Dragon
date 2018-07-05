/***************************************************************************
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
 *		       Online Building and Editing Module		    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: build.c,v 1.109 2004/04/06 22:00:09 dotd Exp $";*/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "mud.h"
#include "currency.h"
#include "justify.h"

DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_asave);
DECLARE_DO_FUN(do_mstat);
DECLARE_DO_FUN(do_ostat);
DECLARE_DO_FUN(do_rstat);
DECLARE_DO_FUN(do_at);

#ifdef USE_ASPELL
void aspell_string(CHAR_DATA *ch, char *str);
#endif

extern int	top_affect;
extern int	top_reset;
extern int	top_ed;
extern int      top_room_vnum;
extern bool	fBootDb;


char *  const   ex_flags [] =
{
    "isdoor", "closed", "locked", "secret", "swim", "pickproof", "fly", "climb",
    "dig", "r1", "nopassdoor", "hidden", "passage", "portal", "r2", "r3",
    "can_climb", "can_enter", "can_leave", "auto", "r4", "searchable",
    "bashed", "bashproof", "nomob", "window", "can_look", "nofly", "r5", "r6"
};

char *	const	r_flags	[] =
{
    "dark", "death", "nomob", "indoors", "lawful", "neutral", "chaotic",
    "nomagic", "tunnel", "private", "safe", "solitary", "petshop", "norecall",
    "donation", "nodropall", "silence", "logspeech", "nodrop", "clanstoreroom",
    "nosummon", "noastral", "teleport", "teleshowdesc", "nofloor",
    "reception", "bank", "riversource", "arena", "nomissile", "prototype", "orphaned"
};

char *	const	o_flags	[] =
{
    "glow", "hum", "dark", "loyal", "evil", "invis", "magic", "nodrop", "bless",
    "antigood", "antievil", "antineutral", "noremove", "inventory",
    "antimage", "antithief", "antiwarrior", "anticleric", "organic", "metal",
    "donation", "clanobject", "clancorpse", "antivampire", "antidruid",
    "hidden", "poisoned", "covering", "deathrot", "burried", "prototype", "r5"
};

char *	const	o2_flags	[] =
{
    "mineral", "brittle", "resistant", "immune", "antimen", "antiwomen",
    "antineuter", "antisun", "antibarbarian", "antiranger", "antipaladin",
    "antipsi", "antimonk", "antiartificer", "antiamazon", "antinecromancer",
    "antiantipal", "r17", "r18", "r19",
    "only_class", "noclone", "rent", "audio",
    "(stored item: internal use only)",
    "r25", "r26", "r27", "r28", "r29", "r30", "r31"
};

char *	const	mag_flags	[] =
{
    "returning", "backstabber", "bane", "loyal", "haste", "drain",
    "lightningblade"
};

char *	const	w_flags	[] =
{
    "take", "finger", "neck", "body", "head", "legs", "feet", "hands", "arms",
    "shield", "about", "waist", "wrist", "wield", "hold", "_dual_", "ears", "eyes",
    "_missile_", "back","nose","ankle","r4","r5","r6",
    "r7","r8","r9","r10","r11","r12","r13"
};

char *	const	area_flags	[] =
{
    "nopkill", "arena", "dark", "r3", "r4", "r5", "r6", "r7", "r8",
    "r9", "r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17",
    "r18", "r19","r20","r21","r22","r23","r24",
    "r25","r26","r27","loaded","reset-boot","initialized","modified"
};

char *	const	o_types	[] =
{
    "none", "light", "scroll", "wand", "staff", "weapon", "_oldweapon_", "_oldmissile_",
    "treasure", "armor", "potion", "_worn", "furniture", "trash", "_oldtrap_",
    "container", "_note", "drinkcon", "key", "food", "money", "pen", "boat",
    "corpse", "corpse_pc", "fountain", "pill", "blood", "bloodstain",
    "scraps", "pipe", "herbcon", "herb", "incense", "fire", "book", "switch",
    "lever", "pullchain", "button", "dial", "rune", "runepouch", "match", "trap",
    "map", "portal", "paper", "tinder", "lockpick", "spike", "disease", "oil",
    "fuel", "blah1", "blah2", "missileweapon", "projectile", "quiver",
    "shovel", "salve", "board", "cook", "keyring", "odor", "pokeball",
    "rock", "stone", "forge", "material", "\n"
};

char *	const	pipe_flags [] =
{
    "r0", "tamped", "lit", "hot", "dirty", "filthy", "goingout", "burnt",
    "fullofash", "singleuse", "bong", "r11", "r12", "r13", "r14", "r15",
    "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25",
    "r26", "r27", "r28", "r29", "r30", "r31"
};

char *	const	a_types	[MAX_APPLY_TYPE+1] =
{
    "none", "strength", "dexterity", "intelligence", "wisdom", "constitution",
    "sex", "class", "level", "age", "height", "weight", "mana", "hit", "move",
    "gold", "experience", "armor", "hitroll", "damroll", "save_poison", "save_rod",
    "save_para", "save_breath", "save_spell", "charisma", "affected", "resistant",
    "immune", "susceptible", "weaponspell", "luck", "backstab", "pick", "track",
    "steal", "sneak", "hide", "palm", "detrap", "dodge", "sf", "scan", "gouge",
    "search", "mount", "disarm", "kick", "parry", "bash", "stun", "punch", "climb",
    "grip", "scribe", "brew", "wearspell", "removespell", "emotion", "mentalstate",
    "stripsn", "remove", "dig", "full", "thirst", "drunk", "blood", "hitregen",
    "manaregen", "moveregen", "antimagic", "affected2", "roomflag", "sectortype",
    "roomlight", "televnum", "teledelay", "cook", "blah1", "race", "hit-n-dam",
    "save_all", "eat_spell", "race_slayer", "align_slayer", "findtrap",
    "numattacks", "barenumdie", "baresizedie", "nummem", "immunespell",
    "absorb", "max"
};

char *	const	a_flags [] =
{
    "blind", "invisible", "detect_evil", "detect_invis", "detect_magic",
    "detect_hidden", "hold", "sanctuary", "faerie_fire", "infrared", "curse",
    "_flaming", "poison", "protect", "_paralysis", "sneak", "hide", "sleep",
    "charm", "flying", "pass_door", "floating", "truesight", "detect_traps",
    "scrying", "fireshield", "shockshield", "haus1", "iceshield", "possess",
    "berserk", "aqua_breath"
};

char *	const	a2_flags [] =
{
    "animal_invis", "heat_stuff", "life_prot", "dragon_ride", "growth",
    "tree_travel", "travelling", "silence", "telepathy", "ethereal",
    "beacon", "r12", "r13", "r14", "r15", "r16", "r17", "r18", "r19", "r20",
    "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30",
    "r31", "r32"
};

char *	const	act_flags [] =
{
    "npc", "sentinel", "scavenger", "nice_thief", "annoying", "aggressive", "stayarea",
    "wimpy", "pet", "train", "practice", "immortal", "deadly", "customsaves",
    "meta_aggr", "guardian", "running", "nowander", "mountable", "mounted", "scholar",
    "secretive", "polymorphed", "mobinvis", "noassist", "illusion", "huge",
    "greet", "teacher", "r13", "prototype", "r14"
};

char *	const	act2_flags [] =
{
    "mage", "warrior", "cleric", "thief", "druid", "monk", "barbarian",
    "paladin", "ranger", "psionist", "artificer", "vampire", "amazon",
    "necromancer", "anti-paladin", "master_vampire", "planar", "r18", "r19", "r20",
    "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30",
    "r31", "r32"
};

char *	const	pc_flags [] =
{
    "r1", "deadly", "unauthed", "norecall", "nointro", "gag", "retired", "guest",
    "nosummon", "pager", "notitled", "web", "r6", "r7",
    "r8", "r9", "r10", "r11", "r12", "r13",
    "r14", "r15", "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24",
    "r25"
};

char *	const	plr_flags [] =
{
    "npc", "boughtpet", "shovedrag", "autoexits", "autoloot", "autosac", "blank",
    "outcast", "brief", "combine", "prompt", "telnet_ga", "holylight",
    "noooc", "roomvnum","silence", "noemote", "attacker", "notell", "log",
    "deny", "freeze", "thief","killer", "litterbug", "ansi", "rip", "nice",
    "flee" ,"autogold", "automap", "afk"
};

char *	const	plr2_flags [] =
{
    "autoassist", "busy", "afkbuffer", "moniafk", "died", "autogain", "mxp",
    "msp", "r09", "r10", "r11", "r12", "r13",
    "r14", "r15", "r16", "planar", "r18", "r19", "r20",
    "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30",
    "r31", "r32"
};

char *	const	trap_flags [] =
{
    "room", "obj", "enter", "leave", "open", "close", "get", "put", "pick",
    "unlock", "north", "south", "east", "r1", "west", "up", "down", "examine",
    "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13",
    "r14", "r15"
};

char *	const	wear_locs [] =
{
    "light", "finger1", "finger2", "neck1", "neck2", "body", "head", "legs",
    "feet", "hands", "arms", "shield", "about", "waist", "wrist1", "wrist2",
    "wield", "hold", "dual_wield", "ear1", "ear2", "eyes", "missile_wield",
    "back", "nose", "ankle1", "ankle2"
};

char *	const	ris_flags [] =
{
    "fire", "cold", "electricity", "energy", "blunt", "pierce", "slash",
    "acid", "poison", "drain", "sleep", "charm", "hold", "nonmagic",
    "plus1", "plus2", "plus3", "plus4", "plus5", "plus6", "magic",
    "paralysis", "good", "evil", "holy", "unholy", "psychic",
    "r6", "r7", "r8", "r9", "r10"
};

char *	const	trig_flags [] =
{
    "up", "unlock", "lock", "d_north", "d_south", "d_east", "d_west", "d_up",
    "d_down", "door", "container", "open", "close", "passage", "oload", "mload",
    "teleport", "teleportall", "teleportplus", "death", "cast", "fakeblade",
    "rand4", "rand6", "trapdoor", "anotherroom", "usedial", "absolutevnum",
    "showroomdesc", "autoreturn", "r2", "r3"
};

char *	const	part_flags [] =
{
    "head", "arms", "legs", "heart", "brains", "guts", "hands", "feet", "fingers",
    "ear", "eye", "long_tongue", "eyestalks", "tentacles", "fins", "wings",
    "tail", "scales", "claws", "fangs", "horns", "tusks", "tailattack",
    "sharpscales", "beak", "haunches", "hooves", "paws", "forelegs", "feathers",
    "chest", "stomach"
};

char *	const	attack_flags [] =
{
    "bite", "claws", "tail", "sting", "punch", "kick", "trip", "bash", "stun",
    "gouge", "backstab", "feed", "drain", "firebreath", "frostbreath",
    "acidbreath", "lightnbreath", "gasbreath", "poison", "nastypoison", "gaze",
    "blindness", "causeserious", "earthquake", "causecritical", "curse",
    "flamestrike", "harm", "fireball", "colorspray", "weaken", "r1"
};

char *	const	defense_flags [] =
{
    "parry", "dodge", "heal", "curelight", "cureserious", "curecritical",
    "dispelmagic", "dispelevil", "sanctuary", "fireshield", "shockshield",
    "shield", "bless", "stoneskin", "teleport", "monsum1", "monsum2", "monsum3",
    "monsum4", "disarm", "iceshield", "grip", "r3", "r4", "r5", "r6", "r7",
    "r8", "r9", "r10", "r11", "r12"
};

char *	const	sect_types [] =
{
    "inside", "city", "field", "forest", "hills", "mountain", "swim",
    "water", "underwater", "air", "desert", "dunno", "oceanfloor",
    "underground", "tree", "fire", "quicksand", "ether", "glacier",
    "earth", "\n"
};

/*
 * Note: I put them all in one big set of flags since almost all of these
 * can be shared between mobs, objs and rooms for the exception of
 * bribe and hitprcnt, which will probably only be used on mobs.
 * ie: drop -- for an object, it would be triggered when that object is
 * dropped; -- for a room, it would be triggered when anything is dropped
 *          -- for a mob, it would be triggered when anything is dropped
 *
 * Something to consider: some of these triggers can be grouped together,
 * and differentiated by different arguments... for example:
 *  hour and time, rand and randiw, speech and speechiw
 *
 */
char *	const	mprog_flags [NUM_PROG_TYPES] =
{
    "act", "speech", "rand", "fight", "death", "hitprcnt", "entry", "greet",
    "allgreet", "give", "bribe", "hour", "time", "wear", "remove", "sac",
    "look", "exa", "zap", "get", "drop", "damage", "repair", "birth",
    "speechiw", "pull", "push", "sleep", "rest", "leave", "script", "use",
    "quest", "command", "areareset", "areainit"
};


char *flag_string( int bitvector, char * const flagarray[] )
{
    static char buf[MAX_STRING_LENGTH];
    int x;

    buf[0] = '\0';
    for ( x = 0; x < 32 ; x++ )
        if ( IS_SET( bitvector, 1 << x ) )
        {
            strcat( buf, flagarray[x] );
            strcat( buf, " " );
        }
    if ( (x=strlen( buf )) > 0 )
        buf[--x] = '\0';

    return buf;
}


bool can_rmodify( CHAR_DATA *ch, ROOM_INDEX_DATA *room )
{
    char bes[16];
    int vnum = room->vnum;
    AREA_DATA *pArea;

    if ( IS_NPC( ch ) )
        return FALSE;

    if (room->area && !IS_AREA_STATUS(room->area, AREA_LOADED))
    {
        send_to_char( "Load the area first.\n\r", ch);
        return FALSE;
    }

    /* let anybody edit so they can practice, etc */
    if (vnum>=VNUM_START_SCRATCH && vnum<=VNUM_END_SCRATCH)
        return TRUE;

    if (is_name(room->area->filename, ch->pcdata->bestowments))
        return TRUE;

    sprintf(bes, "room:%d", vnum);
    if (is_name(bes, ch->pcdata->bestowments))
        return TRUE;

    if ( get_trust( ch ) >= sysdata.level_modify_proto )
    {
        if (room->area && !IS_AREA_FLAG(room->area,AFLAG_MODIFIED))
        {
            SET_AREA_FLAG(room->area,AFLAG_MODIFIED);
            log_printf_plus( LOG_BUILD, GetMaxLevel(ch), SEV_SPAM+9,
                             "%s modified %s", GET_NAME(ch), room->area->name );
        }
        return TRUE;
    }
    if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
    {
        send_to_char( "You must have an assigned area to modify this room.\n\r", ch );
        return FALSE;
    }
    if ( vnum >= pArea->low_r_vnum
         &&   vnum <= pArea->hi_r_vnum )
    {
        if (!IS_AREA_FLAG(pArea,AFLAG_MODIFIED))
        {
            SET_AREA_FLAG(pArea,AFLAG_MODIFIED);
            log_printf_plus( LOG_BUILD, GetMaxLevel(ch), SEV_SPAM+9,
                             "%s modified %s", GET_NAME(ch), pArea->name );
        }
        return TRUE;
    }
    if ( !IS_ROOM_FLAG( room, ROOM_PROTOTYPE) )
    {
        send_to_char( "You cannot modify this room.\n\r", ch );
        return FALSE;
    }

    send_to_char( "That room is not in your allocated range.\n\r", ch );
    return FALSE;
}

bool can_omodify( CHAR_DATA *ch, OBJ_DATA *obj )
{
    char bes[16];
    int vnum = obj->vnum;
    AREA_DATA *pArea;

    if ( IS_NPC( ch ) )
        return FALSE;

    if (obj->pIndexData->area && !IS_AREA_STATUS(obj->pIndexData->area, AREA_LOADED))
    {
        send_to_char( "Load the area first.\n\r", ch);
        return FALSE;
    }

    /* let anybody edit so they can practice, etc */
    if (vnum>=VNUM_START_SCRATCH && vnum<=VNUM_END_SCRATCH)
        return TRUE;

    if (obj->pIndexData->area &&
        is_name(obj->pIndexData->area->filename, ch->pcdata->bestowments))
        return TRUE;

    sprintf(bes, "obj:%d", vnum);
    if (is_name(bes, ch->pcdata->bestowments))
        return TRUE;

    if ( get_trust( ch ) >= sysdata.level_modify_proto )
    {
        if (obj->pIndexData && obj->pIndexData->area &&
            !IS_AREA_FLAG(obj->pIndexData->area,AFLAG_MODIFIED))
        {
            SET_AREA_FLAG(obj->pIndexData->area,AFLAG_MODIFIED);
            log_printf_plus( LOG_BUILD, GetMaxLevel(ch), SEV_SPAM+9,
                             "%s modified %s", GET_NAME(ch), obj->pIndexData->name );
        }
        return TRUE;
    }
    if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
    {
        send_to_char( "You must have an assigned area to modify this object.\n\r", ch );
        return FALSE;
    }
    if ( vnum >= pArea->low_o_vnum
         &&   vnum <= pArea->hi_o_vnum )
    {
        if (!IS_AREA_FLAG(pArea,AFLAG_MODIFIED))
        {
            SET_AREA_FLAG(pArea,AFLAG_MODIFIED);
            log_printf_plus( LOG_BUILD, GetMaxLevel(ch), SEV_SPAM+9,
                             "%s modified %s", GET_NAME(ch), pArea->name );
        }
        return TRUE;
    }
    if ( !IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
    {
        send_to_char( "You cannot modify this object.\n\r", ch );
        return FALSE;
    }

    send_to_char( "That object is not in your allocated range.\n\r", ch );
    return FALSE;
}

bool can_oedit( CHAR_DATA *ch, OBJ_INDEX_DATA *obj )
{
    char bes[16];
    int vnum = obj->ivnum;
    AREA_DATA *pArea;

    if ( IS_NPC( ch ) )
        return FALSE;

    if (obj->area && !IS_AREA_STATUS(obj->area, AREA_LOADED))
    {
        send_to_char( "Load the area first.\n\r", ch);
        return FALSE;
    }

    /* let anybody edit so they can practice, etc */
    if (vnum>=VNUM_START_SCRATCH && vnum<=VNUM_END_SCRATCH)
        return TRUE;

    if (is_name(obj->area->filename, ch->pcdata->bestowments))
        return TRUE;

    sprintf(bes, "obj:%d", vnum);
    if (is_name(bes, ch->pcdata->bestowments))
        return TRUE;

    if ( get_trust( ch ) >= LEVEL_GOD )
    {
        if (obj->area && !IS_AREA_FLAG(obj->area,AFLAG_MODIFIED))
        {
            SET_AREA_FLAG(obj->area,AFLAG_MODIFIED);
            log_printf_plus( LOG_BUILD, GetMaxLevel(ch), SEV_SPAM+9,
                             "%s modified %s", GET_NAME(ch), obj->area->name );
        }
        return TRUE;
    }
    if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
    {
        send_to_char( "You must have an assigned area to modify this object.\n\r", ch );
        return FALSE;
    }
    if ( vnum >= pArea->low_o_vnum
         &&   vnum <= pArea->hi_o_vnum )
    {
        if (!IS_AREA_FLAG(pArea,AFLAG_MODIFIED))
        {
            SET_AREA_FLAG(pArea,AFLAG_MODIFIED);
            log_printf_plus( LOG_BUILD, GetMaxLevel(ch), SEV_SPAM+9,
                             "%s modified %s", GET_NAME(ch), pArea->name );
        }
        return TRUE;
    }
    if ( !IS_OBJ_STAT( obj, ITEM_PROTOTYPE) )
    {
        send_to_char( "You cannot modify this object.\n\r", ch );
        return FALSE;
    }

    send_to_char( "That object is not in your allocated range.\n\r", ch );
    return FALSE;
}


bool can_affect_proto_mob( CHAR_DATA *ch, CHAR_DATA *mob)
{
    AREA_DATA *pArea;

    if (!ch || !mob)
        return FALSE;

    if (IS_NPC(ch) || !IS_IMMORTAL(ch) ||
        get_trust( ch ) >= sysdata.level_modify_proto)
        return TRUE;

    if (!IS_ACT_FLAG( mob, ACT_PROTOTYPE))
        return TRUE;

    if (!ch->pcdata || !(pArea=ch->pcdata->area))
        return FALSE;

    if (pArea!=mob->pIndexData->area)
        return FALSE;

    if (mob->vnum >= pArea->low_m_vnum &&
        mob->vnum <= pArea->hi_m_vnum)
        return FALSE;

    return TRUE;
}

bool can_mmodify( CHAR_DATA *ch, CHAR_DATA *mob )
{
    char bes[16];
    int vnum = 0;
    AREA_DATA *pArea;

    if ( mob == ch )
    {
        if (mob->pIndexData && mob->pIndexData->area &&
            !IS_AREA_FLAG(mob->pIndexData->area,AFLAG_MODIFIED))
        {
            SET_AREA_FLAG(mob->pIndexData->area,AFLAG_MODIFIED);
            log_printf_plus( LOG_BUILD, GetMaxLevel(ch), SEV_SPAM+9,
                             "%s modified %s", GET_NAME(ch), mob->pIndexData->area->name );
        }
        return TRUE;
    }

    if ( !IS_NPC( mob ) )
    {
        if ( get_trust( ch ) >= sysdata.level_modify_proto && get_trust(ch) >
             get_trust( mob ) )
            return TRUE;
        else
            send_to_char( "You can't do that.\n\r", ch );
        return FALSE;
    }

    if ( IS_NPC( ch ) )
        return FALSE;

    if (mob->pIndexData->area && !IS_AREA_STATUS(mob->pIndexData->area, AREA_LOADED))
    {
        send_to_char( "Load the area first.\n\r", ch);
        return FALSE;
    }

    vnum = mob->vnum;

    /* let anybody edit so they can practice, etc */
    if (vnum>=VNUM_START_SCRATCH && vnum<=VNUM_END_SCRATCH)
        return TRUE;

    if (mob->pIndexData->area &&
        is_name(mob->pIndexData->area->filename, ch->pcdata->bestowments))
        return TRUE;

    sprintf(bes, "mob:%d", vnum);
    if (is_name(bes, ch->pcdata->bestowments))
        return TRUE;

    if ( get_trust( ch ) >= sysdata.level_modify_proto )
    {
        if (mob->pIndexData->area && !IS_AREA_FLAG(mob->pIndexData->area,AFLAG_MODIFIED))
        {
            SET_AREA_FLAG(mob->pIndexData->area,AFLAG_MODIFIED);
            log_printf_plus( LOG_BUILD, GetMaxLevel(ch), SEV_SPAM+9,
                             "%s modified %s", GET_NAME(ch), mob->pIndexData->area->name );
        }
        return TRUE;
    }
    if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
    {
        send_to_char( "You must have an assigned area to modify this mobile.\n\r", ch );
        return FALSE;
    }
    if ( vnum >= pArea->low_m_vnum
         &&   vnum <= pArea->hi_m_vnum )
    {
        if (!IS_AREA_FLAG(pArea,AFLAG_MODIFIED))
        {
            SET_AREA_FLAG(pArea,AFLAG_MODIFIED);
            log_printf_plus( LOG_BUILD, GetMaxLevel(ch), SEV_SPAM+9,
                             "%s modified %s", GET_NAME(ch), pArea->name );
        }
        return TRUE;
    }
    if ( !IS_ACT_FLAG( mob, ACT_PROTOTYPE) )
    {
        send_to_char( "You cannot modify this mobile.\n\r", ch );
        return FALSE;
    }

    send_to_char( "That mobile is not in your allocated range.\n\r", ch );
    return FALSE;
}

bool can_medit( CHAR_DATA *ch, MOB_INDEX_DATA *mob )
{
    char bes[16];
    int vnum = mob->ivnum;
    AREA_DATA *pArea;

    if ( IS_NPC( ch ) )
        return FALSE;

    if (mob->area && !IS_AREA_STATUS(mob->area, AREA_LOADED))
    {
        send_to_char( "Load the area first.\n\r", ch);
        return FALSE;
    }

    /* let anybody edit so they can practice, etc */
    if (vnum>=VNUM_START_SCRATCH && vnum<=VNUM_END_SCRATCH)
        return TRUE;

    if (is_name(mob->area->filename, ch->pcdata->bestowments))
        return TRUE;

    sprintf(bes, "mob:%d", vnum);
    if (is_name(bes, ch->pcdata->bestowments))
        return TRUE;

    if ( get_trust( ch ) >= LEVEL_GOD )
    {
        if (mob->area && !IS_AREA_FLAG(mob->area,AFLAG_MODIFIED))
        {
            SET_AREA_FLAG(mob->area,AFLAG_MODIFIED);
            log_printf_plus( LOG_BUILD, GetMaxLevel(ch), SEV_SPAM+9,
                             "%s modified %s", GET_NAME(ch), mob->area->name );
        }
        return TRUE;
    }
    if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
    {
        send_to_char( "You must have an assigned area to modify this mobile.\n\r", ch );
        return FALSE;
    }
    if ( vnum >= pArea->low_m_vnum
         &&   vnum <= pArea->hi_m_vnum )
    {
        if (!IS_AREA_FLAG(pArea,AFLAG_MODIFIED))
        {
            SET_AREA_FLAG(pArea,AFLAG_MODIFIED);
            log_printf_plus( LOG_BUILD, GetMaxLevel(ch), SEV_SPAM+9,
                             "%s modified %s", GET_NAME(ch), pArea->name );
        }
        return TRUE;
    }
    if ( !IS_ACT_FLAG( mob, ACT_PROTOTYPE) )
    {
        send_to_char( "You cannot modify this mobile.\n\r", ch );
        return FALSE;
    }

    send_to_char( "That mobile is not in your allocated range.\n\r", ch );
    return FALSE;
}

int get_otype( char *type )
{
    unsigned int x;

    for ( x = 0; x < (sizeof(o_types) / sizeof(o_types[0]) ); x++ )
        if ( !str_cmp( type, o_types[x] ) )
            return x;
    return -1;
}

int get_pipeflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, pipe_flags[x] ) )
            return x;
    return -1;
}

int get_aflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, a_flags[x] ) )
            return x;
    return -1;
}

int get_a2flag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, a2_flags[x] ) )
            return x;
    return -1;
}

int get_trapflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, trap_flags[x] ) )
            return x;
    return -1;
}

int get_atype( char *type )
{
    int x;

    for ( x = 0; x < MAX_APPLY_TYPE; x++ )
        if ( !str_cmp( type, a_types[x] ) )
            return x;
    return -1;
}

int get_npc_race( char *type )
{
    int x;

    for ( x = 0; x < MAX_RACE; x++ )
        if ( !str_cmp( type, race_table[x].race_name ) )
            return x;
    return -1;
}

int get_wearloc( char *type )
{
    int x;

    for ( x = 0; x < MAX_WEAR; x++ )
        if ( !str_cmp( type, wear_locs[x] ) )
            return x;
    return -1;
}

int get_exflag( char *flag )
{
    int x;

    for ( x = 0; x <= MAX_EXFLAG; x++ )
        if ( !str_cmp( flag, ex_flags[x] ) )
            return x;
    return -1;
}

int get_rflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, r_flags[x] ) )
            return x;
    return -1;
}

int get_mpflag( char *flag )
{
    int x;

    for ( x = 0; x < NUM_PROG_TYPES; x++ )
        if ( !str_cmp( flag, mprog_flags[x] ) )
            return x;
    return -1;
}

int get_oflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, o_flags[x] ) )
            return x;
    return -1;
}

int get_o2flag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, o2_flags[x] ) )
            return x;
    return -1;
}

int get_magflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, mag_flags[x] ) )
            return x;
    return -1;
}

int get_areaflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, area_flags[x] ) )
            return x;
    return -1;
}

int get_plane( char *name )
{
    int x;

    for ( x = FIRST_PLANE; x < LAST_PLANE; x++ )
        if ( !str_cmp( name, plane_names[x] ) )
            return x;
    return PLANE_NORMAL;
}

int get_wflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, w_flags[x] ) )
            return x;
    return -1;
}

int get_actflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, act_flags[x] ) )
            return x;
    return -1;
}

int get_act2flag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, act2_flags[x] ) )
            return x;
    return -1;
}

int get_pcflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, pc_flags[x] ) )
            return x;
    return -1;
}

int get_plrflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, plr_flags[x] ) )
            return x;
    return -1;
}

int get_plr2flag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, plr2_flags[x] ) )
            return x;
    return -1;
}

int get_risflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, ris_flags[x] ) )
            return x;
    return -1;
}

int get_trigflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, trig_flags[x] ) )
            return x;
    return -1;
}

int get_partflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, part_flags[x] ) )
            return x;
    return -1;
}

int get_attackflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, attack_flags[x] ) )
            return x;
    return -1;
}

int get_defenseflag( char *flag )
{
    int x;

    for ( x = 0; x < 32; x++ )
        if ( !str_cmp( flag, defense_flags[x] ) )
            return x;
    return -1;
}

int get_langflag( char *flag )
{
    int x;

    for ( x = 0; lang_array[x] != LANG_UNKNOWN; x++ )
        if ( !str_cmp( flag, lang_names[x] ) )
            return lang_array[x];

    return LANG_UNKNOWN;
}

int get_langnum( char *flag )
{
        int x;

        for ( x = 0; lang_array[x] != LANG_UNKNOWN; x++ )
                if ( !str_cmp( flag, lang_names[x] ) )
                        return x;
        return -1;
}

sh_int get_postype( char *type )
{
    sh_int x;

    for ( x = 0; x < MAX_POSITION; x++ )
        if ( !str_cmp( position_types[x], type ) )
            return x;

    return -1;
}

sh_int get_classtype( char *type )
{
    sh_int x;

    for ( x = FIRST_CLASS; x < MAX_CLASS; x++ )
        if ( !str_prefix( type, pc_class[x] ) )
            return x;

    return CLASS_NONE;
}

int get_sectortype( char *type )
{
    int x;

    for ( x = 0; x < SECT_MAX; x++ )
        if ( !str_cmp( type, sect_types[x] ) )
            return x;
    return -1;
}


/*
 * Remove carriage returns from a line
 */
char *strip_cr( char *str )
{
    static char newstr[MAX_STRING_LENGTH];
    int i, j;

    for ( i=j=0; str[i] != '\0'; i++ )
        if ( str[i] != '\r' )
        {
            newstr[j++] = str[i];
        }
    newstr[j] = '\0';
    return newstr;
}

char *unescape( char *str )
{
    static char newstr[MAX_STRING_LENGTH];
    int i, j;

    for ( i=j=0; str[i] != '\0'; i++ )
	if (str[i] == '\\') {
            newstr[j++] = '\\';
            newstr[j++] = '\\';
	} else if (str[i] == '\'') {
            newstr[j++] = '\\';
            newstr[j++] = '\'';
        } else if ( str[i] == '\r' ) {
	    continue;
	} else if ( str[i] == '\n' ) {
            if ( str[i+1] == '\0' || (str[i+1] == '\r' && str[i+2] == '\0') )
		newstr[j++] = '\0';
	    else
		newstr[j++] = ' ';
	} else if ( str[i] == ' ' && str[i+1] == '\0' ) {
	    newstr[j++] = '\0';
	} else {
            newstr[j++] = str[i];
	}
    newstr[j] = '\0';
    return newstr;
}

char *strip_crlf( char *str )
{
    static char newstr[MAX_STRING_LENGTH];
    int i, j;

    for ( i=j=0; str[i] != '\0'; i++ )
        if ( str[i] != '\r' && str[i] != '\n' )
        {
            newstr[j++] = str[i];
        }
    newstr[j] = '\0';
    return newstr;
}

char *strip_lf( char *str )
{
    static char newstr[MAX_STRING_LENGTH];
    int i, j;

    for ( i=j=0; str[i] != '\0'; i++ )
        if ( str[i] != '\n' )
        {
            newstr[j++] = str[i];
        }
    newstr[j] = '\0';
    return newstr;
}


/*
 * Removes the tildes from a line, except if it's the last character.
 */
void smush_tilde( char *str )
{
    int len;
    char last;
    char *strptr;

    strptr = str;

    len  = strlen( str );
    if ( len )
        last = strptr[len-1];
    else
        last = '\0';

    for ( ; *str != '\0'; str++ )
    {
        if ( *str == '~' )
            *str = '-';
    }
    if ( len )
        strptr[len-1] = last;

    return;
}


void start_editing( CHAR_DATA *ch, char *data )
{
    EDITOR_DATA *edit;
    sh_int lines, size, lpos;
    char c;

    if ( !ch->desc )
    {
        bug( "Fatal: start_editing: no desc" );
        return;
    }
    if ( ch->substate == SUB_RESTRICTED )
        bug( "NOT GOOD: start_editing: ch->substate == SUB_RESTRICTED" );

    set_char_color( AT_GREEN, ch );
    send_to_char( "Begin entering your text now (/? = help /s = save /c = clear /l = list)\n\r", ch );
    send_to_char( "-------------------------------------------------------------------------------\n\r> ", ch );
    if ( ch->editor )
        stop_editing( ch );

    CREATE( edit, EDITOR_DATA, 1 );
    edit->numlines = 0;
    edit->on_line  = 0;
    edit->size     = 0;
    size = 0;  lpos = 0;  lines = 0;
    if ( !data )
    {
        bug("editor: data is NULL!\n\r");
    }
    else
        for ( ;; )
        {
            c = data[size++];
            if ( c == '\0' )
            {
                edit->line[lines][lpos] = '\0';
                if (lpos > 0)
                    lines++;
                break;
            }
            else
                if ( c == '\r' );
                else
                    if ( c == '\n' || lpos > MAX_EDIT_LINE_LENGTH-3)
                    {
                        edit->line[lines][lpos] = '\0';
                        lines++;
                        lpos = 0;
                    }
                    else
                        edit->line[lines][lpos++] = c;
            if ( lines >= MAX_EDIT_LINES || size > 4096 )
            {
                edit->line[lines][lpos] = '\0';
                break;
            }
        }
    edit->numlines = lines;
    edit->size = size;
    edit->on_line = lines;
    ch->editor = edit;
    ch->desc->connected = CON_EDITING;
}

char *copy_buffer( CHAR_DATA *ch )
{
    char buf[MAX_EDIT_LINE_LENGTH*MAX_EDIT_LINES];
    char tmp[MAX_EDIT_LINE_LENGTH+1];
    sh_int x, len;

    if ( !ch )
    {
        bug( "copy_buffer: null ch" );
        return STRALLOC( "" );
    }

    if ( !ch->editor )
    {
        bug( "copy_buffer: null editor" );
        return STRALLOC( "" );
    }

    buf[0] = '\0';
    for ( x = 0; x < ch->editor->numlines; x++ )
    {
        strncpy( tmp, ch->editor->line[x], MAX_EDIT_LINE_LENGTH );
        smush_tilde( tmp );
        len = strlen(tmp);
        if ( tmp && tmp[len-1] == '~' )
            tmp[len-1] = '\0';
        else
            strcat( tmp, "\n\r" );
        strcat( buf, tmp );
    }

    return STRALLOC( buf );
}

void stop_editing( CHAR_DATA *ch )
{
    set_char_color( AT_PLAIN, ch );
    DISPOSE( ch->editor );
    ch->editor = NULL;
    send_to_char( "Done.\n\r", ch );
    ch->dest_buf  = NULL;
    ch->spare_ptr = NULL;
    ch->substate  = SUB_NONE;
    if ( !ch->desc )
    {
        bug( "Fatal: stop_editing: no desc" );
        return;
    }
    ch->desc->connected = CON_PLAYING;
}

void do_goto( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    AREA_DATA *pArea;
    int vnum;

    if (IS_NPC(ch))
    {
        send_to_char("You can't do this as an NPC.\n\r", ch);
        return;
    }

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Goto where?\n\r", ch );
        return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
        vnum = atoi( arg );
        if ( vnum < 0 || room_exists_index( vnum ) )
        {
            send_to_char( "You cannot find that...\n\r", ch );
            return;
        }
        if ( get_trust( ch ) < LEVEL_CREATOR
             ||   vnum < 1 || IS_NPC(ch) || !ch->pcdata->area )
        {
            send_to_char( "No such location.\n\r", ch );
            return;
        }
        if ( get_trust( ch ) < sysdata.level_modify_proto )
        {
            if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
            {
                send_to_char( "You must have an assigned area to create rooms.\n\r", ch );
                return;
            }
            if ( vnum < pArea->low_r_vnum
                 ||   vnum > pArea->hi_r_vnum )
            {
                send_to_char( "That room is not within your assigned range.\n\r", ch );
                return;
            }
        }
        location = make_room( vnum );
        if ( !location )
        {
            bug( "Goto: make_room failed" );
            return;
        }
        location->area = ch->pcdata->area;
        set_char_color( AT_WHITE, ch );
        send_to_char( "Waving your hand, you form order from swirling chaos,\n\rand step into a new reality...\n\r", ch );
    }

    if ( room_is_private( location ) )
    {
        if ( get_trust( ch ) < sysdata.level_override_private )
        {
            send_to_char( "That room is private right now.\n\r", ch );
            return;
        }
        else
            send_to_char( "Overriding private flag!\n\r", ch );
    }

    in_room = ch->in_room;
    if ( ch->fighting )
        stop_fighting( ch, TRUE );


    for ( fch = ch->in_room->first_person; fch; fch = fch->next_in_room )
    {
        if (ch == fch || !can_see(fch, ch))
            continue;

        if (ch->pcdata && ch->pcdata->bamfout[0] != '\0')
            sprintf(buf, "%s", ch->pcdata->bamfout);
        else
            sprintf(buf, "%s leaves in a swirling mist.", PERS(ch, fch));

        set_char_color( AT_IMMORT, fch );
        if (IS_IMMORTAL(fch))
            ch_printf(fch, "(%s) %s\n\r", GET_NAME(ch), buf);
        else
            ch_printf(fch, "%s\n\r", buf);
    }

    ch->regoto = ch->in_room->vnum;

    if (is_other_plane(ch->in_room, location))
        send_to_char("You skip and hop the planes.\n\r", ch);

    char_from_room( ch );
    if ( ch->mount )
    {
        char_from_room( ch->mount );
        char_to_room( ch->mount, location );
    }
    char_to_room( ch, location );

    for ( fch = ch->in_room->first_person; fch; fch = fch->next_in_room )
    {
        if (ch == fch || !can_see(fch, ch))
            continue;

        if (ch->pcdata && ch->pcdata->bamfin[0] != '\0')
            sprintf(buf, "%s", ch->pcdata->bamfin);
        else
            sprintf(buf, "%s appears in a swirling mist.", PERS(ch, fch));

        set_char_color( AT_IMMORT, fch );
        if (IS_IMMORTAL(fch))
            ch_printf(fch, "(%s) %s\n\r", GET_NAME(ch), buf);
        else
            ch_printf(fch, "%s\n\r", buf);
    }

    do_look( ch, "auto" );

    if ( ch->in_room == in_room )
        return;
    for ( fch = in_room->first_person; fch; fch = fch_next )
    {
        fch_next = fch->next_in_room;
        if ( fch->master == ch && IS_IMMORTAL(fch) )
        {
            act( AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR );
            do_goto( fch, argument );
        }
    }
    return;
}

void do_mset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf  [MAX_STRING_LENGTH];
    char outbuf[MAX_STRING_LENGTH];
    int  num,size,plus;
    int  v2;
    char char1,char2;
    CHAR_DATA *victim;
    int value, val2;
    int minattr, maxattr, type;
    bool lockvictim;
    char *origarg = argument;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mob's can't mset\n\r", ch );
        return;
    }

    if ( !ch->desc )
    {
        send_to_char( "You have no descriptor\n\r", ch );
        return;
    }

    switch( ch->substate )
    {
    default:
        break;
    case SUB_MOB_DESC:
        if ( !ch->dest_buf )
        {
            send_to_char( "Fatal error: report to Thoric.\n\r", ch );
            bug( "do_mset: sub_mob_desc: NULL ch->dest_buf" );
            ch->substate = SUB_NONE;
            return;
        }
        victim = (CHAR_DATA *)ch->dest_buf;
        if ( char_died(victim) )
        {
            send_to_char( "Your victim died!\n\r", ch );
            stop_editing( ch );
            return;
        }
        STRFREE( victim->description );
        victim->description = copy_buffer( ch );
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
        {
            STRFREE( victim->pIndexData->description );
            victim->pIndexData->description = QUICKLINK( victim->description );
        }
        stop_editing( ch );
        ch->substate = ch->tempnum;
        return;
    }

    victim = NULL;
    lockvictim = FALSE;
    smash_tilde( argument );

    if ( ch->substate == SUB_REPEATCMD )
    {
        victim = (CHAR_DATA *)ch->dest_buf;
        if ( char_died(victim) )
        {
            send_to_char( "Your victim died!\n\r", ch );
            victim = NULL;
            argument = "done";
        }
        if ( argument[0] == '\0' || !str_cmp( argument, " " )
             ||   !str_cmp( argument, "stat" ) )
        {
            if ( victim )
                do_mstat( ch, victim->name );
            else
                send_to_char( "No victim selected.  Type '?' for help.\n\r", ch );
            return;
        }
        if ( !str_cmp( argument, "done" ) || !str_cmp( argument, "off" ) )
        {
            send_to_char( "Mset mode off.\n\r", ch );
            ch->substate = SUB_NONE;
            ch->dest_buf = NULL;
            if ( ch->pcdata && ch->pcdata->subprompt )
                STRFREE( ch->pcdata->subprompt );
            return;
        }
    }
    if ( victim )
    {
        lockvictim = TRUE;
        strcpy( arg1, victim->name );
        argument = one_argument( argument, arg2 );
        strcpy( arg3, argument );
    }
    else
    {
        lockvictim = FALSE;
        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );
        strcpy( arg3, argument );
    }

    if ( !str_cmp( arg1, "on" ) )
    {
        send_to_char( "Syntax: mset <victim|vnum> on.\n\r", ch );
        return;
    }

    if ( arg1[0] == '\0' || (arg2[0] == '\0' && ch->substate != SUB_REPEATCMD)
         ||   !str_cmp( arg1, "?" ) )
    {
        if ( ch->substate == SUB_REPEATCMD )
        {
            if ( victim )
                send_to_char( "Syntax: <field>  <value>\n\r",		ch );
            else
                send_to_char( "Syntax: <victim> <field>  <value>\n\r",	ch );
        }
        else
            send_to_char( "Syntax: mset <victim> <field>  <value>\n\r",	ch );
        send_to_char( "\n\r",						ch );
        send_to_char( "Field being one of:\n\r",			ch );
        send_to_char( "  str int wis dex con cha lck sex height weight\n\r", ch );
        send_to_char( "  hp mana move practice align race\n\r",	        ch );
        send_to_char( "  hitroll damroll armor affected affected2 level\n\r", ch );
        send_to_char( "  thirst drunk full blood flags flags2\n\r",	ch );
        send_to_char( "  pos defpos part (see BODYPARTS)\n\r",		ch );
        send_to_char( "  sav1 sav2 sav4 sav4 sav5 (see SAVINGTHROWS)\n\r", ch );
        send_to_char( "  resistant immune susceptible absorb (see RIS)\n\r", ch );
        send_to_char( "  attack defense numattacks exp\n\r",		ch );
        send_to_char( "  speaking speaks (see LANGUAGES)\n\r",		ch );
        send_to_char( "  name short long description title spec clan\n\r", ch );
        send_to_char( "  council quest qp qpa favor deity intro rank\n\r", ch );
        send_to_char( "  timer icelisten i3listen\n\r", ch);
        send_to_char( " ", ch);
        for (type=1;type<MAX_CURR_TYPE;type++)
            ch_printf(ch, " %s", curr_types[type]);
        send_to_char( "\n\r\n\r",                                          ch );
        send_to_char( "For editing index/prototype mobiles:\n\r",	ch );
        send_to_char( "  hitnumdie hitsizedie hitplus (hit points)\n\r",ch );
        send_to_char( "  damnumdie damsizedie damplus (damage roll)\n\r",ch );
        send_to_char( "To toggle area flag: aloaded\n\r",ch);
        send_to_char( "To toggle pkill flag: pkill\n\r",ch);
        return;
    }

    if ( !victim && get_trust( ch ) < LEVEL_GOD )
    {
        if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }
    }
    else
        if ( !victim )
        {
            if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
            {
                send_to_char( "No one like that in all the realms.\n\r", ch );
                return;
            }
        }

    if ( get_trust( ch ) < get_trust( victim ) && !IS_NPC( victim ) )
    {
        send_to_char( "You can't do that!\n\r", ch );
        ch->dest_buf = NULL;
        return;
    }
    if ( lockvictim )
        ch->dest_buf = victim;

    if ( IS_NPC(victim) )
    {
        minattr = 1;
        maxattr = 25;
    }
    else
    {
        minattr = 3;
        maxattr = 18;
    }

    if ( !str_cmp( arg2, "on" ) )
    {
        CHECK_SUBRESTRICTED( ch );
        ch_printf( ch, "Mset mode on. (Editing %s).\n\r",
                   victim->name );
        ch->substate = SUB_REPEATCMD;
        ch->dest_buf = victim;
        if ( ch->pcdata )
        {
            if ( ch->pcdata->subprompt )
                STRFREE( ch->pcdata->subprompt );
            if ( IS_NPC(victim) )
                sprintf( buf, "<&CMset &W#%d&w> %%i", victim->vnum );
            else
                sprintf( buf, "<&CMset &W%s&w> %%i", victim->name );
            ch->pcdata->subprompt = STRALLOC( buf );
        }
        return;
    }
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    if ( atoi(arg3) < -1 && value == -1 )
        value = atoi(arg3);

    if ( !str_cmp( arg2, "str" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < minattr || value > maxattr )
        {
            ch_printf( ch, "Strength range is %d to %d.\n\r", minattr, maxattr );
            return;
        }
        victim->perm_str = value;
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->perm_str = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "int" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < minattr || value > maxattr )
        {
            ch_printf( ch, "Intelligence range is %d to %d.\n\r", minattr, maxattr );
            return;
        }
        victim->perm_int = value;
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->perm_int = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "wis" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < minattr || value > maxattr )
        {
            ch_printf( ch, "Wisdom range is %d to %d.\n\r", minattr, maxattr );
            return;
        }
        victim->perm_wis = value;
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->perm_wis = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "dex" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < minattr || value > maxattr )
        {
            ch_printf( ch, "Dexterity range is %d to %d.\n\r", minattr, maxattr );
            return;
        }
        victim->perm_dex = value;
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->perm_dex = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "con" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < minattr || value > maxattr )
        {
            ch_printf( ch, "Constitution range is %d to %d.\n\r", minattr, maxattr );
            return;
        }
        victim->perm_con = value;
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->perm_con = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "cha" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < minattr || value > maxattr )
        {
            ch_printf( ch, "Charisma range is %d to %d.\n\r", minattr, maxattr );
            return;
        }
        victim->perm_cha = value;
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->perm_cha = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "lck" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < minattr || value > maxattr )
        {
            ch_printf( ch, "Luck range is %d to %d.\n\r", minattr, maxattr );
            return;
        }
        victim->perm_lck = value;
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->perm_lck = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "sav1" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < -30 || value > 30 )
        {
            send_to_char( "Saving throw range is -30 to 30.\n\r", ch );
            return;
        }
        victim->saving_poison_death = value;
        SET_ACT_FLAG(victim, ACT_CUSTOMSAVES);
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
        {
            victim->pIndexData->saving_poison_death = value;
            SET_BIT(victim->pIndexData->act, ACT_CUSTOMSAVES);
        }
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "sav2" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < -30 || value > 30 )
        {
            send_to_char( "Saving throw range is -30 to 30.\n\r", ch );
            return;
        }
        victim->saving_wand = value;
        SET_ACT_FLAG(victim, ACT_CUSTOMSAVES);
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
        {
            victim->pIndexData->saving_wand = value;
            SET_BIT(victim->pIndexData->act, ACT_CUSTOMSAVES);
        }
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "sav3" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < -30 || value > 30 )
        {
            send_to_char( "Saving throw range is -30 to 30.\n\r", ch );
            return;
        }
        victim->saving_para_petri = value;
        SET_ACT_FLAG(victim, ACT_CUSTOMSAVES);
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
        {
            victim->pIndexData->saving_para_petri = value;
            SET_BIT(victim->pIndexData->act, ACT_CUSTOMSAVES);
        }
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "sav4" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < -30 || value > 30 )
        {
            send_to_char( "Saving throw range is -30 to 30.\n\r", ch );
            return;
        }
        victim->saving_breath = value;
        SET_ACT_FLAG(victim, ACT_CUSTOMSAVES);
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
        {
            victim->pIndexData->saving_breath = value;
            SET_BIT(victim->pIndexData->act, ACT_CUSTOMSAVES);
        }
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "sav5" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < -30 || value > 30 )
        {
            send_to_char( "Saving throw range is -30 to 30.\n\r", ch );
            return;
        }
        victim->saving_spell_staff = value;
        SET_ACT_FLAG(victim, ACT_CUSTOMSAVES);
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
        {
            victim->pIndexData->saving_spell_staff = value;
            SET_BIT(victim->pIndexData->act, ACT_CUSTOMSAVES);
        }
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "sex" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < 0 || value > 2 )
        {
            send_to_char( "Sex range is 0 to 2.\n\r", ch );
            return;
        }
        victim->sex = value;
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->sex = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "height" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < 1 || value > 32000 )
        {
            send_to_char( "Height range is 1 to 32000.\n\r", ch );
            return;
        }
        victim->height = value;
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->height = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "weight" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < 1 || value > 32000 )
        {
            send_to_char( "Weight range is 1 to 32000.\n\r", ch );
            return;
        }
        victim->weight = value;
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->weight = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "race" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        value = get_npc_race( arg3 );
        if ( value < 0 )
            value = atoi( arg3 );
        if ( value < 0 || value >= MAX_RACE )
        {
            ch_printf( ch, "Race range is 0 to %d.\n", MAX_RACE-1 );
            return;
        }
        victim->race = value;
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->race = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "armor" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < -300 || value > 300 )
        {
            send_to_char( "AC range is -300 to 300.\n\r", ch );
            return;
        }
        victim->armor = value;
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->ac = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "level" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( !IS_NPC(victim) && (get_trust(ch) < LEVEL_SUPREME ))
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }

	argument = one_argument( argument, arg3 );
	value = is_number(arg3) ? atoi(arg3) : -1;

	argument = one_argument( argument, arg3 );
        val2 = get_classtype(arg3);

        if (value < 0 || value > LEVEL_AVATAR + 10 ||
	    val2 < 0 || val2 >= MAX_CLASS) {
            send_to_char("Usage: mset level <level> <class>\n\r", ch);
	    ch_printf(ch, "Level: 0-%d\n\r", LEVEL_AVATAR+10);
	    sprintf(buf, "Class:");
	    for (value=FIRST_CLASS;value<MAX_CLASS;value++)
	    {
		strcat(buf, " ");
		strcat(buf, pc_class[value]);
	    }
	    send_to_char(buf, ch);
            return;
        }

        victim->levels[val2] = value;

        if (value==0)
	{
            REMOVE_BIT(victim->classes[val2], STAT_ACTCLASS);
	    REMOVE_BIT(victim->classes[val2], STAT_OLDCLASS);
	}
        else
	{
            SET_BIT(victim->classes[val2], STAT_ACTCLASS);
	    REMOVE_BIT(victim->classes[val2], STAT_OLDCLASS);
	}

        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
	{
            victim->pIndexData->levels[val2] = value;
            victim->pIndexData->classes[val2] = victim->classes[val2];
            if (value==0)
	    {
        	REMOVE_BIT(victim->pIndexData->classes[val2], STAT_ACTCLASS);
		REMOVE_BIT(victim->pIndexData->classes[val2], STAT_OLDCLASS);
	    }
            else
	    {
        	SET_BIT(victim->pIndexData->classes[val2], STAT_ACTCLASS);
		REMOVE_BIT(victim->pIndexData->classes[val2], STAT_OLDCLASS);
	    }
	}
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "numattacks" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( value >= 0 && value <= 20 )
            victim->numattacks = value*1000;
        else if (value >= 1000 && value <= 20000 )
            victim->numattacks = value;
        else {
            send_to_char("Numattacks value must be between 0 and 20\n\r",ch);
            return;
        }
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->numattacks = value*1000;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "exp" ) || !str_cmp( arg2, "xp" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        victim->exp = value;
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->exp = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( (type=get_currency_type(arg2)) )
    {
        if ( !can_mmodify( ch, victim ) )
          return;
        GET_MONEY(victim,type) = value;
        if ( IS_NPC(victim) && IS_SET( victim->act, ACT_PROTOTYPE ) )
          GET_MONEY(victim->pIndexData,type) = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "hitroll" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        victim->hitroll = URANGE(0, value, 85);
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->hitroll = victim->hitroll;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "damroll" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        victim->damroll = URANGE(0, value, 65);
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->damroll = victim->damroll;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "hp" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        if ( value < 1 || value > 32700 )
        {
            send_to_char( "Hp range is 1 to 32,700 hit points.\n\r", ch );
            return;
        }
        victim->max_hit = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "mana" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( !IS_NPC(victim) && (get_trust(ch) < LEVEL_SUPREME))
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 30000 )
        {
            send_to_char( "Mana range is 0 to 30,000 mana points.\n\r", ch );
            return;
        }
        victim->max_mana = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "move" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( !IS_NPC(victim) && (get_trust(ch) < LEVEL_SUPREME))
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 30000 )
        {
            send_to_char( "Move range is 0 to 30,000 move points.\n\r", ch );
            return;
        }
        victim->max_move = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "practice" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < 0 || value > 100 )
        {
            send_to_char( "Practice range is 0 to 100 sessions.\n\r", ch );
            return;
        }
        victim->practice = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "align" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < -1000 || value > 1000 )
        {
            send_to_char( "Alignment range is -1000 to 1000.\n\r", ch );
            return;
        }
        victim->alignment = value;
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->alignment = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "password" ) )
    {
        char *pwdnew;
        char *p;

        if (get_trust( ch ) < LEVEL_INFINITE )
        {
            send_to_char( "You can't do that.\n\r", ch );
            return;
        }
        if (get_trust(ch) < get_trust(victim))
        {
            send_to_char("I don't think so...\n\r", ch);
            ch_printf(victim, "%s tried to change your password.\n\r", GET_NAME(ch));
        }
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Mobs don't have passwords.\n\r", ch );
            return;
        }

        if ( strlen(arg3) < 5 )
        {
            send_to_char(
                         "New password must be at least five characters long.\n\r", ch );
            return;
        }

        /*
         * No tilde allowed because of player file format.
         */
        pwdnew = crypt( arg3, ch->name );
        for ( p = pwdnew; *p != '\0'; p++ )
        {
            if ( *p == '~' )
            {
                send_to_char(
                             "New password not acceptable, try again.\n\r", ch );
                return;
            }
        }

        DISPOSE( victim->pcdata->pwd );
        victim->pcdata->pwd = str_dup( pwdnew );
        if ( IS_SAVE_FLAG(sysdata, SV_PASSCHG) )
            save_char_obj( victim );
        send_to_char( "Ok.\n\r", ch );
        ch_printf( victim, "Your password has been changed by %s.\n\r", ch->name );
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "home" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        victim->pcdata->home = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "timedie" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        victim->pcdata->time_to_die = current_time + ((60*60) * value);
        ch_printf(ch, "Time to Die set to: %s", ctime(&victim->pcdata->time_to_die));
        return;
    }

    if ( !str_cmp( arg2, "interface" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        victim->pcdata->interface = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

#ifdef I3
    if ( !str_cmp( arg2, "i3listen" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if (I3LISTEN(victim))
            I3STRFREE( I3LISTEN(victim) );

        I3LISTEN(victim) = I3STRALLOC(arg3);
        send_to_char("Ok.\n\r", ch);
        return;
    }
#endif

    if ( !str_cmp( arg2, "quest" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 500 )
        {
            send_to_char( "The current quest range is 0 to 500.\n\r", ch );
            return;
        }

        victim->pcdata->quest_number = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "qpa" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        victim->pcdata->quest_accum = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "qp" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 5000 )
        {
            send_to_char( "The current quest point range is 0 to 5000.\n\r", ch );
            return;
        }

        victim->pcdata->quest_curr = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "favor" ) )
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < -1000 || value > 1000 )
        {
            send_to_char( "Range is from -1000 to 1000.\n\r", ch );
            return;
        }

        victim->pcdata->favor = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "mentalstate" ) )
    {
        if ( value < -100 || value > 100 )
        {
            send_to_char( "Value must be in range -100 to +100.\n\r", ch );
            return;
        }
        victim->mental_state = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "emotion" ) )
    {
        if ( value < -100 || value > 100 )
        {
            send_to_char( "Value must be in range -100 to +100.\n\r", ch );
            return;
        }
        victim->emotional_state = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "thirst" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 100 )
        {
            send_to_char( "Thirst range is 0 to 100.\n\r", ch );
            return;
        }

        victim->pcdata->condition[COND_THIRST] = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "drunk" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 100 )
        {
            send_to_char( "Drunk range is 0 to 100.\n\r", ch );
            return;
        }

        victim->pcdata->condition[COND_DRUNK] = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "full" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 100 )
        {
            send_to_char( "Full range is 0 to 100.\n\r", ch );
            return;
        }

        victim->pcdata->condition[COND_FULL] = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "blood" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > MAX_LEVEL+10 )
        {
            ch_printf( ch, "Blood range is 0 to %d.\n\r", MAX_LEVEL+10 );
            return;
        }

        victim->pcdata->condition[COND_BLOODTHIRST] = value;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
	if ( !IS_NPC(victim) )
	{
	    if ( get_trust( ch ) < LEVEL_SUB_IMPLEM )
	    {
		send_to_char( "Not on PC's.\n\r", ch );
		return;
	    }
	    if ( !check_parse_name( arg3 ) )
	    {
		send_to_char( "Invalid name for a PC.\n\r", ch );
                return;
	    }
	}

        STRFREE( victim->name );
        victim->name = STRALLOC( arg3 );
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
        {
            STRFREE( victim->pIndexData->player_name );
            victim->pIndexData->player_name = QUICKLINK( victim->name );
        }
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "minsnoop" ) )
    {
        if ( get_trust( ch ) < LEVEL_SUB_IMPLEM )
        {
            send_to_char( "You can't do that.\n\r", ch );
            return;
        }
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }
        if ( victim->pcdata )
        {
            victim->pcdata->min_snoop = value;
            return;
        }
        send_to_char("Ok.\n\r", ch);
    }

    if ( !str_cmp( arg2, "clan" ) )
    {
        CLAN_DATA *clan;

        if ( get_trust( ch ) < LEVEL_GREATER )
        {
            send_to_char( "You can't do that.\n\r", ch );
            return;
        }
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( !arg3 || arg3[0] == '\0' )
        {
            if ( victim->pcdata->clan == NULL )
                return;
            if ( !IS_IMMORTAL( victim ) ) {
                --victim->pcdata->clan->members;
                save_clan( victim->pcdata->clan );
            }
            STRFREE( victim->pcdata->clan_name );
            victim->pcdata->clan_name	= STRALLOC( "" );
            victim->pcdata->clan	= NULL;
            send_to_char( "Removed from clan.\n\rPlease make sure you adjust that clan's members accordingly.\n\r", ch );
            return;
        }
        clan = get_clan( arg3 );
        if ( !clan )
        {
            send_to_char( "No such clan.\n\r", ch );
            return;
        }
        if ( victim->pcdata->clan != NULL && !IS_IMMORTAL( victim ) )
        {
            --victim->pcdata->clan->members;
            save_clan( victim->pcdata->clan );
        }
        STRFREE( victim->pcdata->clan_name );
        victim->pcdata->clan_name = QUICKLINK( clan->name );
        victim->pcdata->clan = clan;
        send_to_char( "Done.\n\rPlease make sure you adjust that clan's members accordingly.\n\r", ch );
        if ( !IS_IMMORTAL( victim ) ) {
            ++victim->pcdata->clan->members;
           save_clan( victim->pcdata->clan );
        }
         send_to_char("Ok.\n\r", ch);
       return;
    }

    if ( !str_cmp( arg2, "deity" ))
    {
        DEITY_DATA *deity;

        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

	if ( get_trust( ch ) < LEVEL_GOD )
	{
	    send_to_char("Use supplicate instead.\n\r", ch);
	    return;
	}

        if ( !arg3 || arg3[0] == '\0' )
        {
            STRFREE( victim->pcdata->deity_name );
            victim->pcdata->deity_name        = STRALLOC( "" );
            victim->pcdata->deity             = NULL;
            send_to_char( "Deity removed.\n\r", ch );
            return;
        }

        deity = get_deity( arg3 );
        if ( !deity )
        {
            send_to_char( "No such deity.\n\r", ch );
            return;
        }
        STRFREE( victim->pcdata->deity_name );
        victim->pcdata->deity_name = QUICKLINK( deity->name );
        victim->pcdata->deity = deity;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "council" ) )
    {
        COUNCIL_DATA *council;

        if ( get_trust( ch ) < LEVEL_GOD )
        {
            send_to_char( "You can't do that.\n\r", ch );
            return;
        }
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( !arg3 || arg3[0] == '\0' )
        {
            STRFREE( victim->pcdata->council_name );
            victim->pcdata->council_name	= STRALLOC( "" );
            victim->pcdata->council		= NULL;
            send_to_char( "Removed from council.\n\rPlease make sure you adjust that council's members accordingly.\n\r", ch );
            return;
        }

        council = get_council( arg3 );
        if ( !council )
        {
            send_to_char( "No such council.\n\r", ch );
            return;
        }
        STRFREE( victim->pcdata->council_name );
        victim->pcdata->council_name = QUICKLINK( council->name );
        victim->pcdata->council = council;
        send_to_char( "Done.\n\rPlease make sure you adjust that council's members accordingly.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "short" ) )
    {
        STRFREE( victim->short_descr );
        victim->short_descr = STRALLOC( arg3 );
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
        {
            STRFREE( victim->pIndexData->short_descr );
            victim->pIndexData->short_descr = QUICKLINK( victim->short_descr );
        }
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "long" ) )
    {
        STRFREE( victim->long_descr );
        strcpy( buf, arg3 );
        strcat( buf, "\n\r" );
        victim->long_descr = STRALLOC( buf );
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
        {
            STRFREE( victim->pIndexData->long_descr );
            victim->pIndexData->long_descr = QUICKLINK( victim->long_descr );
        }
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "intro" ) )
    {
        DISPOSE( victim->intro_descr );
        strcpy( buf, arg3 );
        victim->intro_descr = str_dup( buf );
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "rank" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char("Not on NPC's.\n\r", ch);
            return;
        }
        DISPOSE( victim->pcdata->rank );
        strcpy( buf, arg3 );
        victim->pcdata->rank = str_dup( buf );
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_prefix( arg2, "description" ) )
    {
        if ( arg3[0] )
        {
            char tbuf[MAX_INPUT_LENGTH], *fbuf;

            fbuf = Justify(arg3, 75, justify_left);
            snprintf(tbuf, sizeof(tbuf), "%s\n\r", fbuf);

            STRFREE( victim->description );
            victim->description = STRALLOC( tbuf );

            if ( IS_NPC( victim ) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            {
                STRFREE(victim->pIndexData->description );
                victim->pIndexData->description = QUICKLINK( victim->description );
            }
            return;
        }
        CHECK_SUBRESTRICTED( ch );
        if ( ch->substate == SUB_REPEATCMD )
            ch->tempnum = SUB_REPEATCMD;
        else
            ch->tempnum = SUB_NONE;
        ch->substate = SUB_MOB_DESC;
        ch->dest_buf = victim;
        start_editing( ch, victim->description );
        return;
    }

    if ( !str_cmp( arg2, "title" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        set_title( victim, arg3 );
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "spec" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( !IS_NPC(victim) )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg3, "none" ) )
        {
            victim->spec_fun = NULL;
            send_to_char( "Special function removed.\n\r", ch );
            if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
                victim->pIndexData->spec_fun = victim->spec_fun;
            return;
        }

        if ( ( victim->spec_fun = m_spec_lookup( arg3 ) ) == NULL )
        {
            send_to_char( "No such spec fun.\n\r", ch );
            return;
        }
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->spec_fun = victim->spec_fun;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "flags" ) )
    {
        bool pcflag;
        if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_GREATER )
        {
            send_to_char( "You can only modify a mobile's flags.\n\r", ch );
            return;
        }

        if ( !can_mmodify( ch, victim ) )
            return;
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: mset <victim> flags <flag> [flag]...\n\r", ch );
            for ( value = 0; value < 32; value++ )
                ch_printf(ch, "%s ", act_flags[value]);
            send_to_char("\n\r", ch);
            return;
        }
        while ( argument[0] != '\0' )
        {
            pcflag = FALSE;
            argument = one_argument( argument, arg3 );
            value = IS_NPC( victim) ? get_actflag( arg3 ) : get_plrflag( arg3 );

            if ( !IS_NPC( victim ) && ( value < 0 || value > 31 ) )
            {
                pcflag = TRUE;
                value = get_pcflag( arg3 );
            }
            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
            {
                if ( IS_NPC(victim)
                     &&   1 << value == ACT_PROTOTYPE
                     &&   get_trust( ch ) < LEVEL_GREATER
                     &&  !is_name ("protoflag", ch->pcdata->bestowments) )
                    send_to_char( "You cannot change the prototype flag.\n\r", ch );
                else
                    if ( IS_NPC(victim) && 1 << value == ACT_IS_NPC )
                        send_to_char( "If that could be changed, it would cause many problems.\n\r", ch );
                    else
                        if ( IS_NPC(victim) && 1 << value == ACT_POLYMORPHED )
                            send_to_char( "Changing that would be a _bad_ thing.\n\r", ch);
                        else
                        {
                            if ( pcflag )
                                TOGGLE_BIT( victim->pcdata->flags, 1 << value );
                            else
                            {
                                TOGGLE_BIT( victim->act, 1 << value );
                                /* NPC check added by Gorog */
                                if ( IS_NPC(victim) && (1 << value == ACT_PROTOTYPE) )
                                    victim->pIndexData->act = victim->act;
                            }
                        }
            }
        }
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->act = victim->act;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "flags2" ) )
    {
        if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_GREATER )
        {
            send_to_char( "You can only modify a mobile's flags.\n\r", ch );
            return;
        }

        if ( !can_mmodify( ch, victim ) )
            return;
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: mset <victim> flags2 <flag> [flag]...\n\r", ch );
            for ( value = 0; value < 32; value++ )
                ch_printf(ch, "%s ", act2_flags[value]);
            send_to_char("\n\r", ch);
            return;
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = IS_NPC( victim) ? get_act2flag( arg3 ) : get_plr2flag( arg3 );

            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
                TOGGLE_BIT( victim->act2, 1 << value );
        }
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->act2 = victim->act2;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "affected" ) )
    {
        if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
        {
            send_to_char( "You can only modify a mobile's flags.\n\r", ch );
            return;
        }

        if ( !can_mmodify( ch, victim ) )
            return;
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: mset <victim> affected <flag> [flag]...\n\r", ch );
            return;
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_aflag( arg3 );
            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
                TOGGLE_BIT( victim->affected_by, 1 << value );
        }
        if ( IS_NPC( victim ) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->affected_by = victim->affected_by;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "affected2" ) )
    {
        if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
        {
            send_to_char( "You can only modify a mobile's flags.\n\r", ch );
            return;
        }

        if ( !can_mmodify( ch, victim ) )
            return;
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: mset <victim> affected2 <flag> [flag]...\n\r", ch );
            return;
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_a2flag( arg3 );
            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
                TOGGLE_BIT( victim->affected_by2, 1 << value );
        }
        if ( IS_NPC( victim ) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->affected_by2 = victim->affected_by2;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    /*
     * save some more finger-leather for setting RIS stuff
     */
    if ( !str_cmp( arg2, "r" ) )
    {
        if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
        {
            send_to_char( "You can only modify a mobile's ris.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;

        sprintf(outbuf,"%s resistant %s",arg1, arg3);
        do_mset( ch, outbuf );
        return;
    }
    if ( !str_cmp( arg2, "i" ) )
    {
        if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
        {
            send_to_char( "You can only modify a mobile's ris.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;


        sprintf(outbuf,"%s immune %s",arg1, arg3);
        do_mset( ch, outbuf );
        return;
    }
    if ( !str_cmp( arg2, "s" ) )
    {
        if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
        {
            send_to_char( "You can only modify a mobile's ris.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;

        sprintf(outbuf,"%s susceptible %s",arg1, arg3);
        do_mset( ch, outbuf );
        return;
    }
    if ( !str_cmp( arg2, "ri" ) )
    {
        if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
        {
            send_to_char( "You can only modify a mobile's ris.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;

        sprintf(outbuf,"%s resistant %s",arg1, arg3);
        do_mset( ch, outbuf );
        sprintf(outbuf,"%s immune %s",arg1, arg3);
        do_mset( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "rs" ) )
    {
        if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
        {
            send_to_char( "You can only modify a mobile's ris.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;

        sprintf(outbuf,"%s resistant %s",arg1, arg3);
        do_mset( ch, outbuf );
        sprintf(outbuf,"%s susceptible %s",arg1, arg3);
        do_mset( ch, outbuf );
        return;
    }
    if ( !str_cmp( arg2, "is" ) )
    {
        if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
        {
            send_to_char( "You can only modify a mobile's ris.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;

        sprintf(outbuf,"%s immune %s",arg1, arg3);
        do_mset( ch, outbuf );
        sprintf(outbuf,"%s susceptible %s",arg1, arg3);
        do_mset( ch, outbuf );
        return;
    }
    if ( !str_cmp( arg2, "ris" ) )
    {
        if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
        {
            send_to_char( "You can only modify a mobile's ris.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;

        sprintf(outbuf,"%s resistant %s",arg1, arg3);
        do_mset( ch, outbuf );
        sprintf(outbuf,"%s immune %s",arg1, arg3);
        do_mset( ch, outbuf );
        sprintf(outbuf,"%s susceptible %s",arg1, arg3);
        do_mset( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "resistant" ) )
    {
        if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
        {
            send_to_char( "You can only modify a mobile's resistancies.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: mset <victim> resistant <flag> [flag]...\n\r", ch );
            return;
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_risflag( arg3 );
            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
                TOGGLE_BIT( victim->resistant, 1 << value );
        }
        if ( IS_NPC( victim ) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->resistant = victim->resistant;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "immune" ) )
    {
        if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
        {
            send_to_char( "You can only modify a mobile's immunities.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: mset <victim> immune <flag> [flag]...\n\r", ch );
            return;
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_risflag( arg3 );
            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
                TOGGLE_BIT( victim->immune, 1 << value );
        }
        if ( IS_NPC( victim ) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->immune = victim->immune;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "susceptible" ) )
    {
        if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
        {
            send_to_char( "You can only modify a mobile's susceptibilities.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: mset <victim> susceptible <flag> [flag]...\n\r", ch );
            return;
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_risflag( arg3 );
            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
                TOGGLE_BIT( victim->susceptible, 1 << value );
        }
        if ( IS_NPC( victim ) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->susceptible = victim->susceptible;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "absorb" ) )
    {
        if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
        {
            send_to_char( "You can only modify a mobile's absorbs.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: mset <victim> absorb <flag> [flag]...\n\r", ch );
            return;
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_risflag( arg3 );
            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
                TOGGLE_BIT( victim->absorb, 1 << value );
        }
        if ( IS_NPC( victim ) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->absorb = victim->absorb;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "part" ) )
    {
        if ( !IS_NPC( victim ) && get_trust( ch ) < LEVEL_LESSER )
        {
            send_to_char( "You can only modify a mobile's parts.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: mset <victim> part <flag> [flag]...\n\r", ch );
            return;
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_partflag( arg3 );
            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
                TOGGLE_BIT( victim->xflags, 1 << value );
        }
        if ( IS_NPC( victim ) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->xflags = victim->xflags;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "attack" ) )
    {
        if ( !IS_NPC( victim ) )
        {
            send_to_char( "You can only modify a mobile's attacks.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: mset <victim> attack <flag> [flag]...\n\r", ch );
            return;
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_attackflag( arg3 );
            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
                TOGGLE_BIT( victim->attacks, 1 << value );
        }
        if ( IS_NPC( victim ) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->attacks = victim->attacks;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "defense" ) )
    {
        if ( !IS_NPC( victim ) )
        {
            send_to_char( "You can only modify a mobile's defenses.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: mset <victim> defense <flag> [flag]...\n\r", ch );
            return;
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_defenseflag( arg3 );
            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
                TOGGLE_BIT( victim->defenses, 1 << value );
        }
        if ( IS_NPC( victim ) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->defenses = victim->defenses;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "pos" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: mset <victim> pos <pos>\n\r", ch );
            return;
        }
        value = get_postype( argument );
        if ( value < 0 || value >= MAX_POSITION )
        {
            ch_printf( ch, "Invalid position: %s\n\r", arg3 );
            return;
        }
        victim->position = value;
        if ( IS_NPC( victim ) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->position = victim->position;
        send_to_char( "Done.\n\r", ch );
        send_to_char("Ok.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "defpos" ) )
    {
        if ( !IS_NPC(victim) )
        {
            send_to_char( "Mobiles only.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: mset <victim> pos <pos>\n\r", ch );
            return;
        }
        value = get_postype( argument );
        if ( value < 0 || value >= MAX_POSITION )
        {
            ch_printf( ch, "Invalid position: %s\n\r", arg3 );
            return;
        }
        victim->defposition = value;
        if ( IS_NPC( victim ) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->defposition = victim->defposition;
        send_to_char( "Done.\n\r", ch );
        send_to_char("Ok.\n\r", ch);
        return;
    }

    /*
     * save some finger-leather
     */
    if ( !str_cmp( arg2, "hitdie" ) )
    {
        if ( !IS_NPC(victim) )
        {
            send_to_char( "Mobiles only.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;

        sscanf(arg3,"%d %c %d %c %d",&num,&char1,&size,&char2,&plus);
        sprintf(outbuf,"%s hitnumdie %d",arg1, num);
        do_mset( ch, outbuf );

        sprintf(outbuf,"%s hitsizedie %d",arg1, size);
        do_mset( ch, outbuf );

        sprintf(outbuf,"%s hitplus %d",arg1, plus);
        do_mset( ch, outbuf );
        return;
    }
    /*
     * save some more finger-leather
     */
    if ( !str_cmp( arg2, "damdie" ) )
    {
        if ( !IS_NPC(victim) )
        {
            send_to_char( "Mobiles only.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;

        sscanf(arg3,"%d %c %d %c %d",&num,&char1,&size,&char2,&plus);
        sprintf(outbuf,"%s damnumdie %d",arg1, num);
        do_mset( ch, outbuf );
        sprintf(outbuf,"%s damsizedie %d",arg1, size);
        do_mset( ch, outbuf );
        sprintf(outbuf,"%s damplus %d",arg1, plus);
        do_mset( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "hitnumdie" ) )
    {
        if ( !IS_NPC(victim) )
        {
            send_to_char( "Mobiles only.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < 0 || value > 32767 )
        {
            send_to_char( "Number of hitpoint dice range is 0 to 30000.\n\r", ch );
            return;
        }
        if ( IS_NPC( victim ) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->hitnodice = value;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "hitsizedie" ) )
    {
        if ( !IS_NPC(victim) )
        {
            send_to_char( "Mobiles only.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < 0 || value > 32767 )
        {
            send_to_char( "Hitpoint dice size range is 0 to 30000.\n\r", ch );
            return;
        }
        if ( IS_NPC( victim ) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->hitsizedice = value;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "hitplus" ) )
    {
        if ( !IS_NPC(victim) )
        {
            send_to_char( "Mobiles only.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < 0 || value > 32767 )
        {
            send_to_char( "Hitpoint bonus range is 0 to 30000.\n\r", ch );
            return;
        }
        if ( IS_NPC( victim ) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->hitplus = value;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "damnumdie" ) )
    {
        if ( !IS_NPC(victim) )
        {
            send_to_char( "Mobiles only.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < 0 || value > 100 )
        {
            send_to_char( "Number of damage dice range is 0 to 100.\n\r", ch );
            return;
        }
        if ( IS_NPC( victim ) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->damnodice = value;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "damsizedie" ) )
    {
        if ( !IS_NPC(victim) )
        {
            send_to_char( "Mobiles only.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( value < 0 || value > 100 )
        {
            send_to_char( "Damage dice size range is 0 to 100.\n\r", ch );
            return;
        }
        if ( IS_NPC( victim ) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->damsizedice = value;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "damplus" ) )
    {
        if ( !IS_NPC(victim) )
        {
            send_to_char( "Mobiles only.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;

	send_to_char("damplus has been deprecated, use damroll instead.\n\r", ch);

        if ( value < 0 || value > 1000 )
        {
            send_to_char( "Damage bonus range is 0 to 1000.\n\r", ch );
            return;
        }

        if ( IS_NPC( victim ) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->damplus = value;
        send_to_char( "Done.\n\r", ch );
        return;

    }


    if ( !str_cmp( arg2, "aloaded" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Player Characters only.\n\r", ch );
            return;
        }


        if ( !can_mmodify( ch, victim ) )
            return;

        if ( !IS_AREA_STATUS(victim->pcdata->area, AREA_LOADED ) )
        {
            SET_AREA_STATUS( victim->pcdata->area, AREA_LOADED );
            send_to_char( "Your area set to LOADED!\n\r", victim );
            if ( ch != victim )
                send_to_char( "Area set to LOADED!\n\r", ch );
            return;
        }
        else
        {
            REMOVE_AREA_STATUS( victim->pcdata->area, AREA_LOADED );
            send_to_char( "Your area set to NOT-LOADED!\n\r", victim );
            if ( ch != victim )
                send_to_char( "Area set to NON-LOADED!\n\r", ch );
            return;
        }
    }

    if ( !str_cmp( arg2, "pkill" ) )
    {
        if(  IS_NPC(victim) )
        {
            send_to_char( "Player Characters only.\n\r", ch );
            return;
        }

        if ( !can_mmodify( ch, victim ) )
        {
            send_to_char( "You can't do that.\n\r", ch );
            return;
        }

        if ( IS_PC_FLAG( victim, PCFLAG_DEADLY ) )
        {
            REMOVE_PC_FLAG( victim, PCFLAG_DEADLY );
            send_to_char( "You are now a NON-PKILL player.\n\r", victim );
            if ( ch != victim )
                send_to_char( "That player is now non-pkill.\n\r", ch );
            return;
        }
        else
        {
            SET_PC_FLAG( victim, PCFLAG_DEADLY );
            send_to_char( "You are now a PKILL player.\n\r", victim );
            if ( ch != victim )
                send_to_char( "That player is now pkill.\n\r", ch );
            return;
        }
    }

    if ( !str_cmp( arg2, "speaks" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: mset <victim> speaks <language> [language] ...\n\r", ch );
            return;
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_langflag( arg3 );
            if ( value == LANG_UNKNOWN )
                ch_printf( ch, "Unknown language: %s\n\r", arg3 );
            else
                if ( !IS_NPC( victim ) )
                {
                    if ( !(value &= VALID_LANGS) )
                    {
                        ch_printf( ch, "Players may not know %s.\n\r", arg3 );
                        continue;
                    }
                }
            v2 = get_langnum( arg3);
            if ( v2 == -1 )
                ch_printf( ch, "Unknown language: %s\n\r", arg3 );
            else
                TOGGLE_BIT( victim->speaks, 1 << v2 );
        }
        if ( !IS_NPC( victim ) )
        {
            REMOVE_BIT( victim->speaks, race_table[victim->race].language );
            if ( !knows_language( victim, victim->speaking, victim ) )
                victim->speaking = race_table[victim->race].language;
        }
        else
            if ( IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
                victim->pIndexData->speaks = victim->speaks;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "speaking" ) )
    {
        if ( !IS_NPC( victim ) )
        {
            send_to_char( "Players must choose the language they speak themselves.\n\r", ch );
            return;
        }
        if ( !can_mmodify( ch, victim ) )
            return;
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: mset <victim> speaking <language> [language]...\n\r", ch );
            return;
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_langflag( arg3 );
            if ( value == LANG_UNKNOWN )
                ch_printf( ch, "Unknown language: %s\n\r", arg3 );
	    else {
	       v2 = get_langnum( arg3);
               if ( v2 == -1 )
                   ch_printf( ch, "Unknown language: %s\n\r", arg3 );
               else
                   TOGGLE_BIT( victim->speaking, 1 << v2 );
            }
        }
        if ( IS_NPC(victim) && IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
            victim->pIndexData->speaking = victim->speaking;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "timer" ) )
    {
        if ( !can_mmodify( ch, victim ) )
            return;

        victim->timer = value;
        send_to_char("Done.\n\r", ch);
        return;
    }


    /*
     * Generate usage message.
     */
    if ( ch->substate == SUB_REPEATCMD )
    {
        ch->substate = SUB_RESTRICTED;
        interpret( ch, origarg );
        ch->substate = SUB_REPEATCMD;
        ch->last_cmd = do_mset;
    }
    else
        do_mset( ch, "" );
    return;
}


void do_oset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf  [MAX_STRING_LENGTH];
    char outbuf  [MAX_STRING_LENGTH];
    OBJ_DATA *obj, *tmpobj;
    EXTRA_DESCR_DATA *ed;
    bool lockobj;
    char *origarg = argument;

    int value, tmp;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mob's can't oset\n\r", ch );
        return;
    }

    if ( !ch->desc )
    {
        send_to_char( "You have no descriptor\n\r", ch );
        return;
    }

    switch( ch->substate )
    {
    default:
        break;

    case SUB_OBJ_EXTRA:
        if ( !ch->dest_buf )
        {
            send_to_char( "Fatal error: report to Thoric.\n\r", ch );
            bug( "do_oset: sub_obj_extra: NULL ch->dest_buf" );
            ch->substate = SUB_NONE;
            return;
        }
        /*
         * hopefully the object didn't get extracted...
         * if you're REALLY paranoid, you could always go through
         * the object and index-object lists, searching through the
         * extra_descr lists for a matching pointer...
         */
        ed  = (EXTRA_DESCR_DATA *)ch->dest_buf;
        STRFREE( ed->description );
        ed->description = copy_buffer( ch );
        tmpobj = (OBJ_DATA *)ch->spare_ptr;
        stop_editing( ch );
        ch->dest_buf = tmpobj;
        ch->substate = ch->tempnum;
        return;

    case SUB_OBJ_LONG:
        if ( !ch->dest_buf )
        {
            send_to_char( "Fatal error: report to Thoric.\n\r", ch );
            bug( "do_oset: sub_obj_long: NULL ch->dest_buf" );
            ch->substate = SUB_NONE;
            return;
        }
        obj = (OBJ_DATA *)ch->dest_buf;
        if ( obj && obj_extracted(obj) )
        {
            send_to_char( "Your object was extracted!\n\r", ch );
            stop_editing( ch );
            return;
        }
        STRFREE( obj->description );
        obj->description = copy_buffer( ch );
		if ( can_omodify(ch, obj) )
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
        {
            if ( can_omodify(ch, obj) )
            {
                STRFREE( obj->pIndexData->description );
                obj->pIndexData->description = QUICKLINK( obj->description );
            }
        }
        tmpobj = (OBJ_DATA *)ch->spare_ptr;
        stop_editing( ch );
        ch->substate = ch->tempnum;
        ch->dest_buf = tmpobj;
        return;
    }

    obj = NULL;
    smash_tilde( argument );

    if ( ch->substate == SUB_REPEATCMD )
    {
        obj = (OBJ_DATA *)ch->dest_buf;
        if ( obj && obj_extracted(obj) )
        {
            send_to_char( "Your object was extracted!\n\r", ch );
            obj = NULL;
            argument = "done";
        }
        if ( argument[0] == '\0' || !str_cmp( argument, " " )
             ||   !str_cmp( argument, "stat" ) )
        {
            if ( obj )
                do_ostat( ch, obj->name );
            else
                send_to_char( "No object selected.  Type '?' for help.\n\r", ch );
            return;
        }
        if ( !str_cmp( argument, "done" ) || !str_cmp( argument, "off" ) )
        {
            send_to_char( "Oset mode off.\n\r", ch );
            ch->substate = SUB_NONE;
            ch->dest_buf = NULL;
            if ( ch->pcdata && ch->pcdata->subprompt )
                STRFREE( ch->pcdata->subprompt );
            return;
        }
    }
    if ( obj )
    {
        lockobj = TRUE;
        strcpy( arg1, obj->name );
        argument = one_argument( argument, arg2 );
        strcpy( arg3, argument );
    }
    else
    {
        lockobj = FALSE;
        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );
        strcpy( arg3, argument );
    }

    if ( !str_cmp( arg1, "on" ) )
    {
        send_to_char( "Syntax: oset <object|vnum> on.\n\r", ch );
        return;
    }

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !str_cmp( arg1, "?" ) )
    {
        if ( ch->substate == SUB_REPEATCMD )
        {
            if ( obj )
                send_to_char( "Syntax: <field>  <value>\n\r",		ch );
            else
                send_to_char( "Syntax: <object> <field>  <value>\n\r",	ch );
        }
        else
            send_to_char( "Syntax: oset <object> <field>  <value>\n\r",	ch );
        send_to_char( "\n\r",						ch );
        send_to_char( "Field being one of:\n\r",			ch );
        send_to_char( "  flags flags2 wear weight cost rent timer\n\r",	ch );
        send_to_char( "  name short long ed rmed actiondesc\n\r",	ch );
        send_to_char( "  type value0 value1 value2 value3 value4 value5\n\r",	ch );
        send_to_char( "  affect rmaffect layers spec currtype\n\r",   	ch );
        send_to_char( "For weapons:             For armor:\n\r",	ch );
        send_to_char( "  weapontype condition     ac condition\n\r",	ch );
        send_to_char( "For scrolls, potions and pills:\n\r",		ch );
        send_to_char( "  slevel spell1 spell2 spell3\n\r",		ch );
        send_to_char( "For wands and staves:\n\r",			ch );
        send_to_char( "  slevel spell maxcharges charges\n\r",		ch );
        send_to_char( "For containers:          For levers and switches:\n\r", ch );
        send_to_char( "  cflags key capacity      tflags\n\r",		ch );
        return;
    }

    if ( !obj && get_trust(ch) < LEVEL_GOD )
    {
        if ( ( obj = get_obj_here( ch, arg1 ) ) == NULL )
        {
            send_to_char( "You can't find that here.\n\r", ch );
            return;
        }
    }
    else
        if ( !obj )
        {
            if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
            {
                send_to_char( "There is nothing like that in all the realms.\n\r", ch );
                return;
            }
        }
    if ( lockobj )
        ch->dest_buf = obj;
    else
        ch->dest_buf = NULL;

    separate_obj( obj );
    value = atoi( arg3 );

    if ( !str_cmp( arg2, "on" ) )
    {
        ch_printf( ch, "Oset mode on. (Editing '%s' vnum %d).\n\r",
                   obj->name, obj->vnum );
        ch->substate = SUB_REPEATCMD;
        ch->dest_buf = obj;
        if ( ch->pcdata )
        {
            if ( ch->pcdata->subprompt )
                STRFREE( ch->pcdata->subprompt );
            sprintf( buf, "<&COset &W#%d&w> %%i", obj->vnum );
            ch->pcdata->subprompt = STRALLOC( buf );
        }
        return;
    }

    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;
        obj->value[0] = value;
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->value[0] = value;
        return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;
        obj->value[1] = value;
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->value[1] = value;
        return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;
        obj->value[2] = value;
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
        {
            obj->pIndexData->value[2] = value;
            /*	  if ( obj->item_type == ITEM_WEAPON && value != 0 )
             obj->value[2] = obj->pIndexData->value[1] * obj->pIndexData->value[2];*/
        }
        return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;
        obj->value[3] = value;
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->value[3] = value;
        return;
    }

    if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;
        obj->value[4] = value;
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->value[4] = value;
        return;
    }

    if ( !str_cmp( arg2, "value5" ) || !str_cmp( arg2, "v5" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;
        obj->value[5] = value;
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->value[5] = value;
        return;
    }

    if ( !str_cmp( arg2, "type" ) )
    {
        unsigned int val;

        if ( !can_omodify( ch, obj ) )
            return;
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: oset <object> type <type>\n\r", ch );
            for ( val = 0; val < (sizeof(o_types) / sizeof(o_types[0])); val++ )
                ch_printf(ch, "%s ", o_types[val]);
            send_to_char("\n\r", ch);
            return;
        }
        val = get_otype( argument );
        if ( val < 1 || val == ITEM_PORTAL)
        {
            ch_printf( ch, "Invalid type: %s\n\r", arg3 );
            return;
        }
        obj->item_type = (sh_int) val;
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->item_type = obj->item_type;
        return;
    }

    if ( !str_cmp( arg2, "pipeflag" ) || !str_cmp( arg2, "pf" ) )
    {
        if ( obj->item_type != ITEM_PIPE )
        {
            send_to_char("That is not a pipe.\n\r", ch);
            return;
        }
        if ( !can_omodify( ch, obj ) )
            return;
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: oset <object> pipeflag <flag> [flag]...\n\r", ch );
            for ( value = 0; value < 32; value++ )
                ch_printf(ch, "%s ", pipe_flags[value]);
            send_to_char("\n\r", ch);
            return;
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_pipeflag( arg3 );
            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
                TOGGLE_BIT(obj->value[3], 1 << value);
        }
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->value[3] = obj->value[3];
        return;
    }

    if ( !str_cmp( arg2, "spec" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;

        if ( !str_cmp( arg3, "none" ) )
        {
            obj->spec_fun = NULL;
            send_to_char( "Special function removed.\n\r", ch );
            if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
                obj->pIndexData->spec_fun = obj->spec_fun;
            return;
        }

        if ( ( obj->spec_fun = o_spec_lookup( arg3 ) ) == NULL )
        {
            send_to_char( "No such spec fun.\n\r", ch );
            return;
        }
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->spec_fun = obj->spec_fun;
        return;
    }

    if ( !str_cmp( arg2, "flags" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: oset <object> flags <flag> [flag]...\n\r", ch );
            for ( value = 0; value < 32; value++ )
                ch_printf(ch, "%s ", o_flags[value]);
            send_to_char("\n\r", ch);
            return;
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_oflag( arg3 );
            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
            {
                if ( 1 << value == ITEM_PROTOTYPE
                     &&   get_trust( ch ) < sysdata.level_modify_proto
                     &&   !is_name ("protoflag", ch->pcdata->bestowments) )
                    send_to_char( "You cannot change the prototype flag.\n\r", ch );
                else
                {
                    TOGGLE_BIT(obj->extra_flags, 1 << value);
                    if ( 1 << value == ITEM_PROTOTYPE )
                        obj->pIndexData->extra_flags = obj->extra_flags;
                }
            }
        }
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->extra_flags = obj->extra_flags;
        return;
    }

    if ( !str_cmp( arg2, "flags2" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: oset <object> flags2 <flag> [flag]...\n\r", ch );
            for ( value = 0; value < 32; value++ )
                ch_printf(ch, "%s ", o2_flags[value]);
            send_to_char("\n\r", ch);
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_o2flag( arg3 );
            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag2: %s\n\r", arg3 );
            else
                TOGGLE_BIT(obj->extra_flags2, 1 << value);
        }
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->extra_flags2 = obj->extra_flags2;
        return;
    }

    if ( !str_cmp( arg2, "wear" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: oset <object> wear <flag> [flag]...\n\r", ch );
            for ( value = 0; value < 32; value++ )
                ch_printf(ch, "%s ", w_flags[value]);
            send_to_char("\n\r", ch);
            return;
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_wflag( arg3 );
            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
                TOGGLE_BIT( obj->wear_flags, 1 << value );
        }

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->wear_flags = obj->wear_flags;
        return;
    }

    if ( !str_cmp( arg2, "weight" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;
        obj->weight = value;
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->weight = value;
        return;
    }

    if ( !str_cmp( arg2, "cost" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;
        obj->cost = value;
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->cost = value;
        return;
    }

    if ( !str_cmp( arg2, "currtype" ) )
    {
        if ( !can_omodify( ch, obj ) )
          return;
        obj->currtype = get_currency_type(argument);
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
          obj->pIndexData->currtype = get_currency_type(argument);
        return;
    }

    if ( !str_cmp( arg2, "rent" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->rent = value;
        else
            send_to_char( "Item must have prototype flag to set this value.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "layers" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->layers = value;
        else
            send_to_char( "Item must have prototype flag to set this value.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "timer" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;
        obj->timer = value;
        return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
        if ( !can_omodify( ch, obj ) )
            return;
        STRFREE( obj->name );
        obj->name = STRALLOC( arg3 );
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
        {
            STRFREE(obj->pIndexData->name );
            obj->pIndexData->name = QUICKLINK( obj->name );
        }
        return;
    }

    if ( !str_cmp( arg2, "short" ) )
    {
        STRFREE( obj->short_descr );
        obj->short_descr = STRALLOC( arg3 );
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
        {
            STRFREE(obj->pIndexData->short_descr );
            obj->pIndexData->short_descr = QUICKLINK( obj->short_descr );
        }
        else
            /* Feature added by Narn, Apr/96
             * If the item is not proto, add the word 'rename' to the keywords
             * if it is not already there.
             */
        {
            if ( str_infix( "rename", obj->name ) )
            {
                sprintf( buf, "%s %s", obj->name, "rename" );
                STRFREE( obj->name );
                obj->name = STRALLOC( buf );
            }
        }
        return;
    }

    if ( !str_cmp( arg2, "actiondesc" ) )
    {
        if ( strstr( arg3, "%n" )
             ||   strstr( arg3, "%d" )
             ||   strstr( arg3, "%l" ) )
        {
            send_to_char( "Illegal characters!\n\r", ch );
            return;
        }
        STRFREE( obj->action_desc );
        obj->action_desc = STRALLOC( arg3 );
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
        {
            STRFREE(obj->pIndexData->action_desc );
            obj->pIndexData->action_desc = QUICKLINK( obj->action_desc );
        }
        return;
    }

    if ( !str_cmp( arg2, "long" ) )
    {
        if ( arg3[0] )
        {
            STRFREE( obj->description );
            obj->description = STRALLOC( arg3 );
            if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            {
                STRFREE(obj->pIndexData->description );
                obj->pIndexData->description = QUICKLINK( obj->description );
            }
            return;
        }
        CHECK_SUBRESTRICTED( ch );
        if ( ch->substate == SUB_REPEATCMD )
            ch->tempnum = SUB_REPEATCMD;
        else
            ch->tempnum = SUB_NONE;
        if ( lockobj )
            ch->spare_ptr = obj;
        else
            ch->spare_ptr = NULL;
        ch->substate = SUB_OBJ_LONG;
        ch->dest_buf = obj;
        start_editing( ch, obj->description );
        return;
    }

    if ( !str_cmp( arg2, "affect" ) )
    {
        AFFECT_DATA *paf;
        sh_int loc;
        int bitv;

        argument = one_argument( argument, arg2 );
        if ( !arg2 || arg2[0] == '\0' || !argument || argument[0] == 0 )
        {
            send_to_char( "Usage: oset <object> affect <field> <value>\n\r", ch );
            for ( value = 0; value < MAX_APPLY_TYPE; value++ )
                ch_printf(ch, "%s ", a_types[value]);
            send_to_char("\n\r", ch);
            return;
        }
        loc = get_atype( arg2 );
        if ( loc < 1 )
        {
            ch_printf( ch, "Unknown field: %s\n\r", arg2 );
            return;
        }
        if ( loc >= APPLY_AFFECT && loc < APPLY_WEAPONSPELL )
        {
            bitv = 0;
            while ( argument[0] != '\0' )
            {
                argument = one_argument( argument, arg3 );
                if ( loc == APPLY_AFFECT )
                    value = get_aflag( arg3 );
                else
                    value = get_risflag( arg3 );
                if ( value < 0 || value > 31 )
                    ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
                else
                    SET_BIT( bitv, 1 << value );
            }
            if ( !bitv )
                return;
            value = bitv;
        }
        else
        {
            argument = one_argument( argument, arg3 );
            value = atoi( arg3 );
        }
        CREATE( paf, AFFECT_DATA, 1 );
        paf->type		= -1;
        paf->duration		= -1;
        paf->location		= loc;
        paf->modifier		= value;
        paf->bitvector		= 0;
        paf->next		= NULL;
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            LINK( paf, obj->pIndexData->first_affect,
                  obj->pIndexData->last_affect, next, prev );
        else
            LINK( paf, obj->first_affect, obj->last_affect, next, prev );
        ++top_affect;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "rmaffect" ) )
    {
        AFFECT_DATA *paf;
        sh_int loc, count;

        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: oset <object> rmaffect <affect#>\n\r", ch );
            return;
        }
        loc = atoi( argument );
        if ( loc < 1 )
        {
            send_to_char( "Invalid number.\n\r", ch );
            return;
        }

        count = 0;

        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
        {
            OBJ_INDEX_DATA *pObjIndex;

            pObjIndex = obj->pIndexData;
            for ( paf = pObjIndex->first_affect; paf; paf = paf->next )
            {
                if ( ++count == loc )
                {
                    UNLINK( paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
                    DISPOSE( paf );
                    send_to_char( "Removed.\n\r", ch );
                    --top_affect;
                    return;
                }
            }
            send_to_char( "Not found.\n\r", ch );
            return;
        }
        else
        {
            for ( paf = obj->first_affect; paf; paf = paf->next )
            {
                if ( ++count == loc )
                {
                    UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
                    DISPOSE( paf );
                    send_to_char( "Removed.\n\r", ch );
                    --top_affect;
                    return;
                }
            }
            send_to_char( "Not found.\n\r", ch );
            return;
        }
    }

    if ( !str_cmp( arg2, "ed" ) )
    {
        if ( !arg3 || arg3[0] == '\0' )
        {
            send_to_char( "Syntax: oset <object> ed <keywords>\n\r",
                          ch );
            return;
        }
        CHECK_SUBRESTRICTED( ch );
        if ( obj->timer )
        {
            send_to_char("It's not safe to edit an extra description on an object with a timer.\n\rTurn it off first.\n\r", ch );
            return;
        }
        if ( obj->item_type == ITEM_PAPER )
        {
            send_to_char("You can not add an extra description to a note paper at the moment.\n\r", ch);
            return;
        }
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            ed = SetOExtraProto( obj->pIndexData, arg3 );
        else
            ed = SetOExtra( obj, arg3 );
        if ( ch->substate == SUB_REPEATCMD )
            ch->tempnum = SUB_REPEATCMD;
        else
            ch->tempnum = SUB_NONE;
        if ( lockobj )
            ch->spare_ptr = obj;
        else
            ch->spare_ptr = NULL;
        ch->substate = SUB_OBJ_EXTRA;
        ch->dest_buf = ed;
        start_editing( ch, ed->description );
        return;
    }

    if ( !str_cmp( arg2, "rmed" ) )
    {
        if ( !arg3 || arg3[0] == '\0' )
        {
            send_to_char( "Syntax: oset <object> rmed <keywords>\n\r", ch );
            return;
        }
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
        {
            if ( DelOExtraProto( obj->pIndexData, arg3 ) )
                send_to_char( "Deleted.\n\r", ch );
            else
                send_to_char( "Not found.\n\r", ch );
            return;
        }
        if ( DelOExtra( obj, arg3 ) )
            send_to_char( "Deleted.\n\r", ch );
        else
            send_to_char( "Not found.\n\r", ch );
        return;
    }
    /*
     * save some finger-leather
     */
    if ( !str_cmp( arg2, "ris" ) )
    {
        sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
        do_oset( ch, outbuf );
        sprintf(outbuf, "%s affect immune %s", arg1, arg3);
        do_oset( ch, outbuf );
        sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
        do_oset( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "r" ) )
    {
        sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
        do_oset( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "i" ) )
    {
        sprintf(outbuf, "%s affect immune %s", arg1, arg3);
        do_oset( ch, outbuf );
        return;
    }
    if ( !str_cmp( arg2, "s" ) )
    {
        sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
        do_oset( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "ri" ) )
    {
        sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
        do_oset( ch, outbuf );
        sprintf(outbuf, "%s affect immune %s", arg1, arg3);
        do_oset( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "rs" ) )
    {
        sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
        do_oset( ch, outbuf );
        sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
        do_oset( ch, outbuf );
        return;
    }

    if ( !str_cmp( arg2, "is" ) )
    {
        sprintf(outbuf, "%s affect immune %s", arg1, arg3);
        do_oset( ch, outbuf );
        sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
        do_oset( ch, outbuf );
        return;
    }

    /*
     * Make it easier to set special object values by name than number
     * 						-Thoric
     */
    tmp = -1;
    switch( obj->item_type )
    {
    case ITEM_WEAPON:
        if ( !str_cmp( arg2, "weapontype" ) )
        {
            unsigned int x;

            value = -1;
            for ( x = 0; x < sizeof( attack_table ) / sizeof( attack_table[0] ); x++ )
                if ( !str_cmp( arg3, attack_table[x] ) )
                    value = x;
            if ( value < 0 )
            {
                send_to_char( "Unknown weapon type.\n\r", ch );
                return;
            }
            tmp = 3;
            break;
        }
        if ( !str_cmp( arg2, "condition" ) )	tmp = 0;
        break;
    case ITEM_ARMOR:
        if ( !str_cmp( arg2, "condition" ) )	tmp = 3;
        if ( !str_cmp( arg2, "ac" )	)		tmp = 1;
        break;
    case ITEM_SALVE:
        if ( !str_cmp( arg2, "slevel"   ) )		tmp = 0;
        if ( !str_cmp( arg2, "maxdoses" ) )		tmp = 1;
        if ( !str_cmp( arg2, "doses"    ) )		tmp = 2;
        if ( !str_cmp( arg2, "delay"    ) )		tmp = 3;
        if ( !str_cmp( arg2, "spell1"   ) )		tmp = 4;
        if ( !str_cmp( arg2, "spell2"   ) )		tmp = 5;
        if ( tmp >=4 && tmp <= 5 )			value = skill_lookup(arg3);
        break;
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
        if ( !str_cmp( arg2, "slevel" ) )		tmp = 0;
        if ( !str_cmp( arg2, "spell1" ) )		tmp = 1;
        if ( !str_cmp( arg2, "spell2" ) )		tmp = 2;
        if ( !str_cmp( arg2, "spell3" ) )		tmp = 3;
        if ( tmp >=1 && tmp <= 3 )			value = skill_lookup(arg3);
        break;
    case ITEM_STAFF:
    case ITEM_WAND:
        if ( !str_cmp( arg2, "slevel" ) )		tmp = 0;
        if ( !str_cmp( arg2, "spell" ) )
        {
            tmp = 3;
            value = skill_lookup(arg3);
        }
        if ( !str_cmp( arg2, "maxcharges" )	)	tmp = 1;
        if ( !str_cmp( arg2, "charges" ) )		tmp = 2;
        break;
    case ITEM_CONTAINER:
        if ( !str_cmp( arg2, "capacity" ) )		tmp = 0;
        if ( !str_cmp( arg2, "cflags" ) )		tmp = 1;
        if ( !str_cmp( arg2, "key" ) )		tmp = 2;
        break;
    case ITEM_SWITCH:
    case ITEM_LEVER:
    case ITEM_PULLCHAIN:
    case ITEM_BUTTON:
        if ( !str_cmp( arg2, "tflags" ) )
        {
            value = get_trigflag(arg3);
            TOGGLE_BIT(obj->value[0], 1 << value);
            if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
                TOGGLE_BIT(obj->pIndexData->value[0], 1 << value);
            return;
        }
        break;
    }
    if ( tmp >= 0 && tmp <= 3 )
    {
        if ( !can_omodify( ch, obj ) )
            return;
        obj->value[tmp] = value;
        if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
            obj->pIndexData->value[tmp] = value;
        return;
    }

    /*
     * Generate usage message.
     */
    if ( ch->substate == SUB_REPEATCMD )
    {
        ch->substate = SUB_RESTRICTED;
        interpret( ch, origarg );
        ch->substate = SUB_REPEATCMD;
        ch->last_cmd = do_oset;
    }
    else
        do_oset( ch, "" );
    return;
}

/*
 * Returns value 0 - 9 based on directional text.
 */
sh_int get_dir( char *txt )
{
    sh_int edir;
    char c1,c2;

    if ( !str_cmp( txt, "northeast" ) )
        return DIR_NORTHEAST;
    if ( !str_cmp( txt, "northwest" ) )
        return DIR_NORTHWEST;
    if ( !str_cmp( txt, "southeast" ) )
        return DIR_SOUTHEAST;
    if ( !str_cmp( txt, "southwest" ) )
        return DIR_SOUTHWEST;
    if ( !str_cmp( txt, "somewhere" ) )
        return DIR_SOMEWHERE;

    c1 = txt[0];
    if ( c1 == '\0' )
        return DIR_NORTH;
    c2 = txt[1];
    edir = DIR_NORTH;
    switch ( c1 )
    {
    case 'n':
        switch ( c2 )
        {
        default:   edir = DIR_NORTH;     break;	/* north */
        case 'e':  edir = DIR_NORTHEAST; break; /* ne	 */
        case 'w':  edir = DIR_NORTHWEST; break; /* nw	 */
        }
	break;
    case '0':  edir = DIR_NORTH;     break; /* north     */
    case 'e':
    case '1':  edir = DIR_EAST;      break; /* east      */
    case 's':
        switch ( c2 )
        {
        default:   edir = DIR_SOUTH;     break; /* south */
        case 'e':  edir = DIR_SOUTHEAST; break; /* se	 */
        case 'w':  edir = DIR_SOUTHWEST; break; /* sw	 */
        }
	break;
    case '2':  edir = DIR_SOUTH;     break; /* south     */
    case 'w':
    case '3':  edir = DIR_WEST;      break; /* west      */
    case 'u':
    case '4':  edir = DIR_UP;        break; /* up	 */
    case 'd':
    case '5':  edir = DIR_DOWN;      break; /* down      */
    case '6':  edir = DIR_NORTHEAST; break; /* ne	 */
    case '7':  edir = DIR_NORTHWEST; break; /* nw	 */
    case '8':  edir = DIR_SOUTHEAST; break; /* se	 */
    case '9':  edir = DIR_SOUTHWEST; break; /* sw	 */
    case '?':  edir = DIR_SOMEWHERE; break; /* somewhere */
    }

    return edir;
}

char *sprint_reset( CHAR_DATA *ch, RESET_DATA *pReset, sh_int num, bool rlist )
{
    static char buf[MAX_STRING_LENGTH];
    char mobname[MAX_STRING_LENGTH];
    char roomname[MAX_STRING_LENGTH];
    char objname[MAX_STRING_LENGTH];
    static ROOM_INDEX_DATA *room;
    static OBJ_INDEX_DATA *obj, *obj2;
    static MOB_INDEX_DATA *mob;
    int rvnum=0;

    if ( ch->in_room )
        rvnum = ch->in_room->vnum;
    if ( num == 1 )
    {
        room = NULL;
        obj  = NULL;
        obj2 = NULL;
        mob  = NULL;
    }

    switch( pReset->command )
    {
    default:
        sprintf( buf, "%2d) *** BAD RESET: %c %d %d %d %d ***\n\r",
                 num,
                 pReset->command,
                 pReset->extra,
                 pReset->arg1,
                 pReset->arg2,
                 pReset->arg3 );
        break;
    case 'M':
        mob = get_mob_index( pReset->arg1 );
        room = get_room_index( pReset->arg3 );
        if ( mob )
            strcpy( mobname, mob->player_name );
        else
            strcpy( mobname, "Mobile: *BAD VNUM*" );
        if ( room )
            strcpy( roomname, room->name );
        else
            strcpy( roomname, "Room: *BAD VNUM*" );
        sprintf( buf, "%2d) %s (%d) -> %s (%d) [%d]\n\r",
                 num,
                 mobname,
                 pReset->arg1,
                 roomname,
                 pReset->arg3,
                 pReset->arg2 );
        break;
    case 'E':
        if ( !mob )
            strcpy( mobname, "* ERROR: NO MOBILE! *" );
        if ( (obj = get_obj_index( pReset->arg1 )) == NULL )
            strcpy( objname, "Object: *BAD VNUM*" );
        else
            strcpy( objname, obj->name );
        sprintf( buf, "%2d) %s (%d) -> %s (%s) [%d]\n\r",
                 num,
                 objname,
                 pReset->arg1,
                 mobname,
                 wear_locs[pReset->arg3],
                 pReset->arg2 );
        break;
    case 'H':
        if ( pReset->arg1 > 0
             &&  (obj = get_obj_index( pReset->arg1 )) == NULL )
            strcpy( objname, "Object: *BAD VNUM*" );
        else
            if ( !obj )
                strcpy( objname, "Object: *NULL obj*" );
        sprintf( buf, "%2d) Hide %s (%d)\n\r",
                 num,
                 objname,
                 obj ? obj->ivnum : pReset->arg1 );
        break;
    case 'G':
        if ( !mob )
            strcpy( mobname, "* ERROR: NO MOBILE! *" );
        if ( (obj = get_obj_index( pReset->arg1 )) == NULL )
            strcpy( objname, "Object: *BAD VNUM*" );
        else
            strcpy( objname, obj->name );
        sprintf( buf, "%2d) %s (%d) -> %s (carry) [%d]\n\r",
                 num,
                 objname,
                 pReset->arg1,
                 mobname,
                 pReset->arg2 );
        break;
    case 'O':
        if ( (obj = get_obj_index( pReset->arg1 )) == NULL )
            strcpy( objname, "Object: *BAD VNUM*" );
        else
            strcpy( objname, obj->name );
        room = get_room_index( pReset->arg3 );
        if ( !room )
            strcpy( roomname, "Room: *BAD VNUM*" );
        else
            strcpy( roomname, room->name );
        sprintf( buf, "%2d) (object) %s (%d) -> %s (%d) [%d]\n\r",
                 num,
                 objname,
                 pReset->arg1,
                 roomname,
                 pReset->arg3,
                 pReset->arg2 );
        break;
    case 'P':
        if ( (obj2 = get_obj_index( pReset->arg1 )) == NULL )
            strcpy( objname, "Object1: *BAD VNUM*" );
        else
            strcpy( objname, obj2->name );
        if ( pReset->arg3 > 0
             &&  (obj = get_obj_index( pReset->arg3 )) == NULL )
            strcpy( roomname, "Object2: *BAD VNUM*" );
        else
            if ( !obj )
                strcpy( roomname, "Object2: *NULL obj*" );
            else
                strcpy( roomname, obj->name );
        sprintf( buf, "%2d) (Put) %s (%d) -> %s (%d) [%d]\n\r",
                 num,
                 objname,
                 pReset->arg1,
                 roomname,
                 obj ? obj->ivnum : pReset->arg3,
                 pReset->arg2 );
        break;
    case 'D':
        if ( pReset->arg2 < 0 || pReset->arg2 >= MAX_REXITS )
            pReset->arg2 = 0;
        if ( (room = get_room_index( pReset->arg1 )) == NULL )
        {
            strcpy( roomname, "Room: *BAD VNUM*" );
            sprintf( objname, "%s (no exit)",
                     dir_name(pReset->arg2) );
        }
        else
        {
            strcpy( roomname, room->name );
            sprintf( objname, "%s%s",
                     dir_name(pReset->arg2),
                     get_exit(room,pReset->arg2) ? "" : " (NO EXIT!)" );
        }
        switch( pReset->arg3 )
        {
        default:	strcpy( mobname, "(* ERROR *)" );	break;
        case 0:	strcpy( mobname, "Open" );		break;
        case 1:	strcpy( mobname, "Close" );		break;
        case 2:	strcpy( mobname, "Close and lock" );	break;
        }
        sprintf( buf, "%2d) %s [%d] the %s [%d] door %s (%d)\n\r",
                 num,
                 mobname,
                 pReset->arg3,
                 objname,
                 pReset->arg2,
                 roomname,
                 pReset->arg1 );
        break;
    case 'R':
        if ( (room = get_room_index( pReset->arg1 )) == NULL )
            strcpy( roomname, "Room: *BAD VNUM*" );
        else
            strcpy( roomname, room->name );
        sprintf( buf, "%2d) Randomize exits 0 to %d -> %s (%d)\n\r",
                 num,
                 pReset->arg2,
                 roomname,
                 pReset->arg1 );
        break;
    case 'T':
        sprintf( buf, "%2d) TRAP: %d %d %d %d (%s)\n\r",
                 num,
                 pReset->extra,
                 pReset->arg1,
                 pReset->arg2,
                 pReset->arg3,
                 flag_string(pReset->extra, trap_flags) );
        break;
    }
    if ( rlist && (!room || (room && room->vnum != rvnum)) )
        return NULL;
    return buf;
}

void do_rset( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf [MAX_STRING_LENGTH];
    ROOM_INDEX_DATA	*location, *tmp;
    EXTRA_DESCR_DATA	*ed;
    char		dir;
    EXIT_DATA		*xit, *texit;
    int			value;
    int			edir, ekey, evnum;
    char		*origarg = argument;

    if ( !ch->desc )
    {
        send_to_char( "You have no descriptor.\n\r", ch );
        return;
    }

    switch( ch->substate )
    {
    default:
        break;
    case SUB_ROOM_DESC:
        location = (ROOM_INDEX_DATA *)ch->dest_buf;
        if ( !location )
        {
            bug( "rset: sub_room_desc: NULL ch->dest_buf" );
            location = ch->in_room;
        }
        STRFREE( location->description );
        location->description = copy_buffer( ch );
        stop_editing( ch );
        ch->substate = ch->tempnum;
        return;
    case SUB_ROOM_EXTRA:
        ed = (EXTRA_DESCR_DATA *)ch->dest_buf;
        if ( !ed )
        {
            bug( "rset: sub_room_extra: NULL ch->dest_buf" );
            stop_editing( ch );
            return;
        }
        STRFREE( ed->description );
        ed->description = copy_buffer( ch );
        stop_editing( ch );
        ch->substate = ch->tempnum;
        return;
    }

    location = ch->in_room;

    smash_tilde( argument );
    argument = one_argument( argument, arg );
    if ( ch->substate == SUB_REPEATCMD )
    {
        if ( arg[0] == '\0' )
        {
            do_rstat( ch, "" );
            return;
        }
        if ( !str_cmp( arg, "done" ) || !str_cmp( arg, "off" ) )
        {
            send_to_char( "Rset mode off.\n\r", ch );
            if ( ch->pcdata && ch->pcdata->subprompt )
                STRFREE( ch->pcdata->subprompt );
            ch->substate = SUB_NONE;
            return;
        }
    }
    if ( arg[0] == '\0' || !str_cmp( arg, "?" ) )
    {
        if ( ch->substate == SUB_REPEATCMD )
            send_to_char( "Syntax: <field> value\n\r",			ch );
        else
            send_to_char( "Syntax: rset <field> value\n\r",		ch );
        send_to_char( "\n\r",						ch );
        send_to_char( "Field being one of:\n\r",			ch );
        send_to_char( "  name desc randomdesc ed rmed spec currvnum\n\r", ch );
        send_to_char( "  exit bexit exdesc exflags exname exkey\n\r",	ch );
        send_to_char( "  flags sector teledelay televnum tunnel\n\r",	ch );
        send_to_char( "  rlist exdistance elevation liquid depth\n\r",	ch );
        return;
    }

    if ( !can_rmodify( ch, location ) )
        return;

    if ( !str_cmp( arg, "on" ) )
    {
        send_to_char( "Rset mode on.\n\r", ch );
        ch->substate = SUB_REPEATCMD;
        if ( ch->pcdata )
        {
            if ( ch->pcdata->subprompt )
                STRFREE( ch->pcdata->subprompt );
            ch->pcdata->subprompt = STRALLOC( "<&CRset &W#%r&w> %i" );
        }
        return;
    }
    if ( !str_cmp( arg, "substate" ) )
    {
        argument = one_argument( argument, arg2);
        if( !str_cmp( arg2, "north" )  )
        {
            ch->inter_substate = SUB_NORTH;
            return;
        }
        if( !str_cmp( arg2, "east" )  )
        {
            ch->inter_substate = SUB_EAST;
            return;
        }
        if( !str_cmp( arg2, "south" )  )
        {
            ch->inter_substate = SUB_SOUTH;
            return;
        }
        if( !str_cmp( arg2, "west" )  )
        {
            ch->inter_substate = SUB_WEST;
            return;
        }
        if( !str_cmp( arg2, "up" )  )
        {
            ch->inter_substate = SUB_UP;
            return;
        }
        if( !str_cmp( arg2, "down" )  )
        {
            ch->inter_substate = SUB_DOWN;
            return;
        }
        send_to_char( " unrecognized substate in rset\n\r", ch);
        return;
    }


    if ( !str_cmp( arg, "name" ) )
    {
        if ( argument[0] == '\0' )
        {
            send_to_char( "Set the room name.  A very brief single line room description.\n\r", ch );
            send_to_char( "Usage: rset name <Room summary>\n\r", ch );
            return;
        }
        STRFREE( location->name );
        location->name = STRALLOC( argument );
        return;
    }

    if ( !str_cmp( arg, "desc" ) )
    {
        if ( argument[0] )
        {
            char tbuf[MAX_INPUT_LENGTH], *fbuf;

            fbuf = Justify(argument, 75, justify_left);
            snprintf(tbuf, sizeof(tbuf), "%s\n\r", fbuf);

            STRFREE( location->description );
            location->description = STRALLOC( tbuf );

            return;
        }

        if ( ch->substate == SUB_REPEATCMD )
            ch->tempnum = SUB_REPEATCMD;
        else
            ch->tempnum = SUB_NONE;
        ch->substate = SUB_ROOM_DESC;
        ch->dest_buf = location;
        start_editing( ch, location->description );
        return;
    }

    if ( !str_cmp( arg, "randomdesc" ) )
    {
        ROOM_INDEX_DATA *troom;
        char tbuf[MAX_INPUT_LENGTH];
        int start_room, end_room, rand;

        argument = one_argument(argument, tbuf);
        start_room = atoi(tbuf);
        argument = one_argument(argument, tbuf);
        end_room = atoi(tbuf);

        if (start_room <= 0 ||
            end_room <= 0 ||
            start_room == end_room)
        {
            send_to_char("Usage: rset randomdesc <start room> <end room>\n\r", ch);
            send_to_char("  Where start and end rooms are the rooms to copy the description from.\n\r", ch);
            return;
        }

        rand = number_range(start_room, end_room);

        if (!(troom = get_room_index(rand)))
        {
            ch_printf(ch, "Room %d not found.\n\r", rand);
            return;
        }

        STRFREE( location->description );
        location->description = STRALLOC( troom->description );
        send_to_char( "Done.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg, "tunnel" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Set the maximum characters allowed in the room at one time. (0 = unlimited).\n\r", ch );
            send_to_char( "Usage: rset tunnel <value>\n\r", ch );
            return;
        }
        location->tunnel = URANGE( 0, atoi(argument), 1000 );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "ed" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Create an extra description.\n\r", ch );
            send_to_char( "You must supply keyword(s).\n\r", ch );
            return;
        }
        CHECK_SUBRESTRICTED( ch );
        ed = SetRExtra( location, argument );
        if ( ch->substate == SUB_REPEATCMD )
            ch->tempnum = SUB_REPEATCMD;
        else
            ch->tempnum = SUB_NONE;
        ch->substate = SUB_ROOM_EXTRA;
        ch->dest_buf = ed;
        start_editing( ch, ed->description );
        return;
    }

    if ( !str_cmp( arg, "rmed" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Remove an extra description.\n\r", ch );
            send_to_char( "You must supply keyword(s).\n\r", ch );
            return;
        }
        if ( DelRExtra( location, argument ) )
            send_to_char( "Deleted.\n\r", ch );
        else
            send_to_char( "Not found.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "rlist" ) )
    {
        RESET_DATA *pReset;
        char *bptr;
        AREA_DATA *tarea;
        sh_int num;

        tarea = location->area;
        if ( !tarea->first_reset )
        {
            send_to_char( "This area has no resets to list.\n\r", ch );
            return;
        }
        num = 0;
        for ( pReset = tarea->first_reset; pReset; pReset = pReset->next )
        {
            num++;
            if ( (bptr = sprint_reset( ch, pReset, num, TRUE )) == NULL )
                continue;
            send_to_char( bptr, ch );
        }
        return;
    }

    if ( !str_cmp( arg, "spec" ) )
    {
        if ( !str_cmp( argument, "none" ) )
        {
            location->spec_fun = NULL;
            send_to_char( "Special function removed.\n\r", ch );
            return;
        }

        if ( ( location->spec_fun = r_spec_lookup( argument ) ) == NULL )
        {
            send_to_char( "No such spec fun.\n\r", ch );
            return;
        }
        return;
    }

    if ( !str_cmp( arg, "flags" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Toggle the room flags.\n\r", ch );
            send_to_char( "Usage: rset flags <flag> [flag]...\n\r", ch );
            for ( value = 0; value < 32; value++ )
                ch_printf(ch, "%s ", r_flags[value]);
            send_to_char("\n\r", ch);
            return;
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg2 );
            value = get_rflag( arg2 );
            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg2 );
            else
            {
                if ( 1 << value == ROOM_PROTOTYPE
                     &&   get_trust( ch ) < LEVEL_GREATER )
                    send_to_char( "You cannot change the prototype flag.\n\r", ch );
                else {
                    TOGGLE_BIT( location->room_flags, 1 << value );
                    send_to_char("Ok.\n\r", ch);
                }
            }
        }
        return;
    }

    if ( !str_cmp( arg, "teledelay" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Set the delay of the teleport. (0 = off).\n\r", ch );
            send_to_char( "Usage: rset teledelay <value>\n\r", ch );
            return;
        }
        location->tele_delay = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "televnum" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Set the vnum of the room to teleport to.\n\r", ch );
            send_to_char( "Usage: rset televnum <vnum>\n\r", ch );
            return;
        }
        location->tele_vnum = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "sector" ) )
    {
	int x;

        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Set the sector type.\n\r", ch );
            send_to_char( "Usage: rset sector <value>\n\r", ch );
	    buf[0] = '\0';
	    for (x=0; x<SECT_MAX; x++)
	    {
		strcat(buf, sect_types[x]);
		strcat(buf, " ");
	    }
	    send_to_char(buf, ch);
            return;
        }
        location->sector_type = get_sectortype(argument);
        if ( location->sector_type < 0 || location->sector_type >= SECT_MAX )
        {
            location->sector_type = SECT_FIELD;
            send_to_char( "Out of range.\n\r", ch );
        }
        else
            send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "exkey" ) )
    {
        argument = one_argument( argument, arg2 );
        argument = one_argument( argument, arg3 );
        if ( arg2[0] == '\0' || arg3[0] == '\0' )
        {
            send_to_char( "Usage: rset exkey <dir> <key vnum>\n\r", ch );
            return;
        }
        if ( arg2[0] == '#' )
            edir = atoi( arg2+1 );
        else
            edir = get_dir( arg2 );
	xit = get_exit( location, edir );
        value = atoi( arg3 );
        if ( !xit )
        {
            send_to_char( "No exit in that direction.  Use 'rset exit ...' first.\n\r", ch );
            return;
        }
        xit->key = value;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "exname" ) )
    {
        argument = one_argument( argument, arg2 );
        if ( arg2[0] == '\0' )
        {
            send_to_char( "Change or clear exit keywords.\n\r", ch );
            send_to_char( "Usage: rset exname <dir> [keywords]\n\r", ch );
            return;
        }
        if ( arg2[0] == '#' )
            edir = atoi( arg2+1 );
        else
            edir = get_dir( arg2 );
	xit = get_exit( location, edir );
        if ( !xit )
        {
            send_to_char( "No exit in that direction.  Use 'rset exit ...' first.\n\r", ch );
            return;
        }
        STRFREE( xit->keyword );
        xit->keyword = STRALLOC( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "exflags" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Toggle or display exit flags.\n\r", ch );
            send_to_char( "Usage: rset exflags <dir> <flag> [flag]...\n\r", ch );
            for ( value = 0; value < MAX_EXFLAG; value++ )
                ch_printf(ch, "%s ", ex_flags[value]);
            send_to_char("\n\r", ch);
            return;
        }
        argument = one_argument( argument, arg2 );
        if ( arg2[0] == '#' )
            edir = atoi( arg2+1 );
        else
            edir = get_dir( arg2 );
	xit = get_exit( location, edir );
        if ( !xit )
        {
            send_to_char( "No exit in that direction.  Use 'rset exit ...' first.\n\r", ch );
            return;
        }
        if ( argument[0] == '\0' )
        {
            sprintf( buf, "Flags for exit direction: %d  Keywords: %s  Key: %d\n\r[ ",
                     xit->vdir, xit->keyword, xit->key );
            for ( value = 0; value <= MAX_EXFLAG; value++ )
            {
                if ( IS_EXIT_FLAG( xit, 1 << value ) )
                {
                    strcat( buf, ex_flags[value] );
                    strcat( buf, " " );
                }
            }
            strcat( buf, "]\n\r" );
            send_to_char( buf, ch );
            return;
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg2 );
            value = get_exflag( arg2 );
            if ( value < 0 || value > MAX_EXFLAG )
                ch_printf( ch, "Unknown flag: %s\n\r", arg2 );
            else
                TOGGLE_BIT( xit->exit_info, 1 << value );
        }
        return;
    }

    if ( !str_cmp( arg, "ex_flags" ) )
    {
        argument = one_argument( argument, arg2 );
        switch(ch->inter_substate)
        {
        case SUB_EAST : dir = 'e'; edir = 1; break;
        case SUB_WEST : dir = 'w'; edir = 3; break;
        case SUB_SOUTH: dir = 's'; edir = 2; break;
        case SUB_UP   : dir = 'u'; edir = 4; break;
        case SUB_DOWN : dir = 'd'; edir = 5; break;
        default:
        case SUB_NORTH: dir = 'n'; edir = 0; break;
        }

        value = get_exflag(arg2);
        if ( value < 0 )
        {
            send_to_char("Bad exit flag. \n\r", ch);
            return;
        }
        if ( (xit = get_exit(location,edir)) == NULL )
        {
            sprintf(buf,"exit %c 1",dir);
            do_rset(ch,buf);
            xit = get_exit(location,edir);
        }
        TOGGLE_BIT( xit->exit_info, value );
        return;
    }


    if ( !str_cmp( arg, "ex_to_room" ) )
    {
        argument = one_argument( argument, arg2 );
        switch(ch->inter_substate)
        {
        case SUB_EAST : dir = 'e'; edir = 1; break;
        case SUB_WEST : dir = 'w'; edir = 3; break;
        case SUB_SOUTH: dir = 's'; edir = 2; break;
        case SUB_UP   : dir = 'u'; edir = 4; break;
        case SUB_DOWN : dir = 'd'; edir = 5; break;
        default:
        case SUB_NORTH: dir = 'n'; edir = 0; break;
        }
        evnum = atoi(arg2);
        if ( evnum < 1 || evnum > top_room_vnum )
        {
            send_to_char( "Invalid room number.\n\r", ch );
            return;
        }
        if ( (tmp = get_room_index( evnum )) == NULL )
        {
            send_to_char( "Non-existant room.\n\r", ch );
            return;
        }
        if ( (xit = get_exit(location,edir)) == NULL )
        {
            sprintf(buf,"exit %c 1",dir);
            do_rset(ch,buf);
            xit = get_exit(location,edir);
        }
        xit->vnum = evnum;
        return;
    }

    if ( !str_cmp( arg, "ex_key" ) )
    {
        argument = one_argument( argument, arg2 );
        switch(ch->inter_substate)
        {
        case SUB_EAST : dir = 'e'; edir = 1; break;
        case SUB_WEST : dir = 'w'; edir = 3; break;
        case SUB_SOUTH: dir = 's'; edir = 2; break;
        case SUB_UP   : dir = 'u'; edir = 4; break;
        case SUB_DOWN : dir = 'd'; edir = 5; break;
        default:
        case SUB_NORTH: dir = 'n'; edir = 0; break;
        }
        if ( (xit = get_exit(location,edir)) == NULL )
        {
            sprintf(buf,"exit %c 1",dir);
            do_rset(ch,buf);
            xit = get_exit(location,edir);
        }
        xit->key = atoi( arg2 );
        return;
    }

    if ( !str_cmp( arg, "ex_exdesc" ) )
    {
        switch(ch->inter_substate)
        {
        case SUB_EAST : dir = 'e'; edir = 1; break;
        case SUB_WEST : dir = 'w'; edir = 3; break;
        case SUB_SOUTH: dir = 's'; edir = 2; break;
        case SUB_UP   : dir = 'u'; edir = 4; break;
        case SUB_DOWN : dir = 'd'; edir = 5; break;
        default:
        case SUB_NORTH: dir = 'n'; edir = 0; break;
        }
        if ( (xit = get_exit(location, edir)) == NULL )
        {
            sprintf(buf,"exit %c 1",dir);
            do_rset(ch,buf);
        }
        sprintf(buf,"exdesc %c %s",dir,argument);
        do_rset(ch,buf);
        return;
    }

    if ( !str_cmp( arg, "ex_keywords" ) )  /* not called yet */
    {
        switch(ch->inter_substate)
        {
        case SUB_EAST : dir = 'e'; edir = 1; break;
        case SUB_WEST : dir = 'w'; edir = 3; break;
        case SUB_SOUTH: dir = 's'; edir = 2; break;
        case SUB_UP   : dir = 'u'; edir = 4; break;
        case SUB_DOWN : dir = 'd'; edir = 5; break;
        default:
        case SUB_NORTH: dir = 'n'; edir = 0; break;
        }
        if ( (xit = get_exit(location, edir)) == NULL )
        {
            sprintf(buf, "exit %c 1", dir);
            do_rset(ch,buf);
            if ( (xit = get_exit(location, edir)) == NULL )
                return;
        }
        sprintf( buf, "%s %s", xit->keyword, argument );
        STRFREE( xit->keyword );
        xit->keyword = STRALLOC( buf );
        return;
    }

    if ( !str_cmp( arg, "exit" ) )
    {
        bool addexit;

        argument = one_argument( argument, arg2 );
        argument = one_argument( argument, arg3 );
        if ( !arg2 || arg2[0] == '\0' )
        {
            send_to_char( "Create, change or remove an exit.\n\r", ch );
            send_to_char( "Usage: rset exit <dir> [room] [flags] [key] [keywords]\n\r", ch );
            return;
        }
        addexit = FALSE;
        switch( arg2[0] )
        {
        default:	edir = get_dir(arg2);			  break;
        case '+':	edir = get_dir(arg2+1);	addexit = TRUE;	  break;
        case '#':	edir = atoi(arg2+1);                      break;
        }
        if ( !arg3 || arg3[0] == '\0' )
            evnum = 0;
        else
            evnum = atoi( arg3 );
	xit = get_exit(location, edir);
        if ( !evnum )
        {
            if ( xit )
            {
                extract_exit(location, xit);
                send_to_char( "Exit removed.\n\r", ch );
                return;
            }
            send_to_char( "No exit in that direction.\n\r", ch );
            return;
        }
        if ( evnum < 1 || evnum > top_room_vnum )
        {
            send_to_char( "Invalid room number.\n\r", ch );
            return;
        }
        if ( (tmp = get_room_index( evnum )) == NULL )
        {
            send_to_char( "Non-existant room.\n\r", ch );
            return;
        }

        if ( !can_rmodify( ch, tmp ) )
        {
            send_to_char("You cannot make an exit to there.\n\r", ch);
            return;
        }

        if ( addexit || !xit )
        {
            if ( addexit && xit && get_exit_to(location, edir, tmp->vnum) )
            {
                send_to_char( "There is already an exit in that direction leading to that location.\n\r", ch );
                return;
	    }

	    xit = make_exit( location, tmp, edir );
	    if (xit->vdir > LAST_NORMAL_DIR)
		xit->keyword		= STRALLOC( "Arbitrary" );
	    else
		xit->keyword		= STRALLOC( "" );
            xit->description		= STRALLOC( "" );
            xit->key			= 0;
            xit->exit_info		= 0;
            act( AT_IMMORT, "$n reveals a hidden passage!", ch, NULL, NULL, TO_ROOM );
        }
        else
            act( AT_IMMORT, "Something is different...", ch, NULL, NULL, TO_ROOM );
        if ( xit->to_room != tmp )
        {
            xit->to_room = tmp;
            xit->vnum = evnum;
            texit = get_exit_to( xit->to_room, xit->rdir, location->vnum );
            if ( texit )
            {
                texit->rexit = xit;
                xit->rexit = texit;
            }
        }
        argument = one_argument( argument, arg3 );
        if ( arg3 && arg3[0] != '\0' )
            xit->exit_info = atoi( arg3 );
        if ( argument && argument[0] != '\0' )
        {
            one_argument( argument, arg3 );
            ekey = atoi( arg3 );
            if ( ekey != 0 || arg3[0] == '0' )
            {
                argument = one_argument( argument, arg3 );
                xit->key = ekey;
            }
            if ( argument && argument[0] != '\0' )
            {
                STRFREE( xit->keyword );
                xit->keyword = STRALLOC( argument );
            }
        }
        send_to_char( "Done.\n\r", ch );
        return;
    }

    /*
     * Twisted and evil, but works				-Thoric
     * Makes an exit, and the reverse in one shot.
     */
    if ( !str_cmp( arg, "bexit" ) )
    {
        EXIT_DATA *rxit;
        char tmpcmd[MAX_INPUT_LENGTH];
        ROOM_INDEX_DATA *tmploc;
        int vnum, exnum;
        char rvnum[MAX_INPUT_LENGTH];

        argument = one_argument( argument, arg2 );
        argument = one_argument( argument, arg3 );
        if ( !arg2 || arg2[0] == '\0' )
        {
            send_to_char( "Create, change or remove a two-way exit.\n\r", ch );
            send_to_char( "Usage: rset bexit <dir> [room] [flags] [key] [keywords]\n\r", ch );
            return;
        }
        switch( arg2[0] )
        {
        default:
            edir = get_dir( arg2 );
            break;
        case '#':
            edir = atoi( arg2+1 );
            break;
        case '+':
            edir = get_dir( arg2+1 );
            break;
        }
        tmploc = location;
        exnum = edir;
	xit = get_exit(tmploc, edir);
        rxit = NULL;
        vnum = 0;
        rvnum[0] = '\0';
        if ( xit )
        {
            vnum = xit->vnum;
            if ( arg3[0] != '\0' )
                sprintf( rvnum, "%d", tmploc->vnum );
            if ( xit->to_room )
                rxit = get_exit(xit->to_room, xit->rdir);
            else
                rxit = NULL;
        }
        sprintf( tmpcmd, "exit %s %s %s", arg2, arg3, argument );
	do_rset( ch, tmpcmd );
        vnum = 0;
	xit = get_exit(tmploc, edir);
        if ( !rxit && xit )
        {
            vnum = xit->vnum;
            if ( arg3[0] != '\0' )
                sprintf( rvnum, "%d", tmploc->vnum );
            if ( xit->to_room )
                rxit = get_exit(xit->to_room, xit->rdir);
            else
                rxit = NULL;
        }
        if ( vnum )
        {
            sprintf( tmpcmd, "%d rset exit %d %s %s",
 		     vnum,
		     xit->rdir,
                     rvnum,
                     argument );
            do_at( ch, tmpcmd );
        }
        return;
    }

    if ( !str_cmp( arg, "exdistance" ) )
    {
        argument = one_argument( argument, arg2 );
        if ( !arg2 || arg2[0] == '\0' )
        {
            send_to_char( "Set the distance (in rooms) between this room, and the destination room.\n\r", ch );
            send_to_char( "Usage: rset exdistance <dir> [distance]\n\r", ch );
            return;
        }
        if ( arg2[0] == '#' )
            edir = atoi( arg2+1 );
        else
            edir = get_dir( arg2 );
	xit = get_exit( location, edir );
        if ( xit )
        {
            xit->distance = URANGE( 1, atoi(argument), 50 );
            send_to_char( "Done.\n\r", ch );
            return;
        }
        send_to_char( "No exit in that direction.  Use 'rset exit ...' first.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "exdesc" ) )
    {
        argument = one_argument( argument, arg2 );
        if ( !arg2 || arg2[0] == '\0' )
        {
            send_to_char( "Create or clear a description for an exit.\n\r", ch );
            send_to_char( "Usage: rset exdesc <dir> [description]\n\r", ch );
            return;
        }
        if ( arg2[0] == '#' )
            edir = atoi( arg2+1 );
        else
            edir = get_dir( arg2 );
	xit = get_exit( location, edir );
        if ( xit )
        {
            STRFREE( xit->description );
            if ( !argument || argument[0] == '\0' )
                xit->description = STRALLOC( "" );
            else
            {
                sprintf( buf, "%s\n\r", argument );
                xit->description = STRALLOC( buf );
            }
            send_to_char( "Done.\n\r", ch );
            return;
        }
        send_to_char( "No exit in that direction.  Use 'rset exit ...' first.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "elevation" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Set the elevation\n\r", ch );
            send_to_char( "Usage: rset elevation <value>\n\r", ch );
            return;
        }
        location->elevation = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "liquid" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Set the river liquid\n\r", ch );
            send_to_char( "Usage: rset liquid <value>\n\r", ch );
            return;
        }
        location->liquid = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "depth" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Set the river depth.\n\r", ch );
            send_to_char( "Usage: rset depth <value>\n\r", ch );
            return;
        }
        if ( !location->river )
        {
            send_to_char( "That room is not a river.\n\r", ch );
            return;
        }
        location->river->depth = atoi( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "currvnum" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Set the currency vnum.\n\r", ch );
            send_to_char( "Usage: rset currvnum <vnum>\n\r", ch );
            return;
        }
        location->currvnum = atoi( argument );
        assign_currindex(location);
        send_to_char( "Done.\n\r", ch );
        return;
    }

    /*
     * Generate usage message.
     */
    if ( ch->substate == SUB_REPEATCMD )
    {
        ch->substate = SUB_RESTRICTED;
        interpret( ch, origarg );
        ch->substate = SUB_REPEATCMD;
        ch->last_cmd = do_rset;
    }
    else
        do_rset( ch, "" );
    return;
}

void do_ocreate( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    AREA_DATA *pArea = NULL;
    OBJ_INDEX_DATA	*pObjIndex;
    OBJ_DATA		*obj;
    int			 vnum, cvnum;

    if ( IS_NPC(ch) )
    {
        send_to_char( "Mobiles cannot create.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    vnum = is_number( arg ) ? atoi( arg ) : -1;

    if ( vnum == -1 || !argument || argument[0] == '\0' )
    {
        send_to_char( "Usage: ocreate <vnum> [copy vnum] <item name>\n\r", ch );
        return;
    }

    if ( vnum < 1 || vnum > 1048576000 )
    {
        send_to_char( "Bad number.\n\r", ch );
        return;
    }

    one_argument( argument, arg2 );
    cvnum = atoi( arg2 );
    if ( cvnum != 0 )
        argument = one_argument( argument, arg2 );
    if ( cvnum < 1 )
        cvnum = 0;

    if ( obj_exists_index( vnum ) )
    {
        send_to_char( "An object with that number already exists.\n\r", ch );
        return;
    }

    pArea = get_obj_area( vnum );
    if (!pArea)
    {
        send_to_char("That vnum is not within any existing area.\n\r", ch);
        return;
    }

    if ( (vnum<VNUM_START_SCRATCH || vnum>VNUM_END_SCRATCH) &&
         get_trust( ch ) < LEVEL_LESSER &&
         !is_name(pArea->filename, ch->pcdata->bestowments) )
    {
        if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
        {
            send_to_char( "You must have an assigned area to create objects.\n\r", ch );
            return;
        }
        if ( vnum < pArea->low_o_vnum
             &&   vnum > pArea->hi_o_vnum )
        {
            send_to_char( "That number is not in your allocated range.\n\r", ch );
            return;
        }
    }

    pObjIndex = make_object( vnum, cvnum, argument );
    if ( !pObjIndex )
    {
        send_to_char( "Error.\n\r", ch );
        log_string_plus( "do_ocreate: make_object failed.", LOG_BUILD, LEVEL_LOG_CSET, SEV_CRIT );
        return;
    }
    obj = create_object( vnum );
    obj_to_char( obj, ch );
    act( AT_IMMORT, "$n makes some ancient arcane gestures, and opens $s hands to reveal $p!", ch, obj, NULL, TO_ROOM );
    act( AT_IMMORT, "You make some ancient arcane gestures, and open your hands to reveal $p!", ch, obj, NULL, TO_CHAR );
}

void do_mcreate( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    AREA_DATA *pArea;
    MOB_INDEX_DATA	*pMobIndex;
    CHAR_DATA		*mob;
    int			 vnum, cvnum;

    if ( IS_NPC(ch) )
    {
        send_to_char( "Mobiles cannot create.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    vnum = is_number( arg ) ? atoi( arg ) : -1;

    if ( vnum == -1 || !argument || argument[0] == '\0' )
    {
        send_to_char( "Usage: mcreate <vnum> [copy vnum] <mobile name>\n\r", ch );
        return;
    }

    if ( vnum < 1 || vnum > 1048576000 )
    {
        send_to_char( "Bad number.\n\r", ch );
        return;
    }

    one_argument( argument, arg2 );
    cvnum = atoi( arg2 );
    if ( cvnum != 0 )
        argument = one_argument( argument, arg2 );
    if ( cvnum < 1 )
        cvnum = 0;

    if ( mob_exists_index( vnum ) )
    {
        send_to_char( "A mobile with that number already exists.\n\r", ch );
        return;
    }

    pArea = get_mob_area( vnum );
    if (!pArea)
    {
        send_to_char("That vnum is not within any existing area.\n\r", ch);
        return;
    }

    if ( (vnum<VNUM_START_SCRATCH || vnum>VNUM_END_SCRATCH) &&
         get_trust( ch ) < LEVEL_LESSER &&
         !is_name(pArea->filename, ch->pcdata->bestowments) )
    {
        if ( !ch->pcdata || !(pArea=ch->pcdata->area) )
        {
            send_to_char( "You must have an assigned area to create mobiles.\n\r", ch );
            return;
        }
        if ( vnum < pArea->low_m_vnum
             &&   vnum > pArea->hi_m_vnum )
        {
            send_to_char( "That number is not in your allocated range.\n\r", ch );
            return;
        }
    }

    pMobIndex = make_mobile( vnum, cvnum, argument );
    if ( !pMobIndex )
    {
        send_to_char( "Error.\n\r", ch );
        log_string_plus( "do_mcreate: make_mobile failed.", LOG_BUILD, LEVEL_LOG_CSET, SEV_CRIT );
        return;
    }
    mob = create_mobile( vnum );
    char_to_room( mob, ch->in_room );
    act( AT_IMMORT, "$n waves $s arms about, and $N appears at $s command!", ch, NULL, mob, TO_ROOM );
    act( AT_IMMORT, "You wave your arms about, and $N appears at your command!", ch, NULL, mob, TO_CHAR );
}


/*
 * Simple but nice and handle line editor.			-Thoric
 */
void edit_buffer( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    EDITOR_DATA *edit;
    char cmd[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    sh_int x, line, max_buf_lines;
    bool save;

    if ( ( d = ch->desc ) == NULL )
    {
        send_to_char( "You have no descriptor.\n\r", ch );
        return;
    }

    if ( GET_CON_STATE(ch) != CON_EDITING )
    {
        send_to_char( "You can't do that!\n\r", ch );
        bug( "Edit_buffer: GET_CON_STATE(ch) != CON_EDITING" );
        return;
    }

    if ( ch->substate <= SUB_PAUSE )
    {
        send_to_char( "You can't do that!\n\r", ch );
        bug( "Edit_buffer: illegal ch->substate (%d)", ch->substate );
        d->connected = CON_PLAYING;
        return;
    }

    if ( !ch->editor )
    {
        send_to_char( "You can't do that!\n\r", ch );
        bug( "Edit_buffer: null editor" );
        d->connected = CON_PLAYING;
        return;
    }

    edit = ch->editor;
    save = FALSE;
    max_buf_lines = (MAX_EDIT_LINES-1) / 2;

    if ( ch->substate == SUB_MPROG_EDIT || ch->substate == SUB_HELP_EDIT ||
         ch->substate == SUB_WRITING_NOTE )
        max_buf_lines = MAX_EDIT_LINES-1;

    if ( argument[0] == '/' || argument[0] == '\\' )
    {
        one_argument( argument, cmd );
        if ( !str_cmp( cmd+1, "?" ) )
        {
            send_to_char( "Editing commands\n\r----------------------------------------\n\r", ch );
            send_to_char( "/a              abort editing\n\r",	ch );
            send_to_char( "/l              list buffer\n\r",	ch );
            send_to_char( "/c              clear buffer\n\r",	ch );
            send_to_char( "/d [line]       delete line\n\r",	ch );
            send_to_char( "/g <line>       goto line\n\r",	ch );
            send_to_char( "/i <line>       insert line\n\r",	ch );
            send_to_char( "/r <old> <new>  global replace\n\r",	ch );
            if ( get_trust(ch) > LEVEL_IMMORTAL )
                send_to_char( "/! <command>    execute command (do not use another editing command)\n\r",  ch );
	    send_to_char( "/f              format buffer\n\r",  ch );
#ifdef USE_ASPELL
	    send_to_char( "/sp             spell check\n\r",    ch );
#endif
            send_to_char( "/s              save buffer\n\r\n\r> ",ch );
            return;
        }
        if ( !str_cmp( cmd+1, "c" ) )
        {
            memset( edit, '\0', sizeof(EDITOR_DATA) );
            edit->numlines = 0;
            edit->on_line   = 0;
            send_to_char( "Buffer cleared.\n\r> ", ch );
            return;
        }
        if ( !str_cmp( cmd+1, "r" ) )
        {
            char word1[MAX_INPUT_LENGTH];
            char word2[MAX_INPUT_LENGTH];
            char *sptr, *wptr, *lwptr;
            int count, wordln, word2ln, lineln;

            sptr = one_argument( argument, word1 );
            sptr = one_argument( sptr, word1 );
            sptr = one_argument( sptr, word2 );
            if ( word1[0] == '\0' || word2[0] == '\0' )
            {
                send_to_char( "Need word to replace, and replacement.\n\r> ", ch );
                return;
            }
            if ( strcmp( word1, word2 ) == 0 )
            {
                send_to_char( "Done.\n\r> ", ch );
                return;
            }
            count = 0;  wordln = strlen(word1);  word2ln = strlen(word2);
            ch_printf( ch, "Replacing all occurrences of %s with %s...\n\r", word1, word2 );
            for ( x = edit->on_line; x < edit->numlines; x++ )
            {
                lwptr = edit->line[x];
                while ( (wptr = strstr( lwptr, word1 )) != NULL )
                {
                    ++count;
                    lineln = sprintf( buf, "%s%s", word2, wptr + wordln );
                    if (lineln + wptr - edit->line[x] > 79)
                        buf[lineln] = '\0';
                    strcpy( wptr, buf );
                    lwptr = wptr + word2ln;
                }
            }
            ch_printf( ch, "Found and replaced %d occurrence(s).\n\r> ", count );
            return;
        }

        if ( !str_cmp( cmd+1, "i" ) )
        {
            if ( edit->numlines >= max_buf_lines )
                send_to_char( "Buffer is full.\n\r> ", ch );
            else
            {
                if ( argument[2] == ' ' )
                    line = atoi( argument + 2 ) - 1;
                else
                    line = edit->on_line;
                if ( line < 0 )
                    line = edit->on_line;
                if ( line < 0 || line > edit->numlines )
                    send_to_char( "Out of range.\n\r> ", ch );
                else
                {
                    for ( x = ++edit->numlines; x > line; x-- )
                        strcpy( edit->line[x], edit->line[x-1] );
                    strcpy( edit->line[line], "" );
                    send_to_char( "Line inserted.\n\r> ", ch );
                }
            }
            return;
        }
        if ( !str_cmp( cmd+1, "d" ) )
        {
            if ( edit->numlines == 0 )
                send_to_char( "Buffer is empty.\n\r> ", ch );
            else
            {
                if ( argument[2] == ' ' )
                    line = atoi( argument + 2 ) - 1;
                else
                    line = edit->on_line;
                if ( line < 0 )
                    line = edit->on_line;
                if ( line < 0 || line > edit->numlines )
                    send_to_char( "Out of range.\n\r> ", ch );
                else
                {
                    if ( line == 0 && edit->numlines == 1 )
                    {
                        memset( edit, '\0', sizeof(EDITOR_DATA) );
                        edit->numlines = 0;
                        edit->on_line   = 0;
                        send_to_char( "Line deleted.\n\r> ", ch );
                        return;
                    }
                    for ( x = line; x < (edit->numlines - 1); x++ )
                        strcpy( edit->line[x], edit->line[x+1] );
                    strcpy( edit->line[edit->numlines--], "" );
                    if ( edit->on_line > edit->numlines )
                        edit->on_line = edit->numlines;
                    send_to_char( "Line deleted.\n\r> ", ch );
                }
            }
            return;
        }
        if ( !str_cmp( cmd+1, "g" ) )
        {
            if ( edit->numlines == 0 )
                send_to_char( "Buffer is empty.\n\r> ", ch );
            else
            {
                if ( argument[2] == ' ' )
                    line = atoi( argument + 2 ) - 1;
                else
                {
                    send_to_char( "Goto what line?\n\r> ", ch );
                    return;
                }
                if ( line < 0 )
                    line = edit->on_line;
                if ( line < 0 || line > edit->numlines )
                    send_to_char( "Out of range.\n\r> ", ch );
                else
                {
                    edit->on_line = line;
                    ch_printf( ch, "(On line %d)\n\r> ", line+1 );
                }
            }
            return;
        }
        if ( !str_cmp( cmd+1, "l" ) )
        {
            if ( edit->numlines == 0 )
                send_to_char( "Buffer is empty.\n\r> ", ch );
            else
            {
                send_to_char( "------------------\n\r", ch );
                for ( x = 0; x < edit->numlines; x++ )
                    ch_printf( ch, "%2d%c %s\n\r",
                               x+1, edit->on_line==x?']':'>', edit->line[x] );
                send_to_char( "------------------\n\r> ", ch );
            }
            return;
        }
        if ( !str_cmp( cmd+1, "a" ) )
        {
            sprintf(log_buf, "%s done editing.", GET_NAME(ch));
            log_string_plus( log_buf, LOG_BUILD, UMAX(GetMaxLevel(ch),sysdata.log_level), SEV_INFO );
            send_to_char( "\n\rAborting... ", ch );
            stop_editing( ch );
            return;
        }
        if ( get_trust(ch) > LEVEL_IMMORTAL && !str_cmp( cmd+1, "!" ) )
        {
            DO_FUN *last_cmd;
            int substate = ch->substate;

            last_cmd = ch->last_cmd;
            ch->substate = SUB_RESTRICTED;
            interpret(ch, argument+3);
            ch->substate = substate;
            ch->last_cmd = last_cmd;
            set_char_color( AT_GREEN, ch );
            send_to_char( "\n\r> ", ch );
            return;
        }
        if ( !str_cmp( cmd+1, "s" ) )
        {
            sprintf(log_buf, "%s done editing.", GET_NAME(ch));
            log_string_plus( log_buf, LOG_BUILD, UMAX(GetMaxLevel(ch),sysdata.log_level), SEV_INFO );
            d->connected = CON_PLAYING;
            if ( !ch->last_cmd )
                return;
            (*ch->last_cmd) ( ch, "" );
            return;
        }
        if ( !str_cmp( cmd+1, "f" ) )
        {
            int substate, connected;
            void *dest_buf, *spare_ptr;
            char *jbuf, *tbuf;

            tbuf = copy_buffer( ch );
            jbuf = Justify(tbuf, 75, justify_left);
            STRFREE(tbuf);

            substate = ch->substate;
            connected = ch->desc->connected;
            dest_buf = ch->dest_buf;
            spare_ptr = ch->spare_ptr;
            stop_editing( ch );
            start_editing( ch, jbuf );
            ch->substate = substate;
            ch->desc->connected = connected;
            ch->dest_buf = dest_buf;
            ch->spare_ptr = spare_ptr;
            return;
	}
#ifdef USE_ASPELL
	if ( !str_cmp( cmd+1, "sp" ) )
	{
            char *tbuf;
            tbuf = copy_buffer( ch );
	    aspell_string(ch, tbuf);
            STRFREE(tbuf);
            return;
	}
#endif
    }

    if ( edit->size + strlen(argument) + 1 >= MAX_STRING_LENGTH - 1 )
        send_to_char( "You buffer is full.\n\r", ch );
    else
    {
        if ( strlen(argument) > MAX_EDIT_LINE_LENGTH-2 )
        {
            strncpy( buf, argument, MAX_EDIT_LINE_LENGTH-2 );
            buf[MAX_EDIT_LINE_LENGTH-2] = 0;
            send_to_char( "(Long line trimmed)\n\r> ", ch );
        }
        else
            strcpy( buf, argument );
        strcpy( edit->line[edit->on_line++], buf );
        if ( edit->on_line > edit->numlines )
            edit->numlines++;
        if ( edit->numlines > max_buf_lines )
        {
            edit->numlines = max_buf_lines;
            send_to_char( "Buffer full.\n\r", ch );
            save = TRUE;
        }
    }

    if ( save )
    {
        d->connected = CON_PLAYING;
        if ( !ch->last_cmd )
            return;
        (*ch->last_cmd) ( ch, "" );
        return;
    }
    send_to_char( "> ", ch );
}

void free_reset( AREA_DATA *are, RESET_DATA *res )
{
    UNLINK( res, are->first_reset, are->last_reset, next, prev );
    DISPOSE( res );
}

void free_area( AREA_DATA *are )
{
    DISPOSE( are->name );
    DISPOSE( are->filename );
    while ( are->first_reset )
        free_reset( are, are->first_reset );
    DISPOSE( are );
    are = NULL;
}

void assign_area( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char taf[MAX_INPUT_LENGTH];
    AREA_DATA *tarea, *tmp;
    bool created = FALSE;

    if ( IS_NPC( ch ) )
        return;
    if ( get_trust( ch ) >= LEVEL_IMMORTAL
         &&   ch->pcdata->r_range_lo
         &&   ch->pcdata->r_range_hi )
    {
        tarea = ch->pcdata->area;
        sprintf( taf, "%s.are", capitalize( ch->name ) );
        if ( !tarea )
        {
            for ( tmp = first_build; tmp; tmp = tmp->next )
                if ( !str_cmp( taf, tmp->filename ) )
                {
                    tarea = tmp;
                    break;
                }

        }
        if ( !tarea )
        {
            sprintf( log_buf, "Creating area entry for %s", ch->name );
            log_string_plus( log_buf, LOG_NORMAL, GetMaxLevel(ch), SEV_NOTICE );
            CREATE( tarea, AREA_DATA, 1 );
            LINK( tarea, first_build, last_build, next, prev );
            tarea->first_reset	= NULL;
            tarea->last_reset	= NULL;
            sprintf( buf, "{PROTO} %s's area in progress", ch->name );
            tarea->name		= str_dup( buf );
            tarea->filename	= str_dup( taf );
            sprintf( buf2, "%s", ch->name );
            tarea->author 	= STRALLOC( buf2 );
            tarea->age		= 0;
            tarea->nplayer	= 0;

            CREATE(tarea->weather, WEATHER_DATA, 1); /* FB */
	    tarea->weather->temp = 0;
	    tarea->weather->precip = 0;
	    tarea->weather->wind = 0;
	    tarea->weather->temp_vector = 0;
	    tarea->weather->precip_vector = 0;
	    tarea->weather->wind_vector = 0;
	    tarea->weather->climate_temp = 2;
	    tarea->weather->climate_precip = 2;
	    tarea->weather->climate_wind = 2;
	    tarea->weather->first_neighbor = NULL;
	    tarea->weather->last_neighbor = NULL;
	    tarea->weather->echo = NULL;
	    tarea->weather->echo_color = AT_GREY;

            created = TRUE;
        }
        else
        {
            sprintf( log_buf, "Updating area entry for %s", ch->name );
            log_string_plus( log_buf, LOG_NORMAL, GetMaxLevel(ch), SEV_NOTICE );
        }
        tarea->low_r_vnum	= ch->pcdata->r_range_lo;
        tarea->low_o_vnum	= ch->pcdata->o_range_lo;
        tarea->low_m_vnum	= ch->pcdata->m_range_lo;
        tarea->hi_r_vnum	= ch->pcdata->r_range_hi;
        tarea->hi_o_vnum	= ch->pcdata->o_range_hi;
        tarea->hi_m_vnum	= ch->pcdata->m_range_hi;
        ch->pcdata->area	= tarea;
        if ( created )
            sort_area( tarea, TRUE );
    }
}

void do_aassign( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    AREA_DATA *tarea, *tmp;

    if ( IS_NPC( ch ) )
        return;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax: aassign <filename.are>\n\r", ch );
        return;
    }

    if ( !str_cmp( "none", argument )
         ||   !str_cmp( "null", argument )
         ||   !str_cmp( "clear", argument ) )
    {
        ch->pcdata->area = NULL;
        assign_area( ch );
        if ( !ch->pcdata->area )
            send_to_char( "Area pointer cleared.\n\r", ch );
        else
            send_to_char( "Originally assigned area restored.\n\r", ch );
        return;
    }

    sprintf( buf, "%s", argument );
    tarea = NULL;

    /*	if ( get_trust(ch) >= sysdata.level_modify_proto )*/
    if ( get_trust(ch) >= LEVEL_SUB_IMPLEM
         ||  (is_name( buf, ch->pcdata->bestowments )
              &&   get_trust(ch) >= sysdata.level_modify_proto) )
        for ( tmp = first_area; tmp; tmp = tmp->next )
            if ( !str_cmp( buf, tmp->filename ) )
            {
                tarea = tmp;
                break;
            }

    if ( !tarea )
        for ( tmp = first_build; tmp; tmp = tmp->next )
            if ( !str_cmp( buf, tmp->filename ) )
            {
                if ( get_trust(ch) >= sysdata.level_modify_proto
                     ||   is_name( tmp->filename, ch->pcdata->bestowments ) )
                {
                    tarea = tmp;
                    break;
                }
                else
                {
                    send_to_char( "You do not have permission to use that area.\n\r", ch );
                    return;
                }
            }

    if ( !tarea )
    {
        if ( get_trust(ch) >= sysdata.level_modify_proto )
            send_to_char( "No such area.  Use 'zones'.\n\r", ch );
        else
            send_to_char( "No such area.  Use 'newzones'.\n\r", ch );
        return;
    }
    ch->pcdata->area = tarea;
    ch_printf( ch, "Assigning you: %s\n\r", tarea->name );
    return;
}


EXTRA_DESCR_DATA *SetRExtra( ROOM_INDEX_DATA *room, char *keywords )
{
    EXTRA_DESCR_DATA *ed;

    for ( ed = room->first_extradesc; ed; ed = ed->next )
    {
        if ( is_name( keywords, ed->keyword ) )
            break;
    }
    if ( !ed )
    {
        CREATE( ed, EXTRA_DESCR_DATA, 1 );
        LINK( ed, room->first_extradesc, room->last_extradesc, next, prev );
        ed->keyword	= STRALLOC( keywords );
        ed->description	= STRALLOC( "" );
        top_ed++;
    }
    return ed;
}

bool DelRExtra( ROOM_INDEX_DATA *room, char *keywords )
{
    EXTRA_DESCR_DATA *rmed;

    for ( rmed = room->first_extradesc; rmed; rmed = rmed->next )
    {
        if ( is_name( keywords, rmed->keyword ) )
            break;
    }
    if ( !rmed )
        return FALSE;
    UNLINK( rmed, room->first_extradesc, room->last_extradesc, next, prev );
    STRFREE( rmed->keyword );
    STRFREE( rmed->description );
    DISPOSE( rmed );
    top_ed--;
    return TRUE;
}

EXTRA_DESCR_DATA *SetOExtra( OBJ_DATA *obj, char *keywords )
{
    EXTRA_DESCR_DATA *ed;

    for ( ed = obj->first_extradesc; ed; ed = ed->next )
    {
        if ( is_name( keywords, ed->keyword ) )
            break;
    }
    if ( !ed )
    {
        CREATE( ed, EXTRA_DESCR_DATA, 1 );
        LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
        ed->keyword	= STRALLOC( keywords );
        ed->description	= STRALLOC( "" );
        top_ed++;
    }
    return ed;
}

bool DelOExtra( OBJ_DATA *obj, char *keywords )
{
    EXTRA_DESCR_DATA *rmed;

    for ( rmed = obj->first_extradesc; rmed; rmed = rmed->next )
    {
        if ( is_name( keywords, rmed->keyword ) )
            break;
    }
    if ( !rmed )
        return FALSE;
    UNLINK( rmed, obj->first_extradesc, obj->last_extradesc, next, prev );
    STRFREE( rmed->keyword );
    STRFREE( rmed->description );
    DISPOSE( rmed );
    top_ed--;
    return TRUE;
}

EXTRA_DESCR_DATA *SetOExtraProto( OBJ_INDEX_DATA *obj, char *keywords )
{
    EXTRA_DESCR_DATA *ed;

    for ( ed = obj->first_extradesc; ed; ed = ed->next )
    {
        if ( is_name( keywords, ed->keyword ) )
            break;
    }
    if ( !ed )
    {
        CREATE( ed, EXTRA_DESCR_DATA, 1 );
        LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
        ed->keyword	= STRALLOC( keywords );
        ed->description	= STRALLOC( "" );
        top_ed++;
    }
    return ed;
}

bool DelOExtraProto( OBJ_INDEX_DATA *obj, char *keywords )
{
    EXTRA_DESCR_DATA *rmed;

    for ( rmed = obj->first_extradesc; rmed; rmed = rmed->next )
    {
        if ( is_name( keywords, rmed->keyword ) )
            break;
    }
    if ( !rmed )
        return FALSE;
    UNLINK( rmed, obj->first_extradesc, obj->last_extradesc, next, prev );
    STRFREE( rmed->keyword );
    STRFREE( rmed->description );
    DISPOSE( rmed );
    top_ed--;
    return TRUE;
}

bool exit_nonstandard_rev_dir( EXIT_DATA *pexit )
{
    if (pexit->vdir < 0 || pexit->vdir > LAST_NORMAL_DIR)
	return TRUE;
    if (rev_dir[pexit->vdir] != pexit->rdir)
	return TRUE;
    return FALSE;
}

#if defined(USE_DB) || defined(START_DB)
void fold_area( AREA_DATA *tarea, char *filename, bool install )
{
    ROOM_INDEX_DATA	*room;
    MOB_INDEX_DATA	*pMobIndex;
    OBJ_INDEX_DATA	*pObjIndex;
    EXIT_DATA		*xit;
    SHOP_DATA		*pShop;
    REPAIR_DATA		*pRepair;
    int			 vnum;

    sprintf( log_buf, "Saving %s...", tarea->filename );
    fprintf(stderr,"%s\n",log_buf);
    log_string_plus( log_buf, LOG_BUILD, LEVEL_GREATER, SEV_NOTICE );

    db_del_area(tarea);

    if (IS_AREA_FLAG(tarea, AFLAG_MODIFIED))
        REMOVE_AREA_FLAG(tarea, AFLAG_MODIFIED);
    if (IS_AREA_FLAG(tarea, AFLAG_INITIALIZED))
        REMOVE_AREA_FLAG(tarea, AFLAG_INITIALIZED);
    db_insert_area(0,tarea);

    db_insert_climate(0,tarea);

    db_insert_neighbors(0,tarea);

    /* save mobiles */
    for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
    {
        if ( (pMobIndex = get_mob_index( vnum )) == NULL )
            continue;
        if ( install )
            REMOVE_ACT_FLAG( pMobIndex, ACT_PROTOTYPE );

        db_insert_mob(0,pMobIndex);

        if (pMobIndex->mudprogs)
            db_insert_progs(0,tarea,pMobIndex->mudprogs,vnum,"MOB");

        if ( pMobIndex->spec_fun )
        {
            db_insert_special(0,tarea,pMobIndex->spec_fun,vnum,"MOB");
        }

        if ( (pShop = pMobIndex->pShop) != NULL )
            db_insert_shop(0,pShop);

        if ( (pRepair = pMobIndex->rShop) != NULL )
            db_insert_repair(0,pRepair);
    }
    if ( install && vnum < tarea->hi_m_vnum )
        tarea->hi_m_vnum = vnum - 1;

    /* save objects */
    for ( vnum = tarea->low_o_vnum; vnum <= tarea->hi_o_vnum; vnum++ )
    {
        if ( (pObjIndex = get_obj_index( vnum )) == NULL )
            continue;
        if ( install )
            REMOVE_BIT( pObjIndex->extra_flags, ITEM_PROTOTYPE );

        db_insert_obj(0,pObjIndex);

        if (pObjIndex->first_extradesc)
            db_insert_descs(0,tarea, pObjIndex->first_extradesc, pObjIndex->vnum, "OBJ");

        if (pObjIndex->first_affect)
            db_insert_objaffects(0,tarea,pObjIndex);

        if (pObjIndex->mudprogs)
            db_insert_progs(0,tarea,pObjIndex->mudprogs,vnum,"OBJ");

        if ( pObjIndex->spec_fun )
        {
            db_insert_special(0,tarea,pObjIndex->spec_fun,vnum,"OBJ");
        }
    }
    if ( install && vnum < tarea->hi_o_vnum )
        tarea->hi_o_vnum = vnum - 1;

    /* save rooms   */
    for ( vnum = tarea->low_r_vnum; vnum <= tarea->hi_r_vnum; vnum++ )
    {
        if ( (room = get_room_index( vnum )) == NULL )
            continue;
        if ( install )
        {
            CHAR_DATA *victim, *vnext;
            OBJ_DATA  *obj, *obj_next;

            /* remove prototype flag from room */
            REMOVE_ROOM_FLAG( room, ROOM_PROTOTYPE );
            /* purge room of (prototyped) mobiles */
            for ( victim = room->first_person; victim; victim = vnext )
            {
                vnext = victim->next_in_room;
                if ( IS_NPC(victim) )
                    extract_char( victim, TRUE );
            }
            /* purge room of (prototyped) objects */
            for ( obj = room->first_content; obj; obj = obj_next )
            {
                obj_next = obj->next_content;
                extract_obj( obj );
            }
        }

        db_insert_room(0,room);

        for ( xit = room->first_exit; xit; xit = xit->next )
        {
            if ( IS_EXIT_FLAG(xit, EX_PORTAL) ) /* don't fold portals */
                continue;
            db_insert_exit(0,tarea,xit,vnum);
        }
        if (room->first_extradesc)
            db_insert_descs(0,tarea, room->first_extradesc, room->vnum, "ROOM");

        /*
        if ( room->map )
        {
#ifdef OLDMAPS
            fprintf( fpout, "M\n" );
            fprintf( fpout, "%s~\n", strip_cr( room->map )	);
#endif
            fprintf( fpout, "M %d %d %d %c\n",	room->map->vnum
                     , room->map->x
                     , room->map->y
                     , room->map->entry );
        }
        */
        if (room->mudprogs)
            db_insert_progs(0,tarea,room->mudprogs,vnum,"ROOM");

        if ( room->spec_fun )
        {
            db_insert_special(0,tarea,room->spec_fun,vnum,"ROOM");
        }
    }
    if ( install && vnum < tarea->hi_r_vnum )
        tarea->hi_r_vnum = vnum - 1;

    /* save resets   */
    db_insert_resets(0,tarea);

    return;
}
#else

void fold_area( AREA_DATA *tarea, char *filename, bool install )
{
    RESET_DATA		*treset;
    ROOM_INDEX_DATA	*room;
    MOB_INDEX_DATA	*pMobIndex;
    OBJ_INDEX_DATA	*pObjIndex;
    MPROG_DATA		*mprog;
    EXIT_DATA		*xit;
    EXTRA_DESCR_DATA	*ed;
    AFFECT_DATA		*paf;
    SHOP_DATA		*pShop;
    REPAIR_DATA		*pRepair;
    NEIGHBOR_DATA	*neigh;
    char		 buf[MAX_STRING_LENGTH];
    FILE		*fpout;
    int			 vnum;
    int			 val0, val1, val2, val3, val4, val5;
    sh_int		 complexmob;

    if ( !IS_AREA_STATUS(tarea, AREA_LOADED) )
    {
	bug( "fold_area: %s not loaded.", tarea->filename );
	return;
    }

    sprintf( log_buf, "Saving %s...", tarea->filename );
    log_string_plus( log_buf, LOG_BUILD, LEVEL_GREATER, SEV_NOTICE );

    sprintf( buf, "%s.bak", filename );
    rename( filename, buf );
    if ( ( fpout = fopen( filename, "w" ) ) == NULL )
    {
        bug( "fold_area: fopen" );
        perror( filename );
        return;
    }

    sprintf(buf, "%s.bak", filename);

    fprintf( fpout, "#AREA   %s~\n\n\n\n", tarea->name );

    fprintf( fpout, "#VERSION %d\n", AREA_VERSION_WRITE );
    fprintf( fpout, "#AUTHOR %s~\n\n", tarea->author );
    fprintf( fpout, "#RANGES\n");
    fprintf( fpout, "%d %d %d %d\n", tarea->low_soft_range,
             tarea->hi_soft_range,
             tarea->low_hard_range,
             tarea->hi_hard_range );
    fprintf( fpout, "$\n\n");
    if ( tarea->resetmsg )	/* Rennard */
        fprintf( fpout, "#RESETMSG %s~\n\n", tarea->resetmsg );
    if ( tarea->comment )	/* Rennard */
        fprintf( fpout, "#COMMENT %s~\n\n", tarea->comment );
    if (IS_AREA_FLAG(tarea, AFLAG_MODIFIED))
        REMOVE_AREA_FLAG(tarea, AFLAG_MODIFIED);
    if (IS_AREA_FLAG(tarea, AFLAG_INITIALIZED))
        REMOVE_AREA_FLAG(tarea, AFLAG_INITIALIZED);
    if ( tarea->reset_frequency || tarea->flags )
        fprintf( fpout, "#FLAGS\n%d %d\n\n",
                 tarea->flags, tarea->reset_frequency );
    else
        fprintf( fpout, "#FLAGS\n%d\n\n", tarea->flags );

    fprintf( fpout, "#PLANE\n%d\n\n", tarea->plane );

    fprintf( fpout, "#CURRENCY %d\n\n", tarea->currvnum );

    fprintf( fpout, "#HIGHECONOMY" );
    for (vnum=0;vnum<MAX_CURR_TYPE;vnum++)
        fprintf(fpout, " %d", tarea->high_economy[vnum]);
    fprintf( fpout, " -1\n\n");

    fprintf( fpout, "#LOWECONOMY" );
    for (vnum=0;vnum<MAX_CURR_TYPE;vnum++)
        fprintf(fpout, " %d", tarea->low_economy[vnum]);
    fprintf( fpout, " -1\n\n");

    fprintf( fpout, "#CLIMATE %d %d %d\n\n", tarea->weather->climate_temp,
    					     tarea->weather->climate_precip,
    					     tarea->weather->climate_wind);

    /* neighboring weather systems - FB */
    for(neigh = tarea->weather->first_neighbor; neigh; neigh = neigh->next)
    {
    	fprintf( fpout, "#NEIGHBOR %s~\n\n", neigh->name);
    }

    /* save mobiles */
    fprintf( fpout, "#MOBILES\n" );
    for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
    {
        if ( (pMobIndex = get_mob_index( vnum )) == NULL )
            continue;
        if ( install )
            REMOVE_ACT_FLAG( pMobIndex, ACT_PROTOTYPE );
        if ( pMobIndex->perm_str != 13	||   pMobIndex->perm_int   != 13
             ||   pMobIndex->perm_wis != 13	||   pMobIndex->perm_dex   != 13
             ||   pMobIndex->perm_con != 13	||   pMobIndex->perm_cha   != 13
             ||   pMobIndex->perm_lck != 13
             ||   pMobIndex->classes[CLASS_WARRIOR] == 0
             ||   pMobIndex->hitroll  != 0	||   pMobIndex->damroll    != 0
             ||   pMobIndex->race	 != 0	||   !IS_ACTIVE(pMobIndex, CLASS_WARRIOR)
             ||   pMobIndex->attacks	 != 0	||   pMobIndex->defenses   != 0
             ||   pMobIndex->height	 != 0	||   pMobIndex->weight	   != 0
             ||   pMobIndex->speaks	 != 0	||   pMobIndex->speaking   != 0
             ||   pMobIndex->xflags	 != 0   ||   pMobIndex->numattacks != 0
             ||   IS_SET(pMobIndex->act, ACT_CUSTOMSAVES) )
            complexmob = TRUE;
        else
            complexmob = FALSE;
        if (complexmob && (pMobIndex->act2!=0 ||
			   pMobIndex->affected_by2!=0 ||
			   pMobIndex->absorb!=0))
            complexmob = TRUE+1;
        fprintf( fpout, "#%d\n",	vnum				);
        fprintf( fpout, "%s~\n",	pMobIndex->player_name		);
        fprintf( fpout,	"%s~\n",	strip_crlf(pMobIndex->short_descr));
        fprintf( fpout,	"%s~\n",	strip_cr(pMobIndex->long_descr)	);
        fprintf( fpout, "%s~\n",	strip_cr(pMobIndex->description));
        fprintf( fpout, "%d %d %d %c\n",pMobIndex->act,
                 pMobIndex->affected_by,
                 pMobIndex->alignment,
                 complexmob==(TRUE+1) ? 'D' :
                 (complexmob==TRUE ? 'C' : 'S'));

        fprintf( fpout, "%d %d %d ", pMobIndex->levels[MIFirstActive(pMobIndex)],
                 pMobIndex->mobthac0,
                 pMobIndex->ac			);
        fprintf( fpout, "%dd%d+%d ",	pMobIndex->hitnodice,
                 pMobIndex->hitsizedice,
                 pMobIndex->hitplus		);
        fprintf( fpout, "%dd%d+%d\n",	pMobIndex->damnodice,
                 pMobIndex->damsizedice,
                 pMobIndex->damplus		);
        fprintf( fpout, "%d %d\n",	GET_MONEY(pMobIndex,DEFAULT_CURR),
                 pMobIndex->exp			);
        fprintf( fpout, "%d %d %d\n",	pMobIndex->position,
                 pMobIndex->defposition,
                 pMobIndex->sex			);
        if ( complexmob )
        {
            fprintf( fpout, "%d %d %d %d %d %d %d\n",
                     pMobIndex->perm_str,
                     pMobIndex->perm_int,
                     pMobIndex->perm_wis,
                     pMobIndex->perm_dex,
                     pMobIndex->perm_con,
                     pMobIndex->perm_cha,
                     pMobIndex->perm_lck );
            if (IS_SET(pMobIndex->act, ACT_CUSTOMSAVES))
                fprintf( fpout, "%d %d %d %d %d\n",
                         pMobIndex->saving_poison_death,
                         pMobIndex->saving_wand,
                         pMobIndex->saving_para_petri,
                         pMobIndex->saving_breath,
                         pMobIndex->saving_spell_staff );
            else
                fprintf( fpout, "0 0 0 0 0\n" );
            fprintf( fpout, "%d %d %d %d %d %d %d\n",
                     pMobIndex->race,
                     MIFirstActive(pMobIndex),
                     pMobIndex->height,
                     pMobIndex->weight,
                     pMobIndex->speaks,
                     pMobIndex->speaking,
                     pMobIndex->numattacks );
            fprintf( fpout, "%d %d %d %d %d %d %d %d\n",
                     pMobIndex->hitroll,
                     pMobIndex->damroll,
                     pMobIndex->xflags,
                     pMobIndex->resistant,
                     pMobIndex->immune,
                     pMobIndex->susceptible,
                     pMobIndex->attacks,
                     pMobIndex->defenses );
            if (complexmob==TRUE+1)
                fprintf( fpout, "%d %d %d\n",
                         pMobIndex->act2,
                         pMobIndex->affected_by2,
                         pMobIndex->absorb);

        }

        /* looks kinda hokey with the two nested val0 loops, but if
         * any of the monies are nonzero then save them all and break */
        for (val0=0; val0 < MAX_CURR_TYPE; val0++ )
            if (GET_MONEY(pMobIndex,val0))
            {
                fprintf( fpout, "T" );
                for (val0=0; val0 < MAX_CURR_TYPE; val0++ )
                    fprintf(fpout, " %d", GET_MONEY(pMobIndex,val0));
                fprintf( fpout, " -1\n" );
                break;
            }

        if ( pMobIndex->mudprogs )
        {
            for ( mprog = pMobIndex->mudprogs; mprog; mprog = mprog->next )
	    {
                fprintf( fpout, "> %s %s~\n%s~\n",
                         mprog_type_to_name( mprog->progtype ),
                         mprog->arglist, strip_cr(mprog->comlist) );
	    }
            fprintf( fpout, "|\n" );
        }

        fprintf( fpout, "S\n" );
    }
    fprintf( fpout, "#0\n\n\n" );
    if ( install && vnum < tarea->hi_m_vnum )
        tarea->hi_m_vnum = vnum - 1;

    /* save objects */
    fprintf( fpout, "#OBJECTS\n" );
    for ( vnum = tarea->low_o_vnum; vnum <= tarea->hi_o_vnum; vnum++ )
    {
        if ( (pObjIndex = get_obj_index( vnum )) == NULL )
            continue;
        if ( install )
            REMOVE_BIT( pObjIndex->extra_flags, ITEM_PROTOTYPE );
        fprintf( fpout, "#%d\n",	vnum				);
        fprintf( fpout, "%s~\n",	pObjIndex->name			);
        fprintf( fpout, "%s~\n",	strip_crlf(pObjIndex->short_descr));
        fprintf( fpout, "%s~\n",	strip_cr(pObjIndex->description));
        fprintf( fpout, "%s~\n",	strip_cr(pObjIndex->action_desc));
        if ( pObjIndex->layers )
            fprintf( fpout, "%d %d %d %d\n",
                     pObjIndex->item_type,
                     pObjIndex->extra_flags,
                     pObjIndex->wear_flags,
                     pObjIndex->layers );
        else
            fprintf( fpout, "%d %d %d\n",
                     pObjIndex->item_type,
                     pObjIndex->extra_flags,
                     pObjIndex->wear_flags );

        val0 = pObjIndex->value[0];
        val1 = pObjIndex->value[1];
        val2 = pObjIndex->value[2];
        val3 = pObjIndex->value[3];
        val4 = pObjIndex->value[4];
        val5 = pObjIndex->value[5];
        switch ( pObjIndex->item_type )
        {
        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL:
            if ( IS_VALID_SN(val1) )
	    {
		if ( AREA_VERSION_WRITE == 0 ) val1 = skill_table[val1]->slot;
		else	val1 = HAS_SPELL_INDEX;
	    }
	    if ( IS_VALID_SN(val2) )
	    {
		if ( AREA_VERSION_WRITE == 0 ) val2 = skill_table[val2]->slot;
		else	val1 = HAS_SPELL_INDEX;
	    }
	    if ( IS_VALID_SN(val3) )
	    {
		if ( AREA_VERSION_WRITE == 0 ) val3 = skill_table[val3]->slot;
		else	val1 = HAS_SPELL_INDEX;
	    }
            break;
        case ITEM_STAFF:
        case ITEM_WAND:
	    if ( IS_VALID_SN(val3) )
	    {
		if ( AREA_VERSION_WRITE == 0 ) val3 = skill_table[val3]->slot;
		else  val3 = HAS_SPELL_INDEX;
	    }
            break;
        case ITEM_SALVE:
	    if ( IS_VALID_SN(val4) )
	    {
		if ( AREA_VERSION_WRITE == 0 ) val4 = skill_table[val4]->slot;
		else  val4 = HAS_SPELL_INDEX;
	    }
	    if ( IS_VALID_SN(val5) )
	    {
		if ( AREA_VERSION_WRITE == 0 ) val5 = skill_table[val5]->slot;
		else  val5 = HAS_SPELL_INDEX;
	    }
            break;
        }
        if ( val4 || val5 )
            fprintf( fpout, "%d %d %d %d %d %d\n",val0,
                     val1,
                     val2,
                     val3,
                     val4,
                     val5 );
        else
            fprintf( fpout, "%d %d %d %d\n",	val0,
                     val1,
                     val2,
                     val3 );

        fprintf( fpout, "%d %d %d %d\n",
                 pObjIndex->weight,
                 pObjIndex->cost,
                 pObjIndex->rent ? pObjIndex->rent :
                 (int) (pObjIndex->cost / 10),
                 pObjIndex->currtype );


        {
            char b1[MAX_INPUT_LENGTH], b2[MAX_INPUT_LENGTH], b3[MAX_INPUT_LENGTH];
            b1[0]=b2[0]=b3[0]='\0';
            if ( AREA_VERSION_WRITE > 0 )
                switch ( pObjIndex->item_type )
                {
                case ITEM_PILL:
                case ITEM_POTION:
                case ITEM_SCROLL:
                    sprintf(b1, "%s",
                            IS_VALID_SN(pObjIndex->value[1])?
                            skill_table[pObjIndex->value[1]]->name:"NONE");
                    sprintf(b2, "%s",
                            IS_VALID_SN(pObjIndex->value[2])?
                            skill_table[pObjIndex->value[2]]->name:"NONE");
                    sprintf(b3, "%s",
                            IS_VALID_SN(pObjIndex->value[3])?
                            skill_table[pObjIndex->value[3]]->name:"NONE");
                    fprintf( fpout, "'%s' '%s' '%s'\n", b1, b2, b3);
                    break;
                case ITEM_STAFF:
                case ITEM_WAND:
                    sprintf(b1, "%s",
                            IS_VALID_SN(pObjIndex->value[3])?
                            skill_table[pObjIndex->value[3]]->name:"NONE");
                    fprintf( fpout, "'%s'\n", b1);

                    break;
                case ITEM_SALVE:
                    sprintf(b1, "%s",
                            IS_VALID_SN(pObjIndex->value[4])?
                            skill_table[pObjIndex->value[4]]->name:"NONE");
                    sprintf(b2, "%s",
                            IS_VALID_SN(pObjIndex->value[5])?
                            skill_table[pObjIndex->value[5]]->name:"NONE");
                    fprintf( fpout, "'%s' '%s'\n", b1, b2);
                    break;
                }
        }

            if (pObjIndex->extra_flags2>0 || pObjIndex->magic_flags>0)
                fprintf(fpout, "D\n%d %d\n",
                        pObjIndex->extra_flags2,
                        pObjIndex->magic_flags);

        for ( ed = pObjIndex->first_extradesc; ed; ed = ed->next )
        {
            fprintf( fpout, "E\n%s~\n",
                     strip_cr( ed->keyword ) );
            fprintf( fpout, "%s~\n",
                     strip_cr( ed->description ) );
        }

        for ( paf = pObjIndex->first_affect; paf; paf = paf->next )
	{
            fprintf( fpout, "A\n%d %d\n", paf->location,
                     ((paf->location == APPLY_WEAPONSPELL
                       || paf->location == APPLY_WEARSPELL
                       || paf->location == APPLY_REMOVESPELL
                       || paf->location == APPLY_EAT_SPELL
                       || paf->location == APPLY_IMMUNESPELL
                       || paf->location == APPLY_STRIPSN)
                      && IS_VALID_SN(paf->modifier))
                     ? skill_table[paf->modifier]->slot : paf->modifier		);
	}

        if ( pObjIndex->mudprogs )
        {
            for ( mprog = pObjIndex->mudprogs; mprog; mprog = mprog->next )
	    {
                fprintf( fpout, "> %s %s~\n%s~\n",
                         mprog_type_to_name( mprog->progtype ),
                         mprog->arglist, strip_cr(mprog->comlist) );
	    }
            fprintf( fpout, "|\n" );
        }

        fprintf( fpout, "S\n" );
    }
    fprintf( fpout, "#0\n\n\n" );
    if ( install && vnum < tarea->hi_o_vnum )
        tarea->hi_o_vnum = vnum - 1;

    /* save rooms   */
    fprintf( fpout, "#ROOMS\n" );
    for ( vnum = tarea->low_r_vnum; vnum <= tarea->hi_r_vnum; vnum++ )
    {
        if ( (room = get_room_index( vnum )) == NULL )
            continue;
        if ( install )
        {
            CHAR_DATA *victim, *vnext;
            OBJ_DATA  *obj, *obj_next;

            /* remove prototype flag from room */
            REMOVE_ROOM_FLAG( room, ROOM_PROTOTYPE );
            /* purge room of (prototyped) mobiles */
            for ( victim = room->first_person; victim; victim = vnext )
            {
                vnext = victim->next_in_room;
                if ( IS_NPC(victim) )
                    extract_char( victim, TRUE );
            }
            /* purge room of (prototyped) objects */
            for ( obj = room->first_content; obj; obj = obj_next )
            {
                obj_next = obj->next_content;
                extract_obj( obj );
            }
        }
        fprintf( fpout, "#%d\n",	vnum				);
        fprintf( fpout, "%s~\n",	strip_cr( room->name )		);
        fprintf( fpout, "%s~\n",	strip_cr( room->description )	);
        if ( (room->tele_delay > 0 && room->tele_vnum > 0) ||
             room->tunnel > 0 ||
             room->elevation != 1000 ||
             room->liquid != 0 ||
             room->currvnum > 0)
            fprintf( fpout, "0 %d %d %d %d %d %d %d %d\n",
                     room->room_flags & ~ROOM_ORPHANED,
                     room->sector_type,
                     room->tele_delay,
                     room->tele_vnum,
                     room->tunnel,
                     room->elevation,
                     room->liquid,
                     room->currvnum);
        else
            fprintf( fpout, "0 %d %d\n",
                     room->room_flags & ~ROOM_ORPHANED,
                     room->sector_type );

        if (!room->first_exit)
            log_printf_plus( LOG_BUILD, LEVEL_LOG_CSET, SEV_ERR,
                             "fold_area: room %d (%s) has no exits", room->vnum, room->name );

        for ( xit = room->first_exit; xit; xit = xit->next )
        {
            if ( IS_EXIT_FLAG(xit, EX_PORTAL) ) /* don't fold portals */
		continue;
            fprintf( fpout, "D%d\n",		xit->vdir );
            fprintf( fpout, "%s~\n",		strip_crlf( xit->description ) );
            fprintf( fpout, "%s~\n",		strip_crlf( xit->keyword ) );
            if ( xit->distance > 1 || exit_nonstandard_rev_dir(xit) )
		fprintf( fpout, "%d %d %d %d %d\n",
			 xit->exit_info & ~EX_BASHED,
                         xit->key,
                         xit->vnum,
			 xit->distance,
		         xit->rdir );
            else
		fprintf( fpout, "%d %d %d\n",
			 xit->exit_info & ~EX_BASHED,
			 xit->key,
                         xit->vnum );

        }
        for ( ed = room->first_extradesc; ed; ed = ed->next )
        {
            if (!ed->keyword || !*ed->keyword)
                continue;
            fprintf( fpout, "E\n%s~\n",
                     strip_cr( ed->description ) );
            fprintf( fpout, "%s~\n",
                     strip_cr( ed->keyword ) );
        }

        if ( room->map )   /* maps */
        {
#ifdef OLDMAPS
            fprintf( fpout, "M\n" );
            fprintf( fpout, "%s~\n", strip_cr( room->map )	);
#endif
            fprintf( fpout, "M %d %d %d %c\n",	room->map->vnum
                     , room->map->x
                     , room->map->y
                     , room->map->entry );
        }
        if ( room->mudprogs )
        {
            for ( mprog = room->mudprogs; mprog; mprog = mprog->next )
	    {
                fprintf( fpout, "> %s %s~\n%s~\n",
                         mprog_type_to_name( mprog->progtype ),
                         mprog->arglist, strip_cr(mprog->comlist) );
	    }
            fprintf( fpout, "|\n" );
        }
        fprintf( fpout, "S\n" );
    }
    fprintf( fpout, "#0\n\n\n" );
    if ( install && vnum < tarea->hi_r_vnum )
        tarea->hi_r_vnum = vnum - 1;

    /* save resets   */
    fprintf( fpout, "#RESETS\n" );
    for ( treset = tarea->first_reset; treset; treset = treset->next )
    {
        switch( treset->command ) /* extra arg1 arg2 arg3 */
        {
        default:  case '*': break;
        case 'm': case 'M':
        case 'o': case 'O':
        case 'p': case 'P':
        case 'e': case 'E':
        case 'd': case 'D':
        case 't': case 'T':
            fprintf( fpout, "%c %d %d %d %d\n", UPPER(treset->command),
                     treset->extra, treset->arg1, treset->arg2, treset->arg3 );
            break;
        case 'g': case 'G':
        case 'r': case 'R':
            fprintf( fpout, "%c %d %d %d\n", UPPER(treset->command),
                     treset->extra, treset->arg1, treset->arg2 );
            break;
        }
    }
    fprintf( fpout, "S\n\n\n" );

    /* save shops */
    fprintf( fpout, "#SHOPS\n" );
    for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
    {
        if ( (pMobIndex = get_mob_index( vnum )) == NULL )
            continue;
        if ( (pShop = pMobIndex->pShop) == NULL )
            continue;
        fprintf( fpout, " %d   %2d %2d %2d %2d %2d   %3d %3d",
                 pShop->keeper,
                 pShop->buy_type[0],
                 pShop->buy_type[1],
                 pShop->buy_type[2],
                 pShop->buy_type[3],
                 pShop->buy_type[4],
                 pShop->profit_buy,
                 pShop->profit_sell );
        fprintf( fpout, "        %2d %2d    ; %s\n",
                 pShop->open_hour,
                 pShop->close_hour,
                 strip_cr( pMobIndex->short_descr ) );
    }
    fprintf( fpout, "0\n\n\n" );

    /* save repair shops */
    fprintf( fpout, "#REPAIRS\n" );
    for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
    {
        if ( (pMobIndex = get_mob_index( vnum )) == NULL )
            continue;
        if ( (pRepair = pMobIndex->rShop) == NULL )
            continue;
        fprintf( fpout, " %d   %2d %2d %2d         %3d %3d",
                 pRepair->keeper,
                 pRepair->fix_type[0],
                 pRepair->fix_type[1],
                 pRepair->fix_type[2],
                 pRepair->profit_fix,
                 pRepair->shop_type );
        fprintf( fpout, "        %2d %2d    ; %s\n",
                 pRepair->open_hour,
                 pRepair->close_hour,
                 strip_cr( pMobIndex->short_descr ) );
    }
    fprintf( fpout, "0\n\n\n" );

    /* save specials */
    fprintf( fpout, "#SPECIALS\n" );
    for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
    {
        if ( (pMobIndex = get_mob_index( vnum )) != NULL )
            if ( pMobIndex->spec_fun )
	    {
                fprintf( fpout, "M  %d %s\n",	pMobIndex->ivnum,
                         m_lookup_spec( pMobIndex->spec_fun ) );
	    }
        if ( (pObjIndex = get_obj_index( vnum )) != NULL )
            if ( pObjIndex->spec_fun )
	    {
                fprintf( fpout, "O  %d %s\n",	pObjIndex->ivnum,
                         o_lookup_spec( pObjIndex->spec_fun ) );
	    }
        if ( (room = get_room_index( vnum )) != NULL )
            if ( room->spec_fun )
	    {
                fprintf( fpout, "R  %d %s\n",	room->vnum,
                         r_lookup_spec( room->spec_fun ) );
	    }
    }
    fprintf( fpout, "S\n\n\n" );

    /* END */
    fprintf( fpout, "#$\n" );
    FCLOSE( fpout );
    return;
}
#endif

void do_savearea( CHAR_DATA *ch, char *argument )
{
    AREA_DATA	*tarea;
    char	 filename[256];

    if ( IS_NPC(ch) || get_trust( ch ) < LEVEL_CREATOR || !ch->pcdata
         ||  ( argument[0] == '\0' && !ch->pcdata->area) )
    {
        send_to_char( "You don't have an assigned area to save.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
        tarea = ch->pcdata->area;
    else
    {
        bool found;

        if ( get_trust( ch ) < LEVEL_GOD )
        {
            send_to_char( "You can only save your own area.\n\r", ch );
            return;
        }
        for ( found = FALSE, tarea = first_build; tarea; tarea = tarea->next )
            if ( !str_cmp( tarea->filename, argument ) )
            {
                found = TRUE;
                break;
            }
        if ( !found )
        {
            send_to_char( "Area not found.\n\r", ch );
            return;
        }
    }

    if ( !tarea )
    {
        send_to_char( "No area to save.\n\r", ch );
        return;
    }

    /* Ensure not wiping out their area with save before load - Scryn 8/11 */
    if ( !IS_AREA_STATUS(tarea, AREA_LOADED ) )
    {
        send_to_char( "Your area is not loaded!\n\r", ch );
        return;
    }

    sprintf( filename, "%s%s", BUILD_DIR, tarea->filename );
    fold_area( tarea, filename, FALSE );
    send_to_char( "Done.\n\r", ch );
}

void do_loadarea( CHAR_DATA *ch, char *argument )
{
    AREA_DATA	*tarea;
    char	 filename[256];
    int		tmp;

    if ( IS_NPC(ch) || get_trust( ch ) < LEVEL_CREATOR || !ch->pcdata
         ||  ( argument[0] == '\0' && !ch->pcdata->area) )
    {
        send_to_char( "You don't have an assigned area to load.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
        tarea = ch->pcdata->area;
    else
    {
        bool found;

        if ( get_trust( ch ) < LEVEL_GOD )
        {
            send_to_char( "You can only load your own area.\n\r", ch );
            return;
        }
        for ( found = FALSE, tarea = first_build; tarea; tarea = tarea->next )
            if ( !str_cmp( tarea->filename, argument ) )
            {
                found = TRUE;
                break;
            }
        if ( !found )
        {
            send_to_char( "Area not found.\n\r", ch );
            return;
        }
    }

    if ( !tarea )
    {
        send_to_char( "No area to load.\n\r", ch );
        return;
    }

    /* Stops char from loading when already loaded - Scryn 8/11 */
    if ( IS_AREA_STATUS ( tarea, AREA_LOADED) )
    {
        send_to_char( "Your area is already loaded.\n\r", ch );
        return;
    }
    sprintf( filename, "%s%s", BUILD_DIR, tarea->filename );
    send_to_char( "Loading...\n\r", ch );
#ifndef USE_DB
    tarea = create_area( filename );
    load_area_file( tarea );
#endif
    send_to_char( "Linking exits...\n\r", ch );
    fix_area_exits( tarea );
    if ( tarea->first_reset )
    {
        tmp = tarea->nplayer;
        tarea->nplayer = 0;
        send_to_char( "Resetting area...\n\r", ch );
        reset_area( tarea );
        tarea->nplayer = tmp;
    }
    send_to_char( "Done.\n\r", ch );
}

#ifndef USE_DB
void write_area_list( void )
{
    AREA_DATA *tarea;
    FILE *fpout;

    fpout = fopen( AREA_LIST, "w" );
    if ( !fpout )
    {
        bug( "FATAL: cannot open area.lst for writing!\n\r" );
        return;
    }
    fprintf( fpout, "#filename low_r_vnum hi_r_vnum low_m_vnum hi_m_vnum low_o_vnum hi_o_vnum\n");
    for ( tarea = first_area; tarea; tarea = tarea->next )
        fprintf( fpout, "%s %d %d %d %d %d %d %d\n",
                 tarea->filename,   tarea->flags,
                 tarea->low_r_vnum, tarea->hi_r_vnum,
                 tarea->low_m_vnum, tarea->hi_m_vnum,
                 tarea->low_o_vnum, tarea->hi_o_vnum);
    fprintf( fpout, "$\n" );
    FCLOSE( fpout );
}
#endif


/*
 * Dangerous command.  Can be used to install an area that was either:
 *   (a) already installed but removed from area.lst
 *   (b) designed offline
 * The mud will likely crash if:
 *   (a) this area is already loaded
 *   (b) it contains vnums that exist
 *   (c) the area has errors
 *
 * NOTE: Use of this command is not recommended.		-Thoric
 */
void do_unfoldarea( CHAR_DATA *ch, char *argument )
{

    if ( !argument || argument[0] == '\0' )
    {
        send_to_char( "Unfold what?\n\r", ch );
        return;
    }

    fBootDb = TRUE;
/*    load_area_file( last_area, argument );*/
    fBootDb = FALSE;
    return;
}


void do_foldarea( CHAR_DATA *ch, char *argument )
{
    AREA_DATA	*tarea;

    if ( !argument || argument[0] == '\0' )
    {
        send_to_char( "Fold what?\n\r", ch );
        return;
    }

    if ( get_trust(ch) < sysdata.level_modify_proto &&
         !is_name(argument, ch->pcdata->bestowments) )
    {
        send_to_char("You can't fold areas you havn't been bestowed.\n\r", ch);
        return;
    }

#ifndef USE_DB
    write_area_list( );
#endif
    for ( tarea = first_area; tarea; tarea = tarea->next )
    {
        if ( !str_cmp( tarea->filename, argument ) || !str_cmp( "saveall", argument ) )
        {
            send_to_char( "Folding...\n\r", ch );
            fold_area( tarea, tarea->filename, FALSE );
            send_to_char( "Done.\n\r", ch );
            if ( str_cmp( "saveall", argument ) )
                return;
        }
    }
    send_to_char( "No such area exists.\n\r", ch );
    return;
}

extern int top_area;

/*
 * A complicated to use command as it currently exists.		-Thoric
 * Once area->author and area->name are cleaned up... it will be easier
 */
void do_installarea( CHAR_DATA *ch, char *argument )
{
    AREA_DATA	*tarea;
    CHAR_DATA   *vch;
    char	arg[MAX_INPUT_LENGTH];
    char	buf[MAX_STRING_LENGTH];
    int		num;

    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: installarea <filename> [Area title]\n\r", ch );
        return;
    }

    for ( tarea = first_build; tarea; tarea = tarea->next )
    {
        if ( !str_cmp( tarea->filename, arg ) )
        {
            if ( argument && argument[0] != '\0' )
            {
                DISPOSE( tarea->name );
                tarea->name = str_dup( argument );
            }

            /* Fold area with install flag -- auto-removes prototype flags */
            send_to_char( "Saving and installing file...\n\r", ch );
            fold_area( tarea, tarea->filename, TRUE );

            /* Remove from prototype area list */
            UNLINK( tarea, first_build, last_build, next, prev );

            /* Add to real area list */
            LINK( tarea, first_area, last_area, next, prev );

            /* Fix up author if online */
            for ( vch = first_char; vch; vch = vch->next )
                if ( vch->pcdata &&
                     vch->pcdata->area == tarea )
                {
                    /* remove area from author */
                    vch->pcdata->area = NULL;
                    /* clear out author vnums  */
                    vch->pcdata->r_range_lo = 0;
                    vch->pcdata->r_range_hi = 0;
                    vch->pcdata->o_range_lo = 0;
                    vch->pcdata->o_range_hi = 0;
                    vch->pcdata->m_range_lo = 0;
                    vch->pcdata->m_range_hi = 0;
                }

            top_area++;
#ifndef USE_DB
            send_to_char( "Writing area.lst...\n\r", ch );
            write_area_list( );
#endif
            send_to_char( "Resetting new area.\n\r", ch );
            num = tarea->nplayer;
            tarea->nplayer = 0;
            reset_area( tarea );
            tarea->nplayer = num;
            send_to_char( "Renaming author's building file.\n\r", ch );
            sprintf( buf, "%s%s.installed", BUILD_DIR, tarea->filename );
            sprintf( arg, "%s%s", BUILD_DIR, tarea->filename );
            rename( arg, buf );
            send_to_char( "Done.\n\r", ch );
            return;
        }
    }
    send_to_char( "No such area exists.\n\r", ch );
    return;
}

void add_reset_nested( AREA_DATA *tarea, OBJ_DATA *obj )
{
    int limit;

    for ( obj = obj->first_content; obj; obj = obj->next_content )
    {
        limit = obj->pIndexData->count;
        if ( limit < 1 )
            limit = 1;
        add_reset( tarea, 'P', 1, obj->vnum, limit,
                   obj->in_obj->vnum );
        if ( obj->first_content )
            add_reset_nested( tarea, obj );
    }
}


/*
 * Parse a reset command string into a reset_data structure
 */
RESET_DATA *parse_reset( AREA_DATA *tarea, char *argument, CHAR_DATA *ch )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    char letter;
    int extra, val1, val2, val3;
    int value;
    ROOM_INDEX_DATA *room;
    EXIT_DATA	*pexit;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    argument = one_argument( argument, arg4 );
    extra = 0; letter = '*';
    val1 = atoi( arg2 );
    val2 = atoi( arg3 );
    val3 = atoi( arg4 );
    if ( arg1[0] == '\0' )
    {
        send_to_char( "Reset commands: mob obj give equip door rand trap hide.\n\r", ch );
        return NULL;
    }

    if ( !str_cmp( arg1, "hide" ) )
    {
        if ( arg2[0] != '\0' && !obj_exists_index(val1) )
        {
            send_to_char( "Reset: HIDE: no such object\n\r", ch );
            return NULL;
        }
        else
            val1 = 0;
        extra = 1;
        val2 = 0;
        val3 = 0;
        letter = 'H';
    }
    else if ( arg2[0] == '\0' )
    {
        send_to_char( "Reset: not enough arguments.\n\r", ch );
        return NULL;
    }
    else if ( val1 < 1 || val1 > 1048576000 )
    {
        send_to_char( "Reset: value out of range.\n\r", ch );
        return NULL;
    }
    else if ( !str_cmp( arg1, "mob" ) )
    {
        if ( !mob_exists_index(val1) )
        {
            ch_printf(ch, "Reset: MOB: no such mobile (%d)\n\r", val1 );
            return NULL;
        }
        if ( !room_exists_index(val2) )
        {
            send_to_char( "Reset: MOB: no such room\n\r", ch );
            return NULL;
        }
        if ( val3 < 1 )
            val3 = 1;
        letter = 'M';
    }
    else if ( !str_cmp( arg1, "obj" ) )
    {
        if ( !obj_exists_index(val1) )
        {
            ch_printf(ch, "Reset: OBJ: no such object (%d)\n\r", val1 );
            return NULL;
        }
        if ( !room_exists_index(val2) )
        {
            send_to_char( "Reset: OBJ: no such room\n\r", ch );
            return NULL;
        }
        if ( val3 < 1 )
            val3 = 1;
        letter = 'O';
    }
    else if ( !str_cmp( arg1, "give" ) )
    {
        if ( !obj_exists_index(val1) )
        {
            send_to_char( "Reset: GIVE: no such object\n\r", ch );
            return NULL;
        }
        if ( val2 < 1 )
            val2 = 1;
        val3 = val2;
        val2 = 0;
        extra = 1;
        letter = 'G';
    }
    else if ( !str_cmp( arg1, "equip" ) )
    {
        if ( !obj_exists_index(val1) )
        {
            send_to_char( "Reset: EQUIP: no such object\n\r", ch );
            return NULL;
        }
        if ( !is_number(arg3) )
            val2 = get_wearloc(arg3);
        if ( val2 < 0 || val2 >= MAX_WEAR )
        {
            send_to_char( "Reset: EQUIP: invalid wear location\n\r", ch );
            return NULL;
        }
        if ( val3 < 1 )
            val3 = 1;
        extra  = 1;
        letter = 'E';
    }
    else if ( !str_cmp( arg1, "put" ) )
    {
        if ( !obj_exists_index(val1) )
        {
            send_to_char( "Reset: PUT: no such object\n\r", ch );
            return NULL;
        }
        if ( val2 > 0 && !obj_exists_index(val2) )
        {
            send_to_char( "Reset: PUT: no such container\n\r", ch );
            return NULL;
        }
        extra = UMAX(val3, 0);
        argument = one_argument(argument, arg4);
        val3 = (is_number(argument) ? atoi(arg4) : 0);
        if ( val3 < 0 )
            val3 = 0;
        letter = 'P';
    }
    else if ( !str_cmp( arg1, "door" ) )
    {
        if ( (room = get_room_index(val1)) == NULL )
        {
            send_to_char( "Reset: DOOR: no such room\n\r", ch );
            return NULL;
        }
        if ( val2 < 0 || val2 > 9 )
        {
            send_to_char( "Reset: DOOR: invalid exit\n\r", ch );
            return NULL;
        }
        if ( (pexit = get_exit(room, val2)) == NULL
             ||   !IS_EXIT_FLAG( pexit, EX_ISDOOR ) )
        {
            send_to_char( "Reset: DOOR: no such door\n\r", ch );
            return NULL;
        }
        if ( val3 < 0 || val3 > 2 )
        {
            send_to_char( "Reset: DOOR: invalid door state (0 = open, 1 = close, 2 = lock)\n\r", ch );
            return NULL;
        }
        letter = 'D';
        value = val3;
        val3  = val2;
        val2  = value;
    }
    else if ( !str_cmp( arg1, "rand" ) )
    {
        if ( !room_exists_index(val1) )
        {
            send_to_char( "Reset: RAND: no such room\n\r", ch );
            return NULL;
        }
        if ( val2 < 0 || val2 > 9 )
        {
            send_to_char( "Reset: RAND: invalid max exit\n\r", ch );
            return NULL;
        }
        val3 = val2;
        val2 = 0;
        letter = 'R';
    }
    else if ( !str_cmp( arg1, "trap" ) )
    {
        if ( val2 < 1 || val2 > MAX_TRAPTYPE )
        {
            send_to_char( "Reset: TRAP: invalid trap type\n\r", ch );
            return NULL;
        }
        if ( val3 < 0 || val3 > 10000 )
        {
            send_to_char( "Reset: TRAP: invalid trap charges\n\r", ch );
            return NULL;
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg4 );
            value = get_trapflag( arg4 );
            if ( value >= 0 || value < 32 )
                SET_BIT( extra, 1 << value );
            else
            {
                send_to_char( "Reset: TRAP: bad flag\n\r", ch );
                return NULL;
            }
        }
        if ( IS_SET(extra, TRAP_ROOM) && IS_SET(extra, TRAP_OBJ) )
        {
            send_to_char( "Reset: TRAP: Must specify room OR object, not both!\n\r", ch );
            return NULL;
        }
        if ( IS_SET(extra, TRAP_ROOM) && !room_exists_index(val1) )
        {
            send_to_char( "Reset: TRAP: no such room\n\r", ch );
            return NULL;
        }
        if ( IS_SET(extra, TRAP_OBJ)  && val1>0 && !obj_exists_index(val1) )
        {
            send_to_char( "Reset: TRAP: no such object\n\r", ch );
            return NULL;
        }
        if (!IS_SET(extra, TRAP_ROOM) && !IS_SET(extra, TRAP_OBJ) )
        {
            send_to_char( "Reset: TRAP: Must specify ROOM or OBJECT\n\r", ch );
            return NULL;
        }
        /* fix order */
        value = val1;
        val1  = val2;
        val2  = value;
        letter = 'T';
    }
    if ( letter == '*' )
        return NULL;
    else
        return make_reset( letter, extra, val1, val3, val2 );
}

void do_astat( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *tarea;
    bool proto, found;
    char s1[16], s2[16], s3[16], s4[16];

    found = FALSE; proto = FALSE;
    for ( tarea = first_area; tarea; tarea = tarea->next )
        if ( !str_cmp( tarea->filename, argument ) )
        {
            found = TRUE;
            break;
        }

    if ( !found )
        for ( tarea = first_build; tarea; tarea = tarea->next )
            if ( !str_cmp( tarea->filename, argument ) )
            {
                found = TRUE;
                proto = TRUE;
                break;
            }

    if ( !found )
    {
        if ( argument && argument[0] != '\0' )
        {
            send_to_char( "Area not found.  Check 'zones'.\n\r", ch );
            return;
        }
        else
        {
            tarea = ch->in_room->area;
        }
    }

    sprintf(s1,"%s",color_str(AT_SCORE,ch));
    sprintf(s2,"%s",color_str(AT_SCORE2,ch));
    sprintf(s3,"%s",color_str(AT_SCORE3,ch));
    sprintf(s4,"%s",color_str(AT_SCORE4,ch));

    ch_printf( ch, "%sName: %s%s\n\r%sFilename: %s%-20s  %sPrototype: %s%s\n\r",
               s1, s3, tarea->name,
               s1, s3, tarea->filename,
               s1, s3, proto ? "yes" : "no" );
    if ( !proto )
    {
        ch_printf( ch, "%sMax players: %s%d  %sIllegalPks: %s%d  %sGold Looted: %s%d\n\r",
                   s1, s2, tarea->max_players,
                   s1, s2, tarea->illegal_pk,
                   s1, s2, tarea->looted[DEFAULT_CURR] );
        if ( tarea->high_economy[DEFAULT_CURR] )
            ch_printf( ch, "%sArea economy: %s%d %sbillion and %s%d %sgold coins.\n\r",
                       s1, s2, tarea->high_economy[DEFAULT_CURR],
                       s1, s2, tarea->low_economy[DEFAULT_CURR], s1 );
        else
            ch_printf( ch, "%sArea economy: %s%d %sgold coins.\n\r",
                       s1, s2, tarea->low_economy[DEFAULT_CURR], s1 );
        ch_printf( ch, "%sMdeaths: %s%d  %sMkills: %s%d  %sPdeaths: %s%d  %sPkills: %s%d\n\r",
                   s1, s2, tarea->mdeaths,
                   s1, s4, tarea->mkills,
                   s1, s2, tarea->pdeaths,
                   s1, s4, tarea->pkills );
    }
    ch_printf( ch, "%sAuthor: %s%s\n\r%sAge: %s%d  %sNumber of players: %s%d  %sPlane: %s%s\n\r",
               s1, s3, tarea->author?tarea->author:"(none)",
               s1, s2, tarea->age,
               s1, s2, tarea->nplayer,
               s1, s3, plane_names[tarea->plane]);
    ch_printf( ch, "%sArea flags: %s%s\n\r",
               s1, s2, flag_string(tarea->flags, area_flags) );
    ch_printf( ch, "%slow_room: %s%-5d  %shi_room: %s%d\n\r",
               s1, s2, tarea->low_r_vnum,
               s1, s2, tarea->hi_r_vnum );
    ch_printf( ch, "%slow_obj : %s%-5d  %shi_obj : %s%d\n\r",
               s1, s2, tarea->low_o_vnum,
               s1, s2, tarea->hi_o_vnum );
    ch_printf( ch, "%slow_mob : %s%-5d  %shi_mob : %s%d\n\r",
               s1, s2, tarea->low_m_vnum,
               s1, s2, tarea->hi_m_vnum );
    ch_printf( ch, "%ssoft range: %s%d%s-%s%d  %shard range: %s%d%s-%s%d\n\r",
               s1, s2, tarea->low_soft_range,
               s1, s4, tarea->hi_soft_range,
               s1, s2, tarea->low_hard_range,
               s1, s4, tarea->hi_hard_range );
    ch_printf( ch, "%sResetmsg: %s%s\n\r",
               s1, s3, tarea->resetmsg ? tarea->resetmsg : "(default)" );
    ch_printf( ch, "%sReset frequency: %s%d %sminutes.\n\r",
               s1, s2, tarea->reset_frequency ? tarea->reset_frequency : 15, s1);
    ch_printf( ch, "%sComment: %s%s\n\r",
               s1, s3, tarea->comment ? tarea->comment : "(none)" );
}


void do_aset( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *tarea;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    bool proto, found;
    int vnum, value;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    vnum = atoi( argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Usage: aset <area filename> <field> <value>\n\r", ch );
        send_to_char( "\n\rField being one of:\n\r", ch );
        send_to_char( "  low_room hi_room low_obj hi_obj low_mob hi_mob\n\r", ch );
        send_to_char( "  name filename low_soft hi_soft low_hard hi_hard\n\r", ch );
        send_to_char( "  author resetmsg resetfreq flags comment plane\n\r", ch );
        return;
    }

    found = FALSE; proto = FALSE;
    for ( tarea = first_area; tarea; tarea = tarea->next )
        if ( !str_cmp( tarea->filename, arg1 ) )
        {
            found = TRUE;
            break;
        }

    if ( !found )
        for ( tarea = first_build; tarea; tarea = tarea->next )
            if ( !str_cmp( tarea->filename, arg1 ) )
            {
                found = TRUE;
                proto = TRUE;
                break;
            }

    if ( !found )
    {
        send_to_char( "Area not found.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
        DISPOSE( tarea->name );
        tarea->name = str_dup( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "filename" ) && get_trust( ch ) >= LEVEL_SUB_IMPLEM )
    {
        DISPOSE( tarea->filename );
        tarea->filename = str_dup( argument );
        write_area_list( );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "low_economy" ) )
    {
        int currtype;

        argument = one_argument(argument, arg3);
        vnum = atoi(arg3);

        if (!is_number(argument))
            currtype = DEFAULT_CURR;
        else
            currtype = atoi(argument);

        tarea->low_economy[currtype] = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "high_economy" ) )
    {
        int currtype;

        argument = one_argument(argument, arg3);
        vnum = atoi(arg3);

        if (!is_number(argument))
            currtype = DEFAULT_CURR;
        else
            currtype = atoi(argument);

        tarea->high_economy[currtype] = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "low_room" ) )
    {
        tarea->low_r_vnum = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "hi_room" ) )
    {
        tarea->hi_r_vnum = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "low_obj" ) )
    {
        tarea->low_o_vnum = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "hi_obj" ) )
    {
        tarea->hi_o_vnum = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "low_mob" ) )
    {
        tarea->low_m_vnum = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "hi_mob" ) )
    {
        tarea->hi_m_vnum = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "low_soft" ) )
    {
        if ( vnum < 0 || vnum > MAX_LEVEL )
        {
            send_to_char( "That is not an acceptable value.\n\r", ch);
            return;
        }

        tarea->low_soft_range = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "hi_soft" ) )
    {
        if ( vnum < 0 || vnum > MAX_LEVEL )
        {
            send_to_char( "That is not an acceptable value.\n\r", ch);
            return;
        }

        tarea->hi_soft_range = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "low_hard" ) )
    {
        if ( vnum < 0 || vnum > MAX_LEVEL )
        {
            send_to_char( "That is not an acceptable value.\n\r", ch);
            return;
        }

        tarea->low_hard_range = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "hi_hard" ) )
    {
        if ( vnum < 0 || vnum > MAX_LEVEL )
        {
            send_to_char( "That is not an acceptable value.\n\r", ch);
            return;
        }

        tarea->hi_hard_range = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "author" ) )
    {
        STRFREE( tarea->author );
        tarea->author = STRALLOC( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "resetmsg" ) )
    {
        if ( tarea->resetmsg )
            DISPOSE( tarea->resetmsg );
        if ( str_cmp( argument, "clear" ) )
            tarea->resetmsg = str_dup( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    } /* Rennard */

    if ( !str_cmp( arg2, "comment" ) )
    {
        if ( tarea->comment )
            DISPOSE( tarea->comment );
        if ( str_cmp( argument, "clear" ) )
            tarea->comment = str_dup( argument );
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "resetfreq" ) )
    {
        tarea->reset_frequency = vnum;
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "plane" ) )
    {
        tarea->plane = get_plane( argument );
        if ( tarea->plane < FIRST_PLANE || tarea->plane >= LAST_PLANE )
        {
            send_to_char("Bad plane, valid options are: ", ch);
            for (value=FIRST_PLANE; value<LAST_PLANE; value++)
                ch_printf(ch, "%s, ", plane_names[value]);
            send_to_char("\n\r", ch);
            return;
        }
        send_to_char( "Done.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "flags" ) )
    {
        if ( !argument || argument[0] == '\0' )
        {
            send_to_char( "Usage: aset <filename> flags <flag> [flag]...\n\r", ch );

            return;
        }
        while ( argument[0] != '\0' )
        {
            argument = one_argument( argument, arg3 );
            value = get_areaflag( arg3 );
            if ( value < 0 || value > 31 )
                ch_printf( ch, "Unknown flag: %s\n\r", arg3 );
            else
            {
                if ( IS_AREA_FLAG( tarea, 1 << value ) )
                    REMOVE_AREA_FLAG( tarea, 1 << value );
                else
                    SET_AREA_FLAG( tarea, 1 << value );
            }
        }
        return;
    }

    do_aset( ch, "" );
    return;
}


void do_rlist( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA	*room;
    int			 vnum;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    AREA_DATA		*tarea;
    int lrange;
    int trange;

/*    if ( IS_NPC(ch) || get_trust( ch ) < LEVEL_CREATOR || !ch->pcdata
         || ( !ch->pcdata->area && get_trust( ch ) < LEVEL_GREATER ) )
    {
        send_to_char( "You don't have an assigned area.\n\r", ch );
        return;
    }*/

    tarea = ch->pcdata->area;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( tarea )
    {
        if ( arg1[0] == '\0' )		/* cleaned a big scary mess */
            lrange = tarea->low_r_vnum;	/* here.	    -Thoric */
        else
            lrange = atoi( arg1 );
        if ( arg2[0] == '\0' )
            trange = tarea->hi_r_vnum;
        else
            trange = atoi(arg2);

        if ( ( lrange < tarea->low_r_vnum || trange > tarea->hi_r_vnum )
             && get_trust( ch ) < LEVEL_GREATER )
        {
            send_to_char("That is out of your vnum range.\n\r", ch);
            return;
        }
    }
    else
    {
        tarea = ch->in_room->area;
        lrange = ( is_number( arg1 ) ? atoi( arg1 ) : tarea->low_r_vnum );
        trange = ( is_number( arg2 ) ? atoi( arg2 ) :
                   ( is_number( arg1 ) ? lrange : tarea->hi_r_vnum ) );
    }

    for ( vnum = lrange; vnum <= trange; vnum++ )
    {
        if ( (room = get_room_index( vnum )) == NULL )
            continue;
        pager_printf( ch, "%5d) %s\n\r", vnum, room->name );
    }
    return;
}

void do_olist( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA	*obj;
    int			 vnum;
    AREA_DATA		*tarea;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int lrange;
    int trange;

    /*
     * Greater+ can list out of assigned range - Tri (mlist/rlist as well)
     */
/*    if ( IS_NPC(ch) || get_trust( ch ) < LEVEL_CREATOR || !ch->pcdata
         || ( !ch->pcdata->area && get_trust( ch ) < LEVEL_GREATER ) )
    {
        send_to_char( "You don't have an assigned area.\n\r", ch );
        return;
    }*/
    tarea = ch->pcdata->area;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( tarea )
    {
        if ( arg1[0] == '\0' )		/* cleaned a big scary mess */
            lrange = tarea->low_o_vnum;	/* here.	    -Thoric */
        else
            lrange = atoi( arg1 );
        if ( arg2[0] == '\0' )
            trange = tarea->hi_o_vnum;
        else
            trange = atoi(arg2);

        if ((lrange < tarea->low_o_vnum || trange > tarea->hi_o_vnum)
            &&   get_trust( ch ) < LEVEL_GREATER )
        {
            send_to_char("That is out of your vnum range.\n\r", ch);
            return;
        }
    }
    else
    {
        tarea = ch->in_room->area;
        lrange = ( is_number( arg1 ) ? atoi( arg1 ) : tarea->low_o_vnum );
        trange = ( is_number( arg2 ) ? atoi( arg2 ) :
                   ( is_number( arg1 ) ? lrange : tarea->hi_o_vnum ) );
    }

    for ( vnum = lrange; vnum <= trange; vnum++ )
    {
        if ( (obj = get_obj_index( vnum )) == NULL )
            continue;
        pager_printf( ch, "%5d) %-20s (%s) x%d\n\r", vnum,
                      obj->name,
                      obj->short_descr,
                      obj->count );
    }
    return;
}

void do_mlist( CHAR_DATA *ch, char *argument )
{
    MOB_INDEX_DATA	*mob;
    int			 vnum;
    AREA_DATA		*tarea;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int lrange;
    int trange;

/*    if ( IS_NPC(ch) || get_trust( ch ) < LEVEL_CREATOR || !ch->pcdata
         ||  ( !ch->pcdata->area && get_trust( ch ) < LEVEL_GREATER ) )
    {
        send_to_char( "You don't have an assigned area.\n\r", ch );
        return;
    }*/

    tarea = ch->pcdata->area;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( tarea )
    {
        if ( arg1[0] == '\0' )		/* cleaned a big scary mess */
            lrange = tarea->low_m_vnum;	/* here.	    -Thoric */
        else
            lrange = atoi( arg1 );
        if ( arg2[0] == '\0' )
            trange = tarea->hi_m_vnum;
        else
            trange = atoi( arg2 );

        if ( ( lrange < tarea->low_m_vnum || trange > tarea->hi_m_vnum )
             && get_trust( ch ) < LEVEL_GREATER )
        {
            send_to_char("That is out of your vnum range.\n\r", ch);
            return;
        }
    }
    else
    {
        tarea = ch->in_room->area;
        lrange = ( is_number( arg1 ) ? atoi( arg1 ) : tarea->low_m_vnum );
        trange = ( is_number( arg2 ) ? atoi( arg2 ) :
                   ( is_number( arg1 ) ? lrange : tarea->hi_m_vnum ) );
    }

    for ( vnum = lrange; vnum <= trange; vnum++ )
    {
        if ( (mob = get_mob_index( vnum )) == NULL )
            continue;
        pager_printf( ch, "%5d) %-20s '%s' x%d\n\r", vnum,
                      mob->player_name,
                      mob->short_descr,
                      mob->count );
    }
}

void mpedit( CHAR_DATA *ch, MPROG_DATA *mprg, sh_int mptype, char *argument )
{
    if ( mptype != -1 )
    {
        mprg->progtype = mptype;
        if ( mprg->arglist )
            STRFREE( mprg->arglist );
        mprg->arglist = STRALLOC( argument );
    }
    ch->substate = SUB_MPROG_EDIT;
    ch->dest_buf = mprg;
    if ( !mprg->comlist )
        mprg->comlist = STRALLOC( "" );
    start_editing( ch, mprg->comlist );
    return;
}

/*
 * Mobprogram editing - cumbersome				-Thoric
 */
void do_mpedit( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char arg4 [MAX_INPUT_LENGTH];
    CHAR_DATA  *victim;
    MPROG_DATA *mprog, *mprg = NULL, *mprg_next = NULL;
    int value, cnt;
    sh_int mptype = ERROR_PROG;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mob's can't mpedit\n\r", ch );
        return;
    }

    if ( !ch->desc )
    {
        return;
    }

    CHECK_SUBRESTRICTED( ch );

    switch( ch->substate )
    {
    default:
        break;
    case SUB_MPROG_EDIT:
        if ( !ch->dest_buf )
        {
            send_to_char( "Fatal error: report to Thoric.\n\r", ch );
            bug( "do_mpedit: sub_mprog_edit: NULL ch->dest_buf" );
            ch->substate = SUB_NONE;
            return;
        }
        mprog	 = (MPROG_DATA *)ch->dest_buf;
        if ( mprog->comlist )
            STRFREE( mprog->comlist );
        mprog->comlist = copy_buffer( ch );
        stop_editing( ch );
        return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    value = atoi( arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Syntax: mpedit <victim> <command> [number] <program> <value>\n\r", ch );
        send_to_char( "\n\r",						ch );
        send_to_char( "Command being one of:\n\r",			ch );
        send_to_char( "  add delete insert edit list\n\r",		ch );
        send_to_char( "Program being one of:\n\r",			ch );
        send_to_char( "  act speech rand fight hitprcnt greet allgreet\n\r", ch );
        send_to_char( "  entry give bribe death time hour script\n\r",	ch );
        return;
    }

    if ( get_trust( ch ) < LEVEL_GOD )
    {
        if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }
    }
    else
    {
        if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
        {
            send_to_char( "No one like that in all the realms.\n\r", ch );
            return;
        }
    }

    if ( get_trust( ch ) < GetMaxLevel(victim) || !IS_NPC(victim) )
    {
        send_to_char( "You can't do that!\n\r", ch );
        return;
    }

    if ( !can_mmodify( ch, victim ) )
        return;

    if ( !IS_ACT_FLAG( victim, ACT_PROTOTYPE ) )
    {
        send_to_char( "A mobile must have a prototype flag to be mpset.\n\r", ch );
        return;
    }

    mprog = victim->pIndexData->mudprogs;

    set_char_color( AT_GREEN, ch );

    if ( !str_cmp( arg2, "list" ) )
    {
        cnt = 0;
        if ( !mprog )
        {
            send_to_char( "That mobile has no mob programs.\n\r", ch );
            return;
        }
        for ( mprg = mprog; mprg; mprg = mprg->next )
            ch_printf( ch, "%d>%s %s\n\r%s\n\r",
                       ++cnt,
                       mprog_type_to_name( mprg->progtype ),
                       mprg->arglist,
                       mprg->comlist );
        return;
    }

    if ( !str_cmp( arg2, "edit" ) )
    {
        if ( !mprog )
        {
            send_to_char( "That mobile has no mob programs.\n\r", ch );
            return;
        }
        argument = one_argument( argument, arg4 );
        if ( arg4[0] != '\0' )
        {
            mptype = get_mpflag( arg4 );
            if ( mptype == -1 )
            {
                send_to_char( "Unknown program type, valid types are:\n\r", ch );
                for ( mptype = 0; mptype < NUM_PROG_TYPES; mptype++ )
                    ch_printf(ch, "%s ", mprog_flags[mptype]);
                return;
            }
        }
        else
            mptype = -1;
        if ( value < 1 )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }
        cnt = 0;
        for ( mprg = mprog; mprg; mprg = mprg->next )
        {
            if ( ++cnt == value )
            {
                mpedit( ch, mprg, mptype, argument );
                xCLEAR_BITS(victim->pIndexData->progtypes);
                for ( mprg = mprog; mprg; mprg = mprg->next )
                    xSET_BIT(victim->pIndexData->progtypes, mprg->progtype);
                return;
            }
        }
        send_to_char( "Program not found.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "delete" ) )
    {
        int num;
        bool found;

        if ( !mprog )
        {
            send_to_char( "That mobile has no mob programs.\n\r", ch );
            return;
        }
        argument = one_argument( argument, arg4 );
        if ( value < 1 )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }
        cnt = 0; found = FALSE;
        for ( mprg = mprog; mprg; mprg = mprg->next )
        {
            if ( ++cnt == value )
            {
                mptype = mprg->progtype;
                found = TRUE;
                break;
            }
        }
        if ( !found )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }
        cnt = num = 0;
        for ( mprg = mprog; mprg; mprg = mprg->next )
            if ( IS_SET( mprg->progtype, mptype ) )
                num++;
        if ( value == 1 )
        {
            mprg_next = victim->pIndexData->mudprogs;
            victim->pIndexData->mudprogs = mprg_next->next;
        }
        else
            for ( mprg = mprog; mprg; mprg = mprg_next )
            {
                mprg_next = mprg->next;
                if ( ++cnt == (value - 1) )
                {
                    mprg->next = mprg_next->next;
                    break;
                }
            }
        STRFREE( mprg_next->arglist );
        STRFREE( mprg_next->comlist );
        DISPOSE( mprg_next );
        if ( num <= 1 )
            xREMOVE_BIT( victim->pIndexData->progtypes, mptype );
        send_to_char( "Program removed.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "insert" ) )
    {
        if ( !mprog )
        {
            send_to_char( "That mobile has no mob programs.\n\r", ch );
            return;
        }
        argument = one_argument( argument, arg4 );
        mptype = get_mpflag( arg4 );
        if ( mptype == -1 )
        {
            send_to_char( "Unknown program type, valid types are:\n\r", ch );
            for ( mptype = 0; mptype < NUM_PROG_TYPES; mptype++ )
                ch_printf(ch, "%s ", mprog_flags[mptype]);
            return;
        }
        if ( value < 1 )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }
        if ( value == 1 )
        {
            CREATE( mprg, MPROG_DATA, 1 );
            xSET_BIT(victim->pIndexData->progtypes, mptype);
            mpedit( ch, mprg, mptype, argument );
            mprg->next = mprog;
            victim->pIndexData->mudprogs = mprg;
            return;
        }
        cnt = 1;
        for ( mprg = mprog; mprg; mprg = mprg->next )
        {
            if ( ++cnt == value && mprg->next )
            {
                CREATE( mprg_next, MPROG_DATA, 1 );
                xSET_BIT(victim->pIndexData->progtypes, mptype);
                mpedit( ch, mprg_next, mptype, argument );
                mprg_next->next = mprg->next;
                mprg->next	= mprg_next;
                return;
            }
        }
        send_to_char( "Program not found.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "add" ) )
    {
        mptype = get_mpflag( arg3 );
        if ( mptype == -1 )
        {
            send_to_char( "Unknown program type, valid types are:\n\r", ch );
            for ( mptype = 0; mptype < NUM_PROG_TYPES; mptype++ )
                ch_printf(ch, "%s ", mprog_flags[mptype]);
            return;
        }
        if ( mprog != NULL )
            for ( ; mprog->next; mprog = mprog->next );
        CREATE( mprg, MPROG_DATA, 1 );
        if ( mprog )
            mprog->next			= mprg;
        else
            victim->pIndexData->mudprogs	= mprg;
        xSET_BIT(victim->pIndexData->progtypes, mptype);
        mpedit( ch, mprg, mptype, argument );
        mprg->next = NULL;
        return;
    }

    do_mpedit( ch, "" );
}

void do_opedit( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char arg4 [MAX_INPUT_LENGTH];
    OBJ_DATA   *obj;
    MPROG_DATA *mprog, *mprg = NULL, *mprg_next = NULL;
    int value, mptype = 0, cnt;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mob's can't opedit\n\r", ch );
        return;
    }

    if ( !ch->desc )
    {
        send_to_char( "You have no descriptor\n\r", ch );
        return;
    }

    switch( ch->substate )
    {
    default:
        break;
    case SUB_MPROG_EDIT:
        if ( !ch->dest_buf )
        {
            send_to_char( "Fatal error: report to Thoric.\n\r", ch );
            bug( "do_opedit: sub_oprog_edit: NULL ch->dest_buf" );
            ch->substate = SUB_NONE;
            return;
        }
        mprog	 = (MPROG_DATA *)ch->dest_buf;
        if ( mprog->comlist )
            STRFREE( mprog->comlist );
        mprog->comlist = copy_buffer( ch );
        stop_editing( ch );
        return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    value = atoi( arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Syntax: opedit <object> <command> [number] <program> <value>\n\r", ch );
        send_to_char( "\n\r",						ch );
        send_to_char( "Command being one of:\n\r",			ch );
        send_to_char( "  add delete insert edit list\n\r",		ch );
        send_to_char( "Program being one of:\n\r",			ch );
        send_to_char( "  act speech rand wear remove sac zap get\n\r",  ch );
        send_to_char( "  drop damage repair greet exa use\n\r",ch );
        send_to_char( "  pull push (for levers,pullchains,buttons)\n\r",ch );
        send_to_char( "\n\r", ch);
        send_to_char( "Object should be in your inventory to edit.\n\r",ch);
        return;
    }

    if ( get_trust( ch ) < LEVEL_GOD )
    {
        if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
        {
            send_to_char( "You aren't carrying that.\n\r", ch );
            return;
        }
    }
    else
    {
        if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
        {
            send_to_char( "Nothing like that in all the realms.\n\r", ch );
            return;
        }
    }

    if ( !can_omodify( ch, obj ) )
        return;

    if ( !IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
    {
        send_to_char( "An object must have a prototype flag to be opset.\n\r", ch );
        return;
    }

    mprog = obj->pIndexData->mudprogs;

    set_char_color( AT_GREEN, ch );

    if ( !str_cmp( arg2, "list" ) )
    {
        cnt = 0;
        if ( !mprog )
        {
            send_to_char( "That object has no obj programs.\n\r", ch );
            return;
        }
        for ( mprg = mprog; mprg; mprg = mprg->next )
            ch_printf( ch, "%d>%s %s\n\r%s\n\r",
                       ++cnt,
                       mprog_type_to_name( mprg->progtype ),
                       mprg->arglist,
                       mprg->comlist );
        return;
    }

    if ( !str_cmp( arg2, "edit" ) )
    {
        if ( !mprog )
        {
            send_to_char( "That object has no obj programs.\n\r", ch );
            return;
        }
        argument = one_argument( argument, arg4 );
        if ( arg4[0] != '\0' )
        {
            mptype = get_mpflag( arg4 );
            if ( mptype == -1 )
            {
                send_to_char( "Unknown program type, valid types are:\n\r", ch );
                for ( mptype = 0; mptype < NUM_PROG_TYPES; mptype++ )
                    ch_printf(ch, "%s ", mprog_flags[mptype]);
                return;
            }
        }
        else
            mptype = -1;
        if ( value < 1 )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }
        cnt = 0;
        for ( mprg = mprog; mprg; mprg = mprg->next )
        {
            if ( ++cnt == value )
            {
                mpedit( ch, mprg, mptype, argument );
                xCLEAR_BITS(obj->pIndexData->progtypes);
                for ( mprg = mprog; mprg; mprg = mprg->next )
                    xSET_BIT(obj->pIndexData->progtypes, mprg->progtype);
                return;
            }
        }
        send_to_char( "Program not found.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "delete" ) )
    {
        int num;
        bool found;

        if ( !mprog )
        {
            send_to_char( "That object has no obj programs.\n\r", ch );
            return;
        }
        argument = one_argument( argument, arg4 );
        if ( value < 1 )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }
        cnt = 0; found = FALSE;
        for ( mprg = mprog; mprg; mprg = mprg->next )
        {
            if ( ++cnt == value )
            {
                mptype = mprg->progtype;
                found = TRUE;
                break;
            }
        }
        if ( !found )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }
        cnt = num = 0;
        for ( mprg = mprog; mprg; mprg = mprg->next )
            if ( IS_SET( mprg->progtype, mptype ) )
                num++;
        if ( value == 1 )
        {
            mprg_next = obj->pIndexData->mudprogs;
            obj->pIndexData->mudprogs = mprg_next->next;
        }
        else
            for ( mprg = mprog; mprg; mprg = mprg_next )
            {
                mprg_next = mprg->next;
                if ( ++cnt == (value - 1) )
                {
                    mprg->next = mprg_next->next;
                    break;
                }
            }
        STRFREE( mprg_next->arglist );
        STRFREE( mprg_next->comlist );
        DISPOSE( mprg_next );
        if ( num <= 1 )
            xREMOVE_BIT( obj->pIndexData->progtypes, mptype );
        send_to_char( "Program removed.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "insert" ) )
    {
        if ( !mprog )
        {
            send_to_char( "That object has no obj programs.\n\r", ch );
            return;
        }
        argument = one_argument( argument, arg4 );
        mptype = get_mpflag( arg4 );
        if ( mptype == -1 )
        {
            send_to_char( "Unknown program type, valid types are:\n\r", ch );
            for ( mptype = 0; mptype < NUM_PROG_TYPES; mptype++ )
                ch_printf(ch, "%s ", mprog_flags[mptype]);
            return;
        }
        if ( value < 1 )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }
        if ( value == 1 )
        {
            CREATE( mprg, MPROG_DATA, 1 );
            xSET_BIT(obj->pIndexData->progtypes, mptype);
            mpedit( ch, mprg, mptype, argument );
            mprg->next = mprog;
            obj->pIndexData->mudprogs = mprg;
            return;
        }
        cnt = 1;
        for ( mprg = mprog; mprg; mprg = mprg->next )
        {
            if ( ++cnt == value && mprg->next )
            {
                CREATE( mprg_next, MPROG_DATA, 1 );
                xSET_BIT(obj->pIndexData->progtypes, mptype);
                mpedit( ch, mprg_next, mptype, argument );
                mprg_next->next = mprg->next;
                mprg->next	= mprg_next;
                return;
            }
        }
        send_to_char( "Program not found.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "add" ) )
    {
        mptype = get_mpflag( arg3 );
        if ( mptype == -1 )
        {
            send_to_char( "Unknown program type, valid types are:\n\r", ch );
            for ( mptype = 0; mptype < NUM_PROG_TYPES; mptype++ )
                ch_printf(ch, "%s ", mprog_flags[mptype]);
            return;
        }
        if ( mprog != NULL )
            for ( ; mprog->next; mprog = mprog->next );
        CREATE( mprg, MPROG_DATA, 1 );
        if ( mprog )
            mprog->next			 = mprg;
        else
            obj->pIndexData->mudprogs	 = mprg;
        xSET_BIT(obj->pIndexData->progtypes, mptype);
        mpedit( ch, mprg, mptype, argument );
        mprg->next = NULL;
        return;
    }

    do_opedit( ch, "" );
}



/*
 * RoomProg Support
 */
void rpedit( CHAR_DATA *ch, MPROG_DATA *mprg, int mptype, char *argument )
{
    if ( mptype != -1 )
    {
        mprg->progtype = 1 << mptype;
        if ( mprg->arglist )
            STRFREE( mprg->arglist );
        mprg->arglist = STRALLOC( argument );
    }
    ch->substate = SUB_MPROG_EDIT;
    ch->dest_buf = mprg;
    if(!mprg->comlist)
        mprg->comlist = STRALLOC("");
    start_editing( ch, mprg->comlist );
    return;
}

void do_rpedit( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    MPROG_DATA *mprog, *mprg = NULL, *mprg_next = NULL;
    int value, mptype = 0, cnt;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mob's can't rpedit\n\r", ch );
        return;
    }

    if ( !ch->desc )
    {
        send_to_char( "You have no descriptor\n\r", ch );
        return;
    }

    switch( ch->substate )
    {
    default:
        break;
    case SUB_MPROG_EDIT:
        if ( !ch->dest_buf )
        {
            send_to_char( "Fatal error: report to Thoric.\n\r", ch );
            bug( "do_opedit: sub_oprog_edit: NULL ch->dest_buf" );
            ch->substate = SUB_NONE;
            return;
        }
        mprog	 = (MPROG_DATA *)ch->dest_buf;
        if ( mprog->comlist )
            STRFREE( mprog->comlist );
        mprog->comlist = copy_buffer( ch );
        stop_editing( ch );
        return;
    }

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    value = atoi( arg2 );
    /* argument = one_argument( argument, arg3 ); */

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: rpedit <command> [number] <program> <value>\n\r", ch );
        send_to_char( "\n\r",						ch );
        send_to_char( "Command being one of:\n\r",			ch );
        send_to_char( "  add delete insert edit list\n\r",		ch );
        send_to_char( "Program being one of:\n\r",			ch );
        send_to_char( "  act speech rand sleep rest rfight entry\n\r",  ch );
        send_to_char( "  leave death\n\r",                              ch );
        send_to_char( "\n\r",						ch );
        send_to_char( "You should be standing in room you wish to edit.\n\r",ch);
        return;
    }

    if ( !can_rmodify( ch, ch->in_room ) )
        return;

    mprog = ch->in_room->mudprogs;

    set_char_color( AT_GREEN, ch );

    if ( !str_cmp( arg1, "list" ) )
    {
        cnt = 0;
        if ( !mprog )
        {
            send_to_char( "This room has no room programs.\n\r", ch );
            return;
        }
        for ( mprg = mprog; mprg; mprg = mprg->next )
            ch_printf( ch, "%d>%s %s\n\r%s\n\r",
                       ++cnt,
                       mprog_type_to_name( mprg->progtype ),
                       mprg->arglist,
                       mprg->comlist );
        return;
    }

    if ( !str_cmp( arg1, "edit" ) )
    {
        if ( !mprog )
        {
            send_to_char( "This room has no room programs.\n\r", ch );
            return;
        }
        argument = one_argument( argument, arg3 );
        if ( arg3[0] != '\0' )
        {
            mptype = get_mpflag( arg3 );
            if ( mptype == -1 )
            {
                send_to_char( "Unknown program type, valid types are:\n\r", ch );
                for ( mptype = 0; mptype < NUM_PROG_TYPES; mptype++ )
                    ch_printf(ch, "%s ", mprog_flags[mptype]);
                return;
            }
        }
        else
            mptype = -1;
        if ( value < 1 )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }
        cnt = 0;
        for ( mprg = mprog; mprg; mprg = mprg->next )
        {
            if ( ++cnt == value )
            {
                mpedit( ch, mprg, mptype, argument );
                xCLEAR_BITS(ch->in_room->progtypes);
                for ( mprg = mprog; mprg; mprg = mprg->next )
                    xSET_BIT(ch->in_room->progtypes, mprg->progtype);
                return;
            }
        }
        send_to_char( "Program not found.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "delete" ) )
    {
        int num;
        bool found;

        if ( !mprog )
        {
            send_to_char( "That room has no room programs.\n\r", ch );
            return;
        }
        argument = one_argument( argument, arg3 );
        if ( value < 1 )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }
        cnt = 0; found = FALSE;
        for ( mprg = mprog; mprg; mprg = mprg->next )
        {
            if ( ++cnt == value )
            {
                mptype = mprg->progtype;
                found = TRUE;
                break;
            }
        }
        if ( !found )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }
        cnt = num = 0;
        for ( mprg = mprog; mprg; mprg = mprg->next )
            if ( IS_SET( mprg->progtype, mptype ) )
                num++;
        if ( value == 1 )
        {
            mprg_next = ch->in_room->mudprogs;
            ch->in_room->mudprogs = mprg_next->next;
        }
        else
            for ( mprg = mprog; mprg; mprg = mprg_next )
            {
                mprg_next = mprg->next;
                if ( ++cnt == (value - 1) )
                {
                    mprg->next = mprg_next->next;
                    break;
                }
            }
        STRFREE( mprg_next->arglist );
        STRFREE( mprg_next->comlist );
        DISPOSE( mprg_next );
        if ( num <= 1 )
            xREMOVE_BIT( ch->in_room->progtypes, mptype );
        send_to_char( "Program removed.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "insert" ) )
    {
        if ( !mprog )
        {
            send_to_char( "That room has no room programs.\n\r", ch );
            return;
        }
        argument = one_argument( argument, arg3 );
        mptype = get_mpflag( arg2 );
        if ( mptype == -1 )
        {
            send_to_char( "Unknown program type, valid types are:\n\r", ch );
            for ( mptype = 0; mptype < NUM_PROG_TYPES; mptype++ )
                ch_printf(ch, "%s ", mprog_flags[mptype]);
            return;
        }
        if ( value < 1 )
        {
            send_to_char( "Program not found.\n\r", ch );
            return;
        }
        if ( value == 1 )
        {
            CREATE( mprg, MPROG_DATA, 1 );
            xSET_BIT(ch->in_room->progtypes, mptype);
            mpedit( ch, mprg, mptype, argument );
            mprg->next = mprog;
            ch->in_room->mudprogs = mprg;
            return;
        }
        cnt = 1;
        for ( mprg = mprog; mprg; mprg = mprg->next )
        {
            if ( ++cnt == value && mprg->next )
            {
                CREATE( mprg_next, MPROG_DATA, 1 );
                xSET_BIT(ch->in_room->progtypes, mptype);
                mpedit( ch, mprg_next, mptype, argument );
                mprg_next->next = mprg->next;
                mprg->next	= mprg_next;
                return;
            }
        }
        send_to_char( "Program not found.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "add" ) )
    {
        mptype = get_mpflag( arg2 );
        if ( mptype == -1 )
        {
            send_to_char( "Unknown program type, valid types are:\n\r", ch );
            for ( mptype = 0; mptype < NUM_PROG_TYPES; mptype++ )
                ch_printf(ch, "%s ", mprog_flags[mptype]);
            return;
        }
        if ( mprog )
            for ( ; mprog->next; mprog = mprog->next );
        CREATE( mprg, MPROG_DATA, 1 );
        if ( mprog )
            mprog->next		= mprg;
        else
            ch->in_room->mudprogs	= mprg;
        xSET_BIT(ch->in_room->progtypes, mptype);
        mpedit( ch, mprg, mptype, argument );
        mprg->next = NULL;
        return;
    }

    do_rpedit( ch, "" );
}

void do_rdelete( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Delete which room?\n\r", ch );
        return;
    }

    /* Find the room. */
    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    /* Does the player have the right to delete this room? */
    if ( get_trust( ch ) < sysdata.level_modify_proto
         && ( location->vnum < ch->pcdata->r_range_lo ||
              location->vnum > ch->pcdata->r_range_hi ) )
    {
        send_to_char( "That room is not in your assigned range.\n\r", ch );
        return;
    }

    /* We could go to the trouble of clearing out the room, but why? */
    if ( location->first_person || location->first_content )
    {
        send_to_char( "The room must be empty first.\n\r", ch );
        return;
    }

    /* Ok, we've determined that the room exists, it is empty and the
     player has the authority to delete it, so let's dump the thing.
     The function to do it is in db.c so it can access the top-room
     variable. */
    delete_room( location );

    send_to_char( "Room deleted.\n\r", ch );
    return;
}

void do_asave( CHAR_DATA *ch, char *argument )
{
}

void do_odelete( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;
    OBJ_INDEX_DATA *obj;
    OBJ_DATA *temp = NULL;

    argument = one_argument( argument, arg );

    if (ch->substate == SUB_RESTRICTED)
    {
        send_to_char("You can't do that while in 'xset on'\n\r", ch);
        return;
    }

    if ( arg[0] == '\0' )
    {
        send_to_char( "Delete which object?\n\r", ch );
        return;
    }

    /* Find the object. */
    if (!(obj = get_obj_index(atoi(arg))))
    {
        if (!(temp = get_obj_here(ch, arg)))
        {
            send_to_char( "No such object.\n\r", ch );
            return;
        }
        obj = temp->pIndexData;
    }

    /* Does the player have the right to delete this room? */
    if ( get_trust( ch ) < sysdata.level_modify_proto
         && ( obj->ivnum < ch->pcdata->o_range_lo ||
              obj->ivnum > ch->pcdata->o_range_hi ) )
    {
        send_to_char( "That object is not in your assigned range.\n\r", ch );
        return;
    }

    for (vch=first_char;vch;vch=vch->next)
        if (!IS_NPC(vch) && vch->dest_buf == temp)
        {
            ch_printf(ch, "%s is editing that, you cannot delete it.\n\r", GET_NAME(vch));
            return;
        }

    /* Ok, we've determined that the room exists, it is empty and the
     player has the authority to delete it, so let's dump the thing.
     The function to do it is in db.c so it can access the top-room
     variable. */
    delete_obj( obj );

    send_to_char( "Object deleted.\n\r", ch );
    return;
}

void do_mdelete( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *mob;
    CHAR_DATA *vch;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Delete which mob?\n\r", ch );
        return;
    }

    /* Find the mob. */
    if (!(mob = get_mob_index(atoi(arg))))
    {
        send_to_char( "No such mob.\n\r", ch );
        return;
    }

    /* Does the player have the right to delete this room? */
    if ( get_trust( ch ) < sysdata.level_modify_proto
         && ( mob->ivnum < ch->pcdata->m_range_lo ||
              mob->ivnum > ch->pcdata->m_range_hi ) )
    {
        send_to_char( "That mob is not in your assigned range.\n\r", ch );
        return;
    }

   for (vch=first_char;vch;vch=vch->next)
        if (vch->pIndexData == mob)
        {
	    send_to_char("Purge all copies of this mob first.\n\r", ch);
            return;
        }

    /* Ok, we've determined that the mob exists and the player has the
     authority to delete it, so let's dump the thing.
     The function to do it is in db.c so it can access the top_mob_index
     variable. */
    delete_mob( mob );

    send_to_char( "Mob deleted.\n\r", ch );
    return;
}

/*
 * function to allow modification of an area's climate
 * Last modified: July 15, 1997
 * Fireblade
 */
void do_climate(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    AREA_DATA *area;

    /* Little error checking */
    if(!ch)
    {
        bug("do_climate: NULL character.");
        return;
    }
    else if(!ch->in_room)
    {
        bug("do_climate: character not in a room.");
        return;
    }
    else if(!ch->in_room->area)
    {
        bug("do_climate: character not in an area.");
        return;
    }
    else if(!ch->in_room->area->weather)
    {
        bug("do_climate: area with NULL weather data.");
        return;
    }

    set_char_color(AT_PLAIN, ch);

    area = ch->in_room->area;

    argument = strlower(argument);
    argument = one_argument(argument, arg);

    /* Display current climate settings */
    if(arg[0] == '\0')
    {
        NEIGHBOR_DATA *neigh;

        ch_printf(ch, "%s:\n\r", area->name);
        ch_printf(ch, "\tTemperature:\t%s\n\r",
                  temp_settings[area->weather->climate_temp]);
        ch_printf(ch, "\tPrecipitation:\t%s\n\r",
                  precip_settings[area->weather->climate_precip]);
        ch_printf(ch, "\tWind:\t\t%s\n\r",
                  wind_settings[area->weather->climate_wind]);

        if(area->weather->first_neighbor)
            ch_printf(ch, "\n\rNeighboring weather systems:\n\r");

        for(neigh = area->weather->first_neighbor; neigh;
            neigh = neigh->next)
        {
            ch_printf(ch, "\t%s\n\r", neigh->name);
        }

        return;
    }
    /* set climate temperature */
    else if(!str_cmp(arg, "temp"))
    {
        int i;
        argument = one_argument(argument, arg);

        for(i = 0; i < MAX_CLIMATE; i++)
        {
            if(str_cmp(arg, temp_settings[i]))
                continue;

            area->weather->climate_temp = i;
            ch_printf(ch, "The climate temperature "
                      "for %s is now %s.\n\r", area->name,
                      temp_settings[i]);
            break;
        }

        if(i == MAX_CLIMATE)
        {
            ch_printf(ch, "Possible temperature "
                      "settings:\n\r");
            for(i = 0; i < MAX_CLIMATE; i++)
            {
                ch_printf(ch,"\t%s\n\r",
                          temp_settings[i]);
            }
        }

        return;
    }
    /* set climate precipitation */
    else if(!str_cmp(arg, "precip"))
    {
        int i;
        argument = one_argument(argument, arg);

        for(i = 0; i < MAX_CLIMATE; i++)
        {
            if(str_cmp(arg, precip_settings[i]))
                continue;

            area->weather->climate_precip = i;
            ch_printf(ch, "The climate precipitation "
                      "for %s is now %s.\n\r", area->name,
                      precip_settings[i]);
            break;
        }

        if(i == MAX_CLIMATE)
        {
            ch_printf(ch, "Possible precipitation "
                      "settings:\n\r");
            for(i = 0; i < MAX_CLIMATE; i++)
            {
                ch_printf(ch, "\t%s\n\r",
                          precip_settings[i]);
            }
        }

        return;
    }
    /* set climate wind */
    else if(!str_cmp(arg, "wind"))
    {
        int i;
        argument = one_argument(argument, arg);

        for(i = 0; i < MAX_CLIMATE; i++)
        {
            if(str_cmp(arg, wind_settings[i]))
                continue;

            area->weather->climate_wind = i;
            ch_printf(ch, "The climate wind for %s "
                      "is now %s.\n\r", area->name,
                      wind_settings[i]);
            break;
        }

        if(i == MAX_CLIMATE)
        {
            ch_printf(ch, "Possible wind settings:\n\r");
            for(i = 0; i < MAX_CLIMATE; i++)
            {
                ch_printf(ch, "\t%s\n\r",
                          wind_settings[i]);
            }
        }

        return;
    }
    /* add or remove neighboring weather systems */
    else if(!str_cmp(arg, "neighbor"))
    {
        NEIGHBOR_DATA *neigh;
        AREA_DATA *tarea;

        if(argument[0] == '\0')
        {
            ch_printf(ch, "Add or remove which area?\n\r");
            return;
        }

        /* look for a matching list item */
        for(neigh = area->weather->first_neighbor; neigh;
            neigh = neigh->next)
        {
            if(nifty_is_name(argument, neigh->name))
                break;
        }

        /* if the a matching list entry is found, remove it */
        if(neigh)
        {
            /* look for the neighbor area in question */
            if(! (tarea = neigh->address) )
                tarea = get_area(neigh->name);

            /* if there is an actual neighbor area */
            /* remove its entry to this area */
            if(tarea)
            {
                NEIGHBOR_DATA *tneigh;

                tarea = neigh->address;
                for(tneigh = tarea->weather->first_neighbor;
                    tneigh; tneigh = tneigh->next)
                {
                    if(!strcmp(area->name, tneigh->name))
                        break;
                }

                UNLINK(tneigh,
                       tarea->weather->first_neighbor,
                       tarea->weather->last_neighbor,
                       next,
                       prev);
                STRFREE(tneigh->name);
                DISPOSE(tneigh);
            }

            UNLINK(neigh,
                   area->weather->first_neighbor,
                   area->weather->last_neighbor,
                   next,
                   prev);
            ch_printf(ch,"The weather in %s and %s "
                      "no longer affect each other.\n\r",
                      neigh->name, area->name);
            STRFREE(neigh->name);
            DISPOSE(neigh);
        }
        /* otherwise add an entry */
        else
        {
            tarea = get_area(argument);

            if(!tarea)
            {
                ch_printf(ch, "No such area exists.\n\r");
                return;
            }
            else if(tarea == area)
            {
                ch_printf(ch, "%s already affects its "
                          "own weather.\n\r", area->name);
                return;
            }

            /* add the entry */
            CREATE(neigh, NEIGHBOR_DATA, 1);
            neigh->name = STRALLOC(tarea->name);
            neigh->address = tarea;
            LINK(neigh,
                 area->weather->first_neighbor,
                 area->weather->last_neighbor,
                 next,
                 prev);

            /* add an entry to the neighbor's list */
            CREATE(neigh, NEIGHBOR_DATA, 1);
            neigh->name = STRALLOC(area->name);
            neigh->address = area;
            LINK(neigh,
                 tarea->weather->first_neighbor,
                 tarea->weather->last_neighbor,
                 next,
                 prev);

            ch_printf(ch, "The weather in %s and %s now "
                      "affect one another.\n\r", tarea->name,
                      area->name);
        }

        return;
    }
    else
    {
        ch_printf(ch, "Climate may only be followed by one "
                  "of the following fields:\n\r");
        ch_printf(ch, "\ttemp\n\r\tprecip\n\r\twind\n\r\tneighbor\n\r");

        return;
    }
}


#ifdef USE_ASPELL
void aspell_string(CHAR_DATA *ch, char *str)
{
    extern AspellConfig *spell_config;
    AspellCanHaveError *ret, *docret;
    AspellSpeller *speller;
    AspellDocumentChecker *checker;
    AspellToken token;
    const char *word;
    char s1[16], s2[16];
    unsigned int len, lastoffset;

    if (!str || !*str)
    {
        send_to_char("No string given.\n\r", ch);
	return;
    }

    ret = new_aspell_speller(spell_config);

    if (aspell_error_number(ret) != 0)
    {
	ch_printf(ch, "Error: %s", aspell_error_message(ret));
        delete_aspell_can_have_error(ret);
        return;
    }

    speller = to_aspell_speller(ret);
    docret = new_aspell_document_checker(speller);

    if (aspell_error_number(docret) != 0)
    {
	ch_printf(ch, "Doc Error: %s", aspell_error_message(docret));
	delete_aspell_can_have_error(docret);
	return;
    }

    checker = to_aspell_document_checker(docret);

    len = strlen(str);
    aspell_document_checker_process(checker, str, len);

    sprintf(s1,"%s",color_str(AT_PLAIN,ch));
    sprintf(s2,"%s",color_str(AT_DANGER,ch));

    send_to_char(s1, ch);

    lastoffset=0;
    while (1)
    {
	token = aspell_document_checker_next_misspelling(checker);
	if (token.len == 0)
	    break;
	word = str+token.offset;
	if (lastoffset < token.offset)
	    ch_printf(ch, "%.*s", token.offset-lastoffset, str+lastoffset);
	ch_printf(ch, "%s%.*s%s", s2, token.len, word, s1);
	lastoffset = token.offset + token.len;
    }
    if (lastoffset==0)
	send_to_char("Spelling is OK.", ch);
    else if (lastoffset < len)
	ch_printf(ch, "%.*s", len-lastoffset, str+lastoffset);

    send_to_char("\n\r", ch);

    delete_aspell_document_checker(checker);
    delete_aspell_speller(speller);
}

void aspell_word(CHAR_DATA *ch, char *str)
{
    extern AspellConfig *spell_config;
    AspellCanHaveError *ret;
    AspellSpeller *speller;
    const AspellWordList *suggestions;
    AspellStringEnumeration *elements;
    const char *word;
    int correct, count=0;

    if (!str || !*str)
    {
        send_to_char("No word given.\n\r", ch);
	return;
    }

    ret = new_aspell_speller(spell_config);

    if (aspell_error_number(ret) != 0)
    {
	ch_printf(ch, "Error: %s", aspell_error_message(ret));
        delete_aspell_can_have_error(ret);
        return;
    }

    speller = to_aspell_speller(ret);

    if ((correct = aspell_speller_check(speller, str, -1)) != 0)
    {
	send_to_char("Spelling is OK.\n\r", ch);
	delete_aspell_speller(speller);
	return;
    }

    suggestions = aspell_speller_suggest(speller, str, -1);
    elements = aspell_word_list_elements(suggestions);

    send_to_char("Suggestions:\n\r", ch);
    while ((word = aspell_string_enumeration_next(elements)) != 0)
    {
	ch_printf(ch, "%s\n\r", word);
	if (count++ == 20)
            break;
    }

    delete_aspell_string_enumeration(elements);
    delete_aspell_speller(speller);
}
#endif

void do_aspell(CHAR_DATA *ch, char *argument)
{
#ifndef USE_ASPELL
    send_to_char("Not enabled.\n\r", ch);
    return;
#else
    ROOM_INDEX_DATA *room;
    OBJ_INDEX_DATA *obj;
    MOB_INDEX_DATA *mob;
    char *str, *temp_arg;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int vnum=0;

    if (!argument || !*argument)
    {
        send_to_char("Aspell what?\n\r", ch);
        return;
    }

    temp_arg = argument;
    temp_arg = one_argument(temp_arg, arg1);
    temp_arg = one_argument(temp_arg, arg2);

    if (arg2[0] != '\0')
        vnum = atoi(arg2);

    if (!str_cmp(arg1, "room"))
    {
	if (!vnum)
	    room = ch->in_room;
	else
	    room = get_room_index(vnum);
	if (!room)
	{
            send_to_char("Room not found.\n\r", ch);
            return;
	}
	str = room->description;
    }
    else if (!str_cmp(arg1, "obj"))
    {
	obj = get_obj_index(vnum);
	if (!obj)
	{
	    send_to_char("Object not found.\n\r", ch);
	    return;
	}
        str = obj->description;
    }
    else if (!str_cmp(arg1, "mob"))
    {
	mob = get_mob_index(vnum);
	if (!mob)
	{
            send_to_char("Mobile not found.\n\r", ch);
            return;
	}
        str = mob->description;
    }
    else
	str = argument;

    if (!str || !*str)
    {
        send_to_char("Nothing to check.\n\r", ch);
        return;
    }

    if (strchr(str, ' '))
	aspell_string(ch, str);
    else
        aspell_word(ch, str);
#endif
}

