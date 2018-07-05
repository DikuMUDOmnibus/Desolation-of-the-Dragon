if USE_BOA
BOA_C_FILES = boa_alias.c boa_cgi.c boa_config.c boa_debug.c boa_get.c \
	boa_hash.c boa_log.c boa_mud.c boa_queue.c boa_read.c \
	boa_request.c boa_response.c boa_signals.c boa_util.c lex.yy.c \
	y.tab.c

BOA_H_FILES = boa.h boa_compat.h boa_defines.h boa_globals.h

y.tab.c y.tab.h: boa_grammar.y boa_globals.h boa_defines.h
	$(YACC) -d boa_grammar.y

lex.yy.c: boa_lexer.l y.tab.h boa_globals.h boa_defines.h
	$(LEX) boa_lexer.l
endif

if USE_IMC
IMC_C_FILES = imc.c

IMC_H_FILES = imc.h imccfg.h
endif

if USE_I3
I3_C_FILES = i3.c

I3_H_FILES = i3.h i3cfg.h
endif

if USE_IRC
IRC_C_FILES = irc.c

IRC_H_FILES = irc.h
endif

if USE_MUDMSG
MUDMSG_C_FILES = mudmsg.c
endif

MUD_C_FILES = act_comm.c act_info.c act_move.c act_obj.c act_wiz.c ansi.c \
	boards.c build.c chess.c christen.c \
	clans.c comm.c comments.c commodity.c const.c currency.c \
	db.c deity.c dict.c fight.c \
	handler.c hashstr.c idale.c ienvy.c iimp.c \
	interp.c intro.c ismaug.c justify.c \
	magic.c magic2.c makeobjs.c map.c mapout.c misc.c mpxset.c \
	mspecial.c mspecial2.c mxp.c \
	mud_comm.c mud_prog.c multclas.c ospecial.c player.c property.c \
	requests.c reset.c river.c rspecial.c \
	save.c shops.c show.c skills.c skills2.c soundex.c sql.c \
	tables.c track.c update.c ackmapper.c \
	id.c rareobj.c quest.c miml.c variables.c bugtrack.c fcompress.c \
	acro.c vtrack.c netstat.c graphviz.c

MUD_H_FILES = global.h mud.h bet.h ansi.h tables.h gsn.h chess.h \
	mspecial.h ospecial.h rspecial.h poly.h material.h mxp.h \
	justify.h soundex.h rareobj.h quest.h bugtrack.h fcompress.h vtrack.h

C_FILES = $(MUD_C_FILES) $(BOA_C_FILES) $(IMC_C_FILES) $(I3_C_FILES) $(IRC_C_FILES) $(MUDMSG_C_FILES)

H_FILES = $(MUD_H_FILES) $(BOA_H_FILES) $(IMC_H_FILES) $(I3_H_FILES) $(IRC_H_FILES) $(MUDMSG_H_FILES)


bin_PROGRAMS = dotd
dotd_SOURCES = $(C_FILES) $(H_FILES)
nodist_dotd_SOURCES = bldnum.c

bldnum.c: $(C_FILES) $(H_FILES)
	TEMP_DOTD_BUILD=`$(AWK) '{printf("%d", $$5);}' bldnum.c` && \
	TEMP_DOTD_BUILD=`expr $$TEMP_DOTD_BUILD + 1` && \
	echo "int build_number(void) { return $$TEMP_DOTD_BUILD ; }" > bldnum.c

update:
	cvs update
	$(MAKE)
