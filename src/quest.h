/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/


#define QUEST_FILE   SYSTEM_DIR "quest.dat"


typedef struct quest_data       QUEST_DATA;
typedef struct quest_task_data  QUEST_TASK;

typedef enum
{
    QUEST_MOB_FIND, QUEST_OBJ_FIND, QUEST_ROOM_FIND,
    QUEST_MOB_KILL, QUEST_OBJ_GIVE, QUEST_OBJ_PLACE,
    MAX_TASK_TYPE
} quest_task_types;

struct quest_data
{
    QUEST_DATA *next;
    QUEST_DATA *prev;

    char *name;
    char *owner;
    char *creator;
    char *description;

    char *completed_by;
    time_t completed;
    int glory;

    unsigned int flags;

    QUEST_TASK *first_task;
    QUEST_TASK *last_task;
};

struct quest_task_data
{
    QUEST_TASK *next;
    QUEST_TASK *prev;

    char *description;

    char *completed_by;
    time_t completed;
    int glory;

    unsigned int flags;

    sh_int type;

    int vnum1;
    int vnum2;
};


#define QUEST_ACTIVE     BV00
#define QUEST_INORDER    BV01
#define QUEST_AUTORESET  BV02
#define QUEST_ALLOWNPC   BV03
#define QUEST_ONLYONCE   BV04

#define IS_QUEST_FLAG(var, bit)          IS_SET((var)->flags, (bit))
#define SET_QUEST_FLAG(var, bit)         SET_BIT((var)->flags, (bit))
#define REMOVE_QUEST_FLAG(var, bit)      REMOVE_BIT((var)->flags, (bit))


#define TASK_OPTIONAL    BV00

#define IS_TASK_FLAG(var, bit)           IS_SET((var)->flags, (bit))
#define SET_TASK_FLAG(var, bit)          SET_BIT((var)->flags, (bit))
#define REMOVE_TASK_FLAG(var, bit)       REMOVE_BIT((var)->flags, (bit))


void free_quests(void);
void load_quests(void);
void save_quests(void);

bool quest_trigger_objfind(CHAR_DATA *ch, OBJ_DATA *obj);
bool quest_trigger_mobfind(CHAR_DATA *ch, CHAR_DATA *mob);
bool quest_trigger_roomfind(CHAR_DATA *ch, ROOM_INDEX_DATA *room);
bool quest_trigger_mobkill(CHAR_DATA *ch, CHAR_DATA *victim);
bool quest_trigger_objgive(CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *victim);
bool quest_trigger_objplace(CHAR_DATA *ch, OBJ_DATA *obj, ROOM_INDEX_DATA *room);

extern sh_int quests_active;
