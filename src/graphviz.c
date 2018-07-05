#include <stdio.h>

#include "mud.h"

extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];

struct visited
{
    struct visited *next;
    void *data;
    void *data2;
};

struct visited *vlist = NULL;

bool is_visited(void *data)
{
    struct visited *v;

    for (v = vlist; v; v = v->next)
	if (v->data == data)
	    return TRUE;
    return FALSE;
}

bool is_visited_pair(void *data, void *data2)
{
    struct visited *v;

    for (v = vlist; v; v = v->next)
	if ((v->data == data &&
	     v->data2 == data2) ||
	    (v->data == data2 &&
	     v->data2 == data))
	    return TRUE;
    return FALSE;
}

void add_visited(void *data)
{
    struct visited *v;

    CREATE(v, struct visited, 1);
    v->data = data;

    v->next = vlist;
    vlist = v;
}

void add_visited_pair(void *data, void *data2)
{
    struct visited *v;

    CREATE(v, struct visited, 1);
    v->data = data;
    v->data2 = data2;

    v->next = vlist;
    vlist = v;
}

void free_visited(void)
{
    struct visited *v;

    while (vlist)
    {
        v = vlist;
        vlist = vlist->next;
        DISPOSE(v);
    }
}

static void graph_recurse( FILE *fp, ROOM_INDEX_DATA *room, int recurse_level )
{
    EXIT_DATA *xit;

    if (recurse_level++ > 15)
        return;

    if (!room->first_exit || IS_ROOM_FLAG(room, ROOM_ORPHANED))
	return;

    fprintf(fp, "    \"%s (%d)\" -- ", room->name, room->vnum);

    if (room->first_exit != room->last_exit)
	fprintf(fp, "{ ");

    for ( xit = room->first_exit; xit; xit = xit->next )
    {
	if ( IS_EXIT_FLAG(xit, EX_PORTAL) )
	    continue;

	fprintf(fp, "\"%s (%d)\" ", xit->to_room->name, xit->vnum);
    }
    if (room->first_exit != room->last_exit)
	fprintf(fp, "}");
    fprintf(fp, ";\n");

    for ( xit = room->first_exit; xit; xit = xit->next )
    {
	if ( IS_EXIT_FLAG(xit, EX_PORTAL) )
	    continue;

	if (is_visited(xit->to_room))
	    continue;
	add_visited(xit->to_room);

        graph_recurse(fp, xit->to_room, recurse_level);
    }
}

static void graph_area(ROOM_INDEX_DATA *first_room)
{
    FILE *fp;
    char filename[MAX_INPUT_LENGTH];

    sprintf(filename, "%s.dot", first_room->area->filename);

    if (!(fp = fopen(filename, "w")))
        return;

    fprintf(fp, "graph G {\n");

    vlist = NULL;
    add_visited(first_room);

    graph_recurse(fp, first_room, 0);

    free_visited();

    fprintf(fp, "}\n");

    fclose(fp);
}

static void graph_mud(void)
{
    FILE *fp;
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pexit;
    char filename[16];
    int hash;

    sprintf(filename, "mud.dot");

    if (!(fp = fopen(filename, "w")))
        return;

    fprintf(fp, "graph G {\n");

    for ( hash = 0; hash < MAX_KEY_HASH; hash++ )
	for ( pRoomIndex = room_index_hash[hash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
	{
	    for ( pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next )
	    {
		if (IS_ROOM_FLAG(pexit->to_room, ROOM_ORPHANED) ||
		    IS_ROOM_FLAG(pRoomIndex, ROOM_ORPHANED))
		    continue;

		if (is_visited_pair(pRoomIndex->area, pexit->to_room->area))
		    continue;

		if (pexit->to_room->area != pRoomIndex->area )
		{
		    add_visited_pair(pexit->to_room->area, pRoomIndex->area);
		    fprintf(fp, "    \"%s\" -- \"%s\"\n",
			    pexit->to_room->area->name,
                            pRoomIndex->area->name);
		}
	    }
	    if (pRoomIndex->tele_vnum)
	    {
		ROOM_INDEX_DATA *to_room = get_room_index(pRoomIndex->tele_vnum);

		if (!to_room)
		    continue;

		if (IS_ROOM_FLAG(to_room, ROOM_ORPHANED) ||
		    IS_ROOM_FLAG(pRoomIndex, ROOM_ORPHANED))
		    continue;

		if (is_visited_pair(pRoomIndex->area, to_room->area))
		    continue;

		if (to_room->area != pRoomIndex->area )
		{
		    add_visited_pair(to_room->area, pRoomIndex->area);
		    fprintf(fp, "    \"%s\" -- \"%s\"\n",
			    to_room->area->name,
			    pRoomIndex->area->name);
		}

	    }
	}
    fprintf(fp, "}\n");

    fclose(fp);

    free_visited();
}

void do_graphviz(CHAR_DATA *ch, char *argument)
{
    if (!argument || !*argument)
    {
	graph_area( ch->in_room );

	send_to_char( "Done.\n\r", ch );
        return;
    }

    graph_mud();
    send_to_char("Done.\n\r", ch);
}
