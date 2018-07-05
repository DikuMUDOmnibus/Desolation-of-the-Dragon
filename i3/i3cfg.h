/****************************************************************************
 *                   ^     +----- |  / ^     ^ |     | +-\                  *
 *                  / \    |      | /  |\   /| |     | |  \                 *
 *                 /   \   +---   |<   | \ / | |     | |  |                 *
 *                /-----\  |      | \  |  v  | |     | |  /                 *
 *               /       \ |      |  \ |     | +-----+ +-/                  *
 ****************************************************************************
 * AFKMud Copyright 1997-2002 Alsherok. Contributors: Samson, Dwip, Whir,   *
 * Cyberfox, Karangi, Rathian, Cam, Raine, and Tarl.                        *
 *                                                                          *
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag,        *
 * Blodkai, Haus, Narn, Scryn, Swordbearer, Tricops, Gorog, Rennard,        *
 * Grishnakh, Fireblade, and Nivek.                                         *
 *                                                                          *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                      *
 *                                                                          *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen,      *
 * Michael Seifert, and Sebastian Hammer.                                   *
 ****************************************************************************
 *                         Intermud-3 Network Module                        *
 ****************************************************************************/

/*
 * Copyright (c) 2000 Fatal Dimensions
 *
 * See the file "LICENSE" or information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

/* Ported to Smaug 1.4a by Samson of Alsherok.
 * Consolidated for cross-codebase compatibility by Samson of Alsherok.
 * Modifications and enhancements to the code
 * Copyright (c)2001-2002 Roger Libiez ( Samson )
 * Registered with the United States Copyright Office
 * TX 5-562-404
 *
 * Contains codebase specific defines to make the rest of it all work - hopefully.
 * Anything your codebase needs to alter is more than likely going to be stored in here.
 * This should be the only file you need to edit to solve unforseen compiling problems
 * if I've done this properly. And remember, this is all based on what these defines mean
 * in your STOCK code. If you've made adjustments to any of it, then you'll need to adjust
 * them here too.
 */

/* Locations of the configuration files */
#define I3_CONFIG_FILE   "../i3/i3.config"
#define I3_CHANNEL_FILE  "../i3/i3.channels"
#define I3_PASSWORD_FILE "../i3/i3.password"
#define I3_UCACHE_FILE   "../i3/i3.ucache"
#define I3_MUDLIST_FILE  "../i3/i3.mudlist"
#define I3_CHANLIST_FILE "../i3/i3.chanlist"

#if defined(I3SMAUG) || defined(I3CHRONICLES)
   #define SMAUGSOCIAL
   #define SOCIAL_DATA SOCIALTYPE
   #define I3MAXPLAYERS sysdata.maxplayers
   #define I3STRALLOC STRALLOC
   #define I3STRFREE STRFREE
   #define I3MAX_LEVEL MAX_LEVEL
   #define CH_I3DATA(ch)	 ((ch)->pcdata->i3chardata)
   #define CH_LEVEL(ch)		 ((ch)->level)
   #define CH_NAME(ch)		 ((ch)->name)
   #define CH_TITLE(ch) 	 ((ch)->pcdata->title)
   #define CH_RANK(ch)		 ((ch)->pcdata->rank)
   #define CH_SEX(ch)		 ((ch)->sex)
   #define CH_LOGON(ch)		 (&(ch)->logon)
   #define CH_CLASSNAME(ch)	 (class_table[(ch)->class]->who_name)
   #define CH_CLAN(ch)		 ((ch)->pcdata->clan)
   #define CH_CLANNAME(ch)	 ((ch)->pcdata->clan->name)
   #define CH_CLANLEADNAME(ch) ((ch)->pcdata->clan->leader)
   #define CH_CLANONENAME(ch)  ((ch)->pcdata->clan->number1)
   #define CH_CLANTWONAME(ch)  ((ch)->pcdata->clan->number2)
   #define CH_CLANLEADRANK(ch) ((ch)->pcdata->clan->leadrank)
   #define CH_CLANONERANK(ch)  ((ch)->pcdata->clan->onerank)
   #define CH_CLANTWORANK(ch)  ((ch)->pcdata->clan->tworank)
   #define FIRST_I3IGNORE(ch)  ((ch)->pcdata->i3chardata->i3first_ignore)
   #define LAST_I3IGNORE(ch)	 ((ch)->pcdata->i3chardata->i3last_ignore)
   #define I3LISTEN(ch)		 ((ch)->pcdata->i3chardata->i3_listen)
   #define I3REPLY(ch)  	 ((ch)->pcdata->i3chardata->i3_replyname)
   #define CH_AFK(ch)		 (xIS_SET((ch)->act, PLR_AFK))
   #define CH_PK(ch)		 IS_PKILL((ch))
   #define I3INVIS(ch)		 ((ch)->pcdata->i3chardata->i3invis)
   #define WIZINVIS(ch)		 (xIS_SET((ch)->act, PLR_WIZINVIS) && (ch)->pcdata->wizinvis >= this_mud->minlevel )
   #define I3ISINVIS(ch)	 ( WIZINVIS((ch)) || I3INVIS((ch)) )
   #define I3NOTELL(ch) 	 (xIS_SET((ch)->act, PLR_NO_TELL) || IS_SET((ch)->deaf, CHANNEL_TELLS))
#endif

#ifdef I3ROM
   #define first_descriptor descriptor_list
   #define I3MAX_LEVEL MAX_LEVEL
   #define I3MAXPLAYERS -1 /* Rom evidently does not have this available */
   #define I3STRALLOC str_dup
   #define I3STRFREE free_string
   #define CH_I3DATA(ch)	 ((ch)->pcdata->i3chardata)
   #define CH_LEVEL(ch)		((ch)->level)
   #define CH_NAME(ch)		((ch)->name)
   #define CH_TITLE(ch) 	((ch)->pcdata->title)
   #define CH_LOGON(ch)		(&(ch)->logon)
   #define CH_SEX(ch)		((ch)->sex)
   #define FIRST_I3IGNORE(ch) ((ch)->pcdata->i3chardata->i3first_ignore)
   #define LAST_I3IGNORE(ch)	((ch)->pcdata->i3chardata->i3last_ignore)
   #define I3LISTEN(ch)		((ch)->pcdata->i3chardata->i3_listen)
   #define I3REPLY(ch)  	((ch)->pcdata->i3chardata->i3_replyname)
   #define CH_CLAN(ch)		((ch)->clan)
   #define CH_CLANNAME(ch)	(clan_table[(ch)->clan].who_name)
   #define CH_CLANLEADNAME(ch) ( 0 == 1 )
   #define CH_CLANONENAME(ch)  ( 0 == 1 )
   #define CH_CLANTWONAME(ch)  ( 0 == 1 )
   #define CH_CLANLEADRANK(ch) ( "Leader" )
   #define CH_CLANONERANK(ch)  ( "First Officer" )
   #define CH_CLANTWORANK(ch)  ( "Second Officer" )
   #define CH_CLASSNAME(ch)	(class_table[(ch)->class].who_name)
   #define CH_RANK(ch)		(title_table[(ch)->class][(ch)->level][(ch)->sex == SEX_FEMALE ? 1 : 0])
   #define CH_AFK(ch)		(IS_SET((ch)->comm, COMM_AFK ))
   #define I3NOTELL(ch) 	(IS_SET((ch)->comm, COMM_NOTELL ))
   #define CH_PK(ch)		( 0 == 1 )
   #define WIZINVIS(ch)		(IS_IMMORTAL((ch)) && (ch)->invis_level > 0)
   #define I3INVIS(ch)		((ch)->pcdata->i3chardata->i3invis)
   #define I3ISINVIS(ch)	( WIZINVIS((ch)) || I3INVIS((ch)) )
#endif

#ifdef I3MERC
   #define first_descriptor descriptor_list
   #define I3MAX_LEVEL MAX_LEVEL
   #define I3MAXPLAYERS -1 /* Merc doesn't track this */
   #define I3STRALLOC str_dup
   #define I3STRFREE free_string
   #define CH_I3DATA(ch)	 ((ch)->pcdata->i3chardata)
   #define CH_LEVEL(ch)		((ch)->level)
   #define CH_NAME(ch)		((ch)->name)
   #define CH_SEX(ch)		((ch)->sex)
   #define CH_LOGON(ch)		(&(ch)->logon)
   #define CH_TITLE(ch) 	((ch)->pcdata->title)
   #define CH_CLASSNAME(ch)	(class_table[(ch)->class].who_name)
   #define CH_RANK(ch)		(title_table[(ch)->class][(ch)->level][(ch)->sex == SEX_FEMALE ? 1 : 0])
   #define FIRST_I3IGNORE(ch) ((ch)->pcdata->i3chardata->i3first_ignore)
   #define LAST_I3IGNORE(ch)	((ch)->pcdata->i3chardata->i3last_ignore)
   #define I3LISTEN(ch)		((ch)->pcdata->i3chardata->i3_listen)
   #define I3REPLY(ch)  	((ch)->pcdata->i3chardata->i3_replyname)
   #define I3NOTELL(ch) 	(IS_SET((ch)->act, PLR_NO_TELL))
   #define CH_PK(ch)		( 0 == 1 )
   #define CH_AFK(ch)		( 0 == 1 )
   #define CH_CLAN(ch)		( 0 == 1 )
   #define CH_CLANNAME(ch)	( 0 == 1 )
   #define CH_CLANLEADNAME(ch) ( 0 == 1 )
   #define CH_CLANONENAME(ch)  ( 0 == 1 )
   #define CH_CLANTWONAME(ch)  ( 0 == 1 )
   #define CH_CLANLEADRANK(ch) ( "Leader" )
   #define CH_CLANONERANK(ch)  ( "First Officer" )
   #define CH_CLANTWORANK(ch)  ( "Second Officer" )
   #define WIZINVIS(ch)		(IS_IMMORTAL((ch)) && IS_SET((ch)->act, PLR_WIZINVIS))
   #define I3INVIS(ch)		((ch)->pcdata->i3chardata->i3invis)
   #define I3ISINVIS(ch)	( WIZINVIS((ch)) || I3INVIS((ch)) )
#endif

#ifdef I3UENVY
   #define SMAUGSOCIAL
   #define SOCIAL_DATA SOC_INDEX_DATA
   SOC_INDEX_DATA *find_social( char *command );
   #define first_descriptor descriptor_list
   #define I3MAX_LEVEL MAX_LEVEL
   #define I3MAXPLAYERS sysdata.max_players
   #define I3STRALLOC str_dup
   #define I3STRFREE free_string
   #define CH_I3DATA(ch)	 ((ch)->pcdata->i3chardata)
   #define CH_LEVEL(ch)         ((ch)->level)
   #define CH_NAME(ch)          ((ch)->name)
   #define CH_TITLE(ch)         ((ch)->pcdata->title)
   #define CH_SEX(ch)		  ((ch)->sex)
   #define CH_LOGON(ch)		  (&(ch)->logon)
   #define CH_CLASSNAME(ch)     (class_table[(ch)->class]->name)
   #define CH_RANK(ch)		  (title_table[(ch)->class][(ch)->level][(ch)->sex == SEX_FEMALE ? 1 : 0])
   #define CH_CLAN(ch)          ((ch)->pcdata->clan)
   #define CH_CLANNAME(ch)      ((ch)->pcdata->clan->who_name)
   #define CH_CLANLEADNAME(ch) ((ch)->pcdata->clan->overlord)
   #define CH_CLANONENAME(ch)  ( 0 == 1 )
   #define CH_CLANTWONAME(ch)  ( 0 == 1 )
   #define CH_CLANLEADRANK(ch) ( "Overlord" )
   #define CH_CLANONERANK(ch)  ( "Chieftan" )
   #define CH_CLANTWORANK(ch)  ( "Second Officer" )
   #define FIRST_I3IGNORE(ch)   ((ch)->pcdata->i3chardata->i3first_ignore) 
   #define LAST_I3IGNORE(ch)    ((ch)->pcdata->i3chardata->i3last_ignore)
   #define I3LISTEN(ch)         ((ch)->pcdata->i3chardata->i3_listen)
   #define I3REPLY(ch)          ((ch)->pcdata->i3chardata->i3_replyname)
   #define I3NOTELL(ch)         (IS_SET((ch)->act, PLR_NO_TELL))
   #define CH_PK(ch)            (IS_SET((ch)->act, PLR_REGISTER))
   #define CH_AFK(ch)           (IS_SET((ch)->act, PLR_AFK))
   #define WIZINVIS(ch)		(IS_IMMORTAL((ch)) && IS_SET((ch)->act, PLR_WIZINVIS))
   #define I3INVIS(ch)		((ch)->pcdata->i3chardata->i3invis)
   #define I3ISINVIS(ch)	( WIZINVIS((ch)) || I3INVIS((ch)) )
#endif

#ifdef I3ACK
   extern int max_players;
   #define first_descriptor first_desc
   #define I3MAX_LEVEL MAX_LEVEL
   #define I3MAXPLAYERS max_players
   #define I3STRALLOC str_dup
   #define I3STRFREE free_string
   #define CH_I3DATA(ch)	 ((ch)->pcdata->i3chardata)
   #define CH_LEVEL(ch)		((ch)->level)
   #define CH_NAME(ch)		((ch)->name)
   #define CH_TITLE(ch) 	((ch)->pcdata->title)
   #define CH_SEX(ch)		((ch)->sex)
   #define CH_LOGON(ch)		(&(ch)->logon)
   #define CH_CLASSNAME(ch)	(class_table[(ch)->class].who_name)
   #define CH_RANK(ch)		(class_table[(ch)->class].who_name)
   #define I3REPLY(ch)  	((ch)->pcdata->i3chardata->i3_replyname)
   #define FIRST_I3IGNORE(ch) ((ch)->pcdata->i3chardata->i3first_ignore)
   #define LAST_I3IGNORE(ch)	((ch)->pcdata->i3chardata->i3last_ignore)
   #define I3LISTEN(ch)		((ch)->pcdata->i3chardata->i3_listen)
   #define CH_AFK(ch)		(IS_SET((ch)->pcdata->pflags, PFLAG_AFK))
   #define CH_PK(ch)		(IS_SET((ch)->pcdata->pflags, PFLAG_PKOK))
   #define CH_CLAN(ch)		((ch)->pcdata->clan)
   #define CH_CLANNAME(ch)	(clan_table[(ch)->pcdata->clan].clan_name)
   #define CH_CLANLEADNAME(ch) (clan_table[(ch)->pcdata->clan].leader)
   #define CH_CLANONENAME(ch)  ( 0 == 1 )
   #define CH_CLANTWONAME(ch)  ( 0 == 1 )
   #define CH_CLANLEADRANK(ch) ( "Leader" )
   #define CH_CLANONERANK(ch)  ( "First Officer" )
   #define CH_CLANTWORANK(ch)  ( "Second Officer" )
   #define I3NOTELL(ch) 	(IS_SET((ch)->act, PLR_NO_TELL ))
   #define WIZINVIS(ch)		(IS_IMMORTAL((ch)) && (ch)->invis > 0)
   #define I3INVIS(ch)		((ch)->pcdata->i3chardata->i3invis)
   #define I3ISINVIS(ch)	( WIZINVIS((ch)) || I3INVIS((ch)) )
#endif

#ifdef I3CIRCLE
//   #if _CIRCLEMUD < CIRCLEMUD_VERSION(3, 0, 21)
//   #  error "Requires CircleMUD 3.0 bpl21+ (varargs output functions)"
//   #endif

   /* This should be in an act.social.h, if it existed. Introducing
      it in an I3 patch would be too intrusive. */
   struct social_messg 
   {
     int act_nr;
     int hide;
     int min_victim_position;
     char *char_no_arg;
     char *others_no_arg;
     char *char_found;
     char *others_found;
     char *vict_found;
     char *not_found;
     char *char_auto;
     char *others_auto;
   };
   #define social_table soc_mess_list
   extern struct social_messg *soc_mess_list;
   #define SMAUGSOCIAL
   struct social_messg *find_social(const char *name);

   typedef struct social_messg SOCIAL_DATA;
   typedef struct char_data CHAR_DATA;
   typedef struct descriptor_data DESCRIPTOR_DATA;

   extern const char *class_abbrevs[];
   extern int max_players;

   const char *title_female(int chclass, int level);
   const char *title_male(int chclass, int level);
   void smash_tilde(char *str);

   #define capitalize(x)		CAP(x)
   #define first_descriptor		descriptor_list
   #define is_name			is_abbrev
   #define current_time			time(NULL)
   #define get_trust(ch)		GET_LEVEL(ch)
   #define I3STRALLOC			strdup
   #define I3STRFREE			free
   #define I3MAXPLAYERS			max_players	/* comm.c */
   #define PULSE_PER_SECOND		PASSES_PER_SEC
   #define log_string			basic_mud_log
   #define bug				basic_mud_log
   #define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
   #define get_char_world(ch, vict)	get_char_vis((ch), (vict), NULL, FIND_CHAR_WORLD)
   #define write_to_buffer(d, txt, n)	write_to_output((txt), (d))
   #define LEVEL_IMMORTAL		LVL_IMMORT
   #define I3MAX_LEVEL			LVL_IMPL
   #define IS_IMMORTAL(ch)		(GET_LEVEL(ch) >= LVL_IMMORT)
   #define CH_LEVEL(ch)          GET_LEVEL(ch)
   #define CH_NAME(ch)           GET_NAME(ch)
   #define CH_TITLE(ch)          GET_TITLE(ch)
   #define CH_RANK(ch)           (GET_SEX(ch) == SEX_FEMALE ? title_female(GET_CLASS(ch), GET_LEVEL(ch))	\
								: title_male(GET_CLASS(ch), GET_LEVEL(ch)))
   #define CH_SEX(ch)            GET_SEX(ch)
   #define CH_CLASSNAME(ch)      CLASS_ABBR(ch)
   #define CH_CLAN(ch)           FALSE
   #define CH_CLANNAME(ch)       ""
   #define CH_CLANLEADNAME(ch)   FALSE
   #define CH_CLANONENAME(ch)    FALSE
   #define CH_CLANTWONAME(ch)    FALSE
   #define CH_CLANLEADRANK(ch)   "Leader"
   #define CH_CLANONERANK(ch)    "First Officer"
   #define CH_CLANTWORANK(ch)    "Second Officer"
   #define I3REPLY(ch)		((ch)->player_specials->i3_replyname)
   #define I3LISTEN(ch)		((ch)->player_specials->i3_listen)
   #define FIRST_I3IGNORE(ch)      ((ch)->player_specials->i3first_ignore)
   #define LAST_I3IGNORE(ch)       ((ch)->player_specials->i3last_ignore)
   #define WIZINVIS(ch)			(GET_INVIS_LEV(ch) >= this_mud->minlevel)
   #define I3INVIS(ch)           ((ch)->player_specials->i3invis)
   #define I3ISINVIS(ch)		(WIZINVIS((ch)) || I3INVIS((ch)))
   #define I3NOTELL(ch)	 	(PRF_FLAGGED((ch), PRF_NOTELL))
   #define CH_AFK(ch)		FALSE
   #define CH_PK(ch)			FALSE
#endif
