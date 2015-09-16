PY_MSQ=$(PY_MSQ_DIR)/midiseq.so

MAJOR_VERSION=0

MINOR_VERSION=1

PY_MSQ_SRC=$(PY_MSQ_DIR)/pym_midiseq_class.c\
	$(PY_MSQ_DIR)/pym_midiseq_tools.c\
	$(PY_MSQ_DIR)/pym_midiseq_track.c\
	$(PY_MSQ_DIR)/pym_midiseq_output.c\
	$(PY_MSQ_DIR)/pym_midiseq_file.c\
	$(PY_MSQ_DIR)/pym_midiseq_evwr.c\
	$(PY_MSQ_DIR)/pym_midiseq.c

PY_MSQ_OBJ=$(PY_MSQ_SRC:.c=.o)

PY_MSQ_DEPS=$(PY_MSQ_SRC:.c=.d)

PY_PKG=python-2.7

PY_INC=$(shell pkg-config --cflags $(PY_PKG))

$(PY_MSQ_DEPS): CFLAGS=-Wall -Werror -g -fPIC -I$(LIB_MSQ_DIR) $(PY_INC)
$(PY_MSQ_DEPS): CC=gcc

$(PY_OBJ): CFLAGS=-fno-strict-aliasing -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes -fPIC -I$(LIB_MSQ_DIR) $(PY_INC)
$(PY_OBJ): CC=gcc

$(PY_MSQ): CFLAGS=-fno-strict-aliasing -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes -fPIC -I$(LIB_MSQ_DIR) $(PY_INC)
$(PY_MSQ): CC=gcc

$(PY_MSQ): $(LIB_MSQ) $(PY_MSQ_OBJ)
	$(CC) -shared -o $@ $^ -L$(LIB_MSQ_DIR) -lmidiseq -lasound -ljack

.PHONY: clean_pym
clean_pym:
	rm -f $(PY_MSQ_OBJ) $(PY_MSQ) $(PY_MSQ_DEPS)

ifneq ($(findstring clean, $(MAKECMDGOALS)), clean)
-include $(PY_MSQ_DEPS)
endif
