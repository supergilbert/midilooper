#!/bin/sh -e

CURRENT_DIR=$(dirname $0)
SRC_DIR=${CURRENT_DIR}/src

echo "Testing compilation"
if make -f ${SRC_DIR}/midiseq_ext_dev.mk -s; then
    echo "Compilation OK"
else
    echo "\033[31mError while compiling\033[0m" >&2
    exit 1
fi

MIDILOOPER=${SRC_DIR}/midilooper/scripts/midilooper
export PYTHONPATH=${SRC_DIR}
SYNOPSIS="\
$(basename $0) [COMMAND]

COMMAND:
 ipython   ipython with midilooper environment
 gdb       gdb with midilooper
 gdbcore   gdb with midilooper core file
 gdbemacs  gdb in emacs with midilooper
 emacscore gdb in emacs with midilooper core file.
 emacs     emacs with midilooper environment
 valgrind  valgrind with midilooper
 help      show this help"

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
            gdb --core=$1 --args python3
            ;;
        "gdb")
            echo "\033[32mLanching midilooper with gdb.\033[0m"
            shift 1
            gdb --args python3 $MIDILOOPER $@
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
            valgrind -v --track-origins=yes --log-file=/tmp/midilooper_valgrind.log --leak-check=full --show-reachable=yes python3 $MIDILOOPER $@
            ;;
        "help")
            echo "$SYNOPSIS"
            echo "\n... or the following midilooper options.\n"
            $MIDILOOPER --help
            ;;
        *)
            ulimit -c unlimited
            $MIDILOOPER $@
            ;;
    esac
fi
