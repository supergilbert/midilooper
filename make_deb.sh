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
    echo "\033[31m$*\033[0m" >&2
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
    cp -Rf ${CURRENT_DIR}/src/debian_pkg ${CURRENT_DIR}/debian
    ${CURRENT_DIR}/src/update_debchangelog.sh $CURRENT_DIR
}

SYNOPSIS="\
Usage: $0 <debian-type> [-n]

Check and prepare debian package build environment.
(After a success, use debuild or dpkg-buildpackage to generate the package)

debian-type:
  debian-buster   Generate debian buster package directory
  debian-bullseye Generate debian bullseye package directory
  focal           Generate ubuntu focal package directory
  raspbian-buster Generate raspbian buster package directory
  glfw            Generate a package avoiding shlib missing glfw dependency info

  -n              No build
"


if [ $# -eq 0 ]; then
    echo "$SYNOPSIS"
    mlp_die 1 "Need an argument"
fi

case $1 in
    "h" | "help" | "-h" | "--help")
        echo "$SYNOPSIS"
        exit 0
        ;;
esac

case $1 in
    "raspbian-buster"|"debian-buster"|"focal"|"buster")
        echo "Generating $1 type debian directory"
        mlp_prepdeps
        git -C $CURRENT_DIR submodule update --recursive --init
        mlp_copy_debian_buster
        mlp_checkbuilddeps
        echo "$1 debian directory done"
        ;;
    "debian-bullseye"|"bullseye")
        echo "Generating $1 type debian directory and version"
        mlp_prepdeps
        git -C $CURRENT_DIR submodule update --recursive --init
        mlp_copy_debian_buster
        echo 10 > ${CURRENT_DIR}/debian/compat
        mlp_checkbuilddeps
        echo "$1 debian directory done"
        ;;
    "glfw")
        echo "Generating $1 type debian directory and version"
        mlp_prepdeps
        git -C $CURRENT_DIR submodule update --recursive --init
        mlp_copy_debian_buster
        echo 10 > ${CURRENT_DIR}/debian/compat
        cp ${CURRENT_DIR}/debian/shlibdeps_ignore_missing_info_rules ${CURRENT_DIR}/debian/rules
        mlp_checkbuilddeps
        echo "$1 debian directory done"
        ;;
    *)
        echo "$SYNOPSIS"
        echo "Unknown command ($1)" >&2
        exit 1
esac

if [ "$2" = "-n" ]; then
    exit 0
fi

echo "Build unsigned package"
cd $CURRENT_DIR
debuild -b -us -uc
cd -
