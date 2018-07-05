/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
      http://www.dotd.com  dotd@dotd.com
 ******************************************************/


typedef	struct	introduction_data      	INTRO_DATA;

struct introduction_data
{
    INTRO_DATA *next;
    INTRO_DATA *prev;

    char *real_name;
    char *intro_name;
    char *recog_name;

    time_t when;
};

void free_intros(CHAR_DATA *ch);
void free_intro(INTRO_DATA *intro);
void fwrite_introductions(CHAR_DATA *ch, FILE *fp);
void fread_introductions(CHAR_DATA *ch, FILE *fp);

char *PERS(CHAR_DATA *ch, CHAR_DATA *looker);
char *PERSLONG(CHAR_DATA *ch, CHAR_DATA *looker);
