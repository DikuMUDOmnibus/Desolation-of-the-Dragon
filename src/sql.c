/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*static char rcsid[] = "$Id: sql.c,v 1.13 2003/09/09 19:43:24 dotd Exp $";*/

#if defined(START_DB) || defined(USE_DB)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef USE_DB
#include "mysql.h"
#endif
#include "mud.h"
#include "gsn.h"

#define DB_SERVER_NAME          "localhost"
#define DB_USER_NAME            "dotd"
#define DB_USER_PASSWORD        ""
#define DB_NAME                 "dotd"

#define MOB_TABLE_NAME		"mobtype"
#define MOB_TABLE_ROWS          52

#define OBJ_TABLE_NAME		"objtype"
#define OBJ_TABLE_ROWS          24

#define ROOM_TABLE_NAME		"roomtype"
#define ROOM_TABLE_ROWS         12

#define SHOP_TABLE_NAME         "shoptype"
#define SHOP_TABLE_ROWS         11

#define RSHOP_TABLE_NAME        "rshoptype"
#define RSHOP_TABLE_ROWS        9

#define SPEC_TABLE_NAME         "spectype"
#define SPEC_TABLE_ROWS         4

#define CLIM_TABLE_NAME         "climtype"
#define CLIM_TABLE_ROWS         4

#define DESC_TABLE_NAME         "desctype"
#define DESC_TABLE_ROWS         5

#define EXIT_TABLE_NAME         "exittype"
#define EXIT_TABLE_ROWS         9

#define NEIGH_TABLE_NAME        "neightype"
#define NEIGH_TABLE_ROWS        2

#define OBJAFF_TABLE_NAME       "afftype"
#define OBJAFF_TABLE_ROWS       4

#define PROG_TABLE_NAME         "progtype"
#define PROG_TABLE_ROWS         6

#define RESET_TABLE_NAME        "resettype"
#define RESET_TABLE_ROWS        6

#define AREA_TABLE_NAME         "areatype"
#define AREA_TABLE_ROWS         14

#define HELP_TABLE_NAME         "helptype"
#define HELP_TABLE_ROWS         3

#define COMMAND_TABLE_NAME      "commandtype"
#define COMMAND_TABLE_ROWS      5

#define SOCIAL_TABLE_NAME       "socialtype"
#define SOCIAL_TABLE_ROWS       9

#define SKILL_TABLE_NAME        "skilltype"
#define SKILL_TABLE_ROWS        35
#define SKILLAFF_TABLE_ROWS     5

#define HERB_TABLE_NAME         "herbtype"
#define HERB_TABLE_ROWS         SKILL_TABLE_ROWS


MYSQL mysql;
MYSQL_RES *res, *ares, *bres;
MYSQL_ROW row, arow, brow;
MYSQL_FIELD *field;

extern bool fBootDb;

extern int			top_affect;
extern int			top_area;
extern int			top_ed;
extern int			top_exit;
extern int			top_help;
extern int			top_mob_index;
extern int			top_obj_index;
extern int			top_reset;
extern int			top_room;
extern int			top_shop;
extern int			top_repair;
extern int			top_vroom;
extern char *			help_greeting;

extern MOB_INDEX_DATA *	mob_index_hash		[MAX_KEY_HASH];
extern OBJ_INDEX_DATA *	obj_index_hash		[MAX_KEY_HASH];
extern ROOM_INDEX_DATA * room_index_hash      	[MAX_KEY_HASH];

void shutdown_mud( char *reason );
int  mprog_name_to_type	        args( ( char* name ) );
void renumber_put_resets	args( ( AREA_DATA *pArea ) );

int db_state=1000;

void db_stop();
void db_start();

#define escape_string(to,from) \
    do {\
    if (from) {\
    mysql_escape_string((to),(from),strlen((from))); \
    } else { \
    to[0]='\0'; \
    } \
    } while(0)

void exiterr(int exitcode, char *file, int line)
{
    log_printf_plus(LOG_BUG, LEVEL_LOG_CSET, SEV_ERR, "%d at %s:%d: %s", exitcode, file, line, mysql_error(&mysql));
    if (exitcode>1)
        db_stop();
    db_state = exitcode;
}

void db_start()
{
    if (db_state)
        db_state=0;
    
#if 1
    if (!(mysql_connect(&mysql,DB_SERVER_NAME,DB_USER_NAME,DB_USER_PASSWORD)))
    {
        exiterr(1,__FILE__,__LINE__);
        return;
    }
    
    if (mysql_select_db(&mysql,DB_NAME))
    {
        exiterr(2,__FILE__,__LINE__);
        return;
    }
#else
    if (!(mysql_init(&mysql)))
    {
        exiterr(1,__FILE__,__LINE__);
        return;
    }
    
    if (!(mysql_real_connect(&mysql,
                             DB_SERVER_NAME,
                             DB_USER_NAME,
                             DB_USER_PASSWORD,
                             DB_NAME,
                             0,
                             NULL,
                             0)))
    {
        exiterr(2,__FILE__,__LINE__);
        return;
    }
#endif
}

void db_stop()
{
    if (db_state<=1)
        return;
    
    mysql_close(&mysql);
}

void lock_tables(char *tablenames)
{
    char buf[MAX_INPUT_LENGTH];
    
    sprintf(buf,"lock tables %s", tablenames);
    
    if (mysql_query(&mysql,buf))
    {
        exiterr(3,__FILE__,__LINE__);
        return;
    }
}

void unlock_tables()
{
    if (mysql_query(&mysql,"unlock tables"))
    {
        exiterr(3,__FILE__,__LINE__);
        return;
    }
}

int aname_query(char *table, char *name, int fields)
{
    char buf[MAX_STRING_LENGTH];
    char aname[MAX_INPUT_LENGTH];
    
    if (db_state)
        return 0;
    
    escape_string(aname,name);
    sprintf(buf, "select * from %s where aname='%s'",
            table, aname);
    
    if (mysql_query(&mysql,buf))
    {
        exiterr(3,__FILE__,__LINE__);
        return 0;
    }
    
    if (!(res = mysql_store_result(&mysql)))
    {
        exiterr(4,__FILE__,__LINE__);
        return 0;
    }
    
    if (mysql_num_fields(res)!=fields)
    {
        log_printf("%s: expected %d fields got %d",
                   table, fields, mysql_num_fields(res) );
        return FALSE;
    }
    
    return 1;
}

int db_generic_find(char *sql)
{
    int x;
    
    if (db_state)
        return 0;
    
    if (mysql_query(&mysql,sql))
    {
        exiterr(5,__FILE__,__LINE__);
        return 0;
    }
    
    if (!(res=mysql_store_result(&mysql)))
    {
        exiterr(6,__FILE__,__LINE__);
        return 0;
    }
    
    x=mysql_num_rows(res);
    
    mysql_free_result(res);
    
    return x;
}

int db_generic_insdel(char *sql, char *file, int line)
{
    int x;
    
    if (db_state)
        return 0;
    
    if (mysql_query(&mysql,sql))
    {
        exiterr(7,file,line);
        return 0;
    }
    
    x=mysql_affected_rows(&mysql);
    
    return x;
}

int db_find_vnum(char *table_name, int vnum)
{
    char buf[MAX_INPUT_LENGTH];
    
    sprintf(buf, "SELECT vnum FROM %s where vnum=%d", table_name, vnum);
    
    return db_generic_find(buf);
}

int db_del_vnum(char *table_name, int vnum)
{
    char buf[MAX_STRING_LENGTH];
    
    sprintf(buf, "delete from %s where vnum=%d",
            table_name, vnum);
    
    return db_generic_insdel(buf,__FILE__,__LINE__);
}

int db_insert_mob(int del, MOB_INDEX_DATA *mob)
{
    char buf[MAX_STRING_LENGTH*8];
    char player_name[MAX_INPUT_LENGTH], short_descr[MAX_INPUT_LENGTH], \
    long_descr[MAX_INPUT_LENGTH], description[MAX_STRING_LENGTH*4], \
    aname[MAX_INPUT_LENGTH];
    
    if (!mob || !mob->area)
    {
        log_string("db_insert_mob: !mob || !mob->area");
        return 0;
    }
    
    if (del)
        db_del_vnum(MOB_TABLE_NAME,mob->vnum);
    
    escape_string(player_name,mob->player_name);
    escape_string(short_descr,mob->short_descr);
    escape_string(long_descr,mob->long_descr);
    escape_string(description,mob->description);
    escape_string(aname,mob->area->name);
    
    sprintf(buf, "insert into %s values('%s',%d,'%s','%s','%s','%s',"
            "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
            "%d,%d,%d,%d,%d,%d,%d,"
            "%d,%d,%d,%d,%d,"
            "%d,%d,%d,%d,%d,%d,%d,"
            "%d,%d,%d,%d,%d,%d,%d,%d,"
            "%d,%d)",
            MOB_TABLE_NAME,
            aname, mob->vnum, player_name, short_descr, long_descr, description,
            mob->act, mob->affected_by, mob->alignment,
            mob->levels[MIFirstActive(mob)], mob->mobthac0, mob->ac, mob->hitnodice,
            mob->hitsizedice, mob->hitplus, mob->damnodice, mob->damsizedice,
            mob->damplus, GET_MONEY(mob,DEFAULT_CURR), mob->exp, mob->position, mob->defposition,
            mob->sex, mob->perm_str, mob->perm_int, mob->perm_wis, mob->perm_dex,
            mob->perm_con, mob->perm_cha, mob->perm_lck,
            mob->saving_poison_death, mob->saving_wand, mob->saving_para_petri,
            mob->saving_breath, mob->saving_spell_staff, mob->race,
            MIFirstActive(mob), mob->height, mob->weight, mob->speaks,
            mob->speaking, mob->numattacks, mob->hitroll, mob->damroll, mob->xflags,
            mob->resistant, mob->immune, mob->susceptible, mob->attacks,
            mob->defenses, mob->act2, mob->affected_by2);
    
    return db_generic_insdel(buf,__FILE__,__LINE__);
}

void row_to_mob(MOB_INDEX_DATA *mob)
{
    mob->vnum		 	= atoi(row[1]);
    mob->player_name		= STRALLOC(row[2]);
    mob->short_descr		= STRALLOC(row[3]);
    mob->long_descr		= STRALLOC(row[4]);
    mob->description		= STRALLOC(row[5]);
    mob->act                    = atoi(row[6]);
    SET_BIT(mob->act, ACT_IS_NPC);
    mob->affected_by            = atoi(row[7]);
    mob->alignment              = atoi(row[8]);
    mob->levels[atoi(row[36])]  = atoi(row[9]);
    mob->mobthac0               = atoi(row[10]);
    mob->ac                     = atoi(row[11]);
    mob->hitnodice              = atoi(row[12]);
    mob->hitsizedice            = atoi(row[13]);
    mob->hitplus                = atoi(row[14]);
    mob->damnodice              = atoi(row[15]);
    mob->damsizedice            = atoi(row[16]);
    mob->damplus                = atoi(row[17]);
    GET_MONEY(mob,DEFAULT_CURR) = atoi(row[18]);
    mob->exp                    = atoi(row[19]);
    mob->position               = atoi(row[20]);
    mob->defposition            = atoi(row[21]);
    mob->sex                    = atoi(row[22]);
    mob->perm_str               = atoi(row[23]);
    mob->perm_int               = atoi(row[24]);
    mob->perm_wis               = atoi(row[25]);
    mob->perm_dex               = atoi(row[26]);
    mob->perm_con               = atoi(row[27]);
    mob->perm_cha               = atoi(row[28]);
    mob->perm_lck               = atoi(row[29]);
    mob->saving_poison_death    = atoi(row[30]);
    mob->saving_wand            = atoi(row[31]);
    mob->saving_para_petri      = atoi(row[32]);
    mob->saving_breath          = atoi(row[33]);
    mob->saving_spell_staff     = atoi(row[34]);
    mob->race                   = atoi(row[35]);
    mob->classes[atoi(row[36])] = STAT_ACTCLASS;
    mob->height                 = atoi(row[37]);
    mob->weight                 = atoi(row[38]);
    mob->speaks                 = atoi(row[39]);
    mob->speaking               = atoi(row[40]);
    mob->numattacks             = atoi(row[41]);
    mob->hitroll                = atoi(row[42]);
    mob->damroll                = atoi(row[43]);
    mob->xflags                 = atoi(row[44]);
    mob->resistant              = atoi(row[45]);
    mob->immune                 = atoi(row[46]);
    mob->susceptible            = atoi(row[47]);
    mob->attacks                = atoi(row[48]);
    mob->defenses               = atoi(row[49]);
    mob->act2                   = atoi(row[50]);
    mob->affected_by2           = atoi(row[51]);
    
    mob->long_descr[0]          = UPPER(mob->long_descr[0]);
    mob->description[0]       	= UPPER(mob->description[0]);
    mob->pShop	                = NULL;
    mob->rShop	                = NULL;
    
    if ( !mob->speaks && mob->race>=0)
        mob->speaks = race_table[mob->race].language | LANG_COMMON;
    else
        mob->speaks = LANG_COMMON;
    
    if ( !mob->speaking && mob->race>=0)
        mob->speaking = race_table[mob->race].language;
    else
        mob->speaking = LANG_COMMON;
}

int db_fetch_mob(MOB_INDEX_DATA *mob, int vnum)
{
    char buf[MAX_STRING_LENGTH];
    
    if (!db_find_vnum(MOB_TABLE_NAME,vnum))
        return 0;
    
    sprintf(buf, "select * from %s where vnum=%d",
            MOB_TABLE_NAME, vnum);
    
    if (mysql_query(&mysql,buf))
    {
        exiterr(8,__FILE__,__LINE__);
        return 0;
    }
    
    if (!(res = mysql_store_result(&mysql)))
    {
        exiterr(9,__FILE__,__LINE__);
        return 0;
    }
    
    if (mysql_num_fields(res)!=MOB_TABLE_ROWS)
    {
        log_string("db_fetch_mob: wrong number of fields");
        return 0;
    }
    
    row = mysql_fetch_row(res);
    row_to_mob(mob);
    
    mysql_free_result(res);
    
    return 1;
}

void db_load_mobiles( AREA_DATA *tarea )
{
    MOB_INDEX_DATA *mob;
    char buf[MAX_INPUT_LENGTH];
    
    if ( !tarea )
    {
        bug( "db_load_mobiles: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }
    
    if (!aname_query(MOB_TABLE_NAME,tarea->name,MOB_TABLE_ROWS))
        return;
    
    while ((row = mysql_fetch_row(res)))
    {
        bool tmpBootDb = FALSE;
        bool oldmob = FALSE;
        int vnum;
        int iHash;
        
        vnum = atoi(row[1]);
        tmpBootDb = fBootDb;
        fBootDb = FALSE;
        if ( mob_exists_index( vnum ) )
        {
            if ( tmpBootDb )
            {
                bug( "db_load_mobiles: vnum %d duplicated.", vnum );
                shutdown_mud( "duplicate vnum" );
                exit( 1 );
            }
            else
            {
                mob = get_mob_index( vnum );
                sprintf( buf, "Cleaning mobile: %d", vnum );
                log_string_plus( buf, LOG_BUILD, sysdata.log_level, SEV_DEBUG );
                clean_mob( mob );
                oldmob = TRUE;
            }
        }
        else
        {
            oldmob = FALSE;
            CREATE( mob, MOB_INDEX_DATA, 1 );
        }
        fBootDb = tmpBootDb;
        
        if ( fBootDb )
        {
            if ( vnum < tarea->low_m_vnum )
                tarea->low_m_vnum	= vnum;
            if ( vnum > tarea->hi_m_vnum )
                tarea->hi_m_vnum	= vnum;
        }
        
        row_to_mob(mob);
        mob->area = tarea;
        
        if ( !oldmob )
        {
            iHash			= vnum % MAX_KEY_HASH;
            mob->next	                = mob_index_hash[iHash];
            mob_index_hash[iHash]	= mob;
            top_mob_index++;
        }
    }
    
    mysql_free_result(res);
}

int db_insert_obj(int del, OBJ_INDEX_DATA *obj)
{
    char buf[MAX_STRING_LENGTH*8];
    char name[MAX_INPUT_LENGTH], short_descr[MAX_INPUT_LENGTH], \
    action_desc[MAX_INPUT_LENGTH], description[MAX_STRING_LENGTH*4], \
    aname[MAX_INPUT_LENGTH], spell1[MAX_INPUT_LENGTH], spell2[MAX_INPUT_LENGTH], \
    spell3[MAX_INPUT_LENGTH];
    int val0, val1, val2, val3, val4, val5;
    
    if (!obj || !obj->area)
    {
        log_string("db_insert_obj: !obj || !obj->area");
        return 0;
    }
    
    if (del)
        db_del_vnum(OBJ_TABLE_NAME,obj->vnum);
    
    escape_string(name,obj->name);
    escape_string(short_descr,obj->short_descr);
    escape_string(action_desc,obj->action_desc);
    escape_string(description,obj->description);
    escape_string(aname,obj->area->name);
    
    spell1[0]=spell2[0]=spell3[0]='\0';
    val0 = obj->value[0];
    val1 = obj->value[1];
    val2 = obj->value[2];
    val3 = obj->value[3];
    val4 = obj->value[4];
    val5 = obj->value[5];
    
    switch ( obj->item_type )
    {
    case ITEM_PILL:
    case ITEM_POTION:
    case ITEM_SCROLL:
        if ( IS_VALID_SN(val1) )
            val1 = HAS_SPELL_INDEX;
        sprintf(buf, "%s",
                IS_VALID_SN(obj->value[1])?
                skill_table[obj->value[1]]->name:"NONE");
        escape_string(spell1,buf);
        if ( IS_VALID_SN(val2) )
            val1 = HAS_SPELL_INDEX;
        sprintf(buf, "%s",
                IS_VALID_SN(obj->value[2])?
                skill_table[obj->value[2]]->name:"NONE");
        escape_string(spell2,buf);
        if ( IS_VALID_SN(val3) )
            val1 = HAS_SPELL_INDEX;
        sprintf(buf, "%s",
                IS_VALID_SN(obj->value[3])?
                skill_table[obj->value[3]]->name:"NONE");
        escape_string(spell3,buf);
        break;
    case ITEM_STAFF:
    case ITEM_WAND:
        if ( IS_VALID_SN(val3) )
            val3 = HAS_SPELL_INDEX;
        sprintf(buf, "%s",
                IS_VALID_SN(obj->value[3])?
                skill_table[obj->value[3]]->name:"NONE");
        escape_string(spell1,buf);
        break;
    case ITEM_SALVE:
        if ( IS_VALID_SN(val4) )
            val4 = HAS_SPELL_INDEX;
        sprintf(buf, "%s",
                IS_VALID_SN(obj->value[4])?
                skill_table[obj->value[4]]->name:"NONE");
        escape_string(spell1,buf);
        if ( IS_VALID_SN(val5) )
            val5 = HAS_SPELL_INDEX;
        sprintf(buf, "%s",
                IS_VALID_SN(obj->value[5])?
                skill_table[obj->value[5]]->name:"NONE");
        escape_string(spell2,buf);
        break;
    }
    
    sprintf(buf, "insert into %s values('%s',%d,'%s','%s','%s','%s',%d,%d,%d,%d,"
            "%d,%d,%d,%d,%d,%d,%d,%d,%d,'%s','%s','%s',%d,%d)",
            OBJ_TABLE_NAME,
            aname, obj->vnum, name, short_descr, description, action_desc,
            obj->item_type, obj->extra_flags, obj->wear_flags, obj->layers,
            obj->value[0], obj->value[1], obj->value[2], obj->value[3],
            obj->value[4], obj->value[5], obj->weight, obj->cost, obj->rent,
            spell1, spell2, spell3, obj->extra_flags2, obj->magic_flags);
    
    return db_generic_insdel(buf,__FILE__,__LINE__);
}
void row_to_obj(OBJ_INDEX_DATA *obj)
{
    obj->vnum                   = atoi(row[1]);
    obj->name                   = STRALLOC(row[2]);
    obj->short_descr            = STRALLOC(row[3]);
    obj->description            = STRALLOC(row[4]);
    obj->action_desc            = STRALLOC(row[5]);
    obj->item_type              = atoi(row[6]);
    obj->extra_flags            = atoi(row[7]);
    obj->wear_flags             = atoi(row[8]);
    obj->layers                 = atoi(row[9]);
    obj->value[0]               = atoi(row[10]);
    obj->value[1]               = atoi(row[11]);
    obj->value[2]               = atoi(row[12]);
    obj->value[3]               = atoi(row[13]);
    obj->value[4]               = atoi(row[14]);
    obj->value[5]               = atoi(row[15]);
    obj->weight                 = UMAX(1, atoi(row[16]));
    obj->cost                   = atoi(row[17]);
    obj->rent                   = atoi(row[18]);
    /* 19, 20, 21 spells */
    obj->extra_flags2           = atoi(row[22]);
    obj->magic_flags            = atoi(row[23]);
    
    obj->description[0]         = UPPER(obj->description[0]);
    
    if ( area_version == 1 )
    {
        switch ( obj->item_type )
        {
        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL:
            obj->value[1] = skill_lookup ( row[19] );
            obj->value[2] = skill_lookup ( row[20] );
            obj->value[3] = skill_lookup ( row[21] );
            break;
        case ITEM_STAFF:
        case ITEM_WAND:
            obj->value[3] = skill_lookup ( row[19] );
            break;
        case ITEM_SALVE:
            obj->value[4] = skill_lookup ( row[19] );
            obj->value[5] = skill_lookup ( row[20] );
            break;
        }
    } else if ( area_version == 0 ) {
        switch ( obj->item_type )
        {
        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL:
            obj->value[1] = slot_lookup( obj->value[1] );
            obj->value[2] = slot_lookup( obj->value[2] );
            obj->value[3] = slot_lookup( obj->value[3] );
            break;
            
        case ITEM_STAFF:
        case ITEM_WAND:
            obj->value[3] = slot_lookup( obj->value[3] );
            break;
        case ITEM_SALVE:
            obj->value[4] = slot_lookup( obj->value[4] );
            obj->value[5] = slot_lookup( obj->value[5] );
            break;
        }
    }
}

int db_fetch_obj(OBJ_INDEX_DATA *obj, int vnum)
{
    char buf[MAX_STRING_LENGTH];
    
    if (!db_find_vnum(OBJ_TABLE_NAME,vnum))
        return 0;
    
    sprintf(buf, "select * from %s where vnum=%d",
            OBJ_TABLE_NAME, vnum);
    
    if (mysql_query(&mysql,buf))
    {
        exiterr(10,__FILE__,__LINE__);
        return 0;
    }
    
    if (!(res = mysql_store_result(&mysql)))
    {
        exiterr(11,__FILE__,__LINE__);
        return 0;
    }
    
    if (mysql_num_fields(res)!=OBJ_TABLE_ROWS)
    {
        log_string("db_fetch_obj: wrong number of fields");
        return 0;
    }
    
    row = mysql_fetch_row(res);
    row_to_obj(obj);
    
    mysql_free_result(res);
    
    return 1;
}

void db_load_objects( AREA_DATA *tarea )
{
    OBJ_INDEX_DATA *obj;
    char buf[MAX_STRING_LENGTH];
    
    if ( !tarea )
    {
        bug( "db_load_objects: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }
    
    if (!aname_query(OBJ_TABLE_NAME,tarea->name,OBJ_TABLE_ROWS))
        return;
    
    while ((row = mysql_fetch_row(res)))
    {
        bool tmpBootDb = FALSE;
        bool oldobj = FALSE;
        int vnum;
        int iHash;
        
        vnum = atoi(row[1]);
        tmpBootDb = fBootDb;
        fBootDb = FALSE;
        if ( obj_exists_index( vnum ) )
        {
            if ( tmpBootDb )
            {
                bug( "db_load_objects: vnum %d duplicated.", vnum );
                shutdown_mud( "duplicate vnum" );
                exit( 1 );
            }
            else
            {
                obj = get_obj_index( vnum );
                sprintf( buf, "Cleaning object: %d", vnum );
                log_string_plus( buf, LOG_BUILD, sysdata.log_level, SEV_DEBUG );
                clean_obj( obj );
                oldobj = TRUE;
            }
        }
        else
        {
            oldobj = FALSE;
            CREATE( obj, OBJ_INDEX_DATA, 1 );
        }
        fBootDb = tmpBootDb;
        
        if ( fBootDb )
        {
            if ( vnum < tarea->low_o_vnum )
                tarea->low_o_vnum	= vnum;
            if ( vnum > tarea->hi_o_vnum )
                tarea->hi_o_vnum	= vnum;
        }
        
        row_to_obj(obj);
        obj->area = tarea;
        
        if ( !oldobj )
        {
            iHash			= vnum % MAX_KEY_HASH;
            obj->next	                = obj_index_hash[iHash];
            obj_index_hash[iHash]	= obj;
            top_obj_index++;
        }
    }
    
    mysql_free_result(res);
}

int db_insert_room(int del, ROOM_INDEX_DATA *room)
{
    char buf[MAX_STRING_LENGTH*8];
    char name[MAX_INPUT_LENGTH], description[MAX_STRING_LENGTH*4], \
    aname[MAX_INPUT_LENGTH];
    
    if (!room || !room->area)
    {
        log_string("db_insert_room: !room || !room->area");
        return 0;
    }
    
    if (del)
        db_del_vnum(ROOM_TABLE_NAME,room->vnum);
    
    escape_string(name,room->name);
    escape_string(description,room->description);
    escape_string(aname,room->area->name);
    
    sprintf(buf, "insert into %s values('%s',%d,'%s','%s',%d,%d,%d,%d,%d,%d,%d,%d)",
            ROOM_TABLE_NAME,
            aname, room->vnum, name, description, 0, room->room_flags,
            room->sector_type, room->tele_delay, room->tele_vnum,
            room->tunnel, room->elevation, room->liquid);
    
    return db_generic_insdel(buf,__FILE__,__LINE__);
}

void row_to_room(ROOM_INDEX_DATA *room)
{
    room->vnum                   = atoi(row[1]);
    room->name                   = STRALLOC(row[2]);
    room->description            = STRALLOC(row[3]);
    /* 4, blah */
    room->room_flags             = atoi(row[5]);
    room->sector_type            = atoi(row[6]);
    room->tele_delay             = atoi(row[7]);
    room->tele_vnum              = atoi(row[8]);
    room->tunnel                 = atoi(row[9]);
    room->elevation              = atoi(row[10]);
    room->liquid                 = atoi(row[11]);
    
    if (room->sector_type<0 || room->sector_type==SECT_MAX)
        room->sector_type        = 1;
    room->light                  = 0;
    room->river                  = NULL;
    room->first_extradesc        = NULL;
    room->last_extradesc         = NULL;
    room->first_exit             = NULL;
    room->last_exit              = NULL;
}

int db_fetch_room(ROOM_INDEX_DATA *room, int vnum)
{
    char buf[MAX_STRING_LENGTH];
    
    if (!db_find_vnum(ROOM_TABLE_NAME,vnum))
        return 0;
    
    sprintf(buf, "select * from %s where vnum=%d",
            ROOM_TABLE_NAME, vnum);
    
    if (mysql_query(&mysql,buf))
    {
        exiterr(10,__FILE__,__LINE__);
        return 0;
    }
    
    if (!(res = mysql_store_result(&mysql)))
    {
        exiterr(11,__FILE__,__LINE__);
        return 0;
    }
    
    if (mysql_num_fields(res)!=ROOM_TABLE_ROWS)
    {
        log_string("db_fetch_room: wrong number of fields");
        return 0;
    }
    
    row = mysql_fetch_row(res);
    row_to_room(room);
    
    mysql_free_result(res);
    
    return 1;
}

ROOM_INDEX_DATA *db_get_room_index(int vnum)
{
    ROOM_INDEX_DATA *room;

    CREATE(room, ROOM_INDEX_DATA, 1);

    if (!db_fetch_room(room,vnum))
    {
        DISPOSE(room);
        return NULL;
    }

    return room;
}

void db_load_rooms( AREA_DATA *tarea )
{
    ROOM_INDEX_DATA *room;
    char buf[MAX_STRING_LENGTH];
    
    if ( !tarea )
    {
        bug( "db_load_rooms: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }
    
    if (!aname_query(ROOM_TABLE_NAME,tarea->name,ROOM_TABLE_ROWS))
        return;
    
    while ((row = mysql_fetch_row(res)))
    {
        bool tmpBootDb = FALSE;
        bool oldroom = FALSE;
        int vnum;
        int iHash;
        
        vnum = atoi(row[1]);
        tmpBootDb = fBootDb;
        fBootDb = FALSE;
        if ( room_exists_index( vnum ) )
        {
            if ( tmpBootDb )
            {
                bug( "db_load_rooms: vnum %d duplicated.", vnum );
                shutdown_mud( "duplicate vnum" );
                exit( 1 );
            }
            else
            {
                room = get_room_index( vnum );
                sprintf( buf, "Cleaning room: %d", vnum );
                log_string_plus( buf, LOG_BUILD, sysdata.log_level, SEV_DEBUG );
                clean_room( room );
                oldroom = TRUE;
            }
        }
        else
        {
            oldroom = FALSE;
            CREATE( room, ROOM_INDEX_DATA, 1 );
            room->first_person	= NULL;
            room->last_person	= NULL;
            room->first_content	= NULL;
            room->last_content	= NULL;
        }
        fBootDb = tmpBootDb;
        
        
        if ( fBootDb )
        {
            if ( vnum < tarea->low_r_vnum )
                tarea->low_r_vnum	= vnum;
            if ( vnum > tarea->hi_r_vnum )
                tarea->hi_r_vnum	= vnum;
        }
        
        row_to_room(room);
        room->area = tarea;
        
        if ( !oldroom )
        {
            iHash			= vnum % MAX_KEY_HASH;
            room->next	                = room_index_hash[iHash];
            room_index_hash[iHash]	= room;
            top_room++;
        }
    }
    
    mysql_free_result(res);
}

int db_insert_shop(int del, SHOP_DATA *shop)
{
    MOB_INDEX_DATA *mob;
    char buf[MAX_STRING_LENGTH];
    char aname[MAX_INPUT_LENGTH];
    
    if (!shop || !(mob=get_mob_index(shop->keeper)))
    {
        bug("db_insert_shop: !shop || !shop->keeper");
        return 0;
    }
    
    if (del)
    {
        sprintf(buf,"delete from %s where keeper=%d",
                SHOP_TABLE_NAME, shop->keeper);
        db_generic_insdel(buf,__FILE__,__LINE__);
    }
    
    escape_string(aname,mob->area->name);
    
    sprintf(buf, "insert into %s values('%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d)",
            SHOP_TABLE_NAME,
            aname, shop->keeper, shop->buy_type[0], shop->buy_type[1],
            shop->buy_type[2], shop->buy_type[3], shop->buy_type[4],
            shop->profit_buy, shop->profit_sell, shop->open_hour,
            shop->close_hour);
    
    return db_generic_insdel(buf,__FILE__,__LINE__);
}

void db_load_shops( AREA_DATA *tarea )
{
    SHOP_DATA *pShop;
    
    if ( !tarea )
    {
        bug( "db_load_shops: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }
    
    if (!aname_query(SHOP_TABLE_NAME,tarea->name,SHOP_TABLE_ROWS))
        return;
    
    while ((row = mysql_fetch_row(res)))
    {
        MOB_INDEX_DATA *pMobIndex;
        int i;
        
        CREATE( pShop, SHOP_DATA, 1 );
        pShop->keeper		= atoi(row[1]);
        if ( pShop->keeper == 0 )
        {
            DISPOSE(pShop);
            continue;
        }
        for ( i = 0; i < MAX_TRADE; i++ )
            pShop->buy_type[i]	= atoi(row[2+i]);
        pShop->profit_buy	= atoi(row[7]);
        pShop->profit_sell	= atoi(row[8]);
        pShop->profit_buy	= URANGE( pShop->profit_sell+5, pShop->profit_buy, 1000 );
        pShop->profit_sell	= URANGE( 0, pShop->profit_sell, pShop->profit_buy-5 );
        pShop->open_hour	= atoi(row[9]);
        pShop->close_hour	= atoi(row[10]);;
        
        pMobIndex		= get_mob_index( pShop->keeper );
        pMobIndex->pShop	= pShop;
        
        if ( !first_shop )
            first_shop		= pShop;
        else
            last_shop->next	= pShop;
        pShop->next		= NULL;
        pShop->prev		= last_shop;
        last_shop		= pShop;
        top_shop++;
    }
    
    mysql_free_result(res);
}

int db_insert_repair(int del, REPAIR_DATA *repair)
{
    MOB_INDEX_DATA *mob;
    char buf[MAX_STRING_LENGTH];
    char aname[MAX_INPUT_LENGTH];
    
    if (!repair || !(mob=get_mob_index(repair->keeper)))
    {
        bug("db_insert_repair: !repair || !repair->keeper");
        return 0;
    }
    
    if (del)
    {
        sprintf(buf,"delete from %s where keeper=%d",
                RSHOP_TABLE_NAME, repair->keeper);
        db_generic_insdel(buf,__FILE__,__LINE__);
    }
    
    escape_string(aname,mob->area->name);
    
    sprintf(buf, "insert into %s values('%s',%d,%d,%d,%d,%d,%d,%d,%d)",
            RSHOP_TABLE_NAME,
            aname, repair->keeper, repair->fix_type[0], repair->fix_type[1],
            repair->fix_type[2], repair->profit_fix, repair->shop_type,
            repair->open_hour, repair->close_hour);
    
    return db_generic_insdel(buf,__FILE__,__LINE__);
}

void db_load_repairs( AREA_DATA *tarea )
{
    REPAIR_DATA *rShop;
    
    if ( !tarea )
    {
        bug( "db_load_rshops: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }
    
    if (!aname_query(RSHOP_TABLE_NAME,tarea->name,RSHOP_TABLE_ROWS))
        return;
    
    while ((row = mysql_fetch_row(res)))
    {
        MOB_INDEX_DATA *pMobIndex;
        int i;
        
        CREATE( rShop, REPAIR_DATA, 1 );
        rShop->keeper		= atoi(row[1]);
        if ( rShop->keeper == 0 )
        {
            DISPOSE(rShop);
            continue;
        }
        for ( i = 0; i < MAX_FIX; i++ )
            rShop->fix_type[i]	= atoi(row[2+i]);
        rShop->profit_fix	= atoi(row[5]);
        rShop->shop_type	= atoi(row[6]);
        rShop->open_hour	= atoi(row[7]);
        rShop->close_hour	= atoi(row[8]);;
        
        pMobIndex		= get_mob_index( rShop->keeper );
        pMobIndex->rShop	= rShop;
        
        if ( !first_repair )
            first_repair      	= rShop;
        else
            last_repair->next	= rShop;
        rShop->next		= NULL;
        rShop->prev		= last_repair;
        last_repair		= rShop;
        top_repair++;
    }
    
    mysql_free_result(res);
}

int db_insert_special(int del, AREA_DATA *area, SPEC_FUN *spec, int vnum, char *type)
{
    char buf[MAX_STRING_LENGTH];
    char aname[MAX_INPUT_LENGTH];
    char specname[MAX_INPUT_LENGTH];
    
    if (!area || !spec)
    {
        bug("db_insert_special: !area || !spec");
        return 0;
    }
    
    if (del)
    {
        sprintf(buf,"delete from %s where vnum=%d and type='%s'",
                SPEC_TABLE_NAME, vnum, type);
        db_generic_insdel(buf,__FILE__,__LINE__);
    }
    
    escape_string(aname,area->name);
    if (!str_cmp(type, "MOB"))
        sprintf(buf, "%s", m_lookup_spec(spec));
    else if (!str_cmp(type, "OBJ"))
        sprintf(buf, "%s", o_lookup_spec(spec));
    else if (!str_cmp(type, "ROOM"))
        sprintf(buf, "%s", r_lookup_spec(spec));
    else
    {
        bug("db_insert_special: bad type: %s", type);
        return 0;
    }
    
    escape_string(specname,buf);
    
    sprintf(buf, "insert into %s values('%s',%d,'%s','%s')",
            SPEC_TABLE_NAME,
            aname, vnum, type, specname);
    
    return db_generic_insdel(buf,__FILE__,__LINE__);
}

void db_load_specials( AREA_DATA *tarea )
{
    MOB_INDEX_DATA *pMobIndex;
    OBJ_INDEX_DATA *pObjIndex;
    ROOM_INDEX_DATA *room;
    
    if ( !tarea )
    {
        bug( "db_load_specials: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }
    
    if (!aname_query(SPEC_TABLE_NAME,tarea->name,SPEC_TABLE_ROWS))
        return;
    
    while ((row = mysql_fetch_row(res)))
    {
        if (!str_cmp(row[2],"MOB")) {
            pMobIndex		= get_mob_index(atoi(row[1]));
            pMobIndex->spec_fun	= m_spec_lookup(row[4]);
            if ( pMobIndex->spec_fun == NULL )
                bug( "db_load_specials: 'MOB': vnum %d.", pMobIndex->vnum );
        } else if (!str_cmp(row[2],"OBJ")) {
            pObjIndex		= get_obj_index(atoi(row[1]));
            pObjIndex->spec_fun	= o_spec_lookup(row[4]);
            if ( pObjIndex->spec_fun == NULL )
                bug( "db_load_specials: 'OBJ': vnum %d.", pObjIndex->vnum );
        } else if (!str_cmp(row[2],"ROOM")) {
            room		= get_room_index(atoi(row[1]));
            room->spec_fun	= r_spec_lookup(row[4]);
            if ( room->spec_fun == NULL )
                bug( "db_load_specials: 'ROOM': vnum %d.", room->vnum );
        }
    }
    
    mysql_free_result(res);
}

int db_insert_climate(int del, AREA_DATA *area)
{
    char buf[MAX_STRING_LENGTH];
    char aname[MAX_INPUT_LENGTH];
    
    if (!area)
    {
        bug("db_insert_climate: !area");
        return 0;
    }
    
    escape_string(aname,area->name);
    
    if (del)
    {
        sprintf(buf,"delete from %s where aname='%s'",
                CLIM_TABLE_NAME, aname);
        db_generic_insdel(buf,__FILE__,__LINE__);
    }
    
    sprintf(buf, "insert into %s values('%s',%d,%d,%d)",
            CLIM_TABLE_NAME,
            aname, area->weather->climate_temp,
            area->weather->climate_precip, area->weather->climate_wind);
    
    return db_generic_insdel(buf,__FILE__,__LINE__);
}

void db_load_climate( AREA_DATA *tarea )
{
    if ( !tarea )
    {
        bug( "db_load_climate: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }
    
    if (!aname_query(CLIM_TABLE_NAME,tarea->name,CLIM_TABLE_ROWS))
        return;
    
    row = mysql_fetch_row(res);
    
    CREATE(tarea->weather, WEATHER_DATA, 1);
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
    tarea->weather->climate_temp   = atoi(row[1]);
    tarea->weather->climate_precip = atoi(row[2]);
    tarea->weather->climate_wind   = atoi(row[3]);
    
    mysql_free_result(res);
}

int db_insert_descs(int del, AREA_DATA *area, EXTRA_DESCR_DATA *eds, int vnum, char *type)
{
    EXTRA_DESCR_DATA *ed;
    char buf[MAX_STRING_LENGTH*8];
    char aname[MAX_INPUT_LENGTH], keyword[MAX_INPUT_LENGTH],\
    description[MAX_STRING_LENGTH*4];
    int x=0;
    
    if (!area)
    {
        bug("db_insert_descs: !area");
        return 0;
    }
    
    if (del)
    {
        sprintf(buf,"delete from %s where vnum=%d and type='%s'",
                DESC_TABLE_NAME, vnum, type);
        db_generic_insdel(buf,__FILE__,__LINE__);
    }
    
    for (ed=eds;ed;ed=ed->next)
    {
        escape_string(aname,area->name);
        escape_string(keyword,ed->keyword);
        escape_string(description,ed->description);
        
        sprintf(buf, "insert into %s values('%s',%d,'%s','%s','%s')",
                DESC_TABLE_NAME,
                aname, vnum, type,
                keyword, description);
        x+=db_generic_insdel(buf,__FILE__,__LINE__);
    }
    
    return x;
}

void db_load_descs( AREA_DATA *tarea )
{
    OBJ_INDEX_DATA *pObjIndex;
    ROOM_INDEX_DATA *room;
    
    if ( !tarea )
    {
        bug( "db_load_descs: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }
    
    if (!aname_query(DESC_TABLE_NAME,tarea->name,DESC_TABLE_ROWS))
        return;
    
    while ((row = mysql_fetch_row(res)))
    {
        if (!str_cmp(row[2],"OBJ")) {
            EXTRA_DESCR_DATA *ed;
            
            pObjIndex		= get_obj_index(atoi(row[1]));
            if (!pObjIndex)
            {
                bug("db_load_descs: obj %d not found", atoi(row[1]));
                continue;
            }
            
            CREATE( ed, EXTRA_DESCR_DATA, 1 );
            ed->keyword		= STRALLOC(row[3]);
            ed->description	= STRALLOC(row[4]);
            LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc, next, prev );
            top_ed++;
        } else if (!str_cmp(row[2],"ROOM")) {
            EXTRA_DESCR_DATA *ed;
            
            room		= get_room_index(atoi(row[1]));
            if (!room)
            {
                bug("db_load_descs: room %d not found", atoi(row[1]));
                continue;
            }
            
            CREATE( ed, EXTRA_DESCR_DATA, 1 );
            ed->keyword		= STRALLOC(row[3]);
            ed->description	= STRALLOC(row[4]);
            LINK( ed, room->first_extradesc, room->last_extradesc, next, prev );
            top_ed++;
        }
        
    }
    
    mysql_free_result(res);
}

int db_insert_exit(int del, AREA_DATA *area, EXIT_DATA *ex, int vnum)
{
    char buf[MAX_STRING_LENGTH];
    char aname[MAX_INPUT_LENGTH], keyword[MAX_INPUT_LENGTH],
        description[MAX_INPUT_LENGTH];
        
        if (!area)
        {
            bug("db_insert_climate: !area");
            return 0;
        }
        
        if (del)
        {
            sprintf(buf,"delete from %s where vnum=%d and vdir=%d",
                    EXIT_TABLE_NAME, vnum, ex->vdir);
            db_generic_insdel(buf,__FILE__,__LINE__);
        }
        
        escape_string(aname,area->name);
        escape_string(keyword,ex->keyword);
        escape_string(description,ex->description);
        
        sprintf(buf, "insert into %s values('%s',%d,%d,%d,%d,%d,%d,'%s','%s')",
                EXIT_TABLE_NAME,
                aname, vnum, ex->vdir, ex->exit_info & ~EX_BASHED, ex->key,
                ex->vnum, ex->distance, keyword, description);
        
        return db_generic_insdel(buf,__FILE__,__LINE__);
}

void db_load_exits( AREA_DATA *tarea )
{
    ROOM_INDEX_DATA *room;
    
    if ( !tarea )
    {
        bug( "db_load_exits: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }
    
    if (!aname_query(EXIT_TABLE_NAME,tarea->name,EXIT_TABLE_ROWS))
        return;
    
    while ((row = mysql_fetch_row(res)))
    {
        EXIT_DATA *pexit;
        int locks, door;
        
        room = get_room_index(atoi(row[1]));
        if (!room)
        {
            bug("db_load_exits: room %d not found", atoi(row[1]));
            continue;
        }
        
        door = atoi(row[2]);
        if ( door < 0 || door > 10 )
        {
            bug( "fread_exits: vnum %d has bad door number %d.",
                 room->vnum, door );
            if ( fBootDb )
                exit( 1 );
        }
        else
        {
            pexit = make_exit(room, NULL, door );
            
            pexit->vdir		= door;
            locks	        = atoi(row[3]);
            pexit->key		= atoi(row[4]);
            pexit->vnum		= atoi(row[5]);
            pexit->distance	= atoi(row[6]);
            pexit->keyword	= STRALLOC(row[7]);
            pexit->description	= STRALLOC(row[8]);
            
            pexit->exit_info	= 0;
            switch ( locks )
            {
            case 1:  pexit->exit_info = EX_ISDOOR;                break;
            case 2:  pexit->exit_info = EX_ISDOOR | EX_PICKPROOF; break;
            default: pexit->exit_info = locks;
            }
        }
    }
    
    mysql_free_result(res);
}

int db_insert_neighbors(int del, AREA_DATA *area)
{
    NEIGHBOR_DATA *neigh;
    char buf[MAX_STRING_LENGTH];
    char aname[MAX_INPUT_LENGTH], neighname[MAX_INPUT_LENGTH];
    int x=0;
    
    if (!area)
    {
        bug("db_insert_neighbor: !area");
        return 0;
    }
    
    escape_string(aname,area->name);
    
    if (del)
    {
        sprintf(buf,"delete from %s where aname='%s'",
                NEIGH_TABLE_NAME, aname);
        db_generic_insdel(buf,__FILE__,__LINE__);
    }
    
    for (neigh=area->weather->first_neighbor;neigh;neigh=neigh->next)
    {
        escape_string(neighname,neigh->name);
        sprintf(buf, "insert into %s values('%s','%s')",
                NEIGH_TABLE_NAME,
                aname, neighname);
        x+=db_generic_insdel(buf,__FILE__,__LINE__);
    }
    
    return x;
}

void db_load_neighbors( AREA_DATA *tarea )
{
    if ( !tarea )
    {
        bug( "db_load_neighbor: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }
    
    if (!aname_query(NEIGH_TABLE_NAME,tarea->name,NEIGH_TABLE_ROWS))
        return;
    
    while ((row = mysql_fetch_row(res)))
    {
        NEIGHBOR_DATA *nnew;
        
        CREATE(nnew, NEIGHBOR_DATA, 1);
        
        nnew->next    = NULL;
        nnew->prev    = NULL;
        nnew->address = NULL;
        
        nnew->name    = STRALLOC(row[1]);
        LINK(nnew, tarea->weather->first_neighbor, tarea->weather->last_neighbor, next, prev);
    }
    
    mysql_free_result(res);
}

int db_insert_objaffects(int del, AREA_DATA *area, OBJ_INDEX_DATA *obj)
{
    AFFECT_DATA *aff;
    char buf[MAX_STRING_LENGTH];
    char aname[MAX_INPUT_LENGTH];
    int x=0;
    
    if (!area || !obj)
    {
        bug("db_insert_objaffect: !area || !obj");
        return 0;
    }
    
    escape_string(aname,area->name);
    
    if (del)
    {
        sprintf(buf,"delete from %s where aname='%s' and vnum=%d",
                OBJAFF_TABLE_NAME, aname, obj->vnum);
        db_generic_insdel(buf,__FILE__,__LINE__);
    }
    
    for (aff=obj->first_affect;aff;aff=aff->next)
    {
        sprintf(buf, "insert into %s values('%s',%d,%d,%d)",
                OBJAFF_TABLE_NAME,
                aname, obj->vnum, aff->location,
                ((aff->location == APPLY_WEAPONSPELL
                  || aff->location == APPLY_WEARSPELL
                  || aff->location == APPLY_REMOVESPELL
                  || aff->location == APPLY_EAT_SPELL
                  || aff->location == APPLY_IMMUNESPELL
                  || aff->location == APPLY_STRIPSN)
                 && IS_VALID_SN(aff->modifier))
                ? skill_table[aff->modifier]->slot : aff->modifier );
        x+=db_generic_insdel(buf,__FILE__,__LINE__);
    }
    
    return x;
}

void db_load_objaffects( AREA_DATA *tarea )
{
    OBJ_INDEX_DATA *obj;
    
    if ( !tarea )
    {
        bug( "db_load_affects: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }
    
    if (!aname_query(OBJAFF_TABLE_NAME,tarea->name,OBJAFF_TABLE_ROWS))
        return;
    
    while ((row = mysql_fetch_row(res)))
    {
        AFFECT_DATA *paf;
        
        obj = get_obj_index(atoi(row[1]));
        if (!obj)
        {
            bug("db_load_affects: obj %d not found", atoi(row[1]));
            continue;
        }
        
        CREATE( paf, AFFECT_DATA, 1 );
        paf->type		= -1;
        paf->duration		= -1;
        paf->location		= atoi(row[2]);
        if ( paf->location == APPLY_WEAPONSPELL
             ||   paf->location == APPLY_WEARSPELL
             ||   paf->location == APPLY_REMOVESPELL
             ||   paf->location == APPLY_EAT_SPELL
             ||   paf->location == APPLY_IMMUNESPELL
             ||   paf->location == APPLY_STRIPSN )
            paf->modifier		= slot_lookup(atoi(row[3]));
        else
            paf->modifier		= atoi(row[3]);
        paf->bitvector		= 0;
        if (paf->location == APPLY_STUN ||
            paf->location == APPLY_PUNCH ||
            paf->location == APPLY_CLIMB)
        {
            bug( "db_load_affects: obj %d, apply %d", obj->vnum, paf->location );
        }
        LINK( paf, obj->first_affect, obj->last_affect, next, prev );
        top_affect++;
    }
    
    mysql_free_result(res);
}

int db_insert_progs(int del, AREA_DATA *area, MPROG_DATA *mprogs, int vnum, char *type)
{
    MPROG_DATA *mprog;
    char buf[MAX_STRING_LENGTH*8];
    char aname[MAX_INPUT_LENGTH];
    char mtype[MAX_INPUT_LENGTH], marglist[MAX_INPUT_LENGTH],mcomlist[MAX_STRING_LENGTH*4];
    int x=0;
    
    if (!area)
    {
        bug("db_insert_progs: !area");
        return 0;
    }
    
    if (del)
    {
        sprintf(buf,"delete from %s where vnum=%d and type='%s'",
                PROG_TABLE_NAME, vnum, type);
        db_generic_insdel(buf,__FILE__,__LINE__);
    }
    
    escape_string(aname,area->name);
    
    for (mprog=mprogs; mprog; mprog=mprog->next)
    {
        sprintf(buf, "%s", mprog_type_to_name(mprog->type));
        escape_string(mtype,buf);
        escape_string(marglist,mprog->arglist);
        escape_string(mcomlist,mprog->comlist);
        
        sprintf(buf, "insert into %s values('%s',%d,'%s', '%s', '%s', '%s')",
                PROG_TABLE_NAME,
                aname, vnum, type, mtype, marglist, mcomlist);
        x+=db_generic_insdel(buf,__FILE__,__LINE__);
    }
    
    return x;
}

void db_load_progs( AREA_DATA *tarea )
{
    MPROG_DATA *mprg, *original;
    
    if ( !tarea )
    {
        bug( "db_load_progs: no #AREA seen yet." );
        if ( fBootDb )
        {
            shutdown_mud( "No #AREA" );
            exit( 1 );
        }
        else
            return;
    }
    
    
    
    if (!aname_query(PROG_TABLE_NAME,tarea->name,PROG_TABLE_ROWS))
        return;
    
    while ((row = mysql_fetch_row(res)))
    {
        if (!str_cmp(row[2],"MOB")) {
            MOB_INDEX_DATA *pMobIndex;
            
            pMobIndex		= get_mob_index(atoi(row[1]));
            if ( !pMobIndex )
                bug( "db_load_progs: 'MOB': vnum %d.", pMobIndex->vnum );
            
            CREATE( mprg, MPROG_DATA, 1 );
            if ( (original = pMobIndex->mudprogs) != NULL )
                for ( ; original->next; original = original->next );
            
            if ( original )
                original->next = mprg;
            else
                pMobIndex->mudprogs = mprg;
            
            mprg->type = mprog_name_to_type( row[3] );
            switch ( mprg->type )
            {
            case ERROR_PROG:
                bug( "db_load_progs: vnum %d MOBPROG type.", pMobIndex->vnum );
                exit( 1 );
                break;
            case IN_FILE_PROG:
                bug ("db_load_progs: IN_FILE_PROG for mob %d", pMobIndex->vnum);
                break;
            default:
                xSET_BIT(pMobIndex->progtypes, mprg->type);
                mprg->arglist        = STRALLOC(row[4]);
                mprg->comlist        = STRALLOC(row[5]);
                break;
            }
        } else if (!str_cmp(row[2],"OBJ")) {
            OBJ_INDEX_DATA *pObjIndex;
            
            pObjIndex		= get_obj_index(atoi(row[1]));
            if ( !pObjIndex )
                bug( "db_load_progs: 'OBJ': vnum %d.", pObjIndex->vnum );
            
            CREATE( mprg, MPROG_DATA, 1 );
            if ( (original = pObjIndex->mudprogs) != NULL )
                for ( ; original->next; original = original->next );
            
            if ( original )
                original->next = mprg;
            else
                pObjIndex->mudprogs = mprg;
            
            mprg->type = mprog_name_to_type( row[3] );
            switch ( mprg->type )
            {
            case ERROR_PROG:
                bug( "db_load_progs: vnum %d OBJPROG type.", pObjIndex->vnum );
                exit( 1 );
                break;
            case IN_FILE_PROG:
                bug ("db_load_progs: IN_FILE_PROG for obj %d", pObjIndex->vnum);
                break;
            default:
                xSET_BIT(pObjIndex->progtypes, mprg->type);
                mprg->arglist        = STRALLOC(row[4]);
                mprg->comlist        = STRALLOC(row[5]);
                break;
            }
        } else if (!str_cmp(row[2],"ROOM")) {
            ROOM_INDEX_DATA *room;
            
            room		= get_room_index(atoi(row[1]));
            if ( !room )
                bug( "db_load_progs: 'ROOM': vnum %d.", room->vnum );
            
            CREATE( mprg, MPROG_DATA, 1 );
            if ( (original = room->mudprogs) != NULL )
                for ( ; original->next; original = original->next );
            
            if ( original )
                original->next = mprg;
            else
                room->mudprogs = mprg;
            
            mprg->type = mprog_name_to_type( row[3] );
            switch ( mprg->type )
            {
            case ERROR_PROG:
                bug( "db_load_progs: vnum %d ROOMPROG type.", room->vnum );
                exit( 1 );
                break;
            case IN_FILE_PROG:
                bug ("db_load_progs: IN_FILE_PROG for room %d", room->vnum);
                break;
            default:
                xSET_BIT(room->progtypes, mprg->type);
                mprg->arglist        = STRALLOC(row[4]);
                mprg->comlist        = STRALLOC(row[5]);
                break;
            }
        }
    }
    
    mysql_free_result(res);
}

int db_insert_resets(int del, AREA_DATA *area)
{
    RESET_DATA *treset;
    char buf[MAX_STRING_LENGTH*8];
    char aname[MAX_INPUT_LENGTH];
    int x=0;
    
    if (!area)
    {
        bug("db_insert_resets: !area");
        return 0;
    }
    
    escape_string(aname,area->name);
    
    if (del)
    {
        sprintf(buf,"delete from %s where aname='%s'",
                RESET_TABLE_NAME, aname);
        db_generic_insdel(buf,__FILE__,__LINE__);
    }
    
    for (treset=area->first_reset;treset;treset=treset->next)
    {
        sprintf(buf, "insert into %s values('%s','%c',%d,%d,%d,%d)",
                RESET_TABLE_NAME,
                aname, UPPER(treset->command), treset->extra,
                treset->arg1, treset->arg2, treset->arg3);
        x+=db_generic_insdel(buf,__FILE__,__LINE__);
    }
    
    return x;
}

void db_load_resets(AREA_DATA *tarea)
{
    char buf[MAX_INPUT_LENGTH];
    int count=0;
    bool not01=FALSE;
    
    if ( tarea->first_reset )
    {
        if ( fBootDb )
        {
            RESET_DATA *rtmp;
            
            bug( "db_load_resets: WARNING: resets already exist for this area." );
            for ( rtmp = tarea->first_reset; rtmp; rtmp = rtmp->next )
                ++count;
        }
        else
        {
            sprintf( buf, "Cleaning resets: %s", tarea->name );
            log_string_plus( buf, LOG_BUILD, sysdata.log_level, SEV_DEBUG );
            clean_resets( tarea );
        }	
    }
    
    if (!aname_query(RESET_TABLE_NAME,tarea->name,RESET_TABLE_ROWS))
        return;
    
    while ((row = mysql_fetch_row(res)))
    {
        ROOM_INDEX_DATA *pRoomIndex;
        EXIT_DATA *pexit;
        char letter;
        int extra, arg1, arg2, arg3;
        
        letter  = row[1][0];
        
        extra   = atoi(row[2]);
        arg1    = atoi(row[3]);
        arg2    = atoi(row[4]);
        arg3    = atoi(row[5]);
        
        ++count;
        
        switch ( letter )
        {
        default:
            bug( "db_load_resets: bad command '%c'.", letter );
            if ( fBootDb )
                bug( "db_load_resets: %24.24s (%5.5d) bad command '%c'.", tarea->name, count, letter );
            return;
            
        case 'M':
            if ( !mob_exists_index( arg1 ) && fBootDb )
                bug( "db_load_resets: %24.24s (%5.5d) 'M': mobile %d doesn't exist.",
                     tarea->name, count, arg1 );
            if ( !room_exists_index( arg3 ) && fBootDb )
                bug( "db_load_resets: %24.24s (%5.5d) 'M': room %d doesn't exist.",
                     tarea->name, count, arg3 );
            break;
            
        case 'O':
            if ( !obj_exists_index(arg1) && fBootDb )
                bug( "db_load_resets: %24.24s (%5.5d) '%c': object %d doesn't exist.",
                     tarea->name, count, letter, arg1 );
            if ( !room_exists_index(arg3) && fBootDb )
                bug( "db_load_resets: %24.24s (%5.5d) '%c': room %d doesn't exist.",
                     tarea->name, count, letter, arg3 );
            break;
            
        case 'P':
            if ( !obj_exists_index(arg1) && fBootDb )
                bug( "db_load_resets: %24.24s (%5.5d) '%c': object %d doesn't exist.",
                     tarea->name, count, letter, arg1 );
            if ( arg3 > 0 )
            {
                if ( obj_exists_index(arg3) && fBootDb )
                    bug( "db_load_resets: %24.24s (%5.5d) 'P': destination object %d doesn't exist.",
                         tarea->name, count, arg3 );
            }
            else if ( extra > 1 )
                not01 = TRUE;
            break;
            
        case 'G':
        case 'E':
            if ( !obj_exists_index(arg1) && fBootDb )
                bug( "db_load_resets: %24.24s (%5.5d) '%c': object %d doesn't exist.",
                     tarea->name, count, letter, arg1 );
            break;
            
        case 'T':
            break;
            
        case 'H':
            if ( arg1 > 0 )
                if ( !obj_exists_index(arg1) && fBootDb )
                    bug( "db_load_resets: %24.24s (%5.5d) 'H': object %d doesn't exist.",
                         tarea->name, count, arg1 );
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
                        bug( "db_load_resets: 'B': room %d doesn't exist.", arg1 );
                        bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
                             arg3 );
                        if ( fBootDb )
                            bug( "db_load_resets: %24.24s (%5.5d) 'B': room %d doesn't exist.",
                                 tarea->name, count, arg1 );
                    }
                    
                    door = (arg2 & BIT_RESET_DOOR_MASK) >> BIT_RESET_DOOR_THRESHOLD;
                    
                    if ( !(pexit = get_exit(pRoomIndex, door)) )
                    {
                        bug( "db_load_resets: 'B': exit %d not door.", door );
                        bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
                             arg3 );
                        if ( fBootDb )
                            bug( "db_load_resets: %24.24s (%5.5d) 'B': exit %d not door.",
                                 tarea->name, count, door );
                    }
                }
                break;
            case BIT_RESET_ROOM:
                if (!room_exists_index(arg1))
                {
                    bug( "db_load_resets: 'B': room %d doesn't exist.", arg1);
                    bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
                         arg3 );
                    if ( fBootDb )
                        bug( "db_load_resets: %24.24s (%5.5d) 'B': room %d doesn't exist.",
                             tarea->name, count, arg1 );
                }
                break;
            case BIT_RESET_OBJECT:
                if (arg1 > 0)
                    if (!obj_exists_index(arg1) && fBootDb)
                        bug("db_load_resets: %24.24s (%5.5d) 'B': object %d doesn't exist.",
                            tarea->name, count, arg1 );
                break;
            case BIT_RESET_MOBILE:
                if (arg1 > 0)
                    if (!mob_exists_index(arg1) && fBootDb)
                        bug("db_load_resets: %24.24s (%5.5d) 'B': mobile %d doesn't exist.",
                            tarea->name, count, arg1 );
                break;
            default:
                bug( "db_load_resets: %24.24s (%5.5d) 'B': bad type flag (%d).",
                     tarea->name, count, arg2 & BIT_RESET_TYPE_MASK );
                break;
            }
            break;
            
        case 'D':
            pRoomIndex = get_room_index( arg1 );
            if ( !pRoomIndex )
            {
                bug( "db_load_resets: 'D': room %d doesn't exist.", arg1 );
                bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
                     arg3 );
                if ( fBootDb )
                    bug( "db_load_resets: %24.24s (%5.5d) 'D': room %d doesn't exist.",
                         tarea->name, count, arg1 );
                break;
            }
            
            if ( arg2 < 0
                 ||   arg2 >= MAX_REXITS
                 || ( pexit = get_exit(pRoomIndex, arg2)) == NULL
                 || !IS_SET( pexit->exit_info, EX_ISDOOR ) )
            {
                bug( "db_load_resets: 'D': exit %d not door.", arg2 );
                bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
                     arg3 );
                if ( fBootDb )
                    bug( "db_load_resets: %24.24s (%5.5d) 'D': exit %d not door.",
                         tarea->name, count, arg2 );
            }
            
            if ( arg3 < 0 || arg3 > 2 )
            {
                bug( "db_load_resets: 'D': bad 'locks': %d.", arg3 );
                if ( fBootDb )
                    bug( "db_load_resets: %24.24s (%5.5d) 'D': bad 'locks': %d.",
                         tarea->name, count, arg3 );
            }
            break;
            
        case 'R':
            pRoomIndex = get_room_index( arg1 );
            if ( !pRoomIndex && fBootDb )
                bug( "db_load_resets: %24.24s (%5.5d) 'R': room %d doesn't exist.",
                     tarea->name, count, arg1 );
            
            if ( arg2 < 0 || arg2 > 6 )
            {
                bug( "db_load_resets: 'R': bad exit %d.", arg2 );
                if ( fBootDb )
                    bug( "db_load_resets: %24.24s (%5.5d) 'R': bad exit %d.",
                         tarea->name, count, arg2 );
                break;
            }
            
            break;
        }
        
        add_reset( tarea, letter, extra, arg1, arg2, arg3 );
    }
    if ( !not01 )
        renumber_put_resets(tarea);
    
    mysql_free_result(res);
}

int db_insert_area(int del, AREA_DATA *area)
{
    char buf[MAX_STRING_LENGTH*8];
    char aname[MAX_INPUT_LENGTH], author[MAX_INPUT_LENGTH],\
    resetmsg[MAX_INPUT_LENGTH], filename[MAX_INPUT_LENGTH];
    
    if (!area)
    {
        bug("db_insert_area: !area");
        return 0;
    }
    
    escape_string(aname,area->name);
    
    if (del)
    {
        sprintf(buf,"delete from %s where aname='%s'",
                AREA_TABLE_NAME, aname);
        db_generic_insdel(buf,__FILE__,__LINE__);
    }
    
    escape_string(author,area->author);
    escape_string(filename,area->filename);
    if (area->resetmsg)
        escape_string(resetmsg,area->resetmsg);
    else
        resetmsg[0]='\0';
    
    sprintf(buf, "insert into %s values('%s',%d,'%s',%d,%d,%d,%d,'%s',%d,%d,%d,%d,%d,'%s')",
            AREA_TABLE_NAME,
            aname, AREA_VERSION_WRITE, author,
            area->low_soft_range, area->hi_soft_range,
            area->low_hard_range, area->hi_hard_range,
            resetmsg, area->flags, area->reset_frequency,
            area->high_economy[DEFAULT_CURR], area->low_economy[DEFAULT_CURR],
            area->low_r_vnum, filename);
    
    return db_generic_insdel(buf,__FILE__,__LINE__);
}

void db_load_area( )
{
    AREA_DATA *tarea;
    
    CREATE(tarea,AREA_DATA,1);
    tarea->name           = str_dup(arow[0]);
    if ( tarea->author )
        STRFREE( tarea->author );
    tarea->author         = STRALLOC(arow[2]);
    tarea->low_soft_range = atoi(arow[3]);
    tarea->hi_soft_range  = atoi(arow[4]);
    tarea->low_hard_range = atoi(arow[5]);
    tarea->hi_hard_range  = atoi(arow[6]);
    if ( tarea->resetmsg )
        DISPOSE( tarea->resetmsg );
    tarea->resetmsg       = str_dup(arow[7]);
    tarea->flags          = atoi(arow[8]);
    if (IS_SET(tarea->flags, AFLAG_MODIFIED))
        REMOVE_BIT(tarea->flags, AFLAG_MODIFIED);
    if (IS_SET(tarea->flags, AFLAG_RESET_BOOT))
        SET_BIT(tarea->flags, AFLAG_INITIALIZED);
    else if (IS_SET(tarea->flags, AFLAG_INITIALIZED))
        REMOVE_BIT(tarea->flags, AFLAG_INITIALIZED);
    tarea->reset_frequency= atoi(arow[9]);
    tarea->high_economy[DEFAULT_CURR]   = atoi(arow[10]);
    tarea->low_economy[DEFAULT_CURR]    = atoi(arow[11]);
    tarea->filename       = str_dup(arow[13]);
    tarea->age	          = tarea->reset_frequency;
    tarea->nplayer        = 0;
    tarea->low_r_vnum     = 2000000000;
    tarea->low_o_vnum     = 2000000000;
    tarea->low_m_vnum     = 2000000000;
    tarea->hi_r_vnum      = 0;
    tarea->hi_o_vnum      = 0;
    tarea->hi_m_vnum      = 0;
    tarea->first_reset	  = NULL;
    tarea->last_reset     = NULL;
    
    area_version = atoi(arow[1]);
    
    LINK( tarea, first_area, last_area, next, prev );
    top_area++;
    
    db_load_objects(tarea);
    db_load_mobiles(tarea);
    db_load_rooms(tarea);
    db_load_exits(tarea);
    db_load_objaffects(tarea);
    db_load_progs(tarea);
    db_load_specials(tarea);
    db_load_climate(tarea);
    db_load_neighbors(tarea);
    db_load_shops(tarea);
    db_load_repairs(tarea);
    db_load_descs(tarea);
    db_load_resets(tarea);
    
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
    
    /*
     escape_string(aname,tarea->name);
     sprintf(buf, "update areatype set start_range=%d where aname='%s'",
     tarea->low_r_vnum, aname);
     if (mysql_query(&mysql,buf))
     {
     exiterr(12,__FILE__,__LINE__);
     return;
     }
     */
    
    fprintf( stderr, "%-24.24s: R: %5d - %-5d O: %5d - %-5d M: %5d - %d\n",
             tarea->name,
             tarea->low_r_vnum, tarea->hi_r_vnum,
             tarea->low_o_vnum, tarea->hi_o_vnum,
             tarea->low_m_vnum, tarea->hi_m_vnum );
    if (tarea->low_r_vnum < 0 || tarea->hi_r_vnum < 0)
        fprintf( stderr, "%-24s: Bad Room Range\n", tarea->filename);
    if (tarea->low_m_vnum < 0 || tarea->hi_m_vnum < 0)
        fprintf( stderr, "%-24s: Bad Mob Range\n", tarea->filename);
    if (tarea->low_o_vnum < 0 || tarea->hi_o_vnum < 0)
        fprintf( stderr, "%-24s: Bad Obj Range\n", tarea->filename);
    SET_BIT( tarea->status, AREA_LOADED );
}

void db_load_areas()
{
    char buf[MAX_STRING_LENGTH];
    
    sprintf(buf, "select * from %s order by start_range",
            AREA_TABLE_NAME);
    
    if (mysql_query(&mysql,buf))
    {
        exiterr(13,__FILE__,__LINE__);
        return;
    }
    
    if (!(ares = mysql_store_result(&mysql)))
    {
        exiterr(14,__FILE__,__LINE__);
        return;
    }
    
    if (mysql_num_fields(ares)!=AREA_TABLE_ROWS)
    {
        log_printf("%s: expected %d fields got %d",
                   AREA_TABLE_NAME, AREA_TABLE_ROWS, mysql_num_fields(ares) );
        return;
    }
    
    lock_tables(OBJ_TABLE_NAME " read, " MOB_TABLE_NAME " read, " ROOM_TABLE_NAME " read, " \
                EXIT_TABLE_NAME " read, " OBJAFF_TABLE_NAME " read, " PROG_TABLE_NAME " read, " \
                SPEC_TABLE_NAME " read, " CLIM_TABLE_NAME " read, " NEIGH_TABLE_NAME " read, " \
                SHOP_TABLE_NAME " read, " RSHOP_TABLE_NAME " read, " DESC_TABLE_NAME " read, " \
                RESET_TABLE_NAME " read");
    
    while ((arow = mysql_fetch_row(ares)))
    {
        db_load_area();
    }
    
    unlock_tables();
    
    mysql_free_result(ares);
}

void db_del_area(AREA_DATA *area)
{
    char aname[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
    
    lock_tables(OBJ_TABLE_NAME " write, " MOB_TABLE_NAME " write, " ROOM_TABLE_NAME " write, " \
                EXIT_TABLE_NAME " write, " OBJAFF_TABLE_NAME " write, " PROG_TABLE_NAME " write, " \
                SPEC_TABLE_NAME " write, " CLIM_TABLE_NAME " write, " NEIGH_TABLE_NAME " write, " \
                SHOP_TABLE_NAME " write, " RSHOP_TABLE_NAME " write, " DESC_TABLE_NAME " write, " \
                RESET_TABLE_NAME " write, " AREA_TABLE_NAME " write");
    
    escape_string(aname,area->name);
    
    sprintf(buf,"delete from %s where aname='%s'",
            MOB_TABLE_NAME, aname);
    db_generic_insdel(buf,__FILE__,__LINE__);
    sprintf(buf,"delete from %s where aname='%s'",
            OBJ_TABLE_NAME, aname);
    db_generic_insdel(buf,__FILE__,__LINE__);
    sprintf(buf,"delete from %s where aname='%s'",
            ROOM_TABLE_NAME, aname);
    db_generic_insdel(buf,__FILE__,__LINE__);
    sprintf(buf,"delete from %s where aname='%s'",
            SHOP_TABLE_NAME, aname);
    db_generic_insdel(buf,__FILE__,__LINE__);
    sprintf(buf,"delete from %s where aname='%s'",
            RSHOP_TABLE_NAME, aname);
    db_generic_insdel(buf,__FILE__,__LINE__);
    sprintf(buf,"delete from %s where aname='%s'",
            SPEC_TABLE_NAME, aname);
    db_generic_insdel(buf,__FILE__,__LINE__);
    sprintf(buf,"delete from %s where aname='%s'",
            CLIM_TABLE_NAME, aname);
    db_generic_insdel(buf,__FILE__,__LINE__);
    sprintf(buf,"delete from %s where aname='%s'",
            DESC_TABLE_NAME, aname);
    db_generic_insdel(buf,__FILE__,__LINE__);
    sprintf(buf,"delete from %s where aname='%s'",
            EXIT_TABLE_NAME, aname);
    db_generic_insdel(buf,__FILE__,__LINE__);
    sprintf(buf,"delete from %s where aname='%s'",
            NEIGH_TABLE_NAME, aname);
    db_generic_insdel(buf,__FILE__,__LINE__);
    sprintf(buf,"delete from %s where aname='%s'",
            OBJAFF_TABLE_NAME, aname);
    db_generic_insdel(buf,__FILE__,__LINE__);
    sprintf(buf,"delete from %s where aname='%s'",
            PROG_TABLE_NAME, aname);
    db_generic_insdel(buf,__FILE__,__LINE__);
    sprintf(buf,"delete from %s where aname='%s'",
            RESET_TABLE_NAME, aname);
    db_generic_insdel(buf,__FILE__,__LINE__);
    sprintf(buf,"delete from %s where aname='%s'",
            AREA_TABLE_NAME, aname);
    db_generic_insdel(buf,__FILE__,__LINE__);
    
    unlock_tables();
    
}

int db_insert_helps()
{
    HELP_DATA *help;
    char buf[MAX_STRING_LENGTH*8];
    char keywords[MAX_INPUT_LENGTH], htext[MAX_STRING_LENGTH*4];
    int x=0;
    
    sprintf(buf,"delete from %s",
            HELP_TABLE_NAME);
    db_generic_insdel(buf,__FILE__,__LINE__);
    
    for (help=first_help;help;help=help->next)
    {
        escape_string(keywords,help->keyword);
        escape_string(htext,help->text);
        
        sprintf(buf, "insert into %s values(%d,'%s','%s')",
                HELP_TABLE_NAME,
                help->level, keywords, htext);
        
        x+=db_generic_insdel(buf,__FILE__,__LINE__);
    }
    
    return x;
}

void db_load_helps()
{
    HELP_DATA *pHelp;
    char buf[MAX_STRING_LENGTH];
    
    log_string("Loading helps...");
    sprintf(buf, "select * from %s",
            HELP_TABLE_NAME);
    
    if (mysql_query(&mysql,buf))
    {
        exiterr(15,__FILE__,__LINE__);
        return;
    }
    
    if (!(res = mysql_store_result(&mysql)))
    {
        exiterr(16,__FILE__,__LINE__);
        return;
    }
    
    if (mysql_num_fields(res)!=HELP_TABLE_ROWS)
    {
        log_printf("%s: expected %d fields got %d",
                   HELP_TABLE_NAME, HELP_TABLE_ROWS, mysql_num_fields(res));
        return;
    }
    
    while ((row = mysql_fetch_row(res)))
    {
        CREATE( pHelp, HELP_DATA, 1 );
        pHelp->level	= atoi(row[0]);
        pHelp->keyword	= str_dup(row[1]);
        pHelp->text	= str_dup(row[2]);
        
        if ( !str_cmp( pHelp->keyword, "greeting" ) )
            help_greeting = pHelp->text;
        add_help( pHelp );
    }
    
    mysql_free_result(res);
}

int db_insert_commands()
{
    CMDTYPE *command;
    char buf[MAX_STRING_LENGTH];
    char name[MAX_INPUT_LENGTH],code[MAX_INPUT_LENGTH];
    int x,y=0;
    
    sprintf(buf,"delete from %s",
            COMMAND_TABLE_NAME);
    db_generic_insdel(buf,__FILE__,__LINE__);
    
    for ( x = 0; x < 126; x++ )
    {
        for ( command = command_hash[x]; command; command = command->next )
        {
            if ( !command->name || command->name[0] == '\0' )
            {
                bug( "db_insert_commands: blank command in hash bucket %d", x );
                continue;
            }
            escape_string(name,command->name);
            escape_string(code,skill_name(command->do_fun));
            
            sprintf(buf, "insert into %s values('%s','%s',%d,%d,%d)",
                    COMMAND_TABLE_NAME,
                    name,code,command->position,command->level,command->log);
            
            y+=db_generic_insdel(buf,__FILE__,__LINE__);
        }
    }
    
    return y;
}

void db_load_commands()
{
    CMDTYPE *command;
    char buf[MAX_STRING_LENGTH];
    
    log_string("Loading commands...");
    sprintf(buf, "select * from %s",
            COMMAND_TABLE_NAME);
    
    if (mysql_query(&mysql,buf))
    {
        exiterr(17,__FILE__,__LINE__);
        return;
    }
    
    if (!(res = mysql_store_result(&mysql)))
    {
        exiterr(18,__FILE__,__LINE__);
        return;
    }
    
    if (mysql_num_fields(res)!=COMMAND_TABLE_ROWS)
    {
        log_printf("%s: expected %d fields got %d",
                   COMMAND_TABLE_NAME, COMMAND_TABLE_ROWS, mysql_num_fields(res));
        return;
    }
    
    while ((row = mysql_fetch_row(res)))
    {
        CREATE( command, CMDTYPE, 1 );
        
        command->name     = str_dup(row[0]);
        command->do_fun   = skill_function(row[1]);
        command->position = atoi(row[2]);
        command->level    = atoi(row[3]);
        command->log      = atoi(row[4]);
        
        if ( !command->name )
        {
            bug( "db_load_commands: Name not found", 0 );
            free_command( command );
            continue;
        }
        if ( !command->do_fun )
        {
            bug( "db_load_commands: Function not found", 0 );
            free_command( command );
            continue;
        }
        if ( command->do_fun == skill_notfound )
            command->level = MAX_LEVEL;
        add_command( command );
    }
    
    mysql_free_result(res);
}

int db_insert_socials()
{
    SOCIALTYPE *social;
    char buf[MAX_STRING_LENGTH];
    char name[MAX_INPUT_LENGTH],
        cna[MAX_INPUT_LENGTH],\
        ona[MAX_INPUT_LENGTH],
        cf[MAX_INPUT_LENGTH],\
        of[MAX_INPUT_LENGTH],
        vf[MAX_INPUT_LENGTH],\
        ca[MAX_INPUT_LENGTH],
        oa[MAX_INPUT_LENGTH],\
        nf[MAX_INPUT_LENGTH];
        int x,y=0;
        
        sprintf(buf,"delete from %s",
                SOCIAL_TABLE_NAME);
        db_generic_insdel(buf,__FILE__,__LINE__);
        
        for ( x = 0; x < 27; x++ )
        {
            for ( social = social_index[x]; social; social = social->next )
            {
                if ( !social->name || social->name[0] == '\0' )
                {
                    bug( "db_insert_socials: blank social in hash bucket %d", x );
                    continue;
                }
                escape_string(name,social->name);
                escape_string(cna,social->char_no_arg);
                if ( !social->char_no_arg )
                    bug( "db_isnert_socials: NULL char_no_arg in hash bucket %d", x );
                escape_string(ona,social->others_no_arg);
                escape_string(cf,social->char_found);
                escape_string(of,social->others_found);
                escape_string(vf,social->vict_found);
                escape_string(ca,social->char_auto);
                escape_string(oa,social->others_auto);
                escape_string(nf,social->not_found);
                
                sprintf(buf, "insert into %s values('%s','%s','%s','%s','%s','%s','%s','%s','%s')",
                        SOCIAL_TABLE_NAME,
                        name, cna, ona, cf, of, vf, ca, oa, nf);
                
                y+=db_generic_insdel(buf,__FILE__,__LINE__);
            }
        }
        
        return y;
}

void db_load_socials()
{
    SOCIALTYPE *social;
    char buf[MAX_STRING_LENGTH];
    
    log_string("Loading socials...");
    sprintf(buf, "select * from %s",
            SOCIAL_TABLE_NAME);
    
    if (mysql_query(&mysql,buf))
    {
        exiterr(19,__FILE__,__LINE__);
        return;
    }
    
    if (!(res = mysql_store_result(&mysql)))
    {
        exiterr(20,__FILE__,__LINE__);
        return;
    }
    
    if (mysql_num_fields(res)!=SOCIAL_TABLE_ROWS)
    {
        log_printf("%s: expected %d fields got %d",
                   SOCIAL_TABLE_NAME, SOCIAL_TABLE_ROWS, mysql_num_fields(res));
        return;
    }
    
    while ((row = mysql_fetch_row(res)))
    {
        CREATE( social, SOCIALTYPE, 1 );
        
        social->name          = str_dup(row[0]);
        social->char_no_arg   = str_dup(row[1]);
        social->others_no_arg = str_dup(row[2]);
        social->char_found    = str_dup(row[3]);
        social->others_found  = str_dup(row[4]);
        social->vict_found    = str_dup(row[5]);
        social->char_auto     = str_dup(row[6]);
        social->others_auto   = str_dup(row[7]);
        social->not_found     = str_dup(row[8]);
        
        add_social( social );
    }
    
    mysql_free_result(res);
}

int db_insert_skill(int del, char *tablename, SKILLTYPE *skill)
{
    SMAUG_AFF *aff;
    char buf[MAX_STRING_LENGTH*2];
    char name[MAX_INPUT_LENGTH],code[MAX_INPUT_LENGTH],dammsg[MAX_INPUT_LENGTH],\
    wearoff[MAX_INPUT_LENGTH],wearoffroom[MAX_INPUT_LENGTH],\
    wearoffsoon[MAX_INPUT_LENGTH],wearoffsoonr[MAX_INPUT_LENGTH],\
    hitchar[MAX_INPUT_LENGTH],hitroom[MAX_INPUT_LENGTH],hitvict[MAX_INPUT_LENGTH],\
    misschar[MAX_INPUT_LENGTH],missvict[MAX_INPUT_LENGTH],missroom[MAX_INPUT_LENGTH],\
    diechar[MAX_INPUT_LENGTH],dievict[MAX_INPUT_LENGTH],dieroom[MAX_INPUT_LENGTH],\
    immchar[MAX_INPUT_LENGTH],immvict[MAX_INPUT_LENGTH],immroom[MAX_INPUT_LENGTH],\
    dice[MAX_INPUT_LENGTH],comp[MAX_INPUT_LENGTH],teacher[MAX_INPUT_LENGTH],\
    type[MAX_INPUT_LENGTH];
    char sname[MAX_INPUT_LENGTH],duration[MAX_INPUT_LENGTH],modifier[MAX_INPUT_LENGTH];
    int y=0,z=0,min=0;
    
    escape_string(sname,skill->name);
    
    if (del)
    {
        sprintf(buf,"delete from %s%s where sname='%s'",
                "aff",tablename,sname);
        db_generic_insdel(buf,__FILE__,__LINE__);
        sprintf(buf,"delete from %s where name='%s'",
                tablename,sname);
        db_generic_insdel(buf,__FILE__,__LINE__);
    }
    
    escape_string(name,skill->name);
    escape_string(type,skill_tname[skill->type]);
    if (skill->skill_fun)
        escape_string(code,skill_name(skill->skill_fun));
    else
        escape_string(code,spell_name(skill->spell_fun));
    escape_string(dammsg,skill->noun_damage);
    escape_string(wearoff,skill->msg_off);
    escape_string(wearoffroom,skill->msg_off_room);
    escape_string(wearoffsoon,skill->msg_off_soon);
    escape_string(wearoffsoonr,skill->msg_off_soon_room);
    escape_string(hitchar,skill->hit_char);
    escape_string(hitvict,skill->hit_vict);
    escape_string(hitroom,skill->hit_room);
    escape_string(misschar,skill->miss_char);
    escape_string(missvict,skill->miss_vict);
    escape_string(missroom,skill->miss_room);
    escape_string(diechar,skill->die_char);
    escape_string(dievict,skill->die_vict);
    escape_string(dieroom,skill->die_room);
    escape_string(immchar,skill->imm_char);
    escape_string(immvict,skill->imm_vict);
    escape_string(immroom,skill->imm_room);
    escape_string(dice,skill->dice);
    escape_string(comp,skill->components);
    escape_string(teacher,skill->teachers);
    min=10000;
    if ( skill->type != SKILL_HERB )
    {
        for ( z = 0; z < MAX_CLASS; z++ )
            if ( skill->skill_level[z] < min )
                min = skill->skill_level[z];
    }
    
    sprintf(buf, "insert into %s values('%s','%s',"
            "%d,%d,%d,%d,%d,%d,%d,%d,"
            "'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',"
            "%d,%d,%d,'%s','%s',%d)",
            tablename,
            name,type,
            skill->flags,
            skill->target,
            skill->minimum_position,
            skill->saves,
            skill->slot,
            skill->min_mana,
            skill->beats,
            skill->guild,
            code,dammsg,wearoff,wearoffroom,wearoffsoon,wearoffsoonr,
            hitchar,hitvict,hitroom,misschar,missvict,missroom,
            diechar,dievict,dieroom,immchar,immvict,immroom,
            dice,
            skill->value,
            skill->difficulty,
            skill->participants,
            comp,teacher,
            min==10000?0:min);
    
    y+=db_generic_insdel(buf,__FILE__,__LINE__);
    
    for ( aff = skill->affects; aff; aff = aff->next )
    {
        escape_string(duration,aff->duration);
        escape_string(modifier,aff->modifier);
        
        sprintf(buf, "insert into %s%s values('%s','%s',%d,'%s',%d)",
                "aff", tablename,
                sname,duration,aff->location,modifier,aff->bitvector);
        
        y+=db_generic_insdel(buf,__FILE__,__LINE__);
    }
    
    return y;
}

int db_insert_skills()
{
    char buf[MAX_INPUT_LENGTH];
    int x,y=0;
    
    sprintf(buf,"delete from %s",
            SKILL_TABLE_NAME);
    db_generic_insdel(buf,__FILE__,__LINE__);
    sprintf(buf,"delete from %s%s",
            "aff", SKILL_TABLE_NAME);
    db_generic_insdel(buf,__FILE__,__LINE__);
    
    for ( x = 0; x < top_sn; x++ )
    {
        if ( !skill_table[x]->name || skill_table[x]->name[0] == '\0' )
            break;
        
        y+=db_insert_skill(0,SKILL_TABLE_NAME,skill_table[x]);
    }
    
    return y;
}

SKILLTYPE *db_load_skill()
{
    SKILLTYPE *skill;
    SPELL_FUN *spellfun;
    DO_FUN *dofun;
    int x;
    
    CREATE( skill, SKILLTYPE, 1 );
    for ( x = 0; x < MAX_CLASS; x++ )
    {
        skill->skill_level[x] = LEVEL_IMMORTAL;
        skill->skill_adept[x] = 95;
    }
    skill->guild = -1;
    
    skill->name              = str_dup(arow[0]);
    skill->type              = get_skill_tname(arow[1]);
    skill->flags             = atoi(arow[2]);
    skill->target            = atoi(arow[3]);
    skill->minimum_position  = atoi(arow[4]);
    skill->saves             = atoi(arow[5]);
    skill->slot              = atoi(arow[6]);
    skill->min_mana          = atoi(arow[7]);
    skill->beats             = atoi(arow[8]);
    skill->guild             = atoi(arow[9]);
    if ( (spellfun=spell_function(arow[10])) != spell_notfound )
        skill->spell_fun     = spellfun;
    else if ( (dofun=skill_function(arow[10])) != skill_notfound )
        skill->skill_fun     = dofun;
    skill->noun_damage       = str_dup(arow[11]);
    skill->msg_off           = str_dup(arow[12]);
    skill->msg_off_room      = str_dup(arow[13]);
    skill->msg_off_soon      = str_dup(arow[14]);
    skill->msg_off_soon_room = str_dup(arow[15]);
    skill->hit_char          = str_dup(arow[16]);
    skill->hit_vict          = str_dup(arow[17]);
    skill->hit_room          = str_dup(arow[18]);
    skill->miss_char         = str_dup(arow[19]);
    skill->miss_vict         = str_dup(arow[20]);
    skill->miss_room         = str_dup(arow[21]);
    skill->die_char          = str_dup(arow[22]);
    skill->die_vict          = str_dup(arow[23]);
    skill->die_room          = str_dup(arow[24]);
    skill->imm_char          = str_dup(arow[25]);
    skill->imm_vict          = str_dup(arow[26]);
    skill->imm_room          = str_dup(arow[27]);
    skill->dice              = str_dup(arow[28]);
    skill->value             = atoi(arow[29]);
    skill->difficulty        = atoi(arow[30]);
    skill->participants      = atoi(arow[31]);
    skill->components        = str_dup(arow[32]);
    skill->teachers          = str_dup(arow[33]);
    skill->min_level         = atoi(arow[34]);
    
    return skill;
}

void db_load_skillaff(char *tablename, SKILLTYPE *skill)
{
    char buf[MAX_INPUT_LENGTH], sname[MAX_INPUT_LENGTH];
    
    if (!skill)
    {
        bug("db_load_skillaff: !skill");
        return;
    }
    
    if (skill->affects)
    {
        bug("db_load_skillaff: %s already has affects", skill->name);
        return;
    }
    
    escape_string(sname,skill->name);
    
    sprintf(buf, "select * from %s%s where sname='%s'",
            "aff", tablename, sname);
    
    if (mysql_query(&mysql,buf))
    {
        exiterr(21,__FILE__,__LINE__);
        return;
    }
    
    if (!(bres = mysql_store_result(&mysql)))
    {
        exiterr(22,__FILE__,__LINE__);
        return;
    }
    
    if (mysql_num_fields(bres)!=SKILLAFF_TABLE_ROWS)
    {
        log_printf("%s%s: expected %d fields got %d",
                   "aff", tablename, SKILLAFF_TABLE_ROWS, mysql_num_fields(bres));
        return;
    }
    
    while ((brow = mysql_fetch_row(bres)))
    {
        SMAUG_AFF *aff;
        
        CREATE( aff, SMAUG_AFF, 1 );
        
        aff->duration   = str_dup(brow[1]);
        aff->location   = atoi(brow[2]);
        aff->modifier   = str_dup(brow[3]);
        aff->bitvector  = atoi(brow[4]);
        
        aff->next = skill->affects;
        skill->affects = aff;
    }
    
    mysql_free_result(bres);
}

void db_load_skill_table()
{
    char buf[MAX_STRING_LENGTH];
    
    log_string("Loading skills...");
    sprintf(buf, "select * from %s",
            SKILL_TABLE_NAME);
    
    if (mysql_query(&mysql,buf))
    {
        exiterr(23,__FILE__,__LINE__);
        return;
    }
    
    if (!(ares = mysql_store_result(&mysql)))
    {
        exiterr(24,__FILE__,__LINE__);
        return;
    }
    
    if (mysql_num_fields(ares)!=SKILL_TABLE_ROWS)
    {
        log_printf("%s: expected %d fields got %d",
                   SKILL_TABLE_NAME, SKILL_TABLE_ROWS, mysql_num_fields(ares) );
        return;
    }
    
    while ((arow = mysql_fetch_row(ares)))
    {
        if ( top_sn >= MAX_SKILL )
        {
            bug( "db_load_skill_table: more skills than MAX_SKILL %d", MAX_SKILL );
            return;
        }
        skill_table[top_sn] = db_load_skill();
        db_load_skillaff(SKILL_TABLE_NAME,skill_table[top_sn]);
        top_sn++;
    }
    
    mysql_free_result(ares);
}

int db_insert_herbs()
{
    char buf[MAX_INPUT_LENGTH];
    int x,y=0;
    
    sprintf(buf,"delete from %s",
            HERB_TABLE_NAME);
    db_generic_insdel(buf,__FILE__,__LINE__);
    sprintf(buf,"delete from %s%s",
            "aff", HERB_TABLE_NAME);
    db_generic_insdel(buf,__FILE__,__LINE__);
    
    for ( x = 0; x < top_herb; x++ )
    {
        if ( !herb_table[x]->name || herb_table[x]->name[0] == '\0' )
            break;
        
        y+=db_insert_skill(0,HERB_TABLE_NAME,herb_table[x]);
    }
    
    return y;
}

void db_load_herb_table()
{
    char buf[MAX_STRING_LENGTH];
    
    log_string("Loading herbs...");
    sprintf(buf, "select * from %s",
            HERB_TABLE_NAME);
    
    if (mysql_query(&mysql,buf))
    {
        exiterr(25,__FILE__,__LINE__);
        return;
    }
    
    if (!(ares = mysql_store_result(&mysql)))
    {
        exiterr(26,__FILE__,__LINE__);
        return;
    }
    
    if (mysql_num_fields(ares)!=HERB_TABLE_ROWS)
    {
        log_printf("%s: expected %d fields got %d",
                   HERB_TABLE_NAME, HERB_TABLE_ROWS, mysql_num_fields(ares) );
        return;
    }
    
    while ((arow = mysql_fetch_row(ares)))
    {
        if ( top_herb >= MAX_HERB )
        {
            bug( "db_ load_herb_table: more herbs than MAX_HERB %d", MAX_HERB );
            return;
        }
        herb_table[top_herb] = db_load_skill();
        db_load_skillaff(HERB_TABLE_NAME,herb_table[top_herb]);
        top_herb++;
        if ( herb_table[top_herb-1]->slot == 0 )
            herb_table[top_herb-1]->slot = top_herb-1;
    }
    
    mysql_free_result(ares);
}


#endif
