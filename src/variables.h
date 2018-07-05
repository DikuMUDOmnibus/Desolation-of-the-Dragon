/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
      http://www.dotd.com  dotd@dotd.com
 ******************************************************/


typedef struct variable_data             VAR_DATA;
typedef struct variable_collection_data  VARC_DATA;

struct variable_data
{
    VAR_DATA *next;
    VAR_DATA *prev;

    char *name;
    char *val;
};

struct variable_collection_data
{
    VAR_DATA *first_var;
    VAR_DATA *last_var;
};

#define MAX_VAR_NAME_LEN 32

void free_variables(VARC_DATA *varc);
VAR_DATA *get_var(VARC_DATA *varc, char *name);
char *get_var_val(VARC_DATA *varc, char *name);
char *get_var_val_ch(CHAR_DATA *ch, char *name);
char *get_var_val_obj(OBJ_DATA *obj, char *name);
int get_var_val_int(VARC_DATA *varc, char *name);
int get_var_val_int_ch(CHAR_DATA *ch, char *name);
int get_var_val_int_obj(OBJ_DATA *obj, char *name);
void set_var(VARC_DATA **varc, char *name, char *val);
void set_var_int(VARC_DATA **varc, char *name, int val);
void del_var(VARC_DATA *varc, char *name);
int check_var_equals(VARC_DATA *varc, char *name, char *compareval);
void fwrite_variables(VARC_DATA *varc, FILE *fp);
void fread_variable(VARC_DATA **varc, FILE *fp);
char *expand_variables(char *str, CHAR_DATA *ch);
