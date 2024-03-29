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

SRC_DIR=$(realpath $(dirname $0)/..)

VERSION=$(${SRC_DIR}/src/gen_version.sh)

if echo $VERSION | grep -q "modified"; then
    debchange --newversion "$VERSION" "Experimental version with not committed modifications."
else
    if echo $VERSION | grep -q "[[:digit:]]\+\.[[:digit:]]\+\.[[:digit:]]\+"; then
        debchange --newversion "$VERSION" "Untagged version (may be unstable)."
    fi
fi
