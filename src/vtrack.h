/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2003  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

typedef struct vtrack_data VTRACK_DATA;


struct vtrack_data
{
    VTRACK_DATA *next;

    int vnum;

    int flags;
};

#define VTRACK_ROOM     BV00
#define VTRACK_MOB      BV01
#define VTRACK_OBJ      BV02
#define VTRACK_MOBKILL  BV03

void free_vtracks(CHAR_DATA *ch);
void fwrite_vtracks(CHAR_DATA *ch, FILE *fp);
void fread_vtracks(CHAR_DATA *ch, FILE *fp);

void vtrack_add(CHAR_DATA *ch, int vnum, int flags);
#define vtrack_add_room(ch, vnum) vtrack_add((ch), (vnum), VTRACK_ROOM)
#define vtrack_add_mob(ch, vnum) vtrack_add((ch), (vnum), VTRACK_MOB)
#define vtrack_add_obj(ch, vnum) vtrack_add((ch), (vnum), VTRACK_OBJ)
#define vtrack_add_mobkill(ch, vnum) vtrack_add((ch), (vnum), VTRACK_MOBKILL)

int vtrack_count(CHAR_DATA *ch, int flag);
#define vtrack_count_room(ch) vtrack_count((ch), VTRACK_ROOM)
#define vtrack_count_mob(ch) vtrack_count((ch), VTRACK_MOB)
#define vtrack_count_obj(ch) vtrack_count((ch), VTRACK_OBJ)
#define vtrack_count_mobkill(ch) vtrack_count((ch), VTRACK_MOBKILL)

float vtrack_percent(CHAR_DATA *ch, int flag);
#define vtrack_percent_room(ch) vtrack_percent((ch), VTRACK_ROOM)
#define vtrack_percent_mob(ch) vtrack_percent((ch), VTRACK_MOB)
#define vtrack_percent_obj(ch) vtrack_percent((ch), VTRACK_OBJ)
#define vtrack_percent_mobkill(ch) vtrack_percent((ch), VTRACK_MOBKILL)
