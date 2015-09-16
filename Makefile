# Trick to enable "make -f"
current_dir := $(patsubst %/,%,$(dir $(firstword $(MAKEFILE_LIST))))

# Header dependencies
%.d: %.c
	$(CC) $(CFLAGS) -MM -MT "$(patsubst %.c,%.o,$<)" -MF $@ $<

### C part ###

LIB_MSQ_DIR=$(current_dir)/lib_midiseq
include $(LIB_MSQ_DIR)/Rules.mk


### Python part ###

PY_MSQ_DIR=$(current_dir)/python_midiseq
include $(PY_MSQ_DIR)/Rules.mk

### Common part ###

.DEFAULT_GOAL=all
all: pym

pym: $(PY_MSQ)

lib: $(LIB_MSQ)

clean_lib: clean_lib_msq

clean_pyc:
	find $(current_dir)  -iname '*pyc' -exec rm -rf {} \;

clean: clean_lib clean_pym clean_pyc

.PHONY: clean_pyc clean_lib clean pym lib all
