/******************************************************************************
 Snippet: Text justification function.
 Author:  Richard Woolcock (aka KaVir).
 Date:    23rd November 1999.
 ******************************************************************************
 This code is copyright (C) 1999 by Richard Woolcock.  It may be used and
 distributed freely, as long as you don't remove this copyright notice.
 ******************************************************************************/

#ifndef JUSTIFY_HEADER
#define JUSTIFY_HEADER

/******************************************************************************
 Required enumerated types.
 ******************************************************************************/

typedef enum
{
   justify_left,
   justify_right,
   justify_centre
} justify_type;

/******************************************************************************
 Global operation prototypes.
 ******************************************************************************/

char *Justify   ( char *szText, int iAlignment, justify_type eJustify );

#endif /* JUSTIFY_HEADER */

