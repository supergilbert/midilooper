LIB_MSQ=$(LIB_MSQ_DIR)/libmidiseq.a
LIB_MSQ_SRC=$(LIB_MSQ_DIR)/asound/aseq.c\
	$(LIB_MSQ_DIR)/asound/aseq_tool.c\
	$(LIB_MSQ_DIR)/jack/jack_backend.c\
	$(LIB_MSQ_DIR)/midi/midi_meta_ev.c\
	$(LIB_MSQ_DIR)/midi/midi_channel_ev.c\
	$(LIB_MSQ_DIR)/midi/midifile.c\
	$(LIB_MSQ_DIR)/midi/midifile_get_varlen.c\
	$(LIB_MSQ_DIR)/midi/midi_tool.c\
	$(LIB_MSQ_DIR)/midi/midifile_tool.c\
	$(LIB_MSQ_DIR)/loop_engine/track_ctx.c\
	$(LIB_MSQ_DIR)/loop_engine/output_req.c\
	$(LIB_MSQ_DIR)/loop_engine/ev_iterator.c\
	$(LIB_MSQ_DIR)/loop_engine/engine.c\
	$(LIB_MSQ_DIR)/loop_engine/engine_nanosleep.c\
	$(LIB_MSQ_DIR)/loop_engine/engine_jack.c\
	$(LIB_MSQ_DIR)/loop_engine/engine_midisave.c\
	$(LIB_MSQ_DIR)/loop_engine/engine_binding.c\
	$(LIB_MSQ_DIR)/seqtool/seqtool.c\
	$(LIB_MSQ_DIR)/clock/clock.c\
	$(LIB_MSQ_DIR)/tool/tool.c\
	$(LIB_MSQ_DIR)/debug_tool/debug_tool.c\
	$(LIB_MSQ_DIR)/debug_tool/dump_trackst.c

LIB_MSQ_DEPS=$(LIB_MSQ_SRC:.c=.d)

CFLAGS=-Wall -Werror -g -fPIC -I $(LIB_MSQ_DIR)
CC=gcc

LIB_MSQ_OBJ=$(LIB_MSQ_SRC:.c=.o)

$(LIB_MSQ) : $(LIB_MSQ_OBJ)
	ar -rvs $@ $^

.PHONY : clean_msq_lib
clean_lib_msq :
	rm -f $(LIB_MSQ) $(LIB_MSQ_OBJ) $(LIB_MSQ_DEPS)

# Header dependencies
%.d : %.c
	$(CC) $(CFLAGS) -MM -MT "$(patsubst %.c,%.o,$<)" -MF $@ $<

ifneq ($(findstring clean, $(MAKECMDGOALS)), clean)
-include $(LIB_MSQ_DEPS)
endif
