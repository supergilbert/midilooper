#!/bin/sh

MIDILOOPER_PATH=$(dirname $0)/python
export PYTHONPATH=$MIDILOOPER_PATH/msqgtk:$MIDILOOPER_PATH/module
$MIDILOOPER_PATH/msqgtk/midilooper.py
