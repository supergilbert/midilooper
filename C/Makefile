SRC=asound/aseq.c\
	midi/midifile.c\
	midi/midi_meta_ev.c\
	midi/midi_channel_ev.c\
	midi/midiev_tool.c\
	seqtool/seqtool.c\
	clock/clock.c\
	tool/tool.c\
	debug_tool/debug_tool.c\
	beta_engine/engine.c

OBJ=$(SRC:.c=.o)

MIDISEQLIB=./libmidiseq.a

CFLAGS= -Wall -g -fPIC -I ./
CC=gcc

TEST_SRC=test/test_engine.c
TEST_OBJ=$(TEST_SRC:.c=.o)
TEST=test_midiseq

all : $(MIDISEQLIB)

$(MIDISEQLIB) : $(OBJ)
	ar -rvs $(MIDISEQLIB) $(OBJ)

debug_flags :
	CFLAGS=$(CFLAGS) -DDEBUG_MODE

debug : debug_flags $(OBJ)
	ar -rvs $(MIDISEQLIB) $(OBJ)

$(TEST) :  $(OBJ) $(TEST_OBJ)
	$(CC) -o $(TEST) $(TEST_OBJ) $(OBJ) -lasound
	$(CC) -o $(TEST).efence $(TEST_OBJ) $(OBJ) -lasound -lefence

test : $(TEST)

test_clean :
	rm -f $(TEST_OBJ) $(OBJ) $(TEST) $(TEST).efence

clean : test_clean
	rm -f $(OBJ) $(MIDISEQLIB)
