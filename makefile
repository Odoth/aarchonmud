CC      = gcc
PROF	= -I/home/m256ada/include -L/home/m256ada/lib -O
NOCRYPT =
MKTIME	:= \""$(shell date)"\"
BRANCH	:= \""$(shell hg branch)"\"
PARENT	:= \""$(shell hg summary | grep parent | sed 's/parent: //')"\"

C_FLAGS =  -ggdb -rdynamic -w -Wall $(PROF) $(NOCRYPT) -DMKTIME=$(MKTIME) -DBRANCH=$(BRANCH) -DPARENT=$(PARENT)
L_FLAGS =  $(PROF) -llua -ldl

O_FILES = act_comm.o act_enter.o act_info.o act_move.o act_obj.o act_wiz.o \
     alchemy.o alias.o auth.o ban.o bit.o board.o buffer.o clanwar.o comm.o const.o crafting.o db.o db2.o \
     enchant.o effects.o fight.o fight2.o flags.o ftp.o handler.o healer.o hunt.o \
     interp.o lookup.o magic.o magic2.o mem.o mob_cmds.o mob_prog.o \
     nanny.o olc.o olc_act.o olc_mpcode.o olc_save.o passive.o penalty.o pipe.o quest.o \
     ranger.o recycle.o redit-ilab.o remort.o bsave.o scan.o skills.o\
     smith.o social-edit.o special.o stats.o string.o tables.o update.o \
     freeze.o warfare.o  grant.o wizlist.o marry.o forget.o clan.o \
     buildutil.o buffer_util.o simsave.o breath.o tflag.o grep.o vshift.o \
     tattoo.o religion.o playback.o leaderboard.o mob_stats.o \
     mt19937ar.o lua_scripting.o lua_bits.o olc_opcode.o obj_prog.o\
     olc_apcode.o area_prog.o protocol.o

aeaea:  
tester: C_FLAGS += -DTESTER
builder: C_FLAGS += -DBUILDER
remort: C_FLAGS += -DREMORT 
remort_tester: C_FLAGS += -DREMORT -DTESTER

aeaea tester builder remort remort_tester: $(O_FILES)
	rm -f aeaea 
	$(CC) -o aeaea $(O_FILES) $(L_FLAGS) -lcrypt -lm

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<

clean:
	rm *.o
