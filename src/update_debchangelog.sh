#!/bin/sh

# Copyright 2012-2020 Gilbert Romer

# This file is part of midilooper.

# midilooper is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# midilooper is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU Gneneral Public License
# along with midilooper.  If not, see <http://www.gnu.org/licenses/>.

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
