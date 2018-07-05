/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/


typedef struct	rare_obj_data		RARE_OBJ_DATA;


struct rare_obj_data
{
    RARE_OBJ_DATA   *next;
    RARE_OBJ_DATA   *prev;

    char            *owner;
    char            *name;
    int             vnum;
    int             rent;

    time_t          time_added;
};

void free_rare_objs(void);
bool is_rare_obj(OBJ_DATA *obj);
bool delete_char_rare_obj(CHAR_DATA *ch);
void update_char_rare_obj(CHAR_DATA *ch);
void save_rare_objs(void);
void load_rare_objs(void);
int  count_objs_for_reset(OBJ_INDEX_DATA *pObj);
