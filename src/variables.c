/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "mud.h"

static void free_variable(VARC_DATA *varc, VAR_DATA *var)
{
    if (var->name)
        DISPOSE(var->name);
    if (var->val)
        DISPOSE(var->val);
    UNLINK(var, varc->first_var, varc->last_var, next, prev);
    DISPOSE(var);
}

void free_variables(VARC_DATA *varc)
{
    VAR_DATA *var;

    if (!varc)
        return;

    while ((var = varc->first_var))
        free_variable(varc, var);

    DISPOSE(varc);
}

VAR_DATA *get_var(VARC_DATA *varc, char *name)
{
    VAR_DATA *var;

    if (!varc || !name || !*name)
        return NULL;

    for (var = varc->first_var; var; var = var->next)
        if (!str_cmp(var->name, name))
            return var;

    return NULL;
}

char *get_var_val(VARC_DATA *varc, char *name)
{
    VAR_DATA *var;

    if (!varc || !name || !*name)
        return NULL;

    for (var = varc->first_var; var; var = var->next)
        if (!str_cmp(var->name, name))
            return var->val;

    return NULL;
}

char *get_var_val_ch(CHAR_DATA *ch, char *name)
{
    static char sbuf[MAX_INPUT_LENGTH];

    if (!ch || !name || !*name)
        return NULL;

    if ( *name == '_' )
    {
        switch (LOWER(*name+1))
        {
        case 'e':
            if (!str_cmp(name, "_exp")) sprintf(sbuf, "%d", GET_EXP(ch));
            break;
        case 'i':
            if (!str_cmp(name, "_idle"))
            {
                if (IS_NPC(ch) || !ch->desc)
                    sprintf(sbuf, "%d", 0);
                else
                    sprintf(sbuf, "%d", ch->desc->idle);
            }
            break;
        case 'g':
            if (!str_cmp(name, "_gold")) sprintf(sbuf, "%d", GET_MONEY(ch, CURR_GOLD));
            break;
        case 'm':
            if (!str_cmp(name, "_master"))
            {
                if (ch->master)
                    sprintf(sbuf, "%s", GET_NAME(ch->master));
                else
                    strcpy(sbuf, "");
            }
            break;
        default:
            return NULL;
            break;
        }
    }

    return get_var_val(ch->vars, name);
}

char *get_var_val_obj(OBJ_DATA *obj, char *name)
{
//    static char sbuf[MAX_INPUT_LENGTH];

    if (!obj || !name || !*name)
        return NULL;

    if ( *name == '_' )
    {
        switch (LOWER(*name+1))
        {
        default:
            return NULL;
            break;
        }
    }

    return get_var_val(obj->vars, name);
}

int get_var_val_int(VARC_DATA *varc, char *name)
{
    char *val = get_var_val(varc, name);

    if (val)
        return atoi(val);

    return 0;
}

int get_var_val_int_ch(CHAR_DATA *ch, char *name)
{
    char *val = get_var_val_ch(ch, name);

    if (val)
        return atoi(val);

    return 0;
}

int get_var_val_int_obj(OBJ_DATA *obj, char *name)
{
    char *val = get_var_val_obj(obj, name);

    if (val)
        return atoi(val);

    return 0;
}

static VAR_DATA *new_var(VARC_DATA **varc, char *name, char *val)
{
    VAR_DATA *var;
    char vname[MAX_VAR_NAME_LEN];
    unsigned int x;
    int y = 0;

    if (!name || !*name)
        return NULL;

    if (!*varc)
    {
        CREATE((*varc), VARC_DATA, 1);
        (*varc)->first_var = NULL;
        (*varc)->last_var = NULL;
    }

    CREATE(var, VAR_DATA, 1);
    LINK(var, (*varc)->first_var, (*varc)->last_var, next, prev);

    for (x = 0; x < strlen(name) && x < MAX_VAR_NAME_LEN-1; x++)
        if (isalnum(name[x]) || name[x] == '_')
            vname[y++] = name[x];
    vname[y] = '\0';

    if (vname[0] == '\0')
    {
        free_variable((*varc), var);
        return NULL;
    }

    var->name = str_dup(vname);
    if (val)
        var->val = str_dup(val);

    return var;
}

void set_var(VARC_DATA **varc, char *name, char *val)
{
    VAR_DATA *var, *var2 = NULL;
    char *nval;

    if (!*varc)
        CREATE(*varc, VARC_DATA, 1);

    if (val[0] == '%' && (var2 = get_var((*varc), val+1)))
        nval = var2->val;
    else
        nval = val;

    if (!(var = get_var((*varc), name)))
    {
        if (!(var = new_var(varc, name, nval)))
            return;
        if (str_cmp(var->name, name))
            bug("set_var: NOTE: var '%s' renamed to '%s'", name, var->name);
        return;
    }

    if (var->val)
        DISPOSE(var->val);

    if (nval)
        var->val = str_dup(nval);
}

void set_var_int(VARC_DATA **varc, char *name, int val)
{
    char buf[16];
    sprintf(buf, "%d", val);
    set_var(varc, name, buf);
}

void modify_var(VARC_DATA **varc, char *name, char *modstr)
{
    VAR_DATA *var;
    int value;

    if (!modstr || !*modstr)
        return;

    if (!(var = get_var((*varc), name)))
        return;

    value = atoi(var->val);
    value += dice_parse(NULL, value, modstr);

    set_var_int(varc, name, value);
}

void del_var(VARC_DATA *varc, char *name)
{
    VAR_DATA *var;

    if ((var = get_var(varc, name)))
        free_variable(varc, var);
}

int check_var_equals(VARC_DATA *varc, char *name, char *compareval)
{
    VAR_DATA *var;

    if (!(var = get_var(varc, name)))
        return 0;

    if (!str_cmp(var->val, compareval))
        return 1;

    return 0;
}

char *expand_variables(char *str, CHAR_DATA *ch)
{
    static char outbuf[MAX_STRING_LENGTH];
    char word[MAX_INPUT_LENGTH], varname[MAX_INPUT_LENGTH];
    char *point = outbuf, *origstr = str, *t;
    CHAR_DATA *vch = ch;

    if (!ch)
        return origstr;

    while (*str != '\0')
    {
        if ( *str != '%' && *str != '#' )
        {
            *point++ = *str++;
            continue;
        }

        str = one_argument(str, word);

        t = one_argumentx(word, varname, '(');

        if (varname[0] == '\0' || varname[1] == '\0')
        {
            bug("expand_variables: zero length variable name");
            return origstr;
        }

        if (*t != '\0')
        {
            if (t[strlen(t)-1] == ')')
                t[strlen(t)-1] = '\0';
            else
            {
                bug("expand_variables: missing )");
                return origstr;
            }

            if (!(vch = get_char_world(ch, t)))
            {
                bug("expand_variables: %s not found", t);
                return origstr;
            }
        }
        else
        {
            bug("expand_variables: extra (");
            return origstr;
        }

        if (!(t = get_var_val(vch->vars, &varname[1])))
        {
            bug("expand_variables: variable %s not found on %s", varname, GET_NAME(vch));
            return origstr;
        }

        while (*t != '\0')
            *point++ = *t++;
        *point++ = ' ';
    }
    *point = '\0';

    return outbuf;
}

void fwrite_variables(VARC_DATA *varc, FILE *fp)
{
    VAR_DATA *var;

    if (!varc || !varc->first_var)
        return;

    for (var = varc->first_var; var; var = var->next)
        fprintf(fp,
                "#VARIABLE\n"
                "Name         %s~\n"
                "Value        %s~\n"
                "End\n\n",
                var->name, var->val?var->val:"");
}

void fread_variable(VARC_DATA **varc, FILE *fp)
{
    VAR_DATA *var = NULL;
    const char *word;
    char *str;
    bool fMatch;

    for ( ; ; )
    {
        word   = feof( fp ) ? "End" : fread_word( fp );
        fMatch = FALSE;

        switch ( UPPER(word[0]) )
        {
        case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;
        case 'E':
            if ( !str_cmp( word, "End" ) )
                return;
            break;
        case 'N':
            if ( !str_cmp( word, "Name" ) )
            {
                str = fread_string_nohash(fp);
                var = new_var(varc, str, NULL);
                DISPOSE(str);
                fMatch = TRUE;
                break;
            }
            break;
        case 'V':
            if ( !str_cmp( word, "Value" ) )
            {
                str = fread_string_nohash(fp);
                if (var)
                    set_var(varc, var->name, str);
                else
                    bug("Fread_variable: value without name");
                DISPOSE(str);
                fMatch = TRUE;
                break;
            }
            break;
        }

        if ( !fMatch )
        {
            bug( "Fread_variable: no match: %s", word );
            fread_to_eol(fp);
        }
    }
}

void print_variables(CHAR_DATA *ch, VARC_DATA *varc)
{
    VAR_DATA *var;
    int icnt=0;

    if (!varc || !varc->first_var)
    {
        send_to_char("No variables.\n\r", ch);
        return;
    }

    pager_printf(ch, "%-20s %s\n\r", "[Name]", "[Value]");
    for (var = varc->first_var; var; var = var->next)
    {
        pager_printf(ch, "%-20s %s\n\r",
                     var->name, var->val);
        icnt++;
    }
    pager_printf(ch, "%d variables.\n\r", icnt);
}

void do_variables(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *vch = NULL;
    OBJ_DATA *obj = NULL;
    VARC_DATA *varc;
    VAR_DATA *var;

    if (!argument || !*argument)
    {
        send_to_char("Usage:\n\r"
                     "  variables <char|obj>\n\r"
                     "  variables <char|obj> del <name>\n\r"
                     "  variables <char|obj> set <name> [value]\n\r"
                     "  variables <char|obj> mod <name> [modstr]\n\r",
                     ch);
        return;
    }

    argument = one_argument(argument, arg);

    if (!(vch = get_char_world(ch, arg)))
    {
        if (!(obj = get_obj_world(ch, arg)))
        {
            send_to_char("Can't find that.\n\r", ch);
            return;
        }
        varc = obj->vars;
    }
    else
        varc = vch->vars;

    if (!*argument)
    {
        print_variables(ch, varc);
        return;
    }

    argument = one_argument(argument, arg);

    if (!str_cmp(arg, "del"))
    {
        if (!varc || !varc->first_var)
        {
            send_to_char("No variables to delete.\n\r", ch);
            return;
        }

        if (!(var = get_var(varc, argument)))
        {
            send_to_char("Can't find that variable.\n\r", ch);
            return;
        }

        del_var(varc, argument);

        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "set"))
    {
        argument = one_argument(argument, arg);

        if (arg[0] == '\0')
        {
            send_to_char("Variables must have a name.\n\r", ch);
            return;
        }

        if (!varc)
        {
            if (vch)
            {
                CREATE(vch->vars, VARC_DATA, 1);
                varc = vch->vars;
            }
            else if (obj)
            {
                CREATE(obj->vars, VARC_DATA, 1);
                varc = obj->vars;
            }
        }

        set_var(&varc, arg, argument);

        send_to_char("Ok.\n\r", ch);
        return;
    }

    if (!str_cmp(arg, "mod"))
    {
        if (!varc || !varc->first_var)
        {
            send_to_char("No variables to modify.\n\r", ch);
            return;
        }

        argument = one_argument(argument, arg);

        if (!argument || !*argument)
        {
            send_to_char("Modify it how?\n\r", ch);
            return;
        }

        if (!(var = get_var(varc, arg)))
        {
            send_to_char("Can't find that variable.\n\r", ch);
            return;
        }

        modify_var(&varc, arg, argument);

        send_to_char("Ok.\n\r", ch);
        return;
    }

    do_variables(ch, NULL);

}
