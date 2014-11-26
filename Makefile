# C part

MSQ_LIB_PATH=./c_api
include $(MSQ_LIB_PATH)/Rules.mk

# end of C


# Python part


PYMSQLIB_NAME=midiseq.so

MIDILOOPER_PATH=./midilooper
PYMSQLIB_DEST=$(MIDILOOPER_PATH)/$(PYMSQLIB_NAME)

PYMSQLIB_PATH=$(MIDILOOPER_PATH)/module
PYMSQLIB_BUILD=$(PYMSQLIB_PATH)/$(PYMSQLIB_NAME)

# No header dependencies
$(PYMSQLIB_DEST) : $(MSQ_LIB) $(wildcard $(PYMSQLIB_PATH)/*.c)
	python $(PYMSQLIB_PATH)/setup.py install --install-lib=$(PYMSQLIB_PATH) --home=$(PYMSQLIB_PATH)
	cp $(PYMSQLIB_BUILD) $(PYMSQLIB_DEST)

.PHONY : clean_pymsq
clean_pymsq :
	python $(PYMSQLIB_PATH)/setup.py clean
	rm -rf $(PYMSQLIB_DEST) $(PYMSQLIB_BUILD) $(PYMSQLIB_PATH)/build $(PYMSQLIB_PATH)/midiseq-0.0.0.egg-info

.PHONY : clean_pyc
clean_pyc :
	find ./ -iname '*pyc' -exec rm -rf {} \;

# end of Python



.PHONY : clean
clean : clean_msq_lib clean_pymsq clean_pyc

.DEFAULT_GOAL=all
all : $(PYMSQLIB_DEST)
