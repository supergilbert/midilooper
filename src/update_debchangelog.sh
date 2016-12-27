#!/bin/sh

export DEBEMAIL="$(git config --get user.email)"
export DEBFULLNAME="$(git config --get user.name)"

CURRENT_DIR=$(dirname $0)

cd ${CURRENT_DIR}
VERSION=$(dpkg-parsechangelog -S Version)
if ! (git log --max-count=1 --pretty="%d" | grep tag | grep -q $VERSION); then
    LAST_VERSION=$(dpkg-parsechangelog --show-field Version)
    TIMESTAMP=$(git rev-list --timestamp --max-count=1 HEAD | cut -d' ' -f1)
    debchange --newversion "${LAST_VERSION}.${TIMESTAMP}" "Temporary dev modification"
fi
cd -
