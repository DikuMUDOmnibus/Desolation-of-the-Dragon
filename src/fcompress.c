/*
 * Compressed file functions for use in SMAUG-based MUDs
 *
 * Uses ZLIB functions, will also transparently handle uncompressed files
 *
 * To use these functions in your code:
 * Instead of FILE * use gzFile, use gzopen instead of fopen, and gzclose
 * instead of fclose
 *
 * Also note that gzeof doesn't seem to work exactly like feof, in order
 * for gzeof to return non-zero, you must attempt a read from the file.
 * As a result these functions won't error/exit on EOF's like SMAUG's
 * functions do
 */

#include <string.h>
#include <ctype.h>
#include <zlib.h>
#include "mud.h"
#include "fcompress.h"

extern bool fBootDb;

/*
 * Read a letter from a file.
 */
char gz_fread_letter( gzFile gzfp )
{
    char c;

    do
    {
        if ( gzeof(gzfp) )
        {
            bug("gz_fread_letter: EOF encountered on read");
            if ( fBootDb )
                exit(1);
            return '\0';
        }
        c = gzgetc( gzfp );
    }
    while ( isspace(c) );

    return c;
}



/*
 * Read a number from a file.
 */
int gz_fread_number( gzFile gzfp )
{
    int number;
    bool sign;
    char c;

    do
    {
        if ( gzeof(gzfp) )
        {
            bug("fread_number: EOF encountered on read");
            if ( fBootDb )
                exit(1);
            return 0;
        }
        c = gzgetc( gzfp );
    }
    while ( isspace(c) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
        c = gzgetc( gzfp );
    }
    else if ( c == '-' )
    {
        sign = TRUE;
        c = gzgetc( gzfp );
    }

    if ( !isdigit(c) )
    {
        bug( "gz_fread_number: bad format. %d(%c)", c, c );
        if ( fBootDb )
            exit( 1 );
        return 0;
    }

    while ( isdigit(c) )
    {
        if ( gzeof(gzfp) )
        {
            bug("fread_number: EOF encountered on read");
            if ( fBootDb )
                exit(1);
            return number;
        }
        number = number * 10 + c - '0';
        c      = gzgetc( gzfp );
    }

    if ( sign )
        number = 0 - number;

    if ( c == '|' )
        number += gz_fread_number( gzfp );
//    else if ( c != ' ' && c != '\n')
    else if ( !isspace(c) )
    {
//       bug("gz_fread_number: c != ' ', c == %c, number == %d", c, number);
        gzseek(gzfp, -1, SEEK_CUR);
    }

//    bug("gz_fread_number: %d", number);
    return number;
}


/*
 * Read a string from file gzfp
 */
char *gz_fread_string( gzFile gzfp )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;
    char c;
    int ln;

    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
        if ( gzeof(gzfp) )
        {
            bug("gz_fread_string: EOF encountered on read");
            if ( fBootDb )
                exit(1);
            return STRALLOC("");
        }
        c = gzgetc( gzfp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '~' )
        return STRALLOC( "" );

    for ( ;; )
    {
        if ( ln >= (MAX_STRING_LENGTH - 1) )
        {
            bug( "gz_fread_string: string too long" );
            *plast = '\0';
            return STRALLOC( buf );
        }
        switch ( *plast = gzgetc( gzfp ) )
        {
        default:
            plast++; ln++;
            break;

        case EOF:
            bug( "gz_fread_string: EOF" );
            if ( fBootDb )
                exit( 1 );
            *plast = '\0';
            return STRALLOC(buf);
            break;

        case '\n':
            plast++;  ln++;
            *plast++ = '\r';  ln++;
            break;

        case '\r':
            break;

        case '~':
            *plast = '\0';
            return STRALLOC( buf );
        }
    }
}


/*
 * Read a string from file gzfp using str_dup (ie: no string hashing)
 */
char *gz_fread_string_nohash( gzFile gzfp )
{
    char buf[MAX_STRING_LENGTH];
    char *plast;
    char c;
    int ln;

    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
        if ( gzeof(gzfp) )
        {
            bug("gz_fread_string_nohash: EOF encountered on read");
            if ( fBootDb )
                exit(1);
            return str_dup("");
        }
        c = gzgetc( gzfp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '~' )
        return str_dup( "" );

    for ( ;; )
    {
        if ( ln >= (MAX_STRING_LENGTH - 1) )
        {
            bug( "gz_fread_string_nohash: string too long" );
            *plast = '\0';
            return str_dup( buf );
        }
        switch ( *plast = gzgetc( gzfp ) )
        {
        default:
            plast++; ln++;
            break;

        case EOF:
            bug( "gz_fread_string_nohash: EOF" );
            if ( fBootDb )
                exit( 1 );
            *plast = '\0';
            return str_dup(buf);
            break;

        case '\n':
            plast++;  ln++;
            *plast++ = '\r';  ln++;
            break;

        case '\r':
            break;

        case '~':
            *plast = '\0';
            return str_dup( buf );
        }
    }
}


/*
 * Read to end of line (for comments).
 */
void gz_fread_to_eol( gzFile gzfp )
{
    char c;

    do
    {
        if ( gzeof(gzfp) )
        {
            bug("gz_fread_to_eol: EOF encountered on read");
            if ( fBootDb )
                exit(1);
            return;
        }
        c = gzgetc( gzfp );
    }
    while ( c != '\n' && c != '\r' );

/*    do
    {
        c = gzgetc( gzfp );
    }
    while ( c == '\n' || c == '\r' );

    gzseek(gzfp, -1, SEEK_CUR);*/
    return;
}


/*
 * Read to end of line into buffer			-Thoric
 */
char *gz_fread_line( gzFile gzfp, char *line )
{
    char *pline;
    int x;

    x = 0;

    while ((pline = gzgets(gzfp, line, MAX_STRING_LENGTH-1)))
    {
        if (pline[0] != '\0' && pline[0] != '\n')
            break;
    }

    if (!pline)
    {
        bug("gz_fread_line: EOF encountered on read.");
        if ( fBootDb )
            exit(1);
        line[0] = '\0';
        return line;
    }

    while (isspace(*pline)) pline++;

    if (strlen(pline) >= MAX_STRING_LENGTH)
        bug( "gz_fread_line: line too long" );

    return pline;
}


/*
 * Read one word (into buffer).
 */
char *gz_fread_word( gzFile gzfp, char *word )
{
    char *pword;
    char cEnd;

    do
    {
        if ( gzeof(gzfp) )
        {
            bug("gz_fread_word: EOF encountered on read.");
            if ( fBootDb )
                exit(1);
            word[0] = '\0';
            return word;
        }
        cEnd = gzgetc( gzfp );
    }
    while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
        pword   = word;
    }
    else
    {
        word[0] = cEnd;
        pword   = word+1;
        cEnd    = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
        if ( gzeof(gzfp) )
        {
            bug("gz_fread_word: EOF encountered on read.");
            if ( fBootDb )
                exit(1);
            *pword = '\0';
            return word;
        }
        *pword = gzgetc( gzfp );
        if ( cEnd == ' ' ? isspace(*pword) : *pword == cEnd )
        {
            if ( cEnd == ' ' )
            {
//                bug("gz_fread_word: cEnd == ' '");
                gzseek(gzfp, -1, SEEK_CUR);
            }
            *pword = '\0';
            return word;
        }
    }

    bug( "gz_fread_word: word too long" );
    exit( 1 );
    return NULL;
}
