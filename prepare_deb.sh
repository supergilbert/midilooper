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

mlp_prepdeps ()
{
    if ! mlp_quiet command -v git; then
        mlp_die 1 "Need git (to download submodule(s)) ..."
    fi

    if ! mlp_quiet dpkg -S devscripts; then
        mlp_die 1 "Need devscripts ..."
    fi
}

mlp_checkbuilddeps ()
{
    if ! dpkg-checkbuilddeps ${CURRENT_DIR}/debian/control; then
        mlp_die 1 "Need packages listed upper"
    fi
}

mlp_copy_debian_buster ()
{
    rm -rf ${CURRENT_DIR}/debian
    cp -Rf ${CURRENT_DIR}/src/debian_pkg/debian_buster ${CURRENT_DIR}/debian
    ${CURRENT_DIR}/src/update_debchangelog.sh $CURRENT_DIR $DEB_TYPE
}

SYNOPSIS="\
Usage: $0 <debian-type>

Check and prepare debian package build environment.
(After a success, use debuild or dpkg-buildpackage to generate the package)

debian-type:
  debian-buster   Generate debian buster package directory
  debian-bullseye Generate debian bullseye package directory
  focal           Generate ubuntu focal package directory
  raspbian-buster Generate raspbian buster package directory
"

if [ $# -eq 1 ]; then
    case $1 in
        "raspbian-buster"|"debian-buster"|"focal")
            DEB_TYPE=$1
            mlp_prepdeps
            git submodule update --recursive --init
            mlp_copy_debian_buster
            mlp_checkbuilddeps
            echo "$DEB_TYPE package directory generated"
            ;;
        "debian-bullseye")
            DEB_TYPE=$1
            mlp_prepdeps
            git submodule update --recursive --init
            mlp_copy_debian_buster $1
            echo 10 > debian/compat
            mlp_checkbuilddeps
            echo "$DEB_TYPE package directory generated"
            ;;
        *)
            echo "$SYNOPSIS"
            case $1 in
                "h" | "help" | "-h" | "--help")
                ;;
                *)
                    echo "Unknown command ($1)" >&2
            esac
    esac
else
    echo "$SYNOPSIS"
    mlp_die 1 "Need one argument"
fi
