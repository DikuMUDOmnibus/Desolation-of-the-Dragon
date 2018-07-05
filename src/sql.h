/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

#if defined(USE_DB) || defined(START_DB)

int db_insert_mob(int del, MOB_INDEX_DATA *mob);
int db_insert_obj(int del, OBJ_INDEX_DATA *obj);
int db_insert_room(int del, ROOM_INDEX_DATA *room);
int db_insert_shop(int del, SHOP_DATA *shop);
int db_insert_repair(int del, REPAIR_DATA *repair);
int db_insert_special(int del, AREA_DATA *area, SPEC_FUN *spec, int vnum, char *type);
int db_insert_climate(int del, AREA_DATA *area);
int db_insert_descs(int del, AREA_DATA *area, EXTRA_DESCR_DATA *eds, int vnum, char *type);
int db_insert_exit(int del, AREA_DATA *area, EXIT_DATA *ex, int vnum);
int db_insert_neighbors(int del, AREA_DATA *area);
int db_insert_objaffects(int del, AREA_DATA *area, OBJ_INDEX_DATA *obj);
int db_insert_progs(int del, AREA_DATA *area, MPROG_DATA *mprogs, int vnum, char *type);
int db_insert_resets(int del, AREA_DATA *area);
int db_insert_area(int del, AREA_DATA *area);
int db_insert_helps();
int db_insert_commands();
int db_insert_socials();
int db_insert_skills();
int db_insert_herbs();

void db_del_area(AREA_DATA *area);
#endif
