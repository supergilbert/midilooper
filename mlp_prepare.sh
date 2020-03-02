#!/bin/sh

CURRENT_DIR=$(dirname $0)

mlp_quiet ()
{
    "$@" > /dev/null 2>&1
}

mlp_die ()
{
    MLP_ERROR_NUM=$1
    shift
    echo "$*" >&2
    exit $MLP_ERROR_NUM
}

if ! mlp_quiet command -v git; then
    mlp_die 1 "Need git (or download manually submodule(s)) ..."
fi

git submodule update --recursive --init

mlp_deb_check ()
{
    if ! dpkg-checkbuilddeps ${CURRENT_DIR}/debian/control; then
        mlp_die 1 "Need packages listed upper"
    fi
}

if [ $# -eq 1 ]; then
    case $1 in
        "debian-buster" | "debian-bullseye")
            if ! mlp_quiet dpkg -S devscripts; then
                mlp_die 1 "Need devscripts ..."
            fi
            rm -rf ${CURRENT_DIR}/debian
            cp -Rf ${CURRENT_DIR}/src/debian_pkg/debian_buster ${CURRENT_DIR}/debian
            ${CURRENT_DIR}/src/update_debchangelog.sh ${CURRENT_DIR}
            mlp_deb_check
            ;;
        "deb-check")
            if [ ! -d ${CURRENT_DIR}/debian ]; then
                mlp_die 1 "Need a debian directory (see help)"
            fi
            mlp_deb_check
            ;;
        *)
            echo "\
Availble commands:
debian-buster    Generate debian buster directory
debian-bullseye  Generate debian bullseye directory
deb-check       check dependencies (Need a debian directory)
"
            case $1 in
                "help" | "h" | "-h" | "--help")
                ;;
                *)
                    echo "Unknown command ($1)" >&2
            esac
    esac
elif [ $# -gt 1 ]; then
    mlp_die 1 "0 or 1 argument"
fi
