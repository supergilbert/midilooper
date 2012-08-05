SRC=asound/aseq.c\
	midi/midi_meta_ev.c\
	midi/midi_channel_ev.c\
	midi/midifile.c\
	midi/midifile_get_varlen.c\
	midi/midifile_tool.c\
	loop_engine/engine.c\
	loop_engine/engine_midisave.c\
	seqtool/seqtool.c\
	clock/clock.c\
	tool/tool.c\
	debug_tool/debug_tool.c

OBJ=$(SRC:.c=.o)

MIDISEQLIB=./libmidiseq.a

CFLAGS=-Wall -g -fPIC -I ./
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
