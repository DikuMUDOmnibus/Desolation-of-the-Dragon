/******************************************************
            Desolation of the Dragon MUD II
      (C) 2001-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/
#define PROPERTY_FILE   SYSTEM_DIR "property.dat"                               

typedef struct property_data    PROPERTY_DATA;
typedef struct raw_goods_data   RAW_GOODS_DATA;

struct raw_goods_data
{

    long ammount;
    RAW_GOODS_DATA *next_good;
    RAW_GOODS_DATA *prev_good;
};

struct property_data
{
    int vnum;
    int value;
    int currtype;
    int flags;
    int profit_buy;
    int profit_rent;
    int money[MAX_CURR_TYPE];
    char *owner;
    char *name;
    RAW_GOODS_DATA *raw_goods;
    PROPERTY_DATA *next_property;
    PROPERTY_DATA *prev_property;
};

/*
 * Property flags
 */
#define PROPERTY_FORSALE      BV00


#define PROFIT_RENT           1
#define PROFIT_BUY            2
#define PROFIT_SELL           3

int  property_tax(ROOM_INDEX_DATA *room, int cost, int profit_type);
void property_add_tax(ROOM_INDEX_DATA *room, int cost, int type, int profit_type);

