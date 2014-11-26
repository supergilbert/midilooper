
MSQ_LIB=$(MSQ_LIB_PATH)/libmidiseq.a
MSQ_LIB_SRC=$(MSQ_LIB_PATH)/asound/aseq.c\
	$(MSQ_LIB_PATH)/asound/aseq_tool.c\
	$(MSQ_LIB_PATH)/midi/midi_meta_ev.c\
	$(MSQ_LIB_PATH)/midi/midi_channel_ev.c\
	$(MSQ_LIB_PATH)/midi/midifile.c\
	$(MSQ_LIB_PATH)/midi/midifile_get_varlen.c\
	$(MSQ_LIB_PATH)/midi/midifile_tool.c\
	$(MSQ_LIB_PATH)/loop_engine/track_ctx.c\
	$(MSQ_LIB_PATH)/loop_engine/track_req.c\
	$(MSQ_LIB_PATH)/loop_engine/ev_iterator.c\
	$(MSQ_LIB_PATH)/loop_engine/engine.c\
	$(MSQ_LIB_PATH)/loop_engine/engine_midisave.c\
	$(MSQ_LIB_PATH)/seqtool/seqtool.c\
	$(MSQ_LIB_PATH)/clock/clock.c\
	$(MSQ_LIB_PATH)/tool/tool.c\
	$(MSQ_LIB_PATH)/debug_tool/debug_tool.c\
	$(MSQ_LIB_PATH)/debug_tool/dump_trackst.c

MSQ_LIB_DEPS=$(MSQ_LIB_SRC:.c=.d)

CFLAGS=-Wall -Werror -g -fPIC -I $(MSQ_LIB_PATH)
CC=gcc

MSQ_LIB_OBJ=$(MSQ_LIB_SRC:.c=.o)

$(MSQ_LIB) : $(MSQ_LIB_OBJ)
	ar -rvs $@ $^

.PHONY : clean_msq_lib
clean_msq_lib :
	rm -f $(MSQ_LIB) $(MSQ_LIB_OBJ) $(MSQ_LIB_DEPS)

# Header dependencies
%.d : %.c
	$(CC) $(CFLAGS) -MM -MT "$(patsubst %.c,%.o,$<)" -MF $@ $<

ifneq ($(findstring clean, $(MAKECMDGOALS)), clean)
-include $(MSQ_LIB_DEPS)
endif
