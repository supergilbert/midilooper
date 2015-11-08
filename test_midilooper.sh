#!/bin/sh -e

make

MIDILOOPER=$(dirname $0)/python/midilooper.py
export PYTHONPATH=$(dirname $0)/python_module

if [ $# -lt 1 ]; then
	ulimit -c unlimited
	$MIDILOOPER
else
	case $1 in
	"ipython")
		ipython
		;;
	"gdbcore")
		if [ $# -ne 2 ]; then
			echo "\033[31mgdbcore need an argument.\033[0m"
			exit 2
		fi
		echo "\033[32mLanching core file with gdb.\033[0m"
		shift 1
		gdb --core=$1 --args python
		;;
	"gdb")
		echo "\033[32mLanching midilooper with gdb.\033[0m"
		shift 1
		gdb --args python $MIDILOOPER $@
		;;
	"gdbemacs")
		echo "\033[32mLanching midilooper with gdb in emacs.\033[0m"
                shift 1
		emacs --eval "(gdb \"gdb -i=mi --args python $MIDILOOPER $@\")"
		;;
	"emacscore")
		if [ $# -ne 2 ]; then
			echo "\033[31memacscore need an argument.\033[0m"
			exit 3
		fi
		echo "\033[32mLanching midilooper with gdb in emacs.\033[0m"
                shift 1
		emacs --eval "(gdb \"gdb -i=mi --core=$1 --args python\")"
		;;
	"emacs")
		echo "\033[32mLanching emacs with midilooper environment.\033[0m"
                shift 1
		emacs
		;;
	"valgrind")
		echo "\033[32mLanching midilooper with valgrind.\033[0m"
		shift 1
		valgrind -v --track-origins=yes --log-file=/tmp/midilooper_valgrind.log --leak-check=full --show-reachable=yes python $MIDILOOPER $@
		;;
	*)
		ulimit -c unlimited
		$MIDILOOPER $@
		;;
	esac
fi
