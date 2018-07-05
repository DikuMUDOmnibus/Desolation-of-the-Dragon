/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer and Heath Leach
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*static char rcsid[] = "$Id: map.c,v 1.13 2003/12/21 17:20:55 dotd Exp $";*/

#include "mud.h"
#include <stdio.h>
#include <string.h>

#define NORTH     BV00
#define SOUTH     BV01
#define EAST      BV02
#define WEST      BV03
#define NORTHEAST BV04
#define NORTHWEST BV05
#define SOUTHEAST BV06
#define SOUTHWEST BV07
#define FILLER    BV08

#define MAXX 40
#define MAXY 40

/* max distance between rooms in map */
#define POINT_DISTANCE 8

#define EMPTY_SQUARE   0
#define SQUASH_MEX     -1
#define SQUASH_MEY     -2

typedef struct map_d MAP;

struct map_d
{
    int map[MAXX][MAXY];
    short int mflags[MAXX][MAXY];
    int chroom;
};

MAP *m;

#define CENTERX MAXX/2
#define CENTERY MAXY/2

static void write_map(AREA_DATA *area);

static int cx,cy;
AREA_DATA *ab;

static int in_map(int v)
{
    int x,y,z=0;

    for (x=0;x<MAXX;x++)
        for (y=0;y<MAXY;y++)
            if (m->map[x][y]==v)
                z++;
    return z;
}

static void clear_map_block(int x, int y)
{
    m->map[x][y] = EMPTY_SQUARE;
    m->mflags[x][y] = 0;
}

static void init_map(AREA_DATA *area)
{
    int x,y;
    cx=CENTERX;
    cy=CENTERY;
    ab=area;
    for (x=0;x<MAXX;x++)
        for (y=0;y<MAXX;y++)
            clear_map_block(x,y);
}

static int first_y(void)
{
    int x,y;

    for (y=0;y<MAXY;y++)
    {
        for (x=0;x<MAXX;x++)
            if (m->map[y][x])
                return y;
    }
    return 0;
}

static int first_x(void)
{
    int x,y;

    for (x=0;x<MAXX;x++)
    {
        for (y=0;y<MAXY;y++)
            if (m->map[y][x])
                return x;
    }
    return 0;
}

static int last_y(void)
{
    int x,y;

    for (y=MAXY;y>0;y--)
    {
        for (x=MAXX;x>0;x--)
            if (m->map[y][x])
                return y+1;
    }
    return MAXX;
}

static int last_x(void)
{
    int x,y;

    for (x=MAXX;x>0;x--)
    {
        for (y=MAXY;y>0;y--)
            if (m->map[y][x])
                return x+1;
    }
    return MAXX;
}

/* old/broken way */
#if 0
static void copy_map_block(int srcx, int srcy, int destx, int desty)
{
    m->map[destx][desty]    = m->map[srcx][srcy];
    m->mflags[destx][desty] = m->mflags[srcx][srcy];
}

/* shift the block of y's from y to endy one position west */
static void shift_west(int endx)
{
    int x, y;

    if (first_x()<1)
    {
        bug("shift_west: first_x is map edge already, can't shift");
        return;
    }

    for (x = first_x(); x <= endx; x++)
        for (y = 0; y < MAXY; y++)
        {
            copy_map_block(x, y, x-1, y);
            if (x == endx &&
                IS_SET(m->mflags[x+1][y], WEST) &&
                IS_SET(m->mflags[x-1][y], EAST))
            {
                clear_map_block(x, y);
                m->map[x][y] = 1;
                SET_BIT(m->mflags[x][y], EAST);
                SET_BIT(m->mflags[x][y], WEST);
            }
            else
                clear_map_block(x, y);
        }
}

static void shift_east(int startx)
{
    int x, y;

    if (last_x()>=MAXX-1)
    {
        bug("shift_east: last_x is map edge already, can't shift");
        return;
    }

    fprintf(stderr, "%d %d\n", last_x(), startx);
    for (x = last_x(); x > startx; x--)
        for (y = 0; y < MAXY; y++)
        {
            copy_map_block(x, y, x+1, y);
            clear_map_block(x, y);
            if (x == startx)
            {
                if (IS_SET(m->mflags[x-1][y], EAST))
                    SET_BIT(m->mflags[x][y], WEST);
                if (IS_SET(m->mflags[x+1][y], WEST))
                    SET_BIT(m->mflags[x][y], EAST);
                SET_BIT(m->mflags[x][y], FILLER);
                m->map[x][y] = m->map[x+1][y];
            }
        }

    write_map(ab);
}

static void update_map(ROOM_INDEX_DATA *room, int x, int y)
{
    EXIT_DATA *exit;

    if (!room || x<0 || x>=MAXX || y<0 || y>=MAXY)
    {
        bug("Out of range.");
        return;
    }

/*    if (IS_ROOM_FLAG(room, BV31))
        return;*/

    if (m->map[x][y])
    {
        /* something wrong here, map loc should be empty because of shifting */
        return;
    }

    for ( exit = room->first_exit; exit; exit = exit->next )
    {
        if (exit->to_room->vnum==room->vnum)
            continue;
        if ( !get_exit_to(exit->to_room, exit->rdir, room->vnum) )
            continue;

        switch (exit->vdir)
        {
        default:
            break;
        case DIR_NORTH:
            SET_BIT(m->mflags[x][y],NORTH);
            break;
        case DIR_EAST:
            SET_BIT(m->mflags[x][y],EAST);
            break;
        case DIR_SOUTH:
            SET_BIT(m->mflags[x][y],SOUTH);
            break;
        case DIR_WEST:
            SET_BIT(m->mflags[x][y],WEST);
            break;
        case DIR_NORTHEAST:
            SET_BIT(m->mflags[x][y],NORTHEAST);
            break;
        case DIR_NORTHWEST:
            SET_BIT(m->mflags[x][y],NORTHWEST);
            break;
        case DIR_SOUTHEAST:
            SET_BIT(m->mflags[x][y],SOUTHEAST);
            break;
        case DIR_SOUTHWEST:
            SET_BIT(m->mflags[x][y],SOUTHWEST);
            break;
        }
    }

    if (in_map(room->vnum)>=2)
        return;

    if (room->area!=ab)
        return;

    m->map[x][y] = room->vnum;
    /*SET_ROOM_FLAG(room, BV31);*/

    for ( exit = room->first_exit; exit; exit = exit->next )
    {
        int nx, ny;

        if (exit->to_room->vnum==room->vnum)
            continue;

        if (in_map(exit->to_room->vnum))
            continue;

        switch (exit->vdir)
        {
        case DIR_NORTH:
            nx = x; ny = y-1;
            break;
        case DIR_EAST:
            nx = x+1; ny = y;
            break;
        case DIR_SOUTH:
            nx = x; ny = y+1;
            break;
        case DIR_WEST:
            nx = x-1; ny = y;
            break;
        case DIR_NORTHEAST:
            nx = x+1; ny = y-1;
            break;
        case DIR_NORTHWEST:
            nx = x-1; ny = y-1;
            break;
        case DIR_SOUTHEAST:
            nx = x+1; ny = y+1;
            break;
        case DIR_SOUTHWEST:
            nx = x-1; ny = y+1;
            break;
        default:
            continue;
            break;
        }

        if (nx < 0 || nx >= MAXX || ny < 0 || ny >= MAXY)
            continue;

        if (m->map[nx][ny] && exit->to_room->vnum != m->map[nx][ny])
        {
//            if (nx == (x-1) && ny == y)
//                shift_west(nx);
            if (nx == (x+1) && ny == y)
                shift_east(nx);
        }
        update_map(exit->to_room, nx, ny);
    }
}

static void write_map(AREA_DATA *area)
{
    int x,y;
    FILE *fp;
    char l1[MAXX*6],l2[MAXX*6],l3[MAXX*6];
    char b1[10];

    sprintf(l1,"%s.map", area->filename);

    if (!(fp=fopen(l1,"w")))
        return;


    for (x=first_x();x<last_x();x++)
    {
        l1[0]=l2[0]=l3[0]='\0';
        for (y=first_y();y<last_y();y++)
        {
            if (m->map[y][x])
            {
                sprintf(b1, "%c  %c  %c",
                        IS_SET(m->mflags[y][x],NORTHWEST) && m->map[y-1][x-1]  ?'\\' :' ',
                        IS_SET(m->mflags[y][x],NORTH)     && m->map[y][x-1]    ?'|'  :' ',
                        IS_SET(m->mflags[y][x],NORTHEAST) && m->map[y+1][x-1]  ?'/'  :' ' );
                strcat(l1,b1);
                sprintf(b1, "%c%5d%c",
                        IS_SET(m->mflags[y][x],WEST)      && m->map[y-1][x]    ?'-'  :' ',
                        m->map[y][x],
//                        m->map[y][x]?(m->map[y][x]==m->chroom?'X':(IS_SET(m->mflags[y][x],FILLER)?'+':'*')):' ',
                        IS_SET(m->mflags[y][x],EAST)      && m->map[y+1][x]    ?'-'  :' ' );
                strcat(l2,b1);
                sprintf(b1, "%c  %c  %c",
                        IS_SET(m->mflags[y][x],SOUTHWEST) && m->map[y-1][x+1]  ?'/'  :' ',
                        IS_SET(m->mflags[y][x],SOUTH)     && m->map[y][x+1]    ?'|'  :' ',
                        IS_SET(m->mflags[y][x],SOUTHEAST) && m->map[y+1][x+1]  ?'\\' :' ' );
                strcat(l3,b1);
            }
            else
            {
                strcat(l1,"       ");
                strcat(l2,"       ");
                strcat(l3,"       ");
            }
        }
        fprintf(fp,"%s\n%s\n%s\n",l1,l2,l3);
    }

    fclose(fp);
    exit(0);
}
#endif

static void update_map(ROOM_INDEX_DATA *room, int x, int y)
{
    EXIT_DATA *exit;

    if (!room || x<0 || x>=MAXX || y<0 || y>=MAXY)
    {
        bug("Out of range.");
        return;
    }

    if (m->map[x][y])
    {
        bug("Map spacing too small, increase POINT_DISTANCE.");
        return;
    }

    for ( exit = room->first_exit; exit; exit = exit->next )
    {
        /* if exit to self */
        if (exit->to_room->vnum==room->vnum)
            continue;

        /* if not two-way exit */
	if ( !get_exit_to(exit->to_room, exit->rdir, room->vnum) )
	    continue;

        switch (exit->vdir)
        {
        default:
            break;
        case DIR_NORTH:
            SET_BIT(m->mflags[x][y],NORTH);
            break;
        case DIR_EAST:
            SET_BIT(m->mflags[x][y],EAST);
            break;
        case DIR_SOUTH:
            SET_BIT(m->mflags[x][y],SOUTH);
            break;
        case DIR_WEST:
            SET_BIT(m->mflags[x][y],WEST);
            break;
        case DIR_NORTHEAST:
            SET_BIT(m->mflags[x][y],NORTHEAST);
            break;
        case DIR_NORTHWEST:
            SET_BIT(m->mflags[x][y],NORTHWEST);
            break;
        case DIR_SOUTHEAST:
            SET_BIT(m->mflags[x][y],SOUTHEAST);
            break;
        case DIR_SOUTHWEST:
            SET_BIT(m->mflags[x][y],SOUTHWEST);
            break;
        }
    }

    if (in_map(room->vnum)>=2)
        return;

    if (room->area!=ab)
        return;

    m->map[x][y] = room->vnum;
    fprintf(stderr, "%d\n", room->vnum);

    for ( exit = room->first_exit; exit; exit = exit->next )
    {
        int nx, ny;

        /* if exit to self */
        if (exit->to_room->vnum==room->vnum)
            continue;

        /* if other room is already on map */
        if (in_map(exit->to_room->vnum))
            continue;

        switch (exit->vdir)
        {
	case DIR_NORTH:
            nx = x; ny = y-POINT_DISTANCE;
            break;
        case DIR_EAST:
            nx = x+POINT_DISTANCE; ny = y;
            break;
        case DIR_SOUTH:
            nx = x; ny = y+POINT_DISTANCE;
            break;
        case DIR_WEST:
            nx = x-POINT_DISTANCE; ny = y;
            break;
        case DIR_NORTHEAST:
            nx = x+POINT_DISTANCE; ny = y-POINT_DISTANCE;
            break;
        case DIR_NORTHWEST:
            nx = x-POINT_DISTANCE; ny = y-POINT_DISTANCE;
            break;
        case DIR_SOUTHEAST:
            nx = x+POINT_DISTANCE; ny = y+POINT_DISTANCE;
            break;
        case DIR_SOUTHWEST:
            nx = x-POINT_DISTANCE; ny = y+POINT_DISTANCE;
            break;
        default:
            continue;
            break;
        }

        if (nx < 0 || nx >= MAXX || ny < 0 || ny >= MAXY)
            continue;

        update_map(exit->to_room, nx, ny);
    }

}

static void write_map(AREA_DATA *area)
{
    int x,y;
    FILE *fp;
    char l1[MAXX*3],l2[MAXX*3],l3[MAXX*3];
    char b1[10];

    sprintf(l1,"%s.map", area->filename);

    if (!(fp=fopen(l1,"w")))
        return;


    for (x=first_x();x<last_x();x++)
    {
        l1[0]=l2[0]=l3[0]='\0';
        for (y=first_y();y<last_y();y++)
        {
            if (m->map[y][x])
	    {
#if 0
                sprintf(b1, "%c%c%c",
                        IS_SET(m->mflags[y][x],NORTHWEST) /*&& m->map[y-1][x-1]*/  ?'\\' :' ',
                        IS_SET(m->mflags[y][x],NORTH)     /*&& m->map[y][x-1]  */  ?'|'  :' ',
                        IS_SET(m->mflags[y][x],NORTHEAST) /*&& m->map[y+1][x-1]*/  ?'/'  :' ' );
                strcat(l1,b1);
                sprintf(b1, "%c%c%c",
                        IS_SET(m->mflags[y][x],WEST)      /*&& m->map[y-1][x]  */  ?'-'  :' ',
			m->map[y][x]>0?(m->map[y][x]==m->chroom?'X':(IS_SET(m->mflags[y][x],FILLER)?'+':'*')):(m->map[y][x]==SQUASH_MEX?'.':(m->map[y][x]==SQUASH_MEY?',':' ')),
                        IS_SET(m->mflags[y][x],EAST)      /*&& m->map[y+1][x]  */  ?'-'  :' ' );
                strcat(l2,b1);
                sprintf(b1, "%c%c%c",
                        IS_SET(m->mflags[y][x],SOUTHWEST) /*&& m->map[y-1][x+1]*/  ?'/'  :' ',
                        IS_SET(m->mflags[y][x],SOUTH)     /*&& m->map[y][x+1]  */  ?'|'  :' ',
                        IS_SET(m->mflags[y][x],SOUTHEAST) /*&& m->map[y+1][x+1]*/  ?'\\' :' ' );
		strcat(l3,b1);
#endif
		sprintf(b1, "%c",
			m->map[y][x]>0?(m->map[y][x]==m->chroom?'X':(IS_SET(m->mflags[y][x],FILLER)?'+':'*')):(m->map[y][x]==SQUASH_MEX?'.':(m->map[y][x]==SQUASH_MEY?',':' ')));
		strcat(l2,b1);
            }
            else if (m->map[y][x]!=SQUASH_MEX && m->map[y][x]!=SQUASH_MEY)
            {
#if 0
		strcat(l1,"   ");
		strcat(l2,"   ");
		strcat(l3,"   ");
#endif
		strcat(l2," ");
            }
        }
#if 0
	fprintf(fp,"%s\n%s\n%s\n",l1,l2,l3);
#endif
	fprintf(fp,"%s\n",l2);
    }

    fclose(fp);
}

static void squash_map(void)
{
    int x, y, t, nextx, prevx, nexty, prevy;
    bool squash;

    for (x=1;x<MAXX-1;x++)
    {
	squash = TRUE;
	for (y=0;y<MAXY;y++)
	{
	    if (m->map[x][y]!=0)
	    {
		squash = FALSE;
		break;
	    }
	    nextx=prevx=1;
	    for (t=1;t<POINT_DISTANCE;t++)
		if (m->map[x+t][y]>0)
		{
		    nextx=t;
		    break;
		}
	    for (t=1;t<POINT_DISTANCE;t++)
		if (m->map[x-t][y]>0)
		{
		    prevx=t;
		    break;
		}
	    if (m->map[x+nextx][y]>0 && m->map[x-prevx]>0)
		if (!IS_SET(m->mflags[x+nextx][y], EAST) ||
		    !IS_SET(m->mflags[x-prevx][y], WEST))
		{
		    squash = FALSE;
		    break;
		}
	}
	if (squash)
	    for (y=0;y<MAXY;y++)
		m->map[x][y] = SQUASH_MEX;

    }

    for (y=1;y<MAXY-1;y++)
    {
	squash = TRUE;
	for (x=0;x<MAXX;x++)
	{
	    if (m->map[x][y]!=0)
	    {
		squash = FALSE;
		break;
	    }
	    nexty=prevy=1;
	    for (t=1;t<POINT_DISTANCE;t++)
		if (m->map[x][y+t]>0)
		{
		    nexty=t;
		    break;
		}
	    for (t=1;t<POINT_DISTANCE;t++)
		if (m->map[x][y-t]>0)
		{
		    prevy=t;
		    break;
		}
	    if (m->map[x][y+nexty]>0 && m->map[x][y-prevy]>0)
		if (!IS_SET(m->mflags[x][y-prevy], NORTH) ||
		    !IS_SET(m->mflags[x][y+nexty], SOUTH))
		{
		    squash = FALSE;
		    break;
		}
	}
	if (squash)
	    for (x=0;x<MAXX;x++)
		m->map[x][y] = SQUASH_MEY;

    }
}

void do_makemapfile(CHAR_DATA *ch, char *argument)
{
    m = (MAP *)calloc(1,sizeof(MAP));
    init_map(ch->in_room->area);
    m->chroom=ch->in_room->vnum;
    update_map(ch->in_room,cx,cy);
    squash_map();
    write_map(ch->in_room->area);
    free(m);
    send_to_char("Ok.\n\r", ch);
}

