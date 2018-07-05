/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer and Heath Leach
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*
 ** just for polymorph spell(s)
 */

struct PolyType
{
    char name[20];
    int level;
    long number;
    sh_int cl;
};

/*
 ** Must leave all polies for a given class sorted in order of level
 */

#define MAX_POLY 64

struct PolyType PolyList[MAX_POLY] = {
    /* Mage Polies */
    {"goblin",      4, 5100, 0}, 
    {"creepy",      4, 9001, 0},
    {"bat",         4, 7001, 0},
    {"orc",         5, 4005, 0},
    {"trog",        5, 9210, 0}, 
    {"gnoll",       6, 9211, 0},
    {"parrot",      6, 9010, 0},
    {"lizard",      6, 224, 0},
    {"ogre",        8, 4113, 0},
    {"parrot",      8, 9011, 0},
    {"wolf",        8, 3094, 0},
    {"spider",      9, 227, 0},
    {"beast",       9, 242, 0},
    {"minotaur",    9, 247, 0},
    {"snake",       10, 249, 0},
    {"bull",        10, 1008, 0},
    {"warg",        10, 6100, 0},
    {"rat",         11, 7002, 0},
    {"sapling",     12, 1421, 0},
    {"ogre-maji",   12, 257, 0},
    {"black",       12, 230, 0},
    {"giant",	  13, 261, 0},
    {"troll",       14, 4101, 0},
    {"crocodile",   14, 5310, 0},
    {"mindflayer",  14, 7202, 0},
    {"bear",        16, 9024, 0},
    {"blue",        16, 233, 0},
    {"enfan",       18, 21001, 0},
    {"lamia",       18, 5201, 0},
    {"drider",      18, 5011, 0},
    {"wererat",     19, 7203, 0},
    {"wyvern",      20, 3752, 0},
    {"mindflayer",  20, 7201, 0},
    {"spider",      20, 20010, 0},
    {"roc",         22, 3724, 0},
    {"mud",         23, 7000, 0},
    {"enfan",       23, 21004, 0},
    {"giant",       24, 9406, 0},
    {"white",       26, 243, 0},
    {"master",      28, 7200, 0},
    {"red",         30, 7040, 0},
    {"roo",         35, 27411, 0},
    {"brontosaurus",35, 21802, 0},
    {"mulichort",   40, 15830, 0},
    {"beholder",    45, 5200, 0},
    /* Druid Polies */
    {"bear",        10, 9024, 5},
    {"spider",      10, 20010, 5},
    {"lamia",       10, 3648, 5},
    {"lizard",      10, 6822, 5},
    {"bear",        12, 9056, 5},
    {"gator",       12, 9054, 5},
    {"basilisk",    13, 7043, 5},
    {"snake",       15, 6517, 5},
    {"spider",      15, 6113, 5},
    {"lizard",      16, 6505, 5},
    {"allosaurus",  18, 21801, 5},
    {"tiger",       28, 9027, 5},
    {"mulichort",   30, 15830, 5},
    {"tiger",       35, 9055, 5},
    {"lion",        35, 13718, 5},
    {"salamander",  35, 25506, 5}  
};

