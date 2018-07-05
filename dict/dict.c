#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "soundex.h"

#define TRUE   1
#define FALSE  0

typedef struct dict_entry DICT_ENTRY;

struct dict_entry
{
    char *word;
    char *definition;
    DICT_ENTRY *next;
};


DICT_ENTRY *create_dict_entry(char *word, char *def)
{
    DICT_ENTRY *dict;

    dict = calloc(1, sizeof(DICT_ENTRY));
    dict->word = strdup(word);
    dict->definition = strdup(def);
    dict->next = NULL;

    return dict;
}

DICT_ENTRY *word_in_dict(char *word)
{
    FILE *fp;
    char filen[16];
    char t[4096];
    char w[40];
    int len=0,x;
    DICT_ENTRY *first_dict = NULL, *dict;

    if (!*word)
        return NULL;

    if ((len=strlen(word))<3)
        return NULL;

    strcpy(soundexkey, GetSoundexKey(word));
    sprintf(filen,"out/%c%c%c",
            soundexkey[0], soundexkey[1], soundexkey[2]);

    if (!(fp=fopen(filen,"r")))
        return NULL;

    while (!feof(fp))
    {
        fgets(t,4095,fp);
        if (len>3)
        {
            if (tolower(t[3])<tolower(word[3]))
                continue;
            else if (tolower(t[3])>tolower(word[3]))
                continue;
        }
        strncpy(w, t, 40);
        for (x=38;x>=0;x--)
            if (w[x]!=' ')
            {
                w[x+1]='\0';
                break;
            }
        t[strlen(t)-1]='\0';

        if (!strcasecmp(word,w))
        {
            if (first_dict)
            {
                dict->next = create_dict_entry(w, &t[40]);
                dict = dict->next;
            }
            else
            {
                first_dict = create_dict_entry(w, &t[40]);
                dict = first_dict;
            }
        }
    }

    fclose(fp);
    return first_dict;
}

void CHECK(char *word)
{
    DICT_ENTRY *d, *di;

    d = word_in_dict(word);

    if (!d)
        printf("%s: not in dictionary\n", word);

    for (di=d;di;di=di->next)
        printf("[%s: %s]\n", di->word, di->definition);

}

int main(int argc, char *argv[])
{
    FILE *dictfile, *fp;
    char s[4096], t[4096], w[40];
    char *soundexw;
    int x;

    t[0]=s[0]=w[0]='\0';

    if (!(dictfile=fopen("dict.txt","r")))
    {
        printf("Unable to open dict.txt.\n\n");
        return 1;
    }

    if (argc==2)
        while (!feof(dictfile))
        {
            fgets(t,4095,dictfile);
            strncpy(w, t, 40);
            for (x=38;x>=0;x--)
                if (w[x]!=' ')
                {
                    w[x+1]='\0';
                    break;
                }
            t[strlen(t)-1]='\0';
            if (strlen(t)<4 || strlen(t)>4090)
            {
                printf("ERROR: length %s\n", s);
                continue;
            }
            soundexw = GetSoundexKey(w);
            sprintf(s,"out/%c%c%c",
                    soundexw[0], soundexw[1], soundexw[2]);
            if (!(fp=fopen(s,"a")))
            {
                printf("Unable to append %s.\n\n",s);
                continue;
            }
            fprintf(fp,"%s\n",t);
            fclose(fp);
        }

    fclose(dictfile);

    CHECK("reachable");
    CHECK("grounded");
    CHECK("cruel");
    CHECK("gyrocopter");
    CHECK("helicopter");
    CHECK("gilgamesh");
    CHECK("misconstrued");
    CHECK("pinetop");
    CHECK("altruistic");
    CHECK("captain");
    CHECK("destiny");
    CHECK("cornucopia");
    CHECK("capable");
    CHECK("construent");
    CHECK("archmage");
    CHECK("xenophonbic");
    CHECK("discordianism");
    CHECK("xilophone");
    CHECK("burnacle");
    CHECK("barnacle");
    CHECK("xylophone");
    CHECK("chronos");
    CHECK("astronaut");
    CHECK("zebra");
    CHECK("hyperbole");
    CHECK("monster");
    CHECK("pickle");
    return 0;
}
