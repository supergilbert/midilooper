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

mlp_copy_debian_buster ()
{
    rm -rf ${CURRENT_DIR}/debian
    cp -Rf ${CURRENT_DIR}/src/debian_pkg/debian_buster ${CURRENT_DIR}/debian
    ${CURRENT_DIR}/src/update_debchangelog.sh ${CURRENT_DIR}
}

SYNOPSIS="\
Availble commands:
debian-buster    Generate debian buster directory
debian-bullseye  Generate debian bullseye directory
deb-check        check dependencies (Need a debian directory)
"

if ! mlp_quiet dpkg -S devscripts; then
    mlp_die 1 "Need devscripts ..."
fi

if [ $# -eq 1 ]; then
    case $1 in
        "debian-buster")
            mlp_copy_debian_buster
            mlp_deb_check
            ;;
        "debian-bullseye")
            mlp_copy_debian_buster
            echo 10 > debian/compat
            mlp_deb_check
            ;;
        "deb-check")
            if [ ! -d ${CURRENT_DIR}/debian ]; then
                echo "$SYNOPSIS"
                mlp_die 1 "Need a debian directory"
            fi
            mlp_deb_check
            ;;
        *)
            echo "$SYNOPSIS"
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
