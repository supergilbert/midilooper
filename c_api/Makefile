SRC=asound/aseq.c\
	asound/aseq_tool.c\
	midi/midi_meta_ev.c\
	midi/midi_channel_ev.c\
	midi/midifile.c\
	midi/midifile_get_varlen.c\
	midi/midifile_tool.c\
	loop_engine/track_ctx.c\
	loop_engine/track_req.c\
	loop_engine/ev_iterator.c\
	loop_engine/engine.c\
	loop_engine/engine_midisave.c\
	seqtool/seqtool.c\
	clock/clock.c\
	tool/tool.c\
	debug_tool/debug_tool.c\
	debug_tool/dump_trackst.c

OBJ=$(SRC:.c=.o)

MIDISEQLIB=./libmidiseq.a

CFLAGS=-Wall -Werror -g -fPIC -I ./
CC=gcc

all : $(MIDISEQLIB)

$(MIDISEQLIB) : $(OBJ)
	ar -rvs $(MIDISEQLIB) $(OBJ)

clean :
	rm -f $(OBJ) $(MIDISEQLIB)

debug_flags :
	CFLAGS=$(CFLAGS) -DDEBUG_MODE

debug : debug_flags $(OBJ)
	ar -rvs $(MIDISEQLIB) $(OBJ)
