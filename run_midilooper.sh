#!/bin/sh

MIDILOOPER_PATH=$(dirname $0)/midilooper
export PYTHONPATH=$MIDILOOPER_PATH/pygtk:$MIDILOOPER_PATH/module

if [ $# -lt 1 ]; then
	$MIDILOOPER_PATH/pygtk/midilooper.py
else
	if [ $1 = "gdb" ]; then
		echo "\033[31mLanching midilooper with gdb.\033[0m"
		gdb --args python $MIDILOOPER_PATH/pygtk/midilooper.py
	fi
	if [ $1 = "valgrind" ]; then
		echo "\033[31mLanching midilooper with valgrind.\033[0m"
		shift 1
		#valgrind $@ --leak-check=full --show-reachable=yes python $MIDILOOPER_PATH/pygtk/midilooper.py
		valgrind $@ python $MIDILOOPER_PATH/pygtk/midilooper.py
	fi
fi
