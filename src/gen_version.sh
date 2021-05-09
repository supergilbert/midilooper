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

SRC_DIR=$(realpath $(dirname $0)/..)

CHL_VERSION=$(dpkg-parsechangelog -l ${SRC_DIR}/src/debian_pkg/changelog --show-field Version)

# Check if last commit has been tagged with the version
if git -C $SRC_DIR log --max-count=1 --pretty="%d" | grep tag | grep -q $CHL_VERSION; then
    # Check if there is modifications
    if ! git -C $SRC_DIR diff-index --quiet HEAD; then
        MLP_VERSION="${CHL_VERSION}-modified"
    else
        MLP_VERSION="${CHL_VERSION}"
    fi
else
    TIMESTAMP=$(git -C $SRC_DIR rev-list --timestamp --max-count=1 HEAD | cut -d' ' -f1)
    # bis
    if ! git -C $SRC_DIR diff-index --quiet HEAD; then
        MLP_VERSION="${CHL_VERSION}.${TIMESTAMP}-modified"
    else
        MLP_VERSION="${CHL_VERSION}.${TIMESTAMP}"
    fi
fi

echo $MLP_VERSION
