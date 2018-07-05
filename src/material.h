/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer and Heath Leech
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/
#define QUAL_WORTHLESS   0
#define QUAL_POOR        1
#define QUAL_AVERAGE     2
#define QUAL_EXCELENT    3
#define QUAL_UNEARTHLY   4
#define QUAL_GODLY       5
#define MAT_WORTHLESS    0
#define MAT_WOOD         1
#define MAT_LEAD         2
#define MAT_COPPER       3
#define MAT_STEEL        4
#define MAT_SILVER       5
#define MAT_GOLD         6
#define MAT_TITANIUM     7
#define MAT_MITHRIL      8
#define MAT_ADAMANTITE   9
#define MAT_LAST         9
#define MTYPE(obj)       ((obj)->value[0])
#define MGRADE(obj)      ((obj)->value[1])
#define ARTLEV(ch)       GetClassLevel(ch, CLASS_ARTIFICER)

