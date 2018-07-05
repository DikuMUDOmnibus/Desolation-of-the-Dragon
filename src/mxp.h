/******************************************************
            Desolation of the Dragon MUD II
      (C) 2001-2002  Jesse DeFer
          http://www.dotd.com  dotd@dotd.com
 ******************************************************/
/* Mud eXtension Protocol Includes 01/06/2001
   Garil@DOTDII aka Jesse DeFer dotd@dotd.com */

#define MXP_VERSION	"0.3"

#define MXP_TAG_OPEN	"\033[0z"
#define MXP_TAG_SECURE	"\033[1z"
#define MXP_TAG_LOCKED	"\033[2z"

#define MXP_TAG_ROOMEXIT              MXP_TAG_SECURE"<RExits>"
#define MXP_TAG_ROOMEXIT_CLOSE        "</RExits>"MXP_TAG_LOCKED

#define MXP_TAG_ROOMNAME              MXP_TAG_SECURE"<RName>"
#define MXP_TAG_ROOMNAME_CLOSE        "</RName>"MXP_TAG_LOCKED

#define MXP_TAG_ROOMDESC              MXP_TAG_SECURE"<RDesc>"
#define MXP_TAG_ROOMDESC_CLOSE        "</RDesc>"MXP_TAG_LOCKED

#define MXP_TAG_PROMPT                MXP_TAG_SECURE"<Prompt>"
#define MXP_TAG_PROMPT_CLOSE          "</Prompt>"MXP_TAG_LOCKED
#define MXP_TAG_HP                    "<Hp>"
#define MXP_TAG_HP_CLOSE              "</Hp>"
#define MXP_TAG_MAXHP                 "<MaxHp>"
#define MXP_TAG_MAXHP_CLOSE           "</MaxHp>"
#define MXP_TAG_MANA                  "<Mana>"
#define MXP_TAG_MANA_CLOSE            "</Mana>"
#define MXP_TAG_MAXMANA               "<MaxMana>"
#define MXP_TAG_MAXMANA_CLOSE         "</MaxMana>"

#define MXP_SS_FILE     "../system/mxp.style"

#define MXP_ON(ch)      IS_PLR2_FLAG(ch, PLR2_MXP)

extern char mxpprecommand[MAX_INPUT_LENGTH];
extern char mxpposcommand[MAX_INPUT_LENGTH];

void send_mxp_stylesheet( DESCRIPTOR_DATA *d );
void mxpdefcommands(char *pre, char *post);
char *mxp_obj_str(CHAR_DATA *ch, OBJ_DATA *obj);
char *mxp_obj_str_close(CHAR_DATA *ch);
char *mxp_chan_str(CHAR_DATA *ch, const char *verb);
char *mxp_chan_str_close(CHAR_DATA *ch, const char *verb);

