#define FREAD_LINE 1
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
 * 			Database management module			    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: db.c,v 1.163 2004/04/06 22:00:09 dotd Exp $";*/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "mud.h"
#include "gsn.h"

#include "hs.h"
#include "recylist.h"

#include "property.h"
#include "currency.h"
#include "christen.h"
#include "rareobj.h"
#include "quest.h"
#include "bugtrack.h"
#include "fcompress.h"

#ifdef THREADED_AREA_LOAD
#include <pthread.h>
#include <semaphore.h>
#endif
#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#define DEBUG 1

extern	int	_filbuf		args( (FILE *) );

#if defined(KEY)
#undef KEY
#endif

void init_supermob(void);
void init_acro(void);
void init_chess(void);

#define KEY( literal, field, value )					\
    if ( !str_cmp( word, literal ) )	\
    {					\
    field  = value;			\
    fMatch = TRUE;			\
    break;				\
    }


/*
 * Globals.
 */

WIZENT *	first_wiz;
WIZENT *	last_wiz;

time_t                  last_restore_all_time = 0;

HELP_DATA *		first_help;
HELP_DATA *		last_help;

SHOP_DATA *		first_shop;
SHOP_DATA *		last_shop;

REPAIR_DATA *		first_repair;
REPAIR_DATA *		last_repair;

TELEPORT_DATA *		first_teleport;
TELEPORT_DATA *		last_teleport;

OBJ_DATA *		extracted_obj_queue;
EXTRACT_CHAR_DATA *	extracted_char_queue;

CHAR_DATA *		first_char;
CHAR_DATA *		last_char;

char *			help_greeting;
char			log_buf		[4*MAX_STRING_LENGTH];
char			bug_buf		[4*MAX_STRING_LENGTH];

OBJ_DATA *		first_object;
OBJ_DATA *		last_object;
TIME_INFO_DATA		time_info;
WEATHER_DATA		weather_info;

int			weath_unit;	/* global weather param */
int			rand_factor;
int			climate_factor;
int			neigh_factor;
int			max_vector;

int			nummobsloaded;
int			numobjsloaded;
int			physicalobjects;

MAP_INDEX_DATA  *       first_map;	/* maps */

AUCTION_DATA    * 	auction;	/* auctions */


/* for searching */
int	gsn_top_sn;
int	gsn_first_lore;
int	gsn_first_skill;
int	gsn_first_spell;
int	gsn_first_tongue;
int	gsn_first_weapon;
int     gsn_first_psispell;

/* --- */
int gsn_avoid_back_attack;
int gsn_detect_invis;
int gsn_enlarge;
int gsn_detect_evil;
int gsn_detect_magic;
int gsn_sense_life;
int gsn_sanctuary;
int gsn_poly;
int gsn_protection_from_evil;
int gsn_infravision;
int gsn_charm_person;
int gsn_weakness;
int gsn_strength;
int gsn_armor;
int gsn_detect_poison;
int gsn_bless;
int gsn_fly;
int gsn_water_breath;
int gsn_fireshield;
int gsn_faerie_fire;
int gsn_minor_track;
int gsn_major_track;
int gsn_web;
int gsn_silence;
int gsn_tree_travel;
int gsn_haste;
int gsn_slow;
int gsn_barkskin;
int gsn_aid;
int gsn_true_sight;
int gsn_invis_to_animals;
int gsn_dragon_ride;
int gsn_darkness;
int gsn_minor_invulnerability;
int gsn_major_invulnerability;
int gsn_protection_from_energy_drain;
int gsn_wizardeye;
int gsn_protection_from_breath;
int gsn_protection_from_fire_breath;
int gsn_protection_from_frost_breath;
int gsn_protection_from_electric_breath;
int gsn_protection_from_acid_breath;
int gsn_protection_from_gas_breath;
int gsn_anti_magic_shell;
int gsn_paralyze;
int gsn_curse;

int gsn_climb;
int gsn_doorbash;
int gsn_dual_wield;
int gsn_possess;
int gsn_blindness;
int gsn_poison;
int gsn_backstab;
int gsn_berserk;
int gsn_fireball;
int gsn_chill_touch;
int gsn_lightning_bolt;
int gsn_sleep;
int gsn_group_invis;
int gsn_invis;
int gsn_sneak;
int gsn_hunt;
int gsn_hide;
int gsn_steal;
int gsn_spot;
int gsn_bash;
int gsn_mount;
int gsn_disarm;
int gsn_kick;
int gsn_brew;
int gsn_cook;
int gsn_shield;
int gsn_swim;
int gsn_retreat;
int gsn_spy;
int gsn_travelling;

int gsn_comprehend_lang;
int gsn_esp;
int gsn_memorize;

/* had / need looking at */
int gsn_find_traps;
int gsn_detrap;
int gsn_pick_lock;
int gsn_dodge;
int gsn_blast;
int gsn_archery;
int gsn_rescue;
int gsn_quivering_palm;

/* we want / might want */
int gsn_feed;
int gsn_gouge;
int gsn_search;
int gsn_dig;
int gsn_parry;
int gsn_stun;
int gsn_punch;
int gsn_grip;
int gsn_scribe;
int gsn_poison_weapon;

/* zoso? */
int gsn_claw;
int gsn_bite;
int gsn_sting;
int gsn_tail;
int gsn_slice;

int gsn_protection_from_fire;
int gsn_protection_from_cold;
int gsn_protection_from_energy;
int gsn_protection_from_electricity;
int gsn_cure_blindness;
int gsn_cure_poison;
int gsn_stone_skin;
int gsn_refresh;
int gsn_cure_light;
int gsn_heal;

int gsn_energy_drain;
int gsn_earthquake;
int gsn_flamestrike;
int gsn_harm;
int gsn_colour_spray;
int gsn_weaken;
int gsn_cure_serious;
int gsn_cure_critical;
int gsn_dispel_magic;
int gsn_dispel_evil;

int gsn_meditate;
int gsn_psiportal;
int gsn_scry;
int gsn_doorway;
int gsn_tan;
int gsn_canibalize;
int gsn_psishield;
int gsn_mindblank;
int gsn_tower_of_iron_will;
int gsn_great_sight;
int gsn_psistrength;
int gsn_chameleon;

int gsn_slow_poison;
int gsn_cause_light;
int gsn_cause_serious;
int gsn_cause_critical;
int gsn_acid_blast;

int gsn_juggernaut;
int gsn_pray;
int gsn_read_magic;
int gsn_babel;
int gsn_drinking;

int gsn_animal_lore;
int gsn_demonology;
int gsn_giant_lore;
int gsn_necromancy;
int gsn_other_lore;
int gsn_people_lore;
int gsn_reptile_lore;
int gsn_vegetable_lore;

int gsn_feeblemind;
int gsn_mindwipe;

/*
 * Locals.
 */
MOB_INDEX_DATA *	mob_index_hash		[MAX_KEY_HASH];
OBJ_INDEX_DATA *	obj_index_hash		[MAX_KEY_HASH];
ROOM_INDEX_DATA *	room_index_hash		[MAX_KEY_HASH];

AREA_DATA *		first_area;
AREA_DATA *		last_area;
AREA_DATA *		first_build;
AREA_DATA *		last_build;
AREA_DATA *		first_asort;
AREA_DATA *		last_asort;
AREA_DATA *		first_bsort;
AREA_DATA *		last_bsort;

SYSTEM_DATA		sysdata;

int                     char_affects;
int			top_affect;
int			top_area;
int			top_ed;
int			top_exit;
int			top_help;
int			top_mob_index;
int			top_obj_index;
int			top_reset;
int			top_room;
int			top_shop;
int			top_repair;
int			top_vroom;
int                     top_mob_vnum;
int                     top_obj_vnum;
int                     top_room_vnum;

unsigned int unum;

/*
 * Semi-locals.
 */
bool			fBootDb;
FILE *                  fpArea;
char			strArea[MAX_INPUT_LENGTH];


/*
 * Local booting procedures.
 */
void	init_mm		args( ( void ) );

#ifdef USE_DB
void db_load_areas();
void db_load_helps();
#else
static void	load_mobiles	args( ( AREA_DATA *tarea, gzFile gzfp ) );
static void	load_objects	args( ( AREA_DATA *tarea, gzFile gzfp ) );
static void	load_rooms	args( ( AREA_DATA *tarea, gzFile gzfp ) );
static void	load_shops	args( ( AREA_DATA *tarea, gzFile gzfp ) );
static void 	load_repairs	args( ( AREA_DATA *tarea, gzFile gzfp ) );
static void	load_climate	args( ( AREA_DATA *tarea, gzFile gzfp ) );
static void	load_specials	args( ( AREA_DATA *tarea, gzFile gzfp ) );
static void	load_area	args( ( AREA_DATA *tarea, gzFile gzfp ) );
static void	load_resets	args( ( AREA_DATA *tarea, gzFile gzfp ) );
static void     load_author     args( ( AREA_DATA *tarea, gzFile gzfp ) );
static void     load_economy    args( ( AREA_DATA *tarea, gzFile gzfp ) );
static void     load_areacurr   args( ( AREA_DATA *tarea, gzFile gzfp ) );
static void     load_plane      args( ( AREA_DATA *tarea, gzFile gzfp ) );
static void     load_higheconomy args( ( AREA_DATA *tarea, gzFile gzfp ) );
static void     load_loweconomy args( ( AREA_DATA *tarea, gzFile gzfp ) );
static void	load_resetmsg	args( ( AREA_DATA *tarea, gzFile gzfp ) ); /* Rennard */
static void     load_flags      args( ( AREA_DATA *tarea, gzFile gzfp ) );
static void     load_ranges	args( ( AREA_DATA *tarea, gzFile gzfp ) );
static void	load_helps	args( ( AREA_DATA *tarea, gzFile gzfp ) );
static void     load_comment    args( ( AREA_DATA *tarea, gzFile gzfp ) );
#endif
static void	load_buildlist	args( ( void ) );
static void     load_version    args( ( AREA_DATA *tarea, gzFile gzfp ) );

bool	load_systemdata    	args( ( SYSTEM_DATA *sys ) );
void    load_banlist            args( ( void ) );
void	initialize_economy      args( ( void ) );

void    load_currency           args( ( void ) );

void	fix_exits	args( ( void ) );
void	load_weatherdata args( ( void ) );

/*
 * External booting function
 */
void	load_corpses	args( ( void ) );
void	renumber_put_resets	args( ( AREA_DATA *pArea ) );
void	load_properties	args( ( void ) );
void	load_commodities args( ( void ) );
void    orphan_room_search args( ( void ) );
/*
 * MUDprogram locals
 */

#ifndef USE_DB
static MPROG_DATA *	mprog_file_read 	args ( ( char* f, MPROG_DATA* mprg,
                                                         MOB_INDEX_DATA *pMobIndex ) );
/* static int 		oprog_name_to_type	args ( ( char* name ) ); */
static MPROG_DATA *	oprog_file_read 	args ( ( char* f, MPROG_DATA* mprg,
                                                         OBJ_INDEX_DATA *pObjIndex ) );
/* static int 		rprog_name_to_type	args ( ( char* name ) ); */
static MPROG_DATA *	rprog_file_read 	args ( ( char* f, MPROG_DATA* mprg,
                                                         ROOM_INDEX_DATA *pRoomIndex ) );
static void		load_mudprogs           args ( ( AREA_DATA *tarea, gzFile gzfp ) );
static void		load_objprogs           args ( ( AREA_DATA *tarea, gzFile gzfp ) );
static void   		mprog_read_programs     args ( ( gzFile fp,
                                                         MOB_INDEX_DATA *pMobIndex) );
static void   		oprog_read_programs     args ( ( gzFile fp,
                                                         OBJ_INDEX_DATA *pObjIndex) );
static void   		rprog_read_programs     args ( ( gzFile fp,
                                                         ROOM_INDEX_DATA *pRoomIndex) );
#endif

void shutdown_mud( char *reason )
{
    FILE *fp;

    if ( (fp = fopen( SHUTDOWN_FILE, "a" )) != NULL )
    {
        fprintf( fp, "%s\n", reason );
        FCLOSE( fp );
    }
}

#ifdef THREADED_AREA_LOAD
#define NUM_WORKER_THREADS 4
pthread_t pt;
sem_t sem_areas_loaded;

typedef enum
{
    SEM_HELPS, SEM_SHOP, SEM_REPAIR, SEM_AREA,
    NUM_SEMAPHORES
} semaphore_types;

sem_t semaphores[NUM_SEMAPHORES];
sem_t sem_mid[MAX_KEY_HASH],sem_oid[MAX_KEY_HASH],sem_rid[MAX_KEY_HASH];

void init_semaphores()
{
    int x;

    if (sem_init(&sem_areas_loaded, 0, NUM_WORKER_THREADS) == -1)
    {
        perror("sem_init");
        exit(1);
    }

    for (x = 0; x < NUM_SEMAPHORES; x++)
        if (sem_init(&semaphores[x], 0, 1) == -1)
        {
            perror("sem_init");
            exit(1);
        }

    for (x = 0; x < MAX_KEY_HASH; x++)
    {
        if (sem_init(&sem_mid[x], 0, 1) == -1 ||
            sem_init(&sem_oid[x], 0, 1) == -1 ||
            sem_init(&sem_rid[x], 0, 1) == -1)
        {
            perror("sem_init");
            exit(1);
        }
    }
}

void destroy_semaphores()
{
    int x;
    if (sem_destroy(&sem_areas_loaded) == -1)
    {
        perror("sem_destroy");
        exit(1);
    }
    for (x = 0; x < NUM_SEMAPHORES; x++)
        if (sem_destroy(&semaphores[x]) == -1)
        {
            perror("sem_destroy");
            exit(1);
        }
    for (x = 0; x < MAX_KEY_HASH; x++)
        if (sem_destroy(&sem_mid[x]) == -1 ||
            sem_destroy(&sem_oid[x]) == -1 ||
            sem_destroy(&sem_rid[x]) == -1)
        {
            perror("sem_destroy");
            exit(1);
        }
}
#endif

/*
 * Big mama top level function.
 */
void boot_db( bool fCopyOver )
{
    sh_int wear, x;

    /*    show_hash( 32 );*/
    unlink( BOOTLOG_FILE );
    boot_log( "-------------[ Boot Log ]------------" );

    boot_log("Initializing update handler");
    init_update();

    boot_log( "Loading commands" );
    load_commands();

    boot_log( "Loading sysdata configuration..." );

    /* default values */
    sysdata.read_all_mail		= LEVEL_DEMI;
    sysdata.read_mail_free 		= LEVEL_IMMORTAL;
    sysdata.write_mail_free 		= LEVEL_IMMORTAL;
    sysdata.take_others_mail		= LEVEL_DEMI;
    sysdata.muse_level			= LEVEL_DEMI;
    sysdata.think_level			= LEVEL_HIGOD;
    sysdata.build_level			= LEVEL_DEMI;
    sysdata.log_level			= LEVEL_LOG;
    sysdata.level_modify_proto		= LEVEL_LESSER;
    sysdata.level_override_private	= LEVEL_GREATER;
    sysdata.level_mset_player		= LEVEL_LESSER;
    sysdata.stun_plr_vs_plr		= 65;
    sysdata.stun_regular		= 15;
    sysdata.dam_plr_vs_plr		= 100;
    sysdata.dam_plr_vs_mob		= 100;
    sysdata.dam_mob_vs_plr		= 100;
    sysdata.dam_mob_vs_mob		= 100;
    sysdata.level_getobjnotake 		= LEVEL_GREATER;
    sysdata.save_frequency		= 20;	/* minutes */
    sysdata.save_flags			= SV_DEATH | SV_PASSCHG | SV_AUTO
        | SV_PUT | SV_DROP | SV_GIVE
        | SV_AUCTION | SV_ZAPDROP | SV_IDLE;
    sysdata.specials_enabled            = TRUE;
    sysdata.longest_uptime              = 0;

    for (x=LOG_NORMAL;x<LOG_LAST;x++)
    {
        char tbuf[MAX_INPUT_LENGTH];
        sysdata.logdefs[x].level    = LEVEL_IMMORTAL;
        sysdata.logdefs[x].num_logs = 0;
        sprintf(tbuf, "RenameMe%d", x);
        sysdata.logdefs[x].name     = STRALLOC(tbuf);
        switch (x)
        {
        default:
            sysdata.logdefs[x].channel = CHANNEL_LOG;
            break;
        case LOG_BUILD:
            sysdata.logdefs[x].channel = CHANNEL_BUILD;
            break;
        case LOG_MONITOR:
            sysdata.logdefs[x].channel = CHANNEL_MONITOR;
            break;
        case LOG_COMM:
            sysdata.logdefs[x].channel = CHANNEL_COMM;
            break;
        case LOG_PC:
            sysdata.logdefs[x].channel = CHANNEL_LOGPC;
            break;
        case LOG_HTTPD:
            sysdata.logdefs[x].channel = CHANNEL_HTTPD;
            break;
        case LOG_IMC:
            sysdata.logdefs[x].channel = CHANNEL_IMC;
            break;
        case LOG_IMCDEBUG:
            sysdata.logdefs[x].channel = CHANNEL_IMCDEBUG;
            break;
        case LOG_BUG:
            sysdata.logdefs[x].channel = CHANNEL_BUG;
            break;
        case LOG_DEBUG:
            sysdata.logdefs[x].channel = CHANNEL_DEBUG;
            break;
        case LOG_MAGIC:
            sysdata.logdefs[x].channel = CHANNEL_MAGIC;
            break;
        case LOG_IRC:
            sysdata.logdefs[x].channel = CHANNEL_IRC;
            break;
        }
    }

    if ( !load_systemdata(&sysdata) )
    {
        boot_log( "Not found.  Creating new configuration." );
        sysdata.alltimemax = 0;
    }

    boot_log("Loading socials");
    load_socials();
    boot_log("Loading poses");
    load_poses();

    boot_log("Loading skill table");
    load_skill_table();
    sort_skill_table();

    gsn_first_spell    = 0;
    gsn_first_skill    = 0;
    gsn_first_weapon   = 0;
    gsn_first_tongue   = 0;
    gsn_first_lore     = 0;
    gsn_first_psispell = 0;
    gsn_top_sn	       = top_sn;

    for ( x = 0; x < top_sn; x++ )
        if ( !gsn_first_spell && skill_table[x]->type == SKILL_SPELL )
            gsn_first_spell = x;
	else if ( !gsn_first_skill && skill_table[x]->type == SKILL_SKILL )
	    gsn_first_skill = x;
	else if ( !gsn_first_weapon && skill_table[x]->type == SKILL_WEAPON )
	    gsn_first_weapon = x;
	else if ( !gsn_first_tongue && skill_table[x]->type == SKILL_TONGUE )
	    gsn_first_tongue = x;
	else if ( !gsn_first_lore && skill_table[x]->type == SKILL_LORE )
	    gsn_first_lore = x;
	else if ( !gsn_first_psispell && skill_table[x]->type == SKILL_PSISPELL )
	    gsn_first_psispell = x;

    boot_log("Loading classes");
    load_classes();

    boot_log("Loading herb table");
    load_herb_table();

    boot_log("Loading tongues");
    load_tongues();

    boot_log("Making wizlist");
    make_wizlist();

    boot_log("Initializing request pipe");
    init_request_pipe();

    fBootDb		= TRUE;

    char_affects        = 0;
    nummobsloaded	= 0;
    numobjsloaded	= 0;
    physicalobjects	= 0;
    sysdata.maxplayers	= 0;
    first_object	= NULL;
    last_object		= NULL;
    first_char		= NULL;
    last_char		= NULL;
    first_area		= NULL;
    last_area		= NULL;
    first_build		= NULL;
    last_area		= NULL;
    first_shop		= NULL;
    last_shop		= NULL;
    first_repair	= NULL;
    last_repair		= NULL;
    first_teleport	= NULL;
    last_teleport	= NULL;
    first_asort		= NULL;
    last_asort		= NULL;
    extracted_obj_queue	= NULL;
    extracted_char_queue= NULL;
    cur_char		= NULL;
    cur_char_died	= FALSE;
    cur_room		= NULL;
    quitting_char	= NULL;
    loading_char	= NULL;
    saving_char		= NULL;
    CREATE( auction, AUCTION_DATA, 1);
    auction->item 	= NULL;

    weath_unit		= 10;
    rand_factor		= 2;
    climate_factor	= 1;
    neigh_factor	= 3;
    max_vector		= weath_unit*3;

    for ( wear = 0; wear < MAX_WEAR; wear++ )
        for ( x = 0; x < MAX_LAYERS; x++ )
            save_equipment[wear][x] = NULL;

    /*
     * Init random number generator.
     */
    boot_log("Initializing random number generator");
    init_mm( );
    srandom(time(0));
    /*
     * Set time and weather.
     */
    {
        long lhour, lday, lmonth;

        boot_log("Setting time and weather");

        lhour		= (current_time - 650336715)
            / (PULSE_TICK / PULSE_PER_SECOND);
        time_info.hour	= lhour  % 24;
        lday		= lhour  / 24;
        time_info.day	= lday   % 35;
        lmonth		= lday   / 35;
        time_info.month	= lmonth % 17;
        time_info.year	= lmonth / 17;

        if ( time_info.hour <  5 ) time_info.sunlight = SUN_DARK;
        else if ( time_info.hour <  6 ) time_info.sunlight = SUN_RISE;
        else if ( time_info.hour < 19 ) time_info.sunlight = SUN_LIGHT;
        else if ( time_info.hour < 20 ) time_info.sunlight = SUN_SET;
        else                            time_info.sunlight = SUN_DARK;
        /*
         weather_info.change	= 0;
         weather_info.mmhg	= 960;
         if ( time_info.month >= 7 && time_info.month <=12 )
         weather_info.mmhg += number_range( 1, 50 );
         else
         weather_info.mmhg += number_range( 1, 80 );

         if ( weather_info.mmhg <=  980 ) weather_info.sky = SKY_LIGHTNING;
         else if ( weather_info.mmhg <= 1000 ) weather_info.sky = SKY_RAINING;
         else if ( weather_info.mmhg <= 1020 ) weather_info.sky = SKY_CLOUDY;
         else                                  weather_info.sky = SKY_CLOUDLESS;
         */
    }


    /*
     * Assign gsn's for skills which need them.
     */
    boot_log("Assigning gsn's");
    ASSIGN_GSN(gsn_avoid_back_attack,               "avoid back attack");
    ASSIGN_GSN(gsn_detect_invis,                    "detect invisibility");
    ASSIGN_GSN(gsn_enlarge,                         "enlarge");
    ASSIGN_GSN(gsn_detect_evil,                     "detect evil");
    ASSIGN_GSN(gsn_detect_magic,                    "detect magic");
    ASSIGN_GSN(gsn_sense_life,                      "sense life");
    ASSIGN_GSN(gsn_sanctuary,                       "sanctuary");
    ASSIGN_GSN(gsn_poly,                            "polymorph self");
    ASSIGN_GSN(gsn_protection_from_evil,            "protection from evil");
    ASSIGN_GSN(gsn_infravision,                     "infravision");
    ASSIGN_GSN(gsn_charm_person,                    "charm");
    ASSIGN_GSN(gsn_weakness,                        "weakness");
    ASSIGN_GSN(gsn_strength,                        "strength");
    ASSIGN_GSN(gsn_armor,                           "armor");
    ASSIGN_GSN(gsn_detect_poison,                   "detect poison");
    ASSIGN_GSN(gsn_bless,                           "bless");
    ASSIGN_GSN(gsn_fly,                             "fly");
    ASSIGN_GSN(gsn_water_breath,                    "water breath");
    ASSIGN_GSN(gsn_fireshield,                      "fireshield");
    ASSIGN_GSN(gsn_faerie_fire,                     "faerie fire");
    ASSIGN_GSN(gsn_minor_track,                     "minor track");
    ASSIGN_GSN(gsn_major_track,                     "major track");
    ASSIGN_GSN(gsn_web,                             "web");
    ASSIGN_GSN(gsn_silence,                         "silence");
    ASSIGN_GSN(gsn_tree_travel,                     "tree travel");
    ASSIGN_GSN(gsn_haste,                           "haste");
    ASSIGN_GSN(gsn_slow,                            "slow");
    ASSIGN_GSN(gsn_barkskin,                        "barkskin");
    ASSIGN_GSN(gsn_aid,                             "aid");
    ASSIGN_GSN(gsn_true_sight,                      "true sight");
    ASSIGN_GSN(gsn_invis_to_animals,                "invis to animals");
    ASSIGN_GSN(gsn_dragon_ride,                     "dragon ride");
    ASSIGN_GSN(gsn_darkness,                        "darkness");
    ASSIGN_GSN(gsn_minor_invulnerability,           "minor invulnerability");
    ASSIGN_GSN(gsn_major_invulnerability,           "major invulnerability");
    ASSIGN_GSN(gsn_protection_from_energy_drain,    "protection from drain");
    ASSIGN_GSN(gsn_wizardeye,                       "wizardeye");
    ASSIGN_GSN(gsn_protection_from_breath,          "protection from breath");
    ASSIGN_GSN(gsn_protection_from_fire_breath,     "protection fire breath");
    ASSIGN_GSN(gsn_protection_from_frost_breath,    "protection frost breath");
    ASSIGN_GSN(gsn_protection_from_electric_breath, "protection electric breath");
    ASSIGN_GSN(gsn_protection_from_acid_breath,     "protection acid breath");
    ASSIGN_GSN(gsn_protection_from_gas_breath,      "protection gas breath");
    ASSIGN_GSN(gsn_anti_magic_shell,                "anti magic shell");
    ASSIGN_GSN(gsn_paralyze,                       "paralyze");
    ASSIGN_GSN(gsn_curse,                           "curse");

    ASSIGN_GSN(gsn_climb,                           "climb");
    ASSIGN_GSN(gsn_doorbash,                        "doorbash");
    ASSIGN_GSN(gsn_dual_wield,                      "dual wield");
    ASSIGN_GSN(gsn_possess,                         "possess");
    ASSIGN_GSN(gsn_blindness,                       "blindness");
    ASSIGN_GSN(gsn_poison,                          "poison");
    ASSIGN_GSN(gsn_backstab,                        "backstab");
    ASSIGN_GSN(gsn_berserk,                         "berserk");
    ASSIGN_GSN(gsn_fireball,                        "fireball");
    ASSIGN_GSN(gsn_chill_touch,                     "chill touch");
    ASSIGN_GSN(gsn_lightning_bolt,                  "lightning bolt");
    ASSIGN_GSN(gsn_sleep,                           "sleep");
    ASSIGN_GSN(gsn_group_invis,                     "group invisibility");
    ASSIGN_GSN(gsn_invis,                           "invisibility");
    ASSIGN_GSN(gsn_sneak,                           "sneak");
    ASSIGN_GSN(gsn_hunt,                            "hunt");
    ASSIGN_GSN(gsn_hide,                            "hide");
    ASSIGN_GSN(gsn_steal,                           "steal");
    ASSIGN_GSN(gsn_spot,                            "spot");
    ASSIGN_GSN(gsn_bash,                            "bash");
    ASSIGN_GSN(gsn_mount,                           "mount");
    ASSIGN_GSN(gsn_disarm,                          "disarm");
    ASSIGN_GSN(gsn_kick,                            "kick");
    ASSIGN_GSN(gsn_brew,                            "brew");
    ASSIGN_GSN(gsn_cook,                            "cook");
    ASSIGN_GSN(gsn_faerie_fire,                     "faerie fire");
    ASSIGN_GSN(gsn_shield,                          "shield");
    ASSIGN_GSN(gsn_swim,                            "swim");
    ASSIGN_GSN(gsn_retreat,                         "retreat");
    ASSIGN_GSN(gsn_spy,                             "spy");
    ASSIGN_GSN(gsn_travelling,                      "travelling");

    ASSIGN_GSN(gsn_comprehend_lang,                 "comprehend languages");
    ASSIGN_GSN(gsn_esp,                             "esp");
    ASSIGN_GSN(gsn_memorize,                        "memorizing");

    /* had / need looking at */
    ASSIGN_GSN(gsn_find_traps,                      "find traps");
    ASSIGN_GSN(gsn_detrap,                          "detrap");
    ASSIGN_GSN(gsn_pick_lock,                       "pick");
    ASSIGN_GSN(gsn_dodge,                           "dodge");
    ASSIGN_GSN(gsn_brew,                            "brew");
    ASSIGN_GSN(gsn_blast,                           "blast");
    ASSIGN_GSN(gsn_archery,                         "archery");
    ASSIGN_GSN(gsn_rescue,                          "rescue");
    ASSIGN_GSN(gsn_quivering_palm,                  "quivering palm");

    /* we want / might want */
    ASSIGN_GSN(gsn_feed,                            "feed");
    ASSIGN_GSN(gsn_gouge,                           "eyeshot");
    ASSIGN_GSN(gsn_search,                          "search");
    ASSIGN_GSN(gsn_dig,                             "dig");
    ASSIGN_GSN(gsn_parry,                           "parry");
    ASSIGN_GSN(gsn_stun,                            "stun");
    ASSIGN_GSN(gsn_punch,                           "punch");
    ASSIGN_GSN(gsn_grip,                            "grip");
    ASSIGN_GSN(gsn_scribe,                          "scribe");
    ASSIGN_GSN(gsn_poison_weapon,                   "poison weapon");

    /* zoso? */
    /* uhhhm, no. Stock smaug? -- Zoso */
    ASSIGN_GSN(gsn_claw,                            "claw");
    ASSIGN_GSN(gsn_bite,                            "bite");
    ASSIGN_GSN(gsn_sting,                           "sting");
    ASSIGN_GSN(gsn_tail,                            "tail");
    ASSIGN_GSN(gsn_slice,                           "slice");

    ASSIGN_GSN(gsn_protection_from_fire,            "protection from fire");
    ASSIGN_GSN(gsn_protection_from_cold,            "protection from cold");
    ASSIGN_GSN(gsn_protection_from_energy,          "protection from energy");
    ASSIGN_GSN(gsn_protection_from_electricity,     "protection from electricity");
    ASSIGN_GSN(gsn_cure_blindness,                  "cure blindness");
    ASSIGN_GSN(gsn_cure_poison,                     "cure poison");
    ASSIGN_GSN(gsn_stone_skin,                      "stone skin");
    ASSIGN_GSN(gsn_refresh,                         "refresh");
    ASSIGN_GSN(gsn_cure_light,                      "cure light");
    ASSIGN_GSN(gsn_heal,                            "heal");

    ASSIGN_GSN(gsn_energy_drain,                    "energy drain");
    ASSIGN_GSN(gsn_earthquake,                      "earthquake");
    ASSIGN_GSN(gsn_flamestrike,                     "flamestrike");
    ASSIGN_GSN(gsn_harm,                            "harm");
    ASSIGN_GSN(gsn_colour_spray,                    "colour spray");
    ASSIGN_GSN(gsn_weaken,                          "weaken");
    ASSIGN_GSN(gsn_cure_serious,                    "cure serious");
    ASSIGN_GSN(gsn_cure_critical,                   "cure critical");
    ASSIGN_GSN(gsn_dispel_magic,                    "dispel magic");
    ASSIGN_GSN(gsn_dispel_evil,                     "dispel evil");

    ASSIGN_GSN(gsn_meditate,			"meditate");
    ASSIGN_GSN(gsn_psiportal,			"psiportal");
    ASSIGN_GSN(gsn_scry,				"scry");
    ASSIGN_GSN(gsn_doorway,				"doorway");
    ASSIGN_GSN(gsn_tan,				"tan");
    ASSIGN_GSN(gsn_canibalize,			"canibalize");
    ASSIGN_GSN(gsn_psishield,			"psishield");
    ASSIGN_GSN(gsn_mindblank,			"mindblank");
    ASSIGN_GSN(gsn_tower_of_iron_will,			"towerofironwill");
    ASSIGN_GSN(gsn_great_sight,			"greatsight");
    ASSIGN_GSN(gsn_psistrength,			"psistrength");
    ASSIGN_GSN(gsn_chameleon,			"chameleon");

    ASSIGN_GSN(gsn_slow_poison,			"slow poison");
    ASSIGN_GSN(gsn_cause_light,			"cause light");
    ASSIGN_GSN(gsn_cause_serious,			"cause serious");
    ASSIGN_GSN(gsn_cause_critical,			"cause critical");
    ASSIGN_GSN(gsn_acid_blast,			"acid blast");
    ASSIGN_GSN(gsn_juggernaut,			"juggernaut");
    ASSIGN_GSN(gsn_pray,			"pray");
    ASSIGN_GSN(gsn_read_magic,			"read magic");
    ASSIGN_GSN(gsn_babel,			"babel");
    ASSIGN_GSN(gsn_drinking,			"drinking");

    ASSIGN_GSN(gsn_animal_lore,			"animal lore");
    ASSIGN_GSN(gsn_demonology,			"demonology");
    ASSIGN_GSN(gsn_giant_lore,			"giant lore");
    ASSIGN_GSN(gsn_necromancy,			"necromancy");
    ASSIGN_GSN(gsn_other_lore,			"other lore");
    ASSIGN_GSN(gsn_people_lore,			"people lore");
    ASSIGN_GSN(gsn_reptile_lore,       		"reptile lore");
    ASSIGN_GSN(gsn_vegetable_lore,     		"vegetable lore");

    ASSIGN_GSN(gsn_feeblemind,                  "feeblemind");
    ASSIGN_GSN(gsn_mindwipe,                    "mindwipe");

    boot_log("Loading currency...");
    load_currency();

    /*
     * Read in all the area files.
     */

#ifdef USE_DB
    db_load_areas();
    db_load_helps();
#else
    {
        char *ln;
        FILE *fpList;
        AREA_DATA *tarea;
#ifdef THREADED_AREA_LOAD
        int sval;
#endif

        boot_log("Reading in area files...");
        if ( ( fpList = fopen( AREA_LIST, "r" ) ) == NULL )
        {
            perror( AREA_LIST );
            shutdown_mud( "Unable to open area list" );
            exit( 1 );
        }

#ifdef THREADED_AREA_LOAD
        init_semaphores();
#endif

        for ( ; ; )
        {
            struct timeval	  last_time;
            gettimeofday( &last_time, NULL );
            current_time = (time_t) last_time.tv_sec;

            ln = fread_line( fpList );

            one_argument(ln, strArea);
            strncpy(strArea, ln, strlen(strArea));
            ln+=strlen(strArea);

            if ( strArea[0] == '#' )
                continue;
            if ( strArea[0] == '$' )
                break;

            tarea = create_area( strip_crlf( strArea ) );

            sscanf( ln, "%d %d %d %d %d %d %d",
                    &tarea->flags,
                    &tarea->low_r_vnum, &tarea->hi_r_vnum,
                    &tarea->low_m_vnum, &tarea->hi_m_vnum,
                    &tarea->low_o_vnum, &tarea->hi_o_vnum );

#ifdef AREA_LOAD_ON_DEMAND
            if (!IS_AREA_FLAG(tarea, AFLAG_RESET_BOOT))
                continue;
#endif

#ifdef THREADED_AREA_LOAD
            sem_wait(&sem_areas_loaded);
            if (pthread_create(&pt, NULL, &load_area_file, tarea) == -1)
            {
                perror("pthread_create");
                exit(1);
            }
#else
            load_area_file( tarea );
#endif
        }

#ifdef THREADED_AREA_LOAD
        sem_getvalue(&sem_areas_loaded, &sval);
        while (sval < NUM_WORKER_THREADS)
            sem_getvalue(&sem_areas_loaded, &sval);
        destroy_semaphores();
#endif

        FCLOSE( fpList );
    }
#endif

    /*
     *   initialize supermob.
     *    must be done before reset_area!
     *
     */
    init_supermob();
    init_chess();
    init_acro();

    /*
     * Fix up exits.
     * Declare db booting over.
     * Reset all areas once.
     * Load up the notes file.
     */
    boot_log( "Fixing exits" );
    fix_exits( );
    fBootDb	= FALSE;
    boot_log( "Initializing economy" );
    initialize_economy( );
    boot_log( "Resetting areas" );
    area_update( );
    boot_log( "Loading buildlist" );
    load_buildlist( );
    boot_log( "Loading boards" );
    load_boards( );
    boot_log( "Loading clans" );
    load_clans( );
    boot_log( "Loading deities" );
    load_deity( );
    boot_log( "Loading councils" );
    load_councils( );
    boot_log( "Loading bans" );
    load_banlist( );
    boot_log( "Loading corpses" );
    load_corpses( );
    boot_log( "Loading properties" );
    load_properties( );
    boot_log( "Loading commodities" );
    load_commodities( );
    boot_log( "Loading weather" );
    load_weatherdata();
    init_area_weather();
    boot_log( "Loading christen data" );
    load_christens();
    boot_log( "Loading rare object data" );
    load_rare_objs();
    boot_log("Loading quests");
    load_quests();
    boot_log("Loading bugs");
    load_bugtracks();

    boot_log("Searching for orphaned rooms");
    orphan_room_search();

    MOBtrigger = TRUE;
    /* init_maps ( ); */

    boot_log( "------------[ End Boot Log ]----------" );
    return;
}


#ifndef USE_DB
AREA_DATA *create_area( char *filename )
{
    AREA_DATA *pArea;

    CREATE(pArea, AREA_DATA, 1);
    pArea->name         = str_dup( "none" );
    pArea->filename     = str_dup( filename );
    pArea->first_reset	= NULL;
    pArea->last_reset	= NULL;
    pArea->author       = STRALLOC( "unknown" );
    pArea->age		= 15;
    pArea->nplayer	= 0;
    pArea->low_r_vnum	= 0;
    pArea->low_o_vnum	= 0;
    pArea->low_m_vnum	= 0;
    pArea->hi_r_vnum	= 0;
    pArea->hi_o_vnum	= 0;
    pArea->hi_m_vnum	= 0;
    pArea->low_soft_range = 0;
    pArea->hi_soft_range  = MAX_LEVEL;
    pArea->low_hard_range = 0;
    pArea->hi_hard_range  = MAX_LEVEL;
    /* initialize weather data - FB */
    CREATE(pArea->weather, WEATHER_DATA, 1);
    pArea->weather->temp = 0;
    pArea->weather->precip = 0;
    pArea->weather->wind = 0;
    pArea->weather->temp_vector = 0;
    pArea->weather->precip_vector = 0;
    pArea->weather->wind_vector = 0;
    pArea->weather->climate_temp = 2;
    pArea->weather->climate_precip = 2;
    pArea->weather->climate_wind = 2;
    pArea->weather->first_neighbor = NULL;
    pArea->weather->last_neighbor = NULL;
    pArea->weather->echo = NULL;
    pArea->weather->echo_color = AT_GREY;
    pArea->area_version = 0;
    pArea->plane = PLANE_NORMAL;

#ifdef THREADED_AREA_LOAD
    sem_wait(&semaphores[SEM_AREA]);
#endif
    LINK( pArea, first_area, last_area, next, prev );
    top_area++;
#ifdef THREADED_AREA_LOAD
    sem_post(&semaphores[SEM_AREA]);
#endif
    return pArea;
}

/*
 * Load an 'area' header line.
 */
static void load_area( AREA_DATA *tarea, gzFile gzfp )
{
    if ( !tarea )
    {
        bug( "Load_area: tarea not created." );
        if ( fBootDb )
        {
            shutdown_mud( "No tarea" );
            exit( 1 );
        }
        else
            return;
    }

    if (tarea->name)
        DISPOSE(tarea->name);
    tarea->name		= gz_fread_string_nohash( gzfp );
}

/* Load the version number of the area file if none exists, then it
 * is set to version 0 when #AREA is read in which is why we check for
 * the #AREA here.  --Shaddai
 */
void load_version( AREA_DATA *tarea, gzFile gzfp )
{
    if ( !tarea )
    {
        bug( "Load_version: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }

    tarea->area_version   = gz_fread_number( gzfp );
    return;
}

void load_comment( AREA_DATA *tarea, gzFile gzfp )
{
    if ( !tarea )
    {
        bug( "Load_comment: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }

    if (tarea->comment)
        DISPOSE(tarea->comment);
    tarea->comment = gz_fread_string_nohash( gzfp );
    return;
}

/*
 * Load an author section. Scryn 2/1/96
 */
static void load_author( AREA_DATA *tarea, gzFile gzfp )
{
    if ( !tarea )
    {
        bug( "Load_author: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }

    if ( tarea->author )
        STRFREE( tarea->author );
    tarea->author   = gz_fread_string( gzfp );
    return;
}

/*
 * Load an economy section. Thoric
 */
static void load_economy( AREA_DATA *tarea, gzFile gzfp )
{
    if ( !tarea )
    {
        bug( "Load_economy: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }

    tarea->high_economy[DEFAULT_CURR]	= gz_fread_number( gzfp );
    tarea->low_economy[DEFAULT_CURR]	= gz_fread_number( gzfp );
    return;
}

void load_areacurr( AREA_DATA *tarea, gzFile gzfp )
{
    if ( !tarea )
    {
        bug( "Load_areacurr: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }

    tarea->currvnum = gz_fread_number(gzfp);

    return;
}

void load_plane( AREA_DATA *tarea, gzFile gzfp )
{
    if ( !tarea )
    {
        bug( "Load_plane: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }

    tarea->plane = gz_fread_number(gzfp);
    gz_fread_to_eol(gzfp);

    return;
}

void load_higheconomy( AREA_DATA *tarea, gzFile gzfp )
{
    int x1,x2;

    if ( !tarea )
    {
        bug( "Load_higheconomy: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }

    x1=x2=0;
    while ((x1=gz_fread_number(gzfp))>=0)
        if (x2 < MAX_CURR_TYPE)
            tarea->high_economy[x2++] = x1;

    return;
}

void load_loweconomy( AREA_DATA *tarea, gzFile gzfp )
{
    int x1,x2;

    if ( !tarea )
    {
        bug( "Load_loweconomy: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }

    x1=x2=0;
    while ((x1=gz_fread_number(gzfp))>=0)
        if (x2 < MAX_CURR_TYPE)
            tarea->low_economy[x2++] = x1;

    return;
}

/* Reset Message Load, Rennard */
static void load_resetmsg( AREA_DATA *tarea, gzFile gzfp )
{
    if ( !tarea )
    {
        bug( "Load_resetmsg: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }

    if ( tarea->resetmsg )
        DISPOSE( tarea->resetmsg );
    tarea->resetmsg = gz_fread_string_nohash( gzfp );
    return;
}

/*
 * Load area flags. Narn, Mar/96
 */
static void load_flags( AREA_DATA *tarea, gzFile gzfp )
{
    char linebuf[MAX_STRING_LENGTH];
    char *ln;
    int x1, x2;

    if ( !tarea )
    {
        bug( "Load_flags: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }
    ln = gz_fread_line( gzfp, linebuf );
    x1=x2=0;
    sscanf( ln, "%d %d",
            &x1, &x2 );
    tarea->flags = x1;
    if (IS_SET(tarea->flags, AFLAG_MODIFIED))
        REMOVE_BIT(tarea->flags, AFLAG_MODIFIED);

    /* enable this code to have areas reset only when mortals walk into them */
#if 1
    if (IS_SET(tarea->flags, AFLAG_RESET_BOOT))
        SET_BIT(tarea->flags, AFLAG_INITIALIZED);
    else if (IS_SET(tarea->flags, AFLAG_INITIALIZED))
        REMOVE_BIT(tarea->flags, AFLAG_INITIALIZED);
#else
    SET_BIT(tarea->flags, AFLAG_INITIALIZED);
#endif

    tarea->reset_frequency = x2;
    if ( x2 )
        tarea->age = x2;
    return;
}
#endif

/*
 * Adds a help page to the list if it is not a duplicate of an existing page.
 * Page is insert-sorted by keyword.			-Thoric
 * (The reason for sorting is to keep do_hlist looking nice)
 */
void add_help( HELP_DATA *pHelp )
{
    HELP_DATA *tHelp;
    int match;

#ifdef THREADED_AREA_LOAD
    sem_wait(&semaphores[SEM_HELPS]);
#endif
    for ( tHelp = first_help; tHelp; tHelp = tHelp->next )
        if ( pHelp->level == tHelp->level
             &&   strcmp(pHelp->keyword, tHelp->keyword) == 0 )
        {
            bug( "add_help: duplicate: %s.  Deleting.", pHelp->keyword );
            STRFREE( pHelp->text );
            STRFREE( pHelp->keyword );
            DISPOSE( pHelp );
#ifdef THREADED_AREA_LOAD
    sem_post(&semaphores[SEM_HELPS]);
#endif
            return;
        }
        else
            if ( (match=strcmp(pHelp->keyword[0]=='\'' ? pHelp->keyword+1 : pHelp->keyword,
                               tHelp->keyword[0]=='\'' ? tHelp->keyword+1 : tHelp->keyword)) < 0
                 ||   (match == 0 && pHelp->level > tHelp->level) )
            {
                if ( !tHelp->prev )
                    first_help	  = pHelp;
                else
                    tHelp->prev->next = pHelp;
                pHelp->prev		  = tHelp->prev;
                pHelp->next		  = tHelp;
                tHelp->prev		  = pHelp;
                break;
            }

    if ( !tHelp )
        LINK( pHelp, first_help, last_help, next, prev );

    top_help++;
#ifdef THREADED_AREA_LOAD
    sem_post(&semaphores[SEM_HELPS]);
#endif
}

/*
 * Load a help section.
 */
#ifndef USE_DB
static void load_helps( AREA_DATA *tarea, gzFile gzfp )
{
    HELP_DATA *pHelp;

    for ( ; ; )
    {
        CREATE( pHelp, HELP_DATA, 1 );
        pHelp->level	= gz_fread_number( gzfp );
        pHelp->keyword	= gz_fread_string( gzfp );
        if ( pHelp->keyword[0] == '$' )
        {
            STRFREE(pHelp->keyword);
            DISPOSE(pHelp);
            break;
        }
        pHelp->text	= gz_fread_string( gzfp );
        if ( pHelp->keyword[0] == '\0' )
        {
            STRFREE( pHelp->text );
            STRFREE( pHelp->keyword );
            DISPOSE( pHelp );
            continue;
        }

        if ( !str_cmp( pHelp->keyword, "greeting" ) )
            help_greeting = pHelp->text;
        add_help( pHelp );
    }
    SET_AREA_FLAG(tarea, AFLAG_RESET_BOOT);
    if (fBootDb)
        boot_log( "%d help entries loaded.", top_help );
    return;
}
#endif

/*
 * Add a character to the list of all characters		-Thoric
 */
void add_char( CHAR_DATA *ch )
{
    LINK( ch, first_char, last_char, next, prev );
}


/*
 * Load a mob section.
 */
#ifndef USE_DB
static void load_mobiles( AREA_DATA *tarea, gzFile gzfp )
{
    char linebuf[MAX_STRING_LENGTH];
    char log_buf[MAX_STRING_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    int x1, x2;
/*#ifdef FREAD_LINE*/
#if 1
    int x3, x4, x5, x6, x7, x8, x9;
    char *ln;
#endif
    int tmplevel;

    if ( !tarea )
    {
        bug( "Load_mobiles: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }

    for ( ; ; )
    {
        int vnum;
        char letter;
        int iHash;
        bool oldmob;
        bool tmpBootDb;

        letter				= gz_fread_letter( gzfp );
        if ( letter != '#' )
        {
            bug( "Load_mobiles: # not found." );
            if ( fBootDb )
            {
                shutdown_mud( "# not found" );
                exit( 1 );
            }
            else
                return;
        }

        vnum				= gz_fread_number( gzfp );
        if ( vnum == 0 )
            break;

        tmpBootDb = fBootDb;
//        fBootDb = FALSE;
        if ( mob_exists_index( vnum ) )
        {
            if ( tmpBootDb )
            {
                bug( "Load_mobiles: vnum %d duplicated.", vnum );
                shutdown_mud( "duplicate vnum" );
                exit( 1 );
            }
            else
            {
                pMobIndex = get_mob_index( vnum );
                sprintf( log_buf, "Cleaning mobile: %d", vnum );
                log_string_plus( log_buf, LOG_BUILD, sysdata.log_level, SEV_DEBUG );
                clean_mob( pMobIndex );
                oldmob = TRUE;
            }
        }
        else
        {
            oldmob = FALSE;
            CREATE( pMobIndex, MOB_INDEX_DATA, 1 );
        }
//        fBootDb = tmpBootDb;

        pMobIndex->ivnum       		= vnum;
        top_mob_vnum = UMAX(top_mob_vnum, vnum);
        if ( fBootDb )
        {
            if ( !tarea->low_m_vnum )
                tarea->low_m_vnum	= vnum;
            if ( vnum > tarea->hi_m_vnum )
                tarea->hi_m_vnum	= vnum;
        }
        pMobIndex->area			= tarea;

        pMobIndex->player_name		= gz_fread_string( gzfp );
        pMobIndex->short_descr		= gz_fread_string( gzfp );
        pMobIndex->long_descr		= gz_fread_string( gzfp );
        pMobIndex->description		= gz_fread_string( gzfp );

        pMobIndex->long_descr[0]	= UPPER(pMobIndex->long_descr[0]);
        pMobIndex->description[0]	= UPPER(pMobIndex->description[0]);

        pMobIndex->pShop		= NULL;
        pMobIndex->rShop		= NULL;

#ifndef FREAD_LINE
        pMobIndex->act			= gz_fread_number( gzfp );
        SET_BIT(pMobIndex->act, ACT_IS_NPC);
        pMobIndex->affected_by		= gz_fread_number( gzfp );

        pMobIndex->alignment		= gz_fread_number( gzfp );
        letter				= gz_fread_letter( gzfp );
#else
        ln = gz_fread_line( gzfp, linebuf );
        x1=x2=x3=0;
        sscanf( ln, "%d %d %d %c",
                &x1, &x2, &x3, &letter );

        pMobIndex->act			= x1;
        SET_BIT(pMobIndex->act, ACT_IS_NPC);
        pMobIndex->affected_by		= x2;
        pMobIndex->alignment		= x3;
#endif
#ifndef FREAD_LINE
        tmplevel        		= gz_fread_number( gzfp );

        pMobIndex->mobthac0		= gz_fread_number( gzfp );
        pMobIndex->ac			= gz_fread_number( gzfp );
        pMobIndex->hitnodice		= gz_fread_number( gzfp );
        /* 'd'		*/		  gz_fread_letter( gzfp );
        pMobIndex->hitsizedice		= gz_fread_number( gzfp );
        /* '+'		*/		  gz_fread_letter( gzfp );
        pMobIndex->hitplus		= gz_fread_number( gzfp );
        pMobIndex->damnodice		= gz_fread_number( gzfp );
        /* 'd'		*/		  gz_fread_letter( gzfp );
        pMobIndex->damsizedice		= gz_fread_number( gzfp );
        /* '+'		*/		  gz_fread_letter( gzfp );
        pMobIndex->damplus		= gz_fread_number( gzfp );
#else
        ln = gz_fread_line( gzfp, linebuf );
        x1=x2=x3=x4=x5=x6=x7=x8=x9=0;
        sscanf( ln, "%d %d %d %dd%d+%d %dd%d+%d",
                &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9 );

        tmplevel        		= x1;

        pMobIndex->mobthac0		= x2;
        pMobIndex->ac			= x3;
        pMobIndex->hitnodice		= x4;
        pMobIndex->hitsizedice		= x5;
        pMobIndex->hitplus		= x6;
        pMobIndex->damnodice		= x7;
        pMobIndex->damsizedice		= x8;
        pMobIndex->damplus		= x9;
#endif

#ifndef FREAD_LINE
        GET_MONEY(pMobIndex,DEFAULT_CURR)= gz_fread_number( gzfp );
        GET_EXP(pMobIndex)     		= gz_fread_number( gzfp );
#else
        ln = gz_fread_line( gzfp, linebuf );
        x1=x2=0;
        sscanf( ln, "%d %d",
                &x1, &x2 );

        GET_MONEY(pMobIndex,DEFAULT_CURR)= x1;
        GET_EXP(pMobIndex)     		= x2;
#endif
#ifndef FREAD_LINE
        pMobIndex->position		= gz_fread_number( gzfp );
        pMobIndex->defposition		= gz_fread_number( gzfp );
        pMobIndex->sex			= gz_fread_number( gzfp );
#else
        ln = gz_fread_line( gzfp, linebuf );
        x1=x2=x3=0;
        sscanf( ln, "%d %d %d",
                &x1, &x2, &x3 );

        pMobIndex->position		= x1;
        pMobIndex->defposition		= x2;
        pMobIndex->sex			= x3;
#endif

        if ( letter != 'S' && letter != 'C' && letter != 'D' )
        {
            bug( "Load_mobiles: vnum %d: letter '%c' not S, C, or D.",
                 vnum, letter );
            shutdown_mud( "bad mob data" );
            exit( 1 );
        }
        if ( letter == 'C' || letter == 'D' ) /* Realms complex mob-Thoric */
        {
#ifndef FREAD_LINE
            pMobIndex->perm_str			= gz_fread_number( gzfp );
            pMobIndex->perm_int			= gz_fread_number( gzfp );
            pMobIndex->perm_wis			= gz_fread_number( gzfp );
            pMobIndex->perm_dex			= gz_fread_number( gzfp );
            pMobIndex->perm_con			= gz_fread_number( gzfp );
            pMobIndex->perm_cha			= gz_fread_number( gzfp );
            pMobIndex->perm_lck			= gz_fread_number( gzfp );
#else
            ln = gz_fread_line( gzfp, linebuf );
            x1=x2=x3=x4=x5=x6=x7=0;
            sscanf( ln, "%d %d %d %d %d %d %d",
                    &x1, &x2, &x3, &x4, &x5, &x6, &x7 );

            pMobIndex->perm_str			= x1;
            pMobIndex->perm_int			= x2;
            pMobIndex->perm_wis			= x3;
            pMobIndex->perm_dex			= x4;
            pMobIndex->perm_con			= x5;
            pMobIndex->perm_cha			= x6;
            pMobIndex->perm_lck			= x7;
#endif
#ifndef FREAD_LINE
            pMobIndex->saving_poison_death	= gz_fread_number( gzfp );
            pMobIndex->saving_wand		= gz_fread_number( gzfp );
            pMobIndex->saving_para_petri	= gz_fread_number( gzfp );
            pMobIndex->saving_breath		= gz_fread_number( gzfp );
            pMobIndex->saving_spell_staff	= gz_fread_number( gzfp );
#else
            ln = gz_fread_line( gzfp, linebuf );
            x1=x2=x3=x4=x5=0;
            sscanf( ln, "%d %d %d %d %d",
                    &x1, &x2, &x3, &x4, &x5 );

            pMobIndex->saving_poison_death	= x1;
            pMobIndex->saving_wand		= x2;
            pMobIndex->saving_para_petri	= x3;
            pMobIndex->saving_breath		= x4;
            pMobIndex->saving_spell_staff	= x5;
#endif
            /* only used one time to convert old areas */
            /*
            if (pMobIndex->saving_poison_death != 0 ||
                pMobIndex->saving_wand != 0 ||
                pMobIndex->saving_para_petri != 0 ||
                pMobIndex->saving_breath != 0 ||
                pMobIndex->saving_spell_staff != 0)
                SET_BIT(pMobIndex->act, ACT_CUSTOMSAVES);
            */
            if (!IS_SET(pMobIndex->act, ACT_CUSTOMSAVES))
            {
                pMobIndex->saving_poison_death = UMAX(20-tmplevel, 2);
                pMobIndex->saving_wand         = UMAX(20-tmplevel, 2);
                pMobIndex->saving_para_petri   = UMAX(20-tmplevel, 2);
                pMobIndex->saving_breath       = UMAX(20-tmplevel, 2);
                pMobIndex->saving_spell_staff  = UMAX(20-tmplevel, 2);
            }

#ifndef FREAD_LINE
            pMobIndex->race		= gz_fread_number( gzfp );
            x2                          = gz_fread_number( gzfp );
            pMobIndex->height		= gz_fread_number( gzfp );
            pMobIndex->weight		= gz_fread_number( gzfp );
            pMobIndex->speaks		= gz_fread_number( gzfp );
            pMobIndex->speaking		= gz_fread_number( gzfp );
            pMobIndex->numattacks	= gz_fread_number( gzfp );
#else
            ln = gz_fread_line( gzfp, linebuf );
            x1=x2=x3=x4=x5=x6=x7=0;
            sscanf( ln, "%d %d %d %d %d %d %d",
                    &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
            pMobIndex->race		= x1;
            pMobIndex->height		= x3;
            pMobIndex->weight		= x4;
            pMobIndex->speaks		= x5;
            pMobIndex->speaking		= x6;
            pMobIndex->numattacks	= x7;
#endif
            if (pMobIndex->race < 0 || pMobIndex->race >= MAX_RACE)
	    {
		bug("Load_mobiles: bad race: vnum: %d, race: %d",
		    vnum, pMobIndex->race);
                shutdown_mud("invalid race");
                exit( 1 );
	    }


	    /* Heath -- Give mob a ch_class in new system */
            pMobIndex->classes[x2]      = STAT_ACTCLASS;
            pMobIndex->levels[x2]       = tmplevel;

            if ( !pMobIndex->speaks && pMobIndex->race>=0)
                pMobIndex->speaks = race_table[pMobIndex->race].language | LANG_COMMON;
            else
                pMobIndex->speaks = LANG_COMMON;

            if ( !pMobIndex->speaking && pMobIndex->race>=0)
                pMobIndex->speaking = race_table[pMobIndex->race].language;
            else
                pMobIndex->speaking = LANG_COMMON;

#ifndef FREAD_LINE
            pMobIndex->hitroll		= gz_fread_number( gzfp );
            pMobIndex->damroll		= gz_fread_number( gzfp );
            pMobIndex->xflags		= gz_fread_number( gzfp );
            pMobIndex->resistant	= gz_fread_number( gzfp );
            pMobIndex->immune		= gz_fread_number( gzfp );
            pMobIndex->susceptible	= gz_fread_number( gzfp );
            pMobIndex->attacks		= gz_fread_number( gzfp );
            pMobIndex->defenses		= gz_fread_number( gzfp );
            if (letter=='D')
            {
                pMobIndex->act2         = gz_fread_number( gzfp );
                pMobIndex->affected_by2 = gz_fread_number( gzfp );
                pMobIndex->absorb	= gz_fread_number( gzfp );
            }
#else
            ln = gz_fread_line( gzfp, linebuf );
            x1=x2=x3=x4=x5=x6=x7=x8=0;
            sscanf( ln, "%d %d %d %d %d %d %d %d",
                    &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8 );
            pMobIndex->hitroll		= x1;
            pMobIndex->damroll		= x2;
            pMobIndex->xflags		= x3;
            pMobIndex->resistant	= x4;
            pMobIndex->immune		= x5;
            pMobIndex->susceptible	= x6;
            pMobIndex->attacks		= x7;
            pMobIndex->defenses		= x8;
            if (letter=='D')
            {
                ln = gz_fread_line( gzfp, linebuf );
                x1=x2=x3=x4=x5=x6=x7=x8=0;
                sscanf( ln, "%d %d %d %d %d %d %d %d",
                        &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8 );
                pMobIndex->act2		= x1;
                pMobIndex->affected_by2	= x2;
                pMobIndex->absorb	= x3;
            }
#endif
        }
        else
        {
            pMobIndex->perm_str		= 13;
            pMobIndex->perm_dex		= 13;
            pMobIndex->perm_int		= 13;
            pMobIndex->perm_wis		= 13;
            pMobIndex->perm_cha		= 13;
            pMobIndex->perm_con		= 13;
            pMobIndex->perm_lck		= 13;
            pMobIndex->race		= 0;
            pMobIndex->classes[CLASS_WARRIOR] = STAT_ACTCLASS;
            pMobIndex->levels[CLASS_WARRIOR]  = tmplevel;
            pMobIndex->xflags		= 0;
            pMobIndex->resistant	= 0;
            pMobIndex->immune		= 0;
            pMobIndex->susceptible	= 0;
            pMobIndex->numattacks	= 0;
            pMobIndex->attacks		= 0;
            pMobIndex->defenses		= 0;
        }

        for ( ; ; )
        {
            letter = gz_fread_letter( gzfp );

            if ( letter == 'S' || letter == '$' )
            {
                gz_fread_to_eol(gzfp);
                break;
            }
            else if ( letter == '#' )
            {
                gzseek(gzfp, -1, SEEK_CUR);
                break;
            }

            switch(letter)
            {
            case 'T':
                x1=x2=0;
                while ((x1=gz_fread_number(gzfp))>=0)
                    if (x2 < MAX_CURR_TYPE)
                        GET_MONEY(pMobIndex,x2++)=x1;
                if (pMobIndex->gold2)
                {
                    GET_MONEY(pMobIndex,CURR_GOLD) = pMobIndex->gold2;
                    pMobIndex->gold2 = 0;
                }
                break;
            case '>':
                mprog_read_programs( gzfp, pMobIndex );
                break;
            }
        }

        if ( !oldmob )
        {
            iHash			= vnum % MAX_KEY_HASH;

#ifdef THREADED_AREA_LOAD
    sem_wait(&sem_mid[iHash]);
#endif

            pMobIndex->next		= mob_index_hash[iHash];
            mob_index_hash[iHash]	= pMobIndex;
            top_mob_index++;

#ifdef THREADED_AREA_LOAD
    sem_post(&sem_mid[iHash]);
#endif
        }
/* temp special code */
        {
            int xxx;
            for (xxx=0;hack_the_specs[xxx].vnum>0;xxx++)
                if (hack_the_specs[xxx].vnum == vnum)
                {
                    SET_ACT2_FLAG(pMobIndex, hack_the_specs[xxx].flags);
                    break;
                }
        }

    }

    return;
}

/*
 * Load an obj section.
 */
static void load_objects( AREA_DATA *tarea, gzFile gzfp )
{
    char linebuf[MAX_STRING_LENGTH];
    char wordbuf[MAX_INPUT_LENGTH];
    char log_buf[MAX_STRING_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    char letter;
    char *ln;
    int x1, x2, x3, x4, x5, x6;

    if ( !tarea )
    {
        bug( "Load_objects: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }

    for ( ; ; )
    {
        int vnum;
        int iHash;
        bool tmpBootDb;
        bool oldobj;

        letter				= gz_fread_letter( gzfp );
        if ( letter != '#' )
        {
            bug( "Load_objects: # not found in %s", tarea->filename );
            bug( "Load_objects: %c(%d) found", letter, letter );
            if ( fBootDb )
            {
                shutdown_mud( "# not found" );
                exit( 1 );
            }
            else
                return;
        }

        vnum				= gz_fread_number( gzfp );
        if ( vnum == 0 )
            break;

        tmpBootDb = fBootDb;
//        fBootDb = FALSE;
        if ( obj_exists_index( vnum ) )
        {
            if ( tmpBootDb )
            {
                bug( "Load_objects: vnum %d duplicated.", vnum );
                shutdown_mud( "duplicate vnum" );
                exit( 1 );
            }
            else
            {
                pObjIndex = get_obj_index( vnum );
                sprintf( log_buf, "Cleaning object: %d", vnum );
                log_string_plus( log_buf, LOG_BUILD, sysdata.log_level, SEV_DEBUG );
                clean_obj( pObjIndex );
                oldobj = TRUE;
            }
        }
        else
        {
            oldobj = FALSE;
            CREATE( pObjIndex, OBJ_INDEX_DATA, 1 );
        }
//        fBootDb = tmpBootDb;

        pObjIndex->ivnum       		= vnum;
        top_obj_vnum = UMAX(top_obj_vnum, vnum);
        if ( fBootDb )
        {
            if ( !tarea->low_o_vnum )
                tarea->low_o_vnum		= vnum;
            if ( vnum > tarea->hi_o_vnum )
                tarea->hi_o_vnum		= vnum;
        }
        pObjIndex->area			= tarea;
        pObjIndex->name			= gz_fread_string( gzfp );
        pObjIndex->short_descr		= gz_fread_string( gzfp );
        pObjIndex->description		= gz_fread_string( gzfp );
        pObjIndex->action_desc		= gz_fread_string( gzfp );

        /* Commented out by Narn, Apr/96 to allow item short descs like
         Bonecrusher and Oblivion */
        /*pObjIndex->short_descr[0]	= LOWER(pObjIndex->short_descr[0]);*/
        pObjIndex->description[0]	= UPPER(pObjIndex->description[0]);

        ln = gz_fread_line( gzfp, linebuf );
        x1=x2=x3=x4=x5=x6=0;
        sscanf( ln, "%d %d %d %d %d %d",
                &x1, &x2, &x3, &x4, &x5, &x6 );
        pObjIndex->item_type		= x1;
        pObjIndex->extra_flags		= x2;
        pObjIndex->wear_flags		= x3;
        pObjIndex->layers		= x4;

        ln = gz_fread_line( gzfp, linebuf );
        x1=x2=x3=x4=x5=x6=0;
        sscanf( ln, "%d %d %d %d %d %d",
                &x1, &x2, &x3, &x4, &x5, &x6 );
        pObjIndex->value[0]		= x1;
        pObjIndex->value[1]		= x2;
        pObjIndex->value[2]		= x3;
        pObjIndex->value[3]		= x4;
        pObjIndex->value[4]		= x5;
        pObjIndex->value[5]		= x6;

        ln = gz_fread_line( gzfp, linebuf );
        x1=x2=x3=x4=x5=x6=0;
        sscanf( ln, "%d %d %d %d %d %d",
                &x1, &x2, &x3, &x4, &x5, &x6 );
        pObjIndex->weight	        = UMAX( 1, x1 );
        pObjIndex->cost			= x2;
        pObjIndex->rent		  	= x3;
        pObjIndex->currtype = URANGE(FIRST_CURR, x4, LAST_CURR);

        if ( tarea->area_version == 1 )
        {
            switch ( pObjIndex->item_type )
            {
            case ITEM_PILL:
            case ITEM_POTION:
            case ITEM_SCROLL:
                pObjIndex->value[1] = skill_lookup ( gz_fread_word( gzfp, wordbuf )) ;
                pObjIndex->value[2] = skill_lookup ( gz_fread_word( gzfp, wordbuf )) ;
                pObjIndex->value[3] = skill_lookup ( gz_fread_word( gzfp, wordbuf )) ;
                break;
            case ITEM_STAFF:
            case ITEM_WAND:
                pObjIndex->value[3] = skill_lookup ( gz_fread_word( gzfp, wordbuf )) ;
                break;
            case ITEM_SALVE:
                pObjIndex->value[4] = skill_lookup ( gz_fread_word( gzfp, wordbuf )) ;
                pObjIndex->value[5] = skill_lookup ( gz_fread_word( gzfp, wordbuf )) ;
                break;
            }
        }

        for ( ; ; )
        {
            letter = gz_fread_letter( gzfp );

            if ( letter == 'S' )
                break;

            if ( letter == 'A' )
            {
                AFFECT_DATA *paf;

                CREATE( paf, AFFECT_DATA, 1 );
                paf->type		= -1;
                paf->duration		= -1;

                ln = gz_fread_line( gzfp, linebuf );
                x1=x2=0;
                sscanf( ln, "%d %d",
                        &x1, &x2 );

                paf->location		= x1;
                if ( paf->location == APPLY_WEAPONSPELL
                     ||   paf->location == APPLY_WEARSPELL
                     ||   paf->location == APPLY_REMOVESPELL
                     ||   paf->location == APPLY_EAT_SPELL
                     ||   paf->location == APPLY_IMMUNESPELL
                     ||   paf->location == APPLY_STRIPSN )
                    paf->modifier		= slot_lookup( x2 );
                else
                    paf->modifier		= x2;
                if (paf->location == APPLY_EAT_SPELL)
                {
                    bug( "Load_objects: obj %d has eat spell, check sn/slot %d", vnum, paf->modifier );
                }
                paf->bitvector		= 0;
                if (paf->location == APPLY_STUN ||
                    paf->location == APPLY_PUNCH ||
                    paf->location == APPLY_CLIMB)
                {
                    bug( "Load_objects: obj %d, apply %d", vnum, paf->location );
                }
                LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect,
                      next, prev );
                top_affect++;
            }

            else if ( letter == 'D' )
            {
                ln = gz_fread_line( gzfp, linebuf );
                x1=x2=0;
                sscanf( ln, "%d %d",
                        &x1, &x2 );
                pObjIndex->extra_flags2		= x1;
                pObjIndex->magic_flags		= x2;
            }

            else if ( letter == 'E' )
            {
                EXTRA_DESCR_DATA *ed;

                CREATE( ed, EXTRA_DESCR_DATA, 1 );
                ed->keyword		= gz_fread_string( gzfp );
                ed->description		= gz_fread_string( gzfp );
                LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc,
                      next, prev );
                top_ed++;
            }

            else if ( letter == '>' )
            {
                oprog_read_programs( gzfp, pObjIndex );
            }

            else
            {
                /*ungetc( letter, fp );*/
                gzseek(gzfp, -1, SEEK_CUR);
                break;
            }
        }

        switch ( pObjIndex->item_type )
        {
        case ITEM_MISSILE_WEAPON:
            REMOVE_BIT(pObjIndex->wear_flags, ITEM_MISSILE_WIELD);
            break;
        case ITEM_PROJECTILE:
            REMOVE_BIT(pObjIndex->wear_flags, ITEM_WIELD);
            SET_BIT(pObjIndex->wear_flags, ITEM_MISSILE_WIELD);
            break;

        case ITEM_FIREWEAPON:
            bug( "Load_objects: vnum %d was ITEM_FIREWEAPON.", pObjIndex->ivnum );
            pObjIndex->item_type = ITEM_MISSILE_WEAPON;
            REMOVE_BIT(pObjIndex->wear_flags, ITEM_MISSILE_WIELD);
            break;
        case ITEM_MISSILE:
            bug( "Load_objects: vnum %d was ITEM_MISSILE.", pObjIndex->ivnum );
            pObjIndex->item_type = ITEM_PROJECTILE;
            REMOVE_BIT(pObjIndex->wear_flags, ITEM_WIELD);
            SET_BIT(pObjIndex->wear_flags, ITEM_MISSILE_WIELD);
            break;
        }

        /*
         * Translate spell "slot numbers" to internal "skill numbers."
         */
        if ( tarea->area_version == 0 )
            switch ( pObjIndex->item_type )
            {
            case ITEM_PILL:
            case ITEM_POTION:
            case ITEM_SCROLL:
                pObjIndex->value[1] = slot_lookup( pObjIndex->value[1] );
                pObjIndex->value[2] = slot_lookup( pObjIndex->value[2] );
                pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
                break;

            case ITEM_STAFF:
            case ITEM_WAND:
                pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
                break;
            case ITEM_SALVE:
                pObjIndex->value[4] = slot_lookup( pObjIndex->value[4] );
                pObjIndex->value[5] = slot_lookup( pObjIndex->value[5] );
                break;
            }

        if ( !oldobj )
        {
            iHash			= vnum % MAX_KEY_HASH;

#ifdef THREADED_AREA_LOAD
    sem_wait(&sem_oid[iHash]);
#endif

            pObjIndex->next	= obj_index_hash[iHash];
            obj_index_hash[iHash]	= pObjIndex;

            top_obj_index++;
#ifdef THREADED_AREA_LOAD
    sem_post(&sem_oid[iHash]);
#endif
        }
    }

    return;
}

/*
 * Load a reset section.
 */
static void load_resets( AREA_DATA *tarea, gzFile gzfp )
{
    char linebuf[MAX_STRING_LENGTH];
    char log_buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pexit;
    bool not01 = FALSE;
    int count = 0;
    char letter;
    char *ln;
    int extra, arg1, arg2, arg3;

    if ( !tarea )
    {
        bug( "Load_resets: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }

    if ( tarea->first_reset )
    {
        if ( fBootDb )
        {
            RESET_DATA *rtmp;

            for ( rtmp = tarea->first_reset; rtmp; rtmp = rtmp->next )
                ++count;
            bug( "load_resets: WARNING: %d resets already exist for this area.", count );
        }
        else
        {
            /*
             * Clean out the old resets
             */
            sprintf( log_buf, "Cleaning resets: %s", tarea->name );
            log_string_plus( log_buf, LOG_BUILD, sysdata.log_level, SEV_DEBUG );
            clean_resets( tarea );
        }
    }

    for ( ; ; )
    {
        ln = gz_fread_line( gzfp, linebuf );
        extra=arg1=arg2=arg3=0;
        sscanf( ln, "%c %d %d %d %d",
                &letter, &extra, &arg1, &arg2, &arg3 );

        if ( letter == 'S' )
            break;
        else if ( letter == '*' )
            continue;
        else if ( letter == 'G' || letter == 'R')
            arg3 = 0;

        ++count;

        /*
         * Validate parameters.
         * We're calling the index functions for the side effect.
         */
        switch ( letter )
        {
        default:
            bug( "Load_resets: bad command '%c'.", letter );
            if ( fBootDb )
                boot_log( "Load_resets: %.24s (#%d) bad command '%c'.", tarea->name, count, letter );
            return;

        case 'M':
            if ( !mob_exists_index( arg1 ) && fBootDb )
                boot_log( "Load_resets: %.24s (#%d) 'M': mobile %d doesn't exist.",
                          tarea->name, count, arg1 );
            else if ( arg1 < tarea->low_m_vnum || arg1 > tarea->hi_m_vnum )
                boot_log( "Load_resets: %.24s (#%d) 'M': mobile %d from another area.",
                          tarea->name, count, arg1 );
            if ( !room_exists_index( arg3 ) && fBootDb )
                boot_log( "Load_resets: %.24s (#%d) 'M': room %d doesn't exist.",
                          tarea->name, count, arg3 );
            else if ( arg3 < tarea->low_r_vnum || arg3 > tarea->hi_r_vnum )
                boot_log( "Load_resets: %.24s (#%d) 'M': room %d in another area.",
                          tarea->name, count, arg3 );
            break;

        case 'O':
            if ( !obj_exists_index(arg1) && fBootDb )
                boot_log( "Load_resets: %.24s (#%d) 'O': object %d doesn't exist.",
                          tarea->name, count, arg1 );
            else if ( arg1 < tarea->low_o_vnum || arg1 > tarea->hi_o_vnum )
                boot_log( "Load_resets: %.24s (#%d) 'O': object %d from another area.",
                          tarea->name, count, arg1 );
            if ( !room_exists_index(arg3) && fBootDb )
                boot_log( "Load_resets: %.24s (#%d) 'O': room %d doesn't exist.",
                          tarea->name, count, arg3 );
            else if ( arg3 < tarea->low_r_vnum || arg3 > tarea->hi_r_vnum )
                boot_log( "Load_resets: %.24s (#%d) 'O': room %d in another area.",
                          tarea->name, count, arg3 );
            break;

        case 'P':
            if ( !obj_exists_index(arg1) && fBootDb )
                boot_log( "Load_resets: %.24s (#%d) 'P': object %d doesn't exist.",
                          tarea->name, count, arg1 );
            else if ( arg1 < tarea->low_o_vnum || arg1 > tarea->hi_o_vnum )
                boot_log( "Load_resets: %.24s (#%d) 'P': object %d from another area.",
                          tarea->name, count, arg1 );
            if ( arg3 > 0 )
            {
                if ( !obj_exists_index(arg3) && fBootDb )
                    boot_log( "Load_resets: %.24s (#%d) 'P': destination object %d doesn't exist.",
                              tarea->name, count, arg3 );
                else if ( arg3 < tarea->low_o_vnum || arg3 > tarea->hi_o_vnum )
                    boot_log( "Load_resets: %.24s (#%d) 'P': object %d from another area.",
                              tarea->name, count, arg3 );
            }
            else if ( extra > 1 )
                not01 = TRUE;
            break;

        case 'G':
        case 'E':
            if ( !obj_exists_index(arg1) && fBootDb )
                boot_log( "Load_resets: %.24s (#%d) '%c': object %d doesn't exist.",
                          tarea->name, count, letter, arg1 );
            else if ( arg1 < tarea->low_o_vnum || arg1 > tarea->hi_o_vnum )
                boot_log( "Load_resets: %.24s (#%d) '%c': object %d from another area.",
                          tarea->name, count, letter, arg1 );
            break;

        case 'T':
            break;

        case 'H':
            if ( arg1 > 0 )
            {
                if ( !obj_exists_index(arg1) && fBootDb )
                    boot_log( "Load_resets: %.24s (#%d) 'H': object %d doesn't exist.",
                              tarea->name, count, arg1 );
                else if ( arg1 < tarea->low_o_vnum || arg1 > tarea->hi_o_vnum )
                    boot_log( "Load_resets: %.24s (#%d) 'H': object %d from another area.",
                              tarea->name, count, arg1 );
            }
            break;

        case 'B':
            switch(arg2 & BIT_RESET_TYPE_MASK)
            {
            case BIT_RESET_DOOR:
                {
                    int door;

                    pRoomIndex = get_room_index( arg1 );
                    if ( !pRoomIndex )
                    {
                        bug( "Load_resets: 'B': room %d doesn't exist.", arg1 );
                        bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
                             arg3 );
                        if ( fBootDb )
                            boot_log( "Load_resets: %.24s (#%d) 'B': room %d doesn't exist.",
                                      tarea->name, count, arg1 );
                    }
                    if ( arg1 < tarea->low_r_vnum || arg1 > tarea->hi_r_vnum )
                        boot_log( "Load_resets: %.24s (#%d) 'B': room %d in another area.",
                                  tarea->name, count, arg1 );

                    door = (arg2 & BIT_RESET_DOOR_MASK) >> BIT_RESET_DOOR_THRESHOLD;

                    if ( !(pexit = get_exit(pRoomIndex, door)) )
                    {
                        bug( "Load_resets: 'B': exit %d not door.", door );
                        bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
                             arg3 );
                        if ( fBootDb )
                            boot_log( "Load_resets: %.24s (#%d) 'B': exit %d not door.",
                                      tarea->name, count, door );
                    }
                }
                break;
            case BIT_RESET_ROOM:
                if (!room_exists_index(arg1))
                {
                    bug( "Load_resets: 'B': room %d doesn't exist.", arg1);
                    bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
                         arg3 );
                    if ( fBootDb )
                        boot_log( "Load_resets: %.24s (#%d) 'B': room %d doesn't exist.",
                                  tarea->name, count, arg1 );
                    if ( arg1 < tarea->low_r_vnum || arg1 > tarea->hi_r_vnum )
                        boot_log( "Load_resets: %.24s (#%d) 'B': room %d in another area.",
                                  tarea->name, count, arg1 );
                }
                break;
            case BIT_RESET_OBJECT:
                if (arg1 > 0)
                {
                    if (!obj_exists_index(arg1) && fBootDb)
                        boot_log("Load_resets: %.24s (#%d) 'B': object %d doesn't exist.",
                                 tarea->name, count, arg1 );
                    else if ( arg1 < tarea->low_o_vnum || arg1 > tarea->hi_o_vnum )
                        boot_log( "Load_resets: %.24s (#%d) 'B': object %d from another area.",
                                  tarea->name, count, arg1 );
                }
                break;
            case BIT_RESET_MOBILE:
                if (arg1 > 0)
                {
                    if (!mob_exists_index(arg1) && fBootDb)
                        boot_log("Load_resets: %.24s (#%d) 'B': mobile %d doesn't exist.",
                                 tarea->name, count, arg1 );
                    else if ( arg1 < tarea->low_m_vnum || arg1 > tarea->hi_m_vnum )
                        boot_log( "Load_resets: %.24s (#%d) 'B': mobile %d from another area.",
                                  tarea->name, count, arg1 );
                }
                break;
            default:
                boot_log( "Load_resets: %.24s (#%d) 'B': bad type flag (%d).",
                          tarea->name, count, arg2 & BIT_RESET_TYPE_MASK );
                break;
            }
            break;

        case 'D':
            pRoomIndex = get_room_index( arg1 );
            if ( !pRoomIndex )
            {
                bug( "Load_resets: 'D': room %d doesn't exist.", arg1 );
                bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
                     arg3 );
                if ( fBootDb )
                    boot_log( "Load_resets: %.24s (#%d) 'D': room %d doesn't exist.",
                              tarea->name, count, arg1 );
                break;
            }

            if ( arg2 < 0
                 ||   arg2 >= MAX_REXITS
                 || ( pexit = get_exit(pRoomIndex, arg2)) == NULL
                 || !IS_SET( pexit->exit_info, EX_ISDOOR ) )
            {
                bug( "Load_resets: 'D': exit %d not door.", arg2 );
                bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
                     arg3 );
                if ( fBootDb )
                    boot_log( "Load_resets: %.24s (#%d) 'D': exit %d not door.",
                              tarea->name, count, arg2 );
            }

            if ( arg3 < 0 || arg3 > 2 )
            {
                bug( "Load_resets: 'D': bad 'locks': %d.", arg3 );
                if ( fBootDb )
                    boot_log( "Load_resets: %.24s (#%d) 'D': bad 'locks': %d.",
                              tarea->name, count, arg3 );
            }
            break;

        case 'R':
            pRoomIndex = get_room_index( arg1 );
            if ( !pRoomIndex && fBootDb )
                boot_log( "Load_resets: %.24s (#%d) 'R': room %d doesn't exist.",
                          tarea->name, count, arg1 );
            else if ( arg1 < tarea->low_r_vnum || arg1 > tarea->hi_r_vnum )
                boot_log( "Load_resets: %.24s (#%d) 'R': room %d in another area.",
                          tarea->name, count, arg1 );

            if ( arg2 < 0 || arg2 > 6 )
            {
                bug( "Load_resets: 'R': bad exit %d.", arg2 );
                if ( fBootDb )
                    boot_log( "Load_resets: %.24s (#%d) 'R': bad exit %d.",
                              tarea->name, count, arg2 );
                break;
            }

            break;
        }

        /* finally, add the reset */
        add_reset( tarea, letter, extra, arg1, arg2, arg3 );
    }

    if ( !not01 )
        renumber_put_resets(tarea);

    return;
}

/*
 * Load a room section.
 */
static void load_rooms( AREA_DATA *tarea, gzFile gzfp )
{
    char linebuf[MAX_STRING_LENGTH];
    char log_buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *pRoomIndex;
    char *ln;

    if ( !tarea )
    {
        bug( "Load_rooms: no #AREA seen yet." );
        shutdown_mud( "No #AREA" );
        exit( 1 );
    }

    for ( ; ; )
    {
        int vnum;
        char letter;
        int door;
        int iHash;
        bool tmpBootDb;
        bool oldroom;
        int x1, x2, x3, x4, x5, x6, x7, x8;

        letter				= gz_fread_letter( gzfp );
        if ( letter != '#' )
        {
            bug( "Load_rooms: # not found." );
            if ( fBootDb )
            {
                shutdown_mud( "# not found" );
                exit( 1 );
            }
            else
                return;
        }

        vnum				= gz_fread_number( gzfp );
        if ( vnum == 0 )
            break;

        tmpBootDb = fBootDb;
//        fBootDb = FALSE;
        if ( room_exists_index( vnum ) )
        {
            if ( tmpBootDb )
            {
#if 1
                bug( "Load_rooms: vnum %d duplicated.", vnum );
                shutdown_mud( "duplicate vnum" );
                exit( 1 );
#else
                oldroom = TRUE;
#endif
            }
            else
            {
                pRoomIndex = get_room_index( vnum );
                sprintf( log_buf, "Cleaning room: %d", vnum );
                log_string_plus( log_buf, LOG_BUILD, sysdata.log_level, SEV_DEBUG );
                clean_room( pRoomIndex );
                oldroom = TRUE;
            }
        }
        else
        {
            oldroom = FALSE;
            CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
            pRoomIndex->first_person	= NULL;
            pRoomIndex->last_person	= NULL;
            pRoomIndex->first_content	= NULL;
            pRoomIndex->last_content	= NULL;
        }

//        fBootDb = tmpBootDb;
        pRoomIndex->area		= tarea;
        pRoomIndex->vnum		= vnum;
        top_room_vnum = UMAX(top_room_vnum, vnum);
        pRoomIndex->first_extradesc	= NULL;
        pRoomIndex->last_extradesc	= NULL;

        if ( fBootDb )
        {
            if ( !tarea->low_r_vnum )
                tarea->low_r_vnum		= vnum;
            if ( vnum > tarea->hi_r_vnum )
                tarea->hi_r_vnum		= vnum;
        }
        pRoomIndex->name		= gz_fread_string( gzfp );
        pRoomIndex->description		= gz_fread_string( gzfp );

        /* Area number			  gz_fread_number( fp ); */
        ln = gz_fread_line( gzfp, linebuf );
        x1=x2=x3=x4=x5=x6=x7=x8=0;
        x6=1000; /* default elevation */
        sscanf( ln, "0 %d %d %d %d %d %d %d %d",
                &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8 );

        pRoomIndex->room_flags		= x1;
        pRoomIndex->sector_type		= x2;
        pRoomIndex->tele_delay		= x3;
        pRoomIndex->tele_vnum		= x4;
        pRoomIndex->tunnel		= x5;
        pRoomIndex->elevation		= x6;
        pRoomIndex->liquid		= x7;
        pRoomIndex->currvnum            = x8;
        assign_currindex(pRoomIndex);

        /* accidentally got elev and liq backwards... after a saveall this
         * can go away */
        if (pRoomIndex->liquid == 1000)
        {
            pRoomIndex->liquid = 0;
            pRoomIndex->elevation = 1000;
        }

        pRoomIndex->river		= NULL;

        if (pRoomIndex->sector_type < 0 || pRoomIndex->sector_type >= SECT_MAX)
        {
            bug( "Fread_rooms: vnum %d has bad sector_type %d.", vnum ,
                 pRoomIndex->sector_type);
            pRoomIndex->sector_type = 1;
        }
        pRoomIndex->light		= 0;
        pRoomIndex->first_exit		= NULL;
        pRoomIndex->last_exit		= NULL;

        for ( ; ; )
        {
            letter = gz_fread_letter( gzfp );

            if ( letter == 'S' )
                break;

            if ( letter == 'D' )
            {
                EXIT_DATA *pexit;
                int locks;

                door = gz_fread_number( gzfp );
                if ( door < 0 || door >= MAX_REXITS )
                {
                    bug( "Fread_rooms: vnum %d has bad door number %d.", vnum,
                         door );
                    if ( fBootDb )
                        exit( 1 );
                }
                else
                {
                    pexit = make_exit( pRoomIndex, NULL, door );
                    pexit->description	= gz_fread_string( gzfp );
                    pexit->keyword	= gz_fread_string( gzfp );
                    pexit->exit_info	= 0;
                    ln = gz_fread_line( gzfp, linebuf );
		    x1=x2=x3=x4=0;
		    /* if this is an arbitrary exit the reverse is the same,
		     * otherwise it's reverse from rev_dir */
		    if (door > LAST_NORMAL_DIR)
			x5 = door;
		    else
			x5 = rev_dir[door];
                    sscanf( ln, "%d %d %d %d %d",
                            &x1, &x2, &x3, &x4, &x5 );

                    locks			= x1;
                    pexit->key		= UMAX(0,x2);
                    pexit->vnum		= x3;
                    pexit->vdir		= door;
		    pexit->distance	= x4;
                    pexit->rdir         = x5;

                    switch ( locks )
                    {
                    case 1:  pexit->exit_info = EX_ISDOOR;                break;
                    case 2:  pexit->exit_info = EX_ISDOOR | EX_PICKPROOF; break;
                    default: pexit->exit_info = locks;
                    }
                }
            }
            else if ( letter == 'E' )
            {
                EXTRA_DESCR_DATA *ed;

                CREATE( ed, EXTRA_DESCR_DATA, 1 );
                ed->description		= gz_fread_string( gzfp );
                ed->keyword		= gz_fread_string( gzfp );
                LINK( ed, pRoomIndex->first_extradesc, pRoomIndex->last_extradesc,
                      next, prev );
                top_ed++;
            }
            else if ( letter == 'M' )    /* maps */
            {
                MAP_DATA *map;
                MAP_INDEX_DATA *map_index;
                int i, j;

                CREATE( map, MAP_DATA, 1);
                map->vnum                     = gz_fread_number( gzfp );
                map->x                        = gz_fread_number( gzfp );
                map->y                        = gz_fread_number( gzfp );
                map->entry		      = gz_fread_letter( gzfp );

                pRoomIndex->map               = map;
                if(  (map_index = get_map_index(map->vnum)) == NULL  )
                {
                    CREATE( map_index, MAP_INDEX_DATA, 1);
                    map_index->vnum = map->vnum;
                    map_index->next = first_map;
                    first_map       = map_index;
                    for (i = 0; i <  49; i++) {
                        for (j = 0; j <  79; j++) {
                            map_index->map_of_vnums[i][j] = -1;
                            /* map_index->map_of_ptrs[i][j] = NULL; */
                        }
                    }
                }
                if( (map->y <0) || (map->y >48) )
                {
                    bug("Map y coord out of range.  Room %d\n\r", map->y);

                }
                if( (map->x <0) || (map->x >78) )
                {
                    bug("Map x coord out of range.  Room %d\n\r", map->x);

                }
                if(  (map->x >0)
                     &&(map->x <80)
                     &&(map->y >0)
                     &&(map->y <48) )
                    map_index->map_of_vnums[map->y][map->x]=pRoomIndex->vnum;
            }
            else if ( letter == '>' )
            {
                rprog_read_programs( gzfp, pRoomIndex );
            }
            else
            {
                bug( "Load_rooms: vnum %d has flag '%c' not 'DESM>'.", vnum,
                     letter );
                shutdown_mud( "Room flag not DESM>" );
                exit( 1 );
            }

        }

        if ( !oldroom )
        {
            iHash			 = vnum % MAX_KEY_HASH;

#ifdef THREADED_AREA_LOAD
            sem_wait(&sem_rid[iHash]);
#endif

            pRoomIndex->next	 = room_index_hash[iHash];
            room_index_hash[iHash] = pRoomIndex;
            top_room++;

#ifdef THREADED_AREA_LOAD
            sem_post(&sem_rid[iHash]);
#endif
        }
    }

    return;
}

/*
 * Load a shop section.
 */
static void load_shops( AREA_DATA *tarea, gzFile gzfp )
{
    SHOP_DATA *pShop;

    for ( ; ; )
    {
        MOB_INDEX_DATA *pMobIndex;
        int iTrade;

        CREATE( pShop, SHOP_DATA, 1 );
        pShop->keeper		= gz_fread_number( gzfp );
        if ( pShop->keeper == 0 )
        {
            DISPOSE(pShop);
            break;
        }
        for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
            pShop->buy_type[iTrade]	= gz_fread_number( gzfp );
        pShop->profit_buy	= gz_fread_number( gzfp );
        pShop->profit_sell	= gz_fread_number( gzfp );
        pShop->profit_buy	= URANGE( pShop->profit_sell+5, pShop->profit_buy, 1000 );
        pShop->profit_sell	= URANGE( 0, pShop->profit_sell, pShop->profit_buy-5 );
        pShop->open_hour	= gz_fread_number( gzfp );
        pShop->close_hour	= gz_fread_number( gzfp );
        gz_fread_to_eol( gzfp );
        pMobIndex		= get_mob_index( pShop->keeper );
        pMobIndex->pShop	= pShop;

#ifdef THREADED_AREA_LOAD
        sem_wait(&semaphores[SEM_SHOP]);
#endif

        if ( !first_shop )
            first_shop		= pShop;
        else
            last_shop->next	= pShop;
        pShop->next		= NULL;
        pShop->prev		= last_shop;
        last_shop		= pShop;
        top_shop++;

#ifdef THREADED_AREA_LOAD
    sem_post(&semaphores[SEM_SHOP]);
#endif
    }
    return;
}

/*
 * Load a repair shop section.					-Thoric
 */
static void load_repairs( AREA_DATA *tarea, gzFile gzfp )
{
    REPAIR_DATA *rShop;

    for ( ; ; )
    {
        MOB_INDEX_DATA *pMobIndex;
        int iFix;

        CREATE( rShop, REPAIR_DATA, 1 );
        rShop->keeper		= gz_fread_number( gzfp );
        if ( rShop->keeper == 0 )
        {
            DISPOSE(rShop);
            break;
        }
        for ( iFix = 0; iFix < MAX_FIX; iFix++ )
            rShop->fix_type[iFix] = gz_fread_number( gzfp );
        rShop->profit_fix	= gz_fread_number( gzfp );
        rShop->shop_type	= gz_fread_number( gzfp );
        rShop->open_hour	= gz_fread_number( gzfp );
        rShop->close_hour	= gz_fread_number( gzfp );
        gz_fread_to_eol( gzfp );
        pMobIndex		= get_mob_index( rShop->keeper );
        pMobIndex->rShop	= rShop;

#ifdef THREADED_AREA_LOAD
        sem_wait(&semaphores[SEM_REPAIR]);
#endif

        if ( !first_repair )
            first_repair		= rShop;
        else
            last_repair->next	= rShop;
        rShop->next		= NULL;
        rShop->prev		= last_repair;
        last_repair		= rShop;
        top_repair++;

#ifdef THREADED_AREA_LOAD
        sem_post(&semaphores[SEM_REPAIR]);
#endif
    }
    return;
}

/*
 * Load spec proc declarations.
 */
static void load_specials( AREA_DATA *tarea, gzFile gzfp )
{
    char wordbuf[MAX_INPUT_LENGTH];
    char letter = '\0';

    for ( ; ; )
    {
        MOB_INDEX_DATA *pMobIndex;
        OBJ_INDEX_DATA *pObjIndex;
        ROOM_INDEX_DATA *room;

        switch ( letter = gz_fread_letter( gzfp ) )
        {
        default:
            bug( "Load_specials: letter '%c' not *MSOR.", letter );
            exit( 1 );

        case 'S':
            return;

        case '*':
            break;

        case 'M':
            pMobIndex		= get_mob_index	( gz_fread_number ( gzfp ) );
            pMobIndex->spec_fun	= m_spec_lookup	( gz_fread_word   ( gzfp, wordbuf ) );
            if ( pMobIndex->spec_fun == NULL )
                bug( "Load_specials: 'M': vnum %d.", pMobIndex->ivnum );
            break;

        case 'O':
            pObjIndex		= get_obj_index	( gz_fread_number ( gzfp ) );
            pObjIndex->spec_fun	= o_spec_lookup	( gz_fread_word   ( gzfp, wordbuf ) );
            if ( pObjIndex->spec_fun == NULL )
                bug( "Load_specials: 'O': vnum %d.", pObjIndex->ivnum );
            break;

        case 'R':
            room		= get_room_index( gz_fread_number ( gzfp ) );
            room->spec_fun	= r_spec_lookup	( gz_fread_word   ( gzfp, wordbuf ) );
            if ( room->spec_fun == NULL )
                bug( "Load_specials: 'R': vnum %d.", room->vnum );
            break;
        }

        gz_fread_to_eol( gzfp );
    }
}

/*
 * Load soft / hard area ranges.
 */
static void load_ranges( AREA_DATA *tarea, gzFile gzfp )
{
    char linebuf[MAX_STRING_LENGTH];
    int x1, x2, x3, x4;
    char *ln;

    if ( !tarea )
    {
        bug( "Load_ranges: no #AREA seen yet." );
        shutdown_mud( "No #AREA" );
        exit( 1 );
    }

    for ( ; ; )
    {
        ln = gz_fread_line( gzfp, linebuf );

        if (ln[0] == '$')
            break;

        x1=x2=x3=x4=0;
        sscanf( ln, "%d %d %d %d",
                &x1, &x2, &x3, &x4 );

        tarea->low_soft_range = x1;
        tarea->hi_soft_range = x2;
        tarea->low_hard_range = x3;
        tarea->hi_hard_range = x4;
    }
    return;

}

/*
 * Load climate information for the area
 * Last modified: July 13, 1997
 * Fireblade
 */
void load_climate(AREA_DATA *tarea, gzFile gzfp)
{
    if ( !tarea )
    {
        bug("load_climate: no #AREA seen yet");
        if(fBootDb)
        {
            shutdown_mud("No #AREA");
            exit(1);
        }
        else
            return;
    }

    tarea->weather->climate_temp = gz_fread_number(gzfp);
    tarea->weather->climate_precip = gz_fread_number(gzfp);
    tarea->weather->climate_wind = gz_fread_number(gzfp);

    return;
}

/*
 * Load data for a neghboring weather system
 * Last modified: July 13, 1997
 * Fireblade
 */
void load_neighbor(AREA_DATA *tarea, gzFile gzfp)
{
    NEIGHBOR_DATA *dnew;

    if(!tarea)
    {
        bug("load_neighbor: no #AREA seen yet.");
        if(fBootDb)
        {
            shutdown_mud("No #AREA");
            exit(1);
        }
        else
            return;
    }

    CREATE(dnew, NEIGHBOR_DATA, 1);
    dnew->next = NULL;
    dnew->prev = NULL;
    dnew->address = NULL;
    dnew->name = gz_fread_string(gzfp);
    LINK(dnew,
         tarea->weather->first_neighbor,
         tarea->weather->last_neighbor,
         next, prev);

    return;
}
#endif

/*
 * Go through all areas, and set up initial economy based on mob
 * levels and gold
 */
void initialize_economy( void )
{
    AREA_DATA *tarea;
    MOB_INDEX_DATA *mob;
    int idx, money, rng, type=DEFAULT_CURR;

    for ( tarea = first_area; tarea; tarea = tarea->next )
    {
        /* skip area if they already got some money */
	if (tarea->currindex)
	    type = tarea->currindex->primary;
        if ( tarea->high_economy[type] > 0 || tarea->low_economy[type] > 10000 )
            continue;
        rng = tarea->hi_soft_range - tarea->low_soft_range;
        if ( rng )
            rng /= 2;
        else
            rng = 25;
        money = rng * rng * 50000;
        boost_economy( tarea, money, type );
        for ( idx = tarea->low_m_vnum; idx < tarea->hi_m_vnum; idx++ )
            if ( (mob=get_mob_index(idx)) != NULL )
                boost_economy( tarea, GET_MONEY(mob,type) * 10, type );
    }
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits( void )
{
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pexit, *pexit_next, *rev_exit;
    int iHash;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for ( pRoomIndex  = room_index_hash[iHash];
              pRoomIndex;
              pRoomIndex  = pRoomIndex->next )
        {
            bool fexit;

            fexit = FALSE;
            for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit_next )
            {
                pexit_next = pexit->next;
                pexit->rvnum = pRoomIndex->vnum;
                if ( pexit->vnum <= 0 ||
                     (pexit->to_room=get_room_index(pexit->vnum)) == NULL )
                {
                    if ( fBootDb )
                        boot_log( "Fix_exits: room %d, exit %s leads to bad/unloaded vnum (%d)",
                                  pRoomIndex->vnum, exit_name(pexit), pexit->vnum );

                    bug( "Fix_exits: room %d, exit %s leads to bad/unloaded vnum (%d)",
                         pRoomIndex->vnum, exit_name(pexit), pexit->vnum );

                    pexit->to_room = NULL;
/*                    extract_exit( pRoomIndex, pexit );*/
                }
                else
                    fexit = TRUE;
            }
            if ( !fexit )
                SET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
        }
    }

    /* Set all the rexit pointers 	-Thoric */
    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for ( pRoomIndex  = room_index_hash[iHash];
              pRoomIndex;
              pRoomIndex  = pRoomIndex->next )
        {
            for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
            {
                if ( pexit->to_room )
                {
                    rev_exit = get_exit_to( pexit->to_room, pexit->rdir, pRoomIndex->vnum );
                    if ( rev_exit )
                    {
                        pexit->rexit	= rev_exit;
                        rev_exit->rexit	= pexit;
                    }
                }
            }
        }
    }

    return;
}


/*
 * (prelude...) This is going to be fun... NOT!
 * (conclusion) QSort is f*cked!
 */
int exit_comp( EXIT_DATA **xit1, EXIT_DATA **xit2 )
{
    int d1, d2;

    d1 = (*xit1)->vdir;
    d2 = (*xit2)->vdir;

    if ( d1 < d2 )
        return -1;
    if ( d1 > d2 )
        return 1;
    return 0;
}

void sort_exits( ROOM_INDEX_DATA *room )
{
    EXIT_DATA *pexit; /* *texit */ /* Unused */
    EXIT_DATA *exits[MAX_REXITS];
    int x, nexits;

    nexits = 0;
    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
    {
        exits[nexits++] = pexit;
        if ( nexits > MAX_REXITS )
        {
            bug( "sort_exits: more than %d exits in room... fatal", nexits );
            return;
        }
    }
    qsort( &exits[0], nexits, sizeof( EXIT_DATA * ),
           (int(*)(const void *, const void *)) exit_comp );
    for ( x = 0; x < nexits; x++ )
    {
        if ( x > 0 )
            exits[x]->prev	= exits[x-1];
        else
        {
            exits[x]->prev	= NULL;
            room->first_exit	= exits[x];
        }
        if ( x >= (nexits - 1) )
        {
            exits[x]->next	= NULL;
            room->last_exit	= exits[x];
        }
        else
            exits[x]->next	= exits[x+1];
    }
}

void randomize_exits( ROOM_INDEX_DATA *room, sh_int maxdir )
{
    EXIT_DATA *pexit;
    int nexits, /* maxd, */ d0, d1, count, door; /* Maxd unused */
    int vdirs[MAX_REXITS];

    nexits = 0;
    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
        vdirs[nexits++] = pexit->vdir;

    for ( d0 = 0; d0 < nexits; d0++ )
    {
        if ( vdirs[d0] > maxdir )
            continue;
        count = 0;
        while ( vdirs[(d1 = number_range( d0, nexits - 1 ))] > maxdir
                ||      ++count > 5 );
        if ( vdirs[d1] > maxdir )
            continue;
        door		= vdirs[d0];
        vdirs[d0]	= vdirs[d1];
        vdirs[d1]	= door;
    }
    count = 0;
    for ( pexit = room->first_exit; pexit; pexit = pexit->next )
        pexit->vdir = vdirs[count++];

    sort_exits( room );
}


/*
 * Repopulate areas periodically.
 */
void area_update( void )
{
    AREA_DATA *pArea;

    for ( pArea = first_area; pArea; pArea = pArea->next )
    {
        CHAR_DATA *pch;
        int reset_age = pArea->reset_frequency ? pArea->reset_frequency : 15;

        if ( (reset_age == -1 && pArea->age == -1)
             ||    !IS_SET(pArea->flags, AFLAG_INITIALIZED)
             ||    ++pArea->age < (reset_age-1) )
            continue;

        /*
         * Check for PC's.
         */
        if ( pArea->nplayer > 0 && pArea->age == (reset_age-1) )
        {
            char buf[MAX_STRING_LENGTH];

            /* Rennard */
            if ( pArea->resetmsg )
                sprintf( buf, "%s\n\r", pArea->resetmsg );
            else
                buf[0] = '\0';

            for ( pch = first_char; pch; pch = pch->next )
            {
                if ( !IS_NPC(pch)
                     &&   IS_AWAKE(pch)
                     &&   pch->in_room
                     &&   pch->in_room->area == pArea )
                {
                    set_char_color( AT_RESET, pch );
                    send_to_char( buf, pch );
                }
            }
        }

        /*
         * Check age and reset.
         * Note: Mud Academy resets every 3 minutes (not 15).
         */
        if ( pArea->nplayer == 0 || pArea->age >= reset_age )
        {
            ROOM_INDEX_DATA *pRoomIndex;

            log_printf_plus( LOG_BUILD, LEVEL_LOG_CSET, SEV_SPAM+9,
                             "Resetting: %s", pArea->name );
            reset_area( pArea );
            if ( reset_age == -1 )
                pArea->age = -1;
            else
                pArea->age = number_range( 0, reset_age / 5 );
            pRoomIndex = get_room_index( ROOM_VNUM_SCHOOL );
            if ( pRoomIndex != NULL && pArea == pRoomIndex->area
                 &&   pArea->reset_frequency == 0 )
                pArea->age = 15 - 3;
        }
    }
    return;
}


/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile( int vnum )
{
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *mob;
    int i;

    if ( !mob_exists_index(vnum) )
    {
        bug( "Create_mobile: mobile %d not in index", vnum );
        abort();
    }

    pMobIndex = get_mob_index(vnum);

    if ( !pMobIndex )
    {
        bug( "Create_mobile: NULL pMobIndex." );
        abort();
    }

    mob = new_char();
    clear_char( mob );

    mob->pIndexData		= pMobIndex;
    mob->vnum                   = pMobIndex->ivnum;

    mob->editor			= NULL;
    mob->name			= QUICKLINK( pMobIndex->player_name );
    mob->short_descr		= QUICKLINK( pMobIndex->short_descr );
    mob->long_descr		= QUICKLINK( pMobIndex->long_descr  );
    mob->description		= QUICKLINK( pMobIndex->description );
    mob->intro_descr		= NULL;
    mob->spec_fun		= pMobIndex->spec_fun;
    mob->mpscriptpos		= 0;
    mob->act			= pMobIndex->act;
    mob->act2			= pMobIndex->act2;
    mob->affected_by		= pMobIndex->affected_by;
    mob->affected_by2		= pMobIndex->affected_by2;
    mob->alignment		= pMobIndex->alignment;
    mob->sex			= pMobIndex->sex;

    if ( pMobIndex->ac )
        mob->armor		= pMobIndex->ac;
    else
        mob->armor		= interpolate( GetMaxLevel(mob), 100, -100);

    if ( !pMobIndex->hitnodice )
        mob->max_hit		= dice(GetMaxLevel(mob), 8);
    else
        mob->max_hit              = dice(pMobIndex->hitnodice, pMobIndex->hitsizedice)
            + pMobIndex->hitplus;
    GET_HIT(mob)		= GET_MAX_HIT(mob);
    /* lets put things back the way they used to be! -Thoric */
    GET_EXP(mob)	       	= GET_EXP(pMobIndex);
    mob->position		= pMobIndex->position;
    mob->defposition		= pMobIndex->defposition;
    mob->barenumdie		= pMobIndex->damnodice;
    mob->baresizedie		= pMobIndex->damsizedice;
    mob->mobthac0		= pMobIndex->mobthac0;
    mob->hitplus		= pMobIndex->hitplus;
    mob->damplus		= pMobIndex->damplus;

    mob->perm_str		= pMobIndex->perm_str;
    mob->perm_dex		= pMobIndex->perm_dex;
    mob->perm_wis		= pMobIndex->perm_wis;
    mob->perm_int		= pMobIndex->perm_int;
    mob->perm_con		= pMobIndex->perm_con;
    mob->perm_cha		= pMobIndex->perm_cha;
    mob->perm_lck 		= pMobIndex->perm_lck;
    mob->hitroll		= pMobIndex->hitroll;
    mob->damroll		= pMobIndex->damroll;
    mob->race			= pMobIndex->race;

    for (i=0;i<MAX_CURR_TYPE;i++)
        GET_MONEY(mob,i)        = GET_MONEY(pMobIndex,i);
    /*
     * temp stuff for mult ch_class
     */
    for (i = 0; i < MAX_CLASS; ++i) {
        mob->classes[i]         = pMobIndex->classes[i];
        if (IS_ACTIVE(mob, i))
            mob->levels[i]      = number_fuzzy(pMobIndex->levels[i]);
    }
    mob->saving_poison_death	= pMobIndex->saving_poison_death;
    mob->saving_wand		= pMobIndex->saving_wand;
    mob->saving_para_petri	= pMobIndex->saving_para_petri;
    mob->saving_breath		= pMobIndex->saving_breath;
    mob->saving_spell_staff	= pMobIndex->saving_spell_staff;
    mob->height			= pMobIndex->height;
    mob->weight			= pMobIndex->weight;
    mob->resistant		= pMobIndex->resistant;
    mob->immune			= pMobIndex->immune;
    mob->absorb			= pMobIndex->absorb;
    mob->susceptible		= pMobIndex->susceptible;
    mob->attacks		= pMobIndex->attacks;
    mob->defenses		= pMobIndex->defenses;
    mob->numattacks		= pMobIndex->numattacks;
    mob->speaks			= pMobIndex->speaks;
    mob->speaking		= pMobIndex->speaking;
    if ( pMobIndex->xflags == 0 )
        pMobIndex->xflags       = race_bodyparts( mob );
    mob->xflags	                = pMobIndex->xflags;

    if (mob->numattacks > 20000)
        log_printf_plus(LOG_BUILD, sysdata.log_level, SEV_DEBUG, "Mob %s numattacks == %d", mob->name, mob->numattacks);

    /*
     * Insert in list.
     */
    add_char( mob );
    pMobIndex->count++;
    nummobsloaded++;
    return mob;
}



/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object( int vnum )
{
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    AFFECT_DATA *paf, *cpaf;

    if ( !obj_exists_index(vnum) )
    {
        bug( "Create_object: object %d not in index", vnum );
        abort();
    }

    pObjIndex = get_obj_index(vnum);

    if ( !pObjIndex )
    {
        bug( "Create_object: NULL pObjIndex." );
        abort();
    }

    obj = new_obj();

    obj->pIndexData	= pObjIndex;
    obj->vnum           = pObjIndex->ivnum;

    obj->in_room	= NULL;
    obj->wear_loc	= -1;
    obj->count		= 1;
    obj->timer		= 0;

    obj->name		= QUICKLINK( pObjIndex->name 	 );
    obj->short_descr	= QUICKLINK( pObjIndex->short_descr );
    obj->description	= QUICKLINK( pObjIndex->description );
    obj->action_desc	= QUICKLINK( pObjIndex->action_desc );
    obj->item_type	= pObjIndex->item_type;
    obj->extra_flags	= pObjIndex->extra_flags;
    obj->extra_flags2	= pObjIndex->extra_flags2;
    obj->magic_flags	= pObjIndex->magic_flags;
    obj->wear_flags	= pObjIndex->wear_flags;
    obj->value[0]	= pObjIndex->value[0];
    obj->value[1]	= pObjIndex->value[1];
    obj->value[2]	= pObjIndex->value[2];
    obj->value[3]	= pObjIndex->value[3];
    obj->value[4]	= pObjIndex->value[4];
    obj->value[5]	= pObjIndex->value[5];
    obj->weight		= pObjIndex->weight;
    obj->cost		= pObjIndex->cost;
    obj->rent		= pObjIndex->rent;
    obj->currtype       = URANGE(FIRST_CURR, pObjIndex->currtype, LAST_CURR);
    obj->spec_fun	= pObjIndex->spec_fun;

    for ( cpaf = pObjIndex->first_affect; cpaf; cpaf = cpaf->next )
    {
        CREATE( paf, AFFECT_DATA, 1 );
        paf->type		= cpaf->type;
        paf->duration		= cpaf->duration;
        paf->location		= cpaf->location;
        paf->modifier		= cpaf->modifier;
        paf->bitvector		= cpaf->bitvector;
        LINK( paf, obj->first_affect, obj->last_affect, next, prev );
        top_affect++;
    }

    /*
     obj->cost		= number_fuzzy( 10 )
     * number_fuzzy( level ) * number_fuzzy( level );
     */

    if (obj->item_type<0 || obj->item_type>MAX_ITEM_TYPE)
    {
        bug( "Create_object: vnum %d bad type.", pObjIndex->ivnum );
        bug( "------------------------> %d", obj->item_type );
    }

    /*
     * Mess with object properties.
     */
    switch ( obj->item_type )
    {
    case ITEM_NONE:
    case ITEM_LIGHT:
    case ITEM_TREASURE:
    case ITEM_FURNITURE:
    case ITEM_TRASH:
    case ITEM_CONTAINER:
    case ITEM_DRINK_CON:
    case ITEM_KEY:
        break;

    case ITEM_FOOD:
        /*
         * optional food condition (rotting food)		-Thoric
         * value1 is the max condition of the food
         * value4 is the optional initial condition
         */
        if ( obj->value[4] )
            obj->timer = obj->value[4];
        else
            obj->timer = obj->value[1];
        break;

    case ITEM_BOAT:
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
    case ITEM_FOUNTAIN:
    case ITEM_BLOOD:
    case ITEM_BLOODSTAIN:
    case ITEM_SCRAPS:
    case ITEM_PIPE:
    case ITEM_HERB_CON:
    case ITEM_INCENSE:
    case ITEM_FIRE:
    case ITEM_BOOK:
    case ITEM_SWITCH:
    case ITEM_LEVER:
    case ITEM_PULLCHAIN:
    case ITEM_BUTTON:
    case ITEM_DIAL:
    case ITEM_RUNE:
    case ITEM_RUNEPOUCH:
    case ITEM_MATCH:
    case ITEM_TRAP:
    case ITEM_MAP:
    case ITEM_PAPER:
    case ITEM_PEN:
    case ITEM_TINDER:
    case ITEM_LOCKPICK:
    case ITEM_SPIKE:
    case ITEM_DISEASE:
    case ITEM_OIL:
    case ITEM_FUEL:
    case ITEM_SHOVEL:
        break;

    case ITEM_MISSILE_WEAPON:
        REMOVE_BIT(obj->wear_flags, ITEM_MISSILE_WIELD);
        break;
    case ITEM_PROJECTILE:
        REMOVE_BIT(obj->wear_flags, ITEM_WIELD);
        SET_BIT(obj->wear_flags, ITEM_MISSILE_WIELD);
        break;
    case ITEM_QUIVER:
        break;

    case ITEM_FIREWEAPON:
        bug( "Create_object: vnum %d was ITEM_FIREWEAPON.", pObjIndex->ivnum );
	obj->item_type = ITEM_MISSILE_WEAPON;
        REMOVE_BIT(obj->wear_flags, ITEM_MISSILE_WIELD);
	break;

    case ITEM_MISSILE:
        bug( "Create_object: vnum %d was ITEM_MISSILE.", pObjIndex->ivnum );
        obj->item_type = ITEM_PROJECTILE;
        REMOVE_BIT(obj->wear_flags, ITEM_WIELD);
        SET_BIT(obj->wear_flags, ITEM_MISSILE_WIELD);
	break;

    case ITEM_HERB:
        obj->value[3]	= IS_SET( obj->value[3], PIPE_SINGLE_USE ) ? PIPE_SINGLE_USE : 0;
        break;

    case ITEM_PORTAL:
        obj->item_type = ITEM_NONE;
        break;

    case ITEM_SALVE:
        obj->value[3]	= number_fuzzy( obj->value[3] );
        break;

    case ITEM_SCROLL:
        obj->value[0]	= number_fuzzy( obj->value[0] );
        break;

    case ITEM_WAND:
    case ITEM_STAFF:
        obj->value[0]	= number_fuzzy( obj->value[0] );
        obj->value[1]	= number_fuzzy( obj->value[1] );
        obj->value[2]	= obj->value[1];
        break;

    case ITEM_WEAPON:
        /*
         if ( obj->value[1] && obj->value[2] )
         obj->value[2] *= obj->value[1];
         else
         {
         obj->value[1] = number_fuzzy( number_fuzzy( 1 * 30 / 4 + 2 ) );
         obj->value[2] = number_fuzzy( number_fuzzy( 3 * 30 / 4 + 6 ) );
         }
         */
        obj->value[3]--;
        if (obj->value[1]==0 || obj->value[2]==0)
            log_printf_plus(LOG_NORMAL, LEVEL_LOG_CSET, SEV_SPAM, "Weapon %s value1==%d value2==%d",
                       obj->name, obj->value[1], obj->value[2]);
        if (obj->value[0] == 0)
            obj->value[0] = INIT_WEAPON_CONDITION;
        break;

    case ITEM_ARMOR:
        if ( obj->value[0] == 0 )
            obj->value[0]	= number_fuzzy( 30 / 4 + 2 );
        if (obj->value[1] == 0)
            obj->value[1] = obj->value[0];
        break;

    case ITEM_POTION:
    case ITEM_PILL:
        obj->value[0]	= number_fuzzy( number_fuzzy( obj->value[0] ) );
        break;

    case ITEM_MONEY:
	if (obj->value[0]<=0)
	    obj->value[0] = obj->cost;
        obj->value[2]   = URANGE(FIRST_CURR, pObjIndex->value[2], LAST_CURR);
        break;
    }

    LINK( obj, first_object, last_object, next, prev );
    ++pObjIndex->count;
    ++numobjsloaded;
    ++physicalobjects;

    return obj;
}

int def_color( sh_int i )
{
    switch( i )
    {
    case AT_BLACK:	return(DAT_BLACK);
    case AT_BLOOD:	return(DAT_BLOOD);
    case AT_DGREEN:	return(DAT_DGREEN);
    case AT_ORANGE:	return(DAT_ORANGE);
    case AT_DBLUE:	return(DAT_DBLUE);
    case AT_PURPLE:	return(DAT_PURPLE);
    case AT_CYAN:	return(DAT_CYAN);
    case AT_GREY:	return(DAT_GREY);
    case AT_DGREY:	return(DAT_DGREY);
    case AT_RED:	return(DAT_RED);
    case AT_GREEN:	return(DAT_GREEN);
    case AT_YELLOW:	return(DAT_YELLOW);
    case AT_BLUE:	return(DAT_BLUE);
    case AT_PINK:	return(DAT_PINK);
    case AT_LBLUE:	return(DAT_LBLUE);
    case AT_WHITE:	return(DAT_WHITE);
    case AT_BLINK:	return(DAT_BLINK);
    case AT_PLAIN:	return(DAT_PLAIN);
    case AT_ACTION:	return(DAT_ACTION);
    case AT_SAY:	return(DAT_SAY);
    case AT_GOSSIP:	return(DAT_GOSSIP);
    case AT_OOC:	return(DAT_OOC);
    case AT_TELL:	return(DAT_TELL);
    case AT_HIT:	return(DAT_HIT);
    case AT_HITME:	return(DAT_HITME);
    case AT_IMMORT:	return(DAT_IMMORT);
    case AT_HURT:	return(DAT_HURT);
    case AT_FALLING:	return(DAT_FALLING);
    case AT_DANGER:	return(DAT_DANGER);
    case AT_MAGIC:	return(DAT_MAGIC);
    case AT_CONSIDER:	return(DAT_CONSIDER);
    case AT_REPORT:	return(DAT_REPORT);
    case AT_POISON:	return(DAT_POISON);
    case AT_SOCIAL:	return(DAT_SOCIAL);
    case AT_DYING:	return(DAT_DYING);
    case AT_DEAD:	return(DAT_DEAD);
    case AT_SKILL:	return(DAT_SKILL);
    case AT_CARNAGE:	return(DAT_CARNAGE);
    case AT_DAMAGE:	return(DAT_DAMAGE);
    case AT_FLEE:	return(DAT_FLEE);
    case AT_RMNAME:	return(DAT_RMNAME);
    case AT_RMDESC:	return(DAT_RMDESC);
    case AT_OBJECT:	return(DAT_OBJECT);
    case AT_PERSON:	return(DAT_PERSON);
    case AT_LIST:	return(DAT_LIST);
    case AT_BYE:	return(DAT_BYE);
    case AT_GOLD:	return(DAT_GOLD);
    case AT_GTELL:	return(DAT_GTELL);
    case AT_NOTE:	return(DAT_NOTE);
    case AT_HUNGRY:	return(DAT_HUNGRY);
    case AT_THIRSTY:	return(DAT_THIRSTY);
    case AT_FIRE:	return(DAT_FIRE);
    case AT_SOBER:	return(DAT_SOBER);
    case AT_WEAROFF:	return(DAT_WEAROFF);
    case AT_SCORE:	return(DAT_SCORE);
    case AT_SCORE2:	return(DAT_SCORE2);
    case AT_SCORE3:	return(DAT_SCORE3);
    case AT_SCORE4:	return(DAT_SCORE4);
    case AT_RESET:	return(DAT_RESET);
    case AT_LOG:	return(DAT_LOG);
    case AT_DIEMSG:	return(DAT_DIEMSG);
    case AT_WARTALK:	return(DAT_WARTALK);
    case AT_WHO:	return(DAT_WHO);
    case AT_WHO2:	return(DAT_WHO2);
    case AT_WHO3:	return(DAT_WHO3);
    case AT_WHO4:	return(DAT_WHO4);
    case AT_CHESS1:	return(DAT_CHESS1);
    case AT_CHESS2:	return(DAT_CHESS2);
    case AT_DIR_NORTH:  return(DAT_DIR_NORTH);
    case AT_DIR_SOUTH:  return(DAT_DIR_SOUTH);
    case AT_DIR_WEST:   return(DAT_DIR_WEST);
    case AT_DIR_EAST:   return(DAT_DIR_EAST);
    case AT_DIR_UP:     return(DAT_DIR_UP);
    case AT_DIR_DOWN:   return(DAT_DIR_DOWN);
    case AT_DIR_NORTHWEST: return(DAT_DIR_NORTHWEST);
    case AT_DIR_NORTHEAST: return(DAT_DIR_NORTHEAST);
    case AT_DIR_SOUTHWEST: return(DAT_DIR_SOUTHWEST);
    case AT_DIR_SOUTHEAST: return(DAT_DIR_SOUTHEAST);
    case AT_WEATHER:    return(DAT_WEATHER);
    case MAX_COLOR_TYPE:return(DAT_GREY);
    }
    return(DAT_GREY);
}

void SetDefaultColor( CHAR_DATA *ch )
{
    sh_int i;

    for (i=0;i<MAX_COLOR_TYPE;i++)
        ch->colors[i] = def_color(i);
}


CHAR_DATA *new_char(void)
{
    CHAR_DATA *ch;
    CREATE(ch, CHAR_DATA, 1);
    ch->unum = unum++;
    return ch;
}

OBJ_DATA *new_obj(void)
{
    OBJ_DATA *obj;
    CREATE(obj, OBJ_DATA, 1);
    obj->unum = unum++;
    return obj;
}


/*
 * Clear a new character.
 */
void clear_char( CHAR_DATA *ch )
{
    int i;

    ch->editor			= NULL;
    ch->hunting			= NULL;
    ch->fearing			= NULL;
    ch->hating			= NULL;
    ch->name			= NULL;
    ch->short_descr		= NULL;
    ch->long_descr		= NULL;
    ch->description		= NULL;
    ch->intro_descr		= NULL;
    ch->next			= NULL;
    ch->prev			= NULL;
    ch->first_carrying		= NULL;
    ch->last_carrying		= NULL;
    ch->next_in_room		= NULL;
    ch->prev_in_room		= NULL;
    ch->fighting		= NULL;
    ch->switched		= NULL;
    ch->first_affect		= NULL;
    ch->last_affect		= NULL;
    ch->prev_cmd		= NULL;    /* maps */
    ch->last_cmd		= NULL;
    ch->dest_buf		= NULL;
    ch->spare_ptr		= NULL;
    ch->mount			= NULL;
    ch->affected_by		= 0;
    ch->affected_by2		= 0;
    ch->logon			= current_time;
    ch->armor			= 100;
    ch->position		= POS_STANDING;
    ch->practice		= 0;
    GET_HIT(ch)			= 20;
    ch->max_hit			= 20;
    ch->hit_regen		= 0;
    GET_MANA(ch)		= 100;
    ch->max_mana		= 0;
    ch->mana_regen		= 0;
    GET_MOVE(ch)		= 100;
    ch->max_move		= 0;
    ch->move_regen		= 0;
    ch->height			= 72+(int)(72.0*((float)(dice(5,11)-30.0)/100.0));
    ch->weight			= 180+(int)(180.0*((float)(dice(5,11)-30.0)/100.0));;
    ch->xflags			= 0;
    ch->race			= 0;
    ch->antimagicp		= 0;
    ch->spellfail		= 101;

    for (i = 0; i < MAX_CLASS; i++)
    {
        ch->classes[i] = 0;
        ch->levels[i] = 0;
    }

    SetDefaultColor( ch );

    ch->speaking		= LANG_COMMON;
    ch->speaks			= LANG_COMMON;
    ch->barenumdie		= 1;
    ch->baresizedie		= 4;
    ch->substate		= 0;
    ch->tempnum			= 0;
    ch->perm_str		= 13;
    ch->perm_dex		= 13;
    ch->perm_int		= 13;
    ch->perm_wis		= 13;
    ch->perm_cha		= 13;
    ch->perm_con		= 13;
    ch->perm_lck		= 13;
    ch->mod_str			= 0;
    ch->mod_dex			= 0;
    ch->mod_int			= 0;
    ch->mod_wis			= 0;
    ch->mod_cha			= 0;
    ch->mod_con			= 0;
    ch->mod_lck			= 0;
    ch->pagelen                 = 24; 		     /* BUILD INTERFACE */
    ch->inter_page 		= NO_PAGE;           /* BUILD INTERFACE */
    ch->inter_type 		= NO_TYPE;           /* BUILD INTERFACE */
    ch->inter_editing    	= NULL;              /* BUILD INTERFACE */
    ch->inter_editing_vnum	= -1;                /* BUILD INTERFACE */
    ch->inter_substate    	= SUB_NORTH;         /* BUILD INTERFACE */
}


void remove_acro_player(char *name);

/*
 * Free a character.
 */
void free_char( CHAR_DATA *ch )
{
    CHAR_DATA *tch;
    OBJ_DATA *obj;
    AFFECT_DATA *paf;
    TIMER *timer;
    MPROG_ACT_LIST *mpact, *mpact_next;
    NOTE_DATA *comments, *comments_next;

    if ( !ch )
    {
        bug( "Free_char: null ch!" );
        return;
    }
    if ( ch->desc )
        bug( "Free_char: char still has descriptor." );

    while ( (obj = ch->last_carrying) != NULL )
        extract_obj( obj );

    while ( (paf = ch->last_affect) != NULL )
        affect_remove( ch, paf );

    while ( (timer = ch->first_timer) != NULL )
        extract_timer( ch, timer );

    if ( ch->editor )
        stop_editing( ch );

    STRFREE( ch->name		);
    STRFREE( ch->short_descr	);
    STRFREE( ch->long_descr	);
    STRFREE( ch->description	);

    if ( ch->intro_descr )
        DISPOSE( ch->intro_descr	);

    if ( ch->inter_editing )
        DISPOSE( ch->inter_editing );

    stop_hunting( ch );
    for (tch = first_char; tch; tch = tch->next)
        if (!char_died(tch) && is_hunting(tch, ch))
            stop_hunting(tch);

    stop_hating( ch );
    for (tch = first_char; tch; tch = tch->next)
        if (!char_died(tch) && is_hating(tch, ch))
            stop_hating(tch);

    stop_fearing( ch );
    for (tch = first_char; tch; tch = tch->next)
        if (!char_died(tch) && is_fearing(tch, ch))
            stop_fearing(tch);

    free_fight( ch );

    if ( ch->pnote )
        free_note( ch->pnote );

    if ( ch->vars )
        free_variables( ch->vars );

    if ( ch->pcdata )
    {
        if (ch->pcdata->clan_name)
            STRFREE( ch->pcdata->clan_name	);
        if (ch->pcdata->council_name)
            STRFREE( ch->pcdata->council_name );
        if (ch->pcdata->deity_name)
            STRFREE( ch->pcdata->deity_name	);
        if (ch->pcdata->pwd)
            DISPOSE( ch->pcdata->pwd	);  /* no hash */
        /* May cause small memory leak... look into fixing in future */
        if (ch->pcdata->bamfout)
            STRFREE( ch->pcdata->bamfout );
        if (ch->pcdata->bamfin)
            STRFREE( ch->pcdata->bamfin	);

        if (ch->pcdata->rank)
            DISPOSE( ch->pcdata->rank	);
        if (ch->pcdata->title)
            STRFREE( ch->pcdata->title	);
        if (ch->pcdata->bio)
            STRFREE( ch->pcdata->bio	);
        if (ch->pcdata->bestowments)
            DISPOSE( ch->pcdata->bestowments ); /* no hash */
        if (ch->pcdata->homepage)
            DISPOSE( ch->pcdata->homepage	);  /* no hash */
        if (ch->pcdata->authed_by)
            STRFREE( ch->pcdata->authed_by	);
        if (ch->pcdata->prompt)
            STRFREE( ch->pcdata->prompt	);
        if (ch->pcdata->subprompt)
            STRFREE( ch->pcdata->subprompt );

        free_aliases( ch );
	free_game( ch->pcdata->game_board );
	free_vtracks( ch );
        free_intros( ch );

        remove_acro_player( GET_NAME(ch) );
#ifdef IMC
        imc_freechardata( ch );
#endif
#ifdef I3
        free_i3chardata( ch );
#endif
        DISPOSE( ch->pcdata );
    }

    for ( mpact = ch->mpact; mpact; mpact = mpact_next )
    {
        mpact_next = mpact->next;
        if (mpact->buf)
            DISPOSE( mpact->buf );
        DISPOSE( mpact	    );
    }

    for ( comments = ch->comments; comments; comments = comments_next )
    {
        comments_next = comments->next;
        if (comments->text)
            STRFREE( comments->text    );
        if (comments->to_list)
            STRFREE( comments->to_list );
        if (comments->subject)
            STRFREE( comments->subject );
        if (comments->sender)
            STRFREE( comments->sender  );
        if (comments->date)
            STRFREE( comments->date    );
        DISPOSE( comments          );
    }

    DISPOSE( ch );
    return;
}



/*
 * Get an extra description from a list.
 */
char *get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed )
{
    for ( ; ed; ed = ed->next )
        if ( is_name( name, ed->keyword ) )
            return ed->description;

    return NULL;
}



/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index( int vnum )
{
    MOB_INDEX_DATA *pMobIndex;

    if ( vnum < 0 )
        vnum = 0;

    for ( pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH];
          pMobIndex;
          pMobIndex  = pMobIndex->next )
        if ( pMobIndex->ivnum == vnum )
            return pMobIndex;

//    if ( fBootDb )
//        bug( "Get_mob_index: bad vnum %d.", vnum );

    return NULL;
}

bool mob_exists_index( int vnum )
{
    if ( get_mob_index(vnum ) )
        return TRUE;
    return FALSE;
}


/*
 * Translates obj virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum )
{
    OBJ_INDEX_DATA *pObjIndex;

    if ( vnum < 0 )
        vnum = 0;

    for ( pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH];
          pObjIndex;
          pObjIndex  = pObjIndex->next )
        if ( pObjIndex->ivnum == vnum )
            return pObjIndex;

//    if ( fBootDb )
//        bug( "Get_obj_index: bad vnum %d.", vnum );

    return NULL;
}

bool obj_exists_index( int vnum )
{
    if ( get_obj_index(vnum ) )
        return TRUE;
    return FALSE;
}


/*
 * Translates room virtual number to its room index struct.
 * Hash table lookup.
 */
/*#if defined(USE_DB)*/
#if 0
ROOM_INDEX_DATA *get_room_index( int vnum )
{
    return db_get_room_index(vnum);
}
#else
ROOM_INDEX_DATA *get_room_index( int vnum )
{
    ROOM_INDEX_DATA *pRoomIndex;

    if ( vnum < 0 )
        vnum = 0;

    for ( pRoomIndex  = room_index_hash[vnum % MAX_KEY_HASH];
          pRoomIndex;
          pRoomIndex  = pRoomIndex->next )
        if ( pRoomIndex->vnum == vnum )
            return pRoomIndex;

//    if ( fBootDb )
//        bug( "Get_room_index: bad vnum %d.", vnum );

    return NULL;
}
#endif

bool room_exists_index( int vnum )
{
    if ( get_room_index(vnum ) )
        return TRUE;
    return FALSE;
}


bool is_other_plane(ROOM_INDEX_DATA *r1, ROOM_INDEX_DATA *r2)
{
    if (!r1->area || !r2->area)
        return TRUE;

    if (r1->area->plane == PLANE_UNIQUE ||
        r2->area->plane == PLANE_UNIQUE)
        return TRUE;

    if (r1->area->plane == r2->area->plane)
        return FALSE;

    return TRUE;
}


/*
 * Added lots of EOF checks, as most of the file crashes are based on them.
 * If an area file encounters EOF, the fread_* functions will shutdown the
 * MUD, as all area files should be read in in full or bad things will
 * happen during the game.  Any files loaded in without fBootDb which
 * encounter EOF will return what they have read so far.   These files
 * should include player files, and in-progress areas that are not loaded
 * upon bootup.
 * -- Altrag
 */


/*
 * Read a letter from a file.
 */
char fread_letter( FILE *fp )
{
    char c;

    do
    {
        if ( feof(fp) )
        {
            bug("fread_letter: EOF encountered on read.\n\r");
            if ( fBootDb )
                exit(1);
            return '\0';
        }
        c = getc( fp );
    }
    while ( isspace(c) );

    return c;
}



/*
 * Read a number from a file.
 */
int fread_number( FILE *fp )
{
    int number;
    bool sign;
    char c;

    do
    {
        if ( feof(fp) )
        {
            bug("fread_number: EOF encountered on read.\n\r");
            if ( fBootDb )
                exit(1);
            return 0;
        }
        c = getc( fp );
    }
    while ( isspace(c) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
        c = getc( fp );
    }
    else if ( c == '-' )
    {
        sign = TRUE;
        c = getc( fp );
    }

    if ( !isdigit(c) )
    {
        bug( "Fread_number: bad format. (%c)", c );
        if ( fBootDb )
            exit( 1 );
        return 0;
    }

    while ( isdigit(c) )
    {
        if ( feof(fp) )
        {
            bug("fread_number: EOF encountered on read.\n\r");
            if ( fBootDb )
                exit(1);
            return number;
        }
        number = number * 10 + c - '0';
        c      = getc( fp );
    }

    if ( sign )
        number = 0 - number;

    if ( c == '|' )
        number += fread_number( fp );
    else if ( c != ' ' )
        ungetc( c, fp );

    return number;
}


/*
 * custom str_dup using create					-Thoric
 */
char *str_dup( char const *str )
{
    static char *ret;
    int len;

    if ( !str )
        return NULL;

    len = strlen(str)+1;

    CREATE( ret, char, len );
    strcpy( ret, str );
    return ret;
}

/*
 * Read a string from file fp
 */
char *fread_string( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;
    char c;
    int ln;

    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
        if ( feof(fp) )
        {
            bug("fread_string: EOF encountered on read.\n\r");
            if ( fBootDb )
                exit(1);
            return STRALLOC("");
        }
        c = getc( fp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '~' )
        return STRALLOC( "" );

    for ( ;; )
    {
        if ( ln >= (MAX_STRING_LENGTH - 1) )
        {
            bug( "fread_string: string too long" );
            *plast = '\0';
            return STRALLOC( buf );
        }
        switch ( *plast = getc( fp ) )
        {
        default:
            plast++; ln++;
            break;

        case EOF:
            bug( "Fread_string: EOF" );
            if ( fBootDb )
                exit( 1 );
            *plast = '\0';
            return STRALLOC(buf);
            break;

        case '\n':
            plast++;  ln++;
            *plast++ = '\r';  ln++;
            break;

        case '\r':
            break;

        case '~':
            *plast = '\0';
            return STRALLOC( buf );
        }
    }
}

/*
 * Read a string from file fp using str_dup (ie: no string hashing)
 */
char *fread_string_nohash( FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;
    char c;
    int ln;

    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
        if ( feof(fp) )
        {
            bug("fread_string_no_hash: EOF encountered on read.\n\r");
            if ( fBootDb )
                exit(1);
            return str_dup("");
        }
        c = getc( fp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '~' )
        return str_dup( "" );

    for ( ;; )
    {
        if ( ln >= (MAX_STRING_LENGTH - 1) )
        {
            bug( "fread_string_no_hash: string too long" );
            *plast = '\0';
            return str_dup( buf );
        }
        switch ( *plast = getc( fp ) )
        {
        default:
            plast++; ln++;
            break;

        case EOF:
            bug( "Fread_string_no_hash: EOF" );
            if ( fBootDb )
                exit( 1 );
            *plast = '\0';
            return str_dup(buf);
            break;

        case '\n':
            plast++;  ln++;
            *plast++ = '\r';  ln++;
            break;

        case '\r':
            break;

        case '~':
            *plast = '\0';
            return str_dup( buf );
        }
    }
}



/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp )
{
    char c;

    do
    {
        if ( feof(fp) )
        {
            bug("fread_to_eol: EOF encountered on read.\n\r");
            if ( fBootDb )
                exit(1);
            return;
        }
        c = getc( fp );
    }
    while ( c != '\n' && c != '\r' );

    do
    {
        c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}

/*
 * Read to end of line into static buffer			-Thoric
 */
#if 0
char *fread_line( FILE *fp )
{
    static char line[MAX_STRING_LENGTH];
    char *pline;
    char c;
    int ln;

    pline = line;
    line[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
        if ( feof(fp) )
        {
            bug("fread_line: EOF encountered on read.");
            if ( fBootDb )
                exit(1);
            strcpy(line, "");
            return line;
        }
        c = getc( fp );
    }
    while ( isspace(c) );

    ungetc( c, fp );
    do
    {
        if ( feof(fp) )
        {
            bug("fread_line: EOF encountered on read.");
            if ( fBootDb )
                exit(1);
            *pline = '\0';
            return line;
        }
        c = getc( fp );
        *pline++ = c; ln++;
        if ( ln >= (MAX_STRING_LENGTH - 1) )
        {
            bug( "fread_line: line too long" );
            break;
        }
    }
    while ( c != '\n' && c != '\r' );

    do
    {
        c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    *pline = '\0';
    return line;
}
#else
char *fread_line( FILE *fp )
{
    static char line[MAX_STRING_LENGTH+1];
    char *pline;
    int x;

    x = 0;

    while ((pline = fgets(line, MAX_STRING_LENGTH, fp)))
    {
        if (pline[0] != '\0' && pline[0] != '\n')
            break;
    }

    if (!pline)
    {
        bug("fread_line: EOF encountered on read.");
        if ( fBootDb )
            exit(1);
        line[0] = '\0';
        return line;
    }

    while (isspace(*pline)) pline++;

    if (strlen(pline) >= MAX_STRING_LENGTH)
        bug( "fread_line: line too long" );

    return pline;
}
#endif

/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp )
{
    static char word[MAX_INPUT_LENGTH];
    char *pword;
    char cEnd;

    do
    {
        if ( feof(fp) )
        {
            bug("fread_word: EOF encountered on read.");
            if ( fBootDb )
                exit(1);
            word[0] = '\0';
            return word;
        }
        cEnd = getc( fp );
    }
    while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
        pword   = word;
    }
    else
    {
        word[0] = cEnd;
        pword   = word+1;
        cEnd    = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
        if ( feof(fp) )
        {
            bug("fread_word: EOF encountered on read.");
            if ( fBootDb )
                exit(1);
            *pword = '\0';
            return word;
        }
        *pword = getc( fp );
        if ( cEnd == ' ' ? isspace(*pword) : *pword == cEnd )
        {
            if ( cEnd == ' ' )
                ungetc( *pword, fp );
            *pword = '\0';
            return word;
        }
    }

    bug( "Fread_word: word too long" );
    exit( 1 );
    return NULL;
}


void do_memory( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int hash;

    argument = one_argument( argument, arg );
    ch_printf( ch, "CharAff    %6d\n\r", char_affects );
    ch_printf( ch, "1stSpl     %6d    1stSkl     %6d\n\r", gsn_first_spell, gsn_first_skill );
    ch_printf( ch, "1stWpn     %6d    1stTng     %6d\n\r", gsn_first_weapon, gsn_first_tongue );
    ch_printf( ch, "1stLor     %6d    1stPsiSpl  %6d\n\r", gsn_first_lore, gsn_first_psispell);
    ch_printf( ch, "TopSn      %6d\n\r", gsn_top_sn);
    ch_printf( ch, "Affects    %6d    Areas      %6d\n\r",  top_affect, top_area   );
    ch_printf( ch, "ExtDes     %6d    Exits      %6d\n\r", top_ed,	 top_exit   );
    ch_printf( ch, "Helps      %6d    Resets     %6d\n\r", top_help,   top_reset  );
    ch_printf( ch, "IdxMobs    %6d    Mobs       %6d\n\r",
               top_mob_index, nummobsloaded );
    ch_printf( ch, "IdxObjs    %6d    Objs       %6d (%6d)\n\r",
               top_obj_index, numobjsloaded, physicalobjects );
    ch_printf( ch, "TopMobVnum %6d    TopObjVnum %6d             TopRoomVnum    %6d\n\r",
               top_mob_vnum, top_obj_vnum,  top_room_vnum );
    ch_printf( ch, "Rooms      %6d    VRooms     %6d\n\r", top_room, top_vroom   );
    ch_printf( ch, "Shops      %6d    RepShps    %6d\n\r", top_shop,   top_repair );
    ch_printf( ch, "Players    %6d    Maxplrs    %6d\n\r", num_descriptors, sysdata.maxplayers );
    ch_printf( ch, "MaxEver    %6d    Topsn      %6d (%6d)\n\r", sysdata.alltimemax, top_sn, MAX_SKILL );
    ch_printf( ch, "TotLog     %6d    TopUnum    %6d\n\r", sysdata.total_logins, unum );
    ch_printf( ch, "MaxEver time recorded at:   %s\n\r", sysdata.time_of_max );
    if ( !str_cmp( arg, "check" ) )
    {
#ifdef HASHSTR
        send_to_char( check_hash(argument), ch );
#else
        send_to_char( "Hash strings not enabled.\n\r", ch );
#endif
        return;
    }
    if ( !str_cmp( arg, "showhigh" ) )
    {
#ifdef HASHSTR
        show_high_hash( atoi(argument) );
#else
        send_to_char( "Hash strings not enabled.\n\r", ch );
#endif
        return;
    }
    if ( argument[0] != '\0' )
        hash = atoi(argument);
    else
        hash = -1;
    if ( !str_cmp( arg, "hash" ) )
    {
#ifdef HASHSTR
        ch_printf( ch, "Hash statistics:\n\r%s", hash_stats() );
        if ( hash != -1 )
            hash_dump( hash );
#else
        send_to_char( "Hash strings not enabled.\n\r", ch );
#endif
    }
    return;
}



/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy( int number )
{
    switch ( number_bits( 2 ) )
    {
    case 0:  number -= 1; break;
    case 3:  number += 1; break;
    }

    return UMAX( 1, number );
}



/*
 * Generate a random number.
 */
int number_range( int from, int to )
{

    if ( ( to - from ) < 1 )
        return from;

    return (number_mm() % (to-from+1)) + from;
}



/*
 * Generate a percentile roll.
 */
int number_percent( void )
{
    /*    int percent;

     while ( ( percent = number_mm( ) & (128-1) ) > 99 )
     ;

     return 1 + percent;*/
    return number_mm() % 100;
}



/*
 * Generate a random door.
 */
sh_int number_door( void )
{
    sh_int door;

    while ( ( door = number_mm( ) & (16-1) ) > 9 )
        ;

    return door;
    /*    return number_mm() & 10; */
}



int number_bits( int width )
{
    return number_mm( ) & ( ( 1 << width ) - 1 );
}



/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */
static	int	rgiState[2+55];

void init_mm( )
{
    int *piState;
    int iState;

    piState	= &rgiState[2];

    piState[-2]	= 55 - 55;
    piState[-1]	= 55 - 24;

    piState[0]	= ((int) current_time) & ((1 << 30) - 1);
    piState[1]	= 1;
    for ( iState = 2; iState < 55; iState++ )
    {
        piState[iState] = (piState[iState-1] + piState[iState-2])
            & ((1 << 30) - 1);
    }
    return;
}



int number_mm( void )
{
    int *piState;
    int iState1;
    int iState2;
    int iRand;

    piState		= &rgiState[2];
    iState1	 	= piState[-2];
    iState2	 	= piState[-1];
    iRand	 	= (piState[iState1] + piState[iState2])
        & ((1 << 30) - 1);
    piState[iState1]	= iRand;
    if ( ++iState1 == 55 )
        iState1 = 0;
    if ( ++iState2 == 55 )
        iState2 = 0;
    piState[-2]		= iState1;
    piState[-1]		= iState2;
    return iRand >> 6;
}



/*
 * Roll some dice.						-Thoric
 */
int dice( int number, int size )
{
    int idice;
    int sum;

    switch ( size )
    {
    case 0: return 0;
    case 1: return number;
    }

    for ( idice = 1, sum = 0; idice <= number; idice++ )
        sum += number_range(1, size);

    return sum;
}



/*
 * Simple linear interpolation.
 */
int interpolate( int level, int value_00, int value_32 )
{
    return value_00 + level * (value_32 - value_00) / 32;
}


/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
        if ( *str == '~' )
            *str = '-';

    return;
}

/*
 * Encodes the tildes in a string.				-Thoric
 * Used for player-entered strings that go into disk files.
 */
void hide_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
        if ( *str == '~' )
            *str = HIDDEN_TILDE;

    return;
}

char *show_tilde( char *str )
{
    static char buf[MAX_STRING_LENGTH];
    char *bufptr;

    bufptr = buf;
    for ( ; *str != '\0'; str++, bufptr++ )
    {
        if ( *str == HIDDEN_TILDE )
            *bufptr = '~';
        else
            *bufptr = *str;
    }
    *bufptr = '\0';

    return buf;
}



/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
    if ( !astr )
    {
	if ( bstr )
	    bug( "str_cmp: astr: (null)  bstr: %s\n", bstr );
	else
	    bug( "str_cmp: null astr." );
	return TRUE;
    }

    if ( !bstr )
    {
	bug( "str_cmp: astr: %s  bstr: (null)\n", astr );
        return TRUE;
    }

    for ( ; *astr || *bstr; astr++, bstr++ )
    {
        if ( LOWER(*astr) != LOWER(*bstr) )
            return TRUE;
    }

    return FALSE;
}



/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
    if ( !astr )
    {
        bug( "Strn_cmp: null astr." );
        return TRUE;
    }

    if ( !bstr )
    {
        bug( "Strn_cmp: null bstr." );
        return TRUE;
    }

    for ( ; *astr; astr++, bstr++ )
    {
        if ( LOWER(*astr) != LOWER(*bstr) )
            return TRUE;
    }

    return FALSE;
}



/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;
    int ichar;
    char c0;

    if ( !astr )
    {
        bug( "Str_infix: null astr." );
        return TRUE;
    }

    if ( !bstr )
    {
        bug( "Str_infix: null bstr." );
        return TRUE;
    }

    if ( ( c0 = LOWER(astr[0]) ) == '\0' )
        return FALSE;

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);

    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
        if ( c0 == LOWER(bstr[ichar]) && !str_prefix( astr, bstr + ichar ) )
            return FALSE;

    return TRUE;
}



/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;

    if ( !astr )
    {
        bug( "Str_suffix: null astr." );
        return TRUE;
    }

    if ( !bstr )
    {
        bug( "Str_suffix: null bstr." );
        return TRUE;
    }

    sstr1 = strlen(astr);
    sstr2 = strlen(bstr);
    if ( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
        return FALSE;
    else
        return TRUE;
}



/*
 * Returns an initial-capped string.
 */
char *capitalize( const char *str )
{
    static char strcap[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
        strcap[i] = LOWER(str[i]);
    strcap[i] = '\0';
    strcap[0] = UPPER(strcap[0]);
    return strcap;
}


/*
 * Returns a lowercase string.
 */
char *strlower( const char *str )
{
    static char strlow[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
        strlow[i] = LOWER(str[i]);
    strlow[i] = '\0';
    return strlow;
}

/*
 * Returns an uppercase string.
 */
char *strupper( const char *str )
{
    static char strup[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
        strup[i] = UPPER(str[i]);
    strup[i] = '\0';
    return strup;
}

/*
 * Returns TRUE or FALSE if a letter is a vowel			-Thoric
 */
bool isavowel( char letter )
{
    char c;

    c = LOWER( letter );
    if ( c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' )
        return TRUE;
    else
        return FALSE;
}

/*
 * Shove either "a " or "an " onto the beginning of a string	-Thoric
 */
char *aoran( const char *str )
{
    static char temp[MAX_STRING_LENGTH];

    if ( !str )
    {
        bug( "Aoran(): NULL str" );
        return "";
    }

    if ( isavowel(str[0])
         || ( strlen(str) > 1 && LOWER(str[0]) == 'y' && !isavowel(str[1])) )
        strcpy( temp, "an " );
    else
        strcpy( temp, "a " );
    strcat( temp, str );
    return temp;
}


/*
 * Append a string to a file.
 */
void append_file( CHAR_DATA *ch, char *file, char *str )
{
    FILE *fp;
    static int bugnum=0;

    if ( IS_NPC(ch) || str[0] == '\0' )
        return;

    if ( ( fp = fopen( file, "a" ) ) == NULL )
    {
        perror( file );
        send_to_char( "Could not open the file!\n\r", ch );
    }
    else
    {
        bugnum++;
        fprintf( fp, "%10.10ld%3.3d [%5d] %s: %s\n",
                 (long)current_time, bugnum, ch->in_room ? ch->in_room->vnum : 0, ch->name, str );
        FCLOSE( fp );
    }

    return;
}

/*
 * Append a string to a file.
 */
void append_to_file( char *file, char *str )
{
    FILE *fp;

    if ( ( fp = fopen( file, "a" ) ) == NULL )
        perror( file );
    else
    {
        fprintf( fp, "%s\n", str );
        FCLOSE( fp );
    }

    return;
}

void mud_backtrace(void)
{
#ifdef HAVE_BACKTRACE_SYMBOLS
    void *array[20];
    size_t size;
    char **strings;
    size_t i; 

    size = backtrace( array, 20 );
    strings = backtrace_symbols( array, size );

    for( i = 0; i < size; i++ )
        log_string_plus( strings[i], LOG_BUG, LEVEL_IMMORTAL, SEV_DEBUG );

    free( strings );
#endif
}

/*
 * Reports a bug.
 */
void low_bug( const char *str, ... )
{
    char log_buf[MAX_STRING_LENGTH];
    FILE *fp;
    struct stat fst;
    char *strtime;

    strtime                    = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';

    if ( fpArea != NULL )
    {
        int iLine;
        int iChar;

        if ( fpArea == stdin )
        {
            iLine = 0;
        }
        else
        {
	    int c;
            iChar = ftell( fpArea );
            fseek( fpArea, 0, 0 );
            for ( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
            {
                while ( (c=fgetc( fpArea )) != '\n' && c!=EOF)
                    ;
            }
            fseek( fpArea, iChar, 0 );
        }

        sprintf( log_buf, "[*****] FILE: %s LINE: %d", strArea, iLine );
        log_string_plus( log_buf, LOG_BUG, LEVEL_IMMORTAL, SEV_NOTICE );

        if ( stat( SHUTDOWN_FILE, &fst ) != -1 )	/* file exists */
        {
            if ( ( fp = fopen( SHUTDOWN_FILE, "a" ) ) != NULL )
            {
                fprintf( fp, "[*****] %s\n", log_buf );
                FCLOSE( fp );
            }
        }
    }

    strcpy( log_buf, "[*****] BUG: " );
    {
        va_list param;

        va_start(param, str);
        vsprintf( log_buf + strlen(log_buf), str, param );
        va_end(param);
    }
    log_string_plus( log_buf, LOG_BUG, LEVEL_IMMORTAL, SEV_NOTICE );

    if ( ( fp = fopen( BUG_FILE, "a" ) ) != NULL )
    {
        fprintf( fp, "%s :: %s\n", strtime, log_buf );
        FCLOSE( fp );
    }

    mud_backtrace();

    return;
}

/*
 * Add a string to the boot-up log				-Thoric
 */
void boot_log( const char *str, ... )
{
    char log_buf[MAX_STRING_LENGTH];
    FILE *fp;
    va_list param;

    strcpy( log_buf, "[*****] BOOT: " );
    va_start(param, str);
    vsprintf( log_buf+strlen(log_buf), str, param );
    va_end(param);
    log_string_plus( log_buf, LOG_NORMAL, LEVEL_LOG_CSET, SEV_NOTICE );

    if ( ( fp = fopen( BOOTLOG_FILE, "a" ) ) != NULL )
    {
        fprintf( fp, "%s\n", log_buf );
        FCLOSE( fp );
    }

    return;
}

/*
 * Dump a text file to a player, a line at a time		-Thoric
 */
void show_file( CHAR_DATA *ch, char *filename )
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    int c;
    int num = 0;

    if ( (fp = fopen( filename, "r" )) != NULL )
    {
        while ( !feof(fp) )
        {
            while ((buf[num]=fgetc(fp)) != EOF
                   &&      buf[num] != '\n'
                   &&      buf[num] != '\r'
                   &&      num < (MAX_STRING_LENGTH-2))
                num++;
            c = fgetc(fp);
            if ( (c != '\n' && c != '\r') || c == buf[num] )
                ungetc(c, fp);
            buf[num++] = '\n';
            buf[num++] = '\r';
            buf[num  ] = '\0';
            send_to_pager( buf, ch );
            num = 0;
        }
        FCLOSE(fp);
    }
}

/*
 * Show the boot log file					-Thoric
 */
void do_dmesg( CHAR_DATA *ch, char *argument )
{
    set_pager_color( AT_LOG, ch );
    show_file( ch, BOOTLOG_FILE );
}


/*
 * Writes a string to the log, extended version			-Thoric
 */
void log_string_plus( const char *str, sh_int log_type, sh_int level, sh_int severity )
{
    char *strtime;
    int offset;

    strtime                    = ctime( &current_time );
    strtime[strlen(strtime)-1] = '\0';
#ifndef MUD_DEBUG
    if (log_type!=LOG_IMCDEBUG && severity<SEV_SPAM)
#endif
        fprintf( stderr, "%s :: %s\n", strtime, str );
    if ( strncmp( str, "Log ", 4 ) == 0 )
        offset = 4;
    else
        offset = 0;

    if (log_type<0 || log_type>=LOG_LAST)
    {
        fprintf( stderr, "log_string_plus: log_type out of range: %d\n", log_type);
        log_type = LOG_BUG;
    }

    to_channel(str + offset,
               log_type,
               level==LEVEL_LOG_CSET?sysdata.logdefs[log_type].level:level,
               severity);
}

/*
 * wizlist builder!						-Thoric
 */

void towizfile( const char *line )
{
    int filler, xx;
    char outline[MAX_STRING_LENGTH];
    FILE *wfp;

    outline[0] = '\0';

    if ( line && line[0] != '\0' )
    {
        filler = ( 78-strlen( line ) );
        if ( filler < 1 )
            filler = 1;
        filler /= 2;
        for ( xx = 0; xx < filler; xx++ )
            strcat( outline, " " );
        strcat( outline, line );
    }
    strcat( outline, "\n\r" );
    wfp = fopen( WIZLIST_FILE, "a" );
    if ( wfp )
    {
        fputs( outline, wfp );
        FCLOSE( wfp );
    }
}

void add_to_wizlist( char *name, int level )
{
    WIZENT *wiz, *tmp;

#ifdef DEBUG
    log_printf_plus(LOG_DEBUG, LEVEL_LOG_CSET, SEV_INFO,  "Adding %s (%d) to wizlist...", name, level );
#endif

    CREATE( wiz, WIZENT, 1 );
    wiz->name	= str_dup( name );
    wiz->level	= level;

    if ( !first_wiz )
    {
        wiz->last	= NULL;
        wiz->next	= NULL;
        first_wiz	= wiz;
        last_wiz	= wiz;
        return;
    }

    /* insert sort, of sorts */
    for ( tmp = first_wiz; tmp; tmp = tmp->next )
        if ( level > tmp->level )
        {
            if ( !tmp->last )
                first_wiz	= wiz;
            else
                tmp->last->next = wiz;
            wiz->last = tmp->last;
            wiz->next = tmp;
            tmp->last = wiz;
            return;
        }

    wiz->last		= last_wiz;
    wiz->next		= NULL;
    last_wiz->next	= wiz;
    last_wiz		= wiz;
    return;
}

/*
 * Wizlist builder						-Thoric
 */
void make_wizlist( )
{
    DIR *dp;
    struct dirent *dentry;
    FILE *gfp;
    const char *word;
    int ilevel, iflags;
    WIZENT *wiz, *wiznext;
    char buf[MAX_STRING_LENGTH];

    first_wiz = NULL;
    last_wiz  = NULL;

    dp = opendir( GOD_DIR );

    ilevel = 0;
    dentry = readdir( dp );
    while ( dentry )
    {
        if ( dentry->d_name[0] != '.' )
        {
            sprintf( buf, "%s%s", GOD_DIR, dentry->d_name );
            gfp = fopen( buf, "r" );
            if ( gfp )
            {
                word = feof( gfp ) ? "End" : fread_word( gfp );
                ilevel = fread_number( gfp );
                fread_to_eol( gfp );
                word = feof( gfp ) ? "End" : fread_word( gfp );
                if ( !str_cmp( word, "Pcflags" ) )
                    iflags = fread_number( gfp );
                else
                    iflags = 0;
                FCLOSE( gfp );
                if ( IS_SET( iflags, PCFLAG_RETIRED ) )
                    ilevel = MAX_LEVEL - 15;
                if ( IS_SET( iflags, PCFLAG_GUEST ) )
                    ilevel = MAX_LEVEL - 16;
                add_to_wizlist( dentry->d_name, ilevel );
            }
        }
        dentry = readdir( dp );
    }
    closedir( dp );

    buf[0] = '\0';
    unlink( WIZLIST_FILE );
    sprintf(buf," Immortals of %s",MUD_NAME);
    towizfile( buf );
    buf[0] = '\0';
    ilevel = 65535;
    for ( wiz = first_wiz; wiz; wiz = wiz->next )
    {
        if ( wiz->level < ilevel )
        {
            if ( buf[0] )
            {
                towizfile( buf );
                buf[0] = '\0';
            }
            towizfile( "" );
            ilevel = wiz->level;
            switch(ilevel)
            {
            case MAX_LEVEL -  0: towizfile( " Supreme Entity" );	break;
            case MAX_LEVEL -  1: towizfile( " Infinite" );		break;
            case MAX_LEVEL -  2: towizfile( " Eternal" );		break;
            case MAX_LEVEL -  3: towizfile( " Ancient" );		break;
            case MAX_LEVEL -  4: towizfile( " Exalted Gods" );	break;
            case MAX_LEVEL -  5: towizfile( " Ascendant Gods" );	break;
            case MAX_LEVEL -  6: towizfile( " Greater Gods" );	break;
            case MAX_LEVEL -  7: towizfile( " Gods" );		break;
            case MAX_LEVEL -  8: towizfile( " Lesser Gods" );	break;
            case MAX_LEVEL -  9: towizfile( " Immortals" );		break;
            case MAX_LEVEL - 10: towizfile( " Demi Gods" );		break;
            case MAX_LEVEL - 11: towizfile( " Saviors" );		break;
            case MAX_LEVEL - 12: towizfile( " Creators" );		break;
            case MAX_LEVEL - 13: towizfile( " Acolytes" );		break;
            case MAX_LEVEL - 14: towizfile( " Guests" );		break;
            default:	     towizfile( " Servants" );		break;
            }
        }
        if ( strlen( buf ) + strlen( wiz->name ) > 76 )
        {
            towizfile( buf );
            buf[0] = '\0';
        }
        strcat( buf, " " );
        strcat( buf, wiz->name );
        if ( strlen( buf ) > 70 )
        {
            towizfile( buf );
            buf[0] = '\0';
        }
    }

    if ( buf[0] )
        towizfile( buf );

    for ( wiz = first_wiz; wiz; wiz = wiznext )
    {
        wiznext = wiz->next;
        DISPOSE(wiz->name);
        DISPOSE(wiz);
    }
    first_wiz = NULL;
    last_wiz = NULL;
}


void do_makewizlist( CHAR_DATA *ch, char *argument )
{
    make_wizlist();
}


/* mud prog functions */

/* This routine reads in scripts of MUDprograms from a file */

sh_int mprog_name_to_type ( char *name )
{
    if ( !str_cmp( name, "in_file_prog"   ) )	return IN_FILE_PROG;
    if ( !str_cmp( name, "act_prog"       ) )   return ACT_PROG;
    if ( !str_cmp( name, "speech_prog"    ) )	return SPEECH_PROG;
    if ( !str_cmp( name, "rand_prog"      ) ) 	return RAND_PROG;
    if ( !str_cmp( name, "fight_prog"     ) )	return FIGHT_PROG;
    if ( !str_cmp( name, "hitprcnt_prog"  ) )	return HITPRCNT_PROG;
    if ( !str_cmp( name, "death_prog"     ) )	return DEATH_PROG;
    if ( !str_cmp( name, "entry_prog"     ) )	return ENTRY_PROG;
    if ( !str_cmp( name, "greet_prog"     ) )	return GREET_PROG;
    if ( !str_cmp( name, "all_greet_prog" ) )	return ALL_GREET_PROG;
    if ( !str_cmp( name, "give_prog"      ) ) 	return GIVE_PROG;
    if ( !str_cmp( name, "bribe_prog"     ) )	return BRIBE_PROG;
    if ( !str_cmp( name, "time_prog"      ) )	return TIME_PROG;
    if ( !str_cmp( name, "hour_prog"      ) )	return HOUR_PROG;
    if ( !str_cmp( name, "wear_prog"      ) )	return WEAR_PROG;
    if ( !str_cmp( name, "remove_prog"    ) )	return REMOVE_PROG;
    if ( !str_cmp( name, "sac_prog"       ) )	return SAC_PROG;
    if ( !str_cmp( name, "look_prog"      ) )	return LOOK_PROG;
    if ( !str_cmp( name, "exa_prog"       ) )	return EXA_PROG;
    if ( !str_cmp( name, "zap_prog"       ) )	return ZAP_PROG;
    if ( !str_cmp( name, "get_prog"       ) ) 	return GET_PROG;
    if ( !str_cmp( name, "drop_prog"      ) )	return DROP_PROG;
    if ( !str_cmp( name, "damage_prog"    ) )	return DAMAGE_PROG;
    if ( !str_cmp( name, "repair_prog"    ) )	return REPAIR_PROG;
    if ( !str_cmp( name, "greet_prog"     ) )	return GREET_PROG;
    if ( !str_cmp( name, "birth_prog"     ) )	return BIRTH_PROG;
    if ( !str_cmp( name, "speechiw_prog"  ) )	return SPEECHIW_PROG;
    if ( !str_cmp( name, "pull_prog"      ) )   return PULL_PROG;
    if ( !str_cmp( name, "push_prog"      ) )   return PUSH_PROG;
    if ( !str_cmp( name, "sleep_prog"     ) )   return SLEEP_PROG;
    if ( !str_cmp( name, "rest_prog"      ) )	return REST_PROG;
    if ( !str_cmp( name, "rfight_prog"    ) )	return FIGHT_PROG;
    if ( !str_cmp( name, "enter_prog"     ) )	return ENTRY_PROG;
    if ( !str_cmp( name, "leave_prog"     ) )	return LEAVE_PROG;
    if ( !str_cmp( name, "rdeath_prog"    ) )	return DEATH_PROG;
    if ( !str_cmp( name, "script_prog"    ) )	return SCRIPT_PROG;
    if ( !str_cmp( name, "use_prog"	  ) )	return USE_PROG;
    if ( !str_cmp( name, "quest_prog"	  ) )	return QUEST_PROG;
    if ( !str_cmp( name, "command_prog"	  ) )	return COMMAND_PROG;
    if ( !str_cmp( name, "area_reset_prog") )	return AREA_RESET_PROG;
    if ( !str_cmp( name, "area_init_prog") )	return AREA_INIT_PROG;
    return( ERROR_PROG );
}

static MPROG_DATA *mprog_file_read( char *f, MPROG_DATA *mprg,
                                    MOB_INDEX_DATA *pMobIndex )
{

    char        MUDProgfile[ MAX_INPUT_LENGTH ];
    FILE       *progfile;
    char        letter;
    MPROG_DATA *mprg_next, *mprg2;
    bool        done = FALSE;

    sprintf( MUDProgfile, "%s%s", PROG_DIR, f );

    progfile = fopen( MUDProgfile, "r" );
    if ( !progfile )
    {
        bug( "Mob: %d couldn't open mudprog file", pMobIndex->ivnum );
        exit( 1 );
    }

    mprg2 = mprg;
    switch ( letter = fread_letter( progfile ) )
    {
    case '>':
        break;
    case '|':
        bug( "empty mudprog file." );
        exit( 1 );
        break;
    default:
        bug( "in mudprog file syntax error." );
        exit( 1 );
        break;
    }

    while ( !done )
    {
        mprg2->progtype = mprog_name_to_type( fread_word( progfile ) );
        switch ( mprg2->progtype )
        {
        case ERROR_PROG:
            bug( "mudprog file type error" );
            exit( 1 );
            break;
        case IN_FILE_PROG:
            bug( "mprog file contains a call to file." );
            exit( 1 );
            break;
        default:
            xSET_BIT(pMobIndex->progtypes, mprg2->progtype);
            mprg2->arglist       = fread_string( progfile );
            mprg2->comlist       = fread_string( progfile );
            switch ( letter = fread_letter( progfile ) )
            {
            case '>':
                CREATE( mprg_next, MPROG_DATA, 1 );
                mprg_next->next = mprg2;
                mprg2 = mprg_next;
                break;
            case '|':
                done = TRUE;
                break;
            default:
                bug( "in mudprog file syntax error." );
                exit( 1 );
                break;
            }
            break;
        }
    }
    FCLOSE( progfile );
    return mprg2;
}

/* Load a MUDprogram section from the area file.
 */
void load_mudprogs( AREA_DATA *tarea, gzFile gzfp )
{
    char wordbuf[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *iMob;
    MPROG_DATA     *original;
    MPROG_DATA     *working;
    char            letter;
    int             value;

    for ( ; ; )
        switch ( letter = gz_fread_letter( gzfp ) )
        {
        default:
            bug( "Load_mudprogs: bad command '%c'.",letter);
            exit(1);
            break;
        case 'S':
        case 's':
            gz_fread_to_eol( gzfp );
            return;
        case '*':
            gz_fread_to_eol( gzfp );
            break;
        case 'M':
        case 'm':
            value = gz_fread_number( gzfp );
            if ( ( iMob = get_mob_index( value ) ) == NULL )
            {
                bug( "Load_mudprogs: vnum %d doesnt exist", value );
                exit( 1 );
            }

            /* Go to the end of the prog command list if other commands
             exist */

            if ( (original = iMob->mudprogs) != NULL )
                for ( ; original->next; original = original->next );

            CREATE( working, MPROG_DATA, 1 );
            if ( original )
                original->next = working;
            else
                iMob->mudprogs = working;
            working = mprog_file_read( gz_fread_word( gzfp, wordbuf ), working, iMob );
            working->next = NULL;
            gz_fread_to_eol( gzfp );
            break;
        }

    return;

}

/* This procedure is responsible for reading any in_file MUDprograms.
 */
#ifndef USE_DB
static void mprog_read_programs( gzFile gzfp, MOB_INDEX_DATA *pMobIndex)
{
    char wordbuf[MAX_INPUT_LENGTH];
    MPROG_DATA *mprg;
    char        letter;
    bool        done = FALSE;

    CREATE( mprg, MPROG_DATA, 1 );
    pMobIndex->mudprogs = mprg;

    while ( !done )
    {
        mprg->progtype = mprog_name_to_type( gz_fread_word( gzfp, wordbuf ) );
        switch ( mprg->progtype )
        {
        case ERROR_PROG:
            bug( "Load_mobiles: vnum %d MUDPROG type.", pMobIndex->ivnum );
            exit( 1 );
            break;
        case IN_FILE_PROG:
            mprg = mprog_file_read( gz_fread_string( gzfp ), mprg,pMobIndex );
            gz_fread_to_eol( gzfp );
            switch ( letter = gz_fread_letter( gzfp ) )
            {
            case '>':
                CREATE( mprg->next, MPROG_DATA, 1 );
                mprg = mprg->next;
                break;
            case '|':
                mprg->next = NULL;
                gz_fread_to_eol( gzfp );
                done = TRUE;
                break;
            default:
                bug( "Load_mobiles: vnum %d bad MUDPROG.", pMobIndex->ivnum );
                exit( 1 );
                break;
            }
            break;
        default:
            xSET_BIT(pMobIndex->progtypes, mprg->progtype);
            mprg->arglist        = gz_fread_string( gzfp );
            gz_fread_to_eol( gzfp );
            mprg->comlist        = gz_fread_string( gzfp );
            gz_fread_to_eol( gzfp );
            switch ( letter = gz_fread_letter( gzfp ) )
            {
            case '>':
                CREATE( mprg->next, MPROG_DATA, 1 );
                mprg = mprg->next;
                break;
            case '|':
                mprg->next = NULL;
                gz_fread_to_eol( gzfp );
                done = TRUE;
                break;
            default:
                bug( "Load_mobiles: vnum %d bad MUDPROG.", pMobIndex->ivnum );
                exit( 1 );
                break;
            }
            break;
        }
    }

    return;

}


/*************************************************************/
/* obj prog functions */
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

/* This routine reads in scripts of OBJprograms from a file */

static MPROG_DATA *oprog_file_read( char *f, MPROG_DATA *mprg,
                                    OBJ_INDEX_DATA *pObjIndex )
{

    char        MUDProgfile[ MAX_INPUT_LENGTH ];
    FILE       *progfile;
    char        letter;
    MPROG_DATA *mprg_next, *mprg2;
    bool        done = FALSE;

    sprintf( MUDProgfile, "%s%s", PROG_DIR, f );

    progfile = fopen( MUDProgfile, "r" );
    if ( !progfile )
    {
        bug( "Obj: %d couldnt open mudprog file", pObjIndex->ivnum );
        exit( 1 );
    }

    mprg2 = mprg;
    switch ( letter = fread_letter( progfile ) )
    {
    case '>':
        break;
    case '|':
        bug( "empty objprog file." );
        exit( 1 );
        break;
    default:
        bug( "in objprog file syntax error." );
        exit( 1 );
        break;
    }

    while ( !done )
    {
        mprg2->progtype = mprog_name_to_type( fread_word( progfile ) );
        switch ( mprg2->progtype )
        {
        case ERROR_PROG:
            bug( "objprog file type error" );
            exit( 1 );
            break;
        case IN_FILE_PROG:
            bug( "objprog file contains a call to file." );
            exit( 1 );
            break;
        default:
            xSET_BIT(pObjIndex->progtypes, mprg2->progtype);
            mprg2->arglist       = fread_string( progfile );
            mprg2->comlist       = fread_string( progfile );
            switch ( letter = fread_letter( progfile ) )
            {
            case '>':
                CREATE( mprg_next, MPROG_DATA, 1 );
                mprg_next->next = mprg2;
                mprg2 = mprg_next;
                break;
            case '|':
                done = TRUE;
                break;
            default:
                bug( "in objprog file syntax error." );
                exit( 1 );
                break;
            }
            break;
        }
    }
    FCLOSE( progfile );
    return mprg2;
}

/* Load a MUDprogram section from the area file.
 */
static void load_objprogs( AREA_DATA *tarea, gzFile gzfp )
{
    char wordbuf[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *iObj;
    MPROG_DATA     *original;
    MPROG_DATA     *working;
    char            letter;
    int             value;

    for ( ; ; )
        switch ( letter = gz_fread_letter( gzfp ) )
        {
        default:
            bug( "Load_objprogs: bad command '%c'.",letter);
            exit(1);
            break;
        case 'S':
        case 's':
            gz_fread_to_eol( gzfp );
            return;
        case '*':
            gz_fread_to_eol( gzfp );
            break;
        case 'M':
        case 'm':
            value = gz_fread_number( gzfp );
            if ( ( iObj = get_obj_index( value ) ) == NULL )
            {
                bug( "Load_objprogs: vnum %d doesnt exist", value );
                exit( 1 );
            }

            /* Go to the end of the prog command list if other commands
             exist */

            if ( (original = iObj->mudprogs) != NULL )
                for ( ; original->next; original = original->next );

            CREATE( working, MPROG_DATA, 1 );
            if ( original )
                original->next = working;
            else
                iObj->mudprogs = working;
            working = oprog_file_read( gz_fread_word( gzfp, wordbuf ), working, iObj );
            working->next = NULL;
            gz_fread_to_eol( gzfp );
            break;
        }

    return;

}

/* This procedure is responsible for reading any in_file OBJprograms.
 */

static void oprog_read_programs( gzFile gzfp, OBJ_INDEX_DATA *pObjIndex)
{
    char wordbuf[MAX_INPUT_LENGTH];
    MPROG_DATA *mprg;
    char        letter;
    bool        done = FALSE;

    CREATE( mprg, MPROG_DATA, 1 );
    pObjIndex->mudprogs = mprg;

    while ( !done )
    {
        mprg->progtype = mprog_name_to_type( gz_fread_word( gzfp, wordbuf ) );
        switch ( mprg->progtype )
        {
        case ERROR_PROG:
            bug( "Load_objects: vnum %d OBJPROG type.", pObjIndex->ivnum );
            exit( 1 );
            break;
        case IN_FILE_PROG:
            mprg = oprog_file_read( gz_fread_string( gzfp ), mprg,pObjIndex );
            gz_fread_to_eol( gzfp );
            switch ( letter = gz_fread_letter( gzfp ) )
            {
            case '>':
                CREATE( mprg->next, MPROG_DATA, 1 );
                mprg = mprg->next;
                break;
            case '|':
                mprg->next = NULL;
                gz_fread_to_eol( gzfp );
                done = TRUE;
                break;
            default:
                bug( "Load_objects: vnum %d bad OBJPROG.", pObjIndex->ivnum );
                exit( 1 );
                break;
            }
            break;
        default:
            xSET_BIT(pObjIndex->progtypes, mprg->progtype);
            mprg->arglist        = gz_fread_string( gzfp );
            gz_fread_to_eol( gzfp );
            mprg->comlist        = gz_fread_string( gzfp );
            gz_fread_to_eol( gzfp );
            switch ( letter = gz_fread_letter( gzfp ) )
            {
            case '>':
                CREATE( mprg->next, MPROG_DATA, 1 );
                mprg = mprg->next;
                break;
            case '|':
                mprg->next = NULL;
                gz_fread_to_eol( gzfp );
                done = TRUE;
                break;
            default:
                bug( "Load_objects: vnum %d bad OBJPROG.", pObjIndex->ivnum );
                exit( 1 );
                break;
            }
            break;
        }
    }

    return;

}


/*************************************************************/
/* room prog functions */
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

/* This routine reads in scripts of OBJprograms from a file */
static MPROG_DATA *rprog_file_read( char *f, MPROG_DATA *mprg,
                                    ROOM_INDEX_DATA *RoomIndex )
{

    char        MUDProgfile[ MAX_INPUT_LENGTH ];
    FILE       *progfile;
    char        letter;
    MPROG_DATA *mprg_next, *mprg2;
    bool        done = FALSE;

    sprintf( MUDProgfile, "%s%s", PROG_DIR, f );

    progfile = fopen( MUDProgfile, "r" );
    if ( !progfile )
    {
        bug( "Room: %d couldnt open roomprog file", RoomIndex->vnum );
        exit( 1 );
    }

    mprg2 = mprg;
    switch ( letter = fread_letter( progfile ) )
    {
    case '>':
        break;
    case '|':
        bug( "empty roomprog file." );
        exit( 1 );
        break;
    default:
        bug( "in roomprog file syntax error." );
        exit( 1 );
        break;
    }

    while ( !done )
    {
        mprg2->progtype = mprog_name_to_type( fread_word( progfile ) );
        switch ( mprg2->progtype )
        {
        case ERROR_PROG:
            bug( "roomprog file type error" );
            exit( 1 );
            break;
        case IN_FILE_PROG:
            bug( "roomprog file contains a call to file." );
            exit( 1 );
            break;
        default:
            xSET_BIT(RoomIndex->progtypes, mprg2->progtype);
            mprg2->arglist       = fread_string( progfile );
            mprg2->comlist       = fread_string( progfile );
            switch ( letter = fread_letter( progfile ) )
            {
            case '>':
                CREATE( mprg_next, MPROG_DATA, 1 );
                mprg_next->next = mprg2;
                mprg2 = mprg_next;
                break;
            case '|':
                done = TRUE;
                break;
            default:
                bug( "in roomprog file syntax error." );
                exit( 1 );
                break;
            }
            break;
        }
    }
    FCLOSE( progfile );
    return mprg2;
}

/* This procedure is responsible for reading any in_file ROOMprograms.
 */

static void rprog_read_programs( gzFile gzfp, ROOM_INDEX_DATA *pRoomIndex)
{
    char wordbuf[MAX_INPUT_LENGTH];
    MPROG_DATA *mprg;
    char        letter;
    bool        done = FALSE;

    CREATE( mprg, MPROG_DATA, 1 );
    pRoomIndex->mudprogs = mprg;

    while ( !done )
    {
        mprg->progtype = mprog_name_to_type( gz_fread_word( gzfp, wordbuf ) );
        switch ( mprg->progtype )
        {
        case ERROR_PROG:
            bug( "Load_rooms: vnum %d ROOMPROG type.", pRoomIndex->vnum );
            exit( 1 );
            break;
        case IN_FILE_PROG:
            mprg = rprog_file_read( gz_fread_string( gzfp ), mprg,pRoomIndex );
            gz_fread_to_eol( gzfp );
            switch ( letter = gz_fread_letter( gzfp ) )
            {
            case '>':
                CREATE( mprg->next, MPROG_DATA, 1 );
                mprg = mprg->next;
                break;
            case '|':
                mprg->next = NULL;
                gz_fread_to_eol( gzfp );
                done = TRUE;
                break;
            default:
                bug( "Load_rooms: vnum %d bad ROOMPROG.", pRoomIndex->vnum );
                exit( 1 );
                break;
            }
            break;
        default:
            xSET_BIT(pRoomIndex->progtypes, mprg->progtype);
            mprg->arglist        = gz_fread_string( gzfp );
            gz_fread_to_eol( gzfp );
            mprg->comlist        = gz_fread_string( gzfp );
            gz_fread_to_eol( gzfp );
            switch ( letter = gz_fread_letter( gzfp ) )
            {
            case '>':
                CREATE( mprg->next, MPROG_DATA, 1 );
                mprg = mprg->next;
                break;
            case '|':
                mprg->next = NULL;
                gz_fread_to_eol( gzfp );
                done = TRUE;
                break;
            default:
                bug( "Load_rooms: vnum %d bad ROOMPROG.", pRoomIndex->vnum );
                exit( 1 );
                break;
            }
            break;
        }
    }

    return;

}
#endif

/*************************************************************/
/* Function to delete a room index.  Called from do_rdelete in build.c
 Narn, May/96
 */
bool delete_room( ROOM_INDEX_DATA *room )
{
    int hash;
    ROOM_INDEX_DATA *prev, *limbo = get_room_index(ROOM_VNUM_LIMBO);
    OBJ_DATA *o;
    CHAR_DATA *ch;
    EXTRA_DESCR_DATA *ed;
    EXIT_DATA *ex;
    MPROG_ACT_LIST *mpact;
    MPROG_DATA *mp;

    while ((ch = room->first_person) != NULL)
    {
        if (!IS_NPC(ch) && limbo)
        {
            char_from_room(ch);
            char_to_room(ch, limbo);
        }
        else
            extract_char(ch, TRUE);
    }
    while ((o = room->first_content) != NULL)
        extract_obj(o);
    while ((ed = room->first_extradesc) != NULL)
    {
        room->first_extradesc = ed->next;
        STRFREE(ed->keyword);
        STRFREE(ed->description);
        DISPOSE(ed);
        --top_ed;
    }
    while ((ex = room->first_exit) != NULL)
        extract_exit(room, ex);
    while ((mpact = room->mpact) != NULL)
    {
        room->mpact = mpact->next;
        DISPOSE(mpact->buf);
        DISPOSE(mpact);
    }
    while ((mp = room->mudprogs) != NULL)
    {
        room->mudprogs = mp->next;
        STRFREE(mp->arglist);
        STRFREE(mp->comlist);
        DISPOSE(mp);
    }
    if (room->map)
    {
        MAP_INDEX_DATA *mapi;

        if ((mapi = get_map_index(room->map->vnum)) != NULL)
            if (room->map->x > 0 && room->map->x < 80 &&
                room->map->y > 0 && room->map->y < 48)
                mapi->map_of_vnums[room->map->y][room->map->x] = -1;
        DISPOSE(room->map);
    }
    STRFREE(room->name);
    STRFREE(room->description);

    hash = room->vnum%MAX_KEY_HASH;
    if (room == room_index_hash[hash])
        room_index_hash[hash] = room->next;
    else
    {
        for (prev = room_index_hash[hash]; prev; prev = prev->next)
            if (prev->next == room)
                break;
        if (prev)
            prev->next = room->next;
        else
            bug("delete_room: room %d not in hash bucket %d.", room->vnum, hash);
    }
    DISPOSE(room);
    --top_room;
    return TRUE;
}

/* See comment on delete_room. */
bool delete_obj( OBJ_INDEX_DATA *obj )
{
    int hash;
    OBJ_INDEX_DATA *prev;
    OBJ_DATA *o, *o_next;
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf, *paf_next;
    MPROG_DATA *mp;

    /* Remove references to object index */
    for (o = first_object; o; o = o_next)
    {
        o_next = o->next;
        if (o->pIndexData == obj)
            extract_obj(o);
    }
    while ((ed = obj->first_extradesc) != NULL)
    {
        obj->first_extradesc = ed->next;
        STRFREE(ed->keyword);
        STRFREE(ed->description);
        DISPOSE(ed);
        --top_ed;
    }
    for ( paf = obj->first_affect; paf; paf = paf_next )
    {
        paf_next    = paf->next;
        DISPOSE( paf );
        top_affect--;
    }
    while ((mp = obj->mudprogs) != NULL)
    {
        obj->mudprogs = mp->next;
        STRFREE(mp->arglist);
        STRFREE(mp->comlist);
        DISPOSE(mp);
    }
    STRFREE(obj->name);
    STRFREE(obj->short_descr);
    STRFREE(obj->description);
    STRFREE(obj->action_desc);

    hash = obj->ivnum%MAX_KEY_HASH;
    if (obj == obj_index_hash[hash])
        obj_index_hash[hash] = obj->next;
    else
    {
        for (prev = obj_index_hash[hash]; prev; prev = prev->next)
            if (prev->next == obj)
                break;
        if (prev)
            prev->next = obj->next;
        else
            bug("delete_obj: object %d not in hash bucket %d.", obj->ivnum, hash);
    }
    DISPOSE(obj);
    --top_obj_index;
    return TRUE;
}

/* See comment on delete_room. */
bool delete_mob( MOB_INDEX_DATA *mob )
{
    int hash;
    MOB_INDEX_DATA *prev;
    CHAR_DATA *ch, *ch_next;
    MPROG_DATA *mp;

    for (ch = first_char; ch; ch = ch_next)
    {
        ch_next = ch->next;
        if (ch->pIndexData == mob)
            extract_char(ch, TRUE);
    }
    while ((mp = mob->mudprogs) != NULL)
    {
        mob->mudprogs = mp->next;
        STRFREE(mp->arglist);
        STRFREE(mp->comlist);
        DISPOSE(mp);
    }

    if (mob->pShop)
    {
        UNLINK(mob->pShop, first_shop, last_shop, next, prev);
        DISPOSE(mob->pShop);
        --top_shop;
    }

    if (mob->rShop)
    {
        UNLINK(mob->rShop, first_repair, last_repair, next, prev);
        DISPOSE(mob->rShop);
        --top_repair;
    }

    STRFREE(mob->player_name);
    STRFREE(mob->short_descr);
    STRFREE(mob->long_descr);
    STRFREE(mob->description);

    hash = mob->ivnum%MAX_KEY_HASH;
    if (mob == mob_index_hash[hash])
        mob_index_hash[hash] = mob->next;
    else
    {
        for (prev = mob_index_hash[hash]; prev; prev = prev->next)
            if (prev->next == mob)
                break;
        if (prev)
            prev->next = mob->next;
        else
            bug("delete_mob: mobile %d not in hash bucket %d.", mob->ivnum, hash);
    }
    DISPOSE(mob);
    --top_mob_index;
    return TRUE;
}

/*
 * Creat a new room (for online building)			-Thoric
 */
ROOM_INDEX_DATA *make_room( int vnum )
{
    ROOM_INDEX_DATA *pRoomIndex;
    int	iHash;

    CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
    pRoomIndex->first_person	= NULL;
    pRoomIndex->last_person		= NULL;
    pRoomIndex->first_content	= NULL;
    pRoomIndex->last_content	= NULL;
    pRoomIndex->first_extradesc	= NULL;
    pRoomIndex->last_extradesc	= NULL;
    if ( ( pRoomIndex->area = get_room_area( vnum ) ) )
    {
        SET_AREA_FLAG(pRoomIndex->area, AFLAG_MODIFIED);
        log_printf_plus( LOG_BUILD, LEVEL_LOG_CSET, SEV_SPAM+9,
                         "make_room modified %s", pRoomIndex->area->name );
    }
    pRoomIndex->vnum		= vnum;
    top_room_vnum = UMAX(top_room_vnum, vnum);
    pRoomIndex->name		= STRALLOC("Floating in a void");
    pRoomIndex->description		= STRALLOC("");
    pRoomIndex->room_flags		= ROOM_PROTOTYPE;
    pRoomIndex->sector_type		= 1;
    pRoomIndex->light		= 0;
    pRoomIndex->first_exit		= NULL;
    pRoomIndex->last_exit		= NULL;
    pRoomIndex->elevation		= 1000;
    pRoomIndex->liquid		= 0;
    pRoomIndex->river		= NULL;
    xCLEAR_BITS(pRoomIndex->progtypes);

    iHash			= vnum % MAX_KEY_HASH;
    pRoomIndex->next	= room_index_hash[iHash];
    room_index_hash[iHash]	= pRoomIndex;
    top_room++;

    return pRoomIndex;
}

/*
 * Create a new INDEX object (for online building)		-Thoric
 * Option to clone an existing index object.
 */
OBJ_INDEX_DATA *make_object( int vnum, int cvnum, char *name )
{
    OBJ_INDEX_DATA *pObjIndex, *cObjIndex;
    char buf[MAX_STRING_LENGTH];
    int	iHash;

    if ( cvnum > 0 )
        cObjIndex = get_obj_index( cvnum );
    else
        cObjIndex = NULL;
    CREATE( pObjIndex, OBJ_INDEX_DATA, 1 );
    pObjIndex->ivnum			= vnum;
    top_obj_vnum = UMAX(top_obj_vnum, vnum);
    pObjIndex->name			= STRALLOC( name );
    if ( ( pObjIndex->area = get_obj_area( vnum ) ) )
    {
        SET_AREA_FLAG(pObjIndex->area, AFLAG_MODIFIED);
        log_printf_plus( LOG_BUILD, LEVEL_LOG_CSET, SEV_SPAM+9,
                         "make_object modified %s", pObjIndex->area->name );
    }

    pObjIndex->first_affect		= NULL;
    pObjIndex->last_affect		= NULL;
    pObjIndex->first_extradesc	= NULL;
    pObjIndex->last_extradesc	= NULL;
    xCLEAR_BITS(pObjIndex->progtypes);
    if ( !cObjIndex )
    {
        sprintf( buf, "A newly created %s", name );
        pObjIndex->short_descr	= STRALLOC( buf  );
        sprintf( buf, "Some god dropped a newly created %s here.", name );
        pObjIndex->description	= STRALLOC( buf );
        pObjIndex->action_desc	= STRALLOC( "" );
        pObjIndex->short_descr[0]	= LOWER(pObjIndex->short_descr[0]);
        pObjIndex->description[0]	= UPPER(pObjIndex->description[0]);
        pObjIndex->item_type		= ITEM_TRASH;
        pObjIndex->extra_flags	= ITEM_PROTOTYPE;
        pObjIndex->extra_flags2	= 0;
        pObjIndex->magic_flags	= 0;
        pObjIndex->wear_flags		= 0;
        pObjIndex->value[0]		= 0;
        pObjIndex->value[1]		= 0;
        pObjIndex->value[2]		= 0;
        pObjIndex->value[3]		= 0;
        pObjIndex->value[4]		= 0;
        pObjIndex->value[5]		= 0;
        pObjIndex->weight		= 1;
        pObjIndex->cost	                = 0;
        pObjIndex->rent	                = 0;
        pObjIndex->currtype             = DEFAULT_CURR;
        pObjIndex->spec_fun             = NULL;
    }
    else
    {
        EXTRA_DESCR_DATA *ed,  *ced;
        AFFECT_DATA	   *paf, *cpaf;

        pObjIndex->short_descr	= QUICKLINK( cObjIndex->short_descr );
        pObjIndex->description	= QUICKLINK( cObjIndex->description );
        pObjIndex->action_desc	= QUICKLINK( cObjIndex->action_desc );
        pObjIndex->item_type		= cObjIndex->item_type;
        pObjIndex->extra_flags	= cObjIndex->extra_flags
            | ITEM_PROTOTYPE;
        pObjIndex->extra_flags2	= cObjIndex->extra_flags2;
        pObjIndex->magic_flags	= cObjIndex->magic_flags;
        pObjIndex->wear_flags		= cObjIndex->wear_flags;
        pObjIndex->value[0]		= cObjIndex->value[0];
        pObjIndex->value[1]		= cObjIndex->value[1];
        pObjIndex->value[2]		= cObjIndex->value[2];
        pObjIndex->value[3]		= cObjIndex->value[3];
        pObjIndex->value[4]		= cObjIndex->value[4];
        pObjIndex->value[5]		= cObjIndex->value[5];
        pObjIndex->weight		= cObjIndex->weight;
        pObjIndex->cost	                = cObjIndex->cost;
        pObjIndex->rent	                = cObjIndex->rent;
        pObjIndex->currtype             = cObjIndex->currtype;
        pObjIndex->spec_fun             = cObjIndex->spec_fun;
        for ( ced = cObjIndex->first_extradesc; ced; ced = ced->next )
        {
            CREATE( ed, EXTRA_DESCR_DATA, 1 );
            ed->keyword		= QUICKLINK( ced->keyword );
            ed->description		= QUICKLINK( ced->description );
            LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc,
                  next, prev );
            top_ed++;
        }
        for ( cpaf = cObjIndex->first_affect; cpaf; cpaf = cpaf->next )
        {
            CREATE( paf, AFFECT_DATA, 1 );
            paf->type		= cpaf->type;
            paf->duration		= cpaf->duration;
            paf->location		= cpaf->location;
            paf->modifier		= cpaf->modifier;
            paf->bitvector		= cpaf->bitvector;
            LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect,
                  next, prev );
            top_affect++;
        }
    }
    pObjIndex->count		= 0;
    iHash				= vnum % MAX_KEY_HASH;
    pObjIndex->next			= obj_index_hash[iHash];
    obj_index_hash[iHash]		= pObjIndex;
    top_obj_index++;

    return pObjIndex;
}


AREA_DATA *get_room_area(int vnum)
{
    AREA_DATA *tarea;

    for ( tarea = first_area; tarea; tarea = tarea->next )
    {
        if (vnum >= tarea->low_r_vnum &&
            vnum <= tarea->hi_r_vnum)
            return tarea;
    }
    for ( tarea = first_build; tarea; tarea = tarea->next )
    {
        if (vnum >= tarea->low_o_vnum &&
            vnum <= tarea->hi_o_vnum)
            return tarea;
    }

    return NULL;
}

AREA_DATA *get_mob_area(int vnum)
{
    AREA_DATA *tarea;

    for ( tarea = first_area; tarea; tarea = tarea->next )
    {
        if (vnum >= tarea->low_m_vnum &&
            vnum <= tarea->hi_m_vnum)
            return tarea;
    }
    for ( tarea = first_build; tarea; tarea = tarea->next )
    {
        if (vnum >= tarea->low_o_vnum &&
            vnum <= tarea->hi_o_vnum)
            return tarea;
    }

    return get_room_area(vnum);
}

AREA_DATA *get_obj_area(int vnum)
{
    AREA_DATA *tarea;

    for ( tarea = first_area; tarea; tarea = tarea->next )
    {
        if (vnum >= tarea->low_o_vnum &&
            vnum <= tarea->hi_o_vnum)
            return tarea;
    }
    for ( tarea = first_build; tarea; tarea = tarea->next )
    {
        if (vnum >= tarea->low_o_vnum &&
            vnum <= tarea->hi_o_vnum)
            return tarea;
    }

    return get_room_area(vnum);
}

/*
 * Create a new INDEX mobile (for online building)		-Thoric
 * Option to clone an existing index mobile.
 */
MOB_INDEX_DATA *make_mobile( int vnum, int cvnum, char *name )
{
    MOB_INDEX_DATA *pMobIndex, *cMobIndex;
    char buf[MAX_STRING_LENGTH];
    int	iHash;
    int i;

    if ( cvnum > 0 )
        cMobIndex = get_mob_index( cvnum );
    else
        cMobIndex = NULL;
    CREATE( pMobIndex, MOB_INDEX_DATA, 1 );
    pMobIndex->ivnum			= vnum;
    top_mob_vnum = UMAX(top_mob_vnum, vnum);
    if ( ( pMobIndex->area = get_mob_area(vnum) ) )
    {
        SET_AREA_FLAG(pMobIndex->area, AFLAG_MODIFIED);
        log_printf_plus( LOG_BUILD, LEVEL_LOG_CSET, SEV_SPAM+9,
                         "make_mobile modified %s", pMobIndex->area->name );
    }
    pMobIndex->count		= 0;
    pMobIndex->killed		= 0;
    pMobIndex->player_name		= STRALLOC( name );
    xCLEAR_BITS(pMobIndex->progtypes);
    if ( !cMobIndex )
    {
        sprintf( buf, "A newly created %s", name );
        pMobIndex->short_descr	= STRALLOC( buf  );
        sprintf( buf, "Some god abandoned a newly created %s here.\n\r", name );
        pMobIndex->long_descr		= STRALLOC( buf );
        pMobIndex->description	= STRALLOC( "" );
        pMobIndex->short_descr[0]	= LOWER(pMobIndex->short_descr[0]);
        pMobIndex->long_descr[0]	= UPPER(pMobIndex->long_descr[0]);
        pMobIndex->description[0]	= UPPER(pMobIndex->description[0]);
        pMobIndex->act		= ACT_IS_NPC | ACT_PROTOTYPE;
        pMobIndex->act2		= 0;
        pMobIndex->affected_by	= 0;
        pMobIndex->affected_by2	= 0;
        pMobIndex->pShop		= NULL;
        pMobIndex->rShop		= NULL;
        pMobIndex->spec_fun		= NULL;
        pMobIndex->mudprogs		= NULL;
        pMobIndex->alignment		= 0;
        pMobIndex->levels[CLASS_WARRIOR]  = 1;
        pMobIndex->classes[CLASS_WARRIOR] = 1;
        pMobIndex->mobthac0		= 0;
        pMobIndex->ac			= 0;
        pMobIndex->hitnodice		= 0;
        pMobIndex->hitsizedice	= 0;
        pMobIndex->hitplus		= 0;
        pMobIndex->damnodice		= 0;
        pMobIndex->damsizedice	= 0;
        pMobIndex->damplus		= 0;
        pMobIndex->hitroll		= 0;
        pMobIndex->damroll		= 0;
        for (i=0;i<MAX_CURR_TYPE;i++)
            GET_MONEY(pMobIndex,i)      = 0;
        GET_EXP(pMobIndex)		= 0;
        pMobIndex->position		= 8;
        pMobIndex->defposition	= 8;
        pMobIndex->sex		= 0;
        pMobIndex->perm_str		= 13;
        pMobIndex->perm_dex		= 13;
        pMobIndex->perm_int		= 13;
        pMobIndex->perm_wis		= 13;
        pMobIndex->perm_cha		= 13;
        pMobIndex->perm_con		= 13;
        pMobIndex->perm_lck		= 13;
        pMobIndex->race		= 0;
        pMobIndex->xflags		= 0;
        pMobIndex->resistant		= 0;
        pMobIndex->immune		= 0;
        pMobIndex->absorb        	= 0;
        pMobIndex->susceptible	= 0;
        pMobIndex->numattacks		= 0;
        pMobIndex->attacks		= 0;
        pMobIndex->defenses		= 0;
    }
    else
    {
        pMobIndex->short_descr	= QUICKLINK( cMobIndex->short_descr );
        pMobIndex->long_descr		= QUICKLINK( cMobIndex->long_descr  );
        pMobIndex->description	= QUICKLINK( cMobIndex->description );
        pMobIndex->act		= cMobIndex->act | ACT_PROTOTYPE;
        pMobIndex->act2		= cMobIndex->act2;
        pMobIndex->affected_by	= cMobIndex->affected_by;
        pMobIndex->affected_by2	= cMobIndex->affected_by2;
        pMobIndex->pShop		= NULL;
        pMobIndex->rShop		= NULL;
        pMobIndex->spec_fun		= cMobIndex->spec_fun;
        pMobIndex->mudprogs		= NULL;
        pMobIndex->alignment		= cMobIndex->alignment;
        for (i = 0; i < MAX_CLASS; ++i)
            pMobIndex->levels[i]		= cMobIndex->levels[i];
        pMobIndex->mobthac0		= cMobIndex->mobthac0;
        pMobIndex->ac			= cMobIndex->ac;
        pMobIndex->hitnodice		= cMobIndex->hitnodice;
        pMobIndex->hitsizedice	= cMobIndex->hitsizedice;
        pMobIndex->hitplus		= cMobIndex->hitplus;
        pMobIndex->damnodice		= cMobIndex->damnodice;
        pMobIndex->damsizedice	= cMobIndex->damsizedice;
        pMobIndex->damplus		= cMobIndex->damplus;
        pMobIndex->hitroll		= cMobIndex->hitroll;
        pMobIndex->damroll		= cMobIndex->damroll;
        for (i=0;i<MAX_CURR_TYPE;i++)
            GET_MONEY(pMobIndex,i)      = GET_MONEY(cMobIndex,i);
        GET_EXP(pMobIndex)		= GET_EXP(cMobIndex);
        pMobIndex->position		= cMobIndex->position;
        pMobIndex->defposition	= cMobIndex->defposition;
        pMobIndex->sex		= cMobIndex->sex;
        pMobIndex->perm_str		= cMobIndex->perm_str;
        pMobIndex->perm_dex		= cMobIndex->perm_dex;
        pMobIndex->perm_int		= cMobIndex->perm_int;
        pMobIndex->perm_wis		= cMobIndex->perm_wis;
        pMobIndex->perm_cha		= cMobIndex->perm_cha;
        pMobIndex->perm_con		= cMobIndex->perm_con;
        pMobIndex->perm_lck		= cMobIndex->perm_lck;
        pMobIndex->race		= cMobIndex->race;
        pMobIndex->weight		= cMobIndex->weight;
        pMobIndex->height		= cMobIndex->height;
        for (i = FIRST_CLASS; i < MAX_CLASS; ++i)
            pMobIndex->classes[i]		= cMobIndex->classes[i];
        pMobIndex->xflags		= cMobIndex->xflags;
        pMobIndex->resistant		= cMobIndex->resistant;
        pMobIndex->immune		= cMobIndex->immune;
        pMobIndex->absorb		= cMobIndex->absorb;
        pMobIndex->susceptible	= cMobIndex->susceptible;
        pMobIndex->numattacks		= cMobIndex->numattacks;
        pMobIndex->attacks		= cMobIndex->attacks;
        pMobIndex->defenses		= cMobIndex->defenses;
        pMobIndex->speaks		= cMobIndex->speaks;
        pMobIndex->speaking		= cMobIndex->speaking;
        pMobIndex->saving_poison_death= cMobIndex->saving_poison_death;
        pMobIndex->saving_wand	= cMobIndex->saving_wand;
        pMobIndex->saving_para_petri	= cMobIndex->saving_para_petri;
        pMobIndex->saving_breath	= cMobIndex->saving_breath;
        pMobIndex->saving_spell_staff	= cMobIndex->saving_spell_staff;
    }
    iHash				= vnum % MAX_KEY_HASH;
    pMobIndex->next			= mob_index_hash[iHash];
    mob_index_hash[iHash]		= pMobIndex;
    top_mob_index++;

    return pMobIndex;
}

/*
 * Creates a simple exit with no fields filled but rvnum and optionally
 * to_room and vnum.						-Thoric
 * Exits are inserted into the linked list based on vdir.
 */
EXIT_DATA *make_exit( ROOM_INDEX_DATA *pRoomIndex, ROOM_INDEX_DATA *to_room, sh_int door )
{
    EXIT_DATA *pexit, *texit;
    bool broke;

    CREATE( pexit, EXIT_DATA, 1 );
    pexit->vdir	                = door;
    if (door > LAST_NORMAL_DIR)
	pexit->rdir             = door; /* arbitrary exit */
    else
        pexit->rdir             = rev_dir[door];
    pexit->rvnum		= pRoomIndex->vnum;
    pexit->to_room		= to_room;
    pexit->distance		= 1;
    if ( to_room )
    {
        pexit->vnum = to_room->vnum;
        texit = get_exit_to( to_room, pexit->rdir, pRoomIndex->vnum );
        if ( texit )	/* assign reverse exit pointers */
        {
            texit->rexit = pexit;
            pexit->rexit = texit;
        }
    }
    broke = FALSE;
    for ( texit = pRoomIndex->first_exit; texit; texit = texit->next )
        if ( door < texit->vdir )
        {
            broke = TRUE;
            break;
        }
    if ( !pRoomIndex->first_exit )
        pRoomIndex->first_exit	= pexit;
    else
    {
        /* keep exits in incremental order - insert exit into list */
        if ( broke && texit )
        {
            if ( !texit->prev )
                pRoomIndex->first_exit	= pexit;
            else
                texit->prev->next		= pexit;
            pexit->prev			= texit->prev;
            pexit->next			= texit;
            texit->prev			= pexit;
            top_exit++;
            return pexit;
        }
        pRoomIndex->last_exit->next	= pexit;
    }
    pexit->next			= NULL;
    pexit->prev			= pRoomIndex->last_exit;
    pRoomIndex->last_exit		= pexit;
    top_exit++;
    return pexit;
}

void fix_area_exits( AREA_DATA *tarea )
{
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pexit, *rev_exit;
    int rnum;
    bool fexit;

    for ( rnum = tarea->low_r_vnum; rnum <= tarea->hi_r_vnum; rnum++ )
    {
        if ( (pRoomIndex = get_room_index( rnum )) == NULL )
            continue;

        fexit = FALSE;
        for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
        {
            fexit = TRUE;
            pexit->rvnum = pRoomIndex->vnum;
            if ( pexit->vnum <= 0 )
                pexit->to_room = NULL;
            else
                pexit->to_room = get_room_index( pexit->vnum );
        }
        if ( !fexit )
            SET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
    }


    for ( rnum = tarea->low_r_vnum; rnum <= tarea->hi_r_vnum; rnum++ )
    {
        if ( (pRoomIndex = get_room_index( rnum )) == NULL )
            continue;

        for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
        {
            if ( pexit->to_room && !pexit->rexit )
            {
                rev_exit = get_exit_to( pexit->to_room, pexit->rdir, pRoomIndex->vnum );
                if ( rev_exit )
                {
                    pexit->rexit	= rev_exit;
                    rev_exit->rexit	= pexit;
                }
            }
        }
    }
}

#ifndef USE_DB
void load_area_file( AREA_DATA *tarea )
{
    char wordbuf[MAX_INPUT_LENGTH];
    gzFile gzfpArea;
    char *word;
    int x;
    char c;

    /*    FILE *fpin;
     what intelligent person stopped using fpArea?????
     if fpArea isn't being used, then no filename or linenumber
     is printed when an error occurs during loading the area..
     (bug uses fpArea)
     --TRI  */

    if ( !tarea )
    {
        bug( "Load_area: null area!" );
#ifdef THREADED_AREA_LOAD
        sem_post(&sem_areas_loaded);
        pthread_exit(0);
#endif
        return;
    }

    if ( ( gzfpArea = gzopen( tarea->filename, "r" ) ) == NULL )
    {
        perror( tarea->filename );
        bug( "load_area: error loading file (can't open [%s])", tarea->filename );
#ifdef THREADED_AREA_LOAD
        sem_post(&sem_areas_loaded);
        pthread_exit(0);
#endif
        return;
    }

    tarea->area_version = 0;
    for ( ; ; )
    {
        x = 0;
        while ( (c=gz_fread_letter( gzfpArea )) != '#' && x++<10 )
            bug( "load_area: (%s) # not found, [%c] found instead", tarea->filename, c );

        if (c != '#')
        {
            bug( "load_area: (%s) # not found, [%c] found instead", tarea->filename, c );
            exit(1);
        }

        word = gz_fread_word( gzfpArea, wordbuf );

        if ( word[0] == '$' )
            break;
        else if ( !str_cmp( word, "AREA" ) )
        {
            if ( fBootDb )
            {
                load_area    (tarea, gzfpArea);
            }
            else
            {
                DISPOSE( tarea->name );
                tarea->name = gz_fread_string_nohash( gzfpArea );
            }
        }
        /* Rennard */
        else if ( !str_cmp( word, "HELPS"    ) ) load_helps   (tarea, gzfpArea);
        else if ( !str_cmp( word, "AUTHOR"   ) ) load_author  (tarea, gzfpArea);
        else if ( !str_cmp( word, "FLAGS"    ) ) load_flags   (tarea, gzfpArea);
        else if ( !str_cmp( word, "RANGES"   ) ) load_ranges  (tarea, gzfpArea);
        else if ( !str_cmp( word, "ECONOMY"  ) ) load_economy (tarea, gzfpArea);
        else if ( !str_cmp( word, "CURRENCY" ) ) load_areacurr(tarea, gzfpArea);
        else if ( !str_cmp( word, "PLANE"    ) ) load_plane   (tarea, gzfpArea);
        else if ( !str_cmp( word, "HIGHECONOMY")) load_higheconomy (tarea, gzfpArea);
        else if ( !str_cmp( word, "LOWECONOMY")) load_loweconomy (tarea, gzfpArea);
        else if ( !str_cmp( word, "RESETMSG" ) ) load_resetmsg(tarea, gzfpArea);
        else if ( !str_cmp( word, "MOBILES"  ) ) load_mobiles (tarea, gzfpArea);
        else if ( !str_cmp( word, "OBJECTS"  ) ) load_objects (tarea, gzfpArea);
        else if ( !str_cmp( word, "MUDPROGS" ) ) load_mudprogs(tarea, gzfpArea);
        else if ( !str_cmp( word, "OBJPROGS" ) ) load_objprogs(tarea, gzfpArea);
        else if ( !str_cmp( word, "RESETS"   ) ) load_resets  (tarea, gzfpArea);
        else if ( !str_cmp( word, "ROOMS"    ) ) load_rooms   (tarea, gzfpArea);
        else if ( !str_cmp( word, "SHOPS"    ) ) load_shops   (tarea, gzfpArea);
        else if ( !str_cmp( word, "REPAIRS"  ) ) load_repairs (tarea, gzfpArea);
        else if ( !str_cmp( word, "CLIMATE"  ) ) load_climate (tarea, gzfpArea);
        else if ( !str_cmp( word, "SPECIALS" ) ) load_specials(tarea, gzfpArea);
        else if ( !str_cmp( word, "NEIGHBOR" ) ) load_neighbor(tarea, gzfpArea);
        else if ( !str_cmp( word, "VERSION"  ) ) load_version (tarea, gzfpArea);
        else if ( !str_cmp( word, "COMMENT"  ) ) load_comment (tarea, gzfpArea);
        else
        {
	    bug( "load_area: %s: bad section name: %s", tarea->filename, word );
            if ( fBootDb )
                exit( 1 );
            else
            {
                gzclose( gzfpArea );
#ifdef THREADED_AREA_LOAD
                sem_post(&sem_areas_loaded);
                pthread_exit(0);
#endif
                return;
            }
        }
    }
    gzclose( gzfpArea );
    if ( tarea )
    {
        if ( fBootDb )
            sort_area( tarea, FALSE );

        if ( tarea->low_r_vnum>0 && tarea->hi_r_vnum>=tarea->low_r_vnum )
        {
            if ( tarea->low_o_vnum == 0 ||
                 tarea->low_o_vnum>tarea->low_r_vnum)
                tarea->low_o_vnum = tarea->low_r_vnum;
            if ( tarea->hi_o_vnum < tarea->hi_r_vnum )
                tarea->hi_o_vnum = tarea->hi_r_vnum;
            if ( tarea->low_m_vnum == 0 ||
                 tarea->low_m_vnum>tarea->low_r_vnum)
                tarea->low_m_vnum = tarea->low_r_vnum;
            if ( tarea->hi_m_vnum < tarea->hi_r_vnum )
                tarea->hi_m_vnum = tarea->hi_r_vnum;
        }

        fprintf( stderr, "%-24s: R: %5d - %-5d O: %5d - %-5d M: %5d - %d\n",
                 tarea->filename,
                 tarea->low_r_vnum, tarea->hi_r_vnum,
                 tarea->low_o_vnum, tarea->hi_o_vnum,
                 tarea->low_m_vnum, tarea->hi_m_vnum );
        if (tarea->low_r_vnum < 0 || tarea->hi_r_vnum < 0)
            fprintf( stderr, "%-24s: Bad Room Range\n", tarea->filename);
        if (tarea->low_m_vnum < 0 || tarea->hi_m_vnum < 0)
            fprintf( stderr, "%-24s: Bad Mob Range\n", tarea->filename);
        if (tarea->low_o_vnum < 0 || tarea->hi_o_vnum < 0)
            fprintf( stderr, "%-24s: Bad Obj Range\n", tarea->filename);
        if ( !tarea->author )
            tarea->author = STRALLOC( "" );
        SET_BIT( tarea->status, AREA_LOADED );
    }
    else
        fprintf( stderr, "(%s)\n", tarea->filename );

#ifdef THREADED_AREA_LOAD
    sem_post(&sem_areas_loaded);
    pthread_exit(0);
#endif
}

void load_area_demand(int vnum)
{
#ifdef AREA_LOAD_ON_DEMAND
    AREA_DATA *tarea = get_room_area(vnum);

    if ( !tarea || IS_AREA_STATUS(tarea, AREA_LOADED) )
        return;

    log_printf_plus(LOG_MONITOR, LEVEL_LOG_CSET, SEV_NOTICE,
		    "%s loaded on demand",
		    tarea->filename );

    load_area_file( tarea );
    fix_exits( );
#endif
}

#endif


/* Build list of in_progress areas.  Do not load areas.
 * define AREA_READ if you want it to build area names rather than reading
 * them out of the area files. -- Altrag */
static void load_buildlist( void )
{
    DIR *dp;
    struct dirent *dentry;
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    AREA_DATA *pArea;
    char line[81];
    char word[81];
    int low, hi;
    int mlow, mhi, olow, ohi, rlow, rhi;
    bool badfile = FALSE;
    char temp;

    dp = opendir( GOD_DIR );
    dentry = readdir( dp );
    while ( dentry )
    {
        if ( dentry->d_name[0] != '.' )
        {
            sprintf( buf, "%s%s", GOD_DIR, dentry->d_name );
            if ( !(fp = fopen( buf, "r" )) )
            {
                boot_log( "Load_buildlist: invalid file (%s)", buf );
                perror( buf );
                dentry = readdir(dp);
                continue;
            }
            badfile = FALSE;
            rlow=rhi=olow=ohi=mlow=mhi=0;
            while ( !feof(fp) && !ferror(fp) )
            {
                low = 0; hi = 0; word[0] = 0; line[0] = 0;
                if ( (temp = fgetc(fp)) != EOF )
                    ungetc( temp, fp );
                else
                    break;

                fgets(line, 80, fp);
                sscanf( line, "%s %d %d", word, &low, &hi );
                if ( !strcmp( word, "Level" ) )
                {
                    if ( low < LEVEL_IMMORTAL )
                    {
                        boot_log( "%s: God file with level %d < %d",
                                 dentry->d_name, low, LEVEL_IMMORTAL );
                        badfile = TRUE;
                    }
                }
                if ( !strcmp( word, "RoomRange" ) )
                {
                    rlow = low, rhi = hi;
                }
                else if ( !strcmp( word, "MobRange" ) )
                {
                    mlow = low, mhi = hi;
                }
                else if ( !strcmp( word, "ObjRange" ) )
                {
                    olow = low, ohi = hi;
                }
            }
            FCLOSE( fp );
            if ( rlow && rhi && !badfile )
            {
                sprintf( buf, "%s%s.are", BUILD_DIR, dentry->d_name );
                if ( !(fp = fopen( buf, "r" )) )
                {
                    boot_log( "Load_buildlist: cannot open area file for read (%s)", buf );
                    dentry = readdir(dp);
                    continue;
                }
#if !defined(READ_AREA)  /* Dont always want to read stuff.. dunno.. shrug */
                strcpy( word, fread_word( fp ) );
                if ( word[0] != '#' || strcmp( &word[1], "AREA" ) )
                {
                    boot_log( "Make_buildlist: %s.are: no #AREA found.",
                             dentry->d_name );
                    FCLOSE( fp );
                    dentry = readdir(dp);
                    continue;
                }
#endif
                CREATE( pArea, AREA_DATA, 1 );
                sprintf( buf, "%s.are", dentry->d_name );
                pArea->author = STRALLOC( dentry->d_name );
                pArea->filename = str_dup( buf );
#if !defined(READ_AREA)
                pArea->name = fread_string_nohash( fp );
#else
                sprintf( buf, "{PROTO} %s's area in progress", dentry->d_name );
                pArea->name = str_dup( buf );
#endif
                FCLOSE( fp );
                pArea->low_r_vnum = rlow; pArea->hi_r_vnum = rhi;
                pArea->low_m_vnum = mlow; pArea->hi_m_vnum = mhi;
                pArea->low_o_vnum = olow; pArea->hi_o_vnum = ohi;
                pArea->low_soft_range = -1; pArea->hi_soft_range = -1;
                pArea->low_hard_range = -1; pArea->hi_hard_range = -1;

                CREATE(pArea->weather, WEATHER_DATA, 1); /* FB */
                pArea->weather->temp = 0;
                pArea->weather->precip = 0;
                pArea->weather->wind = 0;
                pArea->weather->temp_vector = 0;
                pArea->weather->precip_vector = 0;
                pArea->weather->wind_vector = 0;
                pArea->weather->climate_temp = 2;
                pArea->weather->climate_precip = 2;
                pArea->weather->climate_wind = 2;
                pArea->weather->first_neighbor = NULL;
                pArea->weather->last_neighbor = NULL;
                pArea->weather->echo = NULL;
                pArea->weather->echo_color = AT_GREY;

                pArea->first_reset = NULL; pArea->last_reset = NULL;
                LINK( pArea, first_build, last_build, next, prev );
                fprintf( stderr, "%-24s: R: %5d - %-5d O: %5d - %-5d M: %5d - %-5d\n",
                         pArea->filename,
                         pArea->low_r_vnum, pArea->hi_r_vnum,
                         pArea->low_o_vnum, pArea->hi_o_vnum,
                         pArea->low_m_vnum, pArea->hi_m_vnum );
                sort_area( pArea, TRUE );
            }
        }
        dentry = readdir(dp);
    }
    closedir(dp);
}


/*
 * Sort by room vnums					-Altrag & Thoric
 */
void sort_area( AREA_DATA *pArea, bool proto )
{
    AREA_DATA *area = NULL;
    AREA_DATA *first_sort, *last_sort;
    bool found;

    if ( !pArea )
    {
        bug( "Sort_area: NULL pArea" );
        return;
    }

#ifdef THREADED_AREA_LOAD
    sem_wait(&semaphores[SEM_AREA]);
#endif

    if ( proto )
    {
        first_sort = first_bsort;
        last_sort  = last_bsort;
    }
    else
    {
        first_sort = first_asort;
        last_sort  = last_asort;
    }

    found = FALSE;
    pArea->next_sort = NULL;
    pArea->prev_sort = NULL;

    if ( !first_sort )
    {
        pArea->prev_sort = NULL;
        pArea->next_sort = NULL;
        first_sort	 = pArea;
        last_sort	 = pArea;
        found = TRUE;
    }
    else
        for ( area = first_sort; area; area = area->next_sort )
            if ( pArea->low_r_vnum < area->low_r_vnum )
            {
                if ( !area->prev_sort )
                    first_sort	= pArea;
                else
                    area->prev_sort->next_sort = pArea;
                pArea->prev_sort = area->prev_sort;
                pArea->next_sort = area;
                area->prev_sort  = pArea;
                found = TRUE;
                break;
            }

    if ( !found )
    {
        pArea->prev_sort     = last_sort;
        pArea->next_sort     = NULL;
        last_sort->next_sort = pArea;
        last_sort	     = pArea;
    }

    if ( proto )
    {
        first_bsort = first_sort;
        last_bsort  = last_sort;
    }
    else
    {
        first_asort = first_sort;
        last_asort  = last_sort;
    }

#ifdef THREADED_AREA_LOAD
    sem_post(&semaphores[SEM_AREA]);
#endif
}


/*
 * Display vnums currently assigned to areas		-Altrag & Thoric
 * Sorted, and flagged if loaded.
 */
void show_vnums( CHAR_DATA *ch, int low, int high, bool proto, bool shownl,
                 char *loadst, char *notloadst )
{
    AREA_DATA *pArea, *first_sort;
    int count = 0, loaded = 0;
#ifndef USE_DB
    FILE *afp;
    int f=FALSE;

    if ((afp = fopen("../area/area.lst.new","w")))
        f=TRUE;
#endif

    set_pager_color( AT_PLAIN, ch );
    if ( proto )
        first_sort = first_bsort;
    else
        first_sort = first_asort;
    for ( pArea = first_sort; pArea; pArea = pArea->next_sort )
    {
        if ( IS_SET( pArea->status, AREA_DELETED ) )
            continue;
        if ( pArea->low_r_vnum < low )
            continue;
        if ( pArea->hi_r_vnum > high )
            break;
        if ( IS_SET(pArea->status, AREA_LOADED) )
            loaded++;
        else
            if ( !shownl )
                continue;
#ifndef USE_DB
        if (f && pArea->filename)
            fprintf(afp, "%s\n", pArea->filename);
#endif
        pager_printf(ch, "%-24.24s| R:%6d - %-6d"
                     " O:%6d - %-6d M:%6d - %-6d%s\n\r",
                     pArea->name,
                     pArea->low_r_vnum, pArea->hi_r_vnum,
                     pArea->low_o_vnum, pArea->hi_o_vnum,
                     pArea->low_m_vnum, pArea->hi_m_vnum,
                     IS_SET(pArea->status, AREA_LOADED) ? loadst : notloadst );
        count++;
    }
    pager_printf( ch, "Areas listed: %d  Loaded: %d\n\r", count, loaded );
#ifndef USE_DB
    if (f)
        FCLOSE(afp);
#endif
    return;
}

/*
 * Shows prototype vnums ranges, and if loaded
 */
void do_newzones( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int low, high;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    low = 0;	high = 1048575999;
    if ( arg1[0] != '\0' )
    {
        low = atoi(arg1);
        if ( arg2[0] != '\0' )
            high = atoi(arg2);
    }
    show_vnums( ch, low, high, TRUE, TRUE, " *", "" );
}

/*
 * Shows installed areas, sorted.  Mark unloaded areas with an X
 */
void do_zones( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int low, high;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    low = 0;	high = 1048575999;
    if ( arg1[0] != '\0' )
    {
        low = atoi(arg1);
        if ( arg2[0] != '\0' )
            high = atoi(arg2);
    }
    show_vnums( ch, low, high, FALSE, TRUE, "", " X" );
}

/*
 * Show prototype areas, sorted.  Only show loaded areas
 */
void do_vnums( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int low, high;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    low = 0;	high = 1048575999;
    if ( arg1[0] != '\0' )
    {
        low = atoi(arg1);
        if ( arg2[0] != '\0' )
            high = atoi(arg2);
    }
    show_vnums( ch, low, high, TRUE, FALSE, "", " X" );
}

/*
 * Save system info to data file
 */
void save_sysdata( SYSTEM_DATA sys )
{
    FILE *fp;
    char filename[MAX_INPUT_LENGTH];
    int x;

    sprintf( filename, "%ssysdata.dat", SYSTEM_DIR );

    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
        bug( "save_sysdata: fopen" );
        perror( filename );
    }
    else
    {
        fprintf( fp, "#SYSTEM\n" );
        fprintf( fp, "Highplayers    %d\n",  sys.alltimemax		);
        fprintf( fp, "Highplayertime %s~\n", sys.time_of_max		);
        fprintf( fp, "LongestUptime  %d\n",  sys.longest_uptime         );
        fprintf( fp, "Denynewplayers %d\n",  sys.DENY_NEW_PLAYERS	);
        fprintf( fp, "Nameresolving  %d\n",  sys.NO_NAME_RESOLVING	);
        fprintf( fp, "Waitforauth    %d\n",  sys.WAIT_FOR_AUTH		);
        fprintf( fp, "Specials       %d\n",  sys.specials_enabled	);
        fprintf( fp, "Readallmail    %d\n",  sys.read_all_mail		);
        fprintf( fp, "Readmailfree   %d\n",  sys.read_mail_free		);
        fprintf( fp, "Writemailfree  %d\n",  sys.write_mail_free	);
        fprintf( fp, "Takeothersmail %d\n",  sys.take_others_mail	);
        fprintf( fp, "Muse           %d\n",  sys.muse_level		);
        fprintf( fp, "Think          %d\n",  sys.think_level		);
        fprintf( fp, "Build          %d\n",  sys.build_level		);
        fprintf( fp, "Log            %d\n",  sys.log_level		);
        fprintf( fp, "Protoflag      %d\n",  sys.level_modify_proto	);
        fprintf( fp, "Overridepriv   %d\n",  sys.level_override_private	);
        fprintf( fp, "Msetplayer     %d\n",  sys.level_mset_player	);
        fprintf( fp, "Stunplrvsplr   %d\n",  sys.stun_plr_vs_plr	);
        fprintf( fp, "Stunregular    %d\n",  sys.stun_regular		);
        fprintf( fp, "Damplrvsplr    %d\n",  sys.dam_plr_vs_plr		);
        fprintf( fp, "Damplrvsmob    %d\n",  sys.dam_plr_vs_mob		);
        fprintf( fp, "Dammobvsplr    %d\n",  sys.dam_mob_vs_plr		);
        fprintf( fp, "Dammobvsmob    %d\n",  sys.dam_mob_vs_mob		);
        fprintf( fp, "Forcepc        %d\n",  sys.level_forcepc		);
	if (sys.guild_overseer && *sys.guild_overseer)
	    fprintf( fp, "Guildoverseer  %s~\n", sys.guild_overseer     );
	if (sys.guild_advisor && *sys.guild_advisor)
	    fprintf( fp, "Guildadvisor   %s~\n", sys.guild_advisor	);
        fprintf( fp, "Saveflags      %d\n",  sys.save_flags		);
        fprintf( fp, "Savefreq       %d\n",  sys.save_frequency		);
        fprintf( fp, "Introdisable   %d\n",  sys.intro_disabled		);
        fprintf( fp, "AggroPrcnt     %d\n",  sys.percent_aggr           );
        fprintf( fp, "TotalLogins    %d\n",  sys.total_logins		);
        for (x=0;x<LOG_LAST;x++)
        {
            fprintf( fp, "LogDef         %d %d %s~\n",
                     x, sys.logdefs[x].level, sys.logdefs[x].name       );
        }
        fprintf( fp, "End\n\n"						);
        fprintf( fp, "#END\n"						);
    }
    FCLOSE( fp );
    return;
}


void fread_sysdata( SYSTEM_DATA *sys, FILE *fp )
{
    const char *word = NULL;
    bool fMatch = FALSE;

    sys->time_of_max = NULL;
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
            KEY( "AggroPrcnt",  sys->percent_aggr, fread_number(fp) );

        case 'B':
            KEY( "Build",	   sys->build_level,	  fread_number( fp ) );
            break;

        case 'D':
            KEY( "Damplrvsplr",	   sys->dam_plr_vs_plr,	  fread_number( fp ) );
            KEY( "Damplrvsmob",	   sys->dam_plr_vs_mob,	  fread_number( fp ) );
            KEY( "Dammobvsplr",	   sys->dam_mob_vs_plr,	  fread_number( fp ) );
            KEY( "Dammobvsmob",	   sys->dam_mob_vs_mob,	  fread_number( fp ) );
            KEY( "Denynewplayers", sys->DENY_NEW_PLAYERS, fread_number( fp ) );
            break;

        case 'E':
            if ( !str_cmp( word, "End" ) )
            {
                if ( !sys->time_of_max )
                    sys->time_of_max = str_dup("(not recorded)");
                return;
            }
            break;

        case 'F':
            KEY( "Forcepc",	   sys->level_forcepc,	  fread_number( fp ) );
            break;

        case 'G':
            KEY( "Guildoverseer",  sys->guild_overseer,  fread_string( fp ) );
            KEY( "Guildadvisor",   sys->guild_advisor,   fread_string( fp ) );
            break;

        case 'H':
            KEY( "Highplayers",	   sys->alltimemax,	  fread_number( fp ) );
            KEY( "Highplayertime", sys->time_of_max,      fread_string_nohash( fp ) );
            break;

        case 'I':
            KEY( "Introdisable",   sys->intro_disabled,   fread_number( fp ) );
            break;

        case 'L':
            KEY( "Log",		   sys->log_level,	  fread_number( fp ) );
            KEY( "LongestUptime",  sys->longest_uptime,   fread_number( fp ) );
            if ( !str_cmp( word, "LogDef" ) )
            {
                int chan = fread_number( fp );
                fMatch = TRUE;
                if (chan < 0 || chan >= LOG_LAST)
                {
                    bug( "Fread_sysdata: invalid LogDef channel: %d", chan );
                    fread_to_eol(fp);
                    break;
                }
                sys->logdefs[chan].level = fread_number( fp );
                if (sys->logdefs[chan].name)
                    STRFREE(sys->logdefs[chan].name);
                sys->logdefs[chan].name = fread_string( fp );
            }
            break;

        case 'M':
            KEY( "Msetplayer",	   sys->level_mset_player, fread_number( fp ) );
            KEY( "Muse",	   sys->muse_level,	   fread_number( fp ) );
            break;

        case 'N':
            KEY( "Nameresolving",  sys->NO_NAME_RESOLVING, fread_number( fp ) );
            break;

        case 'O':
            KEY( "Overridepriv",   sys->level_override_private, fread_number( fp ) );
            break;

        case 'P':
            KEY( "Protoflag",	   sys->level_modify_proto, fread_number( fp ) );
            break;

        case 'R':
            KEY( "Readallmail",	   sys->read_all_mail,	fread_number( fp ) );
            KEY( "Readmailfree",   sys->read_mail_free,	fread_number( fp ) );
            break;

        case 'S':
            KEY( "Specials",	   sys->specials_enabled,fread_number( fp ) );
            KEY( "Stunplrvsplr",   sys->stun_plr_vs_plr, fread_number( fp ) );
            KEY( "Stunregular",    sys->stun_regular,	fread_number( fp ) );
            KEY( "Saveflags",	   sys->save_flags,	fread_number( fp ) );
            KEY( "Savefreq",	   sys->save_frequency,	fread_number( fp ) );
            break;

        case 'T':
            KEY( "Takeothersmail", sys->take_others_mail, fread_number( fp ) );
            KEY( "Think",	   sys->think_level,	fread_number( fp ) );
            KEY( "TotalLogins",	   sys->total_logins,	fread_number( fp ) );
            break;


        case 'W':
            KEY( "Waitforauth",	   sys->WAIT_FOR_AUTH,	  fread_number( fp ) );
            KEY( "Writemailfree",  sys->write_mail_free,  fread_number( fp ) );
            break;
        }


        if ( !fMatch )
        {
            bug( "Fread_sysdata: no match: %s", word );
            fread_to_eol( fp );
        }
    }
}



/*
 * Load the sysdata file
 */
bool load_systemdata( SYSTEM_DATA *sys )
{
    char filename[MAX_INPUT_LENGTH];
    FILE *fp;
    bool found;

    found = FALSE;
    sprintf( filename, "%ssysdata.dat", SYSTEM_DIR );

    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {

        found = TRUE;
        for ( ; ; )
        {
            char letter = '\0';
            char *word = NULL;

            letter = fread_letter( fp );
            if ( letter == '*' )
            {
                fread_to_eol( fp );
                continue;
            }

            if ( letter != '#' )
            {
                boot_log( "Load_sysdata_file: # not found." );
                break;
            }

            word = fread_word( fp );
            if ( !str_cmp( word, "SYSTEM" ) )
            {
                fread_sysdata( sys, fp );
                break;
            }
            else
                if ( !str_cmp( word, "END"	) )
                    break;
                else
                {
                    boot_log( "Load_sysdata_file: bad section." );
                    break;
                }
        }
        FCLOSE( fp );
    }

    if ( !sysdata.guild_overseer ) sysdata.guild_overseer = STRALLOC( "" );
    if ( !sysdata.guild_advisor  ) sysdata.guild_advisor  = STRALLOC( "" );
    return found;
}


void load_banlist( void )
{
    BAN_DATA *pban;
    FILE *fp;
    int number;
    char letter = '\0';

    if ( !(fp = fopen( SYSTEM_DIR BAN_LIST, "r" )) )
        return;

    for ( ; ; )
    {
        if ( feof( fp ) )
        {
            boot_log( "Load_banlist: no -1 found." );
            FCLOSE( fp );
            return;
        }
        number = fread_number( fp );
        if ( number == -1 )
        {
            FCLOSE( fp );
            return;
        }
        CREATE( pban, BAN_DATA, 1 );
        pban->level = number;
        pban->name = fread_string_nohash( fp );
        if ( (letter = fread_letter(fp)) == '~' )
            pban->ban_time = fread_string_nohash( fp );
        else
        {
            ungetc(letter, fp);
            pban->ban_time = str_dup( "(unrecorded)" );
        }
        LINK( pban, first_ban, last_ban, next, prev );
    }
}

/* Check to make sure range of vnums is free - Scryn 2/27/96 */

void do_check_vnums( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    AREA_DATA *pArea;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    bool room, mob, obj, all, area_conflict;
    int low_range, high_range;

    room = FALSE;
    mob  = FALSE;
    obj  = FALSE;
    all  = FALSE;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (arg1[0] == '\0')
    {
        send_to_char("Please specify room, mob, object, or all as your first argument.\n\r", ch);
        return;
    }

    if(!str_cmp(arg1, "room"))
        room = TRUE;

    else if(!str_cmp(arg1, "mob"))
        mob = TRUE;

    else if(!str_cmp(arg1, "object"))
        obj = TRUE;

    else if(!str_cmp(arg1, "all"))
        all = TRUE;
    else
    {
        send_to_char("Please specify room, mob, or object as your first argument.\n\r", ch);
        return;
    }

    if(arg2[0] == '\0')
    {
        send_to_char("Please specify the low end of the range to be searched.\n\r", ch);
        return;
    }

    if(argument[0] == '\0')
    {
        send_to_char("Please specify the high end of the range to be searched.\n\r", ch);
        return;
    }

    low_range = atoi(arg2);
    high_range = atoi(argument);

    if (low_range < 1 || low_range > 1048576000 )
    {
        send_to_char("Invalid argument for bottom of range.\n\r", ch);
        return;
    }

    if (high_range < 1 || high_range > 1048576000 )
    {
        send_to_char("Invalid argument for top of range.\n\r", ch);
        return;
    }

    if (high_range < low_range)
    {
        send_to_char("Bottom of range must be below top of range.\n\r", ch);
        return;
    }

    if (all)
    {
        sprintf(buf, "room %d %d", low_range, high_range);
        do_check_vnums(ch, buf);
        sprintf(buf, "mob %d %d", low_range, high_range);
        do_check_vnums(ch, buf);
        sprintf(buf, "object %d %d", low_range, high_range);
        do_check_vnums(ch, buf);
        return;
    }
    set_char_color( AT_PLAIN, ch );

    for ( pArea = first_asort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;
        if ( IS_SET( pArea->status, AREA_DELETED ) )
            continue;
        else
            if (room)
            {
                if ( low_range < pArea->low_r_vnum && pArea->low_r_vnum < high_range )
                    area_conflict = TRUE;

                if ( low_range < pArea->hi_r_vnum && pArea->hi_r_vnum < high_range )
                    area_conflict = TRUE;

                if ( ( low_range >= pArea->low_r_vnum )
                     && ( low_range <= pArea->hi_r_vnum ) )
                    area_conflict = TRUE;

                if ( ( high_range <= pArea->hi_r_vnum )
                     && ( high_range >= pArea->low_r_vnum ) )
                    area_conflict = TRUE;
            }

        if (mob)
        {
            if ( low_range < pArea->low_m_vnum && pArea->low_m_vnum < high_range )
                area_conflict = TRUE;

            if ( low_range < pArea->hi_m_vnum && pArea->hi_m_vnum < high_range )
                area_conflict = TRUE;
            if ( ( low_range >= pArea->low_m_vnum )
                 && ( low_range <= pArea->hi_m_vnum ) )
                area_conflict = TRUE;

            if ( ( high_range <= pArea->hi_m_vnum )
                 && ( high_range >= pArea->low_m_vnum ) )
                area_conflict = TRUE;
        }

        if (obj)
        {
            if ( low_range < pArea->low_o_vnum && pArea->low_o_vnum < high_range )
                area_conflict = TRUE;

            if ( low_range < pArea->hi_o_vnum && pArea->hi_o_vnum < high_range )
                area_conflict = TRUE;

            if ( ( low_range >= pArea->low_o_vnum )
                 && ( low_range <= pArea->hi_o_vnum ) )
                area_conflict = TRUE;

            if ( ( high_range <= pArea->hi_o_vnum )
                 && ( high_range >= pArea->low_o_vnum ) )
                area_conflict = TRUE;
        }

        if (area_conflict)
        {
            sprintf(buf, "Conflict:%-15s| ",
                    (pArea->filename ? pArea->filename : "(invalid)"));
            if(room)
                sprintf( buf2, "R: %5d - %-5d\n\r", pArea->low_r_vnum,
                         pArea->hi_r_vnum);
            if(mob)
                sprintf( buf2, "M: %5d - %-5d\n\r", pArea->low_m_vnum,
                         pArea->hi_m_vnum);
            if(obj)
                sprintf( buf2, "O: %5d - %-5d\n\r", pArea->low_o_vnum,
                         pArea->hi_o_vnum);

            strcat( buf, buf2 );
            send_to_char(buf, ch);
        }
    }
    for ( pArea = first_bsort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;
        if ( IS_SET( pArea->status, AREA_DELETED ) )
            continue;
        else
            if (room)
            {
                if ( low_range < pArea->low_r_vnum && pArea->low_r_vnum < high_range )
                    area_conflict = TRUE;

                if ( low_range < pArea->hi_r_vnum && pArea->hi_r_vnum < high_range )
                    area_conflict = TRUE;

                if ( ( low_range >= pArea->low_r_vnum )
                     && ( low_range <= pArea->hi_r_vnum ) )
                    area_conflict = TRUE;

                if ( ( high_range <= pArea->hi_r_vnum )
                     && ( high_range >= pArea->low_r_vnum ) )
                    area_conflict = TRUE;
            }

        if (mob)
        {
            if ( low_range < pArea->low_m_vnum && pArea->low_m_vnum < high_range )
                area_conflict = TRUE;

            if ( low_range < pArea->hi_m_vnum && pArea->hi_m_vnum < high_range )
                area_conflict = TRUE;
            if ( ( low_range >= pArea->low_m_vnum )
                 && ( low_range <= pArea->hi_m_vnum ) )
                area_conflict = TRUE;

            if ( ( high_range <= pArea->hi_m_vnum )
                 && ( high_range >= pArea->low_m_vnum ) )
                area_conflict = TRUE;
        }

        if (obj)
        {
            if ( low_range < pArea->low_o_vnum && pArea->low_o_vnum < high_range )
                area_conflict = TRUE;

            if ( low_range < pArea->hi_o_vnum && pArea->hi_o_vnum < high_range )
                area_conflict = TRUE;

            if ( ( low_range >= pArea->low_o_vnum )
                 && ( low_range <= pArea->hi_o_vnum ) )
                area_conflict = TRUE;

            if ( ( high_range <= pArea->hi_o_vnum )
                 && ( high_range >= pArea->low_o_vnum ) )
                area_conflict = TRUE;
        }

        if (area_conflict)
        {
            sprintf(buf, "Conflict:%-15s| ",
                    (pArea->filename ? pArea->filename : "(invalid)"));
            if(room)
                sprintf( buf2, "R: %5d - %-5d\n\r", pArea->low_r_vnum,
                         pArea->hi_r_vnum);
            if(mob)
                sprintf( buf2, "M: %5d - %-5d\n\r", pArea->low_m_vnum,
                         pArea->hi_m_vnum);
            if(obj)
                sprintf( buf2, "O: %5d - %-5d\n\r", pArea->low_o_vnum,
                         pArea->hi_o_vnum);

            strcat( buf, buf2 );
            send_to_char(buf, ch);
        }
    }

    /*
     for ( pArea = first_asort; pArea; pArea = pArea->next_sort )
     {
     area_conflict = FALSE;
     if ( IS_SET( pArea->status, AREA_DELETED ) )
     continue;
     else
     if (room)
     if((pArea->low_r_vnum >= low_range)
     && (pArea->hi_r_vnum <= high_range))
     area_conflict = TRUE;

     if (mob)
     if((pArea->low_m_vnum >= low_range)
     && (pArea->hi_m_vnum <= high_range))
     area_conflict = TRUE;

     if (obj)
     if((pArea->low_o_vnum >= low_range)
     && (pArea->hi_o_vnum <= high_range))
     area_conflict = TRUE;

     if (area_conflict)
     ch_printf(ch, "Conflict:%-24s| R: %5d - %-5d"
     " O: %5d - %-5d M: %5d - %-5d\n\r",
     (pArea->filename ? pArea->filename : "(invalid)"),
     pArea->low_r_vnum, pArea->hi_r_vnum,
     pArea->low_o_vnum, pArea->hi_o_vnum,
     pArea->low_m_vnum, pArea->hi_m_vnum );
     }

     for ( pArea = first_bsort; pArea; pArea = pArea->next_sort )
     {
     area_conflict = FALSE;
     if ( IS_SET( pArea->status, AREA_DELETED ) )
     continue;
     else
     if (room)
     if((pArea->low_r_vnum >= low_range)
     && (pArea->hi_r_vnum <= high_range))
     area_conflict = TRUE;

     if (mob)
     if((pArea->low_m_vnum >= low_range)
     && (pArea->hi_m_vnum <= high_range))
     area_conflict = TRUE;

     if (obj)
     if((pArea->low_o_vnum >= low_range)
     && (pArea->hi_o_vnum <= high_range))
     area_conflict = TRUE;

     if (area_conflict)
     sprintf(ch, "Conflict:%-24s| R: %5d - %-5d"
     " O: %5d - %-5d M: %5d - %-5d\n\r",
     (pArea->filename ? pArea->filename : "(invalid)"),
     pArea->low_r_vnum, pArea->hi_r_vnum,
     pArea->low_o_vnum, pArea->hi_o_vnum,
     pArea->low_m_vnum, pArea->hi_m_vnum );
     }
     */

    send_to_char("Done.\n\r", ch);
    return;
}

void free_skill(SKILLTYPE *skill)
{
    SMAUG_AFF *aff, *next_aff;

    if (skill->name)
        DISPOSE(skill->name);

    if (skill->noun_damage)
        DISPOSE(skill->noun_damage);
    if (skill->msg_off)
        DISPOSE(skill->msg_off);
    if (skill->msg_off_room)
        DISPOSE(skill->msg_off_room);
    if (skill->msg_off_soon)
        DISPOSE(skill->msg_off_soon);
    if (skill->msg_off_soon_room)
        DISPOSE(skill->msg_off_soon_room);

    if (skill->hit_char)
        DISPOSE(skill->hit_char);
    if (skill->hit_vict)
        DISPOSE(skill->hit_vict);
    if (skill->hit_room)
        DISPOSE(skill->hit_room);
    if (skill->miss_char)
        DISPOSE(skill->miss_char);
    if (skill->miss_vict)
        DISPOSE(skill->miss_vict);
    if (skill->miss_room)
        DISPOSE(skill->miss_room);
    if (skill->die_char)
        DISPOSE(skill->die_char);
    if (skill->die_vict)
        DISPOSE(skill->die_vict);
    if (skill->die_room)
        DISPOSE(skill->die_room);
    if (skill->imm_char)
        DISPOSE(skill->imm_char);
    if (skill->imm_vict)
        DISPOSE(skill->imm_vict);
    if (skill->imm_room)
        DISPOSE(skill->imm_room);
    if (skill->abs_char)
        DISPOSE(skill->abs_char);
    if (skill->abs_vict)
        DISPOSE(skill->abs_vict);
    if (skill->abs_room)
        DISPOSE(skill->abs_room);

    if (skill->dice)
        DISPOSE(skill->dice);
    if (skill->components)
        DISPOSE(skill->components);
    if (skill->teachers)
        DISPOSE(skill->teachers);

    if (skill->corpse_string)
        DISPOSE(skill->corpse_string);

    if (skill->part_start_char)
        DISPOSE(skill->part_start_char);
    if (skill->part_start_room)
        DISPOSE(skill->part_start_room);
    if (skill->part_end_char)
        DISPOSE(skill->part_end_char);
    if (skill->part_end_vict)
        DISPOSE(skill->part_end_vict);
    if (skill->part_end_room)
        DISPOSE(skill->part_end_room);
    if (skill->part_end_caster)
        DISPOSE(skill->part_end_caster);
    if (skill->part_miss_char)
        DISPOSE(skill->part_miss_char);
    if (skill->part_miss_room)
        DISPOSE(skill->part_miss_room);
    if (skill->part_abort_char)
        DISPOSE(skill->part_abort_char);

    for (aff=skill->affects;aff;aff=next_aff)
    {
        next_aff = aff->next;
        DISPOSE(aff->duration);
        DISPOSE(aff->modifier);
        DISPOSE(aff);
    }

    DISPOSE(skill);
}

void free_board(BOARD_DATA *board)
{
    NOTE_DATA *note, *next_note;

    if (board->note_file)
        DISPOSE(board->note_file);
    if (board->read_group)
        DISPOSE(board->read_group);
    if (board->post_group)
        DISPOSE(board->post_group);
    if (board->extra_readers)
        DISPOSE(board->extra_readers);
    if (board->extra_removers)
        DISPOSE(board->extra_removers);

    for (note=board->first_note;note;note=next_note)
    {
        next_note = note->next;
        free_note(note);
    }

    DISPOSE(board);
}

void free_deity(DEITY_DATA *deity)
{
    if (deity->name)
        STRFREE(deity->name);
    if (deity->description)
        STRFREE(deity->description);
    if (deity->filename)
        DISPOSE(deity->filename);

    DISPOSE(deity);
}

void free_mud_data(void)
{
    DESCRIPTOR_DATA *d, *next_d;
    HELP_DATA *help, *next_help;
    AREA_DATA *area, *next_area;
    SHOP_DATA *shop, *next_shop;
    REPAIR_DATA *repair, *next_repair;
    CHAR_DATA *ch, *next_ch;
    OBJ_DATA *objd, *next_objd;
    ROOM_INDEX_DATA *room, *next_room;
    MOB_INDEX_DATA *mob, *next_mob;
    OBJ_INDEX_DATA *obj, *next_obj;
    SOCIALTYPE *soc, *next_soc;
    CMDTYPE *cmd, *next_cmd;
    BOARD_DATA *board, *next_board;
    DEITY_DATA *deity, *next_deity;
    BAN_DATA *ban;
    int iHash,x,y;

#ifdef I3
    void free_i3data(void);
    void destroy_I3_mud( I3_MUD *mud );
#endif

    void free_properties(void);
    void free_commodities(void);
    void free_currencies(void);
    void free_christens(void);
    void free_clans(void);
    void free_poses(void);
    void free_langs(void);
    void free_room_act_list(void);

    fprintf(stderr, "Descriptors...\n");
    for (d=first_descriptor;d;d=next_d)
    {
        next_d = d->next;
        close_socket(d,TRUE);
    }
    fprintf(stderr, "Help...\n");
    for ( help = first_help; help; help = next_help )
    {
        next_help = help->next;
        STRFREE( help->text );
        STRFREE( help->keyword );
        DISPOSE( help );
    }
    if (supermob)
    {
	UNLINK(supermob, first_char, last_char, next, prev);
	char_from_room(supermob);
	free_char(supermob);
    }
    fprintf(stderr, "Areas...\n");
    for (area=first_area;area;area=next_area)
    {
        next_area = area->next;
        close_area(area, FALSE);
    }
    fprintf(stderr, "Builds...\n");
    for (area=first_build;area;area=next_area)
    {
        next_area = area->next;
        close_area(area, TRUE);
    }
    fprintf(stderr, "Shops...\n");
    for (shop=first_shop;shop;shop=next_shop)
    {
        next_shop = shop->next;
        DISPOSE(shop);
    }
    fprintf(stderr, "Repairs...\n");
    for (repair=first_repair;repair;repair=next_repair)
    {
        next_repair = repair->next;
        DISPOSE(repair);
    }
    fprintf(stderr, "Chars...\n");
    for (ch=first_char;ch;ch=next_ch)
    {
        next_ch = ch->next;
        extract_char(ch,TRUE);
    }
    fprintf(stderr, "Objs...\n");
    for (objd=first_object;objd;objd=next_objd)
    {
        next_objd = objd->next;
        extract_obj(objd);
    }
    fprintf(stderr, "Rooms...\n");
    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
        for ( room = room_index_hash[iHash]; room; room = next_room )
        {
            next_room = room->next;
            delete_room(room);
        }
    fprintf(stderr, "MobIndex...\n");
    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
        for ( mob = mob_index_hash[iHash]; mob; mob = next_mob )
        {
            next_mob = mob->next;
            delete_mob(mob);
        }
    fprintf(stderr, "ObjIndex...\n");
    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
        for ( obj = obj_index_hash[iHash]; obj; obj = next_obj )
        {
            next_obj = obj->next;
            delete_obj(obj);
        }
    fprintf(stderr, "Skills...\n");
    for (x=0;x<top_sn;x++)
    {
        free_skill(skill_table[x]);
    }
    fprintf(stderr, "Herbs...\n");
    for (x=0;x<top_herb;x++)
    {
        free_skill(herb_table[x]);
    }
    fprintf(stderr, "Title...\n");
    for (x=0;x<MAX_CLASS;x++)
        for (y=0;y<MAX_LEVEL+1;y++)
        {
            if (title_table[x][y][0])
                DISPOSE(title_table[x][y][0]);
            if (title_table[x][y][1])
                DISPOSE(title_table[x][y][1]);
        }
    fprintf(stderr, "Social...\n");
    for ( iHash = 0; iHash < 27; iHash++ )
        for ( soc = social_index[iHash]; soc; soc = next_soc )
        {
            next_soc = soc->next;
            free_social(soc);
        }
    fprintf(stderr, "Command...\n");
    for ( iHash = 0; iHash < 126; iHash++ )
        for ( cmd = command_hash[iHash]; cmd; cmd = next_cmd )
        {
            next_cmd = cmd->next;
            free_command(cmd);
        }
    fprintf(stderr, "Boards...\n");
    for (board=first_board;board;board=next_board)
    {
        next_board = board->next;
        free_board(board);
    }
    fprintf(stderr, "Classes...\n");
    for (x=0;x<MAX_CLASS;x++)
    {
        if (class_table[x]->who_name)
            STRFREE(class_table[x]->who_name);
        DISPOSE(class_table[x]);
    }
    fprintf(stderr, "Deitys...\n");
    for (deity=first_deity;deity;deity=next_deity)
    {
        next_deity = deity->next;
        free_deity(deity);
    }

    fprintf(stderr, "Misc...\n");
    if (sysdata.time_of_max)
        DISPOSE(sysdata.time_of_max);
    if (sysdata.guild_overseer)
        STRFREE(sysdata.guild_overseer);
    if (sysdata.guild_advisor)
        STRFREE(sysdata.guild_advisor);
    for (x=LOG_NORMAL;x<LOG_LAST;x++)
	STRFREE(sysdata.logdefs[x].name);

    while ((ban=first_ban))
    {
        DISPOSE(ban->name);
        DISPOSE(ban->ban_time);
        UNLINK(ban,first_ban,last_ban,next,prev);
        DISPOSE(ban);
    }
    free_properties();
    free_commodities();
    free_currencies();
    free_christens();
    free_clans();
    fprintf(stderr, "Poses...\n");
    free_poses();
    free_quests();
    free_bugtracks();
    free_langs();
    free_rare_objs();
#ifdef I3
    fprintf(stderr, "I3...\n");
    free_i3data();
    destroy_I3_mud( this_mud );
#endif
    fprintf(stderr, "Auction...\n");
    DISPOSE(auction);

    free_room_act_list();

    fprintf(stderr, "Done...\n");
}

/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain( void )
{
    return;
}

/*
 * Initialize the weather for all the areas
 * Last Modified: July 21, 1997
 * Fireblade
 */
void init_area_weather()
{
    AREA_DATA *pArea;
    NEIGHBOR_DATA *neigh;
    NEIGHBOR_DATA *next_neigh;

    for(pArea = first_area; pArea; pArea = pArea->next)
    {
        int cf;

        /* init temp and temp vector */
        cf = pArea->weather->climate_temp - 2;
        pArea->weather->temp =
            number_range(-weath_unit, weath_unit) +
            cf * number_range(0, weath_unit);
        pArea->weather->temp_vector =
            cf + number_range(-rand_factor, rand_factor);

        /* init precip and precip vector */
        cf = pArea->weather->climate_precip - 2;
        pArea->weather->precip =
            number_range(-weath_unit, weath_unit) +
            cf * number_range(0, weath_unit);
        pArea->weather->precip_vector =
            cf + number_range(-rand_factor, rand_factor);

        /* init wind and wind vector */
        cf = pArea->weather->climate_wind - 2;
        pArea->weather->wind =
            number_range(-weath_unit, weath_unit) +
            cf * number_range(0, weath_unit);
        pArea->weather->wind_vector =
            cf + number_range(-rand_factor, rand_factor);

        /* check connections between neighbors */
        for(neigh = pArea->weather->first_neighbor; neigh;
            neigh = next_neigh)
        {
            AREA_DATA *tarea;
            NEIGHBOR_DATA *tneigh;

            /* get the address if needed */
            if(!neigh->address)
                neigh->address = get_area(neigh->name);

            /* area does not exist */
            if(!neigh->address)
            {
                tneigh = neigh;
                next_neigh = tneigh->next;
                UNLINK(tneigh,
                       pArea->weather->first_neighbor,
                       pArea->weather->last_neighbor,
                       next,
                       prev);
                STRFREE(tneigh->name);
                DISPOSE(tneigh);
                fold_area(pArea, pArea->filename, FALSE);
                continue;
            }

            /* make sure neighbors both point to each other */
            tarea = neigh->address;
            for(tneigh = tarea->weather->first_neighbor; tneigh;
                tneigh = tneigh->next)
            {
                if(!strcmp(pArea->name, tneigh->name))
                    break;
            }

            if(!tneigh)
            {
                CREATE(tneigh, NEIGHBOR_DATA, 1);
                tneigh->name = STRALLOC(pArea->name);
                LINK(tneigh,
                     tarea->weather->first_neighbor,
                     tarea->weather->last_neighbor,
                     next,
                     prev);
                fold_area(tarea, tarea->filename, FALSE);
            }

            tneigh->address = pArea;

            next_neigh = neigh->next;
        }
    }

    return;
}

/*
 * Load weather data from appropriate file in system dir
 * Last Modified: July 24, 1997
 * Fireblade
 */
void load_weatherdata()
{
    char filename[MAX_INPUT_LENGTH];
    FILE *fp;

    sprintf(filename, "%sweather.dat", SYSTEM_DIR);

    if((fp = fopen(filename, "r")) != NULL)
    {
        for(;;)
        {
            char letter;
            char *word;

            letter = fread_letter(fp);

            if(letter != '#')
            {
                bug("load_weatherdata: # not found");
                return;
            }

            word = fread_word(fp);

            if(!str_cmp(word, "RANDOM"))
                rand_factor = fread_number(fp);
            else if(!str_cmp(word, "CLIMATE"))
                climate_factor = fread_number(fp);
            else if(!str_cmp(word, "NEIGHBOR"))
                neigh_factor = fread_number(fp);
            else if(!str_cmp(word, "UNIT"))
                weath_unit = fread_number(fp);
            else if(!str_cmp(word, "MAXVECTOR"))
                max_vector = fread_number(fp);
            else if(!str_cmp(word, "END"))
            {
                FCLOSE(fp);
                break;
            }
            else
            {
                bug("load_weatherdata: unknown field");
                FCLOSE(fp);
                break;
            }
        }
    }

    return;
}

/*
 * Write data for global weather parameters
 * Last Modified: July 24, 1997
 * Fireblade
 */
void save_weatherdata()
{
    char filename[MAX_INPUT_LENGTH];
    FILE *fp;

    sprintf(filename, "%sweather.dat", SYSTEM_DIR);

    if((fp = fopen(filename, "w")) != NULL)
    {
        fprintf(fp, "#RANDOM %d\n", rand_factor);
        fprintf(fp, "#CLIMATE %d\n", climate_factor);
        fprintf(fp, "#NEIGHBOR %d\n", neigh_factor);
        fprintf(fp, "#UNIT %d\n", weath_unit);
        fprintf(fp, "#MAXVECTOR %d\n", max_vector);
        fprintf(fp, "#END\n");
        FCLOSE(fp);
    }
    else
    {
        bug("save_weatherdata: could not open file");
    }

    return;
}

