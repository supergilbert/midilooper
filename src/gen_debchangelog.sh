#!/bin/sh

CURRENT_DIR=$(dirname $0)

cp ${CURRENT_DIR}/debian/changelog.in ${CURRENT_DIR}/debian/changelog

cd ${CURRENT_DIR}
VERSION=$(dpkg-parsechangelog -S Version)
if ! (git log --max-count=1 --pretty="%d" | grep tag | grep -q $VERSION); then
    LAST_VERSION=$(dpkg-parsechangelog --show-field Version)
    TIMESTAMP=$(git rev-list --timestamp --max-count=1 HEAD | cut -d' ' -f1)
    debchange --newversion "${LAST_VERSION}.${TIMESTAMP}" "Temporary dev modification"
fi
cd -
