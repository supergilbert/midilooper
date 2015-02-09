# Trick to enable "make -f"
current_dir := $(patsubst %/,%,$(dir $(firstword $(MAKEFILE_LIST))))

### C part ###

LIB_MSQ_DIR=$(current_dir)/lib_midiseq
include $(LIB_MSQ_DIR)/Rules.mk


### Python part ###

PY_MSQ_DIR=$(current_dir)/python_midiseq
PY_MSQ=$(PY_MSQ_DIR)/midiseq.so

$(PY_MSQ) : $(LIB_MSQ) $(wildcard $(PY_MSQ_DIR)/*.c $(PY_MSQ_DIR)/*.h)
	python $(PY_MSQ_DIR)/setup.py clean -a
	python $(PY_MSQ_DIR)/setup.py install --install-lib=$(PY_MSQ_DIR)/

.PHONY : clean_pym
clean_pym :
	python $(PY_MSQ_DIR)/setup.py clean -a
	rm -rf $(PY_MSQ) $(PY_MSQ_DIR)/midiseq-0.0.0.egg-info


### Common part ###

.DEFAULT_GOAL=all
all : pym

pym : $(PY_MSQ)

lib : $(LIB_MSQ)

clean_lib : clean_lib_msq

clean_pyc :
	find $(current_dir)  -iname '*pyc' -exec rm -rf {} \;

clean : clean_lib clean_pym clean_pyc

.PHONY : lib_msq py_msq py_msq clean_pyc

