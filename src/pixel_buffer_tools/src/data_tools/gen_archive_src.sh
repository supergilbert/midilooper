#!/bin/sh -e

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

CURRENT_NAME="$(basename $0)"

if [ $# -ne 2 ]
then
    echo "\
${CURRENT_NAME}: error: Wrong synopsis
  This script need an archive name and a file object as argument." >&2
    exit 1
fi

archive_name="$1"

archive_begin="_${archive_name}_begin"
archive_end="_${archive_end}_end"
archive_func="_get_${archive_name}"

archive_hdr="${archive_name}.h"
archive_src="${archive_name}.c"
archive_lds="${archive_name}.lds"
archive_data="$2"

echo "Creation of $archive_hdr."
echo "\
#pragma once
#ifdef __cplusplus
extern \"C\"
{
#endif  /* __cplusplus */

unsigned char *${archive_func}_ptr(void);
unsigned char *${archive_func}_end(void);
unsigned int   ${archive_func}_len(void);

#ifdef __cplusplus
}      /* extern \"C\" */
#endif /* __cplusplus */
" > $archive_hdr

echo "Creation of $archive_src."
echo "\
#include \"${archive_hdr}\"

extern unsigned char $archive_begin, $archive_end;

unsigned char *${archive_func}_ptr(void)
{
  return &${archive_begin};
}

unsigned char *${archive_func}_end(void)
{
  return &${archive_end};
}

unsigned int ${archive_func}_len(void)
{
  return (&${archive_end} - &${archive_begin});
}
" > $archive_src

echo "Creation of $archive_lds."
echo "\
SECTIONS
{
	.rodata . :
	{
		$archive_begin = .;
		$archive_data(.$archive_name);
		$archive_end = .;
	}
}
" > $archive_lds
