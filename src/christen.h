/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/
struct christen_data
{
    CHRISTEN_DATA *next;
    CHRISTEN_DATA *prev;
    int cvnum;
    int ovnum;
    time_t when;
    char *name;
    char *owner;
};

void load_christens(void);
CHRISTEN_DATA *get_christen(int cvnum);
char *get_christen_name(OBJ_DATA *obj);
void save_christens(void);
