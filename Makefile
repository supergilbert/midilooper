# Trick to enable "make -f"
current_dir := $(patsubst %/,%,$(dir $(firstword $(MAKEFILE_LIST))))

# Header dependencies implicit rule
%.d: %.c
	$(CC) $(CFLAGS) -MM -MT "$(patsubst %.c,%.o,$<)" -MF $@ $<

### Python part ###

MSQ_PYM_DIR=$(current_dir)/python_module
include $(MSQ_PYM_DIR)/Rules.mk

### C part ###

MSQ_LIB_DIR=$(current_dir)/lib
include $(MSQ_LIB_DIR)/Rules.mk

### Common part ###

.DEFAULT_GOAL=all
all: lib pym

lib: $(MSQ_LIB)

pym: $(MSQ_PYM)

clean_pyc:
	find $(current_dir)  -iname '*pyc' -exec rm -rf {} \;

clean: clean_pyc clean_pym clean_lib

.PHONY: clean_pyc clean_lib clean pym lib all
