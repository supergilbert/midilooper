#!/bin/sh

MIDILOOPER_PATH=$(dirname $0)/midilooper
export PYTHONPATH=$MIDILOOPER_PATH/pygtk:$MIDILOOPER_PATH/module

if [ $1 = "gdb" ]; then
	echo "\033[31mLanching midilooper with gdb.\033[0m"
	gdb --args python $MIDILOOPER_PATH/pygtk/midilooper.py
else
	$MIDILOOPER_PATH/pygtk/midilooper.py
fi
