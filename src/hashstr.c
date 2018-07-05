/****************************************************************************
 * Advanced string hashing functions (c)1996 D.S.D. Software, written by    *
 * Derek Snider for use in SMAUG.					    *
 *									    *
 * These functions keep track of how many "links" are pointing to the	    *
 * memory allocated, and will free the memory if all the links are removed. *
 * Make absolutely sure you do not mix use of strdup and free with these    *
 * functions, or nasty stuff will happen!				    *
 * Most occurances of strdup/str_dup should be replaced with str_alloc, and *
 * any free/DISPOSE used on the same pointer should be replaced with	    *
 * str_free.  If a function uses strdup for temporary use... it is best if  *
 * it is left as is.  Just don't get usage mixed up between conventions.    *
 * The hashstr_data size is 8 bytes of overhead.  Don't be concerned about  *
 * this as you still save lots of space on duplicate strings.	-Thoric	    *
 ****************************************************************************/

/*static char rcsid[] = "$Id: hashstr.c,v 1.12 2003/08/17 17:29:26 dotd Exp $";*/

#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <assert.h>

#ifdef STRING_HASH_SEMAPHORE
#include <semaphore.h>
#endif

#define STR_HASH_SIZE	1024
#define STR_HASH_SANITY 0xDEADBEEF

struct hashstr_data
{
    unsigned int sanity;
    struct hashstr_data	*next;		/* next hash element */
    unsigned short int	 links;		/* number of links to this string */
    unsigned short int	 length;	/* length of string */
};

char *		str_alloc( char *str );
char *		quick_link( char *str );
int		str_free( char *str );
void		show_hash( int count );
char *		hash_stats( void );

struct hashstr_data *string_hash[STR_HASH_SIZE];
#ifdef STRING_HASH_SEMAPHORE
sem_t string_hash_semaphore[STR_HASH_SIZE];
#endif

/*
 * Check hash table for existing occurance of string.
 * If found, increase link count, and return pointer,
 * otherwise add new string to hash table, and return pointer.
 */
char *str_alloc( char *str )
{
   register int len, hash, psize;
   register struct hashstr_data *ptr;

   len = strlen(str);
   psize = sizeof(struct hashstr_data);
   hash = len % STR_HASH_SIZE;
   for (ptr = string_hash[hash]; ptr; ptr = ptr->next )
       if ( len == ptr->length && !strcmp(str,(char *)ptr+psize) )
       {
	   if ( ptr->sanity != STR_HASH_SANITY )
	   {
	       fprintf(stderr, "str_alloc: bad sanity\n");
	       assert(ptr->sanity);
	       return NULL;
	   }
#ifdef STRING_HASH_SEMAPHORE
           sem_wait(&string_hash_semaphore[hash]);
#endif
           if ( ptr->links < 65535 )
	       ++ptr->links;
#ifdef STRING_HASH_SEMAPHORE
           sem_post(&string_hash_semaphore[hash]);
#endif
	   if ( ptr->links >= 65535 )
	       fprintf( stderr, "str_alloc: string has %d links: %s\n",
		        ptr->links, str );

           return (char *) ptr+psize;
       }
   if (!(ptr = (struct hashstr_data *) calloc(1,len+1+psize)))
   {
       perror("calloc failure");
       abort();
   }
   ptr->sanity          = STR_HASH_SANITY;
   ptr->links		= 1;
   ptr->length		= len;
   if (len)
     strcpy( (char *) ptr+psize, str );
/*     memcpy( (char *) ptr+psize, str, len+1 ); */
   else
     strcpy( (char *) ptr+psize, "" );

#ifdef STRING_HASH_SEMAPHORE
   sem_wait(&string_hash_semaphore[hash]);
#endif

   ptr->next		= string_hash[hash];
   string_hash[hash]	= ptr;

#ifdef STRING_HASH_SEMAPHORE
   sem_post(&string_hash_semaphore[hash]);
#endif

   return (char *) ptr+psize;
}

/*
 * Used to make a quick copy of a string pointer that is known to be already
 * in the hash table.  Function increments the link count and returns the
 * same pointer passed.
 */
char *quick_link( char *str )
{
    register int hash;
    register struct hashstr_data *ptr;

    ptr = (struct hashstr_data *) (str - sizeof(struct hashstr_data));
    if ( ptr->sanity != STR_HASH_SANITY )
    {
	fprintf(stderr, "quick_link: bad sanity\n");
	assert(ptr->sanity);
        return NULL;
    }
    if ( ptr->links == 0 )
    {
	fprintf(stderr, "quick_link: bad pointer\n" );
        assert(ptr->links);
	return NULL;
    }

    hash = ptr->length % STR_HASH_SIZE;

#ifdef STRING_HASH_SEMAPHORE
    sem_wait(&string_hash_semaphore[hash]);
#endif

    if ( ptr->links < 65535 )
        ++ptr->links;

#ifdef STRING_HASH_SEMAPHORE
    sem_post(&string_hash_semaphore[hash]);
#endif

    return str;
}

/*
 * Used to remove a link to a string in the hash table.
 * If all existing links are removed, the string is removed from the
 * hash table and disposed of.
 * returns how many links are left, or -1 if an error occurred.
 */
int str_free( char *str )
{
    register int len, hash;
    register struct hashstr_data *ptr, *ptr2, *ptr2_next;

    len = strlen(str);
    hash = len % STR_HASH_SIZE;
    ptr = (struct hashstr_data *) (str - sizeof(struct hashstr_data));

    if ( ptr->sanity != STR_HASH_SANITY )
    {
	fprintf(stderr, "str_free: bad sanity\n");
	assert(ptr->sanity);
        return -1;
    }

    if ( ptr->links == 65535 )				/* permanent */
	return ptr->links;
    if ( ptr->links == 0 )
    {
        fprintf(stderr, "str_free: bad pointer\n" );
        assert(ptr->links);
	return -1;
    }

#ifdef STRING_HASH_SEMAPHORE
    sem_wait(&string_hash_semaphore[hash]);
#endif

    if ( --ptr->links == 0 )
    {
	if ( string_hash[hash] == ptr )
	{
	    string_hash[hash] = ptr->next;
	    free(ptr);

#ifdef STRING_HASH_SEMAPHORE
            sem_post(&string_hash_semaphore[hash]);
#endif

	    return 0;
	}
	for ( ptr2 = string_hash[hash]; ptr2; ptr2 = ptr2_next )
	{
	    ptr2_next = ptr2->next;
	    if ( ptr2_next == ptr )
	    {
		ptr2->next = ptr->next;
                free(ptr);

#ifdef STRING_HASH_SEMAPHORE
                sem_post(&string_hash_semaphore[hash]);
#endif

		return 0;
	    }
	}
        fprintf( stderr, "str_free: pointer not found for string: %s\n", str );
        assert(ptr->links);
	return -1;
    }

#ifdef STRING_HASH_SEMAPHORE
    sem_post(&string_hash_semaphore[hash]);
#endif

    return ptr->links;
}

void show_hash( int count )
{
    struct hashstr_data *ptr;
    int x, c;

    for ( x = 0; x < count; x++ )
    {
	for ( c = 0, ptr = string_hash[x]; ptr; ptr = ptr->next, c++ );
	fprintf( stderr, " %d", c );
    }
    fprintf( stderr, "\n" );
}

void hash_dump( int hash )
{
    struct hashstr_data *ptr;
    char *str;
    int c, psize;

    if ( hash > STR_HASH_SIZE || hash < 0 )
    {
	fprintf( stderr, "hash_dump: invalid hash size\n" );
	return;
    }
    psize = sizeof(struct hashstr_data);
    for ( c=0, ptr = string_hash[hash]; ptr; ptr = ptr->next, c++ )
    {
	str = (char *) (((int) ptr) + psize);
	fprintf( stderr, "Len:%4d Lnks:%5d Str: %s\n",
	  ptr->length, ptr->links, str );
    }
    fprintf( stderr, "Total strings in hash %d: %d\n", hash, c );
}

char *check_hash( char *str )
{
   static char buf[1024];
   int len, hash, psize, p = 0, c;
   struct hashstr_data *ptr, *fnd;

   buf[0] = '\0';
   len = strlen(str);
   psize = sizeof(struct hashstr_data);
   hash = len % STR_HASH_SIZE;
   for (fnd = NULL, ptr = string_hash[hash], c = 0; ptr; ptr = ptr->next, c++ )
     if ( len == ptr->length && !strcmp(str,(char *)ptr+psize) )
     {
	fnd = ptr;
	p = c+1;
     }
   if ( fnd )
     sprintf( buf, "Hash info on string: %s\n\rLinks: %d  Position: %d/%d  Hash: %d  Length: %d\n\r",
	  str, fnd->links, p, c, hash, fnd->length );
   else
     sprintf( buf, "%s not found.\n\r", str );
   return buf;
}

char *hash_stats( void )
{
    static char buf[1024];
    struct hashstr_data *ptr;
    int x, c, total, totlinks, unique, bytesused, wouldhave, hilink;

    totlinks = unique = total = bytesused = wouldhave = hilink = 0;
    for ( x = 0; x < STR_HASH_SIZE; x++ )
    {
	for ( c = 0, ptr = string_hash[x]; ptr; ptr = ptr->next, c++ )
	{
	   total++;
	   if ( ptr->links == 1 )
	     unique++;
	   if ( ptr->links > hilink )
	     hilink = ptr->links;
	   totlinks += ptr->links;
	   bytesused += (ptr->length + 1 + sizeof(struct hashstr_data));
	   wouldhave += (ptr->links * (ptr->length + 1));
	}
    }
    sprintf( buf, "Hash strings allocated:%8d  Total links  : %d\n\rString bytes allocated:%8d  Bytes saved  : %d\n\rUnique (wasted) links :%8d  Hi-Link count: %d\n\r",
	total, totlinks, bytesused, wouldhave - bytesused, unique, hilink );
    return buf;
}

void show_high_hash( int top )
{
    struct hashstr_data *ptr;
    int x, psize;
    char *str;

    psize = sizeof(struct hashstr_data);
    for ( x = 0; x < STR_HASH_SIZE; x++ )
	for ( ptr = string_hash[x]; ptr; ptr = ptr->next )
	  if ( ptr->links >= top )
	  {
	     str = (char *) (((int) ptr) + psize);
 	     fprintf( stderr, "Links: %5d  String: >%s<\n\r", ptr->links, str );
	  }
}

void hash_check_sanity( void )
{
    struct hashstr_data *ptr;
    int x;

    for ( x = 0; x < STR_HASH_SIZE; x++ )
	for ( ptr = string_hash[x]; ptr; ptr = ptr->next )
	    if ( ptr->sanity != STR_HASH_SANITY )
	    {
		fprintf(stderr, "hash_check_sanity failed\n");
		assert(ptr->sanity);
                return;
	    }
}
