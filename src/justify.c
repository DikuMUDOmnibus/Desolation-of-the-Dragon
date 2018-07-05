/******************************************************************************
 Snippet: Text justification function.
 Author:  Richard Woolcock (aka KaVir).
 Date:    23rd November 1999.
 ******************************************************************************
 This code is copyright (C) 1999 by Richard Woolcock.  It may be used and
 distributed freely, as long as you don't remove this copyright notice.
 ******************************************************************************/

/******************************************************************************
 Remove the following line to use this code in your mud.
 ******************************************************************************/

/*#define STANDALONE_PROGRAM*/

/******************************************************************************
 Required library files.
 ******************************************************************************/

#ifdef STANDALONE_PROGRAM
#define MEMDEBUG
#include "/home/jdefer/memdebug/memdebug.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "justify.h"

/******************************************************************************
 Local operation prototypes.
 ******************************************************************************/

static void AddSpaces ( char **ppszText, int iNumber );

/******************************************************************************
 Standalone main function.
 ******************************************************************************/

#ifdef STANDALONE_PROGRAM
void main( void )
{
   char *pszText = "  a small two-seater Cessna plane crashed into a   cemetery early this afternoon in Mullingarin.irish search and rescue workers have recovered 826 bodies so far and expect that number to climb as digging continues into the night.";
   printf( "Was:[%s]\n\r", pszText );
   printf( "Now:\n\r[%s]\n\r", Justify( pszText, 40, justify_left ) );
   printf( "Now:\n\r[%s]\n\r", Justify( pszText, 40, justify_centre ) );
   printf( "Now:\n\r[%s]\n\r", Justify( pszText, 40, justify_right ) );
}
#endif

/******************************************************************************
 Global operations.
 ******************************************************************************/

/* Function: Justify
 *
 * This function is used to format a piece of text so that it doesn't wrap
 * over the line.  It also allows you to specify the line width and left,
 * right or centre justification.
 *
 * The function takes three parameters, as follows:
 *
 * pszText:    A pointer to the string to be justified.
 * iAlignment: The width in characters of the formatted text.
 * eJustify:   The style of justification: left, right or centre.
 *
 * The return value is a pointer to a non-stack based string which contains
 * the newly formatted text.
 */
char *Justify( char *pszText, int iAlignment, justify_type eJustify )
{
   static char s_szResult[4096];
   char *      pszResult = &s_szResult[0];
   char        szStore[4096];
   int         iMax;
   int         iLength = iAlignment-1;
   int         iLoop = 0;

   if ( strlen( pszText ) < 10 )
   {
      /* You may want to add your own error message routine in here */
      /*      strcpy( s_szResult, "BUG: Justified string cannot be less than 10 characters long." );
        return( &s_szResult[0] );
        */
      return( pszText );
   }

   /* Discard all leading spaces */
   while ( *pszText == ' ' ) pszText++;

   /* Store the character */
   szStore[iLoop++] = *pszText++;

   if ( szStore[iLoop-1] >= 'a' && szStore[iLoop-1] <= 'z' )
   {
      /* Capitalize the first character if it's a letter */
      szStore[iLoop-1] &= ~32;
   }

   /* The first loop goes through the string, copying it into szStore.  The
    * string is formatted to remove all newlines, capitalise new sentences,
    * remove excess white spaces and ensure that full stops, commas and
    * exclaimation marks are all followed by two white spaces.
    */
   while ( *pszText )
   {
      switch ( *pszText )
      {
         default:
            /* Store the character */
            szStore[iLoop++] = *pszText++;
            break;
         case ' ':
            /* There shall only be one space between non-space characters */
            if ( *(pszText+1) != ' ' )
            {
               /* Store the character */
               szStore[iLoop++] = *pszText;
            }
            pszText++;
            break;
         case '.': case '?': case '!':
            /* Store the character */
            szStore[iLoop++] = *pszText++;
            switch ( *pszText )
            {
               default:
                  /* Sentence terminators shall be followed by two spaces */
                  szStore[iLoop++] = ' ';
                  szStore[iLoop++] = ' ';
                  /* Discard all leading spaces */
                  while ( *pszText == ' ' ||
                          *pszText == '\n' ||
                          *pszText == '\r' )
                      pszText++;
                  /* Store the character */
                  szStore[iLoop++] = *pszText++;
                  if ( szStore[iLoop-1] >= 'a' && szStore[iLoop-1] <= 'z' )
                  {
                     /* Capitalize if it's a letter */
                     szStore[iLoop-1] &= ~32;
                  }
                  break;
               case '.': case '?': case '!':
                  /* Multiple terminators shall not be separated by spaces */
                  break;
            }
            break;
         case ',':
            /* Store the character */
            szStore[iLoop++] = *pszText++;
            /* Discard all leading spaces */
            while ( *pszText == ' ' ) pszText++;
            /* Commas shall be followed by one space */
            szStore[iLoop++] = ' ';
            break;
         case '$':
            /* Store the character */
            szStore[iLoop++] = *pszText++;
            /* Discard all leading spaces */
            while ( *pszText == ' ' ) pszText++;
            break;
         case '\n':
            szStore[iLoop++] = ' ';
         case '\r':
            /* Discard newlines and returns */
            pszText++;
            break;
      }
   }

   /* Terminate the string */
   szStore[iLoop] = '\0';

   /* Initialise iMax to the size of szStore */
   iMax = strlen( szStore );

   /* The second loop goes through the string, inserting newlines at every
    * appropriate point.
    */
   while ( iLength < iMax )
   {
      /* Go backwards through the current line searching for a space */
      while ( szStore[iLength] != ' ' && iLength > 1 )
      {
         iLength--;
      }

      if ( szStore[iLength] == ' ' )
      {
         /* If a space is found, replace it with a newline */
         szStore[iLength] = '\n';
         iLength += iAlignment;
      }
      else
      {
         /* If no space is found, drop out of the loop */
         break;
      }
   }

   /* Add spaces to the front of the line as appropriate */
   switch ( eJustify )
   {
      case justify_left:
         /* Do nothing */
         break;
      case justify_right:
         AddSpaces( &pszResult, 80-iAlignment );
         break;
      case justify_centre:
         AddSpaces( &pszResult, (80-iAlignment)/2 );
         break;
   }

   /* Reset the counter */
   iLoop = 0;

   /* The third and final loop goes through the string, making sure that there
    * is a \r (return to beginning of line) following every newline, with no
    * white spaces at the beginning of a particular line of text.
    */
   while ( iLoop < iMax )
   {
      /* Store the character */
      *pszResult++ = szStore[iLoop];
      switch ( szStore[iLoop] )
      {
         default:
            break;
         case '\n':
            /* Insert a return after the newline and remove any leading spaces */
            *pszResult++ = '\r';
            while ( szStore[iLoop+1] == ' ' ) iLoop++;
            /* Add spaces to the front of the line as appropriate */
            switch ( eJustify )
            {
               case justify_left:
                  /* Do nothing */
                  break;
               case justify_right:
                  AddSpaces( &pszResult, 80-iAlignment );
                  break;
               case justify_centre:
                  AddSpaces( &pszResult, (80-iAlignment)/2 );
                  break;
            }
            break;
      }
      iLoop++;
   }

   /* Terminate the string */
   *pszResult++ = '\0';

   return( &s_szResult[0] );
}

/******************************************************************************
 Local operations.
 ******************************************************************************/

/* Function: AddSpaces
 *
 * This function is used to add spaces to the front of a line of text.  It
 * is used for right and centre justification.
 *
 * The function takes two parameters, as follows:
 *
 * ppszText: Pointer to the pointer to the string to have the spaces added to.
 * iNumber:  The number of spaces to be added to the front of the line.
 *
 * There is no return value.
 */
static void AddSpaces( char **ppszText, int iNumber )
{
   int iLoop;

   for ( iLoop = 0; iLoop < iNumber; iLoop++ )
   {
      *(*ppszText)++ = ' ';
   }
}

