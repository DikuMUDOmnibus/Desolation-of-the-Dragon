/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.0 (C) 1994, 1995, 1996 by Derek Snider             |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh and Tricops  |~'~.VxvxV.~'~*
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *	    Misc module for general commands: not skills or spells	    *
 ****************************************************************************
 * Note: Most of the stuff in here would go in act_obj.c, but act_obj was   *
 * getting big.								    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: misc.c,v 1.25 2004/04/06 22:00:10 dotd Exp $";*/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "gsn.h"

DECLARE_DO_FUN(do_quaff);

extern int	top_exit;
int ris_save( CHAR_DATA *ch, int percent, int ris );

/*
 * Fill a container
 * Many enhancements added by Thoric (ie: filling non-drink containers)
 */
void do_fill( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *source;
    sh_int    dest_item, src_item1, src_item2, src_item3;
    int       diff = 0;
    bool      all = FALSE;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    /* munch optional words */
    if ( (!str_cmp( arg2, "from" ) || !str_cmp( arg2, "with" ))
    &&    argument[0] != '\0' )
	argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Fill what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that item.\n\r", ch );
	return;
    }
    else
	dest_item = obj->item_type;

    src_item1 = src_item2 = src_item3 = -1;
    switch( dest_item )
    {
	default:
	  act( AT_ACTION, "$n tries to fill $p... (Don't ask me how)", ch, obj, NULL, TO_ROOM );
	  send_to_char( "You cannot fill that.\n\r", ch );
	  return;
	/* place all fillable item types here */
	case ITEM_DRINK_CON:
	  src_item1 = ITEM_FOUNTAIN;	src_item2 = ITEM_BLOOD;		break;
	case ITEM_HERB_CON:
	  src_item1 = ITEM_HERB;	src_item2 = ITEM_HERB_CON;	break;
	case ITEM_PIPE:
	  src_item1 = ITEM_HERB;	src_item2 = ITEM_HERB_CON;	
	  src_item3 = ITEM_DRINK_CON;	break;
	case ITEM_CONTAINER:
	  src_item1 = ITEM_CONTAINER;	src_item2 = ITEM_CORPSE_NPC;
	  src_item3 = ITEM_CORPSE_PC;	break;
    }

    if ( dest_item == ITEM_CONTAINER )
    {
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	{
	    act( AT_PLAIN, "The $d is closed.", ch, NULL, obj->name, TO_CHAR );
	    return;
	}
	if ( get_obj_weight( obj ) / obj->count
	>=   obj->value[0] )
	{
	   send_to_char( "It's already full as it can be.\n\r", ch );
	   return;
	}
    }
    else
    {
	diff = obj->value[0] - obj->value[1];
	if ( diff < 1 || obj->value[1] >= obj->value[0] )
	{
	    if ( !(dest_item == ITEM_PIPE && IS_SET( obj->value[3], PIPE_BONG ) && obj->value[4] < 25) )
	    {
		send_to_char( "It's already full as it can be.\n\r", ch );
		return;
	    }
	}
    }

    if ( dest_item == ITEM_PIPE
    &&   IS_SET( obj->value[3], PIPE_FULLOFASH ) )
    {
	send_to_char( "It's full of ashes, and needs to be emptied first.\n\r", ch );
	return;
    }

    if ( arg2[0] != '\0' )
    {
      if ( dest_item == ITEM_CONTAINER
      && (!str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 )) )
      {
	all = TRUE;
	source = NULL;
      }
      else
      /* This used to let you fill a pipe from an object on the ground.  Seems
         to me you should be holding whatever you want to fill a pipe with.
         It's nitpicking, but I needed to change it to get a mobprog to work
         right.  Check out Lord Fitzgibbon if you're curious.  -Narn */
      if ( dest_item == ITEM_PIPE )
      {
        if ( ( source = get_obj_carry( ch, arg2 ) ) == NULL )
	{
	   send_to_char( "You don't have that item.\n\r", ch );
	   return;
	}
	if ( source->item_type != src_item1 && source->item_type != src_item2
	&&   source->item_type != src_item3 )
	{
	   act( AT_PLAIN, "You cannot fill $p with $P!", ch, obj, source, TO_CHAR );
	   return;
	}
	if ( source->item_type == src_item3 && 
	     !IS_SET( obj->value[3], PIPE_BONG ) )
	{
	  send_to_char( "You can only fill bongs with liquid.\n\r", ch );
	  return;
	}
      }
      else
      {
	if ( ( source =  get_obj_here( ch, arg2 ) ) == NULL )
	{
	   send_to_char( "You cannot find that item.\n\r", ch );
	   return;
	}
      }
    }
    else
	source = NULL;

    if ( !source && dest_item == ITEM_PIPE )
    {
	send_to_char( "Fill it with what?\n\r", ch );
	return;
    }

    if ( !source )
    {
	bool      found = FALSE;
	OBJ_DATA *src_next;

	found = FALSE;
	separate_obj( obj );
	for ( source = ch->in_room->first_content;
	      source;
	      source = src_next )
	{
	    src_next = source->next_content;
	    if (dest_item == ITEM_CONTAINER)
	    {
		if ( !CAN_WEAR(source, ITEM_TAKE)
		||   (IS_OBJ_STAT( source, ITEM_PROTOTYPE) && !can_take_proto(ch))
                ||    carry_w(ch) + get_obj_weight(source) > can_carry_w(ch)
		||   (get_obj_weight(source) + get_obj_weight(obj)/obj->count)
		    > obj->value[0] )
		  continue;
		if ( all && arg2[3] == '.'
		&&  !nifty_is_name( &arg2[4], source->name ) )
		   continue;
		obj_from_room(source);
		if ( source->item_type == ITEM_MONEY )
		{
		   GET_MONEY(ch,source->value[2]) += source->value[0];
		   extract_obj( source );
		}
		else
		   obj_to_obj(source, obj);
		found = TRUE;
	    }
	    else
	    if (source->item_type == src_item1
	    ||  source->item_type == src_item2
	    ||  source->item_type == src_item3)
	    {
		found = TRUE;
		break;
	    }
	}
	if ( !found )
	{
	    switch( src_item1 )
	    {
		default:
		  send_to_char( "There is nothing appropriate here!\n\r", ch );
		  return;
		case ITEM_FOUNTAIN:
		  send_to_char( "There is no fountain or pool here!\n\r", ch );
		  return;
		case ITEM_BLOOD:
		  send_to_char( "There is no blood pool here!\n\r", ch );
		  return;
		case ITEM_HERB_CON:
		  send_to_char( "There are no herbs here!\n\r", ch );
		  return;
		case ITEM_HERB:
		  send_to_char( "You cannot find any smoking herbs.\n\r", ch );
		  return;
	    }
	}
	if (dest_item == ITEM_CONTAINER)
	{
	  act( AT_ACTION, "You fill $p.", ch, obj, NULL, TO_CHAR );
	  act( AT_ACTION, "$n fills $p.", ch, obj, NULL, TO_ROOM );
	  return;
	}
    }

    if (dest_item == ITEM_CONTAINER)
    {
	OBJ_DATA *otmp, *otmp_next;
	char name[MAX_INPUT_LENGTH];
	CHAR_DATA *gch;
	char *pd;
	bool found = FALSE;

	if ( source == obj )
	{
	    send_to_char( "You can't fill something with itself!\n\r", ch );
	    return;
	}

	switch( source->item_type )
	{
	    default:	/* put something in container */
		if ( !source->in_room	/* disallow inventory items */
		||   !CAN_WEAR(source, ITEM_TAKE)
		||   (IS_OBJ_STAT( source, ITEM_PROTOTYPE) && !can_take_proto(ch))
		||    carry_w(ch) + get_obj_weight(source) > can_carry_w(ch)
		||   (get_obj_weight(source) + get_obj_weight(obj)/obj->count)
		    > obj->value[0] )
		{
		    send_to_char( "You can't do that.\n\r", ch );
		    return;
		}
		separate_obj( obj );
		act( AT_ACTION, "You take $P and put it inside $p.", ch, obj, source, TO_CHAR );
		act( AT_ACTION, "$n takes $P and puts it inside $p.", ch, obj, source, TO_ROOM );
		obj_from_room(source);
		obj_to_obj(source, obj);
		break;
	    case ITEM_MONEY:
		send_to_char( "You can't do that... yet.\n\r", ch );
		break;
	    case ITEM_CORPSE_PC:
		if ( IS_NPC(ch) )
		{
		    send_to_char( "You can't do that.\n\r", ch );
		    return;
		}
		if ( !IS_OBJ_STAT( source, ITEM_CLANCORPSE )
		||   !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
		{
		    pd = source->short_descr;
		    pd = one_argument( pd, name );
		    pd = one_argument( pd, name );
		    pd = one_argument( pd, name );
		    pd = one_argument( pd, name );

		    if ( str_cmp( name, ch->name ) && !IS_IMMORTAL(ch) )
		    {
			bool fGroup;

			fGroup = FALSE;
			for ( gch = first_char; gch; gch = gch->next )
			{
			    if ( !IS_NPC(gch)
			    &&   is_same_group( ch, gch )
			    &&   !str_cmp( name, gch->name ) )
			    {
				fGroup = TRUE;
				break;
			    }
			}
			if ( !fGroup )
			{
			    send_to_char( "That's someone else's corpse.\n\r", ch );
			    return;
			}
		    }
		}
	    case ITEM_CONTAINER:
		if ( source->item_type == ITEM_CONTAINER  /* don't remove */
		&&   IS_SET(source->value[1], CONT_CLOSED) )
		{
		    act( AT_PLAIN, "The $d is closed.", ch, NULL, source->name, TO_CHAR );
		    return;
		}
	    case ITEM_CORPSE_NPC:
		if ( (otmp=source->first_content) == NULL )
		{
		    send_to_char( "It's empty.\n\r", ch );
		    return;
		}
		separate_obj( obj );
		for ( ; otmp; otmp = otmp_next )
		{
		    otmp_next = otmp->next_content;

		    if ( !CAN_WEAR(otmp, ITEM_TAKE)
		    ||   (IS_OBJ_STAT( otmp, ITEM_PROTOTYPE) && !can_take_proto(ch))
		    ||    carry_n(ch) + otmp->count > can_carry_n(ch)
		    ||    carry_w(ch) + get_obj_weight(otmp) > can_carry_w(ch)
		    ||   (get_obj_weight(source) + get_obj_weight(obj)/obj->count)
			> obj->value[0] )
			continue;
		    obj_from_obj(otmp);
		    obj_to_obj(otmp, obj);
		    found = TRUE;
		}
		if ( found )
		{
		   act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
		   act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
		}
		else
		   send_to_char( "There is nothing appropriate in there.\n\r", ch );
		break;
	}
	return;
    }

    if ( source->value[1] < 1 )
    {
	send_to_char( "There's none left!\n\r", ch );
	return;
    }
    if ( source->count > 1 && source->item_type != ITEM_FOUNTAIN )
      separate_obj( source );
    separate_obj( obj );

    switch( source->item_type )
    {
	default:
	  bug( "do_fill: got bad item type: %d", source->item_type );
	  send_to_char( "Something went wrong...\n\r", ch );
	  return;
	case ITEM_FOUNTAIN:
	  if ( obj->value[1] != 0 && obj->value[2] != 0 )
	  {
	     send_to_char( "There is already another liquid in it.\n\r", ch );
	     return;
	  }
	  obj->value[2] = 0;
	  obj->value[1] = obj->value[0];
	  act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
	  act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
	  return;
	case ITEM_BLOOD:
	  if ( obj->value[1] != 0 && obj->value[2] != 13 )
	  {
	     send_to_char( "There is already another liquid in it.\n\r", ch );
	     return;
	  }
	  obj->value[2] = 13;
	  if ( source->value[1] < diff )
	    diff = source->value[1];
	  obj->value[1] += diff;
	  act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
	  act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
	  if ( (source->value[1] -= diff) < 1 )
	  {
 	     extract_obj( source );
	     make_bloodstain( ch );
	  }
	  return;
	case ITEM_HERB:
	  if ( obj->value[1] != 0 && obj->value[2] != source->value[2] )
	  {
	     send_to_char( "There is already another type of herb in it.\n\r", ch );
	     return;
	  }
	  obj->value[2] = source->value[2];
	  if ( source->value[1] < diff )
	    diff = source->value[1];
	  obj->value[1] += diff;
	  act( AT_ACTION, "You fill $p with $P.", ch, obj, source, TO_CHAR );
	  act( AT_ACTION, "$n fills $p with $P.", ch, obj, source, TO_ROOM );
	  if ( (source->value[1] -= diff) < 1 )
 	     extract_obj( source );
	  return;
	case ITEM_HERB_CON:
	  if ( obj->value[1] != 0 && obj->value[2] != source->value[2] )
	  {
	     send_to_char( "There is already another type of herb in it.\n\r", ch );
	     return;
	  }
	  obj->value[2] = source->value[2];
	  if ( source->value[1] < diff )
	    diff = source->value[1];
	  obj->value[1] += diff;
	  source->value[1] -= diff;
	  act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
	  act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
	  return;
	case ITEM_DRINK_CON:
	  if ( obj->item_type == ITEM_PIPE )
	  {
	    if ( obj->value[4] != 0 && obj->value[5] != source->value[2] )
	    {
	       send_to_char( "There is already another liquid in it.\n\r", ch );
	       return;
	    }
	    obj->value[5] = source->value[2];
	    diff = UMIN(25-obj->value[4], source->value[1]);
	    obj->value[4] += diff;
	  }
	  else
	  {
	    if ( obj->value[1] != 0 && obj->value[2] != source->value[2] )
	    {
	       send_to_char( "There is already another liquid in it.\n\r", ch );
	       return;
	    }
	    obj->value[2] = source->value[2];
	    if ( source->value[1] < diff )
	      diff = source->value[1];
	    obj->value[1] += diff;
	  }
	  source->value[1] -= diff;
	  act( AT_ACTION, "You fill $p from $P.", ch, obj, source, TO_CHAR );
	  act( AT_ACTION, "$n fills $p from $P.", ch, obj, source, TO_ROOM );
	  return;
    }
}

void do_drink( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int amount;
    int liquid;

    argument = one_argument( argument, arg );
    /* munch optional words */
    if ( !str_cmp( arg, "from" ) && argument[0] != '\0' )
	argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	for ( obj = ch->in_room->first_content; obj; obj = obj->next_content )
	    if ( (obj->item_type == ITEM_FOUNTAIN)
	    ||   (obj->item_type == ITEM_BLOOD) )
		break;

	if ( !obj )
	{
	    send_to_char( "Drink what?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    if ( obj->count > 1 && obj->item_type != ITEM_FOUNTAIN )
	separate_obj(obj);

    if ( !IS_NPC(ch) && GET_COND(ch, COND_DRUNK) > MAX_COND_VAL/2 )
    {
	send_to_char( "You fail to reach your mouth.  *Hic*\n\r", ch );
	return;
    }

    switch ( obj->item_type )
    {
    default:
	if ( obj->carried_by == ch )
	{
	    act( AT_ACTION, "$n lifts $p up to $s mouth and tries to drink from it...", ch, obj, NULL, TO_ROOM );
	    act( AT_ACTION, "You bring $p up to your mouth and try to drink from it...", ch, obj, NULL, TO_CHAR );
	}
	else
	{
	    act( AT_ACTION, "$n gets down and tries to drink from $p... (Is $e feeling ok?)", ch, obj, NULL, TO_ROOM );
	    act( AT_ACTION, "You get down on the ground and try to drink from $p...", ch, obj, NULL, TO_CHAR );
	}
	break;

    case ITEM_POTION:
	if ( obj->carried_by == ch )
	   do_quaff( ch, obj->name );
	else
	   send_to_char( "You're not carrying that.\n\r", ch );
	break;

    case ITEM_BLOOD:
	if ( IS_VAMPIRE(ch) && !IS_NPC(ch) )
	{
	    if ( obj->timer > 0		/* if timer, must be spilled blood */
	    &&   GetMaxLevel(ch) > 5
	    &&   GET_BLOOD(ch) > (GET_MAX_BLOOD(ch)/4) )
	    {
		send_to_char( "It is above you to stoop to drinking blood from the ground!\n\r", ch );
		send_to_char( "Unless in dire need, you'd much rather have blood from a victim's neck!\n\r", ch );
		return;
	    }
	    if ( GET_BLOOD(ch) < GET_MAX_BLOOD(ch))
	    {
		if ( GET_COND(ch, COND_FULL) >= MAX_COND_VAL
		||   GET_COND(ch, COND_THIRST) >= MAX_COND_VAL )
		{
		    send_to_char( "You are too full to drink any blood.\n\r", ch );
		    return;
		}

		if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
		   act( AT_BLOOD, "$n drinks from the spilled blood.", ch, NULL, NULL, TO_ROOM );
		   set_char_color( AT_BLOOD, ch );
		   send_to_char( "You relish in the replenishment of this vital fluid...\n\r", ch );
		   if (obj->value[1] <=1) 
                   {
			set_char_color( AT_BLOOD, ch );
			send_to_char( "You drink the last drop of blood from the spill.\n\r", ch);
			act( AT_BLOOD, "$n drinks the last drop of blood from the spill.", ch, NULL, NULL, TO_ROOM );
		   }
		}

		gain_condition(ch, COND_BLOODTHIRST, 1);
		gain_condition(ch, COND_FULL, 1);
		gain_condition(ch, COND_THIRST, 1);
		if (--obj->value[1] <=0)
		{
		   extract_obj( obj );
		   make_bloodstain( ch );
		}
	    }
	    else
	      send_to_char( "Alas... you cannot consume any more blood.\n\r", ch );
	}
	else
	  send_to_char( "It is not in your nature to do such things.\n\r", ch );
	break;

    case ITEM_FOUNTAIN:
	if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
	{
	   act( AT_ACTION, "$n drinks from the fountain.", ch, NULL, NULL, TO_ROOM );
	   send_to_char( "You take a long thirst quenching drink.\n\r", ch );
	}

	if ( !IS_NPC(ch) )
	    ch->pcdata->condition[COND_THIRST] = MAX_COND_VAL;
	break;

    case ITEM_DRINK_CON:
	if ( obj->value[1] <= 0 )
	{
	    send_to_char( "It is already empty.\n\r", ch );
	    return;
	}

	if ( ( liquid = obj->value[2] ) >= LIQ_MAX )
	{
	    bug( "Do_drink: bad liquid number %d.", liquid );
	    liquid = obj->value[2] = 0;
	}

	if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
	{
	   act( AT_ACTION, "$n drinks $T from $p.",
		ch, obj, liq_table[liquid].liq_name, TO_ROOM );
	   act( AT_ACTION, "You drink $T from $p.",
		ch, obj, liq_table[liquid].liq_name, TO_CHAR );
	}

	amount = 1; /* UMIN(amount, obj->value[1]); */
	/* what was this? concentrated drinks?  concentrated water
	   too I suppose... sheesh! */

	gain_condition( ch, COND_DRUNK,
	    amount * liq_table[liquid].liq_affect[COND_DRUNK  ] );
	gain_condition( ch, COND_FULL,
	    amount * liq_table[liquid].liq_affect[COND_FULL   ] );
	gain_condition( ch, COND_THIRST,
	    amount * liq_table[liquid].liq_affect[COND_THIRST ] );

	if ( !IS_NPC(ch) )
	{
	    if ( GET_COND(ch, COND_DRUNK) > 24 )
		send_to_char( "You feel quite sloshed.\n\r", ch );
	    else
	    if ( GET_COND(ch, COND_DRUNK)  > 18 )
		send_to_char( "You feel very drunk.\n\r", ch );
	    else
	    if ( GET_COND(ch, COND_DRUNK)  > 12 )
		send_to_char( "You feel drunk.\n\r", ch );
	    else
	    if ( GET_COND(ch, COND_DRUNK)  > 8 )
		send_to_char( "You feel a little drunk.\n\r", ch );
	    else
	    if ( GET_COND(ch, COND_DRUNK)  > 5 )
		send_to_char( "You feel light headed.\n\r", ch );

	    if ( GET_COND(ch, COND_FULL)   > 40 )
		send_to_char( "You are full.\n\r", ch );

	    if ( GET_COND(ch, COND_THIRST) > 40 )
		send_to_char( "You feel bloated.\n\r", ch );
	    else
	    if ( GET_COND(ch, COND_THIRST) > 36 )
		send_to_char( "Your stomach is sloshing around.\n\r", ch );
	    else
	    if ( GET_COND(ch, COND_THIRST) > 30 )
		send_to_char( "You do not feel thirsty.\n\r", ch );
	}

        if ( obj->value[3] )
	{
	    /* The drink was poisoned! */
            AFFECT_DATA af;
            int saves;

            saves = ris_save(ch, GetMaxLevel(ch), RIS_POISON);

            if ( saves < 1000 && !saves_poison_death( saves, ch ) )
            {
                act( AT_POISON, "$n sputters and gags.", ch, NULL, NULL, TO_ROOM );
                act( AT_POISON, "You sputter and gag.", ch, NULL, NULL, TO_CHAR );
                ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
                af.type      = gsn_poison;
                af.duration  = 3 * obj->value[3];
                af.location  = APPLY_NONE;
                af.modifier  = 0;
                af.bitvector = AFF_POISON;
                affect_join( ch, &af );
            }
	}

	obj->value[1] -= amount;
	if ( obj->value[1] <= 0 )
	{
	    send_to_char( "The empty container vanishes.\n\r", ch );
	    extract_obj( obj );
	}
	break;
    }
    WAIT_STATE(ch, PULSE_PER_SECOND );
    return;
}

void do_eat( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    AFFECT_DATA *aff;
    ch_ret retcode;
    int foodcond;

    if ( argument[0] == '\0' )
    {
	send_to_char( "Eat what?\n\r", ch );
	return;
    }

    if ( IS_NPC(ch) || GET_COND(ch, COND_FULL) > 5 )
	if ( ms_find_obj(ch) )
	    return;

    if ( (obj = find_obj(ch, argument, TRUE)) == NULL )
	return;

    if ( !IS_IMMORTAL(ch) )
    {
	if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL )
	{
	    act( AT_ACTION, "$n starts to nibble on $p... ($e must really be hungry)",  ch, obj, NULL, TO_ROOM );
	    act( AT_ACTION, "You try to nibble on $p...", ch, obj, NULL, TO_CHAR );
	    return;
	}

	if ( !IS_NPC(ch) && GET_COND(ch, COND_FULL) > MAX_COND_VAL-8 )
	{
	    send_to_char( "You are too full to eat more.\n\r", ch );
	    return;
	}
    }

    /* required due to object grouping */
    separate_obj( obj );
    if IS_PKILL( ch )
        WAIT_STATE( ch, PULSE_PER_SECOND/4 );
    else
        WAIT_STATE( ch, PULSE_PER_SECOND/2 );
    if ( obj->in_obj )
    {
	act( AT_PLAIN, "You take $p from $P.", ch, obj, obj->in_obj, TO_CHAR );
	act( AT_PLAIN, "$n takes $p from $P.", ch, obj, obj->in_obj, TO_ROOM );
    }
    if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
    {
      if ( !actiondesc(ch,obj,NULL) )
      {
        act( AT_ACTION, "$n eats $p.",  ch, obj, NULL, TO_ROOM );
        act( AT_ACTION, "You eat $p.", ch, obj, NULL, TO_CHAR );
      }
    }

    switch ( obj->item_type )
    {

    case ITEM_FOOD:
	if ( obj->timer > 0 && obj->value[1] > 0 )
	   foodcond = (obj->timer * 10) / obj->value[1];
	else
	   foodcond = 10;

	if ( !IS_NPC(ch) )
	{
	    int condition;

	    condition = GET_COND(ch, COND_FULL);
	    gain_condition( ch, COND_FULL, (obj->value[0] * foodcond) / 10 );
	    if ( condition <= 1 && GET_COND(ch, COND_FULL) > 1 )
		send_to_char( "You are no longer hungry.\n\r", ch );
	    else if ( GET_COND(ch, COND_FULL) > MAX_COND_VAL-8 )
		send_to_char( "You are full.\n\r", ch );
	}

	if (  obj->value[3] != 0
	||   (foodcond < 4 && number_range( 0, foodcond + 1 ) == 0) )
	{
	    /* The food was poisoned! */
	    AFFECT_DATA af;

	    if ( obj->value[3] != 0 )
	    {
		act( AT_POISON, "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
		act( AT_POISON, "You choke and gag.", ch, NULL, NULL, TO_CHAR );
		ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
	    }
	    else
	    {
		act( AT_POISON, "$n gags on $p.", ch, obj, NULL, TO_ROOM );
		act( AT_POISON, "You gag on $p.", ch, obj, NULL, TO_CHAR );
		ch->mental_state = URANGE( 15, ch->mental_state + 5, 100 );
	    }

	    af.type      = gsn_poison;
	    af.duration  = 2 * obj->value[0]
	    		 * (obj->value[3] > 0 ? obj->value[3] : 1);
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = AFF_POISON;
	    affect_join( ch, &af );
	}

        break;

    case ITEM_PILL:
	/* allow pills to fill you, if so desired */
	if ( !IS_NPC(ch) && obj->value[4] )
	{
	    int condition;

	    condition = GET_COND(ch, COND_FULL);
	    gain_condition( ch, COND_FULL, obj->value[4] );
	    if ( condition <= 1 && GET_COND(ch, COND_FULL) > 1 )
		send_to_char( "You are no longer hungry.\n\r", ch );
	    else if ( GET_COND(ch, COND_FULL) > MAX_COND_VAL-4 )
		send_to_char( "You are full.\n\r", ch );
	}
	retcode = obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
	if ( retcode == rNONE )
	  retcode = obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
	if ( retcode == rNONE )
	  retcode = obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
	break;
    }

    retcode = rNONE;
    /*
    for ( aff = obj->pIndexData->first_affect; aff; aff = aff->next )
        if ( aff->location == APPLY_EAT_SPELL &&
             IS_VALID_SN(aff->modifier) &&
             skill_table[aff->modifier]->spell_fun )
        {
            retcode = obj_cast_spell(aff->modifier, GetMaxLevel(ch), ch, ch, obj);
            if ( retcode != rNONE || char_died(ch) )
                break;
        }
    */
    if ( retcode == rNONE && !char_died(ch) )
    {
        for ( aff = obj->first_affect; aff; aff = aff->next )
            if ( aff->location == APPLY_EAT_SPELL &&
                 IS_VALID_SN(aff->modifier) &&
                 skill_table[aff->modifier]->spell_fun )
            {
                retcode = obj_cast_spell(aff->modifier, GetMaxLevel(ch), ch, ch, obj);
                if ( retcode != rNONE || char_died(ch) )
                    break;
            }
    }

    extract_obj( obj );
    return;
}

void do_quaff( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    ch_ret retcode;

    if ( argument[0] == '\0' || !str_cmp(argument, "") )
    {
	send_to_char( "Quaff what?\n\r", ch );
	return;
    }

    if ( (obj = find_obj(ch, argument, TRUE)) == NULL )
	return;

    if ( obj->item_type != ITEM_POTION )
    {
	if ( obj->item_type == ITEM_DRINK_CON )
	   do_drink( ch, obj->name );
	else
	{
	   act( AT_ACTION, "$n lifts $p up to $s mouth and tries to drink from it...", ch, obj, NULL, TO_ROOM );
	   act( AT_ACTION, "You bring $p up to your mouth and try to drink from it...", ch, obj, NULL, TO_CHAR );
	}
	return;
    }

    /*
     * Fullness checking					-Thoric
     */
    if ( !IS_NPC(ch)
    && ( GET_COND(ch, COND_FULL) >= MAX_COND_VAL
    ||   GET_COND(ch, COND_THIRST) >= MAX_COND_VAL ) )
    {
	send_to_char( "Your stomach cannot contain any more.\n\r", ch );
	return;
    }

    separate_obj( obj );
    if ( obj->in_obj )
    {
	act( AT_PLAIN, "You take $p from $P.", ch, obj, obj->in_obj, TO_CHAR );
	act( AT_PLAIN, "$n takes $p from $P.", ch, obj, obj->in_obj, TO_ROOM );
    }

    /*
     * If fighting, chance of dropping potion			-Thoric
     */
    if ( ch->fighting && number_percent( ) > (get_curr_dex(ch) * 2 + 48) )
    {
	act( AT_MAGIC, "$n accidentally drops $p and it smashes into a thousand fragments.", ch, obj, NULL, TO_ROOM );
	act( AT_MAGIC, "Oops... $p gets knocked from your hands and smashes into pieces!", ch, obj, NULL ,TO_CHAR );
    }
    else
    {
	if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
	{
	    act( AT_ACTION, "$n quaffs $p.",  ch, obj, NULL, TO_ROOM );
	    act( AT_ACTION, "You quaff $p.", ch, obj, NULL, TO_CHAR );
	}

        if IS_PKILL( ch )
            WAIT_STATE( ch, PULSE_PER_SECOND/4 );
        else         
	    WAIT_STATE( ch, PULSE_PER_SECOND/2 );

	gain_condition( ch, COND_THIRST, 1 );
	retcode = obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
	if ( retcode == rNONE )
	  retcode = obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
	if ( retcode == rNONE )
	  retcode = obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
    }
    extract_obj( obj );
    return;
}


void do_recite( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *scroll;
    OBJ_DATA *obj;
    ch_ret    retcode;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Recite what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( ( scroll = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that scroll.\n\r", ch );
	return;
    }

    if ( scroll->item_type != ITEM_SCROLL )
    {
	act( AT_ACTION, "$n holds up $p as if to recite something from it...",  ch, scroll, NULL, TO_ROOM );
	act( AT_ACTION, "You hold up $p and stand there with your mouth open.  (Now what?)", ch, scroll, NULL, TO_CHAR );
	return;
    }

    if (scroll->vnum == OBJ_VNUM_SCROLL_SCRIBING)
    {
        if ( IS_NPC(ch) )
        {
            send_to_char( "As a mob, this dialect is foreign to you.\n\r", ch );
            return;
        }

        if (GetMaxLevel(ch) + 10 < scroll->value[0])
        {
            send_to_char( "This scroll is too complex for you to understand.\n\r", ch);
            return;
        }
    }

    if (!IS_ACTIVE(ch, CLASS_MAGE) &&
        !IS_ACTIVE(ch, CLASS_CLERIC) &&
        !IS_ACTIVE(ch, CLASS_DRUID) &&
        !IS_ACTIVE(ch, CLASS_SORCERER) &&
        !IS_ACTIVE(ch, CLASS_NECROMANCER))
    {
        if (gsn_read_magic && number_percent() > LEARNED(ch,gsn_read_magic))
        {
            send_to_char( "After several seconds of study, your head hurts trying to understand.\n\r", ch);
            if (GetMaxLevel(ch)<5)
                send_to_char("Note: You should learn read magic!\n\r(This message will self-destruct when you reach level 5)\n\r", ch);
            WAIT_STATE(ch, 3 * PULSE_VIOLENCE);
            return;
        }
    }

    obj = NULL;
    if ( arg2[0] == '\0' )
	victim = ch;
    else
    {
	if ( ( victim = get_char_room ( ch, arg2 ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "You can't find it.\n\r", ch );
	    return;
	}
    }

    separate_obj( scroll );
    act( AT_MAGIC, "$n recites $p.", ch, scroll, NULL, TO_ROOM );
    act( AT_MAGIC, "You recite $p.", ch, scroll, NULL, TO_CHAR );

    if ( victim != ch )
	WAIT_STATE( ch, 2 * PULSE_VIOLENCE );
    else if IS_PKILL( ch )
        WAIT_STATE( ch, PULSE_PER_SECOND/4 );
    else
        WAIT_STATE( ch, PULSE_PER_SECOND/2 );

    retcode = obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
    if ( retcode == rNONE )
      retcode = obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
    if ( retcode == rNONE )
      retcode = obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );

    extract_obj( scroll );
    return;
}


/*
 * Function to handle the state changing of a triggerobject (lever)  -Thoric
 */
void pullorpush( CHAR_DATA *ch, OBJ_DATA *obj, bool pull )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA		*rch;
    bool		 isup;
    ROOM_INDEX_DATA	*room,  *to_room = NULL;
    EXIT_DATA		*pexit, *pexit_rev;
    int			 edir;
    char		*txt;

    if ( IS_SET( obj->value[0], TRIG_UP ) )
      isup = TRUE;
    else
      isup = FALSE;
    switch( obj->item_type )
    {
	default:
	  sprintf( buf, "You can't %s that!\n\r", pull ? "pull" : "push" );
	  send_to_char( buf, ch );
	  return;
	  break;
	case ITEM_SWITCH:
	case ITEM_LEVER:
	case ITEM_PULLCHAIN:
	  if ( (!pull && isup) || (pull && !isup) )
	  {
		sprintf( buf, "It is already %s.\n\r", isup ? "up" : "down" );
		send_to_char( buf, ch );
		return;
 	  }
	case ITEM_BUTTON:
	  if ( (!pull && isup) || (pull & !isup) )
	  {
		sprintf( buf, "It is already %s.\n\r", isup ? "in" : "out" );
		send_to_char( buf, ch );
		return;
	  }
	  break;
    }
    if( (pull) && HAS_PROG(obj->pIndexData, PULL_PROG) )
    {
	if ( !IS_SET(obj->value[0], TRIG_AUTORETURN ) )
 	  REMOVE_BIT( obj->value[0], TRIG_UP );
 	oprog_pull_trigger( ch, obj );
        return;
    }
    if( (!pull) && HAS_PROG(obj->pIndexData, PUSH_PROG) )
    {
	if ( !IS_SET(obj->value[0], TRIG_AUTORETURN ) )
	  SET_BIT( obj->value[0], TRIG_UP );
	oprog_push_trigger( ch, obj );
        return;
    }

    if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
    {
      sprintf( buf, "$n %s $p.", pull ? "pulls" : "pushes" );
      act( AT_ACTION, buf,  ch, obj, NULL, TO_ROOM );
      sprintf( buf, "You %s $p.", pull ? "pull" : "push" );
      act( AT_ACTION, buf, ch, obj, NULL, TO_CHAR );
    }

    if ( !IS_SET(obj->value[0], TRIG_AUTORETURN ) )
    {
	if ( pull )
	  REMOVE_BIT( obj->value[0], TRIG_UP );
	else
	  SET_BIT( obj->value[0], TRIG_UP );
    }
    if ( IS_SET( obj->value[0], TRIG_TELEPORT )
    ||   IS_SET( obj->value[0], TRIG_TELEPORTALL )
    ||   IS_SET( obj->value[0], TRIG_TELEPORTPLUS ) )
    {
	int flags;

	if ( ( room = get_room_index( obj->value[1] ) ) == NULL )
	{
	    bug( "PullOrPush: obj points to invalid room %d", obj->value[1] );
	    return;
	}
	flags = 0;
	if ( IS_SET( obj->value[0], TRIG_SHOWROOMDESC ) )
	  SET_BIT( flags, TELE_SHOWDESC );
	if ( IS_SET( obj->value[0], TRIG_TELEPORTALL ) )
	  SET_BIT( flags, TELE_TRANSALL );
	if ( IS_SET( obj->value[0], TRIG_TELEPORTPLUS ) )
	  SET_BIT( flags, TELE_TRANSALLPLUS );

	teleport( ch, obj->value[1], flags );
	return;
    }

    if ( IS_SET( obj->value[0], TRIG_RAND4 )
    ||	 IS_SET( obj->value[0], TRIG_RAND6 ) )
    {
	int maxd;

	if ( ( room = get_room_index( obj->value[1] ) ) == NULL )
	{
	    bug( "PullOrPush: obj points to invalid room %d", obj->value[1] );
	    return;
	}

	if ( IS_SET( obj->value[0], TRIG_RAND4 ) )
	  maxd = 3;
	else
	  maxd = 5;

	randomize_exits( room, maxd );
	for ( rch = room->first_person; rch; rch = rch->next_in_room )
	{
	   send_to_char( "You hear a loud rumbling sound.\n\r", rch );
	   send_to_char( "Something seems different...\n\r", rch );
	}
    }
    if ( IS_SET( obj->value[0], TRIG_DOOR ) )
    {
        room = get_room_index( obj->value[1] );
	if ( !room )
	  room = obj->in_room;
	if ( !room )
	{
	  bug( "PullOrPush: obj points to invalid room %d", obj->value[1] );
	  return;
	}	
	if ( IS_SET( obj->value[0], TRIG_D_NORTH ) )
	{
	  edir = DIR_NORTH;
	  txt = "to the north";
	}
	else
	if ( IS_SET( obj->value[0], TRIG_D_SOUTH ) )
	{
	  edir = DIR_SOUTH;
	  txt = "to the south";
	}
	else
	if ( IS_SET( obj->value[0], TRIG_D_EAST ) )
	{
	  edir = DIR_EAST;
	  txt = "to the east";
	}
	else
	if ( IS_SET( obj->value[0], TRIG_D_WEST ) )
	{
	  edir = DIR_WEST;
	  txt = "to the west";
	}
	else
	if ( IS_SET( obj->value[0], TRIG_D_UP ) )
	{
	  edir = DIR_UP;
	  txt = "from above";
	}
	else
	if ( IS_SET( obj->value[0], TRIG_D_DOWN ) )
	{
	  edir = DIR_DOWN;
	  txt = "from below";
	}
	else
	{
	  bug( "PullOrPush: door: no direction flag set." );
	  return;
	}
	pexit = get_exit( room, edir );
	if ( !pexit )
	{
	    if ( !IS_SET( obj->value[0], TRIG_PASSAGE ) )
	    {
		bug( "PullOrPush: obj points to non-exit %d", obj->value[1] );
		return;
	    }
	    to_room = get_room_index( obj->value[2] );
	    if ( !to_room )
	    {
		bug( "PullOrPush: dest points to invalid room %d", obj->value[2] );
		return;
	    }
	    pexit = make_exit( room, to_room, edir );
	    pexit->keyword	= STRALLOC( "" );
	    pexit->description	= STRALLOC( "" );
	    pexit->key		= 0;
	    pexit->exit_info	= 0;
	    top_exit++;
	    act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_CHAR );
	    act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_ROOM );
	    return;
	}
	if ( IS_SET( obj->value[0], TRIG_UNLOCK )
	&&   IS_SET( pexit->exit_info, EX_LOCKED) )
	{
	    REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	    act( AT_PLAIN, "You hear a faint click $T.", ch, NULL, txt, TO_CHAR );
	    act( AT_PLAIN, "You hear a faint click $T.", ch, NULL, txt, TO_ROOM );
	    if ( ( pexit_rev = pexit->rexit ) != NULL
	    &&   pexit_rev->to_room == ch->in_room )
		REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
	    return;
	}
	if ( IS_SET( obj->value[0], TRIG_LOCK   )
	&&  !IS_SET( pexit->exit_info, EX_LOCKED) )
	{
	    SET_BIT(pexit->exit_info, EX_LOCKED);
	    act( AT_PLAIN, "You hear a faint click $T.", ch, NULL, txt, TO_CHAR );
	    act( AT_PLAIN, "You hear a faint click $T.", ch, NULL, txt, TO_ROOM );
	    if ( ( pexit_rev = pexit->rexit ) != NULL
	    &&   pexit_rev->to_room == ch->in_room )
		SET_BIT( pexit_rev->exit_info, EX_LOCKED );
	    return;
	}
	if ( IS_SET( obj->value[0], TRIG_OPEN   )
	&&   IS_SET( pexit->exit_info, EX_CLOSED) )
	{
	    REMOVE_BIT(pexit->exit_info, EX_CLOSED);
	    for ( rch = room->first_person; rch; rch = rch->next_in_room )
		act( AT_ACTION, "The $d opens.", rch, NULL, pexit->keyword, TO_CHAR );
	    if ( ( pexit_rev = pexit->rexit ) != NULL
	    &&   pexit_rev->to_room == ch->in_room )
	    {
		REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
		for ( rch = pexit_rev->to_room->first_person; rch; rch = rch->next_in_room )
		    act( AT_ACTION, "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	    }
	    check_room_for_traps( ch, trap_door[edir]);
	    return;
	}
	if ( IS_SET( obj->value[0], TRIG_CLOSE   )
	&&  !IS_SET( pexit->exit_info, EX_CLOSED) )
	{
	    SET_BIT(pexit->exit_info, EX_CLOSED);
	    for ( rch = room->first_person; rch; rch = rch->next_in_room )
		act( AT_ACTION, "The $d closes.", rch, NULL, pexit->keyword, TO_CHAR );
	    if ( ( pexit_rev = pexit->rexit ) != NULL
	    &&   pexit_rev->to_room == ch->in_room )
	    {
		SET_BIT( pexit_rev->exit_info, EX_CLOSED );
		for ( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )
		    act( AT_ACTION, "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	    }
	    check_room_for_traps( ch, trap_door[edir]);
	    return;
	}
    }
}


void do_pull( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Pull what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
    {
	act( AT_PLAIN, "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return;
    }

    pullorpush( ch, obj, TRUE );
}

void do_push( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Push what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
    {
	act( AT_PLAIN, "I see no $T here.", ch, NULL, arg, TO_CHAR );
	return;
    }

    pullorpush( ch, obj, FALSE );
}

/* pipe commands (light, tamp, smoke) by Thoric */
void do_tamp( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *pipe;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Tamp what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( (pipe = get_obj_carry( ch, arg )) == NULL )
    {
	send_to_char( "You aren't carrying that.\n\r", ch );
	return;
    }
    if ( pipe->item_type != ITEM_PIPE )
    {
	send_to_char( "You can't tamp that.\n\r", ch );
	return;
    }
    if ( !IS_SET( pipe->value[3], PIPE_TAMPED ) )
    {
	act( AT_ACTION, "You gently tamp $p.", ch, pipe, NULL, TO_CHAR );
	act( AT_ACTION, "$n gently tamps $p.", ch, pipe, NULL, TO_ROOM );
	SET_BIT( pipe->value[3], PIPE_TAMPED );
	return;
    }
    send_to_char( "It doesn't need tamping.\n\r", ch );
}

void do_smoke( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *pipe;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Smoke what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( (pipe = get_obj_carry( ch, arg )) == NULL )
    {
	send_to_char( "You aren't carrying that.\n\r", ch );
	return;
    }
    if ( pipe->item_type != ITEM_PIPE )
    {
	act( AT_ACTION, "You try to smoke $p... but it doesn't seem to work.", ch, pipe, NULL, TO_CHAR );
	act( AT_ACTION, "$n tries to smoke $p... (I wonder what $e's been putting his $s pipe?)", ch, pipe, NULL, TO_ROOM );
	return;
    }
    if ( !IS_SET( pipe->value[3], PIPE_LIT ) )
    {
	act( AT_ACTION, "You try to smoke $p, but it's not lit.", ch, pipe, NULL, TO_CHAR );
	act( AT_ACTION, "$n tries to smoke $p, but it's not lit.", ch, pipe, NULL, TO_ROOM );
	return;
    }
    if ( pipe->value[1] > 0 )
    {
	if ( !oprog_use_trigger( ch, pipe, NULL, NULL, NULL ) )
	{
	   act( AT_ACTION, "You draw thoughtfully from $p.", ch, pipe, NULL, TO_CHAR );
	   act( AT_ACTION, "$n draws thoughtfully from $p.", ch, pipe, NULL, TO_ROOM );
	}

	if ( IS_VALID_HERB( pipe->value[2] ) && pipe->value[2] < top_herb )
	{
	    int sn		= pipe->value[2] + TYPE_HERB;
	    SKILLTYPE *skill	= get_skilltype( sn );

            spell_lag(ch, sn);
	    if ( skill->spell_fun )
		obj_cast_spell( sn, UMIN(skill->min_level, GetMaxLevel(ch)),
			ch, ch, NULL );
	    if ( obj_extracted( pipe ) )
		return;
	}
	else
	    bug( "do_smoke: bad herb type %d", pipe->value[2] );

	SET_BIT( pipe->value[3], PIPE_HOT );

	if ( IS_SET( pipe->value[3], PIPE_BONG ) && --pipe->value[4] < 1 &&
	     pipe->value[1] >= 1 )
	{
	    REMOVE_BIT( pipe->value[3], PIPE_LIT );
	    REMOVE_BIT( pipe->value[3], PIPE_TAMPED );
	}
	if ( --pipe->value[1] < 1 )
	{
	    REMOVE_BIT( pipe->value[3], PIPE_LIT );
	    REMOVE_BIT( pipe->value[3], PIPE_TAMPED );
	    SET_BIT( pipe->value[3], PIPE_DIRTY );
	    SET_BIT( pipe->value[3], PIPE_FULLOFASH );
	    if ( IS_SET( pipe->value[3], PIPE_SINGLE_USE ) )
	    {
		send_to_char( "You smoke it all up.\n\r", ch );
		extract_obj( pipe );
	    }
	}
    }
}

void do_light( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *pipe;
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Light what?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( (pipe = get_obj_carry( ch, arg )) == NULL )
    {
	send_to_char( "You aren't carrying that.\n\r", ch );
	return;
    }
    if ( pipe->item_type != ITEM_PIPE )
    {
	send_to_char( "You can't light that.\n\r", ch );
	return;
    }
    if ( !IS_SET( pipe->value[3], PIPE_LIT ) )
    {
	if ( pipe->value[1] < 1 )
	{
	  act( AT_ACTION, "You try to light $p, but it's empty.", ch, pipe, NULL, TO_CHAR );
	  act( AT_ACTION, "$n tries to light $p, but it's empty.", ch, pipe, NULL, TO_ROOM );
	  return;
	}
	if ( IS_SET( pipe->value[3], PIPE_BONG ) && pipe->value[4] < 1 )
	{
	  act( AT_ACTION, "You try to light $p, but it has no water in it.", ch, pipe, NULL, TO_CHAR );
	  act( AT_ACTION, "$n tries to light $p, but it has no water in it.", ch, pipe, NULL, TO_ROOM );
	  return;
	}
	act( AT_ACTION, "You carefully light $p.", ch, pipe, NULL, TO_CHAR );
	act( AT_ACTION, "$n carefully lights $p.", ch, pipe, NULL, TO_ROOM );
	SET_BIT( pipe->value[3], PIPE_LIT );
	return;
    }
    send_to_char( "It's already lit.\n\r", ch );
}

void do_empty( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( !str_cmp( arg2, "into" ) && argument[0] != '\0' )
	argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Empty what?\n\r", ch );
	return;
    }
    if ( ms_find_obj(ch) )
	return;

    if ( (obj = get_obj_carry( ch, arg1 )) == NULL )
    {
	send_to_char( "You aren't carrying that.\n\r", ch );
	return;
    }
    if ( obj->count > 1 )
      separate_obj(obj);

    switch( obj->item_type )
    {
	default:
	  act( AT_ACTION, "You shake $p in an attempt to empty it...", ch, obj, NULL, TO_CHAR );
	  act( AT_ACTION, "$n begins to shake $p in an attempt to empty it...", ch, obj, NULL, TO_ROOM );
	  return;
	case ITEM_PIPE:
	  act( AT_ACTION, "You gently tap $p and empty it out.", ch, obj, NULL, TO_CHAR );
	  act( AT_ACTION, "$n gently taps $p and empties it out.", ch, obj, NULL, TO_ROOM );
	  REMOVE_BIT( obj->value[3], PIPE_FULLOFASH );
	  REMOVE_BIT( obj->value[3], PIPE_LIT );
	  obj->value[1] = 0;
	  return;
	case ITEM_DRINK_CON:
	  if ( obj->value[1] < 1 )
	  {
		send_to_char( "It's already empty.\n\r", ch );
		return;
	  }
	  act( AT_ACTION, "You empty $p.", ch, obj, NULL, TO_CHAR );
	  act( AT_ACTION, "$n empties $p.", ch, obj, NULL, TO_ROOM );
	  obj->value[1] = 0;
	  return;
	case ITEM_CONTAINER:
	  if ( IS_SET(obj->value[1], CONT_CLOSED) )
	  {
		act( AT_PLAIN, "The $d is closed.", ch, NULL, obj->name, TO_CHAR );
		return;
	  }
	  if ( !obj->first_content )
	  {
		send_to_char( "It's already empty.\n\r", ch );
		return;
	  }
	  if ( arg2[0] == '\0' )
	  {
		if ( IS_SET( ch->in_room->room_flags, ROOM_NODROP )
		||   (!IS_NPC(ch) && IS_PLR_FLAG( ch, PLR_LITTERBUG )) )
		{
		       set_char_color( AT_MAGIC, ch );
		       send_to_char( "A magical force stops you!\n\r", ch );
		       set_char_color( AT_TELL, ch );
		       send_to_char( "Someone tells you, 'No littering here!'\n\r", ch );
		       return;
		}
		if ( IS_SET( ch->in_room->room_flags, ROOM_NODROPALL ) 
		||   IS_SET( ch->in_room->room_flags, ROOM_CLANSTOREROOM ) )
		{
		   send_to_char( "You can't seem to do that here...\n\r", ch );
		   return;
		}
		if ( empty_obj( obj, NULL, ch->in_room ) )
		{
		    act( AT_ACTION, "You empty $p.", ch, obj, NULL, TO_CHAR );
		    act( AT_ACTION, "$n empties $p.", ch, obj, NULL, TO_ROOM );
		    if ( IS_SET( sysdata.save_flags, SV_DROP ) )
			save_char_obj( ch );
		}
		else
		    send_to_char( "Hmmm... didn't work.\n\r", ch );
	  }
	  else
	  {
		OBJ_DATA *dest = get_obj_here( ch, arg2 );

		if ( !dest )
		{
		    send_to_char( "You can't find it.\n\r", ch );
		    return;
		}
		if ( dest == obj )
		{
		    send_to_char( "You can't empty something into itself!\n\r", ch );
		    return;
		}
		if ( dest->item_type != ITEM_CONTAINER )
		{
		    send_to_char( "That's not a container!\n\r", ch );
		    return;
		}
		if ( IS_SET(dest->value[1], CONT_CLOSED) )
		{
		    act( AT_PLAIN, "The $d is closed.", ch, NULL, dest->name, TO_CHAR );
		    return;
		}
		separate_obj( dest );
		if ( empty_obj( obj, dest, NULL ) )
		{
		    act( AT_ACTION, "You empty $p into $P.", ch, obj, dest, TO_CHAR );
		    act( AT_ACTION, "$n empties $p into $P.", ch, obj, dest, TO_ROOM );
		    if ( !dest->carried_by
		    &&    IS_SET( sysdata.save_flags, SV_PUT ) )
			save_char_obj( ch );
		}
		else
		    act( AT_ACTION, "$P is too full.", ch, obj, dest, TO_CHAR );
	  }
	  return;
    }
}
 
/*
 * Apply a salve/ointment					-Thoric
 * Support for applying to others.  Pkill concerns dealt with elsewhere.
 */
void do_apply( CHAR_DATA *ch, char *argument )
{

    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *salve;
    OBJ_DATA *obj;
    ch_ret    retcode;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Apply what?\n\r", ch );
	return;
    }
    if ( ch->fighting )
    {
	send_to_char( "You're too busy fighting ...\n\r", ch );
	return;
    }
    if ( ms_find_obj(ch) )
	return;
    if ( ( salve = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "You do not have that.\n\r", ch );
	return;
    }

    obj = NULL;
    if ( arg2[0] == '\0' )
	victim = ch;
    else
    {
	if ( ( victim = get_char_room ( ch, arg2 ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "Apply it to what or who?\n\r", ch );
	    return;
	}
   }

   /* apply salve to another object */
   if ( obj )
   {
	send_to_char( "You can't do that... yet.\n\r", ch );
	return;
   }

   if ( victim->fighting )
   {
	send_to_char( "Wouldn't work very well while they're fighting ...\n\r", ch );
	return;
   }

   if ( salve->item_type != ITEM_SALVE )
   {
	if ( victim == ch )
	{
	    act( AT_ACTION, "$n starts to rub $p on $mself...", ch, salve, NULL, TO_ROOM );
            act( AT_ACTION, "You try to rub $p on yourself...", ch, salve, NULL, TO_CHAR );
	}
	else
	{
	    act( AT_ACTION, "$n starts to rub $p on $N...", ch, salve, victim, TO_NOTVICT );
	    act( AT_ACTION, "$n starts to rub $p on you...", ch, salve, victim, TO_VICT );
	    act( AT_ACTION, "You try to rub $p on $N...", ch, salve, victim, TO_CHAR );
	}
        return;
    }        
    separate_obj( salve );
    --salve->value[1];

    if ( !oprog_use_trigger( ch, salve, NULL, NULL, NULL ) )
    {
        if ( !actiondesc(ch, salve, NULL) )
        {
            if ( salve->value[1] < 1 )
            {
                if ( victim != ch )
                {
                    act( AT_ACTION, "$n rubs the last of $p onto $N.",
                         ch, salve, victim, TO_NOTVICT );
                    act( AT_ACTION, "$n rubs the last of $p onto you.",
                         ch, salve, victim, TO_VICT );
                    act( AT_ACTION, "You rub the last of $p onto $N.",
                         ch, salve, victim, TO_CHAR );
                }
                else
                {
                    act( AT_ACTION, "You rub the last of $p onto yourself.",
                         ch, salve, NULL, TO_CHAR );
                    act( AT_ACTION, "$n rubs the last of $p onto $mself.",
                         ch, salve, NULL, TO_ROOM );
                }
            }
            else
            {
                if ( victim != ch )
                {
                    act( AT_ACTION, "$n rubs $p onto $N.",
                         ch, salve, victim, TO_NOTVICT );
                    act( AT_ACTION, "$n rubs $p onto you.",
                         ch, salve, victim, TO_VICT );
                    act( AT_ACTION, "You rub $p onto $N.",
                         ch, salve, victim, TO_CHAR );
                }
                else
                {
                    act( AT_ACTION, "You rub $p onto yourself.",
                         ch, salve, NULL, TO_CHAR );
                    act( AT_ACTION, "$n rubs $p onto $mself.",
                         ch, salve, NULL, TO_ROOM );
                }                                   
            }
        }
    }
    
    WAIT_STATE( ch, salve->value[3] );
    retcode = obj_cast_spell( salve->value[4], salve->value[0], ch, victim, NULL );
    if ( retcode == rNONE )
	retcode = obj_cast_spell( salve->value[5], salve->value[0], ch, victim, NULL );
    if ( retcode == rCHAR_DIED || retcode == rBOTH_DIED )
    {
	bug( "do_apply:  char died" );
        return;
    }    

    if ( !obj_extracted(salve) && salve->value[1] <= 0 )
	extract_obj( salve );
    return;
}

bool actiondesc( CHAR_DATA *ch, OBJ_DATA *obj, void *vo )
{
    char charbuf[MAX_STRING_LENGTH];
    char roombuf[MAX_STRING_LENGTH];
    char *srcptr = obj->action_desc;
    char *charptr = charbuf;
    char *roomptr = roombuf;
    const char *ichar = NULL;
    const char *iroom = NULL;

    if ( !obj->action_desc || obj->action_desc[0]=='\0' )
        return FALSE;

    while ( *srcptr != '\0' )
    {
        if ( *srcptr == '$' ) 
        {
            srcptr++;
            switch ( *srcptr )
            {
            case 'e':
                ichar = "you";
                iroom = "$e";
                break;
                
            case 'm':
                ichar = "you";
                iroom = "$m";
                break;
                
            case 'n':
                ichar = "you";
                iroom = "$n";
                break;
                
            case 's':
                ichar = "your";
                iroom = "$s";
                break;
                
                /*case 'q':
                 iroom = "s";
                 break;*/
                
            default: 
                srcptr--;
                *charptr++ = *srcptr;
                *roomptr++ = *srcptr;
                break;
            }
        }
        else if ( *srcptr == '%' && *++srcptr == 's' ) 
        {
            ichar = "You";
            iroom = IS_NPC( ch ) ? ch->short_descr : ch->name;
        }
        else
        {
            *charptr++ = *srcptr;
            *roomptr++ = *srcptr;
            srcptr++;
            continue;
        }
        
        while ( ( *charptr = *ichar ) != '\0' )
        {
            charptr++;
            ichar++;
        }
        
        while ( ( *roomptr = *iroom ) != '\0' )
        {
            roomptr++;
            iroom++;
        }
        srcptr++;
    }
    
    *charptr = '\0';
    *roomptr = '\0';
    
    /*
     sprintf( log_buf, "Charbuf: %s", charbuf );
     log_string_plus( log_buf, LOG_HIGH, LEVEL_LESSER );
     sprintf( log_buf, "Roombuf: %s", roombuf );
     log_string_plus( log_buf, LOG_HIGH, LEVEL_LESSER );
     */
    
    switch( obj->item_type )
    {
    case ITEM_BLOOD:
    case ITEM_FOUNTAIN:
        act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
        act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
        return TRUE;
        
    case ITEM_DRINK_CON:
        act( AT_ACTION, charbuf, ch, obj, liq_table[obj->value[2]].liq_name, TO_CHAR );
        act( AT_ACTION, roombuf, ch, obj, liq_table[obj->value[2]].liq_name, TO_ROOM );
        return TRUE;
        
    case ITEM_PIPE:
        return FALSE;
        
    case ITEM_ARMOR:
    case ITEM_WEAPON:
    case ITEM_LIGHT:
    case ITEM_FOOD:
    case ITEM_PILL:
        act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );
        act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );
        return TRUE;
    }

    return FALSE;
}

/*
 * Extended Bitvector Routines					-Thoric
 */

/* check to see if the extended bitvector is completely empty */
bool ext_is_empty( EXT_BV *bits )
{
    int x;
    
    for ( x = 0; x < XBI; x++ )
        if ( bits->bits[x] != 0 )
            return FALSE;
    
    return TRUE;
}

void ext_clear_bits( EXT_BV *bits )
{
    int x;
    
    for ( x = 0; x < XBI; x++ )
        bits->bits[x] = 0;
}

/* for use by xHAS_BITS() -- works like IS_SET() */
int ext_has_bits( EXT_BV *var, EXT_BV *bits )
{
    int x, bit;
    
    for ( x = 0; x < XBI; x++ )
        if ( (bit=(var->bits[x] & bits->bits[x])) != 0 )
            return bit;
    
    return 0;
}

/* for use by xSAME_BITS() -- works like == */
bool ext_same_bits( EXT_BV *var, EXT_BV *bits )
{
    int x;
    
    for ( x = 0; x < XBI; x++ )
        if ( var->bits[x] != bits->bits[x] )
            return FALSE;
    
    return TRUE;
}

/* for use by xSET_BITS() -- works like SET_BIT() */
void ext_set_bits( EXT_BV *var, EXT_BV *bits )
{
    int x;
    
    for ( x = 0; x < XBI; x++ )
        var->bits[x] |= bits->bits[x];
}

/* for use by xREMOVE_BITS() -- works like REMOVE_BIT() */
void ext_remove_bits( EXT_BV *var, EXT_BV *bits )
{
    int x;
    
    for ( x = 0; x < XBI; x++ )
        var->bits[x] &= ~(bits->bits[x]);
}

/* for use by xTOGGLE_BITS() -- works like TOGGLE_BIT() */
void ext_toggle_bits( EXT_BV *var, EXT_BV *bits )
{
    int x;
    
    for ( x = 0; x < XBI; x++ )
        var->bits[x] ^= bits->bits[x];
}

/*
 * Read an extended bitvector from a file.			-Thoric
 */
EXT_BV fread_bitvector( FILE *fp )
{
    EXT_BV ret;
    int c, x = 0;
    int num = 0;
    
    memset( &ret, '\0', sizeof(ret) );
    for ( ;; )
    {
        num = fread_number(fp);
        if ( x < XBI )
            ret.bits[x] = num;
        ++x;
        if ( (c=getc(fp)) != '&' )
        {
            ungetc(c, fp);
            break;
        }
    }
    
    return ret;
}

/* return a string for writing a bitvector to a file */
char *print_bitvector( EXT_BV *bits )
{
    static char buf[XBI * 12];
    char *p = buf;
    int x, cnt = 0;
    
    for ( cnt = XBI-1; cnt > 0; cnt-- )
        if ( bits->bits[cnt] )
            break;
    for ( x = 0; x <= cnt; x++ )
    {
        sprintf(p, "%d", bits->bits[x]);
        p += strlen(p);
        if ( x < cnt )
            *p++ = '&';
    }
    *p = '\0';
    
    return buf;
}

/*
 * Write an extended bitvector to a file			-Thoric
 */
void fwrite_bitvector( EXT_BV *bits, FILE *fp )
{
    fputs( print_bitvector(bits), fp );
}


EXT_BV meb( int bit )
{
    EXT_BV bits;
    
    xCLEAR_BITS(bits);
    if ( bit >= 0 )
        xSET_BIT(bits, bit);
    
    return bits;
}


EXT_BV multimeb( int bit, ... )
{
    EXT_BV bits;
    va_list param;
    int b;
    
    xCLEAR_BITS(bits);
    if ( bit < 0 )
        return bits;
    
    xSET_BIT(bits, bit);
    
    va_start(param, bit);
    
    while ((b = va_arg(param, int)) != -1)
        xSET_BIT(bits, b);
    
    va_end(param);
    
    return bits;
}

EXT_BV convert_bit( int bit )
{
    EXT_BV bits;
    int x;

    for (x=0;x<32;x++)
	if (IS_SET(bit,(1 << x)))
	    xSET_BIT(bits,x);

    return bits;
}

void do_races(CHAR_DATA *ch, char *argument)
{
    int x;

    if ( !str_cmp( argument, "choices" ) )
    {
        char *race_choices(void);
        send_to_char(race_choices(), ch);
        return;
    }

    pager_printf(ch, "%-16.16s  %3s  %3s  %3s  %3s  %3s  %3s  %3s\n\r",
                 "Race Name", "Str", "Wis", "Int", "Con", "Dex", "Cha", "Lck");
    for (x=0;x<MAX_RACE;x++)
    {
        pager_printf(ch, "%3d) %-16.16s  %3d  %3d  %3d  %3d  %3d  %3d  %3d\n\r",
                     x,
                     race_table[x].race_name,
                     race_table[x].str_plus,
                     race_table[x].wis_plus,
                     race_table[x].int_plus,
                     race_table[x].con_plus,
                     race_table[x].dex_plus,
                     race_table[x].cha_plus,
                     race_table[x].lck_plus
                    );
    }

}

void do_multiclasses(CHAR_DATA *ch, char *argument)
{
    char *class_choices(int chrace);

    if (!argument || !*argument)
    {
        send_to_char("Give us a race or we can't show you a list of classes for it.\n\r", ch);
        return;
    }



    send_to_char(class_choices(RACE_HUMAN), ch);
}

void do_ch_classes(CHAR_DATA *ch, char *argument)
{
    int x;
    
    pager_printf(ch, "%-16.16s  %5s  %5s  %5s  %5s  %5s  %9s  %9s\n\r",
                 "Name", "HPmin", "HPmax", "HPlev", "HPadd", "Mana", "XPbase", "XPpower");
    for ( x = 0; x < MAX_CLASS; x++ )
    {
        pager_printf(ch, "%-16.16s  %5d  %5d  %5d  %5d  %5d  %9ld  %9ld\n\r",
                     class_table[x]->who_name,
                     class_table[x]->hp_min,
                     class_table[x]->hp_max,
                     class_table[x]->hp_const_lev,
                     class_table[x]->hp_const_add,
                     class_table[x]->fMana,
                     class_table[x]->exp_base,
                     class_table[x]->exp_power
                    );
    }
}
