#!/bin/sh

MIDILOOPER_PATH=$(dirname $0)/midilooper
export PYTHONPATH=$MIDILOOPER_PATH

if [ $# -lt 1 ]; then
	ulimit -c unlimited
	$MIDILOOPER_PATH/midilooper.py
else
	case $1 in
	"ipython")
		ipython
		;;
	"gdbcore")
		echo "\033[31mLanching core file with gdb.\033[0m"
		shift 1
		gdb --core=$1 --args python
		;;
	"gdb")
		echo "\033[31mLanching midilooper with gdb.\033[0m"
		shift 1
		gdb --args python $MIDILOOPER_PATH/midilooper.py $@
		;;
	"valgrind")
		echo "\033[31mLanching midilooper with valgrind.\033[0m"
		shift 1
		valgrind -v --track-origins=yes --log-file=/tmp/midilooper_valgrind.log --leak-check=full --show-reachable=yes python $MIDILOOPER_PATH/midilooper.py $@
		;;
	*)
		ulimit -c unlimited
		$MIDILOOPER_PATH/midilooper.py $@
		;;
	esac
fi
