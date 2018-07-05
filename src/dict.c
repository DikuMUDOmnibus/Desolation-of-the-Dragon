/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "mud.h"
#include "soundex.h"
#include "justify.h"

#define DICT_WORD_LENGTH 40

DICT_ENTRY *create_dict_entry(char *word, char *def)
{
    DICT_ENTRY *dict;

    CREATE(dict, DICT_ENTRY, 1);
    dict->word = str_dup(word);
    dict->definition = str_dup(def);
    dict->next = NULL;

    return dict;
}

DICT_ENTRY *get_words_in_dict(char *word)
{
    DICT_ENTRY *first_dict = NULL, *dict = NULL;
    FILE *fp;
    char soundexkey[16], filen[16], dictword[DICT_WORD_LENGTH], buf[MAX_STRING_LENGTH];
    int len = 0, x;

    if (!*word)
        return NULL;

    if ((len=strlen(word))<3)
        return NULL;

    strcpy(soundexkey, GetSoundexKey(word));
    sprintf(filen,"../dict/real/%c%c%c",
            soundexkey[0], soundexkey[1], soundexkey[2]);

    if (!(fp=fopen(filen,"r")))
        return NULL;

    while (!feof(fp))
    {
        fgets(buf,MAX_STRING_LENGTH-1,fp);
        if (len>3)
        {
            if (tolower(buf[3])<tolower(word[3]))
                continue;
            else if (tolower(buf[3])>tolower(word[3]))
                continue;
        }
        strncpy(dictword, buf, DICT_WORD_LENGTH);
        for (x=38;x>=0;x--)
            if (dictword[x]!=' ')
            {
                dictword[x+1]='\0';
                break;
            }
        buf[strlen(buf)-1]='\0';

        if (!strcasecmp(word,dictword))
        {
            if (first_dict)
            {
                dict->next = create_dict_entry(dictword, &buf[DICT_WORD_LENGTH]);
                dict = dict->next;
            }
            else
            {
                first_dict = create_dict_entry(dictword, &buf[DICT_WORD_LENGTH]);
                dict = first_dict;
            }
        }
    }

    fclose(fp);
    return first_dict;
}

int num_words_in_dict(char *word)
{
    FILE *fp;
    char soundexkey[16], filen[16], dictword[DICT_WORD_LENGTH], buf[MAX_STRING_LENGTH];
    int len = 0, x, num=0;

    if (!*word)
        return 0;

    if ((len=strlen(word))<3)
        return 0;

    strcpy(soundexkey, GetSoundexKey(word));
    sprintf(filen,"../dict/real/%c%c%c",
            soundexkey[0], soundexkey[1], soundexkey[2]);

    if (!(fp=fopen(filen,"r")))
        return 0;

    while (!feof(fp))
    {
        fgets(buf,MAX_STRING_LENGTH-1,fp);
        if (len>3)
        {
            if (tolower(buf[3])<tolower(word[3]))
                continue;
            else if (tolower(buf[3])>tolower(word[3]))
                continue;
        }
        strncpy(dictword, buf, DICT_WORD_LENGTH);
        for (x=38;x>=0;x--)
            if (dictword[x]!=' ')
            {
                dictword[x+1]='\0';
                break;
            }
        buf[strlen(buf)-1]='\0';

        if (!strcasecmp(word,dictword))
        {
            num++;
        }
    }

    fclose(fp);
    return num;
}

void free_dict_entry(DICT_ENTRY *dict)
{
    DISPOSE(dict->word);
    DISPOSE(dict->definition);
    DISPOSE(dict);
}


void do_dlookup(CHAR_DATA *ch, char *argument)
{
    DICT_ENTRY *d1, *d2;
    char buf[MAX_INPUT_LENGTH], *outbuf;
    int num=0;

    buf[0]='\0';
    d1 = get_words_in_dict(argument);

    if (d1)
        pager_printf(ch, "%s\n\r-----------------------\n\r", d1->word);

    while (d1)
    {
        d2 = d1;
        d1 = d1->next;

        outbuf = Justify(d2->definition, 76, justify_right);
        sprintf(buf, "%s\n\r", outbuf);
        sprintf(outbuf, "%d.", ++num);
	strncpy(buf, outbuf, strlen(outbuf));

        send_to_pager(buf, ch);

        free_dict_entry(d2);
    }

    if (!num)
        send_to_char("Nothing found.\n\r", ch);
    else
        pager_printf(ch, "\n\r-----------------------\n\r%d definitions found.\n\r", num);
}
