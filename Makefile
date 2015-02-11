# Trick to enable "make -f"
current_dir := $(patsubst %/,%,$(dir $(firstword $(MAKEFILE_LIST))))

### C part ###

LIB_MSQ_DIR=$(current_dir)/lib_midiseq
include $(LIB_MSQ_DIR)/Rules.mk


### Python part ###

PY_MSQ_DIR=$(current_dir)/python_midiseq
PY_MSQ=$(PY_MSQ_DIR)/midiseq.so

$(PY_MSQ) : $(LIB_MSQ) $(wildcard $(PY_MSQ_DIR)/*.c $(PY_MSQ_DIR)/*.h)
	cd $(PY_MSQ_DIR) ; python ./setup.py clean -a --build-lib=./build --build-temp=./build ; python ./setup.py build --build-lib=./build --build-temp=./build ; cd -

.PHONY : clean_pym
clean_pym :
	cd $(PY_MSQ_DIR) ; python ./setup.py clean -a --build-lib=./build --build-temp=./build ; cd -


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

