/******************************************************
            Desolation of the Dragon MUD II
      (C) 2001-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/
#include "mud.h"
#include "mxp.h"
#include <string.h>

char mxpprecommand[MAX_INPUT_LENGTH];
char mxpposcommand[MAX_INPUT_LENGTH];

void send_mxp_stylesheet( DESCRIPTOR_DATA *d )
{
    FILE *rpfile;
    int num=0;
    char BUFF[MAX_STRING_LENGTH*2];

    if ((rpfile = fopen(MXP_SS_FILE,"r")) !=NULL) {
        while ((BUFF[num]=fgetc(rpfile)) != EOF)
            num++;
        fclose(rpfile);
        BUFF[num] = 0;
        write_to_buffer(d, BUFF, num);
    }
}

char *mxp_obj_str(CHAR_DATA *ch, OBJ_DATA *obj)
{
    static char mxpbuf[MAX_INPUT_LENGTH];
    char *argument = mxpprecommand;
    char arg[MAX_INPUT_LENGTH];

    if (!MXP_ON(ch) || mxpprecommand[0]=='\0')
        return "";

    if (!strchr(mxpprecommand, '|'))
    {
        sprintf(mxpbuf, MXP_TAG_SECURE"<send \"%s %s %s\">", mxpprecommand, spacetodash(obj->name), mxpposcommand);
        return mxpbuf;
    }

    sprintf(mxpbuf, "%s", MXP_TAG_SECURE"<send href=\"");
    while (*argument && (argument = one_argumentx(argument, arg, '|')))
    {
        sprintf(mxpbuf+strlen(mxpbuf), "%s %s %s|", arg, spacetodash(obj->name), mxpposcommand);

    }
    sprintf(mxpbuf+(strlen(mxpbuf)-1), "\" hint=\"Right-click for menu|%s\">", mxpprecommand);

    return mxpbuf;
}

char *mxp_obj_str_close(CHAR_DATA *ch)
{
    static char mxpbuf[MAX_INPUT_LENGTH];

    if (!MXP_ON(ch) || mxpprecommand[0]=='\0')
        return "";

    sprintf(mxpbuf, "</send>"MXP_TAG_LOCKED);

    return mxpbuf;
}

char *mxp_chan_str(CHAR_DATA *ch, const char *verb)
{
    static char mxpbuf[128];

    if (!MXP_ON(ch) || verb[0]=='\0')
        return "";

    sprintf(mxpbuf, MXP_TAG_SECURE"<%s>", verb);

    return mxpbuf;
}

char *mxp_chan_str_close(CHAR_DATA *ch, const char *verb)
{
    static char mxpbuf[128];

    if (!MXP_ON(ch) || verb[0]=='\0')
        return "";

    sprintf(mxpbuf, "</%s>"MXP_TAG_LOCKED, verb);

    return mxpbuf;
}
