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
 *			     Mud constants module			    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: const.c,v 1.29 2003/09/03 00:38:10 dotd Exp $";*/

#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "mud.h"

/* undef these at EOF */
#define AM 95
#define AC 95
#define AT 85
#define AW 85
#define AV 95
#define AD 95
#define AR 90
#define AA 95

char * const lang_names[] =
{
    "common", "elvish", "dwarven", "pixie", "ogre",
    "orcish", "trollese", "rodent", "insectoid",
    "mammal", "reptile", "dragon", "spiritual",
    "magical", "goblin", "god", "ancient",
    "halfling", "clan", "gith", "gnomish",
    "mindflayer", "aarakocra", "giantish"
};
const int lang_array[] =
{
    LANG_COMMON, LANG_ELVEN, LANG_DWARVEN, LANG_PIXIE,
    LANG_OGRE, LANG_ORCISH, LANG_TROLLISH, LANG_RODENT,
    LANG_INSECTOID, LANG_MAMMAL, LANG_REPTILE,
    LANG_DRAGON, LANG_SPIRITUAL, LANG_MAGICAL,
    LANG_GOBLIN, LANG_GOD, LANG_ANCIENT, LANG_HALFLING,
    LANG_CLAN, LANG_GITH, LANG_GNOMISH, LANG_MINDFLAYER,
    LANG_AARAKOCRA, LANG_GIANTISH, LANG_UNKNOWN
};


/*
 * Race table.
 */

/*
race name   DEF_AFF st dx ws in cn ch lk hp mn re su RESTRICTION LANGUAGE is_pc body_parts
*/
const	struct	race_type	race_table	[MAX_RACE]	=
{
{ "Mutt", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Human", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 1, 0 },
{ "Grey Elf", AFF_INFRARED, 0, 0, 0, 1, 0, 0, 0, 0, 10, 0, 0, 0, LANG_ELVEN, 1, 0 },
{ "Dwarven", AFF_INFRARED, 0, 0, 0, 0, 1, -1, 0, 0, 0, 0, 0, 0, LANG_DWARVEN, 1, 0 },
{ "Halfling", AFF_INFRARED, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_HALFLING, 1, 0 },
{ "Gnome", AFF_INFRARED, -1, 0, 0, 2, 0, 0, -1, 0, 0, 0, 0, 0, LANG_PIXIE, 1, 0 },
{ "Reptilian", 0, 0, 2, 0, -2, 0, 0, 0, 0, 0, 0, 0, 0, LANG_REPTILE, 0, 0 },
{ "Mysterion", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_MAGICAL, 0, 0 },
{ "Lycanthropian", AFF_INFRARED, 2, 0, -2, 0, 0, 0, 0, 0, 0, RIS_NONMAGIC, 0, 0, LANG_COMMON, 0, 0 },
{ "Draconian", AFF_FLYING+AFF_INFRARED, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_DRAGON, 0, 0 },
{ "Undead", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, RIS_COLD+RIS_DRAIN+RIS_SLEEP+RIS_CHARM, RIS_FIRE, 0, LANG_ANCIENT, 0, 0 },
{ "Orcish", AFF_INFRARED, 1, 0, 0, 0, 1, -2, 0, 0, 0, 0, 0, 0, LANG_ORCISH, 1, 0 },
{ "Insectoid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_INSECTOID, 0, 0 },
{ "Arachnoid", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_INSECTOID, 0, 0 },
{ "Saurian", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_REPTILE, 0, 0 },
{ "Icthyiod", AFF_AQUA_BREATH, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_MAMMAL, 0, 0 },
{ "Avian", AFF_FLYING, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_MAMMAL, 0, 0 },
{ "Giant", AFF_INFRARED, 2, -1, -1, -1, 1, 0, 0, 10, 0, 0, 0, 0, LANG_OGRE, 0, 0 },
{ "Carnivorous", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_MAMMAL, 0, 0 },
{ "Parasitic", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_INSECTOID, 0, 0 },
{ "Slime", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_INSECTOID, 0, 0 },
{ "Demonic", AFF_DETECT_INVIS+AFF_DETECT_MAGIC, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_SPIRITUAL, 0, 0 },
{ "Snake", AFF_INFRARED, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_REPTILE, 0, PART_HEAD|PART_HEART|PART_BRAINS|PART_GUTS|PART_EYE|PART_TAIL|PART_SCALES },
{ "Herbivorous", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_MAMMAL, 0, 0 },
{ "Tree", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_MAGICAL, 0, 0 },
{ "Vegan", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_MAGICAL, 0, 0 },
{ "Elemental", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_MAGICAL, 0 , 0},
{ "Planar", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_GITH, 0, 0 },
{ "Diabolic", AFF_INFRARED, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_GITH, 0, 0 },
{ "Ghostly", AFF_FLOATING+AFF_PASS_DOOR+AFF_INVISIBLE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_SPIRITUAL, 0, 0 },
{ "Goblinoid", AFF_INFRARED, -2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_GOBLIN, 1, 0 },
{ "Trollish", AFF_INFRARED, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_TROLLISH, 1, 0 },
{ "Vegman", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_MAGICAL, 0, 0 },
{ "Mindflayer", AFF_DETECT_MAGIC, -1, -1, 2, 2, -1, -1, 0, 0, 10, 0, 0, 0, LANG_GITH, 1, 0 },
{ "Primate", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_MAMMAL, 0, 0 },
{ "Enfan", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_OGRE, 0, 0 },
{ "Drow", AFF_INFRARED, 0, 2, 0, 0, -1, -1, 0, 0, 4, 0, 0, 0, LANG_ELVEN, 1, 0 },
{ "Golem", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_MAGICAL, 0, 0 },
{ "Aarakocra", AFF_FLYING+AFF_INFRARED, -2, 2, 0, 1, -1, 0, 0, 0, 0, 0, 0, 0, LANG_AARAKOCRA, 1, 0 },
{ "Troglodyte", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_OGRE, 0, 0 },
{ "Patryn", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_GITH, 0, 0 },
{ "Labrynthian", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Sartan", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Titan", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Smurf", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_ELVEN, 0, 0 },
{ "Kangaroo", AFF_SNEAK, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_MAMMAL, 0, 0 },
{ "Horse", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_MAMMAL, 0, 0 },
{ "Ratperson", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Astralion", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_GITH, 0, 0 },
{ "God", 0, 5, 5, 5, 5, 5, 5, 5, 20, 20, 0, 0, 0, LANG_GOD, 0, 0 },
{ "Hill Giant", AFF_INFRARED, 2, -1, -1, -1, 1, -1, 0, 5, 0, 0, 0, 0, LANG_OGRE, 0, 0 },
{ "Frost Giant", AFF_INFRARED, 2, -2, 0, 0, 1, -1, 0, 5, 0, RIS_COLD, RIS_FIRE, 0, LANG_OGRE, 0, 0 },
{ "Fire Giant", AFF_INFRARED, 2, -2, 0, 0, 1, -1, 0, 5, 0, RIS_FIRE, RIS_COLD, 0, LANG_OGRE, 0, 0 },
{ "Cloud Giant", AFF_INFRARED, 2, -2, 1, 0, 1, -2, 0, 5, 0, RIS_ACID, RIS_SLEEP, 0, LANG_COMMON, 0, 0 },
{ "Storm Giant", AFF_INFRARED, 2, -2, 1, 0, 1, -2, 0, 0, 0, RIS_ELECTRICITY, 0, 0, LANG_COMMON, 0, 0 },
{ "Stone Giant", AFF_INFRARED, 2, -2, 0, -2, 2, 0, 0, 5, 0, RIS_PIERCE, 0, 0, LANG_COMMON, 0, 0 },
{ "Red Dragon", AFF_FLYING+AFF_INFRARED, 0, 0, 0, 0, 0, 0, 0, 0, 0, RIS_FIRE, RIS_COLD, 0, LANG_DRAGON, 0, 0 },
{ "Black Dragon", AFF_FLYING+AFF_INFRARED, 0, 0, 0, 0, 0, 0, 0, 0, 0, RIS_ACID, RIS_ELECTRICITY, 0, LANG_DRAGON, 0, 0 },
{ "Green Dragon", AFF_FLYING+AFF_INFRARED, 0, 0, 0, 0, 0, 0, 0, 0, 0, RIS_POISON, 0, 0, LANG_DRAGON, 0, 0 },
{ "White Dragon", AFF_FLYING+AFF_INFRARED, 0, 0, 0, 0, 0, 0, 0, 0, 0, RIS_COLD, RIS_FIRE, 0, LANG_DRAGON, 0, 0 },
{ "Blue Dragon", AFF_FLYING+AFF_INFRARED, 0, 0, 0, 0, 0, 0, 0, 0, 0, RIS_ELECTRICITY, RIS_ACID, 0, LANG_DRAGON, 0, 0 },
{ "Silver Dragon", AFF_FLYING+AFF_INFRARED, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_DRAGON, 0, 0 },
{ "Gold Dragon", AFF_FLYING+AFF_INFRARED, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_DRAGON, 0, 0 },
{ "Bronze Dragon", AFF_FLYING+AFF_INFRARED, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_DRAGON, 0, 0 },
{ "Copper Dragon", AFF_FLYING+AFF_INFRARED, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_DRAGON, 0, 0 },
{ "Brass Dragon", AFF_FLYING+AFF_INFRARED, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_DRAGON, 0, 0 },
{ "Vampire", AFF_INFRARED, 2, 0, 0, 0, 0, 0, -2, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Lich", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_ANCIENT, 0, 0 },
{ "Wight", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_ANCIENT, 0, 0 },
{ "Ghast", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_ANCIENT, 0, 0 },
{ "Spectre", AFF_FLOATING+AFF_PASS_DOOR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_SPIRITUAL, 0, 0 },
{ "Zombie", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Skeleton", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Ghoul", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Half Elven", AFF_INFRARED, 0, 1, 0, 0, -1, 0, 0, 0, 3, 0, 0, 0, LANG_COMMON, 1, 0 },
{ "Half Ogre", 0, 1, 0, 1, -1, 1, -2, 0, 3, 0, 0, 0, 0, LANG_COMMON, 1, 0 },
{ "Half Orc", AFF_INFRARED, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 1, 0 },
{ "Half Giant", AFF_INFRARED, 3, -2, 0, -2, 2, -1, 0, 10, 0, 0, 0, 0, LANG_COMMON, 1, 0 },
{ "Lizardman", AFF_INFRARED, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_REPTILE, 0, 0 },
{ "Dark Dwarf", AFF_INFRARED, 1, -1, 1, -1, 1, -1, 0, 0, 0, 0, 0, 0, LANG_DWARVEN, 1, 0 },
{ "Deep Gnome", AFF_INFRARED, -1, 1, -1, 1, -1, 0, 1, 0, 0, 0, 0, 0, LANG_PIXIE, 1, 0 },
{ "Gnoll", AFF_INFRARED, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_TROLLISH, 0, 0 },
{ "High Elf", AFF_INFRARED, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, LANG_ELVEN, 1, 0 },
{ "Sylvan Elf", AFF_INFRARED, 1, 1, -1, -1, 0, 0, 1, 0, 0, 0, 0, 0, LANG_ELVEN, 1, 0 },
{ "Sea Elf", AFF_AQUA_BREATH+AFF_INFRARED, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_ELVEN, 0, 0 },
{ "Tiefling", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Aasimar", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Solar", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Planitar", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Shadow", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Giant Skeleton", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Nilbog", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Houser", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Baku", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Beast Lord", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Deva", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Polaris", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Demodand", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Tarasque", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Diety", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Daemon", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Vagabond", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Pokemon",  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Githzerai",  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Githyanki",  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Briaur",  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Modron",  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Dabus",  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 },
{ "Cranium Rat",  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, LANG_COMMON, 0, 0 }
};
/*Ma  Cl  Th  Wa  Vm  Dr  Ra  Am  Pa  Ba  Ps  Ar  Mo  Ne  Ap  So */
const  sh_int  RacialMax[MAX_RACE][REAL_MAX_CLASS] = {
/* Mutt */
{ 25, 25, 25, 25,  0, 25, 25,  0, 25, 25, 25, 25, 25, 25,  0,  0},
/* Human */
{ 61, 61, 61, 61,  0, 61, 61, 61, 61, 61, 61, 61, 61, 61,  0,  61},
/* Grey Elf */
{ 61, 30, 61, 20,  0, 61, 61,  0,  0,  0, 40,  0, 25, 61,  0,  0},
/* Dwarven */
{  0, 61, 35, 61,  0,  0, 30,  0, 45, 50,  0,  0,  0,  0,  0,  0},
/* Halfling */
{ 40, 40, 61, 30,  0,  0, 40,  0,  0,  0,  0,  0, 20, 20,  0,  0},
/* Gnome */
{ 61, 61, 40, 30,  0,  0,  0,  0,  0,  0,  0,  0,  0, 61,  0,  0},
/* Reptilian */
{ 20, 30, 50, 40,  0, 61,  0,  0,  0,  0,  0,  0, 40, 20,  0,  0},
/* Mysterion */
{ 50, 50,  1, 50,  0, 61,  0,  0,  0,  0,  0,  0,  1, 50,  0,  0},
/* Lycanthropian */
{ 30, 30, 50, 61,  0, 40,  0,  0,  0, 50,  0,  0, 20, 30,  0,  0},
/* Draconian */
{ 61, 20, 30, 61,  0, 20,  0,  0,  0,  0, 61,  0, 61, 61,  0,  0},
/* Undead */
{ 35, 35, 35, 35,  0, 35,  0,  0,  0, 35,  0,  0, 35, 35,  0,  0},
/* Orcish */
{  0, 30, 50, 40,  0,  0,  0,  0,  0, 61,  0,  0,  0,  0,  0,  0},
/* Insectoid */
{ 30, 20, 61, 40,  0, 50,  0,  0,  0, 10,  0,  0, 30, 30,  0,  0},
/* Arachnoid */
{ 20, 30, 61, 40,  0, 50,  0,  0,  0, 10,  0,  0, 30, 20,  0,  0},
/* Saurian */
{ 20, 30, 40, 61,  0, 50,  0,  0,  0, 10,  0,  0, 30, 20,  0,  0},
/* Icthyiod */
{ 20, 40, 50, 30,  0, 61,  0,  0,  0, 10,  0,  0, 30, 20,  0,  0},
/* Avian */
{ 40, 30, 50, 30,  0, 61,  0,  0,  0, 10,  0,  0, 20, 40,  0,  0},
/* Giant */
{ 20, 40, 50, 61,  0, 30,  0,  0,  0, 10,  0,  0, 30, 20,  0,  0},
/* Carnivororous */
{ 40, 30, 20, 61,  0, 30,  0,  0,  0, 10,  0,  0, 50, 40,  0,  0},
/* Parasitic */
{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Slime */
{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Demonic */
{ 61, 30, 50, 40,  0, 30,  0,  0,  0, 10, 50,  0, 20, 61,  0,  0},
/* Snake */
{ 40, 30, 61, 30,  0, 50,  0,  0,  0, 10,  0,  0, 20, 40,  0,  0},
/* Herbivorous */
{ 30, 40, 30, 20,  0, 61,  0,  0,  0, 10,  0,  0, 50, 30,  0,  0},
/* Tree */
{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Vegan */
{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Elemental */
{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Planar */
{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Diabolic */
{ 50, 61, 20, 40,  0, 30,  0,  0,  0, 10, 50,  0, 40, 50,  0,  0},
/* Ghostly */
{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Goblinoid */
{  0, 30, 61, 50,  0,  0,  0,  0,  0, 30,  0,  0,  0,  0,  0,  0},
/* Trollish */
{  0, 35,  0,  0,  0,  0,  0,  0,  0, 61,  0,  0,  0,  0,  0,  0},
/* Vegman */
{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Mindflayer */
{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 61,  0,  0,  0,  0,  0},
/* Primate */
{ 30, 30, 50, 40,  0, 20,  0,  0,  0, 10,  0,  0, 61, 30,  0,  0},
/* Enfan */
{ 50, 30, 40, 61,  0, 20,  0,  0,  0, 10,  0,  0, 30, 50,  0,  0},
/* Drow */
{ 61, 61, 40, 61,  0,  0,  0,  0,  0,  0,  0,  0,  0, 61,  0,  0},
/* Golem */
{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Aarakocra */
{ 61, 30, 61, 40,  0,  0,  0,  0,  0,  0,  0,  0,  0, 61,  0,  0},
/* Troglodyte */
{ 30, 40, 61, 50,  0, 30,  0,  0,  0, 50,  0,  0, 20, 30,  0,  0},
/* Patryn */
{ 61, 30, 30, 50,  0, 20,  0,  0,  0, 10,  0,  0, 40, 61,  0,  0},
/* Labrynthian */
{ 50, 30, 40, 61,  0, 20,  0,  0,  0, 10,  0,  0, 30, 50,  0,  0},
/* Sartan */
{ 50, 61, 20, 30,  0, 40,  0,  0,  0, 10,  0,  0, 30, 50,  0,  0},
/* Tytan */
{ 50, 30, 30, 61,  0, 40,  0,  0,  0, 10,  0,  0, 20, 50,  0,  0},
/* Smurf */
{ 10, 10, 10, 10,  0, 10,  0,  0,  0, 10,  0,  0, 10, 10,  0,  0},
/* Kangaroo */
{ 40, 30, 30, 61,  0, 20,  0,  0,  0, 10,  0,  0, 50, 40,  0,  0},
/* Horse */
{ 30, 40, 30, 20,  0, 61,  0,  0,  0, 10,  0,  0, 50, 30,  0,  0},
/* Ratperson */
{ 61, 30, 61, 20,  0, 40,  0,  0,  0, 10,  0,  0, 20, 61,  0,  0},
/* Astralion */
{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* God */
{ 50, 50, 50, 50,  0, 61, 50,  0, 50, 50, 50, 50, 50, 50,  0,  0},
/* Hill Giant */
{ 20, 40, 50, 61,  0, 30,  0,  0,  0, 61,  0,  0, 30, 20,  0,  0},
/* Frost Giant */
{ 20, 40, 50, 61,  0, 30,  0,  0,  0, 45,  0,  0, 30, 20,  0,  0},
/* Fire Giant */
{ 20, 40, 50, 61,  0, 30,  0,  0,  0, 50,  0,  0, 30, 20,  0,  0},
/* Cloud Giant */
{ 20, 40, 50, 61,  0, 30,  0,  0,  0, 20,  0,  0, 30, 20,  0,  0},
/* Storm Giant */
{ 20, 40, 50, 61,  0, 30,  0,  0,  0, 20,  0,  0, 30, 20,  0,  0},
/* Stone Giant */
{ 20, 40, 50, 61,  0, 30,  0,  0,  0, 20,  0,  0, 30, 20,  0,  0},
/* Red Dragon */
{ 61, 20, 30, 61,  0, 20,  0,  0,  0, 10,  0,  0, 61, 61,  0,  0},
/* Black Dragon */
{ 61, 20, 30, 61,  0, 20,  0,  0,  0, 10,  0,  0, 61, 61,  0,  0},
/* Green Dragon */
{ 61, 20, 30, 61,  0, 20,  0,  0,  0, 10,  0,  0, 61, 61,  0,  0},
/* White Dragon */
{ 61, 20, 30, 61,  0, 20,  0,  0,  0, 10,  0,  0, 61, 61,  0,  0},
/* Blue Dragon */
{ 61, 20, 30, 61,  0, 20,  0,  0,  0, 10,  0,  0, 61, 61,  0,  0},
/* Silver Dragon */
{ 61, 20, 30, 61,  0, 20,  0,  0,  0, 10,  0,  0, 61, 61,  0,  0},
/* Gold Dragon */
{ 61, 20, 30, 61,  0, 20,  0,  0,  0, 10,  0,  0, 61, 61,  0,  0},
/* Bronze Dragon */
{ 61, 20, 30, 61,  0, 20,  0,  0,  0, 10,  0,  0, 61, 61,  0,  0},
/* Copper Dragon */
{ 61, 20, 30, 61,  0, 20,  0,  0,  0, 10,  0,  0, 61, 61,  0,  0},
/* Brass Dragon */
{ 61, 20, 30, 61,  0, 20,  0,  0,  0, 10,  0,  0, 61, 61,  0,  0},
/* Vampire */
{ 35, 35, 35, 35,  0, 35,  0,  0,  0, 10,  0,  0, 35, 35,  0,  0},
/* Lich */
{ 35, 35, 35, 35,  0, 35,  0,  0,  0, 10,  0,  0, 35, 35,  0,  0},
/* Wight */
{ 35, 35, 35, 35,  0, 35,  0,  0,  0, 10,  0,  0, 35, 35,  0,  0},
/* Ghast */
{ 35, 35, 35, 35,  0, 35,  0,  0,  0, 10,  0,  0, 35, 35,  0,  0},
/* Spectre */
{ 35, 35, 35, 35,  0, 35,  0,  0,  0, 10,  0,  0, 35, 35,  0,  0},
/* Zombie */
{ 35, 35, 35, 35,  0, 35,  0,  0,  0, 10,  0,  0, 35, 35,  0,  0},
/* Skeleton */
{ 35, 35, 35, 35,  0, 35,  0,  0,  0, 10,  0,  0, 35, 35,  0,  0},
/* Ghoul */
{ 35, 35, 35, 35,  0, 35,  0,  0,  0, 10,  0,  0, 35, 35,  0,  0},
/* Half Elven */
{ 45, 40, 61, 40,  0, 35, 61,  0, 40,  0, 40,  0, 35, 30,  0,  0},
/* Half Ogre */
{  0, 30,  0, 61,  0, 40,  0,  0,  0, 40,  0,  0,  0,  0,  0,  0},
/* Half Orc */
{  0, 30, 61, 45,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Half Giant */
{  0,  0,  0, 61,  0,  0, 61, 61, 61,  0,  0,  0,  0,  0,  0,  0},
/* Lizardman */
{ 20, 61, 35, 61,  0, 15, 30,  0, 45, 61,  0,  0, 40, 20,  0,  0},
/* Dark Dwarf */
{  0, 61, 45, 61,  0,  0,  0,  0,  0, 50,  0,  0,  0, 20,  0,  0},
/* Deep Gnome */
{ 30, 61, 35, 61,  0,  0,  0,  0,  0,  0,  0,  0,  0, 30,  0,  0},
/* Gnoll */
{ 20, 40, 50, 61,  0, 30,  0,  0,  0,  0,  0,  0, 30, 20,  0,  0},
/* High Elf */
{ 61, 35, 61, 15,  0, 61, 61,  0, 45,  0, 40,  0, 20, 61,  0,  0},
/* Sylvan Elf */
{ 30, 35, 61, 61,  0, 61, 61,  0,  0,  0, 40,  0,  0, 30,  0,  0},
/* Sea Elf */
{ 61, 30, 61, 30,  0, 61, 61,  0, 40,  0, 40,  0, 20, 61,  0,  0},
/* Tiefling */
{ 35, 30, 61, 50,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Aasimar */
{ 61, 40,  0, 61,  0,  0, 61,  0, 61,  0,  0,  0,  0,  0,  0,  0},
/* Solar */
{ 61, 61, 61, 40,  0,  0,  0,  0,  0,  0, 61,  0,  0, 61,  0,  0},
/* Planitar */
{ 61, 61, 61, 40,  0,  0,  0,  0,  0,  0, 61,  0,  0, 61,  0,  0},
/* Shadow */
{  0,  0,  0, 61,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Giant Skeleton */
{  0,  0,  0, 61,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Nilbog */
{  0,  0,  0, 61,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Houser */
{  0,  0,  0, 61,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Baku */
{  0,  0,  0, 61,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Beast Lord */
{  0,  0,  0, 61,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Deva */
{  0,  0,  0, 61,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Polaris */
{  0,  0,  0, 61,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Demodand */
{  0,  0,  0, 61,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Tarasque */
{  0,  0,  0, 61,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Diety */
{  0,  0,  0, 61,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Daemon */
{  0,  0,  0, 61,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Vagabond */
{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 61,  0,  0,  0,  0,  0},
/* Pokemon */
{  0,  0,  0, 60,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Githzerai */
{ 61, 61, 61, 61,  0,  0,  0,  0,  0,  0, 61,  0, 61,  0,  0,  0},
/* Githyanki */
{ 61, 61, 61, 61,  0,  0,  0,  0,  0,  0, 61,  0, 61,  0,  0,  0},
/* Bariaur */
{  0,  0,  0, 60,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Modron */
{  0,  0,  0, 60,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Dabus */
{  0,  0,  0, 60,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
/* Cranium Rat */
{  0,  0,  0, 60,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}

};

/*
char *  const   npc_race        [MAX_NPC_RACE] =
 {
  "Mutt",
  "Human",
  "Grey Elf",
  "Dwarven",
  "Halfling",
  "Gnome",
  "Reptilian",
  "Mysterion",
  "Lycanthropian",
  "Draconian",
  "Undead",
  "Orcish",
  "Insectoid",
  "Arachnoid",
  "Saurian",
  "Icthyiod",
  "Avian",
  "Giant",
  "Carnivororous",
  "Parasitic",
  "Slime",
  "Demonic",
  "Snake",
  "Herbivorous",
  "Tree",
  "Vegan",
  "Elemental",
  "Planar",
  "Diabolic",
  "Ghostly",
  "Goblinoid",
  "Trollish",
  "Vegman",
  "Mindflayer",
  "Primate",
  "Enfan",
  "Drow",
  "Golem",
  "Aarakocra",
  "Troglodyte",
  "Patryn",
  "Labrynthian",
  "Sartan",
  "Tytan",
  "Smurf",
  "Kangaroo",
  "Horse",
  "Ratperson",
  "Astralion",
  "God",
  "Hill Giant",
  "Frost Giant",
  "Fire Giant",
  "Cloud Giant",
  "Storm Giant",
  "Stone Giant",
  "Red Dragon",
  "Black Dragon",
  "Green Dragon",
  "White Dragon",
  "Blue Dragon",
  "Silver Dragon",
  "Gold Dragon",
  "Bronze Dragon",
  "Copper Dragon",
  "Brass Dragon",
  "Vampire",
  "Lich",
  "Wight",
  "Ghast",
  "Spectre",
  "Zombie",
  "Skeleton",
  "Ghoul",
  "Half Elven",
  "Half Ogre",
  "Half Orc",
  "Half Giant",
  "Lizardman",
  "Dark Dwarf",
  "Deep Gnome",
  "Gnoll",
  "High Elf",
  "Sylvan Elf",
  "Sea Elf",
  "Tiefling",
  "Aasimar",
  "Solar",
  "Planitar",
  "Shadow",
  "Giant Skeleton",
  "Nilbog",
  "Houser",
  "Baku",
  "Beast Lord",
  "Deva",
  "Polaris",
  "Demodand",
  "Tarasque",
  "Diety",
  "Daemon",
  "Vagabond",
  "Pokemon",
  "Githzerai",
  "Githyanki",
  "Bariaur",
  "Modron",
  "Dabus",
  "Cranium Rat"
};*/

char *	const	npc_class	[MAX_NPC_CLASS] =
{
"mage", "cleric", "thief", "warrior", "vampire", "druid", "ranger",
"amazon", "paladin", "barbarian", "psionist", "artificer", "monk",
"necromancer", "antipaladin", "pc15", "pc16", "pc17", "pc18", "pc19",
"baker", "butcher", "blacksmith", "mayor", "king", "queen"
};

char * const    pc_class        [REAL_MAX_CLASS] =
{
"mage", "cleric", "thief", "warrior", "vampire", "druid", "ranger",
"amazon", "paladin", "barbarian", "psionist", "artificer", "monk",
"necromancer", "antipaladin", "sorcerer"
};

char * const    short_pc_class  [REAL_MAX_CLASS] =
{
"Ma", "Cl", "Th", "Wa", "Vm", "Dr", "Ra", "Am",
"Pa", "Ba", "Ps", "Ar", "Mo", "Nc", "Ap", "So"
};

/*
 * Attribute bonus tables.
 */
const struct str_app_type str_app[26] = {
	{ -5,-4,   0,  0 },  /* 0  */
	{ -5,-4,   1,  1 },  /* 1  */
	{ -3,-2,   1,  2 },
	{ -3,-1,   5,  3 },  /* 3  */
	{ -2,-1,  10,  4 },
	{ -2,-1,  20,  5 },  /* 5  */
	{ -1, 0,  25,  6 },
	{ -1, 0,  30,  7 },
	{  0, 0,  40,  8 },
	{  0, 0,  50,  9 },
	{  0, 0,  55, 10 }, /* 10  */
	{  0, 0,  70, 11 },
	{  0, 0,  80, 12 },
	{  0, 0,  90, 13 },
	{  0, 0, 100, 14 },
	{  0, 0, 110, 15 }, /* 15  */
	{  0, 1, 120, 16 },
	{  2, 3, 170, 25 }, /* 17 = 18/51-75 dale */
	{  2, 5, 255, 35 }, /* 18 = 18/91-99 dale */
	{  3, 7, 485, 45 },
	{  3, 8, 535, 50 }, /* 20  */
	{  4, 9, 635, 55 },
	{  4,10, 785, 55 },
	{  5,11, 935, 55 },
	{  6,12,1235, 55 },
	{  7,14,1535, 60 } /* 25            */
};

const struct dex_skill_type dex_app_skill[26] = {
	{-99,-99,-90,-99,-60},   /* 0 */
	{-90,-90,-60,-90,-50},   /* 1 */
	{-80,-80,-40,-80,-45},
	{-70,-70,-30,-70,-40},
	{-60,-60,-30,-60,-35},
	{-50,-50,-20,-50,-30},   /* 5 */
	{-40,-40,-20,-40,-25},
	{-30,-30,-15,-30,-20},
	{-20,-20,-15,-20,-15},
	{-15,-10,-10,-20,-10},
	{-10, -5,-10,-15, -5},   /* 10 */
	{ -5,  0, -5,-10,  0},
	{  0,  0,  0, -5,  0},
	{  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0},
	{  0,  0,  0,  0,  0},   /* 15 */
	{  0,  5,  0,  0,  0},
	{  5, 10,  0,  5,  5},
	{ 10, 15,  5, 10, 10},
	{ 15, 20, 10, 15, 15},
	{ 15, 20, 10, 15, 15},   /* 20 */
	{ 20, 25, 10, 15, 20},
	{ 20, 25, 15, 20, 20},
	{ 25, 25, 15, 20, 20},
	{ 25, 30, 15, 25, 25},
	{ 25, 30, 15, 25, 25}    /* 25 */
};

/* [level] backstab multiplyer (thieves only) */
const sh_int backstab_mult[MAX_LEVEL+1] = {
	1,   /* 0 */
	2,   /* 1 */
	2,
	2,
	2,
	2,   /* 5 */
	2,
	2,
	3,   /* 8 */
	3,
	3,   /* 10 */
	3,
	3,
	3,
	3,
	3,   /* 15 */
	4,			/* 16 */
	4,
	4,
	4,
	4,   /* 20 */
	4,
	4,
	4,
	5,    /* 24 */
	5,
	5,
	5,
	5,
	5,   /* 29 */
	5,   /* 30 */
	5,
	6,
	6,
	6,   /* 34 */
	6,
	6,
	6,
	7,
	7,  /* 39 */
	7,
	7,
	7,
	8,
	8,  /* 44 */
	8,
	8,
	8,
	9,
	10,  /* 49 */
        11,
        11,
        12,
        12,
        13,
        14,  /* 55 */
        15,
        16,
        17,
        18, /* 59 */
        19,
        20,
        21,
        22,
        23,
        24,
        25,
        26,
        27,
        28,  /* 69 */
        28,
        28,
        28,
        28,
        28,
        28
};

const struct dex_app_type dex_app[26] = {
	{-7,-7, 60},   /* 0 */
	{-6,-6, 50},   /* 1 */
	{-4,-4, 50},
	{-3,-3, 40},
	{-2,-2, 30},
	{-1,-1, 20},   /* 5 */
	{ 0, 0, 10},
	{ 0, 0, 0},
	{ 0, 0, 0},
	{ 0, 0, 0},
	{ 0, 0, 0},   /* 10 */
	{ 0, 0, 0},
	{ 0, 0, 0},
	{ 0, 0, 0},
	{ 0, 0, 0},
	{ 0, 0,-10},   /* 15 */
	{ 1, 1,-20},
	{ 2, 2,-30},
	{ 2, 2,-40},
	{ 3, 3,-40},
	{ 3, 3,-40},   /* 20 */
	{ 4, 4,-50},
	{ 4, 4,-50},
	{ 4, 4,-50},
	{ 5, 5,-60},
	{ 5, 5,-60}    /* 25 */
};

const struct con_app_type con_app[26] = {
	{-4,20},   /* 0 */
	{-3,25},   /* 1 */
	{-2,30},
	{-2,35},
	{-1,40},
	{-1,45},   /* 5 */
	{-1,50},
	{ 0,55},
	{ 0,60},
	{ 0,65},
	{ 0,70},   /* 10 */
	{ 0,75},
	{ 0,80},
	{ 0,85},
	{ 0,88},
	{ 1,90},   /* 15 */
	{ 2,95},		/* 16 */
	{ 3,97},		/* 17 */
	{ 4,99},		/* 18 */
	{ 4,99},
	{ 5,99},   /* 20 */
	{ 6,99},
	{ 6,99},
	{ 7,99},
	{ 8,99},
	{ 9,100}   /* 25 */
};

const struct int_app_type int_app[26] = {
	{0},
	{1},    /* 1 */
	{2},
	{3},
	{4},
	{5},   /* 5 */
	{6},
	{8},
	{10},
	{12},
	{14},   /* 10 */
	{16},
	{18},
	{20},
	{22},
	{25},   /* 15 */
	{28},
	{32},
	{35},
	{40},
	{45},   /* 20 */
	{50},
	{60},
	{70},
	{80},
	{99}    /* 25 */
};

const struct wis_app_type wis_app[26] = {
	{0},
	{0},
	{0},
	{1},
	{1},
	{1},
	{1},
	{1},
	{2},
	{2},
	{3}, /* 10 */
	{3}, /* 11 */
	{3}, /* 12 */
	{4}, /* 13 */
	{4}, /* 14 */
	{5}, /* 15 */
	{5}, /* 16 */
	{6}, /* 17 */
	{7}, /* 18 */
	{7},
	{7},
	{7},
	{7},
	{7},
	{7},
	{8}
};

const struct cha_app_type cha_app[26] = {
        { 0, -70},		/* 0 */
        { 0, -70},
        { 1, -60},
        { 1, -50},
        { 1, -40},		/* 4 */
        { 2, -30},
        { 2, -20},
        { 3, -10},
        { 4,   0},
        { 5,   0},		/* 9 */
        { 6,   0},
        { 7,   0},
        { 8,   0},
        { 9,  +5},		/* 13 */
        { 10, +10},
        { 12, +15},
        { 14, +20},
        { 17, +25},
        { 20, +30}, /* 18 */
        { 20, +40},
        { 25, +50},
        { 25, +60},
        { 25, +70},
        { 25, +80},
        { 25, +90},
        { 25, +95},
};

/* Have to fix this up - not exactly sure how it works (Scryn) */
const	struct	lck_app_type	lck_app		[26]		=
{
    {   60 },   /* 0 */
    {   50 },   /* 1 */
    {   50 },
    {   40 },
    {   30 },
    {   20 },   /* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   /* 20 */
    { - 60 },
    { - 75 },
    { - 90 },
    { -105 },
    { -120 }    /* 25 */
};

/* [class], [level] (all) */
const int thaco[REAL_MAX_CLASS][111] =
{
    /* mage */
    { 100,20,20,20,20,20,19,19,19,19,19,18,18,18,18,18,17,17,17,17,17,16,16,16,16
    ,16,15,15,15,15,15,14,14,14,14,14,13,13,13,13,13,12,12,12,12,12,11,11,11,11,11,1
    ,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    /* cleric */
    { 100,20,20,20,19,19,19,18,18,18,17,17,17,16,16,16,15,15,15,14,14,14,13,13,13
    ,12,12,12,11,11,11,10,10,10,9,9,9,8,8,8,7,7,7,6,6,6,5,5,5,4,4,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1},
    /* thief */
    { 100,20,20,20,19,19,19,19,18,18,18,17,17,17,17,16,16,16,15,15,15,15,14,14,14
    ,13,13,13,13,12,12,12,11,11,11,11,10,10,10,9,9,9,9,8,8,8,7,7,7,7,6,1,1,1,1,1,1,1
    ,1,1,1,1,1,1,1,1,1,1,1,1},
    /* fighter */
    { 100,20,20,19,19,18,18,17,17,16,16,15,15,14,14,13,13,12,12,11,11,10,10,9,9,8
    ,8,7,7,6,6,5,5,4,4,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    ,1,1,1,1},
    /* vampire (fighter) */
    { 100,20,20,19,19,18,18,17,17,16,16,15,15,14,14,13,13,12,12,11,11,10,10,9,9,8
    ,8,7,7,6,6,5,5,4,4,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    ,1,1,1,1},
    /* druid */
    { 100,20,20,20,19,19,19,18,18,18,17,17,17,16,16,16,15,15,15,14,14,14,13,13,13
    ,12,12,12,11,11,11,10,10,10,9,9,9,8,8,8,7,7,7,6,6,6,5,5,5,4,4,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1},
    /* ranger */
    { 100,20,20,19,19,18,18,17,17,16,16,15,15,14,14,13,13,12,12,11,11,10,10,9,9,8
    ,8,7,7,6,6,5,5,4,4,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    ,1,1,1,1},
    /* amazon (thief) */
    { 100,20,20,20,19,19,19,19,18,18,18,17,17,17,17,16,16,16,15,15,15,15,14,14,14
    ,13,13,13,13,12,12,12,11,11,11,11,10,10,10,9,9,9,9,8,8,8,7,7,7,7,6,1,1,1,1,1,1,1
    ,1,1,1,1,1,1,1,1,1,1,1,1},
    /* paladin */
    { 100,20,20,19,19,18,18,17,17,16,16,15,15,14,14,13,13,12,12,11,11,10,10,9,9,8
    ,8,7,7,6,6,5,5,4,4,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    ,1,1,1,1},
    /* barbarian */
    { 100,20,20,19,19,18,18,17,17,16,16,15,15,14,14,13,13,12,12,11,11,10,10,9,9,8
    ,8,7,7,6,6,5,5,4,4,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    ,1,1,1,1},
    /* psionist */
    { 100,20,20,20,19,19,19,19,18,18,18,17,17,17,17,16,16,16,15,15,15,15,14,14,14
    ,13,13,13,13,12,12,12,11,11,11,11,10,10,10,9,9,9,9,8,8,8,7,7,7,7,6,1,1,1,1,1,1,1
    ,1,1,1,1,1,1,1,1,1,1,1,1},
    /* artificer (sorcerer) */
    { 100,20,20,20,20,20,19,19,19,19,19,18,18,18,18,18,17,17,17,17,17,16,16,16,16
    ,16,15,15,15,15,15,14,14,14,14,14,13,13,13,13,13,12,12,12,12,12,11,11,11,11,11,1
    ,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    /* monk  */
    { 100,20,20,19,19,18,18,17,17,16,16,15,15,14,14,13,13,12,12,11,11,10,10,9,9,8
    ,8,7,7,6,6,5,5,4,4,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    ,1,1,1,1},
    /* necromancer (druid) */
    { 100,20,20,20,19,19,19,18,18,18,17,17,17,16,16,16,15,15,15,14,14,14,13,13,13
    ,12,12,12,11,11,11,10,10,10,9,9,9,8,8,8,7,7,7,6,6,6,5,5,5,4,4,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1},
    /* antipaladin (paladin) */
    { 100,20,20,19,19,18,18,17,17,16,16,15,15,14,14,13,13,12,12,11,11,10,10,9,9,8
    ,8,7,7,6,6,5,5,4,4,3,3,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
    ,1,1,1,1},
    /* mage */
    { 100,20,20,20,20,20,19,19,19,19,19,18,18,18,18,18,17,17,17,17,17,16,16,16,16
    ,16,15,15,15,15,15,14,14,14,14,14,13,13,13,13,13,12,12,12,12,12,11,11,11,11,11,1
    ,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

/*
 * Liquid properties.
 * Used in #OBJECT section of area file.
 */
const	struct	liq_type	liq_table	[LIQ_MAX]	=
{
    { "water",			"clear",	{  0, 1, 10 }	},  /*  0 */
    { "beer",			"amber",	{  3, 2,  5 }	},
    { "wine",			"rose",		{  5, 2,  5 }	},

    { "ale",			"brown",	{  2, 2,  5 }	},
    { "dark ale",		"dark",		{  1, 2,  5 }	},

    { "whisky",			"golden",	{  6, 1,  4 }	},  /*  5 */
    { "lemonade",		"pink",		{  0, 1,  8 }	},
    { "firebreather",		"boiling",	{ 10, 0,  0 }	},
    { "local specialty",	"everclear",	{  3, 3,  3 }	},
    { "slime mold juice",	"green",	{  0, 4, -8 }	},

    { "milk",			"white",	{  0, 3,  6 }	},  /* 10 */
    { "tea",			"tan",		{  0, 1,  6 }	},
    { "coffee",			"black",	{  0, 1,  6 }	},
    { "blood",			"red",		{  0, 2, -1 }	},
    { "salt water",		"clear",	{  0, 1, -2 }	},

    { "cola",			"cherry",	{  0, 1,  5 }	},  /* 15 */
    { "mead",			"honey color",	{  4, 2,  5 }	},  /* 16 */
    { "grog",			"thick brown",	{  3, 2,  5 }	}   /* 17 */
};

char *		attack_table	[DAM_MAX_TYPE] =
{
  "hit",   "cleave", "stab",  "slash", "whip",   "claw", "blast",
  "pound", "crush",  "smash", "bite",  "pierce", "sting", "bolt",
  "arrow", "dart", "stone", "pea"
};

char *		attack_table_plural [DAM_MAX_TYPE] =
{
  "hits",   "cleaves", "stabs",   "slashes", "whips",   "claws", "blasts",
  "pounds", "crushes", "smashes", "bites",   "pierces", "stings", "bolt",
  "arrow", "dart", "stone", "pea"
};

char *		body_location [] =
{
 "head", "arms", "legs", "heart", "brains", "guts", "hands", "feet",
 "fingers", "ear", "eye", "long tongue", "eyestalks", "tentacles", "fins",
 "wings", "tail", "scales", "claws", "fanged mouth", "horns", "tusks",
 "tail", "scales", "beak", "haunches", "hooves", "paws", "forelegs",
 "feathered body", "chest", "stomach"
};

char *		body_location_hit [] =
{
 "on $S head", "on $S arms", "on $S legs", "on $S heart", "on $S brains",
"on $S guts",
"on $S hands", "on $S feet",
 "on $S fingers", "on $S ear", "on $S eye", "on $S long tongue",
"on $S eyestalks",
"on $S tentacles",
"on $S fins",
 "on $S wings", "on $S tail", "on $S scales", "on $S claws",
"on $S fanged mouth",
"on $S horns",
"on $S tusks",
 "on $S tail", "on $S scales", "on $S beak", "on $S haunches",
"on $S hooves",
"on $S paws",
"on $S forelegs",
 "on $S feathered body", "on $S chest", "on $S stomach"
};
/*
struct attack_hit_type location_hit_text[] =
{
  {"in $S body","body",},
  {"in $S left leg",  "left leg"},
  {"in $S right leg", "right leg"},
  {"in $S left arm",  "left arm"},
  {"in $S right arm", "right arm"},
  {"in $S shoulder",  "shoulder"},
  {"in $S neck",      "neck"},
  {"in $S left foot", "left foot"},
  {"in $S right foot","right foot"},
  {"in $S left hand", "left hand"},
  {"in $S right hand","right hand"},
  {"in $S chest",     "chest"},
  {"in $S back",      "back"},
  {"in $S groin",   "groin"},
  {"in $S head",      "head"}
};
*/

char *	const	color_table	[] =
{
"black", "blood", "dgreen", "orange", "dblue", "purple", "cyan",
"grey", "dgrey", "red", "green", "yellow", "blue", "pink", "lblue",
"white", "blink", "plain", "action", "say", "gossip", "yell", "tell",
"hit", "hitme", "immort", "hurt", "falling", "danger", "magic",
"consider", "report", "poison", "social", "dying", "dead", "skill",
"carnage", "damage", "flee", "rmname", "rmdesc", "object", "person",
"list", "bye", "gold", "gtell", "note", "hungry", "thirsty", "fire",
"sober", "wearoff", "score", "score2", "score3", "score4", "reset",
"log", "diemsg", "wartalk", "who", "who2", "who3", "who4", "chess1",
"chess2", "weather", "dirnorth", "dirsouth", "direast", "dirwest",
"dirup", "dirdown", "dirne", "dirnw", "dirse", "dirsw",
"-","-","-","-","-","-","-","-","-","-","-","-","-"
};

const char * con_table[101] =
{
 "NAME", /* -99 */
 "OLD PASSWORD",
 "NEW NAME",
 "NEW PASSWORD",
 "CONFIRM NEW PASSWORD", /* -95 */
 "RACE",
 "PLANAR",
 "SEX",
 "CLASS",
 "SIMPLE OR ADVANCED", /* -90 */
 "HOMETOWN",
 "SCORES",
 "DEADLY",
 "ANSI",
 "INTERFACE", /* -85 */
 "ENTER",
 "MOTD",
 "WAIT1",
 "WAIT2",
 "WAIT3", /* -80 */
 "ACCEPTED",
 "COPYOVER RECOVER",
 "---",
 "---",
 "---", /* -75 */
 "---",
 "---",
 "---",
 "---",
 "---", /* -70 */
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---", /* -60 */
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---", /* -50 */
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---", /* -40 */
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---", /* -30 */
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---", /* -20 */
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---", /* -10 */
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "---",
 "PLAYING", /* 0 */
 "EDITING"
};

char * const position_types[MAX_POSITION] =
{
 "dead", "mortally wounded", "incapacitated", "stunned", "sleeping",
 "resting", "sitting", "fighting", "standing", "mounted",
 "shove", "drag", "meditating"
};

/* Weather constants - FB */

char * const temp_settings[] =
{
	"cold",
	"cool",
	"normal",
	"warm",
	"hot",
        "BUG PLEASE REPORT: 1"
};

char * const precip_settings[] =
{
	"arid",
	"dry",
	"normal",
	"damp",
        "wet",
        "BUG PLEASE REPORT: 2"
};

char * const wind_settings[] =
{
	"still",
	"calm",
	"normal",
	"breezy",
        "windy",
        "BUG PLEASE REPORT: 3"
};

char * const preciptemp_msg[6][6] =
{
	/* precip = 0 */
	{
		"Frigid temperatures settle over the land",
		"It is bitterly cold",
		"The weather is crisp and dry",
		"A comfortable warmth sets in",
		"A dry heat warms the land",
		"Seething heat bakes the land"
	 },
	 /* precip = 1 */
	 {
	 	"A few flurries drift from the high clouds",
	 	"Frozen drops of rain fall from the sky",
	 	"An occasional raindrop falls to the ground",
	 	"Mild drops of rain seep from the clouds",
	 	"It is very warm, and the sky is overcast",
	 	"High humidity intensifies the seering heat"
	 },
	 /* precip = 2 */
	 {
	 	"A brief snow squall dusts the earth",
	 	"A light flurry dusts the ground",
	 	"Light snow drifts down from the heavens",
	 	"A light drizzle mars an otherwise perfect day",
	 	"A few drops of rain fall to the warm ground",
	 	"A light rain falls through the sweltering sky"
	 },
	 /* precip = 3 */
	 {
	 	"Snowfall covers the frigid earth",
	 	"Light snow falls to the ground",
	 	"A brief shower moistens the crisp air",
	 	"A pleasant rain falls from the heavens",
	 	"The warm air is heavy with rain",
	 	"A refreshing shower eases the oppresive heat"
	 },
	 /* precip = 4 */
	 {
	 	"Sleet falls in sheets through the frosty air",
	 	"Snow falls quickly, piling upon the cold earth",
	 	"Rain pelts the ground on this crisp day",
	 	"Rain drums the ground rythmically",
	 	"A warm rain drums the ground loudly",
	 	"Tropical rain showers pelt the seering ground"
	 },
	 /* precip = 5 */
	 {
	 	"A downpour of frozen rain covers the land in ice",
	 	"A blizzard blankets everything in pristine white",
	 	"Torrents of rain fall from a cool sky",
	 	"A drenching downpour obscures the temperate day",
	 	"Warm rain pours from the sky",
	 	"A torrent of rain soaks the heated earth"
	 }
};

char * const windtemp_msg[6][6] =
{
	/* wind = 0 */
	{
		"The frigid air is completely still",
		"A cold temperature hangs over the area",
		"The crisp air is eerily calm",
		"The warm air is still",
		"No wind makes the day uncomfortably warm",
		"The stagnant heat is sweltering"
	},
	/* wind = 1 */
	{
		"A light breeze makes the frigid air seem colder",
		"A stirring of the air intensifies the cold",
		"A touch of wind makes the day cool",
		"It is a temperate day, with a slight breeze",
		"It is very warm, the air stirs slightly",
		"A faint breeze stirs the feverish air"
	},
	/* wind = 2 */
	{
		"A breeze gives the frigid air bite",
		"A breeze swirls the cold air",
		"A lively breeze cools the area",
		"It is a temperate day, with a pleasant breeze",
		"Very warm breezes buffet the area",
		"A breeze ciculates the sweltering air"
	},
	/* wind = 3 */
	{
		"Stiff gusts add cold to the frigid air",
		"The cold air is agitated by gusts of wind",
		"Wind blows in from the north, cooling the area",
		"Gusty winds mix the temperate air",
		"Brief gusts of wind punctuate the warm day",
		"Wind attempts to cut the sweltering heat"
	},
	/* wind = 4 */
	{
		"The frigid air whirls in gusts of wind",
		"A strong, cold wind blows in from the north",
		"Strong wind makes the cool air nip",
		"It is a pleasant day, with gusty winds",
		"Warm, gusty winds move through the area",
		"Blustering winds punctuate the seering heat"
	},
	/* wind = 5 */
	{
		"A frigid gale sets bones shivering",
		"Howling gusts of wind cut the cold air",
		"An angry wind whips the air into a frenzy",
		"Fierce winds tear through the tepid air",
		"Gale-like winds whip up the warm air",
		"Monsoon winds tear the feverish air"
	}
};

char * const precip_msg[] =
{
	"there is not a cloud in the sky",
	"pristine white clouds are in the sky",
        "thick, grey clouds mask the sun",
        "BUG PLEASE REPORT: 6"
};

char * const wind_msg[] =
{
	"there is not a breath of wind in the air",
	"a slight breeze stirs the air",
	"a breeze wafts through the area",
	"brief gusts of wind punctuate the air",
	"angry gusts of wind blow",
        "howling winds whip the air into a frenzy",
        "BUG PLEASE REPORT: 7"
};


/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)	n
#define LI LEVEL_IMMORTAL

#undef AM
#undef AC
#undef AT
#undef AW
#undef AV
#undef AD
#undef AR
#undef AA

#undef LI

/* Materials for do_forge and do_tan */
const int mat_costmul[] = {0, 0, 50, 75, 100, 500, 1000, 200, 15, 10000};
const int mat_hitplus[] = {0, -2, 0, 0, 0, 0, 0, 0, 1, 2};
const int mat_damplus[] = {0, -2, -2, -1, 0, -1, -2, 1, 1, 2};
const int mat_acplus[] = {0, 0, 0, 1, 2, 0, 0, 2, 2, 3};
const char *mat_name[] = {"scrapped", "wooden", "lead", "copper",
                          "steel", "silver", "golden",
                          "titanium", "mithril", "admantite"};
const char *mat_qname[] = {"junk", "poor", "average", "excellent",
                           "unearthly", "godly"};


char * const race_exit_msgs[MAX_RACE] =
{
  "walks",
  "walks",
  "strolls",
  "strides",
  "tramps",
  "saunters",
  "slithers",
  "moves",
  "struts",
  "marches",
  "slowly trudges",
  "strides",
  "flits",
  "rambles",
  "stomps",
  "swims",
  "flys",
  "marches",
  "prowls",
  "slithers",
  "slides",
  "marches",
  "slithers",
  "ambles",
  "trudges",
  "walks",
  "effortlessly glides",
  "totters",
  "shambles",
  "floats",
  "tramps",
  "stomps",
  "ambles",
  "saunters",
  "shambles",
  "strolls",
  "proudly strolls",
  "marches",
  "slithers",
  "marches",
  "stomps",
  "stomps",
  "stomps",
  "stomps",
  "bounces",
  "hops",
  "trots",
  "creeps",
  "flounces",
  "walks",
  "marches",
  "marches",
  "marches",
  "marches",
  "marches",
  "marches",
  "stomps",
  "stomps",
  "stomps",
  "stomps",
  "stomps",
  "stomps",
  "stomps",
  "stomps",
  "stomps",
  "stomps",
  "creeps",
  "trudges",
  "slips",
  "floats",
  "glides",
  "trudges",
  "limps",
  "floats",
  "walks",
  "trudges",
  "strolls",
  "stomps",
  "slithers",
  "creeps",
  "glides",
  "crawls",
  "coasts",
  "ambles",
  "slides",
  "marches",
  "saunters",
  "sweeps",
  "skims",
  "flows",
  "clatters",
  "creeps",
  "rumbles",
  "marches",
  "saunters",
  "glides",
  "swoops",
  "saunters",
  "roars",
  "floats", /* 99 */
  "slinks",
  "steals",
  "trollopes"
};


char * const plane_names[MAX_PLANES] =
{
    "normal",
    "astral",
    "desert",
    "arctic",
    "under ground",
    "fire",
    "water",
    "air",
    "earth",
    "prime",
    "ethereal",
    "other",
    "mount celestia",
    "bytopia",
    "elysium",
    "beastlands",
    "arborea",
    "pandemonium",
    "abyss",
    "carceri",
    "hades",
    "gehenna",
    "baator",
    "acheron",
    "mechanus",
    "arcadia",
    "ysgard",
    "unique",
    "gray waste",
    "limbo",
    "outlands"
};


char * const channel_names[32] =
{
    "auction",
    "chat",
    "quest",
    "immtalk",
    "music",
    "ask",
    "shout",
    "gossip",
    "monitor",
    "log",
    "highgod",
    "clan",
    "build",
    "whisper",
    "avtalk",
    "pray",
    "council",
    "guild",
    "comm",
    "tell",
    "order",
    "newbie",
    "wartalk",
    "logpc",
    "httpd",
    "ooc",
    "imc",
    "bug",
    "debug",
    "magic",
    "imcdebug",
    "blah2"
};
