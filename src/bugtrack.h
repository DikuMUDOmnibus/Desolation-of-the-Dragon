/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

#define BUGTRACK_FILE   SYSTEM_DIR "bugtrack.dat"


typedef struct bugtrack_data BUGTRACK_DATA;

typedef enum
{
    BUGTRACK_INVALID_STATUS = -1,
    BUGTRACK_BUG, BUGTRACK_FIXED, BUGTRACK_NOTABUG, BUGTRACK_DUPLICATE,
    BUGTRACK_WORKSFORME, BUGTRACK_ASSIGNED, BUGTRACK_FEATURE_REQUEST,
    BUGTRACK_IDEA, BUGTRACK_TYPO, BUGTRACK_REJECTED,
    BUGTRACK_MAX_STATUS
} bugtrack_status;

#define BUGTRACK_FIRST_STATUS BUGTRACK_BUG

#define BT_IS_OPEN(bt) ((bt)->status == BUGTRACK_BUG || \
                        (bt)->status == BUGTRACK_ASSIGNED || \
                        (bt)->status == BUGTRACK_IDEA || \
                        (bt)->status == BUGTRACK_TYPO)

struct bugtrack_data
{
    BUGTRACK_DATA *next;
    BUGTRACK_DATA *prev;

    int id;
    char *submitter;
    char *owner;
    char *text;
    char *bugstr;
    sh_int status;
    time_t opened;
    time_t closed;
    time_t updated;
};


void free_bugtracks(void);
void load_bugtracks(void);
void save_bugtracks(void);

void bugtrack_check(CHAR_DATA *ch);
