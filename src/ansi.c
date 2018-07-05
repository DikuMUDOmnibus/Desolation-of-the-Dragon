/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer and Heath Leach
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

/*
 *** DaleMUD	ANSI_PARSER.C
 ***		Parser ansi colors for act();
 */

/*static char rcsid[] = "$Id: ansi.c,v 1.8 2003/12/21 17:20:53 dotd Exp $";*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "ansi.h"
#include "mud.h"

/*	
 $CMBFG, where M is modier, B is back group color and FG is fore 
 $C0001 would be normal, black back, red fore.
 $C1411 would be bold, blue back, light yellow fore 
 */    


char *ansi_parse(char *code )
{
    static char m[MAX_STRING_LENGTH]; 
    char b[512],f[512];
    
    if (!code)
        return(""); 
    /* do modifier */
    switch(code[0]) {
    case '0':sprintf(m,"%s",MOD_NORMAL);
    break;
    case '1':sprintf(m,"%s",MOD_BOLD);
    break;
    case '2':sprintf(m,"%s",MOD_FAINT);
    break;
    /* not used in ansi that I know of */
    case '3':sprintf(m,"%s",MOD_NORMAL);
    break;
    case '4':sprintf(m,"%s",MOD_UNDERLINE);
    break;
    case '5': sprintf(m,"%s",MOD_BLINK);
    break;
    
    case '6': sprintf(m,"%s",MOD_REVERSE);
    break;
    
    default: sprintf(m,"%s",MOD_NORMAL);
    break;
    }
    
    /* do back ground color */
    switch(code[1]) {
    case '0': sprintf(b,"%s",BK_BLACK);
    break;
    case '1': sprintf(b,"%s",BK_RED);
    break;
    case '2': sprintf(b,"%s",BK_GREEN);
    break;
    case '3': sprintf(b,"%s",BK_BROWN);
    break;
    case '4': sprintf(b,"%s",BK_BLUE);
    break;
    case '5': sprintf(b,"%s",BK_MAGENTA);
    break;
    case '6': sprintf(b,"%s",BK_CYAN);
    break;
    case '7': sprintf(b,"%s",BK_LT_GRAY);
    break;
    default:sprintf(b,"%s",BK_BLACK);
    break;
    }
    
    /* do foreground color */
    switch(code[2]) {     
    case '0':  	switch(code[3]) {  		/* 00-09 */
    case '0': sprintf(f,"%s",FG_BLACK);
    break;
    case '1': sprintf(f,"%s",FG_RED);
    break;
    case '2': sprintf(f,"%s",FG_GREEN);
    break;
    case '3': sprintf(f,"%s",FG_BROWN);
    break;
    case '4': sprintf(f,"%s",FG_BLUE);
    break;
    case '5': sprintf(f,"%s",FG_MAGENTA);
    break;
    case '6': sprintf(f,"%s",FG_CYAN);
    break;
    case '7': sprintf(f,"%s",FG_LT_GRAY);
    break;
    case '8': sprintf(f,"%s",FG_DK_GRAY);
    break;
    case '9': sprintf(f,"%s",FG_LT_RED);
    break;
    default: sprintf(f,"%s",FG_DK_GRAY);
    break;
    } break;
    
    case '1':  	switch(code[3]) {  		/* 10-15 */
    case '0': sprintf(f,"%s",FG_LT_GREEN);
    break;
    case '1': sprintf(f,"%s",FG_YELLOW);
    break;
    case '2': sprintf(f,"%s",FG_LT_BLUE);
    break;
    case '3': sprintf(f,"%s",FG_LT_MAGENTA);
    break;
    case '4': sprintf(f,"%s",FG_LT_CYAN);
    break;
    case '5': sprintf(f,"%s",FG_WHITE);
    break;
    default: sprintf(f,"%s",FG_LT_GREEN);
    break;
    } break;
    
    default : sprintf(f,"%s",FG_LT_RED);
    break;  				
    }
    
    
    strcat(m,b); /* add back ground */
    strcat(m,f); /* add foreground */
    
    return(m);
}

void str2ansi(char *p2, char *p1, int start, int stop)
{
    int i,j;
    
    if((start > stop) || (start < 0))
        p2[0] = '\0';    /* null terminate string */
    else
    {
        if (start == stop)
        {
            p2[0] = p1[start];
            p2[1] = '\0';
        }
        else {
            j = 0;
            
            /* start or (start-1) depends on start index */
            /* if starting index for arrays is 0 then use start */
            /* if starting index for arrays is 1 then use start-1 */
            
            for (i=start;i<=stop;i++)   
                p2[j++] = p1[i];
            p2[j] = '\0';    /* null terminate the string */
        }
    }
    
    if (strlen(p2)+1 > 5)
        bug("DOH!");  /* remove this after test period */
}

char *ParseAnsiColors(int UsingAnsi, char *txt)
{
    static char buf[MAX_STRING_LENGTH*4] = "";
    char tmp[MAX_INPUT_LENGTH*4];
    
    register int i,l,f=0;
    
    buf[0]=0;	
    for(i=0,l=0;*txt;) {
        if(*txt=='$' && (toupper(*(txt+1)) == 'C' || 
                         (*(txt+1)=='$' && toupper(*(txt+2)) == 'C'))) {
            if(*(txt+1)=='$')
                txt+=3;
            else
                txt+=2;
            str2ansi(tmp,txt,0,3);
            
            /* if using ANSI */
            if (UsingAnsi)
                strcat(buf,ansi_parse(tmp));
            else
                /* if not using ANSI   */
                strcat(buf,"");   
            
            txt+=4;
            l=strlen(buf);
            f++;
        }
        else {
            buf[l++]=*txt++;
        }
        buf[l]=0;
    }
    /*  if(f && UsingAnsi)
     strcat(buf,ansi_parse("0007")); */
    return buf;
}

char *color_str( sh_int AType, CHAR_DATA *ch )
{
    CHAR_DATA *och;
    
    if (!ch)
        return(atcode_color_str(AType));
    
    if (!ch->desc)
        return("");
    
    och = (ch->desc->original ? ch->desc->original : ch);
    if ( IS_NPC(och) || !IS_SET(och->act, PLR_ANSI) )
        return("");
    
    switch( och->colors[AType] )
    {
    case 0:  return( FG_BLACK );	break;
    case 1:  return( FG_RED );	break;
    case 2:  return( FG_GREEN );	break;
    case 3:  return( FG_BROWN );	break;
    case 4:  return( FG_BLUE );	break;
    case 5:  return( FG_MAGENTA );	break;
    case 6:  return( FG_CYAN );	break;
    case 7:  return( FG_LT_GRAY );	break;
    case 8:  return( FG_DK_GRAY );	break;
    case 9:  return( FG_LT_RED );	break;
    case 10: return( FG_LT_GREEN );	break;
    case 11: return( FG_YELLOW );	break;
    case 12: return( FG_LT_BLUE );	break;
    case 13: return( FG_LT_MAGENTA);break;
    case 14: return( FG_LT_CYAN );	break;
    case 15: return( FG_WHITE );	break;
    
    case 16+0:  return( FG_BLNK_BLACK );	break;
    case 16+1:  return( FG_BLNK_RED );	break;
    case 16+2:  return( FG_BLNK_GREEN );	break;
    case 16+3:  return( FG_BLNK_BROWN );	break;
    case 16+4:  return( FG_BLNK_BLUE );	break;
    case 16+5:  return( FG_BLNK_MAGENTA );	break;
    case 16+6:  return( FG_BLNK_CYAN );	break;
    case 16+7:  return( FG_BLNK_LT_GRAY );	break;
    case 16+8:  return( FG_BLNK_DK_GRAY );	break;
    case 16+9:  return( FG_BLNK_LT_RED );	break;
    case 16+10: return( FG_BLNK_LT_GREEN );	break;
    case 16+11: return( FG_BLNK_YELLOW );	break;
    case 16+12: return( FG_BLNK_LT_BLUE );	break;
    case 16+13: return( FG_BLNK_LT_MAGENTA);break;
    case 16+14: return( FG_BLNK_LT_CYAN );	break;
    case 16+15: return( FG_BLNK_WHITE );	break;
    
    default: return( "\033[m" );	break;
    }
}

char *def_color_str( sh_int AType )
{
    switch( def_color(AType) )
    {
    case 0:  return( FG_BLACK );	break;
    case 1:  return( FG_RED );	break;
    case 2:  return( FG_GREEN );	break;
    case 3:  return( FG_BROWN );	break;
    case 4:  return( FG_BLUE );	break;
    case 5:  return( FG_MAGENTA );	break;
    case 6:  return( FG_CYAN );	break;
    case 7:  return( FG_LT_GRAY );	break;
    case 8:  return( FG_DK_GRAY );	break;
    case 9:  return( FG_LT_RED );	break;
    case 10: return( FG_LT_GREEN );	break;
    case 11: return( FG_YELLOW );	break;
    case 12: return( FG_LT_BLUE );	break;
    case 13: return( FG_LT_MAGENTA);break;
    case 14: return( FG_LT_CYAN );	break;
    case 15: return( FG_WHITE );	break;
    
    case 16+0:  return( FG_BLNK_BLACK );	break;
    case 16+1:  return( FG_BLNK_RED );	break;
    case 16+2:  return( FG_BLNK_GREEN );	break;
    case 16+3:  return( FG_BLNK_BROWN );	break;
    case 16+4:  return( FG_BLNK_BLUE );	break;
    case 16+5:  return( FG_BLNK_MAGENTA );	break;
    case 16+6:  return( FG_BLNK_CYAN );	break;
    case 16+7:  return( FG_BLNK_LT_GRAY );	break;
    case 16+8:  return( FG_BLNK_DK_GRAY );	break;
    case 16+9:  return( FG_BLNK_LT_RED );	break;
    case 16+10: return( FG_BLNK_LT_GREEN );	break;
    case 16+11: return( FG_BLNK_YELLOW );	break;
    case 16+12: return( FG_BLNK_LT_BLUE );	break;
    case 16+13: return( FG_BLNK_LT_MAGENTA);break;
    case 16+14: return( FG_BLNK_LT_CYAN );	break;
    case 16+15: return( FG_BLNK_WHITE );	break;
    
    default: return( "\033[m" );	break;
    }
}

char *atcode_color_str( sh_int AType )
{
    switch( def_color(AType) )
    {
    case 0:  return( "&w" );	break;
    case 1:  return( "&r" );	break;
    case 2:  return( "&g" );	break;
    case 3:  return( "&O" );	break;
    case 4:  return( "&b" );	break;
    case 5:  return( "&p" );	break;
    case 6:  return( "&c" );	break;
    case 7:  return( "&w" );	break;
    case 8:  return( "&z" );	break;
    case 9:  return( "&R" );	break;
    case 10: return( "&G" );	break;
    case 11: return( "&Y" );	break;
    case 12: return( "&B" );	break;
    case 13: return( "&P" );	break;
    case 14: return( "&C" );	break;
    case 15: return( "&W" );	break;
    
    default: return( "&w" );	break;
    }
}

char *uncolorify(const char *arg)
{
    static char retbuf[MAX_STRING_LENGTH];
    const char *c;
    char buf[8];
    
    retbuf[0] = '\0';
    
    for (c=arg;*c;)
    {
        if (*c=='&')
            strcat(retbuf,"&");
        sprintf(buf,"%c",*c++);
        strcat(retbuf,buf);
    }
    
    return(retbuf);
}

int strlen_color(const char *str)
{
    unsigned int len, i;

    if (!str || !*str)
	return 0;

    for (len = i = 0; len < strlen(str); len++)
    {
	if (str[len] == '&')
	{
	    len++;
            continue;
	}
        i++;
    }

    return i;
}

char *center_str_color(const char *str, int width)
{
    static char retbuf[MAX_INPUT_LENGTH];
    int offset, rpos, spos;

    if (width <= 0)
        return "(center_str_color: bad width)";

    width = UMIN(width, MAX_INPUT_LENGTH);

    if (strlen_color(str) < width )
        offset = URANGE(0, ((width - strlen_color(str)) / 2), width/2);
    else
        offset = 0;

    width = UMIN(width+(strlen(str)-strlen_color(str)), MAX_INPUT_LENGTH);

    rpos = 0;
    while (rpos < offset)
        retbuf[rpos++] = ' ';

    spos = 0;
    while (str[spos] != '\0' && rpos < width)
        retbuf[rpos++] = str[spos++];

    while (rpos < width)
        retbuf[rpos++] = ' ';

    retbuf[width-1] = '\0';

    return(retbuf);
}
