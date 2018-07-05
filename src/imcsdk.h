/*
 * IMC2 SDK, Prototypes from:
 */

/*
 * IMC2 AntiFreeze Client - Developed by Mud Domain.
 * This code is certified as 100% LGPL compatible.
 * All aspects have been verified against the core components
 * contained in Oliver's relicensed code. No components licensed
 * under the GPL remain.
 *
 * All additions Copyright (C)2002-2004 Roger Libiez ( Samson )
 * Contributions by Xorith, Copyright (C)2004
 * Contributions by Orion Elder, Copyright (C)2003
 * Comments and suggestions welcome: imc@muddomain.com
 *
 * We would like to thank the following people for their hard work on
 * versions 1.00 Gold through 3.00 Mud-Net even though 
 * their contributions no longer remain:
 *
 * Scion Altera, Shogar, Kratas, Tagith, Noplex,
 * Senir, Trax and Ntanel StormBlade.
 *
 * This work is derived from:
 * IMC2 version 0.10 - an inter-mud communications protocol
 * Copyright (C) 1996 & 1997 Oliver Jowett <oliver@randomly.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 */

void setdata(PACKET *p, imc_char_data *d);
char *getname(CHAR_DATA *ch, imc_char_data *d);
imc_char_data *imc_getdata( CHAR_DATA *ch );
char *imc_mudof( char *fullname );
size_t imcstrlcpy( char *dst, const char *src, size_t siz );
void imc_addkey( PACKET *p, char *key, char *value );
void imc_send( PACKET *p );
void imc_freedata( PACKET *p );
void imc_send_tell( imc_char_data *from, char *to, char *argument, int isreply );
CHAR_DATA *imc_find_user( char *name );
bool check_mud( CHAR_DATA *ch, char *mud );
char *imc_getkey( PACKET *p, char *key, char *def );
void imc_register_packet_handler(char *name, void (*func) ( imc_char_data *from, PACKET *p ));

extern imc_siteinfo_struct imc_siteinfo;
extern int imc_active;
