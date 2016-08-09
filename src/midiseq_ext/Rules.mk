MSQ_PYM=$(MSQ_PYM_DIR)/midiseq.so

MAJOR_VERSION=0

MINOR_VERSION=1

MSQ_PYM_SRC=$(MSQ_PYM_DIR)/pym_midiseq_class.c\
	$(MSQ_PYM_DIR)/pym_midiseq_tools.c\
	$(MSQ_PYM_DIR)/pym_midiseq_track.c\
	$(MSQ_PYM_DIR)/pym_midiseq_output.c\
	$(MSQ_PYM_DIR)/pym_midiseq_file.c\
	$(MSQ_PYM_DIR)/pym_midiseq_evwr.c\
	$(MSQ_PYM_DIR)/pym_midiseq.c

MSQ_PYM_OBJ=$(MSQ_PYM_SRC:.c=.o)

MSQ_PYM_DEPS=$(MSQ_PYM_SRC:.c=.d)

PY_PKG=python-2.7

PY_INC=$(shell pkg-config --cflags $(PY_PKG))

$(MSQ_PYM_DEPS): CFLAGS=-Wall -Werror -g -fPIC -I$(MSQ_LIB_DIR) $(PY_INC)
$(MSQ_PYM_DEPS): CC=gcc

$(PY_OBJ): CFLAGS=-fno-strict-aliasing -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes -fPIC -I$(MSQ_LIB_DIR) $(PY_INC)
$(PY_OBJ): CC=gcc

$(MSQ_PYM): CFLAGS=-fno-strict-aliasing -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes -fPIC -I$(MSQ_LIB_DIR) $(PY_INC)
$(MSQ_PYM): CC=gcc

$(MSQ_PYM): $(MSQ_LIB) $(MSQ_PYM_OBJ)
	$(CC) -shared -o $@ $^ -L$(MSQ_LIB_DIR) -lmidiseq -lasound -ljack

.PHONY: clean_pym
clean_pym:
	rm -f $(MSQ_PYM_OBJ) $(MSQ_PYM) $(MSQ_PYM_DEPS)

ifneq ($(findstring clean, $(MAKECMDGOALS)), clean)
-include $(MSQ_PYM_DEPS)
endif
