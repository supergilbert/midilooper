# Trick to enable "make -f"
current_dir := $(patsubst %/,%,$(dir $(firstword $(MAKEFILE_LIST))))

# Header dependencies implicit rule
%.d: %.c
	$(CC) $(CFLAGS) -MM -MT "$(patsubst %.c,%.o,$<)" -MF $@ $<

NAME=$(current_dir)/midilooper/midiseq.so

MIDISEQ_PATH=$(current_dir)/midiseq
MIDISEQEXT_PATH=$(current_dir)/midiseq_ext

SRC=$(MIDISEQ_PATH)/asound/aseq.c\
	$(MIDISEQ_PATH)/asound/aseq_tool.c\
	$(MIDISEQ_PATH)/jack/jack_backend.c\
	$(MIDISEQ_PATH)/midi/midi_meta_ev.c\
	$(MIDISEQ_PATH)/midi/midi_channel_ev.c\
	$(MIDISEQ_PATH)/midi/midifile.c\
	$(MIDISEQ_PATH)/midi/midifile_get_varlen.c\
	$(MIDISEQ_PATH)/midi/midi_tool.c\
	$(MIDISEQ_PATH)/midi/midifile_tool.c\
	$(MIDISEQ_PATH)/loop_engine/track_ctx.c\
	$(MIDISEQ_PATH)/loop_engine/output_req.c\
	$(MIDISEQ_PATH)/loop_engine/engine_nanosleep.c\
	$(MIDISEQ_PATH)/loop_engine/engine_jack.c\
	$(MIDISEQ_PATH)/loop_engine/engine.c\
	$(MIDISEQ_PATH)/loop_engine/engine_midisave.c\
	$(MIDISEQ_PATH)/loop_engine/engine_binding.c\
	$(MIDISEQ_PATH)/loop_engine/midi_ring_buffer.c\
	$(MIDISEQ_PATH)/seqtool/seqtool.c\
	$(MIDISEQ_PATH)/seqtool/ev_iterator.c\
	$(MIDISEQ_PATH)/clock/clock.c\
	$(MIDISEQ_PATH)/tool/tool.c\
	$(MIDISEQ_PATH)/debug_tool/debug_tool.c\
	$(MIDISEQ_PATH)/debug_tool/dump_trackst.c\
	$(MIDISEQEXT_PATH)/pym_midiseq_class.c\
	$(MIDISEQEXT_PATH)/pym_midiseq_tools.c\
	$(MIDISEQEXT_PATH)/pym_midiseq_track.c\
	$(MIDISEQEXT_PATH)/pym_midiseq_output.c\
	$(MIDISEQEXT_PATH)/pym_midiseq_file.c\
	$(MIDISEQEXT_PATH)/pym_midiseq_evwr.c\
	$(MIDISEQEXT_PATH)/pym_midiseq.c

ifneq ($(OLDJACKAPI),)
CFLAGS=-fno-strict-aliasing -DNDEBUG -g -fwrapv -O2 -Wall -Werror -Wstrict-prototypes -fPIC -I$(MIDISEQ_PATH) $(shell pkg-config --cflags python-2.7) -D__MLP_OLD_JACK
else
CFLAGS=-fno-strict-aliasing -DNDEBUG -g -fwrapv -O2 -Wall -Werror -Wstrict-prototypes -fPIC -I$(MIDISEQ_PATH) $(shell pkg-config --cflags python-2.7)
endif
CC=gcc

OBJ=$(SRC:.c=.o)

DEPS=$(SRC:.c=.d)

.DEFAULT_GOAL=$(NAME)

$(NAME): $(OBJ)
	$(CC) -shared -o $@ $^ -lasound -ljack

.PHONY: clean clean_c clean_pyc

clean_c:
	@rm -f $(NAME) $(OBJ) $(DEPS)
	@echo "C object and dependencies files has bee removed."

clean_pyc:
	@find $(current_dir)/midilooper  -iname '*pyc' -exec rm -rf {} \;
	@echo "Python compiled files has been removed."

clean: clean_c clean_pyc

ifneq ($(findstring clean, $(MAKECMDGOALS)), clean)
-include $(DEPS)
endif
