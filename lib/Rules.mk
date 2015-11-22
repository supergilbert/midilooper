MSQ_LIB=$(MSQ_LIB_DIR)/libmidiseq.a
MSQ_LIB_SRC=$(MSQ_LIB_DIR)/asound/aseq.c\
	$(MSQ_LIB_DIR)/asound/aseq_tool.c\
	$(MSQ_LIB_DIR)/jack/jack_backend.c\
	$(MSQ_LIB_DIR)/midi/midi_meta_ev.c\
	$(MSQ_LIB_DIR)/midi/midi_channel_ev.c\
	$(MSQ_LIB_DIR)/midi/midifile.c\
	$(MSQ_LIB_DIR)/midi/midifile_get_varlen.c\
	$(MSQ_LIB_DIR)/midi/midi_tool.c\
	$(MSQ_LIB_DIR)/midi/midifile_tool.c\
	$(MSQ_LIB_DIR)/loop_engine/track_ctx.c\
	$(MSQ_LIB_DIR)/loop_engine/output_req.c\
	$(MSQ_LIB_DIR)/loop_engine/engine_nanosleep.c\
	$(MSQ_LIB_DIR)/loop_engine/engine_jack.c\
	$(MSQ_LIB_DIR)/loop_engine/engine.c\
	$(MSQ_LIB_DIR)/loop_engine/engine_midisave.c\
	$(MSQ_LIB_DIR)/loop_engine/engine_binding.c\
	$(MSQ_LIB_DIR)/loop_engine/midi_ring_buffer.c\
	$(MSQ_LIB_DIR)/seqtool/seqtool.c\
	$(MSQ_LIB_DIR)/seqtool/ev_iterator.c\
	$(MSQ_LIB_DIR)/clock/clock.c\
	$(MSQ_LIB_DIR)/tool/tool.c\
	$(MSQ_LIB_DIR)/debug_tool/debug_tool.c\
	$(MSQ_LIB_DIR)/debug_tool/dump_trackst.c

MSQ_LIB_OBJ=$(MSQ_LIB_SRC:.c=.o)

MSQ_LIB_DEPS=$(MSQ_LIB_SRC:.c=.d)

$(MSQ_LIB_DEPS): CFLAGS=-Wall -Werror -g -fPIC -I$(MSQ_LIB_DIR)
$(MSQ_LIB_DEPS): CC=gcc

$(MSQ_LIB): CFLAGS=-Wall -Werror -g -fPIC -I$(MSQ_LIB_DIR)
$(MSQ_LIB): CC=gcc

$(MSQ_LIB_OBJ): CFLAGS=-Wall -Werror -g -fPIC -I$(MSQ_LIB_DIR)
$(MSQ_LIB_OBJ): CC=gcc

$(MSQ_LIB): $(MSQ_LIB_OBJ)
	ar -rs $@ $^

.PHONY: clean_lib
clean_lib:
	rm -f $(MSQ_LIB) $(MSQ_LIB_OBJ) $(MSQ_LIB_DEPS)

ifneq ($(findstring clean, $(MAKECMDGOALS)), clean)
-include $(MSQ_LIB_DEPS)
endif
