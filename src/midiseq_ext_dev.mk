# Trick to enable "make -f"
CURRENT_DIR := $(patsubst %/,%,$(dir $(firstword $(MAKEFILE_LIST))))

BUILD_DIR=$(CURRENT_DIR)/build


# Header dependencies implicit rule
$(BUILD_DIR)/%.d: $(CURRENT_DIR)/%.c
	@mkdir -p `dirname $@`
	$(CC) $(CFLAGS) -MM -MT "$(patsubst %.d,%.o,$@)" -MF $@ $<

$(BUILD_DIR)/%.o: $(CURRENT_DIR)/%.c
	@mkdir -p `dirname $@`
	$(CC) $(CFLAGS) -c -o $@ $<

NAME=$(CURRENT_DIR)/midilooper/midiseq.so

MIDISEQ_PATH=$(CURRENT_DIR)/midiseq
MIDISEQEXT_PATH=$(CURRENT_DIR)/midiseq_ext

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
CFLAGS=-pthread -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes -g -fstack-protector-strong -Wformat -Werror=format-security -D_FORTIFY_SOURCE=2 -fPIC -I$(MIDISEQ_PATH) $(shell pkg-config --cflags python3) -D__MLP_OLD_JACK
else
CFLAGS=-pthread -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes -g -fstack-protector-strong -Wformat -Werror=format-security -D_FORTIFY_SOURCE=2 -fPIC -I$(MIDISEQ_PATH) $(shell pkg-config --cflags python3)
endif
CC=gcc
# CC=/usr/bin/x86_64-linux-gnu-gcc

OBJ=$(patsubst $(CURRENT_DIR)/%,$(BUILD_DIR)/%,$(SRC:.c=.o))

TESTDEPS_FILE=$(patsubst $(CURRENT_DIR)/%.c,$(BUILD_DIR)/%.d,$(MIDISEQ_PATH)/midi/midifile.c)
TESTDEPS_PATTERN=$(patsubst ./%,%,$(patsubst $(CURRENT_DIR)/%.c,$(BUILD_DIR)/%.o,$(MIDISEQ_PATH)/midi/midifile.c))

DEPS=$(patsubst $(CURRENT_DIR)/%,$(BUILD_DIR)/%,$(SRC:.c=.d))

$(OBJ): $(DEPS)


.DEFAULT_GOAL=deps_path_not_changed

$(NAME): $(OBJ)
	$(CC) -shared -o $@ $(OBJ) -lasound -ljack

deps_path_not_changed: $(NAME)
	@if [ -r $(TESTDEPS_FILE) ]; then grep -q "$(TESTDEPS_PATTERN)" $(TESTDEPS_FILE) || (echo "Need a clean. Directory seems to have changed." >&2 && false); fi

.PHONY: clean clean_c clean_pyc deps_path_not_changed

clean_c:
	@rm -f $(NAME) $(OBJ) $(DEPS)
	@rm -rf $(BUILD_DIR)
	@echo "C object and dependencies files has bee removed."

clean_pyc:
	@find $(CURRENT_DIR)/midilooper  -iname '*pyc' -exec rm -rf {} \;
	@echo "Python compiled files has been removed."

clean: clean_c clean_pyc

ifneq ($(findstring clean, $(MAKECMDGOALS)), clean)
-include $(DEPS)
endif
